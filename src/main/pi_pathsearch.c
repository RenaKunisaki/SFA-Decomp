#include "dolphin/PPCArch.h"
#include "dolphin/mtx.h"
#include "main/dll/rom_curve_interface.h"
#include "main/gamebits.h"
#include "main/pi_dolphin.h"
#include "main/mm.h"
#include "main/texture.h"
#include "dolphin/os/OSInterrupt.h"
#include "string.h"
#include "main/pi_dolphin_texture_api.h"
#include "main/track_dolphin_api.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/printf.h"
#include "dolphin/os/OSArena.h"
#include "dolphin/gx/GXLighting.h"
#include "dolphin/gx/GXGeometry.h"
#include "dolphin/gx/GXCpu2Efb.h"
#include "dolphin/gx/GXTev.h"
#include "dolphin/gx/GXTexture.h"
#include "dolphin/gx/GXTransform.h"
#include "main/camera.h"
#include "main/gameloop_api.h"
#include "main/map_load.h"
#include "main/map_texscroll.h"
#include "main/table_file.h"
#include "main/rcp_dolphin.h"
#include "main/sky_api.h"
#include "main/textrender_api.h"
#include "main/vecmath_distance_api.h"
#include "track/intersect_api.h"
#include "track/intersect_depth_read_api.h"
#include "main/objprint_load_api.h"
#include "dolphin/os/OSAlloc.h"
#include "main/objmodel.h"
#include "main/rcp_dolphin_render_api.h"
#include "dolphin/gx/GXBump.h"

extern void* lbl_803DCD10;
extern char* lbl_803DCD08;

int pathSearchNodeMatchesTarget(int* ctx, int* ref)
{
    int* node;
    int target;
    target = ctx[4];
    node = (int*)ref[0];
    switch (((s8*)node)[0x19])
    {
    case 0x24:
    {
        u8 idx = ((u8*)ref)[0xc];
        if ((idx & 0x80) == 0)
        {
            if (((u8*)node)[3] != 0)
            {
                return target == ((u8*)node)[3];
            }
            else
            {
                int* p;
                int* arr;
                int i;
                arr = (int*)*(int*)((char*)ctx[0] + (idx << 4));
                for (i = 0, p = arr; i < 4; i++)
                {
                    if ((u32)node[5] == *(u32*)((char*)p + 0x1c))
                    {
                        return target == ((u8*)arr)[i + 4];
                    }
                    p++;
                }
            }
        }
        return 0;
    }
    default:
        return target == (int)node;
    }
}

void pathSearchHeapSiftDown(u8* arr, int size, int idx)
{
    u16* h = (u16*)arr;
    int half;
    u8* childptr;
    u32 key = *(u32*)((int)arr + idx * 8);
    u16 val = h[idx * 4 + 2];
    int child;
    u8* cp;
    half = size >> 1;
    while (idx <= half)
    {
        child = idx + idx;
        if (child < size)
        {
            cp = arr + child * 8;
            if (*(u32*)cp < *(u32*)(cp + 8))
            {
                child++;
            }
        }
        childptr = arr + child * 8;
        if (key >= *(u32*)childptr)
            break;
        *(u32*)(arr + idx * 8) = *(u32*)childptr;
        *(u16*)(arr + idx * 8 + 4) = *(u16*)(childptr + 4);
        idx = child;
    }
    *(u32*)((int)arr + idx * 8) = key;
    h[idx * 4 + 2] = val;
}




static inline void pathSearchHeapInsert(PathSearch* search, u16 index, u32 pri)
{
    int i;
    u32 key;
    u16 idx16;
    int parent;
    u32* heap;
    u16* hh;
    heap = (u32*)search->heap;
    hh = (u16*)heap;
    hh[++search->heapSize * 4 + 2] = index;
    *(u32*)((int)heap + search->heapSize * 8) = pri;
    i = search->heapSize;
    key = *(u32*)((int)heap + i * 8);
    idx16 = hh[i * 4 + 2];
    *heap = -1;
    while (parent = i >> 1, *(u32*)(hh + parent * 4) < key)
    {
        *(u16*)((int)heap + i * 8 + 4) = *(u16*)((int)heap + (int)((long)parent * 8) + 4);
        *(u32*)((int)heap + i * 8) = *(u32*)((int)heap + (int)((long)parent * 8));
        i = parent;
    }
    *(u32*)((int)heap + i * 8) = key;
    hh[i * 4 + 2] = idx16;
}

