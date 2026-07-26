#include "dolphin/os/OSReport.h"
#include "main/frame_timing.h"
#include "track/intersect_depth_state_api.h"
#include "main/asset_load.h"
#include "main/map_load.h"
#include "main/shader_api.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_float_helpers.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/vecmath.h"
#include "main/warpvec.h"
#include "main/zlb.h"
#include "main/dll/cloudaction_interface.h"
#include "main/texture.h"
#include "game/objects/object.h"
#include "main/gameloop_api.h"
#include "sys/objects/lifecycle.h"
#include "main/mapEvent.h"
#include "main/model_light.h"
#include "main/model.h"
#include "main/map_romlist_page.h"
#include "main/map_block.h"
#include "main/shader_init_api.h"
#include "main/newclouds.h"
#include "main/rcp_dolphin.h"
#include "main/rcp_dolphin_api.h"
#include "main/rcp_dolphin_render_api.h"
#include "main/camera.h"
#include "main/loaded_file_flags.h"
#include "main/pi_dolphin.h"
#include "main/screen_transition.h"
#include "main/sky_api.h"
#include "main/sky_interface.h"
#include "main/mm.h"
#include "main/dll/tricky_api.h"
#include "main/dll/dll_0000_gameui_api.h"
#include "main/dll/savegame_env_api.h"
#include "dolphin/os/OSCache.h"
#include "dolphin/os/OSInterrupt.h"
#include "dolphin/mtx.h"
#include "dolphin/gx/GXDispList.h"
#include "dolphin/gx/GXFrameBuffer.h"
#include "dolphin/gx/GXBump.h"
#include "dolphin/gx/GXGet.h"
#include "dolphin/gx/GXGeometry.h"
#include "dolphin/gx/GXLighting.h"
#include "dolphin/gx/GXManage.h"
#include "dolphin/gx/GXPixel.h"
#include "dolphin/gx/GXTev.h"
#include "dolphin/gx/GXTexture.h"
#include "dolphin/gx/GXTransform.h"
#include "main/dll/modgfx.h"
#include "main/newshadows.h"
#include "main/pi_dolphin_texture_api.h"
#include "main/gx_scissor_api.h"
#include "string.h"

u8 gRcpDistortSlotIndex;
u8 gRcpDistortGroup;
void* gRcpDistortTexture;
u32 gRcpWarpDistortListSize;
u8 gRcpWarpDistortListBuilt;

GXColor gRcpDistortAmbColor = {0, 0, 0, 0};
GXColor gRcpDistortMatColor = {0xff, 0xff, 0xff, 0xff};
typedef struct F32Pair
{
    f32 lo;
    f32 hi;
} F32Pair;
extern u8 gRcpWarpDistortListBuilt;
extern u32 gRcpWarpDistortListSize;
extern const f32 lbl_803DEB64;
typedef struct RcpDistortSlot
{
    u8* texture;   // 0x00
    int model;     // 0x04
    int unk8;      // 0x08
    u8 colR;       // 0x0c
    u8 colG;       // 0x0d
    u8 colB;       // 0x0e
    u8 unkF;       // 0x0f
    f32 params[2]; // 0x10
    u8 scaleR;     // 0x18
    u8 scaleB;     // 0x19
    u8 group;      // 0x1a
    u8 mode;       // 0x1b
} RcpDistortSlot;
STATIC_ASSERT(sizeof(RcpDistortSlot) == 0x1c);
STATIC_ASSERT(offsetof(RcpDistortSlot, params) == 0x10);
STATIC_ASSERT(offsetof(RcpDistortSlot, group) == 0x1a);
STATIC_ASSERT(offsetof(RcpDistortSlot, mode) == 0x1b);

extern RcpDistortSlot gRcpDistortSlots[6];
extern u8 gRcpDistortSlotIndex;
extern void* gRcpDistortTexture;
extern u8 gRcpDistortGroup;
extern f32 gRcpScreenWidth;
extern f32 gRcpScreenHeight;
void* textureLoadAsset(int asset);
void* textureAlloc(u16 w, u16 h, int fmt, u8 mip, u8 maxLod, u8 wrapS, u8 wrapT, u8 minFilter, u8 magFilter);
static inline void gxLoadObjectLights(GameObject* model, ModelLightStruct** lights);


