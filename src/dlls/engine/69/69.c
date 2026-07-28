/*
 * DLL 69 / 0x45.
 */
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
#include "main/resource.h"

CameraModeBikeState* gCamTalkBikeState;
#define CAM_TALK_DEFAULT_FOV 85.0f
#define CAM_TALK_PI 3.14159274f
#define CAM_TALK_ANGLE_UNIT_SCALE 32768.0f
#define CAM_TALK_DEFAULT_FOLLOW_DIST 50.0f
#define CAM_TALK_FOLLOW_SMOOTHING 0.2f
#define CAM_TALK_ROLL_SCALE 0.2f
#define CAM_TALK_ROLL_SMOOTHING 0.0625f

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

static void CameraModeBike_resetSmoothing(CameraModeBikeState* st)
{
    st->smoothedYawOffset = (0.0f);
}

void CameraModeBike_update(CameraObject* camera)
{
    float rollSmoothing;
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
        camera->fov = CAM_TALK_DEFAULT_FOV;
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
        sinYaw = mathSinf(CAM_TALK_PI * (camera->anim.rotX - 0x4000) / CAM_TALK_ANGLE_UNIT_SCALE);
        cosYaw = mathCosf(CAM_TALK_PI * (camera->anim.rotX - 0x4000) / CAM_TALK_ANGLE_UNIT_SCALE);
        cosPitch = mathCosf(CAM_TALK_PI * camera->anim.rotY / CAM_TALK_ANGLE_UNIT_SCALE);
        sinPitch = mathSinf(CAM_TALK_PI * camera->anim.rotY / CAM_TALK_ANGLE_UNIT_SCALE);
        st = gCamTalkBikeState;
        heightT = -st->heightInput / (6.0f);
        followTermA = CAM_TALK_FOLLOW_SMOOTHING;
        followTermB = CAM_TALK_DEFAULT_FOLLOW_DIST;
        heightT =
            (heightT < (0.0f)) ? (0.0f) : ((heightT > (1.0f)) ? (1.0f) : heightT);
        st->followDistance += followTermA * (((25.0f) * heightT + followTermB) - st->followDistance);
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
        rollSmoothing = CAM_TALK_ROLL_SMOOTHING;
        camera->anim.rotZ += rollStep * rollSmoothing;
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
    gCamTalkBikeState->defaultFov = CAM_TALK_DEFAULT_FOV;
    gCamTalkBikeState->defaultScale = (1.0f);
    gCamTalkBikeState->followDistance = CAM_TALK_DEFAULT_FOLLOW_DIST;
}

void CameraModeBike_release(void)
{
}

void CameraModeBike_initialise(void)
{
}

ResourceDescriptorCallbacks8 lbl_80319BC8 = {
    {0x00000000, 0x00000000, 0x00000000, 0x00060000},
    {(ResourceDescriptorCallback)CameraModeBike_initialise,
     (ResourceDescriptorCallback)CameraModeBike_release,
     0x00000000,
     (ResourceDescriptorCallback)CameraModeBike_init,
     (ResourceDescriptorCallback)CameraModeBike_update,
     (ResourceDescriptorCallback)CameraModeBike_free,
     (ResourceDescriptorCallback)CameraModeBike_copyToCurrent,
     0x00000000}};
