#include "dlls/objects/599_DR_EarthWar.h"

#include "main/dll/partfx_interface.h"
#include "sys/objects.h"
#include "sys/objects/lifecycle.h"
#include "game/objects/object_setup.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/audio/sfx.h"
#include "main/gamebits.h"
#include "main/game_ui_interface.h"
#include "main/model.h"
#include "main/objHitReact.h"
#include "main/objhits.h"
#include "main/objanim.h"
#include "main/objseq.h"
#include "main/resource.h"
#include "main/dll/path_control_interface.h"
#include "main/objtype.h"
#include "main/obj_link.h"
#include "main/obj_path.h"
#include "main/frame_timing.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_trig_api.h"
#include "dlls/object_descriptor.h"
#include "main/dll/tricky_api.h"
#include "main/dll/dll_002E_moveLib.h"
#include "main/dll/dll_0282_barrelgener.h"
#include "main/camera.h"
#include "main/byte_flags.h"
#include "main/gamebit_ids.h"
#include "game/objects/object.h"
#include "main/object_render.h"
#include "main/objprint_anim_api.h"
#include "main/objprint_character_api.h"
#include "main/objprint_sound_api.h"
#include "main/objprint_api.h"
#include "main/pad.h"
#include "main/dll/baddie_state.h"
#include "main/dll/player_api.h"
#include "main/dll/player_motion_api.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/player_control_interface.h"
#include "main/maketex_timer_api.h"
#include "main/vecmath.h"
#include "dlls/objects/473_DIM2PrisonM.h"
#include "main/newshadows.h"
#include "main/newshadows_audio_api.h"

#define PAD_BUTTON_A 0x100

#define DREARTHWARRIOR_OBJGROUP           0xa
#define DREARTHWARRIOR_PARTFX             0x7e6
#define DREARTHWARRIOR_AIRMETER_BGTEXTURE 0x5cf /* HUD air-meter background texture id */

#define DREARTHWARRIOR_OBJFLAG_PARENT_SLACK 0x1000

#define DREARTHWARRIOR_CHILD_OBJ_HELPER 0x6f5
/* attacker romDefNo whose hits are ignored here (retail OBJECTS.bin). */
#define DREARTHWARRIOR_ATTACKER_SEQID_SWORD 0x23 /* "sword" (DLL 0xE2) */
#define DREARTHWARRIOR_EFFECT_RESOURCE_ID   0x5a /* shared effect resource -> gEarthWarriorResource */

f32 gEarthWarriorMatrix[16];
void* gDREarthWarriorStateHandlers[4];
void* gDREarthWarriorDefaultStateHandler;
void* gEarthWarriorResource;

const EWPathRange gDREarthWarriorLookInitData1 = {{10, 10, 0, 0, 0}};
const EWPathRange gDREarthWarriorLookInitData2 = {{20, 20, 0, 0, 0}};
const EWColorTable gDREarthWarriorColors = {
    {{8, 255, 190, 120}, {8, 255, 255, 120}, {8, 180, 240, 255}, {8, 170, 255, 170}}};
static const u8 gDREarthWarriorPathSetupParam[4] = {1, 1, 1, 1};

/* Legacy helper retained because its literals establish the retail MWCC pool group. */
static void DR_EarthWarrior_setupPathState(u8* pathState, DREarthWarriorInitData* base, EarthWarriorSub* warrior) {
    (*gPathControlInterface)->setup(pathState, 4, base->unkC, base->unk3C, (void*)gDREarthWarriorPathSetupParam);
    warrior->aimAccumY = 0.0f;
    warrior->aimHalfY = (f32)(s32)warrior->yawTurnDir;
}

void DR_EarthWarrior_feed(GameObject* obj, int mode) {
    EarthWarriorState* state = (obj)->extra;
    switch (mode) {
    case 1:
        state->sub.energy += 4;
        objSoundStartTimed(obj, &state->modelSoundState, 0x291, 0x1000, -1, 1);
        state->sub.unk8EC = 4.32f;
        lbl_8033527C[4].maxSpeed = state->sub.unk8EC;
        break;
    default:
        break;
    }
}

int DR_EarthWarrior_updateLeap(GameObject* obj, EarthWarriorSub* warrior, BaddieState* baddie) {
    *(u32*)&warrior->unk360 |= 0x1000000LL;
    baddie->moveSpeed = 0.035f;
    if ((obj)->anim.currentMoveProgress > 0.1f && (obj)->anim.currentMoveProgress < 0.25f &&
        baddie->animSpeedC > warrior->configRow[3].maxSpeed - 0.4f && baddie->inputMagnitude > 0.8f &&
        warrior->frameCounter >= 0x96) {
        ((ByteFlags*)&warrior->flags3F0)->b40 = 1;
        ((ByteFlags*)&warrior->flags3F0)->b80 = 0;
        warrior->soundId = warrior->soundIdReload;
        baddie->moveSpeed = 0.0165f;
        ObjAnim_SetCurrentMove((int)obj, warrior->moveTable[0x1d], 0.0f, 0);
        ObjAnim_SetCurrentEventStepFrames((struct ObjAnimComponent*)obj, 0x10);
        warrior->leapStartYaw = warrior->currentYaw;
        warrior->animSpeedRate = (0.2f + (warrior->configRow[2].maxSpeed + baddie->animSpeedC)) / 60.0f;
        warrior->appliedYaw = warrior->currentYaw;
        warrior->currentYaw += 0x8000;
        baddie->animSpeedC = -baddie->animSpeedC;
        baddie->animSpeedA = -baddie->animSpeedA;
    }
    if (((ByteFlags*)&warrior->flags3F0)->b80 != 0) {
        f32 lim;
        if (baddie->animSpeedC <= (lim = warrior->configRow[2].minSpeed) && baddie->animSpeedA <= lim) {
            warrior->savedYaw = warrior->currentYaw;
            ((ByteFlags*)&warrior->flags3F0)->b40 = 0;
            ((ByteFlags*)&warrior->flags3F0)->b80 = 0;
            return 1;
        }
        warrior->targetAnimSpeed = 0.0f;
        warrior->animSpeedSmoothing = warrior->animSpeedSmoothingReload;
        warrior->flags8D8 |= 8;
    }
    return 0;
}

/* Legacy helper retained because its literals establish the retail MWCC pool group. */
static void DR_EarthWarrior_applySlowTurn(GameObject* obj, EarthWarriorSub* warrior, BaddieState* baddie) {
    baddie->moveSpeed = 0.02f;
    warrior->yawSmoothDivisor *= 2.0f;
    warrior->yawStepScale *= 0.5f;
    warrior->targetAnimSpeed *= 0.75f;
    warrior->appliedYaw = (s16)(32768.0f * obj->anim.currentMoveProgress);
}

