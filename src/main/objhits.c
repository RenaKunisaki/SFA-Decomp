#define OBJHITS_SETTERS_S16
#define OBJHITS_STATE_INDEX_S8
#include <string.h>
#include "main/frame_timing.h"
#include "main/shader_api.h"
#include "main/debug.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "game/objects/object.h"
#include "main/model.h"
#include "main/obj_contact.h"
#include "main/obj_list.h"
#include "main/objhits.h"
#include "main/object_transform.h"
#include "main/vecmath.h"
#include "main/track_dolphin_api.h"
#include "dolphin/os.h"
#include "main/asset_load.h"
#include "main/audio/sfx.h"
#include "main/mm.h"
#include "main/objanim_internal.h"
#include "main/objfx.h"
#include "main/objHitReact_types.h"
#include "main/dll/dll_005A_staffcollision.h"
#include "main/resource.h"
#include "dolphin/os/OSReport.h"
#include "dolphin/mtx.h"
#include "main/dll/objpathtransform_struct.h"
#include "main/game_ui_interface.h"
#include "main/lightmap_api.h"
#include "main/dll/player_api.h"
#include "sys/objects/lifecycle.h"
#include "sys/objects.h"
#include "main/objtype.h"
#include "main/obj_hit_region.h"
#include "main/obj_link.h"
#include "main/objlib_api.h"
#include "main/obj_message.h"
#include "main/obj_path.h"
#include "main/obj_query.h"
#include "main/obj_trigger.h"
#include "main/player_eye_anim.h"
#include "main/pad_api.h"
#include "main/audio/sfx_play_api.h"
#include "main/rcp_dolphin_render_api.h"
#include "main/texture.h"
#include "main/objprint_dolphin_api.h"
#include "main/curve_eval.h"
#include "main/objprint_anim_api.h"
#include "main/objprint_character_api.h"
#include "main/objprint_sound_api.h"
#include "main/newshadows.h"
#include "main/objtexture.h"
#include "main/object_render.h"
#include "main/dll/modgfx.h"
#include "dolphin/gx/GXLighting.h"
#include "dolphin/gx/GXPixel.h"
#include "main/acosf.h"
#include "dolphin/gx/GXGeometry.h"
#include "dolphin/gx/GXTev.h"
#include "dolphin/gx/GXTransform.h"
#include "track/intersect_api.h"
#include "main/objprint_internal.h"
#include "main/objprint_render_api.h"

GameObject* gObjHitsActiveHitVolumeObjects[OBJHITS_ACTIVE_HIT_VOLUME_OBJECT_COUNT] = {NULL};
ObjHitsSweepEntry* gObjHitsSweepEntryPtrs[OBJHITS_SWEEP_ENTRY_CAPACITY];
ObjHitsSweepEntry gObjHitsSweepEntries[OBJHITS_SWEEP_ENTRY_CAPACITY];
ObjHitsContactScratchEntry gObjHitsContactScratch[OBJHITS_CONTACT_SCRATCH_COUNT];
extern ObjHitsPriorityWorkSlot* gObjHitsPriorityHitStates;
extern f32 gObjHitsResponseDominanceRatio;

typedef struct ObjHitsVec3 {
    f32 x;
    f32 y;
    f32 z;
} ObjHitsVec3;

extern f32 gObjHitsPriorityHitTickDelta;
static inline ObjHitsModelBank* ObjHits_GetActiveModel(int obj) {
    ObjAnimComponent* objAnim = (ObjAnimComponent*)obj;
    return (ObjHitsModelBank*)objAnim->banks[objAnim->bankIndex];
}

int ObjHits_CollectSkeletonHitsXZ(f32* point, f32 radius, ObjHitsSkeletonJointData* jointData, int* model,
                                  ObjHitsSkeletonHit* hits, ObjHitsSkeletonHit** outBest, f32 yMax, f32 yMin,
                                  f32* outAccum) {
    float doubledPointX;
    float doubledPointZ;
    float diameter;
    float rootCullDistance;
    float* radii;
    int joint;
    int parent;
    int hitCount;
    ObjHitsModelFileHeader* modelFile;
    ObjHitsSkeletonHit* hit;
    ObjModelJointMatrix* jointMatrix;
    float dx;
    float dz;
    float jointRadius;
    float parentRadius;
    float doubledMidpointDeltaX;
    float doubledMidpointDeltaZ;
    float maxJointDiameter;
    float broadPhaseLimit;
    float jointLength;
    float inverseJointLength;
    float distanceMagnitude;
    ObjHitsVec3 jointPos;
    ObjHitsVec3 parentPos;
    ObjHitsVec3 axisDir;
    float axial;
    float distSq;
    float radSum;

    hitCount = 0;
    if (jointData == NULL) {
        return 0;
    }
    modelFile = *(ObjHitsModelFileHeader**)model;
    radii = jointData->jointRadii;
    diameter = radius + radius;
    hit = hits;
    *outBest = hits;
    *outAccum = gObjHitsScalarZero;
    jointMatrix = ObjModel_GetJointMatrix((u8*)model, 0);
    jointPos.x = jointMatrix->translationX;
    jointPos.y = jointMatrix->translationY;
    jointPos.z = jointMatrix->translationZ;
    dx = jointPos.x - point[0];
    dz = jointPos.z - point[2];
    rootCullDistance = sqrtf(dx * dx + gObjHitsScalarZero + dz * dz) - radius;
    doubledPointX = point[0] + point[0];
    doubledPointZ = point[2] + point[2];
    joint = modelFile->jointCount;
    while (--joint != 0) {
        if (jointData->jointCullDistances[joint] > rootCullDistance) {
            parent = modelFile->joints[joint].parentJoint;
            jointMatrix = ObjModel_GetJointMatrix((u8*)model, joint);
            jointPos.x = jointMatrix->translationX;
            jointPos.y = jointMatrix->translationY;
            jointPos.z = jointMatrix->translationZ;
            jointMatrix = ObjModel_GetJointMatrix((u8*)model, parent);
            parentPos.x = jointMatrix->translationX;
            parentPos.y = jointMatrix->translationY;
            parentPos.z = jointMatrix->translationZ;
            jointData->touchedJoints[joint] = 1;
            jointData->touchedJoints[parent] = 1;
            jointRadius = radii[joint];
            parentRadius = radii[parent];
            if ((!(jointPos.y - jointRadius > yMax) || !(parentPos.y - parentRadius > yMax)) &&
                (!(jointPos.y + jointRadius < yMin) || !(parentPos.y + parentRadius < yMin))) {
                doubledMidpointDeltaX = (parentPos.x + jointPos.x) - doubledPointX;
                doubledMidpointDeltaZ = (parentPos.z + jointPos.z) - doubledPointZ;
                broadPhaseLimit = jointData->jointLengths[joint];
                if (jointRadius > parentRadius) {
                    maxJointDiameter = jointRadius + jointRadius;
                } else {
                    maxJointDiameter = parentRadius + parentRadius;
                }
                broadPhaseLimit = diameter + (broadPhaseLimit + maxJointDiameter);
                broadPhaseLimit = broadPhaseLimit * broadPhaseLimit;
                if (doubledMidpointDeltaX * doubledMidpointDeltaX + gObjHitsScalarZero +
                        doubledMidpointDeltaZ * doubledMidpointDeltaZ <
                    broadPhaseLimit) {
                    axisDir.x = parentPos.x - jointPos.x;
                    axisDir.y = parentPos.y - jointPos.y;
                    axisDir.z = parentPos.z - jointPos.z;
                    jointLength = jointData->jointLengths[joint];
                    if (jointLength != gObjHitsScalarZero) {
                        inverseJointLength = gObjHitsScalarOne / jointLength;
                        axisDir.x = axisDir.x * inverseJointLength;
                        axisDir.y = axisDir.y * inverseJointLength;
                        axisDir.z = axisDir.z * inverseJointLength;
                    }
                    jointData->touchedJoints[joint] = 0;
                    jointData->touchedJoints[parent] = 0;
                    if (ObjHits_TestTaperedCapsuleXZ(point, radius, jointRadius, parentRadius, &jointPos.x, &axisDir.x,
                                                     &parentPos.x, jointData->jointLengths[joint], &axial, &distSq,
                                                     &radSum) != 0) {
                        jointData->touchedJoints[joint] = 1;
                        jointData->touchedJoints[parent] = 1;
                        hit->signedSurfaceDistance = radius + (sqrtf(distSq) - radSum);
                        if (gObjHitsScalarZero == hit->signedSurfaceDistance) {
                            hit->signedSurfaceDistance = 1e-06f;
                        }
                        distanceMagnitude = (hit->signedSurfaceDistance > gObjHitsScalarZero)
                                                ? hit->signedSurfaceDistance
                                                : -hit->signedSurfaceDistance;
                        hit->inverseDistance = gObjHitsScalarOne / distanceMagnitude;
                        *outAccum = *outAccum + hit->inverseDistance;
                        if (hit->signedSurfaceDistance < (*outBest)->signedSurfaceDistance) {
                            *outBest = hit;
                        }
                        hit->pointARef = &jointPos.x;
                        hit->pointBRef = &parentPos.x;
                        hit->pointA[0] = jointPos.x;
                        hit->pointA[1] = jointPos.y;
                        hit->pointA[2] = jointPos.z;
                        hit->pointB[0] = parentPos.x;
                        hit->pointB[1] = parentPos.y;
                        hit->pointB[2] = parentPos.z;
                        hit->capsuleAxial = axial;
                        hit->radiusSum = radSum;
                        hit->centerDistance = sqrtf(distSq);
                        hit->axisDir[0] = axisDir.x;
                        hit->axisDir[1] = axisDir.y;
                        hit->axisDir[2] = axisDir.z;
                        hit->pointIndexA = joint;
                        hit->pointIndexB = parent;
                        if (hitCount < OBJHITS_SKELETON_HIT_CAPACITY) {
                            hit += 1;
                            hitCount += 1;
                        }
                    }
                }
            }
        }
    }
    hit->pointIndexA = OBJHITS_SKELETON_HIT_SENTINEL;
    return hit != hits;
}
int ObjHits_CollectSkeletonHits3D(f32* point, f32 radius, ObjHitsSkeletonJointData* jointData, int* model,
                                  ObjHitsSkeletonHit* hits, ObjHitsSkeletonHit** outBest, f32* outAccum) {
    float doubledPointX;
    float doubledPointZ;
    float diameter;
    float rootCullDistance;
    float* radii;
    int joint;
    int parent;
    int hitCount;
    ObjHitsSkeletonHit* hit;
    ObjHitsModelFileHeader* modelFile;
    ObjModelJointMatrix* jointMatrix;
    float dx;
    float dz;
    float jointRadius;
    float parentRadius;
    float doubledMidpointDeltaX;
    float doubledMidpointDeltaZ;
    float maxJointDiameter;
    float broadPhaseLimit;
    float inverseJointLength;
    float distanceMagnitude;
    ObjHitsVec3 jointPos;
    ObjHitsVec3 parentPos;
    ObjHitsVec3 axisDir;
    float axial;
    float distSq;
    float radSum;

    hitCount = 0;
    if (jointData == NULL) {
        return 0;
    }
    modelFile = *(ObjHitsModelFileHeader**)model;
    radii = jointData->jointRadii;
    diameter = radius + radius;
    hit = hits;
    *outBest = hits;
    *outAccum = gObjHitsScalarZero;
    jointMatrix = ObjModel_GetJointMatrix((u8*)model, 0);
    jointPos.x = jointMatrix->translationX;
    jointPos.y = jointMatrix->translationY;
    jointPos.z = jointMatrix->translationZ;
    dx = jointPos.x - point[0];
    dz = jointPos.z - point[2];
    rootCullDistance = sqrtf(dx * dx + gObjHitsScalarZero + dz * dz) - radius;
    doubledPointX = point[0] + point[0];
    doubledPointZ = point[2] + point[2];
    joint = modelFile->jointCount;
    while (--joint != 0) {
        if (jointData->jointCullDistances[joint] > rootCullDistance) {
            parent = modelFile->joints[joint].parentJoint;
            jointMatrix = ObjModel_GetJointMatrix((u8*)model, joint);
            jointPos.x = jointMatrix->translationX;
            jointPos.y = jointMatrix->translationY;
            jointPos.z = jointMatrix->translationZ;
            jointMatrix = ObjModel_GetJointMatrix((u8*)model, parent);
            parentPos.x = jointMatrix->translationX;
            parentPos.y = jointMatrix->translationY;
            parentPos.z = jointMatrix->translationZ;
            jointRadius = radii[joint];
            parentRadius = radii[parent];
            jointData->touchedJoints[joint] = 1;
            jointData->touchedJoints[parent] = 1;
            doubledMidpointDeltaX = (parentPos.x + jointPos.x) - doubledPointX;
            doubledMidpointDeltaZ = (parentPos.z + jointPos.z) - doubledPointZ;
            broadPhaseLimit = jointData->jointLengths[joint];
            if (jointRadius > parentRadius) {
                maxJointDiameter = jointRadius + jointRadius;
            } else {
                maxJointDiameter = parentRadius + parentRadius;
            }
            broadPhaseLimit = diameter + (broadPhaseLimit + maxJointDiameter);
            broadPhaseLimit = broadPhaseLimit * broadPhaseLimit;
            if (doubledMidpointDeltaX * doubledMidpointDeltaX + gObjHitsScalarZero +
                    doubledMidpointDeltaZ * doubledMidpointDeltaZ <
                broadPhaseLimit) {
                axisDir.x = parentPos.x - jointPos.x;
                axisDir.y = parentPos.y - jointPos.y;
                axisDir.z = parentPos.z - jointPos.z;
                inverseJointLength = gObjHitsScalarOne / jointData->jointLengths[joint];
                axisDir.x = axisDir.x * inverseJointLength;
                axisDir.y = axisDir.y * inverseJointLength;
                axisDir.z = axisDir.z * inverseJointLength;
                if (ObjHits_TestTaperedCapsule3D(point, radius, jointRadius, parentRadius, &jointPos.x, &axisDir.x,
                                                 &parentPos.x, jointData->jointLengths[joint], &axial, &distSq,
                                                 &radSum) != 0) {
                    jointData->touchedJoints[joint] = 1;
                    jointData->touchedJoints[parent] = 1;
                    hit->signedSurfaceDistance = radius + (sqrtf(distSq) - radSum);
                    if (gObjHitsScalarZero == hit->signedSurfaceDistance) {
                        hit->signedSurfaceDistance = 1e-06f;
                    }
                    distanceMagnitude = (hit->signedSurfaceDistance > gObjHitsScalarZero) ? hit->signedSurfaceDistance
                                                                                          : -hit->signedSurfaceDistance;
                    hit->inverseDistance = gObjHitsScalarOne / distanceMagnitude;
                    *outAccum = *outAccum + hit->inverseDistance;
                    if (hit->signedSurfaceDistance < (*outBest)->signedSurfaceDistance) {
                        *outBest = hit;
                    }
                    hit->pointARef = &jointPos.x;
                    hit->pointBRef = &parentPos.x;
                    hit->pointA[0] = jointPos.x;
                    hit->pointA[1] = jointPos.y;
                    hit->pointA[2] = jointPos.z;
                    hit->pointB[0] = parentPos.x;
                    hit->pointB[1] = parentPos.y;
                    hit->pointB[2] = parentPos.z;
                    hit->capsuleAxial = axial;
                    hit->radiusSum = radSum;
                    hit->centerDistance = sqrtf(distSq);
                    hit->axisDir[0] = axisDir.x;
                    hit->axisDir[1] = axisDir.y;
                    hit->axisDir[2] = axisDir.z;
                    hit->pointIndexA = joint;
                    hit->pointIndexB = parent;
                    if (hitCount < OBJHITS_SKELETON_HIT_CAPACITY) {
                        hitCount += 1;
                        hit += 1;
                    }
                }
            }
        }
    }
    hit->pointIndexA = OBJHITS_SKELETON_HIT_SENTINEL;
    return hit != hits;
}

int ObjHits_CalcSkeletonResponseXZ(f32* pos, f32 radius, GameObject* obj, ObjHitsSkeletonHit* hits,
                                   ObjHitsSkeletonJointData* jointPoints, int jointModel, ObjHitsSkeletonHit* bestHit,
                                   f32 t, f32 axial, f32* out) {
    float moveLen;
    float zf;
    int idxA;
    float* pPtr;
    float* aPtr;
    ObjHitsSkeletonHit* saved;
    float* rPtr;
    float* norm;
    float* pb;
    float tdiff;
    struct {
        float out[9];
        ObjHitsVec3 accum;
    } pj;
    float reflect[3];
    float normalOut[3];
    ObjHitsVec3 normAccum;
    ObjHitsVec3 diff;
    ObjHitsVec3 move;
    ObjHitsVec3 projPos;

    aPtr = &pj.out[9];
    saved = hits;
    move.x = (obj)->anim.worldPosX - (obj)->anim.previousWorldPosX;
    move.y = (obj)->anim.localPosY - (obj)->anim.previousWorldPosY;
    move.z = (obj)->anim.worldPosZ - (obj)->anim.previousWorldPosZ;
    moveLen = Vec3_Length(&move.x);
    projPos.x = pos[0];
    projPos.y = pos[1];
    projPos.z = pos[2];
    move.x = move.x * t;
    move.y = move.y * t;
    move.z = move.z * t;
    projPos.x = projPos.x - move.x;
    projPos.y = projPos.y - move.y;
    projPos.z = projPos.z - move.z;
    pj.accum.x = gObjHitsScalarZero;
    pj.accum.y = gObjHitsScalarZero;
    pj.accum.z = gObjHitsScalarZero;
    normAccum.x = gObjHitsScalarZero;
    normAccum.y = gObjHitsScalarZero;
    normAccum.z = gObjHitsScalarZero;
    Vec3_Normalize(ObjHits_CalcTaperedCapsuleNormal(&projPos.x, bestHit->capsuleAxial, bestHit->pointA, bestHit->pointB,
                                                    jointPoints->jointRadii[bestHit->pointIndexA],
                                                    jointPoints->jointRadii[bestHit->pointIndexB],
                                                    jointPoints->jointLengths[bestHit->pointIndexA], normalOut));
    pPtr = pj.out;
    zf = 0.0f;
    for (; (idxA = hits->pointIndexA) != OBJHITS_SKELETON_HIT_SENTINEL; hits = hits + 1) {
        pb = ObjHits_ProjectPointToTaperedCapsuleXZ(
            &projPos.x, radius, hits->capsuleAxial, hits->pointA, hits->pointB, jointPoints->jointRadii[idxA],
            jointPoints->jointRadii[hits->pointIndexB], jointPoints->jointLengths[idxA], pPtr);
        if (axial > zf) {
            hits->inverseDistance = hits->inverseDistance / axial;
        } else {
            hits->inverseDistance = zf;
        }
        pb[0] = pb[0] * hits->inverseDistance;
        pb[1] = pb[1] * hits->inverseDistance;
        pb[2] = pb[2] * hits->inverseDistance;
        pj.accum.x = pj.accum.x + pb[0];
        pj.accum.y = pj.accum.y + pb[1];
        pj.accum.z = pj.accum.z + pb[2];
        norm = ObjHits_CalcTaperedCapsuleNormal(
            pos, hits->capsuleAxial, hits->pointA, hits->pointB, jointPoints->jointRadii[hits->pointIndexA],
            jointPoints->jointRadii[hits->pointIndexB], jointPoints->jointLengths[hits->pointIndexA], normalOut);
        Vec3_Normalize(norm);
        normAccum.x = normAccum.x + norm[0];
        normAccum.y = normAccum.y + norm[1];
        normAccum.z = normAccum.z + norm[2];
    }
    Vec3_Normalize(&normAccum.x);
    diff.x = pj.accum.x - projPos.x;
    diff.y = gObjHitsScalarZero;
    diff.z = pj.accum.z - projPos.z;
    axial = Vec3_Length(&diff.x);
    diff.x = pj.accum.x - pos[0];
    diff.y = gObjHitsScalarZero;
    diff.z = pj.accum.z - pos[2];
    Vec3_Normalize(&move.x);
    if (moveLen > axial) {
        f32 responseSpan = 0.25f;

        tdiff = gObjHitsScalarOne - t;
        t = 0.75 + tdiff * responseSpan;
        move.x = move.x * (t * (moveLen - axial));
        move.y = move.y * (t * (moveLen - axial));
        move.z = move.z * (t * (moveLen - axial));
        Vec3_ReflectAgainstNormal(&normAccum.x, &move.x, rPtr = reflect);
    } else {
        rPtr = reflect;
        rPtr[0] = gObjHitsScalarZero;
        rPtr[1] = gObjHitsScalarZero;
        rPtr[2] = gObjHitsScalarZero;
    }
    pj.accum.x = pj.accum.x + rPtr[0];
    pj.accum.y = pj.accum.y + rPtr[1];
    pj.accum.z = pj.accum.z + rPtr[2];
    rPtr[0] = gObjHitsScalarZero;
    rPtr[1] = gObjHitsScalarZero;
    rPtr[2] = gObjHitsScalarZero;
    hits = saved;
    for (; (idxA = hits->pointIndexA) != OBJHITS_SKELETON_HIT_SENTINEL; hits = hits + 1) {
        pb = ObjHits_ProjectPointToTaperedCapsuleXZ(
            aPtr, radius, hits->capsuleAxial, hits->pointA, hits->pointB, jointPoints->jointRadii[idxA],
            jointPoints->jointRadii[hits->pointIndexB], jointPoints->jointLengths[idxA], pPtr);
        pb[0] = pb[0] * hits->inverseDistance;
        pb[1] = pb[1] * hits->inverseDistance;
        pb[2] = pb[2] * hits->inverseDistance;
        rPtr[0] = rPtr[0] + pb[0];
        rPtr[1] = rPtr[1] + pb[1];
        rPtr[2] = rPtr[2] + pb[2];
    }
    *out = rPtr[0] - pos[0];
    out[1] = gObjHitsScalarZero;
    out[2] = rPtr[2] - pos[2];
    return 1;
}

int ObjHits_CalcSkeletonResponse3D(f32* pos, f32 radius, GameObject* obj, ObjHitsSkeletonHit* hits,
                                   ObjHitsSkeletonJointData* jointPoints, int jointModel, ObjHitsSkeletonHit* bestHit,
                                   f32 t, f32 axial, f32* out) {
    float moveLen;
    float zf;
    int idxA;
    float* pPtr;
    float* aPtr;
    ObjHitsSkeletonHit* saved;
    float* rPtr;
    float* norm;
    float* pb;
    struct {
        float out[9];
        ObjHitsVec3 accum;
    } pj;
    float reflect[3];
    float normalOut[3];
    ObjHitsVec3 normAccum;
    ObjHitsVec3 diff;
    ObjHitsVec3 move;
    ObjHitsVec3 projPos;

    aPtr = &pj.out[9];
    saved = hits;
    move.x = (obj)->anim.localPosX - (obj)->anim.previousLocalPosX;
    move.y = (obj)->anim.localPosY - (obj)->anim.previousLocalPosY;
    move.z = (obj)->anim.localPosZ - (obj)->anim.previousLocalPosZ;
    moveLen = Vec3_Length(&move.x);
    projPos.x = pos[0];
    projPos.y = pos[1];
    projPos.z = pos[2];
    projPos.x = projPos.x - move.x;
    projPos.y = projPos.y - move.y;
    projPos.z = projPos.z - move.z;
    pj.accum.x = gObjHitsScalarZero;
    pj.accum.y = gObjHitsScalarZero;
    pj.accum.z = gObjHitsScalarZero;
    normAccum.x = gObjHitsScalarZero;
    normAccum.y = gObjHitsScalarZero;
    normAccum.z = gObjHitsScalarZero;
    Vec3_Normalize(ObjHits_CalcTaperedCapsuleNormal(&projPos.x, bestHit->capsuleAxial, bestHit->pointA, bestHit->pointB,
                                                    jointPoints->jointRadii[bestHit->pointIndexA],
                                                    jointPoints->jointRadii[bestHit->pointIndexB],
                                                    jointPoints->jointLengths[bestHit->pointIndexA], normalOut));
    pPtr = pj.out;
    zf = 0.0f;
    for (; (idxA = hits->pointIndexA) != OBJHITS_SKELETON_HIT_SENTINEL; hits = hits + 1) {
        pb = ObjHits_ProjectPointToTaperedCapsule3D(
            &projPos.x, radius, hits->capsuleAxial, hits->pointA, hits->pointB, jointPoints->jointRadii[idxA],
            jointPoints->jointRadii[hits->pointIndexB], jointPoints->jointLengths[idxA], pPtr);
        if (axial > zf) {
            hits->inverseDistance = hits->inverseDistance / axial;
        } else {
            hits->inverseDistance = zf;
        }
        pb[0] = pb[0] * hits->inverseDistance;
        pb[1] = pb[1] * hits->inverseDistance;
        pb[2] = pb[2] * hits->inverseDistance;
        pj.accum.x = pj.accum.x + pb[0];
        pj.accum.y = pj.accum.y + pb[1];
        pj.accum.z = pj.accum.z + pb[2];
        norm = ObjHits_CalcTaperedCapsuleNormal(
            pos, hits->capsuleAxial, hits->pointA, hits->pointB, jointPoints->jointRadii[hits->pointIndexA],
            jointPoints->jointRadii[hits->pointIndexB], jointPoints->jointLengths[hits->pointIndexA], normalOut);
        Vec3_Normalize(norm);
        normAccum.x = normAccum.x + norm[0];
        normAccum.y = normAccum.y + norm[1];
        normAccum.z = normAccum.z + norm[2];
    }
    Vec3_Normalize(&normAccum.x);
    diff.x = pj.accum.x - projPos.x;
    diff.y = pj.accum.y - projPos.y;
    diff.z = pj.accum.z - projPos.z;
    axial = Vec3_Length(&diff.x);
    diff.x = pj.accum.x - pos[0];
    diff.y = pj.accum.y - pos[1];
    diff.z = pj.accum.z - pos[2];
    Vec3_Normalize(&move.x);
    if (moveLen > axial) {
        move.x = move.x * (moveLen - axial);
        move.y = move.y * (moveLen - axial);
        move.z = move.z * (moveLen - axial);
        Vec3_ReflectAgainstNormal(&normAccum.x, &move.x, rPtr = reflect);
    } else {
        rPtr = reflect;
        rPtr[0] = gObjHitsScalarZero;
        rPtr[1] = gObjHitsScalarZero;
        rPtr[2] = gObjHitsScalarZero;
    }
    pj.accum.x = pj.accum.x + rPtr[0];
    pj.accum.y = pj.accum.y + rPtr[1];
    pj.accum.z = pj.accum.z + rPtr[2];
    rPtr[0] = gObjHitsScalarZero;
    rPtr[1] = gObjHitsScalarZero;
    rPtr[2] = gObjHitsScalarZero;
    hits = saved;
    for (; (idxA = hits->pointIndexA) != OBJHITS_SKELETON_HIT_SENTINEL; hits = hits + 1) {
        pb = ObjHits_ProjectPointToTaperedCapsule3D(
            aPtr, radius, hits->capsuleAxial, hits->pointA, hits->pointB, jointPoints->jointRadii[idxA],
            jointPoints->jointRadii[hits->pointIndexB], jointPoints->jointLengths[idxA], pPtr);
        pb[0] = pb[0] * hits->inverseDistance;
        pb[1] = pb[1] * hits->inverseDistance;
        pb[2] = pb[2] * hits->inverseDistance;
        rPtr[0] = rPtr[0] + pb[0];
        rPtr[1] = rPtr[1] + pb[1];
        rPtr[2] = rPtr[2] + pb[2];
    }
    *out = rPtr[0] - pos[0];
    out[1] = rPtr[1] - pos[1];
    out[2] = rPtr[2] - pos[2];
    return 1;
}

float* ObjHits_ProjectPointToTaperedCapsuleXZ(float* point, float pointRadius, float axial, float* base, float* tip,
                                              float baseRadius, float tipRadius, float length, float* out) {
    float invLength;
    float zero;
    float axisDir[3];
    float surfacePoint[3];

    zero = gObjHitsScalarZero;
    if (axial < zero) {
        out[0] = point[0] - base[0];
        out[1] = zero;
        out[2] = point[2] - base[2];
        Vec3_Normalize(out);
        pointRadius = pointRadius + baseRadius;
        out[0] = out[0] * pointRadius;
        out[1] = out[1] * pointRadius;
        out[2] = out[2] * pointRadius;
        out[0] = out[0] + base[0];
        out[1] = out[1] + base[1];
        out[2] = out[2] + base[2];
        return out;
    }
    if (axial > length) {
        out[0] = point[0] - tip[0];
        out[1] = zero;
        out[2] = point[2] - tip[2];
        Vec3_Normalize(out);
        pointRadius = pointRadius + tipRadius;
        out[0] = out[0] * pointRadius;
        out[1] = out[1] * pointRadius;
        out[2] = out[2] * pointRadius;
        out[0] = out[0] + tip[0];
        out[1] = out[1] + tip[1];
        out[2] = out[2] + tip[2];
        return out;
    }
    axisDir[0] = tip[0] - base[0];
    axisDir[1] = tip[1] - base[1];
    axisDir[2] = tip[2] - base[2];
    invLength = gObjHitsScalarOne / length;
    axisDir[0] = axisDir[0] * invLength;
    axisDir[1] = axisDir[1] * invLength;
    axisDir[2] = axisDir[2] * invLength;
    Vec3_ScaleAdd(base, axisDir, axial, surfacePoint);
    out[0] = point[0] - surfacePoint[0];
    out[1] = gObjHitsScalarZero;
    out[2] = point[2] - surfacePoint[2];
    Vec3_Normalize(out);
    invLength = (tipRadius - baseRadius) * (axial / length);
    pointRadius = invLength + (baseRadius + pointRadius);
    out[0] = out[0] * pointRadius;
    out[1] = out[1] * pointRadius;
    out[2] = out[2] * pointRadius;
    out[0] = out[0] + surfacePoint[0];
    out[1] = out[1] + surfacePoint[1];
    out[2] = out[2] + surfacePoint[2];
    return out;
}

