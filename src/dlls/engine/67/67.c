#include "main/dll/CAM/pathcam.h"
#include "main/camera_interface.h"
#include "main/dll/CAM/camcontrol_path_state.h"
#include "main/object_transform.h"
#include "string.h"
#include "main/dll/CAM/camlockon.h"
#include "main/vecmath.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "game/objects/object.h"
#include "main/dll/player_api.h"
#include "main/dll/CAM/cutCam.h"
#include "main/dll/CAM/dll_0043_unk.h"
#include "main/pad.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/mm.h"
#include "main/frame_timing.h"
#include "main/resource.h"


u8 camcontrol_samplePathState(f32* outX, f32* height, f32* outZ, GameObject* target, CameraObject* camera)
{
    CamcontrolPathSampleWork work;
    int handler;
    int i;
    f32 pathT;

    memset(&work, 0, 0x144);
    work.model = (int)camera->anim.parent;
    work.sampleX = gCamcontrolPathState->pointsX[gCamcontrolPathState->pathCurve.count - 2];
    work.sampleY = *height;
    work.sampleZ = gCamcontrolPathState->pointsZ[gCamcontrolPathState->pathCurve.count - 2];
    work.localX = work.sampleX;
    work.localY = work.sampleY;
    work.localZ = work.sampleZ;
    Obj_TransformLocalPointToWorld((double)work.localX, (double)work.localY, (double)work.localZ, &work.worldX,
                                   &work.worldY, work.worldZ, work.model);
    work.targetObj = target;
    handler = (int)(*gCameraInterface)->getDefaultHandlerEntry();
    (*(VtableFn*)(**(int**)(handler + 4) + 0x14))(&work, target);
    Obj_TransformLocalPointToWorld(work.sampleX, work.sampleY, work.sampleZ, &work.targetX, &work.targetY, work.targetZ,
                                   work.model);
    (*(VtableFn*)(**(int**)(handler + 4) + 0x24))(&work, 1, 3, &gCamcontrolPathState->curveMin,
                                                  &gCamcontrolPathState->curveMax);
    i = gCamcontrolPathState->pathCurve.count + -3;
    for (; i < gCamcontrolPathState->pathCurve.count; i = i + 1)
    {
        gCamcontrolPathState->pointsX[i] = work.sampleX;
        gCamcontrolPathState->pointsZ[i] = work.sampleZ;
    }
    if (0.0f != gCamcontrolPathState->pathCurve.pathLength)
    {
        pathT = gCamcontrolPathState->pathCurve.pathDistance / gCamcontrolPathState->pathCurve.pathLength;
    }
    else
    {
        pathT = 0.0f;
    }
    if (pathT > 1.0f)
    {
        pathT = 1.0f;
    }
    else if (pathT < 0.0f)
    {
        pathT = 0.0f;
    }
    pathT = Curve_EvalHermite(gCamcontrolPathState->initialiseCurve, pathT, (float*)0x0);
    if (pathT < 0.2f)
    {
        pathT = 0.2f;
    }
    Curve_AdvanceAlongPath(&gCamcontrolPathState->pathCurve, pathT);
    *outX = gCamcontrolPathState->pathCurve.sample[0];
    *outZ = gCamcontrolPathState->pathCurve.sample[2];
    return;
}

CamcontrolPathState* gCamcontrolPathState;

void camcontrol_buildPathAngles(s16* outArr, u16* outCount, s16 baseAngle, s16 deltaAngle,
                                s16 limit)
{
    if (deltaAngle >= limit)
    {
        camcontrol_buildPathAngles(outArr, outCount, baseAngle, deltaAngle >> 1, limit);
        camcontrol_buildPathAngles(outArr, outCount, baseAngle + (deltaAngle >> 1), deltaAngle >> 1,
                                   limit);
    }
    else
    {
        outArr[(*outCount)++] = baseAngle;
    }
}

