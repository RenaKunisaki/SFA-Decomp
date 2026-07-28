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
#include "dolphin/gx/GXLighting.h"
#include "dolphin/gx/GXTev.h"
#include "dolphin/gx/GXTransform.h"

typedef void (*GXSetAlphaCompareIntFn)(int comp0, int ref0, int op, int comp1, int ref1);

int lbl_803DD03C;
f32 gFogNearZ;
f32 gFogFarZ;
u32 gTevStageCursor;
u32 gTevTexCoordCursor;
u32 gTevTexMapCursor;
f32 gFogStartZ;
f32 gFogEndZ;
GXColor gFogColor;
u8 gGxZModeValid;
u8 gGxZCompLocValid;
u8 gGxZModeCompareEnable;
int gGxZModeCompareFunc;
u8 gGxZModeUpdateEnable;
u8 gGxZCompLocCached;
u8 lbl_803DD010;
f32 lbl_803DD00C;
u8 gTevStageCount;
u8 gTevTexGenCount;
u8 gTevChanCount;
u8 gTevIndStageCount;
u32 screenWidth;
u16 gDepthReadResultCount;
u16 gDepthReadPendingCount;
static u32 sIntersectUnused0;
u8 gWaterFxDisabled;
u8 gWaterQuadWriteIdx;
u8 gWaterRippleWriteIdx;
u32 lbl_803DCFF4;
u8 gWaterFxBank;

u8 lbl_803DB678 = 0x60;
u8 gHudTintAlpha = 0xFF;
u32 lbl_803DB67C = 0xFFFFFF60;
GXColor lbl_803DB680 = {0xFF, 0xFF, 0xFF, 0x60};
GXColor lbl_803DB684 = {0xA0, 0xA0, 0xA0, 0x80};
GXColor lbl_803DB688 = {0xFF, 0xFF, 0xFF, 0x60};
GXColor lbl_803DB68C = {0xA0, 0xA0, 0xA0, 0x80};
u32 lbl_803DB690 = 0xC0000000;
u32 lbl_803DB694 = 0x00C00000;
u32 lbl_803DB698 = 0x0000C000;
u32 lbl_803DB69C = 0x666666FF;
GXColor lbl_803DB6A0 = {0, 0, 0, 0x7F};
GXColor lbl_803DB6A4 = {0xFF, 0xFF, 0xFF, 0xFC};
u32 lbl_803DB6A8 = 0xFFFFFFFF;
f32 lbl_803DB6AC = 0.55f;
f32 lbl_803DB6B0 = 2.0f;
f32 lbl_803DB6B4 = 0.25f;
f32 lbl_803DB6B8 = 0.4f;
GXColor lbl_803DB6BC = {0x80, 0x80, 0x80, 0xFF};
f32 lbl_803DB6C0 = 2.0f;
f32 lbl_803DB6C4 = 0.3878f;
f32 lbl_803DB6C8 = 15.0f;
f32 lbl_803DB6CC = 100.0f;
GXColor lbl_803DB6D0 = {0x42, 0x42, 0x42, 0};
GXColor lbl_803DB6D4 = {0x81, 0x81, 0x81, 0};
GXColor lbl_803DB6D8 = {0x19, 0x19, 0x19, 0};
GXColor lbl_803DB6DC = {0x10, 0x10, 0x10, 0xFF};
GXColor lbl_803DB6E0 = {0, 0, 0, 0xFF};
GXColor lbl_803DB6E4 = {0, 0, 0x80, 0};
GXColor lbl_803DB6E8 = {0x80, 0x80, 0, 0};
GXColor lbl_803DB6EC = {0, 0x80, 0, 0};
GXColor lbl_803DB6F0 = {0x80, 0, 0x80, 0};
u32 lbl_803DB6F4 = 0xFFFFFF60;
u32 lbl_803DB6F8 = 0xA0A0A080;

typedef struct
{
    f32 m[6];
} IndMtxInit;

typedef struct
{
    IndMtxInit ind;
    u32 blk[6][7];
} IndStageInitData;


