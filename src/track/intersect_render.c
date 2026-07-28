#include "global.h"
#include "main/dll/partfx_interface.h"
#include "dolphin/card.h"
#include "main/hud_visibility_api.h"
#include "main/map_block.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/dll/waterfx_interface.h"
#include "game/objects/object.h"
#include "sys/objects.h"
#include "main/pi_flush_api.h"
#include "main/rcp_dolphin_api.h"
#include "dolphin/mtx.h"
#include "track/intersect.h"
#include "track/intersect_card_api.h"
#include "track/intersect_fog_api.h"
#include "track/intersect_depth_read_api.h"
#include "main/model.h"
#include "main/model_engine.h"
#include "main/texture.h"
#include "main/dll/player_state.h"
#include "main/sky_interface.h"
#include "main/textrender_api.h"
#include "main/gametext_color_api.h"
#include "main/gametext_command_api.h"
#include "main/gametext_show_str_api.h"
#include "main/gameloop_api.h"
#include "main/frame_timing.h"
#include "main/trig.h"
#include "main/camera.h"
#include "main/track_dolphin_api.h"
#include "main/vecmath.h"
#include "main/object_render.h"
#include "main/screen_transition.h"
#include "dolphin/gx/GXPixel.h"
#include "main/mm.h"
#include "main/newshadows.h"
#include "main/objprint_api.h"
#include "main/maketex_api.h"
#include "main/pad.h"
#include "main/pi_dolphin.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/shader_api.h"
#include "dolphin/gx/GXBump.h"
#include "dolphin/gx/GXCull.h"
#include "dolphin/gx/GXGeometry.h"
#include "dolphin/gx/GXLighting.h"
#include "dolphin/gx/GXTev.h"
#include "dolphin/gx/GXTexture.h"
#include "dolphin/gx/GXTransform.h"

typedef void (*GXSetAlphaCompareIntFn)(int comp0, int ref0, int op, int comp1, int ref1);

#include "track/intersect_internal.h"
extern u8 lbl_803DD010;
extern f32 lbl_803DD00C;

extern u8 lbl_803DB678;
extern u8 gHudTintAlpha;
extern GXColor lbl_803DB680;
extern GXColor lbl_803DB684;
extern GXColor lbl_803DB688;
extern GXColor lbl_803DB68C;
extern u32 lbl_803DB690;
extern u32 lbl_803DB694;
extern u32 lbl_803DB698;
extern u32 lbl_803DB69C;
extern GXColor lbl_803DB6A0;
extern GXColor lbl_803DB6A4;
extern u32 lbl_803DB6A8;
extern f32 lbl_803DB6AC;
extern f32 lbl_803DB6B0;
extern f32 lbl_803DB6B4;
extern f32 lbl_803DB6B8;
extern GXColor lbl_803DB6BC;
extern f32 lbl_803DB6C0;
extern f32 lbl_803DB6C4;
extern f32 lbl_803DB6C8;
extern f32 lbl_803DB6CC;
extern GXColor lbl_803DB6D0;
extern GXColor lbl_803DB6D4;
extern GXColor lbl_803DB6D8;
extern GXColor lbl_803DB6DC;
extern GXColor lbl_803DB6E0;
extern GXColor lbl_803DB6E4;
extern GXColor lbl_803DB6E8;
extern GXColor lbl_803DB6EC;
extern GXColor lbl_803DB6F0;
extern u32 lbl_803DB6F4;
extern u32 lbl_803DB6F8;

typedef struct
{
    f32 m[6];
} IndMtxInit;

typedef struct
{
    IndMtxInit ind;
    u32 blk[6][7];
} IndStageInitData;

static const IndStageInitData lbl_802C1EA8 = {
    {{0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f}},
    {{0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF},
     {2, 2, 2, 2, 2, 1, 0},
     {0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF},
     {2, 2, 2, 1, 0, 0, 0},
     {0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF},
     {2, 1, 0, 0, 0, 0, 0}}};
static const IndMtxInit lbl_802C1F68 = {{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}};


extern f32 lbl_803DEEE0;
extern f32 lbl_803DEF28;
extern GXColor lbl_803E8454;

extern f32 lbl_8030EA10[3][2][3];
extern f32 lbl_8030EA58[2][3];
extern f32 lbl_8030EA70[2][3];
extern f32 lbl_8030EA88[2][3];
extern f32 lbl_8030EAA0[2][3];

int cardDeleteFn_8007d99c(void);
void cardGetMessage(u32* buttons, u32* texts, u32* count);
void showMemCardError(u8 err);

typedef struct StageCountTable
{
    u8 count[7];
} StageCountTable;

static const GXColor sApertureColorBlack = {0, 0, 0, 255};
static const GXColor sApertureColorEdge = {0, 0, 0, 4};
static const GXColor sApertureColorCentre = {0, 0, 0, 255};
static const StageCountTable sProjectedShadowStageCounts = {{3, 3, 2, 2, 1, 1, 1}};
static const GXColor sMoonFxTint = {0, 0, 255, 255};
static const GXColor sDistortKColor0 = {0x42, 0x42, 0x42, 0};
static const GXColor sDistortKColor1 = {0x81, 0x81, 0x81, 0};
static const GXColor sDistortKColor2 = {0x19, 0x19, 0x19, 0};
static const GXColor sDistortTevColor = {0x10, 0x10, 0x10, 0};
static const GXColor sColorFilterKColor0 = {0x42, 0x42, 0x42, 0};
static const GXColor sColorFilterKColor1 = {0x6E, 0x6E, 0x6E, 0};
static const GXColor sColorFilterKColor2 = {0x14, 0x14, 0x14, 0};
static const GXColor sColorFilterTevColor = {0x0A, 0x0A, 0x0A, 255};

extern u32 lbl_803E8450;
void playerEarthWalkerAudioFn_8006f950(u8* obj, f32* pos, u8 flip, u8 type);
void gxSetPeControl_ZCompLoc_(u8 zCompLoc);
void gxSetZMode_(u8 compareEnable, int compareFunc, u8 updateEnable);
void drawViewFinderAperture(f32 sx, f32 sy, u8 a, u8 flag);
int cardProbe(u8 retry);
void showMemCardError(u8 err);
void cardShowLoadingMsg(u8 kind);
int cardCb_8007e6d4(u8 slot, int unused, void* src1, void* src2);
int saveCb_8007e748(int saveId, int size, void* dst);


void playerEarthWalkerAudioFn_8006f950(u8* obj, f32* pos, u8 flip, u8 type);

void gxSetPeControl_ZCompLoc_(u8 zCompLoc)
{
    if (gGxZCompLocCached != zCompLoc || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(zCompLoc);
        gGxZCompLocCached = zCompLoc;
        gGxZCompLocValid = 1;
    }
}


void gxSetZMode_(u8 compareEnable, int compareFunc, u8 updateEnable)
{
    if (gGxZModeCompareEnable != compareEnable || gGxZModeCompareFunc != compareFunc ||
        gGxZModeUpdateEnable != updateEnable || gGxZModeValid == 0)
    {
        GXSetZMode(compareEnable, compareFunc, updateEnable);
        gGxZModeCompareEnable = compareEnable;
        gGxZModeCompareFunc = compareFunc;
        gGxZModeUpdateEnable = updateEnable;
        gGxZModeValid = 1;
    }
}

void resetSomeGxFlags(void)
{
    gGxZModeValid = 0;
    gGxZCompLocValid = 0;
}

void setHudOpacity(u8 opacity)
{
    gHudTintAlpha = opacity;
}

void _gxSetFogParams(void)
{
    GXColor c = gFogColor;
    GXSetFog(GX_FOG_PERSP_EXP, gFogStartZ, gFogEndZ, gFogNearZ, gFogFarZ, c);
}

void fogFn_80070404(f32 a, f32 b)
{
    f32 xc, yc, x, y;
    GXColor c;

    gFogNearZ = Camera_GetNearPlane();
    gFogFarZ = Camera_GetFarPlane();

    x = 0.001f * a;
    y = 0.001f * b;

    xc = (x < 0.0f) ? 0.0f : ((x > lbl_803DEEE0) ? lbl_803DEEE0 : x);
    yc = (y < 0.0f) ? 0.0f : ((y > lbl_803DEEE0) ? lbl_803DEEE0 : y);

    gFogStartZ = xc * (gFogFarZ - gFogNearZ) + gFogNearZ;
    gFogEndZ = yc * (gFogFarZ - gFogNearZ) + gFogNearZ;
    c = gFogColor;
    GXSetFog(GX_FOG_PERSP_EXP, gFogStartZ, gFogEndZ, gFogNearZ, gFogFarZ, c);
}

void getColor803dd01c(u8* rgbOut)
{
    rgbOut[0] = gFogColor.r;
    rgbOut[1] = gFogColor.g;
    rgbOut[2] = gFogColor.b;
}

void setFogColorRgb(u8 red, u8 green, u8 blue)
{
    gFogColor.r = red;
    gFogColor.g = green;
    gFogColor.b = blue;
}

int renderWhirlpool(void* obj_a, void** obj_b, int slot)
{
    extern f32 lbl_803DEEE4;

    void* renderOp;
    void* tex2;
    void* model;
    int handle1;
    u8 ignoredLightColor;
    Mtx scaleMtx;
    f32 fA, fB;
    GXBool wrapBit;
    void (*pcb)(void*, void**, int);

    model = obj_b[0];
    renderOp = ObjModel_GetRenderOp((ModelFileHeader*)model, slot);
    handle1 = *(int*)Shader_getLayer(renderOp, 0);
    selectTexture((Texture*)textureIdxToPtr(handle1), 0);
    selectReflectionTexture(1);
    tex2 = textureIdxToPtr(((ModelRenderOp*)renderOp)->layer0TextureId);
    wrapBit = (((Texture*)tex2)->maxLod - ((Texture*)tex2)->minLod > 0) ? GX_TRUE : GX_FALSE;
    GXInitTexObj((void*)((u8*)tex2 + 0x20), (u8*)tex2 + 0x60, ((Texture*)tex2)->width, ((Texture*)tex2)->height,
                 ((Texture*)tex2)->format, GX_REPEAT, GX_REPEAT, wrapBit);
    selectTexture((Texture*)tex2, 2);
    GXLoadTexMtxImm(lbl_80396850, GX_PTTEXMTX6, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, 0, GX_FALSE, GX_PTTEXMTX6);
    GXLoadTexMtxImm(lbl_80396820, GX_PTTEXMTX7, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_POS, 0, GX_FALSE, GX_PTTEXMTX7);
    newshadows_getReflectionScrollOffsets(&fA, &fB);
    PSMTXScale(scaleMtx, 1.0f, 1.0f, 1.0f);
    scaleMtx[1][3] = -fA;
    GXLoadTexMtxImm(scaleMtx, GX_TEXMTX1, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD3, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);

    if (isHeavyFogEnabled() != 0)
    {
        ((u8*)&lbl_803DB6F4)[0] = ((u8*)&gFogColor)[0];
        ((u8*)&lbl_803DB6F4)[1] = ((u8*)&gFogColor)[1];
        ((u8*)&lbl_803DB6F4)[2] = ((u8*)&gFogColor)[2];
        ((u8*)&lbl_803DB6F4)[3] = 0x80;
    }
    else
    {
        (*gSkyInterface)
            ->getCurrentAmbientAndLightColors((u8*)&lbl_803DB6F4, (u8*)&lbl_803DB6F4 + 1, (u8*)&lbl_803DB6F4 + 2,
                                              &ignoredLightColor, &ignoredLightColor, &ignoredLightColor);
        ((u8*)&lbl_803DB6F4)[0] = (u8)((int)((u8*)&lbl_803DB6F4)[0] >> 3);
        ((u8*)&lbl_803DB6F4)[1] = (u8)((int)((u8*)&lbl_803DB6F4)[1] >> 3);
        ((u8*)&lbl_803DB6F4)[2] = (u8)((int)((u8*)&lbl_803DB6F4)[2] >> 3);
        ((u8*)&lbl_803DB6F4)[3] = lbl_803DB678;
    }
    GXSetTevColor(GX_TEVREG2, *(GXColor*)&lbl_803DB6F4);
    GXSetTevKColor(GX_KCOLOR0, *(GXColor*)&lbl_803DB6F8);
    GXSetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K0);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD2, GX_TEXMAP2);
    GXSetIndTexCoordScale(0, 0, 0);
    GXSetIndTexMtx(1, lbl_8030EAA0, -1);
    GXSetIndTexMtx(2, lbl_8030EAA0, -2);
    GXSetTevIndirect(0, 0, 0, 7, 1, 0, 0, 0, 0, 0);
    GXSetTevIndirect(1, 0, 0, 7, 2, 0, 0, 0, 0, 0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C2, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    if (isHeavyFogEnabled() != 0)
    {
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_DIVIDE_2, GX_TRUE, GX_TEVPREV);
    }
    else
    {
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    }
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_CPREV, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevDirect(GX_TEVSTAGE2);
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD3, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_CPREV, GX_CC_TEXC, GX_CC_TEXA, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
    GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetChanCtrl(GX_COLOR0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_ALPHA0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumIndStages(1);
    GXSetNumChans(1);
    GXSetNumTexGens(4);
    GXSetNumTevStages(3);

    pcb = (void (*)(void*, void**, int))ObjModel_GetPostRenderCallback((ObjModel*)obj_b);
    if (pcb != 0)
    {
        pcb(obj_a, obj_b, slot);
    }
    else
    {
        u8 zCompLoc = 1;
        if (((u8*)obj_a)[0x37] < 0xFF || (((ModelRenderOp*)renderOp)->flags & 0x40000000) != 0 ||
            ((ModelRenderOp*)renderOp)->alpha < 0xFF)
        {
            GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
            if ((((ModelFileHeader*)model)->flags & 0x400) != 0)
            {
                if ((u32)gGxZModeCompareEnable != 0 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 ||
                    gGxZModeValid == 0)
                {
                    GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
                    gGxZModeCompareEnable = 0;
                    gGxZModeCompareFunc = 3;
                    gGxZModeUpdateEnable = 0;
                    gGxZModeValid = 1;
                }
                ((GXSetAlphaCompareIntFn)GXSetAlphaCompare)(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
            }
            else if ((((ModelFileHeader*)model)->flags & 0x2000) != 0)
            {
                zCompLoc = 0;
                if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 1 ||
                    gGxZModeValid == 0)
                {
                    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
                    gGxZModeCompareEnable = 1;
                    gGxZModeCompareFunc = 3;
                    gGxZModeUpdateEnable = 1;
                    gGxZModeValid = 1;
                }
                ((GXSetAlphaCompareIntFn)GXSetAlphaCompare)(GX_GREATER, objGetAlphaCompareThreshold(), GX_AOP_AND, GX_GREATER, objGetAlphaCompareThreshold());
            }
            else
            {
                if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 ||
                    gGxZModeValid == 0)
                {
                    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
                    gGxZModeCompareEnable = 1;
                    gGxZModeCompareFunc = 3;
                    gGxZModeUpdateEnable = 0;
                    gGxZModeValid = 1;
                }
                ((GXSetAlphaCompareIntFn)GXSetAlphaCompare)(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
            }
        }
        else
        {
            if ((((ModelRenderOp*)renderOp)->flags & 0x400) != 0)
            {
                GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
                if ((((ModelFileHeader*)model)->flags & 0x400) != 0)
                {
                    if ((u32)gGxZModeCompareEnable != 0 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 ||
                        gGxZModeValid == 0)
                    {
                        GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
                        gGxZModeCompareEnable = 0;
                        gGxZModeCompareFunc = 3;
                        gGxZModeUpdateEnable = 0;
                        gGxZModeValid = 1;
                    }
                }
                else
                {
                    if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 1 ||
                        gGxZModeValid == 0)
                    {
                        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
                        gGxZModeCompareEnable = 1;
                        gGxZModeCompareFunc = 3;
                        gGxZModeUpdateEnable = 1;
                        gGxZModeValid = 1;
                    }
                }
                ((GXSetAlphaCompareIntFn)GXSetAlphaCompare)(GX_GREATER, 0xC0, GX_AOP_AND, GX_GREATER, 0xC0);
            }
            else
            {
                GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
                if ((((ModelFileHeader*)model)->flags & 0x400) != 0)
                {
                    if ((u32)gGxZModeCompareEnable != 0 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 ||
                        gGxZModeValid == 0)
                    {
                        GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
                        gGxZModeCompareEnable = 0;
                        gGxZModeCompareFunc = 3;
                        gGxZModeUpdateEnable = 0;
                        gGxZModeValid = 1;
                    }
                }
                else
                {
                    if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 1 ||
                        gGxZModeValid == 0)
                    {
                        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
                        gGxZModeCompareEnable = 1;
                        gGxZModeCompareFunc = 3;
                        gGxZModeUpdateEnable = 1;
                        gGxZModeValid = 1;
                    }
                }
                ((GXSetAlphaCompareIntFn)GXSetAlphaCompare)(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
            }
        }
        if ((((ModelRenderOp*)renderOp)->flags & 0x400) != 0)
        {
            zCompLoc = 0;
        }
        if (gGxZCompLocCached != zCompLoc || gGxZCompLocValid == 0)
        {
            GXSetZCompLoc(zCompLoc);
            gGxZCompLocCached = zCompLoc;
            gGxZCompLocValid = 1;
        }
    }
    if ((((ModelRenderOp*)renderOp)->flags & 0x8) != 0)
    {
        GXSetCullMode(GX_CULL_BACK);
    }
    else
    {
        GXSetCullMode(GX_CULL_NONE);
    }
    return 1;
}

