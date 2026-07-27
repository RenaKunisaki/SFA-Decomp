/* DLL 0x0045 - camera talk / NPC-speak mode [80107AEC-8010847C) */
#include "main/dll/CAM/cambike_state.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_trig_api.h"
#include "main/mm.h"
#include "main/dll/CAM/dll_0045_camTalk.h"
#include "main/dll/CAM/cutCam.h"
#include "main/camera_interface.h"
#include "main/object_transform.h"
#include "string.h"
#include "main/frame_timing.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/dll/player_api.h"

CameraModeBikeState* gCamTalkBikeState;
extern const f32 gCamTalkDefaultFov;
extern const f32 gCamTalkPi;
extern const f32 gCamTalkAngleUnitScale;
extern const f32 gCamTalkDefaultFollowDist;
extern const f32 lbl_803E17B4;
extern const f32 lbl_803E17A8;

#define CAM_TALK_FOLLOW_SMOOTHING lbl_803E17A8
#define CAM_TALK_ROLL_SCALE lbl_803E17A8
#define CAM_TALK_ROLL_SMOOTHING lbl_803E17B4

void CameraModeBike_copyToCurrent(f32* inputs)
{
    gCamTalkBikeState->turnInput = inputs[0];
    gCamTalkBikeState->heightInput = inputs[1];
    gCamTalkBikeState->rollInput = inputs[2];
    gCamTalkBikeState->pitchTarget = inputs[3];
}

void CameraModeBike_free(void)
{
    mm_free(gCamTalkBikeState);
    gCamTalkBikeState = 0;
}

void CameraModeBike_update(CameraObject* camera)
{
    float rollStep;
    int targetAngle;
    float followDist;
    float heightT;
    float followTermB;
    float followTermA;
    float yawSmoothing;
    float yawTarget;
    short angleDelta;
    u16 cameraAngle;
    GameObject* target;
    CameraModeBikeState* st;
    float sinYaw;
    float cosYaw;
    float sinPitch;
    float cosPitch;
    float pivotX;
    float pivotY;
    float pivotZ;
    MatrixTransform targetTransform;
    f32 matrix[16];

    (*gCameraInterface)->getDefaultHandlerEntry();
    target = camera->anim.targetObj;
    if (target != NULL)
    {
        camera->fov = gCamTalkDefaultFov;
        targetTransform.x = target->anim.worldPosX;
        targetTransform.y = target->anim.worldPosY;
        targetTransform.z = target->anim.worldPosZ;
        targetTransform.scale = (1.0f);
        targetTransform.rotX = target->anim.rotX;
        targetTransform.rotY = gCamTalkBikeState->pitchTarget;
        targetTransform.rotZ = 0;
        setMatrixFromObjectPos(matrix, &targetTransform);
        Matrix_TransformPoint(matrix, (0.0f), (2e+01f), (0.0f), &pivotX, &pivotY, &pivotZ);
        angleDelta = 0x8000 - target->anim.rotX;
        camera->anim.rotX = angleDelta;
        st = gCamTalkBikeState;
        yawSmoothing = (0.1f);
        yawTarget = (12.0f) * st->turnInput;
        st->smoothedYawOffset += yawSmoothing * (yawTarget - st->smoothedYawOffset);
        camera->anim.rotX = camera->anim.rotX + gCamTalkBikeState->smoothedYawOffset;
        targetAngle = (3072.0f) - gCamTalkBikeState->pitchTarget;
        cameraAngle = camera->anim.rotY;
        angleDelta = targetAngle - cameraAngle;
        if (0x8000 < angleDelta)
        {
            angleDelta = angleDelta - 0xFFFF;
        }
        if (angleDelta < -0x8000)
        {
            angleDelta = angleDelta + 0xFFFF;
        }
        camera->anim.rotY += (angleDelta >> 3);
        sinYaw = mathSinf(gCamTalkPi * (camera->anim.rotX - 0x4000) / gCamTalkAngleUnitScale);
        cosYaw = mathCosf(gCamTalkPi * (camera->anim.rotX - 0x4000) / gCamTalkAngleUnitScale);
        cosPitch = mathCosf(gCamTalkPi * camera->anim.rotY / gCamTalkAngleUnitScale);
        sinPitch = mathSinf(gCamTalkPi * camera->anim.rotY / gCamTalkAngleUnitScale);
        st = gCamTalkBikeState;
        heightT = -st->heightInput / (6.0f);
        followTermA = CAM_TALK_FOLLOW_SMOOTHING;
        followTermB = (25.0f);
        heightT =
            (heightT < (0.0f)) ? (0.0f) : ((heightT > (1.0f)) ? (1.0f) : heightT);
        st->followDistance += followTermA * ((followTermB * heightT + gCamTalkDefaultFollowDist) - st->followDistance);
        followDist = gCamTalkBikeState->followDistance;
        followTermA = followDist * sinPitch;
        followTermB = followDist * cosPitch;
        cosYaw = followTermB * cosYaw;
        followTermB = followTermB * sinYaw;
        camera->anim.worldPosX = pivotX + cosYaw;
        camera->anim.worldPosY = pivotY + followTermA;
        camera->anim.worldPosZ = pivotZ + followTermB;
        targetAngle = CAM_TALK_ROLL_SCALE * gCamTalkBikeState->rollInput;
        cameraAngle = camera->anim.rotZ;
        angleDelta = targetAngle - cameraAngle;
        if (0x8000 < angleDelta)
        {
            angleDelta = angleDelta - 0xFFFF;
        }
        if (angleDelta < -0x8000)
        {
            angleDelta = angleDelta + 0xFFFF;
        }
        rollStep = angleDelta * timeDelta;
        camera->anim.rotZ += rollStep * CAM_TALK_ROLL_SMOOTHING;
        Obj_TransformWorldPointToLocal(camera->anim.worldPosX, camera->anim.worldPosY, camera->anim.worldPosZ,
                                       &camera->anim.localPosX, &camera->anim.localPosY, &camera->anim.localPosZ,
                                       camera->anim.parentAddress);
    }
    return;
}

void CameraModeBike_init(CameraObject* camera)
{

    if (gCamTalkBikeState == 0)
    {
        gCamTalkBikeState = (CameraModeBikeState*)mmAlloc(sizeof(CameraModeBikeState), 0xf, 0);
    }
    memset(gCamTalkBikeState, 0, sizeof(CameraModeBikeState));
    gCamTalkBikeState->entryFov = camera->fov;
    gCamTalkBikeState->defaultFov = (85.0f);
    gCamTalkBikeState->defaultScale = (1.0f);
    gCamTalkBikeState->followDistance = (5e+01f);
}

void CameraModeBike_release(void)
{
}

void CameraModeBike_initialise(void)
{
}
