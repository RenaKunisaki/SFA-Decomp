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
#include "dolphin/gx/GXTransform.h"
#include "string.h"

typedef void (*GXSetAlphaCompareIntFn)(int comp0, int ref0, int op, int comp1, int ref1);



extern GXColor lbl_803DB6A0;

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
extern f32 lbl_803DEF4C;


char sMemoryCardFileNameString[20] = "Star Fox Adventures";

int cardDeleteFn_8007d99c(void);
void cardGetMessage(u32* buttons, u32* texts, u32* count);
void showMemCardError(u8 err);

typedef struct StageCountTable
{
    u8 count[7];
} StageCountTable;

extern volatile s32 gSaveCardState;

u8* gSaveCardImageBuffer;
u8 lbl_803DD05A;
u8 lbl_803DD059;
u8 gSaveCardRetry;
u32 gSaveCardChecksumLo;
u32 gSaveCardChecksumHi;
u32 gSaveCardSerialLo;
u32 gSaveCardSerialHi;
char* lbl_803DD044;
void* lbl_803DD040;
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
 * (lbl_80396820) for tex0, scales tex1 by lbl_803DEF2C through a 3x4
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

void gxTextureSetupFn_8007cf7c(void);

void loadReflectionTexMtxs(void)
{
    f32* base = (f32*)&lbl_803967C0;
    Mtx tmp;
    PSMTXConcat((void*)(base + 36), (void*)(int)base, tmp);
    GXLoadTexMtxImm(tmp, GX_TEXMTX0, GX_MTX3x4);
    PSMTXConcat((void*)(base + 24), (void*)(int)base, tmp);
    GXLoadTexMtxImm(tmp, GX_TEXMTX2, GX_MTX3x4);
}

/*
 * Retail ships a locally-defined empty OSReport that disables debug
 * output.
 */
void OSReport(const char* msg, ...)
{
}

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
int cardLoadFn_8007d72c(void)
{
    int need_format;
    int res;
    u64 serial;
    int ok;

    need_format = 0;
    if (cardProbe(0) == 0)
    {
        ok = 0;
    }
    else
    {
        lbl_803DD040 = mmAlloc(0xA000, -1, 0);
        if (lbl_803DD040 == 0)
        {
            gSaveCardState = 8;
            ok = 0;
        }
        else
        {
            ok = 1;
        }
    }
    if (ok == 0)
    {
        return 0;
    }
    gSaveCardState = 0;
    res = CARDMount(0, lbl_803DD040, (void*)cardSetStatusNoCard2);
    if (res == -13)
    {
        need_format = 1;
    }
    if (res == -6)
    {
        res = CARDCheck(0);
        if (res == -6)
        {
            res = CARDFormat(0);
        }
    }
    else if (res == -13 || res == 0)
    {
        res = CARDGetSerialNo(0, &serial);
        if (res == 0)
        {
            u64 cache = *(u64*)&gSaveCardSerialHi;
            if (cache == 0 || cache != serial)
            {
                res = -0x55;
                gSaveCardState = 0xB;
            }
            else if (need_format)
            {
                res = CARDFormat(0);
            }
            else
            {
                CARDUnmount(0);
                mm_free(lbl_803DD040);
                lbl_803DD040 = 0;
                gSaveCardState = 0xD;
                return 1;
            }
        }
    }
    CARDUnmount(0);
    mm_free(lbl_803DD040);
    lbl_803DD040 = 0;
    switch (res)
    {
    case -2:
        gSaveCardState = 1;
        break;
    case -3:
        if (gSaveCardState != 3)
            gSaveCardState = 2;
        break;
    case -5:
        gSaveCardState = 4;
        break;
    case 0:
        gSaveCardState = 0xD;
        gSaveCardSerialLo = 0;
        gSaveCardSerialHi = 0;
        gSaveCardChecksumLo = 0;
        gSaveCardChecksumHi = 0;
        return 1;
    default:
        break;
    }
    return 0;
}

void saveFn_8007d960(u32 enable)
{
    u8 v = enable;
    lbl_803DD059 = v;
    if (v != 0)
    {
        return;
    }
    gSaveCardSerialLo = 0;
    gSaveCardSerialHi = 0;
    gSaveCardChecksumLo = 0;
    gSaveCardChecksumHi = 0;
}

void cardSetStatusNeedInit(void)
{
    gSaveCardState = 0xd;
}

s32 saveGameGetStatus(void)
{
    return gSaveCardState;
}

