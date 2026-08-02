/*
 * DLL 0x1B5 (slot 437) controls several LightFoot Village NPC object definitions. The
 * retail mappings include SC_chieflig, SC_lightfoo, SC_babyligh, SC_blTarget,
 * and SC_muscleli, so no single object basename represents the complete TU.
 *
 * The shared actor controller handles movement and challenge interactions.
 * Baby LightFoot actors are revealed after reaching their target placements;
 * challenge-gate actors stay interactive until their associated village
 * objectives are complete.
 */

#include "main/render_envfx_api.h"
#include "main/objprint_character_api.h"
#include "dlls/objects/229_Shield.h"
#include "dlls/objects/284.h"
#include "dlls/objects/315_WallAnimato.h"
#include "dlls/objects/328_CFGuardian.h"
#include "dlls/objects/437.h"
#include "main/dll/dll_000D_playershadow.h"
#include "main/dll/dll_01B5_lightfoot.h"
#include "main/sky_api.h"
#include "main/object_render.h"
#include "main/dll/dll_0015_curves.h"
#include "track/intersect_api.h"
#include "main/track_dolphin_api.h"
#include "main/vecmath_distance_api.h"

#include "sys/objects.h"
#include "main/curve_eval.h"
#include "main/objhits.h"
#include "main/audio/sfx_keep_alive_api.h"
#include "main/audio/sfx_object_query_api.h"
#include "main/audio/sfx_position_api.h"
#include "main/audio/sfx_stop_object_api.h"
#include "main/audio/music_api.h"
#include "main/gameloop_api.h"
#include "main/dll/dll_0000_gameui_api.h"
#include "main/lightmap_api.h"
#include "main/objfx.h"
#include "main/screen_transition.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/shader_api.h"
#include "main/pi_dolphin_api.h"
#include "main/dll/player_state.h"
#include "main/dll/baddie_control_interface.h"
#include "main/dll/boneparticleeffect_interface.h"
#include "main/camera_interface.h"
#include "main/camera.h"
#include "main/dll/rom_curve_interface.h"
#include "main/dll/waterfx_interface.h"
#include "dolphin/pad.h"

#include "main/game_ui_interface.h"
#include "main/mapEventTypes.h"
#include "main/mm.h"
#include "main/objanim.h"
#include "main/objseq.h"
#include "main/resource.h"
#include "main/sky_interface.h"
#include "main/vecmath.h"
#include "main/dll/path_control_interface.h"
#include "main/frame_timing.h"
#include "main/pad.h"
#include "dolphin/gx/GXPixel.h"
#include "dolphin/gx/GXTransform.h"
#include "dlls/objects/260_SmallBasket.h"
#define FEAR_TEST_METER_POSITION_INT
#include "main/dll/dll_0000_gameui.h"
#undef FEAR_TEST_METER_POSITION_INT
#include "main/dll/dll_00C9_enemy.h"
#include "main/objtype.h"
#include "main/obj_link.h"
#include "main/dll/dll_029B_arwingandrossstuff.h"
#include "main/dll/player_data.h"
#include "main/dll/tricky_api.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/audio/music_trigger_ids.h"
#include "main/gamebit_ids.h"
#include "main/player_control_interface.h"
#include "main/sky.h"

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
#include "main/objtype.h"
#include "main/obj_link.h"
#include "main/objhits.h"
#include "main/objfx.h"
#include "main/vecmath.h"
#include "sys/objects.h"
#include "sys/objects/lifecycle.h"

#define DLL1B5_WEAPON_DEF_1 0x6F1
#define DLL1B5_WEAPON_DEF_2 0x6F2
#define DLL1B5_OBJECT_GROUP 3

typedef struct Dll1B5ButtonTimingTables {
    u8 pad00[0x60];
    s16 anims[14];
    f32 blends[25];
    u16 gameBits[8];
    f32 meterScales[16];
} Dll1B5ButtonTimingTables;

STATIC_ASSERT(sizeof(Dll1B5ButtonTimingTables) == 0x130);

int Lightfoot_UpdateProximityInteractionState(int obj, int state)
{
    GroundBaddieState* inner = ((GameObject*)obj)->extra;
    if (((BaddieState*)state)->targetObj != NULL)
    {
        if (((Dll1B5ControlState*)inner->control)->targetDistance < inner->aggroRange)
        {
            if (((BaddieState*)state)->moveJustStartedB != 0 ||
                ((BaddieState*)state)->moveDone != 0 || ((BaddieState*)state)->controlMode == 0)
            {
                (*gPlayerInterface)->setState((void*)obj, (void*)state, 4);
            }
        }
        else if (((BaddieState*)state)->moveJustStartedB != 0 ||
                 ((BaddieState*)state)->moveDone != 0)
        {
            (*gPlayerInterface)->setState((void*)obj, (void*)state, 0);
        }
    }
    return 0;
}