static void DR_EarthWarrior_updateSteeringPose(GameObject* obj, EarthWarriorSub* warrior, BaddieState* baddie) {
    int targetAngle;
    int angleDelta;
    s16* primaryLookBone;
    s16* secondaryLookBone;
    f32 responseScale;
    targetAngle = warrior->yawTurnDir << 1;
    if (targetAngle < -0x41) {
        angleDelta = -0x41;
    } else if (targetAngle > 0x41) {
        angleDelta = 0x41;
    } else {
        angleDelta = targetAngle;
    }
    angleDelta = angleDelta * 0xb6;
    angleDelta -= (u16)warrior->aimAccumY;
    if (angleDelta > 0x8000) {
        angleDelta = angleDelta - 0xffff;
    }
    if (angleDelta < -0x8000) {
        angleDelta = angleDelta + 0xffff;
    }
    responseScale = 0.15f;
    angleDelta *= responseScale;
    if (angleDelta < -0x16c) {
        angleDelta = -0x16c;
    } else if (angleDelta > 0x16c) {
        angleDelta = 0x16c;
    }
    warrior->aimAccumY = angleDelta * timeDelta + (f32)(s32) * (s16*)&warrior->aimAccumY;
    warrior->aimHalfY = warrior->aimAccumY / 2;
    {
        f32 step;
        f32 scale;
        f32 ph;

        ph = (f32)(s32)baddie->spawnRotY / 8192.0f;
        scale = 182.0f;
        step = 10.0f;
        angleDelta = (int)(scale * (step * -((ph < -1.0f) ? -1.0f : ((ph > 1.0f) ? 1.0f : ph))));
        angleDelta -= (u16)warrior->aimAccumX;
    }
    if (angleDelta > 0x8000) {
        angleDelta = angleDelta - 0xffff;
    }
    if (angleDelta < -0x8000) {
        angleDelta = angleDelta + 0xffff;
    }
    warrior->aimAccumX += angleDelta;
    primaryLookBone = objFindJointPoseVector(obj, 0);
    secondaryLookBone = objFindJointPoseVector(obj, 9);
    objFindJointPoseVector(obj, 4);
    objFindJointPoseVector(obj, 5);
    if (primaryLookBone != NULL) {
        int clampedY;
        primaryLookBone[0] = -warrior->aimAccumX;
        primaryLookBone[1] = warrior->aimAccumY / 2;
        clampedY = primaryLookBone[1];
        clampedY = (clampedY < -4000) ? -4000 : ((clampedY > 4000) ? 4000 : clampedY);
        primaryLookBone[1] = clampedY;
        primaryLookBone[2] = 0;
    }
    if (secondaryLookBone != NULL) {
        int clampedY;
        int absoluteHalfY;
        secondaryLookBone[1] = warrior->aimHalfY;
        clampedY = secondaryLookBone[1];
        clampedY = (clampedY < -3000) ? -3000 : ((clampedY > 3000) ? 3000 : clampedY);
        secondaryLookBone[1] = clampedY;
        absoluteHalfY = warrior->aimHalfY;
        if (absoluteHalfY < 0) {
            absoluteHalfY = -absoluteHalfY;
        }
        secondaryLookBone[0] = (s16)(absoluteHalfY >> 1);
    }
}
int DR_EarthWarrior_defaultStateHandler(void) {
    return 0x0;
}

int DR_EarthWarrior_stateHandler03(GameObject* obj, int baddie) {
    EarthWarriorState* state = (obj)->extra;
    f32 fz;
    (obj)->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
    fz = 0.0f;
    ((BaddieState*)baddie)->animSpeedC = fz;
    ((BaddieState*)baddie)->animSpeedB = fz;
    ((BaddieState*)baddie)->animSpeedA = fz;
    (obj)->anim.velocityX = fz;
    (obj)->anim.velocityY = fz;
    (obj)->anim.velocityZ = fz;
    if (((BaddieState*)baddie)->moveJustStartedA != 0) {
        if (((ByteFlags*)&state->sub.flags994)->b80) {
            ObjAnim_SetCurrentMove((int)obj, 7, fz, 0);
        } else {
            ObjAnim_SetCurrentMove((int)obj, 8, fz, 0);
        }
        ((BaddieState*)baddie)->moveSpeed = 0.02f;
    }
    if (((BaddieState*)baddie)->moveDone != 0) {
        if (state->sub.mountState == 2) {
            state->sub.energy -= 1;
            if (state->sub.energy <= 0) {
                state->sub.unk8EC = lbl_803DC76C;
                CameraShake_Enable();
                CameraShake_SetOffset(1.0f);
                playerAddHealth(Obj_GetPlayerObject(), -1);
                state->sub.energy = 0;
            }
            return state->sub.savedControlMode + 1;
        }
    }
    return 0;
}

