/*
 * Path-camera ROM-curve graph navigation and sampling helpers (DLL 0x5B /
 * camshipbattle5C family). Operates on the ROM curve-node graph reached
 * through gRomCurveInterface->getById.
 *
 * Each curve node holds a world position (f32 x/y/z at +0x08/+0x0C/+0x10),
 * a packed rotation/fov sample (s16 at +0x34/+0x36/+0x38, s8 at +0x3A),
 * a self id (+0x14), up to five neighbour ids (int[5] at +0x1C), a
 * direction bitmask (+0x1B) splitting neighbours into forward/backward
 * links, and three path-tag bytes (+0x31..+0x33) used to keep a walk on
 * the path picked by `tag`.
 *
 * pathcam_findTaggedNodeWindow resolves the four-node window (prev,
 * cur, next, next-next) around a node along the tagged path.
 * pathcam_buildWindowSamples gathers the per-node samples for that
 * window, extrapolating the missing end nodes and unwrapping angle
 * deltas that cross the +/- bound. pathcam_segmentParam returns the normalised
 * position of (px,pz) along the segment between the two midpoint planes
 * of the window.
 */
#include "main/dll/CAM/camshipbattle5C.h"
#include "main/dll/rom_curve_interface.h"
#include "main/debug.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_float_helpers.h"

typedef RomCurvePathNode RomCurveNode;

/* curve-node field offsets (raw walking-pointer accesses below) */
#define NODE_SELF_ID    0x14
#define NODE_DIR_MASK   0x1B
#define NODE_NEIGHBOURS 0x1C
#define NODE_TAG0       0x31
#define NODE_TAG1       0x32
#define NODE_TAG2       0x33

extern const f32 lbl_803E1890; /* angle delta upper bound */
extern const f32 lbl_803E1894; /* angle delta lower bound */
extern const f32 lbl_803E1898; /* angle unwrap step */
extern const f32 lbl_803E1888; /* angle near/zero threshold */
extern char sPathCamNeedTwoControlPointsError[];

extern f32 lbl_803E18A8; /* midpoint factor (segment normal averaging) */

#define PATHCAM_NEAR_THRESHOLD lbl_803E1888
#define PATHCAM_FAR_THRESHOLD  lbl_803E188C

extern const f32 lbl_803E188C; /* far distance threshold */

void fn_8010A104(int* nodeId, int* leadNodeId, f32 x, f32 y, f32 z, int tag)
{
    int node;
    int linked;
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
            linked = (int)(*gRomCurveInterface)->getById(((RomCurvePathNode*)node)->links[slot]);
            if ((u32)linked != 0 && (((RomCurvePathNode*)linked)->tag0 == tag || ((RomCurvePathNode*)linked)->tag1 == tag ||
                                     ((RomCurvePathNode*)linked)->tag2 == tag))
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
                linked = (int)(*gRomCurveInterface)->getById(((RomCurvePathNode*)node)->links[slot]);
                if ((u32)linked != 0 &&
                    (((RomCurvePathNode*)linked)->tag0 == tag || ((RomCurvePathNode*)linked)->tag1 == tag ||
                     ((RomCurvePathNode*)linked)->tag2 == tag))
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
    fn_8010A47C(node, &span, tag);
    node = (int)(*gRomCurveInterface)->getById(*leadNodeId);
    *leadNodeId = ((RomCurvePathNode*)fn_8010A47C(node, &farSpan, tag))->selfId;
    for (step = 0; step < span; step++)
    {
        node = (int)(*gRomCurveInterface)->getById(*leadNodeId);
        for (slot2 = 0; slot2 < ROM_CURVE_PATH_LINK_COUNT; slot2++)
        {
            if (((RomCurvePathNode*)node)->links[slot2] > -1 &&
                (((RomCurvePathNode*)node)->directionMask & (1 << slot2)) == 0)
            {
                linked = (int)(*gRomCurveInterface)->getById(((RomCurvePathNode*)node)->links[slot2]);
                if ((u32)linked != 0 &&
                    (((RomCurvePathNode*)linked)->tag0 == tag || ((RomCurvePathNode*)linked)->tag1 == tag ||
                     ((RomCurvePathNode*)linked)->tag2 == tag))
                {
                    *leadNodeId = ((RomCurvePathNode*)node)->links[slot2];
                    slot2 = ROM_CURVE_PATH_LINK_COUNT;
                }
            }
        }
    }
}

int fn_8010A47C(int curve, int* count, int tag)
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
                    *w1 = pts[1]->x + (pts[1]->x - pts[2]->x);
                    *w2 = pts[1]->y + (pts[1]->y - pts[2]->y);
                    *w3 = pts[1]->z + (pts[1]->z - pts[2]->z);
                    *w4 = (f32)(pts[1]->sampleA + (pts[1]->sampleA - pts[2]->sampleA));
                    *w5 = (f32)(pts[1]->sampleB + (pts[1]->sampleB - pts[2]->sampleB));
                    *w6 = (f32)(pts[1]->sampleC + (pts[1]->sampleC - pts[2]->sampleC));
                    *w7 = (f32)pts[1]->sampleD + ((f32)pts[1]->sampleD - (f32)pts[2]->sampleD);
                }
                else if (j == 3)
                {
                    *w1 = pts[2]->x + (pts[2]->x - pts[1]->x);
                    *w2 = pts[2]->y + (pts[2]->y - pts[1]->y);
                    *w3 = pts[2]->z + (pts[2]->z - pts[1]->z);
                    *w4 = (f32)(pts[2]->sampleA + (pts[2]->sampleA - pts[1]->sampleA));
                    *w5 = (f32)(pts[2]->sampleB + (pts[2]->sampleB - pts[1]->sampleB));
                    *w6 = (f32)(pts[2]->sampleC + (pts[2]->sampleC - pts[1]->sampleC));
                    *w7 = (f32)pts[2]->sampleD + ((f32)pts[2]->sampleD - (f32)pts[1]->sampleD);
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
                upper = lbl_803E1890;
                for (step = 0; step < 3; step++)
                {
                    v0 = wp[0];
                    v1 = wp[1];
                    d = v0 - v1;
                    if (d > upper || d < lbl_803E1894)
                    {
                        if (v0 < lbl_803E1888)
                        {
                            wp[0] = wp[0] + lbl_803E1898;
                        }
                        else if (v1 < lbl_803E1888)
                        {
                            wp[1] = wp[1] + lbl_803E1898;
                        }
                    }
                    wp++;
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
        sz = pts[1]->z - pts[0]->z;
        sx = pts[1]->x - pts[0]->x;
    }
    else
    {
        sx = dx1;
        sz = dz1;
    }
    nx = lbl_803E18A8 * (sx + dx1);
    nz = lbl_803E18A8 * (sz + dz1);
    len = sqrtf(nx * nx + nz * nz);
    if (0.0f != len)
    {
        nx = nx / len;
        nz = nz / len;
    }
    p1z = pts[1]->z;
    p1x = pts[1]->x;
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
    nx = lbl_803E18A8 * (nsx + sx);
    nz = lbl_803E18A8 * (nsz + sz);
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

char sPathCamNeedTwoControlPointsError[] = "PATHCAM error: need at least two control points\n";
