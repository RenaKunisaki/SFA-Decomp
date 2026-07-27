/*
 * Warp-pad object: handles per-placement travel, progression gates, sequence
 * events, and environment restoration.
 */
#include "dlls/objects/300_Transporter.h"

#include "game/objects/object.h"
#include "main/audio/sfx_play_api.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/gamebits.h"
#include "main/lightmap_render_control_api.h"
#include "main/loaded_file_flags.h"
#include "main/map_load.h"
#include "main/mapEventTypes.h"
#include "main/objprint_render_api.h"
#include "main/pi_dolphin_api.h"
#include "main/rcp_dolphin_api.h"
#include "main/render_envfx_api.h"
#include "main/sky_api.h"

/* Environment effects restored by animation event 8. */
#define TRANSPORTER_ENVFX_G0_A 0x224 /* mapIds 0x43F83 / 0x4977D */
#define TRANSPORTER_ENVFX_G0_B 0x223
#define TRANSPORTER_ENVFX_ENV  0x22E /* shared by G0 and G1 */
#define TRANSPORTER_ENVFX_SKY  0x218 /* shared by G0 and G1 */
#define TRANSPORTER_ENVFX_G1_A 0x217 /* mapIds 0x48506 / 0x4A533 */
#define TRANSPORTER_ENVFX_G1_B 0x216
#define TRANSPORTER_ENVFX_G1_C 0x84
#define TRANSPORTER_ENVFX_G1_D 0x8A
#define TRANSPORTER_ENVFX_G2_A 0x23A /* mapIds 0x4B666 / 0x4B667 */
#define TRANSPORTER_ENVFX_G2_B 0x23B
#define TRANSPORTER_ENVFX_G2_C 0x23E
#define TRANSPORTER_ENVFX_G3_A 0x247 /* mapIds 0x4670D / 0x4827E / 0x49267 */
#define TRANSPORTER_ENVFX_G3_B 0x248
#define TRANSPORTER_ENVFX_G4_A 0x238 /* mapId 0x4CB6A */
#define TRANSPORTER_ENVFX_G4_B 0x239

#define TRANSPORTER_ACTIVATE_DELAY 400
#define TRANSPORTER_ROTATION_SHIFT 8

typedef enum TransporterSequenceEvent {
    TRANSPORTER_EVENT_WARP = 1,
    TRANSPORTER_EVENT_MAP_PROGRESS = 2,
    TRANSPORTER_EVENT_UNLOCK_LEVEL = 3,
    TRANSPORTER_EVENT_LOAD_BLOCKS = 5,
    TRANSPORTER_EVENT_CLEAR_BLOCKS = 6,
    TRANSPORTER_EVENT_PULSE_FX = 7,
    TRANSPORTER_EVENT_RESTORE_ENVIRONMENT = 8,
} TransporterSequenceEvent;

extern s16 lbl_803DCEB8;

ObjectDescriptor gTransporterObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    0,
    0,
    0,
    (ObjectDescriptorCallback)Transporter_init,
    (ObjectDescriptorCallback)Transporter_update,
    (ObjectDescriptorCallback)Transporter_hitDetect,
    (ObjectDescriptorCallback)Transporter_render,
    0,
    0,
    Transporter_getExtraSize,
};

