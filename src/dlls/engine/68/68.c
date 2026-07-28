/*
 * DLL 68 / 0x44 - viewfinder camera mode.
 */
#include "main/audio/sfx.h"
#include "main/debug.h"
#include "main/camera_interface.h"
#include "main/dll/CAM/dll_0045_camTalk.h"
#include "main/dll/CAM/viewfinder_state.h"
#include "main/dll/dll_0044_cameramodeviewfinder.h"
#include "main/dll/DB/DBprotection.h"
#include "main/dll/CAM/cambike_state.h"
#include "main/dll/SB/dll_01E8_sbgalleon.h"
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
#include "main/dll/dll_B8.h"
#include "string.h"
#include "main/dll/CAM/cutCam.h"
#include "main/dll/player_api.h"
#include "main/resource.h"

ViewfinderState* lbl_803DD548;

char sCam5BYDebugFormat[] = "y=%f\n";

#define PAD_BUTTON_B  0x200
#define PAD_TRIGGER_Z 0x010

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
extern f32 lbl_803E17C4;
extern f32 lbl_803E17C8;
extern f32 lbl_803E17CC;
extern f32 lbl_803E17D0;
extern f32 lbl_803E17E0;
extern f32 lbl_803E17E4;
extern f32 lbl_803E17E8;
extern f32 lbl_803E17EC;
extern f32 lbl_803E17F0;
extern f32 lbl_803E17F4;
extern f32 lbl_803E17F8;
extern f32 lbl_803E17FC;
extern f32 lbl_803E1800;
extern f32 lbl_803E1804;
extern f32 lbl_803E1808;
extern f32 lbl_803E180C;
extern f32 lbl_803E1810;
extern f32 gCamViewfinderBrightnessScale;
extern f32 lbl_803E1818;
extern f32 lbl_803E181C;
extern f32 lbl_803E1820;
extern f32 lbl_803E1824;
extern f32 lbl_803E1828;
extern f32 lbl_803E182C;
extern f32 lbl_803E1830;
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
        if (((resetClamp != 0) || (lbl_803DD548->camPosX != prevPosX)) || (lbl_803DD548->camPosZ != prevPosZ))
        {
            lbl_803DD548->clampedPosY = prevPosY;
        }
        lbl_803DD548->camPosX = prevPosX;
        lbl_803DD548->camPosY = prevPosY;
        lbl_803DD548->camPosZ = prevPosZ;
    }
    else
    {
        lbl_803DD548->camPosX = self->anim.worldPosX;
        lbl_803DD548->camPosY = lbl_803E17C0 + self->anim.worldPosY;
        lbl_803DD548->camPosZ = self->anim.worldPosZ;
        lbl_803DD548->clampedPosY = lbl_803DD548->camPosY;
    }
    galleon = getSbGalleon();
    if (galleon != NULL)
    {
        galleonState = DBprotection_getCameraState(galleon);
        if (galleonState == 2)
        {
            localOffset[0] = self->anim.worldPosX - galleon->anim.worldPosX;
            localOffset[1] = (lbl_803E17C0 + self->anim.worldPosY) - galleon->anim.worldPosY;
            localOffset[2] = self->anim.worldPosZ - galleon->anim.worldPosZ;
            vecRotateZXY(&galleon->anim.rotX, localOffset);
            lbl_803DD548->camPosX = galleon->anim.worldPosX + localOffset[0];
            lbl_803DD548->camPosY = galleon->anim.worldPosY + localOffset[1];
            lbl_803DD548->camPosZ = galleon->anim.worldPosZ + localOffset[2];
        }
    }
    return;
}

