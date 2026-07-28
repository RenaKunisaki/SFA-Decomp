#include "main/camera.h"
#include "main/pi_dolphin.h"
#include "main/frame_timing.h"
#include "game/objects/object.h"
#include "main/pause_menu_api.h"
#include "main/shader_api.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "dolphin/gx/GXLegacy.h"
#include "dolphin/gx/GXTransform.h"
#include "dolphin/mtx.h"

f32 gCameraNearPlane = 2.5f;
f32 gCameraFarPlane = 10000.0f;
f32 gCameraAspectRatio = 1.3333334f;
f32 lbl_803DB26C = 1.0f;

f32 gCameraFarPlaneTransitionStart;
f32 gCameraFarPlaneTransitionTarget;
f32 gCameraFovY;
f32 gCameraOrthoTop;
f32 gCameraOrthoBottom;
f32 gCameraOrthoLeft;
f32 gCameraOrthoRight;
s32 gCameraProjectionMode;
u8 gCameraCurrentViewIndex;
u8 cameraViewYOffsetEnabled;
s16 lbl_803DC88A;
s8 gObjTransformMatrixSlot;
s16 cameraViewportYOffset;
s16 gCameraViewportYOffset;
s16 gCameraFarPlaneTransitionFrames;
s16 gCameraFarPlaneTransitionFramesLeft;

CameraMatrix gObjInverseYawTransformMatrices[0x1E];
CameraMatrix gObjYawTransformMatrices[0x22];
f32 gCameraWorldMatrix[64];
CameraMatrix gCameraDefaultModelMatrix;
CameraViewSlot gCameraShakeSlots[0x480 / sizeof(CameraViewSlot)];
CameraMatrix gCameraViewRotationMatrix;
CameraMatrix gCameraInverseViewRotationMatrix;
CameraMatrix gCameraViewMatrix;
CameraMatrix gCameraInverseViewMatrix;
CameraMatrix gCameraProjectionMatrix;

void Obj_RotateLocalOffsetByYaw(f32* local, f32* out, s8 yawIndex) {
    s32 matrixOffset;
    f32* matrix;

    if (yawIndex < 0) {
        out[0] = local[0];
        out[1] = local[1];
        out[2] = local[2];
    } else {
        matrixOffset = yawIndex * 16;
        matrix = (f32*)gObjYawTransformMatrices + matrixOffset;
        Matrix_TransformPoint(matrix, local[0], local[1], local[2], &out[0], &out[1], &out[2]);
    }
}

void Obj_UpdateWorldTransform(CameraViewSlot* view) {
    GameObject* parent;
    s32 matrixOffset;
    f32* matrix;

    parent = view->parentObject;
    if (parent == NULL) {
        view->worldX = view->x;
        view->worldY = view->y;
        view->worldZ = view->z;
        view->worldYaw = view->yaw;
        view->worldPitch = view->pitch;
        view->worldRoll = view->roll;
    } else {
        matrixOffset = parent->anim.transformMatrixIndex * 16;
        matrix = (f32*)gObjYawTransformMatrices + matrixOffset;
        Matrix_TransformPoint(matrix, view->x, view->y, view->z, &view->worldX, &view->worldY, &view->worldZ);
        view->worldYaw = view->yaw - parent->anim.rotX;
        view->worldPitch = view->pitch;
        view->worldRoll = view->roll;
    }
}

s32 Angle_AddWrappedS16(s32 angle, s16* delta) {
    if ((angle += *delta) > 0x8000) {
        angle -= 0xFFFF;
    }
    if (angle >= -0x8000) {
        return angle;
    }
    return angle + 0xFFFF;
}

s32 Angle_SubWrappedS16(s32 angle, s16* delta) {
    if ((angle -= *delta) > 0x8000) {
        angle -= 0xFFFF;
    }
    if (angle >= -0x8000) {
        return angle;
    }
    return angle + 0xFFFF;
}

void Obj_TransformLocalVectorToWorld(f32 x, f32 y, f32 z, f32* outX, f32* outY, f32* outZ, int obj) {
    f32 vec[3];
    s32 matrixOffset;

    vec[0] = x;
    vec[1] = y;
    vec[2] = z;
    matrixOffset = ((GameObject*)obj)->anim.transformMatrixIndex * 16;
    Matrix_TransformVector((f32*)gObjYawTransformMatrices + matrixOffset, vec, vec);
    *outX = vec[0];
    *outY = vec[1];
    *outZ = vec[2];
}

void Obj_TransformWorldVectorToLocal(f32 x, f32 y, f32 z, f32* outX, f32* outY, f32* outZ, u32 obj) {
    f32 vec[3];
    s32 matrixOffset;

    vec[0] = x;
    vec[1] = y;
    vec[2] = z;
    matrixOffset = ((GameObject*)obj)->anim.transformMatrixIndex * 16;
    Matrix_TransformVector((f32*)gObjInverseYawTransformMatrices + matrixOffset, vec, vec);
    *outX = vec[0];
    *outY = vec[1];
    *outZ = vec[2];
}

void Obj_TransformWorldPointToLocal(f32 x, f32 y, f32 z, f32* outX, f32* outY, f32* outZ, int obj) {
    s32 matrixOffset;

    if ((u32)obj != 0) {
        matrixOffset = ((GameObject*)obj)->anim.transformMatrixIndex * 16;
        Matrix_TransformPoint((f32*)gObjInverseYawTransformMatrices + matrixOffset, x, y, z, outX, outY, outZ);
    } else {
        *outX = x;
        *outY = y;
        *outZ = z;
    }
}

void Obj_TransformLocalPointToWorld(f32 x, f32 y, f32 z, f32* outX, f32* outY, f32* outZ, int obj) {
    s32 matrixOffset;

    if ((u32)obj != 0) {
        matrixOffset = ((GameObject*)obj)->anim.transformMatrixIndex * 16;
        Matrix_TransformPoint((f32*)gObjYawTransformMatrices + matrixOffset, x, y, z, outX, outY, outZ);
    } else {
        *outX = x;
        *outY = y;
        *outZ = z;
    }
}

