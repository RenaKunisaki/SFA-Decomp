/*
 * SH_swapston (DLL 0x1B0) - the talking WarpStone hub object.
 *
 * It runs the WarpStone's idle/look-at-target animation behaviour,
 * drives the warp menu sequence that lets the player pick a destination,
 * and renders the player model standing on the stone during the menu.
 */

#include "dlls/objects/432_SH_swapston.h"

#include "dolphin/pad.h"
#include "main/audio/audio_control_api.h"
#include "main/audio/sfx_play_api.h"
#include "main/audio/sfx_position_api.h"
#include "main/audio/sfx_stop_object_api.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/audio/stream_api.h"
#include "main/dll/dll_0000_gameui_api.h"
#include "main/dll/partfx_interface.h"
#include "main/dll/player_api.h"
#include "main/dll/player_spirit_api.h"
#include "main/frame_timing.h"
#include "main/gamebit_ids.h"
#include "main/gamebits_api.h"
#include "main/maketex_random_api.h"
#include "main/maketex_sequence_api.h"
#include "main/mapEvent.h"
#include "main/map_load.h"
#include "main/model_engine.h"
#include "main/model_engine_ui_api.h"
#include "main/object_render.h"
#include "main/obj_group.h"
#include "main/obj_link.h"
#include "main/obj_path.h"
#include "main/obj_query.h"
#include "main/objanim_update.h"
#include "main/objfx.h"
#include "main/objhits.h"
#include "main/objprint_anim_api.h"
#include "main/objprint_api.h"
#include "main/objprint_character_api.h"
#include "main/pad.h"
#include "main/pi_dolphin_api.h"
#include "main/rcp_dolphin_api.h"
#include "main/shader_api.h"
#include "main/textrender_api.h"
#include "main/vecmath.h"
#include "sys/objects.h"
#include "sys/objects/lifecycle.h"

union SClanternAnimEvents {
    ObjAnimEventList list;
    u8 pad[0x20];
} gSClanternObjAnimEvents;
extern int lbl_803DC050;

/*
 * scchieflightfoot - Thorntail dust/sand effect spawner.
 *
 * Provides SHthorntail_updateDustEffects, called by the WarpStone sequence
 * handler. While the runtime's dust state is ACTIVE, the free-running
 * state->dustEffectTimer advances by timeDelta each frame and sweeps through
 * phases keyed off the
 * tuning thresholds in .sdata2 (0, 120, 360, 420, 480 frames):
 *   - rising:  randomly emit small dust puffs (effect 0x7ca)
 *   - 120..360: also emit a growing ground cloud (0x7d2) and arm the burst
 *   - 360..420: on the armed burst, emit 15 large cloud puffs
 *   - 420..480: hold
 *   - >=480:    reset the timer and clear the ACTIVE flag
 * Spawn probability is gated by randomGetRange against the timer scaled by
 * the tuning floats. All effects are parented to the player object.
 */

typedef struct SHthorntailDustEffectParams {
    s16 flags;
    s16 count;
    s16 effectType;
    s16 radius;
    f32 scale;
    Vec position;
} SHthorntailDustEffectParams;

#define DUST_PUFF_EFFECT_ID             0x7ca
#define DUST_CLOUD_EFFECT_ID            0x7d2
#define DUST_PUFF_PARAM_TYPE            0xc0e
#define DUST_SPAWN_CHANCE_RANGE         0x1e0
#define DUST_BURST_PUFF_COUNT           0xf
#define WARPSTONE_DUST_FLAG_BURST_READY 0x02
#define WARPSTONE_DUST_FLAG_ACTIVE      0x04

ObjectDescriptor gWarpStoneObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)warpstone_initialise,
    (ObjectDescriptorCallback)warpstone_release,
    0,
    (ObjectDescriptorCallback)warpstone_init,
    (ObjectDescriptorCallback)warpstone_update,
    (ObjectDescriptorCallback)warpstone_hitDetect,
    (ObjectDescriptorCallback)warpstone_render,
    (ObjectDescriptorCallback)warpstone_free,
    (ObjectDescriptorCallback)warpstone_getObjectTypeId,
    warpstone_getExtraSize,
};

