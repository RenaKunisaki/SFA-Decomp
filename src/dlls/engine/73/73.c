/*
 * DLL 73 / 0x49 - combat camera mode.
 */
#include "main/camera_interface.h"
#include "main/resource.h"
#include "dolphin/mtx/mtx_legacy.h"
#include "main/camera.h"
#include "main/dll/CAM/camcombat_state.h"
#include "main/dll/CAM/cutCam.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/frame_timing.h"

s32 gCamCombatPrevYawDiff;
CameraModeCombatState* gCamCombatState;

#define CAMCOMBAT_SEQID_DIM_BOSS 0x200 /* retail "DIM_Boss" (DLL 0x1E0) */
#define CAMERAMODECOMBAT_CAMACTION_DEFAULT 0x42
#define PAD_BUTTON_B 0x200

typedef struct {
    u8 b0 : 1;
    u8 b1 : 1;
    u8 b2 : 1;
    u8 b3 : 1;
    u8 b4 : 1;
    u8 b5 : 1;
    u8 b6 : 1;
    u8 b7 : 1;
} CameraModeCombatFlags;

void camCombatIntroEvalPos(CameraObject* camera, float* outX, float* outY, float* outZ, f32* targetY)
{
    GameObject* target;
    ObjHitVolumeRuntimeTransform* hitVolumes;
    GameObject* focus;
    u8 curIdx;
    float lim;
    float t;

    target = (GameObject*)camera->targetObj;
    focus = (GameObject*)camera->anim.targetObj;
    hitVolumes = target->anim.hitVolumeTransforms;
    curIdx = target->hitVolumeIndex;
    if ((u32)curIdx != gCamCombatState->pathBlendTargetIndex)
    {
        gCamCombatState->pathBlendStartIndex = gCamCombatState->pathBlendTargetIndex;
        gCamCombatState->pathBlendWeight = 1.0f;
    }
    t = gCamCombatState->pathBlendWeight;
    lim = 0.0f;
    if (t > lim)
    {
        gCamCombatState->pathBlendWeight = t - 0.02f * timeDelta;
        t = gCamCombatState->pathBlendWeight;
        if (gCamCombatState->pathBlendWeight < lim)
        {
            gCamCombatState->pathBlendWeight = lim;
            gCamCombatState->pathBlendStartIndex = target->hitVolumeIndex;
        }
        {
            u8 ci = gCamCombatState->pathBlendStartIndex;
            float w;
            float dx = hitVolumes[ci].centerX - hitVolumes[target->hitVolumeIndex].centerX;
            float dy = hitVolumes[ci].centerY - hitVolumes[target->hitVolumeIndex].centerY;
            float dz = hitVolumes[ci].centerZ - hitVolumes[target->hitVolumeIndex].centerZ;
            w = gCamCombatState->pathBlendWeight;
            dx *= w;
            dy *= w;
            dz *= w;
            dx += hitVolumes[target->hitVolumeIndex].centerX;
            dy += hitVolumes[target->hitVolumeIndex].centerY;
            dz += hitVolumes[target->hitVolumeIndex].centerZ;
            *outX = dx - focus->anim.worldPosX;
            *outY = dy - *targetY;
            *outZ = dz - focus->anim.worldPosZ;
        }
    }
    else
    {
        *outX = hitVolumes[target->hitVolumeIndex].centerX - focus->anim.worldPosX;
        *outY = hitVolumes[target->hitVolumeIndex].centerY - *targetY;
        *outZ = hitVolumes[target->hitVolumeIndex].centerZ - focus->anim.worldPosZ;
    }
    gCamCombatState->pathBlendTargetIndex = target->hitVolumeIndex;
}

void CameraModeCombat_copyToCurrent(void)
{
}

void CameraModeCombat_free(CameraObject* camera)
{
    if (camera->targetObj != NULL)
    {
        (*gCameraInterface)->setTarget(0);
    }
    mm_free(gCamCombatState);
    gCamCombatState = 0;
    Rcp_DisableBlurFilter();
    ((CameraModeCombatFlags*)&camera->smoothingFlags)->b0 = 0;
}

