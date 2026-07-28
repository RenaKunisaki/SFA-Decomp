/*
 * Unnamed GroundBaddie object in DLL slot 206.
 *
 * The object coordinates attacks with sibling instances, switches between
 * submerged and active movement states, and launches a child projectile.
 */
#include "dlls/objects/206.h"
#include "dlls/objects/202.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "game/objects/object.h"
#include "main/audio/sfx_play_api.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/dll/baddie_control_interface.h"
#include "main/dll/partfx_interface.h"
#include "main/frame_timing.h"
#include "main/gamebits_api.h"
#include "main/mapEventTypes.h"
#include "main/object_render.h"
#include "main/obj_group.h"
#include "main/obj_list.h"
#include "main/obj_message.h"
#include "main/objanim.h"
#include "main/objhits.h"
#include "main/objprint_api.h"
#include "main/objseq.h"
#include "main/player_control_interface.h"
#include "main/sky_interface.h"
#include "main/vecmath.h"
#include "sys/objects.h"
#include "sys/objects/lifecycle.h"

#define DLL_CE_OBJGROUP                    3
#define DLL_CE_CHILD_OBJ                   778
#define DLL_CE_CHILD_SETUP_SIZE            0x24
#define DLL_CE_PARTFX_DUST                 0x345
#define DLL_CE_PARTFX_SPRAY                0x343
#define DLL_CE_HIT_VOLUME_SLOT             10
#define DLL_CE_SIBLING_SEQ_ID              0x306
#define DLL_CE_MESSAGE_HIDE                0x80
#define DLL_CE_MESSAGE_RELEASE             0x81
#define DLL_CE_QUERY_STATE_CALLBACK_OFFSET 0x20
#define DLL_CE_MESSAGE_CALLBACK_OFFSET     0x24

#define DLL_CE_EFFECT_PROJECTILE 0x1
#define DLL_CE_EFFECT_DUST       0x2
#define DLL_CE_EFFECT_SPRAY      0x4

#define DLL_CE_COORDINATION_ATTACKING 0x1
#define DLL_CE_COORDINATION_HIDDEN    0x2

u8 gDllCEHitReactionScratch[0x18];
ChukChukStateHandler gChukChukCheckHandlers[6];
ChukChukStateHandler gChukChukMoveHandlers[8];

int gDllCEHitReactionMoves[30] = {
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
};

u8 gDllCEHitReactionDamage[32] = {
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0,   0,
};

ObjectDescriptor12 gDllCEObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_12_SLOTS,
    (ObjectDescriptorCallback)dll_CE_initialise,
    (ObjectDescriptorCallback)dll_CE_release,
    0,
    (ObjectDescriptorCallback)dll_CE_init,
    (ObjectDescriptorCallback)dll_CE_update,
    (ObjectDescriptorCallback)dll_CE_hitDetect,
    (ObjectDescriptorCallback)dll_CE_render,
    (ObjectDescriptorCallback)dll_CE_free,
    (ObjectDescriptorCallback)dll_CE_getObjectTypeId,
    dll_CE_getExtraSize,
    (ObjectDescriptorCallback)dll_CE_setScale,
    (ObjectDescriptorCallback)dll_CE_handleMessage,
};

void iceBaddie_installStateHandlers(void) {
    gIceBaddieStateHandlersA[0] = iceBaddie_updateOpenHitState;
    gIceBaddieStateHandlersA[1] = iceBaddie_updateOpenState;
    gIceBaddieStateHandlersA[2] = iceBaddie_updateHideResetState;
    gIceBaddieStateHandlersA[3] = iceBaddie_updateImpactHitState;
    gIceBaddieStateHandlersA[4] = iceBaddie_updateSpinState;
    gIceBaddieStateHandlersA[5] = iceBaddie_stateHandlerA05;
    gIceBaddieStateHandlersA[6] = iceBaddie_stateHandlerA06;
    gIceBaddieStateHandlersA[7] = iceBaddie_updateHeightBlendState;
    gIceBaddieStateHandlersA[8] = iceBaddie_updateControlMove5State;
    gIceBaddieStateHandlersA[9] = iceBaddie_updateCommDownState;
    gIceBaddieStateHandlersA[10] = iceBaddie_updateDropState;
    gIceBaddieStateHandlersA[11] = iceBaddie_stateHandlerA0B;
    gIceBaddieStateHandlersA[12] = iceBaddie_updateContactHitState;
    gIceBaddieStateHandlersA[13] = iceBaddie_updateLandingState;
    gIceBaddieStateHandlersB[0] = iceBaddie_checkTargetState;
    gIceBaddieStateHandlersB[1] = iceBaddie_stateHandlerB01;
    gIceBaddieStateHandlersB[2] = iceBaddie_stateHandlerB02;
    gIceBaddieStateHandlersB[3] = iceBaddie_stateHandlerB03;
    gIceBaddieStateHandlersB[4] = iceBaddie_stateHandlerB04;
    gIceBaddieStateHandlersB[5] = iceBaddie_stateHandlerB05;
    gIceBaddieStateHandlersB[6] = iceBaddie_stateHandlerB06;
    gIceBaddieStateHandlersB[7] = iceBaddie_stateHandlerB07;
}