void SHthorntail_updateDustEffects(GameObject* obj) {
    void* playerObj;
    WarpStoneState* state;
    int burstCount;
    SHthorntailDustEffectParams effectParams;

    playerObj = Obj_GetPlayerObject();
    state = obj->extra;
    effectParams.position.x = 0.0f;
    effectParams.position.y = 55.0f;
    effectParams.position.z = 0.0f;
    effectParams.effectType = DUST_PUFF_PARAM_TYPE;
    effectParams.count = 1;
    if ((state->dustEffectFlags & WARPSTONE_DUST_FLAG_ACTIVE) != 0) {
        if (state->dustEffectTimer < 120.0f) {
            if ((f32)(s32)randomGetRange(0, DUST_SPAWN_CHANCE_RANGE) < state->dustEffectTimer / 2.0f) {
                (*gPartfxInterface)->spawnObject(playerObj, DUST_PUFF_EFFECT_ID, &effectParams, 2, -1, NULL);
            }
        } else if (state->dustEffectTimer < 360.0f) {
            if ((f32)(s32)randomGetRange(0, DUST_SPAWN_CHANCE_RANGE) < state->dustEffectTimer / 3.0f) {
                (*gPartfxInterface)->spawnObject(playerObj, DUST_PUFF_EFFECT_ID, &effectParams, 2, -1, NULL);
            }
            effectParams.radius = 0x28;
            effectParams.flags = 0;
            effectParams.scale = 0.0009f * ((state->dustEffectTimer - 120.0f) / 240.0f);
            (*gPartfxInterface)->spawnObject(playerObj, DUST_CLOUD_EFFECT_ID, &effectParams, 2, -1, NULL);
            state->dustEffectFlags = state->dustEffectFlags | WARPSTONE_DUST_FLAG_BURST_READY;
        } else if (state->dustEffectTimer < 420.0f) {
            if ((f32)(s32)randomGetRange(0, DUST_SPAWN_CHANCE_RANGE) < state->dustEffectTimer / 2.0f) {
                (*gPartfxInterface)->spawnObject(playerObj, DUST_PUFF_EFFECT_ID, &effectParams, 2, -1, NULL);
            }
            if ((state->dustEffectFlags & WARPSTONE_DUST_FLAG_BURST_READY) != 0) {
                state->dustEffectFlags = state->dustEffectFlags & ~WARPSTONE_DUST_FLAG_BURST_READY;
                effectParams.radius = 0x46;
                effectParams.scale = 0.00036f;
                for (burstCount = DUST_BURST_PUFF_COUNT; (u8)burstCount != 0; burstCount--) {
                    (*gPartfxInterface)->spawnObject(playerObj, DUST_CLOUD_EFFECT_ID, &effectParams, 2, -1, NULL);
                }
            }
        } else {
            if (state->dustEffectTimer < 480.0f) {
            } else {
                state->dustEffectTimer = 0.0f;
                state->dustEffectFlags = state->dustEffectFlags & ~WARPSTONE_DUST_FLAG_ACTIVE;
            }
        }
        state->dustEffectTimer = state->dustEffectTimer + timeDelta;
    }
}

/*
 * sclantern - hanging lantern objects used in SharpClaw-themed areas.
 * SClantern_advanceAnimEvents drives the animation each frame: it fires
 * spark particle SFX at left/right attachment points (path points 0 and 1)
 * on events 1-4, and plays a swing SFX on event 9. Sparks are suppressed
 * during the early frames of move SCLANTERN_SPARK_SUPPRESS_MOVE (0x1b).
 * playerFn_801d6d58 probes the current player's anim-state flags.
 */

#define SCLANTERN_EVENT_LEFT_SPARK_A  1
#define SCLANTERN_EVENT_RIGHT_SPARK_A 2
#define SCLANTERN_EVENT_LEFT_SPARK_B  3
#define SCLANTERN_EVENT_RIGHT_SPARK_B 4
#define SCLANTERN_EVENT_LANTERN_SWING 9
#define SCLANTERN_SPARK_SFX_ID        0x415
#define SCLANTERN_SPARK_SUPPRESS_MOVE 0x1b

