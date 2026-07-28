/*
 * DLL 68 / 0x44 - viewfinder camera mode.
 */
#include "main/audio/sfx.h"
#include "main/debug.h"
#include "main/camera_interface.h"
#include "main/dll/CAM/dll_0045_camTalk.h"
#include "main/dll/CAM/viewfinder_state.h"
#include "main/dll/dll_0044_cameramodeviewfinder.h"
#include "dlls/objects/488_SB_Galleon.h"
#include "main/dll/CAM/cambike_state.h"
#include "main/dll/viewfinder.h"
#include "main/gamebits.h"
#include "main/mm.h"
#include "main/object_transform.h"
#include "sys/objects.h"
#include "main/objhits.h"
#include "main/pad.h"
#include "main/dll/player_motion.h"
#include "main/dll/player_objects.h"
#include "main/rcp_dolphin.h"
#include "main/camera.h"
#include "main/vecmath.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/frame_timing.h"
#include "main/dll/CAM/dll_0001_camcontrol.h"
#include "string.h"
#include "main/dll/CAM/cutCam.h"
#include "main/dll/player_api.h"
#include "main/resource.h"
#include "dolphin/pad.h"

ViewfinderState* gViewfinderState;

char sCam5BYDebugFormat[] = "y=%f\n";

typedef struct CameraModeViewfinderInitArgs
{
    f32 radius;
    f32 yOffset;
    u16 height;
} CameraModeViewfinderInitArgs;

#define VIEWFINDER_CAMMODE_DEFAULT 0x42

#define VIEWFINDER_MODE_ENTER_BLEND 0
#define VIEWFINDER_MODE_YAW_SETTLE  1
#define VIEWFINDER_MODE_ACTIVE      2
#define VIEWFINDER_MODE_EXIT_BLEND  3
#define VIEWFINDER_MODE_FADE_BACK   4
#define VIEWFINDER_MODE_IDLE        5



extern f32 lbl_803E17C0;
extern f32 lbl_803E17D0;
extern f32 lbl_803E1804;
extern f32 gCamViewfinderBrightnessScale;
extern f32 gCamViewfinderPi;

void firstPersonPlaceCamera(GameObject* focus, int resetClamp)
{
    register GameObject* self = focus;
    GameObject* galleon;
    int galleonState;
    float prevPosZ;
    float prevPosY;
    float prevPosX;
    float localOffset[3];

    if (self->anim.classId == 1)
    {
        cameraGetPrevPos2(self, &prevPosX, &prevPosY, &prevPosZ);
        if (((resetClamp != 0) || (gViewfinderState->camPosX != prevPosX)) || (gViewfinderState->camPosZ != prevPosZ))
        {
            gViewfinderState->clampedPosY = prevPosY;
        }
        gViewfinderState->camPosX = prevPosX;
        gViewfinderState->camPosY = prevPosY;
        gViewfinderState->camPosZ = prevPosZ;
    }
    else
    {
        gViewfinderState->camPosX = self->anim.worldPosX;
        gViewfinderState->camPosY = 35.0f + self->anim.worldPosY;
        gViewfinderState->camPosZ = self->anim.worldPosZ;
        gViewfinderState->clampedPosY = gViewfinderState->camPosY;
    }
    galleon = getSbGalleon();
    if (galleon != NULL)
    {
        galleonState = DBprotection_getCameraState(galleon);
        if (galleonState == 2)
        {
            localOffset[0] = self->anim.worldPosX - galleon->anim.worldPosX;
            localOffset[1] = (35.0f + self->anim.worldPosY) - galleon->anim.worldPosY;
            localOffset[2] = self->anim.worldPosZ - galleon->anim.worldPosZ;
            vecRotateZXY(&galleon->anim.rotX, localOffset);
            gViewfinderState->camPosX = galleon->anim.worldPosX + localOffset[0];
            gViewfinderState->camPosY = galleon->anim.worldPosY + localOffset[1];
            gViewfinderState->camPosZ = galleon->anim.worldPosZ + localOffset[2];
        }
    }
    return;
}