int chukChuk_checkChooseAttackState(GameObject* obj, GroundBaddieState* state) {
    int objectCount;
    int objectIndex;
    GroundBaddieState* objectState;
    DllCEControl* control;
    int maxSiblingState;
    int attackingSiblingCount;
    int* objects;
    int shouldDropTarget;
    int attackRoll;

    objectState = obj->extra;
    if (*(char*)&state->baddie.moveDone != '\0' || *(char*)&state->baddie.moveJustStartedB != '\0') {
        control = objectState->control;
        shouldDropTarget =
            (*gBaddieControlInterface)->shouldDropTarget(obj, state, (f32)(u32)objectState->aggroRange, 1);
        if (shouldDropTarget != 0) {
            control->coordinationFlags &= ~DLL_CE_COORDINATION_HIDDEN;
            return 5;
        }
        attackingSiblingCount = 0;
        maxSiblingState = 0;
        objects = ObjList_GetObjects(&objectIndex, &objectCount);
        for (; objectIndex < objectCount; objectIndex++) {
            void* sibling = (void*)objects[objectIndex];
            if (sibling != (void*)obj && ((GameObject*)sibling)->anim.seqId == DLL_CE_SIBLING_SEQ_ID) {
                int siblingState = (*(int (**)(void*, int))(**(int**)&((GameObject*)sibling)->anim.dll +
                                                            DLL_CE_QUERY_STATE_CALLBACK_OFFSET))(sibling, 0);
                if (siblingState > maxSiblingState) {
                    maxSiblingState = siblingState;
                }
                if (siblingState == 4) {
                    attackingSiblingCount++;
                }
            }
        }
        attackRoll = randomGetRange(0, objectState->aggression);
        if (maxSiblingState >= 5 || (control->coordinationFlags & DLL_CE_COORDINATION_ATTACKING) != 0) {
            if ((objectState->configFlags & 2) != 0) {
                control->coordinationFlags |= DLL_CE_COORDINATION_ATTACKING;
            }
            (*gPlayerInterface)->setState((void*)obj, state, 4);
        } else if (attackRoll > 32) {
            if (attackingSiblingCount > 1) {
                (*gPlayerInterface)->setState((void*)obj, state, 2);
            } else {
                (*gPlayerInterface)->setState((void*)obj, state, 4);
            }
        } else if (attackRoll > 16) {
            (*gPlayerInterface)->setState((void*)obj, state, 2);
        } else {
            (*gPlayerInterface)->setState((void*)obj, state, 3);
        }
    }
    return 0;
}

int chukChuk_checkSubmergeState(GameObject* obj, GroundBaddieState* state) {
    GroundBaddieState* objectState = obj->extra;

    if ((s8)state->baddie.moveJustStartedB != 0) {
        f32 zero;

        (*gPlayerInterface)->setState(obj, state, 1);
        {
            DllCEControl* control = objectState->control;

            zero = 0.0f;
            control->soundTimer = zero;
            control->nextSoundTime = zero;
        }
    }
    return 0;
}

int chukChuk_checkYieldState(GameObject* obj, GroundBaddieState* state) {
    GroundBaddieState* objectState;

    if ((s8)state->baddie.moveJustStartedB != 0) {
        objectState = obj->extra;
        objectState->subMode = 0;
        if (objectState->gameBitB != -1) {
            mainSetBits(objectState->gameBitB, 0);
        }
        if (objectState->gameBitA != -1) {
            mainSetBits(objectState->gameBitA, 1);
        }
    }
    return 0;
}

