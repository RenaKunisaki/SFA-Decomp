/*
 * ECSH_Creato (DLL 0x191) - ECSH shrine SharpClaw encounter spawner.
 *
 * A game bit triggers its activation effects and starts a countdown. Once the
 * countdown expires, the creator spawns object ID 0x11, which retail
 * OBJINDEX.bin maps to "sharpclawGr" (DLL 0xC9).
 */
#include "dlls/objects/401_ECSH_Creato.h"

#include "game/objects/object.h"
#include "main/audio/sfx_play_api.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/dll/baddie_state.h"
#include "main/dll/foodbag.h"
#include "main/frame_timing.h"
#include "main/gamebits_api.h"
#include "main/mm.h"
#include "main/object_render.h"
#include "main/resource.h"
#include "sys/objects.h"
#include "sys/objects/lifecycle.h"

typedef struct ECSHCreatorSharpClawSetup {
    ObjPlacement base;
    s16 gameBit;
    s16 gameBit2;
    u8 unknown1C[0x1E - 0x1C];
    s16 unknown1E;
    s16 unknown20;
    s16 droppedItemId;
    s16 unknown24;
    u8 unknown26;
    s8 initialWeaponId;
    u8 objectFlagBits;
    u8 aggroRangeByte;
    s8 initialYaw;
    u8 flags;
    s16 respawnEnabled;
    s8 triggerSequenceId;
    u8 healthByte;
    s16 unknown30;
    u8 hitPoints;
    u8 unknown33;
    u16 unknown34;
    u8 unknown36[0x38 - 0x36];
} ECSHCreatorSharpClawSetup;

STATIC_ASSERT(sizeof(ECSHCreatorSharpClawSetup) == 0x38);
STATIC_ASSERT(offsetof(ECSHCreatorSharpClawSetup, base) == 0x00);
STATIC_ASSERT(offsetof(ECSHCreatorSharpClawSetup, gameBit) == 0x18);
STATIC_ASSERT(offsetof(ECSHCreatorSharpClawSetup, gameBit2) == 0x1A);
STATIC_ASSERT(offsetof(ECSHCreatorSharpClawSetup, unknown1C) == 0x1C);
STATIC_ASSERT(offsetof(ECSHCreatorSharpClawSetup, unknown1E) == 0x1E);
STATIC_ASSERT(offsetof(ECSHCreatorSharpClawSetup, unknown20) == 0x20);
STATIC_ASSERT(offsetof(ECSHCreatorSharpClawSetup, droppedItemId) == 0x22);
STATIC_ASSERT(offsetof(ECSHCreatorSharpClawSetup, unknown24) == 0x24);
STATIC_ASSERT(offsetof(ECSHCreatorSharpClawSetup, unknown26) == 0x26);
STATIC_ASSERT(offsetof(ECSHCreatorSharpClawSetup, initialWeaponId) == 0x27);
STATIC_ASSERT(offsetof(ECSHCreatorSharpClawSetup, objectFlagBits) == 0x28);
STATIC_ASSERT(offsetof(ECSHCreatorSharpClawSetup, aggroRangeByte) == 0x29);
STATIC_ASSERT(offsetof(ECSHCreatorSharpClawSetup, initialYaw) == 0x2A);
STATIC_ASSERT(offsetof(ECSHCreatorSharpClawSetup, flags) == 0x2B);
STATIC_ASSERT(offsetof(ECSHCreatorSharpClawSetup, respawnEnabled) == 0x2C);
STATIC_ASSERT(offsetof(ECSHCreatorSharpClawSetup, triggerSequenceId) == 0x2E);
STATIC_ASSERT(offsetof(ECSHCreatorSharpClawSetup, healthByte) == 0x2F);
STATIC_ASSERT(offsetof(ECSHCreatorSharpClawSetup, unknown30) == 0x30);
STATIC_ASSERT(offsetof(ECSHCreatorSharpClawSetup, hitPoints) == 0x32);
STATIC_ASSERT(offsetof(ECSHCreatorSharpClawSetup, unknown33) == 0x33);
STATIC_ASSERT(offsetof(ECSHCreatorSharpClawSetup, unknown34) == 0x34);
STATIC_ASSERT(offsetof(ECSHCreatorSharpClawSetup, unknown36) == 0x36);