void Obj_GetWorldPosition(u32 obj, f32* outX, f32* outY, f32* outZ) {
    u32 parent;
    s32 matrixOffset;

    parent = (u32)((GameObject*)obj)->anim.parent;
    if (parent == 0) {
        *outX = ((GameObject*)obj)->anim.localPosX;
        *outY = ((GameObject*)obj)->anim.localPosY;
        *outZ = ((GameObject*)obj)->anim.localPosZ;
    } else {
        matrixOffset = ((GameObject*)parent)->anim.transformMatrixIndex * 16;
        Matrix_TransformPoint((f32*)gObjYawTransformMatrices + matrixOffset, ((GameObject*)obj)->anim.localPosX,
                              ((GameObject*)obj)->anim.localPosY, ((GameObject*)obj)->anim.localPosZ, outX, outY, outZ);
    }
}

typedef struct ObjTransformMatrixPool {
    CameraMatrix inverse[0x1E];
    CameraMatrix yaw[0x1F];
    CameraMatrix scratch;
} ObjTransformMatrixPool;

STATIC_ASSERT(offsetof(ObjTransformMatrixPool, yaw) == 0x780);
STATIC_ASSERT(offsetof(ObjTransformMatrixPool, scratch) == 0xF40);
STATIC_ASSERT(sizeof(ObjTransformMatrixPool) == 0xF80);

void Obj_BuildTransformMatricesForYaw(GameObject* obj, s32 yawIndex) {
    ObjTransformMatrixPool* base;
    GameObject* ancestors[4];
    MatrixTransform inverseTransform;
    f32* inverseYawMatrix;
    s32 matrixOffset;
    f32* yawMatrix;
    s8 ancestorCount;
    f32 savedScale;
    s8 hasParent;
    f32* yawMatrices;

    base = (ObjTransformMatrixPool*)gObjInverseYawTransformMatrices;
    matrixOffset = yawIndex * 16;
    yawMatrices = (f32*)base->yaw;
    yawMatrix = yawMatrices + matrixOffset;
    inverseYawMatrix = (f32*)base->inverse + matrixOffset;
    hasParent = 0;
    ancestorCount = 0;
    while (obj != 0) {
        ancestors[ancestorCount] = obj;
        ancestorCount++;
        savedScale = obj->anim.rootMotionScale;
        if ((obj->objectFlags & 8) == 0) {
            obj->anim.rootMotionScale = lbl_803DE5F0;
        }

        if (hasParent == 0) {
            setMatrixFromObjectPos(yawMatrix, (MatrixTransform*)&obj->anim);
        } else {
            setMatrixFromObjectPos(base->scratch, (MatrixTransform*)&obj->anim);
            mtx44_multSafe(yawMatrix, base->scratch, yawMatrix);
        }

        obj->anim.rootMotionScale = savedScale;
        obj = obj->anim.parent;
        hasParent = 1;
    }

    while (ancestorCount > 0) {
        ancestorCount--;
        obj = ancestors[ancestorCount];
        inverseTransform.x = -obj->anim.localPosX;
        inverseTransform.y = -obj->anim.localPosY;
        inverseTransform.z = -obj->anim.localPosZ;
        if ((obj->objectFlags & 8) == 0) {
            inverseTransform.scale = lbl_803DE5F0;
        } else {
            inverseTransform.scale = lbl_803DE5F0 / obj->anim.rootMotionScale;
        }
        inverseTransform.rotX = -obj->anim.rotX;
        inverseTransform.rotY = -obj->anim.rotY;
        inverseTransform.rotZ = -obj->anim.rotZ;
        mtxRotateByVec3s(inverseYawMatrix, &inverseTransform);
    }
}

void Obj_BuildTransformMatrices(GameObject* obj) {
    Obj_BuildTransformMatricesForYaw(obj, obj->anim.transformMatrixIndex);
}

s32 Obj_BuildTransformMatrixSlot(GameObject* obj) {
    Obj_BuildTransformMatricesForYaw(obj, gObjTransformMatrixSlot);
    gObjTransformMatrixSlot++;
    return gObjTransformMatrixSlot - 1;
}

static inline f32 Camera_Expf(f32 x, u32 iterations) {
    f32 y;
    f32 xp;
    f32 n;
    f32 yp;

    y = 1.0f;
    n = 1.0f;
    xp = x;
    yp = 1.0f;

    for (; iterations != 0; iterations--) {
        y += xp / yp;
        n += 1.0f;
        xp *= x;
        yp *= n;
    }

    return y;
}

void Camera_UpdateShakeAndFarPlane(void) {
    CameraViewSlot* slot;
    f32 expTerm;
    f32 shakeTimer;
    f32 sinePhase;
    f32 phaseScale;

    gCameraViewportYOffset = cameraViewportYOffset;
    if (gCameraFarPlaneTransitionFramesLeft != 0) {
        gCameraFarPlaneTransitionFramesLeft -= framesThisStep;
        if (gCameraFarPlaneTransitionFramesLeft < 0) {
            gCameraFarPlaneTransitionFramesLeft = 0;
        }
        gCameraFarPlane = ((f32)gCameraFarPlaneTransitionFramesLeft / gCameraFarPlaneTransitionFrames) *
                              (gCameraFarPlaneTransitionStart - gCameraFarPlaneTransitionTarget) +
                          gCameraFarPlaneTransitionTarget;
    }

    gObjTransformMatrixSlot = 0;
    slot = &gCameraShakeSlots[gCameraCurrentViewIndex];

    if (slot->shakeActive == 0) {
        slot->shakeFlipTimer--;
        while (slot->shakeFlipTimer < 0) {
            slot->shakeFlipTimer++;
            slot->shakeMagnitude = gCameraShakeMagnitudeDecay * -slot->shakeMagnitude;
        }
    } else if (slot->shakeActive == 1) {
        expTerm = Camera_Expf(-slot->shakeFalloff * (shakeTimer = slot->shakeTimer), 20);

        phaseScale = 65535.0f * slot->shakeDuration;
        sinePhase = (gCameraPi * (phaseScale * shakeTimer)) / 32768.0f;
        slot->shakeMagnitude = slot->shakeMagnitudeTarget * expTerm * mathCosf(sinePhase);
        if ((slot->shakeMagnitude < gCameraShakeStopThreshold) &&
            (slot->shakeMagnitude > gCameraShakeStopThresholdNeg)) {
            slot->shakeMagnitude = lbl_803DE60C;
            slot->shakeActive = -1;
        }
        slot->shakeTimer += timeDelta / 60.0f;
    }
}