int chukChuk_checkDeathState(GameObject* obj, GroundBaddieState* state) {
    GroundBaddieState* objectState = obj->extra;
    f32 zero;
    DllCEControl* control;

    if (*(char*)&state->baddie.moveJustStartedB != '\0') {
        control = objectState->control;
        zero = 0.0f;
        control->soundTimer = zero;
        control->nextSoundTime = zero;
        (*gPlayerInterface)->setState(obj, state, 6);
        *(int*)&state->baddie.targetObj = 0;
        *(s8*)&state->baddie.physicsActive = 0;
        *(s8*)&state->baddie.hasTarget = 0;
        ObjHits_DisableObject(obj);
        *(u8*)&obj->anim.resetHitboxMode |= INTERACT_FLAG_DISABLED;
    } else if (*(char*)&state->baddie.moveDone != '\0') {
        ObjMsg_SendToObjects(0, 3, obj, 0xe0000, (u32)obj);
        if (obj->anim.placementData == NULL) {
            Obj_FreeObject(obj);
            return 0;
        }
        return 4;
    }
    return 0;
}

int chukChuk_checkHealthState(GameObject* obj, GroundBaddieState* state) {
    (void)obj;

    if ((s8)state->baddie.hitPoints < 1) {
        return 3;
    }
    if ((s8)state->baddie.moveDone != 0) {
        return 6;
    }
    return 0;
}

int chukChuk_checkTargetState(GameObject* obj, GroundBaddieState* state) {
    if (*(int**)&state->baddie.targetObj != NULL) {
        if ((s8)state->baddie.moveJustStartedB != 0) {
            f32 zero = 0.0f;

            state->baddie.animSpeedB = zero;
            state->baddie.animSpeedA = zero;
            (*gPlayerInterface)->setState(obj, state, 0);
        }
        if ((s8)state->baddie.moveDone != 0) {
            return 6;
        }
    }
    return 0;
}

int chukChuk_updateWindupState(GameObject* obj, GroundBaddieState* state) {
    GroundBaddieState* objectState;
    f32 speed;

    objectState = obj->extra;
    *(s8*)&state->baddie.stateTag = 3;
    state->baddie.moveSpeed = 0.008f;
    speed = 0.0f;
    state->baddie.animSpeedA = speed;
    state->baddie.animSpeedB = speed;
    if (*(char*)&state->baddie.moveJustStartedA != '\0') {
        ObjAnim_SetCurrentMove((int)obj, 1, speed, 0);
        *(s8*)&state->baddie.moveDone = 0;
    }
    if ((state->baddie.moveEventFlags & 1) == 0) {
        if (((GameObject*)Obj_GetPlayerObject())->anim.seqId != 0) {
            Sfx_PlayFromObject((u32)obj, SFXTRIG_wp_stftest122_1f2);
        } else {
            Sfx_PlayFromObject((u32)obj, SFXTRIG_swd);
        }
        Sfx_PlayFromObject((u32)obj, SFXTRIG_en_rfall5_c);
        Sfx_PlayFromObject((u32)obj, SFXTRIG_dn_seal4_c_263);
        state->baddie.moveEventFlags |= 1;
    }
    if ((state->baddie.moveEventFlags & 2) == 0 && obj->anim.currentMoveProgress > 0.3f) {
        Sfx_PlayFromObject((u32)obj, SFXTRIG_wp_iceywindlp16_233);
        state->baddie.moveEventFlags |= 2;
        (*gBaddieControlInterface)->spawnChild(obj, objectState->triggerId, -1, 0);
    }
    return 0;
}

int chukChuk_updateAlertState(GameObject* obj, GroundBaddieState* state) {
    int* objects;
    int objectCount;
    int objectIndex;
    int* playerChild;
    GameObject* player;
    int childState;

    if (*(char*)&state->baddie.moveJustStartedA != '\0') {
        ObjAnim_SetCurrentMove((int)obj, 0, 0.0f, 0);
        *(s8*)&state->baddie.moveDone = 0;
    }
    if (*(char*)&state->baddie.moveJustStartedA != '\0') {
        objects = ObjList_GetObjects(&objectIndex, &objectCount);
        for (; objectIndex < objectCount; objectIndex++) {
            void* sibling = (void*)objects[objectIndex];

            if (sibling != obj && ((GameObject*)sibling)->anim.seqId == DLL_CE_SIBLING_SEQ_ID) {
                (*(void (**)(void*, int, int))(**(int**)&((GameObject*)sibling)->anim.dll +
                                               DLL_CE_MESSAGE_CALLBACK_OFFSET))(sibling, DLL_CE_MESSAGE_RELEASE, 0);
            }
        }
        playerChild = (int*)((GameObject*)Obj_GetPlayerObject())->childObjs[0];
        player = Obj_GetPlayerObject();
        childState = (**(int (**)(int*))(*(int*)(*(int*)&((GameObject*)playerChild)->anim.dll) + 0x44))(playerChild);
        if (childState != 0) {
            if (player->anim.seqId != 0) {
                Sfx_PlayFromObject((u32)obj, SFXTRIG_wp_stftest122_1f2);
            } else {
                Sfx_PlayFromObject((u32)obj, SFXTRIG_dn_boar1_c_95);
            }
        } else {
            if (player->anim.seqId != 0) {
                Sfx_PlayFromObject((u32)obj, SFXTRIG_wp_stftest122_1f2);
            } else {
                Sfx_PlayFromObject((u32)obj, SFXTRIG_swd);
            }
        }
        Sfx_PlayFromObject((u32)obj, SFXTRIG_dn_boar1_c_267);
    }
    *(s8*)&state->baddie.stateTag = 3;
    state->baddie.moveSpeed = 0.015f;
    state->baddie.animSpeedA = 0.0f;
    return 0;
}

