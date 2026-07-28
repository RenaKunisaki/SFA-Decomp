/*
 * DLL 74 / 0x4A - ship-battle camera mode.
 */
#include "main/resource.h"
#include "main/dll/CAM/camshipbattle_state.h"
#include "main/frame_timing.h"
#include "main/dll/dll_004A_cameramodeshipbattle.h"
#include "main/dll/ship_battle_api.h"

CameraModeShipBattleState* gCamShipBattleState;

void CameraModeShipBattle_copyToCurrent(void)
{
}

void CameraModeShipBattle_free(void)
{
    mm_free(gCamShipBattleState);
    gCamShipBattleState = 0;
}

void CameraModeShipBattle_update(CameraObject* cam)
{
    f32 fa;
    f32 fb;
    f32 fc;
    f32 lateralStep;
    CameraModeShipBattleState* state;
    int mode = 0;
    GameObject* focus = (GameObject*)cam->anim.targetObj;
    if (focus != NULL)
    {
        mode = shipBattleFn_801eed24((GameObject*)focus);
    }
    state = gCamShipBattleState;
    if (mode != state->mode)
    {
        if (mode == 2)
        {
            fa = 2.2e+02f;
        }
        else
        {
            fa = 1.2e+02f;
        }
        if (mode != 2 && mode != 5)
        {
            fb = 75.0f;
            fc = 0.0f;
        }
        else
        {
            fb = 105.0f;
            fc = state->smoothedYOffset;
        }
        state->mode = mode;
        gCamShipBattleState->lateralDelta = fa - gCamShipBattleState->targetLateralOffset;
        gCamShipBattleState->startLateralOffset = gCamShipBattleState->targetLateralOffset;
        gCamShipBattleState->verticalDelta = fb - (gCamShipBattleState->verticalOffset + fc);
        gCamShipBattleState->startVerticalOffset = gCamShipBattleState->verticalOffset;
        gCamShipBattleState->blendTimer = 0.0f;
    }
    fb = gCamShipBattleState->blendTimer;
    fa = 1.0f;
    if (fb < fa)
    {
        gCamShipBattleState->blendTimer = 0.005f * timeDelta + fb;
        if (gCamShipBattleState->blendTimer > fa)
        {
            gCamShipBattleState->blendTimer = fa;
        }
        gCamShipBattleState->targetLateralOffset = gCamShipBattleState->blendTimer * gCamShipBattleState->lateralDelta +
                                                   gCamShipBattleState->startLateralOffset;
        gCamShipBattleState->verticalOffset = gCamShipBattleState->blendTimer * gCamShipBattleState->verticalDelta +
                                              gCamShipBattleState->startVerticalOffset;
    }
    if (mode != 2 && mode != 5)
    {
        gCamShipBattleState->smoothedZOffset = -((f32)focus->anim.rotZ / 3367.0f * timeDelta - gCamShipBattleState->smoothedZOffset);
        gCamShipBattleState->smoothedYOffset = -((f32)focus->anim.rotY / 1365.0f * timeDelta - gCamShipBattleState->smoothedYOffset);
        state = gCamShipBattleState;
        fc = 0.02f;
        fb = state->smoothedZOffset;
        fa = fc * fb;
        state->smoothedZOffset = -(fa * timeDelta - fb);
        fb = gCamShipBattleState->smoothedYOffset;
        fa = fc * fb;
        gCamShipBattleState->smoothedYOffset = -(fa * timeDelta - fb);
        cam->anim.worldPosY =
            gCamShipBattleState->smoothedYOffset + (focus->anim.worldPosY + gCamShipBattleState->verticalOffset);
    }
    else
    {
        gCamShipBattleState->smoothedZOffset = -((f32)focus->anim.rotZ / 3367.0f * timeDelta - gCamShipBattleState->smoothedZOffset);
        gCamShipBattleState->smoothedYOffset = -((f32)focus->anim.rotY / 1365.0f * timeDelta - gCamShipBattleState->smoothedYOffset);
        state = gCamShipBattleState;
        fc = 0.02f;
        fb = state->smoothedZOffset;
        fa = fc * fb;
        state->smoothedZOffset = -(fa * timeDelta - fb);
        fb = gCamShipBattleState->smoothedYOffset;
        fa = fc * fb;
        gCamShipBattleState->smoothedYOffset = -(fa * timeDelta - fb);
        cam->anim.worldPosY =
            gCamShipBattleState->smoothedYOffset + (focus->anim.worldPosY + gCamShipBattleState->verticalOffset);
    }
    fa = 98.0f + focus->anim.worldPosX;
    cam->anim.worldPosX = fa + gCamShipBattleState->lateralOffset;
    cam->anim.worldPosZ = focus->anim.worldPosZ + gCamShipBattleState->smoothedZOffset;
    cam->anim.rotY = 0x708;
    cam->anim.rotX = 0x4000;
    cam->anim.rotZ = (s16)(-focus->anim.rotZ >> 3);
    cam->fov = 4e+01f;
    state = gCamShipBattleState;
    lateralStep = (state->targetLateralOffset - state->lateralOffset) / 1e+02f;
    if (lateralStep > 3.0f)
    {
        lateralStep = 3.0f;
    }
    else if (lateralStep < -3.0f)
    {
        lateralStep = -3.0f;
    }
    lateralStep = lateralStep * timeDelta;
    state->lateralOffset = state->lateralOffset + lateralStep;
    Obj_TransformWorldPointToLocal(cam->anim.worldPosX, cam->anim.worldPosY,
                                   cam->anim.worldPosZ, &cam->anim.localPosX,
                                   &cam->anim.localPosY, &cam->anim.localPosZ,
                                   cam->anim.parentAddress);
}

void CameraModeShipBattle_init(void)
{
    float fval;
    u8 zero;

    if (gCamShipBattleState == (CameraModeShipBattleState*)0x0)
    {
        gCamShipBattleState = (CameraModeShipBattleState*)mmAlloc(sizeof(CameraModeShipBattleState), 0xf, 0);
    }
    fval = 0.0f;
    gCamShipBattleState->smoothedZOffset = 0.0f;
    gCamShipBattleState->smoothedYOffset = fval;
    gCamShipBattleState->lateralOffset = 1e+02f;
    fval = 1.2e+02f;
    gCamShipBattleState->startLateralOffset = 1.2e+02f;
    gCamShipBattleState->targetLateralOffset = fval;
    gCamShipBattleState->blendTimer = 1.0f;
    zero = 0;
    gCamShipBattleState->mode = zero;
    fval = 75.0f;
    gCamShipBattleState->startVerticalOffset = 75.0f;
    gCamShipBattleState->verticalOffset = fval;
    return;
}

void CameraModeShipBattle_release(void)
{
}

void CameraModeShipBattle_initialise(void)
{
}

ResourceDescriptorCallbacks8 lbl_80319D18 = {{0x00000000,
                       0x00000000,
                       0x00000000,
                       0x00060000},
                      {(ResourceDescriptorCallback)CameraModeShipBattle_initialise,
                       (ResourceDescriptorCallback)CameraModeShipBattle_release,
                       0x00000000,
                       (ResourceDescriptorCallback)CameraModeShipBattle_init,
                       (ResourceDescriptorCallback)CameraModeShipBattle_update,
                       (ResourceDescriptorCallback)CameraModeShipBattle_free,
                       (ResourceDescriptorCallback)CameraModeShipBattle_copyToCurrent,
                       0x00000000}};