float* ObjHits_ProjectPointToTaperedCapsule3D(float* point, float pointRadius, float axial, float* base, float* tip,
                                              float baseRadius, float tipRadius, float length, float* out) {
    float invLength;
    float axisDir[3];
    float surfacePoint[3];

    if (axial < gObjHitsScalarZero) {
        out[0] = point[0] - base[0];
        out[1] = point[1] - base[1];
        out[2] = point[2] - base[2];
        Vec3_Normalize(out);
        pointRadius = pointRadius + baseRadius;
        out[0] = out[0] * pointRadius;
        out[1] = out[1] * pointRadius;
        out[2] = out[2] * pointRadius;
        out[0] = out[0] + base[0];
        out[1] = out[1] + base[1];
        out[2] = out[2] + base[2];
        return out;
    }
    if (axial > length) {
        out[0] = point[0] - tip[0];
        out[1] = point[1] - tip[1];
        out[2] = point[2] - tip[2];
        Vec3_Normalize(out);
        pointRadius = pointRadius + tipRadius;
        out[0] = out[0] * pointRadius;
        out[1] = out[1] * pointRadius;
        out[2] = out[2] * pointRadius;
        out[0] = out[0] + tip[0];
        out[1] = out[1] + tip[1];
        out[2] = out[2] + tip[2];
        return out;
    }
    axisDir[0] = tip[0] - base[0];
    axisDir[1] = tip[1] - base[1];
    axisDir[2] = tip[2] - base[2];
    invLength = gObjHitsScalarOne / length;
    axisDir[0] = axisDir[0] * invLength;
    axisDir[1] = axisDir[1] * invLength;
    axisDir[2] = axisDir[2] * invLength;
    Vec3_ScaleAdd(base, axisDir, axial, surfacePoint);
    out[0] = point[0] - surfacePoint[0];
    out[1] = point[1] - surfacePoint[1];
    out[2] = point[2] - surfacePoint[2];
    Vec3_Normalize(out);
    invLength = (tipRadius - baseRadius) * (axial / length);
    pointRadius = invLength + (baseRadius + pointRadius);
    out[0] = out[0] * pointRadius;
    out[1] = out[1] * pointRadius;
    out[2] = out[2] * pointRadius;
    out[0] = out[0] + surfacePoint[0];
    out[1] = out[1] + surfacePoint[1];
    out[2] = out[2] + surfacePoint[2];
    return out;
}

float* ObjHits_CalcTaperedCapsuleNormal(float* point, float axial, float* base, float* tip, float baseRadius,
                                        float tipRadius, float length, float* out) {
    float invAxial;
    float radiusDelta;
    float radiusOffset;
    float axisDir[3];
    float normal[3];
    float blended[3];
    float cross[3];
    float surface[3];

    if (axial <= gObjHitsScalarZero) {
        *out = *point - *tip;
        out[1] = point[1] - tip[1];
        out[2] = point[2] - tip[2];
        Vec3_Normalize(out);
        return out;
    }
    if (axial >= length) {
        *out = *point - *tip;
        out[1] = point[1] - tip[1];
        out[2] = point[2] - tip[2];
        Vec3_Normalize(out);
        return out;
    } else {
        radiusDelta = tipRadius - baseRadius;
        radiusOffset = radiusDelta * (axial / length);
        axisDir[0] = tip[0] - base[0];
        axisDir[1] = tip[1] - base[1];
        axisDir[2] = tip[2] - base[2];
        Vec3_Normalize(axisDir);
        Vec3_ScaleAdd(base, axisDir, axial, surface);
        normal[0] = point[0] - surface[0];
        normal[1] = point[1] - surface[1];
        normal[2] = point[2] - surface[2];
        Vec3_Normalize(normal);
        if (radiusDelta == gObjHitsScalarZero) {
            out[0] = normal[0];
            out[1] = normal[1];
            out[2] = normal[2];
            return out;
        } else {
            axisDir[0] = axisDir[0] * axial;
            axisDir[1] = axisDir[1] * axial;
            axisDir[2] = axisDir[2] * axial;
            Vec3_ScaleAdd(axisDir, normal, radiusOffset, blended);
            Vec3_Normalize(blended);
            axisDir[0] = axisDir[0] * (gObjHitsScalarOne / axial);
            invAxial = gObjHitsScalarOne / axial;
            axisDir[1] = axisDir[1] * invAxial;
            axisDir[2] = axisDir[2] * invAxial;
            Vec3_Cross(normal, axisDir, cross);
            Vec3_Normalize(cross);
            Vec3_Cross(cross, blended, out);
        }
    }
    return out;
}

int ObjHits_TestTaperedCapsuleXZ(float* point, float pointRadius, float baseRadius, float tipRadius, float* base,
                                 float* axis, float* tip, float length, float* axial, float* dist2, float* sumR) {
    float deltaX, deltaZ;
    float radialX, radialZ;
    float tipDeltaX, tipDeltaZ;
    float projection;
    float radiusSum;

    deltaX = point[0] - base[0];
    deltaZ = point[2] - base[2];
    *axial = deltaX * axis[0] + deltaZ * axis[2];
    if (*axial > length) {
        tipDeltaX = (tip[0] - point[0]) * (tip[0] - point[0]);
        tipDeltaZ = (tip[2] - point[2]) * (tip[2] - point[2]);
        *dist2 = tipDeltaX + tipDeltaZ;
        radiusSum = pointRadius + tipRadius;
        *sumR = radiusSum;
        return *dist2 <= radiusSum * radiusSum;
    }
    if (*axial < gObjHitsScalarZero) {
        *dist2 = deltaX * deltaX + deltaZ * deltaZ;
        radiusSum = pointRadius + baseRadius;
        *sumR = radiusSum;
        return *dist2 <= radiusSum * radiusSum;
    }
    radialX = axis[0] * (projection = -*axial) + deltaX;
    radialZ = axis[2] * projection + deltaZ;
    *dist2 = radialX * radialX + radialZ * radialZ;
    radiusSum = (*axial / length) * (tipRadius - baseRadius) + (pointRadius + baseRadius);
    *sumR = radiusSum;
    return *dist2 <= radiusSum * radiusSum;
}

int ObjHits_TestTaperedCapsule3D(float* point, float pointRadius, float baseRadius, float tipRadius, float* base,
                                 float* axis, float* tip, float length, float* axial, float* dist2, float* sumR) {
    float deltaX, deltaY, deltaZ;
    float radialX, radialY, radialZ;
    float tipDeltaX, tipDeltaY, tipDeltaZ;
    float radiusSum;

    deltaX = point[0] - base[0];
    deltaY = point[1] - base[1];
    deltaZ = point[2] - base[2];
    *axial = deltaZ * axis[2] + (deltaX * axis[0] + deltaY * axis[1]);
    if (*axial > length) {
        tipDeltaX = tip[0] - point[0];
        tipDeltaY = tip[1] - point[1];
        tipDeltaZ = tip[2] - point[2];
        *dist2 = tipDeltaZ * tipDeltaZ + (tipDeltaX * tipDeltaX + tipDeltaY * tipDeltaY);
        radiusSum = pointRadius + tipRadius;
        *sumR = radiusSum;
        return *dist2 <= radiusSum * radiusSum;
    }
    if (*axial < gObjHitsScalarZero) {
        *dist2 = deltaZ * deltaZ + (deltaX * deltaX + deltaY * deltaY);
        radiusSum = pointRadius + baseRadius;
        *sumR = radiusSum;
        return *dist2 <= radiusSum * radiusSum;
    }
    radialX = axis[0] * -*axial + deltaX;
    radialY = axis[1] * -*axial + deltaY;
    radialZ = axis[2] * -*axial + deltaZ;
    *dist2 = radialZ * radialZ + (radialX * radialX + radialY * radialY);
    radiusSum = (*axial / length) * (tipRadius - baseRadius) + (pointRadius + baseRadius);
    *sumR = radiusSum;
    return *dist2 <= radiusSum * radiusSum;
}

void ObjHits_SortSweepEntries(ObjHitsSweepEntry** sweepPtrs, int entryCount) {
    int maxGap;
    ObjHitsSweepEntry* prevEntry;
    int index;
    int insertIndex;
    int gap;
    ObjHitsSweepEntry* entry;

    gap = 1;
    maxGap = (entryCount - 1) / 9;
    for (; gap <= maxGap; gap = gap * 3 + 1) {
    }
    for (; gap > 0; gap = gap / 3) {
        for (index = gap + 1; index < entryCount; index++) {
            entry = sweepPtrs[index];
            insertIndex = index;
            while ((insertIndex > gap) && (prevEntry = sweepPtrs[insertIndex - gap], prevEntry->minX > entry->minX)) {
                sweepPtrs[insertIndex] = prevEntry;
                insertIndex -= gap;
            }
            sweepPtrs[insertIndex] = entry;
        }
    }
    return;
}

void ObjHits_TickPriorityHitCooldowns(void) {
    int slotOffset;
    short slotIndex;
    u8* base;
    ObjHitsPriorityWorkSlot* workSlot;

    slotIndex = 0;
    slotOffset = 0;
    do {
        base = (u8*)gObjHitsPriorityHitStates;
        workSlot = (ObjHitsPriorityWorkSlot*)(base + slotOffset);
        if (workSlot->active != 0) {
            workSlot->active--;
        }
        slotOffset = slotOffset + OBJHITS_PRIORITY_WORK_SLOT_SIZE;
        slotIndex++;
    } while (slotIndex < OBJHITS_PRIORITY_WORK_SLOT_COUNT);
    gObjHitsPriorityHitTickDelta = timeDelta;
    return;
}

void ObjHitbox_UpdateRotatedBounds(ObjHitbox* hitbox, int advanceMatrix) {
    ObjHitboxTransformState* transformState;
    int matrixBase;
    int matrixFloatOffset;
    MatrixTransform xform;

    transformState = hitbox->transformState;
    if (transformState != 0) {
        if (advanceMatrix != 0) {
            transformState->activeMatrixIndex = (transformState->activeMatrixIndex + 1) & 1;
        }
        matrixFloatOffset = transformState->activeMatrixIndex * OBJHITBOX_STATE_MATRIX_FLOAT_COUNT;
        matrixBase = (int)((float*)transformState->matrices + matrixFloatOffset);
        xform.rotX = -hitbox->rotationX;
        if ((hitbox->def->flags & OBJHITBOX_DEF_CLAMP_Y) != 0) {
            xform.rotY = 0;
        } else {
            xform.rotY = -hitbox->rotationY;
        }
        if ((hitbox->def->flags & OBJHITBOX_DEF_CLAMP_Z) != 0) {
            xform.rotZ = 0;
        } else {
            xform.rotZ = -hitbox->rotationZ;
        }
        xform.scale = gObjHitsScalarOne;
        xform.x = -hitbox->radiusX;
        xform.y = -hitbox->radiusY;
        xform.z = -hitbox->radiusZ;
        mtxRotateByVec3s((float*)matrixBase, &xform);
        xform.rotX = hitbox->rotationX;
        if ((hitbox->def->flags & OBJHITBOX_DEF_CLAMP_Y) != 0) {
            xform.rotY = 0;
        } else {
            xform.rotY = hitbox->rotationY;
        }
        if ((hitbox->def->flags & OBJHITBOX_DEF_CLAMP_Z) != 0) {
            xform.rotZ = 0;
        } else {
            xform.rotZ = hitbox->rotationZ;
        }
        xform.scale = gObjHitsScalarOne;
        xform.x = hitbox->radiusX;
        xform.y = hitbox->radiusY;
        xform.z = hitbox->radiusZ;
        matrixFloatOffset = (transformState->activeMatrixIndex + 2) * OBJHITBOX_STATE_MATRIX_FLOAT_COUNT;
        setMatrixFromObjectPos((float*)transformState->matrices + matrixFloatOffset, &xform);
        if (transformState->resetFrames != 0) {
            transformState->resetFrames--;
        }
    }
    return;
}

int ObjHits_CheckHitVolumes(int objA, int objB, int srcObj, char checkA, char checkB, u32 mask, u32 volMask) {
    ObjHitsContactScratchEntry* contact;
    int countA;
    int countB;
    ObjHitsPriorityState* stateA;
    ObjHitsPriorityState* stateB;
    ObjHitsContactScratchEntry* cw;
    char modeB;
    float* sphB;
    float* curSphA;
    float* curDefA;
    float* spheresA;
    float* spheresB;
    float* defA;
    ObjHitsModelHitVolume* volA;
    ObjHitsModelHitVolume* volB;
    ObjHitsPriorityState* stateSrc;
    s64 volBits;
    ObjHitsContactScratchEntry* contactBase;
    int count;
    char modeA;
    char miss;
    s64 maskB;
    ObjHitsModelHitVolume* p;
    float* pb2;
    ObjHitsModelBank* modelBank;
    ObjHitsModelFileHeader* modelFile;
    s64 maskA;
    ObjHitsContactScratchEntry* cr;
    int result;
    s64 bitA;
    s64 bitB;
    int i;
    int j;
    int k;
    int hit;
    int idxA;
    ObjHitsPriorityState* react;
    u32 linkA;
    u32 linkB;
    u16 link;
    float radiusA;
    float radiusB;
    float dxs;
    float dys;
    float dzs;
    float dsq;
    float radA2;
    float xA;
    float yA;
    float zA;
    float minA;
    float maxA;
    float dax;
    float day;
    float daz;
    float sumSq;
    float ax;
    float ay;
    float az;
    float lenSq;
    float cx;
    float cy;
    float cz;
    float bb;
    float lo;
    float hi;
    float blo;
    float bhi;
    float cc;
    float sb0;
    float disc;
    float q;
    float sc;
    float bestX;
    float bestZ;
    float bestDepth;
    float invLenSq;
    float defs[8];
    float sphs[8];
    u8 volB0[24];
    u8 volA0[24];

    result = 0;
    stateA = (ObjHitsPriorityState*)((ObjAnimComponent*)objA)->hitReactState;
    stateB = (ObjHitsPriorityState*)((ObjAnimComponent*)objB)->hitReactState;
    stateSrc = (ObjHitsPriorityState*)((ObjAnimComponent*)srcObj)->hitReactState;
    if ((stateSrc->secondaryShapeFlags & OBJHITS_SHAPE_MODEL_HIT_VOLUMES) &&
        (*(s8*)&stateSrc->resetHitboxMode != 0 || stateSrc->activeHitboxMode != 0)) {
        return 0;
    }
    if ((stateB->secondaryShapeFlags & OBJHITS_SHAPE_MODEL_HIT_VOLUMES) &&
        (*(s8*)&stateB->resetHitboxMode != 0 || stateB->activeHitboxMode != 0)) {
        return 0;
    }
    modeA = 0;
    modeB = 0;
    if ((checkA != 0 && (stateA->secondaryShapeFlags & OBJHITS_SHAPE_MODEL_HIT_VOLUMES) != 0) ||
        (checkB != 0 && stateA->shapeFlags == OBJHITS_SHAPE_MODEL_HIT_VOLUMES)) {
        modelBank = ObjHits_GetActiveModel(objA);
        modelFile = modelBank->modelFile;
        countA = modelFile->hitVolumeCount;
        spheresA = modelBank->activeHitVolumeSpheres;
        defA = modelBank->hitVolumeSphereBuffers[((modelBank->hitBufferFlags >> 2) & 1) ^ 1];
        volA = modelFile->hitVolumes;
        if ((u32)srcObj != objA) {
            radiusA = stateSrc->secondaryRadiusXZ;
        } else {
            radiusA = stateA->secondaryRadiusXZ;
        }
        if ((((GameObject*)objA)->anim.flags & OBJANIM_FLAG_HIDDEN) != 0) {
            return 0;
        }
    } else {
        countA = 1;
        spheresA = sphs;
        defA = defs;
        volA = (ObjHitsModelHitVolume*)volA0;
        if (stateA->secondaryShapeFlags & OBJHITS_SHAPE_CAPSULE) {
            modeA = 1;
        }
        radiusA = stateA->secondaryRadius;
        sphs[0] = radiusA;
        sphs[1] = ((GameObject*)objA)->anim.worldPosX - playerMapOffsetX;
        sphs[2] = ((GameObject*)objA)->anim.worldPosY;
        sphs[3] = ((GameObject*)objA)->anim.worldPosZ - playerMapOffsetZ;
        defs[0] = radiusA;
        defs[1] = stateA->worldPosX - playerMapOffsetX;
        defs[2] = stateA->worldPosY;
        defs[3] = stateA->worldPosZ - playerMapOffsetZ;
        volA->sphereIndex = 0;
        volA->maskBit = 0;
        volA->linkedSpheres = 0;
    }
    if ((checkA != 0 && (stateB->secondaryShapeFlags & OBJHITS_SHAPE_MODEL_HIT_VOLUMES) != 0) ||
        (checkB != 0 && stateB->shapeFlags == OBJHITS_SHAPE_MODEL_HIT_VOLUMES)) {
        modelBank = ObjHits_GetActiveModel(objB);
        modelFile = modelBank->modelFile;
        countB = modelFile->hitVolumeCount;
        spheresB = modelBank->activeHitVolumeSpheres;
        volB = modelFile->hitVolumes;
        radiusB = stateB->secondaryRadiusXZ;
        if ((((GameObject*)objB)->anim.flags & OBJANIM_FLAG_HIDDEN) != 0) {
            return 0;
        }
    } else {
        countB = 1;
        spheresB = &sphs[4];
        volB = (ObjHitsModelHitVolume*)volB0;
        if (stateB->secondaryShapeFlags & OBJHITS_SHAPE_CAPSULE) {
            modeB = 1;
        }
        radiusB = stateB->secondaryRadius;
        sphs[4] = radiusB;
        sphs[5] = ((GameObject*)objB)->anim.worldPosX - playerMapOffsetX;
        sphs[6] = ((GameObject*)objB)->anim.worldPosY;
        sphs[7] = ((GameObject*)objB)->anim.worldPosZ - playerMapOffsetZ;
        defs[4] = sphs[0];
        defs[5] = stateA->worldPosX - playerMapOffsetX;
        defs[6] = stateA->worldPosY;
        defs[7] = stateA->worldPosZ - playerMapOffsetZ;
        volB->sphereIndex = 0;
        volB->maskBit = 0;
        volB->linkedSpheres = 0;
    }
    if (countA > 64 || countB > 64) {
        debugPrintf(sObjHitsTooManyHitSpheresWarning);
    }
    dxs = ((GameObject*)objA)->anim.worldPosX - ((GameObject*)objB)->anim.worldPosX;
    dys = ((GameObject*)objA)->anim.worldPosY - ((GameObject*)objB)->anim.worldPosY;
    dzs = ((GameObject*)objA)->anim.worldPosZ - ((GameObject*)objB)->anim.worldPosZ;
    dsq = sqrtf(dzs * dzs + (dxs * dxs + (dys * dys)));
    if (dsq > 100.0f + (radiusA + radiusB)) {
        return 0;
    }
    maskA = 0;
    maskB = 0;
    volBits = 0;
    i = 0;
    p = volA;
    for (; i < countA; i++) {
        if (i == p->sphereIndex) {
            if ((mask & 1 << p->maskBit) != 0) {
                maskA |= 1 << i;
            }
            if ((volMask & 1 << p->maskBit) != 0) {
                volBits |= 1 << i;
            }
        }
        p++;
    }
    j = 0;
    p = volB;
    for (; j < countB; j++) {
        if (j == p->sphereIndex) {
            maskB |= 1 << j;
        }
        p++;
    }
    contactBase = gObjHitsContactScratch;
    bestDepth = -1.0f;
    count = 1;
    while (count != 0) {
        count = 0;
        i = 0;
        curSphA = spheresA;
        curDefA = defA;
        contact = contactBase;
        for (; i < countA; i++) {
            bitA = 1 << i;
            if ((maskA & bitA) != 0) {
                radA2 = curSphA[0];
                xA = curSphA[1];
                yA = curSphA[2];
                zA = curSphA[3];
                miss = 1;
                if ((volBits & bitA) != 0) {
                    miss = 0;
                }
                if (miss == 0) {
                    dax = curDefA[1];
                    day = curDefA[2];
                    daz = curDefA[3];
                    ax = xA - dax;
                    ay = yA - day;
                    az = zA - daz;
                    lenSq = az * az + (ax * ax + (ay * ay));
                    if (lenSq > gObjHitsScalarZero) {
                        invLenSq = gObjHitsScalarOne / lenSq;
                    } else {
                        miss = 1;
                    }
                }
                j = 0;
                sphB = spheresB;
                cw = contact;
                minA = yA - radA2;
                maxA = yA + radA2;
                for (; j < countB; j++) {
                    bitB = 1 << j;
                    if ((maskB & bitB) != 0) {
                        hit = 0;
                        if ((i == 0 && modeA != 0) || (j == 0 && modeB != 0)) {
                            if (modeA != 0) {
                                lo = yA + stateA->secondaryCapsuleOffsetA;
                                hi = yA + stateA->secondaryCapsuleOffsetB;
                                blo = sphB[2] - sphB[0];
                                bhi = sphB[2] + sphB[0];
                            } else {
                                lo = minA;
                                hi = maxA;
                                blo = stateB->secondaryCapsuleOffsetA + sphB[2];
                                bhi = stateB->secondaryCapsuleOffsetB + sphB[2];
                            }
                            if ((!(blo < lo) || !(bhi < lo)) && (!(blo > hi) || !(bhi > hi))) {
                                sumSq = radA2 + sphB[0];
                                sumSq = sumSq * sumSq;
                                dxs = xA - sphB[1];
                                dsq = dxs * dxs;
                                if (dsq < sumSq) {
                                    dzs = zA - sphB[3];
                                    dsq = dzs * dzs + dsq;
                                    if (dsq < sumSq) {
                                        dys = gObjHitsScalarZero;
                                        hit = 1;
                                    }
                                }
                            }
                        } else {
                            sumSq = (radA2 + sphB[0]) * (radA2 + sphB[0]);
                            if (miss != 0) {
                                dxs = xA - sphB[1];
                                dsq = dxs * dxs;
                                if (dsq < sumSq) {
                                    dys = yA - sphB[2];
                                    dsq = dys * dys + dsq;
                                    if (dsq < sumSq) {
                                        dzs = zA - sphB[3];
                                        dsq = dzs * dzs + dsq;
                                        if (dsq < sumSq) {
                                            hit = 1;
                                        }
                                    }
                                }
                            } else {
                                cx = dax - sphB[1];
                                cy = day - sphB[2];
                                cz = daz - sphB[3];
                                cc = (cz * cz + (cx * cx + (cy * cy))) - sumSq;
                                bb = cz * az + (cx * ax + (cy * ay));
                                if (!(bb > gObjHitsScalarZero) || !(cc > gObjHitsScalarZero)) {
                                    disc = bb * bb - lenSq * cc;
                                    if (disc >= *(f32*)&gObjHitsScalarZero) {
                                        q = lenSq + bb;
                                        if (q >= *(f32*)&gObjHitsScalarZero || q * q <= disc) {
                                            hit = 1;
                                            sc = sqrtf(disc);
                                            sc = invLenSq * -(bb + sc);
                                            dxs = ax * sc + cx;
                                            dys = ay * sc + cy;
                                            dzs = az * sc + cz;
                                            dsq = dzs * dzs + (dxs * dxs + (dys * dys));
                                        }
                                    }
                                }
                            }
                        }
                        if (hit != 0 && count < 64) {
                            if (checkB != 0) {
                                if (dsq > gObjHitsScalarZero) {
                                    bb = sqrtf(sumSq);
                                    dsq = sqrtf(dsq);
                                    if (bb > gObjHitsScalarZero) {
                                        sumSq = (bb - dsq) / bb;
                                    } else {
                                        sumSq = gObjHitsScalarZero;
                                    }
                                    cw->depth = sumSq;
                                    cw->responseX = dxs * sumSq;
                                    cw->responseZ = dzs * sumSq;
                                }
                            } else {
                                sumSq = sqrtf(dzs * dzs + (dxs * dxs + (dys * dys)));
                                if (sumSq > gObjHitsScalarZero) {
                                    dxs = dxs / sumSq;
                                    dys = dys / sumSq;
                                    dzs = dzs / sumSq;
                                }
                                sb0 = sphB[0];
                                cw->contactOffsetX = dxs * sb0;
                                cw->contactOffsetY = dys * sb0;
                                cw->contactOffsetZ = dzs * sb0;
                            }
                            cw->sphereIndexA = i;
                            cw->sphereIndexB = j;
                            cw++;
                            contact++;
                            count += 1;
                        }
                    }
                    sphB += 4;
                }
            }
            curSphA += 4;
            curDefA += 4;
        }
        maskA = 0;
        maskB = 0;
        k = 0;
        cr = contactBase;
        for (; k < count; k++) {
            idxA = cr->sphereIndexA;
            hit = cr->sphereIndexB;
            linkA = volA[idxA].linkedSpheres;
            linkB = volB[hit].linkedSpheres;
            link = linkA;
            while (link != 0) {
                maskA |= 1 << (idxA + (u16)((link & 0xf000) >> 12));
                link = link << 4;
            }
            link = linkB;
            while (link != 0) {
                maskB |= 1 << (hit + (u16)((link & 0xf000) >> 12));
                link = link << 4;
            }
            if (linkA == 0 && linkB == 0) {
                if (checkA != 0) {
                    pb2 = &spheresB[hit * 4];
                    cx = pb2[1] + cr->contactOffsetX;
                    ObjHits_RecordPositionHit((GameObject*)objB, (GameObject*)objA, stateSrc->hitVolumePriority,
                                              (u8)stateSrc->hitVolumeId, hit, cx,
                                              (modeB != 0) ? spheresA[idxA * 4 + 2] : pb2[2] + cr->contactOffsetY,
                                              pb2[3] + cr->contactOffsetZ);
                    result = 1;
                } else if (checkB != 0) {
                    if (cr->depth > bestDepth) {
                        bestDepth = cr->depth;
                        bestX = cr->responseX;
                        bestZ = cr->responseZ;
                    }
                }
            } else if (linkA == 0) {
                maskA |= 1 << idxA;
            } else if (linkB == 0) {
                maskB |= 1 << hit;
            }
            cr++;
        }
    }
    if (checkA != 0 && result != 0) {
        if ((stateA->flags & 0x80) != 0) {
            react = ObjAnim_GetPriorityHitState((ObjAnimComponent*)objA);
            if (react != 0) {
                react->flags = react->flags & ~OBJHITS_PRIORITY_STATE_ENABLED;
            }
        }
        if ((stateB->flags & 0x80) != 0) {
            react = ObjAnim_GetPriorityHitState((ObjAnimComponent*)objB);
            if (react != 0) {
                react->flags = react->flags & ~OBJHITS_PRIORITY_STATE_ENABLED;
            }
        }
        return 1;
    }
    if (checkB != 0) {
        if (bestDepth > gObjHitsScalarZero) {
            if ((u32)objA == srcObj) {
                ObjHits_RecordObjectHit((GameObject*)objB, (GameObject*)objA, stateSrc->objectPairPriority,
                                        stateSrc->objectPairHitVolume, hit);
                ObjHits_RecordObjectHit((GameObject*)objA, (GameObject*)objB, stateB->objectPairPriority,
                                        stateB->objectPairHitVolume, idxA);
                ObjHits_ApplyPairResponse(objA, objB, -bestX, gObjHitsScalarZero, -bestZ, 0);
                return 1;
            }
        }
    }
    return 0;
}

void ObjHits_OnPlayerHitVolumeMiss(int objA, int objB, int attachment, void* state, void* attachmentState, f32 dt) {
}