int chukChuk_updateSpitState(GameObject* obj, GroundBaddieState* state) {
    GroundBaddieState* objectState = obj->extra;
    int objectCount;
    int objectIndex;

    if ((s32)(s8)state->baddie.moveJustStartedA != 0) {
        ObjHits_EnableObject(obj);
    }
    ObjHits_SetHitVolumeSlot((ObjAnimComponent*)obj, DLL_CE_HIT_VOLUME_SLOT, 1, -1);
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->objectPairPriority = 10;
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->objectPairHitVolume = 1;
    ObjHits_RegisterActiveHitVolumeObject(obj);

    if ((s32)(s8)state->baddie.moveJustStartedA != 0) {
        int* objects = ObjList_GetObjects(&objectIndex, &objectCount);

        while (objectIndex < objectCount) {
            int siblingAddress = objects[objectIndex];

            if ((void*)siblingAddress != (void*)obj &&
                ((GameObject*)siblingAddress)->anim.seqId == DLL_CE_SIBLING_SEQ_ID) {
                (*(int (**)(int, int, int))(**(int**)&((GameObject*)siblingAddress)->anim.dll +
                                            DLL_CE_MESSAGE_CALLBACK_OFFSET))(siblingAddress, DLL_CE_MESSAGE_RELEASE, 0);
            }
            objectIndex++;
        }
    }

    state->baddie.moveSpeed = 0.01f;

    if ((s32)(s8)state->baddie.moveJustStartedA != 0) {
        ObjAnim_SetCurrentMove((int)obj, 10, 0.0f, 0);
        state->baddie.moveDone = 0;
    }
    state->baddie.stateTag = 1;

    if ((state->baddie.eventFlags & BADDIE_EVENT_FOOTSTEP) != 0U) {
        DllCEControl* control = objectState->control;

        state->baddie.eventFlags = state->baddie.eventFlags & ~BADDIE_EVENT_FOOTSTEP;
        control->effectFlags |= DLL_CE_EFFECT_PROJECTILE;
        Sfx_PlayFromObject((u32)obj, SFXTRIG_baddie_rach_bite_266);
    }
    return 0;
}

int chukChuk_updateState3(GameObject* obj, GroundBaddieState* state) {
    if ((s8)state->baddie.moveJustStartedA != 0) {
        ObjHits_EnableObject(obj);
    }
    ObjHits_SetHitVolumeSlot((ObjAnimComponent*)obj, DLL_CE_HIT_VOLUME_SLOT, 1, -1);
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->objectPairPriority = 10;
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->objectPairHitVolume = 1;
    ObjHits_RegisterActiveHitVolumeObject(obj);
    state->baddie.moveSpeed = 0.01f;
    if ((s8)state->baddie.moveJustStartedA != 0) {
        ObjAnim_SetCurrentMove((int)obj, 5, 0.0f, 0);
        state->baddie.moveDone = 0;
    }
    state->baddie.stateTag = 1;
    return 0;
}

