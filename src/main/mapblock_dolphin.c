#define OBJHITS_STATE_INDEX_S8
#define TEX_SETSHADER_U8
#include "main/map_block.h"
#include "main/texture.h"
#include "track/intersect_depth_state_api.h"
#include "track/intersect_depth_read_api.h"
#include "track/intersect_render_setup_api.h"
#include "main/hud_visibility_api.h"
#include "main/lightmap_api.h"
#include "main/shader_api.h"
#include "main/debug.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_float_helpers.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/frustum.h"
#include "main/asset_load.h"
#include "game/objects/object.h"
#include "main/gameloop_api.h"
#include "sys/objects.h"
#include "main/mm.h"
#include "main/model_light.h"
#include "main/model.h"
#include "main/model_render_instrs_api.h"
#include "main/objHitReact.h"
#include "main/objhits.h"
#undef OBJHITS_STATE_INDEX_S8
#include "main/obj_group.h"
#include "main/object_transform.h"
#include "main/vecmath.h"
#include "dolphin/mtx/mtx_legacy.h"
#include "dolphin/os/OSFastCast.h"
#include "dolphin/gx/GXCull.h"
#include "dolphin/gx/GXGeometry.h"
#include "dolphin/gx/GXLighting.h"
#include "dolphin/gx/GXBump.h"
#include "dolphin/gx/GXPixel.h"
#include "dolphin/gx/GXTev.h"
#include "dolphin/gx/GXTransform.h"
#include "main/camera.h"
#include "main/sky_state.h"
#include "main/track_dolphin.h"
#include "main/track_dolphin_api.h"
#include "main/track_dolphin_shadow_api.h"
#include "main/newshadows_shadow_api.h"
#define TRACK_BBOX_FLAGS_S8
#define TRACK_BBOX_MASK_TYPE s8
#define TRACK_BBOX_ARG10_TYPE s8
#include "main/track_bbox_api.h"
#undef TRACK_BBOX_ARG10_TYPE
#undef TRACK_BBOX_MASK_TYPE
#undef TRACK_BBOX_FLAGS_S8
#include "main/dll/player_api.h"
#include "main/pause_menu_api.h"
#include "main/pi_dolphin.h"
#include "dolphin/os/OSCache.h"
#include "main/voxmaps.h"
#include "track/intersect_api.h"
#include "main/rcp_dolphin_api.h"
#include "main/objmodel.h"
#include "main/newshadows.h"
#include "main/sky.h"
#include "main/newshadows_texture_api.h"
#include "main/atan2f_api.h"
#include "main/tex_dolphin.h"
#include "string.h"

typedef struct TrackP6Entry
{
    f32 relX0;
    f32 relY0;
    f32 relZ0;
    f32 relX1;
    f32 relY1;
    f32 relZ1;
    f32 relX2;
    f32 relY2;
    f32 relZ2;
} TrackP6Entry;

typedef struct TrackBlockDescriptor
{
    void* object;
    s16 firstTriangle;
    u8 pad06[2];
    void* currentMatrix;
    void* currentCollisionMatrix;
    void* alternateMatrix;
    void* alternateCollisionMatrix;
} TrackBlockDescriptor;

/* TrackTriangle -- the 0x4c-byte collision triangle record packed into
 * gTrackTriangleBuffer.  Plane and edge-plane normals are prebaked f32;
 * vertex coordinates are stored as s16 triplets grouped by axis
 * (x0 x1 x2 / y0 y1 y2 / z0 z1 z2), which the hit-detect code reads both
 * by field and as an s16 index off the record base. */
typedef struct TrackTriangle
{
    f32 planeD;     /* 0x00 plane equation constant */
    f32 planeN[3];  /* 0x04 plane normal xyz */
    s16 vx[3];      /* 0x10 vertex x coords */
    s16 vy[3];      /* 0x16 vertex y coords */
    s16 vz[3];      /* 0x1c vertex z coords */
    u8 pad22[2];    /* 0x22 */
    f32 edgeN0[3];  /* 0x24 edge 0 outward normal */
    f32 edgeN1[3];  /* 0x30 edge 1 outward normal */
    f32 edgeN2[3];  /* 0x3c edge 2 outward normal */
    u8 surfaceType; /* 0x48 copied into intersect-line records */
    s8 flags;       /* 0x49 0x10 = disabled, 0x4 = force */
    u8 minMaxY;     /* 0x4a lo/hi nibble: s16 index (base 0xb) of min/max height */
    u8 edgeOutBits; /* 0x4b per-edge outside bits from last query */
} TrackTriangle;

