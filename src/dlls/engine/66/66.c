#include "main/dll/CAM/cutCam.h"
#include "main/dll/CAM/dll_0001_camcontrol.h"
#include "main/object_transform.h"
#include "main/camera_interface.h"
#include "main/curve.h"
#include "dolphin/os/OSTime.h"
#include "main/dll/player_api.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/frame_timing.h"
#include "main/track_dolphin_api.h"
#include "dolphin/mtx/vec.h"
#include "main/track_bbox_api.h"
#include "main/mm.h"
#include "string.h"
#include "main/vecmath.h"
#include "main/dll/dll_0042_unk.h"
#include "dolphin/os.h"
#include "main/resource.h"

CamcontrolModeSettings* gCamcontrolModeSettings;
f32 gCamcontrolScaledTimeDelta;
u8 gCutCamBboxBlocked;

#define PAD_TRIGGER_Z 0x10
#define PAD_TRIGGER_L 0x40

#define CAMMODE_CLIMB 0x43
#define CAMMODE_VIEWFINDER 0x44
#define CAMMODE_COMBAT 0x49


int camcontrol_traceMove(float* fromPos, float* toPos, float* outPos, u8* traceWork, char traceMode, u8 runTrace,
                         u8 runBbox, float radius)
{
    u8 blocked;
    int clear;
    float endTmp[3];
    TrackQueryBounds sweptBounds;

    if (outPos == NULL)
    {
        outPos = endTmp;
    }
    *outPos = *toPos;
    outPos[1] = toPos[1];
    outPos[2] = toPos[2];
    *(float*)(traceWork + CAMCONTROL_TRACE_RADIUS_OFFSET) = radius;
    *(s8*)(traceWork + CAMCONTROL_TRACE_BBOX_HIT_OFFSET) = -1;
    *(s8*)(traceWork + CAMCONTROL_TRACE_MODE_OFFSET) = traceMode;
    *(s16*)(traceWork + CAMCONTROL_TRACE_HIT_COUNT_OFFSET) = 0;
    blocked = 0;
    if (runBbox != 0)
    {
        blocked = objBboxFn_800640cc(fromPos, outPos, radius, 1, NULL, NULL, 0x10, 0xffffffff, 0xff, 0);
    }
    else
    {
        blocked = 0;
    }
    gCutCamBboxBlocked = blocked;
    if (runTrace != 0)
    {
        hitDetect_calcSweptSphereBounds(&sweptBounds, fromPos, outPos,
                                        (float*)(traceWork + CAMCONTROL_TRACE_RADIUS_OFFSET), 1);
        hitDetectFn_800691c0(NULL, &sweptBounds, 0x240, 1);
    }
    hitDetectFn_80067958(NULL, fromPos, outPos, 1, traceWork, 0);
    clear = 0;
    if ((gCutCamBboxBlocked == 0) && (*(short*)(traceWork + CAMCONTROL_TRACE_HIT_COUNT_OFFSET) == 0))
    {
        clear = 1;
    }
    return clear;
}
void doNothing_80103660(int unused)
{
}

u8 camcontrol_traceFromTarget(float* fromPos, GameObject* target, float* outPos, void* unused)
{
    float targetPos[3];
    u8 traceRec[111];

    if (target->anim.classId == 1)
    {
        cameraGetPrevPos2(target, &targetPos[0], &targetPos[1], &targetPos[2]);
    }
    else
    {
        targetPos[0] = target->anim.worldPosX;
        targetPos[1] = target->anim.worldPosY + gCamcontrolModeSettings->targetHeight;
        targetPos[2] = target->anim.worldPosZ;
    }
    camcontrol_traceMove(targetPos, fromPos, outPos, traceRec, 3, '\x01', '\x01', (double)4.0f);
    return traceRec[CAMCONTROL_TRACE_BLOCKED_OFFSET];
}

u8 camcontrol_getTargetPosition(CameraObject* camera, ObjAnimComponent* targetAnim, f32* outPos, s16* outRotY)
{
    u8 box[112];
    float prev[3];
    float pos[3];
    f32 d2;
    f32 a;
    f32 b;
    f32 c;
    f32 cosv;
    f32 sinv;
    u32 ang;
    int angleDelta;

    cosv = mathSinf((3.1415927f * targetAnim->rotX) / 32768.0f);
    sinv = mathCosf((3.1415927f * targetAnim->rotX) / 32768.0f);
    d2 = gCamcontrolModeSettings->maxDistance * gCamcontrolModeSettings->maxDistance -
         gCamcontrolModeSettings->lowerHeightOffset * gCamcontrolModeSettings->lowerHeightOffset;
    if (d2 < 5.0f)
    {
        d2 = 5.0f;
    }
    d2 = sqrtf(d2);
    pos[0] = cosv * d2 + targetAnim->worldPosX;
    pos[1] = gCamcontrolModeSettings->lowerHeightOffset + (targetAnim->worldPosY + gCamcontrolModeSettings->targetHeight);
    pos[2] = sinv * d2 + targetAnim->worldPosZ;
    if (targetAnim->classId == 1)
    {
        cameraGetPrevPos2((GameObject*)targetAnim, &prev[0], &prev[1], &prev[2]);
    }
    else
    {
        prev[0] = targetAnim->worldPosX;
        prev[1] = targetAnim->worldPosY + gCamcontrolModeSettings->targetHeight;
        prev[2] = targetAnim->worldPosZ;
    }
    camcontrol_traceMove(prev, pos, outPos, box, 3, '\x01', '\x01', 4.0f);
    (*gCameraInterface)->getRelativePosition(camera, &a, &b, &c, &d2, gCamcontrolModeSettings->targetHeight, 0);
    b = camera->anim.worldPosY - (targetAnim->worldPosY + gCamcontrolModeSettings->targetHeight);
    ang = getAngle(b, d2);
    angleDelta = ang & 0xffff;
    angleDelta -= (u16)camera->anim.rotY;
    if (0x8000 < angleDelta)
    {
        angleDelta = angleDelta - 0xffff;
    }
    if (angleDelta < -0x8000)
    {
        angleDelta = angleDelta + 0xffff;
    }
    if (outRotY != NULL)
    {
        *outRotY = camera->anim.rotY + angleDelta;
    }
    return box[CAMCONTROL_TRACE_BLOCKED_OFFSET];
}

void camcontrol_updateTargetAction(CameraObject* camera, GameObject* target)
{
    short classId;
    u16 buttons;
    int cond;
    CamcontrolAction43Payload action43Payload;
    CamcontrolAction44Payload action44Payload;

    if (target->pendingParentObj == NULL)
    {
        buttons = getButtonsJustPressed(0);
        if (((camera->currentTarget != NULL) &&
             (((classId = ((GameObject*)camera->currentTarget)->anim.classId) == 0x1c) || (classId == 0x2a)) &&
             (target->anim.classId == 1) && ((cond = playerIsStaffActionPending(target)) != 0) &&
             ((cond = fn_80295C0C(target)) != 0)) ||
            ((camera->targetFlags & 2) != 0))
        {
            Camera_setBlendCurveMode(1);
            (*gCameraInterface)->setMode(CAMMODE_COMBAT, 1, 0, 4, &camera->currentTarget, 0x3c, 0xff);
        }
        else if ((((buttons & PAD_TRIGGER_Z) != 0) && (target->anim.classId == 1)) &&
                 (cond = objFn_802962b4((GameObject*)target), cond != 0))
        {
            action44Payload.distance = gCamcontrolModeSettings->minDistance;
            action44Payload.yOffset = gCamcontrolModeSettings->lowerHeightOffset;
            action44Payload.height = gCamcontrolModeSettings->targetHeight;
            Camera_setBlendCurveMode(0);
            (*gCameraInterface)->setMode(CAMMODE_VIEWFINDER, 1, 0, 0xc, &action44Payload, 0xf, 0xfe);
        }
        else
        {
            cond = getCurSeqNo();
            if (((cond == 0) && (buttons = padGetTriggersPressed(0), (buttons & PAD_TRIGGER_L) != 0)) &&
                ((camera->anim.flags & 4) == 0))
            {
                action43Payload.action = 5;
                action43Payload.enabled = 1;
                action43Payload.immediate = 1;
                (*gCameraInterface)->setMode(CAMMODE_CLIMB, 1, 0, 4, &action43Payload, 0, 0xff);
            }
        }
    }
}