u8 CameraShake_IsActive(void) {
    CameraViewSlot* slot = &gCameraShakeSlots[gCameraCurrentViewIndex];

    return slot->shakeActive == 1;
}

void CameraShake_Start(f32 magnitude, f32 duration, f32 falloff) {
    CameraViewSlot* slot = &gCameraShakeSlots[0];

    slot->shakeMagnitude = magnitude;
    slot->shakeMagnitudeTarget = magnitude;
    slot->shakeDuration = duration;
    slot->shakeTimer = lbl_803DE60C;
    slot->shakeFalloff = falloff;
    slot->shakeActive = 1;
}

void CameraShake_SetAllMagnitudes(f32 magnitude) {
    CameraViewSlot* slot = gCameraShakeSlots;
    int group;
    int i;

    for (group = 0; group < 2; group++) {
        for (i = 0; i < 6; i++) {
            CameraViewSlot* p = &slot[i];
            p->shakeMagnitude = magnitude;
            p->shakeActive = 0;
        }
        slot += 6;
    }
}

void CameraShake_ApplyRadial(f32 x, f32 y, f32 z, f32 radius, f32 magnitude) {
    CameraViewSlot* slot;
    s32 i;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 distance;
    s8 inactive;

    slot = gCameraShakeSlots;
    inactive = 0;
    for (i = 0; i <= 7; i++) {
        dx = x - slot[i].x;
        dy = y - slot[i].y;
        dz = z - slot[i].z;
        distance = sqrtf(dx * dx + dy * dy + dz * dz);
        if (distance < radius) {
            slot[i].shakeMagnitude = (magnitude * (radius - distance)) / radius;
            slot[i].shakeActive = inactive;
        }
    }
}

f32* Camera_GetWorldMatrix(void) {
    return gCameraWorldMatrix;
}

void Camera_LoadModelViewMatrix(int unused0, int unused1, MatrixTransform* transform, f32 scale, f32 unused4,
                                f32* matrix) {
    f32* modelMatrix;

    if (matrix != NULL) {
        modelMatrix = matrix;
    } else {
        modelMatrix = gCameraDefaultModelMatrix;
    }

    transform->x -= playerMapOffsetX;
    transform->z -= playerMapOffsetZ;
    setMatrixFromObjectPos(modelMatrix, transform);
    if (lbl_803DE5F0 != scale) {
        mtx44ScaleRow1(modelMatrix, scale);
    }

    if (matrix == NULL) {
        mtx44Transpose(modelMatrix, (f32*)lbl_803967C0);
    } else {
        mtx44Transpose(matrix, (f32*)lbl_803967C0);
    }

    PSMTXConcat((MtxPtr)gCameraViewMatrix, (MtxPtr)lbl_803967C0, (MtxPtr)lbl_803967C0);
    GXLoadPosMtxImm(lbl_803967C0, GX_PNMTX0);
    transform->x += playerMapOffsetX;
    transform->z += playerMapOffsetZ;
}

typedef struct CameraViewportEntry {
    u8 opaque00[0x20];
    s32 scissorX;
    s32 scissorY;
    s32 scissorX2;
    s32 scissorY2;
    s32 flags;
} CameraViewportEntry;

STATIC_ASSERT(offsetof(CameraViewportEntry, scissorX) == 0x20);
STATIC_ASSERT(offsetof(CameraViewportEntry, flags) == 0x30);
STATIC_ASSERT(sizeof(CameraViewportEntry) == 0x34);

typedef struct CameraScreenTransform {
    s16 halfWidthQuarterPixels;
    s16 halfHeightQuarterPixels;
    s16 unk04;
    s16 unk06;
    s16 centerXQuarterPixels;
    s16 centerYQuarterPixels;
    s16 unk0C;
    s16 unk0E;
} CameraScreenTransform;

STATIC_ASSERT(offsetof(CameraScreenTransform, halfWidthQuarterPixels) == 0x0);
STATIC_ASSERT(offsetof(CameraScreenTransform, halfHeightQuarterPixels) == 0x2);
STATIC_ASSERT(offsetof(CameraScreenTransform, centerXQuarterPixels) == 0x8);
STATIC_ASSERT(offsetof(CameraScreenTransform, centerYQuarterPixels) == 0xA);
STATIC_ASSERT(sizeof(CameraScreenTransform) == 0x10);

/*
 * The post-scene path deliberately uses view index 4 even though the viewport table has four entries. Its flags read
 * lands 0x30 bytes into the adjacent screen-transform table, so keep this codegen-proven strided view instead of
 * pretending gCameraViewportEntries owns a fifth entry.
 */
#define CAMERA_VIEWPORT_FLAGS(firstFlags, viewIndex)                                                                   \
    (*(s32*)((u8*)(firstFlags) + (viewIndex) * sizeof(CameraViewportEntry)))
#define CAMERA_SCREEN_PARAM(viewIndex, field)                                                                          \
    gCameraViewportScreenParams[(viewIndex) * (sizeof(CameraScreenTransform) / sizeof(s16)) +                          \
                                offsetof(CameraScreenTransform, field) / sizeof(s16)]
#define SCREEN_RESOLUTION_WIDTH(resolution)    ((resolution) & 0xFFFF)
#define SCREEN_RESOLUTION_HEIGHT(resolution)   ((resolution) >> 16)
#define CAMERA_PIXELS_TO_QUARTER_PIXELS(value) ((value) * 4)
#define CAMERA_QUARTER_PIXELS_TO_PIXELS(value) ((value) >> 2)
#define CAMERA_SCREEN_HEIGHT                   480