int chukChuk_updateAttackState(GameObject* obj, GroundBaddieState* state) {
    int objectCount;
    int objectIndex;
    GroundBaddieState* objectState;
    int* objects;

    objectState = obj->extra;
    if (*(char*)&state->baddie.moveJustStartedA != '\0') {
        ObjHits_EnableObject(obj);
    }
    ObjHits_SetHitVolumeSlot((ObjAnimComponent*)obj, DLL_CE_HIT_VOLUME_SLOT, 1, -1);
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->objectPairPriority = 10;
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->objectPairHitVolume = 1;
    ObjHits_RegisterActiveHitVolumeObject(obj);
    if (*(char*)&state->baddie.moveJustStartedA != '\0') {
        objects = ObjList_GetObjects(&objectIndex, &objectCount);
        for (; objectIndex < objectCount; objectIndex++) {
            void* sibling = (void*)objects[objectIndex];

            if (sibling != obj && ((GameObject*)sibling)->anim.seqId == DLL_CE_SIBLING_SEQ_ID) {
                (*(void (**)(void*, int, int))(**(int**)&((GameObject*)sibling)->anim.dll +
                                               DLL_CE_MESSAGE_CALLBACK_OFFSET))(sibling, DLL_CE_MESSAGE_RELEASE, 0);
            }
        }
        if (randomGetRange(0, 1) != 0) {
            if (*(char*)&state->baddie.moveJustStartedA != '\0') {
                ObjAnim_SetCurrentMove((int)obj, 6, 0.0f, 0);
                *(s8*)&state->baddie.moveDone = 0;
            }
        } else {
            if (*(char*)&state->baddie.moveJustStartedA != '\0') {
                ObjAnim_SetCurrentMove((int)obj, 7, 0.0f, 0);
                *(s8*)&state->baddie.moveDone = 0;
            }
        }
        *(s8*)&state->baddie.stateTag = 1;
        state->baddie.moveSpeed = 0.005f + (f32)(u32)objectState->aggression / 20000.0f;
    }
    state->baddie.animSpeedA = 0.0f;
    return 0;
}

int chukChuk_updateSubmergeState(GameObject* obj, GroundBaddieState* state) {
    GroundBaddieState* objectState;
    DllCEControl* control;

    objectState = obj->extra;
    if (*(char*)&state->baddie.moveJustStartedA != '\0') {
        ObjAnim_SetCurrentMove((int)obj, 14, 0.0f, 0);
        *(s8*)&state->baddie.moveDone = 0;
    }
    if (obj->anim.currentMoveProgress > 0.25f) {
        control = objectState->control;
        control->effectFlags |= DLL_CE_EFFECT_DUST;
    }
    if (*(char*)&state->baddie.moveJustStartedA != '\0') {
        ObjHits_DisableObject(obj);
        state->baddie.moveSpeed = 0.01f;
        state->baddie.animSpeedA = 0.0f;
    }
    if (*(char*)&state->baddie.moveDone != '\0') {
        mainSetBits(objectState->gameBitB, 0);
        ObjAnim_SetCurrentMove((int)obj, 8, 0.0f, 0);
        *(int*)&state->baddie.targetObj = 0;
        *(s8*)&state->baddie.physicsActive = 0;
        *(s8*)&state->baddie.hasTarget = 0;
        objectState->targetState = 0;
        if ((control->coordinationFlags & DLL_CE_COORDINATION_HIDDEN) == 0) {
            *(u8*)&obj->anim.resetHitboxMode |= INTERACT_FLAG_DISABLED;
        }
    }
    return 0;
}

int chukChuk_updateEmergeState(GameObject* obj, GroundBaddieState* state) {
    GroundBaddieState* objectState;
    DllCEControl* control;
    int eventFlags;

    objectState = obj->extra;
    control = objectState->control;
    if (*(char*)&state->baddie.moveJustStartedA != '\0') {
        ObjAnim_SetCurrentMove((int)obj, 11, 0.0f, 0);
        *(s8*)&state->baddie.moveDone = 0;
    }
    if (*(char*)&state->baddie.moveJustStartedA != '\0') {
        *(s8*)&state->baddie.physicsActive = 1;
        mainSetBits(objectState->gameBitB, 1);
        *(u8*)&obj->anim.resetHitboxMode &= ~INTERACT_FLAG_DISABLED;
        obj->anim.alpha = 0xff;
        *(s8*)&state->baddie.stateTag = 1;
        state->baddie.moveSpeed = 0.012f + (f32)(u32)objectState->aggression / 10000.0f;
        ObjHits_EnableObject(obj);
    } else {
        ObjHits_SetHitVolumeSlot((ObjAnimComponent*)obj, DLL_CE_HIT_VOLUME_SLOT, 1, -1);
        ((ObjHitsPriorityState*)obj->anim.hitReactState)->objectPairPriority = 10;
        ((ObjHitsPriorityState*)obj->anim.hitReactState)->objectPairHitVolume = 1;
        ObjHits_RegisterActiveHitVolumeObject(obj);
    }
    if (*(char*)&state->baddie.moveDone != '\0') {
        objectState->targetState = 1;
    }
    eventFlags = state->baddie.eventFlags;
    if ((eventFlags & BADDIE_EVENT_LANDING) != 0) {
        state->baddie.eventFlags = eventFlags & ~BADDIE_EVENT_LANDING;
        control->effectFlags |= DLL_CE_EFFECT_SPRAY;
    }
    if (obj->anim.currentMoveProgress < 0.7f) {
        control->effectFlags |= DLL_CE_EFFECT_DUST;
    }
    return 0;
}

