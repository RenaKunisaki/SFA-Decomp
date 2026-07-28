/*
 * DLL 84 / 0x54 - NPC conversation camera mode.
 */
#include "main/mm.h"
#include "main/resource.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/camera_object.h"
#include "main/camera_interface.h"
#include "main/dll/CAM/camera_mode_54_state.h"
#include "sys/objects.h"
#include "main/frame_timing.h"
#include "main/vecmath.h"
#include "string.h"
#include "main/object_transform.h"
#include "main/obj_list.h"
#include "main/dll/dll_0054_dll54.h"

CameraMode54State* gCameraMode54State;

#define DLL54_CAMMODE_DEFAULT 0x42

#define DLL54_LOOKAT_SEQID 0x2ab
#define DLL54_ORIGIN_SEQID 0x4dc


void dll_54_func06_nop(void)
{
}

void dll_54_func05(void)
{
    mm_free((void*)gCameraMode54State);
    gCameraMode54State = NULL;
}

void dll_54_update(CameraObject* camera)
{
    int i;
    int count;
    f32 zz, xx;
    f32 dx, dy, dz;
    f32 dist;
    f32 nx, nz;
    f32 fx, fz;
    f32 d2, h, t;
    f32 t2;
    f32 lim;
    s16 cur;
    s16 angleDelta;

    if (gCameraMode54State->exitRequested != 0)
    {
        (*gCameraInterface)->setMode(DLL54_CAMMODE_DEFAULT, 0, 1, 0, NULL, 0, 0xff);
    }
    else
    {
        if (gCameraMode54State->lookAtObj == NULL)
        {
            int* arr = (int*)ObjList_GetObjects(&i, &count);
            for (; i < count; i++)
            {
                GameObject* o = (GameObject*)arr[i];
                if (o->anim.seqId == DLL54_LOOKAT_SEQID)
                {
                    gCameraMode54State->lookAtObj = o;
                }
                else if (o->anim.seqId == DLL54_ORIGIN_SEQID)
                {
                    gCameraMode54State->originObj = o;
                }
            }
        }
        if (gCameraMode54State->playerObj == NULL)
        {
            gCameraMode54State->playerObj = (GameObject*)Obj_GetPlayerObject();
        }
        {
            GameObject* a = gCameraMode54State->lookAtObj;
            dx = a->anim.worldPosX - gCameraMode54State->originObj->anim.worldPosX;
            dy = a->anim.worldPosY - gCameraMode54State->originObj->anim.worldPosY;
            dz = a->anim.worldPosZ - gCameraMode54State->originObj->anim.worldPosZ;
        }
        zz = dz * dz;
        xx = dx * dx;
        dist = sqrtf(zz + (dy * dy + xx));
        nx = dx / dist;
        nz = dz / dist;
        fx = -(140.0f * nx - gCameraMode54State->originObj->anim.worldPosX) -
             gCameraMode54State->playerObj->anim.worldPosX;
        fz = -(140.0f * nz - gCameraMode54State->originObj->anim.worldPosZ) -
             gCameraMode54State->playerObj->anim.worldPosZ;
        d2 = sqrtf(fx * fx + fz * fz);
        t = (200.0f - d2) / 200.0f;
        camera->fov = 45.0f + 70.0f * t;
        h = -30.0f + 350.0f * t;
        camera->anim.worldPosX = -(nx * h - gCameraMode54State->originObj->anim.worldPosX);
        camera->anim.worldPosY = (20.0f + gCameraMode54State->originObj->anim.worldPosY) + 60.0f * t;
        camera->anim.worldPosZ = -(nz * h - gCameraMode54State->originObj->anim.worldPosZ);
        camera->anim.rotX = -getAngle(dx, dz);
        camera->anim.rotY = -getAngle(-(100.0f * (dist / 400.0f) - dy), sqrtf(xx + zz));

        if (gCameraMode54State->transitionDone == 0)
        {
            t2 = gCameraMode54State->transitionTimer / 60.0f;
            camera->anim.worldPosX =
                t2 * (gCameraMode54State->startX - camera->anim.worldPosX) + camera->anim.worldPosX;
            camera->anim.worldPosY =
                t2 * (gCameraMode54State->startY - camera->anim.worldPosY) + camera->anim.worldPosY;
            camera->anim.worldPosZ =
                t2 * (gCameraMode54State->startZ - camera->anim.worldPosZ) + camera->anim.worldPosZ;

            cur = camera->anim.rotX;
            angleDelta = (s16)(gCameraMode54State->startYaw - (u16)cur);
            if (angleDelta > 0x8000)
            {
                angleDelta = (s16)(angleDelta - 0xffff);
            }
            if (angleDelta < -0x8000)
            {
                angleDelta += 0xffff;
            }
            camera->anim.rotX = angleDelta * t2 + cur;

            cur = camera->anim.rotY;
            angleDelta = (s16)(gCameraMode54State->startPitch - (u16)cur);
            angleDelta = (angleDelta > 0x8000) ? (s16)(angleDelta - 0xffff) : angleDelta;
            angleDelta = (angleDelta < -0x8000) ? (s16)(angleDelta + 0xffff) : angleDelta;
            camera->anim.rotY = angleDelta * t2 + cur;

            gCameraMode54State->transitionTimer -= timeDelta;
            if (gCameraMode54State->transitionTimer < 0.0f)
            {
                gCameraMode54State->transitionDone = 1;
                gCameraMode54State->transitionTimer = 0.0f;
            }
        }
        Obj_TransformWorldPointToLocal(camera->anim.worldPosX, camera->anim.worldPosY, camera->anim.worldPosZ,
                                       &camera->anim.localPosX, &camera->anim.localPosY, &camera->anim.localPosZ,
                                       *(int*)&camera->anim.parent);
    }
}

void dll_54_init(CameraObject* camera, int unused, CameraObject* source)
{

    if (gCameraMode54State == NULL)
    {
        gCameraMode54State = (CameraMode54State*)mmAlloc(sizeof(CameraMode54State), 15, 0);
    }
    memset(gCameraMode54State, 0, sizeof(CameraMode54State));
    gCameraMode54State->transitionTimer = 60.0f;
    gCameraMode54State->transitionDone = 0;
    if (source != NULL)
    {
        camera->anim.localPosX = source->anim.worldPosX;
        camera->anim.localPosY = source->anim.worldPosY;
        camera->anim.localPosZ = source->anim.worldPosZ;
        camera->anim.rotX = source->anim.rotX;
        camera->anim.rotY = source->anim.rotY;
        camera->anim.rotZ = source->anim.rotZ;
        camera->fov = source->fov;
    }
    gCameraMode54State->startX = camera->anim.worldPosX;
    gCameraMode54State->startY = camera->anim.worldPosY;
    gCameraMode54State->startZ = camera->anim.worldPosZ;
    gCameraMode54State->startYaw = camera->anim.rotX;
    gCameraMode54State->startPitch = camera->anim.rotY;
    gCameraMode54State->startRoll = camera->anim.rotZ;
}

void dll_54_release_nop(void)
{
}

void dll_54_initialise_nop(void)
{
}

ResourceDescriptorCallbacks8 dll_54 = {{0x00000000, 0x00000000, 0x00000000, 0x00060000},
        {(ResourceDescriptorCallback)dll_54_initialise_nop, (ResourceDescriptorCallback)dll_54_release_nop,
        0x00000000, (ResourceDescriptorCallback)dll_54_init, (ResourceDescriptorCallback)dll_54_update,
        (ResourceDescriptorCallback)dll_54_func05, (ResourceDescriptorCallback)dll_54_func06_nop, 0x00000000}};
