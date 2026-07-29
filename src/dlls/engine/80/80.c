/*
 * DLL 80 / 0x50 - crawl camera mode.
 */
#include "main/resource.h"
#include "main/dll/CAM/cutCam.h"
#include "main/camera_interface.h"
#include "main/frame_timing.h"
#include "main/dll/CAM/camcrawl_state.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/object_transform.h"

CameraModeCrawlState* gCameraModeCrawlState;


void CameraModeCrawl_copyToCurrent(void* param1, int param2)
{
    int obj;
    GameObject* target;
    int yaw;
    f32 s, c;
    f32 pos[3];

    if (param1 == NULL)
    {
        return;
    }
    obj = (int)(*gCameraInterface)->getCamera();
    target = (GameObject*)((CameraObject*)obj)->anim.targetObj;
    yaw = target->anim.rotX;

    if (param2 == 0)
    {
        s = mathSinf(3.1415927f * (f32)(s32)target->anim.rotX / 32768.0f);
        c = mathCosf(3.1415927f * (f32)(s32)target->anim.rotX / 32768.0f);
    }
    else
    {
        s = -mathSinf(3.1415927f * (f32)(s32)target->anim.rotX / 32768.0f);
        c = -mathCosf(3.1415927f * (f32)(s32)target->anim.rotX / 32768.0f);
    }
    {
        target->anim.rotX = getAngle(s, c);
    }
    camcontrol_getTargetPosition((CameraObject*)obj, &target->anim, pos, NULL);
    target->anim.rotX = yaw;
    {
        f32 coord;
        coord = pos[0];
        ((CameraObject*)obj)->anim.worldPosX = coord;
        ((CameraObject*)obj)->probePosX = coord;
        coord = pos[1];
        ((CameraObject*)obj)->anim.worldPosY = coord;
        ((CameraObject*)obj)->probePosY = coord;
        coord = pos[2];
        ((CameraObject*)obj)->anim.worldPosZ = coord;
        ((CameraObject*)obj)->probePosZ = coord;
    }
    Obj_TransformWorldPointToLocal(((CameraObject*)obj)->anim.worldPosX, ((CameraObject*)obj)->anim.worldPosY,
                                   ((CameraObject*)obj)->anim.worldPosZ, &((GameObject*)obj)->anim.localPosX,
                                   &((GameObject*)obj)->anim.localPosY, &((GameObject*)obj)->anim.localPosZ,
                                   (GameObject*)((CameraObject*)obj)->anim.parentAddress);
    gCameraModeCrawlState->flags.useDefaultHandler = 1;
}

void CameraModeCrawl_free(void)
{
    mm_free((void*)gCameraModeCrawlState);
    gCameraModeCrawlState = NULL;
}

void CameraModeCrawl_update(CameraObject* camera)
{
    GameObject* target = (GameObject*)camera->anim.targetObj;
    int delta;
    f32 dx, outY, dz, outW;
    int other;

    if (target == NULL)
    {
        return;
    }
    if (gCameraModeCrawlState->flags.useDefaultHandler == 0)
    {
        camera->anim.worldPosX =
            13.0f * mathSinf(3.1415927f * (f32)(s32)target->anim.rotX / 32768.0f) + target->anim.worldPosX;
        camera->anim.worldPosZ =
            13.0f * mathCosf(3.1415927f * (f32)(s32)target->anim.rotX / 32768.0f) + target->anim.worldPosZ;
        camera->anim.worldPosY = 20.0f + target->anim.worldPosY;
        dx = camera->anim.localPosX - target->anim.worldPosX;
        dz = camera->anim.localPosZ - target->anim.worldPosZ;
        {
            int t = 0x8000 - (u16)getAngle(dx, dz);
            delta = t - (u16)camera->anim.rotX;
        }
        if (0x8000 < delta)
        {
            delta = delta - 0xffff;
        }
        if (delta < -0x8000)
        {
            delta = delta + 0xffff;
        }
        camera->anim.rotX = (s16)((f32)(s32)camera->anim.rotX + interpolate((f32)(s32)delta, 0.125f, timeDelta));
        camera->anim.rotX = (s16)(0x8000 - getAngle(dx, dz));
        camera->anim.rotY = 2048;
    }
    else
    {
        other = (int)(*gCameraInterface)->getDefaultHandlerEntry();
        (*gCameraInterface)->getRelativePosition(camera, &dx, &outY, &dz, &outW, 35.0f, 0);
        {
            int t = 0x8000 - (u16)getAngle(dx, dz);
            delta = t - (u16)camera->anim.rotX;
        }
        if (0x8000 < delta)
        {
            delta = delta - 0xffff;
        }
        if (delta < -0x8000)
        {
            delta = delta + 0xffff;
        }
        camera->anim.rotX += delta;
        (*(void (**)(CameraObject*, f32, f32))(*(int*)(*(int*)(other + 4)) + 24))(camera, target->anim.worldPosY,
                                                                                    outW);
    }
    Obj_TransformWorldPointToLocal(camera->anim.worldPosX, camera->anim.worldPosY, camera->anim.worldPosZ,
                                   &camera->anim.localPosX, &camera->anim.localPosY, &camera->anim.localPosZ,
                                   (GameObject*)camera->anim.parentAddress);
}

void CameraModeCrawl_init(void)
{
    if (gCameraModeCrawlState == NULL)
    {
        gCameraModeCrawlState = (CameraModeCrawlState*)mmAlloc(sizeof(CameraModeCrawlState), 15, 0);
        memset(gCameraModeCrawlState, 0, sizeof(CameraModeCrawlState));
    }
}

void CameraModeCrawl_release(void)
{
}

void CameraModeCrawl_initialise(void)
{
}

ResourceDescriptorCallbacks8 gCameraModeCrawlDescriptor = {{0x00000000, 0x00000000, 0x00000000, 0x00060000},
        {(ResourceDescriptorCallback)CameraModeCrawl_initialise, (ResourceDescriptorCallback)CameraModeCrawl_release,
        0x00000000, (ResourceDescriptorCallback)CameraModeCrawl_init, (ResourceDescriptorCallback)CameraModeCrawl_update,
        (ResourceDescriptorCallback)CameraModeCrawl_free, (ResourceDescriptorCallback)CameraModeCrawl_copyToCurrent, 0x00000000}};