void firstPersonExit(CameraObject* camera)
{
    register CameraObject* self = camera;
    GameObject* target;
    float tangent;
    float dx;
    float dz;
    int targetYaw;
    float targetPos[3];
    u8 unusedAngle[4];

    target = (GameObject*)self->anim.targetObj;
    gViewfinderState->posXCurve.start = self->anim.worldPosX;
    tangent = 0.0f;
    gViewfinderState->posXCurve.startTangent = 0.0f;
    gViewfinderState->posXCurve.endTangent = tangent;
    gViewfinderState->posYCurve.start = self->anim.worldPosY;
    gViewfinderState->posYCurve.startTangent = tangent;
    gViewfinderState->posYCurve.endTangent = tangent;
    gViewfinderState->posZCurve.start = self->anim.worldPosZ;
    gViewfinderState->posZCurve.startTangent = tangent;
    gViewfinderState->posZCurve.endTangent = tangent;
    camcontrol_getTargetPosition(self, &target->anim, targetPos, (s16*)unusedAngle);
    gViewfinderState->posXCurve.end = targetPos[0];
    gViewfinderState->posYCurve.end = targetPos[1];
    gViewfinderState->posZCurve.end = targetPos[2];
    dx = gViewfinderState->posXCurve.end - gViewfinderState->posXCurve.start;
    dz = gViewfinderState->posZCurve.end - gViewfinderState->posZCurve.start;
    gViewfinderState->exitDistance = sqrtf(dx * dx + dz * dz);
    gViewfinderState->viewCurve.px = &gViewfinderState->yawCurve.start;
    gViewfinderState->viewCurve.py = &gViewfinderState->pitchCurve.start;
    gViewfinderState->viewCurve.pz = NULL;
    gViewfinderState->viewCurve.count = 4;
    gViewfinderState->viewCurve.dir = 0;
    gViewfinderState->viewCurve.eval = Curve_EvalHermite;
    gViewfinderState->viewCurve.coeffFn = Curve_BuildHermiteCoeffs;
    gViewfinderState->yawCurve.start = (float)(int)self->anim.rotX;
    targetYaw = getAngle((double)(gViewfinderState->posXCurve.end - target->anim.worldPosX),
                         (double)(gViewfinderState->posZCurve.end - target->anim.worldPosZ));
    gViewfinderState->yawCurve.end = (float)(int)(short)(0x8000 - targetYaw);
    tangent = 0.0f;
    gViewfinderState->yawCurve.startTangent = 0.0f;
    gViewfinderState->yawCurve.endTangent = tangent;
    if (((gViewfinderState->yawCurve.start - gViewfinderState->yawCurve.end) > 32768.0f) ||
        ((gViewfinderState->yawCurve.start - gViewfinderState->yawCurve.end) < -32768.0f))
    {
        if (gViewfinderState->yawCurve.start < 0.0f)
        {
            gViewfinderState->yawCurve.start = *(f32*)&gViewfinderState->yawCurve.start + lbl_803E17D0;
        }
        else
        {
            if (gViewfinderState->yawCurve.end < 0.0f)
            {
                gViewfinderState->yawCurve.end = *(f32*)&gViewfinderState->yawCurve.end + lbl_803E17D0;
            }
        }
    }
    gViewfinderState->pitchCurve.start = (float)(int)self->anim.rotY;
    tangent = 0.0f;
    gViewfinderState->pitchCurve.end = 0.0f;
    gViewfinderState->pitchCurve.startTangent = tangent;
    gViewfinderState->pitchCurve.endTangent = tangent;
    if (((gViewfinderState->pitchCurve.start - gViewfinderState->pitchCurve.end) > 32768.0f) ||
        ((gViewfinderState->pitchCurve.start - gViewfinderState->pitchCurve.end) < -32768.0f))
    {
        if (gViewfinderState->pitchCurve.start < 0.0f)
        {
            gViewfinderState->pitchCurve.start = *(f32*)&gViewfinderState->pitchCurve.start + lbl_803E17D0;
        }
        else
        {
            if (gViewfinderState->pitchCurve.end < 0.0f)
            {
                gViewfinderState->pitchCurve.end = *(f32*)&gViewfinderState->pitchCurve.end + lbl_803E17D0;
            }
        }
    }
    curvesMove(&gViewfinderState->viewCurve);
}
void firstPersonDoControls(CameraObject* obj)
{
    short pitchDelta;
    s8 stickX;
    s8 stickY;
    short* camObj;
    int spinI;
    f32 t;
    f32 zoom;
    f32 spin;
    f32 fovTarget;
    f32 zoom2;

    camObj = (short*)obj->anim.targetObj;
    stickX = padGetStickX(0);
    stickY = padGetStickY(0);
    t = (60.0f - obj->fov) / 50.0f;
    zoom = (t < 0.0f) ? 0.0f : ((t > 1.0f) ? 1.0f : t);
    spin = stickX * -(4.0f * zoom - 6.0f);
    spin = interpolate(spin - gViewfinderState->yawSpeed, 0.12f, timeDelta);
    gViewfinderState->yawSpeed = gViewfinderState->yawSpeed + spin;
    if ((gViewfinderState->yawSpeed > -5.0f) && (gViewfinderState->yawSpeed < 5.0f))
    {
        gViewfinderState->yawSpeed = 0.0f;
    }
    spinI = (int)(15360.0f * ((f32)stickY / lbl_803E1804));
    obj->anim.rotX = gViewfinderState->yawSpeed * timeDelta + (f32)obj->anim.rotX;
    pitchDelta = spinI - (obj->anim.rotY & 0xffffU);
    if (0x8000 < pitchDelta)
    {
        pitchDelta = pitchDelta - 0xffff;
    }
    if (pitchDelta < -0x8000)
    {
        pitchDelta = pitchDelta + 0xffff;
    }
    spin = interpolate((f32)pitchDelta, 1.0f / (32.0f * zoom + 16.0f), timeDelta);
    obj->anim.rotY = obj->anim.rotY + spin;
    if (0x3c00 < obj->anim.rotY)
    {
        obj->anim.rotY = 0x3c00;
    }
    if (obj->anim.rotY < -0x3c00)
    {
        obj->anim.rotY = -0x3c00;
    }
    *camObj = 0x8000 - obj->anim.rotX;
    if (camObj[0x22] == 1)
    {
        Player_SetHeading((int)camObj, *camObj);
    }
    if (gViewfinderState->camPosY < gViewfinderState->clampedPosY)
    {
        gViewfinderState->clampedPosY = gViewfinderState->camPosY;
    }
    obj->anim.worldPosX = gViewfinderState->camPosX;
    obj->anim.worldPosY = gViewfinderState->clampedPosY;
    obj->anim.worldPosZ = gViewfinderState->camPosZ;
    if (gViewfinderState->flags.zoomHudEnabled)
    {
        zoom2 = obj->fov;
        stickX = padGetCY(0);
        t = (f32)-stickX;
        t = 0.01f * t;
        zoom2 = t * timeDelta + zoom2;
        viewFinderSetZoom(Camera_GetFovY());
        fovTarget = (zoom2 < 5.0f) ? 5.0f : ((zoom2 > 60.0f) ? 60.0f : zoom2);
        if (gViewfinderState->flags.sfxEnabled)
        {
            if ((fovTarget == obj->fov) && (gViewfinderState->flags.zoomSfxPlaying))
            {
                Sfx_StopFromObject(0, SFXTRIG_and_swipe1);
                gViewfinderState->flags.zoomSfxPlaying = 0;
            }
            if ((fovTarget != obj->fov) && (!gViewfinderState->flags.zoomSfxPlaying))
            {
                Sfx_PlayFromObject(0, SFXTRIG_and_swipe1);
                gViewfinderState->flags.zoomSfxPlaying = 1;
            }
        }
        obj->fov = fovTarget;
    }
}

