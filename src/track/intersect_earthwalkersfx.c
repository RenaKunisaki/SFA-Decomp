#include "global.h"
#include "dolphin/card.h"
#include "dolphin/mtx.h"
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
#include "main/maketex_api.h"
#include "main/pad.h"
#include "main/pi_dolphin.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/shader_api.h"
#include "dolphin/mtx/vec.h"

typedef void (*GXSetAlphaCompareIntFn)(int comp0, int ref0, int op, int comp1, int ref1);


extern u32 lbl_803DCFF4;




extern f32 lbl_803DEE38, lbl_803DEE3C, lbl_803DEE58;
extern f32 lbl_803DEE5C, lbl_803DEE64;
extern f32 lbl_803DEE60;
extern f32 lbl_803DEF4C;


int cardDeleteFn_8007d99c(void);
void cardGetMessage(u32* buttons, u32* texts, u32* count);
void showMemCardError(u8 err);


extern f32 lbl_803DEE20;
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


void playerEarthWalkerAudioFn_8006f950(u8* obj, f32* pos, u8 flip, u8 type)
{
    WaterFxState* base;
    f32 x, y, z;
    f32 ax, px;
    f32 ay, py, az, pz;
    f32 xm, ym, zm;
    f32 groundY;
    Vec axis;
    Vec perp;
    Vec norm;
    f32 fscale;

    base = (WaterFxState*)gWaterFxState;
    if (((GameObject*)obj)->anim.classId == 1)
    {
        gWaterFxBank = *(u8*)&((GameObject*)obj)->anim.bankIndex;
    }
    else if (((GameObject*)obj)->anim.seqId == 0x416)
    {
        gWaterFxBank = 3;
    }
    if (trackGetNearestGroundOffsetAndNormal((GameObject*)obj, ((GameObject*)obj)->anim.localPosX,
                                             ((GameObject*)obj)->anim.localPosY,
                                             ((GameObject*)obj)->anim.localPosZ, &groundY, (f32*)&norm, 0) == 0)
    {
        if (type == 1)
        {
            base->ripples[gWaterRippleWriteIdx].x = pos[0];
            base->ripples[gWaterRippleWriteIdx].y = lbl_803DEE3C + pos[1];
            base->ripples[gWaterRippleWriteIdx].z = pos[2];
            base->ripples[gWaterRippleWriteIdx].id = *(s16*)obj;
            base->ripples[gWaterRippleWriteIdx].alpha = 0xff;
            base->ripples[gWaterRippleWriteIdx].flip = flip;
            gWaterRippleWriteIdx++;
            if (gWaterRippleWriteIdx >= 0x100)
            {
                gWaterRippleWriteIdx = 0;
            }
        }
        PSVECNormalize(&norm, &norm);
        axis.x = lbl_803DEE38;
        axis.y = lbl_803DEE20;
        axis.z = lbl_803DEE20;
        if (__fabs(PSVECDotProduct(&norm, &axis)) >= lbl_803DEE58)
        {
            axis.x = lbl_803DEE20;
            axis.z = lbl_803DEE38;
        }
        PSVECCrossProduct(&norm, &axis, &perp);
        PSVECCrossProduct(&perp, &norm, &axis);
        PSVECNormalize(&axis, &axis);
        PSVECNormalize(&perp, &perp);
        fscale = base->scales[gWaterFxBank];
        PSVECScale(&axis, &axis, fscale);
        PSVECScale(&perp, &perp, fscale);
        x = pos[0];
        y = pos[1];
        z = pos[2];
        ax = axis.x;
        xm = x - ax;
        px = perp.x;
        base->quads[gWaterQuadWriteIdx].v[0] = xm - px;
        ay = axis.y;
        ym = y - ay;
        py = perp.y;
        base->quads[gWaterQuadWriteIdx].v[1] = ym - py;
        az = axis.z;
        zm = z - az;
        pz = perp.z;
        base->quads[gWaterQuadWriteIdx].v[2] = zm - pz;
        x += ax;
        base->quads[gWaterQuadWriteIdx].v[3] = x - px;
        y += ay;
        base->quads[gWaterQuadWriteIdx].v[4] = y - py;
        z += az;
        base->quads[gWaterQuadWriteIdx].v[5] = z - pz;
        base->quads[gWaterQuadWriteIdx].v[6] = px + x;
        base->quads[gWaterQuadWriteIdx].v[7] = py + y;
        base->quads[gWaterQuadWriteIdx].v[8] = pz + z;
        base->quads[gWaterQuadWriteIdx].v[9] = px + xm;
        base->quads[gWaterQuadWriteIdx].v[10] = py + ym;
        base->quads[gWaterQuadWriteIdx].v[11] = pz + zm;
        base->quads[gWaterQuadWriteIdx].angle = 0x10000 - *(s16*)obj;
        base->quads[gWaterQuadWriteIdx].type = type;
        base->quads[gWaterQuadWriteIdx].alpha = 0xff;
        base->quads[gWaterQuadWriteIdx].flip = flip;
        gWaterQuadWriteIdx++;
        if (gWaterQuadWriteIdx >= 0x100)
        {
            gWaterQuadWriteIdx = 0;
        }
    }
}