int DR_EarthWarrior_stateHandler02(GameObject* obj, int controllerState) {
    EarthWarriorState* state = (obj)->extra;
    EarthWarriorSub* warrior = &state->sub;
#define hitState ((ObjHitsPriorityState*)(obj)->anim.hitReactState)
    ((ByteFlags*)&warrior->flags3F1)->b04 = 0;
    ((ByteFlags*)&warrior->flags3F1)->b08 = 0;
    ((ByteFlags*)&warrior->flags3F2)->b10 = 0;
    if (((EarthWarriorState*)controllerState)->baddie.moveJustStartedA != 0) {
        ((ByteFlags*)&warrior->flags3F0)->b80 = 0;
        ((ByteFlags*)&warrior->flags3F0)->b40 = 0;
        *(u8*)&warrior->attackPhase = 0;
        ((ByteFlags*)&warrior->flags3F2)->b10 = 1;
    }
    if (!((ByteFlags*)&warrior->flags3F0)->b80 && !((ByteFlags*)&warrior->flags3F0)->b40 &&
        !((ByteFlags*)&state->sub.flags994)->b01 &&
        (*(int*)&((EarthWarriorState*)controllerState)->baddie.pressedButtons & 0x100)) {
        buttonDisable(0, PAD_BUTTON_A);
        ((ByteFlags*)&state->sub.flags994)->b01 = 1;
        hitState->suppressOutgoingHits = 0;
        ObjAnim_SetCurrentMove((int)obj, 0x14, 0.0f, 0);
        ((EarthWarriorState*)controllerState)->baddie.moveDone = 0;
        Sfx_PlayFromObject(obj, SFXTRIG_earthhuff);
    }
    ((EarthWarriorState*)controllerState)->baddie.flags0 |= 0x800000;
    ((EarthWarriorState*)controllerState)->baddie.stateId = 0;
    warrior->animSpeedMax = 4.32f;
    if (((EarthWarriorState*)controllerState)->baddie.moveJustStartedA != 0) {
        warrior->currentYaw += warrior->turnDegrees * 0xb6;
        warrior->frameCounter = 0;
        warrior->turnDegrees = 0;
    }
    {
        f32 a;
        f32 ph = (((BaddieState*)controllerState)->inputMagnitude - 0.2f) / 0.8f;
        f32 t;
        a = warrior->animSpeedMax - 0.05f;
        t = (ph < 0.0f) ? 0.0f : ((ph > 1.0f) ? 1.0f : ph);
        warrior->targetAnimSpeed = a * (t * warrior->animSpeedScale);
    }
    if (((ByteFlags*)&warrior->flags3F0)->b40) {
        *(u32*)&warrior->unk360 |= 0x1000000LL;
        ((EarthWarriorState*)controllerState)->baddie.moveSpeed = 0.0165f;
        {
            s16 yaw = (32768.0f * (obj)->anim.currentMoveProgress + (f32)warrior->leapStartYaw);
            *(s16*)&warrior->appliedYaw = yaw;
            warrior->savedYaw = yaw;
        }
        if (((EarthWarriorState*)controllerState)->baddie.moveDone != 0) {
            s16 sw;
            ((ByteFlags*)&warrior->flags3F0)->b40 = 0;
            sw = warrior->currentYaw;
            warrior->appliedYaw = sw;
            warrior->savedYaw = sw;
            *(u8*)&warrior->attackPhase = 0xc;
            ((ByteFlags*)&warrior->flags3F1)->b04 = 1;
            ((ByteFlags*)&warrior->flags3F1)->b08 = 1;
        }
        ((EarthWarriorState*)controllerState)->baddie.animSpeedC =
            warrior->animSpeedRate * timeDelta + ((EarthWarriorState*)controllerState)->baddie.animSpeedC;
        warrior->targetAnimSpeed = 0.0f;
        if ((obj)->anim.currentMoveProgress > 0.1f && (obj)->anim.currentMoveProgress < 0.5f) {
            warrior->flags8D8 |= 8;
        }
    } else if (((ByteFlags*)&warrior->flags3F0)->b80) {
        if (DR_EarthWarrior_updateLeap(obj, warrior, (BaddieState*)controllerState) != 0) {
            return 2;
        }
    } else if (((ByteFlags*)&state->sub.flags994)->b01) {
        ((EarthWarriorState*)controllerState)->baddie.moveSpeed = 0.02f;
        if (((EarthWarriorState*)controllerState)->baddie.moveDone != 0) {
            ((ByteFlags*)&state->sub.flags994)->b01 = 0;
            ((ByteFlags*)&warrior->flags3F1)->b08 = 1;
            hitState->suppressOutgoingHits = 0;
        }
        {
            f32 m2;
            f32 m1;
            warrior->yawSmoothDivisor *= (m1 = 2.0f);
            warrior->yawStepScale *= (m2 = 0.5f);
            warrior->currentYawSmoothDivisor *= m1;
            warrior->currentYawStepRate *= m2;
        }
        warrior->targetAnimSpeed *= 0.75f;
        if (warrior->targetAnimSpeed < warrior->configRow[1].maxSpeed) {
            warrior->targetAnimSpeed = warrior->configRow[1].maxSpeed;
        }
        hitState->hitVolumePriority = 0x15;
        hitState->hitVolumeId = 2;
    }
    if (!((ByteFlags*)&state->sub.flags994)->b01 && !((ByteFlags*)&warrior->flags3F0)->b40 &&
        !((ByteFlags*)&warrior->flags3F0)->b80 &&
        ((EarthWarriorState*)controllerState)->baddie.animSpeedC > 0.3f + warrior->configRow[2].maxSpeed &&
        (warrior->unk470 < -0.3f || warrior->frameCounter >= 0x96)) {
        ((ByteFlags*)&warrior->flags3F0)->b80 = 1;
        *(u32*)&warrior->unk360 |= 0x1000000LL;
        warrior->animSpeedRate = ((EarthWarriorState*)controllerState)->baddie.animSpeedA;
        ObjAnim_SetCurrentMove((int)obj, warrior->moveTable[0x1e], 0.0f, 0);
        ((EarthWarriorState*)controllerState)->baddie.moveSpeed = 0.035f;
    }
    if (!((ByteFlags*)&warrior->flags3F0)->b80 && !((ByteFlags*)&warrior->flags3F0)->b40) {
        if (warrior->frameCounter < 0x96) {
            f32 v = interpolate((f32)warrior->yawTurnProgress, 1.0f / warrior->yawSmoothDivisor, timeDelta);
            f32 cap = timeDelta * (warrior->yawStepScale * warrior->yawStepRate);
            if (v > cap) {
                v = cap;
            }
            if (warrior->yawTurnDir < 0) {
                v = -v;
            }
            *(s16*)&warrior->appliedYaw = (182.044f * v + (f32)(s32)warrior->appliedYaw);
        }
        if (warrior->frameCounter < 0x96) {
            f32 v = interpolate((f32)warrior->frameCounter, 1.0f / warrior->currentYawSmoothDivisor, timeDelta);
            f32 cap = warrior->currentYawStepRate * timeDelta;
            if (v > cap) {
                v = cap;
            }
            if (warrior->turnDegrees < 0) {
                v = -v;
            }
            *(s16*)&warrior->currentYaw = (182.044f * v + (f32)(s32)warrior->currentYaw);
        } else if (((EarthWarriorState*)controllerState)->baddie.animSpeedC <= warrior->configRow[0].maxSpeed &&
                   ((EarthWarriorState*)controllerState)->baddie.animSpeedA <= warrior->configRow[1].maxSpeed) {
            warrior->currentYaw += warrior->turnDegrees * 0xb6;
        }
    }
    if (!((ByteFlags*)&warrior->flags3F0)->b40 && !((ByteFlags*)&warrior->flags3F1)->b04) {
        f32 r = interpolate(warrior->targetAnimSpeed - ((EarthWarriorState*)controllerState)->baddie.animSpeedC,
                            warrior->animSpeedSmoothing, timeDelta);
        r = (r < -0.1f * timeDelta) ? -0.1f * timeDelta : ((r > 0.1f * timeDelta) ? 0.1f * timeDelta : r);
        if (warrior->frameCounter >= 0x96 && r > 0.0f) {
            r = 2.0f * -r;
        }
        ((EarthWarriorState*)controllerState)->baddie.animSpeedC += r;
        ((EarthWarriorState*)controllerState)->baddie.animSpeedC =
            (((EarthWarriorState*)controllerState)->baddie.animSpeedC < warrior->configRow[0].minSpeed)
                ? warrior->configRow[0].minSpeed
                : ((((EarthWarriorState*)controllerState)->baddie.animSpeedC > warrior->animSpeedMax)
                       ? warrior->animSpeedMax
                       : ((EarthWarriorState*)controllerState)->baddie.animSpeedC);
        ((EarthWarriorState*)controllerState)->baddie.animSpeedB = 0.0f;
    } else {
        ((EarthWarriorState*)controllerState)->baddie.animSpeedC =
            (((EarthWarriorState*)controllerState)->baddie.animSpeedC < -warrior->animSpeedMax)
                ? -warrior->animSpeedMax
                : ((((EarthWarriorState*)controllerState)->baddie.animSpeedC > warrior->animSpeedMax)
                       ? warrior->animSpeedMax
                       : ((EarthWarriorState*)controllerState)->baddie.animSpeedC);
    }
    ((EarthWarriorState*)controllerState)->baddie.animSpeedA +=
        interpolate(((EarthWarriorState*)controllerState)->baddie.animSpeedC -
                        ((EarthWarriorState*)controllerState)->baddie.animSpeedA,
                    warrior->animSpeedASmoothing, timeDelta);
    if (!((ByteFlags*)&warrior->flags3F0)->b80 && !((ByteFlags*)&warrior->flags3F0)->b40 &&
        !((ByteFlags*)&state->sub.flags994)->b01) {
        f32 blend;
        int i2;
        int skip = 0;
        if (((ByteFlags*)&warrior->flags3F1)->b08) {
            skip = 1;
            blend = 0.0f;
        } else {
            blend = (obj)->anim.currentMoveProgress;
        }
        i2 = (warrior->attackPhase / 4) << 1;
        warrior->attackStage = (i2 >> 1) + 1;
        if (warrior->attackStage > 4) {
            warrior->attackStage = 4;
        }
        warrior->soundId = (warrior->attackStage > 3) ? 0xa : 8;
        {
            f32 v294 = ((EarthWarriorState*)controllerState)->baddie.animSpeedC;
            if (v294 < (&warrior->configRow[0].minSpeed)[i2]) {
                if (warrior->attackPhase == 4) {
                    if (((EarthWarriorState*)controllerState)->baddie.animSpeedA < warrior->configRow[2].minSpeed &&
                        ((BaddieState*)controllerState)->inputMagnitude < 0.2f) {
                        return 2;
                    }
                } else {
                    warrior->attackPhase -= 4;
                }
            } else if (v294 >= (&warrior->configRow[0].maxSpeed)[i2]) {
                if (warrior->attackPhase < 0x14) {
                    if (warrior->attackPhase == 0) {
                        blend = 0.85f;
                    }
                    if (v294 < warrior->animSpeedMax) {
                        *(u8*)&warrior->attackPhase += 4;
                    }
                }
            }
        }
        if ((skip != 0 || (void*)warrior->prevMoveTable != (void*)warrior->moveTable ||
             (obj)->anim.currentMove != warrior->moveTable[warrior->attackPhase]) &&
            (ObjAnim_GetCurrentEventCountdown(&obj->anim) == 0 || ((ByteFlags*)&warrior->flags3F2)->b10 != 0)) {
            if ((obj)->anim.currentMove == 0x14) {
                blend = 0.85f;
            }
            ObjAnim_SetCurrentMove((int)obj, warrior->moveTable[warrior->attackPhase], blend, 0);
        }
    }
    if (!((ByteFlags*)&warrior->flags3F0)->b80 && !((ByteFlags*)&warrior->flags3F0)->b40 &&
        !((ByteFlags*)&state->sub.flags994)->b01) {
        if (ObjAnim_SampleRootCurvePhase(&obj->anim, ((EarthWarriorState*)controllerState)->baddie.animSpeedC,
                                         (f32*)(controllerState + 0x2a0)) == 0) {
            ((EarthWarriorState*)controllerState)->baddie.moveSpeed = 0.005f;
        }
    }
    DR_EarthWarrior_updateSteeringPose(obj, warrior, (BaddieState*)controllerState);
    return 0;
}
#undef hitState

