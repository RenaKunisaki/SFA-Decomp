/*
 * DLL 70 / 0x46 - debug camera mode.
 */
#include "main/camera_interface.h"
#include "main/dll/CAM/camdebug_state.h"
#include "main/dll/CAM/camstatic_state.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/dll/dll_0046_cameramodedebug.h"
#include "main/pad.h"
#include "main/resource.h"
#include "dolphin/pad.h"

CameraModeDebugState* gCamDebugState;

#define CAMCONTROL_ACTION_DEFAULT 0x42

void CameraModeDebug_copyToCurrent_nop(void)
{
}

void CameraModeDebug_free(void)
{
    mm_free(gCamDebugState);
    gCamDebugState = 0;
}

void CameraModeDebug_update(CameraObject* cam)
{
    GameObject* state;
    u16 held;
    f32 move;
    f32 absMove;
    f32 absVel;
    f32 factor;
    f32 radius;

    move = 0.0f;
    state = (GameObject*)cam->anim.targetObj;
    held = getButtonsHeld(0);
    if (((u16)getButtonsJustPressed(0) & PAD_BUTTON_RIGHT) != 0)
    {
        (*gCameraInterface)->setMode(CAMCONTROL_ACTION_DEFAULT, 0, 1, 0, NULL, 0, 0xff);
        return;
    }
    if ((held & PAD_BUTTON_UP) != 0)
    {
        move = -0.04f * gCamDebugState->orbitRadius;
    }
    if ((held & PAD_BUTTON_DOWN) != 0)
    {
        move = 0.04f * gCamDebugState->orbitRadius;
    }
    absMove = (move < 0.0f) ? -move : move;
    {
        CameraModeDebugState* st = gCamDebugState;
        f32 vel = st->radiusVelocity;
        absVel = (vel < 0.0f) ? -vel : vel;
        factor = (absVel > absMove) ? 0.3f : 0.08f;
        st->radiusVelocity = factor * (move - vel) + st->radiusVelocity;
    }
    gCamDebugState->orbitRadius = gCamDebugState->orbitRadius + gCamDebugState->radiusVelocity;
    if (gCamDebugState->orbitRadius < 20.0f)
    {
        gCamDebugState->orbitRadius = 20.0f;
    }
    if (gCamDebugState->orbitRadius > 2000.0f)
    {
        gCamDebugState->orbitRadius = 2000.0f;
    }
    {
        u16 dx = (u16)(padGetCX(0) * 3);
        u16 dy = (u16)(padGetCY(0) * 3);
        cam->anim.rotX = (s16)(cam->anim.rotX - dx);
        cam->anim.rotY = (s16)(cam->anim.rotY + dy);
    }
    {
        f32 cosYaw = mathSinf(3.1415927f * (f32)(s32)(cam->anim.rotX - 0x4000) / 32768.0f);
        f32 sinYaw = mathCosf(3.1415927f * (f32)(s32)(cam->anim.rotX - 0x4000) / 32768.0f);
        f32 cosPitch = mathCosf(3.1415927f * (f32)(s32)cam->anim.rotY / 32768.0f);
        f32 sinPitch = mathSinf(3.1415927f * (f32)(s32)cam->anim.rotY / 32768.0f);
        f32 vy, h, px;
        radius = gCamDebugState->orbitRadius;
        vy = radius * sinPitch;
        h = radius * cosPitch;
        px = h * sinYaw;
        h = h * cosYaw;
        cam->anim.worldPosX = state->anim.worldPosX + px;
        {
            f32 base28 = 20.0f + state->anim.worldPosY;
            cam->anim.worldPosY = base28 + vy;
        }
        cam->anim.worldPosZ = state->anim.worldPosZ + h;
    }
    Obj_TransformWorldPointToLocal(cam->anim.worldPosX, cam->anim.worldPosY, cam->anim.worldPosZ, &cam->anim.localPosX,
                                   &cam->anim.localPosY, &cam->anim.localPosZ, cam->anim.parentAddress);
}

void CameraModeDebug_init(void)
{
    if (gCamDebugState == NULL)
    {
        gCamDebugState = (CameraModeDebugState*)mmAlloc(sizeof(CameraModeDebugState), 0xf, 0);
    }
    gCamDebugState->orbitRadius = 50.0f;
    gCamDebugState->radiusVelocity = 0.0f;
    return;
}

void CameraModeDebug_release_nop(void)
{
}

void CameraModeDebug_initialise_nop(void)
{
}

ResourceDescriptorCallbacks8 lbl_80319C28 = {
    {0x00000000, 0x00000000, 0x00000000, 0x00060000},
    {(ResourceDescriptorCallback)CameraModeDebug_initialise_nop,
     (ResourceDescriptorCallback)CameraModeDebug_release_nop,
     0x00000000,
     (ResourceDescriptorCallback)CameraModeDebug_init,
     (ResourceDescriptorCallback)CameraModeDebug_update,
     (ResourceDescriptorCallback)CameraModeDebug_free,
     (ResourceDescriptorCallback)CameraModeDebug_copyToCurrent_nop,
     0x00000000}};
