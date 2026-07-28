/*
 * DLL 85 / 0x55 - peering camera mode.
 */
#include "main/mm.h"
#include "main/resource.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/camera_object.h"
#include "main/dll/CAM/camperv_state.h"
#include "game/objects/object.h"
#include "main/frame_timing.h"
#include "main/dll/dll_0055_cameramodeperv.h"

CameraModePervState* lbl_803DD5C8;


void CameraModePerv_copyToCurrent(void)
{
}

void CameraModePerv_free(void)
{
    mm_free((void*)lbl_803DD5C8);
    lbl_803DD5C8 = NULL;
}

void CameraModePerv_update(CameraObject* camera)
{
    GameObject* target = (GameObject*)camera->anim.targetObj;

    lbl_803DD5C8->timer -= 0.2f * timeDelta;
    if (lbl_803DD5C8->timer < 20.0f)
    {
        lbl_803DD5C8->timer = 20.0f;
    }
    camera->anim.localPosX =
        target->anim.worldPosX - 5.0f * mathSinf(3.1415927f * (f32)(s32)target->anim.rotX / 32768.0f);
    camera->anim.localPosY = lbl_803DD5C8->cameraY;
    camera->anim.localPosZ =
        target->anim.worldPosZ - 5.0f * mathCosf(3.1415927f * (f32)(s32)target->anim.rotX / 32768.0f);
    camera->anim.rotX = 0;
    camera->anim.rotY = -0x4000;
    camera->anim.rotZ = 0;
}

void CameraModePerv_init(CameraObject* camera)
{

    if (lbl_803DD5C8 == NULL)
    {
        lbl_803DD5C8 = (CameraModePervState*)mmAlloc(sizeof(CameraModePervState), 15, 0);
    }
    lbl_803DD5C8->timer = 100.0f;
    lbl_803DD5C8->cameraY = ((GameObject*)camera->anim.targetObj)->anim.worldPosY - 200.0f;
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