void screenImageDraw(u8 alpha)
{
    extern f32 lbl_803DEEE4;

    Mtx mtx_60;
    Mtx mtx_30;
    int handle;
    f32 fA;
    f32 fB;

    newshadows_getReflectionScrollOffsets(&fA, &fB);
    getNewShadowCausticTexture((u32*)&handle);
    updateReflectionTextures();
    selectReflectionTexture(0);
    selectTexture((Texture*)handle, 1);
    lbl_803DB6E4.a = alpha;
    GXSetTevKColor(GX_KCOLOR0, lbl_803DB6E4);
    GXSetTevKColor(GX_KCOLOR1, lbl_803DB6E8);
    GXSetTevKColor(GX_KCOLOR2, lbl_803DB6EC);
    GXSetTevKColor(GX_KCOLOR3, lbl_803DB6F0);

    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);

    PSMTXScale(mtx_60, 0.2f, 0.2f, lbl_803DEEE4);
    mtx_60[1][3] = -fA;
    GXLoadTexMtxImm(mtx_60, GX_TEXMTX0, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);

    PSMTXScale(mtx_60, 0.25f, 0.25f, lbl_803DEEE4);
    PSMTXRotRad(mtx_30, 'z', 0.7853982f);
    PSMTXConcat(mtx_30, mtx_60, mtx_60);
    mtx_60[0][3] = fB;
    mtx_60[1][3] = fB;
    GXLoadTexMtxImm(mtx_60, GX_TEXMTX1, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);

    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
    GXSetIndTexCoordScale(0, 0, 0);
    GXSetIndTexMtx(1, lbl_8030EA70, -3);
    GXSetTevIndirect(1, 0, 0, 7, 1, 6, 6, 0, 0, 1);

    GXSetIndTexOrder(GX_INDTEXSTAGE1, GX_TEXCOORD2, GX_TEXMAP1);
    GXSetIndTexCoordScale(1, 0, 0);
    GXSetIndTexMtx(2, lbl_8030EA88, -3);
    GXSetTevIndirect(2, 1, 0, 7, 2, 0, 0, 1, 0, 1);

    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_ALPHA_BUMPN);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD0, GX_TEXMAP0, GX_ALPHA_BUMPN);
    GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_APREV, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
    GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_DIVIDE_2, GX_TRUE, GX_TEVPREV);

    GXSetTevKColorSel(GX_TEVSTAGE3, GX_TEV_KCSEL_K0);
    GXSetTevKAlphaSel(GX_TEVSTAGE3, GX_TEV_KASEL_1_2);
    GXSetTevDirect(GX_TEVSTAGE3);
    GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE3, GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE3, GX_CA_KONST, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
    GXSetTevSwapMode(GX_TEVSTAGE3, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
    GXSetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_2, GX_TRUE, GX_TEVREG0);

    GXSetTevKColorSel(GX_TEVSTAGE4, GX_TEV_KCSEL_K1);
    GXSetTevKAlphaSel(GX_TEVSTAGE4, GX_TEV_KASEL_1_2);
    GXSetTevDirect(GX_TEVSTAGE4);
    GXSetTevOrder(GX_TEVSTAGE4, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE4, GX_CC_KONST, GX_CC_ZERO, GX_CC_CPREV, GX_CC_C0);
    GXSetTevAlphaIn(GX_TEVSTAGE4, GX_CA_APREV, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevSwapMode(GX_TEVSTAGE4, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE4, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
    GXSetTevAlphaOp(GX_TEVSTAGE4, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_2, GX_TRUE, GX_TEVREG1);

    GXSetTevKColorSel(GX_TEVSTAGE5, GX_TEV_KCSEL_K2);
    GXSetTevOrder(GX_TEVSTAGE5, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevDirect(GX_TEVSTAGE5);
    GXSetTevColorIn(GX_TEVSTAGE5, GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE5, GX_CA_A0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A1);
    GXSetTevSwapMode(GX_TEVSTAGE5, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE5, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG1);
    GXSetTevAlphaOp(GX_TEVSTAGE5, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXSetTevKColorSel(GX_TEVSTAGE6, GX_TEV_KCSEL_K3);
    GXSetTevKAlphaSel(GX_TEVSTAGE6, GX_TEV_KASEL_1_2);
    GXSetTevColor(GX_TEVREG2, lbl_803DB6E0);
    GXSetTevOrder(GX_TEVSTAGE6, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevDirect(GX_TEVSTAGE6);
    GXSetTevColorIn(GX_TEVSTAGE6, GX_CC_KONST, GX_CC_ZERO, GX_CC_CPREV, GX_CC_C1);
    GXSetTevAlphaIn(GX_TEVSTAGE6, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
    GXSetTevSwapMode(GX_TEVSTAGE6, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE6, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG1);
    GXSetTevAlphaOp(GX_TEVSTAGE6, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXSetTevKAlphaSel(GX_TEVSTAGE7, GX_TEV_KASEL_K0_A);
    GXSetTevDirect(GX_TEVSTAGE7);
    GXSetTevOrder(GX_TEVSTAGE7, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE7, GX_CC_C1, GX_CC_C0, GX_CC_APREV, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE7, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevSwapMode(GX_TEVSTAGE7, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE7, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE7, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXSetNumTexGens(3);
    GXSetNumIndStages(2);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(0);
    GXSetNumTevStages(8);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetCullMode(GX_CULL_NONE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    if ((u32)gGxZModeCompareEnable != 0 || gGxZModeCompareFunc != 7 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        gGxZModeCompareEnable = 0;
        gGxZModeCompareFunc = 7;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetProjection(hudMatrix, GX_ORTHOGRAPHIC);
    GXSetCurrentMtx(GX_IDENTITY);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);

    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = -8;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0;

    GXWGFifo.s16 = 0x280;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = -8;
    GXWGFifo.s16 = 0x80;
    GXWGFifo.s16 = 0;

    GXWGFifo.s16 = 0x280;
    GXWGFifo.s16 = 0x1E0;
    GXWGFifo.s16 = -8;
    GXWGFifo.s16 = 0x80;
    GXWGFifo.s16 = 0x80;

    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0x1E0;
    GXWGFifo.s16 = -8;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0x80;

    Camera_RebuildProjectionMatrix();
    GXSetCurrentMtx(GX_PNMTX0);
}

void doSpiritVisionFilter(void)
{


    updateReflectionTextures();
    selectReflectionTexture(0);
    GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_GREEN, GX_CH_BLUE, GX_CH_RED, GX_CH_ALPHA);
    GXSetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_RED, GX_CH_RED, GX_CH_RED, GX_CH_ALPHA);
    GXSetTevSwapModeTable(GX_TEV_SWAP2, GX_CH_GREEN, GX_CH_GREEN, GX_CH_GREEN, GX_CH_ALPHA);
    GXSetTevSwapModeTable(GX_TEV_SWAP3, GX_CH_BLUE, GX_CH_BLUE, GX_CH_BLUE, GX_CH_ALPHA);

    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);

    GXSetTevKColor(GX_KCOLOR0, lbl_803DB6D0);
    GXSetTevKColor(GX_KCOLOR1, lbl_803DB6D4);
    GXSetTevKColor(GX_KCOLOR2, lbl_803DB6D8);
    GXSetTevColor(GX_TEVREG0, lbl_803DB6DC);

    GXSetNumTexGens(1);
    GXSetNumIndStages(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(0);
    GXSetNumTevStages(4);

    GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_C0);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A0);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP1);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXSetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K1);
    GXSetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K1_A);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP2);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG2);

    GXSetTevKColorSel(GX_TEVSTAGE2, GX_TEV_KCSEL_K2);
    GXSetTevDirect(GX_TEVSTAGE2);
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
    GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP3);
    GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_DIVIDE_2, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXSetTevDirect(GX_TEVSTAGE3);
    GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE3, GX_CC_CPREV, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevAlphaIn(GX_TEVSTAGE3, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
    GXSetTevSwapMode(GX_TEVSTAGE3, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE3, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_4, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetCullMode(GX_CULL_NONE);
    GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
    if ((u32)gGxZModeCompareEnable != 0 || gGxZModeCompareFunc != 7 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        gGxZModeCompareEnable = 0;
        gGxZModeCompareFunc = 7;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetProjection(hudMatrix, GX_ORTHOGRAPHIC);
    GXSetCurrentMtx(GX_IDENTITY);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);

    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = -8;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0;

    GXWGFifo.s16 = 0x280;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = -8;
    GXWGFifo.s16 = 0x80;
    GXWGFifo.s16 = 0;

    GXWGFifo.s16 = 0x280;
    GXWGFifo.s16 = 0x1E0;
    GXWGFifo.s16 = -8;
    GXWGFifo.s16 = 0x80;
    GXWGFifo.s16 = 0x80;

    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0x1E0;
    GXWGFifo.s16 = -8;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0x80;

    Camera_RebuildProjectionMatrix();
    GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
}

void doColorFilter(u8* mod)
{
    GXColor c0, c1, c2, c3;

    c0 = sColorFilterKColor0;
    c1 = sColorFilterKColor1;
    c2 = sColorFilterKColor2;
    c3 = sColorFilterTevColor;
    {
        int s0, s1, s2;
        c0.r = (u8)(c0.r + (s0 = mod[0] >> 3));
        c0.g = (u8)(c0.g + (s1 = mod[1] >> 3));
        c0.b = (u8)(c0.b + (s2 = mod[2] >> 3));
        c1.r = (u8)(c1.r + s0);
        c1.g = (u8)(c1.g + s1);
        c1.b = (u8)(c1.b + s2);
        c2.r = (u8)(c2.r + s0);
        c2.g = (u8)(c2.g + s1);
        c2.b = (u8)(c2.b + s2);
    }

    updateReflectionTextures();
    selectReflectionTexture(0);
    GXSetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_RED, GX_CH_RED, GX_CH_RED, GX_CH_ALPHA);
    GXSetTevSwapModeTable(GX_TEV_SWAP2, GX_CH_GREEN, GX_CH_GREEN, GX_CH_GREEN, GX_CH_ALPHA);
    GXSetTevSwapModeTable(GX_TEV_SWAP3, GX_CH_BLUE, GX_CH_BLUE, GX_CH_BLUE, GX_CH_ALPHA);

    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);

    GXSetTevKColor(GX_KCOLOR0, c0);
    GXSetTevKColor(GX_KCOLOR1, c1);
    GXSetTevKColor(GX_KCOLOR2, c2);
    GXSetTevColor(GX_TEVREG0, c3);

    GXSetNumTexGens(1);
    GXSetNumIndStages(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(0);
    GXSetNumTevStages(3);

    GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_C0);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A0);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP1);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXSetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K1);
    GXSetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K1_A);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP2);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG2);

    GXSetTevKColorSel(GX_TEVSTAGE2, GX_TEV_KCSEL_K2);
    GXSetTevDirect(GX_TEVSTAGE2);
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
    GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP3);
    GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetCullMode(GX_CULL_NONE);
    GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
    if ((u32)gGxZModeCompareEnable != 0 || gGxZModeCompareFunc != 7 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        gGxZModeCompareEnable = 0;
        gGxZModeCompareFunc = 7;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetProjection(hudMatrix, GX_ORTHOGRAPHIC);
    GXSetCurrentMtx(GX_IDENTITY);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);

    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = -8;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0;

    GXWGFifo.s16 = 0x280;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = -8;
    GXWGFifo.s16 = 0x80;
    GXWGFifo.s16 = 0;

    GXWGFifo.s16 = 0x280;
    GXWGFifo.s16 = 0x1E0;
    GXWGFifo.s16 = -8;
    GXWGFifo.s16 = 0x80;
    GXWGFifo.s16 = 0x80;

    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0x1E0;
    GXWGFifo.s16 = -8;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0x80;

    Camera_RebuildProjectionMatrix();
}

static inline f32 distortSqrtf(f32 x)
{
    volatile float y;
    double guess = __frsqrte((double)x);
    guess = 0.5 * guess * (3.0 - guess * guess * x);
    guess = 0.5 * guess * (3.0 - guess * guess * x);
    guess = 0.5 * guess * (3.0 - guess * guess * x);
    y = (float)(x * guess);
    return y;
}