#define RCP_DISTORT_TEXTURE_ID 0x5dc

extern u8 gRcpWarpDistortDisplayList[0x6640];

void Rcp_SetupDistortionRenderState(void);

static inline void gxLoadObjectLights(GameObject* model, ModelLightStruct** lights)
{
    s32 count;
    int n;
    modelLightStruct_selectObjectLights(model, lights, 8, &count, 4);
    modelLightChannels_reset(1);
    modelLightChannel_configure(0, 0, 0);
    for (n = 0; n < count; n++)
    {
        modelLightStruct_loadChannelLight(0, lights[n], model);
    }
    modelLightChannels_applyGXControls();
}

void textureFn_800524ec(GXColor* param);
void Rcp_ApplyTextureStageCounts(void);
void Rcp_ResetTextureStageState(void);
int Rcp_SetupDistortionLights(int model, f32* params);
void Rcp_DrawWarpDistortionMesh(f32 a, f32 b) /* params unused; callers pass (i*32, 0.0f) */
{
    f32 x0;
    f32 y;
    f32 ySq;
    f32 step;
    f32 half;
    f32 x1;
    f32 span;
    f32 distSq;
    f32 bulge;
    f32 col0;
    f32 col1;
    u32 i;
    u32 j;

    if (gRcpWarpDistortListBuilt == 0)
    {
        GXSetMisc(GX_MT_XF_FLUSH, 0);
        DCInvalidateRange(gRcpWarpDistortDisplayList, 0x6640);
        GXBeginDisplayList(gRcpWarpDistortDisplayList, 0x6640);
        i = 0;
        span = 15.0f;
        half = 1.0f;
        step = 2.0f;
        for (; i < 0x10; i++)
        {
            GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT4, 0x22);
            j = 0;
            for (; j <= 0x10; j++)
            {
                col0 = step * (f32)i;
                col1 = step * (f32)(i + 1);
                x0 = col0 / span - half;
                x1 = col1 / span - half;
                y = (step * (f32)j) / span - half;
                ySq = y * y;
                distSq = x0 * x0 + ySq;
                if (distSq < half)
                {
                    bulge = sqrtf(half - distSq);
                }
                else
                {
                    bulge = 0.0f;
                }
                *(volatile f32*)0xCC008000 = x0;
                *(volatile f32*)0xCC008000 = y;
                *(volatile f32*)0xCC008000 = lbl_803DEB64;
                *(volatile f32*)0xCC008000 = x0;
                *(volatile f32*)0xCC008000 = y;
                *(volatile f32*)0xCC008000 = bulge;
                distSq = x1 * x1 + ySq;
                if (distSq < half)
                {
                    bulge = sqrtf(half - distSq);
                }
                else
                {
                    bulge = 0.0f;
                }
                *(volatile f32*)0xCC008000 = x1;
                *(volatile f32*)0xCC008000 = y;
                *(volatile f32*)0xCC008000 = lbl_803DEB64;
                *(volatile f32*)0xCC008000 = x1;
                *(volatile f32*)0xCC008000 = y;
                *(volatile f32*)0xCC008000 = bulge;
            }
        }
        gRcpWarpDistortListSize = GXEndDisplayList();
        gRcpWarpDistortListBuilt = 1;
        GXSetMisc(GX_MT_XF_FLUSH, 8);
    }
    GXCallDisplayList(gRcpWarpDistortDisplayList, gRcpWarpDistortListSize);
}
int Rcp_SetupDistortionLights(int model, f32* params)
{
    ModelLightStruct* la;
    ModelLightStruct* lb;
    la = skyGetSunLight();
    lb = skyGetMoonLight();
    if (la == NULL || lb == NULL)
    {
        return 0;
    }
    modelLightChannels_reset(1);
    modelLightChannel_configure(0, 1, 0);
    modelLightChannel_configure(2, 0, 0);
    modelLightStruct_setSpecularAttenuation(la, params[0], 0.0f);
    modelLightStruct_setSpecularColor(la, 0xff, 0, 0, 0xff);
    modelLightStruct_loadChannelLight(0, la, (GameObject*)model);
    modelLightStruct_setSpecularAttenuation(la, params[1], 0.0f);
    modelLightStruct_setSpecularColor(la, 0, 0, 0xff, 0xff);
    modelLightStruct_loadChannelLight(0, la, (GameObject*)model);
    modelLightStruct_setAngularAttenuation(la, 1.5f, 0.0f, 0.0f);
    modelLightStruct_loadChannelLight(2, la, (GameObject*)model);
    modelLightChannel_configure(1, 1, 0);
    modelLightChannel_configure(3, 0, 0);
    modelLightStruct_setSpecularAttenuation(lb, params[0], 0.0f);
    modelLightStruct_setSpecularColor(lb, 0xff, 0, 0, 0xff);
    modelLightStruct_loadChannelLight(1, lb, (GameObject*)model);
    modelLightStruct_setSpecularAttenuation(lb, params[1], 0.0f);
    modelLightStruct_setSpecularColor(lb, 0, 0, 0xff, 0xff);
    modelLightStruct_loadChannelLight(1, lb, (GameObject*)model);
    modelLightStruct_setAngularAttenuation(lb, 0.5f, 0.0f, 0.0f);
    modelLightStruct_loadChannelLight(3, lb, (GameObject*)model);
    modelLightChannels_applyGXControls();
    modelLightStruct_setAngularAttenuation(la, 1.0f, 0.0f, 0.0f);
    modelLightStruct_setAngularAttenuation(lb, 1.0f, 0.0f, 0.0f);
    return 0;
}
void Rcp_SetupDistortionRenderState(void)
{
    f32 omtx[4][4];
    f32 pmtx[3][4];
    GXSetViewport(0.0f, 0.0f, 32.0f,
                  32.0f, 0.0f, 1.0f);
    GXSetScissor(0, 0, 32, 32);
    GXSetDispCopySrc(0, 0, 32, 32);
    GXSetDispCopyDst(32, 32);
    GXSetTexCopySrc(0, 0, 32, 32);
    C_MTXOrtho(omtx, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 15.0f);
    GXSetProjection(omtx, 1);
    GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
    gxSetZMode_(0, GX_EQUAL, 0);
    GXSetCullMode(GX_CULL_NONE);
    gxSetPeControl_ZCompLoc_(1);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);
    PSMTXIdentity(pmtx);
    GXLoadPosMtxImm(pmtx, GX_PNMTX0);
    GXLoadNrmMtxImm(pmtx, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
}

