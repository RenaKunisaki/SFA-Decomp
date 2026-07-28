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

#define BADDIE_MOVE_STATUS_SIGNED

#include "main/dll/modgfx_interface.h"
#include "main/dll/partfx_interface.h"
#include "game/objects/object_setup.h"
#include "main/model_engine.h"
#include "main/model_engine_ui_api.h"
#include "sys/objects/lifecycle.h"
#include "main/dll/dll_80136a40.h"
#include "main/debug.h"
#include "main/render_envfx_api.h"
#include "game/objects/object.h"
#include "main/model.h"
#include "main/maketex_api.h"
#include "main/objprint_api.h"
#include "main/objprint_character_api.h"
#include "main/objprint_render_api.h"
#include "main/dll/objfx_api.h"
#include "main/dll/player_api.h"
#include "main/dll/player_spirit_api.h"
#include "main/dll/player_state_api.h"
#include "main/dll/player_motion_api.h"
#include "dlls/objects/229_Shield.h"
#include "dlls/objects/284.h"
#include "dlls/objects/315_WallAnimato.h"
#include "dlls/objects/328_CFGuardian.h"
#include "dlls/objects/437.h"
#include "main/dll/dll_000D_playershadow.h"
#include "main/dll/dll_01B5_lightfoot.h"
#include "main/dll/DB/DBprotection.h"
#include "main/dll/SB/dll_01E8_sbgalleon.h"
#include "main/dll/dll_00E2_staff_api.h"
#include "main/dll/viewfinder.h"
#include "main/sky_api.h"
#include "main/object_render.h"
#include "main/dll/dll_0015_curves.h"
#include "track/intersect_api.h"
#include "main/track_dolphin_api.h"
#include "main/track_bbox_api.h"
#include "main/vecmath_distance_api.h"

#include "sys/objects.h"
#include "main/curve_eval.h"
#include "main/objhits.h"
#include "main/audio/sfx_ids.h"
#include "main/audio/stream_api.h"
#include "main/audio/sfx_keep_alive_api.h"
#include "main/audio/sfx_object_query_api.h"
#include "main/audio/sfx_position_api.h"
#include "main/audio/sfx_stop_object_api.h"
#include "main/audio/music_api.h"
#include "main/gameloop_api.h"
#include "main/dll/dll_0000_gameui_api.h"
#include "main/lightmap_api.h"
#include "main/newshadows_audio_api.h"
#include "main/objfx.h"
#include "main/screen_transition.h"
#include "main/object_transform.h"
#include "ghidra_import.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/objseq_api.h"
#include "main/shader_api.h"
#include "main/pi_dolphin_api.h"
#include "main/dll/player_state.h"
#include "main/dll/baddie_control_interface.h"
#include "main/dll/boneparticleeffect_interface.h"
#include "main/camera_interface.h"
#include "main/camera.h"
#include "main/dll/rom_curve_interface.h"
#include "main/dll/waterfx_interface.h"

#include "main/game_ui_interface.h"
#include "main/mapEventTypes.h"
#include "main/mm.h"
#include "main/objanim.h"
#include "main/objanim_update.h"
#include "main/objtexture.h"
#include "main/objseq.h"
#include "main/dll/player_motion.h"
#include "main/dll/player_objects.h"
#include "main/dll/player_status.h"
#include "main/dll/player_target.h"
#include "main/resource.h"
#include "main/sky_interface.h"
#include "main/vecmath.h"
#include "main/dll/path_control_interface.h"
#include "main/frame_timing.h"
#include "main/byte_flags.h"
#include "main/pad.h"
#include "dolphin/mtx/mtx_legacy.h"
#include "dolphin/gx/GXPixel.h"
#include "dolphin/gx/GXTransform.h"
#include "string.h"
#include "main/dll/dll_002F_carryable.h"
#include "dlls/objects/260_SmallBasket.h"
#define FEAR_TEST_METER_POSITION_INT
#include "main/dll/dll_0000_gameui.h"
#undef FEAR_TEST_METER_POSITION_INT
#include "main/dll/dll_00C9_enemy.h"
#include "main/obj_group.h"
#include "main/obj_link.h"
#include "main/obj_message.h"
#include "main/obj_path.h"
#include "main/obj_query.h"
#include "main/player_eye_anim.h"
#include "main/dll/dll_029B_arwingandrossstuff.h"
#include "main/dll/player.h"
#include "main/dll/tricky_api.h"
#include "main/gamebits.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/audio/music_trigger_ids.h"
#include "main/gamebit_ids.h"
#include "main/player_control_interface.h"
#include "main/sky.h"

#undef BADDIE_MOVE_STATUS_SIGNED

/* GameCube controller button masks (tested against PlayerState.buttons* fields) */
#define PAD_BUTTON_A 0x100

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