void camcontrol_buildPathPoints(f32 baseX, f32 baseZ, f32 targetX, f32 baseY, f32 targetZ,
                                f32 targetY, s16 angleRange, s16 angleLimit,
                                int* outPointCount)
{
    u16 angleCount;
    s16 rot[3];
    f32 vec[3];
    s16 pathAngles[CAMCONTROL_PATH_POINT_CAPACITY];
    s16 absAngleRange;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    int i;
    int pointCount;

    if (angleRange < 0)
    {
        absAngleRange = -angleRange;
    }
    else
    {
        absAngleRange = angleRange;
    }

    angleCount = 0;
    camcontrol_buildPathAngles(pathAngles, &angleCount, 0, absAngleRange, angleLimit);

    deltaX = targetX - baseX;
    deltaY = targetY - baseY;
    deltaZ = targetZ - baseZ;
    i = 1;
    pointCount = 3;

    while (i < angleCount)
    {
        vec[0] = deltaX;
        vec[1] = deltaY;
        vec[2] = deltaZ;

        rot[0] = angleRange < 0 ? pathAngles[i] : -pathAngles[i];
        rot[1] = 0;
        rot[2] = 0;
        vecRotateZXY(rot, vec);

        gCamcontrolPathState->pointsX[pointCount] = baseX + vec[0];
        gCamcontrolPathState->pointsY[pointCount] =
            baseY + (deltaY * ((f32)pathAngles[i] / absAngleRange));
        gCamcontrolPathState->pointsZ[pointCount] = baseZ + vec[2];

        i++;
        pointCount++;
    }

    *outPointCount = pointCount;
}

#define PAD_TRIGGER_Z 0x10

typedef struct CameraModeStaffAnimSettings
{
    s16 approachThresholdDegrees;
    u8 turnGate;
    u8 snapToTarget;
} CameraModeStaffAnimSettings;

typedef void (*CameraBoundsFn)(CameraObject* camera, GameObject* target, f32 min, f32 max);

#define CAMMODE_DEFAULT    0x42
#define CAMMODE_VIEWFINDER 0x44
#define CAMMODE_COMBAT     0x49

void camcontrol_updatePathTargetAction(CameraObject* camera, GameObject* target)
{
    u16 buttons;
    GameObject* targetObj;
    CamcontrolPathState* path;
    CameraInterface* cam;
    s16 targetClass;
    int zPressed;
    int canView;
    void* lockSlot;
    void* pendingParent;
    struct
    {
        f32 x;
        f32 z;
        s16 y;
    } actionPayload;

    pendingParent = target->pendingParentObj;
    if (pendingParent != NULL)
    {
        return;
    }
    buttons = getButtonsJustPressed(0);
    targetObj = (GameObject*)camera->currentTarget;
    if ((targetObj != NULL &&
         (targetObj->anim.classId == 0x1c || targetObj->anim.classId == 0x2a) && target->anim.classId == 1 &&
         objFn_80296700(target) != 0) ||
        (camera->targetFlags & 2) != 0)
    {
        lockSlot = &camera->currentTarget;
        cam = *gCameraInterface;
        cam->setMode(CAMMODE_COMBAT, 1, 0, 4, lockSlot, 0x3c, 0xff);
        return;
    }
    zPressed = buttons & PAD_TRIGGER_Z;
    if (zPressed == 0)
    {
        return;
    }
    targetClass = target->anim.classId;
    if (targetClass != 1)
    {
        return;
    }
    canView = objFn_802962b4(target);
    if (canView == 0)
    {
        return;
    }
    path = gCamcontrolPathState;
    actionPayload.x = path->actionParamX;
    actionPayload.z = path->actionParamZ;
    actionPayload.y = path->actionParamY;
    cam = *gCameraInterface;
    cam->setMode(CAMMODE_VIEWFINDER, 1, 0, 0xc, &actionPayload, 0, 0xff);
}

void CameraModeStaffAnim_copyToCurrent(void)
{
}

void camcontrol_releasePathState(void)
{
    mm_free(gCamcontrolPathState);
    gCamcontrolPathState = NULL;
}

