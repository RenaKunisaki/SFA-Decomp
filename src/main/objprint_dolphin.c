#include "game/objects/object.h"
#include "main/texture.h"
#include "main/model_light.h"
#include "main/rcp_dolphin_api.h"
#include "main/frame_timing.h"
#include "main/objprint_render_api.h"
#include "main/objprint_dolphin_api.h"
#include "main/objprintgxcolor.h"
#include "main/model.h"
#include "sys/objects.h"
#include "main/objlib_api.h"
#include "main/shader_api.h"
#include "main/pi_dolphin_api.h"
#include "main/curve_eval.h"
#include "main/audio/sfx.h"
#include "main/objprint_anim_api.h"
#include "main/objprint_character_api.h"
#include "main/objprint_sound_api.h"
#include "main/newshadows.h"
#include "main/objtexture.h"
#include "main/object_render.h"
#include "main/dll/modgfx.h"
#include "main/mm.h"
#include "dolphin/mtx.h"
#include "dolphin/os/OSCache.h"
#include "dolphin/gx/GXBump.h"
#include "dolphin/gx/GXCull.h"
#include "dolphin/gx/GXLighting.h"
#include "dolphin/gx/GXPixel.h"
#include "main/acosf.h"
#include "dolphin/gx/GXBump.h"
#include "dolphin/gx/GXGeometry.h"
#include "dolphin/gx/GXTev.h"
#include "dolphin/gx/GXTransform.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "track/intersect_api.h"
#include "track/intersect_fog_api.h"
#include "main/newshadows_shadow_api.h"
#include "main/dll/player_api.h"
#include "main/objprint_internal.h"
#include "main/dll/partfx_interface.h"
#include "dolphin/os/OSReport.h"
#include "main/gameloop_api.h"
#include "main/pad.h"
#include "main/pi_frame_api.h"
#include "main/pi_data_file_api.h"
#include "main/pi_dolphin.h"
#include "main/pi_flush_api.h"
#include "main/sky_api.h"
#include "main/textrender_api.h"
#include "main/camera_interface.h"
#include "main/mapEvent.h"
#include "main/mldf_fileid.h"
#include "main/model_render_instrs_api.h"
#include "main/model_runtime_api.h"
#include "main/object_transform.h"
#include "main/loaded_file_flags.h"
#include "main/map_load.h"
#include "main/objprint_load_api.h"
#include "main/objprint_api.h"
#include "main/table_file.h"
#include "main/fileio.h"
#include "main/vecmath.h"
#include "main/camera.h"
#include "dolphin/gx/GXDispList.h"
#include "main/dll/FRONT/n_options.h"
#include "main/dll/dll_80136a40.h"
#include "track/intersect_depth_read_api.h"
#include "track/intersect_depth_state_api.h"
#include "track/intersect_hud_api.h"
#include "track/intersect_texture_api.h"
#include "dolphin/os.h"
#include "string.h"

extern s32 lbl_803DCC48;
extern f32 lbl_803DEA04;
extern s32 lbl_803DCC44;
extern u8 lbl_803DCC3E;
extern u32 lbl_803DB468;
extern f32 lbl_803DEA28;
extern f32 lbl_803DEA2C;
extern f32 lbl_803DEA30;
extern f32 lbl_803DEA1C;

int objNormalizeRotationMatrix(f32* matrix, f32* out)
{
    Vec v3;
    Vec v1;
    Vec v2;
    f32 zero;

    v1.x = matrix[0];
    v1.y = matrix[1];
    v1.z = matrix[2];
    v2.x = matrix[4];
    v2.y = matrix[5];
    v2.z = matrix[6];
    v3.x = matrix[8];
    v3.y = matrix[9];
    v3.z = matrix[10];
    zero = 0.0f;

    if ((v1.x == zero && v1.y == zero && v1.z == zero) ||
        (v2.x == zero && v2.y == zero && v2.z == zero) ||
        (v3.x == zero && v3.y == zero && v3.z == zero))
    {
        return 0;
    }

    PSVECNormalize(&v1, &v1);
    PSVECNormalize(&v2, &v2);
    PSVECNormalize(&v3, &v3);

    out[0] = v1.x;
    out[1] = v1.y;
    out[2] = v1.z;
    out[3] = zero;
    out[4] = v2.x;
    out[5] = v2.y;
    out[6] = v2.z;
    out[7] = zero;
    out[8] = v3.x;
    out[9] = v3.y;
    out[10] = v3.z;
    out[11] = zero;
    return 1;
}

extern f32 gObjPrintHalfPi;
extern f32 gObjPrintNegHalfPi;
extern const f32 gObjPrintAngleUnitScale;
extern const f32 gObjPrintTwoPi;

int objRotateFn_8003bce8(f32* m, s16* outA, s16* outB, s16* outC)
{
    f32 buf[12];
    f32 x;
    f32 y;
    f32 z;

    if (objNormalizeRotationMatrix(m, buf) == 0)
    {
        return 0;
    }
    x = asinf(-buf[6]);
    if (x < gObjPrintHalfPi)
    {
        if (x > gObjPrintNegHalfPi)
        {
            y = __kernel_cos(buf[2], buf[10]);
            z = __kernel_cos(buf[4], buf[5]);
        }
        else
        {
            y = __kernel_cos(buf[1], buf[0]);
            z = lbl_803DEA04;
            y = z - y;
        }
    }
    else
    {
        y = __kernel_cos(buf[1], buf[0]);
        z = lbl_803DEA04;
        y = y - z;
    }
    *outC = (s16)(s32)(gObjPrintAngleUnitScale * z / gObjPrintTwoPi);
    *outB = (s16)(s32)(gObjPrintAngleUnitScale * x / gObjPrintTwoPi);
    *outA = (s16)(s32)(gObjPrintAngleUnitScale * y / gObjPrintTwoPi);
    return 1;
}