void ObjHits_CheckObjectHitVolumes(int objA, int objB, int attA, int attB, f32 dt) {
    ObjHitsPriorityState* attStateB;
    ObjHitsPriorityState* stateB;
    ObjHitsPriorityState* attStateA;
    ObjHitsPriorityState* stateA;
    ObjHitsModelBank* hitboxBuf;
    u32 bufIndex;
    u32 mask;
    u8 result;
    stateB = (ObjHitsPriorityState*)((GameObject*)objB)->anim.hitReactState;
    stateA = (ObjHitsPriorityState*)((GameObject*)objA)->anim.hitReactState;
    if ((u32)attA != 0) {
        attStateA = ObjAnim_GetPriorityHitState((ObjAnimComponent*)attA);
    } else {
        attStateA = NULL;
    }
    if ((u32)attB != 0) {
        attStateB = ObjAnim_GetPriorityHitState((ObjAnimComponent*)attB);
    } else {
        attStateB = NULL;
    }
    result = 0;
    if ((stateA->objectHitMask != 0) && (stateA->suppressOutgoingHits == 0)) {
        if (((GameObject*)objA)->anim.classId == 1) {
            hitboxBuf = ObjHits_GetActiveModel(objA);
            bufIndex = (hitboxBuf->hitBufferFlags >> 2) & 1;
            if ((stateA->flags & OBJHITS_PRIORITY_STATE_HITBOX_BUFFER_CACHED) != 0) {
                memcpy(hitboxBuf->hitVolumeSphereBuffers[bufIndex], gObjHitsPrimaryHitboxBufferScratch0,
                       hitboxBuf->modelFile->hitVolumeCount << 4);
                memcpy(hitboxBuf->hitVolumeSphereBuffers[bufIndex ^ 1], gObjHitsPrimaryHitboxBufferScratch1,
                       hitboxBuf->modelFile->hitVolumeCount << 4);
            } else {
                memcpy(gObjHitsPrimaryHitboxBufferScratch0, hitboxBuf->hitVolumeSphereBuffers[bufIndex],
                       hitboxBuf->modelFile->hitVolumeCount << 4);
                memcpy(gObjHitsPrimaryHitboxBufferScratch1, hitboxBuf->hitVolumeSphereBuffers[bufIndex ^ 1],
                       hitboxBuf->modelFile->hitVolumeCount << 4);
            }
            if ((u32)attA != 0) {
                hitboxBuf = ObjHits_GetActiveModel(attA);
                bufIndex = (hitboxBuf->hitBufferFlags >> 2) & 1;
                if ((stateA->flags & OBJHITS_PRIORITY_STATE_HITBOX_BUFFER_CACHED) != 0) {
                    memcpy(hitboxBuf->hitVolumeSphereBuffers[bufIndex], gObjHitsSecondaryHitboxBufferScratch0,
                           hitboxBuf->modelFile->hitVolumeCount << 4);
                    memcpy(hitboxBuf->hitVolumeSphereBuffers[bufIndex ^ 1], gObjHitsSecondaryHitboxBufferScratch1,
                           hitboxBuf->modelFile->hitVolumeCount << 4);
                } else {
                    memcpy(gObjHitsSecondaryHitboxBufferScratch0, hitboxBuf->hitVolumeSphereBuffers[bufIndex],
                           hitboxBuf->modelFile->hitVolumeCount << 4);
                    memcpy(gObjHitsSecondaryHitboxBufferScratch1, hitboxBuf->hitVolumeSphereBuffers[bufIndex ^ 1],
                           hitboxBuf->modelFile->hitVolumeCount << 4);
                    stateA->flags = stateA->flags | OBJHITS_PRIORITY_STATE_HITBOX_BUFFER_CACHED;
                }
            }
        }
        mask = stateA->objectHitMask >> 4;
        if (mask != 0) {
            result = ObjHits_CheckHitVolumes(objA, objB, objA, 1, 0, mask, stateA->skeletonHitMask >> 4);
        }
        if ((((u32)attA != 0) && (result == 0)) && (mask = stateA->objectHitMask & 0xf, mask != 0)) {
            result = ObjHits_CheckHitVolumes(attA, objB, objA, 1, 0, mask, stateA->skeletonHitMask & 0xf);
        }
        if ((result == 0) && (((GameObject*)objA)->anim.classId == 1)) {
            ObjHits_OnPlayerHitVolumeMiss(objA, objB, attA, stateA, attStateA, dt);
        }
    }
    result = 0;
    if (((stateB->sourceMask & 0x80) == 0) && (stateB->objectHitMask != 0) && (stateB->suppressOutgoingHits == 0)) {
        if (((GameObject*)objB)->anim.classId == 1) {
            hitboxBuf = ObjHits_GetActiveModel(objB);
            bufIndex = (hitboxBuf->hitBufferFlags >> 2) & 1;
            if ((stateB->flags & OBJHITS_PRIORITY_STATE_HITBOX_BUFFER_CACHED) != 0) {
                memcpy(hitboxBuf->hitVolumeSphereBuffers[bufIndex], gObjHitsPrimaryHitboxBufferScratch0,
                       hitboxBuf->modelFile->hitVolumeCount << 4);
                memcpy(hitboxBuf->hitVolumeSphereBuffers[bufIndex ^ 1], gObjHitsPrimaryHitboxBufferScratch1,
                       hitboxBuf->modelFile->hitVolumeCount << 4);
            } else {
                memcpy(gObjHitsPrimaryHitboxBufferScratch0, hitboxBuf->hitVolumeSphereBuffers[bufIndex],
                       hitboxBuf->modelFile->hitVolumeCount << 4);
                memcpy(gObjHitsPrimaryHitboxBufferScratch1, hitboxBuf->hitVolumeSphereBuffers[bufIndex ^ 1],
                       hitboxBuf->modelFile->hitVolumeCount << 4);
            }
            if ((u32)attB != 0) {
                hitboxBuf = ObjHits_GetActiveModel(attB);
                bufIndex = (hitboxBuf->hitBufferFlags >> 2) & 1;
                if ((stateB->flags & OBJHITS_PRIORITY_STATE_HITBOX_BUFFER_CACHED) != 0) {
                    memcpy(hitboxBuf->hitVolumeSphereBuffers[bufIndex], gObjHitsSecondaryHitboxBufferScratch0,
                           hitboxBuf->modelFile->hitVolumeCount << 4);
                    memcpy(hitboxBuf->hitVolumeSphereBuffers[bufIndex ^ 1], gObjHitsSecondaryHitboxBufferScratch1,
                           hitboxBuf->modelFile->hitVolumeCount << 4);
                } else {
                    memcpy(gObjHitsSecondaryHitboxBufferScratch0, hitboxBuf->hitVolumeSphereBuffers[bufIndex],
                           hitboxBuf->modelFile->hitVolumeCount << 4);
                    memcpy(gObjHitsSecondaryHitboxBufferScratch1, hitboxBuf->hitVolumeSphereBuffers[bufIndex ^ 1],
                           hitboxBuf->modelFile->hitVolumeCount << 4);
                    stateB->flags = stateB->flags | OBJHITS_PRIORITY_STATE_HITBOX_BUFFER_CACHED;
                }
            }
        }
        mask = stateB->objectHitMask >> 4;
        if (mask != 0) {
            result = ObjHits_CheckHitVolumes(objB, objA, objB, 1, 0, mask, stateB->skeletonHitMask >> 4);
        }
        if ((((u32)attB != 0) && (result == 0)) && (mask = stateB->objectHitMask & 0xf, mask != 0)) {
            result = ObjHits_CheckHitVolumes(attB, objA, objB, 1, 0, mask, stateB->skeletonHitMask & 0xf);
        }
        if ((result == 0) && (((GameObject*)objB)->anim.classId == 1)) {
            ObjHits_OnPlayerHitVolumeMiss(objB, objA, attB, stateB, attStateB, dt);
        }
    }
}
void ObjHits_RegisterActiveHitVolumeObject(GameObject* obj) {
    int index;

    index = 0;
    while (index < OBJHITS_ACTIVE_HIT_VOLUME_OBJECT_COUNT && gObjHitsActiveHitVolumeObjects[index] != NULL) {
        index = index + 1;
    }
    if (index == OBJHITS_ACTIVE_HIT_VOLUME_OBJECT_COUNT) {
        gObjHitsActiveHitVolumeObjects[0] = obj;
        return;
    }
    gObjHitsActiveHitVolumeObjects[index] = obj;
    return;
}

void ObjHits_ApplyPairResponse(int objA, int objB, f32 x, f32 y, f32 z, int flag) {
    ObjAnimComponent* animA;
    ObjAnimComponent* animB;
    ObjHitsPriorityState* stateA;
    ObjHitsPriorityState* stateB;
    f32 localAx;
    f32 localAy;
    f32 localAz;
    f32 localBx;
    f32 localBy;
    f32 localBz;
    int angleA;
    int angleB;
    u32 angle;
    f32 cosVal;
    f32 cosSq;
    f32 weightA;
    f32 weightB;
    f32 sum;
    f32 blend;
    f32 invBlend;

    ObjContact_DispatchCallbacks((GameObject*)objA, (GameObject*)objB);
    animA = &((GameObject*)objA)->anim;
    animB = &((GameObject*)objB)->anim;
    stateA = (ObjHitsPriorityState*)animA->hitReactState;
    stateB = (ObjHitsPriorityState*)animB->hitReactState;
    stateA->flags = stateA->flags | 8;
    stateB->flags = stateB->flags | 8;
    *(int*)stateA = objB;
    *(int*)stateB = objA;
    if (animA->parent != NULL) {
        Obj_TransformWorldVectorToLocal(x, y, z, &localAx, &localAy, &localAz, animA->parent);
    } else {
        localAx = x;
        localAy = y;
        localAz = z;
    }
    if (animB->parent != NULL) {
        Obj_TransformWorldVectorToLocal(x, y, z, &localBx, &localBy, &localBz, animB->parent);
    } else {
        localBx = x;
        localBy = y;
        localBz = z;
    }
    if ((animA->classId == 1) && (stateA->lateralResponseWeight != 0) &&
        ((stateB->flags & OBJHITS_PRIORITY_STATE_IMMOVABLE) == 0)) {
        animA->localPosX = animA->localPosX - localAx;
        animA->localPosY = animA->localPosY - localAy;
        animA->localPosZ = animA->localPosZ - localAz;
        if (flag != 0) {
            animA->worldPosX = animA->worldPosX - x;
            animA->worldPosY = animA->worldPosY - y;
            animA->worldPosZ = animA->worldPosZ - z;
        } else {
            Obj_TransformLocalPointToWorld(animA->localPosX, animA->localPosY, animA->localPosZ, &animA->worldPosX,
                                           &animA->worldPosY, &animA->worldPosZ, animA->parent);
        }
    } else if ((animB->classId == 1) && (stateB->lateralResponseWeight != 0) &&
               ((stateA->flags & OBJHITS_PRIORITY_STATE_IMMOVABLE) == 0)) {
        animB->localPosX = animB->localPosX + localBx;
        animB->localPosY = animB->localPosY + localBy;
        animB->localPosZ = animB->localPosZ + localBz;
        if (flag != 0) {
            animB->worldPosX = animB->worldPosX + x;
            animB->worldPosY = animB->worldPosY + y;
            animB->worldPosZ = animB->worldPosZ + z;
        } else {
            Obj_TransformLocalPointToWorld(animB->localPosX, animB->localPosY, animB->localPosZ, &animB->worldPosX,
                                           &animB->worldPosY, &animB->worldPosZ, animB->parent);
        }
    } else if (stateB->lateralResponseWeight == 0) {
        if (stateA->lateralResponseWeight != 0) {
            animA->localPosX = animA->localPosX - localAx;
            animA->localPosY = animA->localPosY - localAy;
            animA->localPosZ = animA->localPosZ - localAz;
            if (flag != 0) {
                animA->worldPosX = animA->worldPosX - x;
                animA->worldPosY = animA->worldPosY - y;
                animA->worldPosZ = animA->worldPosZ - z;
            } else {
                Obj_TransformLocalPointToWorld(animA->localPosX, animA->localPosY, animA->localPosZ, &animA->worldPosX,
                                               &animA->worldPosY, &animA->worldPosZ, animA->parent);
            }
        }
    } else if (stateA->lateralResponseWeight == 0) {
        if (stateB->lateralResponseWeight != 0) {
            animB->localPosX = animB->localPosX + localBx;
            animB->localPosY = animB->localPosY + localBy;
            animB->localPosZ = animB->localPosZ + localBz;
            if (flag != 0) {
                animB->worldPosX = animB->worldPosX + x;
                animB->worldPosY = animB->worldPosY + y;
                animB->worldPosZ = animB->worldPosZ + z;
            } else {
                Obj_TransformLocalPointToWorld(animB->localPosX, animB->localPosY, animB->localPosZ, &animB->worldPosX,
                                               &animB->worldPosY, &animB->worldPosZ, animB->parent);
            }
        }
    } else {
        angle = getAngle(-x, -z) & 0xffff;
        angleA = animA->rotX - angle;
        if (angleA > 0x8000) {
            angleA -= 0xffff;
        }
        if (angleA < -0x8000) {
            angleA += 0xffff;
        }
        angleB = animB->rotX - (int)((angle + 0x8000) & 0xffff);
        if (angleB > 0x8000) {
            angleB -= 0xffff;
        }
        if (angleB < -0x8000) {
            angleB += 0xffff;
        }
        cosVal = mathCosf((3.1415927f * angleA) / 32768.0f);
        cosSq = cosVal * cosVal;
        weightA = stateA->lateralResponseWeight * cosSq + stateA->axialResponseWeight * (gObjHitsScalarOne - cosSq);
        cosVal = mathCosf((3.1415927f * angleB) / 32768.0f);
        cosSq = cosVal * cosVal;
        weightB = stateB->lateralResponseWeight * cosSq + stateB->axialResponseWeight * (gObjHitsScalarOne - cosSq);
        if (weightA < weightB * gObjHitsResponseDominanceRatio) {
            weightA = gObjHitsScalarZero;
        } else if (weightB < weightA * gObjHitsResponseDominanceRatio) {
            weightB = gObjHitsScalarZero;
        }
        sum = weightA + weightB;
        if (sum > gObjHitsScalarZero) {
            blend = weightB / sum;
        } else {
            blend = gObjHitsScalarZero;
        }
        animA->localPosX = animA->localPosX - localAx * blend;
        animA->localPosY = animA->localPosY - localAy * blend;
        animA->localPosZ = animA->localPosZ - localAz * blend;
        Obj_TransformLocalPointToWorld(animA->localPosX, animA->localPosY, animA->localPosZ, &animA->worldPosX,
                                       &animA->worldPosY, &animA->worldPosZ, animA->parent);
        invBlend = gObjHitsScalarOne - blend;
        animB->localPosX = localBx * invBlend + animB->localPosX;
        animB->localPosY = localBy * invBlend + animB->localPosY;
        animB->localPosZ = localBz * invBlend + animB->localPosZ;
        Obj_TransformLocalPointToWorld(animB->localPosX, animB->localPosY, animB->localPosZ, &animB->worldPosX,
                                       &animB->worldPosY, &animB->worldPosZ, animB->parent);
    }
}

void ObjHits_DetectObjectPair(int objA, int objB) {
    ObjHitsPriorityState* stateA;
    f32 cy;
    f32 cz;
    int distInt;
    f32 segSq;
    f32 dist;
    f32 sumRadius;
    f32 radiusA;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 radiusB;
    f32 nx;
    f32 ny;
    f32 nz;
    f32 yA;
    f32 yB;
    f32 tmp;
    ObjHitsPriorityState* stateB;
    f32 sx;
    char vertical;
    int distClamped;
    f32 cx;
    f32 sy;
    f32 sz;

    stateA = (ObjHitsPriorityState*)((GameObject*)objA)->anim.hitReactState;
    stateB = (ObjHitsPriorityState*)((GameObject*)objB)->anim.hitReactState;
    if (stateA->activeHitboxMode != 0 || stateB->activeHitboxMode != 0) {
        return;
    }
    dx = ((GameObject*)objB)->anim.worldPosX - ((GameObject*)objA)->anim.worldPosX;
    yB = ((GameObject*)objB)->anim.worldPosY;
    yA = ((GameObject*)objA)->anim.worldPosY;
    dy = yB - yA;
    dz = ((GameObject*)objB)->anim.worldPosZ - ((GameObject*)objA)->anim.worldPosZ;
    radiusA = stateA->primaryRadius;
    radiusB = stateB->primaryRadius;
    vertical = 0;
    if (((stateB->shapeFlags & OBJHITBOX_SHAPE_VERTICAL_SPAN) != 0) ||
        ((stateA->shapeFlags & OBJHITBOX_SHAPE_VERTICAL_SPAN) != 0)) {
        if (dy > *(f32*)&gObjHitsScalarZero) {
            if ((stateA->shapeFlags & OBJHITBOX_SHAPE_VERTICAL_SPAN) != 0) {
                yA = yA + stateA->primaryCapsuleOffsetB;
            } else {
                yA = yA + radiusA;
            }
            if ((stateB->shapeFlags & OBJHITBOX_SHAPE_VERTICAL_SPAN) != 0) {
                tmp = yB + stateB->primaryCapsuleOffsetA;
            } else {
                tmp = yB - radiusB;
            }
            if (tmp > yA) {
                return;
            }
        } else {
            if ((stateB->shapeFlags & OBJHITBOX_SHAPE_VERTICAL_SPAN) != 0) {
                yB = yB + stateB->primaryCapsuleOffsetB;
            } else {
                yB = yB + radiusB;
            }
            if ((stateA->shapeFlags & OBJHITBOX_SHAPE_VERTICAL_SPAN) != 0) {
                tmp = yA + stateA->primaryCapsuleOffsetA;
            } else {
                tmp = yA - radiusA;
            }
            if (tmp > yB) {
                return;
            }
        }
        dy = gObjHitsScalarZero;
        vertical = 1;
    }
    dist = dx * dx + dy * dy + dz * dz;
    if (dist != gObjHitsScalarZero) {
        dist = sqrtf(dist);
    }
    distInt = (int)(f32)(int)dist;
    distClamped = distInt;
    if (distInt > 0x400) {
        distClamped = 0x400;
    }
    if (distClamped <= stateA->capsuleScale) {
        stateA->capsuleScale = distClamped;
    }
    if (distInt > 0x400) {
        distInt = 0x400;
    }
    if (distInt <= stateB->capsuleScale) {
        stateB->capsuleScale = distInt;
    }
    if ((stateB->flags & OBJHITS_PRIORITY_STATE_ENABLED) != 0) {
        sumRadius = radiusB + radiusA;
        sx = ((GameObject*)objA)->anim.worldPosX - stateA->worldPosX;
        sy = ((GameObject*)objA)->anim.worldPosY - stateA->worldPosY;
        sz = ((GameObject*)objA)->anim.worldPosZ - stateA->worldPosZ;
        if (vertical != 0) {
            sy = gObjHitsScalarZero;
        }
        segSq = sx * sx + sy * sy + sz * sz;
        if (segSq > gObjHitsScalarOne) {
            cx = ((GameObject*)objB)->anim.worldPosX - stateA->worldPosX;
            cz = ((GameObject*)objB)->anim.worldPosZ - stateA->worldPosZ;
            cy = ((GameObject*)objB)->anim.worldPosY - stateA->worldPosY;
            segSq = (sx * cx + sy * cy + sz * cz) / segSq;
            if ((segSq >= gObjHitsScalarZero) && (segSq <= gObjHitsScalarOne)) {
                f32 oz;
                f32 ox;
                f32 oy;

                tmp = (segSq * sz + stateA->worldPosZ) - ((GameObject*)objB)->anim.worldPosZ;
                oz = tmp * tmp;
                tmp = (segSq * sx + stateA->worldPosX) - ((GameObject*)objB)->anim.worldPosX;
                ox = tmp * tmp;
                tmp = (segSq * sy + stateA->worldPosY) - ((GameObject*)objB)->anim.worldPosY;
                oy = tmp * tmp;
                dist = sqrtf(oz + (ox + oy));
            }
        }
        if ((dist < sumRadius) && (dist > gObjHitsScalarZero)) {
            ObjHits_RecordObjectHit((GameObject*)objB, (GameObject*)objA, stateA->objectPairPriority,
                                    stateA->objectPairHitVolume, 0);
            ObjHits_RecordObjectHit((GameObject*)objA, (GameObject*)objB, stateB->objectPairPriority,
                                    stateB->objectPairHitVolume, 0);
            if (((stateB->flags & OBJHITS_PRIORITY_STATE_NO_SEPARATION_RESPONSE) == 0) &&
                ((stateA->flags & OBJHITS_PRIORITY_STATE_NO_SEPARATION_RESPONSE) == 0)) {
                nx = stateB->worldPosX - stateA->worldPosX;
                ny = stateB->worldPosY - stateA->worldPosY;
                nz = stateB->worldPosZ - stateA->worldPosZ;
                if (vertical != 0) {
                    ny = gObjHitsScalarZero;
                }
                tmp = sqrtf(nx * nx + ny * ny + nz * nz);
                if (tmp > gObjHitsScalarZero) {
                    dx = nx / tmp;
                    dy = ny / tmp;
                    dz = nz / tmp;
                } else {
                    dx = dx / dist;
                    dy = dy / dist;
                    dz = dz / dist;
                }
                dx = dx * (sumRadius - dist);
                dy = dy * (sumRadius - dist);
                dz = dz * (sumRadius - dist);
                ObjHits_ApplyPairResponse(objA, objB, dx, dy, dz, 0);
            }
        }
    }
}

void ObjHits_CheckSkeletonPair(int objA, int objB, void* hits, void* scratchB, void* scratchC, void* scratchD,
                               void* scratchE, int depth) {
    int* hitboxBuf;
    f32 outAxial;
    ObjHitsPriorityState* objAState;
    u8 shapeFlags;
    int hitCount;
    f32 ratio;
    f32 responseX;
    f32 responseY;
    f32 responseZ;
    ObjHitsSkeletonHit* bestHit;
    ObjHitsPriorityState* objBState;
    ObjHitsVec3 point;
    f32 response[3];
    ObjHitsVec3 point3D;
    ObjHitsVec3 pointXZ;

    objBState = (ObjHitsPriorityState*)((GameObject*)objB)->anim.hitReactState;
    objAState = (ObjHitsPriorityState*)((GameObject*)objA)->anim.hitReactState;
    if (*(s8*)&objAState->resetHitboxMode != 0 || *(s8*)&objBState->resetHitboxMode != 0 ||
        objBState->activeHitboxMode != 0 || objAState->activeHitboxMode != 0) {
        return;
    }
    hitboxBuf = (int*)ObjHits_GetActiveModel(objA);
    shapeFlags = objBState->shapeFlags;
    if ((shapeFlags & OBJHITBOX_SHAPE_SKELETON_3D) != 0) {
        point.x = ((GameObject*)objB)->anim.worldPosX - playerMapOffsetX;
        point.y = ((GameObject*)objB)->anim.worldPosY;
        point.z = ((GameObject*)objB)->anim.worldPosZ - playerMapOffsetZ;
        point3D = point;
        hitCount =
            ObjHits_CollectSkeletonHits3D(&point3D.x, objBState->primaryRadius, (ObjHitsSkeletonJointData*)hitboxBuf[5],
                                          hitboxBuf, (ObjHitsSkeletonHit*)hits, &bestHit, &outAxial);
        if (hitCount != 0) {
            ratio = (((GameObject*)objB)->anim.hitboxScale * ((GameObject*)objB)->anim.rootMotionScale) /
                    (((GameObject*)objA)->anim.hitboxScale * ((GameObject*)objA)->anim.rootMotionScale);

            {
                f32* pos = &point.x;
                f32 rad = objBState->primaryRadius;
                u32 ob = objB;
                ObjHitsSkeletonHit* hh = (ObjHitsSkeletonHit*)hits;
                ObjHitsSkeletonJointData* jd = (ObjHitsSkeletonJointData*)hitboxBuf[5];
                int mf = *hitboxBuf;
                ObjHitsSkeletonHit* bh = bestHit;
                ObjHits_CalcSkeletonResponse3D(pos, rad, (GameObject*)(ob), hh, jd, mf, bh,
                                               (ratio < gObjHitsScalarZero)
                                                   ? gObjHitsScalarZero
                                                   : ((ratio > gObjHitsScalarOne) ? gObjHitsScalarOne : ratio),
                                               outAxial, response);
            }
            response[0] = ((responseX = response[0]) < -10.0f) ? -10.0f : ((responseX > 10.0f) ? 10.0f : responseX);
            responseY = response[1];
            response[1] = (responseY < -10.0f) ? -10.0f : ((responseY > 10.0f) ? 10.0f : responseY);
            responseZ = response[2];
            response[2] = (responseZ < -10.0f) ? -10.0f : ((responseZ > 10.0f) ? 10.0f : responseZ);
            ObjHits_ApplyPairResponse(objA, objB, response[0], response[1], (f32)(f64)response[2], 0);
        }
    } else if ((shapeFlags & OBJHITBOX_SHAPE_VERTICAL_SPAN) != 0) {
        point.x = ((GameObject*)objB)->anim.worldPosX - playerMapOffsetX;
        point.y = ((GameObject*)objB)->anim.worldPosY;
        point.z = ((GameObject*)objB)->anim.worldPosZ - playerMapOffsetZ;
        pointXZ = point;
        hitCount = ObjHits_CollectSkeletonHitsXZ(
            &pointXZ.x, objBState->primaryRadius, (ObjHitsSkeletonJointData*)hitboxBuf[5], hitboxBuf,
            (ObjHitsSkeletonHit*)hits, &bestHit, point.y + objBState->primaryCapsuleOffsetB,
            point.y + objBState->primaryCapsuleOffsetA, &outAxial);
        if (hitCount != 0) {
            ratio = (((GameObject*)objB)->anim.hitboxScale * ((GameObject*)objB)->anim.rootMotionScale) /
                    (((GameObject*)objA)->anim.hitboxScale * ((GameObject*)objB)->anim.rootMotionScale);

            {
                f32* pos = &point.x;
                f32 rad = objBState->primaryRadius;
                u32 ob = objB;
                ObjHitsSkeletonHit* hh = (ObjHitsSkeletonHit*)hits;
                ObjHitsSkeletonJointData* jd = (ObjHitsSkeletonJointData*)hitboxBuf[5];
                int mf = *hitboxBuf;
                ObjHitsSkeletonHit* bh = bestHit;
                ObjHits_CalcSkeletonResponseXZ(pos, rad, (GameObject*)(ob), hh, jd, mf, bh,
                                               (ratio < gObjHitsScalarZero)
                                                   ? gObjHitsScalarZero
                                                   : ((ratio > gObjHitsScalarOne) ? gObjHitsScalarOne : ratio),
                                               outAxial, response);
            }
            response[0] = ((responseX = response[0]) < -10.0f) ? -10.0f : ((responseX > 10.0f) ? 10.0f : responseX);
            responseY = response[1];
            response[1] = (responseY < -10.0f) ? -10.0f : ((responseY > 10.0f) ? 10.0f : responseY);
            responseZ = response[2];
            response[2] = (responseZ < -10.0f) ? -10.0f : ((responseZ > 10.0f) ? 10.0f : responseZ);
            ObjHits_ApplyPairResponse(objA, objB, response[0], response[1], (f32)(f64)response[2], 0);
        }
    } else if (((shapeFlags & OBJHITS_SHAPE_SKELETON) != 0) && (depth < 1)) {
        ObjHits_CheckSkeletonPair(objB, objA, hits, scratchB, scratchC, scratchD, scratchE, depth + 1);
    }
}

void ObjHits_CheckTrackContact(int objA, int objB) {
    u32 sphereIdx;
    int mask2;
    u8 contact;
    ObjHitsPriorityState* stateA;
    u32 bits;
    ObjHitsModelBank* modelBank;
    int i;
    ObjHitsModelFileHeader* modelFile;
    float* curSpheres;
    int prevSpheres;
    ObjHitsPriorityState* stateB;
    int pointCount;
    TrackQueryBounds bounds;
    struct {
        u8 out[64];
        f32 radii[4];
        s8 ids[4];
        u8 sevens[4];
        u8 pad58[4];
        int kinds[5];
    } hb;
    float endPoints[18];
    float startPoints[18];
    f32 fConv;

    stateA = (ObjHitsPriorityState*)((GameObject*)objA)->anim.hitReactState;
    mask2 = (u32)objB == objA ? stateA->objectHitMask >> 4 : stateA->objectHitMask & 0xf;
    if ((mask2 != 0) && (stateA->suppressOutgoingHits == 0)) {
        stateB = (ObjHitsPriorityState*)((GameObject*)objB)->anim.hitReactState;
        if ((stateB->secondaryShapeFlags & OBJHITS_SHAPE_MODEL_HIT_VOLUMES) != 0) {
            modelBank = ObjHits_GetActiveModel(objB);
            modelFile = modelBank->modelFile;
            bits = modelBank->hitBufferFlags >> 2 & 1;
            curSpheres = modelBank->hitVolumeSphereBuffers[bits];
            prevSpheres = (int)modelBank->hitVolumeSphereBuffers[bits ^ 1];
            pointCount = 0;
            for (i = 0; i < (int)(u32)modelFile->hitVolumeCount; i = i + 1) {
                if ((i == modelFile->hitVolumes[i].sphereIndex) &&
                    ((mask2 & 1 << modelFile->hitVolumes[i].maskBit) != 0)) {
                    bits = modelFile->hitVolumes[i].linkedSpheres;
                    if (bits != 0) {
                        for (; (u16)bits != 0; bits = (u16)((bits & 0xffff) << 4)) {
                            sphereIdx = (((u16)bits & 0xf000) >> 0xc) + i & 0xffff;
                            if (pointCount < 4) {
                                float* curEntry;
                                int prevEntry;
                                int sphereOff = sphereIdx * 0x10;
                                curEntry = (float*)((u8*)curSpheres + sphereOff);
                                endPoints[pointCount * 3] = playerMapOffsetX + curEntry[1];
                                endPoints[pointCount * 3 + 1] = curEntry[2];
                                endPoints[pointCount * 3 + 2] = playerMapOffsetZ + curEntry[3];
                                prevEntry = prevSpheres + sphereOff;
                                startPoints[pointCount * 3] = playerMapOffsetX + *(float*)(prevEntry + 4);
                                startPoints[pointCount * 3 + 1] = *(float*)(prevEntry + 8);
                                startPoints[pointCount * 3 + 2] = playerMapOffsetZ + *(float*)(prevEntry + 0xc);
                                hb.radii[pointCount] = *curEntry;
                                hb.ids[pointCount] = -1;
                                hb.sevens[pointCount] = 7;
                                pointCount = pointCount + 1;
                            }
                        }
                    } else {
                        if (pointCount < 4) {
                            endPoints[pointCount * 3] = playerMapOffsetX + curSpheres[i * 4 + 1];
                            endPoints[pointCount * 3 + 1] = curSpheres[i * 4 + 2];
                            endPoints[pointCount * 3 + 2] = playerMapOffsetZ + curSpheres[i * 4 + 3];
                            startPoints[pointCount * 3] = playerMapOffsetX + *(float*)(prevSpheres + i * 0x10 + 4);
                            startPoints[pointCount * 3 + 1] = *(float*)(prevSpheres + i * 0x10 + 8);
                            startPoints[pointCount * 3 + 2] =
                                playerMapOffsetZ + *(float*)(prevSpheres + i * 0x10 + 0xc);
                            hb.radii[pointCount] = curSpheres[i * 4];
                            hb.ids[pointCount] = -1;
                            hb.sevens[pointCount] = 7;
                            pointCount = pointCount + 1;
                        }
                    }
                }
            }
        } else {
            endPoints[0] = ((GameObject*)objA)->anim.worldPosX;
            endPoints[1] = ((GameObject*)objA)->anim.worldPosY;
            endPoints[2] = ((GameObject*)objA)->anim.worldPosZ;
            startPoints[0] = ((GameObject*)objA)->anim.previousWorldPosX;
            startPoints[1] = ((GameObject*)objA)->anim.previousWorldPosY;
            startPoints[2] = ((GameObject*)objA)->anim.previousWorldPosZ;
            fConv = (f32)(u32)((GameObject*)objA)->anim.modelInstance->fallbackHitSphereRadius;
            if (fConv < 0.1f) {
                fConv = 0.1f;
            }
            hb.radii[0] = fConv;
            hb.ids[0] = -1;
            hb.sevens[0] = 7;
            pointCount = 1;
        }
        if (pointCount != 0) {
            hitDetect_calcSweptSphereBounds(&bounds, startPoints, endPoints, hb.radii, pointCount);
            trackIntersectBroadphase((GameObject*)objB, &bounds, stateB->trackContactMask, 1);
            contact = trackGetIntersect((GameObject*)objB, startPoints, endPoints, pointCount, hb.out, 0);
            if (contact != 0) {
                if ((contact & 1) != 0) {
                    pointCount = 0;
                } else if ((contact & 2) != 0) {
                    pointCount = 1;
                } else if ((contact & 4) != 0) {
                    pointCount = 2;
                } else {
                    pointCount = 3;
                }
                stateB->contactHitVolume = hb.ids[pointCount];
                stateB->contactPosX = endPoints[pointCount * 3];
                stateB->contactPosY = endPoints[pointCount * 3 + 1];
                stateB->contactPosZ = endPoints[pointCount * 3 + 2];
                if (hb.kinds[pointCount] != 0u) {
                    stateB->contactFlags = stateB->contactFlags | OBJHITS_CONTACT_FLAG_KIND_NONZERO;
                } else {
                    stateB->contactFlags = stateB->contactFlags | OBJHITS_CONTACT_FLAG_KIND0;
                }
            }
        }
    }
}

