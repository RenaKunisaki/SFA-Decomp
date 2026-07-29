#include "main/frame_timing.h"
#include "sys/objects.h"
#include "main/shader_api.h"
#include "main/sky_api.h"
#include "main/camera.h"
#include "main/dll/modgfx_interface.h"
#include "main/dll/waterfx_interface.h"
#include "main/lightmap_api.h"
#include "main/lightmap_render_control_api.h"
#include "main/lightmap_render_queue_api.h"
#include "main/lightmap_text_color_api.h"
#include "main/model.h"
#include "main/model_render_instrs_api.h"
#include "main/modellight_api.h"
#include "main/newclouds.h"
#include "main/objprint_render_api.h"
#include "main/sky_interface.h"
#include "main/vecmath.h"
#include "main/pi_dolphin.h"
#include "dolphin/gx/GXLighting.h"
#include "main/sky_state.h"
#include "main/track_dolphin_api.h"
#include "main/mm.h"
#include "string.h"
#include "main/rcp_dolphin.h"
#include "main/dll/dll_0000_gameui.h"
#include "main/dll/dll_0031_minimap.h"
#include "main/dll/savegame_env_api.h"
#include "main/sky.h"
#include "track/intersect_api.h"
#include "track/intersect_render_setup_api.h"
#include "main/dll/cloudaction.h"
#include "main/trig.h"
#include "main/tex_dolphin.h"
#include "main/acosf_api.h"
#include "dolphin/gx/GXGeometry.h"
#include "dolphin/gx/GXTransform.h"
#include "dolphin/mtx.h"
#include "dolphin/os/OSFastCast.h"

extern f32 widescreenAspect_803DEC1C;
extern f32 lbl_803DB670;

void sceneDraw(void);
void sceneDrawTransparentPolys(void);

#include "main/lightmap_internal.h"


#include "main/render_flags.h"

extern f32 lbl_803DEBFC;

extern u8 lbl_803DCE98; /* count of allocated blocks */


volatile PPCWGPipe GXWGFifo : (0xCC008000);

void renderShadowType3(u8* obj, u32 b, s32 offset);
static inline void GXPosition3s16(const s16 x, const s16 y, const s16 z)
{
    GXWGFifo.s16 = x;
    GXWGFifo.s16 = y;
    GXWGFifo.s16 = z;
}
static inline void GXColor4u8(const u8 r, const u8 g, const u8 b, const u8 a)
{
    GXWGFifo.u8 = r;
    GXWGFifo.u8 = g;
    GXWGFifo.u8 = b;
    GXWGFifo.u8 = a;
}
static inline void GXTexCoord2s16(const s16 s, const s16 t)
{
    GXWGFifo.s16 = s;
    GXWGFifo.s16 = t;
}
static inline void GXPosition1x8(const u8 x) { GXWGFifo.u8 = x; }


extern u32 gLightmapDrawQueue[];
extern s32 gLightmapDrawQueueCount;


typedef struct LightmapDrawEntry
{
    union
    {
        u32 value;
        GameObject* object;
        MapBlockBoundsRec* bounds;
    } arg0;
    union
    {
        u32 value;
        MapBlockData* block;
    } arg1;
    u32 sortKey;
    s32 type;
} LightmapDrawEntry;

typedef union LightmapDrawItem
{
    GameObject* object;
    MapBlockData* block;
} LightmapDrawItem;




#define MAP_BLOCK_LAYER_COUNT 5


void sortVisibleObjectKeysDescending(u32* arr, int n);

void sortVisibleObjectKeysDescending(u32* arr, int n);
void getVisibleObjects(s8* opacity);

static inline void fillBoxRows(u8* map, int* box)
{
    int y, x0;
    int xs, xe;
    u8* p;
    for (y = box[2]; y <= box[3]; y++)
    {
        xs = box[0];
        p = map + (y + 7) * 0x10 + xs;
        xe = box[1];
        for (x0 = xs; x0 <= xe; x0++)
        {
            p[7] = 1;
            p++;
        }
    }
}