int Transporter_sequenceCallback(GameObject* obj, int unused, ObjAnimUpdateState* animUpdate) {
    int i;
    TransporterPlacement* placement = (TransporterPlacement*)obj->anim.placementData;
    TransporterState* state = obj->extra;
    int id;

    for (i = 0; i < animUpdate->eventCount; i++) {
        switch (animUpdate->eventIds[i]) {
        case TRANSPORTER_EVENT_PULSE_FX:
            state->flags |= TRANSPORTER_FLAG_PULSE_FX;
            Sfx_PlayFromObject((u32)obj, SFXTRIG_id_420);
            break;
        case TRANSPORTER_EVENT_MAP_PROGRESS:
            id = placement->base.mapId;
            switch (id) {
            case 0x49C33:
                mainSetBits(GAMEBIT_SH_WarpStoneRelated0884, 1);
                (*gMapEventInterface)->setObjGroupStatus(7, 0, 1);
                (*gMapEventInterface)->setObjGroupStatus(7, 2, 1);
                (*gMapEventInterface)->setObjGroupStatus(7, 3, 1);
                (*gMapEventInterface)->setObjGroupStatus(7, 7, 1);
                (*gMapEventInterface)->setObjGroupStatus(7, 10, 1);
                (*gMapEventInterface)->setObjGroupStatus(10, 7, 0);
            case 0x48506:
            case 0x4977D:
                loadMapAndParent(7);
                unlockLevel(0, 0, 1);
                lockLevel(mapGetDirIdx(7), 1);
                break;
            case 0x43F83:
                loadMapAndParent(0x21);
                lockLevel(mapGetDirIdx(0x21), 1);
                break;
            case 0x4A533:
                loadMapAndParent(0x28);
                lockLevel(mapGetDirIdx(0x28), 1);
                break;
            case 0xC5D:
                unlockLevel(mapGetDirIdx(0x21), 1, 0);
                break;
            case 0x47064:
                loadMapAndParent(0x1C);
                lockLevel(mapGetDirIdx(0x1C), 1);
                lockLevel(mapGetDirIdx(0x1B), 0);
                break;
            case 0x4800C:
                loadMapAndParent(0x22);
                lockLevel(mapGetDirIdx(0xD), 0);
                lockLevel(mapGetDirIdx(0x22), 1);
                break;
            case 0x48018:
                unlockLevel(mapGetDirIdx(0x22), 1, 0);
                mainSetBits(GAMEBIT_WC_ObjGroups, 0);
                (*gMapEventInterface)->setObjGroupStatus(0xD, 0, 1);
                (*gMapEventInterface)->setObjGroupStatus(0xD, 1, 1);
                (*gMapEventInterface)->setObjGroupStatus(0xD, 5, 1);
                (*gMapEventInterface)->setObjGroupStatus(0xD, 10, 1);
                (*gMapEventInterface)->setObjGroupStatus(0xD, 0xB, 1);
                mainSetBits(GAMEBIT_WC_MagicCaveRelated0E05, 0);
                break;
            case 0x45DD6:
                unlockLevel(0, 0, 1);
                lockLevel(mapGetDirIdx(4), 0);
                break;
            case 0x2BA7:
                unlockLevel(0, 0, 1);
                lockLevel(mapGetDirIdx(0x12), 0);
                lockLevel(mapGetDirIdx(0x1F), 1);
                loadMapAndParent(0x1F);
                break;
            case 0x46A40:
                unlockLevel(0, 0, 1);
                lockLevel(mapGetDirIdx(0xE), 0);
                lockLevel(mapGetDirIdx(0x20), 1);
                loadMapAndParent(0x20);
                break;
            case 0x4B666:
                unlockLevel(0, 0, 1);
                lockLevel(mapGetDirIdx(0x32), 0);
                lockLevel(mapGetDirIdx(0x15), 1);
                loadMapAndParent(0x15);
                break;
            case 0x497F4:
                unlockLevel(0, 0, 1);
                lockLevel(mapGetDirIdx(10), 0);
                lockLevel(mapGetDirIdx(0x27), 1);
                loadMapAndParent(0x27);
                break;
            case 0x4CDE6:
                unlockLevel(0, 0, 1);
                lockLevel(mapGetDirIdx(10), 0);
                break;
            }
            break;
        case TRANSPORTER_EVENT_UNLOCK_LEVEL:
            switch (placement->base.mapId) {
            case 0x47064:
                unlockLevel(0, 0, 1);
                break;
            }
            break;
        case TRANSPORTER_EVENT_LOAD_BLOCKS:
            switch (placement->base.mapId) {
            case 0x47064:
                setLoadedFileFlags_blocks1();
                break;
            }
            break;
        case TRANSPORTER_EVENT_CLEAR_BLOCKS:
            switch (placement->base.mapId) {
            case 0x47064:
                clearLoadedFileFlags_blocks1();
                break;
            }
            break;
        case TRANSPORTER_EVENT_WARP:
            switch (placement->base.mapId) {
            case 0x47064:
                clearLoadedFileFlags_blocks1();
                break;
            }
            warpToMap(placement->warpId, 0);
            break;
        case TRANSPORTER_EVENT_RESTORE_ENVIRONMENT:
            id = placement->base.mapId;
            switch (id) {
            case 0x43F83:
            case 0x4977D:
                getEnvfxActImmediately(obj, obj, TRANSPORTER_ENVFX_G0_A, 0);
                getEnvfxActImmediately(obj, obj, TRANSPORTER_ENVFX_G0_B, 0);
                getEnvfxActImmediately(obj, obj, TRANSPORTER_ENVFX_ENV, 0);
                getEnvfxActImmediately(obj, obj, TRANSPORTER_ENVFX_SKY, 0);
                setDrawCloudsAndLights(0);
                skyFn_80088c94(1, 1);
                skyFn_80088e54(0, gTransporterZero[0]);
                break;
            case 0x48506:
            case 0x4A533:
                getEnvfxActImmediately(obj, obj, TRANSPORTER_ENVFX_G1_A, 0);
                getEnvfxActImmediately(obj, obj, TRANSPORTER_ENVFX_G1_B, 0);
                getEnvfxActImmediately(obj, obj, TRANSPORTER_ENVFX_ENV, 0);
                getEnvfxActImmediately(obj, obj, TRANSPORTER_ENVFX_SKY, 0);
                setDrawCloudsAndLights(1);
                getEnvfxActImmediately(obj, obj, TRANSPORTER_ENVFX_G1_C, 0);
                getEnvfxActImmediately(obj, obj, TRANSPORTER_ENVFX_G1_D, 0);
                skyFn_80088c94(1, 0);
                skyFn_80088e54(0, gTransporterZero[0]);
                break;
            case 0x4B666:
                getEnvfxActImmediately(obj, obj, TRANSPORTER_ENVFX_G2_A, 0);
                getEnvfxActImmediately(obj, obj, TRANSPORTER_ENVFX_G2_B, 0);
                break;
            case 0x4B667:
                getEnvfxActImmediately(obj, obj, TRANSPORTER_ENVFX_G2_A, 0);
                getEnvfxActImmediately(obj, obj, TRANSPORTER_ENVFX_G2_B, 0);
                (*gMapEventInterface)->setObjGroupStatus(0x15, 2, 1);
                getEnvfxActImmediately(0, 0, TRANSPORTER_ENVFX_G2_C, 0);
                skyFn_80088e54(1, gTransporterZero[0]);
                break;
            case 0x4670D:
            case 0x4827E:
            case 0x49267:
                getEnvfxActImmediately(obj, obj, TRANSPORTER_ENVFX_G3_A, 0);
                getEnvfxActImmediately(obj, obj, TRANSPORTER_ENVFX_G3_B, 0);
                Rcp_DisableHeatEffect();
                mainSetBits(GAMEBIT_VFP_EnvironmentRelated0EF6, 1);
                break;
            case 0x4CB6A:
                getEnvfxActImmediately(obj, obj, TRANSPORTER_ENVFX_G4_A, 0);
                getEnvfxActImmediately(obj, obj, TRANSPORTER_ENVFX_G4_B, 0);
                skyFn_80088c94(1, 1);
                skyFn_80088e54(0, gTransporterZero[0]);
            case 0x4CB84:
                mainSetBits(GAMEBIT_VFP_EnvironmentRelated0EF6, 0);
                break;
            }
            break;
        }
    }
    Transporter_updateEffects(obj);
    return 0;
}