void firstPersonExit(CameraObject* camera)
{
    register CameraObject* self = camera;
    GameObject* target;
    CameraModeBikeState* st;
    float tangent;
    float dx;
    float dz;
    int targetYaw;
    float targetPos[3];
    u8 unusedAngle[4];

    target = (GameObject*)self->anim.targetObj;
    lbl_803DD548->posXCurve.start = self->anim.worldPosX;
    tangent = lbl_803E17C4;
    lbl_803DD548->posXCurve.startTangent = lbl_803E17C4;
    lbl_803DD548->posXCurve.endTangent = tangent;
    lbl_803DD548->posYCurve.start = self->anim.worldPosY;
    lbl_803DD548->posYCurve.startTangent = tangent;
    lbl_803DD548->posYCurve.endTangent = tangent;
    lbl_803DD548->posZCurve.start = self->anim.worldPosZ;
    lbl_803DD548->posZCurve.startTangent = tangent;
    lbl_803DD548->posZCurve.endTangent = tangent;
    camcontrol_getTargetPosition(self, &target->anim, targetPos, (s16*)unusedAngle);
    lbl_803DD548->posXCurve.end = targetPos[0];
    lbl_803DD548->posYCurve.end = targetPos[1];
    lbl_803DD548->posZCurve.end = targetPos[2];
    dx = lbl_803DD548->posXCurve.end - lbl_803DD548->posXCurve.start;
    dz = lbl_803DD548->posZCurve.end - lbl_803DD548->posZCurve.start;
    lbl_803DD548->exitDistance = sqrtf(dx * dx + dz * dz);
    lbl_803DD548->viewCurve.px = &lbl_803DD548->yawCurve.start;
    lbl_803DD548->viewCurve.py = &lbl_803DD548->pitchCurve.start;
    lbl_803DD548->viewCurve.pz = NULL;
    lbl_803DD548->viewCurve.count = 4;
    lbl_803DD548->viewCurve.dir = 0;
    lbl_803DD548->viewCurve.eval = Curve_EvalHermite;
    lbl_803DD548->viewCurve.coeffFn = Curve_BuildHermiteCoeffs;
    lbl_803DD548->yawCurve.start = (float)(int)self->anim.rotX;
    targetYaw = getAngle((double)(lbl_803DD548->posXCurve.end - target->anim.worldPosX),
                         (double)(lbl_803DD548->posZCurve.end - target->anim.worldPosZ));
    lbl_803DD548->yawCurve.end = (float)(int)(short)(0x8000 - targetYaw);
    tangent = lbl_803E17C4;
    lbl_803DD548->yawCurve.startTangent = lbl_803E17C4;
    lbl_803DD548->yawCurve.endTangent = tangent;
    if (((lbl_803DD548->yawCurve.start - lbl_803DD548->yawCurve.end) > lbl_803E17C8) ||
        ((lbl_803DD548->yawCurve.start - lbl_803DD548->yawCurve.end) < lbl_803E17CC))
    {
        if (lbl_803DD548->yawCurve.start < lbl_803E17C4)
        {
            lbl_803DD548->yawCurve.start = *(f32*)&lbl_803DD548->yawCurve.start + lbl_803E17D0;
        }
        else
        {
            if (lbl_803DD548->yawCurve.end < lbl_803E17C4)
            {
                lbl_803DD548->yawCurve.end = *(f32*)&lbl_803DD548->yawCurve.end + lbl_803E17D0;
            }
        }
    }
    lbl_803DD548->pitchCurve.start = (float)(int)self->anim.rotY;
    tangent = lbl_803E17C4;
    lbl_803DD548->pitchCurve.end = lbl_803E17C4;
    lbl_803DD548->pitchCurve.startTangent = tangent;
    lbl_803DD548->pitchCurve.endTangent = tangent;
    if (((lbl_803DD548->pitchCurve.start - lbl_803DD548->pitchCurve.end) > lbl_803E17C8) ||
        ((lbl_803DD548->pitchCurve.start - lbl_803DD548->pitchCurve.end) < lbl_803E17CC))
    {
        if (lbl_803DD548->pitchCurve.start < lbl_803E17C4)
        {
            lbl_803DD548->pitchCurve.start = *(f32*)&lbl_803DD548->pitchCurve.start + lbl_803E17D0;
        }
        else
        {
            if (lbl_803DD548->pitchCurve.end < lbl_803E17C4)
            {
                lbl_803DD548->pitchCurve.end = *(f32*)&lbl_803DD548->pitchCurve.end + lbl_803E17D0;
            }
        }
    }
    curvesMove(&lbl_803DD548->viewCurve);
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
    t = (lbl_803E17E0 - obj->fov) / lbl_803E17E4;
    zoom = (t < lbl_803E17C4) ? lbl_803E17C4 : ((t > lbl_803E17E8) ? lbl_803E17E8 : t);
    spin = stickX * -(lbl_803E17F0 * zoom - lbl_803E17EC);
    spin = interpolate(spin - lbl_803DD548->yawSpeed, lbl_803E17F4, timeDelta);
    lbl_803DD548->yawSpeed = lbl_803DD548->yawSpeed + spin;
    if ((lbl_803DD548->yawSpeed > lbl_803E17F8) && (lbl_803DD548->yawSpeed < lbl_803E17FC))
    {
        lbl_803DD548->yawSpeed = lbl_803E17C4;
    }
    spinI = (int)(lbl_803E1800 * ((f32)stickY / lbl_803E1804));
    obj->anim.rotX = lbl_803DD548->yawSpeed * timeDelta + (f32)obj->anim.rotX;
    pitchDelta = spinI - (obj->anim.rotY & 0xffffU);
    if (0x8000 < pitchDelta)
    {
        pitchDelta = pitchDelta - 0xffff;
    }
    if (pitchDelta < -0x8000)
    {
        pitchDelta = pitchDelta + 0xffff;
    }
    spin = interpolate((f32)pitchDelta, lbl_803E17E8 / (lbl_803E180C * zoom + lbl_803E1808), timeDelta);
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
    if (lbl_803DD548->camPosY < lbl_803DD548->clampedPosY)
    {
        lbl_803DD548->clampedPosY = lbl_803DD548->camPosY;
    }
    obj->anim.worldPosX = lbl_803DD548->camPosX;
    obj->anim.worldPosY = lbl_803DD548->clampedPosY;
    obj->anim.worldPosZ = lbl_803DD548->camPosZ;
    if (lbl_803DD548->flags.zoomHudEnabled)
    {
        zoom2 = obj->fov;
        stickX = padGetCY(0);
        t = (f32)-stickX;
        t = lbl_803E1810 * t;
        zoom2 = t * timeDelta + zoom2;
        viewFinderSetZoom(Camera_GetFovY());
        fovTarget = (zoom2 < lbl_803E17FC) ? lbl_803E17FC : ((zoom2 > lbl_803E17E0) ? lbl_803E17E0 : zoom2);
        if (lbl_803DD548->flags.sfxEnabled)
        {
            if ((fovTarget == obj->fov) && (lbl_803DD548->flags.zoomSfxPlaying))
            {
                Sfx_StopFromObject(0, SFXTRIG_and_swipe1);
                lbl_803DD548->flags.zoomSfxPlaying = 0;
            }
            if ((fovTarget != obj->fov) && (!lbl_803DD548->flags.zoomSfxPlaying))
            {
                Sfx_PlayFromObject(0, SFXTRIG_and_swipe1);
                lbl_803DD548->flags.zoomSfxPlaying = 1;
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

    cam->anim.worldPosX = lbl_803DD548->camPosX;
    cam->anim.worldPosY = lbl_803DD548->camPosY;
    cam->anim.worldPosZ = lbl_803DD548->camPosZ;
    cam->anim.rotY = 0;
    flag = 0;
    if (cam->blendProgress <= lbl_803E17C4)
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
        lbl_803DD548->viewCurve.px = &lbl_803DD548->yawCurve.start;
        lbl_803DD548->viewCurve.py = NULL;
        lbl_803DD548->viewCurve.pz = NULL;
        lbl_803DD548->viewCurve.count = 4;
        lbl_803DD548->viewCurve.eval = Curve_EvalHermite;
        lbl_803DD548->viewCurve.coeffFn = Curve_BuildHermiteCoeffs;
        lbl_803DD548->viewCurve.dir = 0;
        lbl_803DD548->yawCurve.start = (f32)(s32)cam->anim.rotX;
        lbl_803DD548->yawCurve.end = (f32)(s16)(0x8000 - p2[0]);
        start = lbl_803DD548->yawCurve.start;
        end = lbl_803DD548->yawCurve.end;
        f2 = start - end;
        if (f2 < lbl_803E1818 && f2 > lbl_803E181C)
        {
            lbl_803DD548->yawCurve.end = lbl_803DD548->yawCurve.start;
        }
        else if (f2 > lbl_803E17C8 || f2 < lbl_803E17CC)
        {
            if (start < lbl_803E17C4)
            {
                lbl_803DD548->yawCurve.start += lbl_803E17D0;
            }
            else if (end < lbl_803E17C4)
            {
                lbl_803DD548->yawCurve.end += lbl_803E17D0;
            }
        }
        {
            f32 k = lbl_803E17C4;
            lbl_803DD548->yawCurve.startTangent = k;
            lbl_803DD548->yawCurve.endTangent = k;
        }
        curvesMove(&lbl_803DD548->viewCurve);
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
    mm_free(lbl_803DD548);
    lbl_803DD548 = NULL;
    viewFinderSetZoom(lbl_803E17E0);
}

void CameraModeViewfinder_update(CameraObject* obj)
{
    int camObj;
    GameObject* targetObj;
    int brightness;
    GameObject* exitTarget;
    int angleDiff;
    f32 outA;
    f32 hitY;
    f32 outB;
    f32 hitDist;
    GameObject* shadow2;
    GameObject* shadow;

    camObj = (int)obj->anim.targetObj;
    getButtonsJustPressed(0);
    firstPersonPlaceCamera((GameObject*)camObj, 0);
    switch (lbl_803DD548->mode)
    {
    case VIEWFINDER_MODE_ENTER_BLEND:
        lbl_803DD548->mode = firstPersonEnter(obj, (s16*)obj->anim.targetObj);
        break;
    case VIEWFINDER_MODE_YAW_SETTLE:
        if (Curve_AdvanceAlongPath(&lbl_803DD548->viewCurve, lbl_803E1820) != 0)
        {
            if (lbl_803DD548->flags.zoomHudEnabled)
            {
                Rcp_SetViewFinderHudEnabled(1);
            }
            lbl_803DD548->mode = VIEWFINDER_MODE_ACTIVE;
        }
        obj->anim.rotX = lbl_803DD548->viewCurve.sample[0];
        obj->unk13E = 1;
        break;
    case VIEWFINDER_MODE_ACTIVE:
        if (lbl_803DD548->flags.zoomHudEnabled)
        {
            Rcp_SetViewFinderHudEnabled(1);
        }
        firstPersonDoControls(obj);
        if (getButtonsJustPressed(0) & (PAD_BUTTON_B | PAD_TRIGGER_Z))
        {
            buttonDisable(0, PAD_BUTTON_B);
            firstPersonExit(obj);
            Rcp_SetViewFinderHudEnabled(0);
            lbl_803DD548->mode = VIEWFINDER_MODE_EXIT_BLEND;
        }
        obj->unk13E = 0;
        break;
    case VIEWFINDER_MODE_EXIT_BLEND:
        angleDiff = Curve_AdvanceAlongPath(&lbl_803DD548->viewCurve, lbl_803E1820);
        obj->anim.rotX = lbl_803DD548->viewCurve.sample[0];
        obj->anim.rotY = lbl_803DD548->viewCurve.sample[1];
        if (angleDiff != 0)
        {
            lbl_803DD548->viewCurve.px = &lbl_803DD548->posXCurve.start;
            lbl_803DD548->viewCurve.py = &lbl_803DD548->posYCurve.start;
            lbl_803DD548->viewCurve.pz = &lbl_803DD548->posZCurve.start;
            lbl_803DD548->viewCurve.count = 4;
            lbl_803DD548->viewCurve.dir = 0;
            lbl_803DD548->viewCurve.eval = Curve_EvalHermite;
            lbl_803DD548->viewCurve.coeffFn = Curve_BuildHermiteCoeffs;
            curvesMove(&lbl_803DD548->viewCurve);
            ((GameObject*)obj->anim.targetObj)->anim.flags &= ~OBJANIM_FLAG_HIDDEN;
            firstPersonZoomOutOnExit(0xf, 0xfe);
            lbl_803DD548->mode = VIEWFINDER_MODE_FADE_BACK;
            if (lbl_803DD548->flags.sfxEnabled)
            {
                Sfx_PlayFromObject(0, lbl_803DD548->flags.zoomHudEnabled ? SFXTRIG_and_missilelaunch
                                                                         : SFXTRIG_shop_pricedown);
            }
        }
        obj->unk13E = 1;
        break;
    case VIEWFINDER_MODE_FADE_BACK:
        obj->anim.worldPosX = lbl_803DD548->posXCurve.end;
        obj->anim.worldPosY = lbl_803DD548->posYCurve.end;
        obj->anim.worldPosZ = lbl_803DD548->posZCurve.end;
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
        if (hitDist < lbl_803E182C)
        {
            obj->anim.rotY = 0;
        }
        else
        {
            hitY = obj->anim.worldPosY - (((GameObject*)camObj)->anim.worldPosY + lbl_803E17C0);
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
    case VIEWFINDER_MODE_IDLE:
        break;
    }
    if (ObjHits_GetPriorityHit((GameObject*)obj->anim.targetObj, 0, 0, 0) != 0)
    {
        firstPersonExit(obj);
        obj->anim.worldPosX = lbl_803DD548->posXCurve.end;
        obj->anim.worldPosY = lbl_803DD548->posYCurve.end;
        obj->anim.worldPosZ = lbl_803DD548->posZCurve.end;
        (*gCameraInterface)->setMode(VIEWFINDER_CAMMODE_DEFAULT, 0, 1, 0, NULL, 0, 0);
    }
    logPrintf(sCam5BYDebugFormat, obj->anim.worldPosY);
    Obj_TransformWorldPointToLocal(obj->anim.worldPosX, obj->anim.worldPosY,
                                   obj->anim.worldPosZ, &obj->anim.localPosX, &obj->anim.localPosY,
                                   &obj->anim.localPosZ, (int)obj->anim.parent);
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
    if (lbl_803DD548 == NULL)
    {
        lbl_803DD548 = mmAlloc(sizeof(ViewfinderState), 0xf, 0);
    }
    memset(lbl_803DD548, 0, sizeof(ViewfinderState));
    *(f32*)lbl_803DD548 = a->radius;
    lbl_803DD548->height = (f32)(u32)a->height;
    lbl_803DD548->yOffset = a->yOffset;
    lbl_803DD548->yawSpeed = lbl_803E17C4;
    diff = 0x8000 - obj->anim.rotX - camObj[0];
    if (diff < 0)
    {
        absDiff = -diff;
    }
    else
    {
        absDiff = diff;
    }
    spinRate = diff / lbl_803E17E4;
    rollRate = absDiff / lbl_803E1830;
    lbl_803DD548->viewCurve.px = &lbl_803DD548->posXCurve.start;
    lbl_803DD548->viewCurve.py = &lbl_803DD548->posYCurve.start;
    lbl_803DD548->viewCurve.pz = &lbl_803DD548->posZCurve.start;
    lbl_803DD548->viewCurve.count = 4;
    lbl_803DD548->viewCurve.dir = 0;
    lbl_803DD548->viewCurve.eval = Curve_EvalHermite;
    lbl_803DD548->viewCurve.coeffFn = Curve_BuildHermiteCoeffs;
    dx = obj->anim.worldPosX - ((GameObject*)camObj)->anim.worldPosX;
    dz = obj->anim.worldPosZ - ((GameObject*)camObj)->anim.worldPosZ;
    dist = sqrtf(dx * dx + dz * dz);
    if (lbl_803E17C4 != dist)
    {
        dx = dx / dist;
        dz = dz / dist;
    }
    firstPersonPlaceCamera((GameObject*)camObj, 1);
    cosv = -mathSinf((gCamViewfinderPi * camObj[0]) / lbl_803E17C8);
    sinv = -mathCosf((gCamViewfinderPi * camObj[0]) / lbl_803E17C8);
    lbl_803DD548->posXCurve.start = obj->anim.worldPosX;
    lbl_803DD548->posXCurve.end = lbl_803DD548->camPosX;
    lbl_803DD548->posXCurve.startTangent = -dz * spinRate;
    lbl_803DD548->posXCurve.endTangent = cosv * rollRate;
    lbl_803DD548->posYCurve.start = obj->anim.worldPosY;
    lbl_803DD548->posYCurve.end = lbl_803DD548->camPosY;
    zero = lbl_803E17C4;
    lbl_803DD548->posYCurve.startTangent = zero;
    lbl_803DD548->posYCurve.endTangent = zero;
    lbl_803DD548->posZCurve.start = obj->anim.worldPosZ;
    lbl_803DD548->posZCurve.end = lbl_803DD548->camPosZ;
    lbl_803DD548->posZCurve.startTangent = dx * spinRate;
    lbl_803DD548->posZCurve.endTangent = sinv * rollRate;
    lbl_803DD548->posXCurve.startTangent = zero;
    lbl_803DD548->posXCurve.endTangent = zero;
    lbl_803DD548->posYCurve.startTangent = zero;
    lbl_803DD548->posYCurve.endTangent = zero;
    lbl_803DD548->posZCurve.startTangent = zero;
    lbl_803DD548->posZCurve.endTangent = zero;
    curvesMove(&lbl_803DD548->viewCurve);
    a2 = obj->anim.rotX - (u16)(0x8000 - getAngle(obj->anim.worldPosX - lbl_803DD548->posXCurve.end,
                                          obj->anim.worldPosZ - lbl_803DD548->posZCurve.end));
    if (a2 > 0x8000)
    {
        a2 = a2 - 0xffff;
    }
    if (a2 < -0x8000)
    {
        a2 = a2 + 0xffff;
    }
    lbl_803DD548->yawCurve.start = a2;
    zero = lbl_803E17C4;
    lbl_803DD548->yawCurve.end = zero;
    lbl_803DD548->yawCurve.startTangent = zero;
    lbl_803DD548->yawCurve.endTangent = zero;
    if (lbl_803DD548->yawCurve.start - lbl_803DD548->yawCurve.end > lbl_803E17C8 ||
        lbl_803DD548->yawCurve.start - lbl_803DD548->yawCurve.end < lbl_803E17CC)
    {
        if (lbl_803DD548->yawCurve.start < lbl_803E17C4)
        {
            lbl_803DD548->yawCurve.start += lbl_803E17D0;
        }
        else if (lbl_803DD548->yawCurve.end < lbl_803E17C4)
        {
            lbl_803DD548->yawCurve.end += lbl_803E17D0;
        }
    }
    lbl_803DD548->pitchCurve.start = obj->anim.rotY;
    zero = lbl_803E17C4;
    lbl_803DD548->pitchCurve.end = zero;
    lbl_803DD548->pitchCurve.startTangent = zero;
    lbl_803DD548->pitchCurve.endTangent = zero;
    obj->unk13E = 1;
    if (mainGetBit(GAMEBIT_ITEM_Viewfinder_Got) != 0)
    {
        lbl_803DD548->flags.zoomHudEnabled = 1;
    }
    if (mode == 1)
    {
        lbl_803DD548->mode = VIEWFINDER_MODE_IDLE;
    }
    else
    {
        lbl_803DD548->mode = VIEWFINDER_MODE_ENTER_BLEND;
        lbl_803DD548->flags.sfxEnabled = 1;
        Sfx_PlayFromObject(0, lbl_803DD548->flags.zoomHudEnabled ? SFXTRIG_and_swipe2 : SFXTRIG_shop_priceup);
    }
    lbl_803DD548->flags.zoomSfxPlaying = 0;
    lbl_803DD548->clampedPosY = lbl_803DD548->camPosY;
}

void CameraModeViewfinder_release(void)
{
}

void CameraModeViewfinder_initialise(void)
{
}

ResourceDescriptorCallbacks8 lbl_80319BF8 = {
    {0x00000000, 0x00000000, 0x00000000, 0x00060000},
    {(ResourceDescriptorCallback)CameraModeViewfinder_initialise,
     (ResourceDescriptorCallback)CameraModeViewfinder_release,
     0x00000000,
     (ResourceDescriptorCallback)CameraModeViewfinder_init,
     (ResourceDescriptorCallback)CameraModeViewfinder_update,
     (ResourceDescriptorCallback)CameraModeViewfinder_free,
     (ResourceDescriptorCallback)CameraModeViewfinder_copyToCurrent,
     0x00000000}};