void renderSceneGeometry(u8 renderType, s8* order);


void sceneDraw(void);


void gameFlagFn_8005cd24(int v)
{
    renderFlags = (v != 0) ? (renderFlags | 0x20000) : (renderFlags & ~0x20000);
}

int getDrawDistanceFlag_8005cd48(void) { return renderFlags & RENDERFLAG_DRAW_DISTANCE; }


int setWidescreen(u8 v)
{
    if (v != 0)
    {
        renderFlags |= RENDERFLAG_WIDESCREEN;
        Camera_SetAspectRatio(widescreenAspect_803DEC1C);
    }
    else
    {
        renderFlags &= ~(u64)RENDERFLAG_WIDESCREEN;
        Camera_SetAspectRatio(lbl_803DB670);
    }
    return 0;
}
int isWidescreen(void) { return renderFlags & RENDERFLAG_WIDESCREEN; }
u32 shouldDrawShadows(void) { return renderFlags & RENDERFLAG_DRAW_SHADOWS; }
int shouldDrawClouds(void) { return renderFlags & RENDERFLAG_DRAW_CLOUDS; }

void titleScreenFn_8005cdd4(int v)
{
    if (v != 0) renderFlags &= ~0x2000;
    else renderFlags |= 0x2000;
}

void setDrawLights(int v)
{
    u8* env = saveGameGetEnvState();
    if (v != 0)
    {
        renderFlags |= 0x40;
        env[0x40] |= 0x8;
    }
    else
    {
        renderFlags &= ~0x40LL;
        env[0x40] &= ~0x8;
    }
}

void gameFlagFn_8005ce6c(int v)
{
    renderFlags = (v != 0) ? (renderFlags | 0x20) : (renderFlags & ~0x20);
}

u8 isOvercast(void)
{
    u32 v = renderFlags & RENDERFLAG_OVERCAST;
    u32 t = ((u32) - (s32)v | v) >> 31;
    return t;
}

void setIsOvercast(int v)
{
    renderFlags = (v != 0) ? (renderFlags | RENDERFLAG_OVERCAST) : (renderFlags & ~RENDERFLAG_OVERCAST);
}

void setStarsHidden(int v)
{
    renderFlags = (v != 0) ? (renderFlags | RENDERFLAG_HIDE_STARS) : (renderFlags & ~RENDERFLAG_HIDE_STARS);
}

void setDrawCloudsAndLights(int v)
{
    u8* env = saveGameGetEnvState();
    if (v != 0)
    {
        renderFlags |= 0x50;
        env[0x40] |= 0x9;
    }
    else
    {
        renderFlags &= ~0x50;
        env[0x40] &= ~0x9;
    }
}

void setPendingMapLoad(int v)
{
    renderFlags = (v != 0) ? (renderFlags | RENDERFLAG_PENDING_MAP_LOAD) : (renderFlags & ~RENDERFLAG_PENDING_MAP_LOAD);
}

void lightmapDrawTriangleList(const void* vertexBase, u8* triList, int triCount)
{
    const LightmapVertex* vertices = vertexBase;
    const LightmapVertex* vertex;
    int tri, vtx;

    /* Emit triCount triangles as GX_TRIANGLES; each vertex is 16 bytes:
       s16 pos[3] @0x0, u8 color[4] @0xc, s16 texcoord[2] @0x8. */
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXBegin(GX_TRIANGLES, GX_VTXFMT0, triCount * 3 & 0xffff);
    for (tri = 0; tri < triCount; tri++)
    {
        u8* list = triList;
        for (vtx = 0; vtx < 3; vtx++)
        {
            GXPosition1x8(0);
            vertex = &vertices[list[vtx + 1]];
            GXPosition3s16(vertex->x, vertex->y, vertex->z);
            vertex = &vertices[list[vtx + 1]];
            GXColor4u8(vertex->r, vertex->g, vertex->b, vertex->a);
            vertex = &vertices[list[vtx + 1]];
            GXTexCoord2s16(vertex->s, vertex->t);
        }
        triList = triList + 0x10;
    }
}