typedef union
{
    u8 u8;
    u16 u16;
    u32 u32;
    s16 s16;
    s32 s32;
    f32 f32;
} GolfWGPipe;

extern const f32 lbl_803DEC50;
extern f32 lbl_803DEBCC;
extern u32 gSunFlareScissorX;
extern u32 gSunFlareScissorY;
extern u32 gSunFlareScissorWidth;
extern u32 gSunFlareScissorHeight;
extern u8 lbl_803DCE06;
extern ModelLightStruct* gGlowLightList[];
extern FrustumPlane gViewFrustumPlanes[];
extern u8 lbl_803DCE98;
extern const f32 lbl_803DEC20;
extern int lbl_803DCE80;
extern int gMapBlockIndexCount;
extern int* gMapBlockIndexList;
extern volatile GolfWGPipe GXWGFifo : (0xCC008000);
extern int sSynthFadeUnit;
extern int renderFlags;
extern u8 colorScale;
extern f32 gSunFlareFade;
extern int gSunOcclusionSampleOffsets[];
extern f32 lbl_803DEBD4, lbl_803DEBD8, lbl_803DEBDC;
extern f32 lbl_803DEBFC, lbl_803DEBE4;
extern f32 lbl_803DEC30, lbl_803DEC34, lbl_803DEC38;
extern f32 lbl_803DEC3C, lbl_803DEC40;

void* trackGetBlockDescriptors(u32* outVal);

void mapBlockRender_setVtxDcrs(u8 doSetup, MapBlockData* block, MapShader* shader,
                               ModelRenderInstrsState* state)
{
    int* stateWords;
    u32 val;
    int pos;
    int off;
    u8* p;
    int bit;
    u32 val2;
    int pos2;
    int off2;
    u8* q;
    int bit2;
    u32 val3;
    int pos3;
    int off3;
    u8* r;
    int bit3;
    int i;

    stateWords = (int*)state;
    if (doSetup != 0)
    {
        GXClearVtxDesc();
    }
    pos = stateWords[4];
    off = pos >> 3;
    val = *(u8*)(stateWords[0] + off);
    p = (u8*)stateWords[0] + off;
    val |= p[1] << 8;
    val |= p[2] << 16;
    stateWords[4] = pos + 1;
    bit = (val >> (pos & 7)) & 1;
    if (doSetup != 0)
    {
        GXSetVtxDesc(GX_VA_POS, bit ? GX_INDEX16 : GX_INDEX8);
    }
    pos2 = stateWords[4];
    off2 = pos2 >> 3;
    val2 = *(u8*)(stateWords[0] + off2);
    q = (u8*)stateWords[0] + off2;
    val2 |= q[1] << 8;
    val2 |= q[2] << 16;
    stateWords[4] = pos2 + 1;
    bit2 = (val2 >> (pos2 & 7)) & 1;
    if (doSetup != 0)
    {
        GXSetVtxDesc(GX_VA_CLR0, bit2 ? GX_INDEX16 : GX_INDEX8);
    }
    pos3 = stateWords[4];
    off3 = pos3 >> 3;
    val3 = *(u8*)(stateWords[0] + off3);
    r = (u8*)stateWords[0] + off3;
    val3 |= r[1] << 8;
    val3 |= r[2] << 16;
    stateWords[4] = pos3 + 1;
    bit3 = (val3 >> (pos3 & 7)) & 1;
    if (doSetup != 0)
    {
        if (shader != NULL && (shader->flags & 0x80000000) == 0)
        {
            for (i = 0; i < shader->layerCount; i++)
            {
                GXSetVtxDesc(i + GX_VA_TEX0, bit3 ? GX_INDEX16 : GX_INDEX8);
            }
        }
        else
        {
            GXSetVtxDesc(GX_VA_TEX0, bit3 ? GX_INDEX16 : GX_INDEX8);
        }
    }
}

void setupToRenderMapBlock(MapBlockData* block, void* posMtx);

