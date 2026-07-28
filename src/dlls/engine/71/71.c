/*
 * DLL 71 / 0x47.
 */
#include "main/camera_interface.h"
#include "main/curve.h"
#include "main/dll/CAM/camcannon_state.h"
#include "main/dll/CAM/dll_0047_cameramodeteststrength.h"
#include "main/camera_object.h"
#include "main/dll/rom_curve_interface.h"
#include "game/objects/object.h"
#include "main/mm.h"
#include "main/object_transform.h"
#include "main/pad.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/frame_timing.h"
#include "string.h"
#include "main/vecmath.h"
#include "main/resource.h"
#include "main/debug.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_float_helpers.h"

typedef RomCurvePathNode RomCurveNode;

/* curve-node field offsets (raw walking-pointer accesses below) */
#define NODE_SELF_ID    0x14
#define NODE_DIR_MASK   0x1B
#define NODE_NEIGHBOURS 0x1C
#define NODE_TAG0       0x31
#define NODE_TAG1       0x32
#define NODE_TAG2       0x33

extern char sPathCamNeedTwoControlPointsError[];

#define PATHCAM_NEAR_THRESHOLD 0.0f
#define PATHCAM_FAR_THRESHOLD  1.0f

void pathcam_advanceNodePair(int* nodeId, int* leadNodeId, f32 x, f32 y, f32 z, int tag)
{
    int node;
    RomCurvePathNode* linked;
    int noForwardExit;
    int slot;
    int slot2;
    int step;
    int window[4];
    int span;
    int farSpan;
    int settled;
    f32 dist;
    f32 nearThresh;

    node = (int)(*gRomCurveInterface)->getById(*nodeId);
    noForwardExit = 1;
    for (slot = 0; slot < ROM_CURVE_PATH_LINK_COUNT; slot++)
    {
        if (((RomCurvePathNode*)node)->links[slot] > -1 && (((RomCurvePathNode*)node)->directionMask & (1 << slot)) == 0)
        {
            linked = (RomCurvePathNode*)(*gRomCurveInterface)->getById(((RomCurvePathNode*)node)->links[slot]);
            if ((u32)linked != 0 && (linked->tag0 == tag || linked->tag1 == tag ||
                                     linked->tag2 == tag))
            {
                noForwardExit = 0;
                slot = ROM_CURVE_PATH_LINK_COUNT;
            }
        }
    }
    if (noForwardExit != 0)
    {
        for (slot = 0; slot < ROM_CURVE_PATH_LINK_COUNT; slot++)
        {
            if (((RomCurvePathNode*)node)->links[slot] > -1 && (((RomCurvePathNode*)node)->directionMask & (1 << slot)) != 0)
            {
                linked = (RomCurvePathNode*)(*gRomCurveInterface)->getById(((RomCurvePathNode*)node)->links[slot]);
                if ((u32)linked != 0 &&
                    (linked->tag0 == tag || linked->tag1 == tag ||
                     linked->tag2 == tag))
                {
                    *nodeId = ((RomCurvePathNode*)node)->links[slot];
                    slot = ROM_CURVE_PATH_LINK_COUNT;
                }
            }
        }
    }
    settled = 0;
    nearThresh = PATHCAM_NEAR_THRESHOLD;
    while (settled == 0)
    {
        settled = 1;
        node = (int)(*gRomCurveInterface)->getById(*nodeId);
        pathcam_findTaggedNodeWindow((u8*)node, window, tag);
        dist = pathcam_segmentParam(x, y, z, window);
        if (dist < nearThresh)
        {
            if (window[0] > -1)
            {
                *nodeId = window[0];
                settled = 0;
            }
        }
        else if (dist > PATHCAM_FAR_THRESHOLD)
        {
            if (window[2] > -1 && window[3] > -1)
            {
                *nodeId = window[2];
                settled = 0;
            }
        }
    }
    node = (int)(*gRomCurveInterface)->getById(*nodeId);
    pathcam_walkToPathEnd(node, &span, tag);
    node = (int)(*gRomCurveInterface)->getById(*leadNodeId);
    *leadNodeId = ((RomCurvePathNode*)pathcam_walkToPathEnd(node, &farSpan, tag))->selfId;
    for (step = 0; step < span; step++)
    {
        node = (int)(*gRomCurveInterface)->getById(*leadNodeId);
        for (slot2 = 0; slot2 < ROM_CURVE_PATH_LINK_COUNT; slot2++)
        {
            if (((RomCurvePathNode*)node)->links[slot2] > -1 &&
                (((RomCurvePathNode*)node)->directionMask & (1 << slot2)) == 0)
            {
                linked = (RomCurvePathNode*)(*gRomCurveInterface)->getById(((RomCurvePathNode*)node)->links[slot2]);
                if ((u32)linked != 0 &&
                    (linked->tag0 == tag || linked->tag1 == tag ||
                     linked->tag2 == tag))
                {
                    *leadNodeId = ((RomCurvePathNode*)node)->links[slot2];
                    slot2 = ROM_CURVE_PATH_LINK_COUNT;
                }
            }
        }
    }
}