void doDistortionFilter(f32* pos, f32 radius, u8* mod, f32 angle)
{
    extern f32 lbl_803DEEE4;
    Mtx mtx_d0;
    Mtx mtx_a0;
    Mtx mtx_70;
    f32 indMtx[6];
    int handle1;
    Texture* handle2;
    f32 proj5, proj4, proj3, proj2, proj1, proj0;
    GXColor c0;
    GXColor c1;
    GXColor c2;
    GXColor c3;
    Texture* handle3;
    f32 x, z;

    c0 = sDistortKColor0;
    c1 = sDistortKColor1;
    c2 = sDistortKColor2;
    c3 = sDistortTevColor;
    {
        int b0, b1, b2;
        int s0, s1, s2;
        c0.r = (u8)(c0.r + (s0 = (b0 = mod[0]) >> 2));
        c0.g = (u8)(c0.g + (s1 = (b1 = mod[1]) >> 2));
        c0.b = (u8)(c0.b + (s2 = (b2 = mod[2]) >> 2));
        c1.r = (u8)(c1.r + s0);
        c1.g = (u8)(c1.g + s1);
        c1.b = (u8)(c1.b + s2);
        c2.r = (u8)(c2.r + s0);
        c2.g = (u8)(c2.g + s1);
        c2.b = (u8)(c2.b + s2);
        c3.r = (u8)(c3.r + (b0 >> 3));
        c3.g = (u8)(c3.g + (b1 >> 3));
        c3.b = (u8)(c3.b + (b2 >> 3));
    }

    x = pos[0];
    z = pos[2];
    x = x - playerMapOffsetX;
    z = z - playerMapOffsetZ;
    Camera_ProjectWorldSphere(x, pos[1], z, radius, &proj5, &proj4, &proj3, &proj2, &proj1, &proj0);
    proj3 = proj3 + lbl_803DEEE4;
    c0.a = (u8)(((u32)(16777216.0f * proj3) & 0x00FF0000) >> 16);

    selectReflectionTexture(0);
    getReflectionTexture2((u32*)&handle1);
    selectTexture((Texture*)handle1, 1);
    getNewShadowRadialTexture(&handle2);
    selectTexture(handle2, 2);

    GXSetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_RED, GX_CH_RED, GX_CH_RED, GX_CH_ALPHA);
    GXSetTevSwapModeTable(GX_TEV_SWAP2, GX_CH_GREEN, GX_CH_GREEN, GX_CH_GREEN, GX_CH_ALPHA);
    GXSetTevSwapModeTable(GX_TEV_SWAP3, GX_CH_BLUE, GX_CH_BLUE, GX_CH_BLUE, GX_CH_ALPHA);

    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);

    PSMTXTrans(mtx_a0, 0.5f * (-proj5) - 0.5f, 0.5f * proj4 - 0.5f, 0.0f);
    {
        f32 s = *(f32*)&lbl_803DB6C4;
        PSMTXScale(mtx_70, s / proj2, s / proj1, 0.0f);
    }
    PSMTXConcat(mtx_70, mtx_a0, mtx_d0);
    PSMTXTrans(mtx_a0, 0.5f, 0.5f, 0.0f);
    PSMTXConcat(mtx_a0, mtx_d0, mtx_d0);
    GXLoadTexMtxImm(mtx_d0, GX_TEXMTX0, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);

    {
        f32 r2 = lbl_803DB6C8 / radius;
        f32 sr;
        sr = (r2 > 0.0f) ? distortSqrtf(r2) : r2;
        if (sr > lbl_803DEEE4)
        {
            c1.a = 0xFF;
        }
        else
        {
            c1.a = 255.0f * sr;
        }
        sr = sr * lbl_803DEEE0;
        if (sr > *(f32*)&lbl_803DEEE4)
            sr = *(f32*)&lbl_803DEEE4;
        c3.a = 255.0f * sr;
    }

    GXSetTevKColor(GX_KCOLOR0, c0);
    GXSetTevKColor(GX_KCOLOR1, c1);
    GXSetTevKColor(GX_KCOLOR2, c2);
    GXSetTevColor(GX_TEVREG0, c3);

    getNewShadowDistortionTexture(&handle3);
    selectTexture(handle3, 3);

    {
        f32 ind_s = lbl_803DB6CC / radius;
        if (ind_s > 0.5f)
            ind_s = 0.5f;
        indMtx[0] = ind_s;
        indMtx[1] = 0.0f;
        indMtx[2] = 0.0f;
        indMtx[3] = 0.0f;
        indMtx[4] = ind_s;
        indMtx[5] = 0.0f;
    }

    PSMTXTrans(mtx_a0, 0.5f * (-proj5) - 0.5f, 0.5f * proj4 - 0.5f, 0.0f);
    PSMTXScale(mtx_70, 0.8f, 0.8f, 0.0f);
    PSMTXRotRad(mtx_d0, 'z', angle);
    PSMTXConcat(mtx_70, mtx_a0, mtx_70);
    PSMTXConcat(mtx_d0, mtx_70, mtx_d0);
    PSMTXTrans(mtx_a0, 0.5f, 0.5f, 0.0f);
    PSMTXConcat(mtx_a0, mtx_d0, mtx_d0);
    GXLoadTexMtxImm(mtx_d0, GX_TEXMTX1, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD3, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);

    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD3, GX_TEXMAP3);
    GXSetIndTexCoordScale(0, 0, 0);
    GXSetIndTexMtx(1, (f32(*)[3])indMtx, 1);

    GXSetTevIndirect(2, 0, 0, 7, 1, 0, 0, 0, 0, 0);
    GXSetTevIndirect(3, 0, 0, 7, 1, 0, 0, 0, 0, 0);
    GXSetTevIndirect(4, 0, 0, 7, 1, 0, 0, 0, 0, 0);

    GXSetNumTexGens(4);
    GXSetNumIndStages(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(0);
    GXSetNumTevStages(6);

    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_TEXA, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP1);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_4, GX_TRUE, GX_TEVREG2);

    GXSetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K0_A);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_KONST, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP1);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_4, GX_TRUE, GX_TEVPREV);

    GXSetTevKColorSel(GX_TEVSTAGE2, GX_TEV_KCSEL_K0);
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_C0);
    GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_APREV, GX_CA_A0, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP1);
    GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_4, GX_TRUE, GX_TEVPREV);

    GXSetTevKColorSel(GX_TEVSTAGE3, GX_TEV_KCSEL_K1);
    GXSetTevKAlphaSel(GX_TEVSTAGE3, GX_TEV_KASEL_K1_A);
    GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE3, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE3, GX_CA_ZERO, GX_CA_A2, GX_CA_KONST, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE3, GX_TEV_SWAP0, GX_TEV_SWAP2);
    GXSetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_4, GX_TRUE, GX_TEVREG2);

    GXSetTevKColorSel(GX_TEVSTAGE4, GX_TEV_KCSEL_K2);
    GXSetTevOrder(GX_TEVSTAGE4, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE4, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE4, GX_CA_A2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
    GXSetTevSwapMode(GX_TEVSTAGE4, GX_TEV_SWAP0, GX_TEV_SWAP3);
    GXSetTevColorOp(GX_TEVSTAGE4, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE4, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_4, GX_TRUE, GX_TEVPREV);

    GXSetTevDirect(GX_TEVSTAGE5);
    GXSetTevOrder(GX_TEVSTAGE5, GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE5, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE5, GX_CA_TEXA, GX_CA_ZERO, GX_CA_APREV, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE5, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE5, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE5, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetCullMode(GX_CULL_NONE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_INVSRCALPHA, GX_BL_SRCALPHA, GX_LO_NOOP);
    if ((u32)gGxZModeCompareEnable != 0 || gGxZModeCompareFunc != 7 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        gGxZModeCompareEnable = 0;
        gGxZModeCompareFunc = 7;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetProjection(hudMatrix, GX_ORTHOGRAPHIC);
    GXSetCurrentMtx(GX_IDENTITY);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);

    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = -8;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0;

    GXWGFifo.s16 = 0x280;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = -8;
    GXWGFifo.s16 = 0x80;
    GXWGFifo.s16 = 0;

    GXWGFifo.s16 = 0x280;
    GXWGFifo.s16 = 0x1E0;
    GXWGFifo.s16 = -8;
    GXWGFifo.s16 = 0x80;
    GXWGFifo.s16 = 0x80;

    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0x1E0;
    GXWGFifo.s16 = -8;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0x80;

    Camera_RebuildProjectionMatrix();
}

int gxTextureFn_80072dfc(void* obj_a, void** obj_b, int slot)
{
    extern f32 lbl_803DEEE4;
    Mtx mtx_54;
    Mtx mtx_24;
    void* renderOp;
    void* tex;
    void* model;
    GXColor temp;
    void (*pcb)(void*, void**, int);
    int alpha_byte;
    GXColor fogColor;

    model = obj_b[0];
    renderOp = ObjModel_GetRenderOp((ModelFileHeader*)model, slot);
    tex = (void*)getNewShadowReflectionGradientTexture();
    selectReflectionTexture(0);
    selectTexture((Texture*)tex, 1);
    selectWhirlpoolTexture(2);

    GXLoadTexMtxImm(lbl_80396820, GX_PTTEXMTX7, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_POS, 0, GX_FALSE, GX_PTTEXMTX7);

    if (model == 0 || ((ModelFileHeader*)model)->normalCount != 0)
    {
        PSMTXScale(mtx_54, lbl_803DB6B8, lbl_803DB6B8, 0.0f);
        mtx_54[2][3] = lbl_803DEEE4;
        PSMTXTrans(mtx_24, 0.5f, 0.5f, 0.0f);
        PSMTXConcat(mtx_24, mtx_54, mtx_54);
    }
    else
    {
        PSMTXScale(mtx_54, 0.0f, 0.0f, 0.0f);
        mtx_54[0][3] = 0.5f;
        mtx_54[1][3] = 0.5f;
        mtx_54[2][3] = lbl_803DEEE4;
    }
    GXLoadTexMtxImm(mtx_54, GX_PTTEXMTX6, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_NRM, GX_TEXMTX0, GX_TRUE, GX_PTTEXMTX6);

    PSMTXScale(mtx_54, lbl_803DB6C0, lbl_803DB6C0, 0.0f);
    mtx_54[2][3] = lbl_803DEEE4;
    GXLoadTexMtxImm(mtx_54, GX_PTTEXMTX5, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTTEXMTX5);

    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
    GXSetIndTexCoordScale(0, 0, 0);
    GXSetIndTexMtx(1, lbl_8030EA58, -1);
    GXSetTevIndirect(0, 0, 0, 7, 1, 0, 0, 0, 0, 0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_CPREV, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXSetNumIndStages(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(0);
    GXSetNumTexGens(3);
    GXSetNumTevStages(2);

    alpha_byte = (((ModelRenderOp*)renderOp)->alpha * ((u8*)obj_a)[0x37]) >> 8;
    temp.a = alpha_byte;
    GXSetTevKColor(GX_KCOLOR0, temp);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    GXSetTevKColor(GX_KCOLOR1, lbl_803DB6BC);
    GXSetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K1);

    pcb = (void (*)(void*, void**, int))ObjModel_GetPostRenderCallback((ObjModel*)obj_b);
    if (pcb != 0)
    {
        pcb(obj_a, obj_b, slot);
    }
    else
    {
        u8 zCompLoc = 1;
        int ref1;
        if (((u8*)obj_a)[0x37] < 0xff || (((ModelRenderOp*)renderOp)->flags & 0x40000000) != 0 ||
            ((ModelRenderOp*)renderOp)->alpha < 0xff)
        {
            GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
            if ((((ModelFileHeader*)model)->flags & 0x400) != 0)
            {
                if ((u32)gGxZModeCompareEnable != 0 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 ||
                    gGxZModeValid == 0)
                {
                    GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
                    gGxZModeCompareEnable = 0;
                    gGxZModeCompareFunc = 3;
                    gGxZModeUpdateEnable = 0;
                    gGxZModeValid = 1;
                }
                ((GXSetAlphaCompareIntFn)GXSetAlphaCompare)(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
            }
            else if ((((ModelFileHeader*)model)->flags & 0x2000) != 0)
            {
                zCompLoc = 0;
                if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 1 ||
                    gGxZModeValid == 0)
                {
                    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
                    gGxZModeCompareEnable = 1;
                    gGxZModeCompareFunc = 3;
                    gGxZModeUpdateEnable = 1;
                    gGxZModeValid = 1;
                }
                obj_a = (void*)objGetAlphaCompareThreshold();
                ref1 = objGetAlphaCompareThreshold();
                ((GXSetAlphaCompareIntFn)GXSetAlphaCompare)(GX_GREATER, ref1, GX_AOP_AND, GX_GREATER, (int)obj_a);
            }
            else
            {
                if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 ||
                    gGxZModeValid == 0)
                {
                    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
                    gGxZModeCompareEnable = 1;
                    gGxZModeCompareFunc = 3;
                    gGxZModeUpdateEnable = 0;
                    gGxZModeValid = 1;
                }
                ((GXSetAlphaCompareIntFn)GXSetAlphaCompare)(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
            }
        }
        else
        {
            if ((((ModelRenderOp*)renderOp)->flags & 0x400) != 0)
            {
                GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
                if ((((ModelFileHeader*)model)->flags & 0x400) != 0)
                {
                    if ((u32)gGxZModeCompareEnable != 0 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 ||
                        gGxZModeValid == 0)
                    {
                        GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
                        gGxZModeCompareEnable = 0;
                        gGxZModeCompareFunc = 3;
                        gGxZModeUpdateEnable = 0;
                        gGxZModeValid = 1;
                    }
                }
                else
                {
                    if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 1 ||
                        gGxZModeValid == 0)
                    {
                        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
                        gGxZModeCompareEnable = 1;
                        gGxZModeCompareFunc = 3;
                        gGxZModeUpdateEnable = 1;
                        gGxZModeValid = 1;
                    }
                }
                ((GXSetAlphaCompareIntFn)GXSetAlphaCompare)(GX_GREATER, 192, GX_AOP_AND, GX_GREATER, 192);
            }
            else
            {
                GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
                if ((((ModelFileHeader*)model)->flags & 0x400) != 0)
                {
                    if ((u32)gGxZModeCompareEnable != 0 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 ||
                        gGxZModeValid == 0)
                    {
                        GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
                        gGxZModeCompareEnable = 0;
                        gGxZModeCompareFunc = 3;
                        gGxZModeUpdateEnable = 0;
                        gGxZModeValid = 1;
                    }
                }
                else
                {
                    if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 1 ||
                        gGxZModeValid == 0)
                    {
                        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
                        gGxZModeCompareEnable = 1;
                        gGxZModeCompareFunc = 3;
                        gGxZModeUpdateEnable = 1;
                        gGxZModeValid = 1;
                    }
                }
                ((GXSetAlphaCompareIntFn)GXSetAlphaCompare)(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
            }
        }
        if ((((ModelRenderOp*)renderOp)->flags & 0x400) != 0)
        {
            zCompLoc = 0;
        }
        if (gGxZCompLocCached != zCompLoc || gGxZCompLocValid == 0)
        {
            GXSetZCompLoc(zCompLoc);
            gGxZCompLocCached = zCompLoc;
            gGxZCompLocValid = 1;
        }
    }
    GXSetCullMode(GX_CULL_NONE);
    if ((((ModelFileHeader*)model)->flags & 0x100) != 0)
    {
        fogColor = temp;
        GXSetFog(GX_FOG_NONE, 0.0f, 0.0f, 0.0f, 0.0f, fogColor);
    }
    else
    {
        fogColor = gFogColor;
        GXSetFog(GX_FOG_PERSP_EXP, gFogStartZ, gFogEndZ, gFogNearZ, gFogFarZ, fogColor);
    }
    return 1;
}

/*
 * Three-tex-coord-gen ind+direct TEV setup. Loads the active env-mtx
 * (lbl_80396820) for tex0, scales tex1 by 4.0f through a 3x4
 * matrix from PSMTXScale, and stamps an indirect tex matrix from local
 * stack data. Two TEV stages: stage 0 K-modulates the texture by alpha,
 * stage 1 modulates by the second texture. Uses ind tex stage 0 to warp
 * tex coord 0 by tex1.
 */
void quakeSpellTextureFn_8007366c(u8 alpha)
{
    extern f32 lbl_803DEEE4;

    int handle1;
    int handle2;
    f32 a;
    f32 b;
    GXColor c;
    f32 ind_mtx[2][3];
    Mtx tex_mtx;
    Mtx mtx;

    Camera_GetViewMatrix();
    selectReflectionTexture(0);
    GXLoadTexMtxImm(lbl_80396820, GX_PTTEXMTX6, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, 0, GX_FALSE, GX_PTTEXMTX6);
    newshadows_getReflectionScrollOffsets(&a, &b);
    a = a * lbl_803DEF28;
    getNewShadowCausticTexture((u32*)&handle1);
    selectTexture((Texture*)handle1, 1);
    PSMTXScale((f32(*)[4])tex_mtx, 4.0f, 4.0f, 4.0f);
    tex_mtx[0][3] = a;
    GXLoadTexMtxImm(tex_mtx, GX_TEXMTX1, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);
    ind_mtx[0][0] = 0.5f;
    ind_mtx[0][1] = 0.0f;
    ind_mtx[0][2] = 0.0f;
    ind_mtx[1][0] = 0.0f;
    ind_mtx[1][1] = 0.25f;
    ind_mtx[1][2] = 0.0f;
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
    GXSetIndTexCoordScale(0, 0, 0);
    GXSetIndTexMtx(1, ind_mtx, -3);
    GXSetTevIndirect(0, 0, 0, 7, 1, 0, 0, 0, 0, 0);
    mtx[0][0] = 0.6f;
    mtx[0][1] = 0.0f;
    mtx[0][2] = 0.0f;
    mtx[0][3] = 0.5f;
    mtx[1][0] = 0.0f;
    mtx[1][1] = 0.6f;
    mtx[1][2] = 0.0f;
    mtx[1][3] = 0.5f;
    mtx[2][0] = 0.0f;
    mtx[2][1] = 0.0f;
    mtx[2][2] = 0.0f;
    mtx[2][3] = lbl_803DEEE4;
    GXLoadTexMtxImm(mtx, GX_PTTEXMTX7, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_NRM, GX_TEXMTX0, GX_TRUE, GX_PTTEXMTX7);
    getNewShadowDiskTexture((u32*)&handle2);
    selectTexture((Texture*)handle2, 2);
    c.a = alpha;
    GXSetTevKColor(GX_KCOLOR0, c);
    GXSetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K0_A);
    GXSetNumIndStages(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(0);
    GXSetNumTexGens(3);
    GXSetNumTevStages(2);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
        gGxZModeCompareEnable = 1;
        gGxZModeCompareFunc = 3;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetCullMode(GX_CULL_BACK);
}

void setupAdditiveTintedTexture(void* texture, u32* colorA, u32* colorB)
{
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    selectTexture((Texture*)texture, 0);
    GXSetTevKColor(GX_KCOLOR0, *(GXColor*)colorA);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
    GXSetTevColor(GX_TEVREG0, *(GXColor*)colorB);
    GXSetNumIndStages(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(0);
    GXSetNumTexGens(1);
    GXSetNumTevStages(1);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_C0);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
        gGxZModeCompareEnable = 1;
        gGxZModeCompareFunc = 3;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetCullMode(GX_CULL_BACK);
}

int modelCb_80073d04(u8* obj, int* objB)
{
    extern f32 lbl_803DEEE4;
    int handle;
    GXColor colorK;
    GXColor colorB;
    Mtx texMtx;
    int tex;
    int model;

    colorB = sMoonFxTint;
    model = objB[0];
    tex = (int)textureIdxToPtr(*(int*)Shader_getLayer(ObjModel_GetRenderOp((ModelFileHeader*)model, 0), 0));
    texMtx[0][0] = 0.7f;
    texMtx[0][1] = 0.0f;
    texMtx[0][2] = 0.0f;
    texMtx[0][3] = 0.5f;
    texMtx[1][0] = 0.0f;
    texMtx[1][1] = 0.7f;
    texMtx[1][2] = 0.0f;
    texMtx[1][3] = 0.5f;
    texMtx[2][0] = 0.0f;
    texMtx[2][1] = 0.0f;
    texMtx[2][2] = 0.0f;
    texMtx[2][3] = lbl_803DEEE4;
    GXLoadTexMtxImm(texMtx, GX_PTTEXMTX7, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_NRM, GX_TEXMTX0, GX_TRUE, GX_PTTEXMTX7);
    getNewShadowDiskTexture((u32*)&handle);
    selectTexture((Texture*)handle, 0);
    colorK.a = obj[0x37];
    GXSetTevKColor(GX_KCOLOR0, colorK);
    GXSetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K0_A);
    GXSetTevColor(GX_TEVREG0, colorB);
    GXSetNumIndStages(0);
    GXSetNumTexGens(2);
    GXSetNumTevStages(2);
    GXSetTevDirect(GX_TEVSTAGE0);
    if (((ModelFileHeader*)model)->flags24 & 2)
    {
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_RASA);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    }
    else
    {
        GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
        GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
        GXSetNumChans(0);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_TEXA, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A0);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    }
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    selectTexture((Texture*)tex, 1);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_C0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_APREV, GX_CA_KONST, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
        gGxZModeCompareEnable = 1;
        gGxZModeCompareFunc = 3;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetCullMode(GX_CULL_BACK);
    return 1;
}

int moonFxCb_80074110(u8* obj, int* objB, int slot)
{
    GXColor colorK;
    GXColor colorFog;
    Mtx mtx;
    int op;
    int tex;
    f32 tx;

    op = (int)ObjModel_GetRenderOp((ModelFileHeader*)objB[0], slot);
    tex = (int)textureIdxToPtr(*(int*)Shader_getLayer((void*)op, 0));
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    lbl_803DD010 = mainGetBit(0x2ba);
    tx = lbl_803DD010 / 30.0f;
    PSMTXTrans(mtx, tx, 0.0f, 0.0f);
    GXLoadTexMtxImm(mtx, GX_TEXMTX0, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetNumTexGens(2);
    GXSetNumTevStages(3);
    GXSetNumIndStages(0);
    selectTexture((Texture*)tex, 0);
    colorK.a = (((ModelRenderOp*)op)->alpha * obj[0x37]) >> 8;
    GXSetTevKColor(GX_KCOLOR0, colorK);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    colorFog.a = 0x3e;
    GXSetTevKColor(GX_KCOLOR1, colorFog);
    GXSetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K1_A);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_4, GX_TRUE, GX_TEVREG0);
    GXSetTevDirect(GX_TEVSTAGE2);
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_APREV, GX_CA_ZERO, GX_CA_A0, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
        gGxZModeCompareEnable = 1;
        gGxZModeCompareFunc = 3;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetCullMode(GX_CULL_NONE);
    GXSetFog(GX_FOG_NONE, 0.0f, 0.0f, 0.0f, 0.0f, colorFog);
    return 1;
}

int modelCb_80074518(void* obj_a, void** obj_b, int slot)
{
    extern f32 lbl_803DEEE4;
    Mtx mtx_90;
    Mtx mtx_60;
    Mtx mtx_30;
    f32 indMtx[6];
    void* renderOp;
    void* tex;
    void* model;
    GXColor temp;
    int alpha_byte;
    void (*pcb)(void*, void**, int);

    *(IndMtxInit*)indMtx = lbl_802C1F68;

    model = obj_b[0];
    renderOp = ObjModel_GetRenderOp((ModelFileHeader*)model, slot);
    tex = textureIdxToPtr(*(int*)Shader_getLayer(renderOp, 0));

    PSMTXScale(mtx_60, lbl_803DB6B4, lbl_803DB6B4, 0.0f);
    mtx_60[2][3] = lbl_803DEEE4;
    GXLoadTexMtxImm(mtx_60, GX_PTTEXMTX7, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_NRM, GX_TEXMTX0, GX_TRUE, GX_PTTEXMTX7);
    GXSetNumTexGens(2);
    GXSetNumTevStages(2);
    GXSetNumIndStages(2);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD0, GX_TEXMAP2);
    GXSetIndTexCoordScale(0, 0, 0);
    GXSetIndTexMtx(1, (f32(*)[3])indMtx, 0);
    GXSetTevIndirect(0, 0, 0, 7, 1, 0, 0, 0, 0, 0);
    selectTexture((Texture*)tex, 0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ONE);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXSetIndTexOrder(GX_INDTEXSTAGE1, GX_TEXCOORD0, GX_TEXMAP2);
    GXSetIndTexCoordScale(1, 0, 0);
    GXSetTevIndirect(1, 1, 0, 7, 1, 0, 0, 1, 0, 0);
    PSMTXScale(mtx_30, lbl_803DB6B0, lbl_803DB6B0, lbl_803DEEE4);
    PSMTXConcat(mtx_30, lbl_80396820, mtx_90);
    PSMTXTrans(mtx_30, 0.5f * (lbl_803DEEE4 - lbl_803DB6B0),
               0.5f * (lbl_803DEEE4 - lbl_803DB6B0), 0.0f);
    PSMTXConcat(mtx_30, mtx_90, mtx_90);
    GXLoadTexMtxImm(mtx_90, GX_PTTEXMTX6, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_POS, 0, GX_TRUE, GX_PTTEXMTX6);

    alpha_byte = (((ModelRenderOp*)renderOp)->alpha * ((u8*)obj_a)[0x37]) >> 8;
    ((u8*)&temp)[3] = alpha_byte;
    GXSetTevKColor(GX_KCOLOR0, temp);
    GXSetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K0_A);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    pcb = (void (*)(void*, void**, int))ObjModel_GetPostRenderCallback((ObjModel*)obj_b);
    if (pcb != 0)
    {
        pcb(obj_a, obj_b, slot);
    }
    else
    {
        u8 zCompLoc = 1;
        if (((u8*)obj_a)[0x37] < 0xff || (((ModelRenderOp*)renderOp)->flags & 0x40000000) != 0 ||
            ((ModelRenderOp*)renderOp)->alpha < 0xff)
        {
            GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
            if ((((ModelFileHeader*)model)->flags & 0x400) != 0)
            {
                if ((u32)gGxZModeCompareEnable != 0 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 ||
                    gGxZModeValid == 0)
                {
                    GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
                    gGxZModeCompareEnable = 0;
                    gGxZModeCompareFunc = 3;
                    gGxZModeUpdateEnable = 0;
                    gGxZModeValid = 1;
                }
                ((GXSetAlphaCompareIntFn)GXSetAlphaCompare)(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
            }
            else if ((((ModelFileHeader*)model)->flags & 0x2000) != 0)
            {
                zCompLoc = 0;
                if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 1 ||
                    gGxZModeValid == 0)
                {
                    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
                    gGxZModeCompareEnable = 1;
                    gGxZModeCompareFunc = 3;
                    gGxZModeUpdateEnable = 1;
                    gGxZModeValid = 1;
                }
                {
                    int b;
                    alpha_byte = objGetAlphaCompareThreshold();
                    b = objGetAlphaCompareThreshold();
                    ((GXSetAlphaCompareIntFn)GXSetAlphaCompare)(GX_GREATER, b, GX_AOP_AND, GX_GREATER, alpha_byte);
                }
            }
            else
            {
                if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 ||
                    gGxZModeValid == 0)
                {
                    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
                    gGxZModeCompareEnable = 1;
                    gGxZModeCompareFunc = 3;
                    gGxZModeUpdateEnable = 0;
                    gGxZModeValid = 1;
                }
                ((GXSetAlphaCompareIntFn)GXSetAlphaCompare)(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
            }
        }
        else
        {
            if ((((ModelRenderOp*)renderOp)->flags & 0x400) != 0)
            {
                GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
                if ((((ModelFileHeader*)model)->flags & 0x400) != 0)
                {
                    if ((u32)gGxZModeCompareEnable != 0 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 ||
                        gGxZModeValid == 0)
                    {
                        GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
                        gGxZModeCompareEnable = 0;
                        gGxZModeCompareFunc = 3;
                        gGxZModeUpdateEnable = 0;
                        gGxZModeValid = 1;
                    }
                }
                else
                {
                    if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 1 ||
                        gGxZModeValid == 0)
                    {
                        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
                        gGxZModeCompareEnable = 1;
                        gGxZModeCompareFunc = 3;
                        gGxZModeUpdateEnable = 1;
                        gGxZModeValid = 1;
                    }
                }
                ((GXSetAlphaCompareIntFn)GXSetAlphaCompare)(GX_GREATER, 0xC0, GX_AOP_AND, GX_GREATER, 0xC0);
            }
            else
            {
                GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
                if ((((ModelFileHeader*)model)->flags & 0x400) != 0)
                {
                    if ((u32)gGxZModeCompareEnable != 0 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 ||
                        gGxZModeValid == 0)
                    {
                        GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
                        gGxZModeCompareEnable = 0;
                        gGxZModeCompareFunc = 3;
                        gGxZModeUpdateEnable = 0;
                        gGxZModeValid = 1;
                    }
                }
                else
                {
                    if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 1 ||
                        gGxZModeValid == 0)
                    {
                        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
                        gGxZModeCompareEnable = 1;
                        gGxZModeCompareFunc = 3;
                        gGxZModeUpdateEnable = 1;
                        gGxZModeValid = 1;
                    }
                }
                ((GXSetAlphaCompareIntFn)GXSetAlphaCompare)(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
            }
        }
        if ((((ModelRenderOp*)renderOp)->flags & 0x400) != 0)
        {
            zCompLoc = 0;
        }
        if (gGxZCompLocCached != zCompLoc || gGxZCompLocValid == 0)
        {
            GXSetZCompLoc(zCompLoc);
            gGxZCompLocCached = zCompLoc;
            gGxZCompLocValid = 1;
        }
    }
    if ((((ModelRenderOp*)renderOp)->flags & 0x8) != 0)
    {
        GXSetCullMode(GX_CULL_BACK);
    }
    else
    {
        GXSetCullMode(GX_CULL_NONE);
    }
    return 1;
}

