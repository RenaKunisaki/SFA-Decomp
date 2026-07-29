/*
 * DLL 85 / 0x55 - peering camera mode.
 */
#include "main/resource.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/dll/CAM/camperv_state.h"
#include "game/objects/object.h"
#include "main/frame_timing.h"
#include "main/dll/dll_0055_cameramodeperv.h"

CameraModePervState* gCameraModePervState;


void CameraModePerv_copyToCurrent(void)
{
}

void CameraModePerv_free(void)
{
    mm_free((void*)gCameraModePervState);
    gCameraModePervState = NULL;
}

void CameraModePerv_update(CameraObject* camera)
{
    GameObject* target = (GameObject*)camera->anim.targetObj;

    gCameraModePervState->timer -= 0.2f * timeDelta;
    if (gCameraModePervState->timer < 20.0f)
    {
        gCameraModePervState->timer = 20.0f;
    }
    camera->anim.localPosX =
        target->anim.worldPosX - 5.0f * mathSinf(3.1415927f * (f32)(s32)target->anim.rotX / 32768.0f);
    camera->anim.localPosY = gCameraModePervState->cameraY;
    camera->anim.localPosZ =
        target->anim.worldPosZ - 5.0f * mathCosf(3.1415927f * (f32)(s32)target->anim.rotX / 32768.0f);
    camera->anim.rotX = 0;
    camera->anim.rotY = -0x4000;
    camera->anim.rotZ = 0;
}

void CameraModePerv_init(CameraObject* camera)
{

    if (gCameraModePervState == NULL)
    {
        gCameraModePervState = (CameraModePervState*)mmAlloc(sizeof(CameraModePervState), 15, 0);
    }
    gCameraModePervState->timer = 100.0f;
    gCameraModePervState->cameraY = ((GameObject*)camera->anim.targetObj)->anim.worldPosY - 200.0f;
}

void CameraModePerv_release(void)
{
}

void CameraModePerv_initialise(void)
{
}

ResourceDescriptorCallbacks8 lbl_80319F58 = {{0x00000000, 0x00000000, 0x00000000, 0x00060000},
        {(ResourceDescriptorCallback)CameraModePerv_initialise, (ResourceDescriptorCallback)CameraModePerv_release,
        0x00000000, (ResourceDescriptorCallback)CameraModePerv_init, (ResourceDescriptorCallback)CameraModePerv_update,
        (ResourceDescriptorCallback)CameraModePerv_free, (ResourceDescriptorCallback)CameraModePerv_copyToCurrent, 0x00000000}};