#define ECSH_CREATOR_EFFECT_RESOURCE_ID  0x82
#define ECSH_CREATOR_SHARPCLAW_OBJECT_ID 0x11

#define ECSH_CREATOR_SPAWN_TIMER           100
#define ECSH_CREATOR_SHARPCLAW_HIT_POINTS_BASE 2

#define ECSH_CREATOR_SHARPCLAW_INITIAL_WEAPON_ID       3
#define ECSH_CREATOR_SHARPCLAW_FLAGS                   2
#define ECSH_CREATOR_SHARPCLAW_DISABLE_CAMERA_TARGET   0x20
#define ECSH_CREATOR_SHARPCLAW_AGGRO_RANGE_BYTE        0xFF
#define ECSH_CREATOR_SHARPCLAW_INVALID_DROPPED_ITEM_ID -1
#define ECSH_CREATOR_SHARPCLAW_NO_TRIGGER_SEQUENCE     -1

#define ECSH_CREATOR_SETUP_ALLOC_TYPE  0xE
#define ECSH_CREATOR_SETUP_ALLOC_FLAGS 0
#define ECSH_CREATOR_CHILD_SETUP_FLAGS 5
#define ECSH_CREATOR_NO_MAP_ID         -1
#define ECSH_CREATOR_NO_OBJECT_INDEX   -1

#define ECSH_CREATOR_INITIAL_YAW_SHIFT 8
#define ECSH_CREATOR_FULL_ALPHA        0xFF

int ecshCreator_getExtraSize(void) {
    return sizeof(ECSHCreatorState);
}

int ecshCreator_getObjectTypeId(void) {
    return 0;
}

void ecshCreator_free(void) {
}

void ecshCreator_render(GameObject* obj, int renderArg2, int renderArg3, int renderArg4, int renderArg5, s8 visible) {
    s32 isVisible;

    isVisible = visible;
    if (isVisible != 0) {
        objRenderModelAndHitVolumes(obj, renderArg2, renderArg3, renderArg4, renderArg5, 1.0f);
    }
}

void ecshCreator_hitDetect(void) {
}