static void camCombatTraceMove(f32* prevPos, CameraObject* camera, u8* trace)
{
    camcontrol_traceMove(prevPos, &camera->anim.worldPosX, &camera->anim.worldPosX, trace, 3, 1, 1, 4.0f);
}

void CameraModeCombat_update(CameraObject* cam)
{
    f32 vec[3];
    f32 prevZ;
    f32 prevY;
    f32 prevX;
    f32 dy;
    f32 ty;
    f32 dx;
    f32 dz;
    f32 n[3];
    u8 trace[116];
    CameraViewSlot* view = Camera_GetCurrentViewSlot();
    GameObject* tgt;
    ObjHitVolumeRuntimeTransform* hitVolumes;
    GameObject* focus;
    f32 dist;
    f32 px;
    f32 py;
    f32 pz;
    f32 range;
    f32 step;
    f32 zoom;
    f32 mag;
    f32 speed;
    f32 lim;
    f32 sinAngle;
    f32 cosAngle;
    f32 t;
    f32 fa;
    f32 fb;
    int ang;
    int diff;
    u32 binAngleDelta;
    short classId;

    if (gCamCombatState->invalidTarget != 0)
    {
        if (cam->targetObj != NULL)
        {
            if (*(u8*)&((GameObject*)cam->targetObj)->anim.resetHitboxMode & 0x40)
            {
                return;
            }
            if (cam->targetFlags & 2)
            {
                return;
            }
            (*gCameraInterface)->setTarget(0);
        }
        (*gCameraInterface)->setMode(CAMERAMODECOMBAT_CAMACTION_DEFAULT, 0, 1, 0, NULL, 0x1e, 0xff);
    }
    else
    {
        focus = (GameObject*)cam->anim.targetObj;
        if (focus->anim.classId == 1 && objAnimFn_80296328(focus) == 0)
        {
            if (cam->targetObj != NULL)
            {
                if (*(u8*)&((GameObject*)cam->targetObj)->anim.resetHitboxMode & 0x40)
                {
                    return;
                }
                if (cam->targetFlags & 2)
                {
                    return;
                }
                (*gCameraInterface)->setTarget(0);
            }
            (*gCameraInterface)->setMode(CAMERAMODECOMBAT_CAMACTION_DEFAULT, 0, 1, 0, NULL, 0x1e, 0xff);
        }
        else
        {
            tgt = (GameObject*)cam->targetObj;
            if (tgt == NULL || (tgt->objectFlags & OBJECT_OBJFLAG_FREED) || (*(u8*)&tgt->anim.resetHitboxMode & 0x28))
            {
                if (tgt != NULL)
                {
                    if (*(u8*)&tgt->anim.resetHitboxMode & 0x40)
                    {
                        return;
                    }
                    if (cam->targetFlags & 2)
                    {
                        return;
                    }
                    (*gCameraInterface)->setTarget(0);
                }
                (*gCameraInterface)->setMode(CAMERAMODECOMBAT_CAMACTION_DEFAULT, 0, 1, 0, NULL, 0x1e, 0xff);
            }
            else
            {
                hitVolumes = tgt->anim.hitVolumeTransforms;
                if (hitVolumes != NULL)
                {
                    range = (f32)(s32)((u32)tgt->anim.modelInstance->hitVolumes[0].bounds[1] << 2);
                    if (((u16)getButtonsJustPressed(0) & PAD_BUTTON_B) && fn_8029630C(focus) != 0)
                    {
                        if (cam->targetObj != NULL)
                        {
                            if (*(u8*)&((GameObject*)cam->targetObj)->anim.resetHitboxMode & 0x40)
                            {
                                return;
                            }
                            if (cam->targetFlags & 2)
                            {
                                return;
                            }
                            (*gCameraInterface)->setTarget(0);
                        }
                        (*gCameraInterface)->setMode(CAMERAMODECOMBAT_CAMACTION_DEFAULT, 0, 1, 0, NULL, 0x1e, 0xff);
                    }
                    else
                    {
                        ty = 20.0f + focus->anim.worldPosY;
                        classId = tgt->anim.classId;
                        if (classId == 0x1c || classId == 0x6d || classId == 0x2a)
                        {
                            if (tgt->anim.seqId == CAMCOMBAT_SEQID_DIM_BOSS)
                            {
                                ty += 20.0f;
                            }
                            if (tgt->anim.modelInstance->hitVolumeCount > 1)
                            {
                                camCombatIntroEvalPos((CameraObject*)cam, &dx, &dy, &dz, &ty);
                            }
                            else
                            {
                                dx = hitVolumes[tgt->hitVolumeIndex].centerX - focus->anim.worldPosX;
                                dy = hitVolumes[tgt->hitVolumeIndex].centerY - ty;
                                dz = hitVolumes[tgt->hitVolumeIndex].centerZ - focus->anim.worldPosZ;
                            }
                        }
                        else
                        {
                            ty = 20.0f + focus->anim.worldPosY;
                            dx = hitVolumes[tgt->hitVolumeIndex].centerX - focus->anim.worldPosX;
                            dy = hitVolumes[tgt->hitVolumeIndex].centerY - ty;
                            dz = hitVolumes[tgt->hitVolumeIndex].centerZ - focus->anim.worldPosZ;
                        }
                        fa = dx * dx;
                        fb = dz * dz;
                        dist = sqrtf(fa + fb);
                        cam->letterboxTargetOffset = 0x30;
                        cam->letterboxStep = 1;
                        if (dist > range)
                        {
                            if (cam->targetObj != NULL)
                            {
                                if (*(u8*)&((GameObject*)cam->targetObj)->anim.resetHitboxMode & 0x40)
                                {
                                    return;
                                }
                                if (cam->targetFlags & 2)
                                {
                                    return;
                                }
                                (*gCameraInterface)->setTarget(0);
                            }
                            (*gCameraInterface)->setMode(CAMERAMODECOMBAT_CAMACTION_DEFAULT, 0, 1, 0, NULL, 0x1e, 0xff);
                        }
                        else
                        {
                            cameraGetPrevPos2(focus, &prevX, &prevY, &prevZ);
                            px = 0.35f * dx + focus->anim.worldPosX;
                            py = 5.0f + ty;
                            pz = 0.35f * dz + focus->anim.worldPosZ;
                            ang = getAngle(dx, dz);
                            binAngleDelta = (ang & 0xffff) + 0x8000;
                            diff = (int)cam->anim.rotX - ((0x8000 - binAngleDelta) & 0xffff);
                            if (diff > 0x8000)
                            {
                                diff = diff - 0xffff;
                            }
                            if (diff < -0x8000)
                            {
                                diff = diff + 0xffff;
                            }
                            if (diff > 9000)
                            {
                                step = interpolate((f32)(s32)(diff - 9000), 1.0f / 12.0f, timeDelta);
                                cam->anim.rotX = (s16)((f32)(s32)cam->anim.rotX - step);
                            }
                            else if (diff < -9000)
                            {
                                step = interpolate((f32)(s32)(diff + 9000), 1.0f / 12.0f, timeDelta);
                                cam->anim.rotX = (s16)((f32)(s32)cam->anim.rotX - step);
                            }
                            if (diff < 3000 && diff > 0)
                            {
                                if (gCamCombatPrevYawDiff < 3000 && diff < 1000 && gCamCombatPrevYawDiff > diff)
                                {
                                    step = interpolate((f32)(s32)(-diff - 3000), 0.0078125f, timeDelta);
                                    cam->anim.rotX = (s16)((f32)(s32)cam->anim.rotX + step);
                                }
                                else
                                {
                                    step = interpolate((f32)(s32)(3000 - diff), 0.0078125f, timeDelta);
                                    cam->anim.rotX = (s16)((f32)(s32)cam->anim.rotX + step);
                                }
                            }
                            else if (diff > -3000 && diff < 0)
                            {
                                if (gCamCombatPrevYawDiff > -3000 && diff > -1000 && gCamCombatPrevYawDiff < diff)
                                {
                                    step = interpolate((f32)(s32)(3000 - diff), 0.0078125f, timeDelta);
                                    cam->anim.rotX = (s16)((f32)(s32)cam->anim.rotX + step);
                                }
                                else
                                {
                                    step = interpolate((f32)(s32)(-diff - 3000), 0.0078125f, timeDelta);
                                    cam->anim.rotX = (s16)((f32)(s32)cam->anim.rotX + step);
                                }
                            }
                            gCamCombatPrevYawDiff = diff;
                            if (diff < 0)
                            {
                                diff = -diff;
                            }
                            if (diff > 9000)
                            {
                                diff = 9000;
                            }
                            step = (f32)(s32)(9000 - diff);
                            zoom = step / 9000.0f;
                            step = interpolate(35.0f - gCamCombatState->heightOffset, 0.04f, timeDelta);
                            gCamCombatState->heightOffset = gCamCombatState->heightOffset + step;
                            fb = 1.0f - zoom;
                            fb = 0.8f + fb;
                            step = interpolate(
                                fb / 1.8f - gCamCombatState->zoomOffset,
                                0.1f, timeDelta);
                            gCamCombatState->zoomOffset = gCamCombatState->zoomOffset + step;
                            sinAngle = mathSinf((3.1415927f * (f32)(s32)cam->anim.rotX) / 32768.0f);
                            cosAngle = mathCosf((3.1415927f * (f32)(s32)cam->anim.rotX) / 32768.0f);
                            t = gCamCombatState->followDistance * sinAngle;
                            n[0] = px + t;
                            t = gCamCombatState->followDistance * cosAngle;
                            n[2] = pz - t;
                            dy *= 0.6f;
                            dy = ty - dy;
                            dy = dy + gCamCombatState->heightOffset;
                            step = interpolate(cam->anim.worldPosY - dy, 0.05f, timeDelta);
                            n[1] = cam->anim.worldPosY - step;
                            PSVECSubtract(n, &cam->anim.worldPosX, vec);
                            mag = PSVECMag(vec);
                            if (mag > 0.0f)
                            {
                                PSVECNormalize(vec, vec);
                            }
                            if (cam->blendProgress <= 0.0f)
                            {
                                fa = focus->anim.previousWorldPosX - focus->anim.worldPosX;
                                fb = focus->anim.previousWorldPosZ - focus->anim.worldPosZ;
                                speed = sqrtf(fa * fa + fb * fb);
                                lim = speed * (3.0f * timeDelta);
                                if ((f64)lim < 0.5)
                                {
                                    lim = 0.5f;
                                }
                                if (mag < 0.0f)
                                {
                                    mag = 0.0f;
                                }
                                else if (mag > lim)
                                {
                                    mag = lim;
                                }
                            }
                            PSVECScale(vec, vec,
                                       (mag < 0.0f)
                                           ? 0.0f
                                           : ((mag > 20.0f) ? 20.0f : mag));
                            PSVECAdd(&cam->anim.worldPosX, vec, &cam->anim.worldPosX);
                            camCombatTraceMove(&prevX, (CameraObject*)cam, trace);
                            t = 0.1f * dz + focus->anim.worldPosZ;
                            fb = view->x - (0.1f * dx + focus->anim.worldPosX);
                            dy = view->y - py;
                            fa = view->z - t;
                            t = sqrtf(fb * fb + fa * fa);
                            ang = getAngle(dy, t) & 0xffff;
                            binAngleDelta = ang - ((int)cam->anim.rotY & 0xffffU);
                            if ((int)binAngleDelta > 0x8000)
                            {
                                binAngleDelta = binAngleDelta - 0xffff;
                            }
                            if ((int)binAngleDelta < -0x8000)
                            {
                                binAngleDelta = binAngleDelta + 0xffff;
                            }
                            step = interpolate((f32)(s32)binAngleDelta, 0.125f, timeDelta);
                            cam->anim.rotY = (s16)
                            ((f32)(s32)
                            cam->anim.rotY + step
                            )
                            ;
                            fa = 10.0f + dist;
                            if (fa < 70.0f)
                            {
                                fa = 70.0f;
                            }
                            if (fa > 150.0f)
                            {
                                fa = 150.0f;
                            }
                            fa = fa - gCamCombatState->followDistance;
                            step = powfBitEstimate(0.04f, timeDelta);
                            fa = fa * step;
                            if (fa > 5.0f * timeDelta)
                            {
                                fa = 5.0f * timeDelta;
                            }
                            else if (fa < -5.0f * timeDelta)
                            {
                                fa = -5.0f * timeDelta;
                            }
                            gCamCombatState->followDistance = gCamCombatState->followDistance + fa;
                            turnOnBlurFilter(tgt->anim.worldPosX, tgt->anim.worldPosY, tgt->anim.worldPosZ, 1, 0);
                            if (0.0f == cam->blendProgress)
                            {
                                ((struct { u8 b7 : 1; } *)&cam->smoothingFlags)->b7 = 1;
                            }
                            Obj_TransformWorldPointToLocal(cam->anim.worldPosX,
                                                           cam->anim.worldPosY,
                                                           cam->anim.worldPosZ,
                                                           &cam->anim.localPosX,
                                                           &cam->anim.localPosY,
                                                           &cam->anim.localPosZ,
                                                           *(int*)&cam->anim.parent);
                        }
                    }
                }
            }
        }
    }
}