void setFogColorCallback(int unused, u8 red, u8 green, u8 blue, int wpad0)
{
    setFogColorRgb(red, green, blue);
}


void _textSetColor(void* context, int red, int green, int blue, int alpha)
{
    _gxSetTevColor1(red, green, blue, alpha);
}

void setTextColor(void* context, int a, int b, int c, int d)
{
    _gxSetTevColor2(a, b, c, d);
}

void doNothing_8005D148(int arg0, int arg1)
{
}


void lightmapDrawQueuedObject(GameObject* obj);
void mapBlockRenderMain(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx);
void mapBlockRenderWater(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx);
void mapBlockRenderTransparent(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx);
void lightmap_sortTransparentDrawQueue(void);

void getVisibleObjects(s8 * opacity);


void renderSceneGeometry(u8 renderType, s8* order);

void doNothing_8005D14C(int arg0, int arg1)
{
}
void renderShadowType3(u8* obj, u32 b, s32 offset)
{
    Vec stk;
    s32 t;
    if (gLightmapDrawQueueCount == 1000)
    {
        sceneDrawTransparentPolys();
        gLightmapDrawQueueCount = 0;
    }
    if (((GameObject*)obj)->anim.parent != NULL)
    {
        stk.x = ((GameObject*)obj)->anim.worldPosX;
        stk.y = ((GameObject*)obj)->anim.worldPosY;
        stk.z = ((GameObject*)obj)->anim.worldPosZ;
    }
    else
    {
        stk.x = ((GameObject*)obj)->anim.worldPosX - playerMapOffsetX;
        stk.y = ((GameObject*)obj)->anim.worldPosY;
        stk.z = ((GameObject*)obj)->anim.worldPosZ - playerMapOffsetZ;
    }
    PSMTXMultVec((MtxPtr)Camera_GetViewMatrix(), &stk, &stk);
    t = (s32) - stk.z + offset;
    t = t < 0 ? 0 : (t > 0x7ffffff ? 0x7ffffff : t);
    gLightmapDrawQueue[gLightmapDrawQueueCount * 4] = (u32)obj;
    gLightmapDrawQueue[gLightmapDrawQueueCount * 4 + 2] = t | ((b & 0xff) << 27);
}

void lightmap_sortTransparentDrawQueue(void)
{
    int i, j;
    int gap = 1;
    LightSortEntry tmp;
    while (gap <= (gLightmapDrawQueueCount - 1) / 9)
        gap = gap * 3 + 1;
    while (gap > 0)
    {
        for (i = gap + 1; i <= gLightmapDrawQueueCount; i++)
        {
            tmp = ((LightSortEntry*)gLightmapDrawQueue)[i - 1];
            j = i;
            while (j > gap && ((LightSortEntry*)gLightmapDrawQueue)[j - gap - 1].key < tmp.key)
            {
                ((LightSortEntry*)gLightmapDrawQueue)[j - 1] = ((LightSortEntry*)gLightmapDrawQueue)[j - gap - 1];
                j -= gap;
            }
            ((LightSortEntry*)gLightmapDrawQueue)[j - 1] = tmp;
        }
        gap /= 3;
    }
}




extern f32 lbl_803DEC20;