void modelMtxFn_8003be38(u8* def, int* model, f32* mtxA, f32* mtxB)
{
    void* cache;
    int count;
    int i;
    MtxPtr mid;
    MtxPtr dstB;
    MtxPtr dstA;
    f32 fill;

    cache = getCache();
    count = (s32)(u32)def[0xf3] + (s32)(u32)def[0xf4];
    dstA = (MtxPtr)((u8*)cache + 0x2700);
    mid = (MtxPtr)cache;
    dstB = (MtxPtr)((u8*)cache + 0x12c0);
    cacheQueueWait(0);
    i = 0;
    fill = lbl_803DEA04;
    for (; i < count; i++)
    {
        PSMTXConcat((MtxPtr)mtxA, dstA, mid);
        PSMTXConcat(mid, (MtxPtr)mtxB, dstB);
        dstB[0][3] = fill;
        dstB[1][3] = fill;
        dstB[2][3] = fill;
        dstA += 4;
        mid += 3;
        dstB += 3;
    }
    lbl_803DCC48 = 2;
}

void modelCalcVtxGroupMtxs(ModelFileHeader* def, ObjModel* model)
{
    Mtx ma;
    Mtx mb;
    Mtx trans;
    int off;
    int i;

    for (i = 0, off = 0; i < def->extraJointCount; i++)
    {
        MtxPtr out;
        MtxPtr m2;
        MtxPtr m1;
        char* jd;
        u8* grp;
        f32 w;
        f32 wi;

        grp = def->unk54 + off;
        out = (MtxPtr)ObjModel_GetJointMatrix((u8*)model, i + def->jointCount);
        m1 = (MtxPtr)ObjModel_GetJointMatrix((u8*)model, grp[0]);
        m2 = (MtxPtr)ObjModel_GetJointMatrix((u8*)model, grp[1]);

        w = (f32)grp[2] / 4.0f;
        wi = 1.0f - w;

        jd = (char*)def->jointData + grp[0] * 0x1c;
        PSMTXTrans(trans, -((ModelBone*)jd)->tail[0], -((ModelBone*)jd)->tail[1], -((ModelBone*)jd)->tail[2]);
        PSMTXConcat(m1, trans, ma);
        jd = (char*)def->jointData + grp[1] * 0x1c;
        PSMTXTrans(trans, -((ModelBone*)jd)->tail[0], -((ModelBone*)jd)->tail[1], -((ModelBone*)jd)->tail[2]);
        PSMTXConcat(m2, trans, mb);

        out[0][0] = ma[0][0] * w + mb[0][0] * wi;
        out[0][1] = ma[0][1] * w + mb[0][1] * wi;
        out[0][2] = ma[0][2] * w + mb[0][2] * wi;
        out[0][3] = ma[0][3] * w + mb[0][3] * wi;
        out[1][0] = ma[1][0] * w + mb[1][0] * wi;
        out[1][1] = ma[1][1] * w + mb[1][1] * wi;
        out[1][2] = ma[1][2] * w + mb[1][2] * wi;
        out[1][3] = ma[1][3] * w + mb[1][3] * wi;
        out[2][0] = ma[2][0] * w + mb[2][0] * wi;
        out[2][1] = ma[2][1] * w + mb[2][1] * wi;
        out[2][2] = ma[2][2] * w + mb[2][2] * wi;
        out[2][3] = ma[2][3] * w + mb[2][3] * wi;
        off += 4;
    }
}

void modelCalcVtxGroupMtxs(ModelFileHeader* def, ObjModel* model);

void modelInitMtxs(ModelFileHeader* def, ObjModel* model)
{
    int cache;
    int mtx;
    int count;
    u8 rem;

    cache = (int)getCache();
    if (def->extraJointCount != 0)
    {
        modelCalcVtxGroupMtxs(def, model);
    }
    count = (s32)(u32)def->jointCount + (s32)(u32)def->extraJointCount;
    if (count >= 2 && count <= 0x64)
    {
        mtx = (int)ObjModel_GetJointMatrix((u8*)model, 0);
        DCFlushRange((void*)mtx, count << 6);
        rem = (u8)(count << 1);
        cache += 0x2700;
        while (rem >= 0x80)
        {
            copyToCache((void*)cache, (void*)mtx, 0);
            rem -= 0x80;
            mtx += 0x1000;
            cache += 0x1000;
        }
        if (rem != 0)
        {
            copyToCache((void*)cache, (void*)mtx, rem);
        }
        lbl_803DCC48 = 1;
    }
    else
    {
        lbl_803DCC48 = 3;
    }
}

typedef struct IndTexMtx23
{
    f32 m[2][3];
} IndTexMtx23;

STATIC_ASSERT(sizeof(IndTexMtx23) == 0x18);

const IndTexMtx23 lbl_802C1B10 = {{{0.5f, 0.0f, 0.0f}, {0.0f, 0.5f, 0.0f}}};
const IndTexMtx23 lbl_802C1B28 = {{{0.0f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.5f}}};
const IndTexMtx23 lbl_802C1B40 = {{{0.5f, 0.0f, 0.0f}, {0.0f, 0.5f, 0.0f}}};
const IndTexMtx23 lbl_802C1B58 = {{{0.0f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.5f}}};

extern u8 lbl_803DCC3D;
extern u32 lbl_803DE9FC;
extern u32 lbl_803DEA00;
extern u32 lbl_803DB470;
extern int lbl_803DB498;
extern int lbl_803DB49C;
extern f32 lbl_803DEA34;
extern f32 lbl_803DEA38;