void CameraModeCombat_init(CameraObject* camera, u32 unused, GameObject** targetPtr)
{
    float dx;
    float dz;
    ObjHitVolumeRuntimeTransform* hitVolume;
    GameObject* target;
    GameObject* focus;

    camera->targetObj = *targetPtr;
    focus = (GameObject*)camera->anim.targetObj;
    if (gCamCombatState == (CameraModeCombatState*)0x0)
    {
        gCamCombatState = (CameraModeCombatState*)mmAlloc(0x1c, 0xf, 0);
    }
    dx = 0.0f;
    gCamCombatState->heightOffset = 0.0f;
    gCamCombatState->zoomOffset = 1.0f;
    gCamCombatState->invalidTarget = 0;
    gCamCombatState->unk11 = 0;
    gCamCombatState->pathBlendStartIndex = 1;
    gCamCombatState->pathBlendTargetIndex = 1;
    gCamCombatState->pathBlendWeight = dx;
    if (focus->anim.classId != 1)
    {
        gCamCombatState->invalidTarget = 1;
    }
    else
    {
        target = (GameObject*)camera->targetObj;
        if (target == NULL)
        {
            gCamCombatState->invalidTarget = 1;
        }
        else
        {
            if (target->anim.hitVolumeTransforms == NULL)
            {
                dx = focus->anim.worldPosX - target->anim.worldPosX;
                dz = focus->anim.worldPosZ - target->anim.worldPosZ;
            }
            else
            {
                hitVolume = &target->anim.hitVolumeTransforms[target->hitVolumeIndex];
                dx = hitVolume->centerX - focus->anim.worldPosX;
                dz = hitVolume->centerZ - focus->anim.worldPosZ;
            }
            if (target->anim.classId != 0x6d)
            {
                gCamCombatState->followDistance = sqrtf(dx * dx + dz * dz);
            }
            else
            {
                gCamCombatState->followDistance = 200.0f;
            }
            gCamCombatState->unk10 = 0;
        }
    }
    return;
}

void CameraModeCombat_release(void)
{
}

void CameraModeCombat_initialise(void)
{
}


ResourceDescriptorCallbacks8 lbl_80319CE8 = {{0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00060000},
                       {(ResourceDescriptorCallback)CameraModeCombat_initialise,
                        (ResourceDescriptorCallback)CameraModeCombat_release,
                        0x00000000,
                        (ResourceDescriptorCallback)CameraModeCombat_init,
                        (ResourceDescriptorCallback)CameraModeCombat_update,
                        (ResourceDescriptorCallback)CameraModeCombat_free,
                        (ResourceDescriptorCallback)CameraModeCombat_copyToCurrent,
                        0x00000000}};