void lightmapQueueShadowRow(MapBlockBoundsRec* bounds, MapBlockData* block, s32 selector)
{
    Vec stk;
    s32 t;
    f32 maxXs;
    f32 minXs;
    f32 maxYs;
    f32 maxW;
    f32 minYs;
    f32 maxZs;
    f32 maxD;
    f32 minZs;
    f32 minW;
    f32 minD;

    if (gLightmapDrawQueueCount == 1000)
    {
        sceneDrawTransparentPolys();
        gLightmapDrawQueueCount = 0;
    }
    maxXs = __OSs16tof32(&bounds->maxX);
    minXs = __OSs16tof32(&bounds->minX);
    maxYs = __OSs16tof32(&bounds->maxY);
    maxW = maxYs * lbl_803DEC20 + block->transform[1][3];
    minYs = __OSs16tof32(&bounds->minY);
    maxZs = __OSs16tof32(&bounds->maxZ);
    maxD = maxZs * lbl_803DEC20 + block->transform[2][3];
    minZs = __OSs16tof32(&bounds->minZ);
    stk.x = lbl_803DEBFC * ((minXs * lbl_803DEC20 + block->transform[0][3]) +
                             (maxXs * lbl_803DEC20 + block->transform[0][3]));
    minW = minYs * lbl_803DEC20 + block->transform[1][3];
    stk.y = lbl_803DEBFC * (minW + maxW);
    minD = minZs * lbl_803DEC20 + block->transform[2][3];
    stk.z = lbl_803DEBFC * (minD + maxD);
    PSMTXMultVec((MtxPtr)Camera_GetViewMatrix(), &stk, &stk);
    t = (s32) - stk.z;
    t = t < 0 ? 0 : (t > 0x7ffffff ? 0x7ffffff : t);
    gLightmapDrawQueue[gLightmapDrawQueueCount * 4] = (u32)bounds;
    gLightmapDrawQueue[gLightmapDrawQueueCount * 4 + 1] = (u32)block;
    gLightmapDrawQueue[gLightmapDrawQueueCount * 4 + 2] = t | ((selector & 0xff) << 27);
}


void sortVisibleObjectKeysDescending(u32* arr, int n);