int cardDeleteFn_8007d99c(void)
{
    int res;
    int ok;

    gSaveCardRetry = 0;

    do
    {
        if (cardProbe(0) == 0)
        {
            ok = 0;
        }
        else
        {
            lbl_803DD040 = mmAlloc(0xA000, -1, 0);
            if (lbl_803DD040 == 0)
            {
                gSaveCardState = 8;
                ok = 0;
            }
            else
            {
                ok = 1;
            }
        }
        if (ok == 0)
        {
            return 0;
        }
        gSaveCardState = 0;
        res = CARDMount(0, lbl_803DD040, (CARDCallback)cardSetStatusNoCard2);
        if (res == 0 || res == -6)
        {
            res = CARDCheck(0);
        }
        if (res == 0)
        {
            res = CARDDelete(0, sMemoryCardFileName);
        }
        CARDUnmount(0);
        mm_free(lbl_803DD040);
        lbl_803DD040 = 0;

        switch (res + 13)
        {
        case 11:
            gSaveCardState = 1;
            break;
        case 10:
            if (gSaveCardState != 3)
                gSaveCardState = 2;
            break;
        case 0:
            gSaveCardState = 6;
            break;
        case 8:
            gSaveCardState = 4;
            break;
        case 13:
            gSaveCardState = 13;
            return 1;
        }
        showMemCardError(0);
    } while (gSaveCardRetry != 0);
    return 0;
}

int _saveGame(int slot, void* save, void* data)
{
    int ret;
    gSaveCardRetry = 0;
    cardShowLoadingMsg(1);
    do
    {
        ret = saveGame_prepareAndWrite(0, slot, 0, save, data, (SaveGameCallback)cardCb_8007e6d4);
        showMemCardError(0);
        if (gSaveCardRetry != 0)
        {
            cardShowLoadingMsg(1);
        }
    } while (gSaveCardRetry != 0);
    return ret;
}

int maybeTryLoadSave(void* data)
{
    int ret;
    gSaveCardRetry = 0;
    cardShowLoadingMsg(0);
    do
    {
        ret = saveGame_prepareAndWrite(1, 0, 0, data, NULL, (SaveGameCallback)saveCb_8007e748);
        showMemCardError(1);
        if (gSaveCardRetry != 0)
        {
            cardShowLoadingMsg(0);
        }
    } while (gSaveCardRetry != 0);
    return ret;
}

int loadSaveGame(int slot, void* save)
{
    int ret;
    gSaveCardRetry = 0;
    cardShowLoadingMsg(0);
    do
    {
        ret = saveGame_prepareAndWrite(1, slot, 0, save, NULL, (SaveGameCallback)saveCb_8007e77c);
        showMemCardError(0);
        if (gSaveCardRetry != 0)
        {
            cardShowLoadingMsg(0);
        }
    } while (gSaveCardRetry != 0);
    return ret;
}

int memCardFn_8007dd04(u8 retry)
{
    int ret;

    if (retry != 0)
    {
        gSaveCardRetry = 0;
        cardShowLoadingMsg(2);
    }
    do
    {
        ret = saveGame(0);
        if (ret != 0)
        {
            if (lbl_803DD05A != 0)
            {
                lbl_803DD05A = 0;
                CARDClose(&gSaveCardFileInfo.fileInfo);
            }
            CARDUnmount(0);
            mm_free(lbl_803DD040);
            lbl_803DD040 = 0;
            gSaveCardState = 13;
            if (ret == 2)
            {
                ret = saveGame_prepareAndWrite(0, 0, 0, NULL, NULL, NULL);
            }
        }
        if (retry != 0)
        {
            showMemCardError(0);
        }
        if (gSaveCardRetry != 0)
        {
            cardShowLoadingMsg(2);
        }
    } while (gSaveCardRetry != 0 && retry != 0);
    return ret;
}
int cardProbe(u8 retry)
{

    s32 memSize;
    s32 sectorSize;
    s32 res;

    if (retry != 0)
    {
        gSaveCardRetry = 0;
    }
    do
    {
        res = -1;
        while (res == -1)
        {
            res = CARDProbeEx(0, &memSize, &sectorSize);
        }
        if (res == 0)
        {
            if (sectorSize == 0x2000)
            {
                gSaveCardState = 13;
                return 1;
            }
            gSaveCardState = 7;
        }
        else if (res == -3)
        {
            gSaveCardState = 2;
        }
        else if (res == -2)
        {
            gSaveCardState = 1;
        }
        else
        {
            gSaveCardState = 0;
        }
        if (retry != 0)
        {
            showMemCardError(0);
        }
    } while (gSaveCardRetry != 0 && retry != 0);
    return 0;
}

void _initCardAndDsp(void)
{
    CARDInit();
}