void chukChuk_spawnIceBall(GameObject* obj, GroundBaddieState* state) {
    f32 duration;
    f32 distanceRatio;
    ObjPlacement* setup;
    GameObject* projectile;

    if (Obj_IsLoadingLocked() == 0) {
        setup = Obj_AllocObjectSetup(DLL_CE_CHILD_SETUP_SIZE, DLL_CE_CHILD_OBJ);
        setup->posX = obj->anim.localPosX;
        setup->posY = 15.0f + obj->anim.localPosY;
        setup->posZ = obj->anim.localPosZ;
        setup->color[0] = 1;
        setup->color[1] = 1;
        setup->color[2] = 0xff;
        setup->color[3] = 0xff;
        projectile = Obj_SetupObject(setup, 5, -1, -1, 0);
        if (projectile != NULL) {
            distanceRatio = state->baddie.targetDistance / (f32)(u32)state->aggroRange;
            duration = 50.0f * distanceRatio;
            projectile->anim.velocityX =
                (((GameObject*)state->baddie.targetObj)->anim.localPosX - obj->anim.localPosX) / duration;
            projectile->anim.velocityY =
                ((90.0f * distanceRatio + ((GameObject*)state->baddie.targetObj)->anim.localPosY) -
                 obj->anim.localPosY) /
                duration;
            projectile->anim.velocityZ =
                (((GameObject*)state->baddie.targetObj)->anim.localPosZ - obj->anim.localPosZ) / duration;
            *(int*)&projectile->ownerObj = (int)obj;
        }
    }
}

void chukChuk_acquireTarget(GameObject* obj, GroundBaddieState* objectState, GroundBaddieState* state) {
    DllCEControl* control = objectState->control;
    GameObject* target;

    target = (*gBaddieControlInterface)->findAggroTarget(obj, state, (f32)(u32)objectState->aggroRange, 0x8000);

    if (target != NULL && (objectState->configFlags & 0x4) == 0) {
        int disabledSoundId = -1;

        (*gBaddieControlInterface)
            ->startHitReaction(obj, state, (char*)objectState + 0x35c, objectState->gameBitB, NULL, 0, 0, 8,
                               disabledSoundId);
        *(int*)&state->baddie.targetObj = (int)target;
        state->baddie.hasTarget = 0;
        objectState->targetState = 1;
    } else {
        GameObject* player = Obj_GetPlayerObject();
        f32 playerDistance;
        struct {
            f32 x, y, z;
        } delta;
        f32* deltaAddress = &delta.x;

        (void)deltaAddress;
        if (player != NULL) {
            delta.x = player->anim.worldPosX - obj->anim.worldPosX;
            delta.y = player->anim.worldPosY - obj->anim.worldPosY;
            delta.z = player->anim.worldPosZ - obj->anim.worldPosZ;
            playerDistance = sqrtf(delta.z * delta.z + (delta.x * delta.x + delta.y * delta.y));
        } else {
            playerDistance = 10000.0f;
        }
        if (control->soundTimer > control->nextSoundTime) {
            if (playerDistance < 400.0f) {
                Sfx_PlayFromObject((u32)obj, SFXTRIG_dn_boar1_c_265);
                control->nextSoundTime += (f32)(s32)randomGetRange(50, 250);
            }
        }
        control->soundTimer += timeDelta;
    }
}

void chukChuk_updateTargeting(GameObject* obj, int objectStateAddress, int stateAddress) {
    GameObject* player;
    char* target;
    int hitReactionUpdated;
    struct {
        f32 x, y, z;
    } delta;
    f32* deltaAddress = &delta.x;

    (void)deltaAddress;
    player = Obj_GetPlayerObject();
    target = *(char**)&((GroundBaddieState*)stateAddress)->baddie.targetObj;
    if (target != NULL) {
        delta.x = ((GameObject*)target)->anim.worldPosX - obj->anim.worldPosX;
        delta.y = ((GameObject*)target)->anim.worldPosY - obj->anim.worldPosY;
        delta.z = ((GameObject*)target)->anim.worldPosZ - obj->anim.worldPosZ;
        ((GroundBaddieState*)stateAddress)->baddie.targetDistance =
            sqrtf(delta.z * delta.z + (delta.x * delta.x + delta.y * delta.y));
    }

    if ((((GroundBaddieState*)objectStateAddress)->configFlags & 0x20) == 0) {
        (*gBaddieControlInterface)
            ->pollCameraTarget(obj, (void*)stateAddress, &((GroundBaddieState*)objectStateAddress)->flags400, 2, 3,
                               ((GroundBaddieState*)objectStateAddress)->soundIdA,
                               ((GroundBaddieState*)objectStateAddress)->soundIdB);
    }

    (*gBaddieControlInterface)
        ->processMessages(obj, (void*)stateAddress, (void*)(objectStateAddress + 0x35c),
                          ((GroundBaddieState*)objectStateAddress)->gameBitB, NULL, 0, 0, 8);

    hitReactionUpdated =
        (*gBaddieControlInterface)
            ->updateHitReaction(obj, (void*)stateAddress, (char*)objectStateAddress + 0x35c,
                                ((GroundBaddieState*)objectStateAddress)->gameBitB, gDllCEHitReactionMoves,
                                gDllCEHitReactionDamage, 1, gDllCEHitReactionScratch);

    if (hitReactionUpdated != 0) {
        void* playerChild = player->childObjs[0];

        (*(void (**)(void*))(**(int**)&((GameObject*)playerChild)->anim.dll + 0x50))(playerChild);
    }
}