void screenFn_8000e944(void* viewportArg) {
    u32 resolution;
    u32 height;
    s32* viewportFlags;
    u32 width;
    u8 viewIndex;
    u32 halfWidth;
    s16 halfHeightQuarterPixels;

    gCameraCurrentViewIndex = 4;
    resolution = getScreenResolution();
    height = SCREEN_RESOLUTION_HEIGHT(resolution);
    width = SCREEN_RESOLUTION_WIDTH(resolution);
    viewportFlags = (s32*)(gCameraViewportEntries + offsetof(CameraViewportEntry, flags));

    if ((CAMERA_VIEWPORT_FLAGS(viewportFlags, gCameraCurrentViewIndex) & 1) == 0) {
        gxSetScissorRect(0, 0, 0, 0, width - 1, height - 1);
        halfWidth = width / 2;
        viewIndex = gCameraCurrentViewIndex;
        if ((CAMERA_VIEWPORT_FLAGS(viewportFlags, viewIndex) & 1) == 0) {
            CAMERA_SCREEN_PARAM(viewIndex, centerXQuarterPixels) = (s16)CAMERA_PIXELS_TO_QUARTER_PIXELS(halfWidth);
            halfHeightQuarterPixels = (s16)CAMERA_PIXELS_TO_QUARTER_PIXELS(height / 2);
            CAMERA_SCREEN_PARAM(viewIndex, centerYQuarterPixels) = halfHeightQuarterPixels;
            CAMERA_SCREEN_PARAM(viewIndex, halfWidthQuarterPixels) = (s16)CAMERA_PIXELS_TO_QUARTER_PIXELS(halfWidth);
            CAMERA_SCREEN_PARAM(viewIndex, halfHeightQuarterPixels) = halfHeightQuarterPixels;
        }
    } else {
        Camera_ApplyCurrentViewport(viewportArg);
        viewIndex = gCameraCurrentViewIndex;
        if ((CAMERA_VIEWPORT_FLAGS(viewportFlags, viewIndex) & 1) == 0) {
            CAMERA_SCREEN_PARAM(viewIndex, centerXQuarterPixels) = 0;
            CAMERA_SCREEN_PARAM(viewIndex, centerYQuarterPixels) = 0;
            CAMERA_SCREEN_PARAM(viewIndex, halfWidthQuarterPixels) = 0;
            CAMERA_SCREEN_PARAM(viewIndex, halfHeightQuarterPixels) = 0;
        }
    }

    gCameraCurrentViewIndex = 0;
}

void Camera_NdcToScreen(f32 ndcX, f32 ndcY, f32 ndcZ, s32* outX, s32* outY, s32* outZ) {
    f32 coord;

    if (outX != NULL) {
        coord = ndcX * (f32)CAMERA_QUARTER_PIXELS_TO_PIXELS(CAMERA_SCREEN_PARAM(0, halfWidthQuarterPixels));
        coord = coord + (f32)CAMERA_QUARTER_PIXELS_TO_PIXELS(CAMERA_SCREEN_PARAM(0, centerXQuarterPixels));
        *outX = coord;
    }

    if (outY != NULL) {
        coord = ndcY * (f32)CAMERA_QUARTER_PIXELS_TO_PIXELS(CAMERA_SCREEN_PARAM(0, halfHeightQuarterPixels));
        coord = coord + (f32)CAMERA_QUARTER_PIXELS_TO_PIXELS(CAMERA_SCREEN_PARAM(0, centerYQuarterPixels));
        *outY = coord;
        *outY = CAMERA_SCREEN_HEIGHT - *outY;
    }

    if (outZ != NULL) {
        *outZ = (s32)(gCameraDepth24BitMax * (lbl_803DE5F0 + ndcZ));
    }
}

void Camera_ProjectWorldSphere(f32 x, f32 y, f32 z, f32 radius, f32* outX, f32* outY, f32* outZ, f32* outRadiusX,
                               f32* outRadiusY, f32* outRadiusZ) {
    Vec pos;
    f32 w;
    f32 invW;

    pos.x = x;
    pos.y = y;
    pos.z = z;
    PSMTXMultVec((MtxPtr)gCameraViewMatrix, &pos, &pos);

    *outX = gCameraProjectionMatrix[3] + (gCameraProjectionMatrix[0] * pos.x + gCameraProjectionMatrix[1] * pos.y +
                                          gCameraProjectionMatrix[2] * pos.z);
    *outY = gCameraProjectionMatrix[7] + (gCameraProjectionMatrix[4] * pos.x + gCameraProjectionMatrix[5] * pos.y +
                                          gCameraProjectionMatrix[6] * pos.z);
    *outZ = gCameraProjectionMatrix[11] + (gCameraProjectionMatrix[8] * pos.x + gCameraProjectionMatrix[9] * pos.y +
                                           gCameraProjectionMatrix[10] * pos.z);

    w = gCameraProjectionMatrix[15] + (gCameraProjectionMatrix[12] * pos.x + gCameraProjectionMatrix[13] * pos.y +
                                       gCameraProjectionMatrix[14] * pos.z);
    if (lbl_803DE60C != w) {
        invW = lbl_803DE5F0 / w;
        *outX *= invW;
        *outY *= invW;
        *outZ *= invW;

        pos.z += radius;
        if (pos.z > -1.0f) {
            pos.z = -1.0f;
        }

        w = gCameraProjectionMatrix[15] + (gCameraProjectionMatrix[12] * pos.x + gCameraProjectionMatrix[13] * pos.y +
                                           gCameraProjectionMatrix[14] * pos.z);
        if (lbl_803DE60C != w) {
            invW = lbl_803DE5F0 / w;
            *outRadiusX = fabsf(invW * (radius * gCameraProjectionMatrix[0]));
            *outRadiusY = fabsf(invW * (radius * gCameraProjectionMatrix[5]));
            *outRadiusZ = fabsf(invW * (radius * gCameraProjectionMatrix[10]));
        }
    }
}