void ObjHits_Update(int objectCount) {
    u8 skeletonScratchB[1036];
    u8 skeletonScratchC[1040];
    u8 skeletonHits[1512];
    u8 skeletonScratchD[100];
    u8 skeletonScratchE[100];
    int listObj;
    ObjHitsSweepEntry** entrySlot;
    ObjHitsSweepEntry* nextEntry;
    int slotIndex;
    int obj;
    ObjHitsPriorityState* objState;
    int candidateIndex;
    int slotCount;
    int candObj;
    ObjHitsSweepEntry** entrySlotBase;
    ObjHitsPriorityState* candState;
    int currentIndex;
    u32 attachedObj;
    ObjHitsSweepEntry* sweepEntries;
    int listCount;
    int startIndex;
    ObjHitsSweepEntry* entry;
    ObjHitsSweepEntry* candidateEntry;
    int* objectList;
    u32 candAttachedObj;
    f32 axisDiff;
    f32 diff;
    int hitVolumeIndex;

    objectList = ObjList_GetObjects(&startIndex, &listCount);
    sweepEntries = gObjHitsSweepEntries;
    sweepEntries->minX = -36288576.0f;
    sweepEntries->maxX = -36288576.0f;
    gObjHitsSweepEntryPtrs[0] = sweepEntries;
    slotCount = 1;
    entrySlotBase = &gObjHitsSweepEntryPtrs[1];
    nextEntry = &sweepEntries[1];
    entrySlot = entrySlotBase;
    for (; objectCount > 0; objectCount--) {
        {
            ObjHitsPriorityState* listState;

            listObj = *objectList;
            listState = (ObjHitsPriorityState*)((GameObject*)listObj)->anim.hitReactState;
            if (listState != NULL) {
                if (((listState->flags &
                      (OBJHITS_PRIORITY_STATE_ENABLED | OBJHITS_PRIORITY_STATE_NO_SEPARATION_RESPONSE)) != 0) &&
                    (listState->shapeFlags != 8) && (slotCount < OBJHITS_SWEEP_ENTRY_CAPACITY)) {
                    *entrySlot = nextEntry;
                    (*entrySlot)->obj = listObj;
                    (*entrySlot)->minX = ((GameObject*)listObj)->anim.worldPosX - listState->sweepRadiusX;
                    nextEntry++;
                    entrySlot++;
                    gObjHitsSweepEntryPtrs[slotCount++]->maxX =
                        ((GameObject*)listObj)->anim.worldPosX + listState->sweepRadiusX;
                }
                listState->flags = listState->flags & ~OBJHITS_PRIORITY_STATE_PAIR_RESPONSE_APPLIED;
                listState->contactFlags = 0;
                listState->contactHitVolume = -1;
                *(int*)listState = 0;
                attachedObj = (u32)((GameObject*)listObj)->childObjs[0];
                if ((attachedObj != 0) && (((GameObject*)attachedObj)->anim.classId == 0x2d)) {
                    listState = ObjAnim_GetPriorityHitState((ObjAnimComponent*)attachedObj);
                    listState->flags = listState->flags & ~OBJHITS_PRIORITY_STATE_PAIR_RESPONSE_APPLIED;
                    listState->contactFlags = 0;
                    listState->contactHitVolume = -1;
                    *(int*)listState = 0;
                }
            }
            objectList++;
        }
    }
    ObjHits_SortSweepEntries(gObjHitsSweepEntryPtrs, slotCount);
    currentIndex = 1;
    slotIndex = 1;
    entrySlot = entrySlotBase;
    for (; slotIndex < slotCount; entrySlot++, slotIndex++) {
        entry = *entrySlot;
        obj = entry->obj;
        objState = (ObjHitsPriorityState*)((GameObject*)obj)->anim.hitReactState;
        attachedObj = (u32)((GameObject*)obj)->childObjs[0];
        if ((attachedObj != 0) && ((ObjAnim_GetPriorityHitState((ObjAnimComponent*)attachedObj) == NULL) ||
                                   ((ObjAnim_GetPriorityHitState((ObjAnimComponent*)attachedObj)->flags &
                                     OBJHITS_PRIORITY_STATE_ENABLED) == 0))) {
            attachedObj = 0;
        }
        if ((objState->flags & 4) != 0) {
            ObjHitsSweepEntry** skipSlot;
            candidateIndex = currentIndex;
            skipSlot = &gObjHitsSweepEntryPtrs[currentIndex];
            for (; (entry->minX > (*skipSlot)->maxX) && (candidateIndex < slotCount); candidateIndex++) {
                skipSlot++;
            }
            currentIndex = candidateIndex;
            while ((candidateIndex < slotCount) &&
                   ((*entrySlot)->maxX > gObjHitsSweepEntryPtrs[candidateIndex]->minX)) {
                candidateEntry = gObjHitsSweepEntryPtrs[candidateIndex];
                if ((*entrySlot)->minX > candidateEntry->maxX) {
                    candidateIndex++;
                    continue;
                }
                {
                    candObj = candidateEntry->obj;
                    candState = (ObjHitsPriorityState*)((GameObject*)candObj)->anim.hitReactState;
                    if ((slotIndex != candidateIndex) && ((u32)((GameObject*)obj)->anim.parent != candObj)) {
                        diff = ((GameObject*)obj)->anim.worldPosZ - ((GameObject*)candObj)->anim.worldPosZ;
                        diff = (diff > gObjHitsScalarZero) ? diff : -diff;
                        if (diff < objState->primaryRadiusXZ + candState->primaryRadiusXZ) {
                            diff = ((GameObject*)obj)->anim.worldPosY - ((GameObject*)candObj)->anim.worldPosY;
                            diff = (diff > *(const f32*)&gObjHitsScalarZero) ? diff : -diff;
                            if ((diff < objState->primaryRadiusY + candState->primaryRadiusY) &&
                                ((objState->flags & OBJHITS_PRIORITY_STATE_POSITION_DIRTY) == 0) &&
                                ((candState->flags & OBJHITS_PRIORITY_STATE_POSITION_DIRTY) == 0) &&
                                (((candState->flags & 4) == 0) || (slotIndex >= candidateIndex)) &&
                                ((((GameObject*)obj)->anim.modelInstance->runtimeSourceHitMask &
                                  candState->targetMask) != 0) &&
                                ((((GameObject*)candObj)->anim.modelInstance->runtimeSourceHitMask &
                                  objState->targetMask) != 0)) {
                                if ((candState->shapeFlags & OBJHITS_SHAPE_SKELETON) != 0) {
                                    ObjHits_CheckSkeletonPair(candObj, obj, skeletonHits, skeletonScratchB,
                                                              skeletonScratchC, skeletonScratchD, skeletonScratchE, 0);
                                } else if ((objState->shapeFlags & OBJHITS_SHAPE_SKELETON) != 0) {
                                    ObjHits_CheckSkeletonPair(obj, candObj, skeletonHits, skeletonScratchB,
                                                              skeletonScratchC, skeletonScratchD, skeletonScratchE, 0);
                                } else if ((objState->shapeFlags == OBJHITS_SHAPE_MODEL_HIT_VOLUMES) ||
                                           (candState->shapeFlags == OBJHITS_SHAPE_MODEL_HIT_VOLUMES)) {
                                    if ((objState->lateralResponseWeight != 0) ||
                                        (candState->lateralResponseWeight != 0)) {
                                        ObjHits_CheckHitVolumes(obj, candObj, obj, 0, 1, 0xffffffff, 0);
                                    }
                                } else if ((objState->lateralResponseWeight != 0) ||
                                           (candState->lateralResponseWeight != 0)) {
                                    ObjHits_DetectObjectPair(obj, candObj);
                                }
                            }
                        }
                        if (diff < objState->secondaryRadiusXZ + candState->secondaryRadiusXZ) {
                            axisDiff =
                                (((GameObject*)obj)->anim.worldPosY - ((GameObject*)candObj)->anim.worldPosY >
                                 gObjHitsScalarZero)
                                    ? ((GameObject*)obj)->anim.worldPosY - ((GameObject*)candObj)->anim.worldPosY
                                    : -(((GameObject*)obj)->anim.worldPosY - ((GameObject*)candObj)->anim.worldPosY);
                            if ((axisDiff < objState->secondaryRadiusY + candState->secondaryRadiusY) &&
                                ((objState->flags & OBJHITS_PRIORITY_STATE_HIT_EXCLUDED) == 0) &&
                                ((candState->flags & OBJHITS_PRIORITY_STATE_HIT_EXCLUDED) == 0) &&
                                ((objState->sourceMask & candState->targetMask) != 0) &&
                                (((candState->sourceMask & 0x80) != 0) ||
                                 ((candState->sourceMask & objState->targetMask) != 0))) {
                                candAttachedObj = (u32)((GameObject*)candObj)->childObjs[0];
                                if ((candAttachedObj != 0) &&
                                    ((ObjAnim_GetPriorityHitState((ObjAnimComponent*)candAttachedObj) == NULL) ||
                                     ((ObjAnim_GetPriorityHitState((ObjAnimComponent*)candAttachedObj)->flags &
                                       OBJHITS_PRIORITY_STATE_ENABLED) == 0))) {
                                    candAttachedObj = 0;
                                }
                                ObjHits_CheckObjectHitVolumes(obj, candObj, attachedObj, candAttachedObj, timeDelta);
                            }
                        }
                    }
                }
                candidateIndex++;
            }
        }
    }
    for (slotIndex = 1, entrySlot = entrySlotBase; slotIndex < slotCount; entrySlot++, slotIndex++) {
        obj = (*entrySlot)->obj;
        if (((((GameObject*)obj)->anim.hitReactState)->flags & OBJHITS_PRIORITY_STATE_TRACK_CONTACT) != 0) {
            ObjHits_CheckTrackContact(obj, obj);
            attachedObj = (u32)((GameObject*)obj)->childObjs[0];
            if (attachedObj != 0) {
                ObjHits_CheckTrackContact(obj, attachedObj);
            }
        }
    }
    for (slotIndex = 1; slotIndex < slotCount; entrySlotBase++, slotIndex++) {
        obj = (*entrySlotBase)->obj;
        objState = (ObjHitsPriorityState*)((GameObject*)obj)->anim.hitReactState;
        objState->localPosX = ((GameObject*)obj)->anim.localPosX;
        objState->localPosY = ((GameObject*)obj)->anim.localPosY;
        objState->localPosZ = ((GameObject*)obj)->anim.localPosZ;
        if (((GameObject*)obj)->anim.parent != NULL) {
            Obj_TransformLocalPointToWorld(objState->localPosX, objState->localPosY, objState->localPosZ,
                                           &objState->worldPosX, &objState->worldPosY, &objState->worldPosZ,
                                           ((GameObject*)obj)->anim.parent);
        } else {
            objState->worldPosX = ((GameObject*)obj)->anim.localPosX;
            objState->worldPosY = ((GameObject*)obj)->anim.localPosY;
            objState->worldPosZ = ((GameObject*)obj)->anim.localPosZ;
        }
        objState->activeHitboxMode = 0;
        objState->flags = objState->flags & ~OBJHITS_PRIORITY_STATE_HITBOX_BUFFER_CACHED;
        if (((objState->priorityHitCount != 0) ||
             ((objState->flags & OBJHITS_PRIORITY_STATE_PAIR_RESPONSE_APPLIED) != 0)) &&
            ((objState->flags & OBJHITS_PRIORITY_STATE_POSITION_DIRTY) == 0) && ((objState->flags & 0x4000) == 0)) {
            ((GameObject*)obj)->anim.velocityX =
                oneOverTimeDelta * (((GameObject*)obj)->anim.localPosX - ((GameObject*)obj)->anim.previousLocalPosX);
            ((GameObject*)obj)->anim.velocityZ =
                oneOverTimeDelta * (((GameObject*)obj)->anim.localPosZ - ((GameObject*)obj)->anim.previousLocalPosZ);
        }
    }
    ((int*)(int)gObjHitsActiveHitVolumeObjects)[hitVolumeIndex = 0] = 0;
    ((int*)(int)gObjHitsActiveHitVolumeObjects)[++hitVolumeIndex] = 0;
    ((int*)(int)gObjHitsActiveHitVolumeObjects)[++hitVolumeIndex] = 0;
    ((int*)(int)gObjHitsActiveHitVolumeObjects)[++hitVolumeIndex] = 0;
    ((int*)(int)gObjHitsActiveHitVolumeObjects)[++hitVolumeIndex] = 0;
}

char sObjHitsTooManyHitSpheresWarning[] = "HIT VOLUMES: an object has too many hit spheres\n";

f32 gObjHitsResponseDominanceRatio = 0.4f;
char sObjHitReactResetString[7] = "reset\n";

const StaffCollisionColorArgs gObjHitReactEffectColorArgs = {8, 0xB4, 0xF0, 0xFF};

u32 ObjHitReact_Update(int obj, ObjHitReactEntry* reactionEntryTable, u32 reactionEntryCount, u32 reactionState, float* reactionStepScale);
u32 ObjHitReact_Update(int obj, ObjHitReactEntry* reactionEntryTable, u32 reactionEntryCount, u32 reactionState,
                       float* reactionStepScale) {
    ObjAnimDef* animDef;
    ObjAnimComponent* objAnim;
    int moveEnded;
    int hitType;
    ObjHitReactEntry* reactionEntry;
    StaffCollisionInterface** effectResource;
    bool sfxActive;
    PartFxSpawnParams effectParams;
    StaffCollisionColorArgs effectColorArgs;
    int hitSphereIndex;

    objAnim = (ObjAnimComponent*)obj;
    effectColorArgs = gObjHitReactEffectColorArgs;
    if ((reactionState & OBJHITREACT_REACTION_STATE_MASK) != OBJHITREACT_REACTION_STATE_INACTIVE) {
        OSReport(sObjHitReactHitstateFrameString, objAnim->currentMoveProgress);
        moveEnded = ObjAnim_AdvanceCurrentMove((int)obj, (double)*reactionStepScale, (double)timeDelta, NULL);
        if (moveEnded != 0) {
            OSReport(sObjHitReactResetString);
            reactionState = OBJHITREACT_REACTION_STATE_INACTIVE;
        }
    }
    hitType = ObjHits_GetPriorityHitWithPosition((GameObject*)(obj), 0, &hitSphereIndex, 0, &effectParams.posX,
                                                 &effectParams.posY, &effectParams.posZ);
    if (hitType != 0) {
        ObjAnimBank* bank = ObjAnim_GetActiveBank(objAnim);
        effectParams.posX = effectParams.posX + playerMapOffsetX;
        effectParams.posZ = effectParams.posZ + playerMapOffsetZ;
        effectParams.scale = gObjHitsScalarOne;
        effectParams.rotZ = 0;
        effectParams.rotY = 0;
        effectParams.rotX = 0;
        animDef = bank->animDef;
        hitSphereIndex = ObjAnim_GetHitReactEntryIndex(animDef, hitSphereIndex);
        if (hitSphereIndex >= (int)(reactionEntryCount & OBJHITREACT_ENTRY_COUNT_MASK)) {
            OSReport(sObjHitReactSphereOverflowString, hitSphereIndex);
            hitSphereIndex = 0;
        }
        reactionEntry = &reactionEntryTable[hitSphereIndex];
        if (hitType != OBJHITREACT_COLLISION_SKIP_REACTION) {
            if ((reactionEntry->primaryHitSfxId > OBJHITREACT_NO_SFX_ID) &&
                (sfxActive = Sfx_IsPlayingFromObject((GameObject*)obj, (u16)reactionEntry->primaryHitSfxId),
                 !sfxActive)) {
                Sfx_PlayFromObject((GameObject*)(u32)obj, reactionEntry->primaryHitSfxId);
            }
            if ((reactionEntry->secondaryHitSfxId > OBJHITREACT_NO_SFX_ID) &&
                (sfxActive = Sfx_IsPlayingFromObject((GameObject*)obj, (u16)reactionEntry->secondaryHitSfxId),
                 !sfxActive)) {
                Sfx_PlayFromObject((GameObject*)(u32)obj, reactionEntry->secondaryHitSfxId);
            }
            if (reactionEntry->hitEffectMode == OBJHITREACT_HIT_FX_MODE_EFFECT) {
                effectResource = Resource_Acquire(OBJHITREACT_HIT_EFFECT_ID, OBJHITREACT_HIT_EFFECT_RESOURCE_COUNT);
                (*effectResource)
                    ->spawn(OBJHITREACT_HIT_EFFECT_PARENT_NONE, OBJHITREACT_HIT_EFFECT_MODE, &effectParams,
                            OBJHITREACT_HIT_EFFECT_SPAWN_FLAGS, OBJHITREACT_HIT_EFFECT_NO_SOURCE, &effectColorArgs);
                if (effectResource != NULL) {
                    Resource_Release(effectResource);
                }
            } else {
                objDoHitParticleFx((void*)obj, 0.014f, &effectParams, OBJHITREACT_ALT_EFFECT_COUNT, NULL);
            }
        }
        if (((reactionState & OBJHITREACT_REACTION_STATE_MASK) == OBJHITREACT_REACTION_STATE_INACTIVE) &&
            (reactionEntry->reactionMoveId > OBJHITREACT_NO_REACTION_ANIM)) {
            ObjAnim_SetCurrentMove(obj, reactionEntry->reactionMoveId, gObjHitsScalarZero, 0);
            *reactionStepScale = reactionEntry->reactionStepScale;
            reactionState = OBJHITREACT_REACTION_STATE_ACTIVE;
        }
    }
    return reactionState;
}

void ObjHitReact_ResetActiveObjects(int objectCount) {
    ObjHitReactState* hitState;
    ObjAnimComponent* objAnim;
    ObjAnimComponent** objectListCursor;
    int stateActive;
    int resetPending;
    int objectListCount;
    int startIndex;

    objectListCursor = (ObjAnimComponent**)ObjList_GetObjects(&startIndex, &objectListCount);
    gObjHitReactResetObjectCount = 0;
    while (objectCount > 0) {
        objAnim = *objectListCursor;
        hitState = objAnim->hitReactState;
        if (hitState != NULL) {
            stateActive = hitState->flags & OBJHITS_PRIORITY_STATE_ENABLED;
            if (stateActive != 0) {
                resetPending = hitState->shapeFlags & OBJHITREACT_SHAPE_RESET_UPDATE;
                if (resetPending != 0) {
                    if (gObjHitReactResetObjectCount < OBJHITREACT_MAX_RESET_OBJECTS) {
                        gObjHitReactResetObjects[gObjHitReactResetObjectCount++] = objAnim;
                    }
                    hitState->activeHit = 0;
                    hitState->flags = (s16)(hitState->flags & ~OBJHITS_PRIORITY_STATE_PAIR_RESPONSE_APPLIED);
                    hitState->resetFrameCount = OBJHITREACT_RESET_FRAME_COUNT;
                }
            }
        }
        objectListCursor = objectListCursor + 1;
        objectCount = objectCount + -1;
    }
}

int ObjHitbox_AllocRotatedBounds(ObjHitbox* hitbox, u32 arena) {
    ObjHitboxTransformState* transformState;

    transformState = (ObjHitboxTransformState*)roundUpTo4(arena);
    hitbox->transformState = transformState;
    if (hitbox->transformState != NULL) {
        hitbox->transformState->activeMatrixIndex = 0;
        hitbox->transformState->resetFrames = OBJHITBOX_ROTATED_BOUNDS_RESET_FRAMES;
        hitbox->transformState->contactObjectCount = 0;
        ObjHitbox_UpdateRotatedBounds(hitbox, 1);
        ObjHitbox_UpdateRotatedBounds(hitbox, 1);
    }
    return (u32)transformState + sizeof(ObjHitboxTransformState);
}

void ObjHitReact_LoadMoveEntries(ObjAnimComponent* objAnim, ObjAnimBank* bank, int objType, ObjHitReactState* hitState,
                                 int moveId, int async) {
    int moveEntryWordIndex;
    s16* moveEntryTable;
    s16* moveEntry;
    s16 entryByteOffset;

    moveEntryTable = (s16*)objAnim->modelInstance->hitReactMoveTable;
    hitState->activeEntryByteCount = 0;
    if (moveEntryTable != NULL) {
        for (moveEntryWordIndex = 0, moveEntry = moveEntryTable;
             ((ObjHitReactMoveEntry*)moveEntry)->moveId != OBJHITREACT_MOVE_ID_END;
             moveEntry += OBJHITREACT_MOVE_ENTRY_SHORT_COUNT,
            moveEntryWordIndex += OBJHITREACT_MOVE_ENTRY_SHORT_COUNT) {
            if (moveId == ((ObjHitReactMoveEntry*)moveEntry)->moveId) {
                moveEntry = &moveEntryTable[moveEntryWordIndex];
                entryByteOffset = ((ObjHitReactMoveEntry*)moveEntry)->firstEntryByteOffset;
                hitState->activeEntryByteCount = ((ObjHitReactMoveEntry*)moveEntry)->entryByteCount;
                if (hitState->activeEntryByteCount > hitState->entryBufferByteCapacity) {
                    hitState->activeEntryByteCount = hitState->entryBufferByteCapacity;
                }
                if (async == 0) {
                    getTabEntry(hitState->entries, OBJHITREACT_ENTRY_TAB_FILE_ID, entryByteOffset,
                                hitState->activeEntryByteCount);
                    return;
                }
                fileLoadToBufferOffset(OBJHITREACT_ENTRY_TAB_FILE_ID, hitState->entries, entryByteOffset,
                                       hitState->activeEntryByteCount);
                return;
            }
        }
    }
    return;
}

u32 ObjHitReact_InitState(int objType, ObjAnimBank* bank, ObjHitReactState* hitState, u32 entryArena,
                          ObjAnimComponent* objAnim) {
    ObjHitReactEntry* entries;

    if (bank == NULL) {
        return entryArena;
    }
    hitState->entryBufferByteCapacity = OBJHITREACT_ENTRY_ARENA_BYTES;
    entries = (ObjHitReactEntry*)roundUpTo8(entryArena);
    hitState->entries = entries;
    entryArena = (u32)entries + hitState->entryBufferByteCapacity;
    hitState->activeHitboxMode = OBJHITREACT_ACTIVE_HITBOX_MODE;
    if ((hitState->shapeFlags & OBJHITS_SHAPE_RESET_MODE_MASK) != 0) {
        hitState->resetHitboxMode = OBJHITREACT_RESET_HITBOX_MODE;
    }
    ObjHitReact_LoadMoveEntries(objAnim, bank, objType, hitState, 0, 1);
    return entryArena;
}

char sObjHitReactHitstateFrameString[] = "hitstate frame=%f\n";
char sObjHitReactSphereOverflowString[] = "objHitReact.c: sphere overflow! %d\n";

extern char sObjAddObjectTypeReachedMaxTypes[];

#define OBJTYPE_COUNT                 0x54
#define OBJTYPE_INDEX_COUNT           (OBJTYPE_COUNT + 1)
#define OBJTYPE_LIST_MAX              0x100
#define OBJLIB_PRIMARY_ROM_PAGE_COUNT 0x50
#define OBJHITREGION_ROM_ENTRY_TYPE   0x130

typedef struct ObjectTypeIndexTable {
    u8 offsets[OBJTYPE_INDEX_COUNT];
    u8 reserved[3];
} ObjectTypeIndexTable;

STATIC_ASSERT(sizeof(ObjectTypeIndexTable) == 0x58);

u32 gObjectTypeList[OBJTYPE_LIST_MAX];
ObjectTypeIndexTable gObjectTypeIndices;

typedef struct ObjContactCallbackEntry {
    GameObject* objA;
    GameObject* objB;
    ObjContactCallback callback;
} ObjContactCallbackEntry;

typedef struct ObjHitRegionPlacement {
    ObjPlacement base;
    u16 id;
    u16 halfX;
    u16 halfY;
    u16 halfZ;
    u8 yaw;
    u8 pitch;
} ObjHitRegionPlacement;

STATIC_ASSERT(offsetof(ObjHitRegionPlacement, id) == 0x18);
STATIC_ASSERT(offsetof(ObjHitRegionPlacement, yaw) == 0x20);

ObjContactCallbackEntry gObjContactCallbacks[0xC0 / sizeof(ObjContactCallbackEntry)];
extern void* gObjHitsWorkBuffer;
extern u8 gObjectTypeListCount;
extern int gObjContactCallbackCount;
#define OBJMSG_QUEUE_OFFSET        0xdc
#define OBJMSG_SEND_INCLUDE_SENDER 0x1
#define OBJMSG_SEND_MATCH_ANY      0x2
#define OBJMSG_SEND_MATCH_OBJTYPE  0x4

#define OBJCONTACT_CALLBACK_CAPACITY    0x10
#define OBJCONTACT_CALLBACK_LAST_INDEX  (OBJCONTACT_CALLBACK_CAPACITY - 1)
#define OBJTRIGGER_FLAGS_OFFSET         0xaf
#define OBJTRIGGER_CURRENT_ENABLE_FLAG  0x01
#define OBJTRIGGER_CURRENT_BLOCK_FLAG   0x08
#define OBJTRIGGER_ID_ENABLE_FLAG       0x04
#define OBJTRIGGER_ID_BLOCK_FLAG        0x10
#define OBJTRIGGER_BUTTON_DISABLE_INDEX 0
#define OBJTRIGGER_BUTTON_DISABLE_FLAG  0x100
#define OBJTRIGGER_PLAYER_STATE_NONE    -1
#define OBJTRIGGER_PLAYER_STATE_CLEAR   0x40

#define OBJLINK_PARENT_OFFSET      0xc4
#define OBJLINK_CHILD_LIST_OFFSET  0xc8
#define OBJLINK_CHILD_COUNT_OFFSET 0xeb
#define OBJLINK_FLAGS_OFFSET       0xb0
#define OBJLINK_FLAGS_MODE_MASK    0x0007
#define OBJLINK_FLAGS_DEAD         0x0040

#define OBJ_MODEL_INSTANCE_OFFSET     0x50
#define OBJ_ACTIVE_MODEL_INDEX_OFFSET 0xad
#define OBJ_POSITION_X_OFFSET         0x0c
#define OBJ_POSITION_Y_OFFSET         0x10
#define OBJ_POSITION_Z_OFFSET         0x14

#define OBJ_MODEL_JOINT_COUNT_OFFSET 0xf3
#define OBJLIB_BLINK_LEFT_JOINT_TAG  5
#define OBJLIB_BLINK_RIGHT_JOINT_TAG 4

/* hit-object romDefNo that triggers the staff-impact sfx (retail OBJECTS.bin). */
#define OBJLIB_HITOBJ_SEQID_STAFF  0x69 /* "staff" (DLL 0xE2) */
#define OBJPATH_POINTS_OFFSET      0x2c
#define OBJPATH_POINT_COUNT_OFFSET 0x58
#define OBJPATH_ROOT_JOINT_INDEX   -1
typedef struct ObjMsgEntry {
    u32 message;
    u32 sender;
    u32 param;
} ObjMsgEntry;

typedef struct ObjMsgQueue {
    u32 count;
    u32 capacity;
    ObjMsgEntry entries[1];
} ObjMsgQueue;

STATIC_ASSERT(sizeof(ObjMsgEntry) == 0xC);
STATIC_ASSERT(offsetof(ObjMsgQueue, entries) == 0x8);

