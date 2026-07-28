/*
 * NW_tricky (DLL 0x1A2) - SnowHorn Wastes controller for Tricky.
 *
 * The herding phase directs the matching group-3 objects toward whichever of
 * the player or Tricky is nearer and offers live configured targets to
 * Tricky's play-ball command. The energy phase synchronizes Tricky's talk
 * state with the map-event energy gauge.
 */
#include "dlls/objects/418_NW_tricky.h"

#include "game/objects/object.h"
#include "main/audio/sfx_ids.h"
#include "main/audio/sfx_stop_channel_api.h"
#include "main/dll/dll_00C9_enemy.h"
#include "main/dll/dll_80136a40.h"
#include "main/frame_timing.h"
#include "main/gamebit_ids.h"
#include "main/gamebits_api.h"
#include "main/mapEventTypes.h"
#include "main/obj_group.h"
#include "main/vecmath_distance_api.h"
#include "sys/objects.h"
#include "sys/objects/lifecycle.h"

#define NW_TRICKY_HERDING_COMPLETE_GAMEBIT 0xD11
#define NW_TRICKY_HERD_OBJECT_GROUP        3
#define NW_TRICKY_HERD_OBJECT_SEQUENCE_ID  0x13A

#define NW_TRICKY_SOUND_CHANNEL       16
#define NW_TRICKY_SOUND_VOLUME        0x1000
#define NW_TRICKY_HERD_SOUND_INTERVAL 600.0f

#define NW_TRICKY_PLAY_BALL_COMMAND_ENABLED 1
#define NW_TRICKY_MINIMUM_TARGET_HEALTH     0.0f

#define NW_TRICKY_MINIMUM_ENERGY             4
#define NW_TRICKY_ENERGY_LOW_GAMEBIT_VALUE   1
#define NW_TRICKY_ENERGY_READY_GAMEBIT_VALUE 0xFF
#define NW_TRICKY_ENERGY_UPDATE_INTERVAL     2000.0f

typedef struct NwTrickyPlayBallTargetIdList {
    int ids[NW_TRICKY_PLAY_BALL_TARGET_ID_COUNT];
} NwTrickyPlayBallTargetIdList;

typedef struct NwTrickyCompanionInterface {
    void* unknown00[13];
    void (*commandPlayBall)(GameObject* tricky, int enabled, GameObject* target);
    void* unknown38[2];
    u8 (*isPlayingBall)(GameObject* tricky);
} NwTrickyCompanionInterface;

STATIC_ASSERT(sizeof(NwTrickyPlayBallTargetIdList) == 0x0C);
STATIC_ASSERT(offsetof(NwTrickyCompanionInterface, commandPlayBall) == 0x34);
STATIC_ASSERT(offsetof(NwTrickyCompanionInterface, isPlayingBall) == 0x40);

#define NW_TRICKY_COMPANION_INTERFACE(tricky) ((NwTrickyCompanionInterface*)*(tricky)->anim.dll)

const int gNwTrickyPlayBallTargetIds[NW_TRICKY_PLAY_BALL_TARGET_POOL_SIZE] = {0xF5B, 0x43EC9, 0x43ED6, 0};

int nwTricky_processAnimEvents(GameObject* unusedObj, int unusedArg, ObjAnimUpdateState* unusedAnimUpdate) {
    (void)unusedObj;
    (void)unusedArg;
    (void)unusedAnimUpdate;

    Sfx_StopObjectChannel((u32)getTrickyObject(), NW_TRICKY_SOUND_CHANNEL);
    return 0;
}

int nwTricky_getExtraSize(void) {
    return sizeof(NwTrickyState);
}

void nwTricky_free(GameObject* unusedObj) {
    (void)unusedObj;

    mainSetBits(GAMEBIT_Tricky_Usable, 1);
}