u32 SClantern_advanceAnimEvents(int obj, f32 moveStepScale) {
    u32 advanceResult;
    GameObject* lantern;
    int pointIndex;
    int i;
    float posZ;
    float posY;
    float posX;

    pointIndex = 0;
    lantern = (GameObject*)obj;
    gSClanternObjAnimEvents.list.triggerCount = 0;
    gSClanternObjAnimEvents.list.rootCurveValid = 0;
    advanceResult = ObjAnim_AdvanceCurrentMove((int)obj, moveStepScale, timeDelta, &gSClanternObjAnimEvents.list);
    if (gSClanternObjAnimEvents.list.rootCurveValid != 0) {
        lantern->anim.rotX += gSClanternObjAnimEvents.list.rootPitch;
    }
    i = 0;
    while (i < gSClanternObjAnimEvents.list.triggerCount) {
        switch (gSClanternObjAnimEvents.list.triggeredIds[i]) {
        case SCLANTERN_EVENT_LEFT_SPARK_A:
            pointIndex = 1;
            break;
        case SCLANTERN_EVENT_RIGHT_SPARK_A:
            pointIndex = 2;
            break;
        case SCLANTERN_EVENT_LEFT_SPARK_B:
            pointIndex = 1;
            break;
        case SCLANTERN_EVENT_RIGHT_SPARK_B:
            pointIndex = 2;
            break;
        case SCLANTERN_EVENT_LANTERN_SWING:
            Sfx_PlayFromObject(obj, SFXTRIG_swapstone_move_short);
            break;
        case 0:
        case 5:
        case 6:
        case 7:
        case 8:
        default:
            break;
        }
        i++;
    }
    if (pointIndex != 0) {
        ObjPath_GetPointWorldPosition((GameObject*)obj, pointIndex - 1, &posX, &posY, &posZ, 0);
        if (!((lantern->anim.currentMove == SCLANTERN_SPARK_SUPPRESS_MOVE) &&
              (lantern->anim.currentMoveProgress < 0.8f))) {
            Sfx_PlayAtPositionFromObject(obj, posX, posY, posZ, SCLANTERN_SPARK_SFX_ID);
        }
    }
    return advanceResult;
}

int lbl_803DDBF4;

u32 playerFn_801d6d58(void) {
    u32 playerObj;

    (*gMapEventInterface)->getCurChar();
    playerObj = (u32)Obj_GetPlayerObject();
    objGetAnimStateFlags((GameObject*)playerObj, 0xff);
    return 2;
}

int warpstone_testEvent(u32 obj, u32 unused, int option) {
    s8 horizontal;
    s8 vertical;

    Obj_GetPlayerObject();
    padGetAnalogInput(0, &horizontal, &vertical);

    switch (option) {
    case 0x14:
        if (horizontal < 0) {
            loadMapAndParent(0x42);
            unlockLevel(0, 0, 1);
            lockLevel(mapGetDirIdx(0x42), 0);
            lockLevel(mapGetDirIdx(7), 1);
            (*gMapEventInterface)->setMapAct(0x42, 1);
            Sfx_PlayFromObject(0, SFXTRIG_menu_pause_up);
            return 1;
        }
        break;

    case 0x15:
        if (vertical > 0 && lbl_803DC050 == 0) {
            Sfx_PlayFromObject(0, SFXTRIG_menu_pause_up);
            return 1;
        }
        break;

    case 0x16:
        if (horizontal > 0 && playerHasKrazoaSpirit(1, 0) != 0) {
            loadMapAndParent(0x42);
            lockLevel(mapGetDirIdx(0x42), 0);
            lockLevel(mapGetDirIdx(7), 1);
            if (mainGetBit(GAMEBIT_ITEM_TestCombatSpirit_Got) != 0) {
                (*gMapEventInterface)->setMapAct(0x42, 2);
            } else if (mainGetBit(GAMEBIT_ITEM_SpiritTestFear_Got) != 0) {
                (*gMapEventInterface)->setMapAct(0x42, 2);
            } else if (mainGetBit(GAMEBIT_ITEM_SpiritTestStrength_Got) != 0) {
                (*gMapEventInterface)->setMapAct(0x42, 2);
            } else if (mainGetBit(GAMEBIT_ITEM_Spirit5_Got) != 0) {
                (*gMapEventInterface)->setMapAct(0x42, 2);
            }
            Sfx_PlayFromObject(0, SFXTRIG_menu_pause_up);
            return 1;
        }
        break;

    case 0x17: {
        int hasSpirit = playerHasKrazoaSpirit(1, 0);
        if (horizontal > 0 && hasSpirit == 0) {
            Sfx_PlayFromObject(0, SFXTRIG_menu_pause_up);
            return 1;
        }
        break;
    }

    case 0x18:
        lbl_803DDBF4 = 1;
        if (vertical > 0) {
            loadMapAndParent(9);
            lockLevel(mapGetDirIdx(9), 0);
            lockLevel(mapGetDirIdx(7), 1);
            Sfx_PlayFromObject(0, SFXTRIG_menu_pause_up);
            return 1;
        }
        break;

    case 0x19:
        if ((getButtonsJustPressed(0) & PAD_BUTTON_B) != 0) {
            unlockLevel(0, 0, 1);
            mapUnload(mapGetDirIdx(0x42), 0x20000000);
            mapUnload(mapGetDirIdx(0x17), 0x20000000);
            Sfx_PlayFromObject(0, SFXTRIG_menu_pause_down);
            return 1;
        }
        break;
    }

    return 0;
}