void Camera_ProjectWorldPointWithOffset(f32 x, f32 y, f32 z, f32 offset, f32* outX, f32* outY, f32* outZ) {
    Vec pos;
    Vec offsetVec;
    f32 w;
    f32 invW;

    pos.x = x;
    pos.y = y;
    pos.z = z;
    PSMTXMultVec((MtxPtr)gCameraViewMatrix, &pos, &pos);
    PSVECNormalize(&pos, &offsetVec);
    PSVECScale(&offsetVec, &offsetVec, offset);
    PSVECSubtract(&pos, &offsetVec, &pos);

    *outX = gCameraProjectionMatrix[3] + (gCameraProjectionMatrix[0] * pos.x + gCameraProjectionMatrix[1] * pos.y +
                                          gCameraProjectionMatrix[2] * pos.z);
    *outY = gCameraProjectionMatrix[7] + (gCameraProjectionMatrix[4] * pos.x + gCameraProjectionMatrix[5] * pos.y +
                                          gCameraProjectionMatrix[6] * pos.z);
    *outZ = gCameraProjectionMatrix[11] + (gCameraProjectionMatrix[8] * pos.x + gCameraProjectionMatrix[9] * pos.y +
                                           gCameraProjectionMatrix[10] * pos.z);

    w = gCameraProjectionMatrix[15] + (gCameraProjectionMatrix[12] * pos.x + gCameraProjectionMatrix[13] * pos.y +
                                       gCameraProjectionMatrix[14] * pos.z);
    if (lbl_803DE60C != w) {
        invW = lbl_803DE5F0 / w;
        *outX *= invW;
        *outY *= invW;
        *outZ *= invW;
    }
}

void Camera_ProjectWorldPoint(f32 x, f32 y, f32 z, f32* outX, f32* outY, f32* outZ, f32* outViewZ) {
    Vec pos;
    f32 w;
    f32 invW;

    pos.x = x;
    pos.y = y;
    pos.z = z;
    PSMTXMultVec((MtxPtr)gCameraViewMatrix, &pos, &pos);

    *outViewZ = pos.z;
    *outX = gCameraProjectionMatrix[3] + (gCameraProjectionMatrix[0] * pos.x + gCameraProjectionMatrix[1] * pos.y +
                                          gCameraProjectionMatrix[2] * pos.z);
    *outY = gCameraProjectionMatrix[7] + (gCameraProjectionMatrix[4] * pos.x + gCameraProjectionMatrix[5] * pos.y +
                                          gCameraProjectionMatrix[6] * pos.z);
    *outZ = gCameraProjectionMatrix[11] + (gCameraProjectionMatrix[8] * pos.x + gCameraProjectionMatrix[9] * pos.y +
                                           gCameraProjectionMatrix[10] * pos.z);

    w = gCameraProjectionMatrix[15] + (gCameraProjectionMatrix[12] * pos.x + gCameraProjectionMatrix[13] * pos.y +
                                       gCameraProjectionMatrix[14] * pos.z);
    if (lbl_803DE60C != w) {
        invW = lbl_803DE5F0 / w;
        *outX *= invW;
        *outY *= invW;
        *outZ *= invW;
    }
}

void Camera_ApplyCurrentViewport(void* viewportArg) {
    u16 width;
    int viewportY;
    u32 screenSize;

    screenSize = getScreenResolution();
    viewportY = SCREEN_RESOLUTION_HEIGHT(screenSize);
    width = screenSize;
    screenSize = viewportY;
    viewportY = gCameraViewportYOffset + 6;
    screenSize -= viewportY;
    gxSetScissorRect(0, 0, 0, viewportY, width, screenSize);
}

void Camera_UpdateProjection(void* viewportArg, int unused) {
    u8 viewIndex = gCameraCurrentViewIndex;
    u8 activeViewIndex;
    u32 resolution = getScreenResolution();
    u32 screenHeight = SCREEN_RESOLUTION_HEIGHT(resolution);
    u32 screenWidth = SCREEN_RESOLUTION_WIDTH(resolution);
    CameraViewportEntry* base = (CameraViewportEntry*)gCameraViewportEntries;
    CameraViewportEntry* viewport;

    if ((base[viewIndex].flags & 1) != 0) {
        u8 savedViewIndex = gCameraCurrentViewIndex;

        gCameraCurrentViewIndex = viewIndex;
        gxSetScissorRect(0, 0, base[viewIndex & 0xff].scissorX, base[viewIndex & 0xff].scissorY,
                         base[viewIndex & 0xff].scissorX2, base[viewIndex & 0xff].scissorY2);

        viewport = (CameraViewportEntry*)gCameraViewportEntries;
        activeViewIndex = gCameraCurrentViewIndex;
        viewport += activeViewIndex;
        if ((viewport->flags & 1) == 0) {
            CAMERA_SCREEN_PARAM(activeViewIndex, centerXQuarterPixels) = 0;
            CAMERA_SCREEN_PARAM(activeViewIndex, centerYQuarterPixels) = 0;
            CAMERA_SCREEN_PARAM(activeViewIndex, halfWidthQuarterPixels) = 0;
            CAMERA_SCREEN_PARAM(activeViewIndex, halfHeightQuarterPixels) = 0;
        }

        gCameraCurrentViewIndex = savedViewIndex;
        if (gCameraProjectionMode == 1) {
            C_MTXOrtho((Mtx44Ptr)gCameraProjectionMatrix, gCameraOrthoTop, gCameraOrthoBottom, gCameraOrthoLeft,
                       gCameraOrthoRight, gCameraNearPlane, gCameraFarPlane);
        } else {
            C_MTXPerspective((Mtx44Ptr)gCameraProjectionMatrix, gCameraFovY, gCameraAspectRatio, gCameraNearPlane,
                             gCameraFarPlane);
            C_MTXLightPerspective((MtxPtr)lbl_80396850, gCameraFovY, gCameraAspectRatio, 0.4f, 0.4f, 0.5f, 0.5f);
            C_MTXLightPerspective((MtxPtr)lbl_803967F0, gCameraFovY, gCameraAspectRatio, 0.5f, 0.5f, 0.5f, 0.5f);
            C_MTXLightPerspective((MtxPtr)lbl_80396820, gCameraFovY, gCameraAspectRatio, 0.5f, -0.5f, 0.5f, 0.5f);
        }
        GXSetProjection((Mtx44Ptr)gCameraProjectionMatrix, gCameraProjectionMode);
        gCameraCurrentViewIndex = viewIndex;
    } else {
        u32 halfScreenHeight = screenHeight / 2;
        u32 halfScreenWidth = screenWidth / 2;

        activeViewIndex = gCameraCurrentViewIndex;
        viewport = (CameraViewportEntry*)gCameraViewportEntries;
        viewport += activeViewIndex;
        if ((viewport->flags & 1) == 0) {
            CAMERA_SCREEN_PARAM(activeViewIndex, centerXQuarterPixels) =
                (s16)CAMERA_PIXELS_TO_QUARTER_PIXELS(halfScreenWidth);
            CAMERA_SCREEN_PARAM(activeViewIndex, centerYQuarterPixels) =
                (s16)CAMERA_PIXELS_TO_QUARTER_PIXELS(halfScreenHeight);
            CAMERA_SCREEN_PARAM(activeViewIndex, halfWidthQuarterPixels) =
                (s16)CAMERA_PIXELS_TO_QUARTER_PIXELS(halfScreenWidth);
            CAMERA_SCREEN_PARAM(activeViewIndex, halfHeightQuarterPixels) =
                (s16)CAMERA_PIXELS_TO_QUARTER_PIXELS(halfScreenHeight);
        }

        if (gCameraProjectionMode == 1) {
            C_MTXOrtho((Mtx44Ptr)gCameraProjectionMatrix, gCameraOrthoTop, gCameraOrthoBottom, gCameraOrthoLeft,
                       gCameraOrthoRight, gCameraNearPlane, gCameraFarPlane);
        } else {
            C_MTXPerspective((Mtx44Ptr)gCameraProjectionMatrix, gCameraFovY, gCameraAspectRatio, gCameraNearPlane,
                             gCameraFarPlane);
            C_MTXLightPerspective((MtxPtr)lbl_80396850, gCameraFovY, gCameraAspectRatio, 0.4f, 0.4f, 0.5f, 0.5f);
            C_MTXLightPerspective((MtxPtr)lbl_803967F0, gCameraFovY, gCameraAspectRatio, 0.5f, 0.5f, 0.5f, 0.5f);
            C_MTXLightPerspective((MtxPtr)lbl_80396820, gCameraFovY, gCameraAspectRatio, 0.5f, -0.5f, 0.5f, 0.5f);
        }
        GXSetProjection((Mtx44Ptr)gCameraProjectionMatrix, gCameraProjectionMode);
        Camera_ApplyCurrentViewport(viewportArg);
        gCameraCurrentViewIndex = viewIndex;
    }
}