void setupToRenderMapBlock(MapBlockData* block, void* posMtx)
{
    f32 out[12];
    f32 tmp[12];
    f32 fc;

    GXLoadPosMtxImm((const f32 (*)[4])posMtx, GX_PNMTX0);
    PSMTXCopy((f32*)posMtx, tmp);
    fc = lbl_803DEBCC;
    tmp[3] = fc;
    tmp[7] = fc;
    tmp[11] = fc;
    GXLoadNrmMtxImm((const f32 (*)[4])tmp, GX_PNMTX0);
    PSMTXConcat((f32*)lbl_803967F0, (f32*)posMtx, out);
    GXLoadTexMtxImm((const f32 (*)[4])out, GX_TEXMTX2, GX_MTX3x4);
    GXSetArray(GX_VA_POS, block->vertices, 6);
    GXSetArray(GX_VA_CLR0, block->vertexColors, 2);
    GXSetArray(GX_VA_TEX0, block->vertexTexCoords, 4);
    GXSetArray(GX_VA_TEX1, block->vertexTexCoords, 4);
}

void renderMapBlock(MapBlockData* block, u8 type)
{
    ModelRenderInstrsState state;
    f32 m[16];
    u16 instructionCount;
    void* instructions;
    MapShader* shader;
    u8 doSetup;
    int done;
    void* viewMtx;

    shader = NULL;
    doSetup = FALSE;
    if (type == 1)
    {
        instructions = block->renderInstrsTransp;
        instructionCount = block->nRenderInstrsTransp;
    }
    else if (type == 2)
    {
        instructions = block->renderInstrsWater;
        instructionCount = block->nRenderInstrsWater;
    }
    else
    {
        instructions = block->renderInstrsMain;
        instructionCount = block->nRenderInstrsMain;
        doSetup = TRUE;
    }
    if (instructionCount == 0)
        return;
    viewMtx = Camera_GetViewMatrix();
    PSMTXConcat(viewMtx, (f32*)block->transform, m);
    if (doSetup)
        setupToRenderMapBlock(block, m);
    modelRenderInstrsState_init(&state, instructions, instructionCount << 3, instructionCount << 3);
    done = FALSE;
    while (!done)
    {
        u32 word;
        int op;
        int pos = state.bit;
        u8* bp;

        bp = state.instrs;
        bp += pos >> 3;
        word = bp[0];
        word |= bp[1] << 8;
        word |= bp[2] << 16;
        state.bit = pos + 4;
        op = (word >> (pos & 7)) & 0xf;
        switch (op)
        {
        case 3:
            mapBlockRender_setVtxDcrs(doSetup, block, shader, &state);
            break;
        case 1:
            shader = mapBlockRender_setShader(doSetup, block, &state);
            break;
        case 2:
            mapBlockRender_callList(doSetup, 0, block, shader, &state, m);
            break;
        case 4:
        {
            u32 word2;
            int cnt;
            int i;
            u8* bp2;
            int pos2 = pos + 4;
            bp2 = state.instrs;
            bp2 += pos2 >> 3;
            word2 = bp2[0];
            word2 |= bp2[1] << 8;
            word2 |= bp2[2] << 16;
            state.bit = pos2 + 4;
            cnt = (word2 >> (pos2 & 7)) & 0xf;
            for (i = 0; i < cnt; i++)
                modelRenderInstrsState_advance(&state, 8);
            break;
        }
        case 5:
            done = TRUE;
            break;
        }
    }
}