int DR_EarthWarrior_stateHandler01(GameObject* obj, int baddie) {
    EarthWarriorState* state = (obj)->extra;
    EarthWarriorSub* warrior = &state->sub;
    int moveId;
    if (((BaddieState*)baddie)->moveJustStartedA != 0) {
        ((BaddieState*)baddie)->animSpeedC = 0.0f;
    }
    ((BaddieState*)baddie)->animSpeedA -=
        interpolate(((BaddieState*)baddie)->animSpeedA, warrior->animSpeedASmoothing, timeDelta);
    if (((BaddieState*)baddie)->animSpeedA <= *(f32*)((char*)lbl_8033527C + 0x8)) {
        ((BaddieState*)baddie)->animSpeedA = 0.0f;
    }
    {
        f32 z = 0.0f;
        ((BaddieState*)baddie)->animSpeedB = z;
        (obj)->anim.velocityX = z;
        (obj)->anim.velocityZ = z;
    }
    if (!((ByteFlags*)&warrior->flags3F0)->b80 && !((ByteFlags*)&warrior->flags3F0)->b40 &&
        !((ByteFlags*)&state->sub.flags994)->b01 && (*(int*)&((BaddieState*)baddie)->pressedButtons & 0x100)) {
        buttonDisable(0, PAD_BUTTON_A);
        ((ByteFlags*)&state->sub.flags994)->b01 = 1;
        ((ObjHitsPriorityState*)(obj)->anim.hitReactState)->suppressOutgoingHits = 0;
        ObjAnim_SetCurrentMove((int)obj, 0x14, 0.0f, 0);
        ((BaddieState*)baddie)->moveDone = 0;
        return 3;
    }
    if (*(f32*)&((EarthWarriorState*)baddie)->baddie.trackedObj >= 0.22f &&
        ((BaddieState*)baddie)->inputMagnitude >= 0.22f &&
        ((BaddieState*)baddie)->animSpeedC >= warrior->configRow[0].maxSpeed) {
        return 3;
    }
    moveId = warrior->moveTable[0];
    ((BaddieState*)baddie)->stateId = 0;
    warrior->animSpeedMax = 4.32f;
    {
        f32 a;
        f32 ph = (((BaddieState*)baddie)->inputMagnitude - 0.2f) / 0.8f;
        f32 t;
        a = warrior->animSpeedMax - 0.05f;
        t = (ph < 0.0f) ? 0.0f : ((ph > 1.0f) ? 1.0f : ph);
        warrior->targetAnimSpeed = a * (t * warrior->animSpeedScale);
    }
    ((BaddieState*)baddie)->animSpeedC += interpolate(warrior->targetAnimSpeed - ((BaddieState*)baddie)->animSpeedC,
                                                      warrior->animSpeedSmoothing, timeDelta);
    if (((BaddieState*)baddie)->moveJustStartedA != 0) {
        warrior->yawTurnProgress = 0;
        warrior->yawTurnDir = 0;
        warrior->frameCounter = 0;
        warrior->turnDegrees = 0;
        warrior->soundId = 8;
        warrior->attackStage = 0;
        ((BaddieState*)baddie)->velSmoothTime = 8.0f;
        ((BaddieState*)baddie)->moveSpeed = 0.005f;
    }
    if ((obj)->anim.currentMove == warrior->moveTable[0x18] ||
        (obj)->anim.currentMove == warrior->moveTable[0x19]) {
        if (((BaddieState*)baddie)->moveDone != 0 && ObjAnim_GetCurrentEventCountdown(&obj->anim) == 0 &&
            !((ByteFlags*)&state->sub.flags994)->b01) {
            ObjAnim_SetCurrentMove((int)obj, moveId, 0.0f, 0);
            ((BaddieState*)baddie)->moveSpeed = 0.005f;
        }
    } else if (!((ByteFlags*)&state->sub.flags994)->b01) {
        ObjAnim_SetCurrentMove((int)obj, moveId, 0.0f, 0);
        ((BaddieState*)baddie)->moveSpeed = 0.005f;
    }
    {
        f32 v = interpolate((f32)warrior->yawTurnProgress, 1.0f / warrior->yawSmoothDivisor, timeDelta);
        f32 cap = timeDelta * (warrior->yawStepScale * warrior->yawStepRate);
        v = (v < cap) ? v : cap;
        if (warrior->yawTurnDir < 0) {
            v = -v;
        }
        *(s16*)&warrior->appliedYaw = (182.044f * v + (f32)(s32)warrior->appliedYaw);
    }
    {
        f32 v = interpolate((f32)warrior->frameCounter, 1.0f / warrior->currentYawSmoothDivisor, timeDelta);
        f32 cap = warrior->currentYawStepRate * timeDelta;
        v = (v < cap) ? v : cap;
        if (warrior->turnDegrees < 0) {
            v = -v;
        }
        *(s16*)&warrior->currentYaw = (182.044f * v + (f32)(s32)warrior->currentYaw);
    }
    DR_EarthWarrior_updateSteeringPose(obj, warrior, (BaddieState*)baddie);
    return 0;
}

int DR_EarthWarrior_stateHandler00(GameObject* obj) {
    EarthWarriorState* state = obj->extra;
    state->sub.flags98C |= 0x20;
    return 2;
}

int DR_EarthWarrior_SeqFn(GameObject* obj, int unused, ObjSeqState* animUpdate) {
    EarthWarriorState* state = (obj)->extra;
    int i;
    f32 fz;
    (obj)->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
    if (dll_2E_updateSequenceTurn(obj, (ObjSeqState*)animUpdate, &state->moveLib, 0, 0) != 0) {
        return 1;
    }
    for (i = 0; i < animUpdate->eventCount; i++) {
        int eventId = animUpdate->eventIds[i];
        switch (eventId) {
        case 0xa:
            break;
        case 0xe:
        case 0xf:
            state->moveLib.modeBits |= 1;
            ((ObjHitsPriorityState*)(obj)->anim.hitReactState)->shapeFlags &= ~0x20;
            break;
        case 0x10:
            state->moveLib.modeBits &= ~1;
            ((ObjHitsPriorityState*)(obj)->anim.hitReactState)->shapeFlags |= 0x20;
            break;
        }
    }
    *(u32*)&state->sub.unk360 |= 0x800000LL;
    (*gPathControlInterface)->attachObject((void*)obj, (char*)&state->baddie + 4);
    fz = 0.0f;
    state->baddie.animSpeedC = fz;
    state->baddie.animSpeedB = fz;
    state->baddie.animSpeedA = fz;
    (obj)->anim.velocityX = fz;
    (obj)->anim.velocityY = fz;
    (obj)->anim.velocityZ = fz;
    return 0;
}