void Rcp_UpdateDistortionTextures(void)
{
    union
    {
        Mtx m;
        f64 a8;
    } mtxu;
#define mtx mtxu.m
    ModelLightStruct* lights[8];
    GXColor outColor;
    GXColor texColor;
    GXColor matColor;
    u8* e;
    u8* slots[1];
    int i;
    int clearSlot;
    u8 group;
    int k;
    int model[1];
    u8* tex;

    Rcp_SetupDistortionRenderState();
    PSMTXScale(mtx, 0.5f, -0.5f, 0.5f);
    mtx[0][3] = 0.5f;
    mtx[1][3] = 0.5f;
    GXLoadTexMtxImm(mtx, GX_TEXMTX0, GX_MTX2x4);
    GXSetChanAmbColor(GX_COLOR0A0, gRcpDistortAmbColor);
    GXSetChanAmbColor(GX_COLOR1A1, gRcpDistortAmbColor);
    GXSetTexCopyDst(0x20, 0x20, GX_TF_RGBA8, GX_FALSE);
    modelTextureFn_80089970(2);
    i = 0;
    slots[0] = (u8*)gRcpDistortSlots;
    for (; i < 6; i++)
    {
        tex = ((RcpDistortSlot*)slots[0])[i].texture;
        if (((Texture*)tex)->refCount != 0 && ((RcpDistortSlot*)slots[0])[i].mode == 1 &&
            gRcpDistortGroup == ((RcpDistortSlot*)slots[0])[i].group)
        {
            matColor.r = (((RcpDistortSlot*)slots[0])[i].colR * ((RcpDistortSlot*)slots[0])[i].scaleR) >> 8;
            matColor.g = 0;
            matColor.b = (((RcpDistortSlot*)slots[0])[i].colB * ((RcpDistortSlot*)slots[0])[i].scaleB) >> 8;
            matColor.a = 0xff;
            GXSetChanMatColor(GX_COLOR0A0, matColor);
            GXSetChanMatColor(GX_COLOR1A1, matColor);
            Rcp_SetupDistortionLights(((RcpDistortSlot*)slots[0])[i].model, ((RcpDistortSlot*)slots[0])[i].params);
            Rcp_ResetTextureStageState();
            textureFn_8004ff20(gRcpDistortTexture, (f32*)mtx, &texColor, 0);
            Rcp_ApplyTextureStageCounts();
            Rcp_DrawWarpDistortionMesh((f32)(i * 0x20), 0.0f);
            GXCopyTex(((RcpDistortSlot*)slots[0])[i].texture + 0x60, 0);
            tex = ((RcpDistortSlot*)slots[0])[i].texture;
            if (((Texture*)tex)->preloaded != 0)
            {
                GXPreLoadEntireTexture(textureGetGXTexObj((Texture*)tex),
                                       textureGetGXTexRegion((Texture*)tex));
            }
        }
    }
    Rcp_ResetTextureStageState();
    textureFn_800524ec(&gRcpDistortMatColor);
    Rcp_ApplyTextureStageCounts();
    GXSetChanMatColor(GX_COLOR0, gRcpDistortMatColor);
    clearSlot = 5;
    k = 5;
    e = (u8*)gRcpDistortSlots + 0x8c;
    group = gRcpDistortGroup;
    for (; k >= 0; k--)
    {
        if (*(u16*)(*(u8**)e + 0xe) != 0 && e[0x1b] == 0 && group == e[0x1a])
        {
            clearSlot = k;
            break;
        }
        e -= 0x1c;
    }
    i = 0;
    for (; i < 6; i++)
    {
        if (((Texture*)((RcpDistortSlot*)slots[0])[i].texture)->refCount != 0 &&
            ((RcpDistortSlot*)slots[0])[i].mode == 0 && gRcpDistortGroup == ((RcpDistortSlot*)slots[0])[i].group)
        {
            model[0] = ((RcpDistortSlot*)slots[0])[i].model;
            modelTextureFn_80089970(2 - (i - 3));
            gxLoadObjectLights((GameObject*)model[0], lights);
            lightGetColor(0, &outColor.r, &outColor.g, &outColor.b);
            GXSetChanAmbColor(GX_COLOR0, outColor);
            Rcp_DrawWarpDistortionMesh((f32)(i * 0x20), 0.0f);
            GXCopyTex(((RcpDistortSlot*)slots[0])[i].texture + 0x60,
                      (i == clearSlot) ? GX_TRUE : GX_FALSE);
            tex = ((RcpDistortSlot*)slots[0])[i].texture;
            if (((Texture*)tex)->preloaded != 0)
            {
                GXPreLoadEntireTexture(textureGetGXTexObj((Texture*)tex),
                                       textureGetGXTexRegion((Texture*)tex));
            }
        }
    }
    GXSetViewport(0.0f, 0.0f, gRcpScreenWidth, gRcpScreenHeight,
                  0.0f, 1.0f);
    GXSetScissor(0, 0, 0x280, 0x1e0);
    GXSetDispCopySrc(0, 0, 0x280, 0x1e0);
    GXSetDispCopyDst(0x280, 0x1e0);
    GXSetTexCopySrc(0, 0, 0x280, 0x1e0);
    Camera_ApplyFullViewport();
    gRcpDistortGroup = 0;
}
void ShaderDef_free(int* def)
{
    void* s;
    void* p1 = (void*)def[0];
    int i;
    void* p2;
    int j;

    if (p1 != NULL)
    {
        for (i = 0; i < 6; i++)
        {
            s = gRcpDistortSlots[i].texture;
            if (((Texture*)s)->refCount != 0 && s == p1)
            {
                (((Texture*)gRcpDistortSlots[i].texture)->refCount)--;
                break;
            }
        }
    }
    p2 = (void*)def[1];
    if (p2 == NULL)
        return;
    for (j = 0; j < 6; j++)
    {
        if (((Texture*)gRcpDistortSlots[j].texture)->refCount != 0 && gRcpDistortSlots[j].texture == p2)
        {
            (((Texture*)gRcpDistortSlots[j].texture)->refCount)--;
            return;
        }
    }
}