int firstPersonEnter(CameraObject* cam, s16* p2)
{
    f32 f2;
    f32 start;
    f32 end;
    GameObject* state;
    int conv;
    int flag;
    GameObject* other;

    cam->anim.worldPosX = gViewfinderState->camPosX;
    cam->anim.worldPosY = gViewfinderState->camPosY;
    cam->anim.worldPosZ = gViewfinderState->camPosZ;
    cam->anim.rotY = 0;
    flag = 0;
    if (cam->blendProgress <= 0.0f)
    {
        flag = 1;
    }
    conv = (int)(gCamViewfinderBrightnessScale * cam->blendProgress);
    state = (GameObject*)cam->anim.targetObj;
    if (conv < 1)
    {
        conv = 1;
    }
    if (state != NULL)
    {
        state->anim.alpha = conv;
        if (Obj_GetPlayerObject() == state)
        {
            Player_GetHeldObject(state, &other);
            if (other != NULL)
            {
                other->anim.alpha = conv;
                if (other->anim.alpha == 1)
                {
                    other->anim.alpha = 0;
                }
            }
        }
    }
    if (flag != 0)
    {
        gViewfinderState->viewCurve.px = &gViewfinderState->yawCurve.start;
        gViewfinderState->viewCurve.py = NULL;
        gViewfinderState->viewCurve.pz = NULL;
        gViewfinderState->viewCurve.count = 4;
        gViewfinderState->viewCurve.eval = Curve_EvalHermite;
        gViewfinderState->viewCurve.coeffFn = Curve_BuildHermiteCoeffs;
        gViewfinderState->viewCurve.dir = 0;
        gViewfinderState->yawCurve.start = (f32)(s32)cam->anim.rotX;
        gViewfinderState->yawCurve.end = (f32)(s16)(0x8000 - p2[0]);
        start = gViewfinderState->yawCurve.start;
        end = gViewfinderState->yawCurve.end;
        f2 = start - end;
        if (f2 < 1820.0f && f2 > -1820.0f)
        {
            gViewfinderState->yawCurve.end = gViewfinderState->yawCurve.start;
        }
        else if (f2 > 32768.0f || f2 < -32768.0f)
        {
            if (start < 0.0f)
            {
                gViewfinderState->yawCurve.start += 65535.0f;
            }
            else if (end < 0.0f)
            {
                gViewfinderState->yawCurve.end += 65535.0f;
            }
        }
        {
            f32 k = 0.0f;
            gViewfinderState->yawCurve.startTangent = k;
            gViewfinderState->yawCurve.endTangent = k;
        }
        curvesMove(&gViewfinderState->viewCurve);
        return 1;
    }
    return 0;
}