void renderGlows(void)
{
    f32 px, py, pz;
    s32 sx, sy, sz;
    u8 amb[3];
    GXColor fogCol;
    f32 sunMtx[12];
    f32 dir[3];
    f32 cam[3];
    void* viewMtx;
    u8 alpha;
    u8 sunAlpha;
    f32 sunDot;
    f32 cx, cy, cz;
    int i;
    ModelLightStruct* e;

    fogCol = *(GXColor*)&sSynthFadeUnit;
    GXSetCullMode(GX_CULL_NONE);
    Camera_RebuildProjectionMatrix();
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    textureSetupFn_800799c0();
    gxTextureFn_800794e0();
    textRenderSetupFn_80079804();
    GXSetFog(GX_FOG_NONE, 0.0f, 0.0f, 0.0f, 0.0f, fogCol);
    gxBlendFn_800789ac();
    alpha = 0xff;
    gSunFlareScissorWidth = 0;
    gSunFlareScissorHeight = 0;
    sunAlpha = skyGetSunRenderAlpha(2);
    if (sunAlpha != 0 && (renderFlags & 0x40))
    {
        viewMtx = Camera_GetViewMatrix();
        skyGetSunLightDirection(0, &dir[0], &dir[1], &dir[2]);
        cam[0] = *(f32*)((char*)viewMtx + 0x20);
        cam[1] = *(f32*)((char*)viewMtx + 0x24);
        cam[2] = *(f32*)((char*)viewMtx + 0x28);
        sunDot = PSVECDotProduct(dir, cam);
        if (sunDot > lbl_803DEBCC)
        {
            int occ;
            f32 fade;
            skyBuildSunModelMatrix((f32(*)[4])sunMtx);
            Camera_ProjectWorldPointWithOffset(sunMtx[3], sunMtx[7], sunMtx[11], lbl_803DEBD4, &px, &py, &pz);
            Camera_NdcToScreen(px, py, pz, &sx, &sy, &sz);
            gSunFlareScissorX = sx - 0x10;
            gSunFlareScissorWidth = 0x20;
            gSunFlareScissorY = sy - 0x10;
            gSunFlareScissorHeight = 0x20;
            if ((int)gSunFlareScissorX < 0)
                gSunFlareScissorX = 0;
            else if ((int)gSunFlareScissorX > 0x280)
                gSunFlareScissorX = 0x280;
            if ((int)gSunFlareScissorY < 0)
                gSunFlareScissorY = 0;
            else if ((int)gSunFlareScissorY > 0x1e0)
                gSunFlareScissorY = 0x1e0;
            if ((int)gSunFlareScissorX + 0x20 > 0x280)
                gSunFlareScissorWidth = 0x280 - gSunFlareScissorX;
            if ((int)gSunFlareScissorY + 0x20 > 0x1e0)
                gSunFlareScissorHeight = 0x1e0 - gSunFlareScissorY;
            occ = 0;
            for (i = 0; i < 5; i++)
            {
                int d = depthReadRequestPoll(sx + gSunOcclusionSampleOffsets[i * 2],
                                             sy + gSunOcclusionSampleOffsets[i * 2 + 1], (void*)i);
                if (sz <= d && pauseMenuGetState() == 0)
                    occ++;
            }
            fade = (f32)(u32)occ / lbl_803DEBE4 - gSunFlareFade;
            if (fade > lbl_803DEC30)
                fade = lbl_803DEC30;
            else if (fade < lbl_803DEC34)
                fade = lbl_803DEC34;
            gSunFlareFade = gSunFlareFade + fade;
            sunDot = sunDot * gSunFlareFade;
            if (sunDot > lbl_803DEBCC)
            {
                PSMTXConcat(viewMtx, sunMtx, sunMtx);
                GXLoadPosMtxImm((const f32 (*)[4])sunMtx, GX_PNMTX0);
                GXSetCurrentMtx(GX_PNMTX0);
                selectTexture((Texture*)((int)skyGetSkyTexture()), 0);
                getAmbientColor(0, &amb[0], &amb[1], &amb[2]);
                sunDot = (f32)(u32)sunAlpha * sunDot;
                _gxSetTevColor2(amb[0], amb[1], amb[2], (int)(lbl_803DEBFC * sunDot));
                alpha = lbl_803DEBD8 - lbl_803DEC38 * sunDot;
                fade = lbl_803DEC3C * sunDot;
                sunDot = fade * lbl_803DEC40;
                GXBegin(GX_QUADS, GX_VTXFMT2, 4);
                GXWGFifo.f32 = -sunDot;
                GXWGFifo.f32 = -sunDot;
                GXWGFifo.f32 = lbl_803DEBCC;
                GXWGFifo.f32 = lbl_803DEBCC;
                GXWGFifo.f32 = lbl_803DEBCC;
                GXWGFifo.f32 = sunDot;
                GXWGFifo.f32 = -sunDot;
                GXWGFifo.f32 = lbl_803DEBCC;
                GXWGFifo.f32 = lbl_803DEBDC;
                GXWGFifo.f32 = lbl_803DEBCC;
                GXWGFifo.f32 = sunDot;
                GXWGFifo.f32 = sunDot;
                GXWGFifo.f32 = lbl_803DEBCC;
                GXWGFifo.f32 = lbl_803DEBDC;
                GXWGFifo.f32 = lbl_803DEBDC;
                GXWGFifo.f32 = -sunDot;
                GXWGFifo.f32 = sunDot;
                GXWGFifo.f32 = lbl_803DEBCC;
                GXWGFifo.f32 = lbl_803DEBCC;
                GXWGFifo.f32 = lbl_803DEBDC;
            }
        }
    }
    colorScale = alpha;
    if (lbl_803DCE06 != 0)
    {
        for (i = 0; i < lbl_803DCE06; i++)
        {
            int d;
            e = gGlowLightList[i];
            Camera_ProjectWorldPointWithOffset(e->worldX - playerMapOffsetX, e->worldY, e->worldZ - playerMapOffsetZ,
                                               e->glowProjectionRadius, &px, &py, &pz);
            Camera_NdcToScreen(px, py, pz, &sx, &sy, &sz);
            d = depthReadRequestPoll(sx, sy, e);
            if (sz <= d && pauseMenuGetState() == 0)
                e->glowAlphaStep = 0x10;
            else
                e->glowAlphaStep = -0x10;
        }
        GXSetCurrentMtx(GX_IDENTITY);
        gxTextureFn_800794e0();
        gxBlendFn_800789ac();
        for (i = 0; i < lbl_803DCE06; i++)
        {
            e = gGlowLightList[i];
            if (e->glowAlpha != 0)
            {
                f32 f = e->activeIntensity;
                selectTexture((Texture*)((int)e->glowTexture), 0);
                _gxSetTevColor2((int)((f32)(u32)e->glowColor[0] * e->activeIntensity),
                                (int)((f32)(u32)e->glowColor[1] * e->activeIntensity),
                                (int)((f32)(u32)e->glowColor[2] * e->activeIntensity),
                                (u8)((int)(e->glowColor[3] * e->glowAlpha) >> 8));
                GXBegin(GX_QUADS, GX_VTXFMT2, 4);
                cz = e->viewZ;
                cy = e->viewY - e->glowScale;
                cx = e->viewX - e->glowScale;
                GXWGFifo.f32 = cx;
                GXWGFifo.f32 = cy;
                GXWGFifo.f32 = cz;
                GXWGFifo.f32 = lbl_803DEBCC;
                GXWGFifo.f32 = lbl_803DEBCC;
                cz = e->viewZ;
                cy = e->viewY - e->glowScale;
                cx = e->viewX + e->glowScale;
                GXWGFifo.f32 = cx;
                GXWGFifo.f32 = cy;
                GXWGFifo.f32 = cz;
                GXWGFifo.f32 = lbl_803DEBDC;
                GXWGFifo.f32 = lbl_803DEBCC;
                cz = e->viewZ;
                cy = e->viewY + e->glowScale;
                cx = e->viewX + e->glowScale;
                GXWGFifo.f32 = cx;
                GXWGFifo.f32 = cy;
                GXWGFifo.f32 = cz;
                GXWGFifo.f32 = lbl_803DEBDC;
                GXWGFifo.f32 = lbl_803DEBDC;
                cz = e->viewZ;
                cy = e->viewY + e->glowScale;
                cx = e->viewX - e->glowScale;
                GXWGFifo.f32 = cx;
                GXWGFifo.f32 = cy;
                GXWGFifo.f32 = cz;
                GXWGFifo.f32 = lbl_803DEBCC;
                GXWGFifo.f32 = lbl_803DEBDC;
            }
        }
        GXSetCurrentMtx(GX_PNMTX0);
    }
}