static inline int pathSearchFindPointNode(PathSearch* search, PathPoint* point, int* countOut, int* visitedOut)
{
    int index = 0;
    int offset = 0;
    int n;

    *countOut = search->nodeCount;
    for (n = *countOut; n > 0; n--)
    {
        PathSearchNode* scanNode = (PathSearchNode*)((u8*)search->nodes + offset);
        if (scanNode->point == point)
        {
            *visitedOut = scanNode->visited;
            return index;
        }
        offset += 0x10;
        index++;
    }
    return -1;
}

void pathSearchEnqueuePoint(int* q, int* elem, int idx, u32 d, char* obj)
{
    PathSearch* search = (PathSearch*)q;
    PathPoint* point = (PathPoint*)obj;
    int pos;
    u16* hh;
    int cnt2;
    PathSearchNode* node;
    u32* heap;
    int z[2];
    PathSearchNode* node4;
    int visited;
    int cnt;
    if (pathSearchNodeMatchesTarget(q, elem) != 0)
    {
        cnt = search->nodeCount;
        if (cnt != 0xfe)
        {
            node = &search->nodes[search->nodeCount++];
            node->point = point;
            node->routeDistance = d;
            node->parentIndex = (u16)idx;
            node->distanceToTarget = (u32)vec3f_distanceSquared(node->point->position, search->targetPosition);
        }
        pathSearchHeapInsert(search, cnt, 0xfffffffe);
    }
    z[0] = pathSearchFindPointNode(search, point, &cnt2, &visited);
    if (z[0] >= 0 && visited == 0)
    {
        PathSearchNode* node3 = &search->nodes[z[0]];
        if (d < node3->routeDistance)
        {
            int s2;
            u16 target;
            int j;
            u32 newpri;
            u32* entry;
            u32 old;
            node3->parentIndex = idx;
            node3->routeDistance = d;
            newpri = node3->distanceToTarget + node3->routeDistance;
            s2 = search->heapSize;
            heap = (u32*)search->heap;
            hh = (u16*)heap;
            j = 0;
            target = z[0];
            for (; j <= s2; j++)
            {
                if (target == *(u16*)(heap + j * 2 + 1))
                {
                    pos = j;
                    j = s2 + 1;
                }
            }
            entry = heap + pos * 2;
            old = *entry;
            *entry = newpri;
            if (newpri < old)
            {
                pathSearchHeapSiftDown((u8*)heap, s2, pos);
            }
            else if (newpri > old)
            {
                u32 pri = *entry;
                u16 idx16 = ((u16*)entry)[2];
                int parent;
                *heap = -1;
                while (parent = pos >> 1, *(u32*)(hh + parent * 4) < pri)
                {
                    *(u16*)((int)heap + pos * 8 + 4) = *(u16*)((int)heap + (int)((long)parent * 8) + 4);
                    *(u32*)((int)heap + pos * 8) = *(u32*)((int)heap + (int)((long)parent * 8));
                    pos = parent;
                }
                *(u32*)((int)heap + pos * 8) = pri;
                hh[pos * 4 + 2] = idx16;
            }
        }
    }
    else if (z[0] < 0)
    {
        if (cnt2 == 0xfe)
        {
            node4 = NULL;
        }
        else
        {
            node4 = &search->nodes[search->nodeCount++];
            node4->point = point;
            node4->routeDistance = d;
            node4->parentIndex = (u16)idx;
            node4->distanceToTarget = (u32)vec3f_distanceSquared(node4->point->position, search->targetPosition);
        }
        if (node4 != NULL)
        {
            if (node4->distanceToTarget > search->closestDistance)
            {
                u32 newpri = node4->distanceToTarget + node4->routeDistance;
                pathSearchHeapInsert(search, cnt2, -1 - newpri);
            }
            else
            {
                u32 newpri;
                if (node4->distanceToTarget < search->closestDistance)
                {
                    search->closestDistance = node4->distanceToTarget;
                }
                newpri = node4->distanceToTarget + node4->routeDistance;
                pathSearchHeapInsert(search, cnt2, -1 - newpri);
            }
        }
    }
}