u32 objCallback_80074d04(int handle, void* model)
{
    extern f32 lbl_803DEEE4;

    Mtx mtx_ec;
    Mtx mtx_bc;
    Mtx mtx_8c;
    Mtx mtx_5c;
    f32 indMtx_44[6];
    f32 indMtx_2c[6];
    int handle1, handle2;
    f32 f1, f2;
    f32 f31_val;
    GXColor temp;
    f32* viewMtx;

    viewMtx = Camera_GetViewMatrix();
    if (model != 0)
    {
        ObjModelJointMatrix* jm = ObjModel_GetJointMatrix((u8*)model, 0);
        f32 px, py, pz, dist;
        PSMTXConcat((f32(*)[4])viewMtx, (f32(*)[4])jm, mtx_8c);
        px = mtx_8c[0][3];
        py = mtx_8c[1][3];
        pz = mtx_8c[2][3];
        dist = px * px + py * py + pz * pz;
        if (dist > 0.0f)
        {
            volatile float root;
            double guess = __frsqrte((double)dist);
            guess = 0.5 * guess * (3.0 - guess * guess * dist);
            guess = 0.5 * guess * (3.0 - guess * guess * dist);
            guess = 0.5 * guess * (3.0 - guess * guess * dist);
            root = (float)(dist * guess);
            dist = root;
        }
        f31_val = 200.0f / dist;
        if (f31_val > lbl_803DEEE4)
            f31_val = lbl_803DEEE4;
    }
    else
    {
        f31_val = lbl_803DEEE4;
    }

    selectReflectionTexture(0);
    GXLoadTexMtxImm(lbl_80396820, GX_PTTEXMTX6, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, 0, GX_FALSE, GX_PTTEXMTX6);
    newshadows_getReflectionScrollOffsets(&f1, &f2);
    f1 *= 4.0f;
    f2 *= 4.0f;
    getNewShadowCausticTexture((u32*)&handle1);
    selectTexture((Texture*)handle1, 1);

    PSMTXScale(mtx_ec, 4.0f, 4.0f, 4.0f);
    mtx_ec[0][3] = f1;
    GXLoadTexMtxImm(mtx_ec, GX_TEXMTX1, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);

    {
        f32 v = 0.5f * f31_val;
        indMtx_44[0] = v;
        indMtx_44[1] = 0.0f;
        indMtx_44[2] = 0.0f;
        indMtx_44[3] = 0.0f;
        indMtx_44[4] = v;
        indMtx_44[5] = 0.0f;
    }
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
    GXSetIndTexCoordScale(0, 0, 0);
    GXSetIndTexMtx(1, (f32(*)[3])indMtx_44, -4);
    GXSetTevIndirect(0, 0, 0, 7, 1, 6, 6, 0, 0, 0);

    PSMTXScale(mtx_bc, 0.83f, 0.83f, 0.83f);
    PSMTXRotRad(mtx_5c, 'z', 0.7853982f);
    PSMTXConcat(mtx_5c, mtx_bc, mtx_bc);
    mtx_bc[0][3] = f2;
    mtx_bc[1][3] = f2;
    GXLoadTexMtxImm(mtx_bc, GX_TEXMTX2, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX2, GX_FALSE, GX_PTIDENTITY);

    {
        f32 v44 = 0.3536f * f31_val;
        f32 v48 = -0.3536f * f31_val;
        indMtx_2c[0] = v44;
        indMtx_2c[1] = v44;
        indMtx_2c[2] = 0.0f;
        indMtx_2c[3] = v48;
        indMtx_2c[4] = v44;
        indMtx_2c[5] = 0.0f;
    }
    GXSetIndTexOrder(GX_INDTEXSTAGE1, GX_TEXCOORD2, GX_TEXMAP1);
    GXSetIndTexCoordScale(1, 0, 0);
    GXSetIndTexMtx(2, (f32(*)[3])indMtx_2c, -4);
    GXSetTevIndirect(1, 1, 0, 7, 2, 0, 0, 1, 0, 0);

    ((f32*)mtx_8c)[0] = lbl_803DB6AC;
    ((f32*)mtx_8c)[1] = 0.0f;
    ((f32*)mtx_8c)[2] = 0.0f;
    ((f32*)mtx_8c)[3] = 0.5f;
    ((f32*)mtx_8c)[4] = 0.0f;
    ((f32*)mtx_8c)[5] = lbl_803DB6AC;
    ((f32*)mtx_8c)[6] = 0.0f;
    ((f32*)mtx_8c)[7] = 0.5f;
    ((f32*)mtx_8c)[8] = 0.0f;
    ((f32*)mtx_8c)[9] = 0.0f;
    ((f32*)mtx_8c)[10] = 0.0f;
    ((f32*)mtx_8c)[11] = lbl_803DEEE4;
    GXLoadTexMtxImm((f32(*)[4])mtx_8c, GX_PTTEXMTX7, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD3, GX_TG_MTX3x4, GX_TG_NRM, GX_TEXMTX0, GX_FALSE, GX_PTTEXMTX7);

    getNewShadowDiskTexture((u32*)&handle2);
    selectTexture((Texture*)handle2, 2);

    GXSetNumIndStages(2);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(0);
    GXSetNumTexGens(4);
    GXSetNumTevStages(3);

    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    ((u8*)&temp)[3] = ((u8*)(int)handle)[0x37];
    GXSetTevKColor(GX_KCOLOR0, temp);
    GXSetTevKAlphaSel(GX_TEVSTAGE2, GX_TEV_KASEL_K0_A);
    GXSetTevDirect(GX_TEVSTAGE2);
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD3, GX_TEXMAP2, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
        gGxZModeCompareEnable = 1;
        gGxZModeCompareFunc = 3;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetCullMode(GX_CULL_BACK);
    return 1;
}