void getSunFlareScissorRect(int* outX, int* outY, int* outWidth, int* outHeight)
{
    *outX = gSunFlareScissorX;
    *outY = gSunFlareScissorY;
    *outWidth = gSunFlareScissorWidth;
    *outHeight = gSunFlareScissorHeight;
}

static inline int isGlowInFrustum(ModelLightStruct* light)
{
    FrustumPlane* plane;
    u8 i;
    f32 offsetX;
    f32 offsetZ;
    f32 bias;

    i = 0;
    offsetZ = playerMapOffsetZ;
    offsetX = playerMapOffsetX;
    bias = lbl_803DEBCC;
    for (; i < 5; i++)
    {
        f32 dot;
        plane = &gViewFrustumPlanes[i];
        dot = light->worldY * plane->normalY + plane->normalX * (light->worldX - offsetX) +
                  plane->normalZ * (light->worldZ - offsetZ) + plane->distance + bias;
        if (dot < bias)
        {
            return 0;
        }
    }
    return 1;
}

void queueGlowRender(ModelLightStruct* light)
{
    int visible;
    u8 idx;

    if (lbl_803DCE06 >= 100)
        return;

    visible = isGlowInFrustum(light);
    {
        u8 vis = visible;
        if (vis == 0 && light->glowAlpha == 0)
            return;
        if (vis == 0)
        {
            light->glowAlphaStep = -0x10;
        }
    }
    idx = lbl_803DCE06++;
    gGlowLightList[idx] = light;
}

void trackPackVector(short* out, float* vec)
{
    int yScaled;
    int zScaled;

    yScaled = (int)(lbl_803DEC50 * vec[1]);
    zScaled = (int)(lbl_803DEC50 * vec[2]);
    *out = (short)(int)(lbl_803DEC50 * *vec);
    out[1] = yScaled;
    out[2] = zScaled;
}