int Lightfoot_UpdateProximityInteractionState(int obj, int state)
{
    PlayerState* inner = ((GameObject*)obj)->extra;
    if (((PlayerState*)state)->baddie.targetObj != NULL)
    {
        if (*(u16*)((char*)*(int*)((char*)inner + 0x40c) + 0x22) < inner->proximityRange)
        {
            if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedB != 0 ||
                *(s8*)&((PlayerState*)state)->baddie.moveDone != 0 || ((PlayerState*)state)->baddie.controlMode == 0)
            {
                (*gPlayerInterface)->setState((void*)obj, (void*)state, 4);
            }
        }
        else if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedB != 0 ||
                 *(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            (*gPlayerInterface)->setState((void*)obj, (void*)state, 0);
        }
    }
    return 0;
}

int Lightfoot_UpdateCompletionInteraction(int obj, int state)
{
    int data = *(int*)&((GameObject*)obj)->anim.placementData;
    int inner = *(int*)&((GameObject*)obj)->extra;
    int control = *(int*)((char*)inner + 0x40c);
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedB != 0 ||
        *(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
    {
        if (mainGetBit(((Dll437Placement*)data)->eventGameBit) != 0)
        {
            *(u8*)((char*)inner + 0x404) |= 1;
        }
        if ((*(u8*)((char*)inner + 0x404) & 1) != 0)
        {
            if (((PlayerState*)state)->baddie.controlMode != 3)
            {
                ((Dll437ControlState*)control)->completionCountdown = 4;
                (*gPlayerInterface)->setState((void*)obj, (void*)state, 3);
            }
            if (((Dll437ControlState*)control)->completionCountdown != 0)
            {
                ((Dll437ControlState*)control)->completionCountdown -= 1;
                if (((Dll437ControlState*)control)->completionCountdown == 0)
                {
                    mainSetBits(((Dll437Placement*)data)->completionGameBit, 1);
                    mainSetBits(((Dll437Placement*)data)->activeGameBit, 0);
                    ((GameObject*)obj)->anim.alpha = 0;
                    ((GameObject*)obj)->anim.flags |= OBJANIM_FLAG_HIDDEN;
                    ((Dll437ControlState*)control)->completionTimer = 120.0f;
                    ((Dll437ControlState*)control)->lifeTimer = 100.0f;
                }
            }
        }
        else
        {
            if (((PlayerState*)state)->baddie.controlMode != 1)
            {
                if (mainGetBit(((Dll437Placement*)data)->activeGameBit) != 0)
                {
                    (*gPlayerInterface)->setState((void*)obj, (void*)state, 1);
                }
            }
        }
    }
    return 0;
}

int Lightfoot_UpdateChallengeGateInteraction(int obj, int state)
{
    int inner = *(int*)&((GameObject*)obj)->extra;
    int r4c;
    int sub;
    int v;

    if (((PlayerState*)state)->baddie.targetObj != NULL)
    {
        sub = *(int*)((char*)inner + 0x40c);
        v = (s16) * (u16*)((char*)sub + 0x20);
        if (v < 0)
        {
            v = -v;
        }
        if ((u16)v < 0x1770)
        {
            r4c = *(int*)&((GameObject*)obj)->anim.placementData;
            *(u8*)&((GameObject*)obj)->anim.resetHitboxMode &= ~INTERACT_FLAG_DISABLED;
            switch (*(int*)((char*)r4c + 0x14))
            {
            case 0x46a51:
                if (mainGetBit(GAMEBIT_LV_ChallengeGate1Complete))
                {
                    *(u8*)&((GameObject*)obj)->anim.resetHitboxMode |= INTERACT_FLAG_DISABLED;
                }
                break;
            case 0x46a55:
                if (mainGetBit(GAMEBIT_LV_ChallengeGate2Complete))
                {
                    *(u8*)&((GameObject*)obj)->anim.resetHitboxMode |= INTERACT_FLAG_DISABLED;
                }
                break;
            case 0x49928:
                if (mainGetBit(GAMEBIT_SC_ChallengeGate3Complete))
                {
                    *(u8*)&((GameObject*)obj)->anim.resetHitboxMode |= INTERACT_FLAG_DISABLED;
                }
                break;
            }
            if ((*(u8*)&((GameObject*)obj)->anim.resetHitboxMode & INTERACT_FLAG_ACTIVATED) != 0)
            {
                buttonDisable(0, PAD_BUTTON_A);
                switch (*(int*)((char*)r4c + 0x14))
                {
                case 0x46a51:
                    if (mainGetBit(0xc38) != 0 && mainGetBit(0xc39) != 0 && mainGetBit(0xc3a) != 0)
                    {
                        if (mainGetBit(GAMEBIT_LV_ChallengeGate1Complete) == 0)
                        {
                            mainSetBits(GAMEBIT_LV_ChallengeGate1Complete, 1);
                            (*gObjectTriggerInterface)->runSequence(3, (void*)obj, -1);
                            *(u8*)((char*)sub + 0x2e) = 1;
                            *(u8*)&((GameObject*)obj)->anim.resetHitboxMode |= INTERACT_FLAG_DISABLED;
                        }
                    }
                    else
                    {
                        (*gObjectTriggerInterface)->runSequence(2, (void*)obj, -1);
                    }
                    break;
                case 0x46a55:
                    if (mainGetBit(0xc3b) != 0 && mainGetBit(0xc3c) != 0 && mainGetBit(0xc3d) != 0)
                    {
                        if (mainGetBit(GAMEBIT_LV_ChallengeGate2Complete) == 0)
                        {
                            mainSetBits(GAMEBIT_LV_ChallengeGate2Complete, 1);
                            (*gObjectTriggerInterface)->runSequence(5, (void*)obj, -1);
                            *(u8*)((char*)sub + 0x2e) = 1;
                            *(u8*)&((GameObject*)obj)->anim.resetHitboxMode |= INTERACT_FLAG_DISABLED;
                        }
                    }
                    else
                    {
                        (*gObjectTriggerInterface)->runSequence(4, (void*)obj, -1);
                    }
                    break;
                case 0x49928:
                    if (mainGetBit(0xc3e) != 0 && mainGetBit(0xc3f) != 0 && mainGetBit(0xc40) != 0)
                    {
                        if (mainGetBit(GAMEBIT_SC_ChallengeGate3Complete) == 0)
                        {
                            mainSetBits(GAMEBIT_SC_ChallengeGate3Complete, 1);
                            (*gObjectTriggerInterface)->runSequence(7, (void*)obj, -1);
                            *(u8*)((char*)sub + 0x2e) = 1;
                            *(u8*)&((GameObject*)obj)->anim.resetHitboxMode |= INTERACT_FLAG_DISABLED;
                        }
                    }
                    else
                    {
                        (*gObjectTriggerInterface)->runSequence(6, (void*)obj, -1);
                    }
                    break;
                }
            }
        }
        else
        {
            *(u8*)&((GameObject*)obj)->anim.resetHitboxMode |= INTERACT_FLAG_DISABLED;
        }
        if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedB != 0 ||
            *(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            (*gPlayerInterface)->setState((void*)obj, (void*)state, 0);
        }
    }
    return 0;
}

int Lightfoot_UpdateWanderSteering(GameObject* obj, int state, f32 fv)
{
    PlayerState* inner = obj->extra;
    int sub = *(int*)((char*)inner + 0x40c);
    if (((PlayerState*)sub)->baddie.posX <= 0.0f)
    {
        Sfx_PlayFromObject((int)obj, SFXTRIG_htop_hurry1);
        ((PlayerState*)sub)->baddie.posX = (f32)randomGetRange(0x78, 0xb4);
    }
    ((PlayerState*)state)->baddie.moveSpeed =
        0.04f * (1.0f - (f32)(u16) * (u16*)((char*)sub + 0x22) / (f32)(u16)inner->proximityRange);
    if (((PlayerState*)state)->baddie.moveSpeed < 0.01f)
    {
        ((PlayerState*)state)->baddie.moveSpeed = 0.01f;
    }
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0 ||
        *(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
    {
        u8 r;
        if (*(u8*)((char*)sub + 0x2c) != 0)
        {
            *(u8*)((char*)sub + 0x2c) -= 1;
        }
        else
        {
            r = (*gBaddieControlInterface)
                    ->getClearDirectionMask(obj, (void*)state, 50.0f);
            if ((r & 1) == 0)
            {
                if (r & 4)
                {
                    obj->anim.rotX += 0x7ff8;
                    *(u8*)((char*)sub + 0x2c) = 3;
                }
                else if (r & 2)
                {
                    obj->anim.rotX -= 0x3ffc;
                    *(u8*)((char*)sub + 0x2c) = 3;
                }
                else if (r & 8)
                {
                    obj->anim.rotX += 0x3ffc;
                    *(u8*)((char*)sub + 0x2c) = 3;
                }
            }
        }
        ObjAnim_SetCurrentMove((int)obj, 0x14, 0.0f, 0);
    }
    if (*(u8*)((char*)sub + 0x2c) == 0)
    {
        obj->anim.rotX +=
            (s16)((f32)(s32)((u16) * (u16*)((char*)sub + 0x20) - 0x7fff) * timeDelta / 4.0f);
    }
    (*gPlayerInterface)->updateAnimRootMotion(obj, (void*)state, fv, 1);
    return 0;
}

int Lightfoot_UpdateRandomTurn(int obj, int state, f32 fv)
{
    int inner = *(int*)&((GameObject*)obj)->extra;
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        Sfx_PlayFromObject(obj, *(u16*)((char*)*(int*)((char*)inner + 0x40c) + 0x2a));
        if (randomGetRange(0, 1) != 0)
        {
            ((GameObject*)obj)->anim.rotX += 0x8AA9;
        }
        else
        {
            ((GameObject*)obj)->anim.rotX -= 0x8AA9;
        }
        ObjAnim_SetCurrentMove(obj, 0x23, 0.0f, 0);
    }
    ((PlayerState*)state)->baddie.moveSpeed = 0.017f;
    (*gPlayerInterface)->updateAnimRootMotion((void*)obj, (void*)state, fv, 1);
    return 0;
}

PlayerLightfootAnimTable lbl_80334EE8 = {
    {{10, 11, 12, 0, 11, 10, 10, -1}, {0.01f, 0.01f, 0.005f, 0.003f, 0.01f, 0.01f, 0.01f, -1.0f}},
    {{7, 10, 8, 9, 11, 12, 7, -1}, {0.0115f, 0.0105f, 0.005f, 0.003f, 0.01f, 0.011f, 0.012f, -1.0f}},
    {43, 45, 45, 45, 45, 46, 47, 47, 47, 47, 47, 47, 53, -1},
    {0.02f, 0.021f, 0.022f, 0.023f, 0.024f, 0.015f, 0.08f, 0.07f, 0.06f, 0.05f, 0.04f, 0.03f,
     0.01f, -1.0f},
};

s16 gPlayerMoveTableC[8] = {51, 50, 52, 50, 52, 51, -1, 0};

PlayerLightfootMoveSpeeds gPlayerMoveSpeedTable = {
    {0.013f, 0.005f, 0.01f, 0.005f, 0.013f, 0.01f, -1.0f},
    {0x768, 0x769, 0x76A, 0x76B, 0xA50, 0xA51, 0xA52, 0xA53},
    {0.708f, 0.629f, 0.551f, 0.472f, 0.393f, 0.314f, 0.236f, 0.157f},
};

int Lightfoot_UpdateTargetAnimationCycle(GameObject* obj, int state, f32 fv)
{
    int inner = *(int*)&obj->extra;
    int a4 = *(int*)((char*)inner + 0x40c);
    void* p = ((PlayerState*)state)->baddie.targetObj;
    if (p != NULL)
    {
        characterSetHeadYawToTarget(obj, (GameObject*)p, (CharacterEyeAnimState*)(inner + 0x3ac), 0x19);
    }
    if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0 ||
        *(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        int q = *(int*)&obj->anim.placementData;
        obj->anim.localPosX = *(f32*)((char*)q + 0x8);
        obj->anim.localPosZ = *(f32*)((char*)q + 0x10);
        *(u16*)((char*)a4 + 0x24) += 1;
        if (gPlayerMoveTableC[*(u16*)((char*)a4 + 0x24)] == -1)
        {
            *(u16*)((char*)a4 + 0x24) = 0;
        }
        ObjAnim_SetCurrentMove((int)obj, gPlayerMoveTableC[*(u16*)((char*)a4 + 0x24)], 0.0f, 0);
    }
    ((PlayerState*)state)->baddie.moveSpeed =
        gPlayerMoveSpeedTable.speeds[*(u16*)((char*)a4 + 0x24)];
    (*gPlayerInterface)->updateAnimRootMotion(obj, (void*)state, fv, 1);
    return 0;
}

int Lightfoot_UpdateButtonTimingChallenge(GameObject* obj, int state, f32 fv)
{
    const Dll437Placement* placement;
    EmitCtrlTbl* controls = (EmitCtrlTbl*)&lbl_80334EE8;
    GroundBaddieState* actor = obj->extra;
    Dll437ButtonTimingControlState* challenge = actor->control;
    BaddieState* playerState = (BaddieState*)state;
    GameObject* target = playerState->targetObj;
    if (target != NULL)
    {
        characterSetHeadYawToTarget(obj, target, (CharacterEyeAnimState*)actor->eyeAnimState, 0x19);
    }
    if (obj->userData2 == 0)
    {
        challenge->previousPhase2 = challenge->previousPhase;
        challenge->previousPhase = challenge->phase;
        challenge->phase += (u16)(1200.0f * timeDelta);
    }
    if (challenge->animationIndex < 4)
    {
        int meterPosition =
            (s16)(90.0f * mathSinf(3.1415927f * (f32)challenge->phase / 32768.0f));
        u16 successRange = (int)(90.0f * controls->scales[challenge->difficulty]);
        if (obj->userData2 == 0)
        {
            if ((s16)challenge->phase * (s16)challenge->previousPhase < 0)
            {
                Sfx_PlayFromObject(0, SFXTRIG_lockon3_off);
            }
        }
        setAButtonIcon(6);
        fearTestMeterSetRange(0x60, (u8)successRange, meterPosition);
        if ((getButtonsJustPressed(0) & 0x100) && obj->userData2 == 0)
        {
            int distanceFromCenter = meterPosition < 0 ? -meterPosition : meterPosition;
            if (distanceFromCenter <= successRange)
            {
                Sfx_PlayFromObject(0, SFXTRIG_menuups16k);
                obj->userData2 = 2;
            }
            else
            {
                Sfx_PlayFromObject(0, SFXTRIG_lowoxy_beep);
                obj->userData2 = 3;
            }
            fearTestMeterSetFadeIn(0);
        }
    }
    else
    {
        fearTestMeterSetFadeIn(0);
    }
    if (playerState->moveDone != 0 || playerState->moveJustStartedA != 0)
    {
        if (playerState->moveJustStartedA != 0)
        {
            int index;
            u16* gameBit;
            challenge->difficulty = 0;
            for (index = 0, gameBit = controls->bits; index < 8; gameBit++, index++)
            {
                if (mainGetBit(*gameBit) != 0)
                {
                    challenge->difficulty += 1;
                }
            }
            challenge->phase = (u16)randomGetRange(0, 0xffff);
            challenge->previousPhase = challenge->phase;
            challenge->previousPhase2 = challenge->previousPhase;
            fearTestMeterSetRange(
                0x60, (u8)(int)(96.0f * controls->scales[challenge->difficulty]),
                (int)(90.0f * mathSinf(3.1415927f * (f32)challenge->phase / 32768.0f)));
            fearTestMeterSetFadeIn(1);
            setAButtonIcon(6);
        }
        placement = (const Dll437Placement*)obj->anim.placementData;
        if (playerState->moveJustStartedA != 0)
        {
            challenge->animationIndex = 0;
            obj->anim.localPosX = placement->base.posX;
            obj->anim.localPosZ = placement->base.posZ;
        }
        else
        {
            challenge->animationIndex += 1;
        }
        if (controls->anims[challenge->animationIndex] == -1)
        {
            challenge->animationIndex = 0;
            obj->anim.localPosX = placement->base.posX;
            obj->anim.localPosZ = placement->base.posZ;
            mainSetBits(placement->completionGameBit, 1);
            mainSetBits(placement->activeGameBit, 0);
            return 3;
        }
        ObjAnim_SetCurrentMove((int)obj, controls->anims[challenge->animationIndex], 0.0f, 0);
    }
    playerState->moveSpeed = controls->blends[challenge->animationIndex];
    (*gPlayerInterface)->updateAnimRootMotion(obj, (void*)state, fv, 1);
    return 0;
}

int Lightfoot_UpdateAnimationCycle(GameObject* obj, int state, f32 fv)
{
    int inner = *(int*)&obj->extra;
    void* p = ((PlayerState*)state)->baddie.targetObj;
    int a4;
    s16* moves;
    f32* blends;
    if (p != NULL)
    {
        characterSetHeadYawToTarget(obj, (GameObject*)p, (CharacterEyeAnimState*)(inner + 0x3ac), 0x19);
    }
    a4 = *(int*)((char*)inner + 0x40c);
    moves = *(s16**)((char*)a4 + 0);
    blends = *(f32**)((char*)a4 + 4);
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0 ||
        *(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
    {
        *(u8*)((char*)a4 + 0x2c) = 0;
        *(u16*)((char*)a4 + 0x24) += 1;
        if (moves[*(u16*)((char*)a4 + 0x24)] == -1)
        {
            *(u16*)((char*)a4 + 0x24) = 0;
        }
        if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
        {
            obj->anim.currentMoveProgress = (f32)randomGetRange(0, 0x63) / 100.0f;
            ObjAnim_SetCurrentMove((int)obj, moves[*(u16*)((char*)a4 + 0x24)], obj->anim.currentMoveProgress,
                                   0);
        }
        else
        {
            ObjAnim_SetCurrentMove((int)obj, moves[*(u16*)((char*)a4 + 0x24)], 0.0f, 0);
        }
    }
    ((PlayerState*)state)->baddie.moveSpeed = blends[*(u16*)((char*)a4 + 0x24)];
    (*gPlayerInterface)->updateAnimRootMotion(obj, (void*)state, fv, 0);
    return 0;
}

void Lightfoot_RecordCompletedChallengeTargetHit(GameObject* obj, int inner, int animState)
{
    int idx;

    if (*(u8*)((char*)animState + 0x2e) == 0)
        return;
    if ((*(u16*)((char*)inner + 0x400) & 2) == 0)
        return;

    idx = *(int*)&obj->anim.placementData;
    if (*(u32*)((char*)idx + 0x14) == 0x46A51 && mainGetBit(0xc49) == 0)
    {
        mainSetBits(0xc49, 1);
    }
    else if (*(u32*)((char*)idx + 0x14) == 0x46A55 && mainGetBit(0xc4a) == 0)
    {
        mainSetBits(0xc4a, 1);
    }
    else if (*(u32*)((char*)idx + 0x14) == 0x49928 && mainGetBit(0xc4b) == 0)
    {
        mainSetBits(0xc4b, 1);
    }
    *(u8*)((char*)animState + 0x2e) = 0;
}

/*
 * Mask passed to hitDetectFn_80065e50 / hitDetectFn_800691c0 to pick what a
 * collision query tests. Low byte = behaviour flags; the high bits select the
 * map-surface type (consumed by mapLoadBlocksFn_800685cc).
 */
static void Lightfoot_RearmScuffBurst(GameObject* obj, f32* timer, f32* params)
{
    *timer = *timer + 15.0f;
    params[1] = 35.0f;
    objfx_spawnPulseBurst(obj, 0.2f * obj->anim.rootMotionScale, 3, 3, 0, params);
}

void Lightfoot_ProcessHitResponseFlags(int obj, int inner)
{
    if (*(int*)&((PlayerState*)inner)->baddie.eventFlags & 4)
    {
        *(int*)&((PlayerState*)inner)->baddie.eventFlags &= ~4;
        Sfx_PlayFromObject(obj, SFXTRIG_sc_spotfox02);
    }
    if (*(int*)&((PlayerState*)inner)->baddie.eventFlags & 2)
    {
        *(int*)&((PlayerState*)inner)->baddie.eventFlags &= ~2;
        Sfx_PlayFromObject(obj, SFXTRIG_sc_spotfox02);
    }
    if (*(int*)&((PlayerState*)inner)->baddie.eventFlags & 1)
    {
        *(int*)&((PlayerState*)inner)->baddie.eventFlags &= ~1;
        if (randomGetRange(0, 2) == 0)
        {
            Sfx_PlayFromObject(obj, SFXTRIG_skeep_mumb4);
        }
    }
    if (*(int*)&((PlayerState*)inner)->baddie.eventFlags & 0x80)
    {
        *(int*)&((PlayerState*)inner)->baddie.eventFlags &= ~0x80;
        Sfx_PlayFromObject(obj, SFXTRIG_wp_swdtest322);
    }
    if (*(int*)&((PlayerState*)inner)->baddie.eventFlags & 0x200)
    {
        *(int*)&((PlayerState*)inner)->baddie.eventFlags &= ~0x200;
        Sfx_PlayFromObject(obj, SFXTRIG_sk_trwhin3);
    }
    if (*(int*)&((PlayerState*)inner)->baddie.eventFlags & 0x40)
    {
        *(int*)&((PlayerState*)inner)->baddie.eventFlags &= ~0x40;
        Sfx_PlayFromObject(obj, SFXTRIG_wp_swdtest322_135);
    }
    if (*(int*)&((PlayerState*)inner)->baddie.eventFlags & 0x800)
    {
        *(int*)&((PlayerState*)inner)->baddie.eventFlags &= ~0x800;
        ObjHits_RecordObjectHit(Obj_GetPlayerObject(), (GameObject*)obj, 0x19, 2, 1);
        Sfx_PlayFromObject(obj, SFXTRIG_wp_simp1_c);
        CameraShake_Start(2.5f, 5.0f, 4.0f);
        doRumble(11.0f);
    }
}

void Lightfoot_ResetScriptedPosition(GameObject* obj)
{
    switch (*(int*)((char*)*(int*)&obj->anim.placementData + 0x14))
    {
    case 0x34316:
        obj->anim.worldPosX = -2692.46f;
        obj->anim.worldPosY = -981.0f;
        obj->anim.worldPosZ = 497.2f;
        obj->anim.rotX = 0x2565;
        break;
    case 0x33E3C:
        obj->anim.worldPosX = -2746.88f;
        obj->anim.worldPosY = -997.0f;
        obj->anim.worldPosZ = 407.7f;
        obj->anim.rotX = 0x1c42;
        break;
    case 0x33E34:
        obj->anim.worldPosX = -2789.69f;
        obj->anim.worldPosY = -997.0f;
        obj->anim.worldPosZ = 457.71f;
        obj->anim.rotX = 0x1d00;
        break;
    case 0x45C47:
        obj->anim.worldPosX = -2682.64f;
        obj->anim.worldPosY = -981.0f;
        obj->anim.worldPosZ = 450.29f;
        obj->anim.rotX = 0x32c1;
        break;
    case 0x460B6:
        obj->anim.worldPosX = -2737.49f;
        obj->anim.worldPosY = -981.0f;
        obj->anim.worldPosZ = 529.58f;
        obj->anim.rotX = 0x119f;
        break;
    }
}

void Lightfoot_UpdateAttachedChild(GameObject* obj, int inner)
{
    int animState = *(int*)((char*)inner + 0x40c);
    GameObject* child;
    ObjPlacement* setup;

    if (*(s16*)((char*)animState + 0x26) == *(s16*)((char*)animState + 0x28))
        return;
    if (obj->anim.alpha == 0)
        return;

    child = obj->childObjs[0];
    if (child != NULL)
    {
        ObjLink_DetachChild(obj, child);
        Obj_FreeObject(child);
    }
    if (Obj_IsLoadingLocked())
    {
        if (*(s16*)((char*)animState + 0x28) > 0)
        {
            setup = Obj_AllocObjectSetup(0x20, *(s16*)((char*)animState + 0x28));
            child = Obj_SetupObject(setup, 4, obj->anim.mapEventSlot, -1, obj->anim.parent);
            ObjLink_AttachChild(obj, child, 0);
            *(s16*)((char*)animState + 0x26) = *(s16*)((char*)animState + 0x28);
        }
    }
    else
    {
        *(s16*)((char*)animState + 0x26) = 0;
    }
}

void Lightfoot_UpdatePlayerInteraction(int obj, int inner, int state)
{
    int p = *(int*)((char*)inner + 0x40c);
    int sub = *(int*)&((GameObject*)obj)->anim.placementData;
    int mode;
    int v;

    (*gBaddieControlInterface)
        ->getTargetGeometry((GameObject*)obj, Obj_GetPlayerObject(), 0x10, (u16*)((char*)p + 0x1e),
                            (u16*)((char*)p + 0x20), (u16*)((char*)p + 0x22));
    ((PlayerState*)state)->baddie.targetDistance = (f32)(u32) * (u16*)((int)p + 0x22);
    mode = ((GameObject*)obj)->userData2;
    if (mode == 2)
    {
        (*gObjectTriggerInterface)->runSequence(0, (void*)obj, -1);
        ((GameObject*)obj)->userData2 = 1;
    }
    else if (mode == 3)
    {
        (*gObjectTriggerInterface)->runSequence(1, (void*)obj, -1);
        ((GameObject*)obj)->userData2 = 1;
    }
    else
    {
        characterDoEyeAnims((GameObject*)obj, (void*)(inner + 0x3ac));
        ((PlayerState*)state)->baddie.targetObj = Obj_GetPlayerObject();
        v = *(int*)&((PlayerState*)sub)->baddie.posX;
        if (v >= 0x49942 || v < 0x4993f)
        {
            (*gBaddieControlInterface)
                ->updateGravity((GameObject*)obj, (void*)state, 0.17f, 1);
        }
        ((PlayerState*)inner)->pendingParentObj = *(int*)&((GameObject*)obj)->pendingParentObj;
        *(int*)&((GameObject*)obj)->pendingParentObj = 0;
        (*gPlayerInterface)
            ->update((void*)obj, (void*)state, timeDelta, timeDelta, gDll437StateHandlers,
                     gDll437SubstateHandlers);
        *(int*)&((GameObject*)obj)->pendingParentObj = ((PlayerState*)inner)->pendingParentObj;
        Lightfoot_ProcessHitResponseFlags(obj, inner);
    }
}

int Lightfoot_SeqFn(GameObject* obj, int unused, ObjAnimUpdateState* animUpdate)
{
    int inner = *(int*)&obj->extra;
    int placement = *(int*)&obj->anim.placementData;
    int timerRec;
    int mode;
    u8 i;
    u8 j;
    f32 scale;
    f32 zero;
    f32 fv;
    f32 snd[3];
    f32 arr[6];

    timerRec = *(int*)((char*)inner + 0x40c);
    fv = *(f32*)((char*)timerRec + 0x10);
    if (fv != (zero = 0.0f))
    {
        *(f32*)((char*)timerRec + 0x10) = fv - timeDelta;
        if (*(f32*)((char*)timerRec + 0x10) <= zero)
        {
            Obj_FreeObject((GameObject*)obj);
        }
    }
    for (i = 0; i < animUpdate->eventCount; i++)
    {
        switch (animUpdate->eventIds[i])
        {
        case 1:
            *(u8*)((char*)inner + 0x404) = *(u8*)((char*)inner + 0x404) | 1;
            mainSetBits(*(s16*)((char*)placement + 0x1c), 1);
            arr[3] = 0.0f;
            arr[4] = 35.0f;
            arr[5] = 0.0f;
            j = 0x19;
            scale = 0.8f;
            for (; j != 0; j--)
            {
                objfx_spawnPulseBurst(obj, scale * obj->anim.rootMotionScale, 3, 0, 0, arr);
            }
            break;
        }
    }
    if (*(s16*)((char*)placement + 0x1a) == 0x64c)
    {
        Lightfoot_UpdatePlayerInteraction((int)obj, inner, inner);
        if ((*(u8*)((char*)inner + 0x404) & 1) != 0 && (obj->objectFlags & OBJECT_OBJFLAG_RENDERED) != 0)
        {
            timerRec = *(int*)((char*)inner + 0x40c);
            *(f32*)((char*)timerRec + 0xc) = *(f32*)((char*)timerRec + 0xc) - timeDelta;
            if (*(f32*)((char*)timerRec + 0xc) <= 0.0f)
            {
                mode = 3;
                *(f32*)((char*)timerRec + 0xc) += 15.0f;
            }
            else
            {
                mode = 0;
            }
            snd[0] = 0.0f;
            snd[1] = 35.0f;
            snd[2] = 0.0f;
            Sfx_KeepAliveLoopedObjectSound((int)obj, SFXTRIG_foot_metal_scuff_455);
            objfx_spawnPulseBurst(obj, 0.2f * obj->anim.rootMotionScale, 3, mode, 0, snd);
        }
    }
    *(u16*)((char*)inner + 0x400) = *(u16*)((char*)inner + 0x400) | 2;
    return 0;
}

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
            objRenderModelAndHitVolumes(obj, renderArg2, renderArg3, renderArg4, renderArg5, 1.0f);
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
    if (lifeTimer != (terminalLifeTimer = 0.0f)) {
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
                    Vec_distance(&obj->anim.worldPosX, &((GameObject*)other)->anim.worldPosX) < 25.0f) {
                    mainSetBits(((Dll437State*)inner)->groundBaddie.gameBitA, 1);
                    effectParams[3] = 0.0f;
                    effectParams[4] = 10.0f;
                    effectParams[5] = 0.0f;
                    for (effectCount = 0x14; effectCount != 0; effectCount--) {
                        objfx_spawnDirectionalBurst(obj, 5, 5.0f, 5, 6, 0x64, 10.0f,
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
                    Vec_distance(&obj->anim.worldPosX, &((GameObject*)other)->anim.worldPosX) < 25.0f) {
                    mainSetBits(((Dll437State*)inner)->groundBaddie.gameBitA, 1);
                    effectParams[3] = 0.0f;
                    effectParams[4] = 10.0f;
                    effectParams[5] = 0.0f;
                    for (effectCount = 0x14; effectCount != 0; effectCount--) {
                        objfx_spawnDirectionalBurst(obj, 5, 5.0f, 5, 6, 0x64, 10.0f,
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
            effectParams[3] = 0.0f;
            effectParams[4] = 24.0f;
            effectParams[5] = 0.0f;
            objfx_spawnArcedBurst(obj, 5, 0.75f, 1, 6, 0x32, 25.0f, 25.0f,
                                  48.0f, effectParams, 0);
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
            if (((Dll437ControlState*)controlState)->pulseTimer <= 0.0f) {
                workValue = 3;
                ((Dll437ControlState*)controlState)->pulseTimer += 15.0f;
            } else {
                workValue = 0;
            }
            pulseOffset[0] = 0.0f;
            pulseOffset[1] = 35.0f;
            pulseOffset[2] = 0.0f;
            Sfx_KeepAliveLoopedObjectSound((int)obj, SFXTRIG_foot_metal_scuff_455);
            objfx_spawnPulseBurst(obj, 0.2f * obj->anim.rootMotionScale, 3, workValue, 0, pulseOffset);
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
        ->initGroundBaddie(obj, (u8*)placement, (u8*)inner, 5, 3, 0x108, initFlags, 20.0f);
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
            obj->anim.currentMoveProgress = (f32)(s32)randomGetRange(0, 0x63) / 100.0f;
            break;
        case 0x33E3C:
            ((Dll437ControlState*)control)->moveIds = gDll437MoveIds0;
            ((Dll437ControlState*)control)->moveSpeeds = gDll437MoveSpeeds0;
            ((Dll437ControlState*)control)->weaponDefNo = DLL437_WEAPON_DEF_1;
            *(u8*)&obj->anim.resetHitboxMode = (u8)(*(u8*)&obj->anim.resetHitboxMode | INTERACT_FLAG_DISABLED);
            obj->anim.currentMoveProgress = (f32)(s32)randomGetRange(0, 0x63) / 100.0f;
            break;
        case 0x33E34:
            ((Dll437ControlState*)control)->moveIds = gDll437MoveIds1;
            ((Dll437ControlState*)control)->moveSpeeds = gDll437MoveSpeeds1;
            ((Dll437ControlState*)control)->weaponDefNo = DLL437_WEAPON_DEF_1;
            *(u8*)&obj->anim.resetHitboxMode = (u8)(*(u8*)&obj->anim.resetHitboxMode | INTERACT_FLAG_DISABLED);
            obj->anim.currentMoveProgress = (f32)(s32)randomGetRange(0, 0x63) / 100.0f;
            break;
        case 0x45C47:
            ((Dll437ControlState*)control)->moveIds = gDll437MoveIds2;
            ((Dll437ControlState*)control)->moveSpeeds = gDll437MoveSpeeds2;
            ObjHits_DisableObject(obj);
            ((Dll437ControlState*)control)->weaponDefNo = DLL437_WEAPON_DEF_2;
            *(u8*)&obj->anim.resetHitboxMode = (u8)(*(u8*)&obj->anim.resetHitboxMode | INTERACT_FLAG_DISABLED);
            obj->anim.currentMoveProgress = (f32)(s32)randomGetRange(0, 0x63) / 100.0f;
            break;
        case 0x460B6:
            ((Dll437ControlState*)control)->moveIds = gDll437MoveIds4;
            ((Dll437ControlState*)control)->moveSpeeds = gDll437MoveSpeeds4;
            ObjHits_DisableObject(obj);
            *(u8*)&obj->anim.resetHitboxMode = (u8)(*(u8*)&obj->anim.resetHitboxMode | INTERACT_FLAG_DISABLED);
            obj->anim.currentMoveProgress = (f32)(s32)randomGetRange(0, 0x63) / 100.0f;
            break;
        case 0x3433F:
            ((Dll437ControlState*)control)->moveIds = (s16*)(playerAnimTableBase + 0x30);
            ((Dll437ControlState*)control)->moveSpeeds = (f32*)(playerAnimTableBase + 0x40);
            *(u8*)&obj->anim.resetHitboxMode = (u8)(*(u8*)&obj->anim.resetHitboxMode | INTERACT_FLAG_DISABLED);
            obj->anim.currentMoveProgress = (f32)(s32)randomGetRange(0, 0x63) / 100.0f;
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
            obj->anim.currentMoveProgress = (f32)(s32)randomGetRange(0, 0x63) / 100.0f;
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
    ObjAnim_SetMoveProgress((ObjAnimComponent*)obj, (f32)(s32)randomGetRange(0, 0x63) / 100.0f);
    ((Dll437ControlState*)control)->movementSfxId = (u16)(randomGetRange(0, 1) != 0 ? 0x133 : 0x134);
    ((Dll437ControlState*)control)->pulseTimer = 15.0f;
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

Dll437StateHandler gDll437StateHandlers[DLL437_STATE_HANDLER_COUNT];
Dll437SubstateHandler gDll437SubstateHandlers[DLL437_SUBSTATE_HANDLER_COUNT];