void Camera_GetCurrentViewport(s32* outX, s32* outY, u32* outRight, s32* outBottom) {
    u32 resolution = getScreenResolution();

    *outX = 0;
    *outRight = SCREEN_RESOLUTION_WIDTH(resolution);
    *outY = gCameraViewportYOffset + 6;
    *outBottom = SCREEN_RESOLUTION_HEIGHT(resolution) - (gCameraViewportYOffset + 6);
}

void Camera_SetCurrentViewIndex(int index) {
    if (index >= 0 && index < 4) {
        gCameraCurrentViewIndex = index;
        return;
    }
    gCameraCurrentViewIndex = 0;
}

f32 Camera_DistanceToCurrentViewPosition(f32 x, f32 y, f32 z) {
    CameraViewSlot* slot = &gCameraShakeSlots[gCameraCurrentViewIndex];
    f32 delta;
    f32 dz;
    f32 dx;
    f32 dy;

    delta = z - slot->z;
    dz = delta * delta;
    delta = x - slot->x;
    dx = delta * delta;
    delta = y - slot->y;
    dy = delta * delta;
    return sqrtf(dz + (dx + dy));
}

void Camera_SetCurrentViewRotation(int yaw, int pitch, int roll) {
    CameraViewSlot* slot = &gCameraShakeSlots[gCameraCurrentViewIndex];

    slot->yaw = yaw;
    slot->pitch = pitch;
    slot->roll = roll;
}

void Camera_SetCurrentViewPosition(f32 x, f32 y, f32 z) {
    CameraViewSlot* slot = &gCameraShakeSlots[gCameraCurrentViewIndex];

    slot->x = x;
    slot->y = y;
    slot->z = z;
}

f32* Camera_GetViewRotationMatrix(void) {
    return gCameraViewRotationMatrix;
}

f32* Camera_GetInverseViewRotationMatrix(void) {
    return gCameraInverseViewRotationMatrix;
}

f32* Camera_GetViewMatrix(void) {
    return gCameraViewMatrix;
}

f32* Camera_GetInverseViewMatrix(void) {
    return gCameraInverseViewMatrix;
}

typedef struct CameraMatrixStorage {
    CameraMatrix inverseYawTransforms[0x1E];
    CameraMatrix yawTransforms[0x22];
    f32 worldMatrix[64];
    CameraMatrix defaultModelMatrix;
    CameraViewSlot viewSlots[12];
    CameraMatrix viewRotationMatrix;
    CameraMatrix inverseViewRotationMatrix;
    CameraMatrix viewMatrix;
    CameraMatrix inverseViewMatrix;
    CameraMatrix projectionMatrix;
} CameraMatrixStorage;

STATIC_ASSERT(offsetof(CameraMatrixStorage, yawTransforms) == 0x780);
STATIC_ASSERT(offsetof(CameraMatrixStorage, worldMatrix) == 0x1000);
STATIC_ASSERT(offsetof(CameraMatrixStorage, defaultModelMatrix) == 0x1100);
STATIC_ASSERT(offsetof(CameraMatrixStorage, viewSlots) == 0x1140);
STATIC_ASSERT(offsetof(CameraMatrixStorage, projectionMatrix) == 0x16C0);
STATIC_ASSERT(sizeof(CameraMatrixStorage) == 0x1700);

