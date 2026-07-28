/*
 * DLL 437 controls several LightFoot Village NPC object definitions. The
 * retail mappings include SC_chieflig, SC_lightfoo, SC_babyligh, SC_blTarget,
 * and SC_muscleli, so no single object basename represents the complete TU.
 *
 * The shared actor controller handles movement and challenge interactions.
 * Baby LightFoot actors are revealed after reaching their target placements;
 * challenge-gate actors stay interactive until their associated village
 * objectives are complete.
 */

#include "dlls/objects/437.h"

#include "game/objects/object.h"
#include "main/audio/sfx_keep_alive_api.h"
#include "main/audio/sfx_play_api.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/dll/baddie_control_interface.h"
#include "main/dll/dll_01B5_lightfoot.h"
#include "main/dll/player_api.h"
#include "main/frame_timing.h"
#include "main/gamebit_ids.h"
#include "main/gamebits_api.h"
#include "main/object_render.h"
#include "main/obj_group.h"
#include "main/obj_link.h"
#include "main/obj_list.h"
#include "main/objhits.h"
#include "main/objfx.h"
#include "main/vecmath.h"
#include "sys/objects.h"
#include "sys/objects/lifecycle.h"

#define DLL437_WEAPON_DEF_1 0x6F1
#define DLL437_WEAPON_DEF_2 0x6F2
#define DLL437_OBJECT_GROUP 3

Dll437SubstateHandler gDll437SubstateHandlers[DLL437_SUBSTATE_HANDLER_COUNT];
Dll437StateHandler gDll437StateHandlers[DLL437_STATE_HANDLER_COUNT];

s16 gDll437MoveIds0[2] = {0x33, -1};
f32 gDll437MoveSpeeds0[2] = {0.0009f, -1.0f};
s16 gDll437MoveIds1[2] = {0x33, -1};
f32 gDll437MoveSpeeds1[2] = {0.001f, -1.0f};
s16 gDll437MoveIds2[2] = {0x36, -1};
f32 gDll437MoveSpeeds2[2] = {0.003f, -1.0f};
s16 gDll437MoveIds3[2] = {0x128, -1};
f32 gDll437MoveSpeeds3[2] = {0.01f, -1.0f};
s16 gDll437MoveIds4[2] = {1, -1};
f32 gDll437MoveSpeeds4[2] = {0.01f, -1.0f};

const f32 gDll437TargetRange = 25.0f;
const f32 gDll437DirectionalBurstOffsetY = 10.0f;
const f32 gDll437TargetBurstOffsetY = 24.0f;
const f32 gDll437TargetBurstScale = 0.75f;
const f32 gDll437TargetBurstAngleMax = 48.0f;
const f32 gDll437PathRadius = 20.0f;

int dll437_getExtraSize(void) {
    return sizeof(Dll437State);
}

int dll437_getObjectTypeId(void) {
    return 0x14B;
}

void dll437_free(GameObject* obj, int preserveChildren) {
    void* child;
    int inner = *(int*)&obj->extra;
    int count;
    int i;

    ObjGroup_RemoveObject((u32)obj, DLL437_OBJECT_GROUP);
    count = obj->childCount;
    for (i = 0; i < count; i++) {
        child = obj->childObjs[0];
        if (child != NULL) {
            ObjLink_DetachChild(obj, child);
            if (preserveChildren == 0) {
                Obj_FreeObject(child);
            }
        }
    }
    (*gBaddieControlInterface)->releaseState(obj, (void*)inner, 0x20);
}

void dll437_render(GameObject* obj, int renderArg2, int renderArg3, int renderArg4, int renderArg5, s8 visible) {
    s32 visibleValue = visible;

    if (visibleValue != 0) {
        switch (obj->userData1) {
        case 0:
            objRenderModelAndHitVolumes(obj, renderArg2, renderArg3, renderArg4, renderArg5, lbl_803E8188);
            break;
        default:
            break;
        }
    }
}

void dll437_hitDetect(void) {
}