int pathcam_walkToPathEnd(int curve, int* count, int tag)
{
    int slot;
    int done;
    int linked;

    done = 0;
    *count = 0;
    while (done == 0)
    {
        done = 1;
        if ((((RomCurvePathNode*)curve)->type != 0x1b) && (((RomCurvePathNode*)curve)->type != 0x1a))
        {
            for (slot = 0; slot < ROM_CURVE_PATH_LINK_COUNT; slot++)
            {
                if ((((RomCurvePathNode*)curve)->links[slot] > -1) &&
                    ((((RomCurvePathNode*)curve)->directionMask & (1 << slot)) != 0))
                {
                    linked = (int)(*gRomCurveInterface)->getById(((RomCurvePathNode*)curve)->links[slot]);
                    if (((u32)linked != 0) &&
                        ((((RomCurvePathNode*)linked)->tag0 == tag || (((RomCurvePathNode*)linked)->tag1 == tag)) ||
                         (((RomCurvePathNode*)linked)->tag2 == tag)))
                    {
                        curve = linked;
                        done = 0;
                        slot = ROM_CURVE_PATH_LINK_COUNT;
                    }
                }
            }
        }
        if (done == 0)
        {
            (*count)++;
        }
    }
    return curve;
}

void pathcam_buildWindowSamples(int* nodes, f32* o1, f32* o2, f32* o3, f32* o4, f32* o5, f32* o6, f32* o7)
{
    f32* wp;
    int* np;
    f32 *w1, *w2, *w3, *w4, *w5, *w6, *w7;
    RomCurveNode** ppNode;
    f32 *q1, *q2, *q3, *q4, *q5, *q6, *q7;
    RomCurveNode* node;
    int j;
    RomCurveNode** pwNode;
    int i;
    int step;
    f32* axisOut;
    int axis;
    f32 wrap, d, near, lower, upper, v0, v1;
    RomCurveNode* pts[4];

    i = 0;
    np = nodes;
    pwNode = pts;
    ppNode = pwNode;
    q1 = o1;
    q2 = o2;
    q3 = o3;
    q4 = o4;
    q5 = o5;
    q6 = o6;
    q7 = o7;
    for (; i < 4; i++)
    {
        *ppNode = (RomCurveNode*)(*gRomCurveInterface)->getById(*np);
        node = *ppNode;
        if (node != NULL)
        {
            *q1 = node->x;
            *q2 = node->y;
            *q3 = node->z;
            *q4 = (f32)node->sampleA;
            *q5 = (f32)node->sampleB;
            *q6 = (f32)node->sampleC;
            *q7 = (f32)node->sampleD;
        }
        np++;
        ppNode++;
        q1++;
        q2++;
        q3++;
        q4++;
        q5++;
        q6++;
        q7++;
    }

    if (pts[1] == NULL || pts[2] == NULL)
    {
        return;
    }
    {
        j = 0;
        w1 = o1;
        w2 = o2;
        w3 = o3;
        w4 = o4;
        w5 = o5;
        w6 = o6;
        w7 = o7;
        for (; j < 4; j++)
        {
            if (*pwNode == NULL)
            {
                if (j == 0)
                {
                    node = pts[1];
                    *w1 = node->x + (node->x - pts[2]->x);
                    *w2 = node->y + (node->y - pts[2]->y);
                    *w3 = node->z + (node->z - pts[2]->z);
                    *w4 = (f32)(node->sampleA + (node->sampleA - pts[2]->sampleA));
                    *w5 = (f32)(node->sampleB + (node->sampleB - pts[2]->sampleB));
                    *w6 = (f32)(node->sampleC + (node->sampleC - pts[2]->sampleC));
                    *w7 = (f32)node->sampleD + ((f32)node->sampleD - (f32)pts[2]->sampleD);
                }
                else if (j == 3)
                {
                    node = pts[2];
                    *w1 = node->x + (node->x - pts[1]->x);
                    *w2 = node->y + (node->y - pts[1]->y);
                    *w3 = node->z + (node->z - pts[1]->z);
                    *w4 = (f32)(node->sampleA + (node->sampleA - pts[1]->sampleA));
                    *w5 = (f32)(node->sampleB + (node->sampleB - pts[1]->sampleB));
                    *w6 = (f32)(node->sampleC + (node->sampleC - pts[1]->sampleC));
                    *w7 = (f32)node->sampleD + ((f32)node->sampleD - (f32)pts[1]->sampleD);
                }
            }
            pwNode++;
            w1++;
            w2++;
            w3++;
            w4++;
            w5++;
            w6++;
            w7++;
        }

        axis = 0;
        do
        {
            if (axis == 0)
            {
                axisOut = o4;
            }
            else if (axis == 1)
            {
                axisOut = o5;
            }
            else
            {
                axisOut = o6;
            }
            if (axisOut != NULL)
            {
                wp = axisOut;
                upper = 32768.0f;
                step = 0;
                while (step < 3)
                {
                    v0 = wp[0];
                    v1 = wp[1];
                    d = v0 - v1;
                    if (d > upper || d < -32768.0f)
                    {
                        if (v0 < 0.0f)
                        {
                            wp[0] += 65535.0f;
                        }
                        else if (v1 < 0.0f)
                        {
                            wp[1] += 65535.0f;
                        }
                    }
                    wp++;
                    step++;
                }
            }
            axis++;
        } while (axis < 3);
    }
}