int cameraFn_80103b40(CameraObject* cam, f32* outA, f32* outB, int angle)
{
    int tgt0;
    float probe[75];
    u8 box[136];
    float pathA[21];
    float pathB[21];
    float prev[3];
    f32 spinA;
    f32 spinB;
    f32 spinC;
    f32 spinD;
    GameObject* tgt;
    int ang;
    float* pA;
    float* pB;
    float* pp;
    float* pA0;
    float* pB0;
    int result;
    int s;
    int i;
    int found1;
    int found2;
    int dir;
    int d;
    f32 cosv;
    f32 rad;
    f32 dx;
    f32 dz;
    f32 sinv;
    f32 t;
    f32 v;

    OSGetTick(); /* timing probe; return value intentionally unused */
    result = 0;
    (*gCameraInterface)->getRelativePosition(cam, &spinB, &spinC, &spinD, &spinA, gCamcontrolModeSettings->targetHeight, 0);
    tgt0 = *(int*)&cam->anim.targetObj;
    *(int*)&probe[35] = tgt0;
    probe[1] = cam->anim.worldPosY;
    pathA[0] = cam->anim.worldPosX;
    pathA[1] = cam->anim.worldPosY;
    pathA[2] = cam->anim.worldPosZ;
    pathB[0] = pathA[0];
    pathB[1] = pathA[1];
    pathB[2] = pathA[2];
    if (((GameObject*)tgt0)->anim.classId == 1)
    {
        cameraGetPrevPos2((GameObject*)tgt0, &prev[0], &prev[1], &prev[2]);
    }
    else
    {
        prev[0] = ((GameObject*)tgt0)->anim.worldPosX;
        prev[1] = ((GameObject*)tgt0)->anim.worldPosY + gCamcontrolModeSettings->targetHeight;
        prev[2] = ((GameObject*)tgt0)->anim.worldPosZ;
    }
    s = 0xf;
    i = 0;
    found1 = -1;
    found2 = -1;
    ang = 0xaaa;
    pA0 = pathA;
    pA = pA0;
    pB0 = pathB;
    pB = pB0;
    pp = probe;
    while ((s16)s <= 0x5a)
    {
        if (found1 == -1)
        {
            dx = spinD;
            dz = spinB;
            tgt = (GameObject*)(*(int*)&cam->anim.targetObj);
            rad = (3.1415927f * (f32)(s16)ang) / 32768.0f;
            cosv = mathSinf(rad);
            sinv = mathCosf(rad);
            t = dz * sinv - dx * cosv;
            v = t * cosv + dx * sinv;
            t = t + tgt->anim.worldPosX;
            probe[0] = t;
            v = v + tgt->anim.worldPosZ;
            probe[2] = v;
            pA[3] = probe[0];
            pA[4] = probe[1];
            pA[5] = probe[2];
            if (camcontrol_traceMove(prev, pp, NULL, box, 7, '\0', '\0', 3.9f) != 0)
            {
                found1 = i;
            }
        }
        if (found2 == -1)
        {
            dx = spinD;
            dz = spinB;
            tgt = (GameObject*)(*(int*)&cam->anim.targetObj);
            rad = (3.1415927f * (f32)(s16)(-s * 0xb6)) / 32768.0f;
            cosv = mathSinf(rad);
            sinv = mathCosf(rad);
            t = dz * sinv - dx * cosv;
            v = t * cosv + dx * sinv;
            t = t + tgt->anim.worldPosX;
            probe[0] = t;
            v = v + tgt->anim.worldPosZ;
            probe[2] = v;
            pB[3] = probe[0];
            pB[4] = probe[1];
            pB[5] = probe[2];
            if (camcontrol_traceMove(prev, pp, NULL, box, 7, '\0', '\0', 3.9f) != 0)
            {
                found2 = i;
            }
        }
        pA = pA + 3;
        pB = pB + 3;
        i++;
        ang = ang + 0xaaa;
        s = s + 0xf;
    }
    if (found1 == -1)
    {
        found1 = 6;
    }
    else
    {
        for (i = 0; i <= found1; i++)
        {
            if (camcontrol_traceMove(pA0, pathA + (i + 1) * 3, NULL, box, 7, '\0', '\0', 3.9f) == 0)
            {
                found1 = 6;
                break;
            }
            pA0 = pA0 + 3;
        }
    }
    if (found2 == -1)
    {
        found2 = 6;
    }
    else
    {
        for (i = 0; i <= found2; i++)
        {
            if (camcontrol_traceMove(pB0, pathB + (i + 1) * 3, NULL, box, 7, '\0', '\0', 3.9f) == 0)
            {
                found2 = 6;
                break;
            }
            pB0 = pB0 + 3;
        }
    }
    dir = 0;
    if (found1 < found2)
    {
        dir = 1;
    }
    else if (found2 < found1)
    {
        dir = -1;
    }
    else if (found1 < 6)
    {
        dir = 1;
    }
    if (dir != 0)
    {
        f32 f;
        f32 g;
        d = (0x8000 - cam->anim.rotX) - (angle & 0xffff);
        if (0x8000 < d)
        {
            d = d - 0xffff;
        }
        if (d < -0x8000)
        {
            d = d + 0xffff;
        }
        if (d < 0)
        {
            d = -d;
        }
        f = cam->unkC4 * cam->unkC4;
        if (f < 1.0f)
        {
            f = 1.0f;
        }
        f *= 3.0f;
        g = 0.0f;
        g += f;
        g = g + d / 500.0f;
        if (g < 10.0f)
        {
            g = 10.0f;
        }
        if (g > 100.0f)
        {
            g = 100.0f;
        }
        if (dir == -1)
        {
            g = -g;
        }
        g = g * gCamcontrolScaledTimeDelta + gCamcontrolModeSettings->avoidanceYawOffset;
        if (g > 1000.0f)
        {
            g = 1000.0f;
        }
        else if (g < -1000.0f)
        {
            g = -1000.0f;
        }
        gCamcontrolModeSettings->avoidanceYawOffset = g;
        result = 1;
    }
    return result;
}

/*
 * Wall-avoidance update for the cut-scene camera.  Builds a 13-point fan of
 * candidate camera positions (0x555-step yaw increments alternating left /
 * right of the camera-to-target axis), batch-registers their swept-sphere
 * bounds with the hit-detect system, then line-of-sight traces the current
 * camera position.  If the view is blocked it asks cameraFn_80103b40 to pick
 * a swing direction, and while an avoidance yaw offset is active it rotates
 * the camera around the target and decays the offset by 0.9 per frame
 * (snapping to zero inside +/-0.5).
 */
