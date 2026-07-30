/*
 * DFropenode (DLL 0x175) implements the Dragon Rock rope/cradle system.
 * It owns the rope mesh builder, spring simulation, construction helpers,
 * object callbacks, and rendering code.
 */
#include "dlls/objects/373_DFropenode.h"

#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "dolphin/mtx.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/camera.h"
#include "main/dll/DF/DFbarrel.h"
#include "main/frame_timing.h"
#include "main/gamebits.h"
#include "main/lightmap_api.h"
#include "main/mm.h"
#include "main/objtype.h"
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

/*
 * Build the six-vertex mesh for one rope segment. The template is rotated
 * around the Y axis and its two end caps are translated onto the link nodes.
 */
void DFropenode_buildRopeSegmentMesh(void* templateData, int angle, float* startNode, float* endNode,
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
void DFropenode_integrateRopeNodes(DFRope* self) {
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
void DFropenode_updateRopeSimulation(DFRope* self) {
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
        DFropenode_integrateRopeNodes(self);
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

void DFropenode_attachRopeLink(DFRopeLink* linkSelf, DFRopeNode* firstNode, DFRopeNode* secondNode) {
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
DFRope* DFropenode_createRope(f32 startX, f32 startY, f32 startZ, f32 endX, f32 endY, f32 endZ, f32 unused, s32 count,
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
        DFropenode_attachRopeLink(link, linkNode, nextNode);
        link++;
        linkNode++;
    }
    return rope;
}

void DFropenode_setMinY(GameObject* obj, float value) {
    ((DFropenodeState*)obj->extra)->minimumY = value;
}

int DFropenode_isVisible(GameObject* obj) {
    DFropenodeState* state = obj->extra;

    return (s16)(state->hidden == 0);
}

void DFropenode_setVisible(GameObject* obj, int value) {
    u32 bit;
    u8 bitByte;
    DFropenodeState* state;
    GameObject* linkedNode;

    state = obj->extra;
    bit = (value == 0);
    bitByte = bit;
    state->hidden = bitByte;
    linkedNode = state->linkedNode;
    if (linkedNode != NULL) {
        state = linkedNode->extra;
        state->hidden = bitByte;
    }
}

int DFropenode_getAngle(GameObject* obj) {
    return ((DFropenodeState*)obj->extra)->ropeYaw;
}

void DFropenode_clearLinkedObj(GameObject* obj) {
    ((DFropenodeState*)obj->extra)->linkedNode = NULL;
}

f32 DFropenode_projectPointOntoSegment(f32* x, f32* y, f32* z, f32 startX, f32 startY, f32 startZ, f32 endX, f32 endY,
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

int DFropenode_findNearestRopePoint(GameObject* obj, f32 worldX, f32 worldY, f32 worldZ, float* distanceOut,
                                    float* phaseOut, u8* sideOut) {
    int i;
    DFropenodeState* state;
    f32 x;
    f32 y;
    f32 z;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 distance;
    int result;

    state = obj->extra;
    if ((((DFropenodePlacement*)obj->anim.placementData)->roleFlags & DFROPENODE_ROLE_ROPE_OWNER) == 0) {
        return 0;
    }
    if (state->linkedNode == NULL) {
        return 0;
    }
    if (worldX < state->boundsMinX || worldX > state->boundsMaxX || worldZ < state->boundsMinZ ||
        worldZ > state->boundsMaxZ) {
        return 0;
    }
    *distanceOut = 10000.0f;
    worldX -= obj->anim.localPosX;
    worldY -= obj->anim.localPosY;
    worldZ -= obj->anim.localPosZ;
    {
        i = 0;
        result = 0;
        for (; i < state->rope->count - 1; i++) {
            DFRopeNode* node;
            f32 phase;

            x = worldX;
            y = worldY;
            z = worldZ;
            node = &state->rope->nodes[i];
            phase = DFropenode_projectPointOntoSegment(&x, &y, &z, node->pos[0], node->pos[1], node->pos[2],
                                                       node[1].pos[0], node[1].pos[1], node[1].pos[2]);
            if (phase >= 0.0f && phase < 1.0f) {
                dx = x - worldX;
                dy = y - worldY;
                dz = z - worldZ;
                distance = sqrtf(dx * dx + dy * dy + dz * dz);
                if (distance < *distanceOut) {
                    result = i + 1;
                    *distanceOut = distance;
                    *phaseOut = (f32)i + phase;
                }
            }
        }
    }
    if (result != 0) {
        if (result - 1 <= ((int)state->rope->count >> 1)) {
            *sideOut = 0;
        } else {
            *sideOut = 1;
        }
    }
    return result;
}

void DFropenode_applyForceAtPhase(f32 phase, f32 force, GameObject* obj) {
    DFropenodeState* extra;
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

void DFropenode_advancePhaseByDistance(GameObject* obj, float* phase, f32 distance) {
    DFropenodeState* extra;
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

void DFropenode_getWorldPosAtPhase(f32 phase, GameObject* obj, float* xOut, float* yOut, float* zOut) {
    DFropenodeState* extra;
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

void DFropenode_getPlaneEquation(GameObject* obj, f32* out) {
    DFropenodeState* p = obj->extra;
    out[0] = p->planeNormalX;
    out[1] = p->planeNormalY;
    out[2] = p->planeNormalZ;
    out[3] = p->planeDistance;
}

int DFropenode_syncRopeToEndpoints(GameObject* obj) {
    DFropenodeState* extra;
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
        endObj = extra->linkedNode;
    } else {
        endObj = baseObj;
        baseObj = ((DFropenodeState*)baseObj->extra)->linkedNode;
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
    extra->ropeYaw = angle;

    length = sqrtf(dx * dx + dy * dy + dz * dz);
    length = length / (f32)(extra->rope->count - 1);
    link = extra->rope->links;
    extra->rope->damping = 0.1f;
    for (i = 0; i < extra->rope->count - 1; i++, link++) {
        link->restLength = length;
    }

    i = extra->rope->count - 1;
    extra->rope->nodes[i].pos[0] = dx;
    extra->rope->nodes[i].pos[1] = dy;
    extra->rope->nodes[i].pos[2] = dz;

    extra->boundsMinX = baseObj->anim.localPosX;
    extra->boundsMinZ = baseObj->anim.localPosZ;
    extra->boundsMaxX = endObj->anim.localPosX;
    extra->boundsMaxZ = endObj->anim.localPosZ;
    if (extra->boundsMinX > extra->boundsMaxX) {
        temp = extra->boundsMinX;
        extra->boundsMinX = extra->boundsMaxX;
        extra->boundsMaxX = temp;
    }
    if (extra->boundsMinZ > extra->boundsMaxZ) {
        temp = extra->boundsMinZ;
        extra->boundsMinZ = extra->boundsMaxZ;
        extra->boundsMaxZ = temp;
    }

    clampY = extra->minimumY;
    if (clampY) {
        clampY -= baseObj->anim.localPosY;
        for (i = 0; i < extra->rope->count - 1; i++) {
            if (extra->rope->nodes[i].pos[1] < clampY) {
                extra->rope->nodes[i].pos[1] = clampY;
            }
        }
    }

    extra->boundsMinX = extra->boundsMinX - (margin = 25.0f);
    extra->boundsMinZ -= margin;
    extra->boundsMaxX += margin;
    extra->boundsMaxZ += margin;
    return 0;
}

int DFropenode_getExtraSize(void) {
    return 0x34;
}

int DFropenode_getObjectTypeId(void) {
    return 0;
}

void DFropenode_free(GameObject* obj) {
    void* node;
    int** objs;
    int count;
    int i;

    node = obj->extra;
    objFreeObjectType((u32)obj, DFROPENODE_OBJGROUP);
    if (((DFropenodeState*)node)->rope != NULL && ((DFropenodeState*)node)->rope != NULL) {
        mm_free(((DFropenodeState*)node)->rope);
    }
    node = ((DFropenodeState*)node)->linkedNode;
    if (node != NULL) {
        objs = (int**)objGetAllOfType(DFROPENODE_OBJGROUP, &count);
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

void DFropenode_render(GameObject* obj, int p2, int p3) {
    ObjAnimComponent* objAnim;
    DFropenodeState* extra;
    DFropenodePlacement* objDef;
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
    objDef = (DFropenodePlacement*)objAnim->placementData;
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

    if (((objDef->roleFlags & DFROPENODE_ROLE_ROPE_OWNER) != 0) && (extra->linkedNode != NULL) &&
        (extra->rope != NULL)) {
        originalScale = (obj)->anim.rootMotionScale;
        (obj)->anim.rootMotionScale = 0.01f;
        Camera_LoadModelViewMatrix(0, p3, (MatrixTransform*)obj, 1.0f, 0.0f, NULL);
        (obj)->anim.rootMotionScale = originalScale;
        gxTevResetStages();
        gxTevTextureTimesColor1Stage();
        gxTevCommitStages();
        if (objDef->variant == 1) {
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
            selectTexture((Texture*)(gRopeNodeTextures[objDef->variant]), 0);
            setTextColor(&p2, renderState.blue, renderState.green, renderState.red, (u8)alpha);
        }
        node = extra->rope->nodes;
        for (segment = 0; segment < (int)(extra->rope->count - 1); segment++) {
            node++;
            DFropenode_buildRopeSegmentMesh((u8*)gRopeNodeSegmentDataA, extra->ropeYaw, (node - 1)->pos, node->pos,
                                            segmentVerts);
            lightmapDrawTriangleList(segmentVerts, (u8*)gRopeNodeDisplayList, 6);
        }
        if (objDef->variant == 1) {
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
                DFropenode_buildRopeSegmentMesh((u8*)gRopeNodeSegmentDataB, extra->ropeYaw, (node - 1)->pos, node->pos,
                                                segmentVerts);
                lightmapDrawTriangleList(segmentVerts, (u8*)gRopeNodeDisplayList, 6);
            }
        }
    }
}

void DFropenode_hitDetect(void) {
}

void DFropenode_update(GameObject* obj) {
    DFropenodeState* extra;
    DFropenodePlacement* placement;
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

    placement = (DFropenodePlacement*)obj->anim.placementData;
    extra = obj->extra;
    if ((placement->roleFlags & DFROPENODE_ROLE_ROPE_OWNER) == 0) {
        return;
    }

    linkedObj = extra->linkedNode;
    if (linkedObj == NULL) {
        objects = (GameObject**)ObjList_GetObjects(&objectIndex, &objectCount);
        objectIndex = 0;
        while ((objectIndex < objectCount) && (linkedObj == NULL)) {
            candidateObj = *objects;
            if ((candidateObj->anim.classId == 0x36) &&
                ((s32)placement->roleFlags ==
                 ((DFropenodePlacement*)candidateObj->anim.placementData)->roleFlags - 1)) {
                linkedObj = candidateObj;
            }
            objects++;
            objectIndex++;
        }
        if (linkedObj == NULL) {
            return;
        }

        ((DFropenodeState*)linkedObj->extra)->linkedNode = obj;
        extra = obj->extra;
        extra->linkedNode = linkedObj;

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
        extra->ropeYaw = angle;

        extra->rope = DFropenode_createRope(0.0f, 0.0f, 0.0f, dx, dy, dz, length, 0x10,
                                            (gRopeNodeTickScales)[placement->variant]);

        extra->boundsMinX = obj->anim.localPosX;
        extra->boundsMinZ = obj->anim.localPosZ;
        extra->boundsMaxX = linkedObj->anim.localPosX;
        extra->boundsMaxZ = linkedObj->anim.localPosZ;
        if (extra->boundsMinX > extra->boundsMaxX) {
            temp = extra->boundsMinX;
            extra->boundsMinX = extra->boundsMaxX;
            extra->boundsMaxX = temp;
        }
        if (extra->boundsMinZ > extra->boundsMaxZ) {
            temp = extra->boundsMinZ;
            extra->boundsMinZ = extra->boundsMaxZ;
            extra->boundsMaxZ = temp;
        }
        {
            extra->boundsMinX -= 25.0f;
            extra->boundsMinZ -= 25.0f;
            extra->boundsMaxX += 25.0f;
            extra->boundsMaxZ += 25.0f;
        }

        baseX = obj->anim.localPosX;
        baseY = obj->anim.localPosY;
        baseZ = obj->anim.localPosZ;
        linkedX = linkedObj->anim.localPosX;
        linkedY = linkedObj->anim.localPosY;
        linkedZ = linkedObj->anim.localPosZ;
        liftedY = 20.0f + baseY;

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

    DFropenode_updateRopeSimulation(extra->rope);
}

void DFropenode_init(GameObject* obj, DFropenodePlacement* placement) {

    DFropenodeState* extra;

    extra = obj->extra;
    if ((gRopeNodeVariantVisibleFlags)[placement->variant] == 0) {
        (obj)->anim.flags = (obj)->anim.flags & ~0x80;
    }
    objAddObjectType((int)obj, DFROPENODE_OBJGROUP);
    (obj)->animEventCallback = DFropenode_syncRopeToEndpoints;
    extra->rope = NULL;
    extra->linkedNode = NULL;
    (obj)->anim.alpha = 0x46;
}

void DFropenode_release(void) {
    int i;

    for (i = 0; i < 2; i++) {
        textureFree((Texture*)(gRopeNodeTextures[i]));
    }
}

void DFropenode_initialise(void) {
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
    (ObjectDescriptorCallback)DFropenode_initialise,
    (ObjectDescriptorCallback)DFropenode_release,
    0,
    (ObjectDescriptorCallback)DFropenode_init,
    (ObjectDescriptorCallback)DFropenode_update,
    (ObjectDescriptorCallback)DFropenode_hitDetect,
    (ObjectDescriptorCallback)DFropenode_render,
    (ObjectDescriptorCallback)DFropenode_free,
    (ObjectDescriptorCallback)DFropenode_getObjectTypeId,
    DFropenode_getExtraSize,
    (ObjectDescriptorCallback)DFropenode_getPlaneEquation,
    (ObjectDescriptorCallback)DFropenode_getWorldPosAtPhase,
    (ObjectDescriptorCallback)DFropenode_advancePhaseByDistance,
    (ObjectDescriptorCallback)DFropenode_applyForceAtPhase,
    (ObjectDescriptorCallback)DFropenode_findNearestRopePoint,
    (ObjectDescriptorCallback)DFropenode_getAngle,
    (ObjectDescriptorCallback)DFropenode_setVisible,
    (ObjectDescriptorCallback)DFropenode_isVisible,
    (ObjectDescriptorCallback)DFropenode_setMinY,
    (ObjectDescriptorCallback)DFropenode_clearLinkedObj,
};