void Camera_UpdateViewMatrices(void) {
    CameraMatrixStorage* storage;
    CameraViewSlot* viewSlots;
    CameraViewSlot* slot;
    MatrixTransform transform;
    f32 rotationMatrix[16];

    storage = (CameraMatrixStorage*)gObjInverseYawTransformMatrices;
    viewSlots = storage->viewSlots;
    slot = &viewSlots[gCameraCurrentViewIndex];
    transform.x = -(slot->x - playerMapOffsetX);
    transform.y = -slot->y;
    transform.z = -(slot->z - playerMapOffsetZ);
    transform.rotX = slot->yaw + 0x8000;
    transform.rotY = slot->pitch;
    transform.rotZ = slot->roll;
    transform.scale = lbl_803DE5F0;
    if (pauseMenuGetState() == 0) {
        if (cameraViewYOffsetEnabled != 0) {
            transform.y -= slot->shakeMagnitude;
        }
        transform.x += lbl_803DE60C;
        transform.y += lbl_803DE60C;
        transform.z += lbl_803DE60C;
    }

    mtxRotateByVec3s(rotationMatrix, &transform);
    mtx44Transpose(rotationMatrix, storage->viewMatrix);

    transform.x = slot->x - playerMapOffsetX;
    transform.y = slot->y;
    transform.z = slot->z - playerMapOffsetZ;
    transform.rotX = -(slot->yaw + 0x8000);
    transform.rotY = -slot->pitch;
    transform.rotZ = -slot->roll;
    transform.scale = lbl_803DE5F0;
    if (pauseMenuGetState() == 0) {
        if (cameraViewYOffsetEnabled != 0) {
            transform.y += slot->shakeMagnitude;
        }
        transform.x -= lbl_803DE60C;
        transform.y -= lbl_803DE60C;
        transform.z -= lbl_803DE60C;
    }

    setMatrixFromObjectPos(storage->worldMatrix, &transform);
    mtx44Transpose(storage->worldMatrix, storage->inverseViewMatrix);
    PSMTXCopy((MtxPtr)storage->viewMatrix, (MtxPtr)storage->viewRotationMatrix);
    storage->viewRotationMatrix[11] = storage->viewRotationMatrix[7] = storage->viewRotationMatrix[3] = lbl_803DE60C;
    PSMTXCopy((MtxPtr)storage->inverseViewMatrix, (MtxPtr)storage->inverseViewRotationMatrix);
    storage->inverseViewRotationMatrix[11] = storage->inverseViewRotationMatrix[7] =
        storage->inverseViewRotationMatrix[3] = lbl_803DE60C;
}

void Camera_ApplyFullViewport(void) {
    GXRenderModeObj* renderMode = gRenderModeObj;

    if (renderMode->field_rendering != 0) {
        GXSetViewportJitter(lbl_803DE60C, lbl_803DE60C, renderMode->fbWidth, renderMode->xfbHeight, lbl_803DE60C,
                            lbl_803DE5F0, gViewportJitterField);
    } else {
        GXSetViewport(lbl_803DE60C, lbl_803DE60C, renderMode->fbWidth, renderMode->xfbHeight, lbl_803DE60C,
                      lbl_803DE5F0);
    }
}

void Camera_ApplyEffectDepthViewport(void) {
    GXRenderModeObj* renderMode = gRenderModeObj;

    if (renderMode->field_rendering != 0) {
        GXSetViewportJitter(lbl_803DE60C, lbl_803DE60C, renderMode->fbWidth, renderMode->xfbHeight, (-0.075f),
                            lbl_803DE5F0, gViewportJitterField);
    } else {
        GXSetViewport(lbl_803DE60C, lbl_803DE60C, renderMode->fbWidth, renderMode->xfbHeight, (-0.075f), lbl_803DB26C);
    }
}

void Camera_ApplyTransparentViewport(void) {
    GXRenderModeObj* renderMode = gRenderModeObj;

    if (renderMode->field_rendering != 0) {
        GXSetViewportJitter(lbl_803DE60C, lbl_803DE60C, renderMode->fbWidth, renderMode->xfbHeight, (-0.01f),
                            lbl_803DE5F0, gViewportJitterField);
    } else {
        GXSetViewport(lbl_803DE60C, lbl_803DE60C, renderMode->fbWidth, renderMode->xfbHeight, (-0.01f), lbl_803DE5F0);
    }
}

void Camera_ApplyDecalViewport(void) {
    GXRenderModeObj* renderMode = gRenderModeObj;

    if (renderMode->field_rendering != 0) {
        GXSetViewportJitter(lbl_803DE60C, lbl_803DE60C, renderMode->fbWidth, renderMode->xfbHeight, (-0.05f),
                            lbl_803DE5F0, gViewportJitterField);
    } else {
        GXSetViewport(lbl_803DE60C, lbl_803DE60C, renderMode->fbWidth, renderMode->xfbHeight, (-0.05f), lbl_803DE5F0);
    }
}

u16 Camera_GetCurrentViewPitch(void) {
    return gCameraShakeSlots[gCameraCurrentViewIndex].pitch;
}

u16 Camera_GetCurrentViewYaw(void) {
    return gCameraShakeSlots[gCameraCurrentViewIndex].yaw;
}

CameraViewSlot* Camera_GetCurrentViewSlot(void) {
    return &gCameraShakeSlots[gCameraCurrentViewIndex];
}

int Camera_IsViewYOffsetEnabled(void) {
    return cameraViewYOffsetEnabled;
}

void Camera_DisableViewYOffset(void) {
    cameraViewYOffsetEnabled = 0;
}

void Camera_EnableViewYOffset(void) {
    cameraViewYOffsetEnabled = 1;
}

s16 Camera_GetViewportYOffset(void) {
    return cameraViewportYOffset;
}

void Camera_SetViewportYOffset(s16 yOffset) {
    cameraViewportYOffset = yOffset;
}

f32* Camera_GetProjectionMatrix(void) {
    return gCameraProjectionMatrix;
}

void Camera_RebuildProjectionMatrix(void) {
    if (gCameraProjectionMode == 1) {
        C_MTXOrtho((Mtx44Ptr)gCameraProjectionMatrix, gCameraOrthoTop, gCameraOrthoBottom, gCameraOrthoLeft,
                   gCameraOrthoRight, gCameraNearPlane, gCameraFarPlane);
    } else {
        C_MTXPerspective((Mtx44Ptr)gCameraProjectionMatrix, gCameraFovY, gCameraAspectRatio, gCameraNearPlane,
                         gCameraFarPlane);
        C_MTXLightPerspective((MtxPtr)lbl_80396850, gCameraFovY, gCameraAspectRatio, 0.4f, 0.4f, 0.5f, 0.5f);
        C_MTXLightPerspective((MtxPtr)lbl_803967F0, gCameraFovY, gCameraAspectRatio, 0.5f, 0.5f, 0.5f, 0.5f);
        C_MTXLightPerspective((MtxPtr)lbl_80396820, gCameraFovY, gCameraAspectRatio, 0.5f, -0.5f, 0.5f, 0.5f);
    }
    GXSetProjection((Mtx44Ptr)gCameraProjectionMatrix, gCameraProjectionMode);
}