void camcontrol_updateWallAvoidance(CameraObject* camera, GameObject* target)
{
    float path[39];
    float endPts[13][3];
    u8 box[112];
    float radii[13];
    TrackQueryBounds bounds;
    float prev[3];
    f32 outB[2];
    f32 outA[2];
    int ang;
    float* p;
    int i;
    int j;
    f32 dz;
    f32 dx;
    f32 rad;
    f32 sinv;
    f32 cosv;
    f32 t;
    f32 z;
    u32 blocked;
    u8 trace;
    s16 spin;

    Obj_TransformLocalPointToWorld(camera->anim.localPosX, camera->anim.localPosY, camera->anim.localPosZ,
                                   &camera->anim.worldPosX, &camera->anim.worldPosY, &camera->anim.worldPosZ,
                                   camera->anim.parent);
    gCutCamBboxBlocked = 0;
    if (target->anim.classId == 1)
    {
        cameraGetPrevPos2(target, &prev[0], &prev[1], &prev[2]);
    }
    else
    {
        prev[0] = target->anim.worldPosX;
        prev[1] = target->anim.worldPosY + gCamcontrolModeSettings->targetHeight;
        prev[2] = target->anim.worldPosZ;
    }
    path[0] = camera->anim.worldPosX;
    path[1] = camera->anim.worldPosY;
    path[2] = camera->anim.worldPosZ;
    dx = path[0] - prev[0];
    dz = path[2] - prev[2];
    i = 1;
    ang = 0xaaa;
    p = path + 3;
    do
    {
        rad = (3.1415927f * (f32)(s16)ang) / 32768.0f;
        cosv = mathSinf(rad);
        sinv = mathCosf(rad);
        t = dx * sinv - dz * cosv;
        z = t * cosv + dz * sinv;
        z = z + target->anim.worldPosZ;
        p[0] = t + target->anim.worldPosX;
        p[1] = camera->anim.worldPosY;
        p[2] = z;
        rad = (3.1415927f * (f32)(s16)(-i * 0xaaa)) / 32768.0f;
        cosv = mathSinf(rad);
        sinv = mathCosf(rad);
        t = dx * sinv - dz * cosv;
        z = t * cosv + dz * sinv;
        z = z + target->anim.worldPosZ;
        p[3] = t + target->anim.worldPosX;
        p[4] = camera->anim.worldPosY;
        p[5] = z;
        ang = ang + 0x1554;
        p = p + 6;
        i = i + 2;
    } while (i <= 0xc);
    for (j = 0; j <= 0xc; j++)
    {
        endPts[j][0] = prev[0];
        endPts[j][1] = prev[1];
        endPts[j][2] = prev[2];
        radii[j] = 3.9f;
    }
    hitDetect_calcSweptSphereBounds(&bounds, (float*)path, (float*)endPts, radii, 0xd);
    hitDetectFn_800691c0(NULL, &bounds, 0x248, 1);
    trace = camcontrol_traceMove(prev, &camera->anim.worldPosX, NULL, box, 7, '\0', '\0', 3.9f);
    blocked = 0;
    if (trace == 0)
    {
        blocked = 1;
    }
    trace = blocked; /* reused u8 temp: narrowed copy of the blocked flag */
    gCamcontrolModeSettings->collisionBlocked = trace;
    if (trace != 0)
    {
        gCamcontrolModeSettings->wallAvoidanceFlags.b7 = 0;
        if (cameraFn_80103b40(camera, outA, outB, target->anim.rotX) == 0)
        {
            gCamcontrolModeSettings->avoidanceYawOffset = 0.0f;
        }
    }
    if (0.0f != gCamcontrolModeSettings->avoidanceYawOffset)
    {
        spin = (s16)(int)gCamcontrolModeSettings->avoidanceYawOffset;
        if ((spin < -0x1e) || (0x1e < spin))
        {
            f32 rad;

            rad = (3.1415927f * spin) / 32768.0f;
            cosv = mathSinf(rad);
            sinv = mathCosf(rad);
            t = dx * sinv - dz * cosv;
            camera->anim.worldPosX = t + target->anim.worldPosX;
            z = t * cosv + dz * sinv;
            camera->anim.worldPosZ = z + target->anim.worldPosZ;
        }
        gCamcontrolModeSettings->avoidanceYawOffset *= 0.9f;
        if ((gCamcontrolModeSettings->avoidanceYawOffset < 0.5f) && (gCamcontrolModeSettings->avoidanceYawOffset > -0.5f))
        {
            gCamcontrolModeSettings->avoidanceYawOffset = 0.0f;
        }
    }
    Obj_TransformWorldPointToLocal(camera->anim.worldPosX, camera->anim.worldPosY, camera->anim.worldPosZ,
                                   &camera->anim.localPosX, &camera->anim.localPosY, &camera->anim.localPosZ,
                                   camera->anim.parent);
}

void camcontrol_updateModeSettings(int camera)
{
    f32 blend;
    f32 ratio;
    float curve[4];

    if (gCamcontrolModeSettings->transitionTimer != 0)
    {
        gCamcontrolModeSettings->transitionTimer -= framesThisStep;
        if (gCamcontrolModeSettings->transitionTimer < 0)
        {
            gCamcontrolModeSettings->transitionTimer = 0;
        }
        ratio = (f32)(gCamcontrolModeSettings->transitionDuration - gCamcontrolModeSettings->transitionTimer) /
                (f32)(s32)gCamcontrolModeSettings->transitionDuration;
        curve[0] = 0.0f;
        curve[1] = 1.0f;
        curve[2] = 0.0f;
        curve[3] = 0.0f;
        blend = Curve_EvalHermite(curve, ratio, NULL);
        gCamcontrolModeSettings->targetHeight =
            blend * (gCamcontrolModeSettings->targetTargetHeight - gCamcontrolModeSettings->savedTargetHeight) +
            gCamcontrolModeSettings->savedTargetHeight;
        gCamcontrolModeSettings->minDistance = blend * (gCamcontrolModeSettings->targetMinDistance - gCamcontrolModeSettings->savedMinDistance) +
                                      gCamcontrolModeSettings->savedMinDistance;
        gCamcontrolModeSettings->maxDistance = blend * (gCamcontrolModeSettings->targetMaxDistance - gCamcontrolModeSettings->savedMaxDistance) +
                                      gCamcontrolModeSettings->savedMaxDistance;
        gCamcontrolModeSettings->lowerHeightOffset =
            blend * (gCamcontrolModeSettings->targetLowerHeightOffset - gCamcontrolModeSettings->savedLowerHeightOffset) +
            gCamcontrolModeSettings->savedLowerHeightOffset;
        gCamcontrolModeSettings->upperHeightOffset =
            blend * (gCamcontrolModeSettings->targetUpperHeightOffset - gCamcontrolModeSettings->savedUpperHeightOffset) +
            gCamcontrolModeSettings->savedUpperHeightOffset;
        gCamcontrolModeSettings->distanceAdjustRate =
            blend * (gCamcontrolModeSettings->targetDistanceAdjustRate - gCamcontrolModeSettings->savedDistanceAdjustRate) +
            gCamcontrolModeSettings->savedDistanceAdjustRate;
        gCamcontrolModeSettings->heightAdjustRate =
            blend * (gCamcontrolModeSettings->targetHeightAdjustRate - gCamcontrolModeSettings->savedHeightAdjustRate) +
            gCamcontrolModeSettings->savedHeightAdjustRate;
        gCamcontrolModeSettings->slideRightAmount =
            blend * (gCamcontrolModeSettings->targetSlideRightAmount - gCamcontrolModeSettings->savedSlideRightAmount) +
            gCamcontrolModeSettings->savedSlideRightAmount;
        gCamcontrolModeSettings->slideLeftAmount =
            blend * (gCamcontrolModeSettings->targetSlideLeftAmount - gCamcontrolModeSettings->savedSlideLeftAmount) +
            gCamcontrolModeSettings->savedSlideLeftAmount;
        ((CameraObject*)camera)->fov =
            blend * (gCamcontrolModeSettings->fov - gCamcontrolModeSettings->savedFov) + gCamcontrolModeSettings->savedFov;
    }
}

typedef struct CameraModeNormalInitData
{
    u8 pad0;
    s8 transitionFrames;
    s8 fov;
    u8 minDistance;
    u8 maxDistance;
    u8 pad5;
    u8 lowerHeightOffset;
    u8 letterboxOffset;
    u8 upperHeightOffset;
    u8 slideRight;
    u8 slideLeft;
    u8 distanceAdjustRate;
    u8 heightAdjustRate;
    u8 snapToTarget;
    u8 pad0e[0x19 - 0x0e];
    u8 fovWide;
    u16 maxDistanceWide;
    u16 minDistanceWide;
    u8 pad1e;
    u8 heightOffsetWide;
} CameraModeNormalInitData;