int modelRenderCb_8003c268(int obj, int* model, int ropIdx)
{
    Mtx mtx4;
    Mtx mtx3;
    Mtx mtx2;
    Mtx mtxR;
    Mtx mtx5;
    IndTexMtx23 mtxA;
    IndTexMtx23 mtxB;
    GXColor kc;
    Texture** noiseTextures;
    int noiseFrameCount;
    int t164;
    f32 sx;
    f32 sy;
    GXColor kc2;
    Texture** shadowTable;
    int shadowStride;
    int shadowRows;
    u8* rop;
    f32 fz;
    u8 v;

    kc = *(GXColor*)&lbl_803DE9FC;
    mtxA = lbl_802C1B40;
    mtxB = lbl_802C1B58;
    rop = (u8*)ObjModel_GetRenderOp((ModelFileHeader*)*model, ropIdx);
    if ((((ModelRenderOp*)rop)->flags & 0x200) == 0)
    {
        if ((lbl_803DCC44 & 3) != 0)
        {
            lbl_803DCC3E = 0;
            return 0;
        }
        lbl_803DCC3E = 1;
        objRenderFuzzFn_8003d6f8((void*)obj);
        return 1;
    }
    lbl_803DCC3E = 1;
    getNewShadowNoiseTextureFrames(&noiseTextures, &noiseFrameCount);
    fz = (f32)lbl_803DCC44 / (f32)(s32)noiseFrameCount;
    fz = fz * fz;
    fz = fz * lbl_803DEA28;
    selectTexture((Texture*)(textureIdxToPtr(*(u32*)Shader_getLayer(rop, 0))), 0);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD2, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    v = *(u8*)(obj + 0xf1);
    kc.b = v;
    kc.g = v;
    kc.r = v;
    GXSetTevKColor(GX_KCOLOR0, kc);
    GXSetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K0_A);
    GXSetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K0);
    PSMTXScale(mtx3, lbl_803DEA2C, *(f32*)&lbl_803DEA2C, lbl_803DEA04);
    PSMTXTrans(mtx2, lbl_803DEA28, *(f32*)&lbl_803DEA28, lbl_803DEA1C);
    PSMTXConcat(mtx2, mtx3, mtx3);
    GXLoadTexMtxImm(mtx3, GX_PTTEXMTX1, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_NRM, GX_TEXMTX0, GX_FALSE, GX_PTTEXMTX1);
    selectTexture((Texture*)(ObjModel_GetRenderOpTextureRefs((ObjModel*)model, ropIdx)->texture0), 1);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0A0);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_RASC);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG1);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    getNewShadowCausticTexture((u32*)&t164);
    selectTexture((Texture*)((void*)t164), 4);
    newshadows_getReflectionScrollOffsets(&sx, &sy);
    PSMTXTrans(mtxR, lbl_803DEA28 * sx, *(f32*)&lbl_803DEA28 * sy, lbl_803DEA04);
    mtxR[0][0] = lbl_803DEA1C;
    mtxR[1][1] = lbl_803DEA1C;
    GXLoadTexMtxImm(mtxR, GX_PTTEXMTX2, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTTEXMTX2);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP4);
    GXSetIndTexCoordScale(0, 0, 0);
    mtxA.m[0][0] = fz;
    mtxA.m[1][1] = fz;
    GXSetIndTexMtx(GX_ITM_0, mtxA.m, (s8)lbl_803DB498);
    GXSetTevIndirect(2, 0, 0, 7, 1, 6, 6, 0, 0, 0);
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_CPREV, GX_CC_C1, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    selectTexture((Texture*)(textureIdxToPtr(((ModelRenderOp*)rop)->layer1TextureId)), 2);
    GXSetTexCoordGen2(GX_TEXCOORD3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetIndTexOrder(GX_INDTEXSTAGE1, GX_TEXCOORD3, GX_TEXMAP2);
    GXSetIndTexCoordScale(1, 0, 0);
    mtxB.m[0][1] = fz;
    mtxB.m[1][2] = fz;
    GXSetIndTexMtx(GX_ITM_1, mtxB.m, (s8)lbl_803DB49C);
    GXSetTevIndirect(3, 1, 0, 7, 2, 0, 0, 1, 0, 1);
    selectTexture(noiseTextures[lbl_803DCC44], 3);
    PSMTXScale(mtx4, lbl_803DEA30, *(f32*)&lbl_803DEA30, lbl_803DEA1C);
    GXLoadTexMtxImm(mtx4, GX_PTTEXMTX0, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD4, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_TRUE, GX_PTTEXMTX0);
    GXSetTevKColorSel(GX_TEVSTAGE3, GX_TEV_KCSEL_1_2);
    GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD4, GX_TEXMAP3, GX_ALPHA_BUMPN);
    GXSetTevColorIn(GX_TEVSTAGE3, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV, GX_CC_CPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE3, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE3, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE3, GX_TEV_SUB, GX_TB_ADDHALF, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    if (lbl_803DCC44 < 0xc)
    {
        GXSetNumTevStages(4);
        GXSetNumIndStages(2);
        GXSetNumTexGens(5);
    }
    else
    {
        ModelLightStruct* lt;
        kc2 = *(GXColor*)&lbl_803DEA00;
        lt = objCreateLight((void*)obj, 0);
        if (lt != NULL)
        {
            modelLightStruct_setLightKind(lt, MODEL_LIGHT_KIND_DIRECTIONAL);
            modelLightStruct_setDirection(lt, lbl_803DEA04, lbl_803DEA34, *(f32*)&lbl_803DEA04);
            modelLightStruct_setDiffuseColor(lt, 0xff, 0xff, 0xff, 0xff);
            modelLightChannels_reset(0);
            modelLightChannel_configure(2, 0, 0);
            GXSetChanAmbColor(GX_ALPHA0, *(GXColor*)&lbl_803DB470);
            GXSetChanMatColor(GX_ALPHA0, *(GXColor*)&lbl_803DB468);
            modelLightStruct_loadChannelLight(2, lt, (GameObject*)obj);
            modelLightChannels_applyGXControls();
            ModelLightStruct_free(lt);
        }
        GXSetTevKColor(GX_KCOLOR0, kc2);
        GXSetTevKAlphaSel(GX_TEVSTAGE5, GX_TEV_KASEL_K0_A);
        GXSetTevKColorSel(GX_TEVSTAGE5, GX_TEV_KCSEL_K0);
        newshadows_getShadowTextureTable4x8(&shadowTable, &shadowStride, &shadowRows);
        selectTexture(shadowTable[(lbl_803DCC44 - 0xc) + lbl_803DCC3D * shadowStride], 5);
        PSMTXScale(mtx5, lbl_803DEA38, *(f32*)&lbl_803DEA38, lbl_803DEA1C);
        GXLoadTexMtxImm(mtx5, GX_PTTEXMTX3, GX_MTX3x4);
        GXSetTexCoordGen2(GX_TEXCOORD5, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_TRUE, GX_PTTEXMTX3);
        GXSetTevDirect(GX_TEVSTAGE4);
        GXSetTevOrder(GX_TEVSTAGE4, GX_TEXCOORD5, GX_TEXMAP5, GX_COLOR0A0);
        GXSetTevColorIn(GX_TEVSTAGE4, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE4, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
        GXSetTevSwapMode(GX_TEVSTAGE4, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(GX_TEVSTAGE4, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaOp(GX_TEVSTAGE4, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG1);
        GXSetTevDirect(GX_TEVSTAGE5);
        GXSetTevOrder(GX_TEVSTAGE5, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE5, GX_CC_CPREV, GX_CC_KONST, GX_CC_A1, GX_CC_ZERO);
        GXSetTevAlphaIn(GX_TEVSTAGE5, GX_CA_APREV, GX_CA_A1, GX_CA_A1, GX_CA_ZERO);
        GXSetTevSwapMode(GX_TEVSTAGE5, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(GX_TEVSTAGE5, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaOp(GX_TEVSTAGE5, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetNumTevStages(6);
        GXSetNumIndStages(2);
        GXSetNumTexGens(6);
    }
    GXSetCullMode(GX_CULL_BACK);
    {
        GXSetFog(GX_FOG_NONE, 0.0f, 0.0f, 0.0f, 0.0f, *(GXColor*)&lbl_803DB468);
    }
    gxSetZMode_(1, 3, 0);
    gxSetPeControl_ZCompLoc_(1);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    return 1;
}

extern GXColorS10 lbl_803DE9F4;
extern ObjPrintGXColor lbl_803DB494;
extern u8 lbl_803DCC35;
extern u8 lbl_803DCC36;
extern s32 lbl_803DCC5C;
extern u8 lbl_803DCC60;
extern int lbl_803DB48C;
extern int lbl_803DB490;

static inline int shaderProjDisabled(ModelLightStruct* light)
{
    int flag;
    int mode;
    modelLightStruct_getProjectionTevModes(light, &flag, &mode);
    return flag;
}

int shaderFuzzFn_8003cc1c(GameObject* obj, ObjModel* model, int ropIdx)
{
    Mtx mtx4;
    Mtx mtx3;
    Mtx mtx2;
    Mtx mtxR;
    IndTexMtx23 mtxA;
    IndTexMtx23 mtxB;
    GXColorS10 s10;
    int stage;
    int coord;
    Texture** noiseTextures;
    int noiseFrameCount;
    int texRef4;
    f32 sx;
    f32 sy;
    int projFlagOut1;
    u8* rop;
    f32 fz;
    int projBlendMode;
    u8 fancy;

    s10 = lbl_803DE9F4;
    mtxA = lbl_802C1B10;
    mtxB = lbl_802C1B28;
    rop = (u8*)ObjModel_GetRenderOp(model->file, ropIdx);
    if ((((ModelRenderOp*)rop)->flags & 0x200) == 0)
    {
        lbl_803DCC3E = 0;
        return 0;
    }
    lbl_803DCC3E = 1;
    getNewShadowNoiseTextureFrames(&noiseTextures, &noiseFrameCount);
    if (lbl_803DCC35 != 0)
    {
        fz = lbl_803DEA04;
    }
    else
    {
        fz = (f32)lbl_803DCC44 / (f32)(s32)noiseFrameCount;
        fz = fz * lbl_803DEA28;
    }
    selectTexture((Texture*)(textureIdxToPtr(*(u32*)Shader_getLayer(rop, 0))), 0);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    if (lbl_803DCC36 == 0)
    {
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    }
    else
    {
        if (lbl_803DCC36 == 1)
        {
            u8 v = lbl_803DCC44 << 4;
            lbl_803DB494.b = v;
            lbl_803DB494.g = v;
            lbl_803DB494.r = v;
            GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ONE, GX_CC_KONST, GX_CC_ZERO);
        }
        else
        {
            if (lbl_803DCC44 < 8)
            {
                lbl_803DB494.b = lbl_803DCC44 << 5;
            }
            else
            {
                lbl_803DB494.b = 0xff;
            }
            lbl_803DB494.g = lbl_803DB494.b;
            lbl_803DB494.r = lbl_803DB494.b;
            GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ZERO, GX_CC_KONST, GX_CC_ZERO);
        }
        GXSetTevKColor(GX_KCOLOR1, *(GXColor*)&lbl_803DB494);
        GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K1_A);
        GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K1);
    }
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD2, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    {
        u8 v = obj->unkF1;
        s10.b = v;
        s10.g = v;
        s10.r = v;
        s10.a = obj->anim.pad37[0] - 0xff;
    }
    GXSetTevColorS10(GX_TEVREG2, s10);
    PSMTXScale(mtx3, lbl_803DEA2C, *(f32*)&lbl_803DEA2C, lbl_803DEA04);
    PSMTXTrans(mtx2, lbl_803DEA28, *(f32*)&lbl_803DEA28, lbl_803DEA1C);
    PSMTXConcat(mtx2, mtx3, mtx3);
    GXLoadTexMtxImm(mtx3, GX_PTTEXMTX1, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_NRM, GX_TEXMTX0, GX_FALSE, GX_PTTEXMTX1);
    selectTexture((Texture*)(ObjModel_GetRenderOpTextureRefs(model, ropIdx)->texture0), 1);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0A0);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C2, GX_CC_RASC);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A2);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG1);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    if (lbl_803DCC5C != 0 && shaderProjDisabled(lbl_803DCC64) == 0)
    {
        fancy = 1;
    }
    else
    {
        fancy = 0;
    }
    if (fancy)
    {
        GXSetTevDirect(GX_TEVSTAGE2);
        GXLoadTexMtxImm((MtxPtr)modelLightStruct_getProjectionTexMtx(lbl_803DCC64), GX_PTTEXMTX3, GX_MTX3x4);
        GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_POS, GX_PNMTX0, GX_FALSE, GX_PTTEXMTX3);
        if (lbl_803DCC60 == 0 || lbl_803DCC60 == 2)
        {
            GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD1, GX_TEXMAP5, GX_COLOR0A0);
        }
        else
        {
            GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD1, GX_TEXMAP5, GX_COLOR1A1);
        }
        selectTexture((Texture*)(modelLightStruct_getProjectionTexture(lbl_803DCC64)), 5);
        modelLightStruct_getProjectionTevModes(lbl_803DCC64, &projFlagOut1, &projBlendMode);
        if (projBlendMode == 2)
        {
            GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_C1, GX_CC_TEXC, GX_CC_ZERO);
        }
        else if (projBlendMode == 3)
        {
            GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_C1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ZERO);
        }
        else if (projBlendMode == 1)
        {
            GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C1);
        }
        else if (lbl_803DCC60 == 0 || lbl_803DCC60 == 1)
        {
            GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_C1);
        }
        else
        {
            GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_RASA, GX_CC_TEXC, GX_CC_C1);
        }
        GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP0);
        if (projBlendMode == 1)
        {
            GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG1);
        }
        else
        {
            GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG1);
        }
        GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
        GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        stage = 3;
        coord = 5;
    }
    else
    {
        stage = 2;
        coord = 1;
    }
    getNewShadowCausticTexture((u32*)&texRef4);
    selectTexture((Texture*)((void*)texRef4), 4);
    newshadows_getReflectionScrollOffsets(&sx, &sy);
    PSMTXTrans(mtxR, lbl_803DEA28 * sx, *(f32*)&lbl_803DEA28 * sy, lbl_803DEA04);
    mtxR[0][0] = lbl_803DEA1C;
    mtxR[1][1] = lbl_803DEA1C;
    GXLoadTexMtxImm(mtxR, GX_PTTEXMTX2, GX_MTX3x4);
    GXSetTexCoordGen2(coord, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTTEXMTX2);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, coord, GX_TEXMAP4);
    GXSetIndTexCoordScale(0, 0, 0);
    mtxA.m[0][0] = fz;
    mtxA.m[1][1] = fz;
    GXSetIndTexMtx(GX_ITM_0, mtxA.m, (s8)lbl_803DB48C);
    GXSetTevIndirect(stage, 0, 0, 7, 1, 6, 6, 0, 0, 0);
    GXSetTevOrder(stage, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevSwapMode(stage, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorIn(stage, GX_CC_ZERO, GX_CC_CPREV, GX_CC_C1, GX_CC_ZERO);
    GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
    GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    if (*(void**)(rop + 0x38) != NULL)
    {
        selectTexture((Texture*)(textureIdxToPtr(((ModelRenderOp*)rop)->layer1TextureId)), 2);
        GXSetTexCoordGen2(GX_TEXCOORD3, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
        GXSetIndTexOrder(GX_INDTEXSTAGE1, GX_TEXCOORD3, GX_TEXMAP2);
        GXSetIndTexCoordScale(1, 0, 0);
        mtxB.m[0][1] = fz;
        mtxB.m[1][2] = fz;
        GXSetIndTexMtx(GX_ITM_1, mtxB.m, (s8)lbl_803DB490);
        GXSetTevIndirect(stage + 1, 1, 0, 7, 2, 0, 0, 1, 0, 1);
    }
    else
    {
        GXSetIndTexOrder(GX_INDTEXSTAGE1, GX_TEXCOORD3, GX_TEXMAP2);
        GXSetIndTexCoordScale(1, 0, 0);
        mtxB.m[0][1] = lbl_803DEA04;
        mtxB.m[1][2] = lbl_803DEA04;
        GXSetIndTexMtx(GX_ITM_1, mtxB.m, -0xf);
        GXSetTevIndirect(stage + 1, 1, 0, 7, 2, 0, 0, 1, 0, 0);
    }
    selectTexture(noiseTextures[lbl_803DCC44], 3);
    PSMTXScale(mtx4, lbl_803DEA30, *(f32*)&lbl_803DEA30, lbl_803DEA1C);
    GXLoadTexMtxImm(mtx4, GX_PTTEXMTX0, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD4, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_TRUE, GX_PTTEXMTX0);
    GXSetTevKColorSel(stage + 1, GX_TEV_KCSEL_1_2);
    if (*(void**)(rop + 0x38) != NULL)
    {
        GXSetTevOrder(stage + 1, GX_TEXCOORD4, GX_TEXMAP3, GX_ALPHA_BUMPN);
        GXSetTevAlphaIn(stage + 1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_APREV);
    }
    else
    {
        GXSetTevOrder(stage + 1, GX_TEXCOORD4, GX_TEXMAP3, GX_COLOR_NULL);
        GXSetTevAlphaIn(stage + 1, GX_CA_TEXA, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
    }
    GXSetTevColorIn(stage + 1, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV, GX_CC_CPREV);
    GXSetTevSwapMode(stage + 1, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(stage + 1, GX_TEV_SUB, GX_TB_ADDHALF, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(stage + 1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    if (fancy)
    {
        GXSetNumTevStages(5);
        GXSetNumTexGens(6);
    }
    else
    {
        GXSetNumTevStages(4);
        GXSetNumTexGens(5);
    }
    GXSetNumIndStages(2);
    GXSetCullMode(GX_CULL_BACK);
    if ((model->file->flags & 0x100) != 0)
    {
        GXSetFog(GX_FOG_NONE, 0.0f, 0.0f, 0.0f, 0.0f, *(GXColor*)&lbl_803DB468);
    }
    else
    {
        _gxSetFogParams();
    }
    gxSetZMode_(1, 3, 0);
    gxSetPeControl_ZCompLoc_(1);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    return 1;
}

ModelLightStruct* lbl_803DCC64;
u8 lbl_803DCC60;
s32 lbl_803DCC5C;
u8 gObjOverrideColor[3];
GXColor gObjCurChanColor;
f32 gObjShadowDist;
u8 gObjShadowNear;
s32 lbl_803DCC48;
s32 lbl_803DCC44;
s32 gObjFuzzStep;
u8 lbl_803DCC3E;
u8 lbl_803DCC3D;
u8 gObjAlphaCompareThreshold;
f32 gObjFuzzPhase;
u8 lbl_803DCC36;
u8 lbl_803DCC35;
u8 lbl_803DCC34;
u32 gObjCachedModel;
u32 gObjCachedTexture;
u8 gObjRenderSetupDone;
u8 lbl_803DCC29;
u8 gObjOverrideColorPending;
u32 curObjMtx;
u8 lbl_803DCC20;

u32 lbl_803DB468 = 0xFFFFFFFF;
u32 gObjGxDefaultChanColor = 0xFF;
u32 lbl_803DB470 = 0;
u32 gObjGxVtxDescCache = 0xFFFFFFFF;
u8 gObjGxBlendModeCache = 0xFF;
u8 gObjGxZCompLocCache = 0xFF;
u32 gObjGxAlphaCompareCache = 0xFFFFFFFF;
u8 gObjGxZWriteCache = 0xFF;
u8 gObjGxZCompareCache = 0xFF;
u8 gObjGxCullModeCache = 0xFF;
u8 gObjGxKColorCache[4] = {0};
u8 gObjShadowColor[4] = {0x20, 0x30, 0xFF, 0xFF};
int lbl_803DB48C = -1;
int lbl_803DB490 = -1;
ObjPrintGXColor lbl_803DB494 = {0xFF, 0xFF, 0xFF, 0xFF};
int lbl_803DB498 = -3;
int lbl_803DB49C = -1;

const int lbl_802C1B70[56] = {
    0x2B, 0x1, 0x2A, 0x2, 0x2F, 0x8, 0x30, 0x4,
    0x46, 0x1, 0x45, 0x2, 0x49, 0x8, 0x4A, 0x4,
    0x24, 0x20, 0x23, 0x10, 0x4E, 0x20, 0x4D, 0x10,
    0x21, 0x80, 0x20, 0x40, 0x4C, 0x80, 0x4B, 0x40,
    0x25, 0x100, 0x26, 0x200, 0x47, 0x100, 0x48, 0x200,
    0x1B, 0x1000, 0x1A, 0x2000, 0x54, 0x1000, 0x53, 0x2000,
    0xD, 0x400, 0xE, 0x800, 0x55, 0x400, 0x56, 0x800
};


/*
 * One render op ("shader") record from the model file's renderOps array,
 * bound by opcode 1 of the render-instruction stream.  Layer records
 * (Shader_getLayer) precede these fields; byte 0x41 holds the layer count
 * and byte 0x40 the layer blend flags (0x10 = additive path).
 * flags (+0x3C) bits seen in this file: 8 = backface cull, 0x100 = extra
 * projected-texture pass, 0x400 = alpha-test opaque, 0x200 = fuzz overlay
 * eligible, 0x20000 = water/caustic hook, 0x100000 = decal second layer,
 * 0x40000000 = force blend.
 */
typedef struct ObjModelRenderOp
{
    u8 pad0[0x18 - 0x0];
    u32 textureId;
    u32 unk1C;
    u8 pad20[0x24 - 0x20];
    u32 indirectTextureId;
    u8 pad28[0x34 - 0x28];
    u32 envTextureId;
    u8 pad38[0x3C - 0x38];
    u32 flags;
} ObjModelRenderOp;
typedef struct
{
    u8* data;
    int pad[3];
    int pos;
} MtxBitStream;
typedef u8 (*ObjModelRenderCb)(int* obj, int* am, int p3);



// ObjModelRenderOp.flags (+0x3C) bits
#define SHADER_FLAG_BACKFACE_CULL      0x8
#define SHADER_FLAG_PROJECTED_TEX_PASS 0x100
#define SHADER_FLAG_ALPHA_TEST_OPAQUE  0x400
#define SHADER_FLAG_WATER_CAUSTIC      0x20000
#define SHADER_FLAG_DECAL_LAYER        0x100000
#define SHADER_FLAG_FORCE_BLEND        0x40000000

#define OBJPRINT_MODEL_DEF(obj)         (((ObjAnimComponent*)(obj))->modelInstance)

extern u32 lbl_803DE9F0;

void objRenderFuzzFn_8003d6f8(void* objArg)
{
    ModelLightStruct* renderHandle;
    int obj = (int)objArg;
    volatile u32 savedEnvColor;
    Texture** shadowTable;
    int shadowStride;
    int shadowParam;
    float mtx[12];

    savedEnvColor = lbl_803DE9F0;
    renderHandle = objCreateLight((void*)obj, '\0');
    if (renderHandle != 0x0)
    {
        modelLightStruct_setLightKind(renderHandle, 4);
        modelLightStruct_setDirection(renderHandle, lbl_803DEA04, lbl_803DEA34, *(f32*)&lbl_803DEA04);
        modelLightStruct_setDiffuseColor(renderHandle, 0xff, 0xff, 0xff, 0xff);
        modelLightChannels_reset(0);
        modelLightChannel_configure(2, 0, 0);
        GXSetChanAmbColor(GX_ALPHA0, *(GXColor*)&lbl_803DB470);
        GXSetChanMatColor(GX_ALPHA0, *(GXColor*)&lbl_803DB468);
        modelLightStruct_loadChannelLight(2, renderHandle, (GameObject*)obj);
        modelLightChannels_applyGXControls();
        ModelLightStruct_free(renderHandle);
    }
    GXSetTevKColor(GX_KCOLOR0, *(GXColor*)&savedEnvColor);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
    newshadows_getShadowTextureTable4x8(&shadowTable, &shadowStride, &shadowParam);
    selectTexture(shadowTable[(lbl_803DCC44 >> 2) + lbl_803DCC3D * shadowStride], 0);
    PSMTXScale((MtxPtr)mtx, lbl_803DEA38, *(f32*)&lbl_803DEA38, lbl_803DEA1C);
    GXLoadTexMtxImm((const f32 (*)[4])mtx, 0x40, 0);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_TRUE, GX_PTTEXMTX0);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD1, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_DIVIDE_2, GX_TRUE, GX_TEVPREV);
    GXSetNumTevStages(1);
    GXSetNumIndStages(0);
    GXSetNumTexGens(2);
    GXSetCullMode(2);
    GXSetFog(0, 0.0f, 0.0f, 0.0f, 0.0f, *(GXColor*)&lbl_803DB468);
    gxSetZMode_(1, 3, 0);
    gxSetPeControl_ZCompLoc_(1);
    GXSetBlendMode(1, 4, 5, 5);
    return;
}



typedef union
{
    u8 u8;
    u16 u16;
    u32 u32;
    s16 s16;
    s32 s32;
    f32 f32;
} ObjWGPipe;

extern ObjWGPipe GXWGFifo : (0xCC008000);

extern u8 gObjGxPosMtxIdTable[12];

void objRenderFn_8003d980(u8* obj, int* p2)
{
    f32 wm[16];
    f32 cm[16];
    f32 sm[12];
    MatrixTransform blk;
    int* mdl = p2;
    u8* data = (u8*)mdl[22];
    s16 b;
    s16 c;
    u16* idx;
    int off;
    s16* v;
    int i;
    s16* verts;
    s16* uvs;
    u8* tri;
    s16 a;
    s16* uv;
    f32* vm = Camera_GetViewMatrix();
    Obj_BuildWorldTransformMatrix((GameObject*)obj, wm, 0);
    PSMTXConcat((MtxPtr)vm, (MtxPtr)wm, (MtxPtr)cm);
    GXLoadPosMtxImm((const f32 (*)[4])cm, gObjGxPosMtxIdTable[0]);
    GXSetCurrentMtx(gObjGxPosMtxIdTable[0]);
    PSMTXScale((MtxPtr)sm, lbl_803DEA1C / ((GameObject*)obj)->anim.rootMotionScale,
               lbl_803DEA1C / ((GameObject*)obj)->anim.rootMotionScale, lbl_803DEA1C);
    cm[3] = lbl_803DEA04;
    cm[7] = lbl_803DEA04;
    cm[11] = lbl_803DEA04;
    PSMTXConcat((MtxPtr)cm, (MtxPtr)sm, (MtxPtr)cm);
    GXLoadTexMtxImm((const f32 (*)[4])cm, 0x1e, 0);
    gxTextureFn_80072dfc(obj, (void**)mdl, 0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    verts = *(s16**)(data + 4);
    uvs = *(s16**)(data + 8);
    GXBegin(GX_TRIANGLES, GX_VTXFMT7, *(u16*)(data + 0xc) * 3);
    {
        i = 0;
        off = 0;
        for (; i < *(u16*)(data + 0xc); i++)
        {
            int k;
            tri = *(u8**)data + off;
            idx = (u16*)tri;
            for (k = 0; k < 3; k++)
            {
                v = verts + *idx * 3;
                c = v[2];
                b = v[1];
                a = v[0];
                GXWGFifo.s16 = a;
                GXWGFifo.s16 = b;
                GXWGFifo.s16 = c;
                {
                    u8 b2;
                    u8 c2;
                    u8 a2;
                    c2 = tri[8];
                    b2 = tri[7];
                    a2 = tri[6];
                    GXWGFifo.u8 = a2;
                    GXWGFifo.u8 = b2;
                    GXWGFifo.u8 = c2;
                }
                uv = uvs + *idx * 2;
                b = uv[1];
                a = uv[0];
                GXWGFifo.s16 = a;
                GXWGFifo.s16 = b;
                idx++;
            }
            off += 0xa;
        }
    }
    GXSetCurrentMtx(0);
    if (randomGetRange(0, 5) == 0)
    {
        int m = randomGetRange(0, *(s16*)(data + 0xe) - 1) * 3;
        f32 fs = ((GameObject*)obj)->anim.rootMotionScale;
        blk.x = fs * (f32)(verts[m] >> 8) + ((GameObject*)obj)->anim.localPosX;
        blk.y = fs * (f32)(verts[m + 1] >> 8) + ((GameObject*)obj)->anim.localPosY;
        blk.z = fs * (f32)(verts[m + 2] >> 8) + ((GameObject*)obj)->anim.localPosZ;
        blk.scale = lbl_803DEA1C;
        blk.rotX = 0;
        blk.rotZ = 0;
        blk.rotY = 0;
        (*gPartfxInterface)->spawnObject(obj, 0x7fd, &blk, 0x200001, -1, NULL);
    }
}

void objFn_8003dc50(u8* obj, u8* model)
{
    int t2;
    int t10;
    int en2;
    int chan;
    u8 ch;
    u16 f;
    u8 b;
    ModelLightStruct* larr[6];
    s32 count;
    GXColor c;

    count = 0;
    lbl_803DCC5C = 0;
    b = ((ModelFileHeader*)obj)->flags24;
    t2 = b & 2;
    if (t2)
    {
        en2 = 1;
    }
    else
    {
        en2 = 0;
    }
    t10 = b & 0x10;
    chan = t10 ? 4 : 0;
    if (((ModelFileHeader*)obj)->shaderFlags & 2)
    {
        if (t2 || t10)
        {
            gObjCurChanColor.a = 0;
            GXSetChanAmbColor((u8)chan, gObjCurChanColor);
            GXSetChanCtrl(GX_COLOR0, GX_TRUE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
            GXSetChanCtrl(GX_ALPHA0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
            GXSetNumChans(1);
        }
        else
        {
            GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
            GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
            GXSetNumChans(0);
        }
    }
    else
    {
        modelLightChannels_reset(0);
        ch = chan;
        modelLightChannel_configure(ch, 0, en2);
        f = ((ModelFileHeader*)obj)->shaderFlags;
        if (!(f & 9))
        {
            int mode;
            if (f & 0xc)
            {
                mode = 2;
                GXSetChanAmbColor(ch, *(GXColor*)&gObjGxDefaultChanColor);
            }
            else
            {
                int l;
                mode = 6;
                l = OBJPRINT_MODEL_DEF(model)->modelLightMaskIndex;
                if (l == 0)
                {
                    modelTextureFn_80089970(((GameObject*)model)->lightColorSlot);
                    textureColorFn_8008991c(((GameObject*)model)->lightColorSlot, &c.r, &c.g, &c.b);
                }
                else
                {
                    lightGetColor(l, &c.r, &c.g, &c.b);
                }
                c.a = 0;
                GXSetChanAmbColor(ch, c);
            }
            {
                u32 nl = (*(u8**)(model + 0x50))[0x8c];
                if (nl != 0)
                {
                    modelLightStruct_selectObjectLights((GameObject*)model, larr, nl, &count, mode);
                }
            }
            if (count == 0)
            {
                GXSetChanMatColor(ch, *(GXColor*)&gObjGxDefaultChanColor);
            }
            else
            {
                GXSetChanMatColor(ch, *(GXColor*)&lbl_803DB468);
            }
            {
                int i;
                ModelLightStruct** p;
                i = 0;
                p = larr;
                for (; i < count; i++)
                {
                    modelLightStruct_loadChannelLight(ch, *p, (GameObject*)model);
                    p++;
                }
            }
        }
        else
        {
            if (f & 1)
            {
                GXSetChanMatColor(chan & 0xff, *(GXColor*)&lbl_803DB468);
            }
            else
            {
                GXSetChanMatColor(chan & 0xff, *(GXColor*)&gObjGxDefaultChanColor);
            }
        }
        {
            u32 nf = ((ModelFileHeader*)obj)->texMtxCount;
            if (nf != 0)
            {
                modelLightStruct_selectObjectLights((GameObject*)model, &lbl_803DCC64, nf, &lbl_803DCC5C, 8);
                if ((OBJPRINT_MODEL_DEF(model)->renderFlags & OBJDEF_RENDERFLAG_PROJECTED_SHADOW) || gObjShadowNear)
                {
                    lbl_803DCC5C = 0;
                }
                {
                    u8 got;
                    ModelLightStruct** lp;
                    u8* sp;
                    int k;
                    got = 0;
                    k = 0;
                    lp = &lbl_803DCC64;
                    sp = &lbl_803DCC60;
                    for (; k < lbl_803DCC5C; k++)
                    {
                        int t = modelLightStruct_getProjectedLightChannelPreference(*lp);
                        if (!got && t == 1)
                        {
                            *sp = 1;
                            got = 1;
                        }
                        else if (k == 0)
                        {
                            *sp = 2;
                        }
                        else
                        {
                            *sp = 3;
                        }
                        modelLightChannel_configure(*sp, 2, 0);
                        modelLightStruct_loadChannelLight(*sp, *lp, (GameObject*)model);
                        GXSetChanAmbColor(*sp, *(GXColor*)&lbl_803DB470);
                        GXSetChanMatColor(*sp, *(GXColor*)&lbl_803DB468);
                        lp++;
                        sp++;
                    }
                }
            }
        }
        modelLightChannels_applyGXControls();
        {
            u8 b5f = OBJPRINT_MODEL_DEF(model)->renderFlags;
            if ((b5f & 4) || gObjShadowNear)
            {
                lbl_803DCC5C = 2;
            }
            else if (b5f & 0x11)
            {
                lbl_803DCC5C = 1;
            }
        }
    }
}