int Lightfoot_UpdateCompletionInteraction(int obj, int state)
{
    Dll1B5Placement* data = (Dll1B5Placement*)((GameObject*)obj)->anim.placementData;
    GroundBaddieState* inner = ((GameObject*)obj)->extra;
    Dll1B5ControlState* control = inner->control;
    if (((BaddieState*)state)->moveJustStartedB != 0 ||
        ((BaddieState*)state)->moveDone != 0)
    {
        if (mainGetBit(data->eventGameBit) != 0)
        {
            inner->configFlags |= 1;
        }
        if ((inner->configFlags & 1) != 0)
        {
            if (((BaddieState*)state)->controlMode != 3)
            {
                ((Dll1B5ControlState*)control)->completionCountdown = 4;
                (*gPlayerInterface)->setState((void*)obj, (void*)state, 3);
            }
            if (((Dll1B5ControlState*)control)->completionCountdown != 0)
            {
                ((Dll1B5ControlState*)control)->completionCountdown -= 1;
                if (((Dll1B5ControlState*)control)->completionCountdown == 0)
                {
                    mainSetBits(data->completionGameBit, 1);
                    mainSetBits(data->activeGameBit, 0);
                    ((GameObject*)obj)->anim.alpha = 0;
                    ((GameObject*)obj)->anim.flags |= OBJANIM_FLAG_HIDDEN;
                    ((Dll1B5ControlState*)control)->completionTimer = 120.0f;
                    ((Dll1B5ControlState*)control)->lifeTimer = 100.0f;
                }
            }
        }
        else
        {
            if (((BaddieState*)state)->controlMode != 1)
            {
                if (mainGetBit(data->activeGameBit) != 0)
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
    GroundBaddieState* inner = ((GameObject*)obj)->extra;
    Dll1B5Placement* r4c;
    Dll1B5ControlState* sub;
    int v;

    if (((BaddieState*)state)->targetObj != NULL)
    {
        sub = inner->control;
        v = (s16)sub->targetYawDelta;
        if (v < 0)
        {
            v = -v;
        }
        if ((u16)v < 0x1770)
        {
            r4c = (Dll1B5Placement*)((GameObject*)obj)->anim.placementData;
            ((GameObject*)obj)->anim.resetHitboxFlags &= ~INTERACT_FLAG_DISABLED;
            switch (r4c->base.ident)
            {
            case 0x46a51:
                if (mainGetBit(GAMEBIT_LV_ChallengeGate1Complete))
                {
                    ((GameObject*)obj)->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
                }
                break;
            case 0x46a55:
                if (mainGetBit(GAMEBIT_LV_ChallengeGate2Complete))
                {
                    ((GameObject*)obj)->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
                }
                break;
            case 0x49928:
                if (mainGetBit(GAMEBIT_SC_ChallengeGate3Complete))
                {
                    ((GameObject*)obj)->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
                }
                break;
            }
            if ((((GameObject*)obj)->anim.resetHitboxFlags & INTERACT_FLAG_ACTIVATED) != 0)
            {
                buttonDisable(0, PAD_BUTTON_A);
                switch (r4c->base.ident)
                {
                case 0x46a51:
                    if (mainGetBit(0xc38) != 0 && mainGetBit(0xc39) != 0 && mainGetBit(0xc3a) != 0)
                    {
                        if (mainGetBit(GAMEBIT_LV_ChallengeGate1Complete) == 0)
                        {
                            mainSetBits(GAMEBIT_LV_ChallengeGate1Complete, 1);
                            (*gObjectTriggerInterface)->runSequence(3, (void*)obj, -1);
                            sub->challengeCompletePending = 1;
                            ((GameObject*)obj)->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
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
                            sub->challengeCompletePending = 1;
                            ((GameObject*)obj)->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
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
                            sub->challengeCompletePending = 1;
                            ((GameObject*)obj)->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
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
            ((GameObject*)obj)->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
        }
        if (((BaddieState*)state)->moveJustStartedB != 0 ||
            ((BaddieState*)state)->moveDone != 0)
        {
            (*gPlayerInterface)->setState((void*)obj, (void*)state, 0);
        }
    }
    return 0;
}

int Lightfoot_UpdateWanderSteering(GameObject* obj, int state, f32 fv)
{
    GroundBaddieState* inner = obj->extra;
    Dll1B5ControlState* sub = inner->control;
    if (sub->wanderTimer <= 0.0f)
    {
        Sfx_PlayFromObject(obj, SFXTRIG_htop_hurry1);
        sub->wanderTimer = (f32)randomGetRange(0x78, 0xb4);
    }
    ((BaddieState*)state)->moveSpeed =
        0.04f * (1.0f - (f32)sub->targetDistance / (f32)inner->aggroRange);
    if (((BaddieState*)state)->moveSpeed < 0.01f)
    {
        ((BaddieState*)state)->moveSpeed = 0.01f;
    }
    if (((BaddieState*)state)->moveJustStartedA != 0 ||
        ((BaddieState*)state)->moveDone != 0)
    {
        u8 r;
        if (sub->completionCountdown != 0)
        {
            sub->completionCountdown -= 1;
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
                    sub->completionCountdown = 3;
                }
                else if (r & 2)
                {
                    obj->anim.rotX -= 0x3ffc;
                    sub->completionCountdown = 3;
                }
                else if (r & 8)
                {
                    obj->anim.rotX += 0x3ffc;
                    sub->completionCountdown = 3;
                }
            }
        }
        ObjAnim_SetCurrentMove((int)obj, 0x14, 0.0f, 0);
    }
    if (sub->completionCountdown == 0)
    {
        obj->anim.rotX +=
            (s16)((f32)(s32)(sub->targetYawDelta - 0x7fff) * timeDelta / 4.0f);
    }
    (*gPlayerInterface)->updateAnimRootMotion(obj, (void*)state, fv, 1);
    return 0;
}

int Lightfoot_UpdateRandomTurn(int obj, int state, f32 fv)
{
    GroundBaddieState* inner = ((GameObject*)obj)->extra;
    if (((BaddieState*)state)->moveJustStartedA != 0)
    {
        Sfx_PlayFromObject((GameObject*)(u32)obj, ((Dll1B5ControlState*)inner->control)->movementSfxId);
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
    ((BaddieState*)state)->moveSpeed = 0.017f;
    (*gPlayerInterface)->updateAnimRootMotion((void*)obj, (void*)state, fv, 1);
    return 0;
}

PlayerLightfootAnimTable gPlayerLightfootAnimTable = {
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
    GroundBaddieState* inner = obj->extra;
    Dll1B5ControlState* a4 = inner->control;
    void* p = ((BaddieState*)state)->targetObj;
    if (p != NULL)
    {
        characterSetHeadYawToTarget(obj, (GameObject*)p, &inner->eyeAnimState, 0x19);
    }
    if (((BaddieState*)state)->moveDone != 0 ||
        ((BaddieState*)state)->moveJustStartedA != 0)
    {
        ObjPlacement* q = (ObjPlacement*)obj->anim.placementData;
        obj->anim.localPosX = q->posX;
        obj->anim.localPosZ = q->posZ;
        a4->moveIndex += 1;
        if (gPlayerMoveTableC[a4->moveIndex] == -1)
        {
            a4->moveIndex = 0;
        }
        ObjAnim_SetCurrentMove((int)obj, gPlayerMoveTableC[a4->moveIndex], 0.0f, 0);
    }
    ((BaddieState*)state)->moveSpeed =
        gPlayerMoveSpeedTable.speeds[a4->moveIndex];
    (*gPlayerInterface)->updateAnimRootMotion(obj, (void*)state, fv, 1);
    return 0;
}

int Lightfoot_UpdateButtonTimingChallenge(GameObject* obj, int state, f32 fv)
{
    const Dll1B5Placement* placement;
    Dll1B5ButtonTimingTables* controls = (Dll1B5ButtonTimingTables*)&gPlayerLightfootAnimTable;
    GroundBaddieState* actor = obj->extra;
    Dll1B5ButtonTimingControlState* challenge = actor->control;
    BaddieState* playerState = (BaddieState*)state;
    GameObject* target = playerState->targetObj;
    if (target != NULL)
    {
        characterSetHeadYawToTarget(obj, target, &actor->eyeAnimState, 0x19);
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
            (s16)(90.0f * mathSinf(3.1415927f * challenge->phase / 32768.0f));
        u16 successRange = (int)(90.0f * controls->meterScales[challenge->difficulty]);
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
            for (index = 0, gameBit = controls->gameBits; index < 8; gameBit++, index++)
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
                0x60, (u8)(int)(96.0f * controls->meterScales[challenge->difficulty]),
                (int)(90.0f * mathSinf(3.1415927f * challenge->phase / 32768.0f)));
            fearTestMeterSetFadeIn(1);
            setAButtonIcon(6);
        }
        placement = (const Dll1B5Placement*)obj->anim.placementData;
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
    GroundBaddieState* inner = obj->extra;
    void* p = ((BaddieState*)state)->targetObj;
    Dll1B5ControlState* a4;
    const s16* moves;
    const f32* blends;
    if (p != NULL)
    {
        characterSetHeadYawToTarget(obj, (GameObject*)p, &inner->eyeAnimState, 0x19);
    }
    a4 = inner->control;
    moves = a4->moveIds;
    blends = a4->moveSpeeds;
    if (((BaddieState*)state)->moveJustStartedA != 0 ||
        ((BaddieState*)state)->moveDone != 0)
    {
        a4->completionCountdown = 0;
        a4->moveIndex += 1;
        if (moves[a4->moveIndex] == -1)
        {
            a4->moveIndex = 0;
        }
        if (((BaddieState*)state)->moveJustStartedA != 0)
        {
            obj->anim.currentMoveProgress = (f32)randomGetRange(0, 0x63) / 100.0f;
            ObjAnim_SetCurrentMove((int)obj, moves[a4->moveIndex], obj->anim.currentMoveProgress,
                                   0);
        }
        else
        {
            ObjAnim_SetCurrentMove((int)obj, moves[a4->moveIndex], 0.0f, 0);
        }
    }
    ((BaddieState*)state)->moveSpeed = blends[a4->moveIndex];
    (*gPlayerInterface)->updateAnimRootMotion(obj, (void*)state, fv, 0);
    return 0;
}

void Lightfoot_RecordCompletedChallengeTargetHit(GameObject* obj, GroundBaddieState* inner, Dll1B5ControlState* animState)
{
    ObjPlacement* idx;

    if (animState->challengeCompletePending == 0)
        return;
    if ((inner->flags400 & 2) == 0)
        return;

    idx = (ObjPlacement*)obj->anim.placementData;
    if (idx->ident == 0x46A51 && mainGetBit(0xc49) == 0)
    {
        mainSetBits(0xc49, 1);
    }
    else if (idx->ident == 0x46A55 && mainGetBit(0xc4a) == 0)
    {
        mainSetBits(0xc4a, 1);
    }
    else if (idx->ident == 0x49928 && mainGetBit(0xc4b) == 0)
    {
        mainSetBits(0xc4b, 1);
    }
    animState->challengeCompletePending = 0;
}

/*
 * Mask passed to trackGetHeight / trackIntersectBroadphase to pick what a
 * collision query tests. Low byte = behaviour flags; the high bits select the
 * map-surface type (consumed by trackBuildBlockTriangles).
 */
void Lightfoot_ProcessHitResponseFlags(int obj, BaddieState* inner)
{
    if (inner->eventFlags & 4)
    {
        inner->eventFlags &= ~4;
        Sfx_PlayFromObject((GameObject*)(u32)obj, SFXTRIG_sc_spotfox02);
    }
    if (inner->eventFlags & 2)
    {
        inner->eventFlags &= ~2;
        Sfx_PlayFromObject((GameObject*)(u32)obj, SFXTRIG_sc_spotfox02);
    }
    if (inner->eventFlags & 1)
    {
        inner->eventFlags &= ~1;
        if (randomGetRange(0, 2) == 0)
        {
            Sfx_PlayFromObject((GameObject*)(u32)obj, SFXTRIG_skeep_mumb4);
        }
    }
    if (inner->eventFlags & 0x80)
    {
        inner->eventFlags &= ~0x80;
        Sfx_PlayFromObject((GameObject*)(u32)obj, SFXTRIG_wp_swdtest322);
    }
    if (inner->eventFlags & 0x200)
    {
        inner->eventFlags &= ~0x200;
        Sfx_PlayFromObject((GameObject*)(u32)obj, SFXTRIG_sk_trwhin3);
    }
    if (inner->eventFlags & 0x40)
    {
        inner->eventFlags &= ~0x40;
        Sfx_PlayFromObject((GameObject*)(u32)obj, SFXTRIG_wp_swdtest322_135);
    }
    if (inner->eventFlags & 0x800)
    {
        inner->eventFlags &= ~0x800;
        ObjHits_RecordObjectHit(Obj_GetPlayerObject(), (GameObject*)obj, 0x19, 2, 1);
        Sfx_PlayFromObject((GameObject*)(u32)obj, SFXTRIG_wp_simp1_c);
        CameraShake_StartDampened(2.5f, 5.0f, 4.0f);
        doRumble(11.0f);
    }
}

void Lightfoot_ResetScriptedPosition(GameObject* obj)
{
    switch (obj->anim.placement->ident)
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

void Lightfoot_UpdateAttachedChild(GameObject* obj, GroundBaddieState* inner)
{
    Dll1B5ControlState* animState = inner->control;
    GameObject* child;
    ObjPlacement* setup;

    if (animState->weaponDefNoSentinel == animState->weaponDefNo)
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
        if (animState->weaponDefNo > 0)
        {
            setup = Obj_AllocObjectSetup(0x20, animState->weaponDefNo);
            child = objSetupObject(setup, 4, obj->anim.mapEventSlot, -1, obj->anim.parent);
            ObjLink_AttachChild(obj, child, 0);
            animState->weaponDefNoSentinel = animState->weaponDefNo;
        }
    }
    else
    {
        animState->weaponDefNoSentinel = 0;
    }
}

void Lightfoot_UpdatePlayerInteraction(int obj, GroundBaddieState* inner, int state)
{
    Dll1B5ControlState* p = inner->control;
    ObjPlacement* sub = (ObjPlacement*)((GameObject*)obj)->anim.placementData;
    int mode;
    int v;

    (*gBaddieControlInterface)
        ->getTargetGeometry((GameObject*)obj, Obj_GetPlayerObject(), 0x10, &p->targetSector,
                            &p->targetYawDelta, &p->targetDistance);
    ((BaddieState*)state)->targetDistance = (f32)(u32)p->targetDistance;
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
        characterDoEyeAnims((GameObject*)obj, &inner->eyeAnimState);
        ((BaddieState*)state)->targetObj = Obj_GetPlayerObject();
        v = sub->ident;
        if (v >= 0x49942 || v < 0x4993f)
        {
            (*gBaddieControlInterface)
                ->updateGravity((GameObject*)obj, (void*)state, 0.17f, 1);
        }
        inner->savedPendingParentObj = ((GameObject*)obj)->pendingParentObj;
        ((GameObject*)obj)->pendingParentObj = 0;
        (*gPlayerInterface)
            ->update((void*)obj, (void*)state, timeDelta, timeDelta, gDll1B5StateHandlers,
                     gDll1B5SubstateHandlers);
        ((GameObject*)obj)->pendingParentObj = inner->savedPendingParentObj;
        Lightfoot_ProcessHitResponseFlags(obj, &inner->baddie);
    }
}

int Lightfoot_SeqFn(GameObject* obj, int unused, ObjSeqState* animUpdate)
{
    GroundBaddieState* inner = obj->extra;
    Dll1B5Placement* placement = (Dll1B5Placement*)obj->anim.placementData;
    Dll1B5ControlState* timerRec;
    int mode;
    u8 i;
    u8 j;
    f32 scale;
    f32 zero;
    f32 fv;
    f32 snd[3];
    f32 arr[6];

    timerRec = inner->control;
    fv = timerRec->lifeTimer;
    if (fv != (zero = 0.0f))
    {
        timerRec->lifeTimer = fv - timeDelta;
        if (timerRec->lifeTimer <= zero)
        {
            Obj_FreeObject((GameObject*)obj);
        }
    }
    for (i = 0; i < animUpdate->eventCount; i++)
    {
        switch (animUpdate->eventIds[i])
        {
        case 1:
            inner->configFlags = inner->configFlags | 1;
            mainSetBits(placement->eventGameBit, 1);
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
    if (placement->behaviorId == DLL1B5_COMPLETION_GAMEBIT_SC_TOTEM_BOND)
    {
        Lightfoot_UpdatePlayerInteraction((int)obj, inner, (int)inner);
        if ((inner->configFlags & 1) != 0 && (obj->objectFlags & OBJECT_OBJFLAG_RENDERED) != 0)
        {
            timerRec = inner->control;
            timerRec->pulseTimer = timerRec->pulseTimer - timeDelta;
            if (timerRec->pulseTimer <= 0.0f)
            {
                mode = 3;
                timerRec->pulseTimer += 15.0f;
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
    inner->flags400 = inner->flags400 | 2;
    return 0;
}

s16 gDll1B5MoveIds0[2] = {0x33, -1};
f32 gDll1B5MoveSpeeds0[2] = {0.0009f, -1.0f};
s16 gDll1B5MoveIds1[2] = {0x33, -1};
f32 gDll1B5MoveSpeeds1[2] = {0.001f, -1.0f};
s16 gDll1B5MoveIds2[2] = {0x36, -1};
f32 gDll1B5MoveSpeeds2[2] = {0.003f, -1.0f};
s16 gDll1B5MoveIds3[2] = {0x128, -1};
f32 gDll1B5MoveSpeeds3[2] = {0.01f, -1.0f};
s16 gDll1B5MoveIds4[2] = {1, -1};
f32 gDll1B5MoveSpeeds4[2] = {0.01f, -1.0f};

int dll437_getExtraSize(void) {
    return sizeof(Dll1B5State);
}

int dll437_getObjectTypeId(void) {
    return 0x14B;
}

void dll437_free(GameObject* obj, int preserveChildren) {
    void* child;
    int inner = *(int*)&obj->extra;
    int count;
    int i;

    objFreeObjectType((u32)obj, DLL1B5_OBJECT_GROUP);
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
    Dll1B5State* inner = obj->extra;
    int workValue = obj->anim.placementDataAddress;
    Dll1B5ControlState* control = inner->groundBaddie.control;
    f32 pulseOffset[3];
    f32 effectParams[6];
    u8 effectCount;
    f32 terminalLifeTimer;
    f32 lifeTimer;

    lifeTimer = control->lifeTimer;
    if (lifeTimer != (terminalLifeTimer = 0.0f)) {
        control->lifeTimer = lifeTimer - timeDelta;
        if (control->lifeTimer <= terminalLifeTimer) {
            Obj_FreeObject(obj);
        }
    }

    if (obj->anim.romDefNo == DLL1B5_SEQUENCE_ID_SC_BABY_LIGHTFOOT && inner->groundBaddie.gameBitA != -1) {
        switch (((Dll1B5Placement*)workValue)->base.ident) {
        case 0x4993F:
        case 0x49940:
        case 0x49941:
            if (mainGetBit(0xC44)) {
                obj->userData1 = mainGetBit(inner->groundBaddie.gameBitA);
            } else {
                obj->userData1 = 1;
            }
            break;
        case 0x499AC:
        case 0x499AE:
        case 0x499AF:
            if (mainGetBit(0xC42) && mainGetBit(inner->groundBaddie.gameBitA) == 0) {
                void* other = ObjList_FindObjectById(0x499B5);

                if (other != NULL &&
                    Vec_distance(&obj->anim.worldPosX, &((GameObject*)other)->anim.worldPosX) < 25.0f) {
                    mainSetBits(inner->groundBaddie.gameBitA, 1);
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
                obj->userData1 = mainGetBit(inner->groundBaddie.gameBitA);
            } else {
                obj->userData1 = 1;
            }
            break;
        case 0x499B0:
        case 0x499B1:
        case 0x499B2:
            if (mainGetBit(0xC46) && mainGetBit(inner->groundBaddie.gameBitA) == 0) {
                void* other = ObjList_FindObjectById(0x499B6);

                if (other != NULL &&
                    Vec_distance(&obj->anim.worldPosX, &((GameObject*)other)->anim.worldPosX) < 25.0f) {
                    mainSetBits(inner->groundBaddie.gameBitA, 1);
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
                obj->userData1 = mainGetBit(inner->groundBaddie.gameBitA);
            } else {
                obj->userData1 = 1;
            }
            break;
        default:
            obj->userData1 = mainGetBit(inner->groundBaddie.gameBitA) == 0;
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
        if (((((Dll1B5Placement*)workValue)->base.ident == 0x499B5 && mainGetBit(0xC42) &&
              (mainGetBit(0xC3B) == 0 || mainGetBit(0xC3C) == 0 || mainGetBit(0xC3D) == 0)) ||
             (((Dll1B5Placement*)workValue)->base.ident == 0x499B6 && mainGetBit(0xC46) &&
              (mainGetBit(0xC3E) == 0 || mainGetBit(0xC3F) == 0 || mainGetBit(0xC40) == 0)))) {
            effectParams[3] = 0.0f;
            effectParams[4] = 24.0f;
            effectParams[5] = 0.0f;
            objfx_spawnArcedBurst(obj, 5, 0.75f, 1, 6, 0x32, 25.0f, 25.0f,
                                  48.0f, effectParams, 0);
        }
    } else {
        Lightfoot_UpdateAttachedChild(obj, &inner->groundBaddie);
        if (inner->groundBaddie.flags400 & 0x2) {
            Lightfoot_RecordCompletedChallengeTargetHit(obj, &inner->groundBaddie, control);
            Lightfoot_ResetScriptedPosition(obj);
            obj->userData2 = 0;
            inner->groundBaddie.flags400 &= ~0x2;
        }
        Lightfoot_UpdatePlayerInteraction((int)obj, &inner->groundBaddie, (int)inner);
        if ((inner->groundBaddie.configFlags & 1) && (obj->objectFlags & OBJECT_OBJFLAG_RENDERED)) {
            Dll1B5ControlState* controlState = inner->groundBaddie.control;

            controlState->pulseTimer -= timeDelta;
            if (controlState->pulseTimer <= 0.0f) {
                workValue = 3;
                controlState->pulseTimer += 15.0f;
            } else {
                workValue = 0;
            }
            pulseOffset[0] = 0.0f;
            pulseOffset[1] = 35.0f;
            pulseOffset[2] = 0.0f;
            Sfx_KeepAliveLoopedObjectSound((int)obj, SFXTRIG_foot_metal_scuff_455);
            objfx_spawnPulseBurst(obj, 0.2f * obj->anim.rootMotionScale, 3, workValue, 0, pulseOffset);
        }
        control->wanderTimer -= timeDelta;
    }
}

void dll437_init(GameObject* obj, const Dll1B5Placement* placement, int isReload) {
    PlayerLightfootAnimTable* playerAnimTableBase = &gPlayerLightfootAnimTable;
    int inner = *(int*)&obj->extra;
    const Dll1B5Placement* placementData = placement;
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
    control = (int)((Dll1B5State*)inner)->groundBaddie.control;
    ((Dll1B5ControlState*)control)->weaponDefNoSentinel = -1;
    ((Dll1B5ControlState*)control)->weaponDefNo = ((Dll1B5ControlState*)control)->weaponDefNoSentinel;
    obj->objectFlags = (u16)(obj->objectFlags | (placement->objectFlags & 0x7));
    if (placement->completionGameBit == DLL1B5_COMPLETION_GAMEBIT_SC_TOTEM_BOND) {
        ((GroundBaddieState*)inner)->baddie.controlMode = 2;
        ((GroundBaddieState*)inner)->baddie.substate = 1;
        ObjHits_DisableObject(obj);
        ((Dll1B5ControlState*)control)->moveIndex = randomGetRange(0, 3);
        ((Dll1B5ControlState*)control)->weaponDefNo = DLL1B5_WEAPON_DEF_1;
        ((Dll1B5ControlState*)control)->moveIds = gDll1B5MoveIds0;
        ((Dll1B5ControlState*)control)->moveSpeeds = gDll1B5MoveSpeeds0;
        obj->anim.resetHitboxFlags = obj->anim.resetHitboxFlags | INTERACT_FLAG_DISABLED;
        obj->userData2 = 0;
    } else {
        switch (placementData->base.ident) {
        case 0x34316:
            ((Dll1B5ControlState*)control)->moveIds = gDll1B5MoveIds3;
            ((Dll1B5ControlState*)control)->moveSpeeds = gDll1B5MoveSpeeds3;
            ObjHits_DisableObject(obj);
            obj->anim.resetHitboxFlags = obj->anim.resetHitboxFlags | INTERACT_FLAG_DISABLED;
            obj->anim.currentMoveProgress = (f32)(s32)randomGetRange(0, 0x63) / 100.0f;
            break;
        case 0x33E3C:
            ((Dll1B5ControlState*)control)->moveIds = gDll1B5MoveIds0;
            ((Dll1B5ControlState*)control)->moveSpeeds = gDll1B5MoveSpeeds0;
            ((Dll1B5ControlState*)control)->weaponDefNo = DLL1B5_WEAPON_DEF_1;
            obj->anim.resetHitboxFlags = obj->anim.resetHitboxFlags | INTERACT_FLAG_DISABLED;
            obj->anim.currentMoveProgress = (f32)(s32)randomGetRange(0, 0x63) / 100.0f;
            break;
        case 0x33E34:
            ((Dll1B5ControlState*)control)->moveIds = gDll1B5MoveIds1;
            ((Dll1B5ControlState*)control)->moveSpeeds = gDll1B5MoveSpeeds1;
            ((Dll1B5ControlState*)control)->weaponDefNo = DLL1B5_WEAPON_DEF_1;
            obj->anim.resetHitboxFlags = obj->anim.resetHitboxFlags | INTERACT_FLAG_DISABLED;
            obj->anim.currentMoveProgress = (f32)(s32)randomGetRange(0, 0x63) / 100.0f;
            break;
        case 0x45C47:
            ((Dll1B5ControlState*)control)->moveIds = gDll1B5MoveIds2;
            ((Dll1B5ControlState*)control)->moveSpeeds = gDll1B5MoveSpeeds2;
            ObjHits_DisableObject(obj);
            ((Dll1B5ControlState*)control)->weaponDefNo = DLL1B5_WEAPON_DEF_2;
            obj->anim.resetHitboxFlags = obj->anim.resetHitboxFlags | INTERACT_FLAG_DISABLED;
            obj->anim.currentMoveProgress = (f32)(s32)randomGetRange(0, 0x63) / 100.0f;
            break;
        case 0x460B6:
            ((Dll1B5ControlState*)control)->moveIds = gDll1B5MoveIds4;
            ((Dll1B5ControlState*)control)->moveSpeeds = gDll1B5MoveSpeeds4;
            ObjHits_DisableObject(obj);
            obj->anim.resetHitboxFlags = obj->anim.resetHitboxFlags | INTERACT_FLAG_DISABLED;
            obj->anim.currentMoveProgress = (f32)(s32)randomGetRange(0, 0x63) / 100.0f;
            break;
        case 0x3433F:
            ((Dll1B5ControlState*)control)->moveIds = playerAnimTableBase->heavyScuff.anims;
            ((Dll1B5ControlState*)control)->moveSpeeds = playerAnimTableBase->heavyScuff.rates;
            obj->anim.resetHitboxFlags = obj->anim.resetHitboxFlags | INTERACT_FLAG_DISABLED;
            obj->anim.currentMoveProgress = (f32)(s32)randomGetRange(0, 0x63) / 100.0f;
            break;
        case 0x46A51:
            if (mainGetBit(GAMEBIT_LV_ChallengeGate1Complete)) {
                obj->anim.resetHitboxFlags = obj->anim.resetHitboxFlags | INTERACT_FLAG_DISABLED;
            }
            ((Dll1B5ControlState*)control)->moveIds = playerAnimTableBase->lightScuff.anims;
            ((Dll1B5ControlState*)control)->moveSpeeds = playerAnimTableBase->lightScuff.rates;
            break;
        case 0x46A55:
            if (mainGetBit(GAMEBIT_LV_ChallengeGate2Complete)) {
                obj->anim.resetHitboxFlags = obj->anim.resetHitboxFlags | INTERACT_FLAG_DISABLED;
            }
            ((Dll1B5ControlState*)control)->moveIds = playerAnimTableBase->lightScuff.anims;
            ((Dll1B5ControlState*)control)->moveSpeeds = playerAnimTableBase->lightScuff.rates;
            break;
        case 0x49928:
            if (mainGetBit(GAMEBIT_SC_ChallengeGate3Complete)) {
                obj->anim.resetHitboxFlags = obj->anim.resetHitboxFlags | INTERACT_FLAG_DISABLED;
            }
            ((Dll1B5ControlState*)control)->moveIds = playerAnimTableBase->lightScuff.anims;
            ((Dll1B5ControlState*)control)->moveSpeeds = playerAnimTableBase->lightScuff.rates;
            break;
        case 0x499AC:
        case 0x499AE:
        case 0x499AF:
        case 0x499B0:
        case 0x499B1:
        case 0x499B2:
            ((GroundBaddieState*)inner)->baddie.substate = 2;
            ((Dll1B5ControlState*)control)->moveIds = playerAnimTableBase->heavyScuff.anims;
            ((Dll1B5ControlState*)control)->moveSpeeds = playerAnimTableBase->heavyScuff.rates;
            ((Dll1B5ControlState*)control)->wanderTimer = (f32)(s32)randomGetRange(0x78, 0xB4);
            obj->anim.currentMoveProgress = (f32)(s32)randomGetRange(0, 0x63) / 100.0f;
            break;
        case 0x499B5:
        case 0x499B6:
            obj->userData1 = 1;
            ((Dll1B5ControlState*)control)->moveIds = playerAnimTableBase->heavyScuff.anims;
            ((Dll1B5ControlState*)control)->moveSpeeds = playerAnimTableBase->heavyScuff.rates;
            break;
        default:
            ((Dll1B5ControlState*)control)->moveIds = playerAnimTableBase->lightScuff.anims;
            ((Dll1B5ControlState*)control)->moveSpeeds = playerAnimTableBase->lightScuff.rates;
            break;
        }
    }
    Lightfoot_ResetScriptedPosition(obj);
    ObjAnim_SetMoveProgress((ObjAnimComponent*)obj, (f32)(s32)randomGetRange(0, 0x63) / 100.0f);
    ((Dll1B5ControlState*)control)->movementSfxId = (u16)(randomGetRange(0, 1) != 0 ? 0x133 : 0x134);
    ((Dll1B5ControlState*)control)->pulseTimer = 15.0f;
    if (obj->userData1 != 0) {
        ObjHits_DisableObject(obj);
    }
}

void dll437_release(void) {
}

void dll437_initialise(void) {
    gDll1B5StateHandlers[0] = Lightfoot_UpdateAnimationCycle;
    gDll1B5StateHandlers[1] = Lightfoot_UpdateButtonTimingChallenge;
    gDll1B5StateHandlers[2] = Lightfoot_UpdateTargetAnimationCycle;
    gDll1B5StateHandlers[3] = (Dll1B5StateHandler)Lightfoot_UpdateRandomTurn;
    gDll1B5StateHandlers[4] = Lightfoot_UpdateWanderSteering;
    gDll1B5SubstateHandlers[0] = Lightfoot_UpdateChallengeGateInteraction;
    gDll1B5SubstateHandlers[1] = Lightfoot_UpdateCompletionInteraction;
    gDll1B5SubstateHandlers[2] = Lightfoot_UpdateProximityInteractionState;
}

ObjectDescriptor gDll1B5ObjDescriptor = {
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
    dll437_getExtraSize,
};

Dll1B5StateHandler gDll1B5StateHandlers[DLL1B5_STATE_HANDLER_COUNT];
Dll1B5SubstateHandler gDll1B5SubstateHandlers[DLL1B5_SUBSTATE_HANDLER_COUNT];