void camcontrol_updateVerticalBounds(CameraObject* camera, int flags, int collisionFlag, float* upperBound,
                                     float* lowerBound)
{
    float pt0;
    float wy;
    float diff;
    float bestUpper;
    float bestLower;
    float zLim;
    float zB;
    int res;
    int count;
    int i;
    int j;
    GameObject* camObj;
    int cameraAddr;
    TrackQueryBounds bounds;
    f32 pos[3];
    TrackGroundHit** hits;

    cameraAddr = (int)camera;
    camObj = (GameObject*)((int)camera->anim.targetObj);
    if ((flags & 1) != 0)
    {
        float range = 4.0f;
        *(float*)(cameraAddr + 0x74) = range;
        *(s8*)(cameraAddr + 0x84) = -1;
        *(s8*)(cameraAddr + 0x88) = collisionFlag;
        res =
            objBboxFn_800640cc(&camera->probePosX, &camera->anim.worldPosX, range, 1, NULL, NULL, 0x10,
                               0xffffffff, 0xff, 0);
        camera->cameraCollisionActive = res;
        pos[0] = camera->anim.worldPosX;
        pos[1] = camera->anim.worldPosY;
        pos[2] = camera->anim.worldPosZ;
        hitDetect_calcSweptSphereBounds(&bounds, &camera->probePosX, pos, (float*)(cameraAddr + 0x74), 1);
        hitDetectFn_800691c0(camObj, &bounds, 0x240, 1);
        hitDetectFn_80067958(camObj, &camera->probePosX, pos, 1, &camera->anim.hostedMapSlot, 0);
        camera->anim.worldPosX = pos[0];
        camera->anim.worldPosY = pos[1];
        camera->anim.worldPosZ = pos[2];
    }
    if ((flags & 2) != 0)
    {
        count = hitDetectFn_80065e50(camObj, camera->anim.worldPosX, camera->anim.worldPosY,
                                     camera->anim.worldPosZ, &hits, 1, 0x40);
        *upperBound = -100000.0f;
        *lowerBound = 100000.0f;
        bestUpper = 100000.0f;
        bestLower = 100000.0f;
        zLim = 0.0f;
        for (i = 0; i < count; i++)
        {
            zB = 10.0f;
            if (hits[i]->normalY < zLim)
            {
                pt0 = hits[i]->height;
                wy = camera->anim.worldPosY;
                if (pt0 > wy - zB)
                {
                    diff = wy - pt0;
                    if (diff < zLim)
                    {
                        diff = -diff;
                    }
                    if (diff < bestLower)
                    {
                        *lowerBound = pt0;
                        camera->boundHitZLower = hits[i]->normalY;
                        bestLower = diff;
                    }
                }
            }
        }
        zLim = 0.0f;
        for (j = 0; j < count; j++)
        {
            zB = 10.0f;
            if (hits[j]->normalY > zLim)
            {
                pt0 = hits[j]->height;
                wy = camera->anim.worldPosY;
                if (pt0 < zB + wy)
                {
                    diff = wy - pt0;
                    if (diff < zLim)
                    {
                        diff = -diff;
                    }
                    if (diff < bestUpper)
                    {
                        *upperBound = pt0;
                        camera->boundHitZUpper = hits[j]->normalY;
                        bestUpper = diff;
                    }
                }
            }
        }
    }
    Obj_TransformWorldPointToLocal(camera->anim.worldPosX, camera->anim.worldPosY, camera->anim.worldPosZ,
                                   &camera->anim.localPosX, &camera->anim.localPosY, &camera->anim.localPosZ,
                                   (GameObject*)camera->anim.parentAddress);
}

void CameraModeNormal_func0A(float* minDistanceOut, float* maxDistanceOut, float* lowerHeightOffsetOut,
                             float* upperHeightOffsetOut, float* targetHeightOut)
{
    *minDistanceOut = gCamcontrolModeSettings->minDistance;
    *maxDistanceOut = gCamcontrolModeSettings->maxDistance;
    if (lowerHeightOffsetOut != NULL)
    {
        *lowerHeightOffsetOut = gCamcontrolModeSettings->lowerHeightOffset;
    }
    if (upperHeightOffsetOut != NULL)
    {
        *upperHeightOffsetOut = gCamcontrolModeSettings->upperHeightOffset;
    }
    if (targetHeightOut != NULL)
    {
        *targetHeightOut = gCamcontrolModeSettings->targetHeight;
    }
}

void camslide_update(CameraObject* camera, GameObject* target, f32 upperBound, f32 lowerBound)
{


    CamSlideObjectState* state;
    f32 minHeight;
    u32 angle;
    int slideAngleCur;
    f32 upperY;
    f32 lowerY;
    f32 minDistSpan;
    f32 slideOffset;
    f64 approach;
    f32 mtx[16];
    CamSlideRot rot;
    f32 relX;
    f32 step;
    f32 relZ;
    f32 dist;
    f32 outX;
    f32 outY;
    f32 outZ;

    (*gCameraInterface)
        ->getRelativePosition(camera, &relX, &step, &relZ, &dist, gCamcontrolModeSettings->targetHeight, 0);
    dist = relZ * relZ + (relX * relX + step * step);
    if (dist > 0.0f)
    {
        dist = sqrtf(dist);
    }
    if (dist < 5.0f)
    {
        dist = 5.0f;
    }
    upperY =
        gCamcontrolModeSettings->upperHeightOffset + (target->anim.worldPosY + gCamcontrolModeSettings->targetHeight);
    lowerY = gCamcontrolModeSettings->lowerHeightOffset + (target->anim.worldPosY + gCamcontrolModeSettings->targetHeight);
    if (target->anim.classId == 1)
    {
        state = (CamSlideObjectState*)target->extra;
        angle = getAngle((f64)relX, relZ);
        rot.angles[0] = (s16)(0x8000 - angle);
        rot.angles[1] = 0;
        rot.angles[2] = 0;
        rot.scale = 1.0f;
        rot.transX = 0.0f;
        rot.transY = 0.0f;
        rot.transZ = 0.0f;
        mtxRotateByVec3s(mtx, rot.angles);
        Matrix_TransformPoint(mtx, state->vectorX, state->vectorY, state->vectorZ, &outX, &outY, &outZ);
        angle = 0x4000 - (getAngle((f64)outY, outZ) & 0xffff);
        gCamcontrolModeSettings->slideAngle +=
            (int)(framesThisStep * ((int)angle - gCamcontrolModeSettings->slideAngle)) >> 5;
    }
    else
    {
        gCamcontrolModeSettings->slideAngle -= (int)(gCamcontrolModeSettings->slideAngle * framesThisStep) >> 5;
    }
    slideAngleCur = gCamcontrolModeSettings->slideAngle;
    if (slideAngleCur < 0)
    {
        slideOffset = gCamcontrolModeSettings->slideLeftAmount * mathSinf((3.1415927f * slideAngleCur) / 32768.0f);
    }
    else if (slideAngleCur > 0)
    {
        slideOffset = gCamcontrolModeSettings->slideRightAmount * mathSinf((3.1415927f * slideAngleCur) / 32768.0f);
    }
    else
    {
        slideOffset = 0.0f;
    }
    lowerY += slideOffset;
    upperY += slideOffset;
    minDistSpan = gCamcontrolModeSettings->minDistance - 25.0f;
    if (minDistSpan < 30.0f)
    {
        minDistSpan = 30.0f;
    }
    if (target->anim.classId == 1)
    {
        if (playerGetProbeHitDist((GameObject*)(target)) <= 30.0f)
        {
            step = 0.8f * gCamcontrolModeSettings->maxDistance - gCamcontrolModeSettings->lowerHeightOffset;
            step *= 0.05f;
            if (step > 10.0f)
            {
                step = 10.0f;
            }
            gCamcontrolModeSettings->lowerHeightOffset = gCamcontrolModeSettings->lowerHeightOffset + step;
            if (gCamcontrolModeSettings->lowerHeightOffset > gCamcontrolModeSettings->maxDistance)
            {
                gCamcontrolModeSettings->lowerHeightOffset = gCamcontrolModeSettings->maxDistance;
            }
            step = 0.8f * gCamcontrolModeSettings->maxDistance - gCamcontrolModeSettings->upperHeightOffset;
            step *= 0.05f;
            if (step > 10.0f)
            {
                step = 10.0f;
            }
            gCamcontrolModeSettings->upperHeightOffset = gCamcontrolModeSettings->upperHeightOffset + step;
            if (gCamcontrolModeSettings->upperHeightOffset > gCamcontrolModeSettings->maxDistance)
            {
                gCamcontrolModeSettings->upperHeightOffset = gCamcontrolModeSettings->maxDistance;
            }
        }
        else
        {
            step = gCamcontrolModeSettings->baseLowerHeightOffset - gCamcontrolModeSettings->lowerHeightOffset;
            step *= 0.05f;
            if (step > -0.1f)
            {
                step = -0.1f;
            }
            if (step < -10.0f)
            {
                step = -10.0f;
            }
            gCamcontrolModeSettings->lowerHeightOffset = gCamcontrolModeSettings->lowerHeightOffset + step;
            if (gCamcontrolModeSettings->lowerHeightOffset < gCamcontrolModeSettings->baseLowerHeightOffset)
            {
                gCamcontrolModeSettings->lowerHeightOffset = gCamcontrolModeSettings->baseLowerHeightOffset;
            }
            step = gCamcontrolModeSettings->baseUpperHeightOffset - gCamcontrolModeSettings->upperHeightOffset;
            step *= 0.05f;
            if (step > -0.1f)
            {
                step = -0.1f;
            }
            if (step < -10.0f)
            {
                step = -10.0f;
            }
            gCamcontrolModeSettings->upperHeightOffset = gCamcontrolModeSettings->upperHeightOffset + step;
            if (gCamcontrolModeSettings->upperHeightOffset < gCamcontrolModeSettings->baseUpperHeightOffset)
            {
                gCamcontrolModeSettings->upperHeightOffset = gCamcontrolModeSettings->baseUpperHeightOffset;
            }
            if (dist > 30.0f)
            {
                if (dist <= minDistSpan)
                {
                    f32 d = minDistSpan - 30.0f;
                    if (d > 0.0f)
                    {
                        dist = (dist - 30.0f) / d;
                    }
                    if (dist < 0.0f)
                    {
                        dist = 0.0f;
                    }
                    else if (dist > 1.0f)
                    {
                        dist = 1.0f;
                    }
                    lowerY =
                        dist * ((gCamcontrolModeSettings->targetHeight + gCamcontrolModeSettings->lowerHeightOffset) -
                                 35.0f) +
                        (35.0f + target->anim.worldPosY);
                    upperY =
                        dist * ((gCamcontrolModeSettings->targetHeight + gCamcontrolModeSettings->upperHeightOffset) -
                                 (minHeight = 35.0f)) +
                        (35.0f + target->anim.worldPosY);
                }
            }
            else
            {
                upperY = 0.8f * (30.0f - dist) + (35.0f + target->anim.worldPosY);
                lowerY = upperY;
            }
        }
    }
    if (camera->anim.worldPosY < lowerY)
    {
        step = lowerY - camera->anim.worldPosY;
    }
    else if (camera->anim.worldPosY > upperY)
    {
        step = upperY - camera->anim.worldPosY;
    }
    else
    {
        step = 0.0f;
    }
    approach = step = interpolate((f64)step, gCamcontrolModeSettings->heightAdjustRate, timeDelta);
    if ((f32)approach > -0.1f && (f32)approach < 0.1f)
    {
        step = 0.0f;
    }
    camera->anim.worldPosY = camera->anim.worldPosY + step;
    if (camera->anim.worldPosY > 100.0f + upperY)
    {
        camera->anim.worldPosY = 100.0f + upperY;
    }
    if (gCamcontrolModeSettings->upperHeightOffset > gCamcontrolModeSettings->baseUpperHeightOffset)
    {
        if (gCamcontrolModeSettings->clampFlags.b6 && camera->anim.worldPosY > gCamcontrolModeSettings->heightLockLimit)
        {
            camera->anim.worldPosY = gCamcontrolModeSettings->heightLockLimit;
        }
        if (target->anim.velocityY > 0.0f)
        {
            gCamcontrolModeSettings->clampFlags.b6 = 0;
        }
    }
    else
    {
        gCamcontrolModeSettings->clampFlags.b6 = 0;
    }
}