void warpstone_loadBaseUi(void) {
    loadUiDll(0x1);
}

int warpstone_SeqFn(GameObject* obj, u32 unused, int animObj) {
    int state = *(int*)&(obj)->extra;
    int i;
    int child;
    u8 command;
    ObjAnimUpdateState* animUpdate = (ObjAnimUpdateState*)animObj;

    if (animatedObjGetSeqId(animUpdate) == 0x35f) {
        ObjSeq_SetSlotValue(animUpdate, 0x2648);
        if (getCurUiDll() != 0x10) {
            loadUiDll(0x10);
        }
    }

    child = *(int*)state;
    if ((void*)child != NULL) {
        ObjAnim_AdvanceCurrentMove(
            child, (obj)->anim.currentMoveProgress - ((GameObject*)child)->anim.currentMoveProgress, timeDelta, NULL);
    }

    animUpdate->conditionCallback = (ObjAnimSequenceConditionCallback)warpstone_testEvent;
    animUpdate->freeCallback = (ObjAnimSequenceFreeCallback)warpstone_loadBaseUi;

    if ((s8)animUpdate->sequenceEventActive != 0) {
        ((WarpStoneState*)state)->sequenceFlags = ((WarpStoneState*)state)->sequenceFlags & ~3;
        if ((s32)playerFn_801d6d58() != 0) {
            ((WarpStoneState*)state)->sequenceFlags = ((WarpStoneState*)state)->sequenceFlags | 1;
        }
        {
            int hit;
            if (mainGetBit(GAMEBIT_ITEM_WaterSpellStone1_Got) != 0) {
                hit = 1;
            } else if (mainGetBit(GAMEBIT_ITEM_FireSpellStone1_Got) != 0) {
                hit = 1;
            } else {
                hit = 0;
            }
            if (hit) {
                ((WarpStoneState*)state)->sequenceFlags = ((WarpStoneState*)state)->sequenceFlags | 2;
            }
        }
        animUpdate->sequenceEventActive = 0;

        if (mainGetBit(((WarpStoneState*)state)->sequenceGameBit) != 0 &&
            animatedObjGetSeqId((ObjAnimUpdateState*)animObj) == 0x35f) {
            AudioStream_CancelPrepared();
            seqClearTaskTexts();
            doNothing_8000CF54(0);
            animUpdate->sequenceControlFlags |= OBJSEQ_CONTROL_SET_LATCH_A;
        }
    }

    for (i = 0; i < animUpdate->eventCount; i++) {
        command = animUpdate->eventIds[i];
        switch (command) {
        case 0x17:
            ((WarpStoneState*)state)->dustEffectFlags = ((WarpStoneState*)state)->dustEffectFlags | 4;
            Sfx_PlayFromObject(0, SFXTRIG_id_420);
            break;

        case 3:
            ((WarpStoneState*)state)->pathPointIndex = 0;
            break;

        case 4:
            ((WarpStoneState*)state)->pathPointIndex = 1;
            break;

        case 6:
            CMenu_SetFadeCounter(0);
            loadUiDll(1);
            warpToMap(0x7e, 1);
            break;

        case 7:
            CMenu_SetFadeCounter(0);
            loadUiDll(1);
            mainSetBits(GAMEBIT_SH_WarpStoneRelated0884, 1);
            warpToMap(0x7e, 1);
            break;

        case 0xa:
            ((WarpStoneState*)state)->sequenceToggle = ((WarpStoneState*)state)->sequenceToggle ^ 1;
            break;

        case 9:
            (*gMapEventInterface)->setMapAct(0x17, 1);
            (*gMapEventInterface)->setMapAct(0xe, 2);
            CMenu_SetFadeCounter(0);
            loadUiDll(1);
            break;

        case 0xc:
            CMenu_SetFadeCounter(0);
            loadUiDll(1);
            warpToMap(0x33, 0);
            break;

        case 0xd:
            subtitleFn_8001b700();
        case 0xe:
        case 0xf:
        case 0x10:
        case 0x11:
            if (getCurUiDll() == 0x10) {
                UiDllVTable** uiDll = getDLL16();
                (*uiDll)->setState(animUpdate->eventIds[i] - 0xd);
            }
            mainSetBits(((WarpStoneState*)state)->sequenceGameBit, 1);
            mainSetBits(GAMEBIT_SH_SawWarpStoneIntro, 1);
            break;

        case 0x12:
            (*gMapEventInterface)->setObjGroupStatus(7, 0xa, 0);
            break;

        case 0x14:
            unlockLevel(0, 0, 1);
            break;

        case 0x15:
            unlockLevel(0, 0, 1);
            mapUnload(mapGetDirIdx(0x42), 0x20000000);
            break;

        case 0x16:
            unlockLevel(0, 0, 1);
            mapUnload(mapGetDirIdx(0x42), 0x20000000);
            break;
        }
    }

    SHthorntail_updateDustEffects(obj);
    return 0;
}