void dll437_update(GameObject* obj) {
    int inner = *(int*)&obj->extra;
    int workValue = *(int*)&obj->anim.placementData;
    int control = (int)((Dll437State*)inner)->groundBaddie.control;
    f32 pulseOffset[3];
    f32 effectParams[6];
    u8 effectCount;
    f32 terminalLifeTimer;
    f32 lifeTimer;

    lifeTimer = ((Dll437ControlState*)control)->lifeTimer;
    if (lifeTimer != (terminalLifeTimer = lbl_803E8180)) {
        ((Dll437ControlState*)control)->lifeTimer = lifeTimer - timeDelta;
        if (((Dll437ControlState*)control)->lifeTimer <= terminalLifeTimer) {
            Obj_FreeObject(obj);
        }
    }

    if (obj->anim.seqId == DLL437_SEQUENCE_ID_SC_BABY_LIGHTFOOT && ((Dll437State*)inner)->groundBaddie.gameBitA != -1) {
        switch (((Dll437Placement*)workValue)->base.mapId) {
        case 0x4993F:
        case 0x49940:
        case 0x49941:
            if (mainGetBit(0xC44)) {
                obj->userData1 = mainGetBit(((Dll437State*)inner)->groundBaddie.gameBitA);
            } else {
                obj->userData1 = 1;
            }
            break;
        case 0x499AC:
        case 0x499AE:
        case 0x499AF:
            if (mainGetBit(0xC42) && mainGetBit(((Dll437State*)inner)->groundBaddie.gameBitA) == 0) {
                void* other = ObjList_FindObjectById(0x499B5);

                if (other != NULL &&
                    Vec_distance(&obj->anim.worldPosX, &((GameObject*)other)->anim.worldPosX) < gDll437TargetRange) {
                    mainSetBits(((Dll437State*)inner)->groundBaddie.gameBitA, 1);
                    effectParams[3] = lbl_803E8180;
                    effectParams[4] = gDll437DirectionalBurstOffsetY;
                    effectParams[5] = lbl_803E8180;
                    for (effectCount = 0x14; effectCount != 0; effectCount--) {
                        objfx_spawnDirectionalBurst(obj, 5, lbl_803E81D0, 5, 6, 0x64, gDll437DirectionalBurstOffsetY,
                                                    effectParams, 0);
                    }
                    if (mainGetBit(0xC3B) && mainGetBit(0xC3C) && mainGetBit(0xC3D)) {
                        Sfx_PlayFromObject(0, SFXTRIG_mpick1_b);
                    } else {
                        Sfx_PlayFromObject(0, SFXTRIG_sc_menuups16k_409);
                    }
                }
                obj->userData1 = mainGetBit(((Dll437State*)inner)->groundBaddie.gameBitA);
            } else {
                obj->userData1 = 1;
            }
            break;
        case 0x499B0:
        case 0x499B1:
        case 0x499B2:
            if (mainGetBit(0xC46) && mainGetBit(((Dll437State*)inner)->groundBaddie.gameBitA) == 0) {
                void* other = ObjList_FindObjectById(0x499B6);

                if (other != NULL &&
                    Vec_distance(&obj->anim.worldPosX, &((GameObject*)other)->anim.worldPosX) < gDll437TargetRange) {
                    mainSetBits(((Dll437State*)inner)->groundBaddie.gameBitA, 1);
                    effectParams[3] = lbl_803E8180;
                    effectParams[4] = gDll437DirectionalBurstOffsetY;
                    effectParams[5] = lbl_803E8180;
                    for (effectCount = 0x14; effectCount != 0; effectCount--) {
                        objfx_spawnDirectionalBurst(obj, 5, lbl_803E81D0, 5, 6, 0x64, gDll437DirectionalBurstOffsetY,
                                                    effectParams, 0);
                    }
                    if (mainGetBit(0xC3E) && mainGetBit(0xC3F) && mainGetBit(0xC40)) {
                        Sfx_PlayFromObject(0, SFXTRIG_mpick1_b);
                    } else {
                        Sfx_PlayFromObject(0, SFXTRIG_sc_menuups16k_409);
                    }
                }
                obj->userData1 = mainGetBit(((Dll437State*)inner)->groundBaddie.gameBitA);
            } else {
                obj->userData1 = 1;
            }
            break;
        default:
            obj->userData1 = mainGetBit(((Dll437State*)inner)->groundBaddie.gameBitA) == 0;
            break;
        }

        if (obj->userData1 != 0) {
            ObjHits_DisableObject(obj);
            obj->anim.flags |= OBJANIM_FLAG_HIDDEN;
        } else {
            ObjHits_EnableObject(obj);
            obj->anim.flags &= ~OBJANIM_FLAG_HIDDEN;
        }
    }

    if (obj->userData1 != 0) {
        if (((((Dll437Placement*)workValue)->base.mapId == 0x499B5 && mainGetBit(0xC42) &&
              (mainGetBit(0xC3B) == 0 || mainGetBit(0xC3C) == 0 || mainGetBit(0xC3D) == 0)) ||
             (((Dll437Placement*)workValue)->base.mapId == 0x499B6 && mainGetBit(0xC46) &&
              (mainGetBit(0xC3E) == 0 || mainGetBit(0xC3F) == 0 || mainGetBit(0xC40) == 0)))) {
            effectParams[3] = lbl_803E8180;
            effectParams[4] = gDll437TargetBurstOffsetY;
            effectParams[5] = lbl_803E8180;
            objfx_spawnArcedBurst(obj, 5, gDll437TargetBurstScale, 1, 6, 0x32, gDll437TargetRange, gDll437TargetRange,
                                  gDll437TargetBurstAngleMax, effectParams, 0);
        }
    } else {
        Lightfoot_UpdateAttachedChild(obj, inner);
        if (((Dll437State*)inner)->groundBaddie.flags400 & 0x2) {
            Lightfoot_RecordCompletedChallengeTargetHit(obj, inner, control);
            Lightfoot_ResetScriptedPosition(obj);
            obj->userData2 = 0;
            ((Dll437State*)inner)->groundBaddie.flags400 &= ~0x2;
        }
        Lightfoot_UpdatePlayerInteraction((int)obj, inner, inner);
        if ((((Dll437State*)inner)->groundBaddie.configFlags & 1) && (obj->objectFlags & OBJECT_OBJFLAG_RENDERED)) {
            int controlState = (int)((Dll437State*)inner)->groundBaddie.control;

            ((Dll437ControlState*)controlState)->pulseTimer -= timeDelta;
            if (((Dll437ControlState*)controlState)->pulseTimer <= lbl_803E8180) {
                workValue = 3;
                ((Dll437ControlState*)controlState)->pulseTimer += lbl_803E81C0;
            } else {
                workValue = 0;
            }
            pulseOffset[0] = lbl_803E8180;
            pulseOffset[1] = lbl_803E81C4;
            pulseOffset[2] = lbl_803E8180;
            Sfx_KeepAliveLoopedObjectSound((int)obj, SFXTRIG_foot_metal_scuff_455);
            objfx_spawnPulseBurst(obj, lbl_803E81C8 * obj->anim.rootMotionScale, 3, workValue, 0, pulseOffset);
        }
        ((Dll437ControlState*)control)->wanderTimer -= timeDelta;
    }
}