void firstperson_updatePitch(f32 targetY, f32 dist, CameraObject* camera)
{
    int pitchDelta;

    pitchDelta =
        getAngle((f64)(camera->anim.worldPosY - (targetY + gCamcontrolModeSettings->targetHeight)), dist) & 0xffff;
    pitchDelta -= camera->anim.rotY & 0xffff;
    if (pitchDelta > 0x8000)
    {
        pitchDelta -= 0xffff;
    }
    if (pitchDelta < -0x8000)
    {
        pitchDelta += 0xffff;
    }
    camera->anim.rotY =
        (s16)(camera->anim.rotY + (int)interpolate((f64)(f32)pitchDelta,
                                                   (f64)(1.0f / gCamcontrolModeSettings->yawResponseFrames),
                                                   timeDelta));
}

void CameraModeNormal_follow(CameraObject* camera, ObjAnimComponent* target)
{

    f32 dx;
    f32 dz;
    f32 dy;
    f32 dist;
    f32 clamped;
    f32 targetX;
    f32 targetZ;
    f32 ratio;
    f32 speed;

    (*gCameraInterface)->getRelativePosition(camera, &dx, &dz, &dy, &dist, gCamcontrolModeSettings->targetHeight, 1);
    dist = dy * dy + (dx * dx + dz * dz);
    if (dist > 0.0f)
    {
        dist = sqrtf(dist);
    }
    if (dist < 5.0f)
    {
        dist = 5.0f;
    }
    if (dist > 2.0f * gCamcontrolModeSettings->maxDistance)
    {
        camcontrol_getTargetPosition(camera, target, &camera->anim.worldPosX, &camera->anim.rotY);
        Obj_TransformWorldPointToLocal(camera->anim.worldPosX, camera->anim.worldPosY, camera->anim.worldPosZ,
                                       &camera->anim.localPosX, &camera->anim.localPosY, &camera->anim.localPosZ,
                                       camera->anim.parent);
        camera->probePosX = camera->anim.worldPosX;
        camera->probePosY = camera->anim.worldPosY;
        camera->probePosZ = camera->anim.worldPosZ;
        (*gCameraInterface)
            ->getRelativePosition(camera, &dx, &dz, &dy, &dist, gCamcontrolModeSettings->targetHeight, 1);
        dist = dy * dy + (dx * dx + dz * dz);
        if (dist > 0.0f)
        {
            dist = sqrtf(dist);
        }
        if (dist < 5.0f)
        {
            dist = 5.0f;
        }
    }

    if (dist > gCamcontrolModeSettings->maxDistance)
    {
        clamped = gCamcontrolModeSettings->maxDistance;
        gCamcontrolModeSettings->wallAvoidanceFlags.b7 = 0;
        gCamcontrolModeSettings->clampFlags.b7 = 1;
    }
    else if (dist < gCamcontrolModeSettings->minDistance)
    {
        clamped = gCamcontrolModeSettings->minDistance;
        gCamcontrolModeSettings->clampFlags.b7 = 0;
    }
    else
    {
        clamped = dist;
        gCamcontrolModeSettings->clampFlags.b7 = 0;
    }

    targetX = camera->anim.localPosX;
    targetZ = camera->anim.localPosZ;
    if ((gCamcontrolModeSettings->wallAvoidanceFlags.b7 == 0) && (clamped != dist) &&
        (0.0f != gCamcontrolModeSettings->distanceAdjustRate))
    {
        if (dist < 1.0f)
        {
            dist = 1.0f;
        }
        ratio = interpolate(dist - clamped, gCamcontrolModeSettings->distanceAdjustRate, timeDelta);
        ratio = (dist + ratio) / dist;
        if (ratio > 0.0f)
        {
            targetX = target->localPosX + dx / ratio;
            targetZ = target->localPosZ + dy / ratio;
        }
    }

    dx = targetX - camera->anim.localPosX;
    dy = targetZ - camera->anim.localPosZ;
    dist = sqrtf(dx * dx + dy * dy);
    if (dist > 0.0f)
    {
        dx = dx / dist;
        dy = dy / dist;
    }
    ratio = PSVECMag(&target->velocity);
    speed = 1.5f * timeDelta;
    speed = ratio * speed;
    if (speed < 1.0f)
    {
        speed = 1.0f;
    }
    dist = dist < 0.0f ? 0.0f : (dist > speed ? speed : dist);
    dist = dist < 0.0f ? 0.0f : (dist > 20.0f ? 20.0f : dist);
    camera->anim.localPosX = dx * dist + camera->anim.localPosX;
    camera->anim.localPosZ = dy * dist + camera->anim.localPosZ;

    if (gCamcontrolModeSettings->upperHeightOffset > gCamcontrolModeSettings->baseUpperHeightOffset)
    {
        dx = camera->anim.localPosX - target->localPosX;
        dy = camera->anim.localPosZ - target->localPosZ;
        dist = sqrtf(dx * dx + dy * dy);
        if (dist < 0.25f * gCamcontrolModeSettings->minDistance)
        {
            if (dist > 0.0f)
            {
                dx = dx / dist;
                dy = dy / dist;
            }
            dist = 0.25f * gCamcontrolModeSettings->minDistance;
            camera->anim.localPosX = dist * dx + target->localPosX;
            camera->anim.localPosZ = dist * dy + target->localPosZ;
        }
    }
}