void CameraModeViewfinder_copyToCurrent(s16* camObj)
{
    u8* src = (u8*)camObj;
    u8* cur;

    cur = (u8*)(*gCameraInterface)->getCamera();
    if ((cur != NULL) && (src != NULL))
    {
        *(s16*)(cur + 0) = *(s16*)(src + 0);
        *(s16*)(cur + 2) = *(s16*)(src + 2);
        *(s16*)(cur + 4) = *(s16*)(src + 4);
        *(f32*)(cur + 12) = *(f32*)(src + 8);
        *(f32*)(cur + 16) = *(f32*)(src + 12);
        *(f32*)(cur + 20) = *(f32*)(src + 16);
        *(f32*)(cur + 24) = *(f32*)(src + 8);
        *(f32*)(cur + 28) = *(f32*)(src + 12);
        *(f32*)(cur + 32) = *(f32*)(src + 16);
        *(f32*)(cur + 180) = *(f32*)(src + 20);
    }
}

void CameraModeViewfinder_free(int camObj)
{
    GameObject* player;
    GameObject* viewObj;
    GameObject* outBuf[3];

    ((GameObject*)((GameObject*)camObj)->anim.targetObj)->anim.flags &= ~OBJANIM_FLAG_HIDDEN;
    Rcp_SetViewFinderHudEnabled(0);
    viewObj = (GameObject*)((CameraObject*)camObj)->anim.targetObj;
    if (viewObj != NULL)
    {
        viewObj->anim.alpha = 0xff;
        player = Obj_GetPlayerObject();
        if (player == viewObj)
        {
            Player_GetHeldObject(viewObj, outBuf);
            if (outBuf[0] != NULL)
            {
                outBuf[0]->anim.alpha = 0xff;
                if (outBuf[0]->anim.alpha == 1)
                {
                    outBuf[0]->anim.alpha = 0;
                }
            }
        }
    }
    Sfx_StopFromObject(0, SFXTRIG_and_swipe1);
    mm_free(gViewfinderState);
    gViewfinderState = NULL;
    viewFinderSetZoom(60.0f);
}

