#include "game/objects/object.h"
#include "main/dll/player_api.h"
#include "main/frame_timing.h"
#include "main/hud_visibility_api.h"
#include "sys/objects.h"
#include "main/shader_api.h"
#include "main/shader_map_api.h"
#include "main/sky_api.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_float_helpers.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/camera_interface.h"
#include "main/camera.h"
#include "main/dll_000A_expgfx.h"
#include "main/dll/cloudaction_interface.h"
#include "main/dll/modgfx_interface.h"
#include "main/dll/waterfx_interface.h"
#include "main/frustum.h"
#include "main/lightmap_api.h"
#include "main/lightmap_lifecycle_api.h"
#include "main/lightmap_render_control_api.h"
#include "main/lightmap_render_queue_api.h"
#include "main/lightmap_text_color_api.h"
#include "main/model_render_instrs_api.h"
#include "main/modellight_api.h"
#include "main/newclouds.h"
#include "main/obj_list.h"
#include "main/objprint_render_api.h"
#include "main/sky_interface.h"
#include "main/vecmath.h"
#include "main/pi_dolphin.h"
#include "dolphin/gx/GXLighting.h"
#include "dolphin/gx/GXManage.h"
#include "main/sky_state.h"
#include "main/track_dolphin_api.h"
#include "main/mm.h"
#include "string.h"
#include "main/newshadows.h"
#include "main/newshadows_shadow_api.h"
#include "main/rcp_dolphin.h"
#include "main/dll/dll_0000_gameui.h"
#include "main/dll/dll_0031_minimap.h"
#include "main/dll/dll_00E2_staff_api.h"
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
#include "dolphin/mtx/mtx_legacy.h"
#include "dolphin/os/OSFastCast.h"

extern f32 widescreenAspect_803DEC1C;
extern f32 lbl_803DB670;

void sceneDraw(void);
void sceneDrawTransparentPolys(void);

typedef struct
{
    f32 lo;
    f32 hi;
} F32Pair;

typedef struct EnvironmentUpdateInterface
{
    void (*create)(void);
    void (*destroy)(void);
    void (*update)(void);
} EnvironmentUpdateInterface;

extern u32 renderFlags;
/* Global renderFlags bits (decoded by the accessor fns below: shouldDrawShadows,
 * shouldDrawClouds, getDrawDistanceFlag, isOvercast, setPendingMapLoad,
 * setStarsHidden). */
#define RENDERFLAG_WIDESCREEN      0x8
#define RENDERFLAG_DRAW_CLOUDS     0x10
#define RENDERFLAG_DRAW_SHADOWS    0x80
#define RENDERFLAG_PENDING_MAP_LOAD 0x1000
#define RENDERFLAG_DRAW_DISTANCE   0x10000
#define RENDERFLAG_OVERCAST        0x40000
#define RENDERFLAG_HIDE_STARS      0x80000

extern f32 lbl_803DEBFC;

extern u8 lbl_803DCE98; /* count of allocated blocks */
typedef struct
{
    u32 a;
    u32 b;
    u32 key;
    u32 d;
} LightSortEntry;

typedef union
{
    u8 u8;
    u16 u16;
    u32 u32;
    s8 s8;
    s16 s16;
    s32 s32;
    f32 f32;
} PPCWGPipe;

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

void updateVisibleGeometry(void);

MapBlockData* mapGetBlock(int i);

extern u32 lbl_8037E0C0[];
extern s32 lbl_803DCE30;

s8* mapGetBlockIdx(int layer);

MapBlockData* mapGetBlockAtPos(int x, int y, int layer);

void* RomList_GetLoadedPages(void);


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

typedef struct MapLayerBuffers
{
    u8 reserved[0x41cc];
    u8* cellStates[5];
    u8* blockDescriptors[5];
    u8* blockIndices[5];
} MapLayerBuffers;

typedef struct
{
    u8 pad[0x4114];
    u32 deferred[20];
} LightmapDrawQueue;


int coordsToMapCell(f32 x, f32 z);