void CameraModeNormal_copyToCurrent(CamcontrolFirstPersonActionSettings* settings)
{
    float fval;
    CameraObject* camera;

    camera = (CameraObject*)(*gCameraInterface)->getCamera();
    gCamcontrolModeSettings->savedTargetHeight = gCamcontrolModeSettings->targetHeight;
    gCamcontrolModeSettings->savedLowerHeightOffset = gCamcontrolModeSettings->lowerHeightOffset;
    gCamcontrolModeSettings->savedUpperHeightOffset = gCamcontrolModeSettings->upperHeightOffset;
    gCamcontrolModeSettings->savedMinDistance = gCamcontrolModeSettings->minDistance;
    gCamcontrolModeSettings->savedMaxDistance = gCamcontrolModeSettings->maxDistance;
    gCamcontrolModeSettings->savedFov = camera->fov;
    gCamcontrolModeSettings->savedSlideRightAmount = gCamcontrolModeSettings->slideRightAmount;
    gCamcontrolModeSettings->savedSlideLeftAmount = gCamcontrolModeSettings->slideLeftAmount;
    gCamcontrolModeSettings->savedHeightAdjustRate = gCamcontrolModeSettings->heightAdjustRate;
    gCamcontrolModeSettings->savedDistanceAdjustRate = gCamcontrolModeSettings->distanceAdjustRate;
    fval = settings->targetHeight;
    gCamcontrolModeSettings->targetHeight = fval;
    gCamcontrolModeSettings->targetTargetHeight = fval;
    fval = (f32)(u32)settings->lowerHeightOffset;
    gCamcontrolModeSettings->lowerHeightOffset = fval;
    gCamcontrolModeSettings->baseLowerHeightOffset = fval;
    gCamcontrolModeSettings->targetLowerHeightOffset = fval;
    fval = (f32)(u32)settings->upperHeightOffset;
    gCamcontrolModeSettings->upperHeightOffset = fval;
    gCamcontrolModeSettings->baseUpperHeightOffset = fval;
    gCamcontrolModeSettings->targetUpperHeightOffset = fval;
    fval = (f32)(u32)settings->minDistance;
    gCamcontrolModeSettings->minDistance = fval;
    gCamcontrolModeSettings->targetMinDistance = fval;
    fval = (f32)(u32)settings->maxDistance;
    gCamcontrolModeSettings->maxDistance = fval;
    gCamcontrolModeSettings->targetMaxDistance = fval;
    fval = settings->fov;
    camera->fov = fval;
    gCamcontrolModeSettings->fov = fval;
    fval = (f32)(u32)settings->slideRightAmount;
    gCamcontrolModeSettings->slideRightAmount = fval;
    gCamcontrolModeSettings->targetSlideRightAmount = fval;
    fval = (f32)(u32)settings->slideLeftAmount;
    gCamcontrolModeSettings->slideLeftAmount = fval;
    gCamcontrolModeSettings->targetSlideLeftAmount = fval;
    if (settings->distanceAdjustRate != 0)
    {
        fval = (f32)(u32)settings->distanceAdjustRate / 255.0f;
        gCamcontrolModeSettings->distanceAdjustRate = fval;
        gCamcontrolModeSettings->targetDistanceAdjustRate = fval;
    }
    else
    {
        gCamcontrolModeSettings->targetDistanceAdjustRate = 0.09f;
    }
    if (settings->heightAdjustRate != 0)
    {
        fval = (f32)(u32)settings->heightAdjustRate / 255.0f;
        gCamcontrolModeSettings->heightAdjustRate = fval;
        gCamcontrolModeSettings->targetHeightAdjustRate = fval;
    }
    else
    {
        gCamcontrolModeSettings->targetHeightAdjustRate = 0.09f;
    }
    gCamcontrolModeSettings->transitionTimer = 0;
    gCamcontrolModeSettings->transitionDuration = 0;
}

void CameraModeNormal_free(CameraObject* camera)
{
    gCamcontrolModeSettings->savedWorldX = camera->anim.worldPosX;
    gCamcontrolModeSettings->savedWorldY = camera->anim.worldPosY;
    gCamcontrolModeSettings->savedWorldZ = camera->anim.worldPosZ;
    gCamcontrolModeSettings->savedRotX = camera->anim.rotX;
    gCamcontrolModeSettings->savedRotY = camera->anim.rotY;
    gCamcontrolModeSettings->savedRotZ = camera->anim.rotZ;
    gCamcontrolModeSettings->wallAvoidanceFlags.b6 = 0;
}

void CameraModeNormal_update(CameraObject* camera)
{
    GameObject* target[1];
    float fa;
    int val;
    u32 angleDelta;
    int yaw;
    float aimZ2;
    float aimY2;
    float aimX2;
    float aimZ;
    float aimY;
    float aimX;
    float dx2;
    u8 relPosScratch[4];
    float dz;
    float dy;
    float dx;
    u8 wallTraceScratch[116];
    u8 probeTraceScratch[112];

    target[0] = (GameObject*)camera->anim.targetObj;
    if (target[0] == NULL)
    {
        return;
    }
    if (target[0]->anim.classId == 1)
    {
        playerGetTimeScale((GameObject*)target[0], &dx);
        gCamcontrolScaledTimeDelta = timeDelta * dx;
        val = EmissionController_IsLingering((GameObject*)target[0]);
        switch (val)
        {
        case 1:
            gCamcontrolModeSettings->heightAdjustRate = 0.0f;
            gCamcontrolModeSettings->yawResponseFrames = 0xff;
            break;
        case 2:
            gCamcontrolModeSettings->heightAdjustRate = 0.008f;
            gCamcontrolModeSettings->yawResponseFrames = 0xc;
            break;
        case 4:
            gCamcontrolModeSettings->heightAdjustRate = 0.2f;
            gCamcontrolModeSettings->yawResponseFrames = 2;
            break;
        case 3:
            gCamcontrolModeSettings->heightAdjustRate = 0.055f;
            gCamcontrolModeSettings->yawResponseFrames = 8;
            break;
        default:
            gCamcontrolModeSettings->heightAdjustRate = gCamcontrolModeSettings->targetHeightAdjustRate;
            gCamcontrolModeSettings->yawResponseFrames = 8;
            break;
        }
    }
    else
    {
        gCamcontrolScaledTimeDelta = timeDelta;
    }
    camera->unk13E = 0;
    camcontrol_updateModeSettings((int)camera);
    camcontrol_updateWallAvoidance(camera, target[0]);
    CameraModeNormal_follow(camera, &target[0]->anim);
    Obj_TransformLocalPointToWorld(camera->anim.localPosX, camera->anim.localPosY, camera->anim.localPosZ,
                                   &camera->anim.worldPosX, &camera->anim.worldPosY, &camera->anim.worldPosZ,
                                   camera->anim.parent);
    camslide_update(camera, target[0], gCamcontrolModeSettings->verticalUpperBound,
                    gCamcontrolModeSettings->verticalLowerBound);
    camcontrol_updateVerticalBounds(camera, 1, 8, &gCamcontrolModeSettings->verticalUpperBound,
                                    &gCamcontrolModeSettings->verticalLowerBound);
    if (gCamcontrolModeSettings->wallAvoidanceFlags.b7 == 0)
    {
        gCamcontrolModeSettings->targetActionFlags = *(u8*)((int)camera + 0xa2);
        if (((camera->cameraCollisionActive != 0) ||
             ((gCamcontrolModeSettings->targetActionFlags == 1 && (*(f32*)((u8*)camera + 0x38) >= 0.0f)))) &&
            (gCamcontrolModeSettings->clampFlags.b7 == 0))
        {
            if (((camera->anim.worldPosY > 30.0f + target[0]->anim.worldPosY) &&
                 (camera->anim.worldPosY < 70.0f + target[0]->anim.worldPosY)) &&
                (camera->anim.parent == NULL))
            {
                gCamcontrolModeSettings->wallAvoidanceFlags.b7 = 1;
            }
        }
        if ((((gCamcontrolModeSettings->targetActionFlags & 0x10) != 0) &&
             (*(f32*)((u8*)camera + 0x38) < -0.707f)) &&
            (target[0]->anim.velocityY <= 0.0f))
        {
            gCamcontrolModeSettings->clampFlags.b6 = 1;
            gCamcontrolModeSettings->heightLockLimit = camera->anim.worldPosY;
        }
    }
    else
    {
        fa = 0.0f;
        camera->boundHitZUpper = fa;
        camera->boundHitZLower = fa;
        if ((*(u8*)((int)camera + 0xa2) == 1) && (*(f32*)((u8*)camera + 0x38) < fa))
        {
            gCamcontrolModeSettings->wallAvoidanceFlags.b7 = 0;
        }
        if ((camera->anim.worldPosY > 75.0f + target[0]->anim.worldPosY) ||
            (camera->anim.worldPosY < 20.0f + target[0]->anim.worldPosY))
        {
            gCamcontrolModeSettings->wallAvoidanceFlags.b7 = 0;
        }
    }
    if (gCamcontrolModeSettings->clampFlags.b7 != 0)
    {
        if ((gCamcontrolModeSettings->targetActionFlags == 1) || (camera->cameraCollisionActive != 0))
        {
            gCamcontrolModeSettings->wallAvoidanceTimer += 1;
        }
        else
        {
            gCamcontrolModeSettings->wallAvoidanceTimer = 0;
        }
        if (10 < gCamcontrolModeSettings->wallAvoidanceTimer)
        {
            if (target[0]->anim.classId == 1)
            {
                cameraGetPrevPos2(target[0], &aimX2, &aimY2, &aimZ2);
            }
            else
            {
                aimX2 = target[0]->anim.worldPosX;
                aimY2 = target[0]->anim.worldPosY + gCamcontrolModeSettings->targetHeight;
                aimZ2 = target[0]->anim.worldPosZ;
            }
            camcontrol_traceMove(&aimX2, &camera->anim.worldPosX, &camera->anim.worldPosX, wallTraceScratch, 3, 1, 1,
                                 4.0f);
            camera->probePosX = camera->anim.worldPosX;
            camera->probePosY = camera->anim.worldPosY;
            camera->probePosZ = camera->anim.worldPosZ;
            gCamcontrolModeSettings->wallAvoidanceTimer = 0;
        }
    }
    if (gCamcontrolModeSettings->wallAvoidanceFlags.b7 == 0)
    {
        if ((gCamcontrolModeSettings->targetActionFlags & 0x10) != 0)
        {
            gCamcontrolModeSettings->collisionProbeTimer += 1;
        }
        else
        {
            gCamcontrolModeSettings->collisionProbeTimer = 0;
        }
        if (5 < gCamcontrolModeSettings->collisionProbeTimer)
        {
            if (target[0]->anim.classId == 1)
            {
                cameraGetPrevPos2(target[0], &aimX, &aimY, &aimZ);
            }
            else
            {
                aimX = target[0]->anim.worldPosX;
                aimY = target[0]->anim.worldPosY + gCamcontrolModeSettings->targetHeight;
                aimZ = target[0]->anim.worldPosZ;
            }
            camcontrol_traceMove(&aimX, &camera->anim.worldPosX, &camera->anim.worldPosX, probeTraceScratch, 3, 1, 1,
                                 4.0f);
            camera->probePosX = camera->anim.worldPosX;
            camera->probePosY = camera->anim.worldPosY;
            camera->probePosZ = camera->anim.worldPosZ;
            gCamcontrolModeSettings->collisionProbeTimer = 0;
        }
    }
    (*gCameraInterface)
        ->getRelativePosition(camera, &dx2, (f32*)relPosScratch, &dz, &dy, gCamcontrolModeSettings->targetHeight, 0);
    yaw = 0x8000 - (u16)getAngle(dx2, dz);
    gCamcontrolModeSettings->pitchOffset = 0;
    camera->anim.rotX = yaw - gCamcontrolModeSettings->pitchOffset;
    angleDelta =
        0xffffu &
        getAngle(camera->anim.worldPosY - (target[0]->anim.worldPosY + gCamcontrolModeSettings->targetHeight), dy);
    angleDelta = angleDelta - ((int)camera->anim.rotY & 0xffffU);
    if (0x8000 < (int)angleDelta)
    {
        angleDelta = angleDelta - 0xffff;
    }
    if ((int)angleDelta < -0x8000)
    {
        angleDelta = angleDelta + 0xffff;
    }
    val = interpolate((f32)(int)angleDelta, 1.0f / (f32)(u32)gCamcontrolModeSettings->yawResponseFrames,
                      timeDelta);
    camera->anim.rotY = camera->anim.rotY + val;
    camcontrol_updateTargetAction(camera, target[0]);
    val = interpolate((f32)camera->anim.rotZ, 0.125f, timeDelta);
    camera->anim.rotZ = camera->anim.rotZ - val;
    Obj_TransformWorldPointToLocal(camera->anim.worldPosX, camera->anim.worldPosY, camera->anim.worldPosZ,
                                   &camera->anim.localPosX, &camera->anim.localPosY, &camera->anim.localPosZ,
                                   camera->anim.parent);
}

