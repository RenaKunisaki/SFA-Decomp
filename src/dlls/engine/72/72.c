/*
 * DLL 72 / 0x48 - static camera mode.
 */
#include "main/camera_interface.h"
#include "main/camera_object.h"
#include "main/object_transform.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/dll/CAM/camstatic_state.h"
#include "main/frame_timing.h"
#include "main/dll/dll_0048_cameramodestatic.h"
#include "main/resource.h"

CameraModeStaticState* gCameraModeStaticState;

#define CAMSTATIC_CAMMODE_DEFAULT 0x42


void* camStaticFindNearestAnchor(f32 x, f32 y, f32 z, int filter1, int filter2)
{
    int* list;
    int i;
    void* best;
    f32 bestDist;
    int count;
    GameObject* obj;
    int* tmpList;
    f32 dx, dy, dz;
    f32 yy;
    f32 dist;

    bestDist = 100000.0f;
    best = NULL;
    tmpList = (int*)ObjGroup_GetObjects(7, &count);
    for (i = 0, list = tmpList; i < count; i++)
    {
        obj = (GameObject*)*list;
        if (obj->anim.classId == filter2 &&
            *(u8*)(obj->anim.placementDataAddress + 0x18) == filter1)
        {
            dx = x - obj->anim.worldPosX;
            dy = y - obj->anim.worldPosY;
            dz = z - obj->anim.worldPosZ;
            yy = dy * dy;
            dist = sqrtf(yy + dx * dx + dz * dz);
            if (dist < bestDist)
            {
                bestDist = dist;
                best = obj;
            }
        }
        list++;
    }
    return best;
}

void CameraModeStatic_copyToCurrent(void)
{
}

void CameraModeStatic_free(void)
{
    mm_free(gCameraModeStaticState);
    gCameraModeStaticState = 0;
}

void CameraModeStatic_update(short* camObj)
{
    int angle;
    u32 pitch;
    int placement;
    int viewObj;
    f32 dx;
    f32 dy;
    f32 dz;

    if (gCameraModeStaticState->missingObject != 0)
    {
        (*gCameraInterface)->setMode(CAMSTATIC_CAMMODE_DEFAULT, 0, 1, 0, NULL, 0, 0xff);
    }
    else
    {
        viewObj = *(int*)(camObj + 0x52);
        placement = (int)gCameraModeStaticState->staticObject->anim.placementData;
        if ((((CameraModeStaticPlacement*)placement)->flags & 1) == 0)
        {
            *camObj = ((CameraModeStaticPlacement*)placement)->yaw + 0x8000;
        }
        if ((((CameraModeStaticPlacement*)placement)->flags & 2) == 0)
        {
            camObj[1] = ((CameraModeStaticPlacement*)placement)->pitch;
        }
        if ((((CameraModeStaticPlacement*)placement)->flags & 4) == 0)
        {
            camObj[2] = ((CameraModeStaticPlacement*)placement)->roll;
        }
        ((CameraObject*)camObj)->anim.worldPosX = gCameraModeStaticState->staticObject->anim.worldPosX;
        ((CameraObject*)camObj)->anim.worldPosY = gCameraModeStaticState->staticObject->anim.worldPosY;
        ((CameraObject*)camObj)->anim.worldPosZ = gCameraModeStaticState->staticObject->anim.worldPosZ;
        ((CameraObject*)camObj)->fov = (float)(u32)((CameraModeStaticPlacement*)placement)->fovByte;
        dx = ((CameraObject*)camObj)->anim.worldPosX - *(float*)(viewObj + 0x18);
        dy = ((CameraObject*)camObj)->anim.worldPosY - *(float*)(viewObj + 0x1c);
        dz = ((CameraObject*)camObj)->anim.worldPosZ - *(float*)(viewObj + 0x20);
        if ((((CameraModeStaticPlacement*)placement)->flags & 1) != 0)
        {
            angle = getAngle(dx, dz);
            *camObj = 0x8000 - angle;
        }
        if ((((CameraModeStaticPlacement*)placement)->flags & 2) != 0)
        {
            pitch = getAngle(dy, sqrtf(dx * dx + dz * dz)) & 0xffff;
            angle = (pitch - (int)((CameraModeStaticPlacement*)placement)->pitch) - (u32)(u16)camObj[1];
            if (0x8000 < angle)
            {
                angle = angle + -0xffff;
            }
            if (angle < -0x8000)
            {
                angle = angle + 0xffff;
            }
            camObj[1] += (int)(angle * framesThisStep) >> 3;
        }
        if ((((CameraModeStaticPlacement*)placement)->flags & 4) != 0)
        {
            viewObj = camObj[2] - (u32)(u16) * (short*)(viewObj + 4);
            if (0x8000 < viewObj)
            {
                viewObj = viewObj + -0xffff;
            }
            if (viewObj < -0x8000)
            {
                viewObj = viewObj + 0xffff;
            }
            camObj[2] += (int)(viewObj * framesThisStep) >> 3;
        }
        Obj_TransformWorldPointToLocal(
            ((CameraObject*)camObj)->anim.worldPosX, ((CameraObject*)camObj)->anim.worldPosY,
            ((CameraObject*)camObj)->anim.worldPosZ, &((CameraObject*)camObj)->anim.localPosX,
            &((CameraObject*)camObj)->anim.localPosY, &((CameraObject*)camObj)->anim.localPosZ,
            (GameObject*)((CameraObject*)camObj)->anim.parentAddress);
    }
    return;
}