void hudDrawRect(int x1, int y1, int x2, int y2, GXColor color)
{
    f32 zero = 0.0f;

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetCullMode(GX_CULL_NONE);
    GXSetProjection(hudMatrix, GX_ORTHOGRAPHIC);
    if ((u32)gGxZModeCompareEnable != 0 || gGxZModeCompareFunc != 7 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        gGxZModeCompareEnable = 0;
        gGxZModeCompareFunc = 7;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    color.a = (u8)(((s32)color.a * gHudTintAlpha) >> 8);
    GXSetTevKColor(GX_KCOLOR0, color);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetChanCtrl(GX_COLOR0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_ALPHA0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(1);
    GXSetNumIndStages(0);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXBegin(GX_QUADS, GX_VTXFMT1, 4);

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = x1 << 2;
    GXWGFifo.s16 = y1 << 2;
    GXWGFifo.s16 = -8;
    GXWGFifo.f32 = zero;
    GXWGFifo.f32 = zero;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = x2 << 2;
    GXWGFifo.s16 = y1 << 2;
    GXWGFifo.s16 = -8;
    GXWGFifo.f32 = zero;
    GXWGFifo.f32 = zero;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = x2 << 2;
    GXWGFifo.s16 = y2 << 2;
    GXWGFifo.s16 = -8;
    GXWGFifo.f32 = zero;
    GXWGFifo.f32 = zero;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = x1 << 2;
    GXWGFifo.s16 = y2 << 2;
    GXWGFifo.s16 = -8;
    GXWGFifo.f32 = zero;
    GXWGFifo.f32 = zero;

    Camera_RebuildProjectionMatrix();
}

void drawViewFinderLine(f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3, f32 x4, f32 y4, GXColor* color)
{
    f32 zero = 0.0f;
    f32 scale = 4.0f;
    f32 fy4, fx4, fy3, fx3, fy2, fx2, fy1, fx1;
    fx1 = scale * x1;
    fy1 = scale * y1;
    fx2 = scale * x2;
    fy2 = scale * y2;
    fx3 = scale * x3;
    fy3 = scale * y3;
    fx4 = scale * x4;
    fy4 = scale * y4;

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetCullMode(GX_CULL_NONE);
    GXSetProjection(hudMatrix, GX_ORTHOGRAPHIC);
    if ((u32)gGxZModeCompareEnable != 0 || gGxZModeCompareFunc != 7 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        gGxZModeCompareEnable = 0;
        gGxZModeCompareFunc = 7;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    color->a = (u8)(((s32)color->a * gHudTintAlpha) >> 8);
    GXSetTevKColor(GX_KCOLOR0, *color);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetChanCtrl(GX_COLOR0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_ALPHA0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(1);
    GXSetNumIndStages(0);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXBegin(GX_QUADS, GX_VTXFMT1, 4);

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = fx1;
    GXWGFifo.s16 = fy1;
    GXWGFifo.s16 = -8;
    GXWGFifo.f32 = zero;
    GXWGFifo.f32 = zero;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = fx2;
    GXWGFifo.s16 = fy2;
    GXWGFifo.s16 = -8;
    GXWGFifo.f32 = zero;
    GXWGFifo.f32 = zero;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = fx3;
    GXWGFifo.s16 = fy3;
    GXWGFifo.s16 = -8;
    GXWGFifo.f32 = zero;
    GXWGFifo.f32 = zero;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = fx4;
    GXWGFifo.s16 = fy4;
    GXWGFifo.s16 = -8;
    GXWGFifo.f32 = zero;
    GXWGFifo.f32 = zero;

    Camera_RebuildProjectionMatrix();
}

void hudDrawTriangle(f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3, GXColor color)
{
    f32 zero = 0.0f;
    f32 scale = 4.0f;
    f32 fy3, fx3, fy2, fx2, fy1, fx1;
    fx1 = scale * x1;
    fy1 = scale * y1;
    fx2 = scale * x2;
    fy2 = scale * y2;
    fx3 = scale * x3;
    fy3 = scale * y3;

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetCullMode(GX_CULL_NONE);
    GXSetProjection(hudMatrix, GX_ORTHOGRAPHIC);
    if ((u32)gGxZModeCompareEnable != 0 || gGxZModeCompareFunc != 7 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        gGxZModeCompareEnable = 0;
        gGxZModeCompareFunc = 7;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    color.a = (u8)(((s32)color.a * gHudTintAlpha) >> 8);
    GXSetTevKColor(GX_KCOLOR0, color);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetChanCtrl(GX_COLOR0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_ALPHA0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(1);
    GXSetNumIndStages(0);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXBegin(GX_TRIANGLES, GX_VTXFMT1, 3);

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = fx1;
    GXWGFifo.s16 = fy1;
    GXWGFifo.s16 = -8;
    GXWGFifo.f32 = zero;
    GXWGFifo.f32 = zero;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = fx2;
    GXWGFifo.s16 = fy2;
    GXWGFifo.s16 = -8;
    GXWGFifo.f32 = zero;
    GXWGFifo.f32 = zero;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = fx3;
    GXWGFifo.s16 = fy3;
    GXWGFifo.s16 = -8;
    GXWGFifo.f32 = zero;
    GXWGFifo.f32 = zero;

    Camera_RebuildProjectionMatrix();
}

void skyDrawFn_80075d5c(int x1, int y1, int x2, int y2, f32 u1, f32 v1, f32 u2, f32 v2, int z)
{

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetCullMode(GX_CULL_NONE);
    GXSetProjection(hudMatrix, GX_ORTHOGRAPHIC);
    GXBegin(GX_QUADS, GX_VTXFMT1, 4);

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = x1;
    GXWGFifo.s16 = y1;
    GXWGFifo.s16 = z;
    GXWGFifo.f32 = u1;
    GXWGFifo.f32 = v1;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = x2;
    GXWGFifo.s16 = y1;
    GXWGFifo.s16 = z;
    GXWGFifo.f32 = u2;
    GXWGFifo.f32 = v1;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = x2;
    GXWGFifo.s16 = y2;
    GXWGFifo.s16 = z;
    GXWGFifo.f32 = u2;
    GXWGFifo.f32 = v2;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = x1;
    GXWGFifo.s16 = y2;
    GXWGFifo.s16 = z;
    GXWGFifo.f32 = u1;
    GXWGFifo.f32 = v2;

    Camera_RebuildProjectionMatrix();
}

void textRenderChar(int x1, int y1, int x2, int y2, f32 u1, f32 v1, f32 u2, f32 v2)
{

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetCullMode(GX_CULL_NONE);
    GXSetProjection(hudMatrix, GX_ORTHOGRAPHIC);
    GXBegin(GX_QUADS, GX_VTXFMT1, 4);

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = x1;
    GXWGFifo.s16 = y1;
    GXWGFifo.s16 = -8;
    GXWGFifo.f32 = u1;
    GXWGFifo.f32 = v1;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = x2;
    GXWGFifo.s16 = y1;
    GXWGFifo.s16 = -8;
    GXWGFifo.f32 = u2;
    GXWGFifo.f32 = v1;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = x2;
    GXWGFifo.s16 = y2;
    GXWGFifo.s16 = -8;
    GXWGFifo.f32 = u2;
    GXWGFifo.f32 = v2;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = x1;
    GXWGFifo.s16 = y2;
    GXWGFifo.s16 = -8;
    GXWGFifo.f32 = u1;
    GXWGFifo.f32 = v2;

    Camera_RebuildProjectionMatrix();
}

void drawPartialTexture(void* obj, f32 sx, f32 sy, int alpha_mod, int scale, int width, int height, int u_offset,
                        int v_offset)
{
    GXColor c;
    s32 alpha;
    s32 w;
    u16 drawScale;
    f32 u1, u0, v0, v1;

    c.r = 0xFF;
    c.g = 0xFF;
    c.b = 0xFF;
    alpha = (u8)alpha_mod;
    alpha *= gHudTintAlpha;
    c.a = (u8)(alpha >> 8);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetTevKColor(GX_KCOLOR0, c);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    if (((u32*)obj)[0x14] != 0)
    {
        GXSetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K0_A);
        GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL);
        GXSetTevDirect(GX_TEVSTAGE1);
        GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
        GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetNumTevStages(2);
    }
    else
    {
        GXSetNumTevStages(1);
    }
    GXSetNumIndStages(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(0);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    textureFn_8004c264((Texture*)obj, 0);
    GXSetCullMode(GX_CULL_NONE);
    GXSetProjection(hudMatrix, GX_ORTHOGRAPHIC);
    if ((u32)gGxZModeCompareEnable != 0 || gGxZModeCompareFunc != 7 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        gGxZModeCompareEnable = 0;
        gGxZModeCompareFunc = 7;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    drawScale = scale;
    w = (s32)(((u32)(width << 2) * drawScale) >> 8);
    sx = 4.0f * sx;
    sy = 4.0f * sy;
    u0 = (f32)(u32)u_offset / (f32)((u16*)obj)[5];
    v0 = (f32)(u32)v_offset / (f32)((u16*)obj)[6];
    u1 = (f32)(u32)(width + u_offset) / (f32)((u16*)obj)[5];
    v1 = (f32)(u32)(height + v_offset) / (f32)((u16*)obj)[6];

    GXBegin(GX_QUADS, GX_VTXFMT1, 4);

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = sx;
    GXWGFifo.s16 = sy;
    GXWGFifo.s16 = -8;
    GXWGFifo.f32 = u0;
    GXWGFifo.f32 = v0;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = (s16)(sx + (f32)(u32)w);
    GXWGFifo.s16 = sy;
    GXWGFifo.s16 = -8;
    GXWGFifo.f32 = u1;
    GXWGFifo.f32 = v0;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = (s16)(sx + (f32)(u32)w);
    GXWGFifo.s16 = (s16)(sy + (f32)(u32)(((u32)(height << 2) * drawScale) >> 8));
    GXWGFifo.s16 = -8;
    GXWGFifo.f32 = u1;
    GXWGFifo.f32 = v1;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = sx;
    GXWGFifo.s16 = (s16)(sy + (f32)(u32)(((u32)(height << 2) * drawScale) >> 8));
    GXWGFifo.s16 = -8;
    GXWGFifo.f32 = u0;
    GXWGFifo.f32 = v1;

    Camera_RebuildProjectionMatrix();
}

/*
 * Generic ortho-projected single-color quad blit. Sets the GX state up
 * fresh (no tex coords, color from constant K0, additive blend, fixed
 * 0x3C texmtx) then emits four GX_VTXFMT1 vertices at z=-0x18C with
 * width 4*size_x and height 4*size_y in screen pixels. Used as the
 * "draw fullscreen tint" primitive by the dialog code in cardShowLoadingMsg.
 */
void drawRect(f32 sx, f32 sy, int x, int y)
{

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetColorUpdate(GX_FALSE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ONE);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetChanCtrl(GX_COLOR0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_ALPHA0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(1);
    GXSetNumIndStages(0);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetCullMode(GX_CULL_NONE);
    GXSetProjection(hudMatrix, GX_ORTHOGRAPHIC);
    if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 7 || gGxZModeUpdateEnable != 1 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);
        gGxZModeCompareEnable = 1;
        gGxZModeCompareFunc = 7;
        gGxZModeUpdateEnable = 1;
        gGxZModeValid = 1;
    }
    if ((u32)gGxZCompLocCached != 0 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_FALSE);
        gGxZCompLocCached = 0;
        gGxZCompLocValid = 1;
    }
    GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
    GXSetCurrentMtx(GX_IDENTITY);
    sx = 4.0f * sx;
    sy = 4.0f * sy;
    GXBegin(GX_QUADS, GX_VTXFMT1, 4);

    GXWGFifo.s16 = sx;
    GXWGFifo.s16 = sy;
    GXWGFifo.s16 = -0x18C;

    GXWGFifo.s16 = (s16)(sx + (f32)((u32)x * 4));
    GXWGFifo.s16 = sy;
    GXWGFifo.s16 = -0x18C;

    GXWGFifo.s16 = (s16)(sx + (f32)((u32)x * 4));
    GXWGFifo.s16 = (s16)(sy + (f32)((u32)y * 4));
    GXWGFifo.s16 = -0x18C;

    GXWGFifo.s16 = sx;
    GXWGFifo.s16 = (s16)(sy + (f32)((u32)y * 4));
    GXWGFifo.s16 = -0x18C;

    Camera_RebuildProjectionMatrix();
    GXSetColorUpdate(GX_TRUE);
}

void drawScaledTexture(void* obj, f32 sx, f32 sy, int alpha_mod, int scale, int width, int height, int flags)
{
    GXColor c;
    s32 w, h;
    s32 alpha;
    f32 u0, u1, v0, v1;
    u8 fbits;

    c.r = 0xFF;
    c.g = 0xFF;
    c.b = 0xFF;
    alpha = (u8)alpha_mod;
    alpha *= gHudTintAlpha;
    c.a = (u8)(alpha >> 8);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetTevKColor(GX_KCOLOR0, c);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    if (((u32*)obj)[0x14] != 0)
    {
        GXSetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K0_A);
        GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL);
        GXSetTevDirect(GX_TEVSTAGE1);
        GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
        GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetNumTevStages(2);
    }
    else
    {
        GXSetNumTevStages(1);
    }
    GXSetNumIndStages(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(0);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    textureFn_8004c264((Texture*)obj, 0);
    GXSetCullMode(GX_CULL_NONE);
    GXSetProjection(hudMatrix, GX_ORTHOGRAPHIC);
    if ((u32)gGxZModeCompareEnable != 0 || gGxZModeCompareFunc != 7 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        gGxZModeCompareEnable = 0;
        gGxZModeCompareFunc = 7;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    fbits = (u8)flags;
    if ((fbits & 4) != 0)
    {
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    }
    else
    {
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    }
    w = (s32)(((u32)(width << 2) * (u16)scale) >> 8);
    h = (s32)(((u32)(height << 2) * (u16)scale) >> 8);
    sx = 4.0f * sx;
    sy = 4.0f * sy;
    {
        f32 ur = (f32)(u32)width / (f32)(u16)((u16*)obj)[5];
        f32 vr = (f32)(u32)height / (f32)(u16)((u16*)obj)[6];
        if ((fbits & 1) != 0)
        {
            u0 = ur;
            u1 = 0.0f;
        }
        else
        {
            u0 = 0.0f;
            u1 = ur;
        }
        if ((fbits & 2) != 0)
        {
            v0 = vr;
            v1 = 0.0f;
        }
        else
        {
            v0 = 0.0f;
            v1 = vr;
        }
    }
    GXBegin(GX_QUADS, GX_VTXFMT1, 4);

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = sx;
    GXWGFifo.s16 = sy;
    GXWGFifo.s16 = -8;
    GXWGFifo.f32 = u0;
    GXWGFifo.f32 = v0;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = (s16)(sx + (f32)(u32)w);
    GXWGFifo.s16 = sy;
    GXWGFifo.s16 = -8;
    GXWGFifo.f32 = u1;
    GXWGFifo.f32 = v0;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = (s16)(sx + (f32)(u32)w);
    GXWGFifo.s16 = (s16)(sy + (f32)(u32)h);
    GXWGFifo.s16 = -8;
    GXWGFifo.f32 = u1;
    GXWGFifo.f32 = v1;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = sx;
    GXWGFifo.s16 = (s16)(sy + (f32)(u32)h);
    GXWGFifo.s16 = -8;
    GXWGFifo.f32 = u0;
    GXWGFifo.f32 = v1;

    Camera_RebuildProjectionMatrix();
}

/*
 * Caller-coloured asset blit. Same mechanic as drawTexture but the K0
 * color comes from a writable GXColor the caller passes in (we apply the
 * gHudTintAlpha alpha tint to it in place). The flag arg picks between
 * "raster passthrough" (TevColorIn 0xF/0xF/0xF/0xE) and "K-tint replace"
 * (TevColorIn 0xF/0xE/0x8/0xF).
 */
void hudDrawColored(int obj, int x, int y, u32* color, int scale, int flag)
{
    f32 zero = 0.0f;
    extern const f32 lbl_803DEEE4;

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    ((GXColor*)color)->a = (u8)(((s32)((GXColor*)color)->a * gHudTintAlpha) >> 8);
    GXSetTevKColor(GX_KCOLOR0, *(GXColor*)color);
    GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevDirect(GX_TEVSTAGE0);
    if ((u8)flag != 0)
    {
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
    }
    else
    {
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
    }
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_4, GX_TRUE, GX_TEVPREV);
    if ((u32)((Texture*)obj)->imageOffset != 0)
    {
        GXSetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K0_A);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
        GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_4, GX_TRUE, GX_TEVPREV);
        GXSetNumTevStages(2);
    }
    else
    {
        GXSetNumTevStages(1);
    }
    GXSetNumIndStages(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(0);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    textureFn_8004c264((Texture*)obj, 0);
    GXSetCullMode(GX_CULL_NONE);
    GXSetProjection(hudMatrix, GX_ORTHOGRAPHIC);
    if ((u32)gGxZModeCompareEnable != 0 || gGxZModeCompareFunc != 7 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        gGxZModeCompareEnable = 0;
        gGxZModeCompareFunc = 7;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    if ((u8)flag != 0)
    {
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    }
    else
    {
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    }
    {
        s32 w, h;
        w = ((((Texture*)obj)->width << 2) * (u16)scale) / 256;
        h = ((((Texture*)obj)->height << 2) * (u16)scale) / 256;
        GXBegin(GX_QUADS, GX_VTXFMT1, 4);

        GXWGFifo.u8 = 0x3C;
        GXWGFifo.s16 = (s16)(x << 2);
        GXWGFifo.s16 = (s16)(y << 2);
        GXWGFifo.s16 = -8;
        GXWGFifo.f32 = zero;
        GXWGFifo.f32 = zero;

        GXWGFifo.u8 = 0x3C;
        GXWGFifo.s16 = (s16)((x << 2) + w);
        GXWGFifo.s16 = (s16)(y << 2);
        GXWGFifo.s16 = -8;
        GXWGFifo.f32 = *(const f32*)&lbl_803DEEE4;
        GXWGFifo.f32 = zero;

        GXWGFifo.u8 = 0x3C;
        GXWGFifo.s16 = (s16)((x << 2) + w);
        GXWGFifo.s16 = (s16)((y << 2) + h);
        GXWGFifo.s16 = -8;
        GXWGFifo.f32 = lbl_803DEEE4;
        GXWGFifo.f32 = lbl_803DEEE4;

        GXWGFifo.u8 = 0x3C;
        GXWGFifo.s16 = (s16)(x << 2);
        GXWGFifo.s16 = (s16)((y << 2) + h);
        GXWGFifo.s16 = -8;
        GXWGFifo.f32 = zero;
        GXWGFifo.f32 = lbl_803DEEE4;
    }
    Camera_RebuildProjectionMatrix();
}

/*
 * Quad-from-asset blit: takes an "asset record" (with width at +0xA,
 * height at +0xC, and an optional second-stage flag at +0x50), a per-
 * call alpha multiplier, screen-pos (sx, sy), and a u16 size scale.
 * Composes K0 from RGB(255,255,255) plus the global alpha tint
 * (alpha * gHudTintAlpha >> 8); if the asset opts in, layers a second
 * tex stage that further K-multiplies by the texture. Final width and
 * height are 4 * asset_dim * scale >> 8 in screen pixels at z=-8.
 */
void drawTexture(void* obj, f32 sx, f32 sy, int alpha_mod, int scale)
{
    f32 zero = 0.0f;
    extern const f32 lbl_803DEEE4;
    GXColor c;
    s32 w, h;
    s32 alpha;

    c.r = 0xFF;
    c.g = 0xFF;
    c.b = 0xFF;
    alpha = (u8)alpha_mod;
    alpha *= gHudTintAlpha;
    c.a = (u8)(alpha >> 8);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetTevKColor(GX_KCOLOR0, c);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    if (((u32*)obj)[0x14] != 0)
    {
        GXSetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K0_A);
        GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL);
        GXSetTevDirect(GX_TEVSTAGE1);
        GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
        GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetNumTevStages(2);
    }
    else
    {
        GXSetNumTevStages(1);
    }
    GXSetNumIndStages(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(0);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    textureFn_8004c264((Texture*)obj, 0);
    GXSetCullMode(GX_CULL_NONE);
    GXSetProjection(hudMatrix, GX_ORTHOGRAPHIC);
    if ((u32)gGxZModeCompareEnable != 0 || gGxZModeCompareFunc != 7 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        gGxZModeCompareEnable = 0;
        gGxZModeCompareFunc = 7;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    w = ((((u16*)obj)[5] << 2) * (u16)scale) / 256;
    h = ((((u16*)obj)[6] << 2) * (u16)scale) / 256;
    sx = 4.0f * sx;
    sy = 4.0f * sy;
    GXBegin(GX_QUADS, GX_VTXFMT1, 4);

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = sx;
    GXWGFifo.s16 = sy;
    GXWGFifo.s16 = -8;
    GXWGFifo.f32 = zero;
    GXWGFifo.f32 = zero;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = (s16)(sx + (f32)(u32)w);
    GXWGFifo.s16 = sy;
    GXWGFifo.s16 = -8;
    GXWGFifo.f32 = *(const f32*)&lbl_803DEEE4;
    GXWGFifo.f32 = zero;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = (s16)(sx + (f32)(u32)w);
    GXWGFifo.s16 = (s16)(sy + (f32)(u32)h);
    GXWGFifo.s16 = -8;
    GXWGFifo.f32 = lbl_803DEEE4;
    GXWGFifo.f32 = lbl_803DEEE4;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = sx;
    GXWGFifo.s16 = (s16)(sy + (f32)(u32)h);
    GXWGFifo.s16 = -8;
    GXWGFifo.f32 = zero;
    GXWGFifo.f32 = lbl_803DEEE4;

    Camera_RebuildProjectionMatrix();
}

void objectShadow_setupSwappedProjectedTexture(ProjectedShadowTexture* shadow, u32* colorPtr, Mtx mtx)
{
    Mtx tmp;

    GXSetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_ALPHA, GX_CH_RED, GX_CH_ALPHA, GX_CH_RED);
    PSMTXConcat(shadow->textureMtx, mtx, tmp);
    GXLoadTexMtxImm(tmp, GX_TEXMTX0, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    selectTexture(shadow->texture, 0);
    GXSetTevKColor(GX_KCOLOR0, *(GXColor*)colorPtr);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
    GXSetTevColor(GX_TEVREG1, *(GXColor*)&lbl_803DB6A8);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_A1, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP1);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVREG0);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_COMP_RGB8_GT, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetNumIndStages(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(0);
    GXSetNumTexGens(1);
    GXSetNumTevStages(1);
    if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
        gGxZModeCompareEnable = 1;
        gGxZModeCompareFunc = 3;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
}

void objectShadow_setupProjectedTexture(ProjectedShadowTexture* shadow, u32* colorPtr, Mtx mtx)
{
    Mtx tmp;

    PSMTXConcat(shadow->textureMtx, mtx, tmp);
    GXLoadTexMtxImm(tmp, GX_TEXMTX0, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    selectTexture(shadow->texture, 0);
    GXSetTevKColor(GX_KCOLOR0, *(GXColor*)colorPtr);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetNumIndStages(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(0);
    GXSetNumTexGens(1);
    GXSetNumTevStages(1);
    if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
        gGxZModeCompareEnable = 1;
        gGxZModeCompareFunc = 3;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
}

void objectShadow_setupProjectedTextureDepthFade(ProjectedShadowTexture* shadow, u32* colorPtr, Mtx mtx, f32 depth)
{
    extern f32 lbl_803DEEE4;
    Mtx m58;
    Mtx m28;
    Vec v;
    GXColor c;
    int handle;
    GXColor kc;
    f32 z;
    f32 d;
    f32 q;
    u8 t;

    kc = lbl_803E8454;
    PSMTXConcat(shadow->textureMtx, mtx, m58);
    GXLoadTexMtxImm(m58, GX_TEXMTX0, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    selectTexture(shadow->texture, 0);
    t = ((u8*)colorPtr)[3];
    ((u8*)colorPtr)[3] = (t >> 1) + (t >> 2);
    c.r = ((u8*)colorPtr)[3];
    c.g = ((u8*)colorPtr)[3];
    c.b = ((u8*)colorPtr)[3];
    GXSetTevKColor(GX_KCOLOR0, c);
    GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    v.x = mtx[0][3];
    v.y = mtx[1][3];
    v.z = mtx[2][3];
    PSMTXMultVec(shadow->depthMtx, &v, &v);
    z = -v.z;
    getNewShadowRampTexture((u32*)&handle);
    selectTexture((Texture*)handle, 1);
    m58[0][0] = 0.0f;
    m58[0][1] = 0.0f;
    d = z - depth;
    m58[0][2] = lbl_803DEEE4 / (q = z - d);
    m58[0][3] = z / q;
    m58[1][0] = 0.0f;
    m58[1][1] = 0.0f;
    m58[1][2] = 0.0f;
    m58[1][3] = 0.0f;
    PSMTXConcat(shadow->depthMtx, mtx, m28);
    PSMTXConcat(m58, m28, m28);
    GXLoadTexMtxImm(m28, GX_TEXMTX1, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_CPREV, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumIndStages(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(0);
    GXSetNumTexGens(2);
    GXSetNumTevStages(2);
    GXSetFog(GX_FOG_PERSP_EXP, gFogStartZ, gFogEndZ, gFogNearZ, gFogFarZ, kc);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_INVSRCCLR, GX_LO_NOOP);
    if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
        gGxZModeCompareEnable = 1;
        gGxZModeCompareFunc = 3;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
}

void objectShadow_setupProjectedTextureChannel(ProjectedShadowTexture* shadow, u32* colorPtr, Mtx mtx, f32 scale)
{
    extern f32 lbl_803DEEE4;
    typedef struct
    {
        u32 w[7];
    } Blk28;
    Mtx mtx_110;
    Mtx mtx_e0;
    Blk28 buf_c4;
    Blk28 buf_a8;
    Blk28 buf_8c;
    Blk28 buf_70;
    Blk28 buf_54;
    Blk28 buf_38;
    StageCountTable stab;
    GXColor temp;
    GXColor color2;
    f32 vec3[3];
    int handle;
    GXColor fog_var;
    int stage_idx;
    u32 stage_count;
    int stage_base;
    f32 f31_val;

    buf_c4 = *(Blk28*)&lbl_802C1EA8.blk[0];
    buf_a8 = *(Blk28*)&lbl_802C1EA8.blk[1];
    buf_8c = *(Blk28*)&lbl_802C1EA8.blk[2];
    buf_70 = *(Blk28*)&lbl_802C1EA8.blk[3];
    buf_54 = *(Blk28*)&lbl_802C1EA8.blk[4];
    buf_38 = *(Blk28*)&lbl_802C1EA8.blk[5];
    stab = sProjectedShadowStageCounts;
    *(u32*)&fog_var = lbl_803E8450;

    PSMTXConcat(shadow->textureMtx, mtx, mtx_110);
    GXLoadTexMtxImm(mtx_110, GX_TEXMTX0, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);

    selectTexture(shadow->texture, 0);

    if (shadow->mode < 8)
    {
        GXSetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_RED, GX_CH_RED, GX_CH_RED, GX_CH_RED);
        stage_idx = shadow->mode - 1;
    }
    else if (shadow->mode < 0x10)
    {
        GXSetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_ALPHA, GX_CH_ALPHA, GX_CH_ALPHA, GX_CH_ALPHA);
        stage_idx = shadow->mode - 9;
    }
    if (stage_idx < 0)
        stage_idx = 0;

    ((u8*)&color2)[0] = 0x7F;
    ((u8*)&color2)[1] = 0x7F;
    ((u8*)&color2)[2] = 0x7F;
    GXSetTevColor(GX_TEVREG0, color2);

    ((u8*)colorPtr)[3] = (u8)((((u8*)colorPtr)[3] >> 1) + (((u8*)colorPtr)[3] >> 2));
    ((u8*)&temp)[0] = ((u8*)colorPtr)[3];
    ((u8*)&temp)[1] = ((u8*)colorPtr)[3];
    ((u8*)&temp)[2] = ((u8*)colorPtr)[3];
    GXSetTevKColor(GX_KCOLOR0, temp);

    stage_base = 0;
    stage_count = stab.count[stage_idx];
    if (stage_count != 0)
    {
        GXSetTevDirect(GX_TEVSTAGE0);
        GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP1);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, buf_c4.w[stage_idx]);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, buf_a8.w[stage_idx], GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        stage_base = 1;
    }

    if (stage_count > 1)
    {
        GXSetTevDirect(stage_base);
        GXSetTevSwapMode(stage_base, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevOrder(stage_base, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
        GXSetTevColorIn(stage_base, GX_CC_ZERO, GX_CC_CPREV, GX_CC_ONE, buf_8c.w[stage_idx]);
        GXSetTevAlphaIn(stage_base, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
        GXSetTevColorOp(stage_base, GX_TEV_ADD, GX_TB_ZERO, buf_70.w[stage_idx], GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaOp(stage_base, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        stage_base++;
    }

    if (stage_count > 2)
    {
        GXSetTevDirect(stage_base);
        GXSetTevSwapMode(stage_base, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevOrder(stage_base, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
        GXSetTevColorIn(stage_base, GX_CC_ZERO, GX_CC_CPREV, GX_CC_ONE, buf_54.w[stage_idx]);
        GXSetTevAlphaIn(stage_base, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
        GXSetTevColorOp(stage_base, GX_TEV_ADD, GX_TB_ZERO, buf_38.w[stage_idx], GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaOp(stage_base, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        stage_base++;
    }

    GXSetTevDirect(stage_base);
    GXSetTevSwapMode(stage_base, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevKColorSel(stage_base, GX_TEV_KCSEL_K0);
    GXSetTevOrder(stage_base, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    if (stage_count == 0)
    {
        GXSetTevColorIn(stage_base, GX_CC_TEXC, GX_CC_C0, GX_CC_KONST, GX_CC_ZERO);
    }
    else
    {
        GXSetTevColorIn(stage_base, GX_CC_CPREV, GX_CC_C0, GX_CC_KONST, GX_CC_ZERO);
    }
    GXSetTevAlphaIn(stage_base, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevColorOp(stage_base, GX_TEV_COMP_R8_GT, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(stage_base, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    vec3[0] = mtx[0][3];
    vec3[1] = mtx[1][3];
    vec3[2] = mtx[2][3];
    PSMTXMultVec(shadow->depthMtx, (Vec*)vec3, (Vec*)vec3);
    f31_val = -vec3[2];

    getNewShadowRampTexture((u32*)&handle);
    selectTexture((Texture*)handle, 1);

    {
        f32 d2;
        mtx_110[0][0] = 0.0f;
        mtx_110[0][1] = 0.0f;
        mtx_110[0][2] = lbl_803DEEE4 / (d2 = f31_val - (f31_val - scale));
        mtx_110[0][3] = f31_val / d2;
        mtx_110[1][0] = 0.0f;
        mtx_110[1][1] = 0.0f;
        mtx_110[1][2] = 0.0f;
        mtx_110[1][3] = 0.0f;
    }
    PSMTXConcat(shadow->depthMtx, mtx, mtx_e0);
    PSMTXConcat(mtx_110, mtx_e0, mtx_e0);
    GXLoadTexMtxImm(mtx_e0, GX_TEXMTX1, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);

    GXSetTevDirect(stage_base + 1);
    GXSetTevSwapMode(stage_base + 1, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevOrder(stage_base + 1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
    GXSetTevColorIn(stage_base + 1, GX_CC_CPREV, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevAlphaIn(stage_base + 1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevColorOp(stage_base + 1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(stage_base + 1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXSetNumIndStages(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(0);
    GXSetNumTexGens(2);
    GXSetNumTevStages((u8)(stage_count + 2));

    GXSetFog(GX_FOG_PERSP_EXP, gFogStartZ, gFogEndZ, gFogNearZ, gFogFarZ, fog_var);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_INVSRCCLR, GX_LO_NOOP);

    if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
        gGxZModeCompareEnable = 1;
        gGxZModeCompareFunc = 3;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
}

void gxSetOpaqueZWriteMode(void)
{
    if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 1 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
        gGxZModeCompareEnable = 1;
        gGxZModeCompareFunc = 3;
        gGxZModeUpdateEnable = 1;
        gGxZModeValid = 1;
    }
    GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
}

void gxSetOpaqueNoZWriteMode(void)
{
    if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
        gGxZModeCompareEnable = 1;
        gGxZModeCompareFunc = 3;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
}

void gxBlendFn_800788dc(void)
{
    if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
        gGxZModeCompareEnable = 1;
        gGxZModeCompareFunc = 3;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
}

void gxBlendFn_800789ac(void)
{
    if ((u32)gGxZModeCompareEnable != 0 || gGxZModeCompareFunc != 7 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        gGxZModeCompareEnable = 0;
        gGxZModeCompareFunc = 7;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
}

void textBlendSetupFn_80078a7c(void)
{
    if ((u32)gGxZModeCompareEnable != 0 || gGxZModeCompareFunc != 7 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        gGxZModeCompareEnable = 0;
        gGxZModeCompareFunc = 7;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
}

void gxBlendFn_80078b4c(void)
{
    if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
        gGxZModeCompareEnable = 1;
        gGxZModeCompareFunc = 3;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
}

void gxDebugTextureFn_80078c1c(void)
{
    GXSetCullMode(GX_CULL_NONE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C0, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(0);
    GXSetNumTexGens(1);
    GXSetNumTevStages(1);
    if ((u32)gGxZModeCompareEnable != 0 || gGxZModeCompareFunc != 7 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        gGxZModeCompareEnable = 0;
        gGxZModeCompareFunc = 7;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
}

void gxTevModulateRasStage(void)
{
    GXSetTevOrder(gTevStageCursor, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevDirect(gTevStageCursor);
    GXSetTevColorIn(gTevStageCursor, GX_CC_ZERO, GX_CC_CPREV, GX_CC_RASC, GX_CC_ZERO);
    GXSetTevAlphaIn(gTevStageCursor, GX_CA_ZERO, GX_CA_APREV, GX_CA_RASA, GX_CA_ZERO);
    GXSetTevSwapMode(gTevStageCursor, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(gTevStageCursor, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(gTevStageCursor, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    gTevStageCursor += 1;
    gTevStageCount += 1;
    gTevChanCount += 1;
}

void gxTevRasTimesColor1Stage(void)
{
    GXSetTevOrder(gTevStageCursor, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevDirect(gTevStageCursor);
    GXSetTevColorIn(gTevStageCursor, GX_CC_ZERO, GX_CC_RASC, GX_CC_C1, GX_CC_ZERO);
    GXSetTevAlphaIn(gTevStageCursor, GX_CA_ZERO, GX_CA_RASA, GX_CA_A1, GX_CA_ZERO);
    GXSetTevSwapMode(gTevStageCursor, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(gTevStageCursor, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(gTevStageCursor, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    gTevStageCursor += 1;
    gTevStageCount += 1;
    gTevChanCount += 1;
}

void textRenderSetup(void)
{
    GXSetTevOrder(gTevStageCursor, gTevTexCoordCursor, gTevTexMapCursor, GX_COLOR_NULL);
    GXSetTevDirect(gTevStageCursor);
    GXSetTevColorIn(gTevStageCursor, GX_CC_C1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevAlphaIn(gTevStageCursor, GX_CA_ZERO, GX_CA_A1, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevSwapMode(gTevStageCursor, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(gTevStageCursor, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(gTevStageCursor, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTexCoordGen2(gTevTexCoordCursor, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    gTevStageCursor += 1;
    gTevStageCount += 1;
    gTevTexCoordCursor += 1;
    gTevTexGenCount += 1;
    gTevTexMapCursor += 1;
}

void gxTevAddColor1Stage(void)
{
    GXSetTevOrder(gTevStageCursor, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevDirect(gTevStageCursor);
    GXSetTevColorIn(gTevStageCursor, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C1);
    GXSetTevAlphaIn(gTevStageCursor, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A1);
    GXSetTevSwapMode(gTevStageCursor, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(gTevStageCursor, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(gTevStageCursor, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    gTevStageCursor += 1;
    gTevStageCount += 1;
    gTevChanCount += 1;
}

void gxTevPassRasStage(void)
{
    GXSetTevOrder(gTevStageCursor, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevDirect(gTevStageCursor);
    GXSetTevColorIn(gTevStageCursor, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
    GXSetTevAlphaIn(gTevStageCursor, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
    GXSetTevSwapMode(gTevStageCursor, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(gTevStageCursor, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(gTevStageCursor, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    gTevStageCursor += 1;
    gTevStageCount += 1;
    gTevChanCount += 1;
}

void gxTexColorFn_80079254(void)
{
    GXSetTevOrder(gTevStageCursor, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevDirect(gTevStageCursor);
    GXSetTevColorIn(gTevStageCursor, GX_CC_ZERO, GX_CC_CPREV, GX_CC_C1, GX_CC_ZERO);
    GXSetTevAlphaIn(gTevStageCursor, GX_CA_ZERO, GX_CA_APREV, GX_CA_A1, GX_CA_ZERO);
    GXSetTevSwapMode(gTevStageCursor, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(gTevStageCursor, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(gTevStageCursor, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    gTevStageCursor += 1;
    gTevStageCount += 1;
    gTevChanCount += 1;
}

void gxTevAddTextureFrameBlendStages(void)
{
    GXSetTevOrder(gTevStageCursor, gTevTexCoordCursor, gTevTexMapCursor, GX_COLOR_NULL);
    GXSetTevDirect(gTevStageCursor);
    GXSetTevColorIn(gTevStageCursor, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevAlphaIn(gTevStageCursor, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevSwapMode(gTevStageCursor, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(gTevStageCursor, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(gTevStageCursor, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    gTevStageCursor += 1;
    gTevStageCount += 1;
    gTevTexMapCursor += 1;
    GXSetTevOrder(gTevStageCursor, gTevTexCoordCursor, gTevTexMapCursor, GX_COLOR_NULL);
    GXSetTevDirect(gTevStageCursor);
    GXSetTevColorIn(gTevStageCursor, GX_CC_CPREV, GX_CC_TEXC, GX_CC_A0, GX_CC_ZERO);
    GXSetTevAlphaIn(gTevStageCursor, GX_CA_APREV, GX_CA_TEXA, GX_CA_A0, GX_CA_ZERO);
    GXSetTevSwapMode(gTevStageCursor, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(gTevStageCursor, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(gTevStageCursor, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTexCoordGen2(gTevTexCoordCursor, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    gTevStageCursor += 1;
    gTevStageCount += 1;
    gTevTexCoordCursor += 1;
    gTevTexGenCount += 1;
    gTevTexMapCursor += 1;
}

void gxTextureFn_800794e0(void)
{
    GXSetTevOrder(gTevStageCursor, gTevTexCoordCursor, gTevTexMapCursor, GX_COLOR_NULL);
    GXSetTevDirect(gTevStageCursor);
    GXSetTevColorIn(gTevStageCursor, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C1);
    GXSetTevAlphaIn(gTevStageCursor, GX_CA_ZERO, GX_CA_TEXA, GX_CA_A1, GX_CA_ZERO);
    GXSetTevSwapMode(gTevStageCursor, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(gTevStageCursor, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(gTevStageCursor, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTexCoordGen2(gTevTexCoordCursor, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    gTevStageCursor += 1;
    gTevStageCount += 1;
    gTevTexMapCursor += 1;
    gTevTexCoordCursor += 1;
    gTevTexGenCount += 1;
}

void textRenderSetupFn_800795e8(void)
{
    GXSetTevOrder(gTevStageCursor, gTevTexCoordCursor, gTevTexMapCursor, GX_COLOR_NULL);
    GXSetTevDirect(gTevStageCursor);
    GXSetTevColorIn(gTevStageCursor, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C1, GX_CC_ZERO);
    GXSetTevAlphaIn(gTevStageCursor, GX_CA_ZERO, GX_CA_TEXA, GX_CA_A1, GX_CA_ZERO);
    GXSetTevSwapMode(gTevStageCursor, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(gTevStageCursor, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(gTevStageCursor, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTexCoordGen2(gTevTexCoordCursor, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    gTevStageCursor += 1;
    gTevStageCount += 1;
    gTevTexMapCursor += 1;
    gTevTexCoordCursor += 1;
    gTevTexGenCount += 1;
}

void geomDrawFn_800796f0(void)
{
    GXSetTevOrder(gTevStageCursor, gTevTexCoordCursor, gTevTexMapCursor, GX_COLOR0A0);
    GXSetTevDirect(gTevStageCursor);
    GXSetTevColorIn(gTevStageCursor, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO);
    GXSetTevAlphaIn(gTevStageCursor, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
    GXSetTevSwapMode(gTevStageCursor, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(gTevStageCursor, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(gTevStageCursor, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTexCoordGen2(gTevTexCoordCursor, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    gTevStageCursor += 1;
    gTevStageCount += 1;
    gTevTexMapCursor += 1;
    gTevTexCoordCursor += 1;
    gTevTexGenCount += 1;
    gTevChanCount += 1;
}

/*
 * Closes out the TEV pipeline configuration that drawViewFinderAperture etc. open:
 * pushes the current ind-stage / chan-ctrl / tex-gen counts in
 * gTevIndStageCount..00B back into GX, and if the global tint alpha
 * gHudTintAlpha isn't fully transparent (0xFF) appends one final TEV
 * stage that K-multiplies the tint over the existing color, advancing
 * gTevStageCursor (TEV stage cursor) and gTevStageCount (stage count).
 */
void textRenderSetupFn_80079804(void)
{
    GXColor c;

    GXSetNumIndStages(gTevIndStageCount);
    if (gTevChanCount != 0)
    {
        GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
        GXSetNumChans(1);
    }
    else
    {
        GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
        GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
        GXSetNumChans(0);
    }
    GXSetNumTexGens(gTevTexGenCount);
    if (gHudTintAlpha < 0xFF)
    {
        c.a = gHudTintAlpha;
        GXSetTevKColor(GX_KCOLOR0, c);
        GXSetTevKAlphaSel(gTevStageCursor, GX_TEV_KASEL_K0_A);
        GXSetTevOrder(gTevStageCursor, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
        GXSetTevDirect(gTevStageCursor);
        GXSetTevColorIn(gTevStageCursor, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
        GXSetTevAlphaIn(gTevStageCursor, GX_CA_ZERO, GX_CA_APREV, GX_CA_KONST, GX_CA_ZERO);
        GXSetTevSwapMode(gTevStageCursor, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(gTevStageCursor, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaOp(gTevStageCursor, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        gTevStageCursor = gTevStageCursor + 1;
        gTevStageCount++;
    }
    GXSetNumTevStages(gTevStageCount);
    if (gTevChanCount != 0)
    {
        GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
    }
}

void textureSetupFn_800799c0(void)
{
    gTevIndStageCount = 0;
    gTevChanCount = 0;
    gTevTexGenCount = 0;
    gTevStageCount = 0;
    gTevStageCursor = 0;
    gTevTexCoordCursor = 0;
    gTevTexMapCursor = 0;
}

void _gxSetTevColor2(u8 r, u8 g, u8 b, u8 a)
{
    GXColor c;
    c.r = r;
    c.g = g;
    c.b = b;
    c.a = a;
    GXSetTevColor(GX_TEVREG1, c);
}

void _gxSetTevColor1(u8 r, u8 g, u8 b, u8 a)
{
    GXColor c;
    c.r = r;
    c.g = g;
    c.b = b;
    c.a = a;
    GXSetTevColor(GX_TEVREG0, c);
}

/*
 * Fullscreen 640x480 texture-tinted quad with shape-controlled alpha:
 * `flag != 0` lights the screen with three pre-set GXColors stamped into
 * K0/T1/T2; `flag == 0` instead does a single K0 modulate where K0's
 * alpha is the caller's byte divided by 4. Builds a per-call 3x4 tex
 * coord matrix that scales the source texture by 1/sx and 1/sy with a
 * sub-pixel offset baked from -320.0f/50.
 */
void drawViewFinderAperture(f32 sx, f32 sy, u8 a, u8 flag)
{
    extern f32 lbl_803DEEE4;
    Texture* handle;
    GXColor c0, c1, c2;
    Mtx mtx;

    c0 = sApertureColorBlack;
    c1 = sApertureColorEdge;
    c2 = sApertureColorCentre;
    getNewShadowRadialTexture(&handle);
    selectTexture(handle, 0);
    {
        f32 dec = 0.5f;
        f32 zero = 0.0f;
        f32 inv_sx = dec / sx;
        f32 inv_sy = dec / sy;
        mtx[0][0] = inv_sx;
        mtx[0][1] = zero;
        mtx[0][2] = zero;
        mtx[0][3] = -320.0f * inv_sx + dec;
        mtx[1][0] = zero;
        mtx[1][1] = inv_sy;
        mtx[1][2] = zero;
        mtx[1][3] = -240.0f * inv_sy + dec;
        mtx[2][0] = zero;
        mtx[2][1] = zero;
        mtx[2][2] = zero;
        mtx[2][3] = lbl_803DEEE4;
    }
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXLoadTexMtxImm(mtx, GX_TEXMTX0, GX_MTX2x4);
    GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    if (flag != 0)
    {
        c0.a = a;
        GXSetTevKColor(GX_KCOLOR0, c0);
        GXSetTevColor(GX_TEVREG0, c1);
        GXSetTevColor(GX_TEVREG1, c2);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_TEXA, GX_CA_A0, GX_CA_A1, GX_CA_KONST);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_COMP_RGB8_GT, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    }
    else
    {
        c0.a = (u8)((s32)a >> 2);
        GXSetTevKColor(GX_KCOLOR0, c0);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_TEXA, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_4, GX_TRUE, GX_TEVPREV);
    }
    GXSetNumTexGens(1);
    GXSetNumTevStages(1);
    GXSetNumIndStages(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(0);
    GXClearVtxDesc();
    GXSetCurrentMtx(GX_IDENTITY);
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetCullMode(GX_CULL_NONE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_INVSRCALPHA, GX_BL_SRCALPHA, GX_LO_NOOP);
    if ((u32)gGxZModeCompareEnable != 0 || gGxZModeCompareFunc != 7 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        gGxZModeCompareEnable = 0;
        gGxZModeCompareFunc = 7;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetProjection(hudMatrix, GX_ORTHOGRAPHIC);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);

    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = -8;

    GXWGFifo.s16 = 0x280;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = -8;

    GXWGFifo.s16 = 0x280;
    GXWGFifo.s16 = 0x1E0;
    GXWGFifo.s16 = -8;

    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0x1E0;
    GXWGFifo.s16 = -8;

    Camera_RebuildProjectionMatrix();
    GXSetCurrentMtx(GX_PNMTX0);
}

void drawFn_80079e64(f32 s1, u8 mtxIdx, void* vec, f32 s2, u8 alpha0, u8 alpha1, f32 s3)
{
    extern f32 lbl_803DEEE4;
    Mtx mtx_58;
    Mtx mtx_28;
    int handle1;
    int handle2;
    f32 ratio1;
    f32 angle;
    f32 ratio2;
    f32 fade1;
    f32 fade2;
    GXColor c_K2;
    GXColor c_K0;
    GXColor c_K1;

    c_K0.a = alpha0;
    c_K1.a = alpha1;
    ratio1 = ((f32)(u32)Camera_GetCurrentViewYaw() - 32768.0f) / 8192.0f;
    ratio2 = ((f32)(u32)Camera_GetCurrentViewPitch() - 32768.0f) / 8192.0f;
    if (getHudHiddenFrameCount() != 0)
    {
        angle = lbl_803DD00C;
    }
    else
    {
        f32 t = atanf_fast(((f32*)vec)[0] / ((f32*)vec)[1]);
        angle = lbl_803DD00C + interpolate(t - lbl_803DD00C, 0.05f, timeDelta);
        lbl_803DD00C = angle;
    }
    c_K2.a = mtxIdx;

    getReflectionTexture2((u32*)&handle1);
    selectTexture((Texture*)handle1, 0);
    getNewShadowSnowFlashTexture((u32*)&handle2);
    selectTexture((Texture*)handle2, 1);

    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);

    PSMTXScale(mtx_58, 6.0f * (f32)s2, 6.0f * (f32)s2, 0.0f);
    PSMTXTrans(mtx_28, ratio1 * s3, ratio2 * s3 + (f32)s1, 0.0f);
    PSMTXConcat(mtx_28, mtx_58, mtx_58);
    PSMTXRotRad(mtx_28, 'z', angle);
    PSMTXConcat(mtx_58, mtx_28, mtx_58);
    PSMTXTrans(mtx_28, -0.5f, -0.5f, 0.0f);
    PSMTXConcat(mtx_58, mtx_28, mtx_58);
    GXLoadTexMtxImm(mtx_58, GX_TEXMTX0, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);

    PSMTXScale(mtx_58, 12.0f * (f32)s2, 12.0f * (f32)s2, 0.0f);
    fade1 = lbl_803DEEE0 * ratio1;
    fade2 = *(f32*)&lbl_803DEEE0 * ratio2;
    PSMTXTrans(mtx_28, fade1 * s3, 0.75f * (f32)s1 + fade2 * s3, 0.0f);
    PSMTXConcat(mtx_28, mtx_58, mtx_58);
    PSMTXRotRad(mtx_28, 'z', 0.5f * angle);
    PSMTXConcat(mtx_58, mtx_28, mtx_58);
    PSMTXTrans(mtx_28, -0.5f, -0.5f, 0.0f);
    PSMTXConcat(mtx_58, mtx_28, mtx_58);
    GXLoadTexMtxImm(mtx_58, GX_TEXMTX1, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);

    GXSetTevKColor(GX_KCOLOR0, c_K0);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_KONST, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_4, GX_TRUE, GX_TEVPREV);

    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_APREV, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, GX_TRUE, GX_TEVPREV);

    GXSetTevKColor(GX_KCOLOR1, c_K1);
    GXSetTevKAlphaSel(GX_TEVSTAGE2, GX_TEV_KASEL_K1_A);
    GXSetTevDirect(GX_TEVSTAGE2);
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_KONST, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
    GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_4, GX_TRUE, GX_TEVREG0);

    GXSetTevDirect(GX_TEVSTAGE3);
    GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD2, GX_TEXMAP1, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE3, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE3, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE3, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
    GXSetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_4, GX_TRUE, GX_TEVREG0);

    GXSetTevKAlphaSel(GX_TEVSTAGE4, GX_TEV_KASEL_1);
    GXSetTevDirect(GX_TEVSTAGE4);
    GXSetTevOrder(GX_TEVSTAGE4, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE4, GX_CC_CPREV, GX_CC_C0, GX_CC_A0, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE4, GX_CA_APREV, GX_CA_KONST, GX_CA_A0, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE4, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE4, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE4, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXSetTevKColor(GX_KCOLOR2, c_K2);
    GXSetTevKAlphaSel(GX_TEVSTAGE5, GX_TEV_KASEL_K2_A);
    GXSetTevDirect(GX_TEVSTAGE5);
    GXSetTevOrder(GX_TEVSTAGE5, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE5, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE5, GX_CA_ZERO, GX_CA_APREV, GX_CA_KONST, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE5, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE5, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE5, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXSetNumTexGens(3);
    GXSetNumTevStages(6);
    GXSetNumIndStages(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(0);

    GXClearVtxDesc();
    GXSetCurrentMtx(GX_IDENTITY);
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetCullMode(GX_CULL_NONE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 1 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_TRUE, GX_LESS, GX_FALSE);
        gGxZModeCompareEnable = 1;
        gGxZModeCompareFunc = 1;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetProjection(hudMatrix, GX_ORTHOGRAPHIC);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);

    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = -8;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0;

    GXWGFifo.s16 = 0x280;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = -8;
    GXWGFifo.s16 = 0x80;
    GXWGFifo.s16 = 0;

    GXWGFifo.s16 = 0x280;
    GXWGFifo.s16 = 0x1E0;
    GXWGFifo.s16 = -8;
    GXWGFifo.s16 = 0x80;
    GXWGFifo.s16 = 0x80;

    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0x1E0;
    GXWGFifo.s16 = -8;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0x80;

    Camera_RebuildProjectionMatrix();
    GXSetCurrentMtx(GX_PNMTX0);
}

void doHeatEffect(u8 alpha)
{
    extern f32 lbl_803DEEE4;
    Mtx mtx_44;
    f32 indMtx[6];
    int handle2;
    int handle1;
    f32 fA;
    f32 fB;
    f32 mulY;
    f32 mulX;
    s16 v;
    u8 k;
    u8 a2;
    u8 a1;

    *(IndMtxInit*)indMtx = lbl_802C1EA8.ind;
    v = (s16)Camera_GetCurrentViewPitch();
    if (v < 0)
    {
        k = (((u16)(int)v >> 8) - 0xc0) << 2;
    }
    else
    {
        k = 0xff;
    }
    a1 = (alpha * 0xff) >> 8;
    a2 = (k * alpha) >> 8;

    selectReflectionTexture(0);
    getReflectionTexture2((u32*)&handle1);
    selectTexture((Texture*)handle1, 1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);

    newshadows_getReflectionScrollOffsets(&fA, &fB);
    fA *= 10.0f;
    fB *= 10.0f;
    getNewShadowCausticTexture((u32*)&handle2);
    selectTexture((Texture*)handle2, 2);

    mathSinCosf(3.142f * fA, &mulX, &mulY);
    mulY *= 0.5f;
    mulX *= 0.5f;

    indMtx[0] = mulY;
    indMtx[1] = mulX;
    indMtx[3] = -mulX;
    indMtx[4] = mulY;

    PSMTXScale(mtx_44, 7.0f, 7.0f, lbl_803DEEE4);
    mtx_44[0][3] = fA;
    mtx_44[1][3] = -fB;
    GXLoadTexMtxImm(mtx_44, GX_PTTEXMTX0, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTTEXMTX0);

    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP2);
    GXSetIndTexCoordScale(0, 0, 0);
    GXSetIndTexMtx(1, (f32(*)[3])indMtx, -6);
    GXSetTevIndirect(1, 0, 0, 7, 1, 0, 0, 0, 0, 0);

    GXSetTevKColor(GX_KCOLOR0, lbl_803DB6A4);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_KONST, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_4, GX_TRUE, GX_TEVPREV);

    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_4, GX_TRUE, GX_TEVPREV);

    GXSetTevDirect(GX_TEVSTAGE2);
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_APREV, GX_CA_RASA, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_4, GX_TRUE, GX_TEVPREV);

    GXSetNumTexGens(2);
    GXSetNumTevStages(3);
    GXSetNumIndStages(1);
    GXSetNumChans(1);
    GXClearVtxDesc();
    GXSetCurrentMtx(GX_IDENTITY);
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetCullMode(GX_CULL_NONE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 1 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_TRUE, GX_LESS, GX_FALSE);
        gGxZModeCompareEnable = 1;
        gGxZModeCompareFunc = 1;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetProjection(hudMatrix, 1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = -8;
    GXWGFifo.u8 = 0;
    GXWGFifo.u8 = 0;
    GXWGFifo.u8 = 0;
    GXWGFifo.u8 = a2;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0x280;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = -8;
    GXWGFifo.u8 = 0;
    GXWGFifo.u8 = 0;
    GXWGFifo.u8 = 0;
    GXWGFifo.u8 = a2;
    GXWGFifo.s16 = 0x80;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0x280;
    GXWGFifo.s16 = 0x1e0;
    GXWGFifo.s16 = -8;
    GXWGFifo.u8 = 0;
    GXWGFifo.u8 = 0;
    GXWGFifo.u8 = 0;
    GXWGFifo.u8 = a1;
    GXWGFifo.s16 = 0x80;
    GXWGFifo.s16 = 0x80;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0x1e0;
    GXWGFifo.s16 = -8;
    GXWGFifo.u8 = 0;
    GXWGFifo.u8 = 0;
    GXWGFifo.u8 = 0;
    GXWGFifo.u8 = a1;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0x80;
    Camera_RebuildProjectionMatrix();
    GXSetCurrentMtx(GX_PNMTX0);
}

/*
 * Fullscreen 640x480 textured quad with caller-supplied alpha. The alpha
 * is multiplied by 255.0f (a 0..255 scale), converted to int and
 * stamped into byte 3 of the K0 GXColor cache (lbl_803DB6A0). Sets up
 * one TEV stage that K-multiplies the texture by alpha; uses fixed UVs
 * 0..0x80 so the texture maps once across the screen. Used when fading
 * the screen to texture (e.g. boot logo / "now loading").
 */
void renderMotionBlur(f32 alpha)
{
    Mtx mtx;

    lbl_803DB6A0.a = 255.0f * alpha;
    selectReflectionTexture(0);
    GXSetTevKColor(GX_KCOLOR0, lbl_803DB6A0);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    PSMTXIdentity(mtx);
    GXLoadTexMtxImm(mtx, GX_TEXMTX2, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetCullMode(GX_CULL_NONE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    if ((u32)gGxZModeCompareEnable != 0 || gGxZModeCompareFunc != 7 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        gGxZModeCompareEnable = 0;
        gGxZModeCompareFunc = 7;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetProjection(hudMatrix, GX_ORTHOGRAPHIC);
    GXSetNumTexGens(1);
    GXSetNumTevStages(1);
    GXSetNumIndStages(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(0);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_ZERO);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = -8;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = 0x280;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = -8;
    GXWGFifo.s16 = 0x80;
    GXWGFifo.s16 = 0;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = 0x280;
    GXWGFifo.s16 = 0x1E0;
    GXWGFifo.s16 = -8;
    GXWGFifo.s16 = 0x80;
    GXWGFifo.s16 = 0x80;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0x1E0;
    GXWGFifo.s16 = -8;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0x80;

    Camera_RebuildProjectionMatrix();
}

void doBlurFilter(f32 wx, f32 wy, f32 wz, u8 param4, u8 param5)
{
    extern f32 lbl_803DEEE4;
    Mtx mtx_27;
    Mtx mtx_24;
    Mtx mtx_2A;
    Mtx mtx_2D;
    Mtx mtx_30;
    GXColor c1;
    GXColor c0;
    int handle;
    f32 pz, px, py, pw;
    int stage_base;

    wx = wx - playerMapOffsetX;
    wz = wz - playerMapOffsetZ;
    Camera_ProjectWorldPoint(wx, wy, wz, &px, &py, &pz, &pw);
    pz = pz + lbl_803DEEE4;
    c0.a = (u8)(((u32)(16777216.0f * pz) & 0x00FF0000) >> 16);
    selectReflectionTexture(0);
    getReflectionTexture2((u32*)&handle);
    selectTexture((Texture*)handle, 1);
    GXSetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_RED, GX_CH_RED, GX_CH_RED, GX_CH_GREEN);

    PSMTXIdentity(mtx_24);
    mtx_24[1][3] = -0.0041666667f;
    GXLoadTexMtxImm(mtx_24, GX_TEXMTX2, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX2, GX_FALSE, GX_PTIDENTITY);

    PSMTXIdentity(mtx_2A);
    mtx_2A[1][3] = -0.0041666667f;
    GXLoadTexMtxImm(mtx_2A, GX_TEXMTX4, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX4, GX_FALSE, GX_PTIDENTITY);

    PSMTXIdentity(mtx_2D);
    mtx_2D[0][3] = 0.003125f;
    GXLoadTexMtxImm(mtx_2D, GX_TEXMTX5, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD3, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX5, GX_FALSE, GX_PTIDENTITY);

    PSMTXIdentity(mtx_30);
    mtx_30[0][3] = -0.003125f;
    GXLoadTexMtxImm(mtx_30, GX_TEXMTX6, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD4, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX6, GX_FALSE, GX_PTIDENTITY);

    GXSetTexCoordGen2(GX_TEXCOORD5, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);

    PSMTXIdentity(mtx_27);
    GXLoadTexMtxImm(mtx_27, GX_TEXMTX3, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX3, GX_FALSE, GX_PTIDENTITY);

    GXSetTevKColor(GX_KCOLOR0, c0);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    c1 = *(GXColor*)&lbl_803DB69C;
    GXSetTevKColor(GX_KCOLOR1, c1);

    GXSetNumTexGens(6);
    GXSetNumIndStages(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(0);

    stage_base = 0;
    if (param5 == 0)
    {
        if (param4 == 0)
        {
            GXSetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K0_A);
            GXSetNumTevStages(7);

            GXSetTevDirect(GX_TEVSTAGE0);
            GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
            GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
            GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_TEXA, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
            GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
            GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG2);
            GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_SUB, GX_TB_ZERO, GX_CS_DIVIDE_2, GX_TRUE, GX_TEVREG2);
            stage_base = 1;
        }
        else
        {
            GXSetNumTevStages(6);
        }

        GXSetTevDirect(stage_base);
        GXSetTevOrder(stage_base, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
        GXSetTevColorIn(stage_base, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
        GXSetTevAlphaIn(stage_base, GX_CA_KONST, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
        GXSetTevSwapMode(stage_base, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(stage_base, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaOp(stage_base, GX_TEV_SUB, GX_TB_ZERO, GX_CS_DIVIDE_2, GX_TRUE, GX_TEVPREV);

        GXSetTevKColorSel(stage_base + 1, GX_TEV_KCSEL_K1);
        GXSetTevDirect(stage_base + 1);
        GXSetTevOrder(stage_base + 1, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevColorIn(stage_base + 1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
        if (param4 == 0)
        {
            GXSetTevAlphaIn(stage_base + 1, GX_CA_APREV, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A2);
        }
        else
        {
            GXSetTevAlphaIn(stage_base + 1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
        }
        GXSetTevSwapMode(stage_base + 1, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(stage_base + 1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaOp(stage_base + 1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_DIVIDE_2, GX_TRUE, GX_TEVPREV);

        GXSetTevKColorSel(stage_base + 2, GX_TEV_KCSEL_K1);
        GXSetTevDirect(stage_base + 2);
        GXSetTevOrder(stage_base + 2, GX_TEXCOORD2, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevColorIn(stage_base + 2, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
        GXSetTevAlphaIn(stage_base + 2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
        GXSetTevSwapMode(stage_base + 2, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(stage_base + 2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaOp(stage_base + 2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_4, GX_TRUE, GX_TEVPREV);

        GXSetTevKColorSel(stage_base + 3, GX_TEV_KCSEL_K1);
        GXSetTevDirect(stage_base + 3);
        GXSetTevOrder(stage_base + 3, GX_TEXCOORD3, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevColorIn(stage_base + 3, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
        GXSetTevAlphaIn(stage_base + 3, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
        GXSetTevSwapMode(stage_base + 3, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(stage_base + 3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaOp(stage_base + 3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_4, GX_TRUE, GX_TEVPREV);

        GXSetTevKColorSel(stage_base + 4, GX_TEV_KCSEL_K1);
        GXSetTevDirect(stage_base + 4);
        GXSetTevOrder(stage_base + 4, GX_TEXCOORD4, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevColorIn(stage_base + 4, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
        GXSetTevAlphaIn(stage_base + 4, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
        GXSetTevSwapMode(stage_base + 4, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(stage_base + 4, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaOp(stage_base + 4, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_4, GX_TRUE, GX_TEVPREV);

        GXSetTevKColorSel(stage_base + 5, GX_TEV_KCSEL_K1);
        GXSetTevDirect(stage_base + 5);
        GXSetTevOrder(stage_base + 5, GX_TEXCOORD5, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevColorIn(stage_base + 5, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
        GXSetTevAlphaIn(stage_base + 5, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
        GXSetTevSwapMode(stage_base + 5, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(stage_base + 5, GX_TEV_ADD, GX_TB_ZERO, GX_CS_DIVIDE_2, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaOp(stage_base + 5, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_4, GX_TRUE, GX_TEVPREV);
    }
    else
    {
        GXSetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K0_A);
        GXSetNumTevStages(7);

        GXSetTevDirect(GX_TEVSTAGE0);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_TEXA, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
        GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG2);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG2);

        GXSetTevDirect(GX_TEVSTAGE1);
        GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
        GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_KONST, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
        GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

        GXSetTevKColorSel(GX_TEVSTAGE2, GX_TEV_KCSEL_K1);
        GXSetTevDirect(GX_TEVSTAGE2);
        GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
        GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_APREV, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A2);
        GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_4, GX_TRUE, GX_TEVPREV);

        GXSetTevKColorSel(GX_TEVSTAGE3, GX_TEV_KCSEL_K1);
        GXSetTevDirect(GX_TEVSTAGE3);
        GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD2, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE3, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE3, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
        GXSetTevSwapMode(GX_TEVSTAGE3, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_4, GX_TRUE, GX_TEVPREV);

        GXSetTevKColorSel(GX_TEVSTAGE4, GX_TEV_KCSEL_K1);
        GXSetTevDirect(GX_TEVSTAGE4);
        GXSetTevOrder(GX_TEVSTAGE4, GX_TEXCOORD3, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE4, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE4, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
        GXSetTevSwapMode(GX_TEVSTAGE4, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(GX_TEVSTAGE4, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaOp(GX_TEVSTAGE4, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_4, GX_TRUE, GX_TEVPREV);

        GXSetTevKColorSel(GX_TEVSTAGE5, GX_TEV_KCSEL_K1);
        GXSetTevDirect(GX_TEVSTAGE5);
        GXSetTevOrder(GX_TEVSTAGE5, GX_TEXCOORD4, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE5, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE5, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
        GXSetTevSwapMode(GX_TEVSTAGE5, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(GX_TEVSTAGE5, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaOp(GX_TEVSTAGE5, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_4, GX_TRUE, GX_TEVPREV);

        GXSetTevKColorSel(GX_TEVSTAGE6, GX_TEV_KCSEL_K1);
        GXSetTevDirect(GX_TEVSTAGE6);
        GXSetTevOrder(GX_TEVSTAGE6, GX_TEXCOORD5, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE6, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE6, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
        GXSetTevSwapMode(GX_TEVSTAGE6, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(GX_TEVSTAGE6, GX_TEV_ADD, GX_TB_ZERO, GX_CS_DIVIDE_2, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaOp(GX_TEVSTAGE6, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    }

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetCullMode(GX_CULL_NONE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    if ((u32)gGxZModeCompareEnable != 0 || gGxZModeCompareFunc != 7 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        gGxZModeCompareEnable = 0;
        gGxZModeCompareFunc = 7;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetProjection(hudMatrix, GX_ORTHOGRAPHIC);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = -8;
    GXWGFifo.u8 = 0xFF;
    GXWGFifo.u8 = 0xFF;
    GXWGFifo.u8 = 0xFF;
    GXWGFifo.u8 = 0xFF;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = 0x280;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = -8;
    GXWGFifo.u8 = 0xFF;
    GXWGFifo.u8 = 0xFF;
    GXWGFifo.u8 = 0xFF;
    GXWGFifo.u8 = 0xFF;
    GXWGFifo.s16 = 0x80;
    GXWGFifo.s16 = 0;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = 0x280;
    GXWGFifo.s16 = 0x1E0;
    GXWGFifo.s16 = -8;
    GXWGFifo.u8 = 0xFF;
    GXWGFifo.u8 = 0xFF;
    GXWGFifo.u8 = 0xFF;
    GXWGFifo.u8 = 0xFF;
    GXWGFifo.s16 = 0x80;
    GXWGFifo.s16 = 0x80;

    GXWGFifo.u8 = 0x3C;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0x1E0;
    GXWGFifo.s16 = -8;
    GXWGFifo.u8 = 0xFF;
    GXWGFifo.u8 = 0xFF;
    GXWGFifo.u8 = 0xFF;
    GXWGFifo.u8 = 0xFF;
    GXWGFifo.s16 = 0;
    GXWGFifo.s16 = 0x80;

    Camera_RebuildProjectionMatrix();
}

void setupWaterReflectionTev(int handle1, int handle2)
{
    extern f32 lbl_803DEEE4;
    Mtx mtx_30;
    GXColor temp;
    GXColor temp2;
    GXColor k0;
    GXColor k1;
    GXColor k2;
    GXColor tev1;
    GXColor tev2;
    f32 (*indBase[1])[2][3];

    indBase[0] = lbl_8030EA10;
    selectReflectionTexture(0);
    selectTexture((Texture*)handle1, 1);
    selectTexture((Texture*)handle2, 2);

    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXLoadTexMtxImm(lbl_80396820, GX_PTTEXMTX7, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, 0, GX_FALSE, GX_PTTEXMTX7);
    PSMTXScale(mtx_30, 12.0f, lbl_803DEEE4, 0.0f);
    GXLoadTexMtxImm(mtx_30, GX_TEXMTX0, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);

    if (isHeavyFogEnabled() != 0)
    {
        ((u8*)&temp)[0] = ((u8*)&gFogColor)[0];
        ((u8*)&temp)[1] = ((u8*)&gFogColor)[1];
        ((u8*)&temp)[2] = ((u8*)&gFogColor)[2];
    }
    else
    {
        u8 ignoredLightColor;
        (*gSkyInterface)
            ->getCurrentAmbientAndLightColors(&((u8*)&temp)[0], &((u8*)&temp)[1], &((u8*)&temp)[2], &ignoredLightColor,
                                              &ignoredLightColor, &ignoredLightColor);
    }

    k0 = *(GXColor*)&lbl_803DB690;
    ((void (*)(int, GXColor*))GXSetTevKColor)(0, &k0);
    GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
    k1 = *(GXColor*)&lbl_803DB694;
    ((void (*)(int, GXColor*))GXSetTevKColor)(1, &k1);
    GXSetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K1);
    k2 = *(GXColor*)&lbl_803DB698;
    ((void (*)(int, GXColor*))GXSetTevKColor)(2, &k2);
    GXSetTevKColorSel(GX_TEVSTAGE2, GX_TEV_KCSEL_K2);

    ((u8*)&temp)[0] = (u8)((int)((u8*)&temp)[0] >> 2);
    ((u8*)&temp)[1] = (u8)((int)((u8*)&temp)[1] >> 2);
    ((u8*)&temp)[2] = (u8)((int)((u8*)&temp)[2] >> 2);
    tev1 = temp;
    ((void (*)(int, GXColor*))GXSetTevColor)(1, &tev1);

    ((u8*)&temp2)[0] = (u8)(((u8*)&temp)[0] + 0xC0);
    ((u8*)&temp2)[1] = (u8)(((u8*)&temp)[1] + 0xC0);
    ((u8*)&temp2)[2] = (u8)(((u8*)&temp)[2] + 0xC0);
    tev2 = temp2;
    ((void (*)(int, GXColor*))GXSetTevColor)(2, &tev2);

    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
    GXSetIndTexCoordScale(0, 0, 0);
    GXSetIndTexMtx(1, indBase[0][0], -1);
    GXSetIndTexMtx(2, indBase[0][1], -1);
    GXSetIndTexMtx(3, indBase[0][2], -1);
    GXSetTevIndirect(0, 0, 0, 7, 1, 0, 0, 0, 0, 0);
    GXSetTevIndirect(1, 0, 0, 7, 2, 0, 0, 0, 0, 1);
    GXSetTevIndirect(2, 0, 0, 7, 3, 0, 0, 0, 0, 0);
    GXSetNumIndStages(1);
    GXSetNumTexGens(3);
    GXSetNumTevStages(4);
    GXSetNumChans(1);

    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_C0);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_ALPHA_BUMPN);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_RASA, GX_CA_APREV, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
    GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXSetTevDirect(GX_TEVSTAGE3);
    GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE3, GX_CC_CPREV, GX_CC_C1, GX_CC_TEXA, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE3, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
    GXSetTevSwapMode(GX_TEVSTAGE3, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
        gGxZModeCompareEnable = 1;
        gGxZModeCompareFunc = 3;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
}

void setupReflectionIndirectTev(u8 flag)
{
    f32 mtx[6];

    selectReflectionTexture(1);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX2, GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    mtx[0] = 0.0f;
    mtx[1] = 0.5f;
    mtx[2] = 0.0f;
    mtx[3] = 0.0f;
    mtx[4] = 0.0f;
    mtx[5] = 0.5f;
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD0, GX_TEXMAP0);
    GXSetIndTexCoordScale(0, 0, 0);
    GXSetIndTexMtx(1, (void*)mtx, -2);
    GXSetTevIndirect(1, 0, 0, 7, 1, 0, 0, 0, 0, 1);
    GXSetNumIndStages(1);
    GXSetNumTexGens(2);
    GXSetNumTevStages(2);
    GXSetChanCtrl(GX_COLOR0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_ALPHA0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(1);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    if (flag != 0)
    {
        GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
    }
    else
    {
        GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO);
    }
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_ALPHA_BUMPN);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_RASA, GX_CA_APREV, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
}

void setupReflectionDistortTev(int texHandle)
{
    extern f32 lbl_803DEEE4;

    u8 ignoredLightColor;
    f32 sOff;
    f32 tOff;
    f32 indMtx[6];
    Mtx scaleMtx;

    selectReflectionTexture(0);
    selectTexture((Texture*)texHandle, 1);
    newshadows_getReflectionScrollOffsets(&sOff, &tOff);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX2, GX_FALSE, GX_PTIDENTITY);
    PSMTXScale(scaleMtx, 1.0f, 1.0f, 1.0f);
    GXLoadTexMtxImm(scaleMtx, GX_TEXMTX1, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);
    indMtx[0] = 0.0f;
    indMtx[1] = 0.5f;
    indMtx[2] = 0.0f;
    indMtx[3] = 0.0f;
    indMtx[4] = 0.0f;
    indMtx[5] = 0.5f;
    if (isHeavyFogEnabled())
    {
        lbl_803DB688.r = gFogColor.r;
        lbl_803DB688.g = gFogColor.g;
        lbl_803DB688.b = gFogColor.b;
        lbl_803DB688.a = 0x80;
    }
    else
    {
        (*gSkyInterface)
            ->getCurrentAmbientAndLightColors(&lbl_803DB688.r, &lbl_803DB688.g, &lbl_803DB688.b, &ignoredLightColor,
                                              &ignoredLightColor, &ignoredLightColor);
        lbl_803DB688.r = lbl_803DB688.r >> 3;
        lbl_803DB688.g = lbl_803DB688.g >> 3;
        lbl_803DB688.b = lbl_803DB688.b >> 3;
        lbl_803DB688.a = lbl_803DB678;
    }
    GXSetTevColor(GX_TEVREG2, lbl_803DB688);
    GXSetTevKColor(GX_KCOLOR0, lbl_803DB68C);
    GXSetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K0);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
    GXSetIndTexCoordScale(0, 0, 0);
    GXSetIndTexMtx(1, (f32(*)[3])indMtx, -1);
    GXSetIndTexMtx(2, (f32(*)[3])indMtx, -2);
    GXSetTevIndirect(0, 0, 0, 7, 1, 0, 0, 0, 0, 0);
    GXSetTevIndirect(1, 0, 0, 7, 2, 0, 0, 0, 0, 1);
    GXSetNumIndStages(1);
    GXSetNumChans(1);
    GXSetNumTexGens(3);
    GXSetNumTevStages(2);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C2, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    if (isHeavyFogEnabled())
    {
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_DIVIDE_2, GX_TRUE, GX_TEVPREV);
    }
    else
    {
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    }
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD2, GX_TEXMAP0, GX_ALPHA_BUMPN);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_CPREV, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_A1, GX_CA_RASA, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
        gGxZModeCompareEnable = 1;
        gGxZModeCompareFunc = 3;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
}

void setupReflectionBumpDistortTev(void* texture)
{
    extern f32 lbl_803DEEE4;

    u8 ignoredLightColor;
    f32 sOff;
    f32 tOff;
    f32 indMtx[6];
    Mtx scaleMtx;

    selectReflectionTexture(0);
    loadNewShadowBumpTexture(1);
    newshadows_getReflectionScrollOffsets(&sOff, &tOff);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX2, GX_FALSE, GX_PTIDENTITY);
    PSMTXScale(scaleMtx, 1.0f, 1.0f, 1.0f);
    GXLoadTexMtxImm(scaleMtx, GX_TEXMTX1, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);
    indMtx[0] = 0.25f;
    indMtx[1] = 0.0f;
    indMtx[2] = 0.0f;
    indMtx[3] = 0.0f;
    indMtx[4] = 0.25f;
    indMtx[5] = 0.0f;
    if (isHeavyFogEnabled())
    {
        lbl_803DB680.r = gFogColor.r;
        lbl_803DB680.g = gFogColor.g;
        lbl_803DB680.b = gFogColor.b;
        lbl_803DB680.a = 0x80;
    }
    else
    {
        (*gSkyInterface)
            ->getCurrentAmbientAndLightColors(&lbl_803DB680.r, &lbl_803DB680.g, &lbl_803DB680.b, &ignoredLightColor,
                                              &ignoredLightColor, &ignoredLightColor);
        lbl_803DB680.r = lbl_803DB680.r >> 3;
        lbl_803DB680.g = lbl_803DB680.g >> 3;
        lbl_803DB680.b = lbl_803DB680.b >> 3;
        lbl_803DB680.a = lbl_803DB678;
    }
    GXSetTevColor(GX_TEVREG2, lbl_803DB680);
    GXSetTevKColor(GX_KCOLOR0, lbl_803DB684);
    GXSetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K0);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
    GXSetIndTexCoordScale(0, 0, 0);
    GXSetIndTexMtx(1, (f32(*)[3])indMtx, -1);
    GXSetIndTexMtx(2, (f32(*)[3])indMtx, -2);
    GXSetTevIndirect(0, 0, 0, 7, 1, 0, 0, 0, 0, 0);
    GXSetTevIndirect(1, 0, 0, 7, 2, 0, 0, 0, 0, 3);
    GXSetNumIndStages(1);
    GXSetNumChans(1);
    GXSetNumTexGens(3);
    GXSetNumTevStages(2);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C2, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    if (isHeavyFogEnabled())
    {
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_DIVIDE_2, GX_TRUE, GX_TEVPREV);
    }
    else
    {
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    }
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD2, GX_TEXMAP0, GX_ALPHA_BUMPN);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_CPREV, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_A1, GX_CA_RASA, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
        gGxZModeCompareEnable = 1;
        gGxZModeCompareFunc = 3;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
}

void gxTextureSetupFn_8007cf7c(void);


/*
 * Retail ships a locally-defined empty OSReport that disables debug
 * output.
 */
void OSReport(const char* msg, ...);


int cardDeleteFn_8007d99c(void);

int _saveGame(int slot, void* save, void* data);

int maybeTryLoadSave(void* data);


int cardProbe(u8 retry);


void cardGetMessage(u32* buttons, u32* texts, u32* count);

void showMemCardError(u8 err);

/*
 * Per-frame "blocking" dialog renderer driven by the card-write retry
 * loops in _saveGame/DBC0/DC5C/DD04. Pumps 60 frames of the GX/dialog
 * pipeline; on each frame either lets the active controller draw its own
 * popup (gScreenTransitionInterface[0]->vtbl[1]) or falls back to hudDrawColored over the
 * cached prompt id in lbl_803DB708, then routes the OK/Cancel/back text
 * to gameTextFn_80016810 based on the dialog kind passed in.
 */
void cardShowLoadingMsg(u8 kind);

/*
 * Card-write callback dispatched through saveGame_prepareAndWrite from _saveGame.
 * Stages a per-slot 0x6EC-byte block plus the shared 0xE4-byte trailer
 * into the card-IO buffer (lbl_803DD044), then asks saveGame_doWrite(2) to
 * commit; if that fails it falls back to saveGame_doWrite(1).
 */
int cardCb_8007e6d4(u8 slot, int unused, void* src1, void* src2);

/*
 * Card-write callback dispatched through saveGame_prepareAndWrite from maybeTryLoadSave.
 * Copies the 0xE4-byte block at offset 0x1F14 in the card buffer (held in
 * lbl_803DD044) into the caller-supplied destination.
 */
int saveCb_8007e748(int saveId, int size, void* dst);


/* .bss block 0x80391DC0-0x803967C0 */

void gxTextureSetupFn_8007cf7c(void)
{
    extern u32 lbl_803DB67C;

    Mtx mtx_cc;
    Mtx mtx_9c;
    Mtx mtx_6c;
    f32 indMtx_54[6];
    f32 indMtx_3c[6];
    f32 indMtx_24[6];
    int handle1;
    f32 fA, fB;
    GXColor temp;

    newshadows_getReflectionScrollOffsets(&fA, &fB);
    selectReflectionTexture(0);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    getNewShadowCausticTexture((u32*)&handle1);
    selectTexture((Texture*)handle1, 1);

    PSMTXScale(mtx_cc, 1.0f, 1.0f, 1.0f);
    mtx_cc[1][3] = fA;
    GXLoadTexMtxImm(mtx_cc, GX_TEXMTX3, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX3, GX_FALSE, GX_PTIDENTITY);

    indMtx_54[0] = 0.5f;
    indMtx_54[1] = 0.0f;
    indMtx_54[2] = 0.0f;
    indMtx_54[3] = 0.0f;
    indMtx_54[4] = 0.5f;
    indMtx_54[5] = 0.0f;
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
    GXSetIndTexCoordScale(0, 0, 0);
    GXSetIndTexMtx(1, (f32(*)[3])indMtx_54, -2);
    GXSetTevIndirect(0, 0, 0, 7, 1, 6, 6, 0, 0, 0);

    PSMTXScale(mtx_9c, 0.83f, 0.83f, 0.83f);
    PSMTXRotRad(mtx_6c, 'z', 0.7853982f);
    PSMTXConcat(mtx_6c, mtx_9c, mtx_9c);
    mtx_9c[0][3] = fB;
    mtx_9c[1][3] = fB;
    GXLoadTexMtxImm(mtx_9c, GX_TEXMTX4, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX4, GX_FALSE, GX_PTIDENTITY);

    indMtx_3c[0] = 0.3f;
    indMtx_3c[1] = 0.3f;
    indMtx_3c[2] = 0.0f;
    indMtx_3c[3] = -0.3f;
    indMtx_3c[4] = 0.3f;
    indMtx_3c[5] = 0.0f;
    GXSetIndTexOrder(GX_INDTEXSTAGE1, GX_TEXCOORD2, GX_TEXMAP1);
    GXSetIndTexCoordScale(1, 0, 0);
    GXSetIndTexMtx(2, (f32(*)[3])indMtx_3c, -4);
    GXSetTevIndirect(1, 1, 0, 7, 2, 0, 0, 1, 0, 0);

    if (isHeavyFogEnabled() != 0)
    {
        ((u8*)&lbl_803DB67C)[0] = ((u8*)&gFogColor)[0];
        ((u8*)&lbl_803DB67C)[1] = ((u8*)&gFogColor)[1];
        ((u8*)&lbl_803DB67C)[2] = ((u8*)&gFogColor)[2];
        ((u8*)&lbl_803DB67C)[3] = 0x80;
    }
    else
    {
        u8 ignoredLightColor;
        u8* p1;
        u8* p2;
        (*gSkyInterface)
            ->getCurrentAmbientAndLightColors((u8*)&lbl_803DB67C, p1 = (u8*)&lbl_803DB67C + 1,
                                              p2 = (u8*)&lbl_803DB67C + 2, &ignoredLightColor, &ignoredLightColor,
                                              &ignoredLightColor);
        ((u8*)&lbl_803DB67C)[0] = (u8)(((u8*)&lbl_803DB67C)[0] >> 3);
        *p1 = (u8)(*p1 >> 3);
        *p2 = (u8)(*p2 >> 3);
        ((u8*)&lbl_803DB67C)[3] = lbl_803DB678;
    }
    temp = *(GXColor*)&lbl_803DB67C;
    GXSetTevKColor(GX_KCOLOR0, temp);
    GXSetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K0_A);
    GXSetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K0);

    GXSetNumIndStages(2);
    GXSetNumChans(1);
    GXSetNumTexGens(4);
    GXSetNumTevStages(4);

    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_KONST, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
    if (isHeavyFogEnabled() != 0)
    {
        GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_DIVIDE_2, GX_TRUE, GX_TEVREG0);
    }
    else
    {
        GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
    }
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);

    indMtx_24[0] = 0.0f;
    indMtx_24[1] = 0.5f;
    indMtx_24[2] = 0.0f;
    indMtx_24[3] = -0.5f;
    indMtx_24[4] = 0.0f;
    indMtx_24[5] = 0.0f;
    GXSetIndTexMtx(3, (f32(*)[3])indMtx_24, -5);
    GXSetTevIndirect(2, 0, 0, 7, 2, 6, 6, 0, 0, 0);
    GXSetTevIndirect(3, 1, 0, 7, 3, 0, 0, 1, 0, 0);
    GXSetTexCoordGen2(GX_TEXCOORD3, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);

    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD3, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE3, GX_CC_TEXC, GX_CC_C0, GX_CC_A0, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE3, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
    GXSetTevSwapMode(GX_TEVSTAGE3, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetCullMode(GX_CULL_NONE);
    if ((u32)gGxZModeCompareEnable != 1 || gGxZModeCompareFunc != 3 || gGxZModeUpdateEnable != 0 || gGxZModeValid == 0)
    {
        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
        gGxZModeCompareEnable = 1;
        gGxZModeCompareFunc = 3;
        gGxZModeUpdateEnable = 0;
        gGxZModeValid = 1;
    }
    if ((u32)gGxZCompLocCached != 1 || gGxZCompLocValid == 0)
    {
        GXSetZCompLoc(GX_TRUE);
        gGxZCompLocCached = 1;
        gGxZCompLocValid = 1;
    }
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
}