void mapBlockRenderMain(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx)
{
    int state[5];
    int countShifted;
    int cursor;
    u32 v;
    int* base;
    struct MapShader* newR;
    int nibble;
    int i;
    u8* s0;

    countShifted = block->nRenderInstrsMain << 3;
    modelRenderInstrsState_init((ModelRenderInstrsState*)state, block->renderInstrsMain, countShifted, countShifted);
    modelRenderInstrsState_setBit((ModelRenderInstrsState*)state, bounds->renderBitOffset);
    state[4] += 4;
    mapBlockRender_drawDimmedAabbLights(bounds, block, viewMtx);
    newR = mapBlockRender_setLightmapShader(block, (ModelRenderInstrsState*)state);
    state[4] += 4;
    mapBlockRender_setVtxDcrs(1, block, newR, (ModelRenderInstrsState*)state);
    cursor = state[4] + 4;
    state[4] = cursor;
    countShifted = cursor >> 3;
    s0 = (u8*)state[0];
    v = s0[countShifted];
    base = (int*)(state[0] + countShifted);
    v = v | ((u32)*(u8*)((char*)base + 1) << 8);
    v = v | ((u32)*(u8*)((char*)base + 2) << 16);
    state[4] += 4;
    nibble = (v >> (cursor & 7)) & 0xf;
    for (i = 0; i < nibble; i++)
    {
        *(int*)&state[4] = state[4] + 8;
    }
    state[4] += 4;
    mapBlockRender_drawLightmapIndirectPasses(block, newR, (ModelRenderInstrsState*)state,
                                               (float (*)[4])viewMtx);
}
void mapBlockRenderWater(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx)
{
    int state[5];
    Mtx m;
    int countShifted;
    struct MapShader* newR;
    int cursor;
    u32 v;
    int* base;
    int nibble;
    int i;
    u8* s0;

    PSMTXConcat((MtxPtr)gCameraLightPerspectiveScaledMatrix, (MtxPtr)viewMtx, m);
    GXLoadTexMtxImm(m, GX_TEXMTX0, GX_MTX3x4);
    PSMTXConcat((MtxPtr)gCameraLightPerspectiveFlipYMatrix, (MtxPtr)viewMtx, m);
    GXLoadTexMtxImm(m, GX_TEXMTX1, GX_MTX3x4);
    setupWaterCausticTev();
    countShifted = block->nRenderInstrsWater << 3;
    modelRenderInstrsState_init((ModelRenderInstrsState*)state, block->renderInstrsWater, countShifted, countShifted);
    modelRenderInstrsState_setBit((ModelRenderInstrsState*)state, bounds->renderBitOffset);
    state[4] += 4;
    newR = mapBlockRender_setShader(1, block, (ModelRenderInstrsState*)state);
    state[4] += 4;
    mapBlockRender_setVtxDcrs(1, block, newR, (ModelRenderInstrsState*)state);
    cursor = state[4] + 4;
    state[4] = cursor;
    countShifted = cursor >> 3;
    s0 = (u8*)state[0];
    v = s0[countShifted];
    base = (int*)(state[0] + countShifted);
    v = v | ((u32)*(u8*)((char*)base + 1) << 8);
    v = v | ((u32)*(u8*)((char*)base + 2) << 16);
    state[4] += 4;
    nibble = (v >> (cursor & 7)) & 0xf;
    for (i = 0; i < nibble; i++)
    {
        *(int*)&state[4] = state[4] + 8;
    }
    state[4] += 4;
    mapBlockRender_callList(1, 1, block, newR, (ModelRenderInstrsState*)state, viewMtx);
}
void mapBlockRenderTransparent(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx)
{
    int state[5];
    int countShifted;
    struct MapShader* newR;
    int cursor;
    u32 v;
    int* base;
    int nibble;
    int i;
    u8* s0;

    Camera_ApplyTransparentViewport();
    countShifted = block->nRenderInstrsTransp << 3;
    modelRenderInstrsState_init((ModelRenderInstrsState*)state, block->renderInstrsTransp, countShifted, countShifted);
    modelRenderInstrsState_setBit((ModelRenderInstrsState*)state, bounds->renderBitOffset);
    state[4] += 4;
    newR = mapBlockRender_setShader(1, block, (ModelRenderInstrsState*)state);
    state[4] += 4;
    mapBlockRender_setVtxDcrs(1, block, newR, (ModelRenderInstrsState*)state);
    cursor = state[4] + 4;
    state[4] = cursor;
    countShifted = cursor >> 3;
    s0 = (u8*)state[0];
    v = s0[countShifted];
    base = (int*)(state[0] + countShifted);
    v = v | ((u32)*(u8*)((char*)base + 1) << 8);
    v = v | ((u32)*(u8*)((char*)base + 2) << 16);
    state[4] += 4;
    nibble = (v >> (cursor & 7)) & 0xf;
    for (i = 0; i < nibble; i++)
    {
        *(int*)&state[4] = state[4] + 8;
    }
    state[4] += 4;
    mapBlockRender_callList(1, 1, block, newR, (ModelRenderInstrsState*)state, viewMtx);
    Camera_ApplyFullViewport();
}


void lightmapDrawQueuedObject(GameObject* obj)
{
    ObjModel* model = Obj_GetActiveModel(obj);
    if (model->renderAttachment != NULL)
    {
        objRenderFn_8003d980((u8*)obj, (int*)model);
    }
    else
    {
        ObjModelState* shadow;
        (*gModgfxInterface)->renderEffects(NULL, 0, 0, 1, obj);
        renderResetFn_8003fc60();
        objRender(0, 0, 0, 0, obj, 1);
        Camera_ApplyDecalViewport();
        shadow = (ObjModelState*)(obj->anim.modelState);
        if (shadow != NULL && shadow->shadowCastSlot != NULL)
        {
            objShadowRender(obj, 0, 0, framesThisStep);
        }
        else if (((ObjAnimComponent*)obj)->modelInstance->shadowType == OBJ_SHADOW_TYPE_CRASH)
        {
            objDrawGroundShadow(obj, model);
        }
        Camera_ApplyFullViewport();
    }
}

static inline void lightmapSetObjAmbColor(void)
{
    GXColor color;

    objGetColor(0, (u8*)&color, (u8*)&color + 1, (u8*)&color + 2);
    GXSetChanAmbColor(GX_COLOR0, color);
    GXSetNumChans(1);
}