int warpstone_getExtraSize(void) {
    return sizeof(WarpStoneState);
}

int warpstone_getObjectTypeId(void) {
    return 0x48;
}

void warpstone_free(GameObject* obj, int mode) {
    int* state = (obj)->extra;
    if (*(void**)state != NULL && mode == 0) {
        ObjLink_DetachChild(obj, (GameObject*)state[0]);
        Obj_FreeObject((GameObject*)state[0]);
    }
}

void warpstone_render(GameObject* obj, int renderArg2, int renderArg3, int renderArg4, int renderArg5, s8 visible) {
    GameObject* player;
    int* state = obj->extra;
    int* model;
    f32 z;
    f32 y;
    f32 x;
    s32 visibleValue = visible;
    if (visibleValue != 0) {
        objRenderModelAndHitVolumes(obj, renderArg2, renderArg3, renderArg4, renderArg5, 1.0f);
        player = Obj_GetPlayerObject();
        if (player != NULL && fn_80296464(player) != 0) {
            model = (int*)Obj_GetActiveModel(player);
            *(u16*)((char*)model + 24) = (u16)(*(u16*)((char*)model + 24) & ~0x8);
            ObjPath_GetPointWorldPosition(obj, ((WarpStoneState*)state)->pathPointIndex, &x, &y, &z, 0);
            objSetPos(player, x, y, z);
            playerRender((int)player, renderArg2, renderArg3, renderArg4, renderArg5, -1);
        }
    }
}

void warpstone_hitDetect(GameObject* obj) {
    int* state = (obj)->extra;
    f32 pos[3];
    f32 lightPos[3];

    if (ObjHits_GetPriorityHitWithPosition(obj, 0, 0, 0, &pos[0], &pos[1], &pos[2]) != 0) {
        pos[0] += playerMapOffsetX;
        pos[2] += playerMapOffsetZ;
        objLightFn_8009a1dc((void*)obj, 0.01f, lightPos, 1, 0);
        if (randomChanceOneIn(3) != 0) {
            Sfx_PlayFromObject((int)obj, SFXTRIG_swapstone_move_short_2bc);
        } else {
            Sfx_PlayFromObject((int)obj, SFXTRIG_swapstone_move_short_2bc);
        }
        objAudioFn_800393f8(obj, (ObjSoundState*)((u8*)state + offsetof(WarpStoneState, soundState)), 171, -1280, -1,
                            0);
    }
}