void pathSearchExpandNode(int* q, int* elem, int idx)
{
    u8 mask;
    char* p;
    char* node;
    char* obj;
    int bit;
    int t;
    node = (char*)elem[0];
    if (*(u8*)((char*)q + 0x28) != 0)
    {
        t = *(s8*)(node + 0x1b);
    }
    else
    {
        t = ~*(s8*)(node + 0x1b);
    }
    bit = 0;
    p = node;
    mask = t;
    for (; bit < 4; bit++)
    {
        int nodeId = *(int*)(p + 0x1c);
        if (nodeId > -1 && (mask & (1 << bit)) != 0)
        {
            obj = (char*)(*gRomCurveInterface)->getById(nodeId);
            if (obj != 0)
            {
                switch (*(s8*)(obj + 0x19))
                {
                case 0x24:
                {
                    s16 ev1;
                    s16 ev2;
                    mainGetBit(0x4e2);
                    ev1 = *(s16*)(obj + 0x30);
                    if (ev1 == -1 || mainGetBit(ev1) != 0)
                    {
                        ev2 = *(s16*)(obj + 0x32);
                        if (ev2 == -1 || mainGetBit(ev2) == 0)
                        {
                            if (!(*(s8*)(obj + 0x1a) == 8 && *(s8*)(node + 0x1a) == 9))
                            {
                                f32 d = vec3f_distanceSquared((f32*)(node + 8), (f32*)(obj + 8));
                                pathSearchEnqueuePoint(q, elem, idx, (u32)((f32)(u32)elem[2] + d), obj);
                            }
                        }
                    }
                    break;
                }
                default:
                    lbl_803DCD08 = obj;
                    break;
                }
            }
        }
        p += 4;
    }
}
PathPoint* pathSearchGetNextPoint(PathSearch* search)
{
    PathPoint** path;
    int index = search->pathIndex;
    if (index < search->pathCount)
    {
        path = search->path;
        search->pathIndex++;
        return path[index];
    }
    return NULL;
}

int pathSearchBuildPath(PathSearch* search)
{
    int* p = (int*)search;
    int node;
    u32 cur;
    u32 prev;
    int i;
    int count;
    int* entry;

    prev = p[7];
    node = *p + prev * 0x10;
    *(u8*)(node + 0xd) = 0xff;
    while ((cur = *(u8*)(node + 0xc)) != 0xff)
    {
        node = *p + cur * 0x10;
        *(u8*)(node + 0xd) = prev;
        prev = cur;
    }
    if (*(u8*)(node + 0xd) == 0xff)
    {
        entry = NULL;
    }
    else
    {
        entry = (int*)(*p + (u32) * (u8*)(node + 0xd) * 0x10);
    }
    count = 0;
    i = 0;
    while (entry != NULL)
    {
        *(int*)(p[2] + i) = *entry;
        i += 4;
        count++;
        if (count >= 100)
        {
            entry = NULL;
        }
        else if (*(u8*)((int)entry + 0xd) == 0xff)
        {
            entry = NULL;
        }
        else
        {
            entry = (int*)(*p + (u32) * (u8*)((int)entry + 0xd) * 0x10);
        }
    }
    *(s16*)((int)p + 0x2a) = count;
    *(u16*)(p + 0xb) = 0;
    return count;
}