void CameraModeViewfinder_update(CameraObject* obj)
{
    GameObject* camObj;
    GameObject* targetObj;
    GameObject* exitTarget;
    int angleDiff;
    f32 outA;
    f32 hitY;
    f32 outB;
    f32 hitDist;
    GameObject* shadow2;
    GameObject* shadow;

    camObj = (GameObject*)obj->anim.targetObj;
    getButtonsJustPressed(0);
    firstPersonPlaceCamera(camObj, 0);
    switch (gViewfinderState->mode)
    {
    case VIEWFINDER_MODE_ENTER_BLEND:
        gViewfinderState->mode = firstPersonEnter(obj, (s16*)obj->anim.targetObj);
        break;
    case VIEWFINDER_MODE_YAW_SETTLE:
        if (Curve_AdvanceAlongPath(&gViewfinderState->viewCurve, 1000.0f) != 0)
        {
            if (gViewfinderState->flags.zoomHudEnabled)
            {
                Rcp_SetViewFinderHudEnabled(1);
            }
            gViewfinderState->mode = VIEWFINDER_MODE_ACTIVE;
        }
        obj->anim.rotX = gViewfinderState->viewCurve.sample[0];
        obj->unk13E = 1;
        break;
    case VIEWFINDER_MODE_ACTIVE:
        if (gViewfinderState->flags.zoomHudEnabled)
        {
            Rcp_SetViewFinderHudEnabled(1);
        }
        firstPersonDoControls(obj);
        if (getButtonsJustPressed(0) & (PAD_BUTTON_B | PAD_TRIGGER_Z))
        {
            buttonDisable(0, PAD_BUTTON_B);
            firstPersonExit(obj);
            Rcp_SetViewFinderHudEnabled(0);
            gViewfinderState->mode = VIEWFINDER_MODE_EXIT_BLEND;
        }
        obj->unk13E = 0;
        break;
    case VIEWFINDER_MODE_EXIT_BLEND:
        angleDiff = Curve_AdvanceAlongPath(&gViewfinderState->viewCurve, 1000.0f);
        obj->anim.rotX = gViewfinderState->viewCurve.sample[0];
        obj->anim.rotY = gViewfinderState->viewCurve.sample[1];
        if (angleDiff != 0)
        {
            gViewfinderState->viewCurve.px = &gViewfinderState->posXCurve.start;
            gViewfinderState->viewCurve.py = &gViewfinderState->posYCurve.start;
            gViewfinderState->viewCurve.pz = &gViewfinderState->posZCurve.start;
            gViewfinderState->viewCurve.count = 4;
            gViewfinderState->viewCurve.dir = 0;
            gViewfinderState->viewCurve.eval = Curve_EvalHermite;
            gViewfinderState->viewCurve.coeffFn = Curve_BuildHermiteCoeffs;
            curvesMove(&gViewfinderState->viewCurve);
            ((GameObject*)obj->anim.targetObj)->anim.flags &= ~OBJANIM_FLAG_HIDDEN;
            firstPersonZoomOutOnExit(0xf, 0xfe);
            gViewfinderState->mode = VIEWFINDER_MODE_FADE_BACK;
            if (gViewfinderState->flags.sfxEnabled)
            {
                Sfx_PlayFromObject(0, gViewfinderState->flags.zoomHudEnabled ? SFXTRIG_and_missilelaunch
                                                                         : SFXTRIG_shop_pricedown);
            }
        }
        obj->unk13E = 1;
        break;
    case VIEWFINDER_MODE_FADE_BACK:
    {
        int brightness;

        obj->anim.worldPosX = gViewfinderState->posXCurve.end;
        obj->anim.worldPosY = gViewfinderState->posYCurve.end;
        obj->anim.worldPosZ = gViewfinderState->posZCurve.end;
        {
            f32 fade = (1.0f - obj->blendProgress) - 0.2f;
            if (fade < 0.0f)
            {
                fade = 0.0f;
            }
            fade *= 1.25f;
            if (fade > 1.0f)
            {
                fade = 1.0f;
            }
            brightness = (int)(gCamViewfinderBrightnessScale * fade);
        }
        targetObj = (GameObject*)obj->anim.targetObj;
        if (brightness < 1)
        {
            brightness = 1;
        }
        if (targetObj != NULL)
        {
            targetObj->anim.alpha = brightness;
            if (Obj_GetPlayerObject() == targetObj)
            {
                Player_GetHeldObject(targetObj, &shadow2);
                if (shadow2 != NULL)
                {
                    shadow2->anim.alpha = brightness;
                    if (shadow2->anim.alpha == 1)
                    {
                        shadow2->anim.alpha = 0;
                    }
                }
            }
        }
        brightness = 0;
        if (obj->blendProgress <= 0.0f)
        {
            brightness = 1;
        }
        (*gCameraInterface)->getRelativePosition(obj, &outA, &hitY, &outB, &hitDist, 0.0f, 0);
        if (hitDist < 10.0f)
        {
            obj->anim.rotY = 0;
        }
        else
        {
            hitY = obj->anim.worldPosY - (camObj->anim.worldPosY + lbl_803E17C0);
            angleDiff = (getAngle(hitY, hitDist) & 0xffff) - (obj->anim.rotY & 0xffffU);
            if (angleDiff > 0x8000)
            {
                angleDiff = angleDiff - 0xffff;
            }
            if (angleDiff < -0x8000)
            {
                angleDiff = angleDiff + 0xffff;
            }
            obj->anim.rotY = *(s16*)&obj->anim.rotY + (int)((f32)angleDiff * timeDelta) / 8;
        }
        if (brightness != 0)
        {
            (*gCameraInterface)->setMode(VIEWFINDER_CAMMODE_DEFAULT, 0, 1, 0, NULL, 0, 0xff);
            exitTarget = (GameObject*)obj->anim.targetObj;
            if (exitTarget != NULL)
            {
                exitTarget->anim.alpha = 0xff;
                if (Obj_GetPlayerObject() == exitTarget)
                {
                    Player_GetHeldObject(exitTarget, &shadow);
                    if (shadow != NULL)
                    {
                        shadow->anim.alpha = 0xff;
                        if (shadow->anim.alpha == 1)
                        {
                            shadow->anim.alpha = 0;
                        }
                    }
                }
            }
        }
        obj->unk13E = 1;
        break;
    }
    case VIEWFINDER_MODE_IDLE:
        break;
    }
    if (ObjHits_GetPriorityHit((GameObject*)obj->anim.targetObj, 0, 0, 0) != 0)
    {
        firstPersonExit(obj);
        obj->anim.worldPosX = gViewfinderState->posXCurve.end;
        obj->anim.worldPosY = gViewfinderState->posYCurve.end;
        obj->anim.worldPosZ = gViewfinderState->posZCurve.end;
        (*gCameraInterface)->setMode(VIEWFINDER_CAMMODE_DEFAULT, 0, 1, 0, NULL, 0, 0);
    }
    logPrintf(sCam5BYDebugFormat, obj->anim.worldPosY);
    Obj_TransformWorldPointToLocal(obj->anim.worldPosX, obj->anim.worldPosY,
                                   obj->anim.worldPosZ, &obj->anim.localPosX, &obj->anim.localPosY,
                                   &obj->anim.localPosZ, obj->anim.parent);
}