void mapGetBlockOriginForPos(f32 x, f32 y, f32 z, f32* outX, f32* outZ);

#define MAP_BLOCK_LAYER_COUNT 5

int isInBounds(f32 x, f32 z);


int objPosToMapBlockIdx(f32 x, f32 y, f32 z);


int* mapRomListFindItem(int needle, int* out_idx, int* out_outer, int* out_type, int* out_lastpage);

void sortVisibleObjectKeysDescending(u32* arr, int n);

void sortVisibleObjectKeysDescending(u32* arr, int n);
void getVisibleObjects(s8* opacity);

void renderObjects(s8* opacity);
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


void sceneRender(int wpad0, int wpad1, int wpad2, int wpad3, int wpad4, int wpad5);

void doNothing_beforeTitleScreen(void);
void updateEnvironment(int mode);
void initMapBlocks(void);

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
    void* env = saveGameGetEnvState();
    if (v != 0)
    {
        renderFlags |= 0x40;
        *(u8*)((char*)env + 0x40) |= 0x8;
    }
    else
    {
        renderFlags &= ~0x40LL;
        *(u8*)((char*)env + 0x40) &= ~0x8;
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
    void* env = saveGameGetEnvState();
    if (v != 0)
    {
        renderFlags |= 0x50;
        *(u8*)((char*)env + 0x40) |= 0x9;
    }
    else
    {
        renderFlags &= ~0x50;
        *(u8*)((char*)env + 0x40) &= ~0x9;
    }
}

void setPendingMapLoad(int v)
{
    renderFlags = (v != 0) ? (renderFlags | RENDERFLAG_PENDING_MAP_LOAD) : (renderFlags & ~RENDERFLAG_PENDING_MAP_LOAD);
}

void drawFn_8005cf8c(const void* vertexBase, u8* triList, int triCount)
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


void objDrawFn_8005da48(GameObject* obj);
void modelRenderFn_8005d4ec(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx);
void modelRenderFn_8005d69c(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx);
void modelRenderFn_8005d894(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx);
void lightmap_sortTransparentDrawQueue(void);

void getVisibleObjects(s8 * opacity);


void renderSceneGeometry(u8 renderType, s8* order);

void doNothing_8005D14C(int arg0, int arg1)
{
}
void renderShadowType3(u8* obj, u32 b, s32 offset)
{
    f32 stk[3];
    s32 t;
    if (lbl_803DCE30 == 1000)
    {
        sceneDrawTransparentPolys();
        lbl_803DCE30 = 0;
    }
    if (((GameObject*)obj)->anim.parent != NULL)
    {
        stk[0] = ((GameObject*)obj)->anim.worldPosX;
        stk[1] = ((GameObject*)obj)->anim.worldPosY;
        stk[2] = ((GameObject*)obj)->anim.worldPosZ;
    }
    else
    {
        stk[0] = ((GameObject*)obj)->anim.worldPosX - playerMapOffsetX;
        stk[1] = ((GameObject*)obj)->anim.worldPosY;
        stk[2] = ((GameObject*)obj)->anim.worldPosZ - playerMapOffsetZ;
    }
    PSMTXMultVec((f32*)Camera_GetViewMatrix(), stk, stk);
    t = (s32) - stk[2] + offset;
    t = t < 0 ? 0 : (t > 0x7ffffff ? 0x7ffffff : t);
    lbl_8037E0C0[lbl_803DCE30 * 4] = (u32)obj;
    lbl_8037E0C0[lbl_803DCE30 * 4 + 2] = t | ((b & 0xff) << 27);
}

void lightmap_sortTransparentDrawQueue(void)
{
    int i, j;
    int gap = 1;
    LightSortEntry tmp;
    while (gap <= (lbl_803DCE30 - 1) / 9)
        gap = gap * 3 + 1;
    while (gap > 0)
    {
        for (i = gap + 1; i <= lbl_803DCE30; i++)
        {
            tmp = ((LightSortEntry*)lbl_8037E0C0)[i - 1];
            j = i;
            while (j > gap && ((LightSortEntry*)lbl_8037E0C0)[j - gap - 1].key < tmp.key)
            {
                ((LightSortEntry*)lbl_8037E0C0)[j - 1] = ((LightSortEntry*)lbl_8037E0C0)[j - gap - 1];
                j -= gap;
            }
            ((LightSortEntry*)lbl_8037E0C0)[j - 1] = tmp;
        }
        gap /= 3;
    }
}


