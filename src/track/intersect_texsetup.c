#include "global.h"
#include "main/dll/partfx_interface.h"
#include "dolphin/card.h"
#include "main/hud_visibility_api.h"
#include "main/map_block.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/dll/waterfx_interface.h"
#include "main/audio/sfx.h"
#include "main/gamebits.h"
#include "game/objects/object.h"
#include "sys/objects.h"
#include "main/pi_flush_api.h"
#include "main/rcp_dolphin_api.h"
#include "dolphin/gx.h"
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
#include "main/newshadows_audio_api.h"
#include "dolphin/gx/GXBump.h"
#include "dolphin/gx/GXCull.h"
#include "dolphin/gx/GXGeometry.h"
#include "dolphin/gx/GXTev.h"
#include "dolphin/gx/GXTransform.h"

typedef void (*GXSetAlphaCompareIntFn)(int comp0, int ref0, int op, int comp1, int ref1);

extern int lbl_803DD03C;
extern f32 gFogNearZ;
extern f32 gFogFarZ;
extern u32 gTevStageCursor;
extern u32 gTevTexCoordCursor;
extern u32 gTevTexMapCursor;
extern f32 gFogStartZ;
extern f32 gFogEndZ;
extern GXColor gFogColor;
extern u8 gGxZModeValid;
extern u8 gGxZCompLocValid;
extern u8 gGxZModeCompareEnable;
extern int gGxZModeCompareFunc;
extern u8 gGxZModeUpdateEnable;
extern u8 gGxZCompLocCached;
extern u8 lbl_803DD010;
extern f32 lbl_803DD00C;
extern u8 gTevStageCount;
extern u8 gTevTexGenCount;
extern u8 gTevChanCount;
extern u8 gTevIndStageCount;
extern u32 screenWidth;
extern u16 gDepthReadResultCount;
extern u16 gDepthReadPendingCount;

extern u8 gWaterFxDisabled;
extern u8 gWaterQuadWriteIdx;
extern u8 gWaterRippleWriteIdx;
extern u32 lbl_803DCFF4;
extern u8 gWaterFxBank;

extern u8 lbl_803DB678;
extern u8 gHudTintAlpha;
extern u32 lbl_803DB67C;
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




extern f32 lbl_803DEE90;

extern f32 hudMatrix[4][4];
extern f32 hudScale;
extern f32 gSynthDelayedActionWord0, gSynthFadeMask;
extern f32 lbl_803DEF08, lbl_803DEF20;
extern double lbl_803DEF10, lbl_803DEF18;
extern f32 lbl_803DEE40;
extern f32 lbl_803DEE38, lbl_803DEE3C, lbl_803DEE44, lbl_803DEE48, lbl_803DEE58;
extern f32 lbl_803DEE5C, lbl_803DEE64;
extern f32 lbl_803DEE60;
extern f32 lbl_803DEED8, lbl_803DEEE8, lbl_803DEEEC, lbl_803DEEF4;
extern f32 lbl_803DEF24, lbl_803DEF28, lbl_803DEF30, lbl_803DEF34, lbl_803DEF38;
extern f32 lbl_803DEF4C, lbl_803DEF50, lbl_803DEF54, lbl_803DEF58, lbl_803DEF5C;
extern f32 lbl_803DEF60, lbl_803DEF64, lbl_803DEF68, lbl_803DEF6C, lbl_803DEF70, lbl_803DEF74;
extern f32 lbl_803DEF78, lbl_803DEF7C, lbl_803DEF80, lbl_803DEF84, lbl_803DEF88;
extern f32 lbl_803DEF90, lbl_803DEF94, lbl_803DEF98, lbl_803DEF9C;
extern GXColor lbl_803DEEB4, lbl_803E8454;

extern f32 lbl_8030EA10[3][2][3];
extern f32 lbl_8030EA58[2][3];
extern f32 lbl_8030EA70[2][3];
extern f32 lbl_8030EA88[2][3];
extern f32 lbl_8030EAA0[2][3];
extern char sMemoryCardFileNameString[20];

int cardDeleteFn_8007d99c(void);
void cardGetMessage(u32* buttons, u32* texts, u32* count);
void showMemCardError(u8 err);

typedef struct StageCountTable
{
    u8 count[7];
} StageCountTable;

extern u32 lbl_803DEEA0, lbl_803DEEA4, lbl_803DEEA8;
extern StageCountTable lbl_803DEEAC;
extern u32 lbl_803DEEB8, lbl_803DEEBC, lbl_803DEEC0, lbl_803DEEC4;
extern u32 lbl_803DEEC8, lbl_803DEECC, lbl_803DEED0, lbl_803DEED4, lbl_803E8450;
extern volatile s32 gSaveCardState;
extern u8 lbl_803DD059;
extern u32 gSaveCardSerialHi;
extern u32 gSaveCardSerialLo;
extern u32 gSaveCardChecksumHi;
extern u32 gSaveCardChecksumLo;
extern f32 lbl_803DEE20;
extern f32 lbl_803DEE24;
typedef struct RippleEntry
{
    f32 x, y, z;
    u16 id;
    u8 alpha;
    u8 flip;
} RippleEntry;
typedef struct SplashQuad
{
    f32 v[12];
    u16 angle;
    u8 type;
    u8 alpha;
    u8 flip;
    u8 pad[3];
} SplashQuad;

