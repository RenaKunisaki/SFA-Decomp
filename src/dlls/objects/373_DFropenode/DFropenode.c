/*
 * DFropenode (DLL 0x175) implements the Dragon Rock rope/cradle system.
 * It owns the rope mesh builder, spring simulation, construction helpers,
 * object callbacks, and rendering code.
 */
#include "dlls/object_descriptor.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "dolphin/mtx.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/camera.h"
#include "main/dll/DF/dfropenode.h"
#include "main/frame_timing.h"
#include "main/gamebits.h"
#include "main/lightmap_api.h"
#include "main/mm.h"
#include "main/obj_group.h"
#include "main/obj_list.h"
#include "main/sky.h"
#include "main/texture.h"
#include "string.h"
#include "track/intersect_api.h"
#include "game/objects/object_setup.h"
#include "dolphin/mtx/vec.h"

#define DFBARREL_SWAY_LIMIT          0x32
#define DFBARREL_SWAY_DIR_INCREASING 1
#define DFBARREL_SWAY_DIR_DECREASING 2

#define DFBARREL_NODE_LINKS_OFFSET 0x28
#define DFROPENODE_OBJGROUP        0x17

extern f32 gRopeNodeDamping;
extern const f32 gRopeNodeBoundsMargin;
extern f32 gRopeNodeLiftHeight;
extern f32 gRopeNodeMaxDistance;
extern const f32 lbl_803E4DFC;

int gRopeNodeTextureAssetIds[2] = {0x3CA, 0x5DD};
void* gRopeNodeTextures[2] = {0};
f32 gRopeNodeTickScales[2] = {0.1f, 0.13f};
u8 gRopeNodeVariantVisibleFlags[8] = {0, 1, 0, 0, 0, 0, 0, 0};