void CameraModeStatic_init(u8* cam, int p2, int* p3)
{
    u8* setup;
    GameObject* state;
    GameObject* best;
    s16 yaw;
    s16 pitch;
    s16 roll;
    f32 dx;
    f32 dy;
    f32 dz;

    state = ((CameraObject*)cam)->anim.targetObj;
    if (gCameraModeStaticState == NULL)
    {
        gCameraModeStaticState = (CameraModeStaticState*)mmAlloc(sizeof(CameraModeStaticState), 15, 0);
    }
    gCameraModeStaticState->active = 1;
    gCameraModeStaticState->missingObject = 0;
    best = (GameObject*)camStaticFindNearestAnchor(state->anim.worldPosX, state->anim.worldPosY, state->anim.worldPosZ, *p3, 18);
    if (best == NULL)
    {
        gCameraModeStaticState->missingObject = 1;
        return;
    }
    gCameraModeStaticState->staticObject = best;
    setup = (u8*)best->anim.placementData;
    dx = best->anim.worldPosX - state->anim.worldPosX;
    dy = best->anim.worldPosY - state->anim.worldPosY;
    dz = best->anim.worldPosZ - state->anim.worldPosZ;
    if ((((CameraModeStaticPlacement*)setup)->flags & 1) != 0)
    {
        yaw = 0x8000 - getAngle(dx, dz);
    }
    else
    {
        yaw = ((CameraModeStaticPlacement*)setup)->yaw + 0x8000;
    }
    if ((((CameraModeStaticPlacement*)setup)->flags & 2) != 0)
    {
        pitch = (s16)getAngle(dy, sqrtf(dx * dx + dz * dz));
        pitch -= ((CameraModeStaticPlacement*)setup)->pitch;
    }
    else
    {
        pitch = ((CameraModeStaticPlacement*)setup)->pitch;
    }
    if ((((CameraModeStaticPlacement*)setup)->flags & 4) != 0)
    {
        roll = state->anim.rotZ;
    }
    else
    {
        roll = ((CameraModeStaticPlacement*)setup)->roll;
    }
    {
        f32 fov = (f32)(u32)((CameraModeStaticPlacement*)setup)->fovByte;
        ((CameraObject*)cam)->anim.worldPosX = best->anim.worldPosX;
        ((CameraObject*)cam)->anim.worldPosY = best->anim.worldPosY;
        ((CameraObject*)cam)->anim.worldPosZ = best->anim.worldPosZ;
        ((CameraObject*)cam)->anim.rotX = yaw;
        ((CameraObject*)cam)->anim.rotY = pitch;
        ((CameraObject*)cam)->anim.rotZ = roll;
        ((CameraObject*)cam)->fov = fov;
    }
    Obj_TransformWorldPointToLocal(((CameraObject*)cam)->anim.worldPosX, ((CameraObject*)cam)->anim.worldPosY,
                                   ((CameraObject*)cam)->anim.worldPosZ, &((CameraObject*)cam)->anim.localPosX,
                                   &((CameraObject*)cam)->anim.localPosY, &((CameraObject*)cam)->anim.localPosZ,
                                   (GameObject*)((CameraObject*)cam)->anim.parentAddress);
}

void CameraModeStatic_release(void)
{
}

void CameraModeStatic_initialise(void)
{
}

ResourceDescriptorCallbacks8 gCameraModeStaticDescriptor = {
    {0x00000000, 0x00000000, 0x00000000, 0x00060000},
    {(ResourceDescriptorCallback)CameraModeStatic_initialise,
     (ResourceDescriptorCallback)CameraModeStatic_release,
     0x00000000,
     (ResourceDescriptorCallback)CameraModeStatic_init,
     (ResourceDescriptorCallback)CameraModeStatic_update,
     (ResourceDescriptorCallback)CameraModeStatic_free,
     (ResourceDescriptorCallback)CameraModeStatic_copyToCurrent,
     0x00000000}};