void sceneDrawTransparentPolys(void)
{
    int i;
    LightmapDrawItem item;
    GameObject* player;
    LightmapDrawEntry* entries;
    f32 m[16];

    lightmap_sortTransparentDrawQueue();
    i = 0;
    entries = (LightmapDrawEntry*)gLightmapDrawQueue;
    for (; i < gLightmapDrawQueueCount; i++)
    {
        switch (entries[i].type)
        {
        case 0:
            expgfx_renderSourcePools(entries[i].arg0.value, 0);
            lightmapDrawQueuedObject(entries[i].arg0.object);
            expgfx_renderSourcePools(entries[i].arg0.value, 1);
            break;
        case 1:
            item.object = entries[i].arg0.object;
            Obj_GetActiveModel(item.object);
            player = Obj_GetPlayerObject();
            if (item.object == player)
            {
                if (playerIsDisguised(item.object) == 0)
                {
                    fn_802B4ED8(item.object, 1, 1);
                }
            }
            else
            {
                objRenderFuzz((int*)item.object);
            }
            break;
        case 2:
            Camera_ApplyDecalViewport();
            objShadowRender(entries[i].arg0.object, 0, 0, framesThisStep);
            Camera_ApplyFullViewport();
            break;
        case 3:
            Camera_ApplyDecalViewport();
            objDrawGroundShadow(entries[i].arg0.object, Obj_GetActiveModel(entries[i].arg0.object));
            Camera_ApplyFullViewport();
            break;
        case 4:
            item.block = entries[i].arg1.block;
            GXSetChanCtrl(GX_COLOR0, GX_TRUE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
            GXSetChanCtrl(GX_ALPHA0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
            lightmapSetObjAmbColor();
            PSMTXConcat((MtxPtr)Camera_GetViewMatrix(), item.block->transform, (MtxPtr)m);
            setupToRenderMapBlock(item.block, m);
            mapBlockRenderTransparent(entries[i].arg0.bounds, entries[i].arg1.block, m);
            break;
        case 5:
            item.block = entries[i].arg1.block;
            GXSetChanCtrl(GX_COLOR0, GX_TRUE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
            GXSetChanCtrl(GX_ALPHA0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
            lightmapSetObjAmbColor();
            PSMTXConcat((MtxPtr)Camera_GetViewMatrix(), item.block->transform, (MtxPtr)m);
            setupToRenderMapBlock(item.block, m);
            mapBlockRenderWater(entries[i].arg0.bounds, entries[i].arg1.block, m);
            break;
        case 6:
            item.block = entries[i].arg1.block;
            GXSetChanCtrl(GX_COLOR0, GX_TRUE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
            GXSetChanCtrl(GX_ALPHA0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
            lightmapSetObjAmbColor();
            PSMTXConcat((MtxPtr)Camera_GetViewMatrix(), item.block->transform, (MtxPtr)m);
            setupToRenderMapBlock(item.block, m);
            mapBlockRenderMain(entries[i].arg0.bounds, entries[i].arg1.block, m);
            break;
        case 7:
            drawGlow(entries[i].arg0.value, entries[i].arg1.value);
            break;
        case 8:
            drawFn_8006f500();
            break;
        case 9:
            (*gWaterfxInterface)->render(0, 0);
        }
    }
}


void lightmap_queueExternalRenderEntry(u32 a, u32 b, f32* p)
{
    s32 t;
    if (gLightmapDrawQueueCount == 1000)
    {
        sceneDrawTransparentPolys();
        gLightmapDrawQueueCount = 0;
    }
    t = (s32) - p[2];
    t = t < 0 ? 0 : (t > 0x7ffffff ? 0x7ffffff : t);
    gLightmapDrawQueue[gLightmapDrawQueueCount * 4] = a;
    gLightmapDrawQueue[gLightmapDrawQueueCount * 4 + 1] = b;
    gLightmapDrawQueue[gLightmapDrawQueueCount * 4 + 2] = t | 0x38000000;
    gLightmapDrawQueue[gLightmapDrawQueueCount * 4 + 3] = 7;
    gLightmapDrawQueueCount++;
}