void shaderInit(u8* def, ModelRenderOpTextureRefs* textures, GameObject* obj, int unused)
{
    RcpDistortSlot* slot;
    void* s;

    if (*(void**)(def + 0x8) != NULL)
    {
        if (obj != NULL)
            slot = &gRcpDistortSlots[6 - (obj->lightColorSlot + 1)];
        else
            slot = &gRcpDistortSlots[5];
        s = slot->texture;
        (((Texture*)s)->refCount)++;
        textures->texture0 = slot->texture;
    }
    if (*(void**)(def + 0x14) == NULL)
        return;
    if (def[0x20] >= 6)
        slot = gRcpDistortSlots;
    else
        slot = &gRcpDistortSlots[def[0x20] >> 1];
    s = slot->texture;
    (((Texture*)s)->refCount)++;
    textures->texture1 = slot->texture;
}

extern f32 gRcpDistortScaleA;
extern f32 gRcpDistortPowExp;

typedef struct RcpDistortConfig
{
    f32 radius;
    f32 strength;
} RcpDistortConfig;
extern RcpDistortConfig gRcpDistortConfigs[6];

void Rcp_InitDistortionEffects(void)
{
    int i;
    RcpDistortSlot* slots;
    f32* cfg;
    u32 pairIdx;
    RcpDistortSlot* slot;
    f32 strengthScale;
    f32 radiusScale;
    f32 strength;
    f32 falloff;

    i = 0;
    slots = gRcpDistortSlots;
    for (; i < 6; i++)
    {
        slots[i].texture = (u8*)textureAlloc(0x20, 0x20, 6, 0, 0, 0, 0, 1, 1);
        slots[i].group = 0;
    }
    gRcpDistortSlotIndex = i = 0;
    cfg = &gRcpDistortConfigs[0].radius;
    slots = gRcpDistortSlots;
    radiusScale = gRcpDistortScaleA;
    strengthScale = 255.0f;
    do
    {
        strength = cfg[i * 2 + 1];
        (slot = &slots[gRcpDistortSlotIndex])->colR = 0xff;
        slot->colG = 0xff;
        slot->colB = 0xff;
        falloff = radiusScale / powfCoreHighPrecision(cfg[i * 2], gRcpDistortPowExp);
        slot = &slots[gRcpDistortSlotIndex];
        pairIdx = i & 1;
        slot->params[pairIdx] = falloff;
        *(s8*)(&slot->scaleR + pairIdx) = strengthScale * strength;
        slot->mode = 1;
        if (pairIdx != 0)
        {
            gRcpDistortSlotIndex = gRcpDistortSlotIndex + 1;
        }
        i++;
    } while (i < 6);
    /* mode = 0 for the three remaining slots */
    gRcpDistortSlots[gRcpDistortSlotIndex++].mode = 0;
    gRcpDistortSlots[gRcpDistortSlotIndex++].mode = 0;
    gRcpDistortSlots[gRcpDistortSlotIndex++].mode = 0;
    gRcpDistortTexture = textureLoadAsset(RCP_DISTORT_TEXTURE_ID);
}
char sThreadStateAttrSuspendFormat[] = "thread: state=%d attr=%d suspend=%d\n";