void CameraModeViewfinder_init(CameraObject* obj, int mode, int* args)
{
    s16* camObj;
    s16 diff;
    s16 absDiff;
    s16 a2;
    f32 dx;
    f32 dz;
    f32 dist;
    f32 spinRate;
    f32 rollRate;
    f32 cosv;
    f32 sinv;
    f32 zero;
    CameraModeViewfinderInitArgs* a = (CameraModeViewfinderInitArgs*)args;

    camObj = (s16*)obj->anim.targetObj;
    if (gViewfinderState == NULL)
    {
        gViewfinderState = mmAlloc(sizeof(ViewfinderState), 0xf, 0);
    }
    memset(gViewfinderState, 0, sizeof(ViewfinderState));
    *(f32*)gViewfinderState = a->radius;
    gViewfinderState->height = (f32)(u32)a->height;
    gViewfinderState->yOffset = a->yOffset;
    gViewfinderState->yawSpeed = 0.0f;
    diff = 0x8000 - obj->anim.rotX - camObj[0];
    if (diff < 0)
    {
        absDiff = -diff;
    }
    else
    {
        absDiff = diff;
    }
    spinRate = diff / 50.0f;
    rollRate = absDiff / 90.0f;
    gViewfinderState->viewCurve.px = &gViewfinderState->posXCurve.start;
    gViewfinderState->viewCurve.py = &gViewfinderState->posYCurve.start;
    gViewfinderState->viewCurve.pz = &gViewfinderState->posZCurve.start;
    gViewfinderState->viewCurve.count = 4;
    gViewfinderState->viewCurve.dir = 0;
    gViewfinderState->viewCurve.eval = Curve_EvalHermite;
    gViewfinderState->viewCurve.coeffFn = Curve_BuildHermiteCoeffs;
    dx = obj->anim.worldPosX - ((GameObject*)camObj)->anim.worldPosX;
    dz = obj->anim.worldPosZ - ((GameObject*)camObj)->anim.worldPosZ;
    dist = sqrtf(dx * dx + dz * dz);
    if (0.0f != dist)
    {
        dx = dx / dist;
        dz = dz / dist;
    }
    firstPersonPlaceCamera((GameObject*)camObj, 1);
    cosv = -mathSinf((gCamViewfinderPi * camObj[0]) / 32768.0f);
    sinv = -mathCosf((gCamViewfinderPi * camObj[0]) / 32768.0f);
    gViewfinderState->posXCurve.start = obj->anim.worldPosX;
    gViewfinderState->posXCurve.end = gViewfinderState->camPosX;
    gViewfinderState->posXCurve.startTangent = -dz * spinRate;
    gViewfinderState->posXCurve.endTangent = cosv * rollRate;
    gViewfinderState->posYCurve.start = obj->anim.worldPosY;
    gViewfinderState->posYCurve.end = gViewfinderState->camPosY;
    zero = 0.0f;
    gViewfinderState->posYCurve.startTangent = zero;
    gViewfinderState->posYCurve.endTangent = zero;
    gViewfinderState->posZCurve.start = obj->anim.worldPosZ;
    gViewfinderState->posZCurve.end = gViewfinderState->camPosZ;
    gViewfinderState->posZCurve.startTangent = dx * spinRate;
    gViewfinderState->posZCurve.endTangent = sinv * rollRate;
    gViewfinderState->posXCurve.startTangent = zero;
    gViewfinderState->posXCurve.endTangent = zero;
    gViewfinderState->posYCurve.startTangent = zero;
    gViewfinderState->posYCurve.endTangent = zero;
    gViewfinderState->posZCurve.startTangent = zero;
    gViewfinderState->posZCurve.endTangent = zero;
    curvesMove(&gViewfinderState->viewCurve);
    a2 = obj->anim.rotX - (u16)(0x8000 - getAngle(obj->anim.worldPosX - gViewfinderState->posXCurve.end,
                                          obj->anim.worldPosZ - gViewfinderState->posZCurve.end));
    if (a2 > 0x8000)
    {
        a2 = a2 - 0xffff;
    }
    if (a2 < -0x8000)
    {
        a2 = a2 + 0xffff;
    }
    gViewfinderState->yawCurve.start = a2;
    zero = 0.0f;
    gViewfinderState->yawCurve.end = zero;
    gViewfinderState->yawCurve.startTangent = zero;
    gViewfinderState->yawCurve.endTangent = zero;
    if (gViewfinderState->yawCurve.start - gViewfinderState->yawCurve.end > 32768.0f ||
        gViewfinderState->yawCurve.start - gViewfinderState->yawCurve.end < -32768.0f)
    {
        if (gViewfinderState->yawCurve.start < 0.0f)
        {
            gViewfinderState->yawCurve.start += 65535.0f;
        }
        else if (gViewfinderState->yawCurve.end < 0.0f)
        {
            gViewfinderState->yawCurve.end += 65535.0f;
        }
    }
    gViewfinderState->pitchCurve.start = obj->anim.rotY;
    zero = 0.0f;
    gViewfinderState->pitchCurve.end = zero;
    gViewfinderState->pitchCurve.startTangent = zero;
    gViewfinderState->pitchCurve.endTangent = zero;
    obj->unk13E = 1;
    if (mainGetBit(GAMEBIT_ITEM_Viewfinder_Got) != 0)
    {
        gViewfinderState->flags.zoomHudEnabled = 1;
    }
    if (mode == 1)
    {
        gViewfinderState->mode = VIEWFINDER_MODE_IDLE;
    }
    else
    {
        gViewfinderState->mode = VIEWFINDER_MODE_ENTER_BLEND;
        gViewfinderState->flags.sfxEnabled = 1;
        Sfx_PlayFromObject(0, gViewfinderState->flags.zoomHudEnabled ? SFXTRIG_and_swipe2 : SFXTRIG_shop_priceup);
    }
    gViewfinderState->flags.zoomSfxPlaying = 0;
    gViewfinderState->clampedPosY = gViewfinderState->camPosY;
}

void CameraModeViewfinder_release(void)
{
}

void CameraModeViewfinder_initialise(void)
{
}

ResourceDescriptorCallbacks8 gCameraModeViewfinderDescriptor = {
    {0x00000000, 0x00000000, 0x00000000, 0x00060000},
    {(ResourceDescriptorCallback)CameraModeViewfinder_initialise,
     (ResourceDescriptorCallback)CameraModeViewfinder_release,
     0x00000000,
     (ResourceDescriptorCallback)CameraModeViewfinder_init,
     (ResourceDescriptorCallback)CameraModeViewfinder_update,
     (ResourceDescriptorCallback)CameraModeViewfinder_free,
     (ResourceDescriptorCallback)CameraModeViewfinder_copyToCurrent,
     0x00000000}};