f32 Camera_GetFarPlane(void) {
    return gCameraFarPlane;
}

void Camera_SetFarPlane(f32 farPlane, int transitionFrames) {
    if (transitionFrames != 0) {
        s16 frames = transitionFrames;
        gCameraFarPlaneTransitionFrames = frames;
        gCameraFarPlaneTransitionFramesLeft = frames;
        gCameraFarPlaneTransitionStart = gCameraFarPlane;
        gCameraFarPlaneTransitionTarget = farPlane;
    } else {
        gCameraFarPlane = farPlane;
    }
}

f32 Camera_GetNearPlane(void) {
    return gCameraNearPlane;
}

f32 Camera_GetAspectRatio(void) {
    return gCameraAspectRatio;
}

void Camera_SetAspectRatio(f32 aspectRatio) {
    gCameraAspectRatio = aspectRatio;
}

f32 Camera_GetFovY(void) {
    return gCameraFovY;
}

void Camera_SetFovY(f32 fovY) {
    if (fovY == 0.0f) {
        fovY = 60.0f;
    }
    gCameraFovY = fovY;
}

void Camera_InitState(void) {
    CameraMatrixStorage* storage = (CameraMatrixStorage*)gObjInverseYawTransformMatrices;
    u32 i;
    CameraViewSlot* slot;

    for (i = 0; i < 12; i++) {
        slot = (CameraViewSlot*)((u8*)storage + (u8)i * sizeof(CameraViewSlot));
        slot = (CameraViewSlot*)((u8*)slot + offsetof(CameraMatrixStorage, viewSlots));
        slot->roll = 0;
        slot->pitch = 0;
        slot->yaw = 0x7FF8;
        slot->x = gCameraDefaultPosition;
        slot->y = gCameraDefaultPosition;
        slot->z = gCameraDefaultPosition;
        slot->unk20.x = lbl_803DE60C;
        slot->unk20.y = lbl_803DE60C;
        slot->unk20.z = lbl_803DE60C;
        slot->shakeMagnitude = lbl_803DE60C;
        slot->parentObject = NULL;
        slot->unk5A = 0;
        slot->fovY = 60.0f;
    }

    gCameraCurrentViewIndex = 0;
    cameraViewYOffsetEnabled = 0;
    gObjTransformMatrixSlot = 0;
    gCameraViewportYOffset = 0;
    cameraViewportYOffset = 0;
    gCameraFarPlane = gCameraDefaultFarPlane;
    gCameraFarPlaneTransitionFramesLeft = 0;
    gCameraFovY = 60.0f;
    gCameraProjectionMode = 0;

    if (gCameraProjectionMode == 1) {
        C_MTXOrtho((Mtx44Ptr)storage->projectionMatrix, gCameraOrthoTop, gCameraOrthoBottom, gCameraOrthoLeft,
                   gCameraOrthoRight, gCameraNearPlane, gCameraFarPlane);
    } else {
        C_MTXPerspective((Mtx44Ptr)storage->projectionMatrix, gCameraFovY, gCameraAspectRatio, gCameraNearPlane,
                         gCameraFarPlane);
        C_MTXLightPerspective((MtxPtr)lbl_80396850, gCameraFovY, gCameraAspectRatio, 0.4f, 0.4f, 0.5f, 0.5f);
        C_MTXLightPerspective((MtxPtr)lbl_803967F0, gCameraFovY, gCameraAspectRatio, 0.5f, 0.5f, 0.5f, 0.5f);
        C_MTXLightPerspective((MtxPtr)lbl_80396820, gCameraFovY, gCameraAspectRatio, 0.5f, (-0.5f), 0.5f, 0.5f);
    }
    GXSetProjection((Mtx44Ptr)storage->projectionMatrix, gCameraProjectionMode);

    matrixFn_8006ff0c(storage->worldMatrix + 32, &lbl_803DC88A, gCameraFovY, gCameraAspectRatio, gCameraNearPlane,
                      gCameraFarPlane, lbl_803DE5F0);
    copyMatrix44(storage->worldMatrix + 32, storage->yawTransforms[33]);
}

u8 gCameraViewportEntries[208] = {
    0, 0,   0, 0,   0, 0,  0, 0,   0, 0,   1, 64,  0, 0,  0, 240, 0, 0,   0, 160, 0, 0,   0, 120, 0, 0,   1, 64, 0, 0,
    0, 240, 0, 0,   0, 0,  0, 0,   0, 0,   0, 0,   1, 63, 0, 0,   0, 239, 0, 0,   0, 0,   0, 0,   0, 0,   0, 0,  0, 0,
    0, 0,   1, 64,  0, 0,  0, 240, 0, 0,   0, 160, 0, 0,  0, 120, 0, 0,   1, 64,  0, 0,   0, 240, 0, 0,   0, 0,  0, 0,
    0, 0,   0, 0,   1, 63, 0, 0,   0, 239, 0, 0,   0, 0,  0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   1, 64,  0, 0,  0, 240,
    0, 0,   0, 160, 0, 0,  0, 120, 0, 0,   1, 64,  0, 0,  0, 240, 0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   1, 63, 0, 0,
    0, 239, 0, 0,   0, 0,  0, 0,   0, 0,   0, 0,   0, 0,  0, 0,   1, 64,  0, 0,   0, 240, 0, 0,   0, 160, 0, 0,  0, 120,
    0, 0,   1, 64,  0, 0,  0, 240, 0, 0,   0, 0,   0, 0,  0, 0,   0, 0,   1, 63,  0, 0,   0, 239, 0, 0,   0, 0,
};

s16 gCameraViewportScreenParams[160] = {
    0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0,
    0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0,
    0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0,
    0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0,
    0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0, 0, 0, 511, 0,
};