typedef struct ObjMsgQueueCursor {
    u32 count;
    u32 capacity;
    ObjMsgEntry entry;
    ObjMsgEntry nextEntry;
} ObjMsgQueueCursor;

STATIC_ASSERT(offsetof(ObjMsgQueueCursor, entry) == 0x8);
STATIC_ASSERT(offsetof(ObjMsgQueueCursor, nextEntry) == 0x14);
STATIC_ASSERT(sizeof(ObjMsgQueueCursor) == 0x20);

typedef struct ObjPathPoint {
    f32 x;
    f32 y;
    f32 z;
    s16 rotX;
    s16 rotY;
    s16 rotZ;
    s8 modelIndex[6];
} ObjPathPoint;

void ObjHitbox_SetStateIndex(GameObject* object, ObjHitReactState* hitStatePtr, int stateIndex) {
    ObjHitsPriorityState* priorityState;
    int modelOrSlotIndex;
    ObjHitsPriorityWorkSlot* workSlot;

    modelOrSlotIndex = object->anim.modelInstance->modelCount;
    if (stateIndex >= modelOrSlotIndex) {
        stateIndex = modelOrSlotIndex - 1;
    } else if (stateIndex < 0) {
        stateIndex = 0;
    }
    priorityState = (ObjHitsPriorityState*)hitStatePtr;
    if (priorityState->stateIndex == stateIndex) {
        return;
    }
    for (modelOrSlotIndex = 0; (s16)modelOrSlotIndex < OBJHITS_PRIORITY_WORK_SLOT_COUNT;
         modelOrSlotIndex++) {
        workSlot = &gObjHitsPriorityHitStates[modelOrSlotIndex];
        if ((workSlot->active != 0) && (workSlot->object == object)) {
            workSlot->active = 0;
        }
    }
    priorityState->stateIndex = stateIndex;
    return;
}

void ObjHits_SetTargetMask(GameObject* obj, u8 targetMask) {
    ObjHitsPriorityState* hitState;

    hitState = (ObjHitsPriorityState*)obj->anim.hitReactState;
    if (hitState == 0) {
        return;
    }
    hitState->targetMask = targetMask;
    return;
}

void ObjHitbox_SetSphereRadius(ObjAnimComponent* obj, s16 radius) {
    ObjHitsPriorityState* hitState = (ObjHitsPriorityState*)obj->hitReactState;

    if (hitState != 0) {
        if ((hitState->shapeFlags & OBJHITS_SHAPE_SPHERE) != 0) {
            hitState->primaryRadius = radius;
            hitState->primaryRadiusSquared = (float)(s32)hitState->primaryRadius * (float)(s32)hitState->primaryRadius;
            hitState->primaryRadiusY = obj->hitboxScale * obj->rootMotionScale;
            if ((float)(s32)hitState->primaryRadius > hitState->primaryRadiusY) {
                hitState->primaryRadiusY = (float)(s32)hitState->primaryRadius;
            }
            hitState->primaryRadiusXZ = obj->hitboxScale * obj->rootMotionScale;
            if ((float)(s32)hitState->primaryRadius > hitState->primaryRadiusXZ) {
                hitState->primaryRadiusXZ = (float)(s32)hitState->primaryRadius;
            }
        }

        if ((hitState->secondaryShapeFlags & OBJHITS_SHAPE_SPHERE) != 0) {
            hitState->secondaryRadius = radius;
            hitState->secondaryRadiusY = obj->hitboxScale * obj->rootMotionScale;
            if ((float)(s32)hitState->primaryRadius > hitState->secondaryRadiusY) {
                hitState->secondaryRadiusY = (float)(s32)hitState->secondaryRadius;
            }
            hitState->secondaryRadiusXZ = obj->hitboxScale * obj->rootMotionScale;
            if ((float)(s32)hitState->primaryRadius > hitState->secondaryRadiusXZ) {
                hitState->secondaryRadiusXZ = (float)(s32)hitState->secondaryRadius;
            }
        }

        hitState->sweepRadiusX = hitState->primaryRadiusXZ;
        if (hitState->secondaryRadiusXZ > hitState->sweepRadiusX) {
            hitState->sweepRadiusX = hitState->secondaryRadiusXZ;
        }
    }
    return;
}

void ObjHitbox_SetCapsuleBounds(ObjAnimComponent* obj, s16 radius, s16 verticalMin, s16 verticalMax) {
    ObjHitsPriorityState* hitState;
    float absMin;
    float absMax;
    s32 absVal;
    s16 r16 = radius;
    s16 vmin = verticalMin;
    s16 vmax = verticalMax;

    hitState = (ObjHitsPriorityState*)obj->hitReactState;
    if (hitState != 0) {
        if ((hitState->shapeFlags & OBJHITS_SHAPE_CAPSULE) != 0) {
            hitState->primaryCapsuleOffsetA = vmin;
            hitState->primaryCapsuleOffsetB = vmax;
            hitState->primaryRadius = r16;
            hitState->primaryRadiusSquared = (float)(s32)hitState->primaryRadius * (float)(s32)hitState->primaryRadius;
            hitState->capsuleScale = OBJHITBOX_DEFAULT_CAPSULE_SCALE;
            hitState->primaryRadiusY = obj->hitboxScale * obj->rootMotionScale;
            absVal = vmin;
            if (absVal < 0) {
                absVal = -absVal;
            }
            absMin = (float)absVal;
            absVal = vmax;
            if (absVal < 0) {
                absVal = -absVal;
            }
            absMax = (float)absVal;
            if (absMin > absMax) {
                absMax = absMin;
            }
            if (absMax > hitState->primaryRadiusY) {
                hitState->primaryRadiusY = absMax;
            }
            hitState->primaryRadiusXZ = obj->hitboxScale * obj->rootMotionScale;
            if ((float)(s32)hitState->primaryRadius > hitState->primaryRadiusXZ) {
                hitState->primaryRadiusXZ = (float)(s32)hitState->primaryRadius;
            }
        }
        if ((hitState->secondaryShapeFlags & OBJHITS_SHAPE_CAPSULE) != 0) {
            hitState->secondaryCapsuleOffsetA = vmin;
            hitState->secondaryCapsuleOffsetB = vmax;
            hitState->secondaryRadius = r16;
            hitState->secondaryRadiusY = obj->hitboxScale * obj->rootMotionScale;
            absVal = vmin;
            if (absVal < 0) {
                absVal = -absVal;
            }
            absMin = (float)absVal;
            absVal = vmax;
            if (absVal < 0) {
                absVal = -absVal;
            }
            absMax = (float)absVal;
            if (absMin > absMax) {
                absMax = absMin;
            }
            if (absMax > hitState->secondaryRadiusY) {
                hitState->secondaryRadiusY = absMax;
            }
            hitState->secondaryRadiusXZ = obj->hitboxScale * obj->rootMotionScale;
            if ((float)(s32)hitState->primaryRadius > hitState->secondaryRadiusXZ) {
                hitState->secondaryRadiusXZ = (float)(s32)hitState->secondaryRadius;
            }
        }
        hitState->sweepRadiusX = hitState->primaryRadiusXZ;
        if (hitState->secondaryRadiusXZ > hitState->sweepRadiusX) {
            hitState->sweepRadiusX = hitState->secondaryRadiusXZ;
        }
    }
    return;
}

void ObjHits_ClearHitVolumes(ObjAnimComponent* obj) {
    ObjHitsPriorityState* hitState = (ObjHitsPriorityState*)obj->hitReactState;
    hitState->hitVolumePriority = 0;
    hitState->hitVolumeId = 0;
    hitState->objectHitMask = 0;
    hitState->skeletonHitMask = 0;
}

void ObjHits_SetHitVolumeMasks(ObjAnimComponent* obj, int hitVolume, int hitType, int sourceMask) {
    ObjHitsPriorityState* hitState = (ObjHitsPriorityState*)obj->hitReactState;
    hitState->hitVolumePriority = hitVolume;
    hitState->hitVolumeId = hitType;

    if (sourceMask == 0) {
        return;
    }

    hitState->objectHitMask = sourceMask << 4;
    hitState->skeletonHitMask = sourceMask << 4;
}

void ObjHits_SetHitVolumeSlot(ObjAnimComponent* obj, int hitVolume, int hitType, int sourceSlot) {
    ObjHitsPriorityState* hitState = (ObjHitsPriorityState*)obj->hitReactState;

    if (hitState == 0) {
        return;
    }

    hitState->hitVolumePriority = hitVolume;
    hitState->hitVolumeId = hitType;

    if (sourceSlot == -1) {
        return;
    }

    hitState->objectHitMask = 1 << (sourceSlot + 4);
    hitState->skeletonHitMask = 1 << (sourceSlot + 4);
}

void ObjHits_ClearSourceMask(ObjAnimComponent* obj, int sourceMask) {
    ObjHitsPriorityState* hitState = (ObjHitsPriorityState*)obj->hitReactState;
    hitState->sourceMask &= ~sourceMask;
}

void ObjHits_SetSourceMask(ObjAnimComponent* obj, u8 sourceMask) {
    ObjHitsPriorityState* hitState = (ObjHitsPriorityState*)obj->hitReactState;
    hitState->sourceMask |= sourceMask;
}

void ObjHits_ClearFlags(ObjAnimComponent* obj, int flags) {
    ObjHitsPriorityState* hitState = (ObjHitsPriorityState*)obj->hitReactState;
    hitState->flags &= ~flags;
}

void ObjHits_SetFlags(ObjAnimComponent* obj, int flags) {
    ObjHitsPriorityState* hitState = (ObjHitsPriorityState*)obj->hitReactState;
    hitState->flags |= flags;
}

void ObjHits_MarkObjectPositionDirty(ObjAnimComponent* obj) {
    ObjHitsPriorityState* hitState = (ObjHitsPriorityState*)obj->hitReactState;
    hitState->flags |= OBJHITS_PRIORITY_STATE_POSITION_DIRTY;
}

void ObjHits_SyncObjectPositionIfDirty(GameObject* obj) {
    ObjHitsPriorityState* hitState;
    s16 flags;

    hitState = (ObjHitsPriorityState*)obj->anim.hitReactState;
    if (hitState == 0) {
        return;
    }
    flags = hitState->flags;
    if ((flags & OBJHITS_PRIORITY_STATE_POSITION_DIRTY) == 0) {
        return;
    }
    hitState->flags = (s16)(flags & ~OBJHITS_PRIORITY_STATE_POSITION_DIRTY);
    hitState->localPosX = obj->anim.localPosX;
    hitState->localPosY = obj->anim.localPosY;
    hitState->localPosZ = obj->anim.localPosZ;
    hitState->worldPosX = obj->anim.worldPosX;
    hitState->worldPosY = obj->anim.worldPosY;
    hitState->worldPosZ = obj->anim.worldPosZ;
    return;
}

void ObjHits_DisableObject(GameObject* obj) {
    ObjHitsPriorityState* hitState;

    hitState = (ObjHitsPriorityState*)obj->anim.hitReactState;
    if (hitState == 0) {
        return;
    }
    hitState->flags = (s16)(hitState->flags & ~OBJHITS_PRIORITY_STATE_ENABLED);
    return;
}

void ObjHits_EnableObject(GameObject* obj) {
    ObjHitsPriorityState* hitState;
    s16 flags;

    hitState = (ObjHitsPriorityState*)obj->anim.hitReactState;
    if (hitState == 0) {
        return;
    }
    flags = hitState->flags;
    if ((flags & OBJHITS_PRIORITY_STATE_ENABLED) != 0) {
        return;
    }
    hitState->flags = (s16)(flags | OBJHITS_PRIORITY_STATE_ENABLED);
    hitState->localPosX = obj->anim.localPosX;
    hitState->localPosY = obj->anim.localPosY;
    hitState->localPosZ = obj->anim.localPosZ;
    hitState->worldPosX = obj->anim.worldPosX;
    hitState->worldPosY = obj->anim.worldPosY;
    hitState->worldPosZ = obj->anim.worldPosZ;
    return;
}

int ObjHits_IsObjectEnabled(ObjAnimComponent* obj) {
    return ((ObjHitsPriorityState*)obj->hitReactState)->flags & OBJHITS_PRIORITY_STATE_ENABLED;
}

void ObjHits_SyncObjectPosition(GameObject* obj) {
    ObjHitsPriorityState* hitState;

    hitState = (ObjHitsPriorityState*)obj->anim.hitReactState;
    if (hitState == 0) {
        return;
    }
    hitState->localPosX = obj->anim.localPosX;
    hitState->localPosY = obj->anim.localPosY;
    hitState->localPosZ = obj->anim.localPosZ;
    hitState->worldPosX = obj->anim.worldPosX;
    hitState->worldPosY = obj->anim.worldPosY;
    hitState->worldPosZ = obj->anim.worldPosZ;
    return;
}

int ObjHits_AllocObjectState(GameObject* obj, u32 arena) {
    u32 stateArena;
    ObjHitsPriorityState* hitState;

    stateArena = roundUpTo4(arena);
    obj->anim.hitReactState = (ObjHitReactState*)stateArena;
    hitState = (ObjHitsPriorityState*)obj->anim.hitReactState;
    ObjHits_RefreshObjectState(obj);
    hitState->activeHitboxMode = OBJHITS_ACTIVE_HITBOX_MODE;
    if ((hitState->shapeFlags & OBJHITS_SHAPE_RESET_MODE_MASK) != 0) {
        hitState->resetHitboxMode = OBJHITS_RESET_HITBOX_MODE;
    }
    return stateArena + 0xb8;
}

void ObjHits_RefreshObjectState(GameObject* object) {
    ObjAnimComponent* obj;
    ObjHitsPriorityState* hitState;
    ObjAnimBank* activeBank;
    short capsuleOffsetA;
    short capsuleOffsetB;

    obj = &object->anim;
    hitState = (ObjHitsPriorityState*)obj->hitReactState;
    if (hitState != 0) {
        hitState->flags = obj->modelInstance->hitboxFlags;
        hitState->shapeFlags = obj->modelInstance->primaryHitboxShapeFlags;
        if ((hitState->shapeFlags & OBJHITS_SHAPE_SKELETON) != 0) {
            activeBank = ObjAnim_GetActiveBank(obj);
            if (((activeBank->animDef->flags & OBJANIM_DEF_FLAG_SKELETON_HITBOXES) == 0) ||
                (*(void**)(((int*)activeBank) + 5) == 0)) {
                hitState->shapeFlags &= ~OBJHITS_SHAPE_SKELETON;
            }
        }
        hitState->lateralResponseWeight = obj->modelInstance->lateralResponseWeight;
        hitState->axialResponseWeight = obj->modelInstance->axialResponseWeight;
        hitState->primaryRadius = obj->modelInstance->primaryHitboxRadius;
        hitState->primaryCapsuleOffsetA = obj->modelInstance->primaryCapsuleOffsetA;
        hitState->primaryCapsuleOffsetB = obj->modelInstance->primaryCapsuleOffsetB;
        *(s8*)&hitState->stateIndex = (s8)(int)obj->modelInstance->hitboxStateIndex;
        hitState->capsuleScale = OBJHITBOX_DEFAULT_CAPSULE_SCALE;
        hitState->primaryRadiusSquared = (float)(s32)hitState->primaryRadius * (float)(s32)hitState->primaryRadius;
        hitState->secondaryShapeFlags = obj->modelInstance->secondaryHitboxShapeFlags;
        hitState->secondaryRadius = obj->modelInstance->secondaryHitboxRadius;
        hitState->secondaryCapsuleOffsetA = obj->modelInstance->secondaryCapsuleOffsetA;
        hitState->secondaryCapsuleOffsetB = obj->modelInstance->secondaryCapsuleOffsetB;
        hitState->primaryRadiusY = obj->hitboxScale * obj->rootMotionScale;
        if ((hitState->shapeFlags & OBJHITS_SHAPE_CAPSULE) != 0) {
            capsuleOffsetA = (hitState->primaryCapsuleOffsetA < 0) ? -hitState->primaryCapsuleOffsetA
                                                                   : hitState->primaryCapsuleOffsetA;
            capsuleOffsetB = (hitState->primaryCapsuleOffsetB < 0) ? -hitState->primaryCapsuleOffsetB
                                                                   : hitState->primaryCapsuleOffsetB;
            if (capsuleOffsetA > capsuleOffsetB) {
                capsuleOffsetB = capsuleOffsetA;
            }
            if ((float)(s32)capsuleOffsetB > hitState->primaryRadiusY) {
                hitState->primaryRadiusY = (float)(s32)capsuleOffsetB;
            }
        } else if ((hitState->shapeFlags & OBJHITS_SHAPE_SPHERE) != 0) {
            if ((float)(s32)hitState->primaryRadius > hitState->primaryRadiusY) {
                hitState->primaryRadiusY = (float)(s32)hitState->primaryRadius;
            }
        }
        hitState->primaryRadiusXZ = obj->hitboxScale * obj->rootMotionScale;
        if (((hitState->shapeFlags & OBJHITS_SHAPE_CAPSULE) != 0) ||
            ((hitState->shapeFlags & OBJHITS_SHAPE_SPHERE) != 0)) {
            if ((float)(s32)hitState->primaryRadius > hitState->primaryRadiusXZ) {
                hitState->primaryRadiusXZ = (float)(s32)hitState->primaryRadius;
            }
        }
        hitState->secondaryRadiusY = obj->hitboxScale * obj->rootMotionScale;
        if ((hitState->secondaryShapeFlags & OBJHITS_SHAPE_CAPSULE) != 0) {
            capsuleOffsetA = (hitState->secondaryCapsuleOffsetA < 0) ? -hitState->secondaryCapsuleOffsetA
                                                                     : hitState->secondaryCapsuleOffsetA;
            capsuleOffsetB = (hitState->secondaryCapsuleOffsetB < 0) ? -hitState->secondaryCapsuleOffsetB
                                                                     : hitState->secondaryCapsuleOffsetB;
            if (capsuleOffsetA > capsuleOffsetB) {
                capsuleOffsetB = capsuleOffsetA;
            }
            if ((float)(s32)capsuleOffsetB > hitState->secondaryRadiusY) {
                hitState->secondaryRadiusY = (float)(s32)capsuleOffsetB;
            }
        } else if ((hitState->secondaryShapeFlags & OBJHITS_SHAPE_SPHERE) != 0) {
            if ((float)(s32)hitState->secondaryRadius > hitState->secondaryRadiusY) {
                hitState->secondaryRadiusY = (float)(s32)hitState->secondaryRadius;
            }
        }
        hitState->secondaryRadiusXZ = obj->hitboxScale * obj->rootMotionScale;
        if (((hitState->secondaryShapeFlags & OBJHITS_SHAPE_CAPSULE) != 0) ||
            ((hitState->secondaryShapeFlags & OBJHITS_SHAPE_SPHERE) != 0)) {
            if ((float)(s32)hitState->secondaryRadius > hitState->secondaryRadiusXZ) {
                hitState->secondaryRadiusXZ = (float)(s32)hitState->secondaryRadius;
            }
        }
        hitState->sweepRadiusX = hitState->primaryRadiusXZ;
        if (hitState->secondaryRadiusXZ > hitState->sweepRadiusX) {
            hitState->sweepRadiusX = hitState->secondaryRadiusXZ;
        }
        hitState->sourceMask = obj->modelInstance->sourceHitMask;
        hitState->targetMask = obj->modelInstance->targetHitMask;
    }
    return;
}

int ObjHits_RecordObjectHit(GameObject* obj, GameObject* hitObj, s8 priority, int hitVolume, s8 sphereIndex) {
    ObjAnimComponent* sourceObj;
    ObjAnimComponent* targetObj;
    ObjHitsPriorityState* hitState;
    int hitSlot;
    u8 hitVolumeId;

    if (priority == '\0') {
        return 0;
    }
    sourceObj = (ObjAnimComponent*)obj;
    targetObj = (ObjAnimComponent*)hitObj;
    hitState = (ObjHitsPriorityState*)sourceObj->hitReactState;
    if ((hitState->flags & OBJHITS_PRIORITY_STATE_ENABLED) == 0) {
        return 0;
    }
    if ((targetObj != NULL) && (targetObj->hitReactState != NULL)) {
        ((ObjHitsPriorityState*)targetObj->hitReactState)->lastHitObject = (u32)obj;
    }
    hitSlot = 0;
    hitVolumeId = hitVolume;
    while (hitSlot < hitState->priorityHitCount) {
        if ((void*)hitState->hitObjects[hitSlot] == (void*)hitObj) {
            if (hitState->priorities[hitSlot] > priority) {
                hitState->sphereIndices[hitSlot] = sphereIndex;
                hitState->priorities[hitSlot] = priority;
                hitState->hitVolumes[hitSlot] = hitVolumeId;
                hitState->hitPosX[hitSlot] = sourceObj->localPosX;
                hitState->hitPosY[hitSlot] = sourceObj->localPosY;
                hitState->hitPosZ[hitSlot] = sourceObj->localPosZ;
            }
            hitSlot = hitState->priorityHitCount + 1;
        }
        hitSlot = hitSlot + 1;
    }
    if ((hitSlot == hitState->priorityHitCount) && (hitState->priorityHitCount < OBJHITS_PRIORITY_HIT_COUNT)) {
        hitState->sphereIndices[hitState->priorityHitCount] = sphereIndex;
        hitState->priorities[hitState->priorityHitCount] = priority;
        hitState->hitVolumes[hitState->priorityHitCount] = hitVolumeId;
        hitState->hitObjects[hitState->priorityHitCount] = (int)hitObj;
        hitState->hitPosX[hitState->priorityHitCount] = sourceObj->localPosX;
        hitState->hitPosY[hitState->priorityHitCount] = sourceObj->localPosY;
        hitState->hitPosZ[hitState->priorityHitCount] = sourceObj->localPosZ;
        hitState->priorityHitCount++;
    }
    return 1;
}

int ObjHits_RecordPositionHit(GameObject* obj, GameObject* hitObj, s8 priority, int hitVolume, s8 sphereIndex,
                              f32 hitPosX, f32 hitPosY, f32 hitPosZ) {
    ObjAnimComponent* sourceObj;
    ObjAnimComponent* targetObj;
    ObjHitsPriorityState* hitState;
    int hitSlot;
    u8 hitVolumeId;

    if ((int)priority == '\0') {
        return 0;
    }
    sourceObj = (ObjAnimComponent*)obj;
    targetObj = (ObjAnimComponent*)hitObj;
    hitState = (ObjHitsPriorityState*)sourceObj->hitReactState;
    if ((hitState->flags & OBJHITS_PRIORITY_STATE_ENABLED) == 0) {
        return 0;
    }
    if ((targetObj != NULL) && (targetObj->hitReactState != NULL)) {
        ((ObjHitsPriorityState*)targetObj->hitReactState)->lastHitObject = (u32)obj;
    }
    hitSlot = 0;
    hitVolumeId = hitVolume;
    while (hitSlot < hitState->priorityHitCount) {
        if ((void*)hitState->hitObjects[hitSlot] == (void*)hitObj) {
            if (hitState->priorities[hitSlot] > (s8)priority) {
                hitState->sphereIndices[hitSlot] = sphereIndex;
                hitState->priorities[hitSlot] = priority;
                hitState->hitVolumes[hitSlot] = hitVolumeId;
                hitState->hitPosX[hitSlot] = hitPosX;
                hitState->hitPosY[hitSlot] = hitPosY;
                hitState->hitPosZ[hitSlot] = hitPosZ;
            }
            hitSlot = hitState->priorityHitCount + 1;
        }
        hitSlot = hitSlot + 1;
    }
    if ((hitSlot == hitState->priorityHitCount) && (hitState->priorityHitCount < OBJHITS_PRIORITY_HIT_COUNT)) {
        hitState->sphereIndices[hitState->priorityHitCount] = sphereIndex;
        hitState->priorities[hitState->priorityHitCount] = priority;
        hitState->hitVolumes[hitState->priorityHitCount] = hitVolumeId;
        hitState->hitObjects[hitState->priorityHitCount] = (int)hitObj;
        hitState->hitPosX[hitState->priorityHitCount] = hitPosX;
        hitState->hitPosY[hitState->priorityHitCount] = hitPosY;
        hitState->hitPosZ[hitState->priorityHitCount] = hitPosZ;
        hitState->priorityHitCount++;
    }
    return 1;
}

void ObjHits_AddContactObject(GameObject* obj, GameObject* contactObj) {
    int contactObjectIndex;
    int contactObjectCount;
    int contactOffset;
    int i;
    int storeState;
    int transformState;

    transformState = *(int*)((int)obj + OBJHITBOX_TRANSFORM_STATE_OFFSET);
    if ((u32)transformState == 0) {
        return;
    }
    contactObjectCount = (int)*(char*)(transformState + OBJHITBOX_STATE_CONTACT_OBJECT_COUNT_OFFSET);
    if (contactObjectCount >= OBJHITBOX_CONTACT_OBJECT_COUNT) {
        return;
    }
    contactOffset = 0;
    for (i = 0; i < contactObjectCount; i++) {
        u32 entryObj = *(u32*)(transformState + contactOffset + OBJHITBOX_STATE_CONTACT_OBJECTS_OFFSET);
        if (entryObj == (u32)contactObj) {
            return;
        }
        contactOffset = contactOffset + 4;
    }
    storeState = *(int*)((u8*)obj + OBJHITBOX_TRANSFORM_STATE_OFFSET);
    contactObjectIndex = (*(char*)(transformState + OBJHITBOX_STATE_CONTACT_OBJECT_COUNT_OFFSET))++;
    *(int*)(storeState + OBJHITBOX_STATE_CONTACT_OBJECTS_OFFSET + contactObjectIndex * 4) = (int)contactObj;
}

int ObjHits_GetPriorityHitWithPosition(GameObject* obj, GameObject** outHitObject, int* outSphereIndex,
                                       u32* outHitVolume, float* outHitPosX, float* outHitPosY, float* outHitPosZ) {
    u8 hitPriority;
    int hitCount;
    ObjHitsPriorityState* hitState;
    int hitSlot;
    u8 bestPriority;
    s8 bestHitSlot;

    hitState = *(ObjHitsPriorityState**)&(obj)->anim.hitReactState;
    if (hitState == 0) {
        return 0;
    }
    hitCount = hitState->priorityHitCount;
    if (hitCount != 0) {
        bestPriority = OBJHITS_PRIORITY_INVALID;
        bestHitSlot = -1;
        for (hitSlot = 0; hitSlot < hitCount; hitSlot++) {
            hitPriority = hitState->priorities[hitSlot];
            if ((s8)hitPriority < (s8)bestPriority) {
                bestPriority = hitPriority;
                bestHitSlot = hitSlot;
            }
        }
        if (bestHitSlot != -1) {
            if (outHitObject != NULL) {
                *outHitObject = (GameObject*)hitState->hitObjects[bestHitSlot];
            }

            if (outSphereIndex != 0x0) {
                *outSphereIndex = hitState->sphereIndices[bestHitSlot];
            }

            if (outHitVolume != 0x0) {
                *outHitVolume = hitState->hitVolumes[bestHitSlot];
            }

            if (outHitPosX != (float*)0x0) {
                *outHitPosX = hitState->hitPosX[bestHitSlot];
                *outHitPosY = hitState->hitPosY[bestHitSlot];
                *outHitPosZ = hitState->hitPosZ[bestHitSlot];
            }

            return (s8)bestPriority;
        }
    }
    return 0;
}

int ObjHits_GetPriorityHit(GameObject* obj, int* outHitObject, int* outSphereIndex, u32* outHitVolume) {
    u8 hitPriority;
    int hitCount;
    ObjHitsPriorityState* hitState;
    int hitSlot;
    u8 bestPriority;
    s8 bestHitSlot;

    hitState = *(ObjHitsPriorityState**)&(obj)->anim.hitReactState;
    if (hitState == 0) {
        return 0;
    }
    hitCount = hitState->priorityHitCount;
    if (hitCount != 0) {
        bestPriority = OBJHITS_PRIORITY_INVALID;
        bestHitSlot = -1;
        for (hitSlot = 0; hitSlot < hitCount; hitSlot++) {
            hitPriority = hitState->priorities[hitSlot];
            if ((s8)hitPriority < (s8)bestPriority) {
                bestPriority = hitPriority;
                bestHitSlot = hitSlot;
            }
        }
        if (bestHitSlot != -1) {
            if (outHitObject != 0x0) {
                *outHitObject = hitState->hitObjects[bestHitSlot];
            }
            if (outSphereIndex != 0x0) {
                *outSphereIndex = hitState->sphereIndices[bestHitSlot];
            }
            if (outHitVolume != 0x0) {
                *outHitVolume = hitState->hitVolumes[bestHitSlot];
            }
            return (int)(s8)bestPriority;
        }
    }
    return 0;
}

void ObjHitReact_UpdateResetObjects(void) {
    ObjAnimComponent* obj;
    int objectIndex;
    int objectOffset;

    objectIndex = 0;
    objectOffset = 0;
    for (; objectIndex < gObjHitReactResetObjectCount; objectIndex = objectIndex + 1) {
        obj = gObjHitReactResetObjects[objectIndex];
        if (((obj->modelInstance->flags & OBJMODEL_FLAG_SKIP_RESET_UPDATE) == 0) &&
            (obj->activeHitboxMode != OBJHITREACT_DISABLED_HITBOX_MODE)) {
            Obj_UpdateObject((GameObject*)obj);
        }
        objectOffset = objectOffset + 4;
    }
    objectOffset = 0;
    for (; objectOffset < gObjHitReactResetObjectCount; objectOffset = objectOffset + 1) {
        ObjHitbox_UpdateRotatedBounds((ObjHitbox*)gObjHitReactResetObjects[objectOffset], 1);
    }
    return;
}