void camclimb_update(CameraObject* cam)
{
    u8 needsReset;
    u32 angle;
    int defaultHandler;
    int yawDelta;
    GameObject* target;
    int pointIndex;
    f32 localPosZ[4];
    f32 localPosY;
    f32 localPosX;
    f32 relX;
    f32 relY;
    f32 relZ;
    f32 relDistXZ;
    f32* pYaddr;

    if (gCamcontrolPathState->active != 0)
    {
        (*gCameraInterface)->setMode(CAMMODE_DEFAULT, 0, 1, 0, NULL, 0, 0xff);
    }
    else
    {
        if ((u32)gCamcontrolPathState->localFrameObj != *(u32*)&cam->anim.parent)
        {
            for (pointIndex = 0; pointIndex < gCamcontrolPathState->pathCurve.count; pointIndex++)
            {
                Obj_TransformLocalPointToWorld(
                    gCamcontrolPathState->pointsX[pointIndex], gCamcontrolPathState->pointsY[pointIndex],
                    gCamcontrolPathState->pointsZ[pointIndex], &gCamcontrolPathState->pointsX[pointIndex],
                    &gCamcontrolPathState->pointsY[pointIndex], &gCamcontrolPathState->pointsZ[pointIndex],
                    gCamcontrolPathState->localFrameObj);
            }
            for (pointIndex = 0; pointIndex < gCamcontrolPathState->pathCurve.count; pointIndex++)
            {
                Obj_TransformWorldPointToLocal(
                    gCamcontrolPathState->pointsX[pointIndex], gCamcontrolPathState->pointsY[pointIndex],
                    gCamcontrolPathState->pointsZ[pointIndex], &gCamcontrolPathState->pointsX[pointIndex],
                    &gCamcontrolPathState->pointsY[pointIndex], &gCamcontrolPathState->pointsZ[pointIndex],
                    *(int*)&cam->anim.parent);
            }
            gCamcontrolPathState->localFrameObj = *(int*)&cam->anim.parent;
        }
        target = (GameObject*)cam->anim.targetObj;
        *(pYaddr = &localPosY) = cam->anim.localPosY;
        needsReset = (u8)camcontrol_samplePathState(&localPosX, pYaddr, localPosZ, target, cam);
        cam->anim.localPosX = localPosX;
        cam->anim.localPosZ = localPosZ[0];
        defaultHandler = (int)(*gCameraInterface)->getDefaultHandlerEntry();
        Obj_TransformLocalPointToWorld(cam->anim.localPosX, cam->anim.localPosY, cam->anim.localPosZ,
                                       &cam->anim.worldPosX, &cam->anim.worldPosY, &cam->anim.worldPosZ,
                                       *(int*)&cam->anim.parent);
        (*(CameraBoundsFn*)(**(int**)(defaultHandler + 4) + 0x1c))(cam, target, -100000.0f, 100000.0f);
        (*(VtableFn*)(**(int**)(defaultHandler + 4) + 0x24))(cam, 1, 3, &gCamcontrolPathState->curveMin,
                                                             &gCamcontrolPathState->curveMax);
        if ((cam->anim.currentMove != 0) || (cam->cameraCollisionActive != 0))
        {
            gCamcontrolPathState->initialiseCurve[4] = gCamcontrolPathState->initialiseCurve[4] + timeDelta;
        }
        if (gCamcontrolPathState->initialiseCurve[4] > 0.0f)
        {
            needsReset = camcontrol_getTargetPosition(cam, &target->anim, &cam->anim.worldPosX, &cam->anim.rotY);
            if (needsReset == 1)
            {
                doNothing_80103660(1);
            }
            cam->probePosX = cam->anim.worldPosX;
            cam->probePosY = cam->anim.worldPosY;
            cam->probePosZ = cam->anim.worldPosZ;
            needsReset = 1;
        }
        (*gCameraInterface)->getRelativePosition(cam, &relX, &relY, &relZ, &relDistXZ, 0.0f, 0);
        angle = getAngle((double)relX, (double)relZ);
        yawDelta = 0x8000 - (angle & 0xffff);
        yawDelta = yawDelta - (u32)(u16)cam->anim.rotX;
        if (0x8000 < yawDelta)
        {
            yawDelta = yawDelta + -0xffff;
        }
        if (yawDelta < -0x8000)
        {
            yawDelta = yawDelta + 0xffff;
        }
        cam->anim.rotX += yawDelta;
        (*(VtableFn*)(**(int**)(defaultHandler + 4) + 0x18))(cam, (double)target->anim.worldPosY, (double)relDistXZ);
        if (needsReset != 0)
        {
            (*gCameraInterface)->setMode(CAMMODE_DEFAULT, 0, 1, 0, NULL, 0, 0xff);
        }
        camcontrol_updatePathTargetAction(cam, target);
        Obj_TransformWorldPointToLocal(cam->anim.worldPosX, cam->anim.worldPosY, cam->anim.worldPosZ,
                                       &cam->anim.localPosX, &cam->anim.localPosY, &cam->anim.localPosZ,
                                       *(int*)&cam->anim.parent);
    }
    return;
}