void dll_CE_handleMessage(GameObject* obj, int message) {
    GroundBaddieState* objectState = obj->extra;
    GroundBaddieState* stateAlias = (GroundBaddieState*)(int)objectState;

    switch ((u8)message) {
    case DLL_CE_MESSAGE_HIDE:
        ((DllCEControl*)objectState->control)->coordinationFlags |= DLL_CE_COORDINATION_HIDDEN;
        Sfx_PlayFromObject((u32)obj, SFXTRIG_dn_boar1_c_264);
        (*gPlayerInterface)->setState((void*)obj, (void*)stateAlias, 1);
        stateAlias->baddie.substate = 4;
        *(s8*)&stateAlias->baddie.moveJustStartedB = 1;
        break;
    case DLL_CE_MESSAGE_RELEASE:
        objectState->configFlags &= ~4;
        break;
    }
}

s16 dll_CE_setScale(GameObject* obj) {
    return ((BaddieState*)obj->extra)->controlMode;
}

int dll_CE_getExtraSize(void) {
    return sizeof(GroundBaddieState) + sizeof(DllCEControl);
}

int dll_CE_getObjectTypeId(void) {
    return 0x49;
}

void dll_CE_free(GameObject* obj) {
    GroundBaddieState* state = obj->extra;

    ObjGroup_RemoveObject((int)obj, DLL_CE_OBJGROUP);
    {
        GameObject* child = obj->childObjs[0];

        if (child != NULL) {
            Obj_FreeObject(child);
            obj->childObjs[0] = NULL;
        }
    }
    (*gBaddieControlInterface)->releaseState(obj, state, 32);
}

void dll_CE_render(GameObject* obj, int fwdArg2, int fwdArg3, int fwdArg4, int fwdArg5, s8 visible) {
    GroundBaddieState* state = obj->extra;
    f32 alpha;
    f32 zero = 0.0f;

    if (visible == 0 || obj->userData1 != 0 || state->targetState == 0) {
        return;
    }
    alpha = state->glowAlpha;
    if (alpha != zero) {
        objSetGlowColor(200, 0, 0, alpha);
    }
    objRenderModelAndHitVolumes(obj, fwdArg2, fwdArg3, fwdArg4, fwdArg5, 1.0f);
}

void dll_CE_hitDetect(GameObject* obj) {
    (void)obj;
}

static void chukChuk_clearCheckHandlers(void)
{
    int i;
    for (i = 0; i < 6; i++)
        gChukChukCheckHandlers[i] = NULL;
}