int pathSearchStep(PathSearch* search, u32 n_)
{
    int n;
    int* q = (int*)search;
    int idx;
    int done;
    int result;
    int* elem;
    int* heap;
    n = n_;
    done = 0;
    result = 0;
    while (done == 0 && n != 0)
    {
        heap = *(int**)((char*)q + 0x4);
        if (*(s16*)((char*)q + 0x22) == 0)
        {
            idx = -1;
        }
        else
        {
            idx = *(u16*)((char*)heap + 0xc);
            *(int*)((char*)heap + 0x8) = *(int*)((int)heap + *(s16*)((char*)q + 0x22) * 8);
            *(u16*)((char*)heap + 0xc) = *(u16*)((char*)heap + (*(s16*)((char*)q + 0x22))-- * 8 + 4);
            pathSearchHeapSiftDown((u8*)heap, *(s16*)((char*)q + 0x22), 1);
        }
        if (idx >= 0)
        {
            elem = (int*)(*(int*)((char*)q + 0) + idx * 16);
            *(int*)((char*)q + 0x1c) = idx;
            if (pathSearchNodeMatchesTarget(q, elem) != 0)
            {
                done = 1;
                result = 1;
            }
            else
            {
                *((u8*)elem + 0xe) = 1;
                pathSearchExpandNode(q, elem, idx);
            }
        }
        else
        {
            done = 1;
            result = -1;
        }
        n--;
    }
    return result;
}

int pathSearchBegin(PathSearch* queue, PathPoint* startPoint, f32* targetPosition, int pathId, u32 routeFlags)
{
    int i;
    PathSearchNode* node;
    PathHeapEntry* heap;
    int nodeCount;
    u32 priority;
    int parent;
    u16 nodeIndex;
    u16* heapHalves;
    u16 startNodeIndex;

    queue->heapSize = 0;
    queue->nodeCount = 0;
    for (i = 0; i < 0xfe; i++)
    {
        queue->heap[i].priority = 0;
        queue->nodes[i].visited = 0;
    }
    queue->startPoint = startPoint;
    queue->targetPosition = targetPosition;
    queue->pathId = pathId;
    queue->routeFlags = routeFlags & 1;
    queue->closestDistance = 10000;
    nodeCount = queue->nodeCount;
    if (nodeCount == 0xfe)
    {
        node = NULL;
    }
    else
    {
        node = &queue->nodes[queue->nodeCount++];
        node->point = startPoint;
        node->routeDistance = 0;
        node->parentIndex = 0xff;
        node->distanceToTarget = (u32)vec3f_distanceSquared(node->point->position, queue->targetPosition);
    }
    i = node->distanceToTarget + node->routeDistance;
    heap = queue->heap;
    heapHalves = (u16*)queue->heap;
    startNodeIndex = queue->nodeCount - 1;
    heapHalves[(++queue->heapSize) * 4 + 2] = startNodeIndex;
    heap[queue->heapSize].priority = -1 - i;
    i = queue->heapSize;
    priority = heap[i].priority;
    nodeIndex = heapHalves[i * 4 + 2];
    heap[0].priority = -1;
    while (parent = i >> 1, *(u32*)(heapHalves + parent * 4) < priority)
    {
        *(u16*)((int)heap + i * 8 + 4) = *(u16*)((int)heap + (int)((long)parent * 8) + 4);
        *(u32*)((int)heap + i * 8) = *(u32*)((int)heap + (int)((long)parent * 8));
        i = parent;
    }
    heap[i].priority = priority;
    heapHalves[i * 4 + 2] = nodeIndex;
    return 0;
}


void freeAndNull(void** p)
{
    if (*p != NULL)
    {
        mm_free(*p);
        *p = NULL;
    }
}

void trickyVoxAllocFn_8004b5d4(PathSearch* search)
{
    search->nodes = (PathSearchNode*)mmAlloc(0x1960, 0x10, 0);
    search->heap = (PathHeapEntry*)((u8*)search->nodes + 0xfe0);
    search->path = (PathPoint**)((u8*)search->heap + 0x7f0);
}


void allocSomething32bytes(void)
{
    lbl_803DCD10 = mmAlloc(0x20, 0xff, 0);
}