void CameraModeNormal_init(CameraObject* cam, int mode, u8* data)
{
    GameObject* target;
    f32 vOutA;
    f32 vOutB;
    f32 vOutC;
    f32 vOutD;
    f32 fVal;
    u32 uVal;
    CameraModeNormalInitData* p = (CameraModeNormalInitData*)data;

    gCamcontrolModeSettings->wallAvoidanceFlags.b7 = 0;
    gCamcontrolModeSettings->collisionState = 0;
    gCamcontrolModeSettings->collisionProbeTimer = 0;
    gCamcontrolModeSettings->wallAvoidanceTimer = 0;
    gCamcontrolModeSettings->clampFlags.b7 = 0;
    gCamcontrolModeSettings->yawResponseFrames = 8;
    target = (GameObject*)cam->anim.targetObj;
    switch (mode)
    {
    case 0:
        memset(gCamcontrolModeSettings, 0, sizeof(CamcontrolModeSettings));
        if (data != NULL)
        {
            fVal = (f32)(u32)p->minDistanceWide;
            gCamcontrolModeSettings->minDistance = fVal;
            gCamcontrolModeSettings->targetMinDistance = fVal;
            fVal = (f32)(u32)p->maxDistanceWide;
            gCamcontrolModeSettings->maxDistance = fVal;
            gCamcontrolModeSettings->targetMaxDistance = fVal;
            fVal = (f32)(u32)p->heightOffsetWide;
            gCamcontrolModeSettings->baseLowerHeightOffset = fVal;
            gCamcontrolModeSettings->lowerHeightOffset = fVal;
            gCamcontrolModeSettings->targetLowerHeightOffset = fVal;
            fVal = (f32)(u32)p->heightOffsetWide;
            gCamcontrolModeSettings->baseUpperHeightOffset = fVal;
            gCamcontrolModeSettings->upperHeightOffset = fVal;
            gCamcontrolModeSettings->targetUpperHeightOffset = fVal;
        }
        fVal = 35.0f;
        gCamcontrolModeSettings->targetHeight = fVal;
        gCamcontrolModeSettings->targetTargetHeight = fVal;
        fVal = 0.09f;
        gCamcontrolModeSettings->distanceAdjustRate = fVal;
        gCamcontrolModeSettings->targetDistanceAdjustRate = fVal;
        fVal = 0.04f;
        gCamcontrolModeSettings->savedHeightAdjustRate = fVal;
        gCamcontrolModeSettings->heightAdjustRate = fVal;
        gCamcontrolModeSettings->targetHeightAdjustRate = fVal;
        fVal = 50.0f;
        gCamcontrolModeSettings->slideRightAmount = fVal;
        gCamcontrolModeSettings->targetSlideRightAmount = fVal;
        fVal = 30.0f;
        gCamcontrolModeSettings->slideLeftAmount = fVal;
        gCamcontrolModeSettings->targetSlideLeftAmount = fVal;
        gCamcontrolModeSettings->pad24 = -100000.0f;
        gCamcontrolModeSettings->pad20 = 100000.0f;
        gCamcontrolModeSettings->initialized = 1;
        gCamcontrolModeSettings->fov = cam->fov;
        camcontrol_getTargetPosition(cam, &target->anim, &cam->anim.worldPosX, &cam->anim.rotY);
        fVal = cam->anim.worldPosX;
        cam->anim.localPosX = fVal;
        cam->probePosX = fVal;
        cam->savedLocalPos.x = fVal;
        fVal = cam->anim.worldPosY;
        cam->anim.localPosY = fVal;
        cam->probePosY = fVal;
        cam->savedLocalPos.y = fVal;
        fVal = cam->anim.worldPosZ;
        cam->anim.localPosZ = fVal;
        cam->probePosZ = fVal;
        cam->savedLocalPos.z = fVal;
        cam->anim.rotX = 0;
        cam->anim.rotZ = 0;
        if (data != NULL)
        {
            cam->fov = (f32)(u32)p->fovWide;
        }
        break;
    case 4:
        camcontrol_getTargetPosition(cam, &target->anim, &cam->anim.worldPosX, &cam->anim.rotY);
        Obj_TransformWorldPointToLocal(cam->anim.worldPosX, cam->anim.worldPosY, cam->anim.worldPosZ,
                                       &cam->anim.localPosX, &cam->anim.localPosY, &cam->anim.localPosZ,
                                       (GameObject*)cam->anim.parentAddress);
        (*gCameraInterface)
            ->getRelativePosition(cam, &vOutA, &vOutB, &vOutC, &vOutD, gCamcontrolModeSettings->targetHeight, 0);
        vOutB = cam->anim.localPosY - (target->anim.localPosY + gCamcontrolModeSettings->targetHeight);
        cam->anim.rotY = getAngle(vOutB, vOutD);
        cam->anim.rotZ = 0;
        cam->probePosX = cam->anim.worldPosX;
        cam->probePosY = cam->anim.worldPosY;
        cam->probePosZ = cam->anim.worldPosZ;
        cam->savedLocalPos.x = cam->anim.localPosX;
        cam->savedLocalPos.y = cam->anim.localPosY;
        cam->savedLocalPos.z = cam->anim.localPosZ;
        cam->fov = gCamcontrolModeSettings->fov;
        gCamcontrolModeSettings->transitionTimer = 0;
        break;
    case 2:
        if (data != NULL)
        {
            gCamcontrolModeSettings->targetTargetHeight = 35.0f;
            fVal = (f32)(u32)p->lowerHeightOffset;
            gCamcontrolModeSettings->baseLowerHeightOffset = fVal;
            gCamcontrolModeSettings->targetLowerHeightOffset = fVal;
            fVal = (f32)(u32)p->upperHeightOffset;
            gCamcontrolModeSettings->baseUpperHeightOffset = fVal;
            gCamcontrolModeSettings->targetUpperHeightOffset = fVal;
            gCamcontrolModeSettings->targetMinDistance = (f32)(u32)p->minDistance;
            gCamcontrolModeSettings->targetMaxDistance = (f32)(u32)p->maxDistance;
            gCamcontrolModeSettings->fov = p->fov;
            gCamcontrolModeSettings->targetSlideRightAmount = (f32)(u32)p->slideRight;
            gCamcontrolModeSettings->targetSlideLeftAmount = (f32)(u32)p->slideLeft;
            uVal = p->distanceAdjustRate;
            if (uVal != 0)
            {
                gCamcontrolModeSettings->targetDistanceAdjustRate = uVal / 255.0f;
            }
            else
            {
                gCamcontrolModeSettings->targetDistanceAdjustRate = 0.09f;
            }
            uVal = p->heightAdjustRate;
            if (uVal != 0)
            {
                gCamcontrolModeSettings->targetHeightAdjustRate = uVal / 255.0f;
            }
            else
            {
                gCamcontrolModeSettings->targetHeightAdjustRate = 0.09f;
            }
            gCamcontrolModeSettings->transitionTimer = (s16)p->transitionFrames;
            gCamcontrolModeSettings->transitionDuration = (s16)p->transitionFrames;
            *(u8*)&cam->letterboxTargetOffset = p->letterboxOffset;
        }
        else
        {
            gCamcontrolModeSettings->targetTargetHeight = gCamcontrolModeSettings->savedTargetHeight;
            fVal = gCamcontrolModeSettings->savedLowerHeightOffset;
            gCamcontrolModeSettings->baseLowerHeightOffset = fVal;
            gCamcontrolModeSettings->targetLowerHeightOffset = fVal;
            fVal = gCamcontrolModeSettings->savedUpperHeightOffset;
            gCamcontrolModeSettings->baseUpperHeightOffset = fVal;
            gCamcontrolModeSettings->targetUpperHeightOffset = fVal;
            gCamcontrolModeSettings->targetMinDistance = gCamcontrolModeSettings->savedMinDistance;
            gCamcontrolModeSettings->targetMaxDistance = gCamcontrolModeSettings->savedMaxDistance;
            gCamcontrolModeSettings->fov = gCamcontrolModeSettings->savedFov;
            gCamcontrolModeSettings->targetSlideRightAmount = gCamcontrolModeSettings->savedSlideRightAmount;
            gCamcontrolModeSettings->targetSlideLeftAmount = gCamcontrolModeSettings->savedSlideLeftAmount;
            gCamcontrolModeSettings->targetDistanceAdjustRate = gCamcontrolModeSettings->savedDistanceAdjustRate;
            gCamcontrolModeSettings->targetHeightAdjustRate = gCamcontrolModeSettings->savedHeightAdjustRate;
            gCamcontrolModeSettings->transitionTimer = 0x3c;
            gCamcontrolModeSettings->transitionDuration = 0x3c;
        }
        gCamcontrolModeSettings->savedTargetHeight = gCamcontrolModeSettings->targetHeight;
        gCamcontrolModeSettings->savedLowerHeightOffset = gCamcontrolModeSettings->lowerHeightOffset;
        gCamcontrolModeSettings->savedUpperHeightOffset = gCamcontrolModeSettings->upperHeightOffset;
        gCamcontrolModeSettings->savedMinDistance = gCamcontrolModeSettings->minDistance;
        gCamcontrolModeSettings->savedMaxDistance = gCamcontrolModeSettings->maxDistance;
        gCamcontrolModeSettings->savedFov = cam->fov;
        gCamcontrolModeSettings->savedSlideRightAmount = gCamcontrolModeSettings->slideRightAmount;
        gCamcontrolModeSettings->savedSlideLeftAmount = gCamcontrolModeSettings->slideLeftAmount;
        gCamcontrolModeSettings->savedDistanceAdjustRate = gCamcontrolModeSettings->distanceAdjustRate;
        gCamcontrolModeSettings->savedHeightAdjustRate = gCamcontrolModeSettings->heightAdjustRate;
        if ((data != NULL) && (p->snapToTarget != 0))
        {
            camcontrol_getTargetPosition(cam, &target->anim, &cam->anim.worldPosX, &cam->anim.rotY);
            Obj_TransformWorldPointToLocal(cam->anim.worldPosX, cam->anim.worldPosY, cam->anim.worldPosZ,
                                           &cam->anim.localPosX, &cam->anim.localPosY, &cam->anim.localPosZ,
                                           (GameObject*)cam->anim.parentAddress);
            gCamcontrolModeSettings->transitionTimer = 0;
        }
        break;
    case 3:
        cam->fov = gCamcontrolModeSettings->fov;
        cam->anim.worldPosX = gCamcontrolModeSettings->savedWorldX;
        cam->anim.worldPosY = gCamcontrolModeSettings->savedWorldY;
        cam->anim.worldPosZ = gCamcontrolModeSettings->savedWorldZ;
        Obj_TransformWorldPointToLocal(cam->anim.worldPosX, cam->anim.worldPosY, cam->anim.worldPosZ,
                                       &cam->anim.localPosX, &cam->anim.localPosY, &cam->anim.localPosZ,
                                       (GameObject*)cam->anim.parentAddress);
        cam->anim.rotX = gCamcontrolModeSettings->savedRotX;
        cam->anim.rotY = gCamcontrolModeSettings->savedRotY;
        cam->anim.rotZ = gCamcontrolModeSettings->savedRotZ;
        cam->savedLocalPos.x = cam->anim.localPosX;
        cam->savedLocalPos.y = cam->anim.localPosY;
        cam->savedLocalPos.z = cam->anim.localPosZ;
        cam->probePosX = cam->anim.worldPosX;
        cam->probePosY = cam->anim.worldPosY;
        cam->probePosZ = cam->anim.worldPosZ;
        gCamcontrolModeSettings->transitionTimer = 0;
        break;
    case 1:
        cam->fov = gCamcontrolModeSettings->fov;
        gCamcontrolModeSettings->wallAvoidanceFlags.b7 = gCamcontrolModeSettings->wallAvoidanceFlags.b6;
        break;
    }
    gCamcontrolModeSettings->wallAvoidanceFlags.b6 = 0;
    cam->unk13E = 1;
}