extern f32 lbl_803DEF20;
extern f32 lbl_803DEE40;
extern f32 lbl_803DEE38, lbl_803DEE3C, lbl_803DEE44, lbl_803DEE48;
extern f32 lbl_803DEF4C;

f32 lbl_8030EA10[3][2][3] = {
    {{0.0f, 0.5f, 0.0f}, {0.0f, 0.0f, -0.5f}},
    {{0.0f, 0.8f, 0.0f}, {0.0f, 0.0f, 0.8f}},
    {{0.0f, -0.2f, 0.0f}, {0.0f, 0.0f, 0.2f}}};
f32 lbl_8030EA58[2][3] = {{0.5f, 0.0f, 0.0f}, {0.0f, 0.5f, 0.0f}};
f32 lbl_8030EA70[2][3] = {{0.5f, 0.0f, 0.0f}, {0.0f, 0.5f, 0.0f}};
f32 lbl_8030EA88[2][3] = {{0.5f, 0.0f, 0.0f}, {0.0f, 0.5f, 0.0f}};
f32 lbl_8030EAA0[2][3] = {{0.5f, 0.0f, 0.0f}, {0.0f, 0.5f, 0.0f}};
typedef struct StageCountTable
{
    u8 count[7];
} StageCountTable;

extern f32 lbl_803DEE20;
extern f32 lbl_803DEE24;
#include "track/intersect_internal.h"
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
void timeFn_8006f400(f32 step)
{
    int i;
    SplashQuad* quads;
    RippleEntry* ripples;

    quads = gWaterSplashQuads;
    ripples = gWaterRipples;

    for (i = 0; i < 256; i++)
    {
        if (quads[i].alpha != 0)
        {
            if (quads[i].alpha - step <= lbl_803DEE20)
                quads[i].alpha = 0;
            else
                quads[i].alpha -= step;
        }
        if (ripples[i].alpha != 0)
        {
            if (ripples[i].alpha - step <= lbl_803DEE20)
                ripples[i].alpha = 0;
            else
                ripples[i].alpha -= step;
        }
    }
}