int Transporter_getExtraSize(void) {
    return sizeof(TransporterState);
}

void Transporter_render(void) {
}

void Transporter_hitDetect(int obj) {
    register int self = obj;
    register TransporterPlacement* placement = (TransporterPlacement*)((GameObject*)self)->anim.placementData;
    register TransporterState* state = ((GameObject*)self)->extra;

    if (lbl_803DCEB8 > -1) {
        ((GameObject*)self)->anim.resetHitboxFlags &= ~(INTERACT_FLAG_DISABLED | INTERACT_FLAG_PROMPT_SUPPRESSED);
        state->flags |= TRANSPORTER_FLAG_INTERACTIVE;
        if (((GameObject*)self)->anim.hitVolumeTransforms != NULL) {
            objRenderFn_80041018((GameObject*)self);
        }
        return;
    }

    if (placement->warpId != TRANSPORTER_WARP_ID_NONE && (state->flags & TRANSPORTER_FLAG_DISABLED) == 0) {
        if (state->triggerMode != 0 || state->countdownActive != 0) {
            ((GameObject*)self)->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
            state->flags &= ~TRANSPORTER_FLAG_INTERACTIVE;
        } else if (placement->enableGameBit != TRANSPORTER_GAME_BIT_NONE && mainGetBit(placement->enableGameBit) == 0) {
            ((GameObject*)self)->anim.resetHitboxFlags &= ~INTERACT_FLAG_DISABLED;
            ((GameObject*)self)->anim.resetHitboxFlags |= INTERACT_FLAG_PROMPT_SUPPRESSED;
            state->flags &= ~TRANSPORTER_FLAG_INTERACTIVE;
        } else {
            ((GameObject*)self)->anim.resetHitboxFlags &= ~(INTERACT_FLAG_DISABLED | INTERACT_FLAG_PROMPT_SUPPRESSED);
            state->flags |= TRANSPORTER_FLAG_INTERACTIVE;
        }
        if (((GameObject*)self)->anim.hitVolumeTransforms != NULL) {
            objRenderFn_80041018((GameObject*)self);
        }
        return;
    }

    if ((state->flags & TRANSPORTER_FLAG_WARP_A) != 0) {
        ((GameObject*)self)->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
    } else {
        ((GameObject*)self)->anim.resetHitboxFlags &= ~INTERACT_FLAG_DISABLED;
        ((GameObject*)self)->anim.resetHitboxFlags |= INTERACT_FLAG_PROMPT_SUPPRESSED;
    }
    state->flags &= ~TRANSPORTER_FLAG_INTERACTIVE;
}