void DR_EarthWarrior_handleRiderScale(GameObject* obj, f32 scale) {
    MatrixTransform v;
    f32 lp0, lp1, lp2;
    int mtx = ObjPath_GetPointModelMtx(obj, 2);
    ObjPath_GetPointLocalPosition(obj, 2, &lp0, &lp1, &lp2);
    v.x = lp0;
    v.y = lp1;
    v.z = lp2;
    v.rotX = 0;
    v.rotY = 0;
    v.rotZ = 0;
    v.scale = scale / (obj)->anim.modelInstance->rootMotionScaleBase;
    setMatrixFromObjectPos(gEarthWarriorMatrix, &v);
    mtx44_mult(gEarthWarriorMatrix, (void*)mtx, gEarthWarriorMatrix);
    objSetModelMatrixOverride(gEarthWarriorMatrix);
}

void DR_EarthWarrior_resetToRomListPosition(void) {
}

int DR_EarthWarrior_getRacePosition(void) {
    return 0x0;
}

f32 DR_EarthWarrior_func19(GameObject* obj, f32* out) {
    EarthWarriorState* state = obj->extra;
    f32 animSpeed;
    animSpeed = 0.001f * state->baddie.animSpeedC + 0.005f;
    *out = -((animSpeed < 0.005f) ? 0.005f : ((animSpeed > 0.01f) ? 0.01f : animSpeed));
    return 0.0f;
}

void DR_EarthWarrior_getPlayerAnim(GameObject* obj, f32* steeringAngle, int* leanAngle) {
    EarthWarriorState* state = obj->extra;
    *steeringAngle = (f32)(s32)state->sub.aimAccumY;
    *leanAngle = state->sub.aimAccumX;
}

void DR_EarthWarrior_setMountState(GameObject* obj, int mountState) {
    EarthWarriorState* state = obj->extra;
    state->sub.mountState = mountState;
    if (mountState == 0) {
        mainSetBits(0x7bc, 0);
        mainSetBits(0x7d4, 1);
        state->moveLib.modeBits &= ~1;
        ((ByteFlags*)&state->sub.flags994)->b02 = 0;
        (*gGameUIInterface)->airMeterShutdown();
    } else {
        EarthWarriorState* reloadedState = obj->extra;
        DREarthWarriorPlacement* placement = (DREarthWarriorPlacement*)obj->anim.placementData;
        ((ByteFlags*)&reloadedState->sub.flags994)->b02 = 1;
        (*gGameUIInterface)->initAirMeter(placement->energyCapacity, DREARTHWARRIOR_AIRMETER_BGTEXTURE);
        (*gGameUIInterface)->runAirMeter(reloadedState->sub.energy);
        mainSetBits(0x7bc, 1);
        mainSetBits(0x7d4, 0);
    }
}

int DR_EarthWarrior_getMountState(void) {
    return 0x0;
}

void DR_EarthWarrior_getCameraPosition(GameObject* obj, f32* x, f32* y, f32* z) {
    *x = obj->anim.localPosX;
    *y = obj->anim.localPosY;
    *z = obj->anim.localPosZ;
}

int DR_EarthWarrior_getDismountSide(GameObject* obj) {
    EarthWarriorState* state = obj->extra;
    if (state->sub.dismountSide != 0) {
        return 2;
    }
    return 1;
}

int DR_EarthWarrior_canDismount(void) {
    return 0x0;
}

void DR_EarthWarrior_getRiderPosition(GameObject* obj, f32* x, f32* y, f32* z) {
    EarthWarriorState* state = obj->extra;
    *x = state->sub.riderPosX;
    *y = state->sub.riderPosY;
    *z = state->sub.riderPosZ;
}

int DR_EarthWarrior_getMountSide(GameObject* obj) {
    EarthWarriorState* state = obj->extra;
    if (state->sub.mountSide != 0) {
        return 1;
    }
    return 2;
}

int DR_EarthWarrior_canMount(void) {
    return 0x0;
}

int DR_EarthWarrior_getExtraSize(void) {
    return 0x14fc;
}

int DR_EarthWarrior_getObjectTypeId(void) {
    return 0x43;
}

void DR_EarthWarrior_free(GameObject* obj) {
    EarthWarriorState* state = (obj)->extra;
    if (state->sub.modelChain != NULL) {
        ObjModelChain_Free(state->sub.modelChain);
    }
    objFreeObjectType((int)obj, DREARTHWARRIOR_OBJGROUP);
    if (((ByteFlags*)&state->sub.flags994)->b02) {
        (*gGameUIInterface)->airMeterShutdown();
    }
    if (state->helperObj != NULL) {
        ObjLink_DetachChild(obj, state->helperObj);
        Obj_FreeObject(state->helperObj);
    }
}

void DR_EarthWarrior_render(GameObject* obj, int gdl, int mtxs, int vtxs, int pols, s8 visibility) {
    EarthWarriorState* state = (obj)->extra;
    if (visibility == -1) {
        objRenderModelAndHitVolumes(obj, gdl, mtxs, vtxs, pols, 1.0f);
        ObjPath_GetPointWorldPosition(obj, 0xb, &state->sub.riderPosX, &state->sub.riderPosY, &state->sub.riderPosZ, 0);
        ObjPath_GetPointWorldPositionArray(obj, 3, 4, (f32*)state->pathPoints);
    } else if (visibility != 0) {
        objRenderModelAndHitVolumes(obj, gdl, mtxs, vtxs, pols, 1.0f);
        ObjPath_GetPointWorldPosition(obj, 0xb, &state->sub.riderPosX, &state->sub.riderPosY, &state->sub.riderPosZ, 0);
        ObjPath_GetPointWorldPositionArray(obj, 3, 4, (f32*)state->pathPoints);
        dll_2E_setTargetFromPathPoint(obj, &state->moveLib, 0);
    }
}