void cardGetMessage(u32* buttons, u32* texts, u32* count)
{
    if (lbl_803DD059 != 0 && (gSaveCardState == 7 || gSaveCardState == 9))
    {
        gSaveCardState = 11;
    }
    switch (gSaveCardState)
    {
    case 0:
        *count = 0;
        gSaveCardState = 13;
        return;
    case 1:
        buttons[0] = 1;
        buttons[1] = 2;
        texts[0] = 0x325;
        texts[1] = 0x51D;
        texts[2] = 0x51B;
        *count = 2;
        return;
    case 2:
        buttons[0] = 1;
        buttons[1] = 2;
        texts[0] = 0x51A;
        texts[1] = 0x51D;
        texts[2] = 0x51B;
        *count = 2;
        return;
    case 3:
        buttons[0] = 1;
        buttons[1] = 2;
        texts[0] = 0x51A;
        texts[1] = 0x51D;
        texts[2] = 0x51B;
        *count = 2;
        return;
    case 4:
        buttons[0] = 1;
        buttons[1] = 2;
        texts[0] = 0x329;
        texts[1] = 0x51D;
        texts[2] = 0x51B;
        *count = 2;
        return;
    case 5:
        buttons[0] = 1;
        buttons[1] = 2;
        buttons[2] = 0;
        texts[0] = 0x51F;
        texts[1] = 0x51D;
        texts[2] = 0x51B;
        texts[3] = 0x326;
        *count = 3;
        return;
    case 6:
        buttons[0] = 1;
        buttons[1] = 2;
        buttons[2] = 0;
        texts[0] = 0x51E;
        texts[1] = 0x51D;
        texts[2] = 0x51B;
        texts[3] = 0x326;
        *count = 3;
        return;
    case 7:
        buttons[0] = 1;
        buttons[1] = 2;
        texts[0] = 0x51C;
        texts[1] = 0x51D;
        texts[2] = 0x51B;
        *count = 2;
        return;
    case 8:
        *count = 0;
        return;
    case 9:
        buttons[0] = 1;
        buttons[1] = 2;
        buttons[2] = 3;
        texts[0] = 0x32A;
        texts[1] = 0x51D;
        texts[2] = 0x51B;
        texts[3] = 0x520;
        *count = 3;
        return;
    case 10:
        buttons[0] = 2;
        buttons[1] = 4;
        texts[0] = 0x497;
        texts[1] = 0x51B;
        texts[2] = 0x522;
        *count = 2;
        return;
    case 11:
    case 12:
        buttons[0] = 1;
        buttons[1] = 2;
        texts[0] = 0x521;
        texts[1] = 0x51D;
        texts[2] = 0x51B;
        *count = 2;
        return;
    case 13:
    default:
        *count = 0;
        gSaveCardState = 13;
        return;
    }
}

void showMemCardError(u8 err)
{
    int opts[8];
    int msgs[8];
    int count;
    u32 saved;
    int sel;
    u8 submenu;
    int timer;
    u8 held;
    int* m;
    int y;
    int i;
    int j;
    int yy;
    char* t;
    int v;

    sel = 0;
    submenu = 0;
    timer = 0;
    held = 0;
    gSaveCardRetry = 0;
    if (gSaveCardState == 0xd || (err != 0 && gSaveCardState == 0xc))
    {
        return;
    }
    do
    {
        checkReset();
        padUpdate();
        mmFreeTick(0);
        timer += 0x3e8;
        waitNextFrame();
        saved = lbl_803DB708;
        hudDrawColored(getReflectionTexture1(), 0, 0, &saved, 0x200, 0);
        if (submenu != 0)
        {
            opts[0] = 6;
            opts[1] = 5;
            msgs[0] = 0x327;
            msgs[1] = 0x321;
            msgs[2] = 0x320;
            count = 2;
        }
        else
        {
            cardGetMessage((u32*)opts, (u32*)msgs, (u32*)&count);
        }
        gameTextSetColor(0xff, 0xc0, 0x40, 0xff);
        for (i = 0, m = msgs, y = 0x64; i < count + 1; m++, y += 0x14, i++)
        {
            t = (char*)gameTextGet(*m);
            yy = y + ((i > 0) ? 0x64 : 0);
            for (j = 0; j < *(u16*)(t + 2); j++)
            {
                gameTextShowStr((*(char***)(t + 8))[j], 0, 0, yy);
                yy += 0x18;
            }
            if (i == sel)
            {
                v = (int)(47.0f * fcos16HighPrecision(timer) + 208.0f);
                gameTextSetColor(v, v, v, 0xff);
            }
            else
            {
            gameTextSetColor(0xa0, 0xa0, 0xa0, 0xff);
            }
        }
        gameTextRun();
        GXFlush_(1, 0);
        if (padGetStickY(0) < 0 || padGetCY(0) < 0)
        {
            if (held == 0)
            {
                sel++;
                held = 1;
            }
        }
        else if (padGetStickY(0) > 0 || padGetCY(0) > 0)
        {
            if (held == 0)
            {
                sel--;
                held = 1;
            }
        }
        else
        {
            held = 0;
        }
        if (sel < 0)
        {
            sel = 0;
        }
        else if (sel > count - 1)
        {
            sel = count - 1;
        }
        if (getButtonsJustPressed(0) & 0x100)
        {
            switch (opts[sel])
            {
            case 0:
                submenu = 1;
                sel = 0;
                break;
            case 1:
                gSaveCardState = 0xd;
                gSaveCardRetry = 1;
                break;
            case 2:
                lbl_803DB424 = 0;
                gSaveCardState = 0xd;
                break;
            case 3:
                setGameState(6);
                lbl_803DB424 = 0;
                gSaveCardState = 0xd;
                break;
            case 4:
                cardDeleteFn_8007d99c();
                memCardFn_8007dd04(0);
                if (gSaveCardState == 0xd)
                {
                    gSaveCardRetry = 1;
                }
                break;
            case 5:
                submenu = 0;
                if (cardLoadFn_8007d72c() != 0)
                {
                    memCardFn_8007dd04(0);
                }
                if (gSaveCardState == 0xd)
                {
                    gSaveCardRetry = 1;
                }
                break;
            case 6:
                submenu = 0;
                break;
            default:
                gSaveCardState = 0xd;
            }
        }
    } while (gSaveCardState != 0xd);
}