void trackUnpackVector(s16* in, f32* out)
{
    out[0] = (f32)(s32)in[0] * lbl_803DEC20;
    out[1] = (f32)(s32)in[1] * lbl_803DEC20;
    out[2] = (f32)(s32)in[2] * lbl_803DEC20;
}

/* fn_80067B84 -- gather model triangles overlapping a swept bbox into the
 * hit-detect triangle buffer at cur (0x4c-byte records); returns advanced
 * cursor. */
u32 trackGetPackedSurfaceType(int* obj)
{
    u32 v = obj[4];
    v &= 0x00FF0000;
    return v >> 16;
}

int mapBlockGetPolygonGroupType(void* obj)
{
    return (*(u32*)&((GameObject*)obj)->anim.localPosY & 0xff000000) >> 24;
}

int mapBlockCountTrianglesByType(MapBlockData* block, int type)
{
    int entry;
    int offset;
    int total;
    int i;
    int count;
    total = 0;
    offset = 0;
    count = block->polyGroupCount;
    for (i = 0; i < count; i++)
    {
        entry = (int)block->polygonGroups + offset;
        if (type == (int)((*(u32*)(entry + 0x10) & 0xff000000) >> 24))
        {
            total += *(u16*)(entry + 0x14) - *(u16*)entry;
        }
        offset += 0x14;
    }
    return total;
}

void* mapBlockGetPolygon(int* obj, int idx)
{
    return (char*)((int**)obj)[0x4c / 4] + idx * 8;
}

void* mapBlockGetPolygonGroup(void* obj, int idx)
{
    return (char*)((int**)obj)[0x50 / 4] + idx * 0x14;
}

void* mapBlockGetEdge(int* obj, int idx)
{
    return (char*)((int**)obj)[0x68 / 4] + idx * 0x1c;
}

MapShader* mapBlockGetShader(MapBlockData* obj, int idx)
{
    return obj->shaders + idx;
}

void MapBlock_initShaders(MapBlockData* block)
{
    int i;
    int j;
    int ref;
    MapShader* sh;
    for (i = 0; i < block->shaderCount; i++)
    {
        sh = &block->shaders[i];
        for (j = 0; j < sh->layerCount; j++)
        {
            ref = sh->layers[j].textureIndex;
            if (ref != -1)
            {
                sh->layers[j].texture = block->textures[ref].texture;
                ref = sh->layers[j].overrideType;
                if ((u32)ref != 0u)
                {
                    mapTextureOverrideAcquire(sh->layers[j].texture, 0, ref);
                }
            }
            else
            {
                sh->layers[j].texture = NULL;
            }
            sh->layers[j].scrollMtx = 0xff;
        }
        ref = sh->auxTextureIndex;
        if (ref != -1)
        {
            sh->auxTexture = block->textures[ref].texture;
        }
        else
        {
            sh->auxTexture = NULL;
        }
    }
}

static inline void* mapBlockRelocatePointer(MapBlockData* block, void* offset)
{
    return (u8*)block + (u32)offset;
}

void MapBlock_init(MapBlockData* block)
{
    int i;

    if (block->textures != NULL)
        block->textures = mapBlockRelocatePointer(block, block->textures);
    if (block->gcPolygons != NULL)
        block->gcPolygons = mapBlockRelocatePointer(block, block->gcPolygons);
    if (block->polygonGroups != NULL)
        block->polygonGroups = mapBlockRelocatePointer(block, block->polygonGroups);
    block->vertices = mapBlockRelocatePointer(block, block->vertices);
    block->vertexColors = mapBlockRelocatePointer(block, block->vertexColors);
    block->vertexTexCoords = mapBlockRelocatePointer(block, block->vertexTexCoords);
    if (block->renderInstrsMain != NULL)
        block->renderInstrsMain = mapBlockRelocatePointer(block, block->renderInstrsMain);
    if (block->renderInstrsTransp != NULL)
        block->renderInstrsTransp = mapBlockRelocatePointer(block, block->renderInstrsTransp);
    if (block->renderInstrsWater != NULL)
        block->renderInstrsWater = mapBlockRelocatePointer(block, block->renderInstrsWater);
    block->displayLists = mapBlockRelocatePointer(block, block->displayLists);
    if (block->shaders != NULL)
        block->shaders = mapBlockRelocatePointer(block, block->shaders);

    for (i = 0; i < block->edgeCount; i++)
    {
        block->displayLists[i].dlist = mapBlockRelocatePointer(block, block->displayLists[i].dlist);
    }
}