void DR_EarthWarrior_hitDetect(GameObject* obj) {
    f32 hz;
    f32 hy;
    f32 hx;
    void* hitObj;
    struct {
        s16 angles[4];
        f32 mat[4];
    } v;
    EWColorTable rows;
    EarthWarriorState* state = obj->extra;
    ObjHitsPriorityState* hitState = (ObjHitsPriorityState*)obj->anim.hitReactState;
    rows = gDREarthWarriorColors;
    if (!(obj->objectFlags & DREARTHWARRIOR_OBJFLAG_PARENT_SLACK)) {
        if (hitState->contactFlags != 0) {
            int i = hitState->contactHitVolume;
            i = (i < 0) ? 0 : ((i > 0x23) ? 0x23 : i);
            v.mat[0] = 1.0f;
            v.angles[2] = 0;
            v.angles[1] = 0;
            v.angles[0] = 0;
            v.mat[1] = hitState->contactPosX;
            v.mat[2] = hitState->contactPosY;
            v.mat[3] = hitState->contactPosZ;
            (*(void (*)(int, int, void*, int, int, void*))(*(int*)(*(int*)gEarthWarriorResource + 0x4)))(
                0, 1, &v, 0x401, -1, rows.rows[gDREarthWarriorRowIndices[i]]);
            ((ObjHitsPriorityState*)obj->anim.hitReactState)->suppressOutgoingHits = 1;
            doRumble(10.0f);
        }
        if (hitState->lastHitObject != 0) {
            doRumble(10.0f);
        }
        obj->anim.rotX = state->sub.appliedYaw;
        if (state->baddie.controlMode != 3) {
            int hit = ObjHits_GetPriorityHitWithPosition(obj, (int*)&hitObj, 0, 0, &hx, &hy, &hz);
            if (hit != 0) {
                if (objGetFlagsE5_2((u8*)obj) != 0 && state->sub.mountState == 2) {
                    return;
                }
                Obj_SpawnHitLightAndFade(obj, (const Vec3f*)&hx, 5.0f);
                if (hit == 0x1a || hitObj == Obj_GetPlayerObject() ||
                    ((GameObject*)hitObj)->anim.romDefNo == DREARTHWARRIOR_ATTACKER_SEQID_SWORD) {
                    return;
                }
                {
                    objSoundStartTimed(obj, &state->modelSoundState, 0x28e, 0x1000, -1, 1);
                    {
                        s16 d = obj->anim.rotX - (u16)((GameObject*)hitObj)->anim.rotX;
                        if (d > 0x8000) {
                            d = (s16)(d - 0xffff);
                        }
                        if (d < -0x8000) {
                            d += 0xffff;
                        }
                        if (d > 0x4000 || d < -0x4000) {
                            ((ByteFlags*)&state->sub.flags994)->b80 = 0;
                        } else {
                            ((ByteFlags*)&state->sub.flags994)->b80 = 1;
                        }
                    }
                    state->sub.savedControlMode = state->baddie.controlMode;
                    (*gPlayerInterface)->setState(obj, state, 3);
                }
            }
        }
        if (*(int*)state & 0x800000) {
            if ((state->baddie.groundContact != 0 || (state->baddie.surfaceFlags & 0xf0)) &&
                state->sub.footstepCooldown <= 0.0f && state->baddie.animSpeedA > 3.408f) {
                doRumble((f32)(int)randomGetRange(2, 5));
                state->sub.footstepCooldown = 30.0f;
                Sfx_PlayFromObject(obj, SFXTRIG_foot_run_jingle4);
            }
            if (state->baddie.groundContact != 0 || (((ObjHitsPriorityState*)obj->anim.hitReactState)->flags & 8)) {
                f32 spd;
                f32 vcos;
                f32 vsin;
                spd = sqrtf(obj->anim.velocityX * obj->anim.velocityX + obj->anim.velocityZ * obj->anim.velocityZ);
                obj->anim.velocityX = oneOverTimeDelta * (obj->anim.worldPosX - obj->anim.previousWorldPosX);
                obj->anim.velocityZ = oneOverTimeDelta * (obj->anim.worldPosZ - obj->anim.previousWorldPosZ);
                vcos = mathSinf((3.1415927f * (f32)(s32)state->sub.currentYaw) / 32768.0f);
                vsin = mathCosf((3.1415927f * (f32)(s32)state->sub.currentYaw) / 32768.0f);
                state->baddie.animSpeedA = -obj->anim.velocityZ * vsin - obj->anim.velocityX * vcos;
                state->baddie.animSpeedA *= 2.0f;
                state->baddie.animSpeedA =
                    (state->baddie.animSpeedA < 1.2960001f)
                        ? 1.2960001f
                        : ((state->baddie.animSpeedA > state->sub.animSpeedMax) ? state->sub.animSpeedMax
                                                                                : state->baddie.animSpeedA);
                state->baddie.animSpeedA = (state->baddie.animSpeedA < 0.0f)
                                               ? 0.0f
                                               : ((state->baddie.animSpeedA > spd) ? spd : state->baddie.animSpeedA);
                if (!((ByteFlags*)&state->sub.flags3F0)->b40) {
                    state->baddie.animSpeedC = state->baddie.animSpeedA;
                }
            }
            state->baddie.flags0 &= ~0x800000;
        }
        state->sub.footstepCooldown -= timeDelta;
        if (state->sub.footstepCooldown < 0.0f) {
            state->sub.footstepCooldown = 0.0f;
        }
        if ((void*)state != NULL) {
            ObjModelChain_AdvancePhase(state->sub.modelChain);
        }
    }
}

void DR_EarthWarrior_runController(GameObject* obj, int updateRate, int frameIndex) {
    int state = *(int*)&obj->extra;
    int sub;
    int slot;
    Obj_GetPlayerObject();
    sub = state + 0xb58;
    slot = (int)Camera_GetCurrent();
    ((EarthWarriorState*)state)->baddie.hitPoints = 0;
    ((EarthWarriorState*)state)->baddie.flags0 &= ~0x8000;
    if (((EarthWarriorState*)state)->sub.mountState == 2) {
        ((EarthWarriorState*)state)->baddie.moveInputX = (f32)padGetStickX(0);
        ((EarthWarriorState*)state)->baddie.moveInputZ = (f32)padGetStickY(0);
        *(int*)&((EarthWarriorState*)state)->baddie.pressedButtons = getButtonsJustPressed(0);
        *(int*)&((EarthWarriorState*)state)->baddie.heldButtons = getButtonsHeld(0);
        ((EarthWarriorState*)state)->baddie.cameraYaw = *(s16*)slot;
    } else {
        f32 v = 0.0f;
        ((EarthWarriorState*)state)->baddie.moveInputX = v;
        ((EarthWarriorState*)state)->baddie.moveInputZ = v;
        *(int*)&((EarthWarriorState*)state)->baddie.pressedButtons = 0;
        *(int*)&((EarthWarriorState*)state)->baddie.heldButtons = 0;
        ((EarthWarriorState*)state)->baddie.cameraYaw = 0;
    }
    ((EarthWarriorState*)state)->baddie.flags0 |= 0x1000000;
    playerUpdateMotionState(obj, sub, state);
    (*gPlayerInterface)
        ->update(obj, (void*)state, timeDelta, timeDelta, gDREarthWarriorStateHandlers,
                 &gDREarthWarriorDefaultStateHandler);
    obj->anim.rotY = (s16)(obj->anim.rotY + (((EarthWarriorState*)state)->baddie.spawnRotY >> 2));
    obj->anim.rotZ = (s16)(obj->anim.rotZ + (((EarthWarriorState*)state)->baddie.spawnRotZ >> 2));
    if (((ByteFlags*)&((EarthWarriorState*)state)->sub.flags994)->b02) {
        (*gGameUIInterface)->runAirMeter(((EarthWarriorState*)state)->sub.energy);
    }
    playerUpdateVelocityFromMotion(obj, sub, state, timeDelta);
    playerClampVelocityAndMove(obj, timeDelta);
    (*gPathControlInterface)->update((void*)obj, (void*)(state + 4), timeDelta);
    (*gPathControlInterface)->apply((void*)obj, (void*)(state + 4));
    (*gPathControlInterface)->advance((void*)obj, (void*)(state + 4), timeDelta);
    obj->anim.rotX = ((EarthWarriorSub*)sub)->appliedYaw;
}