STATIC_ASSERT(sizeof(RippleEntry) == 0x10);
STATIC_ASSERT(sizeof(SplashQuad) == 0x38);

extern f32 gWaterFxState[4];
extern Texture* gWaterFxTextures[4];
extern RippleEntry gWaterRipples[0x100];
extern SplashQuad gWaterSplashQuads[0x100];

typedef struct
{
    f32 scales[4];
    Texture* textures[4];
    RippleEntry ripples[0x100];
    SplashQuad quads[0x100];
} WaterFxState;
STATIC_ASSERT(offsetof(WaterFxState, textures) == 0x10);
STATIC_ASSERT(offsetof(WaterFxState, ripples) == 0x20);
STATIC_ASSERT(offsetof(WaterFxState, quads) == 0x1020);
STATIC_ASSERT(sizeof(WaterFxState) == 0x4820);
extern f32 gGxPi;
extern f32 lbl_803DEE6C;
extern f32 lbl_803DEE70;
extern f32 lbl_803DEE74;
extern f32 lbl_803DEE78;
extern f32 lbl_803DEE7C;
extern f32 lbl_803DEE80;
extern int lbl_803DD03C;
extern int lbl_803968C0[];
extern f32 lbl_803DEE98;
extern f32 lbl_803DEE9C;
extern u8 gSaveCardRetry;
void playerEarthWalkerAudioFn_8006f950(u8* obj, f32* pos, u8 flip, u8 type);
void mtx44Identity(f32* mat);
void gxSetPeControl_ZCompLoc_(u8 zCompLoc);
void gxSetZMode_(u8 compareEnable, int compareFunc, u8 updateEnable);
void drawViewFinderAperture(f32 sx, f32 sy, u8 a, u8 flag);
int cardProbe(u8 retry);
void showMemCardError(u8 err);
void cardShowLoadingMsg(u8 kind);
int cardCb_8007e6d4(u8 slot, int unused, void* src1, void* src2);
int saveCb_8007e748(int saveId, int size, void* dst);

/* Per-frame alpha decrement of the two water-effect pools. */
void timeFn_8006f400(f32 step);

void drawFn_8006f500(void);

void playerEarthWalkerAudioFn_8006f950(u8* obj, f32* pos, u8 flip, u8 type);

void waterFxSetDisabled(int disabled);

void waterFxInit(void);

/* Queues a GXPeekZ read at (x,y) tagged by an opaque requestKey (callers pass
 * any unique value - object ptrs, loop indices, even a function address) and
 * returns the previously completed result for that key, 0 until ready. */
int depthReadRequestPoll(int x, int y, void* requestKey);

u32 getScreenResolution(void);

void setScreenWidth(u32 width);

void clearScreenWidth(void);

void matrixFn_8006ff0c(float* mat, short* out, f32 fov, f32 aspect, f32 near, f32 far, f32 scale);

void normalize(f32* x, f32* y, f32* z);

/* 4x4 identity fill. */
void mtx44Identity(f32* mat);



void gxSetPeControl_ZCompLoc_(u8 zCompLoc);


void gxSetZMode_(u8 compareEnable, int compareFunc, u8 updateEnable);

void resetSomeGxFlags(void);

void setHudOpacity(u8 opacity);

void _gxSetFogParams(void);

void fogFn_80070404(f32 a, f32 b);

void getColor803dd01c(u8* rgbOut);

void setFogColorRgb(u8 red, u8 green, u8 blue);

int renderWhirlpool(void* obj_a, void** obj_b, int slot);

void screenImageDraw(u8 alpha);

void doSpiritVisionFilter(void);

void doColorFilter(u8* mod);

void doDistortionFilter(f32* pos, f32 radius, u8* mod, f32 angle);

int gxTextureFn_80072dfc(void* obj_a, void** obj_b, int slot);

/*
 * Three-tex-coord-gen ind+direct TEV setup. Loads the active env-mtx
 * (lbl_80396820) for tex0, scales tex1 by hudScale through a 3x4
 * matrix from PSMTXScale, and stamps an indirect tex matrix from local
 * stack data. Two TEV stages: stage 0 K-modulates the texture by alpha,
 * stage 1 modulates by the second texture. Uses ind tex stage 0 to warp
 * tex coord 0 by tex1.
 */