typedef union
{
    double d;

    struct
    {
        u32 hi;
        u32 lo;
    } u;
} F64Cvt;




extern f32 lbl_803DEC20;

void lightmapQueueShadowRow(MapBlockBoundsRec* bounds, MapBlockData* block, s32 selector)
{
    f32 stk[3];
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

    if (lbl_803DCE30 == 1000)
    {
        sceneDrawTransparentPolys();
        lbl_803DCE30 = 0;
    }
    maxXs = __OSs16tof32(&bounds->maxX);
    minXs = __OSs16tof32(&bounds->minX);
    maxYs = __OSs16tof32(&bounds->maxY);
    maxW = maxYs * lbl_803DEC20 + block->transform[1][3];
    minYs = __OSs16tof32(&bounds->minY);
    maxZs = __OSs16tof32(&bounds->maxZ);
    maxD = maxZs * lbl_803DEC20 + block->transform[2][3];
    minZs = __OSs16tof32(&bounds->minZ);
    stk[0] = lbl_803DEBFC * ((minXs * lbl_803DEC20 + block->transform[0][3]) +
                             (maxXs * lbl_803DEC20 + block->transform[0][3]));
    minW = minYs * lbl_803DEC20 + block->transform[1][3];
    stk[1] = lbl_803DEBFC * (minW + maxW);
    minD = minZs * lbl_803DEC20 + block->transform[2][3];
    stk[2] = lbl_803DEBFC * (minD + maxD);
    PSMTXMultVec((f32*)Camera_GetViewMatrix(), stk, stk);
    t = (s32) - stk[2];
    t = t < 0 ? 0 : (t > 0x7ffffff ? 0x7ffffff : t);
    lbl_8037E0C0[lbl_803DCE30 * 4] = (u32)bounds;
    lbl_8037E0C0[lbl_803DCE30 * 4 + 1] = (u32)block;
    lbl_8037E0C0[lbl_803DCE30 * 4 + 2] = t | ((selector & 0xff) << 27);
}


void sortVisibleObjectKeysDescending(u32* arr, int n);