int gWarpStoneLookToggleChance = 300;
int gWarpStoneHeadAimMode = 1;
int gWarpStoneHeadAimHeightOffset = 200;
s16 gWarpStoneHeadYawOffset = 0x800;
int gWarpStoneMumbleChance = 3;
int gWarpStoneYawnChance = 4;
int lbl_803DC050 = 1;

typedef struct WarpStoneFlags {
    u8 b7 : 1;
    u8 lookAtPlayer : 1;
    u8 b5 : 1;
    u8 sfxFired : 1;
    u8 lo : 4;
} WarpStoneFlags;

#define WARPSTONE_TARGET_OBJECT_GROUP 8

s16 gWarpStoneHeadPitchOffset;
s16 gWarpStoneYawBias;

void warpstone_update(int obj) {
    int state;
    int child;
    int advanceResult;
    int target;
    s16* modelVec;
    int yawDelta;
    int moveId;

    state = *(int*)&((GameObject*)obj)->extra;
    child = *(int*)state;
    if ((void*)child != NULL) {
        ObjLink_DetachChild((GameObject*)obj, (GameObject*)child);
        Obj_FreeObject(*(GameObject**)state);
        *(int*)state = 0;
    }

    advanceResult = SClantern_advanceAnimEvents(obj, 0.0055555557f);
    if (((GameObject*)obj)->anim.currentMove == 0) {
        if (randomChanceOneIn(100) != 0) {
            objAudioFn_800393f8((GameObject*)obj, (ObjSoundState*)(state + offsetof(WarpStoneState, soundState)), 0xab,
                                -0x100, -1, 0);
        }
        if (randomChanceOneIn(500) != 0) {
            objAudioFn_800393f8((GameObject*)obj, (ObjSoundState*)(state + offsetof(WarpStoneState, soundState)), 0x417,
                                -0x500, -1, 0);
        }
    }

    if (mainGetBit(GAMEBIT_ITEM_RockCandy_Used) != 0) {
        if (randomChanceOneIn(gWarpStoneLookToggleChance) != 0) {
            ((WarpStoneFlags*)(state + offsetof(WarpStoneState, behaviorFlags)))->lookAtPlayer =
                (((WarpStoneFlags*)(state + offsetof(WarpStoneState, behaviorFlags)))->lookAtPlayer == 0);
        }
        if (((WarpStoneFlags*)(state + offsetof(WarpStoneState, behaviorFlags)))->lookAtPlayer == 0) {
            ((WarpStoneFlags*)(state + offsetof(WarpStoneState, behaviorFlags)))->lookAtPlayer = mainGetBit(0xa45);
        }
    }

    if (((WarpStoneFlags*)(state + offsetof(WarpStoneState, behaviorFlags)))->lookAtPlayer != 0) {
        target = (int)Obj_GetPlayerObject();
    } else {
        target = ObjGroup_FindNearestObject(WARPSTONE_TARGET_OBJECT_GROUP, (GameObject*)obj, 0);
    }

    ((GameObject*)obj)->anim.localPosY += gWarpStoneHeadAimHeightOffset;
    characterAimHeadAtTarget((GameObject*)(obj), (void*)target, (void*)(state + offsetof(WarpStoneState, headAimState)),
                             0x23, 1, gWarpStoneHeadAimMode);
    modelVec = objModelGetVecFn_800395d8((GameObject*)(obj), 0);
    ((GameObject*)obj)->anim.localPosY -= gWarpStoneHeadAimHeightOffset;

    if (modelVec != NULL) {
        modelVec[1] = modelVec[1] + gWarpStoneHeadPitchOffset;
        modelVec[0] = 0;
        modelVec[0] += gWarpStoneHeadYawOffset;
    }

    if (advanceResult != 0) {
        ((WarpStoneFlags*)(state + offsetof(WarpStoneState, behaviorFlags)))->sfxFired = 0;
        yawDelta = Obj_GetYawDeltaToObject((GameObject*)obj, (GameObject*)target, NULL);
        yawDelta = (s16)(yawDelta - gWarpStoneYawBias);
        {
            int mag = yawDelta - 0x8000;
            mag = (mag >= 0) ? mag : -mag;
            if (mag > 0x18e3) {
                if (yawDelta > 0) {
                    if (yawDelta > 0xe38) {
                        moveId = 0x17;
                    } else {
                        moveId = 0x16;
                    }
                } else if (yawDelta < -0xe38) {
                    moveId = 0x19;
                } else {
                    moveId = 0x18;
                }
                if (((GameObject*)obj)->anim.currentMove != moveId) {
                    ObjAnim_SetCurrentMove(obj, moveId, 0.0f, 0);
                }
            } else if (((GameObject*)obj)->anim.currentMove != 0) {
                ObjAnim_SetCurrentMove(obj, 0, 0.0f, 0);
                Sfx_StopFromObject(obj, SFXTRIG_swapstone_move_long);
            } else if (randomChanceOneIn(gWarpStoneMumbleChance) != 0) {
                Sfx_PlayFromObject(obj, SFXTRIG_swapstone_mumble);
                ObjAnim_SetCurrentMove(obj, 0x1b, 0.0f, 0);
            } else if (randomChanceOneIn(gWarpStoneYawnChance) != 0) {
                Sfx_PlayFromObject(obj, SFXTRIG_swapstone_move_long);
                ObjAnim_SetCurrentMove(obj, 0x1a, 0.0f, 0);
            }
        }
    }

    objAnimFn_80038f38((GameObject*)obj, (char*)(state + offsetof(WarpStoneState, soundState)));
    characterDoEyeAnims((GameObject*)obj, (void*)(state + offsetof(WarpStoneState, eyeAnimState)));
    if (mainGetBit(GAMEBIT_SH_SawWarpStoneIntro) == 0) {
        ((WarpStoneState*)state)->activated = 0;
    }
    if (((WarpStoneFlags*)(state + offsetof(WarpStoneState, behaviorFlags)))->sfxFired != 0) {
        return;
    }

    switch (((GameObject*)obj)->anim.currentMove) {
    case 0x17:
    case 0x19:
        if (((GameObject*)obj)->anim.currentMoveProgress > 0.5f) {
            Sfx_PlayFromObject(obj, SFXTRIG_swapstone_move_long);
            ((WarpStoneFlags*)(state + offsetof(WarpStoneState, behaviorFlags)))->sfxFired = 1;
        }
        break;
    case 0x16:
    case 0x18:
        if (((GameObject*)obj)->anim.currentMoveProgress > 0.5f) {
            Sfx_PlayFromObject(obj, SFXTRIG_swapstone_move_short_2bc);
            ((WarpStoneFlags*)(state + offsetof(WarpStoneState, behaviorFlags)))->sfxFired = 1;
        }
        break;
    case 0x1a:
        if (((GameObject*)obj)->anim.currentMoveProgress > 0.6f) {
            Sfx_PlayFromObject(obj, SFXTRIG_swapstone_yawn);
            ((WarpStoneFlags*)(state + offsetof(WarpStoneState, behaviorFlags)))->sfxFired = 1;
        }
        break;
    case 0x1b:
        if (((GameObject*)obj)->anim.currentMoveProgress > 0.25f) {
            Sfx_PlayFromObject(obj, SFXTRIG_swapstone_move_short);
            ((WarpStoneFlags*)(state + offsetof(WarpStoneState, behaviorFlags)))->sfxFired = 1;
        }
        break;
    }
}

void warpstone_init(GameObject* obj, const WarpStonePlacement* placement) {
    int state;
    s16 rotX;

    state = *(int*)&obj->extra;
    rotX = (s16)(placement->rotXByte << 8);
    obj->anim.rotX = rotX;
    obj->animEventCallback = warpstone_SeqFn;
    ((WarpStoneState*)state)->sequenceGameBit = GAMEBIT_SH_WarpStoneRelated015A;
    ((WarpStoneState*)state)->resetGameBit = GAMEBIT_ITEM_RockCandyRelated0886;
    ObjHits_EnableObject(obj);
    if (mainGetBit(GAMEBIT_SH_SawWarpStoneIntro) != 0 && mainGetBit(GAMEBIT_SH_WarpStoneRelated015A) != 0) {
        ((WarpStoneState*)state)->activated = 1;
    } else {
        ((WarpStoneState*)state)->activated = 0;
    }
    mainSetBits(((WarpStoneState*)state)->resetGameBit, 0);
    *(int*)state = 0;
}

void warpstone_release(void) {
}

void warpstone_initialise(void) {
}