void quakeSpellTextureFn_8007366c(u8 alpha);

void setupAdditiveTintedTexture(void* texture, u32* colorA, u32* colorB);

int modelCb_80073d04(u8* obj, int* objB);

int moonFxCb_80074110(u8* obj, int* objB, int slot);

int modelCb_80074518(void* obj_a, void** obj_b, int slot);

u32 objCallback_80074d04(int handle, void* model);

void hudDrawRect(int x1, int y1, int x2, int y2, GXColor color);

void drawViewFinderLine(f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3, f32 x4, f32 y4, GXColor* color);

void hudDrawTriangle(f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3, GXColor color);

void skyDrawFn_80075d5c(int x1, int y1, int x2, int y2, f32 u1, f32 v1, f32 u2, f32 v2, int z);

void textRenderChar(int x1, int y1, int x2, int y2, f32 u1, f32 v1, f32 u2, f32 v2);

void drawPartialTexture(void* obj, f32 sx, f32 sy, int alpha_mod, int scale, int width, int height, int u_offset,
                        int v_offset);

/*
 * Generic ortho-projected single-color quad blit. Sets the GX state up
 * fresh (no tex coords, color from constant K0, additive blend, fixed
 * 0x3C texmtx) then emits four GX_VTXFMT1 vertices at z=-0x18C with
 * width 4*size_x and height 4*size_y in screen pixels. Used as the
 * "draw fullscreen tint" primitive by the dialog code in cardShowLoadingMsg.
 */
void drawRect(f32 sx, f32 sy, int x, int y);

void drawScaledTexture(void* obj, f32 sx, f32 sy, int alpha_mod, int scale, int width, int height, int flags);

/*
 * Caller-coloured asset blit. Same mechanic as drawTexture but the K0
 * color comes from a writable GXColor the caller passes in (we apply the
 * gHudTintAlpha alpha tint to it in place). The flag arg picks between
 * "raster passthrough" (TevColorIn 0xF/0xF/0xF/0xE) and "K-tint replace"
 * (TevColorIn 0xF/0xE/0x8/0xF).
 */
void hudDrawColored(int obj, int x, int y, u32* color, int scale, int flag);

/*
 * Quad-from-asset blit: takes an "asset record" (with width at +0xA,
 * height at +0xC, and an optional second-stage flag at +0x50), a per-
 * call alpha multiplier, screen-pos (sx, sy), and a u16 size scale.
 * Composes K0 from RGB(255,255,255) plus the global alpha tint
 * (alpha * gHudTintAlpha >> 8); if the asset opts in, layers a second
 * tex stage that further K-multiplies by the texture. Final width and
 * height are 4 * asset_dim * scale >> 8 in screen pixels at z=-8.
 */
void drawTexture(void* obj, f32 sx, f32 sy, int alpha_mod, int scale);

void objectShadow_setupSwappedProjectedTexture(ProjectedShadowTexture* shadow, u32* colorPtr, Mtx mtx);

void objectShadow_setupProjectedTexture(ProjectedShadowTexture* shadow, u32* colorPtr, Mtx mtx);

void objectShadow_setupProjectedTextureDepthFade(ProjectedShadowTexture* shadow, u32* colorPtr, Mtx mtx, f32 depth);

void objectShadow_setupProjectedTextureChannel(ProjectedShadowTexture* shadow, u32* colorPtr, Mtx mtx, f32 scale);

void gxSetOpaqueZWriteMode(void);

void gxSetOpaqueNoZWriteMode(void);

void gxBlendFn_800788dc(void);

void gxBlendFn_800789ac(void);

void textBlendSetupFn_80078a7c(void);

void gxBlendFn_80078b4c(void);

void gxDebugTextureFn_80078c1c(void);

void gxTevModulateRasStage(void);

void gxTevRasTimesColor1Stage(void);

void textRenderSetup(void);

void gxTevAddColor1Stage(void);

void gxTevPassRasStage(void);

void gxTexColorFn_80079254(void);

void gxTevAddTextureFrameBlendStages(void);

void gxTextureFn_800794e0(void);

void textRenderSetupFn_800795e8(void);

void geomDrawFn_800796f0(void);

/*
 * Closes out the TEV pipeline configuration that drawViewFinderAperture etc. open:
 * pushes the current ind-stage / chan-ctrl / tex-gen counts in
 * gTevIndStageCount..00B back into GX, and if the global tint alpha
 * gHudTintAlpha isn't fully transparent (0xFF) appends one final TEV
 * stage that K-multiplies the tint over the existing color, advancing
 * gTevStageCursor (TEV stage cursor) and gTevStageCount (stage count).
 */
void textRenderSetupFn_80079804(void);

void textureSetupFn_800799c0(void);

void _gxSetTevColor2(u8 r, u8 g, u8 b, u8 a);