void ObjHits_ResetWorkBuffers(void) {
    int slotIndex;

    for (slotIndex = 0; slotIndex < OBJHITS_PRIORITY_WORK_SLOT_COUNT; slotIndex++) {
        gObjHitsPriorityHitStates[slotIndex].active = 0;
    }
    gObjHitReactResetObjectCount = 0;
}

ObjAnimComponent** ObjHitReact_GetResetObjects(int* outObjectCount) {
    *outObjectCount = gObjHitReactResetObjectCount;
    return gObjHitReactResetObjects;
}

void ObjHits_InitWorkBuffers(void) {
    int hitVolumeIndex;

    gObjHitReactResetObjects =
        (ObjAnimComponent**)mmAlloc(OBJHITREACT_MAX_RESET_OBJECTS * sizeof(ObjAnimComponent*), 0xe, 0);
    gObjHitsPriorityHitStates = mmAlloc(OBJHITS_PRIORITY_WORK_SLOT_COUNT * sizeof(ObjHitsPriorityWorkSlot), 0xe, 0);
    gObjHitsWorkBuffer = mmAlloc(0x1900, 0xe, 0);
    gObjHitsPrimaryHitboxBufferScratch0 = mmAlloc(0x400, 0xe, 0);
    gObjHitsPrimaryHitboxBufferScratch1 = mmAlloc(0x400, 0xe, 0);
    gObjHitsSecondaryHitboxBufferScratch0 = mmAlloc(0x400, 0xe, 0);
    gObjHitsSecondaryHitboxBufferScratch1 = mmAlloc(0x400, 0xe, 0);
    gObjHitsPriorityHitTickDelta = 2.0f;
    ((int*)(int)gObjHitsActiveHitVolumeObjects)[hitVolumeIndex = 0] = 0;
    ((int*)(int)gObjHitsActiveHitVolumeObjects)[++hitVolumeIndex] = 0;
    ((int*)(int)gObjHitsActiveHitVolumeObjects)[++hitVolumeIndex] = 0;
    ((int*)(int)gObjHitsActiveHitVolumeObjects)[++hitVolumeIndex] = 0;
    ((int*)(int)gObjHitsActiveHitVolumeObjects)[++hitVolumeIndex] = 0;
    return;
}

int objIsObjectType(u32 obj, int group) {
    u32* entry;
    u32 index;
    u32 limit;
    u32 limitXorIndex;
    int halfDiff;

    if ((group < 0) || (group >= OBJTYPE_COUNT)) {
        return 0;
    }
    index = gObjectTypeIndices.offsets[group];
    limit = gObjectTypeIndices.offsets[group + 1];
    for (entry = gObjectTypeList + index; ((int)index < (int)limit && (obj != *entry));
         entry = entry + 1, index = index + 1) {
    }
    limitXorIndex = limit ^ index;
    halfDiff = (int)limitXorIndex >> 1;
    limitXorIndex = limitXorIndex & limit;
    return (u32)(halfDiff - limitXorIndex) >> 0x1f;
}

int objGetNearestType(int group, float* point, float* maxDistance) {
    u32* entry;
    u32 nearest;
    int index;
    int limit;
    float distanceSq;
    float bestDistanceSq;

    nearest = 0;
    bestDistanceSq = *maxDistance * *maxDistance;
    if ((group < 0) || (group >= OBJTYPE_COUNT)) {
        return 0;
    }
    index = gObjectTypeIndices.offsets[group];
    limit = gObjectTypeIndices.offsets[group + 1];
    entry = gObjectTypeList + index;
    while (index < limit) {
        if (*entry != 0) {
            distanceSq = PSVECSquareDistance((Vec*)point, &((GameObject*)*entry)->anim.worldPos);
            if (distanceSq < bestDistanceSq) {
                bestDistanceSq = distanceSq;
                nearest = *entry;
            }
            entry++;
            index++;
        }
    }
    if (nearest != 0) {
        *maxDistance = sqrtf(bestDistanceSq);
    }
    return nearest;
}

GameObject* objGetNearestTypeToExcludingSelf(int group, GameObject* obj, float* maxDistance) {
    u32* entry;
    GameObject* nearest;
    int index;
    int limit;
    float distanceSq;
    float bestDistanceSq;

    nearest = 0;
    if ((group < 0) || (group >= OBJTYPE_COUNT)) {
        return 0;
    }
    if (maxDistance != (float*)0x0) {
        bestDistanceSq = *maxDistance * *maxDistance;
    } else {
        bestDistanceSq = 3.4028235e38f;
    }
    index = gObjectTypeIndices.offsets[group];
    limit = gObjectTypeIndices.offsets[group + 1];
    entry = gObjectTypeList + index;
    while (index < limit) {
        if ((GameObject*)*entry != obj) {
            distanceSq = vec3f_distanceSquared(&obj->anim.worldPosX, &((GameObject*)*entry)->anim.worldPosX);
            if (distanceSq < bestDistanceSq) {
                bestDistanceSq = distanceSq;
                nearest = (GameObject*)*entry;
            }
        }
        entry++;
        index++;
    }
    if ((nearest != 0) && (maxDistance != (float*)0x0)) {
        *maxDistance = sqrtf(bestDistanceSq);
    }
    return nearest;
}

GameObject* objGetNearestTypeTo(int group, GameObject* obj, float* maxDistance) {
    u32* entry;
    GameObject* nearest;
    GameObject* o;
    int index;
    int limit;
    float distanceSq;
    float bestDistanceSq;

    nearest = 0;
    if ((group < 0) || (group >= OBJTYPE_COUNT)) {
        return 0;
    }
    if (maxDistance != (float*)0x0) {
        bestDistanceSq = *maxDistance * *maxDistance;
    } else {
        bestDistanceSq = 3.4028235e38f;
    }
    o = obj;
    index = gObjectTypeIndices.offsets[group];
    limit = gObjectTypeIndices.offsets[group + 1];
    entry = gObjectTypeList + index;
    while (index < limit) {
        if ((GameObject*)*entry != o) {
            distanceSq = vec3f_distanceSquared(&o->anim.worldPosX, &((GameObject*)*entry)->anim.worldPosX);
            if (distanceSq < bestDistanceSq) {
                bestDistanceSq = distanceSq;
                nearest = (GameObject*)*entry;
            }
        }
        entry++;
        index++;
    }
    if ((nearest != 0) && (maxDistance != (float*)0x0)) {
        *maxDistance = sqrtf(bestDistanceSq);
    }
    return nearest;
}

GameObject** objGetAllOfType(int group, int* countOut) {
    if (group < 0 || group >= OBJTYPE_COUNT) {
        *countOut = 0;
        return 0x0;
    }
    *countOut = gObjectTypeIndices.offsets[group + 1] - gObjectTypeIndices.offsets[group];
    return (GameObject**)(gObjectTypeList + gObjectTypeIndices.offsets[group]);
}

void objFreeObjectType(int obj, int group) {
    u8* offset;
    u8 count;
    int index;
    int limit;
    u32* entries;

    if ((group < 0) || (group >= OBJTYPE_COUNT)) {
        return;
    }
    offset = gObjectTypeIndices.offsets;
    index = offset[group];
    offset += group;
    limit = offset[1];
    entries = gObjectTypeList + index;
    while ((index < limit) && (*entries != obj)) {
        entries++;
        index++;
    }
    if (index >= limit) {
        return;
    }
    count = (gObjectTypeListCount -= 1);
    entries = gObjectTypeList + index;
    while (index < count) {
        *entries = entries[1];
        entries++;
        index++;
    }
    group++;
    offset = gObjectTypeIndices.offsets + group;
    while (group <= OBJTYPE_COUNT) {
        (*offset)--;
        offset++;
        group++;
    }
}

int objGetObjectType(u32 obj) {
    int group;
    int objectIndex;

    for (objectIndex = 0; objectIndex < (int)(u32)gObjectTypeListCount; objectIndex++) {
        u32 entryObj = gObjectTypeList[objectIndex];
        if (entryObj == obj) {
            group = 0;
            while (((int)(u32)gObjectTypeIndices.offsets[group] <= objectIndex) && (group < OBJTYPE_INDEX_COUNT)) {
                group++;
            }
            return group;
        }
    }
    return 0;
}

void objAddObjectType(int obj, int group) {
    u8* offset;
    int count;
    int index;
    int limit;
    int insertIndex;
    u32* entries;

    if ((group < 0) || (group >= OBJTYPE_COUNT)) {
        return;
    }
    if ((int)(u32)gObjectTypeListCount >= OBJTYPE_LIST_MAX) {
        OSReport(sObjAddObjectTypeReachedMaxTypes);
        return;
    }
    offset = gObjectTypeIndices.offsets;
    insertIndex = offset[group];
    offset += group;
    limit = offset[1];
    entries = gObjectTypeList + insertIndex;
    for (index = insertIndex; index < limit; index++) {
        if (*entries == obj) {
            return;
        }
        entries++;
    }
    insertIndex = (limit - insertIndex == 0) ? insertIndex : (limit - 1);
    gObjectTypeListCount++;
    count = (int)(u32)gObjectTypeListCount;
    count--;
    entries = gObjectTypeList + count;
    for (index = count; insertIndex < index; index--) {
        *entries = entries[-1];
        entries--;
    }
    gObjectTypeList[insertIndex] = obj;
    group++;
    offset = gObjectTypeIndices.offsets + group;
    while (group <= OBJTYPE_COUNT) {
        (*offset)++;
        offset++;
        group++;
    }
}

void objTypeInit(void) {
    memset(gObjectTypeIndices.offsets, 0, sizeof(gObjectTypeIndices.offsets));
    gObjectTypeListCount = 0;
    return;
}

int ObjMsg_Peek(void* obj, u32* outMessage, u32* outSender, u32* outParam) {
    ObjMsgQueue* queue;

    if (obj == 0x0) {
        return 0;
    }
    queue = *(ObjMsgQueue**)((u8*)obj + OBJMSG_QUEUE_OFFSET);
    if ((queue != (ObjMsgQueue*)0x0) && (queue->count != 0)) {
        if (outMessage != 0x0) {
            *outMessage = queue->entries[0].message;
        }
        if (outSender != 0x0) {
            *outSender = queue->entries[0].sender;
        }
        if (outParam != 0x0) {
            *outParam = queue->entries[0].param;
        }
        return 1;
    }
    return 0;
}

int ObjMsg_Pop(void* obj, u32* outMessage, u32* outSender, u32* outParam) {
    ObjMsgQueue* queue;
    ObjMsgQueueCursor* slot;
    u32 i;

    if (obj == 0x0) {
        return 0;
    }
    queue = *(ObjMsgQueue**)((u8*)obj + OBJMSG_QUEUE_OFFSET);
    if ((queue != (ObjMsgQueue*)0x0) && (queue->count != 0)) {
        queue->count = queue->count - 1;
        if (outMessage != 0x0) {
            *outMessage = queue->entries[0].message;
        }
        if (outSender != 0x0) {
            *outSender = queue->entries[0].sender;
        }
        if (outParam != 0x0) {
            *outParam = queue->entries[0].param;
        }
        for (i = 0; i < queue->count; i = i + 1) {
            slot = (ObjMsgQueueCursor*)((u8*)queue + ((i + i + i) << 2));
            slot->entry.message = slot->nextEntry.message;
            slot->entry.sender = slot->nextEntry.sender;
            slot->entry.param = slot->nextEntry.param;
        }
        return 1;
    }
    return 0;
}

void ObjMsg_SendToNearbyObjects(int targetId, float radius, u32 flags, void* sender, u32 message, u32 param) {
    int* objects;
    u32 count;
    int maskedFlags;
    ObjMsgQueue* queue;
    ObjMsgQueueCursor* slot;
    int objectIndex;
    int objectCount;
    GameObject* obj;
    int includeSender;
    int matchAny;
    GameObject* s;

    objects = ObjList_GetObjects(&objectIndex, &objectCount);
    maskedFlags = flags & 0xffff;
    includeSender = maskedFlags & OBJMSG_SEND_INCLUDE_SENDER;
    matchAny = maskedFlags & OBJMSG_SEND_MATCH_ANY;
    s = (GameObject*)sender;
    for (; objectIndex < objectCount; objectIndex = objectIndex + 1) {
        obj = (GameObject*)objects[objectIndex];
        if (((obj != sender) || (includeSender == 0)) && ((obj->anim.romDefNo == (s16)targetId || (matchAny != 0))) &&
            ((Vec_distance(&s->anim.worldPosX, &obj->anim.worldPosX) < radius && (obj != 0x0)) &&
             (queue = *(ObjMsgQueue**)((u8*)obj + OBJMSG_QUEUE_OFFSET), queue != (ObjMsgQueue*)0x0))) {
            count = queue->count;
            if (count < queue->capacity) {
                slot = (ObjMsgQueueCursor*)((u8*)queue + ((count + count + count) << 2));
                slot->entry.message = message;
                slot->entry.sender = (u32)sender;
                slot->entry.param = param;
                queue->count = queue->count + 1;
            } else {
                debugPrintf(sObjMsgOverflowInObjectWarning, message, (int)obj->anim.classId, (int)obj->anim.romDefNo,
                            (int)s->anim.romDefNo);
            }
        }
    }
    return;
}

void ObjMsg_SendToObjects(int targetId, u32 flags, void* sender, u32 message, u32 param) {
    int* objects;
    u32 count;
    int maskedFlags;
    ObjMsgQueue* queue;
    ObjMsgQueueCursor* slot;
    int objectIndex;
    int objectCount;
    GameObject* obj;

    objects = ObjList_GetObjects(&objectIndex, &objectCount);
    maskedFlags = flags & 0xffff;
    if ((maskedFlags & OBJMSG_SEND_MATCH_OBJTYPE) != 0) {
        for (; objectIndex < objectCount; objectIndex = objectIndex + 1) {
            obj = (GameObject*)objects[objectIndex];
            if (((obj != sender) || ((maskedFlags & OBJMSG_SEND_INCLUDE_SENDER) == 0)) &&
                (((maskedFlags & OBJMSG_SEND_MATCH_ANY) != 0 || (targetId == obj->anim.romDefNo))) &&
                ((obj != 0x0 &&
                  (queue = *(ObjMsgQueue**)((u8*)obj + OBJMSG_QUEUE_OFFSET), queue != (ObjMsgQueue*)0x0)))) {
                count = queue->count;
                if (count < queue->capacity) {
                    slot = (ObjMsgQueueCursor*)((u8*)queue + ((count + count + count) << 2));
                    slot->entry.message = message;
                    slot->entry.sender = (u32)sender;
                    slot->entry.param = param;
                    queue->count = queue->count + 1;
                } else {
                    debugPrintf(sObjMsgOverflowInObjectWarning, message, (int)obj->anim.classId,
                                (int)obj->anim.romDefNo, (int)((GameObject*)sender)->anim.romDefNo);
                }
            }
        }
    } else {
        for (; objectIndex < objectCount; objectIndex = objectIndex + 1) {
            obj = (GameObject*)objects[objectIndex];
            if (((obj != sender) || ((maskedFlags & OBJMSG_SEND_INCLUDE_SENDER) == 0)) &&
                (((maskedFlags & OBJMSG_SEND_MATCH_ANY) != 0 || (targetId == obj->anim.classId))) &&
                ((obj != 0x0 &&
                  (queue = *(ObjMsgQueue**)((u8*)obj + OBJMSG_QUEUE_OFFSET), queue != (ObjMsgQueue*)0x0)))) {
                count = queue->count;
                if (count < queue->capacity) {
                    slot = (ObjMsgQueueCursor*)((u8*)queue + ((count + count + count) << 2));
                    slot->entry.message = message;
                    slot->entry.sender = (u32)sender;
                    slot->entry.param = param;
                    queue->count = queue->count + 1;
                } else {
                    debugPrintf(sObjMsgOverflowInObjectWarning, message, (int)obj->anim.classId,
                                (int)obj->anim.romDefNo, (int)((GameObject*)sender)->anim.romDefNo);
                }
            }
        }
    }
    return;
}

u32 ObjMsg_SendToObject(GameObject* obj, u32 message, void* sender, u32 param) {
    u32 count;
    void* senderObj;
    ObjMsgQueue* queue;
    ObjMsgQueueCursor* slot;

    senderObj = sender;
    if (obj == NULL) {
        return 0;
    }
    queue = *(ObjMsgQueue**)((u8*)obj + OBJMSG_QUEUE_OFFSET);
    if (queue != (ObjMsgQueue*)0x0) {
        count = queue->count;
        if (count < queue->capacity) {
            slot = (ObjMsgQueueCursor*)((u8*)queue + ((count + count + count) << 2));
            slot->entry.message = message;
            slot->entry.sender = (u32)senderObj;
            slot->entry.param = param;
            queue->count = queue->count + 1;
            return queue->count;
        }
        debugPrintf(sObjMsgOverflowInObjectWarning, message, (int)obj->anim.classId, (int)obj->anim.romDefNo,
                    (int)((GameObject*)senderObj)->anim.romDefNo);
    }
    return 0;
}

void ObjMsg_AllocQueue(void* obj, int capacity) {
    int queueBytes;
    ObjMsgQueue* queue;

    if (((capacity != 0) && (obj != 0x0)) && (*(ObjMsgQueue**)((u8*)obj + OBJMSG_QUEUE_OFFSET) == (ObjMsgQueue*)0x0)) {
        queueBytes = (capacity * 3 + 2) * 4;
        queue = (ObjMsgQueue*)mmAlloc(queueBytes, 0xe, 0);
        queue->count = 0;
        queue->capacity = capacity;
        *(ObjMsgQueue**)((u8*)obj + OBJMSG_QUEUE_OFFSET) = queue;
    }
    return;
}

int Obj_IsObjectAlive(GameObject* objArg) {
    u32 alive;
    GameObject* obj = objArg;

    alive = 0;
    if ((obj != NULL) && ((obj->objectFlags & OBJLINK_FLAGS_DEAD) == 0)) {
        alive = 1;
    }
    return alive;
}

bool ObjTrigger_UpdateIdBlockFlag(int obj) {
    int disguised;
    u8 flags;

    disguised = (int)Obj_GetPlayerObject();
    disguised = playerIsDisguised((GameObject*)disguised);
    if (disguised != 0) {
        flags = *(u8*)(obj + OBJTRIGGER_FLAGS_OFFSET) | OBJTRIGGER_ID_BLOCK_FLAG;
        *(u8*)(obj + OBJTRIGGER_FLAGS_OFFSET) = flags;
        return false;
    }
    flags = *(u8*)(obj + OBJTRIGGER_FLAGS_OFFSET) & ~OBJTRIGGER_ID_BLOCK_FLAG;
    *(u8*)(obj + OBJTRIGGER_FLAGS_OFFSET) = flags;
    return true;
}

int ObjHits_PollPriorityHitWithCooldown(GameObject* obj, float* cooldown, GameObject** outHitObject, float* outHitPos) {
    int collisionType;

    collisionType = 0;
    *cooldown = *cooldown - timeDelta;
    if (*cooldown <= 0.0f) {
        if (outHitPos != (float*)0x0) {
            collisionType = ObjHits_GetPriorityHitWithPosition(obj, outHitObject, 0x0, 0x0, outHitPos, outHitPos + 1,
                                                               outHitPos + 2);
            if (collisionType != 0) {
                ObjHits_ConvertHitPositionToWorld(obj, outHitPos);
            }
        } else {
            collisionType = ObjHits_GetPriorityHit(obj, (int*)outHitObject, 0x0, 0x0);
        }
        if (collisionType != 0) {
            *cooldown = 30.0f;
        }
    }
    return collisionType;
}

int ObjHits_PollPriorityHitEffectWithCooldown(GameObject* obj, u32 hitFxMode, u32 colorR, u32 colorG, u32 colorB,
                                              u16 sfxId, float* cooldown) {
    int collisionType;
    StaffCollisionInterface** effectResource;
    PartFxSpawnParams effectParams;
    StaffCollisionColorArgs effectArgs;
    GameObject* hitObject;

    *cooldown = *cooldown - timeDelta;
    collisionType = ObjHits_GetPriorityHitWithPosition(obj, &hitObject, 0x0, 0x0, &effectParams.posX,
                                                       &effectParams.posY, &effectParams.posZ);
    if ((*cooldown <= 0.0f) && (collisionType != 0)) {
        *cooldown = 45.0f;
        if ((collisionType != 0x1a) && (collisionType != 5)) {
            effectParams.posX = effectParams.posX + playerMapOffsetX;
            effectParams.posZ = effectParams.posZ + playerMapOffsetZ;
            effectParams.scale = 1.0f;
            effectParams.rotZ = 0;
            effectParams.rotY = 0;
            effectParams.rotX = 0;
            effectResource = Resource_Acquire(OBJHITREACT_HIT_EFFECT_ID, OBJHITREACT_HIT_EFFECT_RESOURCE_COUNT);
            effectArgs.count = hitFxMode & 0xff;
            effectArgs.red = colorR & 0xff;
            effectArgs.green = colorG & 0xff;
            effectArgs.blue = colorB & 0xff;
            (*effectResource)
                ->spawn(OBJHITREACT_HIT_EFFECT_PARENT_NONE, OBJHITREACT_HIT_EFFECT_MODE, &effectParams,
                        OBJHITREACT_HIT_EFFECT_SPAWN_FLAGS, OBJHITREACT_HIT_EFFECT_NO_SOURCE, &effectArgs);
            if (((sfxId != 0) && (hitObject != 0)) && (hitObject->anim.romDefNo == OBJLIB_HITOBJ_SEQID_STAFF)) {
                Sfx_PlayFromObject(obj, sfxId);
            }
        }
    }
    return collisionType;
}

void ObjLink_DetachChild(GameObject* obj, GameObject* child) {
    int dst;
    int slot;
    int i;

    i = 0;
    for (slot = (int)obj; i < (int)obj->childCount; i++) {
        if (*(GameObject**)(slot + OBJLINK_CHILD_LIST_OFFSET) == child) {
            break;
        }
        slot += 4;
    }
    dst = (int)obj + i * 4;
    while (i < (int)obj->childCount - 1) {
        *(int*)(dst + OBJLINK_CHILD_LIST_OFFSET) = *(int*)(dst + OBJLINK_CHILD_LIST_OFFSET + sizeof(int));
        dst += 4;
        i++;
    }
    obj->childCount--;
    obj->childObjs[obj->childCount] = NULL;
    child->ownerObj = (void*)0;
    return;
}

void ObjLink_AttachChild(GameObject* parent, GameObject* child, int linkMode) {
    int childIndex;
    GameObject* parentObj;
    GameObject* childObj;

    parentObj = parent;
    childObj = child;
    childIndex = (int)parentObj->childCount;
    parentObj->childCount += 1;
    parentObj->childObjs[childIndex] = child;
    childObj->ownerObj = parent;
    childObj->objectFlags = (u16)(childObj->objectFlags & ~OBJLINK_FLAGS_MODE_MASK);
    childObj->objectFlags = (u16)(childObj->objectFlags | linkMode);
    childObj->colorFadeFlags = 0;
    return;
}

void ObjContact_DispatchCallbacks(GameObject* objA, GameObject* objB) {
    int objARefCount;
    int objBRefCount;
    int count;
    ObjContactCallbackEntry* entry;

    objARefCount = objA->contactRefCount;
    objBRefCount = objB->contactRefCount;
    entry = gObjContactCallbacks;
    count = gObjContactCallbackCount;
    while ((objARefCount != 0) && (objBRefCount != 0) && (count-- != 0)) {
        if ((entry->objA == objA) && (entry->objB == objB)) {
            objARefCount = objARefCount - 1;
            entry->callback(objA, objB);
        }
        if ((entry->objA == objB) && (entry->objB == objA)) {
            objBRefCount = objBRefCount - 1;
            entry->callback(objB, objA);
        }
        entry++;
    }
    return;
}

void ObjContact_RemoveObjectCallbacks(GameObject* obj) {
    int count;
    ObjContactCallbackEntry* entry;

    entry = gObjContactCallbacks;
    count = gObjContactCallbackCount;
    while (count-- > 0) {
        if ((entry->objA == obj) || (entry->objB == obj)) {
            gObjContactCallbackCount--;
            count--;
            entry->objA->contactRefCount--;
            entry->objB->contactRefCount--;
            if ((gObjContactCallbackCount != OBJCONTACT_CALLBACK_LAST_INDEX) && (gObjContactCallbackCount != 0)) {
                *entry = gObjContactCallbacks[gObjContactCallbackCount];
            }
        }
        entry++;
    }
    return;
}

int ObjContact_AddCallback(GameObject* obj, GameObject* otherObj, ObjContactCallback callback) {
    int count;
    ObjContactCallbackEntry* entry;
    int i;

    if ((obj == NULL) || (otherObj == NULL)) {
        return 0;
    }
    entry = gObjContactCallbacks;
    count = gObjContactCallbackCount;
    for (i = 0; i != count; i++) {
        if ((entry->objA == obj) && (entry->objB == otherObj)) {
            return 0;
        }
        entry++;
    }
    if (count >= OBJCONTACT_CALLBACK_CAPACITY) {
        return 0;
    }
    entry = &gObjContactCallbacks[count];
    entry->objA = obj;
    entry->objB = otherObj;
    entry->callback = callback;
    obj->contactRefCount += 1;
    otherObj->contactRefCount += 1;
    gObjContactCallbackCount = gObjContactCallbackCount + 1;
    return 1;
}

int ObjTrigger_IsSetById(int obj, int eventId) {
    int playerState;
    int triggerFlags;
    int flagEnabled;
    int flagBlocked;

    triggerFlags = *(u8*)(obj + OBJTRIGGER_FLAGS_OFFSET);
    flagEnabled = triggerFlags & OBJTRIGGER_ID_ENABLE_FLAG;
    if (flagEnabled != 0) {
        flagBlocked = triggerFlags & OBJTRIGGER_ID_BLOCK_FLAG;
        if ((flagBlocked == 0) &&
            (playerState = (*gGameUIInterface)->isItemBeingUsed((int)(short)eventId), playerState != 0)) {
            playerState = objGetAnimState80A((GameObject*)(Obj_GetPlayerObject()));
            if (playerState == OBJTRIGGER_PLAYER_STATE_NONE) {
                buttonDisable(OBJTRIGGER_BUTTON_DISABLE_INDEX, OBJTRIGGER_BUTTON_DISABLE_FLAG);
                return 1;
            }
        }
    }
    return 0;
}

int ObjTrigger_IsSet(int objPtr) {
    GameObject* obj = (GameObject*)objPtr;
    u32 flags;
    int playerState;
    int triggerFlags;
    int flagEnabled;
    int flagBlocked;

    if (obj->anim.modelInstance->hitVolumes == NULL) {
        return 0;
    }
    flags = buttonGetDisabled(0);
    if ((flags & OBJTRIGGER_BUTTON_DISABLE_FLAG) == 0) {
        triggerFlags = obj->anim.resetHitboxFlags;
        flagEnabled = triggerFlags & OBJTRIGGER_CURRENT_ENABLE_FLAG;
        if (flagEnabled != 0) {
            flagBlocked = triggerFlags & OBJTRIGGER_CURRENT_BLOCK_FLAG;
            if ((flagBlocked == 0) && (playerState = (*gGameUIInterface)->isAnyItemBeingUsed(), playerState == 0)) {
                playerState = objGetAnimState80A((GameObject*)(Obj_GetPlayerObject()));
                if ((playerState == OBJTRIGGER_PLAYER_STATE_NONE) || (playerState == OBJTRIGGER_PLAYER_STATE_CLEAR)) {
                    buttonDisable(OBJTRIGGER_BUTTON_DISABLE_INDEX, OBJTRIGGER_BUTTON_DISABLE_FLAG);
                    return 1;
                }
            }
        }
    }
    return 0;
}

GameObject* ObjList_FindNearestObjectByDefNo(GameObject* obj, int defNo, float* maxDistanceSq) {
    int startIndex;
    int objectCount;
    float invalidDistance;
    float distanceSq;
    u32 otherObj;
    int objectIndex;
    int* objects;
    int* walker;
    GameObject* foundObj;

    objects = ObjList_GetObjects(&startIndex, &objectCount);
    foundObj = 0;
    *maxDistanceSq = *maxDistanceSq * *maxDistanceSq;
    walker = objects + startIndex;

    if (defNo != -1) {
        objectIndex = startIndex;
        walker = objects + startIndex;

        while (objectIndex < objectCount) {
            otherObj = *walker;
            if (((defNo == ((GameObject*)otherObj)->anim.romDefNo) && ((int)obj != otherObj)) &&
                (distanceSq = vec3f_distanceSquared(&(obj)->anim.worldPosX, &((GameObject*)otherObj)->anim.worldPosX),
                 distanceSq < *maxDistanceSq)) {
                *maxDistanceSq = distanceSq;
                foundObj = (GameObject*)*walker;
            }
            walker++;
            objectIndex++;
        }
    } else {
        objectIndex = startIndex;
        walker = objects + startIndex;
        invalidDistance = 0.0f;

        while (objectIndex < objectCount) {
            distanceSq = vec3f_distanceSquared(&(obj)->anim.worldPosX, &((GameObject*)*walker)->anim.worldPosX);
            if ((distanceSq != invalidDistance) && (distanceSq < *maxDistanceSq)) {
                *maxDistanceSq = distanceSq;
                foundObj = (GameObject*)*walker;
            }
            walker++;
            objectIndex++;
        }
    }

    return foundObj;
}