u32 gRopeNodeSegmentDataA[24] = {
    0x00000064, 0x00000000, 0x01000000, 0xffffffff, 0xff38ff9c, 0x00000000, 0x00000000, 0xffffffff,
    0x00c8ff9c, 0x00000000, 0x02000000, 0xffffffff, 0x00000001, 0x00000000, 0x01000200, 0xffffffff,
    0xff38ff9c, 0x00000000, 0x00000200, 0xffffffff, 0x00c8ff9c, 0x00000000, 0x02000200, 0xffffffff,
};
u32 gRopeNodeSegmentDataB[24] = {
    0x000000c8, 0x00000000, 0x00800000, 0xffffff80, 0xfe70ff38, 0x00000000, 0x00000000, 0xffffff80,
    0x0190ff38, 0x00000000, 0x01000000, 0xffffff80, 0x000000c8, 0x00000000, 0x00800100, 0xffffff80,
    0xfe70ff38, 0x00000000, 0x00000100, 0xffffff80, 0x0190ff38, 0x00000000, 0x01000100, 0xffffff80,
};
const u8 gRopeNodeDisplayList[96] = {0, 0, 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 4, 3, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 2, 3, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 4, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 5, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

typedef struct DfropenodePlacement {
    ObjPlacement base;
    u8 flags18; /* bit0 enables rope-render pass */
    u8 pad19[0x1B - 0x19];
    u8 textureIndex; /* gRopeNodeTextures index; 1 = white/active style */
    s16 fadeGameBit; /* game bit gating the node fade-out */
    u8 pad1E[0x20 - 0x1E];
} DfropenodePlacement;

/*
 * Build the six-vertex mesh for one rope segment. The template is rotated
 * around the Y axis and its two end caps are translated onto the link nodes.
 */
void dfropenode_buildRopeSegmentMesh(void* templateData, int angle, float* startNode, float* endNode,
                                     LightmapVertex* out) {
    s16 startX;
    s16 startY;
    s16 startZ;
    s16 endX;
    s16 endY;
    s16 endZ;
    LightmapVertex* vertex;
    int i;
    float angleRadians;
    f32 vertexX;

    startX = 100.0f * startNode[0];
    startY = 100.0f * startNode[1];
    startZ = 100.0f * startNode[2];
    endX = 100.0f * endNode[0];
    endY = 100.0f * endNode[1];
    endZ = 100.0f * endNode[2];
    memcpy(out, templateData, 0x60);

    i = 0;
    vertex = out;
    angleRadians = (3.1415927f * (float)(short)angle) / 32768.0f;
    for (; i < 6; i++) {
        vertexX = (float)(int)vertex->x;
        vertex->x = vertexX * mathCosf(angleRadians);
        vertex->z = -vertexX * mathSinf(angleRadians);
        vertex++;
    }

    out[0].x += startX;
    out[0].y += startY;
    out[0].z += startZ;
    out[3].x += endX;
    out[3].y += endY;
    out[3].z += endZ;
    out[1].x += startX;
    out[1].y += startY;
    out[1].z += startZ;
    out[4].x += endX;
    out[4].y += endY;
    out[4].z += endZ;
    out[2].x += startX;
    out[2].y += startY;
    out[2].z += startZ;
    out[5].x += endX;
    out[5].y += endY;
    out[5].z += endZ;
    return;
}

/*
 * Integrate the spring forces attached to every unlocked rope node.
 */
void DFPulley_integrateLinks(DFRope* self) {
    DFRopeNode* part;
    int j;
    int i;
    Vec accel;
    Vec velscaled;
    Vec scaled;
    f32 mag;
    f32 zero;

    part = self->nodes;
    i = 0;
    zero = 0.0f;
    for (; i < self->count; i++, part++) {
        accel.z = zero;
        accel.y = zero;
        accel.x = zero;

        if (part->locked == 0) {
            for (j = 0; j < part->linkCount; j++) {
                DFRopeLink* link = part->links[j];
                if (part == link->a) {
                    PSVECAdd(&accel, (Vec*)link->force, &accel);
                } else {
                    PSVECSubtract(&accel, (Vec*)link->force, &accel);
                }
            }
            mag = PSVECMag(&accel);
            if (mag > self->maxSlack) {
                PSVECScale(&accel, &accel, self->maxSlack / mag);
            }
            PSVECScale(&accel, &accel, self->stepPerTick);
            PSVECAdd(&accel, (Vec*)part->force, &accel);
            PSVECAdd((Vec*)part->velocity, &accel, (Vec*)part->velocity);
            PSVECScale((Vec*)part->velocity, &velscaled, self->damping);
            PSVECSubtract((Vec*)part->velocity, &velscaled, (Vec*)part->velocity);
            part->velocity[1] = self->step * self->inverseTicks + part->velocity[1];
            PSVECScale((Vec*)part->velocity, &scaled, self->step);
            PSVECAdd((Vec*)part->pos, &scaled, (Vec*)part->pos);
        }
    }
}

/*
 * Apply rope sway, solve each spring link, integrate the nodes, and clear
 * accumulated forces for the next tick.
 */
void DFRope_UpdateSimulation(DFRope* self) {
    int j;
    DFRopeLink* link;
    int k;
    DFRopeNode* parts;
    int i;
    DFRopeNode* partIter;
    Vec tmp;
    f32 zero;
    DFRopeNode* partsInit;

    partsInit = self->nodes;
    parts = partsInit;

    if (self->sway < -DFBARREL_SWAY_LIMIT) {
        self->direction = DFBARREL_SWAY_DIR_INCREASING;
    }
    if (self->sway > DFBARREL_SWAY_LIMIT) {
        self->direction = DFBARREL_SWAY_DIR_DECREASING;
    }
    if ((s8)self->direction == DFBARREL_SWAY_DIR_DECREASING) {
        self->sway--;
    } else {
        self->sway++;
    }

    i = 1;
    partIter = partsInit + 1;
    {
        f32 rate = 0.01f;
        for (; i < self->count - 1; i++) {
            partIter->force[0] = partIter->force[0] + rate * (f32)(int)self->sway;
            partIter++;
        }
    }

    k = 0;
    zero = 0.0f;
    for (; k < self->enabled; k++) {
        link = self->links;
        for (j = 0; j < self->count - 1; j++, link++) {
            PSVECSubtract((Vec*)link->a, (Vec*)link->b, &tmp);
            link->length = PSVECMag(&tmp);
            if (link->length > link->maxLength) {
                link->restLength = 0.0f;
            }
            if (zero == link->restLength) {
                link->force[2] = zero;
                link->force[1] = zero;
                link->force[0] = zero;
            } else {
                PSVECScale(&tmp, (Vec*)link->force, -link->stiffness * (link->length - link->restLength));
            }
        }
        DFPulley_integrateLinks(self);
    }

    i = 0;
    {
        f32 cleanZero = 0.0f;
        for (; i < self->count; i++, parts++) {
            parts->force[0] = cleanZero;
            parts->force[1] = cleanZero;
            parts->force[2] = cleanZero;
        }
    }
}

void DFRopeLink_AttachNodes(DFRopeLink* linkSelf, DFRopeNode* firstNode, DFRopeNode* secondNode) {
    u8* nodeLinkIter;
    int firstLinkIndex;
    int secondLinkIndex;

    firstLinkIndex = 0;
    secondLinkIndex = 0;
    nodeLinkIter = (u8*)firstNode;
    while (*(u32*)(nodeLinkIter + DFBARREL_NODE_LINKS_OFFSET) != 0) {
        nodeLinkIter += 4;
        firstLinkIndex++;
    }
    nodeLinkIter = (u8*)secondNode;
    while (*(u32*)(nodeLinkIter + DFBARREL_NODE_LINKS_OFFSET) != 0) {
        nodeLinkIter += 4;
        secondLinkIndex++;
    }
    if (firstLinkIndex > firstNode->linkCount || secondLinkIndex > secondNode->linkCount) {
        return;
    }
    firstNode->links[firstLinkIndex] = linkSelf;
    secondNode->links[secondLinkIndex] = linkSelf;
    linkSelf->a = firstNode;
    linkSelf->b = secondNode;
}

/*
 * Allocate a rope, seed evenly-spaced nodes between its endpoints, pin the
 * ends, and attach each spring link to its node pair.
 */
DFRope* DFRope_Create(f32 startX, f32 startY, f32 startZ, f32 endX, f32 endY, f32 endZ, f32 unused, s32 count,
                      f32 tickScale) {
    DFRope* rope;
    DFRopeNode* nodes;
    DFRopeNode* node;
    s32 linkCount;
    DFRopeLink* link;
    DFRopeNode* nextNode;
    DFRopeNode* linkNode;
    s32 nodesSize;
    s32 allocSize;
    u8* base;
    s32 i;
    s32 linkIndex;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 length;

    dx = endX - startX;
    dy = endY - startY;
    dz = endZ - startZ;
    length = sqrtf(dz * dz + (dx * dx + dy * dy));

    dx = dx / (f32)(count - 1);
    dy = dy / (f32)(count - 1);
    dz = dz / (f32)(count - 1);

    nodesSize = count * sizeof(DFRopeNode);
    allocSize = sizeof(DFRope) + nodesSize + (count - 1) * sizeof(DFRopeLink);
    base = (u8*)mmAlloc(allocSize, 0xFF, 0);
    rope = (DFRope*)base;
    rope->nodes = (DFRopeNode*)(base + sizeof(DFRope));
    rope->links = (DFRopeLink*)(base + nodesSize + sizeof(DFRope));
    rope->count = count;
    rope->totalLength = length;
    rope->start[0] = startX;
    rope->start[1] = startY;
    rope->start[2] = startZ;
    rope->end[0] = endX;
    rope->end[1] = endY;
    rope->end[2] = endZ;
    rope->sway = 0;
    rope->direction = 1;
    rope->damping = 0.025f;
    rope->enabled = 1;
    rope->step = 0.01f;
    if (rope->step * length > 5.0f) {
        rope->step = 5.0f / length;
    }
    rope->maxSlack = 5000.0f;
    rope->stepPerTick = rope->step / tickScale;
    rope->inverseTicks = -9.81f / tickScale;

    nodes = rope->nodes;
    for (i = 0, node = nodes; i < count; node++, i++) {
        node->pos[0] = i * dx + rope->start[0];
        node->pos[1] = i * dy + rope->start[1];
        node->pos[2] = i * dz + rope->start[2];
        node->velocity[2] = 0.0f;
        node->velocity[1] = 0.0f;
        node->velocity[0] = 0.0f;
        node->force[2] = 0.0f;
        node->force[1] = 0.0f;
        node->force[0] = 0.0f;
        node->locked = 0;
        if ((i == 0) || (i == count - 1)) {
            node->linkCount = 1;
        } else if ((i == 1) || (i == count - 2)) {
            node->linkCount = 2;
        } else {
            node->linkCount = 2;
        }
        {
            s32 j;
            for (j = 0; j < node->linkCount; j++) {
                node->links[j] = NULL;
            }
        }
    }

    nodes[count - 1].locked = 1;
    nodes[0].locked = 1;

    linkIndex = 0;
    link = rope->links;
    linkNode = nodes;
    linkCount = count - 1;
    for (; linkIndex < linkCount; linkIndex++) {
        link->restLength = rope->totalLength / linkCount;
        link->stiffness = 10.0f;
        link->force[2] = 0.0f;
        link->force[1] = 0.0f;
        link->force[0] = 0.0f;
        link->maxLength = 1000.0f * link->restLength;
        nextNode = (DFRopeNode*)((u8*)nodes + (linkIndex + 1) * sizeof(DFRopeNode));
        DFRopeLink_AttachNodes(link, linkNode, nextNode);
        link++;
        linkNode++;
    }
    return rope;
}

void dfropenode_setMinY(GameObject* obj, float value) {
    ((DFropenodeExtra*)obj->extra)->minY = value;
}

int dfropenode_isVisible(GameObject* obj) {
    DFropenodeExtra* extra = ((DFropenodeExtra*)obj->extra);

    return (s16)(extra->hidden == 0);
}

void dfropenode_setVisible(GameObject* obj, int value) {
    u32 bit;
    u8 bitByte;
    DFropenodeExtra* extra;
    void* linkedObj;

    extra = ((DFropenodeExtra*)obj->extra);
    bit = (value == 0);
    bitByte = bit;
    extra->hidden = bitByte;
    linkedObj = extra->linkedObj;
    if (linkedObj != NULL) {
        extra = ((DFropenodeExtra*)((GameObject*)linkedObj)->extra);
        extra->hidden = bitByte;
    }
}

int dfropenode_getAngle(GameObject* obj) {
    return ((DFropenodeExtra*)obj->extra)->angle;
}

void dfropenode_clearLinkedObj(GameObject* obj) {
    ((DFropenodeExtra*)obj->extra)->linkedObj = 0;
}

f32 DFRope_projectPointOntoSegment(f32* x, f32* y, f32* z, f32 startX, f32 startY, f32 startZ, f32 endX, f32 endY,
                                   f32 endZ) {
    f32 dx;
    f32 dy;
    f32 dz;
    f32 t;

    dx = endX - startX;
    dy = endY - startY;
    dz = endZ - startZ;
    if ((0.0f == dx) && (0.0f == dz)) {
        t = 0.0f;
    } else {
        t = (dx * (*x - startX) + dz * (*z - startZ)) / (dx * dx + dz * dz);
    }
    if (t < 0.0f) {
        *x = startX;
        *y = startY;
        *z = startZ;
    } else if (t >= 1.0f) {
        *x = endX;
        *y = endY;
        *z = endZ;
    } else {
        *x = t * dx + startX;
        *y = t * dy + startY;
        *z = t * dz + startZ;
    }
    return t;
}

int dfropenode_findNearestRopePoint(GameObject* obj, f32 worldX, f32 worldY, f32 worldZ, float* distanceOut,
                                    float* phaseOut, u8* sideOut) {
    int offset;
    int i;
    DFropenodeExtra* extra;
    f32 phase;
    f32 localY;
    f32 localX;
    f32 best;
    f32 localZ;
    f32 x;
    f32 y;
    f32 z;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 distance;
    int result;

    extra = obj->extra;
    if ((((DfropenodePlacement*)obj->anim.placementData)->flags18 & 1) == 0) {
        return 0;
    }
    if (extra->linkedObj == NULL) {
        return 0;
    }
    if (worldX < extra->minX || worldX > extra->maxX || worldZ < extra->minZ || worldZ > extra->maxZ) {
        return 0;
    }
    *distanceOut = gRopeNodeMaxDistance;
    localX = worldX - obj->anim.localPosX;
    localY = worldY - obj->anim.localPosY;
    localZ = worldZ - obj->anim.localPosZ;
    {
        i = 0;
        result = 0;
        offset = 0;
        best = lbl_803E4DFC;
        for (; i < extra->rope->count - 1; i++) {
            DFRopeNode* node;

            x = localX;
            y = localY;
            z = localZ;
            node = (DFRopeNode*)((int)extra->rope->nodes + offset);
            phase = DFRope_projectPointOntoSegment(&x, &y, &z, node->pos[0], node->pos[1], node->pos[2], node[1].pos[0],
                                                   node[1].pos[1], node[1].pos[2]);
            if (phase >= best && phase < 1.0f) {
                dx = x - localX;
                dy = y - localY;
                dz = z - localZ;
                distance = sqrtf(dx * dx + dy * dy + dz * dz);
                if (distance < *distanceOut) {
                    result = i + 1;
                    *distanceOut = distance;
                    *phaseOut = (f32)i + phase;
                }
            }
            offset += 0x34;
        }
    }
    if (result != 0) {
        if (result - 1 <= ((int)extra->rope->count >> 1)) {
            *sideOut = 0;
        } else {
            *sideOut = 1;
        }
    }
    return result;
}

void dfropenode_applyForceAtPhase(f32 phase, f32 force, GameObject* obj) {
    DFropenodeExtra* extra;
    s8 idx;
    f32 fraction;
    DFRopeNode* node;

    extra = (obj)->extra;
    phase = phase - (f32)(s8)phase;
    idx = (s8)phase;
    fraction = phase - (f32)idx;
    node = &extra->rope->nodes[idx];
    node->force[1] = force * fraction + node->force[1];
    fraction = 1.0f - fraction;
    node = &extra->rope->nodes[idx];
    node->force[1] = force * fraction + node->force[1];
}

void dfropenode_advancePhaseByDistance(GameObject* obj, float* phase, f32 distance) {
    DFropenodeExtra* extra;
    s32 raw;
    s8 idx;
    int node;
    f32 ph;
    f32 x0;
    f32 dx;
    f32 dz;
    f32 len;

    extra = obj->extra;
    ph = *phase;
    raw = (s32)ph;
    idx = (s8)raw;
    *phase = ph - (f32)idx;
    node = (int)extra->rope->nodes;
    x0 = *((f32*)node + idx * 13);
    node = node + idx * 0x34;
    dx = x0 - *(f32*)(node + 0x34);
    dz = *(f32*)(node + 8) - *(f32*)(node + 0x3c);
    len = sqrtf(dx * dx + dz * dz);
    distance = distance / len;
    *phase = *phase + distance;
    *phase = *phase + (f32)(s8)raw;
}

void dfropenode_getWorldPosAtPhase(f32 phase, GameObject* obj, float* xOut, float* yOut, float* zOut) {
    DFropenodeExtra* extra;
    s8 idx;
    f32 x0;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 fraction;

    extra = (obj)->extra;
    idx = (s8)phase;
    fraction = phase - (f32)idx;
    dy = extra->rope->nodes[idx + 1].pos[1] - extra->rope->nodes[idx].pos[1];
    dz = extra->rope->nodes[idx + 1].pos[2] - extra->rope->nodes[idx].pos[2];
    x0 = extra->rope->nodes[idx].pos[0];
    dx = extra->rope->nodes[idx + 1].pos[0] - x0;
    *xOut = dx * fraction + ((obj)->anim.localPosX + x0);
    *yOut = dy * fraction + ((obj)->anim.localPosY + extra->rope->nodes[idx].pos[1]);
    *zOut = dz * fraction + ((obj)->anim.localPosZ + extra->rope->nodes[idx].pos[2]);
}

void dfropenode_getPlaneEquation(GameObject* obj, f32* out) {
    DFropenodeExtra* p = obj->extra;
    out[0] = p->planeNormalX;
    out[1] = p->planeNormalY;
    out[2] = p->planeNormalZ;
    out[3] = p->planeDistance;
}

int dfropenode_syncRopeToEndpoints(GameObject* obj) {
    DFropenodeExtra* extra;
    GameObject* endObj;
    GameObject* baseObj;
    int i;
    DFRopeLink* link;
    int flag;
    s16 angle;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 length;
    f32 clampY;
    f32 temp;
    f32 margin;

    baseObj = (GameObject*)(int)obj;
    flag = ((u8*)baseObj->anim.placementData)[0x18] & 1;
    if (flag != 0) {
        extra = baseObj->extra;
        endObj = extra->linkedObj;
    } else {
        endObj = baseObj;
        baseObj = ((DFropenodeExtra*)baseObj->extra)->linkedObj;
        if (baseObj == NULL) {
            return 0;
        }
        extra = baseObj->extra;
    }

    if ((extra->rope == NULL) || (endObj == NULL)) {
        return 0;
    }

    dx = endObj->anim.localPosX - baseObj->anim.localPosX;
    dy = endObj->anim.localPosY - baseObj->anim.localPosY;
    dz = endObj->anim.localPosZ - baseObj->anim.localPosZ;

    angle = getAngle(dx, dz);
    if (angle > 0x8000) {
        angle = angle - 0xffff;
    }
    if (angle < -0x8000) {
        angle = angle + 0xffff;
    }
    extra->angle = angle;

    length = sqrtf(dx * dx + dy * dy + dz * dz);
    length = length / (f32)(extra->rope->count - 1);
    link = extra->rope->links;
    extra->rope->damping = gRopeNodeDamping;
    for (i = 0; i < extra->rope->count - 1; i++, link++) {
        link->restLength = length;
    }

    i = extra->rope->count - 1;
    extra->rope->nodes[i].pos[0] = dx;
    extra->rope->nodes[i].pos[1] = dy;
    extra->rope->nodes[i].pos[2] = dz;

    extra->minX = baseObj->anim.localPosX;
    extra->minZ = baseObj->anim.localPosZ;
    extra->maxX = endObj->anim.localPosX;
    extra->maxZ = endObj->anim.localPosZ;
    if (extra->minX > extra->maxX) {
        temp = extra->minX;
        extra->minX = extra->maxX;
        extra->maxX = temp;
    }
    if (extra->minZ > extra->maxZ) {
        temp = extra->minZ;
        extra->minZ = extra->maxZ;
        extra->maxZ = temp;
    }

    if (extra->minY != lbl_803E4DFC) {
        clampY = extra->minY - baseObj->anim.localPosY;
        for (i = 0; i < extra->rope->count - 1; i++) {
            if (extra->rope->nodes[i].pos[1] < clampY) {
                extra->rope->nodes[i].pos[1] = clampY;
            }
        }
    }

    extra->minX = extra->minX - (margin = gRopeNodeBoundsMargin);
    extra->minZ -= margin;
    extra->maxX += margin;
    extra->maxZ += margin;
    return 0;
}

int dfropenode_getExtraSize(void) {
    return 0x34;
}

int dfropenode_getObjectTypeId(void) {
    return 0;
}

void dfropenode_free(GameObject* obj) {
    void* node;
    int** objs;
    int count;
    int i;

    node = obj->extra;
    ObjGroup_RemoveObject((u32)obj, DFROPENODE_OBJGROUP);
    if (((DFropenodeExtra*)node)->rope != NULL && ((DFropenodeExtra*)node)->rope != NULL) {
        mm_free(((DFropenodeExtra*)node)->rope);
    }
    node = ((DFropenodeExtra*)node)->linkedObj;
    if (node != NULL) {
        objs = (int**)ObjGroup_GetObjects(DFROPENODE_OBJGROUP, &count);
        for (i = 0; i < count; i++) {
            if ((void*)objs[i] == node) {
                (*(void (***)(void*)) * (void**)((char*)node + 0x68))[17](node);
            }
        }
    }
}

typedef struct DfropenodeRenderState {
    u8 red;
    u8 green;
    u8 blue;
} DfropenodeRenderState;

void dfropenode_render(GameObject* obj, int p2, int p3) {
    ObjAnimComponent* objAnim;
    DFropenodeExtra* extra;
    DfropenodePlacement* objDef;
    int eventId;
    int fadeAlpha;
    u32 oldAlpha;
    DFRopeNode* node;
    s16 segment;
    DfropenodeRenderState renderState;
    LightmapVertex segmentVerts[6];
    f32 originalScale;

    objAnim = &(obj)->anim;
    extra = (obj)->extra;
    objDef = (DfropenodePlacement*)objAnim->placementData;
    eventId = objDef->fadeGameBit;
    if ((eventId != 0) && (mainGetBit(eventId) != 0)) {
        oldAlpha = objAnim->alpha;
        if (oldAlpha == 0x46) {
            Sfx_PlayFromObject((int)obj, SFXTRIG_ocean_beamlp);
        }
        fadeAlpha = oldAlpha - framesThisStep;
        if (fadeAlpha <= 0) {
            objAnim->alpha = 0;
            return;
        }
        objAnim->alpha = (u8)fadeAlpha;
    } else {
        if (objAnim->alpha == 0) {
            Sfx_PlayFromObject((int)obj, SFXTRIG_tile_buzzlp);
        }
        if (objAnim->alpha < 0x46) {
            objAnim->alpha += framesThisStep;
        } else {
            objAnim->alpha = 0x46;
        }
    }

    if (((objDef->flags18 & 1) != 0) && (extra->linkedObj != NULL) && (extra->rope != NULL)) {
        originalScale = (obj)->anim.rootMotionScale;
        (obj)->anim.rootMotionScale = 0.01f;
        Camera_LoadModelViewMatrix(0, p3, (MatrixTransform*)obj, 1.0f, 0.0f, NULL);
        (obj)->anim.rootMotionScale = originalScale;
        gxTevResetStages();
        gxTevTextureTimesColor1Stage();
        gxTevCommitStages();
        if (objDef->textureIndex == 1) {
            renderState.red = 0xff;
            renderState.green = 0xff;
            renderState.blue = 0xff;
        } else {
            objAnim->alpha = 0xff;
            getAmbientColor(0, &renderState.blue, &renderState.green, &renderState.red);
            renderState.green = (u8)(renderState.green * 200 >> 8);
            renderState.red = (u8)(renderState.red * 0xaa >> 8);
        }
        {
            int alpha;

            if (objAnim->alpha > 0x46) {
                gxSetOpaqueZWriteMode();
                alpha = 0xff;
            } else {
                gxSetAlphaBlendZTest();
                alpha = (objAnim->alpha + objAnim->alpha) >> 1;
            }
            selectTexture((Texture*)(gRopeNodeTextures[objDef->textureIndex]), 0);
            setTextColor(&p2, renderState.blue, renderState.green, renderState.red, (u8)alpha);
        }
        node = extra->rope->nodes;
        for (segment = 0; segment < (int)(extra->rope->count - 1); segment++) {
            node++;
            dfropenode_buildRopeSegmentMesh((u8*)gRopeNodeSegmentDataA, extra->angle, (node - 1)->pos, node->pos,
                                            segmentVerts);
            lightmapDrawTriangleList(segmentVerts, (u8*)gRopeNodeDisplayList, 6);
        }
        if (objDef->textureIndex == 1) {
            Sfx_KeepAliveLoopedObjectSound((int)obj, SFXTRIG_waterblock_wave);
            gxSetAlphaBlendZTest();
            {
                int alpha;

                alpha = (u8)(objAnim->alpha + randomGetRange(0, objAnim->alpha));
                setTextColor(&p2, renderState.blue, renderState.green, renderState.red, alpha);
            }
            node = extra->rope->nodes;
            for (segment = 0; segment < (int)(extra->rope->count - 1); segment++) {
                node++;
                dfropenode_buildRopeSegmentMesh((u8*)gRopeNodeSegmentDataB, extra->angle, (node - 1)->pos, node->pos,
                                                segmentVerts);
                lightmapDrawTriangleList(segmentVerts, (u8*)gRopeNodeDisplayList, 6);
            }
        }
    }
}

void dfropenode_hitDetect(void) {
}

void dfropenode_update(GameObject* obj) {
    DFropenodeExtra* extra;
    u8* objDef;
    GameObject* linkedObj;
    GameObject** objects;
    int objectCount;
    int objectIndex;
    GameObject* candidateObj;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 length;
    s16 angle;
    f32 temp;
    f32 baseX;
    f32 baseY;
    f32 baseZ;
    f32 linkedX;
    f32 linkedY;
    f32 linkedZ;
    f32 liftedY;
    f32 normalX;
    f32 normalY;
    f32 normalZ;
    f32 normalLength;

    objDef = (u8*)obj->anim.placementData;
    extra = obj->extra;
    if ((objDef[0x18] & 1) == 0) {
        return;
    }

    linkedObj = extra->linkedObj;
    if (linkedObj == NULL) {
        objects = (GameObject**)ObjList_GetObjects(&objectIndex, &objectCount);
        objectIndex = 0;
        while ((objectIndex < objectCount) && (linkedObj == NULL)) {
            candidateObj = *objects;
            if ((candidateObj->anim.classId == 0x36) && ((s32)objDef[0x18] == ((u8*)candidateObj->anim.placementData)[0x18] - 1)) {
                linkedObj = candidateObj;
            }
            objects++;
            objectIndex++;
        }
        if (linkedObj == NULL) {
            return;
        }

        ((DFropenodeExtra*)linkedObj->extra)->linkedObj = obj;
        extra = obj->extra;
        extra->linkedObj = linkedObj;

        dx = linkedObj->anim.localPosX - obj->anim.localPosX;
        dy = linkedObj->anim.localPosY - obj->anim.localPosY;
        dz = linkedObj->anim.localPosZ - obj->anim.localPosZ;
        length = sqrtf(dz * dz + (dx * dx + dy * dy));
        angle = getAngle(dx, dz);
        if (angle > 0x8000) {
            angle = (s16)(angle - 0xFFFF);
        }
        if (angle < -0x8000) {
            angle += 0xFFFF;
        }
        extra->angle = angle;

        extra->rope = DFRope_Create(0.0f, 0.0f, 0.0f, dx, dy, dz, length, 0x10,
                                    (gRopeNodeTickScales)[((DfropenodePlacement*)objDef)->textureIndex]);

        extra->minX = obj->anim.localPosX;
        extra->minZ = obj->anim.localPosZ;
        extra->maxX = linkedObj->anim.localPosX;
        extra->maxZ = linkedObj->anim.localPosZ;
        if (extra->minX > extra->maxX) {
            temp = extra->minX;
            extra->minX = extra->maxX;
            extra->maxX = temp;
        }
        if (extra->minZ > extra->maxZ) {
            temp = extra->minZ;
            extra->minZ = extra->maxZ;
            extra->maxZ = temp;
        }
        {
            extra->minX -= gRopeNodeBoundsMargin;
            extra->minZ -= gRopeNodeBoundsMargin;
            extra->maxX += gRopeNodeBoundsMargin;
            extra->maxZ += gRopeNodeBoundsMargin;
        }

        baseX = obj->anim.localPosX;
        baseY = obj->anim.localPosY;
        baseZ = obj->anim.localPosZ;
        linkedX = linkedObj->anim.localPosX;
        linkedY = linkedObj->anim.localPosY;
        linkedZ = linkedObj->anim.localPosZ;
        liftedY = gRopeNodeLiftHeight + baseY;

        normalX = liftedY * (baseZ - linkedZ) + (baseY * (linkedZ - baseZ) + (linkedY * (baseZ - baseZ)));
        normalY = baseZ * (baseX - linkedX) + (baseZ * (linkedX - baseX) + (linkedZ * (baseX - baseX)));
        normalZ = baseX * (baseY - linkedY) + (baseX * (linkedY - liftedY) + (linkedX * (liftedY - baseY)));
        normalLength = sqrtf(normalZ * normalZ + (normalX * normalX + normalY * normalY));
        if (normalLength > 0.0f) {
            normalX /= normalLength;
            normalY /= normalLength;
            normalZ /= normalLength;
        }
        extra->planeNormalX = normalX;
        extra->planeNormalY = normalY;
        extra->planeNormalZ = normalZ;
        extra->planeDistance = -(baseZ * normalZ + (baseX * normalX + baseY * normalY));
    }

    DFRope_UpdateSimulation(extra->rope);
}

void dfropenode_init(GameObject* obj, u8* objDef) {

    DFropenodeExtra* extra;

    extra = obj->extra;
    if ((gRopeNodeVariantVisibleFlags)[((DfropenodePlacement*)objDef)->textureIndex] == 0) {
        (obj)->anim.flags = (obj)->anim.flags & ~0x80;
    }
    ObjGroup_AddObject((int)obj, DFROPENODE_OBJGROUP);
    (obj)->animEventCallback = dfropenode_syncRopeToEndpoints;
    extra->rope = NULL;
    extra->linkedObj = NULL;
    (obj)->anim.alpha = 0x46;
}

void dfropenode_release(void) {
    int i;

    for (i = 0; i < 2; i++) {
        textureFree((Texture*)(gRopeNodeTextures[i]));
    }
}

void dfropenode_initialise(void) {
    int i;

    for (i = 0; i < 2; i++) {
        (gRopeNodeTextures)[i] = textureLoadAsset((gRopeNodeTextureAssetIds)[i]);
    }
}

ObjectDescriptor20 gDFropenodeObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_20_SLOTS,
    (ObjectDescriptorCallback)dfropenode_initialise,
    (ObjectDescriptorCallback)dfropenode_release,
    0,
    (ObjectDescriptorCallback)dfropenode_init,
    (ObjectDescriptorCallback)dfropenode_update,
    (ObjectDescriptorCallback)dfropenode_hitDetect,
    (ObjectDescriptorCallback)dfropenode_render,
    (ObjectDescriptorCallback)dfropenode_free,
    (ObjectDescriptorCallback)dfropenode_getObjectTypeId,
    dfropenode_getExtraSize,
    (ObjectDescriptorCallback)dfropenode_getPlaneEquation,
    (ObjectDescriptorCallback)dfropenode_getWorldPosAtPhase,
    (ObjectDescriptorCallback)dfropenode_advancePhaseByDistance,
    (ObjectDescriptorCallback)dfropenode_applyForceAtPhase,
    (ObjectDescriptorCallback)dfropenode_findNearestRopePoint,
    (ObjectDescriptorCallback)dfropenode_getAngle,
    (ObjectDescriptorCallback)dfropenode_setVisible,
    (ObjectDescriptorCallback)dfropenode_isVisible,
    (ObjectDescriptorCallback)dfropenode_setMinY,
    (ObjectDescriptorCallback)dfropenode_clearLinkedObj,
};