void dll437_init(GameObject* obj, const Dll437Placement* placement, int isReload) {
    u8* playerAnimTableBase = (u8*)&lbl_80334EE8;
    int inner = *(int*)&obj->extra;
    const Dll437Placement* placementData = placement;
    int control;
    u8 initFlags = 0x16;

    if (isReload != 0) {
        initFlags |= 1;
    }
    (*gBaddieControlInterface)
        ->initGroundBaddie(obj, (u8*)placement, (u8*)inner, 5, 3, 0x108, initFlags, gDll437PathRadius);
    obj->animEventCallback = Lightfoot_SeqFn;
    ((GroundBaddieState*)inner)->baddie.controlMode = 0;
    ((GroundBaddieState*)inner)->baddie.substate = 0;
    obj->objectFlags = (u16)(obj->objectFlags | OBJECT_OBJFLAG_HITDETECT_DISABLED);
    control = (int)((Dll437State*)inner)->groundBaddie.control;
    ((Dll437ControlState*)control)->weaponDefNoSentinel = -1;
    ((Dll437ControlState*)control)->weaponDefNo = ((Dll437ControlState*)control)->weaponDefNoSentinel;
    obj->objectFlags = (u16)(obj->objectFlags | (placement->objectFlags & 0x7));
    if (placement->behaviorId == 0x64C) {
        ((GroundBaddieState*)inner)->baddie.controlMode = 2;
        ((GroundBaddieState*)inner)->baddie.substate = 1;
        ObjHits_DisableObject(obj);
        ((Dll437ControlState*)control)->moveIndex = randomGetRange(0, 3);
        ((Dll437ControlState*)control)->weaponDefNo = DLL437_WEAPON_DEF_1;
        ((Dll437ControlState*)control)->moveIds = gDll437MoveIds0;
        ((Dll437ControlState*)control)->moveSpeeds = gDll437MoveSpeeds0;
        *(u8*)&obj->anim.resetHitboxMode = (u8)(*(u8*)&obj->anim.resetHitboxMode | INTERACT_FLAG_DISABLED);
        obj->userData2 = 0;
    } else {
        switch (placementData->base.mapId) {
        case 0x34316:
            ((Dll437ControlState*)control)->moveIds = gDll437MoveIds3;
            ((Dll437ControlState*)control)->moveSpeeds = gDll437MoveSpeeds3;
            ObjHits_DisableObject(obj);
            *(u8*)&obj->anim.resetHitboxMode = (u8)(*(u8*)&obj->anim.resetHitboxMode | INTERACT_FLAG_DISABLED);
            obj->anim.currentMoveProgress = (f32)(s32)randomGetRange(0, 0x63) / lbl_803E817C;
            break;
        case 0x33E3C:
            ((Dll437ControlState*)control)->moveIds = gDll437MoveIds0;
            ((Dll437ControlState*)control)->moveSpeeds = gDll437MoveSpeeds0;
            ((Dll437ControlState*)control)->weaponDefNo = DLL437_WEAPON_DEF_1;
            *(u8*)&obj->anim.resetHitboxMode = (u8)(*(u8*)&obj->anim.resetHitboxMode | INTERACT_FLAG_DISABLED);
            obj->anim.currentMoveProgress = (f32)(s32)randomGetRange(0, 0x63) / lbl_803E817C;
            break;
        case 0x33E34:
            ((Dll437ControlState*)control)->moveIds = gDll437MoveIds1;
            ((Dll437ControlState*)control)->moveSpeeds = gDll437MoveSpeeds1;
            ((Dll437ControlState*)control)->weaponDefNo = DLL437_WEAPON_DEF_1;
            *(u8*)&obj->anim.resetHitboxMode = (u8)(*(u8*)&obj->anim.resetHitboxMode | INTERACT_FLAG_DISABLED);
            obj->anim.currentMoveProgress = (f32)(s32)randomGetRange(0, 0x63) / lbl_803E817C;
            break;
        case 0x45C47:
            ((Dll437ControlState*)control)->moveIds = gDll437MoveIds2;
            ((Dll437ControlState*)control)->moveSpeeds = gDll437MoveSpeeds2;
            ObjHits_DisableObject(obj);
            ((Dll437ControlState*)control)->weaponDefNo = DLL437_WEAPON_DEF_2;
            *(u8*)&obj->anim.resetHitboxMode = (u8)(*(u8*)&obj->anim.resetHitboxMode | INTERACT_FLAG_DISABLED);
            obj->anim.currentMoveProgress = (f32)(s32)randomGetRange(0, 0x63) / lbl_803E817C;
            break;
        case 0x460B6:
            ((Dll437ControlState*)control)->moveIds = gDll437MoveIds4;
            ((Dll437ControlState*)control)->moveSpeeds = gDll437MoveSpeeds4;
            ObjHits_DisableObject(obj);
            *(u8*)&obj->anim.resetHitboxMode = (u8)(*(u8*)&obj->anim.resetHitboxMode | INTERACT_FLAG_DISABLED);
            obj->anim.currentMoveProgress = (f32)(s32)randomGetRange(0, 0x63) / lbl_803E817C;
            break;
        case 0x3433F:
            ((Dll437ControlState*)control)->moveIds = (s16*)(playerAnimTableBase + 0x30);
            ((Dll437ControlState*)control)->moveSpeeds = (f32*)(playerAnimTableBase + 0x40);
            *(u8*)&obj->anim.resetHitboxMode = (u8)(*(u8*)&obj->anim.resetHitboxMode | INTERACT_FLAG_DISABLED);
            obj->anim.currentMoveProgress = (f32)(s32)randomGetRange(0, 0x63) / lbl_803E817C;
            break;
        case 0x46A51:
            if (mainGetBit(GAMEBIT_LV_ChallengeGate1Complete)) {
                *(u8*)&obj->anim.resetHitboxMode = (u8)(*(u8*)&obj->anim.resetHitboxMode | INTERACT_FLAG_DISABLED);
            }
            ((Dll437ControlState*)control)->moveIds = (s16*)playerAnimTableBase;
            ((Dll437ControlState*)control)->moveSpeeds = (f32*)(playerAnimTableBase + 0x10);
            break;
        case 0x46A55:
            if (mainGetBit(GAMEBIT_LV_ChallengeGate2Complete)) {
                *(u8*)&obj->anim.resetHitboxMode = (u8)(*(u8*)&obj->anim.resetHitboxMode | INTERACT_FLAG_DISABLED);
            }
            ((Dll437ControlState*)control)->moveIds = (s16*)playerAnimTableBase;
            ((Dll437ControlState*)control)->moveSpeeds = (f32*)(playerAnimTableBase + 0x10);
            break;
        case 0x49928:
            if (mainGetBit(GAMEBIT_SC_ChallengeGate3Complete)) {
                *(u8*)&obj->anim.resetHitboxMode = (u8)(*(u8*)&obj->anim.resetHitboxMode | INTERACT_FLAG_DISABLED);
            }
            ((Dll437ControlState*)control)->moveIds = (s16*)playerAnimTableBase;
            ((Dll437ControlState*)control)->moveSpeeds = (f32*)(playerAnimTableBase + 0x10);
            break;
        case 0x499AC:
        case 0x499AE:
        case 0x499AF:
        case 0x499B0:
        case 0x499B1:
        case 0x499B2:
            ((GroundBaddieState*)inner)->baddie.substate = 2;
            ((Dll437ControlState*)control)->moveIds = (s16*)(playerAnimTableBase + 0x30);
            ((Dll437ControlState*)control)->moveSpeeds = (f32*)(playerAnimTableBase + 0x40);
            ((Dll437ControlState*)control)->wanderTimer = (f32)(s32)randomGetRange(0x78, 0xB4);
            obj->anim.currentMoveProgress = (f32)(s32)randomGetRange(0, 0x63) / lbl_803E817C;
            break;
        case 0x499B5:
        case 0x499B6:
            obj->userData1 = 1;
            ((Dll437ControlState*)control)->moveIds = (s16*)(playerAnimTableBase + 0x30);
            ((Dll437ControlState*)control)->moveSpeeds = (f32*)(playerAnimTableBase + 0x40);
            break;
        default:
            ((Dll437ControlState*)control)->moveIds = (s16*)playerAnimTableBase;
            ((Dll437ControlState*)control)->moveSpeeds = (f32*)(playerAnimTableBase + 0x10);
            break;
        }
    }
    Lightfoot_ResetScriptedPosition(obj);
    ObjAnim_SetMoveProgress((ObjAnimComponent*)obj, (f32)(s32)randomGetRange(0, 0x63) / lbl_803E817C);
    ((Dll437ControlState*)control)->movementSfxId = (u16)(randomGetRange(0, 1) != 0 ? 0x133 : 0x134);
    ((Dll437ControlState*)control)->pulseTimer = lbl_803E81C0;
    if (obj->userData1 != 0) {
        ObjHits_DisableObject(obj);
    }
}