int ObjList_ContainsObject(int obj) {
    u32* entry;
    int i;
    int count;

    entry = ObjList_GetObjects(&i, &count);
    i = 0;
    while (i < count) {
        if (*entry == obj) {
            return 1;
        }
        entry = entry + 1;
        i = i + 1;
    }
    return 0;
}

void ObjPath_GetPointWorldPositionArray(GameObject* obj, int pointIndex, int count, float* positions) {
    float* position;
    int i;

    i = 0;
    position = positions;
    while (i < count) {
        ObjPath_GetPointWorldPosition(obj, pointIndex + i, position, position + 1, position + 2, 0);
        position = position + 3;
        i++;
    }
}

void ObjPath_GetPointLocalPosition(GameObject* obj, int pointIndex, float* xOut, float* yOut, float* zOut) {
    *xOut = ((ObjPathPoint*)(*(int*)(*(int*)&obj->anim.modelInstance + OBJPATH_POINTS_OFFSET) +
                             pointIndex * sizeof(ObjPathPoint)))
                ->x;
    *yOut = *(f32*)(*(int*)(*(int*)&obj->anim.modelInstance + OBJPATH_POINTS_OFFSET) + 4 +
                    pointIndex * sizeof(ObjPathPoint));
    *zOut = *(f32*)(*(int*)(*(int*)&obj->anim.modelInstance + OBJPATH_POINTS_OFFSET) + 8 +
                    pointIndex * sizeof(ObjPathPoint));
    return;
}

void ObjPath_GetPointLocalMtx(GameObject* obj, int pointIndex, float* mtxOut) {
    ObjPathPoint* pathPoint;
    ObjPathTransform transform;

    pathPoint = (ObjPathPoint*)(*(int*)(*(int*)&obj->anim.modelInstance + OBJPATH_POINTS_OFFSET));
    transform.x = pathPoint[pointIndex].x;
    pathPoint += pointIndex;
    transform.y = pathPoint->y;
    transform.z = pathPoint->z;
    transform.rotX = pathPoint->rotX;
    transform.rotY = pathPoint->rotY;
    transform.rotZ = pathPoint->rotZ;
    transform.scale = 1.0f;
    setMatrixFromObjectTransposed(&transform, mtxOut);
    return;
}

u32 ObjPath_GetPointModelMtx(GameObject* obj, int pointIndex) {
    int* model;
    ObjPathPoint* pathPoint;
    int jointIndex;

    model = (int*)Obj_GetActiveModel(obj);
    pathPoint = (ObjPathPoint*)(*(int*)(*(int*)&obj->anim.modelInstance + OBJPATH_POINTS_OFFSET));
    pathPoint += pointIndex;
    jointIndex = pathPoint->modelIndex[(int)*(char*)((int)obj + OBJ_ACTIVE_MODEL_INDEX_OFFSET)];
    if ((jointIndex >= 0) && (jointIndex < (int)(u32) * (u8*)(*model + OBJ_MODEL_JOINT_COUNT_OFFSET))) {
        return (u32)ObjModel_GetJointMatrix((u8*)model, jointIndex);
    } else {
        return (u32)ObjModel_GetJointMatrix((u8*)model, 0);
    }
}

void ObjPath_GetPointWorldPosition(GameObject* obj, int pointIndex, float* outX, float* outY, float* outZ,
                                   int useInputPosition) {
    int pointOffset;
    ObjPathPoint* pathPoint;
    int* model;
    float* jointMtx;
    int jointIndex;
    ObjPathTransform transform;
    float rootMtx[16];
    float transposedMtx[12];
    float concatMtx[12];
    float rotMtx[16];

    if ((pointIndex < 0) ||
        (pointIndex >= (int)(u32) * (u8*)(*(int*)&obj->anim.modelInstance + OBJPATH_POINT_COUNT_OFFSET))) {
        *outX = obj->anim.localPosX;
        *outY = obj->anim.localPosY;
        *outZ = obj->anim.localPosZ;
    } else {
        model = (int*)Obj_GetActiveModel(obj);
        pathPoint = (ObjPathPoint*)(*(int*)(*(int*)&obj->anim.modelInstance + OBJPATH_POINTS_OFFSET));
        pointOffset = pointIndex * sizeof(ObjPathPoint);
        pathPoint = (ObjPathPoint*)((int)pathPoint + pointOffset);
        jointIndex = pathPoint->modelIndex[(int)*(char*)((int)obj + OBJ_ACTIVE_MODEL_INDEX_OFFSET)];
        if ((jointIndex < OBJPATH_ROOT_JOINT_INDEX) ||
            (jointIndex >= (int)(u32) * (u8*)(*model + OBJ_MODEL_JOINT_COUNT_OFFSET))) {
            *outX = obj->anim.localPosX;
            *outY = obj->anim.localPosY;
            *outZ = obj->anim.localPosZ;
        } else {
            if (jointIndex == OBJPATH_ROOT_JOINT_INDEX) {
                Obj_BuildWorldTransformMatrix((GameObject*)obj, rootMtx, 0);
                jointMtx = rootMtx;
            } else {
                jointMtx = (f32*)ObjModel_GetJointMatrix((u8*)model, jointIndex);
            }
            if (useInputPosition != 0) {
                transform.x = *outX;
                transform.y = *outY;
                transform.z = *outZ;
                transform.rotX = 0;
                transform.rotY = 0;
                transform.rotZ = 0;
            } else {
                transform.x = *(f32*)(*(int*)(*(int*)&obj->anim.modelInstance + OBJPATH_POINTS_OFFSET) + pointOffset);
                pathPoint =
                    (ObjPathPoint*)(*(int*)(*(int*)&obj->anim.modelInstance + OBJPATH_POINTS_OFFSET) + pointOffset);
                transform.y = pathPoint->y;
                transform.z = pathPoint->z;
                transform.rotX = pathPoint->rotX;
                transform.rotY = pathPoint->rotY;
                transform.rotZ = pathPoint->rotZ;
            }
            mtxRotateByVec3s(rotMtx, &transform);
            mtx44Transpose(rotMtx, transposedMtx);
            PSMTXConcat((MtxPtr)jointMtx, (MtxPtr)transposedMtx, (MtxPtr)concatMtx);
            *outX = concatMtx[3] + playerMapOffsetX;
            *outY = concatMtx[7];
            *outZ = concatMtx[11] + playerMapOffsetZ;
        }
    }
}

int Obj_GetYawDeltaToObject(GameObject* obj, GameObject* target, float* distOut) {
    int yawDelta;
    float dx;
    float dz;

    dx = obj->anim.localPosX - target->anim.localPosX;
    dz = obj->anim.localPosZ - target->anim.localPosZ;
    yawDelta = (s16)getAngle(dx, dz);
    if (distOut != (float*)0x0) {
        *distOut = sqrtf(dx * dx + dz * dz);
    }
    yawDelta = (int)(short)yawDelta - (u32)(u16) * (s16*)obj;
    if (0x8000 < yawDelta) {
        yawDelta = yawDelta + -0xffff;
    }
    if (yawDelta < -0x8000) {
        yawDelta = yawDelta + 0xffff;
    }
    return (int)(short)yawDelta;
}

u32 ObjHitRegion_FindContainingId(f32 x, f32 y, f32 z) {
    MapRomListPage** lists;
    MapRomListPage* list;
    ObjHitRegionPlacement* entry;
    int listIndex;
    int entryOffset;
    int hitId;

    hitId = -1;
    lists = RomList_GetLoadedPages();
    for (listIndex = 0; listIndex < OBJLIB_PRIMARY_ROM_PAGE_COUNT; listIndex++) {
        list = lists[listIndex];
        if (list != 0) {
            entry = (ObjHitRegionPlacement*)list->objects;
            entryOffset = 0;
            while (entryOffset < (int)(u32)list->objectDataSize) {
                if (entry->base.objectId == OBJHITREGION_ROM_ENTRY_TYPE) {
                    f32 yawSin = mathSinf(3.1415927f * (f32) - (s32)((u32)entry->yaw << 8) / 32768.0f);
                    f32 yawCos = mathCosf(3.1415927f * (f32) - (s32)((u32)entry->yaw << 8) / 32768.0f);
                    f32 pitchSin = mathSinf(3.1415927f * (f32) - (s32)((u32)entry->pitch << 8) / 32768.0f);
                    f32 pitchCos = mathCosf(3.1415927f * (f32) - (s32)((u32)entry->pitch << 8) / 32768.0f);
                    f32 deltaZ;
                    f32 deltaY;
                    f32 deltaX;
                    f32 localX;
                    f32 yawZ;
                    f32 localY;
                    f32 localZ;
                    deltaX = x - entry->base.posX;
                    deltaY = y - entry->base.posY;
                    deltaZ = z - entry->base.posZ;
                    localX = deltaX * yawCos - deltaZ * yawSin;
                    yawZ = deltaX * yawSin + deltaZ * yawCos;
                    localY = deltaY * pitchCos - yawZ * pitchSin;
                    localZ = deltaY * pitchSin + yawZ * pitchCos;

                    if (localX < 0.0f) {
                        localX = -localX;
                    }
                    if (localY < 0.0f) {
                        localY = -localY;
                    }
                    if (localZ < 0.0f) {
                        localZ = -localZ;
                    }
                    if ((localX <= (f32)(u32)entry->halfX) && (localY <= (f32)(u32)entry->halfY) &&
                        (localZ <= (f32)(u32)entry->halfZ)) {
                        hitId = entry->id;
                    }
                }
                entryOffset += entry->base.size * 4;
                entry = (ObjHitRegionPlacement*)((u8*)entry + entry->base.size * 4);
            }
        }
    }
    return hitId & 0xffff;
}

/* Eye-blink state machine (PlayerBlinkState.mode). amount = eyelid closure 0..255. */
typedef enum ObjLibBlinkMode {
    OBJLIB_BLINK_MODE_OPEN = 0,       /* eyes open; randomly start a blink or a wink */
    OBJLIB_BLINK_MODE_CLOSING = 1,    /* eyelids ramping shut (amount -> 255) */
    OBJLIB_BLINK_MODE_CLOSED = 2,     /* fully shut; randomly start opening */
    OBJLIB_BLINK_MODE_OPENING = 3,    /* eyelids ramping open (amount -> 0) */
    OBJLIB_BLINK_MODE_WINK_RIGHT = 4, /* hold shut, right eye scaled apart */
    OBJLIB_BLINK_MODE_WINK_LEFT = 5,  /* hold shut, left eye scaled apart */
} ObjLibBlinkMode;

typedef struct PlayerBlinkState {
    u8 pad[0x2b];
    u8 mode;   /* 0x2b */
    u8 timer;  /* 0x2c */
    u8 amount; /* 0x2d */
} PlayerBlinkState;

static inline int playerEyeAnim_FindJoint(ObjAnimComponent* objAnim, int tag) {
    int jointCount;
    u8* jointData;
    int poseOffset;
    int jointDataOffset;
    ObjModelInstance* model;
    int joint;

    joint = 0;
    model = objAnim->modelInstance;
    if (model != 0) {
        jointDataOffset = 0;
        poseOffset = 0;
        for (jointCount = model->jointCount; jointCount > 0; jointCount--) {
            jointData = (u8*)model->jointData;
            if (((int)*(u8*)((int)jointData + objAnim->bankIndex + jointDataOffset + 1) != 0xff) &&
                ((int)jointData[jointDataOffset] == tag)) {
                joint = (int)objAnim->jointPoseData + poseOffset;
            }
            jointDataOffset += model->modelCount + 1;
            poseOffset += 0x12;
        }
    }
    return joint;
}

void playerUpdateBlinkAnimation(int obj, int blinkState, u16 flags) {

    PlayerBlinkState* bs = (PlayerBlinkState*)blinkState;
    f32 leftScale;
    s16 rotation;
    ObjAnimComponent* objAnim;
    f32 phase;
    u8 step;
    f32 rightScale;
    f32 wave;

    objAnim = (ObjAnimComponent*)obj;
    step = 3.0f * timeDelta;
    rightScale = (leftScale = 1.0f);
    switch (bs->mode) {
    case OBJLIB_BLINK_MODE_OPEN:
        bs->timer = (u8)((f32)bs->timer + timeDelta);
        bs->amount = 0;
        if (((u16)flags & 1) != 0) {
            if (randomGetRange(0, 100) == 1) {
                switch (bs->mode) {
                case OBJLIB_BLINK_MODE_OPEN:
                    bs->mode = OBJLIB_BLINK_MODE_CLOSING;
                    bs->timer = 0;
                    bs->amount = 0;
                    break;
                case OBJLIB_BLINK_MODE_OPENING:
                    bs->mode = OBJLIB_BLINK_MODE_CLOSING;
                    break;
                }
            } else if (randomGetRange(0, 75) == 1) {
                if (randomGetRange(0, 1) == 0) {
                    bs->mode = OBJLIB_BLINK_MODE_WINK_RIGHT;
                } else {
                    bs->mode = OBJLIB_BLINK_MODE_WINK_LEFT;
                }
            }
        }
        break;
    case OBJLIB_BLINK_MODE_CLOSING:
        bs->timer = (u8)((f32)bs->timer + timeDelta);
        if ((s16)bs->amount + (s16)step > 255) {
            step = (u8)(255 - bs->amount);
            bs->mode = OBJLIB_BLINK_MODE_CLOSED;
        }
        bs->amount += step;
        break;
    case OBJLIB_BLINK_MODE_CLOSED:
        bs->timer = (u8)((f32)bs->timer + timeDelta);
        if (randomGetRange(0, 100) == 1) {
            switch (bs->mode) {
            case OBJLIB_BLINK_MODE_CLOSING:
            case OBJLIB_BLINK_MODE_CLOSED:
                bs->mode = OBJLIB_BLINK_MODE_OPENING;
                break;
            case OBJLIB_BLINK_MODE_WINK_RIGHT:
            case OBJLIB_BLINK_MODE_WINK_LEFT:
                bs->mode = OBJLIB_BLINK_MODE_OPEN;
                break;
            }
        }
        break;
    case OBJLIB_BLINK_MODE_OPENING:
        bs->timer = (u8)((f32)bs->timer + timeDelta);
        if ((s16)bs->amount - (s16)step < 0) {
            step = bs->amount;
            bs->mode = OBJLIB_BLINK_MODE_OPEN;
        }
        bs->amount -= step;
        break;
    case OBJLIB_BLINK_MODE_WINK_RIGHT:
        bs->timer = (u8)(16.0f * timeDelta + bs->timer);
        bs->amount = 0xff;
        rightScale = 0.0f;
        if (randomGetRange(0, 25) == 1) {
            switch (bs->mode) {
            case OBJLIB_BLINK_MODE_CLOSING:
            case OBJLIB_BLINK_MODE_CLOSED:
                bs->mode = OBJLIB_BLINK_MODE_OPENING;
                break;
            case OBJLIB_BLINK_MODE_WINK_RIGHT:
            case OBJLIB_BLINK_MODE_WINK_LEFT:
                bs->mode = OBJLIB_BLINK_MODE_OPEN;
                break;
            }
        }
        break;
    case OBJLIB_BLINK_MODE_WINK_LEFT:
        bs->timer = (u8)(16.0f * timeDelta + bs->timer);
        bs->amount = 0xff;
        leftScale = 0.0f;
        if (randomGetRange(0, 25) == 1) {
            switch (bs->mode) {
            case OBJLIB_BLINK_MODE_CLOSING:
            case OBJLIB_BLINK_MODE_CLOSED:
                bs->mode = OBJLIB_BLINK_MODE_OPENING;
                break;
            case OBJLIB_BLINK_MODE_WINK_RIGHT:
            case OBJLIB_BLINK_MODE_WINK_LEFT:
                bs->mode = OBJLIB_BLINK_MODE_OPEN;
                break;
            }
        }
        break;
    }

    phase = 0.09856f * bs->timer;
    wave = 0.25f * mathCosfHighPrecision(phase);
    wave = wave * bs->amount / 255.0f;
    rotation = (32768.0f * (leftScale * wave)) / 3.142f;
    *(s16*)(playerEyeAnim_FindJoint(objAnim, OBJLIB_BLINK_LEFT_JOINT_TAG) + 2) = rotation;

    rotation = (32768.0f * (rightScale * wave)) / 3.142f;
    *(s16*)(playerEyeAnim_FindJoint(objAnim, OBJLIB_BLINK_RIGHT_JOINT_TAG) + 2) = -rotation;
}

void objSetLookAtFlip(int mode, u8 enabled) {
    if ((int)(u8)mode != 0) {
        return;
    }
    gObjLookAtControlFlags.flip = enabled;
}

char sObjAddObjectTypeReachedMaxTypes[38] = "objAddObjectType: Reached MAXTYPES!!\n\000";

char sObjMsgOverflowInObjectWarning[64] = "objmsg (%x): overflow in object %d defno=%d FROM: defno %d\n";

int gObjLookAtTurnRateDivisor = 100;
f32 gObjMouthBlendFrames = 20.0f;

void objSoundUpdateMouth(GameObject* obj, ObjSoundState* state) {
    s16* found;
    int timer;

    timer = (s32)state->timer;
    found = objFindJointVecByKey(obj, 1);

    if (state->active != 0) {
        state->active = 0;
    } else if (Sfx_IsPlayingFromObjectChannel(obj, 0x10) != 0) {
        if (timer != -1) {
            timer -= framesThisStep;
            if (timer < 0) {
                Sfx_StopObjectChannel(obj, 0x10);
                state->blendWeight = 0.0f;
                state->pitch = 0;
            }
            state->timer = timer;
        }
    } else {
        state->timer = -1.0f;
        state->pitch = 0;
        if (state->blendWeight > 0.0f) {
            ObjModel* pi;
            state->blendWeight = 0.0f;
            pi = (ObjModel*)OBJPRINT_ACTIVE_BANK(obj);
            if (pi->file->morphTargetCount != 0) {
                ObjModel_SetBlendChannelTargets(pi, 2, pi->blendChannels[2].morphTargetB, -1,
                                                1.0f / gObjMouthBlendFrames, 0);
            }
        }
    }

    if (found != NULL) {
        found[0] = (s16)((found[0] + state->pitch) >> 1);
    }
}

void objKfAnimUpdate(GameObject* obj, ObjKfAnimState* state) {
    int frame;
    ObjModel* model;
    int kfval;
    int* kf;

    f32 t;

    if (state->frame < 0) {
        return;
    }
    t = state->timer - timeDelta;
    state->timer = t;
    if (t < 0.0f) {
        frame = state->frame;
        if (frame >= state->frameCount) {
            state->frame = -1;
            model = (ObjModel*)OBJPRINT_ACTIVE_BANK(obj);
            if (model->file->morphTargetCount != 0) {
                ObjModel_SetBlendChannelTargets(model, 2, model->blendChannels[2].morphTargetB, -1,
                                                1.0f / gObjMouthBlendFrames, 0);
            }
        } else {
            if (frame == 1) {
                Sfx_PlayFromObjectChannel(obj, 0x10, state->sfxId);
            }
            kf = state->keyframes;
            frame = state->frame;
            state->frame = frame + 1;
            kfval = kf[frame];
            model = (ObjModel*)OBJPRINT_ACTIVE_BANK(obj);
            if (model->file->morphTargetCount != 0) {
                ObjModel_SetBlendChannelTargets(model, 2, model->blendChannels[2].morphTargetB, kfval - 1,
                                                1.0f / gObjMouthBlendFrames, 0);
            }
            state->timer = state->timer + state->timerStep;
        }
    }
}

void objKfAnimStop(ObjKfAnimState* state) {
    state->frame = -1;
}

void objSoundStart(u32 obj, void* p, u16 sfxId) {
    if (Sfx_IsPlayingFromObjectChannel((GameObject*)obj, 0x10) == 0) {
        Sfx_PlayFromObjectChannel((GameObject*)obj, 0x10, sfxId);
        ((ObjSoundState*)p)->timer = -1.0f;
        ((ObjSoundState*)p)->pitch = -0x500;
        ((ObjSoundState*)p)->active = 1;
        ((ObjSoundState*)p)->blendWeight = 1.0f;
    }
}

void objSoundStartFromDef(GameObject* obj, ObjSoundState* state, ObjSoundDef* soundDef, u8 force) {
    u16 sfx;
    s16 pitch;
    u32 count;
    ObjModel* model;
    int did;

    pitch = soundDef->pitch;
    sfx = (u16)soundDef->sfxId;
    if (force != 0 || Sfx_IsPlayingFromObjectChannel(obj, 0x10) == 0) {
        Sfx_PlayFromObjectChannel(obj, 0x10, sfx);
        state->timer = -1.0f;
        state->pitch = (s16)(-pitch);
        state->active = 1;
        state->blendWeight = 1.0f;
    }
    count = soundDef->blendCount;
    if (count != 0) {
        model = (ObjModel*)OBJPRINT_ACTIVE_BANK(obj);
        if (model->file->morphTargetCount != 0) {
            ObjModel_SetBlendChannelTargets(model, 2, model->blendChannels[2].morphTargetB, count - 1,
                                            1.0f / gObjMouthBlendFrames, 0);
            did = 1;
        } else {
            did = 0;
        }
        if (did != 0) {
            soundDef->pitch = 0;
        }
    }
}

void objSoundStartTimed(GameObject* obj, ObjSoundState* state, u16 sfx, int pitch, int duration, u8 force) {
    if (force == 0 && Sfx_IsPlayingFromObjectChannel(obj, 0x10) != 0) {
        return;
    }
    Sfx_PlayFromObjectChannel(obj, 0x10, sfx);
    state->timer = duration;
    state->pitch = (s16)(-pitch);
    state->active = 1;
    state->blendWeight = 1.0f;
}

int gObjLookAtJointKeys[10] = {0, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11, 0x12, 0x13};

int* objGetLookAtJointKeys(void);
int* objGetLookAtJointKeys(void) {
    return gObjLookAtJointKeys;
}

ObjTextureRuntimeSlot* objFindTexture(GameObject* obj, int target, int unusedMaterialIndex) {
    ObjTextureRuntimeSlot* result = NULL;
    ObjDef* modelDef = (obj)->anim.modelInstance;
    if (modelDef != NULL) {
        int count;
        ObjTextureSlotDef* entries = modelDef->textureSlotDefs;
        if (entries == NULL) {
            return NULL;
        }
        {
            int i;
            count = modelDef->textureSlotCount;
            for (i = 0; i < count; i++) {
                if (target == entries[i].tag) {
                    result = &(obj)->anim.textureSlots[i];
                }
            }
        }
    }
    return result;
}

void objGetJointWorldPosition(GameObject* obj, int key, f32* outPosition);
void objGetJointWorldPosition(GameObject* obj, int key, f32* outPosition) {
    int* table;
    int i;
    int k;
    int n;
    int joint;
    int model;

    table = (void*)(obj)->anim.modelInstance;
    i = 0;
    n = (s32)(u32)((ObjDef*)table)->jointCount;
    for (k = 0; k < n; k++) {
        if (key == (int)(*(u8**)&((ObjDef*)table)->jointData)[i]) {
            joint = (*(u8**)&((ObjDef*)table)->jointData + i + OBJPRINT_ACTIVE_BANK_INDEX(obj))[1];
            break;
        }
        i = i + ((ObjDef*)table)->modelCount + 1;
    }
    model = (int)Obj_GetActiveModel(obj);
    model = (int)ObjModel_GetJointMatrix((u8*)model, joint);
    outPosition[0] = ((ObjModelJointMatrix*)model)->translationX;
    outPosition[1] = ((ObjModelJointMatrix*)model)->translationY;
    outPosition[2] = ((ObjModelJointMatrix*)model)->translationZ;
    outPosition[0] += playerMapOffsetX;
    outPosition[2] += playerMapOffsetZ;
}

s16* objFindJointPoseVector(GameObject* obj, int key);
s16* objFindJointPoseVector(GameObject* obj, int key) {
    int vecOffset;
    int jointData;
    int entryIdx;
    void* modelDef;
    s16* result;
    int count;
    int i;

    result = NULL;
    modelDef = OBJPRINT_MODEL_INSTANCE(obj);
    if (modelDef != NULL) {
        entryIdx = 0;
        vecOffset = 0;
        count = OBJPRINT_JOINT_COUNT(modelDef);
        for (i = 0; i < count; i++) {
            jointData = *(int*)&((ObjDef*)modelDef)->jointData;
            if ((int)*(u8*)(jointData + OBJPRINT_ACTIVE_BANK_INDEX(obj) + entryIdx + 1) != 0xff &&
                (s32) * (u8*)(jointData + entryIdx) == key) {
                result = (s16*)((char*)(obj)->anim.jointPoseData + vecOffset);
            }
            entryIdx += OBJPRINT_MODEL_COUNT(modelDef) + 1;
            vecOffset += 0x12;
        }
    }
    return result;
}

void characterDoEyeMovements(GameObject* obj, CharacterEyeAnimState* state, f32 unused);

void characterDoEyeMovements(GameObject* obj, CharacterEyeAnimState* state, f32 unused) {
    ObjTextureRuntimeSlot* foundA;
    ObjTextureRuntimeSlot* foundB;
    s16 t;
    int flag;
    s8 timer;

    foundA = characterFindEyeJoint(obj, 1);
    foundB = characterFindEyeJoint(obj, 0);
    if (foundA == NULL || foundB == NULL) {
        return;
    }

    flag = 0;
    t = state->movementStep;
    if (t == 0) {
        flag = 1;
    }
    if (t > 0) {
        if (foundA->offsetS >= state->movementTarget) {
            flag = 1;
        }
    }
    if (t < 0) {
        if (foundA->offsetS <= state->movementTarget) {
            flag = 1;
        }
    }
    if (flag != 0) {
        state->movementTarget = randomGetRange(-0x3e8, 0x3e8);
        state->movementStep = (state->movementTarget < foundA->offsetS) ? -0x96 : 0x96;
        state->movementTimer = randomGetRange(0x1e, 0x64);
    }
    timer = state->movementTimer;
    if (timer > 0) {
        state->movementTimer = timer - framesThisStep;
    } else {
        foundA->offsetS = (s16)(foundA->offsetS + state->movementStep * framesThisStep);
        foundA->offsetT = 0;
        foundB->offsetS = foundA->offsetS;
        foundB->offsetT = 0;
    }
}

static int characterTrackJointPitch(s16* curve, s16* state, f32 a, f32 b) {
    f32 buf[4];
    f32 ratio;
    s16 lo;
    s16 hi;

    buf[0] = a;
    buf[1] = a;
    buf[2] = b;
    buf[3] = -b;

    lo = curve[10];
    hi = curve[11];
    if (lo != hi) {
        ratio = ((f32)(s32)*state - (f32)(s32)hi) / ((f32)(s32)lo - (f32)(s32)hi);
    } else {
        return 1;
    }

    if (ratio > 1.0f) {
        ratio = 1.0f;
    } else if (ratio < 0.0f) {
        ratio = 0.0f;
    }

    {
        f32 rate = Curve_EvalHermite(buf, ratio, 0);
        if (curve[10] < curve[11]) {
            rate = -rate;
        }
        *state = rate * timeDelta + (f32)(s32)*state;
    }

    if (1.0f == ratio || *state >= 8191 || *state <= -8191) {
        *state = curve[10];
        return 1;
    }
    return 0;
}
static int characterTrackJointYaw(s16* curve, s16* state) {
    f32 buf[4];
    f32 ratio;
    s16 lo;
    s16 hi;

    buf[0] = 10.0f;
    buf[1] = 10.0f;
    buf[2] = 500.0f;
    buf[3] = -500.0f;

    lo = curve[10];
    hi = curve[11];
    if (lo != hi) {
        ratio = ((f32)(s32)state[1] - (f32)(s32)hi) / ((f32)(s32)lo - (f32)(s32)hi);
    } else {
        return 1;
    }

    if (ratio > 1.0f) {
        ratio = 1.0f;
    } else if (ratio < 0.0f) {
        ratio = 0.0f;
    }

    {
        f32 rate = Curve_EvalHermite(buf, ratio, 0);
        if (curve[10] < curve[11]) {
            rate = -rate;
        }
        state[1] = rate * timeDelta + (f32)(s32)state[1];
    }

    if (1.0f == ratio || state[1] >= 8191 || state[1] <= -8191) {
        state[1] = curve[10];
        return 1;
    }
    return 0;
}

static void characterHeadLookAlert(int obj, CharacterEyeAnimState* curve, s16* state, f32 val) {
    int masked;
    int flag;

    masked = (curve->headTrackMode >> 8) & 0xff;
    if (val > 0.1f) {
        flag = 1;
    } else {
        flag = 0;
    }
    if (masked != flag) {
        curve->headTrackMode = (s16)(flag << 8 | 4);
        curve->headYawStart = state[1];
        curve->headYaw = 0;
        curve->headTrackTimer = 0;
    }

    switch ((u8)curve->headTrackMode) {
    case 0:
        curve->headTrackMode = (s16)(flag << 8);
        curve->headTrackTimer = randomGetRange(0x32, 0xc8);
        break;
    case 1:
        curve->headTrackTimer -= framesThisStep;
        if (curve->headTrackTimer < 0) {
            if (randomGetRange(0, 100) > 90) {
                curve->headTrackMode = (s16)(flag << 8 | 5);
                if (curve->lookAtActive != 0) {
                    if (randomGetRange(0, 100) > 0) {
                        curve->headYaw = 0x1fff;
                        if (randomGetRange(0, 1) == 0) {
                            curve->headYaw = -curve->headYaw;
                        }
                    }
                } else {
                    curve->headYaw = 0x1fff;
                    if (randomGetRange(0, 1) == 0) {
                        curve->headYaw = -curve->headYaw;
                    }
                }
            }
        }
        break;
    case 2:
        break;
    case 5:
        if (curve->headTrackTimer > 0) {
            curve->headTrackTimer -= framesThisStep;
        } else if (characterTrackJointYaw((s16*)curve, state)) {
            curve->headTrackMode = (s16)(flag << 8 | 6);
            curve->headYaw = -curve->headYaw;
            curve->headTrackTimer = randomGetRange(0x14, 0x64);
        }
        break;
    case 6:
        if (curve->headTrackTimer > 0) {
            curve->headTrackTimer -= framesThisStep;
        } else if (characterTrackJointYaw((s16*)curve, state)) {
            curve->headTrackMode = (s16)(flag << 8 | 4);
            curve->headYaw = 0;
            curve->headTrackTimer = randomGetRange(0x14, 0x64);
        }
        break;
    case 4:
        if (curve->headTrackTimer > 0) {
            curve->headTrackTimer -= framesThisStep;
        } else if (characterTrackJointYaw((s16*)curve, state)) {
            curve->headTrackMode = (s16)(flag << 8);
            state[1] = 0;
        }
        break;
    }
}