void dll_CE_update(GameObject* obj, int unusedA, int unusedB) {
    GroundBaddieState* state;
    DllCEPlacement* placement;
    DllCEControl* control;
    int spawnCount;
    f32 sunTime;

    (void)unusedA;
    (void)unusedB;

    state = obj->extra;
    placement = (DllCEPlacement*)obj->anim.placementData;
    if (obj->userData1 != 0) {
        if ((state->baddie.substate != 3 || (state->configFlags & 1) != 0) &&
            (*gMapEventInterface)->shouldNotSaveTime(placement->base.mapId) != 0) {
            (*gBaddieControlInterface)->initGroundBaddie(obj, (u8*)placement, (u8*)state, 7, 6, 0x102, 0x26, 20.0f);
            state->targetState = 0;
            Sfx_PlayFromObject((u32)obj, SFXTRIG_dn_seal4_c_263);
            ObjAnim_SetCurrentMove((int)obj, 8, 0.0f, OBJANIM_MOVE_CONTROL_SKIP_EVENT_COUNTDOWN);
            *(s8*)&state->baddie.moveDone = 0;
            obj->anim.alpha = 0xff;
            *(u8*)&obj->anim.resetHitboxMode |= INTERACT_FLAG_DISABLED;
        }
    } else if (obj->userData2 == 0) {
        obj->anim.localPosX = placement->base.posX;
        obj->anim.localPosY = placement->base.posY;
        obj->anim.localPosZ = placement->base.posZ;
        (*gObjectTriggerInterface)->runSequence(placement->sequenceId, obj, -1);
        obj->userData2 = 1;
    } else {
        if ((*gBaddieControlInterface)->isObjectValid(obj, state, 0) == 0) {
            state->targetState = 0;
        } else if ((state->configFlags & 0x10) != 0 && (*gSkyInterface)->getSunPosition(&sunTime) == 0) {
            state->targetState = 0;
        } else {
            chukChuk_updateTargeting(obj, (int)state, (int)state);
            if (state->targetState == 0) {
                chukChuk_acquireTarget(obj, state, state);
            } else {
                control = state->control;
                if ((control->effectFlags & DLL_CE_EFFECT_PROJECTILE) != 0) {
                    chukChuk_spawnIceBall(obj, state);
                }
                if ((control->effectFlags & DLL_CE_EFFECT_DUST) != 0) {
                    (*gPartfxInterface)->spawnObject((void*)obj, DLL_CE_PARTFX_DUST, NULL, 1, -1, NULL);
                }
                if ((control->effectFlags & DLL_CE_EFFECT_SPRAY) != 0) {
                    spawnCount = 0;
                    do {
                        (*gPartfxInterface)->spawnObject((void*)obj, DLL_CE_PARTFX_SPRAY, NULL, 1, -1, NULL);
                        spawnCount++;
                    } while (spawnCount < 10);
                }
                control->effectFlags = 0;
                (*gBaddieControlInterface)->updateGravity(obj, state, 0.0f, -1);
                (*gPlayerInterface)->rotateTowardTarget(obj, state, timeDelta, 4);
                state->savedObjC0 = *(int*)&obj->pendingParentObj;
                *(int*)&obj->pendingParentObj = 0;
                (*gPlayerInterface)
                    ->update(obj, state, timeDelta, timeDelta, gChukChukMoveHandlers, gChukChukCheckHandlers);
                *(int*)&obj->pendingParentObj = state->savedObjC0;
            }
            obj->anim.localPosY = placement->base.posY - 2.0f;
        }
    }
}

void dll_CE_init(GameObject* obj, DllCEPlacement* placement, int flags) {
    GroundBaddieState* state;
    u8 mode;
    DllCEControl* control;

    state = obj->extra;
    mode = 6;
    if (flags != 0) {
        mode |= 1;
    }
    if ((placement->flags & 0x20) == 0) {
        mode |= 8;
    }
    (*gBaddieControlInterface)->initGroundBaddie(obj, (u8*)placement, (u8*)state, 7, 6, 0x102, mode, 20.0f);
    obj->animEventCallback = NULL;
    control = state->control;
    control->soundTimer = (f32)(int)randomGetRange(10, 300);
    ObjAnim_SetCurrentMove((int)obj, 8, 0.0f, 0);
    *(u8*)&obj->anim.resetHitboxMode |= INTERACT_FLAG_DISABLED;
    (*gPlayerInterface)->setState(obj, state, 0);
    state->baddie.substate = 0;
    *(s8*)&state->baddie.physicsActive = 0;
    ObjHits_DisableObject(obj);
}

void dll_CE_release(void) {
}

void dll_CE_initialise(void) {
    gChukChukMoveHandlers[0] = chukChuk_updateEmergeState;
    gChukChukMoveHandlers[1] = chukChuk_updateSubmergeState;
    gChukChukMoveHandlers[2] = chukChuk_updateAttackState;
    gChukChukMoveHandlers[3] = chukChuk_updateState3;
    gChukChukMoveHandlers[4] = chukChuk_updateSpitState;
    gChukChukMoveHandlers[5] = chukChuk_updateAlertState;
    gChukChukMoveHandlers[6] = chukChuk_updateWindupState;
    gChukChukCheckHandlers[0] = chukChuk_checkTargetState;
    gChukChukCheckHandlers[1] = chukChuk_checkHealthState;
    gChukChukCheckHandlers[2] = chukChuk_checkDeathState;
    gChukChukCheckHandlers[3] = chukChuk_checkYieldState;
    gChukChukCheckHandlers[4] = chukChuk_checkSubmergeState;
    gChukChukCheckHandlers[5] = chukChuk_checkChooseAttackState;
}