void drawFn_8006f500(void)
{
    GXColor color;
    Mtx camTrans;
    Mtx posMtx;
    Mtx rot;
    Mtx trans;
    SplashQuad* quad;
    f32* view;
    int i;
    f32 tTop;
    f32 tBot;
    u8 alpha;

    if (Obj_GetPlayerObject() == NULL)
    {
        return;
    }
    Camera_ApplyDecalViewport();
    GXSetCurrentMtx(GX_PNMTX0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetNumTevStages(1);
    GXSetNumIndStages(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetCullMode(GX_CULL_NONE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    selectTexture(gWaterFxTextures[gWaterFxBank], 0);
    view = Camera_GetViewMatrix();
    PSMTXTrans(camTrans, -playerMapOffsetX, lbl_803DEE20, -playerMapOffsetZ);
    PSMTXConcat((MtxP)view, camTrans, posMtx);
    GXLoadPosMtxImm(posMtx, GX_PNMTX0);
    gxSetZMode_(1, 3, 0);
    gxSetPeControl_ZCompLoc_(1);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    i = 0;
    for (; i < 0x100; i++)
    {
        quad = &gWaterSplashQuads[i];
        alpha = quad->alpha;
        if (alpha != 0)
        {
            if (quad->type == 1)
            {
                color.a = alpha >> 2;
            }
            else
            {
                color.a = alpha >> 1;
            }
            GXSetTevKColor(GX_KCOLOR0, color);
            if (quad->flip != 0)
            {
                tTop = lbl_803DEE38;
                tBot = lbl_803DEE20;
                PSMTXRotRad(rot, 0x7a,
                            lbl_803DEE3C * (lbl_803DEE40 * (f32)(int)(0x8000 - quad->angle)) / lbl_803DEE44);
            }
            else
            {
                tTop = lbl_803DEE20;
                tBot = lbl_803DEE38;
                PSMTXRotRad(rot, 0x7a, lbl_803DEE3C * (lbl_803DEE40 * (f32)(u32)quad->angle) / lbl_803DEE44);
            }
            PSMTXTrans(trans, lbl_803DEE48, *(f32*)&lbl_803DEE48, lbl_803DEE20);
            PSMTXConcat(rot, trans, rot);
            PSMTXTrans(trans, lbl_803DEE24, *(f32*)&lbl_803DEE24, lbl_803DEE20);
            PSMTXConcat(trans, rot, rot);
            GXLoadTexMtxImm(rot, GX_TEXMTX0, GX_MTX2x4);
            GXBegin(GX_QUADS, GX_VTXFMT2, 4);
            {
                f32 px, py, pz;
                pz = quad->v[2];
                py = quad->v[1];
                px = quad->v[0];
                GXWGFifo.f32 = px;
                GXWGFifo.f32 = py;
                GXWGFifo.f32 = pz;
            }
            GXWGFifo.f32 = lbl_803DEE20;
            GXWGFifo.f32 = tTop;
            {
                f32 px, py, pz;
                pz = quad->v[5];
                py = quad->v[4];
                px = quad->v[3];
                GXWGFifo.f32 = px;
                GXWGFifo.f32 = py;
                GXWGFifo.f32 = pz;
            }
            GXWGFifo.f32 = lbl_803DEE38;
            GXWGFifo.f32 = tTop;
            {
                f32 px, py, pz;
                pz = quad->v[8];
                py = quad->v[7];
                px = quad->v[6];
                GXWGFifo.f32 = px;
                GXWGFifo.f32 = py;
                GXWGFifo.f32 = pz;
            }
            GXWGFifo.f32 = lbl_803DEE38;
            GXWGFifo.f32 = tBot;
            {
                f32 px, py, pz;
                pz = quad->v[11];
                py = quad->v[10];
                px = quad->v[9];
                GXWGFifo.f32 = px;
                GXWGFifo.f32 = py;
                GXWGFifo.f32 = pz;
            }
            GXWGFifo.f32 = lbl_803DEE20;
            GXWGFifo.f32 = tBot;
        }
    }
    Camera_ApplyFullViewport();
}

void playerEarthWalkerAudioFn_8006f950(u8* obj, f32* pos, u8 flip, u8 type);


/* 4x4 identity fill. */
void mtx44Identity(f32* mat);


void gxSetPeControl_ZCompLoc_(u8 zCompLoc);


void gxSetZMode_(u8 compareEnable, int compareFunc, u8 updateEnable);


void drawPartialTexture(void* obj, f32 sx, f32 sy, int alpha_mod, int scale, int width, int height, int u_offset,
                        int v_offset);


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


/*
 * Fullscreen 640x480 texture-tinted quad with shape-controlled alpha:
 * `flag != 0` lights the screen with three pre-set GXColors stamped into
 * K0/T1/T2; `flag == 0` instead does a single K0 modulate where K0's
 * alpha is the caller's byte divided by 4. Builds a per-call 3x4 tex
 * coord matrix that scales the source texture by 1/sx and 1/sy with a
 * sub-pixel offset baked from lbl_803DEF4C/50.
 */
void drawViewFinderAperture(f32 sx, f32 sy, u8 a, u8 flag);


/*
 * Retail ships a locally-defined empty OSReport that disables debug
 * output.
 */
void OSReport(const char* msg, ...);


int _saveGame(int slot, void* save, void* data);

int maybeTryLoadSave(void* data);


int cardProbe(u8 retry);


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
f32 gWaterFxState[4];
Texture* gWaterFxTextures[4];
RippleEntry gWaterRipples[0x100];
SplashQuad gWaterSplashQuads[0x100];
DepthReadRequest gDepthReadResults[0x14];
DepthReadRequest gDepthReadPendingQueue[0x14];

f32 lbl_803967C0[3][4];
f32 lbl_803967F0[3][4];
f32 lbl_80396820[3][4];
f32 lbl_80396850[3][4];
f32 hudMatrix[4][4];
int lbl_803968C0[0x10];
SaveCardFileInfo gSaveCardFileInfo;
