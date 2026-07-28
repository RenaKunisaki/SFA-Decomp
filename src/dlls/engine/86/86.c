/*
 * DLL 86 / 0x56 - Arwing camera mode.
 */
#include "main/camera_interface.h"
#include "main/resource.h"
#include "main/dll/CAM/camcloudrunner_state.h"
#include "main/frame_timing.h"
#include "game/objects/object.h"
#include "main/dll/ARW/dll_029A_arwarwing.h"
#include "main/object_transform.h"
#include "dolphin/mtx/mtx_legacy.h"
#include "main/vecmath.h"

typedef struct CameraArwingWork
{
    f32 offsetX;
    f32 offsetY;
    f32 offsetZ;
    f32 basePosX;
    f32 basePosY;
    f32 basePosZ;
    u8 pad18[0x24 - 0x18];
    f32 xScale;
    f32 yScale;
    f32 unk2C;
    f32 initOffsetX;
    f32 initOffsetY;
    f32 posZOffset;
    f32 zEaseDenom;
    f32 zEaseNum;
    f32 yawScale;
    f32 pitchScale;
    f32 rollScale;
    f32 rollRate;
    s16 inputYaw;
    s16 inputPitch;
    s16 inputRoll;
    u8 zScaleNear;
    u8 zScaleFar;
    u8 pad5C[0x5E - 0x5C];
    u8 active;
    u8 pad5F[0x60 - 0x5F];
} CameraArwingWork;

f32 gCamArwingWork[24];
void CameraModeArwing_copyToCurrent(void* p1, u32 kind)
{
    if (kind == 12)
    {
        gCamArwingWork[0] = ((f32*)p1)[0];
        gCamArwingWork[1] = ((f32*)p1)[1];
        gCamArwingWork[2] = ((f32*)p1)[2];
        return;
    }
    if (kind == 6)
    {
        ((CameraArwingWork*)gCamArwingWork)->inputYaw = ((s16*)p1)[0];
        ((CameraArwingWork*)gCamArwingWork)->inputPitch = ((s16*)p1)[1];
        ((CameraArwingWork*)gCamArwingWork)->inputRoll = ((s16*)p1)[2];
        return;
    }
    if (kind == 4)
    {
        ((CameraArwingWork*)gCamArwingWork)->posZOffset = ((f32*)p1)[0];
        return;
    }
    ((CameraArwingWork*)gCamArwingWork)->zEaseDenom = ((f32*)p1)[0];
    ((CameraArwingWork*)gCamArwingWork)->zEaseNum = ((f32*)p1)[1];
}

