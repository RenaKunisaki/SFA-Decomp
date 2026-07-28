/*
 * DLL 81 / 0x51 - cannon camera mode.
 */
#include "main/resource.h"
#include "main/dll/CAM/camera_mode_cannon_state.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/dll/dll_0051_cameramodecannon.h"
#include "main/objprint_api.h"

CameraModeCannonState* gCameraModeCannonState;


void CameraModeCannon_copyToCurrent(void)
{
}

void CameraModeCannon_free(void)
{
    mm_free((void*)gCameraModeCannonState);
    gCameraModeCannonState = NULL;
}

void CameraModeCannon_update(CameraObject* camera)
{
    s16* vec;
    s16 yaw;
    s16 delta;

    vec = objModelGetVecFn_800395d8((GameObject*)gCameraModeCannonState->target, 0);
    if (gCameraModeCannonState->target == NULL)
    {
        return;
    }
    yaw = camera->anim.rotX;
    delta = (s16)((0x8000 - gCameraModeCannonState->target->anim.rotX) - vec[1] - yaw);
    camera->anim.rotX = (f32)(s32)yaw + (f32)(s32)delta / 5.0f;
    camera->anim.localPosX =
        gCameraModeCannonState->target->anim.localPosX - 60.0f * mathSinf(3.1415927f * (f32)(s32)(-camera->anim.rotX) / 32768.0f);
    camera->anim.localPosY = 80.0f + gCameraModeCannonState->target->anim.localPosY;
    camera->anim.localPosZ =
        gCameraModeCannonState->target->anim.localPosZ - 60.0f * mathCosf(3.1415927f * (f32)(s32)(-camera->anim.rotX) / 32768.0f);
}

void CameraModeCannon_init(CameraObject* camera, int unused, int* p3)
{

    if (gCameraModeCannonState == NULL)
    {
        gCameraModeCannonState = (CameraModeCannonState*)mmAlloc(sizeof(CameraModeCannonState), 15, 0);
    }
    if (p3 != NULL)
    {
        gCameraModeCannonState->target = (GameObject*)*p3;
    }
    else
    {
        gCameraModeCannonState->target = NULL;
    }
    camera->anim.rotY = 2800;
}

void CameraModeCannon_release(void)
{
}

void CameraModeCannon_initialise(void)
{
}

ResourceDescriptorCallbacks8 lbl_80319E98 = {{0x00000000, 0x00000000, 0x00000000, 0x00060000},
        {(ResourceDescriptorCallback)CameraModeCannon_initialise, (ResourceDescriptorCallback)CameraModeCannon_release,
        0x00000000, (ResourceDescriptorCallback)CameraModeCannon_init, (ResourceDescriptorCallback)CameraModeCannon_update,
        (ResourceDescriptorCallback)CameraModeCannon_free, (ResourceDescriptorCallback)CameraModeCannon_copyToCurrent, 0x00000000}};