void MapBlock_initHits(MapBlockData* block, int index)
{
    int i;
    int* table = (int*)lbl_803DCE80;
    int fileOff = table[index];
    int size = table[index + 1] - fileOff;
    MapHitLine* entry;
    s16 value;

    if (size > 0)
    {
        block->hits = mmAlloc(size, 5, 0);
        fileLoadToBufferOffset(MLDF_FILEID_HITS_BIN, block->hits, fileOff, size);
    }
    block->hitCount = (u32)size / sizeof(MapHitLine);
    i = 0;
    while (i < block->hitCount)
    {
        entry = &block->hits[i];
        if (entry->x[0] < 0 || (value = entry->x[1]) < 0 || entry->x[0] > 0x280 || value > 0x280)
        {
            entry->kind = 0x40;
        }
        entry = &block->hits[i];
        if (entry->z[0] < 0 || (value = entry->z[1]) < 0 || entry->z[0] > 0x280 || value > 0x280)
        {
            entry->kind = 0x40;
        }
        i++;
    }
    block->auxData = NULL;
    block->unk9E = 0;
    block->flags4 &= ~0x40;
}

MapBlockData* MapBlock_loadFromFile(int blockId)
{
    int compressedLen;
    int decompressedSize;
    void* buf;
    int blockOff = 0;
    int* table;
    int tableEntry;
    if (blockId <= gMapBlockIndexCount)
    {
        table = gMapBlockIndexList;
        if (table != 0)
        {
            tableEntry = table[blockId];
            if (tableEntry != -1)
            {
                if (tableEntry != 0 || table[blockId + 1] != 0)
                {
                    blockOff = tableEntry;
                    checkLoadBlock(tableEntry, &compressedLen, &decompressedSize);
                }
                else
                {
                    return 0;
                }
            }
        }
    }
    else
    {
        return 0;
    }
    if (compressedLen <= 0)
    {
        return 0;
    }
    if (decompressedSize > 0x32000)
    {
        return 0;
    }
    buf = mmAlloc(decompressedSize, 5, 0);
    if (buf == 0)
    {
        return 0;
    }
    loadAndDecompressDataFile(MLDF_FILEID_BLOCKS_BIN_A, buf, blockOff, compressedLen, 0, 0, 0);
    return buf;
}

void gxErrorFn_80060b40(void)
{
    int n;
    int i;

    i = 0;
    n = lbl_803DCE98;
    for (; i < n; i++)
    {
    }
}

int return0_80060B90(void* wpad0)
{
    return 0x0;
}

void mapGetBlocks(void** outLayerTables, u32* outBlocks)
{
    *outLayerTables = gMapBlockLayerTables;
    *outBlocks = (u32)gMapBlocks;
}

void mapClearBlockEdgeFlags(void)
{
    int i;
    int j;
    MapBlockData* block;

    for (i = 0; i < lbl_803DCE98; i++)
    {
        block = gMapBlocks[i];
        if (block != NULL)
        {
            for (j = 0; j < block->edgeCount; j++)
            {
                block->displayLists[j].flags = 0;
            }
        }
    }
}