void CameraModeNormal_release(void)
{
    mm_free(gCamcontrolModeSettings);
    gCamcontrolModeSettings = 0;
}

void CameraModeNormal_initialise(void)
{
    gCamcontrolModeSettings = (CamcontrolModeSettings*)mmAlloc(sizeof(CamcontrolModeSettings), 0xf, 0);
    memset(gCamcontrolModeSettings, 0, sizeof(CamcontrolModeSettings));
}

ResourceDescriptorCallbacks12 lbl_80319B58 = {
    {0x00000000, 0x00000000, 0x00000000, 0x000b0000},
    {(ResourceDescriptorCallback)CameraModeNormal_initialise,
     (ResourceDescriptorCallback)CameraModeNormal_release,
     0x00000000,
     (ResourceDescriptorCallback)CameraModeNormal_init,
     (ResourceDescriptorCallback)CameraModeNormal_update,
     (ResourceDescriptorCallback)CameraModeNormal_free,
     (ResourceDescriptorCallback)CameraModeNormal_copyToCurrent,
     (ResourceDescriptorCallback)CameraModeNormal_follow,
     (ResourceDescriptorCallback)firstperson_updatePitch,
     (ResourceDescriptorCallback)camslide_update,
     (ResourceDescriptorCallback)CameraModeNormal_func0A,
     (ResourceDescriptorCallback)camcontrol_updateVerticalBounds}};