u8 gInflateFixedLiteralCodeLengths[ZLB_FIXED_LITERAL_SYMBOL_COUNT] = {
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
    0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
    0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
    0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
    0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
    0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
    0x09, 0x09, 0x09, 0x09, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
};

u16 gInflateFixedLiteralDecodeTable[ZLB_FIXED_LITERAL_TABLE_SIZE] = {
    0x0100, 0x0100, 0x0100, 0x0100, 0x0101, 0x0101, 0x0101, 0x0101, 0x0102, 0x0102, 0x0102, 0x0102, 0x0103, 0x0103,
    0x0103, 0x0103, 0x0104, 0x0104, 0x0104, 0x0104, 0x0105, 0x0105, 0x0105, 0x0105, 0x0106, 0x0106, 0x0106, 0x0106,
    0x0107, 0x0107, 0x0107, 0x0107, 0x0108, 0x0108, 0x0108, 0x0108, 0x0109, 0x0109, 0x0109, 0x0109, 0x010A, 0x010A,
    0x010A, 0x010A, 0x010B, 0x010B, 0x010B, 0x010B, 0x010C, 0x010C, 0x010C, 0x010C, 0x010D, 0x010D, 0x010D, 0x010D,
    0x010E, 0x010E, 0x010E, 0x010E, 0x010F, 0x010F, 0x010F, 0x010F, 0x0110, 0x0110, 0x0110, 0x0110, 0x0111, 0x0111,
    0x0111, 0x0111, 0x0112, 0x0112, 0x0112, 0x0112, 0x0113, 0x0113, 0x0113, 0x0113, 0x0114, 0x0114, 0x0114, 0x0114,
    0x0115, 0x0115, 0x0115, 0x0115, 0x0116, 0x0116, 0x0116, 0x0116, 0x0117, 0x0117, 0x0117, 0x0117, 0x0000, 0x0000,
    0x0001, 0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0004, 0x0005, 0x0005, 0x0006, 0x0006, 0x0007, 0x0007,
    0x0008, 0x0008, 0x0009, 0x0009, 0x000A, 0x000A, 0x000B, 0x000B, 0x000C, 0x000C, 0x000D, 0x000D, 0x000E, 0x000E,
    0x000F, 0x000F, 0x0010, 0x0010, 0x0011, 0x0011, 0x0012, 0x0012, 0x0013, 0x0013, 0x0014, 0x0014, 0x0015, 0x0015,
    0x0016, 0x0016, 0x0017, 0x0017, 0x0018, 0x0018, 0x0019, 0x0019, 0x001A, 0x001A, 0x001B, 0x001B, 0x001C, 0x001C,
    0x001D, 0x001D, 0x001E, 0x001E, 0x001F, 0x001F, 0x0020, 0x0020, 0x0021, 0x0021, 0x0022, 0x0022, 0x0023, 0x0023,
    0x0024, 0x0024, 0x0025, 0x0025, 0x0026, 0x0026, 0x0027, 0x0027, 0x0028, 0x0028, 0x0029, 0x0029, 0x002A, 0x002A,
    0x002B, 0x002B, 0x002C, 0x002C, 0x002D, 0x002D, 0x002E, 0x002E, 0x002F, 0x002F, 0x0030, 0x0030, 0x0031, 0x0031,
    0x0032, 0x0032, 0x0033, 0x0033, 0x0034, 0x0034, 0x0035, 0x0035, 0x0036, 0x0036, 0x0037, 0x0037, 0x0038, 0x0038,
    0x0039, 0x0039, 0x003A, 0x003A, 0x003B, 0x003B, 0x003C, 0x003C, 0x003D, 0x003D, 0x003E, 0x003E, 0x003F, 0x003F,
    0x0040, 0x0040, 0x0041, 0x0041, 0x0042, 0x0042, 0x0043, 0x0043, 0x0044, 0x0044, 0x0045, 0x0045, 0x0046, 0x0046,
    0x0047, 0x0047, 0x0048, 0x0048, 0x0049, 0x0049, 0x004A, 0x004A, 0x004B, 0x004B, 0x004C, 0x004C, 0x004D, 0x004D,
    0x004E, 0x004E, 0x004F, 0x004F, 0x0050, 0x0050, 0x0051, 0x0051, 0x0052, 0x0052, 0x0053, 0x0053, 0x0054, 0x0054,
    0x0055, 0x0055, 0x0056, 0x0056, 0x0057, 0x0057, 0x0058, 0x0058, 0x0059, 0x0059, 0x005A, 0x005A, 0x005B, 0x005B,
    0x005C, 0x005C, 0x005D, 0x005D, 0x005E, 0x005E, 0x005F, 0x005F, 0x0060, 0x0060, 0x0061, 0x0061, 0x0062, 0x0062,
    0x0063, 0x0063, 0x0064, 0x0064, 0x0065, 0x0065, 0x0066, 0x0066, 0x0067, 0x0067, 0x0068, 0x0068, 0x0069, 0x0069,
    0x006A, 0x006A, 0x006B, 0x006B, 0x006C, 0x006C, 0x006D, 0x006D, 0x006E, 0x006E, 0x006F, 0x006F, 0x0070, 0x0070,
    0x0071, 0x0071, 0x0072, 0x0072, 0x0073, 0x0073, 0x0074, 0x0074, 0x0075, 0x0075, 0x0076, 0x0076, 0x0077, 0x0077,
    0x0078, 0x0078, 0x0079, 0x0079, 0x007A, 0x007A, 0x007B, 0x007B, 0x007C, 0x007C, 0x007D, 0x007D, 0x007E, 0x007E,
    0x007F, 0x007F, 0x0080, 0x0080, 0x0081, 0x0081, 0x0082, 0x0082, 0x0083, 0x0083, 0x0084, 0x0084, 0x0085, 0x0085,
    0x0086, 0x0086, 0x0087, 0x0087, 0x0088, 0x0088, 0x0089, 0x0089, 0x008A, 0x008A, 0x008B, 0x008B, 0x008C, 0x008C,
    0x008D, 0x008D, 0x008E, 0x008E, 0x008F, 0x008F, 0x0118, 0x0118, 0x0119, 0x0119, 0x011A, 0x011A, 0x011B, 0x011B,
    0x011C, 0x011C, 0x011D, 0x011D, 0x011E, 0x011E, 0x011F, 0x011F, 0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095,
    0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F, 0x00A0, 0x00A1, 0x00A2, 0x00A3,
    0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF, 0x00B0, 0x00B1,
    0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
    0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD,
    0x00CE, 0x00CF, 0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 0x00D8, 0x00D9, 0x00DA, 0x00DB,
    0x00DC, 0x00DD, 0x00DE, 0x00DF, 0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9,
    0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF, 0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
    0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF,
};