static inline f32 CameraModeStaffAnim_angleToRadians(int angle)
{
    return (3.1415927f * angle) / 32768.0f;
}

void CameraModeStaffAnim_init(CameraObject* camera, int unused, u8* settings)
{
    GameObject* target;
    int view;
    f32 sinFacing;
    f32 cosFacing;
    f32 relAngleRad;
    f32 relCos;
    f32 relSin;
    int facingDelta;
    s16 approachAngle;
    s16 turnAmount;
    s16 absTurn;
    s16 pathAngle;
    s16 threshold;
    f32 pathRadius;
    f32 pathScale;
    f32 baseX;
    f32 baseZ;
    f32 dx;
    f32 dz;
    f32 localPos[3];
    int pointCount;
    int i;
    CameraModeStaffAnimSettings* cfg = (CameraModeStaffAnimSettings*)settings;

    cfg->snapToTarget = 1;
    target = (GameObject*)camera->anim.targetObj;

    if (gCamcontrolPathState == NULL)
    {
        gCamcontrolPathState = mmAlloc(sizeof(CamcontrolPathState), 0xf, 0);
    }
    memset(gCamcontrolPathState, 0, sizeof(CamcontrolPathState));

    view = (int)(*gCameraInterface)->getDefaultHandlerEntry();
    (*(void (**)(f32*, f32*, f32*, int, f32*))(**(int**)(view + 4) + 0x20))(
        &gCamcontrolPathState->actionParamX, &gCamcontrolPathState->pad08, &gCamcontrolPathState->actionParamZ, 0,
        &gCamcontrolPathState->actionParamY);

    gCamcontrolPathState->active = 0;
    gCamcontrolPathState->localFrameObj = *(int*)&camera->anim.parent;

    sinFacing = mathSinf(CameraModeStaffAnim_angleToRadians(target->anim.rotX));
    cosFacing = mathCosf(CameraModeStaffAnim_angleToRadians(target->anim.rotX));

    if ((void*)gCamcontrolPathState->localFrameObj != NULL)
    {
        facingDelta = target->anim.rotX - ((s16*)gCamcontrolPathState->localFrameObj)[0];
    }
    else
    {
        facingDelta = target->anim.rotX;
    }

    relAngleRad = CameraModeStaffAnim_angleToRadians(facingDelta);
    relCos = mathSinf(relAngleRad);
    relSin = mathCosf(relAngleRad);

    approachAngle = target->anim.rotX - (u16)getAngle(camera->anim.worldPosX - target->anim.worldPosX,
                                                      camera->anim.worldPosZ - target->anim.worldPosZ);
    if (approachAngle > 0x8000)
    {
        approachAngle = approachAngle - 0xffff;
    }
    if (approachAngle < -0x8000)
    {
        approachAngle = approachAngle + 0xffff;
    }
    if (approachAngle < 0)
    {
        approachAngle = -approachAngle;
    }

    threshold = (s16)(182.04445f * (f32)cfg->approachThresholdDegrees);
    if (approachAngle < threshold)
    {
        gCamcontrolPathState->active = 1;
    }
    else
    {
        pathRadius = gCamcontrolPathState->actionParamX * gCamcontrolPathState->actionParamX -
                     gCamcontrolPathState->actionParamZ * gCamcontrolPathState->actionParamZ;
        if (pathRadius < 5.0f)
        {
            pathRadius = 5.0f;
        }
        pathRadius = sqrtf(pathRadius);

        localPos[0] = (sinFacing * pathRadius) + target->anim.worldPosX;
        localPos[1] =
            gCamcontrolPathState->actionParamZ + (target->anim.worldPosY + gCamcontrolPathState->actionParamY);
        localPos[2] = (cosFacing * pathRadius) + target->anim.worldPosZ;

        if (cfg->snapToTarget != 0)
        {
            camcontrol_getTargetPosition(camera, &target->anim, localPos, NULL);
        }

        Obj_TransformWorldPointToLocal(localPos[0], localPos[1], localPos[2], &localPos[0], &localPos[1], &localPos[2],
                                       *(int*)&camera->anim.parent);

        for (pointCount = 0; pointCount < 3; pointCount++)
        {
            gCamcontrolPathState->pointsX[pointCount] = camera->anim.localPosX;
            gCamcontrolPathState->pointsY[pointCount] = camera->anim.localPosY;
            gCamcontrolPathState->pointsZ[pointCount] = camera->anim.localPosZ;
        }

        dx = camera->anim.localPosX - localPos[0];
        dz = camera->anim.localPosZ - localPos[2];
        pathRadius = 0.5f * sqrtf(dx * dx + dz * dz);
        turnAmount = getAngle(-relCos, -relSin) - (u16)getAngle(dx, dz);

        if (turnAmount > 0x8000)
        {
            turnAmount = turnAmount - 0xffff;
        }
        if (turnAmount < -0x8000)
        {
            turnAmount = turnAmount + 0xffff;
        }

        pathAngle = turnAmount;
        if (turnAmount < 0)
        {
            turnAmount = -turnAmount;
        }

        if (turnAmount > 0x4000)
        {
            absTurn = 0;
        }
        else
        {
            absTurn = 0x4000 - turnAmount;
        }

        if (pathAngle < 0)
        {
            pathAngle = -(absTurn << 1);
        }
        else
        {
            pathAngle = absTurn << 1;
        }

        if (absTurn != 0)
        {
            pathScale = pathRadius / mathSinf(CameraModeStaffAnim_angleToRadians(absTurn));
        }
        else
        {
            pathScale = 0.0f;
        }

        baseX = localPos[0] - (relCos * pathScale);
        baseZ = localPos[2] - (relSin * pathScale);
        gCamcontrolPathState->pathCurve.px = gCamcontrolPathState->pointsX;
        gCamcontrolPathState->pathCurve.py = gCamcontrolPathState->pointsY;
        gCamcontrolPathState->pathCurve.pz = gCamcontrolPathState->pointsZ;
        gCamcontrolPathState->pathCurve.eval = Curve_EvalBSpline;
        gCamcontrolPathState->pathCurve.coeffFn = Curve_BuildBSplineCoeffs;

        camcontrol_buildPathPoints(baseX, baseZ, camera->anim.localPosX, camera->anim.localPosY, camera->anim.localPosZ,
                                   localPos[1], pathAngle, 0x1555, &pointCount);

        i = pointCount;
        for (; i < pointCount + 3; i++)
        {
            gCamcontrolPathState->pointsX[i] = localPos[0];
            gCamcontrolPathState->pointsY[i] = localPos[1];
            gCamcontrolPathState->pointsZ[i] = localPos[2];
        }

        gCamcontrolPathState->pathCurve.count = i;
        gCamcontrolPathState->pathCurve.dir = 0;
        curvesMove(&gCamcontrolPathState->pathCurve);

        if (pathAngle < 0)
        {
            pathAngle = -pathAngle;
        }
        if ((pathAngle > 0x2000) && (cfg->turnGate != 0))
        {
            Sfx_PlayFromObject(0, SFXTRIG_mv_totem_stop);
        }

        pathScale = gCamcontrolPathState->pathCurve.pathLength;
        (*gCameraInterface)
            ->initialise(pathScale, &gCamcontrolPathState->initialiseCurve[0], 20.0f, 0.5f, 1.0f,
                         -10.0f);

        gCamcontrolPathState->curveMin = -100000.0f;
        gCamcontrolPathState->curveMax = 100000.0f;
    }
}

void CameraModeStaffAnim_release(void)
{
}

void CameraModeStaffAnim_initialise(void)
{
}

ResourceDescriptorCallbacks8 lbl_80319B98 = {
    {0x00000000, 0x00000000, 0x00000000, 0x00060000},
    {(ResourceDescriptorCallback)CameraModeStaffAnim_initialise,
     (ResourceDescriptorCallback)CameraModeStaffAnim_release,
     0x00000000,
     (ResourceDescriptorCallback)CameraModeStaffAnim_init,
     (ResourceDescriptorCallback)camclimb_update,
     (ResourceDescriptorCallback)camcontrol_releasePathState,
     (ResourceDescriptorCallback)CameraModeStaffAnim_copyToCurrent,
     0x00000000}};