void CameraModeArwing_free(void)
{
}
void CameraModeArwing_update(GameObject* obj)
{
    int targetYaw, targetPitch;
    u8* targetObj = *(u8**)&obj->anim.targetObj;
    int angleDelta;

    obj->anim.worldPosX = gCamArwingWork[0] * ((CameraArwingWork*)gCamArwingWork)->xScale;
    obj->anim.worldPosX = obj->anim.worldPosX + ((CameraArwingWork*)gCamArwingWork)->basePosX;
    obj->anim.worldPosY = gCamArwingWork[1] * ((CameraArwingWork*)gCamArwingWork)->yScale;
    obj->anim.worldPosY = obj->anim.worldPosY + ((CameraArwingWork*)gCamArwingWork)->basePosY;
    obj->anim.worldPosZ = ((GameObject*)targetObj)->anim.worldPosZ + ((CameraArwingWork*)gCamArwingWork)->
        posZOffset;

    if ((s8)targetObj[0xac] != 0x26)
    {
        f32 t = ((CameraArwingWork*)gCamArwingWork)->zEaseNum / ((CameraArwingWork*)gCamArwingWork)->zEaseDenom;
        t = t - 1.0f;
        if (t < 0.0f)
        {
            obj->anim.worldPosZ =
                (f32) - (s32)((CameraArwingWork*)gCamArwingWork)->zScaleNear * t + obj->anim.worldPosZ;
        }
        else
        {
            obj->anim.worldPosZ =
                (f32) - (s32)((CameraArwingWork*)gCamArwingWork)->zScaleFar * t + obj->anim.worldPosZ;
        }
    }

    targetYaw = (s32)((f32)((CameraArwingWork*)gCamArwingWork)->inputYaw *
        ((CameraArwingWork*)gCamArwingWork)->yawScale);
    targetPitch = (s32)((f32)((CameraArwingWork*)gCamArwingWork)->inputPitch *
        ((CameraArwingWork*)gCamArwingWork)->pitchScale);

    if (arwarwing_isDead((GameObject*)targetObj) != 0)
    {
        f32 relX, relY, relZ, relDist;
        int step;
        CameraArwingWork* work;
        ((CameraArwingWork*)gCamArwingWork)->rollRate = 500.0f;
        work = (CameraArwingWork*)gCamArwingWork;
        (*gCameraInterface)->getRelativePosition(obj, &relX, &relY, &relZ, &relDist, 0.0f, 0);
        obj->anim.rotZ = work->rollRate * timeDelta +
            (f32)obj->anim.rotZ;
        angleDelta = 0x8000 - (u16)getAngle(relX, relZ);
        targetYaw = (u16)getAngle(relY, relDist);
        angleDelta -= (u16)obj->anim.rotX;
        if (angleDelta > 0x8000)
        {
            angleDelta = angleDelta - 0xffff;
        }
        if (angleDelta < -0x8000)
        {
            angleDelta = angleDelta + 0xffff;
        }
        step = (s32)((f32)angleDelta * timeDelta);
        obj->anim.rotX = step * 0.0625f + (f32) * (s16*)obj;
        angleDelta = targetYaw - (u16)obj->anim.rotY;
        if (angleDelta > 0x8000)
        {
            angleDelta = angleDelta - 0xffff;
        }
        if (angleDelta < -0x8000)
        {
            angleDelta = angleDelta + 0xffff;
        }
        step = (s32)((f32)angleDelta * timeDelta);
        obj->anim.rotY = step * 0.0625f + (f32)obj->anim.rotY;
    }
    else if (arwarwing_isExplodingOrWarping((GameObject*)targetObj) != 0)
    {
        f32 rollVel = ((CameraArwingWork*)gCamArwingWork)->rollRate * 0.98f;
        ((CameraArwingWork*)gCamArwingWork)->rollRate = rollVel;
        obj->anim.rotZ = rollVel * timeDelta + (f32)obj->anim.rotZ;
    }
    else
    {
        f32 step;
        int targetRoll = (s32)((f32)((CameraArwingWork*)gCamArwingWork)->inputRoll *
            ((CameraArwingWork*)gCamArwingWork)->rollScale);
        targetRoll = targetRoll - (u16)obj->anim.rotZ;
        if (targetRoll > 0x8000)
        {
            targetRoll = targetRoll - 0xffff;
        }
        if (targetRoll < -0x8000)
        {
            targetRoll = targetRoll + 0xffff;
        }
        step = (f32)targetRoll * timeDelta;
        obj->anim.rotZ = step * 0.0625f + (f32)obj->anim.rotZ;
        targetYaw = targetYaw - (u16)obj->anim.rotX;
        if (targetYaw > 0x8000)
        {
            targetYaw = targetYaw - 0xffff;
        }
        if (targetYaw < -0x8000)
        {
            targetYaw = targetYaw + 0xffff;
        }
        step = (f32)targetYaw * timeDelta;
        obj->anim.rotX = step * 0.0625f + (f32) * (s16*)obj;
        targetPitch = targetPitch - (u16)obj->anim.rotY;
        if (targetPitch > 0x8000)
        {
            targetPitch = targetPitch - 0xffff;
        }
        if (targetPitch < -0x8000)
        {
            targetPitch = targetPitch + 0xffff;
        }
        step = (f32)targetPitch * timeDelta;
        obj->anim.rotY = step * 0.0625f + (f32)obj->anim.rotY;
    }
    Obj_TransformWorldPointToLocal(obj->anim.worldPosX, obj->anim.worldPosY,
                                   obj->anim.worldPosZ,
                                   &obj->anim.localPosX, &obj->anim.localPosY,
                                   &obj->anim.localPosZ,
                                   *(int*)&obj->anim.parent);
}
void CameraModeArwing_init(GameObject* obj, int mode, int unused)
{
    GameObject* a4 = ((GameObject**)obj)[0xA4 / 4];
    char* base;
    f32* p;
    f32 fc2;
    f32 fc;
    if (mode != 1)
    {
        ((CameraArwingWork*)gCamArwingWork)->basePosX = a4->anim.worldPosX;
        ((CameraArwingWork*)gCamArwingWork)->basePosY = a4->anim.worldPosY;
        ((CameraArwingWork*)gCamArwingWork)->basePosZ = a4->anim.worldPosZ;
    }
    *(p = (f32*)((base = (char*)gCamArwingWork) + 48)) = 0.0f;
    *(f32*)(base + 52) = 20.0f;
    *(f32*)(base + 56) = -165.0f;
    PSVECAdd(&a4->anim.worldPosX, p, &obj->anim.worldPosX);
    ((CameraArwingWork*)gCamArwingWork)->active = 1;
    ((CameraArwingWork*)gCamArwingWork)->yawScale = -0.2f;
    ((CameraArwingWork*)gCamArwingWork)->pitchScale = 0.1f;
    ((CameraArwingWork*)gCamArwingWork)->rollScale = 0.3f;
    ((CameraArwingWork*)gCamArwingWork)->xScale = 0.8f;
    ((CameraArwingWork*)gCamArwingWork)->yScale = 0.65f;
    fc = 0.0f;
    ((CameraArwingWork*)gCamArwingWork)->unk2C = fc;
    fc2 = 13.0f;
    ((CameraArwingWork*)gCamArwingWork)->zEaseNum = fc2;
    ((CameraArwingWork*)gCamArwingWork)->zEaseDenom = fc2;
    ((CameraArwingWork*)gCamArwingWork)->zScaleFar = 90;
    ((CameraArwingWork*)gCamArwingWork)->zScaleNear = 100;
    ((CameraArwingWork*)gCamArwingWork)->offsetZ = fc;
    ((CameraArwingWork*)gCamArwingWork)->offsetY = fc;
    ((CameraArwingWork*)gCamArwingWork)->offsetX = fc;
    obj->anim.worldPosX = a4->anim.worldPosX;
    obj->anim.worldPosY = a4->anim.worldPosY;
    obj->anim.worldPosZ = a4->anim.worldPosZ + *(f32*)(base + 56);
}

void CameraModeArwing_release(void)
{
}


void CameraModeArwing_initialise(void)
{
}

ResourceDescriptorCallbacks8 lbl_80319F88 = {{0x00000000, 0x00000000, 0x00000000, 0x00060000},
        {(ResourceDescriptorCallback)CameraModeArwing_initialise, (ResourceDescriptorCallback)CameraModeArwing_release,
        0x00000000, (ResourceDescriptorCallback)CameraModeArwing_init, (ResourceDescriptorCallback)CameraModeArwing_update,
        (ResourceDescriptorCallback)CameraModeArwing_free, (ResourceDescriptorCallback)CameraModeArwing_copyToCurrent, 0x00000000}};