void nwTricky_update(GameObject* obj) {
    int herdObjectCount;
    NwTrickyPlayBallTargetIdList targetIds;
    NwTrickyState* state;
    GameObject* tricky;
    GameObject* player;
    GameObject** herdObjects;
    GameObject** completedHerdScan;
    GameObject** activeHerdScan;
    int* targetId;
    int targetIndex;
    GameObject* target;
    f32 playerDistanceSquared;
    f32 phaseTimer;
    f32 minimumHealth;
    int herdObjectIndex;

    state = obj->extra;
    tricky = getTrickyObject();
    player = Obj_GetPlayerObject();
    targetIds = *(NwTrickyPlayBallTargetIdList*)gNwTrickyPlayBallTargetIds;

    if (tricky == NULL) {
        return;
    }

    switch (state->phase) {
    case NW_TRICKY_PHASE_HERDING:
        if (mainGetBit(NW_TRICKY_HERDING_COMPLETE_GAMEBIT)) {
            herdObjects = (GameObject**)ObjGroup_GetObjects(NW_TRICKY_HERD_OBJECT_GROUP, &herdObjectCount);
            for (herdObjectIndex = 0, completedHerdScan = herdObjects; herdObjectIndex < herdObjectCount;
                 completedHerdScan++, herdObjectIndex++) {
                if ((*completedHerdScan)->anim.seqId == NW_TRICKY_HERD_OBJECT_SEQUENCE_ID) {
                    enemy_setTrackedObj(*completedHerdScan, player);
                }
            }
            mainSetBits(GAMEBIT_Tricky_Usable, 1);
            state->phase = NW_TRICKY_PHASE_ENERGY;
        } else {
            if (mainGetBit(GAMEBIT_ITEM_TrickyStayFind_Got)) {
                if (NW_TRICKY_COMPANION_INTERFACE(tricky)->isPlayingBall(tricky) == 0) {
                    mainSetBits(GAMEBIT_Tricky_Usable, 0);
                    state->phaseTimer = 0.0f;
                }

                for (targetIndex = 0, targetId = targetIds.ids, minimumHealth = NW_TRICKY_MINIMUM_TARGET_HEALTH;
                     targetIndex < NW_TRICKY_PLAY_BALL_TARGET_ID_COUNT; targetId++, targetIndex++) {
                    target = ObjList_FindObjectById(*targetId);
                    if (target != NULL && enemy_getHealthFraction(target) > minimumHealth) {
                        NW_TRICKY_COMPANION_INTERFACE(tricky)->commandPlayBall(
                            tricky, NW_TRICKY_PLAY_BALL_COMMAND_ENABLED, target);
                        break;
                    }
                }

                state->phaseTimer += timeDelta;
                phaseTimer = state->phaseTimer;
                if (phaseTimer >= NW_TRICKY_HERD_SOUND_INTERVAL) {
                    state->phaseTimer = phaseTimer - NW_TRICKY_HERD_SOUND_INTERVAL;
                    trickyTryPlaySound(tricky, SFXwp_rolovr_6, NW_TRICKY_SOUND_VOLUME);
                }
            }

            herdObjects = (GameObject**)ObjGroup_GetObjects(NW_TRICKY_HERD_OBJECT_GROUP, &herdObjectCount);
            for (herdObjectIndex = 0, activeHerdScan = herdObjects; herdObjectIndex < herdObjectCount;
                 activeHerdScan++, herdObjectIndex++) {
                if ((*activeHerdScan)->anim.seqId == NW_TRICKY_HERD_OBJECT_SEQUENCE_ID) {
                    playerDistanceSquared =
                        vec3f_distanceSquared(&(*activeHerdScan)->anim.worldPosX, &player->anim.worldPosX);
                    if (vec3f_distanceSquared(&(*activeHerdScan)->anim.worldPosX, &tricky->anim.worldPosX) <
                        playerDistanceSquared) {
                        enemy_setTrackedObj(*activeHerdScan, tricky);
                    } else {
                        enemy_setTrackedObj(*activeHerdScan, player);
                    }
                }
            }
        }
        break;
    case NW_TRICKY_PHASE_ENERGY:
        if (!(tricky->objectFlags & OBJECT_OBJFLAG_PARENT_SLACK)) {
            state->phaseTimer += timeDelta;
        }
        if (mainGetBit(GAMEBIT_TrickyTalk) == NW_TRICKY_ENERGY_LOW_GAMEBIT_VALUE) {
            if ((*gMapEventInterface)->getTrickyEnergy()[0] >= NW_TRICKY_MINIMUM_ENERGY) {
                mainSetBits(GAMEBIT_TrickyTalk, NW_TRICKY_ENERGY_READY_GAMEBIT_VALUE);
            }
        }
        phaseTimer = state->phaseTimer;
        if (phaseTimer >= NW_TRICKY_ENERGY_UPDATE_INTERVAL) {
            state->phaseTimer = phaseTimer - NW_TRICKY_ENERGY_UPDATE_INTERVAL;
            if (mainGetBit(GAMEBIT_TrickyTalk) == NW_TRICKY_ENERGY_READY_GAMEBIT_VALUE) {
                if ((*gMapEventInterface)->getTrickyEnergy()[0] < NW_TRICKY_MINIMUM_ENERGY) {
                    mainSetBits(GAMEBIT_TrickyTalk, NW_TRICKY_ENERGY_LOW_GAMEBIT_VALUE);
                }
            }
        }
        break;
    }
}

void nwTricky_init(GameObject* obj) {
    obj->animEventCallback = nwTricky_processAnimEvents;
    obj->objectFlags = (u16)(obj->objectFlags | (OBJECT_OBJFLAG_HIDDEN | OBJECT_OBJFLAG_HITDETECT_DISABLED));
}

ObjectDescriptor gNWTrickyObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    0,
    0,
    0,
    (ObjectDescriptorCallback)nwTricky_init,
    (ObjectDescriptorCallback)nwTricky_update,
    0,
    0,
    (ObjectDescriptorCallback)nwTricky_free,
    0,
    (ObjectDescriptorExtraSizeCallback)nwTricky_getExtraSize,
};