void pathcam_findTaggedNodeWindow(u8* node, int* out, int tag)
{
    int i;
    u8* neighbour;
    int idx;
    int forward;

    out[0] = -1;
    out[1] = -1;
    out[2] = -1;
    out[3] = -1;

    if (node == NULL)
    {
        return;
    }

    out[1] = *(int*)(node + NODE_SELF_ID);

    i = 0;
    for (; i < 5; i++)
    {
        idx = *(int*)(node + i * 4 + NODE_NEIGHBOURS);
        if (idx > -1)
        {
            neighbour = (u8*)(*gRomCurveInterface)->getById(idx);
            if (neighbour != NULL)
            {
                if (neighbour[NODE_TAG0] == tag || neighbour[NODE_TAG1] == tag || neighbour[NODE_TAG2] == tag)
                {
                    forward = (s8)node[NODE_DIR_MASK] & (1 << i);
                    if (forward != 0)
                    {
                        out[0] = *(int*)(node + i * 4 + NODE_NEIGHBOURS);
                    }
                    else if (forward == 0)
                    {
                        out[2] = *(int*)(node + i * 4 + NODE_NEIGHBOURS);
                    }
                }
            }
        }
    }

    idx = out[2];
    if (idx > -1)
    {
        u8* node2 = (u8*)(*gRomCurveInterface)->getById(idx);
        if (node2 != NULL)
        {
            if (node2[NODE_TAG0] == tag || node2[NODE_TAG1] == tag || node2[NODE_TAG2] == tag)
            {
                i = 0;
                for (; i < 5; i++)
                {
                    idx = *(int*)(node2 + i * 4 + NODE_NEIGHBOURS);
                    if (idx > -1)
                    {
                        forward = (s8)node2[NODE_DIR_MASK] & (1 << i);
                        if (forward == 0)
                        {
                            neighbour = (u8*)(*gRomCurveInterface)->getById(idx);
                            if (neighbour != NULL)
                            {
                                if (neighbour[NODE_TAG0] == tag || neighbour[NODE_TAG1] == tag ||
                                    neighbour[NODE_TAG2] == tag)
                                {
                                    out[3] = *(int*)(node2 + i * 4 + NODE_NEIGHBOURS);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (out[1] < 0 || out[2] < 0)
    {
        debugPrintf(sPathCamNeedTwoControlPointsError);
    }
}

f32 pathcam_segmentParam(f32 px, f32 unused, f32 pz, int* obj)
{
    RomCurveNode* pts[4];
    int* sp;
    RomCurveNode** dp;
    int i;
    f32 dx1;
    f32 dz1;
    f32 psx;
    f32 sx;
    f32 sz;
    f32 nsz;
    f32 nsx;
    f32 nz;
    f32 nx;
    f32 len;
    f32 t1;
    f32 t2;
    f32 negdot;
    f32 p1x;
    f32 p1z;
    for (i = 0, sp = obj, dp = pts; i < 4; i++)
    {
        *dp = (RomCurveNode*)(*gRomCurveInterface)->getById(*sp);
        sp++;
        dp++;
    }
    dx1 = pts[2]->x - pts[1]->x;
    dz1 = pts[2]->z - pts[1]->z;
    if (pts[0] != NULL)
    {
        psx = pts[1]->x - pts[0]->x;
        sz = pts[1]->z - pts[0]->z;
    }
    else
    {
        psx = dx1;
        sz = dz1;
    }
    nx = 0.5f * (psx + dx1);
    nz = 0.5f * (sz + dz1);
    len = sqrtf(nx * nx + nz * nz);
    if (0.0f != len)
    {
        nx = nx / len;
        nz = nz / len;
    }
    p1x = pts[1]->x;
    p1z = pts[1]->z;
    negdot = nx * p1x + nz * p1z;
    negdot = -negdot;
    t1 = nx * dx1 + nz * dz1;
    if (0.0f != t1)
    {
        t1 = -(negdot + (nx * px + nz * pz)) / t1;
    }
    sx = pts[2]->x - p1x;
    sz = pts[2]->z - p1z;
    if (pts[3] != NULL)
    {
        nsx = pts[3]->x - pts[2]->x;
        nsz = pts[3]->z - pts[2]->z;
    }
    else
    {
        nsx = sx;
        nsz = sz;
    }
    nx = 0.5f * (nsx + sx);
    nz = 0.5f * (nsz + sz);
    len = sqrtf(nx * nx + nz * nz);
    if (0.0f != len)
    {
        nx = nx / len;
        nz = nz / len;
    }
    negdot = nx * pts[2]->x + nz * pts[2]->z;
    negdot = -negdot;
    t2 = nx * dx1 + nz * dz1;
    if (0.0f != t2)
    {
        t2 = -(negdot + (nx * px + nz * pz)) / t2;
    }
    return -t1 / (t2 - t1);
}


CamCannonState* gCamCannonState;

#define CAMTESTSTRENGTH_CAMMODE_DEFAULT 0x42

void cameraModeTestStrengthFn_8010b238(f32 fovEnd, CameraObject* camera, f32* posEnd, s32 rotXEnd, s32 rotYEnd,
                                       s32 rotZEnd);


u32 camTestStrengthUpdateBlend(CameraObject* camera, u32 flagsIn)
{
    u8 flags;
    f32 speed;
    f32 t;

    gCamCannonState->posXEnd = camera->anim.localPosX;
    gCamCannonState->posYEnd = camera->anim.localPosY;
    gCamCannonState->posZEnd = camera->anim.localPosZ;
    gCamCannonState->rotXEnd = camera->anim.rotX;
    gCamCannonState->rotYEnd = camera->anim.rotY;
    gCamCannonState->rotZEnd = camera->anim.rotZ;
    gCamCannonState->fovEnd = camera->fov;

    if (0.0f != gCamCannonState->duration)
    {
        speed = gCamCannonState->elapsed / gCamCannonState->duration;
    }
    else
    {
        speed = 0.0f;
    }
    if (speed > 1.0f)
    {
        speed = 1.0f;
    }
    speed = Curve_EvalHermite(gCamCannonState->speedCurve, speed, 0x0);
    if (speed < 0.2f)
    {
        speed = 0.2f;
    }
    gCamCannonState->elapsed += speed * timeDelta;

    t = 0.0f;
    if (t != gCamCannonState->duration)
    {
        t = gCamCannonState->elapsed / gCamCannonState->duration;
    }
    if (t > 1.0f)
    {
        t = 1.0f;
    }
    camera->anim.localPosX = Curve_EvalLinear(&gCamCannonState->posXStart, t, NULL);
    camera->anim.localPosY = Curve_EvalLinear(&gCamCannonState->posYStart, t, NULL);
    camera->anim.localPosZ = Curve_EvalLinear(&gCamCannonState->posZStart, t, NULL);
    camera->fov = Curve_EvalLinear(&gCamCannonState->fovStart, t, NULL);

    if (((gCamCannonState->rotXStart - gCamCannonState->rotXEnd) > 32768.0f) ||
        ((gCamCannonState->rotXStart - gCamCannonState->rotXEnd) < -32768.0f))
    {
        if (gCamCannonState->rotXStart < 0.0f)
        {
            gCamCannonState->rotXStart += 65535.0f;
        }
        else if (gCamCannonState->rotXEnd < 0.0f)
        {
            gCamCannonState->rotXEnd += 65535.0f;
        }
    }
    if (((gCamCannonState->rotYStart - gCamCannonState->rotYEnd) > 32768.0f) ||
        ((gCamCannonState->rotYStart - gCamCannonState->rotYEnd) < -32768.0f))
    {
        if (gCamCannonState->rotYStart < 0.0f)
        {
            gCamCannonState->rotYStart += 65535.0f;
        }
        else if (gCamCannonState->rotYEnd < 0.0f)
        {
            gCamCannonState->rotYEnd += 65535.0f;
        }
    }
    if (((gCamCannonState->rotZStart - gCamCannonState->rotZEnd) > 32768.0f) ||
        ((gCamCannonState->rotZStart - gCamCannonState->rotZEnd) < -32768.0f))
    {
        if (gCamCannonState->rotZStart < 0.0f)
        {
            gCamCannonState->rotZStart += 65535.0f;
        }
        else if (gCamCannonState->rotZEnd < 0.0f)
        {
            gCamCannonState->rotZEnd += 65535.0f;
        }
    }

    flags = flagsIn;
    if ((flags & 1) == 0)
    {
        camera->anim.rotX = Curve_EvalLinear(&gCamCannonState->rotXStart, t, NULL);
    }
    if ((flags & 2) == 0)
    {
        camera->anim.rotY = Curve_EvalLinear(&gCamCannonState->rotYStart, t, NULL);
    }
    if ((flags & 4) == 0)
    {
        camera->anim.rotZ = Curve_EvalLinear(&gCamCannonState->rotZStart, t, NULL);
    }
    return t >= 1.0f;
}

void cameraModeTestStrengthFn_8010b238(f32 fovEnd, CameraObject* camera, f32* posEnd, s32 rotXEnd, s32 rotYEnd,
                                       s32 rotZEnd)
{
    f32 dx;
    f32 dy;
    f32 dz;

    gCamCannonState->transitionComplete = 0;
    gCamCannonState->posXStart = camera->anim.localPosX;
    gCamCannonState->posYStart = camera->anim.localPosY;
    gCamCannonState->posZStart = camera->anim.localPosZ;
    gCamCannonState->rotXStart = (f32)(s32)camera->anim.rotX;
    gCamCannonState->rotYStart = (f32)(s32)camera->anim.rotY;
    gCamCannonState->rotZStart = (f32)(s32)camera->anim.rotZ;
    gCamCannonState->fovStart = camera->fov;
    gCamCannonState->posXEnd = posEnd[0];
    gCamCannonState->posYEnd = posEnd[1];
    gCamCannonState->posZEnd = posEnd[2];
    gCamCannonState->rotXEnd = rotXEnd;
    gCamCannonState->rotYEnd = rotYEnd;
    gCamCannonState->rotZEnd = rotZEnd;
    gCamCannonState->fovEnd = fovEnd;
    gCamCannonState->elapsed = 0.0f;
    dx = gCamCannonState->posXEnd - gCamCannonState->posXStart;
    dy = gCamCannonState->posYEnd - gCamCannonState->posYStart;
    dz = gCamCannonState->posZEnd - gCamCannonState->posZStart;
    gCamCannonState->duration = sqrtf(dx * dx + dy * dy + dz * dz);
    (*gCameraInterface)
        ->initialise(gCamCannonState->duration, gCamCannonState->speedCurve, 100.0f, 0.1f, 0.1f, -5.0f);
}

void CameraModeTestStrength_copyToCurrent(void)
{
}

void CameraModeTestStrength_free(void)
{
    mm_free((void*)gCamCannonState);
    gCamCannonState = 0;
}

void CameraModeTestStrength_update(CameraObject* cam)
{
    int lockRoll;
    GameObject* obj;
    int lockPitch;
    int lockYaw;
    int node;
    int flags;
    f32 t;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 param;
    int yaw;
    int node2;
    int nextWindow[4];
    int prevWindow[4];
    f32 x[4];
    f32 y[4];
    f32 z[4];
    f32 pitchS[4];
    f32 yawS[4];
    f32 rollS[4];
    f32 fov[4];

    if (gCamCannonState->pathFailed != 0)
    {
        (*gCameraInterface)->setMode(CAMTESTSTRENGTH_CAMMODE_DEFAULT, 0, 1, 0, NULL, 0, 0xff);
    }
    else
    {
        obj = cam->anim.targetObj;
        getButtonsJustPressed(0);
        node = (int)(*gRomCurveInterface)->getById(gCamCannonState->nextNodeId);
        node2 = (int)(*gRomCurveInterface)->getById(gCamCannonState->prevNodeId);
        pathcam_findTaggedNodeWindow((u8*)node2, prevWindow, gCamCannonState->pathTag);
        pathcam_findTaggedNodeWindow((u8*)node, nextWindow, gCamCannonState->pathTag);
        pathcam_buildWindowSamples(prevWindow, x, y, z, pitchS, yawS, rollS, fov);
        param = pathcam_segmentParam(obj->anim.worldPosX, obj->anim.worldPosY,
                            obj->anim.worldPosZ, nextWindow);
        if (param < 0.0f)
        {
            if (nextWindow[0] > -1)
            {
                gCamCannonState->nextNodeId = nextWindow[0];
                node2 = (int)(*gRomCurveInterface)->getById(gCamCannonState->nextNodeId);
                pathcam_findTaggedNodeWindow((u8*)node2, nextWindow, gCamCannonState->pathTag);
                if (prevWindow[0] > -1)
                {
                    gCamCannonState->prevNodeId = prevWindow[0];
                    node2 = (int)(*gRomCurveInterface)->getById(gCamCannonState->prevNodeId);
                    pathcam_findTaggedNodeWindow((u8*)node2, prevWindow, gCamCannonState->pathTag);
                    pathcam_buildWindowSamples(prevWindow, x, y, z, pitchS, yawS, rollS, fov);
                    param = pathcam_segmentParam(obj->anim.worldPosX, obj->anim.worldPosY,
                                        obj->anim.worldPosZ, nextWindow);
                    gCamCannonState->pathProgress += 1.0f;
                }
                else
                {
                    param = 0.0f;
                }
            }
            else
            {
                param = 0.0f;
            }
        }
        else if (param > 1.0f)
        {
            if (nextWindow[2] > -1 && nextWindow[3] > -1)
            {
                gCamCannonState->nextNodeId = nextWindow[2];
                node2 = (int)(*gRomCurveInterface)->getById(gCamCannonState->nextNodeId);
                pathcam_findTaggedNodeWindow((u8*)node2, nextWindow, gCamCannonState->pathTag);
                if (prevWindow[2] > -1 && prevWindow[3] > -1)
                {
                    gCamCannonState->prevNodeId = prevWindow[2];
                    node2 = (int)(*gRomCurveInterface)->getById(gCamCannonState->prevNodeId);
                    pathcam_findTaggedNodeWindow((u8*)node2, prevWindow, gCamCannonState->pathTag);
                    pathcam_buildWindowSamples(prevWindow, x, y, z, pitchS, yawS, rollS, fov);
                    param = pathcam_segmentParam(obj->anim.worldPosX, obj->anim.worldPosY,
                                        obj->anim.worldPosZ, nextWindow);
                    gCamCannonState->pathProgress -= 1.0f;
                }
                else
                {
                    param = 1.0f;
                }
            }
            else
            {
                param = 1.0f;
            }
        }
        t = 0.3f * (param - gCamCannonState->pathProgress) + gCamCannonState->pathProgress;
        gCamCannonState->pathProgress = t;
        cam->anim.worldPosX = Curve_EvalBSpline(x, t, 0);
        cam->anim.worldPosY = Curve_EvalBSpline(y, t, 0);
        cam->anim.worldPosZ = Curve_EvalBSpline(z, t, 0);
        node2 = (int)(*gRomCurveInterface)->getById(gCamCannonState->prevNodeId);
        flags = *(u8*)(node2 + 0x3b);
        lockPitch = flags & 1;
        if (lockPitch == 0)
        {
            cam->anim.rotX = (int)Curve_EvalCatmullRom(pitchS, t, 0) + 0x8000;
        }
        lockYaw = flags & 2;
        if (lockYaw == 0)
        {
            cam->anim.rotY = Curve_EvalCatmullRom(yawS, t, 0);
        }
        lockRoll = flags & 4;
        if (lockRoll == 0)
        {
            cam->anim.rotZ = Curve_EvalCatmullRom(rollS, t, 0);
        }
        cam->fov = Curve_EvalBSpline(fov, t, 0);
        if (gCamCannonState->transitionComplete == 0 && (s32)camTestStrengthUpdateBlend((CameraObject*)cam, (u32)flags) != 0)
        {
            gCamCannonState->transitionComplete = 1;
        }
        dx = cam->anim.worldPosX - obj->anim.worldPosX;
        dy = cam->anim.worldPosY - obj->anim.worldPosY;
        dz = cam->anim.worldPosZ - obj->anim.worldPosZ;
        if (lockPitch != 0)
        {
            cam->anim.rotX = 0x8000 - getAngle(dx, dz);
        }
        if (lockYaw != 0)
        {
            int delta;
            yaw = getAngle(dy, sqrtf(dx * dx + dz * dz)) & 0xffff;
            delta = (int)(((f32)yaw - Curve_EvalCatmullRom(yawS, t, 0)) - (f32)(cam->anim.rotY & 0xffff));
            if (delta > 0x8000)
            {
                delta -= 0xffff;
            }
            if (delta < -0x8000)
            {
                delta += 0xffff;
            }
            cam->anim.rotY += ((int)(delta * framesThisStep) >> 3);
        }
        if (lockRoll != 0)
        {
            int delta = cam->anim.rotZ - (obj->anim.rotZ & 0xffff);
            if (delta > 0x8000)
            {
                delta -= 0xffff;
            }
            if (delta < -0x8000)
            {
                delta += 0xffff;
            }
            cam->anim.rotZ += ((int)(delta * framesThisStep) >> 3);
        }
        if (gCamCannonState->linkedObject != NULL)
        {
            f32 v;
            v = cam->anim.worldPosX;
            ((GameObject*)gCamCannonState->linkedObject)->anim.worldPosX = v;
            ((GameObject*)gCamCannonState->linkedObject)->anim.localPosX = v;
            v = cam->anim.worldPosY;
            ((GameObject*)gCamCannonState->linkedObject)->anim.worldPosY = v;
            ((GameObject*)gCamCannonState->linkedObject)->anim.localPosY = v;
            v = cam->anim.worldPosZ;
            ((GameObject*)gCamCannonState->linkedObject)->anim.worldPosZ = v;
            ((GameObject*)gCamCannonState->linkedObject)->anim.localPosZ = v;
        }
        Obj_TransformWorldPointToLocal(cam->anim.worldPosX, cam->anim.worldPosY,
                                       cam->anim.worldPosZ, &cam->anim.localPosX,
                                       &cam->anim.localPosY, &cam->anim.localPosZ,
                                       *(int*)&cam->anim.parent);
    }
}

void CameraModeTestStrength_init(CameraObject* cam, int param2, int* param3)
{
    int romNode;
    GameObject* obj;
    int curveNode2;
    s16 pitch;
    s16 yaw;
    s16 roll;
    f32 t;
    f32 px;
    f32 py;
    f32 pz;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 fov;
    f32 pos[3];
    int nextW[4];
    int prevW[4];
    f32 pitchS[4];
    f32 yawS[4];
    f32 rollS[4];
    f32 fovS[4];
    f32 xS[4];
    f32 yS[4];
    f32 zS[4];
    int tags[2];

    obj = cam->anim.targetObj;
    if (gCamCannonState == 0)
    {
        gCamCannonState = (CamCannonState*)mmAlloc(sizeof(CamCannonState), 0xf, 0);
    }
    memset(gCamCannonState, 0, sizeof(CamCannonState));
    gCamCannonState->pathTag = *param3;
    gCamCannonState->transitionComplete = 1;
    tags[0] = 9;
    tags[1] = 0x1b;
    gCamCannonState->nextNodeId = (*gRomCurveInterface)->find(
        obj->anim.worldPosX, obj->anim.worldPosY, obj->anim.worldPosZ,
        tags, 2, gCamCannonState->pathTag);
    tags[0] = 8;
    tags[1] = 0x1a;
    gCamCannonState->prevNodeId = (*gRomCurveInterface)->find(
        obj->anim.worldPosX, obj->anim.worldPosY, obj->anim.worldPosZ,
        tags, 2, gCamCannonState->pathTag);
    pathcam_advanceNodePair(&gCamCannonState->nextNodeId, &gCamCannonState->prevNodeId, obj->anim.worldPosX,
                obj->anim.worldPosY, obj->anim.worldPosZ, gCamCannonState->pathTag);
    romNode = (int)(*gRomCurveInterface)->getById(gCamCannonState->prevNodeId);
    curveNode2 = (int)(*gRomCurveInterface)->getById(gCamCannonState->nextNodeId);
    pathcam_findTaggedNodeWindow((u8*)romNode, prevW, gCamCannonState->pathTag);
    pathcam_findTaggedNodeWindow((u8*)curveNode2, nextW, gCamCannonState->pathTag);
    pathcam_buildWindowSamples(prevW, xS, yS, zS, pitchS, yawS, rollS, fovS);
    t = pathcam_segmentParam(obj->anim.worldPosX, obj->anim.worldPosY,
                    obj->anim.worldPosZ, nextW);
    if (t < 0.0f)
    {
        t = 0.0f;
    }
    else if (t > 1.0f)
    {
        t = 1.0f;
    }
    px = Curve_EvalBSpline(xS, t, 0);
    py = Curve_EvalBSpline(yS, t, 0);
    pz = Curve_EvalBSpline(zS, t, 0);
    dx = px - obj->anim.worldPosX;
    dy = py - obj->anim.worldPosY;
    dz = pz - obj->anim.worldPosZ;
    if ((*(u8*)(romNode + 0x3b) & 1) != 0)
    {
        pitch = (s16)(0x8000 - getAngle(dx, dz));
    }
    else
    {
        pitch = (s16)((int)Curve_EvalCatmullRom(pitchS, t, 0) + 0x8000);
    }
    if ((*(u8*)(romNode + 0x3b) & 4) != 0)
    {
        roll = obj->anim.rotZ;
    }
    else
    {
        roll = Curve_EvalCatmullRom(rollS, t, 0);
    }
    if ((*(u8*)(romNode + 0x3b) & 2) != 0)
    {
        yaw = (s16)getAngle(dy, sqrtf(dx * dx + dz * dz));
        yaw = (f32)yaw - Curve_EvalCatmullRom(yawS, t, 0);
    }
    else
    {
        yaw = Curve_EvalCatmullRom(yawS, t, 0);
    }
    fov = Curve_EvalBSpline(fovS, t, 0);
    pos[0] = px;
    pos[1] = py;
    pos[2] = pz;
    if (*((u8*)param3 + 4) == 0 && param2 != 3)
    {
        cameraModeTestStrengthFn_8010b238(fov, cam, pos, pitch, yaw, roll);
    }
    else
    {
        cam->anim.worldPosX = px;
        cam->anim.worldPosY = py;
        cam->anim.worldPosZ = pz;
        Obj_TransformWorldPointToLocal(cam->anim.worldPosX, cam->anim.worldPosY,
                                       cam->anim.worldPosZ, &cam->anim.localPosX,
                                       &cam->anim.localPosY, &cam->anim.localPosZ,
                                       *(int*)&cam->anim.parent);
        cam->anim.rotX = pitch;
        cam->anim.rotY = yaw;
        cam->anim.rotZ = roll;
        cam->fov = fov;
    }
    gCamCannonState->pathProgress = t;
}

void CameraModeTestStrength_release(void)
{
}

void CameraModeTestStrength_initialise(void)
{
}

ResourceDescriptorCallbacks7 lbl_80319C88 = {
    {0x00000000, 0x00000000, 0x00000000, 0x00060000},
    {(ResourceDescriptorCallback)CameraModeTestStrength_initialise,
     (ResourceDescriptorCallback)CameraModeTestStrength_release,
     0x00000000,
     (ResourceDescriptorCallback)CameraModeTestStrength_init,
     (ResourceDescriptorCallback)CameraModeTestStrength_update,
     (ResourceDescriptorCallback)CameraModeTestStrength_free,
     (ResourceDescriptorCallback)CameraModeTestStrength_copyToCurrent}};

char sPathCamNeedTwoControlPointsError[] = "PATHCAM error: need at least two control points\n";