void DR_EarthWarrior_update(GameObject* obj) {
    EarthWarriorState* state = (obj)->extra;
    int j;
    int i;
#define hitState ((ObjHitsPriorityState*)(obj)->anim.hitReactState)
    Obj_GetPlayerObject();
    hitState->hitVolumePriority = 0;
    hitState->hitVolumeId = 0;
    if (state->helperObj == NULL && Obj_IsLoadingLocked() != 0) {
        ObjPlacement* setup = Obj_AllocObjectSetup(0x18, DREARTHWARRIOR_CHILD_OBJ_HELPER);
        GameObject* newObj = objSetupObject(setup, 4, (obj)->anim.mapEventSlot, -1, (obj)->anim.parent);
        ObjLink_AttachChild(obj, newObj, 2);
        state->helperObj = newObj;
    }
    state->sub.turnThreshold = 5;
    (obj)->anim.resetHitboxFlags &= ~INTERACT_FLAG_DISABLED;
    if (state->sub.mountState == 2) {
        setAButtonIcon(0x13);
        (obj)->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
        hitState->lateralResponseWeight = 0xf4;
        hitState->axialResponseWeight = 0xf4;
        DR_EarthWarrior_runController(obj, timeDelta, -1);
    } else {
        f32 z;
        hitState->lateralResponseWeight = 0;
        hitState->axialResponseWeight = 0;
        z = 0.0f;
        state->baddie.animSpeedC = z;
        state->baddie.animSpeedB = z;
        state->baddie.animSpeedA = z;
        (obj)->anim.velocityX = z;
        (obj)->anim.velocityY = z;
        (obj)->anim.velocityZ = z;
        DR_EarthWarrior_runController(obj, framesThisStep, -1);
    }
    characterDoEyeAnims(obj, &state->eyeAnimState);
    objSoundUpdateMouth(obj, &state->modelSoundState);
    dll_2E_updateLookAt(obj, &state->moveLib);
    if ((obj)->anim.resetHitboxFlags & INTERACT_FLAG_ACTIVATED) {
        ((ByteFlags*)&state->sub.flags994)->b10 = 1;
        if ((*gGameUIInterface)->isItemBeingUsed(0xc1) != 0) {
            (*gObjectTriggerInterface)->runSequence(1, (void*)obj, -1);
            buttonDisable(0, PAD_BUTTON_A);
            state->sub.energy += 4;
            mainSetBits(GAMEBIT_ITEM_TrickyFood_Count, mainGetBit(GAMEBIT_ITEM_TrickyFood_Count) - 1);
        } else if (state->sub.talkSequenceId != -1) {
            if ((*gGameUIInterface)->isAnyItemBeingUsed() == 0) {
                if (((ByteFlags*)&state->sub.flags994)->b08 == 0) {
                    (*gObjectTriggerInterface)->runSequence(state->sub.talkSequenceId, (void*)obj, -1);
                    buttonDisable(0, PAD_BUTTON_A);
                } else {
                    ((ByteFlags*)&state->sub.flags994)->b10 = 1;
                }
            }
        }
    }
    state->baddie.surfaceFlags |= 0x10;
    {
        f32 saved = (obj)->anim.velocityY;
        (obj)->anim.velocityY = 0.0f;
        state->baddie.eventFlags &= ~7;
        objAudioDispatchEventMask(obj, state->baddie.eventFlags, state->sub.soundId, state->pathPoints,
                                  (void*)((char*)state + 0x4), state->baddie.animSpeedA,
                                  (state->sub.soundId == 8) ? 2.5f : 2.75f);
        (obj)->anim.velocityY = saved;
    }
    if (state->sub.flags8D8 & 8) {
        f32 vecA[3];
        struct {
            s16 angles[4];
            f32 mat[4];
        } w;
        vecA[0] = 0.05f * (obj)->anim.velocityX;
        vecA[1] = 0.0f;
        vecA[2] = 0.05f * (obj)->anim.velocityZ;
        for (i = 0; i < 4; i++) {
            w.mat[1] = 8.0f * (obj)->anim.velocityX + state->pathPoints[i].x;
            w.mat[2] = state->pathPoints[i].y;
            w.mat[3] = 8.0f * (obj)->anim.velocityZ + state->pathPoints[i].z;
            w.mat[0] = 1.0f;
            w.angles[0] = 2;
            for (j = 2; j != 0; j--) {
                (*gPartfxInterface)->spawnObject((void*)obj, DREARTHWARRIOR_PARTFX, &w, 0x200001, -1, vecA);
            }
        }
        state->sub.flags8D8 &= ~8;
    }
#undef hitState
}
void DR_EarthWarrior_init(GameObject* obj, DREarthWarriorPlacement* def) {
    DREarthWarriorInitData* base = (DREarthWarriorInitData*)gDREarthWarriorInitData;
    EarthWarriorState* state = (obj)->extra;
    u32 stk = *(const u32*)gDREarthWarriorPathSetupParam;
    EWPathRange r2 = gDREarthWarriorLookInitData1;
    EWPathRange r1 = gDREarthWarriorLookInitData2;
    u8* pathState;
    (obj)->anim.rotX = (s16)(def->spawnYaw << 8);
    (obj)->animEventCallback = DR_EarthWarrior_SeqFn;
    objAddObjectType((int)obj, DREARTHWARRIOR_OBJGROUP);
    state->sub.setupVariant = def->setupVariant;
    state->sub.turnThreshold = 5;
    state->sub.talkSequenceId = -1;
    (*gPlayerInterface)->init(obj, state, 4, 1);
    state->baddie.flags0 |= 0x4000;
    state->baddie.gravity = 0.17f;
    pathState = (u8*)&state->baddie + 4;
    (*gPathControlInterface)->init(pathState, 0, 0x48683, 1);
    (*gPathControlInterface)->setup(pathState, 4, base->unkC, base->unk3C, &stk);
    (*gPathControlInterface)->setLocalPointCollision(pathState, 1, base->unk4C, base->unk64, 8);
    pathState[0x264] = 0x28;
    (*gPathControlInterface)->attachObject((void*)obj, pathState);
    ObjHits_EnableObject(obj);
    ((ObjHitsPriorityState*)(obj)->anim.hitReactState)->trackContactMask = 9;
    dll_2E_initState(obj, &state->moveLib, -0x2000, 0x31c7, 2);
    dll_2E_setMoveTables(&state->moveLib, &r1, &r2, 2);
    dll_2E_setLookAtMaxDistance(&state->moveLib, 150.0f);
    state->moveLib.modeBits |= 2;
    state->sub.unk8EC = 4.32f;
    state->sub.energy = def->energyCapacity;
    state->sub.moveTable = (const s16*)base->moveTable;
    state->sub.configRow = (const EWSpeedRange*)base->configRow;
    {
        f32 v = 1.0f;
        state->sub.unk834 = v;
        state->sub.animSpeedASmoothing = v;
    }
    state->sub.animSpeedSmoothingReload = 0.06f;
    state->sub.paramCurve0 = base->paramCurve0Data;
    state->sub.paramCurve0Count = 0x29;
    state->sub.paramCurve1 = base->paramCurve1Data;
    state->sub.paramCurve1Count = 0x29;
    state->sub.paramCurve2 = base->paramCurve2Data;
    state->sub.paramCurve2Count = 0x2e;
    state->sub.paramCurve3 = base->paramCurve1Data;
    state->sub.paramCurve3Count = 0x29;
    state->sub.paramCurve4 = base->paramCurve2Data;
    state->sub.paramCurve4Count = 0x2e;
    state->sub.unk7E0 = 5.555f;
    {
        s16 h = (obj)->anim.rotX;
        state->sub.savedYaw = h;
        state->sub.unk474 = h;
        state->sub.currentYaw = h;
        state->sub.appliedYaw = h;
    }
    ((ByteFlags*)&state->sub.flags994)->b08 = 0;
    state->sub.talkSequenceId = 2;
    storeZeroToFloatParam(&state->sub.airMeterTimer);
    s16toFloat(&state->sub.airMeterTimer, 0x1e);
    ((ByteFlags*)&state->sub.flags994)->b02 = 0;
    state->sub.unk99D = 1;
    state->helperObj = NULL;
    if (mainGetBit(0x9ec) != 0) {
        state->sub.unk995 = 1;
    }
    state->sub.modelChain = ObjModelChain_Alloc(&gEarthWarriorTailChainDesc, 1);
    ObjModelChain_SetOrigin(state->sub.modelChain, 0.15f, 0.75f, -0.05f);
    obj->afterBonesCallback = dim2prisonmammoth_updateModelChain;
    ObjModelChain_SetEnabled(state->sub.modelChain, 1);
}

void DR_EarthWarrior_release(void) {
    if (gEarthWarriorResource != NULL) {
        Resource_Release(gEarthWarriorResource);
        gEarthWarriorResource = NULL;
    }
}

void DR_EarthWarrior_initialise(void) {
    ((void**)gDREarthWarriorStateHandlers)[0] = DR_EarthWarrior_stateHandler00;
    ((void**)gDREarthWarriorStateHandlers)[1] = DR_EarthWarrior_stateHandler01;
    ((void**)gDREarthWarriorStateHandlers)[2] = DR_EarthWarrior_stateHandler02;
    ((void**)gDREarthWarriorStateHandlers)[3] = DR_EarthWarrior_stateHandler03;
    gDREarthWarriorDefaultStateHandler = DR_EarthWarrior_defaultStateHandler;
    if (gEarthWarriorResource == NULL) {
        gEarthWarriorResource = Resource_Acquire(DREARTHWARRIOR_EFFECT_RESOURCE_ID, 1);
    }
}