void dll437_release(void) {
}

void dll437_initialise(void) {
    gDll437StateHandlers[0] = Lightfoot_UpdateAnimationCycle;
    gDll437StateHandlers[1] = Lightfoot_UpdateButtonTimingChallenge;
    gDll437StateHandlers[2] = Lightfoot_UpdateTargetAnimationCycle;
    gDll437StateHandlers[3] = (Dll437StateHandler)Lightfoot_UpdateRandomTurn;
    gDll437StateHandlers[4] = Lightfoot_UpdateWanderSteering;
    gDll437SubstateHandlers[0] = Lightfoot_UpdateChallengeGateInteraction;
    gDll437SubstateHandlers[1] = Lightfoot_UpdateCompletionInteraction;
    gDll437SubstateHandlers[2] = Lightfoot_UpdateProximityInteractionState;
}

ObjectDescriptor gDll437ObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)dll437_initialise,
    (ObjectDescriptorCallback)dll437_release,
    0,
    (ObjectDescriptorCallback)dll437_init,
    (ObjectDescriptorCallback)dll437_update,
    (ObjectDescriptorCallback)dll437_hitDetect,
    (ObjectDescriptorCallback)dll437_render,
    (ObjectDescriptorCallback)dll437_free,
    (ObjectDescriptorCallback)dll437_getObjectTypeId,
    (ObjectDescriptorExtraSizeCallback)dll437_getExtraSize,
};