u8 gInflateFixedDistanceCodeLengths[ZLB_FIXED_DISTANCE_SYMBOL_COUNT] = {
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
};

u8 gInflateFixedDistanceDecodeTable[ZLB_FIXED_DISTANCE_SYMBOL_COUNT] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
};

u8 gInflateBitReverseTable[ZLB_BIT_REVERSE_TABLE_SIZE] = {
    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0, 0x08, 0x88, 0x48,
    0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8, 0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4,
    0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4, 0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C,
    0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC, 0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2,
    0x32, 0xB2, 0x72, 0xF2, 0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A,
    0xFA, 0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6, 0x0E, 0x8E,
    0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE, 0x01, 0x81, 0x41, 0xC1, 0x21,
    0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1, 0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
    0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9, 0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55,
    0xD5, 0x35, 0xB5, 0x75, 0xF5, 0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD,
    0x7D, 0xFD, 0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3, 0x0B,
    0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB, 0x07, 0x87, 0x47, 0xC7,
    0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7, 0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F,
    0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF,
};

int lbl_8030CEE0[9] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8,
};

typedef struct TevSwapEntry
{
    int r;
    int g;
    int b;
} TevSwapEntry;
TevSwapEntry gRcpTevSwapTable[24] = {
    {0, 1, 2}, {0, 2, 1}, {1, 0, 2}, {2, 0, 1}, {1, 2, 0}, {2, 1, 0}, {0, 0, 2}, {0, 2, 0},
    {2, 0, 0}, {0, 0, 1}, {0, 1, 0}, {1, 0, 0}, {1, 1, 2}, {1, 2, 1}, {2, 1, 1}, {1, 1, 0},
    {1, 0, 1}, {0, 1, 1}, {2, 2, 0}, {2, 0, 2}, {0, 2, 2}, {2, 2, 1}, {2, 1, 2}, {1, 2, 2},
};

RcpDistortConfig gRcpDistortConfigs[6] ALIGN_DECL(8) = {
    {0.5f, 1.0f}, {0.5f, 0.5f}, {0.4f, 1.0f}, {0.3f, 0.8f}, {0.2f, 1.0f}, {0.4f, 0.5f},
};
u8 gRcpWarpDistortDisplayList[0x6640];
RcpDistortSlot gRcpDistortSlots[6];