u8 gDREarthWarriorInitData[132] = {
    0x02, 0x8F, 0x08, 0x00, 0x01, 0x00, 0x02, 0x90, 0x10, 0x00, 0x03, 0x00, 0xC1, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xC1, 0x40, 0x00, 0x00, 0x41, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC1, 0x40, 0x00, 0x00, 0x41, 0x10,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x41, 0x40, 0x00, 0x00, 0xC1, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x41,
    0x40, 0x00, 0x00, 0x3D, 0xCC, 0xCC, 0xCD, 0x3D, 0xCC, 0xCC, 0xCD, 0x3D, 0xCC, 0xCC, 0xCD, 0x3D, 0xCC, 0xCC, 0xCD,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xC2, 0x0C, 0x00, 0x00, 0x42, 0x0C, 0x00, 0x00, 0x40, 0xA0, 0x00, 0x00, 0x40, 0xA0, 0x00, 0x00, 0x40, 0xA0,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x0A,
};

EWSpeedRange lbl_8033527C[6] = {
    {0.005f, 0.24000001f}, {0.192f, 1.2960001f}, {1.248f, 2.256f},
    {2.2080002f, 3.408f},  {3.3600001f, 4.32f},  {4.3f, 4.32f},
};

u8 gDREarthWarriorRowIndices[960] = {
    0,  0,   0,   0,   0,  0,   0,   0,   0,  0,   0,   0,   0,  0,   0,   0,   0,  0,   0,   0,   0,  0,   0,   0,
    1,  2,   0,   0,   0,  0,   0,   0,   0,  0,   0,   0,   0,  2,   0,   2,   0,  2,   0,   2,   0,  22,  0,   22,
    0,  22,  0,   22,  0,  22,  0,   22,  0,  22,  0,   22,  0,  22,  0,   22,  0,  22,  0,   22,  0,  4,   0,   4,
    0,  4,   0,   4,   0,  4,   0,   4,   0,  4,   0,   4,   0,  2,   0,   2,   0,  2,   0,   2,   0,  2,   0,   28,
    0,  27,  0,   2,   65, 64,  0,   0,   65, 64,  0,   0,   65, 64,  0,   0,   65, 64,  0,   0,   65, 64,  0,   0,
    65, 64,  0,   0,   65, 64,  0,   0,   65, 64,  0,   0,   65, 64,  0,   0,   65, 64,  0,   0,   65, 64,  0,   0,
    65, 64,  0,   0,   65, 64,  0,   0,   65, 64,  0,   0,   65, 64,  0,   0,   65, 64,  0,   0,   65, 64,  0,   0,
    65, 64,  0,   0,   65, 80,  0,   0,   65, 128, 0,   0,   66, 0,   0,   0,   66, 0,   0,   0,   66, 0,   0,   0,
    66, 0,   0,   0,   66, 0,   0,   0,   66, 0,   0,   0,   66, 0,   0,   0,   66, 0,   0,   0,   66, 0,   0,   0,
    66, 0,   0,   0,   66, 0,   0,   0,   66, 0,   0,   0,   66, 0,   0,   0,   66, 0,   0,   0,   66, 0,   0,   0,
    66, 0,   0,   0,   66, 0,   0,   0,   66, 0,   0,   0,   66, 0,   0,   0,   66, 0,   0,   0,   66, 0,   0,   0,
    65, 128, 0,   0,   65, 128, 0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,
    65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,
    65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,
    65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,
    65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,
    65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,
    65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   64, 224, 0,   0,
    64, 224, 0,   0,   64, 224, 0,   0,   64, 224, 0,   0,   64, 224, 0,   0,   64, 224, 0,   0,   64, 224, 0,   0,
    64, 224, 0,   0,   64, 224, 0,   0,   64, 224, 0,   0,   64, 224, 0,   0,   64, 224, 0,   0,   64, 224, 0,   0,
    64, 208, 0,   0,   64, 192, 0,   0,   64, 176, 0,   0,   64, 160, 0,   0,   64, 153, 153, 154, 64, 128, 0,   0,
    64, 102, 102, 102, 64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154,
    64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154,
    64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154,
    64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154,
    64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154, 65, 0,   0,   0,   65, 0,   0,   0,   64, 160, 0,   0,
    64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,
    64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,
    64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,
    64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,
    64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,
    64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,
    64, 160, 0,   0,   64, 160, 0,   0,   65, 96,  0,   0,   65, 96,  0,   0,   65, 96,  0,   0,   65, 96,  0,   0,
    65, 96,  0,   0,   65, 96,  0,   0,   65, 96,  0,   0,   65, 96,  0,   0,   65, 96,  0,   0,   65, 96,  0,   0,
    65, 96,  0,   0,   65, 96,  0,   0,   65, 96,  0,   0,   65, 80,  0,   0,   65, 64,  0,   0,   65, 48,  0,   0,
    65, 32,  0,   0,   65, 25,  153, 154, 65, 0,   0,   0,   64, 230, 102, 102, 64, 217, 153, 154, 64, 217, 153, 154,
    64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154,
    64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154,
    64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154,
    64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154,
};

s32 gEarthWarriorTailChainJointIndices[4] = {0x17, 0x18, 0x19, 0x1A};

ObjModelChainDesc gEarthWarriorTailChain = {gEarthWarriorTailChainJointIndices, 4};
ObjModelChainDesc* gEarthWarriorTailChainDesc = &gEarthWarriorTailChain;

ObjectDescriptor24WithPadding gDR_EarthWarriorObjDescriptor = {
    {
        0,
        0,
        0,
        OBJECT_DESCRIPTOR_FLAGS_24_SLOTS,
        (ObjectDescriptorCallback)DR_EarthWarrior_initialise,
        (ObjectDescriptorCallback)DR_EarthWarrior_release,
        0,
        (ObjectDescriptorCallback)DR_EarthWarrior_init,
        (ObjectDescriptorCallback)DR_EarthWarrior_update,
        (ObjectDescriptorCallback)DR_EarthWarrior_hitDetect,
        (ObjectDescriptorCallback)DR_EarthWarrior_render,
        (ObjectDescriptorCallback)DR_EarthWarrior_free,
        (ObjectDescriptorCallback)DR_EarthWarrior_getObjectTypeId,
        (ObjectDescriptorExtraSizeCallback)DR_EarthWarrior_getExtraSize,
        (ObjectDescriptorCallback)DR_EarthWarrior_canMount,
        (ObjectDescriptorCallback)DR_EarthWarrior_getMountSide,
        (ObjectDescriptorCallback)DR_EarthWarrior_getRiderPosition,
        (ObjectDescriptorCallback)DR_EarthWarrior_canDismount,
        (ObjectDescriptorCallback)DR_EarthWarrior_getDismountSide,
        (ObjectDescriptorCallback)DR_EarthWarrior_getCameraPosition,
        (ObjectDescriptorCallback)DR_EarthWarrior_getMountState,
        (ObjectDescriptorCallback)DR_EarthWarrior_setMountState,
        (ObjectDescriptorCallback)DR_EarthWarrior_getPlayerAnim,
        (ObjectDescriptorCallback)DR_EarthWarrior_func19,
        (ObjectDescriptorCallback)DR_EarthWarrior_getRacePosition,
        (ObjectDescriptorCallback)DR_EarthWarrior_resetToRomListPosition,
        (ObjectDescriptorCallback)DR_EarthWarrior_handleRiderScale,
        (ObjectDescriptorCallback)DR_EarthWarrior_feed,
    },
    0,
};