int fn_80060C14(int* obj, int triBuf, void* planesOut, int vertsOut, int p7, f32 offX, f32 offZ, int p8, int kindMask)
{
    int j;
    f32 lm[12];
    u8* descBytes = trackGetBlockDescriptors((u32*)&j);
    u8* end = descBytes + j * 0x18;
    int total;
    int grp;
    int outOff;

    grp = 0;
    outOff = 0;
    j = grp;
    total = 0;
    kindMask = kindMask ? 4 : 8;
    for (; descBytes < end; descBytes += 0x18)
    {
        u32 id = *(u32*)descBytes;
        if (id == 0 || id == *(u32*)&((GameObject*)obj)->anim.parent)
        {
            f32 fx = ((GameObject*)obj)->anim.localPosX;
            f32 fz = ((GameObject*)obj)->anim.localPosZ;
            f32* outA;

            if (id == 0)
            {
                fx -= offX;
                fz -= offZ;
            }
            j = (s16)((TrackBlockDescriptor*)descBytes)->firstTriangle;
            outA = (f32*)((char*)planesOut + outOff);
            while (j < (s16)((TrackBlockDescriptor*)descBytes)[1].firstTriangle && grp < 0x4b0 && total < 0xe10)
            {
                if (kindMask & ((TrackTriangle*)triBuf + j)->flags)
                {
                    ((TrackP6Entry*)vertsOut)->relX0 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vx[0]) - fx;
                    ((TrackP6Entry*)vertsOut)->relY0 =
                        __OSs16tof32(&((TrackTriangle*)triBuf + j)->vy[0]) - ((GameObject*)obj)->anim.localPosY;
                    ((TrackP6Entry*)vertsOut)->relZ0 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vz[0]) - fz;
                    ((TrackP6Entry*)vertsOut)->relX1 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vx[1]) - fx;
                    ((TrackP6Entry*)vertsOut)->relY1 =
                        __OSs16tof32(&((TrackTriangle*)triBuf + j)->vy[1]) - ((GameObject*)obj)->anim.localPosY;
                    ((TrackP6Entry*)vertsOut)->relZ1 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vz[1]) - fz;
                    ((TrackP6Entry*)vertsOut)->relX2 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vx[2]) - fx;
                    ((TrackP6Entry*)vertsOut)->relY2 =
                        __OSs16tof32(&((TrackTriangle*)triBuf + j)->vy[2]) - ((GameObject*)obj)->anim.localPosY;
                    ((TrackP6Entry*)vertsOut)->relZ2 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vz[2]) - fz;
                    outA[0] = ((TrackTriangle*)triBuf + j)->planeN[0];
                    outA[1] = ((TrackTriangle*)triBuf + j)->planeN[1];
                    outA[2] = ((TrackTriangle*)triBuf + j)->planeN[2];
                    *(u8*)((char*)outA + 0x10) = *(u8*)&((TrackTriangle*)triBuf + j)->flags;
                    vertsOut += 0x24;
                    total += 3;
                    outA = (f32*)((char*)outA + 0x14);
                    grp += 1;
                    outOff += 0x14;
                }
                j++;
            }
        }
        else
        {
            f32* m = *(f32**)((char*)descBytes + 0xc);
            f32* p6start;
            int totalStart;
            f32* outA;

            lm[0] = m[0];
            lm[1] = m[4];
            lm[2] = m[8];
            lm[3] = m[12] - ((GameObject*)obj)->anim.localPosX;
            lm[4] = m[1];
            lm[5] = m[5];
            lm[6] = m[9];
            lm[7] = m[13] - ((GameObject*)obj)->anim.localPosY;
            lm[8] = m[2];
            lm[9] = m[6];
            lm[10] = m[10];
            lm[11] = m[14] - ((GameObject*)obj)->anim.localPosZ;
            p6start = (f32*)vertsOut;
            totalStart = total;
            j = (s16)((TrackBlockDescriptor*)descBytes)->firstTriangle;
            outA = (f32*)((char*)planesOut + outOff);
            while (j < (s16)((TrackBlockDescriptor*)descBytes)[1].firstTriangle && grp < 0x4b0 && total < 0xe10)
            {
                if (kindMask & ((TrackTriangle*)triBuf + j)->flags)
                {
                    ((TrackP6Entry*)vertsOut)->relX0 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vx[0]);
                    ((TrackP6Entry*)vertsOut)->relY0 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vy[0]);
                    ((TrackP6Entry*)vertsOut)->relZ0 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vz[0]);
                    ((TrackP6Entry*)vertsOut)->relX1 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vx[1]);
                    ((TrackP6Entry*)vertsOut)->relY1 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vy[1]);
                    ((TrackP6Entry*)vertsOut)->relZ1 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vz[1]);
                    ((TrackP6Entry*)vertsOut)->relX2 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vx[2]);
                    ((TrackP6Entry*)vertsOut)->relY2 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vy[2]);
                    ((TrackP6Entry*)vertsOut)->relZ2 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vz[2]);
                    outA[0] = ((TrackTriangle*)triBuf + j)->planeN[0];
                    outA[1] = ((TrackTriangle*)triBuf + j)->planeN[1];
                    outA[2] = ((TrackTriangle*)triBuf + j)->planeN[2];
                    *(u8*)((char*)outA + 0x10) = *(u8*)&((TrackTriangle*)triBuf + j)->flags;
                    vertsOut += 0x24;
                    total += 3;
                    outA = (f32*)((char*)outA + 0x14);
                    grp += 1;
                    outOff += 0x14;
                }
                j++;
            }
            if (totalStart < total)
            {
                PSMTXMultVecArray(lm, p6start, p6start, total - totalStart);
            }
        }
    }
    return grp;
}