/*
 * Per-frame "blocking" dialog renderer driven by the card-write retry
 * loops in _saveGame/DBC0/DC5C/DD04. Pumps 60 frames of the GX/dialog
 * pipeline; on each frame either lets the active controller draw its own
 * popup (gScreenTransitionInterface[0]->vtbl[1]) or falls back to hudDrawColored over the
 * cached prompt id in lbl_803DB708, then routes the OK/Cancel/back text
 * to gameTextFn_80016810 based on the dialog kind passed in.
 */
void cardShowLoadingMsg(u8 kind)
{
    int* buttons;
    u32 saved;
    int frame;
    int j;
    int count;
    f32 rectAlpha;
    void (*draw)(int, int, int);
    u8 mode = kind;

    gameTextSetWindow(0);
    for (frame = 0; frame < 0x3C; frame++)
    {
        padUpdate();
        mmFreeTick(0);
        waitNextFrame();
        count = getButtonObjects(&buttons) & 0xFF;
        if ((u32)count != 0)
        {
            draw = (*gScreenTransitionInterface)->init;
            draw(0, 0, 0);
            rectAlpha = 0.0f;
            drawRect(rectAlpha, rectAlpha, 0x280, 0x1E0);
            for (j = 0; j < count; j++)
            {
                objRenderModelAndHitVolumes((GameObject*)buttons[j], 0, 0, 0, 0, 1.0f);
            }
            curUiDllDraw(0, 0, 0, 0);
        }
        else
        {
            saved = lbl_803DB708;
            hudDrawColored(getReflectionTexture1(), 0, 0, &saved, 0x200, 0);
        }
    gameTextSetColor(0xFF, 0xFF, 0xFF, 0xFF);
        if (mode == 1)
        {
            gameTextFn_80016810(0x323, 0, 0xC8);
        }
        else if (mode == 2)
        {
            gameTextFn_80016810(0x573, 0, 0xC8);
        }
        else
        {
            gameTextFn_80016810(0x56C, 0, 0xC8);
        }
        gameTextRun();
        GXFlush_(1, 0);
    }
}

/*
 * Card-write callback dispatched through saveGame_prepareAndWrite from _saveGame.
 * Stages a per-slot 0x6EC-byte block plus the shared 0xE4-byte trailer
 * into the card-IO buffer (lbl_803DD044), then asks saveGame_doWrite(2) to
 * commit; if that fails it falls back to saveGame_doWrite(1).
 */
int cardCb_8007e6d4(u8 slot, int unused, void* src1, void* src2)
{
    int ret;
    memcpy(lbl_803DD044 + slot * 0x6EC + 0xA50, src1, 0x6EC);
    memcpy(lbl_803DD044 + 0x1F14, src2, 0xE4);
    ret = saveGame_doWrite(2);
    if (ret == 0)
    {
        ret = saveGame_doWrite(1);
    }
    return ret;
}

/*
 * Card-write callback dispatched through saveGame_prepareAndWrite from maybeTryLoadSave.
 * Copies the 0xE4-byte block at offset 0x1F14 in the card buffer (held in
 * lbl_803DD044) into the caller-supplied destination.
 */
int saveCb_8007e748(int saveId, int size, void* dst)
{
    memcpy(dst, lbl_803DD044 + 0x1F14, 0xE4);
    return 0;
}


/* .bss block 0x80391DC0-0x803967C0 */