void Transporter_update(GameObject* obj) {
    register int self = (int)obj;
    register TransporterPlacement* placement = (TransporterPlacement*)((GameObject*)self)->anim.placementData;
    if (placement->warpId != TRANSPORTER_WARP_ID_NONE) {
        Transporter_updateInteraction((GameObject*)self);
    }
    Transporter_updateEffects((GameObject*)self);
}

void Transporter_init(GameObject* obj, TransporterPlacement* placement) {
    TransporterState* state;
    int id;

    state = obj->extra;
    state->activateDelay = TRANSPORTER_ACTIVATE_DELAY;
    state->flags = 0;
    obj->anim.rotX = (s16)((u16)(placement->rotXHigh << TRANSPORTER_ROTATION_SHIFT));
    obj->userData1 = 0;
    obj->animEventCallback = Transporter_sequenceCallback;
    obj->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;

    id = placement->base.mapId;
    switch (id) {
    case 0x4670D:
    case 0x4827E:
    case 0x49267:
    case 0x4CB6A:
    case 0x4CB84:
        state->flags |= TRANSPORTER_FLAG_WARP_A | TRANSPORTER_FLAG_DISABLED | TRANSPORTER_FLAG_WARP_B;
        break;
    case 0x48506:
    case 0x45753:
    case 0x463C0:
    case 0x45DD6:
    case 0x4977D:
    case 0x49C33:
    case 0x4B666:
    case 0x4B667:
        state->flags |= TRANSPORTER_FLAG_WARP_B;
        break;
    case 0x4C986:
        state->flags |= TRANSPORTER_FLAG_DISABLED | TRANSPORTER_FLAG_WARP_C;
        break;
    case 0x47064:
        state->flags |= TRANSPORTER_FLAG_WARP_C;
        break;
    case 0x43F83:
        if (mainGetBit(GAMEBIT_K1_SPIRIT_COLLECTED) != 0 || mainGetBit(GAMEBIT_K1_SPIRIT_DEPOSITED) != 0 ||
            mainGetBit(GAMEBIT_K1_ReturnPadGuard) != 0) {
            state->flags |= TRANSPORTER_FLAG_DISABLED;
        }
        break;
    case 0x2BA7:
        if (mainGetBit(GAMEBIT_ITEM_TestCombatSpirit_Got) != 0 || mainGetBit(GAMEBIT_ITEM_Spirit2_Used) != 0 ||
            mainGetBit(GAMEBIT_WM_SpiritPlace2Ready) != 0) {
            state->flags |= TRANSPORTER_FLAG_DISABLED;
        }
        break;
    case 0x46A40:
        if (mainGetBit(GAMEBIT_ITEM_SpiritTestFear_Got) != 0 || mainGetBit(GAMEBIT_ITEM_Spirit3_Released) != 0 ||
            mainGetBit(GAMEBIT_WM_SpiritPlace3Ready) != 0) {
            state->flags |= TRANSPORTER_FLAG_DISABLED;
        }
        break;
    case 0x497F4:
        if (mainGetBit(GAMEBIT_ITEM_SpiritTestStrength_Got) != 0 || mainGetBit(GAMEBIT_ITEM_Spirit4_Used) != 0 ||
            mainGetBit(GAMEBIT_WM_SpiritPlace4Ready) != 0) {
            state->flags |= TRANSPORTER_FLAG_DISABLED;
        }
        break;
    case 0x4800C:
        if (mainGetBit(GAMEBIT_ITEM_Spirit5_Got) != 0 || mainGetBit(GAMEBIT_ITEM_Spirit5_Released) != 0 ||
            mainGetBit(GAMEBIT_WM_SpiritPlace5Ready) != 0) {
            state->flags |= TRANSPORTER_FLAG_DISABLED;
        }
        break;
    case 0x4A533:
        if (mainGetBit(GAMEBIT_ITEM_Spirit6_Got) != 0 || mainGetBit(GAMEBIT_ITEM_Spirit6_Released) != 0 ||
            mainGetBit(GAMEBIT_WM_SpiritPlace6Ready) != 0) {
            state->flags |= TRANSPORTER_FLAG_DISABLED;
        }
        break;
    }

    if ((state->flags & TRANSPORTER_FLAG_WARP_A) != 0) {
        obj->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
    }
}