void ecshCreator_update(GameObject* obj) {
    const ECSHCreatorPlacement* placement;
    ECSHCreatorState* state;
    Dll82Interface** effectResource;
    ECSHCreatorSharpClawSetup* spawnSetup;
    GameObject* sharpClaw;

    placement = (const ECSHCreatorPlacement*)obj->anim.placementData;
    state = obj->extra;
    if (obj->userData2 == 0 && (u32)mainGetBit(state->triggerGameBit) != 0) {
        effectResource = Resource_Acquire(ECSH_CREATOR_EFFECT_RESOURCE_ID, 1);
        (*effectResource)->spawn(obj, 0, NULL, 1, -1, NULL);
        (*effectResource)->spawn(obj, 1, NULL, 1, -1, NULL);
        Sfx_PlayFromObject((u32)obj, SFXTRIG_wp_hitpos_6);
        Resource_Release(effectResource);
        state->spawnTimerRate = 1;
        obj->userData2 = 1;
    }
    if (state->spawnTimerRate != 0) {
        state->spawnTimer -= state->spawnTimerRate * framesThisStep;
    }
    if (Obj_IsLoadingLocked() != 0 && state->spawnTimer <= 0) {
        spawnSetup =
            mmAlloc(sizeof(ECSHCreatorSharpClawSetup), ECSH_CREATOR_SETUP_ALLOC_TYPE, ECSH_CREATOR_SETUP_ALLOC_FLAGS);
        spawnSetup->base.posX = placement->base.posX;
        spawnSetup->base.posY = placement->base.posY;
        spawnSetup->base.posZ = placement->base.posZ;
        spawnSetup->base.objectId = ECSH_CREATOR_SHARPCLAW_OBJECT_ID;
        spawnSetup->base.mapId = ECSH_CREATOR_NO_MAP_ID;
        spawnSetup->base.color[0] = placement->base.color[0];
        spawnSetup->base.color[1] = placement->base.color[1];
        spawnSetup->base.color[2] = placement->base.color[2];
        spawnSetup->base.color[3] = placement->base.color[3];
        spawnSetup->initialWeaponId = ECSH_CREATOR_SHARPCLAW_INITIAL_WEAPON_ID;
        spawnSetup->objectFlagBits = 0;
        spawnSetup->gameBit = state->triggerGameBit + placement->childGameBitOffset;
        spawnSetup->unknown30 = -1;
        spawnSetup->initialYaw = (s8)(obj->anim.rotX >> ECSH_CREATOR_INITIAL_YAW_SHIFT);
        spawnSetup->flags = ECSH_CREATOR_SHARPCLAW_FLAGS;
        spawnSetup->unknown20 = 0;
        spawnSetup->unknown1E = 0;
        spawnSetup->droppedItemId = ECSH_CREATOR_SHARPCLAW_INVALID_DROPPED_ITEM_ID;
        spawnSetup->aggroRangeByte = ECSH_CREATOR_SHARPCLAW_AGGRO_RANGE_BYTE;
        spawnSetup->triggerSequenceId = ECSH_CREATOR_SHARPCLAW_NO_TRIGGER_SEQUENCE;
        spawnSetup->unknown24 = 0;
        spawnSetup->respawnEnabled = 0;
        spawnSetup->unknown34 = 0xFFFF;
        spawnSetup->gameBit2 = 0;
        spawnSetup->hitPoints = state->sharpClawHitPoints;
        sharpClaw = Obj_SetupObject(&spawnSetup->base, ECSH_CREATOR_CHILD_SETUP_FLAGS, obj->anim.mapEventSlot,
                                    ECSH_CREATOR_NO_OBJECT_INDEX, obj->anim.parent);
        if (sharpClaw != NULL) {
            ((GroundBaddieState*)sharpClaw->extra)->configFlags = ECSH_CREATOR_SHARPCLAW_DISABLE_CAMERA_TARGET;
        }
        state->spawnTimer = ECSH_CREATOR_SPAWN_TIMER;
        state->spawnTimerRate = 0;
    }
}

void ecshCreator_init(GameObject* obj, const ECSHCreatorPlacement* placement) {
    ECSHCreatorState* state = obj->extra;

    obj->anim.rotX = (s16)((s32)placement->initialYaw << ECSH_CREATOR_INITIAL_YAW_SHIFT);
    obj->userData2 = 0;
    state->spawnTimer = ECSH_CREATOR_SPAWN_TIMER;
    state->spawnTimerRate = 0;
    obj->anim.renderAlpha = ECSH_CREATOR_FULL_ALPHA;
    obj->anim.alpha = ECSH_CREATOR_FULL_ALPHA;
    state->triggerGameBit = placement->triggerGameBit;
    state->sharpClawHitPoints = ECSH_CREATOR_SHARPCLAW_HIT_POINTS_BASE;
    state->sharpClawHitPoints += placement->hitPointsOffset;
}

void ecshCreator_release(void) {
}

void ecshCreator_initialise(void) {
}

ObjectDescriptor gECSHCreatorObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)ecshCreator_initialise,
    (ObjectDescriptorCallback)ecshCreator_release,
    0,
    (ObjectDescriptorCallback)ecshCreator_init,
    (ObjectDescriptorCallback)ecshCreator_update,
    (ObjectDescriptorCallback)ecshCreator_hitDetect,
    (ObjectDescriptorCallback)ecshCreator_render,
    (ObjectDescriptorCallback)ecshCreator_free,
    (ObjectDescriptorCallback)ecshCreator_getObjectTypeId,
    ecshCreator_getExtraSize,
};