static void characterHeadLookIdle(GameObject* obj, CharacterEyeAnimState* curve, s16* state, f32 val) {
    int masked;
    int flag;

    masked = (curve->headTrackMode >> 8) & 0xff;
    if (val > 0.1f) {
        flag = 1;
    } else {
        flag = 0;
    }
    if (masked != flag) {
        curve->headTrackMode = (s16)(flag << 8);
    }

    switch ((u8)curve->headTrackMode) {
    case 0:
        if (curve->lookAtActive != 0) {
            curve->headTrackMode = (s16)(flag << 8 | 3);
            curve->headYawStart = state[1];
            curve->headTrackBlend = 1.0f;
        } else {
            curve->headTrackMode = (s16)(flag << 8 | 1);
            curve->headTrackTimer = randomGetRange(100, 400);
            curve->headYaw = state[1];
        }
        break;
    case 1:
        curve->headTrackTimer -= framesThisStep;
        if (curve->headTrackTimer < 0) {
            int old = curve->headYaw;
            curve->headYaw = randomGetRange(0, 0x1fff);
            if (old > 0) {
                if (old - curve->headYaw < 0xe38) {
                    curve->headYaw += 0xe38;
                }
                if (curve->headYaw > 0x1fff) {
                    curve->headYaw = 0x1fff;
                }
                curve->headYaw = -curve->headYaw;
            } else {
                if (curve->headYaw - old < 0xe38) {
                    curve->headYaw += 0xe38;
                }
                if (curve->headYaw > 0x1fff) {
                    curve->headYaw = 0x1fff;
                }
            }
            curve->headTrackMode = (s16)(flag << 8 | 2);
            curve->headTrackTimer = 0;
            curve->headYawStart = state[1];
        }
        break;
    case 2:
        if (curve->lookAtActive != 0 || characterTrackJointYaw((s16*)curve, state) != 0) {
            curve->headTrackMode = (s16)(flag << 8);
        }
        break;
    case 3:
        if (curve->lookAtActive == 0) {
            curve->headTrackMode = (s16)(flag << 8);
        } else {
            int angle;
            int n;
            angle = getAngle(obj->anim.localPosX - curve->lookAtPosX,
                             obj->anim.localPosZ - curve->lookAtPosZ);
            curve->headYaw = (s16)(angle - (u16)obj->anim.rotX);
            if (curve->headYaw > 0x8000) {
                curve->headYaw = (s16)(curve->headYaw - 0xffff);
            }
            if (curve->headYaw < -0x8000) {
                curve->headYaw = (s16)(curve->headYaw + 0xffff);
            }
            n = curve->headYaw;
            if (n > 0x1fff || n < -0x1fff) {
                curve->headTrackMode = (s16)(flag << 8);
            } else {
                f32 t = curve->headTrackBlend;
                f32 lo = 0.0f;
                if (t > lo) {
                    f32 nv;
                    state[1] = t * (f32)(curve->headYawStart - n) + n;
                    nv = -(0.01f * timeDelta - curve->headTrackBlend);
                    curve->headTrackBlend = nv;
                    if (nv < lo) {
                        curve->headTrackBlend = lo;
                    }
                } else {
                    state[1] = n;
                }
            }
        }
        break;
    }

    if (state[1] < -0x1fff) {
        state[1] = -0x1fff;
    } else if (state[1] > 0x1fff) {
        state[1] = 0x1fff;
    }
}

void characterHeadLookRelax(GameObject* obj, void* state);
void characterHeadLookRelax(GameObject* obj, void* state) {
    s16* found;

    found = objFindJointVecByKey(obj, 0);
    if (found == NULL) {
        return;
    }
    if (found[0] != 0) {
        found[0] = (s16)((s32)found[0] * 3 / 4);
    }
    if (found[1] != 0) {
        found[1] = (s16)((s32)found[1] * 3 / 4);
    }
    ((CharacterEyeAnimState*)state)->headTrackMode = 0;
}

void characterUpdateHeadLook(GameObject* obj, CharacterEyeAnimState* state, f32 val) {
    s16* found;
    int flag;

    found = objFindJointVecByKey(obj, 0);
    if (found != NULL) {
        if (found[0] != 0) {
            found[0] = (s16)(found[0] * 3 / 4);
        }
        if (val < 0.0f) {
            val = -val;
        }
        if (val <= 0.1f) {
            characterHeadLookIdle(obj, state, found, val);
        } else {
            characterHeadLookAlert((int)obj, state, found, val);
        }
        state->headTrackMode = (s16)(u16)(u8)state->headTrackMode;
        if (val > 0.1f) {
            flag = 1;
        } else {
            flag = 0;
        }
        state->headTrackMode = (s16)(state->headTrackMode | (flag << 8));
    }
}
s16 objJointTracksAimAtTarget(GameObject* obj, GameObject* target, f32* pos, u8* p4, s16* spd, f32 yOff, int unused, int basePitch);
s16 objJointTracksAimAtTarget(GameObject* obj, GameObject* target, f32* pos, u8* p4, s16* spd, f32 yOff, int unused,
                              int basePitch) {
    s16 src[2];
    s16 dst[2];
    GameObject* go = obj;
    s16* found[1];
    s16* sp2;
    f32 dx, dy, dz, dist;
    int i;
    s16 ret;

    sp2 = spd + 0xf;
    dx = pos[0] - target->anim.localPosX;
    dz = pos[2] - target->anim.localPosZ;
    dy = (pos[1] + yOff) - target->anim.localPosY;
    dist = sqrtf(dx * dx + dz * dz);

    src[0] = (s16)getAngle(dx, dz) - (u16)go->anim.rotX;
    if (src[0] > 0x8000) {
        src[0] = (s16)(src[0] - 0xffff);
    }
    if (src[0] < -0x8000) {
        src[0] = (s16)(src[0] + 0xffff);
    }
    src[1] = basePitch - (u16)-getAngle(dist, dy);
    if (src[1] > 0x8000) {
        src[1] = (s16)(src[1] - 0xffff);
    }
    if (src[1] < -0x8000) {
        src[1] = (s16)(src[1] + 0xffff);
    }

    ret = src[0];
    if (gObjLookAtControlFlags.flip) {
        src[0] -= 0x8000;
        src[1] = -src[1];
        gObjLookAtControlFlags.flip = 0;
    }

    i = 0;
    while (i < 10) {
        int key;
        void* m[1];

        key = gObjLookAtJointKeys[i];
        found[0] = NULL;
        m[0] = (void*)go->anim.modelInstance;
        if (m[0] != NULL) {
            int iv[2];
            int n;
            int j;
            iv[0] = (int)found[0];
            iv[1] = (int)found[0];
            n = ((ObjDef*)m[0])->jointCount;
            for (j = 0; j < n; j++) {
                int entries = *(int*)&((ObjDef*)m[0])->jointData;
                if ((int)*(u8*)(entries + OBJPRINT_ACTIVE_BANK_INDEX(go) + iv[0] + 1) != 0xff &&
                    key == (int)*(u8*)(entries + iv[0])) {
                    found[0] = (s16*)((int)go->anim.jointPoseData + iv[1]);
                }
                iv[0] += ((ObjDef*)m[0])->modelCount + 1;
                iv[1] += 0x12;
            }
        }
        if (found[0] == NULL) {
            int t = (s16)ret;
            t = (t >= 0) ? t : -t;
            return (s16)(t < 0x100);
        }

        {
            int n2;
            for (n2 = 0; n2 < 2; n2++) {
                s16 v;
                s16 lim;
                if (n2 % 2 != 0) {
                    lim = (s16)(182.04f * (f32)sp2[i]);
                } else {
                    lim = (s16)(182.04f * (f32)spd[i]);
                }
                v = src[n2];
                dst[n2] = v;
                if (v > lim) {
                    dst[n2] = lim;
                    src[n2] -= lim;
                } else if (v < -lim) {
                    dst[n2] = -(s16)lim;
                    src[n2] += lim;
                } else {
                    src[n2] = 0;
                }
            }
        }

        if (p4 != NULL) {
            ((ObjJointTrackPair*)p4)->yaw.angle = dst[0];
            characterTrackJointYaw((s16*)p4, found[0]);
            ((ObjJointTrackPair*)p4)->pitch.angle = dst[1];
            characterTrackJointPitch((s16*)(p4 + 0x30), found[0], 10.0f, 500.0f);
            p4 += 0x60;
        } else {
            s16* fv = found[0];
            s16 d1 = (s16)((s16)((fv[1] + dst[0]) >> 1) - fv[1]);
            s16 lim;
            s16 d2;
            int t2;
            int lim3;

            lim = (d1 < framesThisStep * ((s16)(s32)(182.04f * (f32)-spd[i]) / gObjLookAtTurnRateDivisor))
                      ? framesThisStep * ((s16)(s32)(182.04f * (f32)-spd[i]) / gObjLookAtTurnRateDivisor)
                      : ((d1 > framesThisStep * ((s16)(s32)(182.04f * (f32)spd[i]) / gObjLookAtTurnRateDivisor))
                             ? framesThisStep * ((s16)(s32)(182.04f * (f32)spd[i]) / gObjLookAtTurnRateDivisor)
                             : d1);
            d2 = (s16)((s16)((fv[0] + dst[1]) >> 1) - fv[0]);
            t2 = (s16)(s32)(182.04f * (f32)sp2[i]);
            lim3 = (d2 < framesThisStep * (-t2 / (gObjLookAtTurnRateDivisor << 1)))
                       ? framesThisStep * (-t2 / (gObjLookAtTurnRateDivisor << 1))
                       : ((d2 > framesThisStep * (t2 / (gObjLookAtTurnRateDivisor << 1)))
                              ? framesThisStep * (t2 / (gObjLookAtTurnRateDivisor << 1))
                              : d2);
            fv[0] += (s16)lim3;
            fv[1] += lim;
        }

        if (i == 0) {
            ret -= found[0][1];
        }
        i++;
    }
    return src[0];
}

int characterTrackJointList(GameObject* objArg, int* keyList, int countArg, u8* p4Arg);
int characterTrackJointList(GameObject* objArg, int* keyList, int countArg, u8* p4Arg) {
    int* keys;
    int i;
    int total;
    u8* p4;
    int count;
    GameObject* obj;
    s16* found;

    obj = objArg;
    count = countArg;
    p4 = p4Arg;
    total = 0;
    i = 0;
    keys = keyList;
    while (i < count) {
        found = objFindJointVecByKey(obj, *keys);
        total += characterTrackJointYaw((s16*)p4, found);
        total += characterTrackJointPitch((s16*)(p4 + 0x30), found, 10.0f, 500.0f);
        keys++;
        i++;
        p4 += 0x60;
    }
    return (count * 2 - total) == 0;
}

void objJointTracksSetAngles(u8* channelData, int count, s16 yaw, s16 pitch);
void objJointTracksSetAngles(u8* channelData, int count, s16 yaw, s16 pitch) {
    ObjJointTrackPair* tracks = (ObjJointTrackPair*)channelData;

    while (count > 0) {
        tracks->yaw.angle = yaw;
        tracks->pitch.angle = pitch;
        tracks++;
        count--;
    }
}

void characterDoEyeMovements(GameObject* obj, CharacterEyeAnimState* state, f32 unused);

void objModelClearJointVectors(GameObject* obj);
void objModelClearJointVectors(GameObject* obj) {
    s16* found;
    int slot;

    for (slot = 0; slot < 0x16; slot++) {
        found = objFindJointVecByKey(obj, slot);
        if (found != NULL) {
            found[0] = 0;
            found[1] = 0;
            found[2] = 0;
        }
    }
}

void characterClampJointVecs(GameObject* obj, int* keys, int count, int lo, int hi);
void characterClampJointVecs(GameObject* obj, int* keys, int count, int lo, int hi) {
    s16* found;
    int idx;
    int v;

    for (idx = 0; idx < count; idx++) {
        found = objFindJointVecByKey(obj, *keys);
        if (found != NULL) {
            v = found[0];
            if (v < lo) {
                v = lo;
            } else if (v > hi) {
                v = hi;
            }
            found[0] = v;
            v = found[1];
            if (v < lo) {
                v = lo;
            } else if (v > hi) {
                v = hi;
            }
            found[1] = v;
            v = found[2];
            if (v < lo) {
                v = lo;
            } else if (v > hi) {
                v = hi;
            }
            found[2] = v;
        }
        keys++;
    }
}

void characterDecayJointVecs(GameObject* obj, int* keys, int count);
void characterDecayJointVecs(GameObject* obj, int* keys, int count) {
    s16* found;
    int idx;

    for (idx = 0; idx < count; idx++) {
        found = objFindJointVecByKey(obj, *keys);
        if (found != NULL) {
            found[1] = (s16)(found[1] * 3 >> 2);
            found[0] = (s16)(found[0] * 3 >> 2);
            found[2] = (s16)(found[2] * 3 >> 2);
        }
        keys++;
    }
}

void objJointTracksCaptureCurrentAngles(GameObject* obj, int* keys, int count, u8* out);
void objJointTracksCaptureCurrentAngles(GameObject* obj, int* keys, int count, u8* out) {
    s16* found;
    int idx;

    for (idx = 0; idx < count;) {
        found = objFindJointVecByKey(obj, *keys);
        if (found != NULL) {
            ((ObjJointTrackPair*)out)->yaw.angleStart = found[1];
            ((ObjJointTrackPair*)out)->pitch.angleStart = found[0];
        }
        keys++;
        idx++;
        out += 0x60;
    }
}

void characterAimHeadAtTarget(GameObject* obj, void* tgt, void* p3, int a, u8 inv, int b) {
    s16 ang[2];
    s16* found[1];
    void* m[1];

    found[0] = NULL;
    m[0] = (void*)(obj)->anim.modelInstance;
    if (m[0] != NULL) {
        int iv[2];
        int n;
        int j;
        iv[0] = (int)found[0];
        iv[1] = (int)found[0];
        n = ((ObjDef*)m[0])->jointCount;
        for (j = 0; j < n; j++) {
            int entries = *(int*)&((ObjDef*)m[0])->jointData;
            if ((int)*(u8*)(entries + OBJPRINT_ACTIVE_BANK_INDEX(obj) + iv[0] + 1) != 0xff &&
                (int)*(u8*)(entries + iv[0]) == 0) {
                found[0] = (s16*)((char*)(obj)->anim.jointPoseData + iv[1]);
            }
            iv[0] += ((ObjDef*)m[0])->modelCount + 1;
            iv[1] += 0x12;
        }
    }
    if (found[0] != NULL) {
        if (tgt == NULL) {
            found[0][1] = found[0][1] >> 1;
            found[0][0] = found[0][0] >> 1;
        } else {
            f32 dx = (obj)->anim.localPosX - ((GameObject*)tgt)->anim.localPosX;
            f32 dz = (obj)->anim.localPosZ - ((GameObject*)tgt)->anim.localPosZ;
            f32 dy = (obj)->anim.localPosY - ((GameObject*)tgt)->anim.localPosY;
            f32 dist = sqrtf(dx * dx + dz * dz);
            ObjJointTrackChannel* channel;
            s16* ap;
            int minB;
            int negA;
            int i;
            f32 prodB;

            ang[0] = (s16)getAngle(dx, dz) - (u16)(obj)->anim.rotX;
            if (ang[0] > 0x8000) {
                ang[0] = (s16)(ang[0] - 0xffff);
            }
            if (ang[0] < -0x8000) {
                ang[0] = (s16)(ang[0] + 0xffff);
            }
            if (inv != 0) {
                ang[0] = (s16)(ang[0] + 0x8000);
            }
            ang[1] = (s16)((s16)getAngle(dist, dy) - 0x3fff);

            a = (s16)(182.04f * a);
            channel = p3;
            ap = ang;
            prodB = 182.04f * b;
            minB = -(s16)(s32)prodB;
            negA = -a;
            for (i = 0; i < 2; i++) {
                int v;
                int w;
                f64 pd = prodB;
                *ap -= channel->angle;
                v = *ap;
                if (v < minB) {
                    w = minB;
                } else {
                    if (v > (s16)(s32)pd) {
                        v = (s32)pd;
                    }
                    w = (s16)v;
                }
                *ap = (s16)w;
                channel->angle += *ap;
                if (channel->angle > a) {
                    channel->angle = a;
                }
                if (channel->angle < negA) {
                    channel->angle = negA;
                }
                channel++;
                ap++;
            }
            found[0][1] = ((ObjJointTrackPair*)p3)->yaw.angle;
            found[0][0] = ((ObjJointTrackPair*)p3)->pitch.angle;
        }
    }
}

s16 gObjColorFilterRed;
s16 gObjColorFilterGreen;
s16 gObjColorFilterBlue;
f32* gObjModelMatrixOverride;
u8 gObjGlowColorRed;
u8 gObjGlowColorGreen;
u8 gObjGlowColorBlue;
u8 gObjGlowColorAlpha;
u8 gObjGlowColorEnabled;
u8 gObjColorFilterEnabled;

void characterSetHeadYawToTarget(GameObject* obj, GameObject* target, CharacterEyeAnimState* state, int maxAngle) {
    s16* found;

    found = objFindJointVecByKey(obj, 0);
    if (found != NULL) {
        state->headYaw = (s16)((s16)getAngle((obj)->anim.localPosX - target->anim.localPosX,
                                             (obj)->anim.localPosZ - target->anim.localPosZ) -
                               (obj)->anim.rotX);
        maxAngle = (s16)(182.04f * maxAngle);
        if (state->headYaw > maxAngle) {
            state->headYaw = maxAngle;
        }
        if (state->headYaw < -maxAngle) {
            state->headYaw = -maxAngle;
        }
        found[1] = state->headYaw;
    }
}

void characterCloseEyes(GameObject* obj, void* state) {
    ObjTextureRuntimeSlot* foundA;
    ObjTextureRuntimeSlot* foundB;
    int val;

    foundA = characterFindEyeJoint(obj, 5);
    foundB = characterFindEyeJoint(obj, 4);
    if (foundA == NULL || foundB == NULL) {
        return;
    }
    val = foundB->textureId;
    val += framesThisStep * 0x30;
    if (val >= 0x200) {
        val = 0x200;
    }
    foundA->textureId = val;
    foundB->textureId = val;
    ((CharacterEyeAnimState*)state)->blinkState = 1;
}

void characterDoEyeMovements(GameObject* obj, CharacterEyeAnimState* state, f32 unused);

void characterDoEyeAnims(GameObject* obj, void* stateData) {
    CharacterEyeAnimState* state = stateData;
    ObjTextureRuntimeSlot* a;
    ObjTextureRuntimeSlot* b;

    a = characterFindEyeJoint(obj, 5);
    b = characterFindEyeJoint(obj, 4);

    if (a == NULL || b == NULL) {
        return;
    }
    {
        int st;
        int v;

        v = b->textureId;
        st = state->blinkState;

        switch (st & 0xf) {
        case 0: {
            s8 blinkTimer = state->blinkTimer;
            if (blinkTimer > 0) {
                state->blinkTimer = blinkTimer - framesThisStep;
            } else if (randomGetRange(0, 1000) > 0x3de) {
                state->blinkState = 1;
                state->blinkTimer = 0;
            }
        } break;
        case 1:
            if ((st & 0x80) != 0) {
                v = v - framesThisStep * 0x60;
                if (v < 0) {
                    v = 0;
                    state->blinkState = 0;
                    state->blinkTimer = 0;
                }
            } else {
                v = v + framesThisStep * 0x60;
                if (v > 0x200) {
                    if (v - 0x200 < 0) {
                        v = 0;
                        state->blinkState = 0;
                    } else {
                        v = 0x2ff;
                        state->blinkState = -127;
                    }
                    state->blinkTimer = 0x28;
                }
            }
            a->textureId = v;
            b->textureId = v;
            break;
        }
        characterDoEyeMovements(obj, state, 0.0f);
    }
}

void characterHeadLookCalm(GameObject* obj, s16* state, f32 value) {
    s16* found;

    found = objFindJointVecByKey(obj, 0);
    if (found != NULL) {
        if (found[0] != 0) {
            found[0] = (s16)(found[0] * 3 / 4);
        }
        characterHeadLookIdle(obj, (CharacterEyeAnimState*)state, found, 0.0f);
        ((CharacterEyeAnimState*)state)->headTrackMode = (s16)(u16)(u8)((CharacterEyeAnimState*)state)->headTrackMode;
    }
}

void objSetGlowColor(int red, int green, int blue, u8 alpha);
void objSetGlowColor(int red, int green, int blue, u8 alpha) {
    gObjGlowColorRed = red;
    gObjGlowColorGreen = green;
    gObjGlowColorBlue = blue;
    gObjGlowColorEnabled = 1;
    gObjGlowColorAlpha = alpha;
}

void objSetColorFilter(s16 a, s16 b, s16 c);
void objSetColorFilter(s16 a, s16 b, s16 c) {
    gObjColorFilterRed = a;
    gObjColorFilterGreen = b;
    gObjColorFilterBlue = c;
    gObjColorFilterEnabled = 1;
}

#define OBJPRINT_CHILD_TABLE(staff) (*(char**)(*(char**)((staff) + 0x50) + 0x2c))

void staffUpdateSegmentTransforms(int staffArg, GameObject* objArg, int modelArg, int a, int b, int c);
void staffUpdateSegmentTransforms(int staffArg, GameObject* objArg, int modelArg, int a, int b, int c) {
    f32 va[3];
    Vec vb;
    int k;
    char* q;
    Vec* vp;
    Vec* vp0;
    int i;
    char* base;
    u8* model;
    int obj;
    char* staff;

    staff = (char*)staffArg;
    obj = (int)objArg;
    model = (u8*)modelArg;

    if (*(u8*)(*(char**)(staff + 0x50) + 0x58) >= 2 && ((GameObject*)staff)->anim.classId == 0x2d) {
        int off;
        base = (char*)((GameObject*)staff)->extra;
        i = 0;
        k = 1;
        off = 0x18;
        q = base;
        vp0 = (Vec*)va;
        vp = vp0;

        while (i < *(s16*)(base + 0xb0)) {
            if (k < *(u8*)(*(char**)(staff + 0x50) + 0x58)) {
                MtxPtr jm;
                int joint;
                joint = ((ChildEnt*)(OBJPRINT_CHILD_TABLE(staff) + off))[1].joints[OBJPRINT_ACTIVE_BANK_INDEX(staff)];
                jm = (MtxPtr)ObjModel_GetJointMatrix(model, joint);
                vp->x = ((ChildEnt*)(OBJPRINT_CHILD_TABLE(staff) + off))[1].pos[0];
                va[1] = ((ChildEnt*)(OBJPRINT_CHILD_TABLE(staff) + off))[1].pos[1];
                va[2] = ((ChildEnt*)(OBJPRINT_CHILD_TABLE(staff) + off))[1].pos[2];
                PSMTXMultVec(jm, vp, vp);
                vp->x = vp->x + playerMapOffsetX;
                va[2] = va[2] + playerMapOffsetZ;
                *(f32*)(q + 0x6c) = vp->x;
                *(f32*)(q + 0x74) = va[1];
                *(f32*)(q + 0x7c) = va[2];
            }
            if (k < *(u8*)(*(char**)(staff + 0x50) + 0x58)) {
                ChildEnt* row = (ChildEnt*)(OBJPRINT_CHILD_TABLE(staff) + off);
                int idx2 = row->joints[OBJPRINT_ACTIVE_BANK_INDEX(staff)];
                MtxPtr mtx2 =
                    (MtxPtr)(idx2 * 0x40 + *(int*)(model + ((((ObjModel*)model)->bufferFlags & 1) * 4) + 0xc));
                vb.x = row->pos[0];
                vb.y = ((ChildEnt*)(OBJPRINT_CHILD_TABLE(staff) + off))->pos[1];
                vb.z = ((ChildEnt*)(OBJPRINT_CHILD_TABLE(staff) + off))->pos[2];
                PSMTXMultVec(mtx2, &vb, &vb);
                vb.x = vb.x + playerMapOffsetX;
                vb.z = vb.z + playerMapOffsetZ;
                *(f32*)(q + 0x54) = vb.x;
                *(f32*)(q + 0x5c) = vb.y;
                *(f32*)(q + 0x64) = vb.z;
            }
            k += 2;
            off += 0x30;
            q += 4;
            i++;
            vp = vp0;
        }

        if (*(s16*)(base + 0xb0) != 0) {
            char* r = base + *(s16*)(base + 0xb2) * 4;
            va[0] = *(f32*)(r + 0x6c);
            va[1] = *(f32*)(r + 0x74);
            va[2] = *(f32*)(r + 0x7c);
            (*(void (**)(int, int, Vec*))(*(int*)((GameObject*)staff)->anim.dll + 0x28))((int)staff, obj, &vb);
            va[0] = va[0] - vb.x;
            va[1] = va[1] - vb.y;
            va[2] = va[2] - vb.z;
            ((GameObject*)staff)->anim.rotX = getAngle(va[0], va[2]);
            {
                f32 dx = va[0] * va[0];
                f32 dz = va[2] * va[2];
                ((GameObject*)staff)->anim.rotY = (s16)(-getAngle(va[1], sqrtf(dx + dz)) + 0x4000);
            }
            ((GameObject*)staff)->anim.rotZ = 0;
        }
    }
}

void objRenderShadowIfVisible(GameObject* obj, int wpad0, int wpad1, int wpad2, int wpad3, int wpad4) {
    void** arr = *(void***)&(obj)->anim.banks;
    s8 idx = (obj)->anim.bankIndex;
    if (arr[idx] != NULL) {
        objRenderShadow(obj);
    }
}

void objRenderModelAndHitVolumes(GameObject* obj, int p2, int p3, int p4, int p5, f32 scale) {
    int** table = OBJPRINT_BANK_TABLE((int*)obj);
    (void)scale;
    if (table[OBJPRINT_ACTIVE_BANK_INDEX(obj)] != NULL) {
        objRenderModel(obj);
        if (obj->anim.hitVolumeTransforms != NULL) {
            objUpdateHitVolumeTransforms(obj);
        }
    }
}

void objSetModelMatrixOverride(f32* matrix);
void objSetModelMatrixOverride(f32* matrix) {
    gObjModelMatrixOverride = matrix;
}

void objRender(int a, int b, int c, int d, GameObject* obj, int flag) {
    void* sub;
    int walk;
    int i;
    void (*vfn)(int, int, int, int, int, int);

    if ((obj->objectFlags & OBJECT_OBJFLAG_FREED) != 0 || obj->ownerObj != NULL) {
        return;
    }
    if ((obj->anim.flags & OBJANIM_FLAG_HIDDEN) != 0) {
        return;
    }
    sub = *(void**)&obj->anim.parent;
    if (sub != NULL && (((GameObject*)sub)->anim.flags & OBJANIM_FLAG_HIDDEN) != 0) {
        return;
    }

    doNothing_beforeRenderObject(4);
    obj->objectFlags |= OBJECT_OBJFLAG_RENDERED;
    sub = *(void**)&obj->anim.dll;
    if (sub != NULL) {
        if ((obj->objectFlags & OBJECT_OBJFLAG_HIDDEN) == 0) {
            vfn = *(void (**)(int, int, int, int, int, int))(*(int*)sub + 0x10);
            if (vfn != NULL) {
                vfn((int)obj, a, b, c, d, flag);
            }
        } else if ((s8)flag != 0 && OBJPRINT_ACTIVE_BANK(obj) != NULL) {
            objRenderModel(obj);
            if (obj->anim.hitVolumeTransforms != NULL) {
                objUpdateHitVolumeTransforms(obj);
            }
        }
    } else if ((s8)flag != 0) {
        switch (obj->anim.romDefNo) {
        case 0:
        case 0x1f:
            playerRender((int)obj, a, b, c, d, flag);
            break;
        default:
            if (OBJPRINT_ACTIVE_BANK(obj) != NULL) {
                objRenderModel(obj);
                if (obj->anim.hitVolumeTransforms != NULL) {
                    objUpdateHitVolumeTransforms(obj);
                }
            }
            break;
        }
    }
    doNothing_afterRenderObject();
    for (i = 0, walk = (int)obj; i < (s32)(u32)obj->childCount; i++) {
        int staff = *(int*)&((GameObject*)walk)->childObjs[0];
        if (((GameObject*)staff)->anim.classId == 0x2d) {
            staffUpdateSegmentTransforms(staff, obj, (int)OBJPRINT_ACTIVE_BANK(staff), a, b, c);
        }
        walk += 4;
    }
}
int objGetAlphaCompareThreshold(void);
int objGetAlphaCompareThreshold(void) {
    return gObjAlphaCompareThreshold;
}

void objSetAlphaCompareThreshold(u8 x);
void objSetAlphaCompareThreshold(u8 x) {
    gObjAlphaCompareThreshold = x;
}