void _gxSetTevColor1(u8 r, u8 g, u8 b, u8 a);

/*
 * Fullscreen 640x480 texture-tinted quad with shape-controlled alpha:
 * `flag != 0` lights the screen with three pre-set GXColors stamped into
 * K0/T1/T2; `flag == 0` instead does a single K0 modulate where K0's
 * alpha is the caller's byte divided by 4. Builds a per-call 3x4 tex
 * coord matrix that scales the source texture by 1/sx and 1/sy with a
 * sub-pixel offset baked from lbl_803DEF4C/50.
 */
void drawViewFinderAperture(f32 sx, f32 sy, u8 a, u8 flag);

void drawFn_80079e64(f32 s1, u8 mtxIdx, void* vec, f32 s2, u8 alpha0, u8 alpha1, f32 s3);

void doHeatEffect(u8 alpha);

/*
 * Fullscreen 640x480 textured quad with caller-supplied alpha. The alpha
 * is multiplied by lbl_803DEF20 (a 0..255 scale), converted to int and
 * stamped into byte 3 of the K0 GXColor cache (lbl_803DB6A0). Sets up
 * one TEV stage that K-multiplies the texture by alpha; uses fixed UVs
 * 0..0x80 so the texture maps once across the screen. Used when fading
 * the screen to texture (e.g. boot logo / "now loading").
 */
void renderMotionBlur(f32 alpha);

void doBlurFilter(f32 wx, f32 wy, f32 wz, u8 param4, u8 param5);

void setupWaterReflectionTev(int handle1, int handle2);

void setupReflectionIndirectTev(u8 flag);

void setupReflectionDistortTev(int texHandle);

void setupReflectionBumpDistortTev(void* texture);

void gxTextureSetupFn_8007cf7c(void)
{
    extern f32 lbl_803DEEDC;

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

    indMtx_54[0] = gSynthDelayedActionWord0;
    indMtx_54[1] = lbl_803DEEDC;
    indMtx_54[2] = lbl_803DEEDC;
    indMtx_54[3] = lbl_803DEEDC;
    indMtx_54[4] = gSynthDelayedActionWord0;
    indMtx_54[5] = lbl_803DEEDC;
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

    indMtx_3c[0] = lbl_803DEF84;
    indMtx_3c[1] = lbl_803DEF84;
    indMtx_3c[2] = lbl_803DEEDC;
    indMtx_3c[3] = lbl_803DEF88;
    indMtx_3c[4] = lbl_803DEF84;
    indMtx_3c[5] = lbl_803DEEDC;
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

    indMtx_24[0] = lbl_803DEEDC;
    indMtx_24[1] = gSynthDelayedActionWord0;
    indMtx_24[2] = lbl_803DEEDC;
    indMtx_24[3] = lbl_803DEEF4;
    indMtx_24[4] = lbl_803DEEDC;
    indMtx_24[5] = lbl_803DEEDC;
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

void loadReflectionTexMtxs(void);

/*
 * Retail ships a locally-defined empty OSReport that disables debug
 * output.
 */
void OSReport(const char* msg, ...);

/*
 * Card init / serial-no validation. Mounts slot 0; if the mount comes back
 * "no card filesystem" (-13) it remembers we need to format. On a check
 * error (-6) it runs CARDCheck; if that also returns -6 it formats. On a
 * clean mount (or after the recovery path) it reads the card serial and
 * compares against the cached pair (gSaveCardSerialHi/Lo). If the cached pair
 * is zero, or doesn't match the live card, the cache is rejected with a
 * "wrong card" error code (-0x55, gSaveCardState = 11). Otherwise CARDFormat
 * if we still owe one, else success: clear the cache, set state 13,
 * unmount, return 1.
 */
int cardLoadFn_8007d72c(void);

void saveFn_8007d960(u32 enable);

void cardSetStatusNeedInit(void);

s32 saveGameGetStatus(void);

int cardDeleteFn_8007d99c(void);

int _saveGame(int slot, void* save, void* data);

int maybeTryLoadSave(void* data);

int loadSaveGame(int slot, void* save);

int memCardFn_8007dd04(u8 retry);
int cardProbe(u8 retry);

void _initCardAndDsp(void);

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
extern f32 gWaterFxState[4];
extern Texture* gWaterFxTextures[4];
extern RippleEntry gWaterRipples[0x100];
extern SplashQuad gWaterSplashQuads[0x100];
extern DepthReadRequest gDepthReadResults[0x14];
extern DepthReadRequest gDepthReadPendingQueue[0x14];

extern f32 lbl_803967C0[3][4];
extern f32 lbl_803967F0[3][4];
extern f32 lbl_80396820[3][4];
extern f32 lbl_80396850[3][4];
extern f32 hudMatrix[4][4];
extern int lbl_803968C0[0x10];
extern SaveCardFileInfo lbl_80396900;