void modelRenderFn_8005d4ec(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx)
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
void modelRenderFn_8005d69c(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx)
{
    int state[5];
    f32 m[12];
    int countShifted;
    struct MapShader* newR;
    int cursor;
    u32 v;
    int* base;
    int nibble;
    int i;
    u8* s0;

    PSMTXConcat((f32*)lbl_80396850, viewMtx, m);
    GXLoadTexMtxImm((const f32 (*)[4])m, GX_TEXMTX0, GX_MTX3x4);
    PSMTXConcat((f32*)lbl_80396820, viewMtx, m);
    GXLoadTexMtxImm((const f32 (*)[4])m, GX_TEXMTX1, GX_MTX3x4);
    gxTextureSetupFn_8007cf7c();
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
void modelRenderFn_8005d894(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx)
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


void objDrawFn_8005da48(GameObject* obj)
{
    int* model = (int*)Obj_GetActiveModel(obj);
    if (*(void**)((char*)model + 0x58) != NULL)
    {
        objRenderFn_8003d980((u8*)obj, model);
    }
    else
    {
        void* shadow;
        (*gModgfxInterface)->renderEffects(NULL, 0, 0, 1, obj);
        renderResetFn_8003fc60();
        objRender(0, 0, 0, 0, obj, 1);
        Camera_ApplyDecalViewport();
        shadow = obj->anim.modelState;
        if (shadow != NULL && ((ObjModelState*)shadow)->shadowCastSlot != NULL)
        {
            objShadowFn_80062498(obj, 0, 0, framesThisStep);
        }
        else if (((ObjAnimComponent*)obj)->modelInstance->shadowType == OBJ_SHADOW_TYPE_CRASH)
        {
            objDrawFn_80061654(obj, (ObjModel*)model);
        }
        Camera_ApplyFullViewport();
    }
}

void sceneDrawTransparentPolys(void)
{
    GXColor c5;
    int i;
    LightmapDrawItem item;
    GameObject* player;
    GXColor c4;
    LightmapDrawEntry* entries;
    GXColor c6;
    f32 m[16];

    lightmap_sortTransparentDrawQueue();
    i = 0;
    entries = (LightmapDrawEntry*)lbl_8037E0C0;
    for (; i < lbl_803DCE30; i++)
    {
        switch (entries[i].type)
        {
        case 0:
            expgfx_renderSourcePools(entries[i].arg0.value, 0);
            objDrawFn_8005da48(entries[i].arg0.object);
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
            objShadowFn_80062498(entries[i].arg0.object, 0, 0, framesThisStep);
            Camera_ApplyFullViewport();
            break;
        case 3:
            Camera_ApplyDecalViewport();
            objDrawFn_80061654(entries[i].arg0.object, Obj_GetActiveModel(entries[i].arg0.object));
            Camera_ApplyFullViewport();
            break;
        case 4:
            item.block = entries[i].arg1.block;
            GXSetChanCtrl(GX_COLOR0, GX_TRUE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
            GXSetChanCtrl(GX_ALPHA0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
            objGetColor(0, (u8*)&c4, (u8*)&c4 + 1, (u8*)&c4 + 2);
            GXSetChanAmbColor(GX_COLOR0, c4);
            GXSetNumChans(1);
            PSMTXConcat((f32*)Camera_GetViewMatrix(), (f32*)item.block->transform, m);
            setupToRenderMapBlock(item.block, m);
            modelRenderFn_8005d894(entries[i].arg0.bounds, entries[i].arg1.block, m);
            break;
        case 5:
            item.block = entries[i].arg1.block;
            GXSetChanCtrl(GX_COLOR0, GX_TRUE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
            GXSetChanCtrl(GX_ALPHA0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
            objGetColor(0, (u8*)&c5, (u8*)&c5 + 1, (u8*)&c5 + 2);
            GXSetChanAmbColor(GX_COLOR0, c5);
            GXSetNumChans(1);
            PSMTXConcat((f32*)Camera_GetViewMatrix(), (f32*)item.block->transform, m);
            setupToRenderMapBlock(item.block, m);
            modelRenderFn_8005d69c(entries[i].arg0.bounds, entries[i].arg1.block, m);
            break;
        case 6:
            item.block = entries[i].arg1.block;
            GXSetChanCtrl(GX_COLOR0, GX_TRUE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
            GXSetChanCtrl(GX_ALPHA0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
            objGetColor(0, (u8*)&c6, (u8*)&c6 + 1, (u8*)&c6 + 2);
            GXSetChanAmbColor(GX_COLOR0, c6);
            GXSetNumChans(1);
            PSMTXConcat((f32*)Camera_GetViewMatrix(), (f32*)item.block->transform, m);
            setupToRenderMapBlock(item.block, m);
            modelRenderFn_8005d4ec(entries[i].arg0.bounds, entries[i].arg1.block, m);
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
    if (lbl_803DCE30 == 1000)
    {
        sceneDrawTransparentPolys();
        lbl_803DCE30 = 0;
    }
    t = (s32) - p[2];
    t = t < 0 ? 0 : (t > 0x7ffffff ? 0x7ffffff : t);
    lbl_8037E0C0[lbl_803DCE30 * 4] = a;
    lbl_8037E0C0[lbl_803DCE30 * 4 + 1] = b;
    lbl_8037E0C0[lbl_803DCE30 * 4 + 2] = t | 0x38000000;
    lbl_8037E0C0[lbl_803DCE30 * 4 + 3] = 7;
    lbl_803DCE30++;
}