void waterFxSetDisabled(int disabled)
{
    int i;
    SplashQuad* quads;
    RippleEntry* ripples;

    gWaterFxDisabled = disabled;
    if (disabled != 0)
    {
        return;
    }
    quads = gWaterSplashQuads;
    ripples = gWaterRipples;
    for (i = 0; i < 32; i++)
    {
        quads[i * 8].alpha = 0;
        ripples[i * 8].alpha = 0;
        quads[i * 8 + 1].alpha = 0;
        ripples[i * 8 + 1].alpha = 0;
        quads[i * 8 + 2].alpha = 0;
        ripples[i * 8 + 2].alpha = 0;
        quads[i * 8 + 3].alpha = 0;
        ripples[i * 8 + 3].alpha = 0;
        quads[i * 8 + 4].alpha = 0;
        ripples[i * 8 + 4].alpha = 0;
        quads[i * 8 + 5].alpha = 0;
        ripples[i * 8 + 5].alpha = 0;
        quads[i * 8 + 6].alpha = 0;
        ripples[i * 8 + 6].alpha = 0;
        quads[i * 8 + 7].alpha = 0;
        ripples[i * 8 + 7].alpha = 0;
    }
    gWaterQuadWriteIdx = 0;
    gWaterRippleWriteIdx = 0;
}

void waterFxInit(void)
{
    int i;
    WaterFxState* waterFx = (WaterFxState*)gWaterFxState;
    SplashQuad* quads = waterFx->quads;
    RippleEntry* ripples = waterFx->ripples;

    for (i = 0; i < 16; i++)
    {
        quads[0].alpha = 0;
        ripples[0].alpha = 0;
        quads[1].alpha = 0;
        ripples[1].alpha = 0;
        quads[2].alpha = 0;
        ripples[2].alpha = 0;
        quads[3].alpha = 0;
        ripples[3].alpha = 0;
        quads[4].alpha = 0;
        ripples[4].alpha = 0;
        quads[5].alpha = 0;
        ripples[5].alpha = 0;
        quads[6].alpha = 0;
        ripples[6].alpha = 0;
        quads[7].alpha = 0;
        ripples[7].alpha = 0;
        quads[8].alpha = 0;
        ripples[8].alpha = 0;
        quads[9].alpha = 0;
        ripples[9].alpha = 0;
        quads[10].alpha = 0;
        ripples[10].alpha = 0;
        quads[11].alpha = 0;
        ripples[11].alpha = 0;
        quads[12].alpha = 0;
        ripples[12].alpha = 0;
        quads[13].alpha = 0;
        ripples[13].alpha = 0;
        quads[14].alpha = 0;
        ripples[14].alpha = 0;
        quads[15].alpha = 0;
        ripples[15].alpha = 0;
        quads += 16;
        ripples += 16;
    }
    waterFx->textures[0] = textureLoadAsset(0x19);
    waterFx->textures[1] = textureLoadAsset(0x18);
    waterFx->textures[2] = textureLoadAsset(0x1A);
    waterFx->textures[3] = textureLoadAsset(0x646);
    waterFx->scales[0] = lbl_803DEE5C;
    waterFx->scales[1] = lbl_803DEE60;
    waterFx->scales[2] = lbl_803DEE60;
    waterFx->scales[3] = lbl_803DEE64;
    gWaterFxDisabled = 0;
    gWaterQuadWriteIdx = 0;
    gWaterRippleWriteIdx = 0;
    lbl_803DCFF4 = 0;
}


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

