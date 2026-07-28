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
extern s32 gObjFuzzLayerIndex;
extern u8 lbl_803DCC3E;
extern u32 lbl_803DB468;
extern f32 lbl_803DEA28;

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

int objMatrixToRotation(f32* m, s16* outA, s16* outB, s16* outC)
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
            z = 0.0f;
            y = z - y;
        }
    }
    else
    {
        y = __kernel_cos(buf[1], buf[0]);
        z = 0.0f;
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
    fill = 0.0f;
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

#include "main/objprint_dolphin_internal.h"

const IndTexMtx23 lbl_802C1B10 = {{{0.5f, 0.0f, 0.0f}, {0.0f, 0.5f, 0.0f}}};
const IndTexMtx23 lbl_802C1B28 = {{{0.0f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.5f}}};
const IndTexMtx23 lbl_802C1B40 = {{{0.5f, 0.0f, 0.0f}, {0.0f, 0.5f, 0.0f}}};
const IndTexMtx23 lbl_802C1B58 = {{{0.0f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.5f}}};

extern u8 lbl_803DCC3D;
extern u32 lbl_803DEA00;
extern u32 lbl_803DB470;
extern int lbl_803DB498;
extern int lbl_803DB49C;

int modelRenderCb_8003c268(int obj, int* model, int ropIdx)
{
    Mtx mtx4;
    Mtx mtx3;
    Mtx mtx2;
    Mtx mtxR;
    Mtx mtx5;
    IndTexMtx23 mtxA;
    IndTexMtx23 mtxB;
    GXColor kc = {0xFF, 0xFF, 0xFF, 0xFF};
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

    mtxA = lbl_802C1B40;
    mtxB = lbl_802C1B58;
    rop = (u8*)ObjModel_GetRenderOp((ModelFileHeader*)*model, ropIdx);
    if ((((ModelRenderOp*)rop)->flags & 0x200) == 0)
    {
        if ((gObjFuzzLayerIndex & 3) != 0)
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
    fz = (f32)gObjFuzzLayerIndex / (f32)(s32)noiseFrameCount;
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
    PSMTXScale(mtx3, -0.5f, -0.5f, 0.0f);
    PSMTXTrans(mtx2, lbl_803DEA28, *(f32*)&lbl_803DEA28, 1.0f);
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
    PSMTXTrans(mtxR, lbl_803DEA28 * sx, *(f32*)&lbl_803DEA28 * sy, 0.0f);
    mtxR[0][0] = 1.0f;
    mtxR[1][1] = 1.0f;
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
    selectTexture(noiseTextures[gObjFuzzLayerIndex], 3);
    PSMTXScale(mtx4, 37.5f, 37.5f, 1.0f);
    GXLoadTexMtxImm(mtx4, GX_PTTEXMTX0, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD4, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_TRUE, GX_PTTEXMTX0);
    GXSetTevKColorSel(GX_TEVSTAGE3, GX_TEV_KCSEL_1_2);
    GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD4, GX_TEXMAP3, GX_ALPHA_BUMPN);
    GXSetTevColorIn(GX_TEVSTAGE3, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV, GX_CC_CPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE3, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
    GXSetTevSwapMode(GX_TEVSTAGE3, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE3, GX_TEV_SUB, GX_TB_ADDHALF, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    if (gObjFuzzLayerIndex < 0xc)
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
            modelLightStruct_setDirection(lt, 0.0f, -0.707f, 0.0f);
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
        selectTexture(shadowTable[(gObjFuzzLayerIndex - 0xc) + lbl_803DCC3D * shadowStride], 5);
        PSMTXScale(mtx5, 20.0f, 20.0f, 1.0f);
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
    GXColorS10 s10 = {0xFF, 0xFF, 0xFF, 0xFF};
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
        fz = 0.0f;
    }
    else
    {
        fz = (f32)gObjFuzzLayerIndex / (f32)(s32)noiseFrameCount;
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
            u8 v = gObjFuzzLayerIndex << 4;
            lbl_803DB494.b = v;
            lbl_803DB494.g = v;
            lbl_803DB494.r = v;
            GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ONE, GX_CC_KONST, GX_CC_ZERO);
        }
        else
        {
            if (gObjFuzzLayerIndex < 8)
            {
                lbl_803DB494.b = gObjFuzzLayerIndex << 5;
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
    PSMTXScale(mtx3, -0.5f, -0.5f, 0.0f);
    PSMTXTrans(mtx2, lbl_803DEA28, *(f32*)&lbl_803DEA28, 1.0f);
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
    PSMTXTrans(mtxR, lbl_803DEA28 * sx, *(f32*)&lbl_803DEA28 * sy, 0.0f);
    mtxR[0][0] = 1.0f;
    mtxR[1][1] = 1.0f;
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
        mtxB.m[0][1] = 0.0f;
        mtxB.m[1][2] = 0.0f;
        GXSetIndTexMtx(GX_ITM_1, mtxB.m, -0xf);
        GXSetTevIndirect(stage + 1, 1, 0, 7, 2, 0, 0, 1, 0, 0);
    }
    selectTexture(noiseTextures[gObjFuzzLayerIndex], 3);
    PSMTXScale(mtx4, 37.5f, 37.5f, 1.0f);
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
s32 gObjFuzzLayerIndex;
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
u8 gObjRenderingShadowPass;
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






#define OBJPRINT_MODEL_DEF(obj)         (((ObjAnimComponent*)(obj))->modelInstance)


void objRenderFuzzFn_8003d6f8(void* objArg)
{
    ModelLightStruct* renderHandle;
    int obj = (int)objArg;
    GXColor savedEnvColor = {0xD8, 0xE0, 0xFF, 0xFF};
    Texture** shadowTable;
    int shadowStride;
    int shadowParam;
    float mtx[12];

    renderHandle = objCreateLight((void*)obj, '\0');
    if (renderHandle != 0x0)
    {
        modelLightStruct_setLightKind(renderHandle, 4);
        modelLightStruct_setDirection(renderHandle, 0.0f, -0.707f, 0.0f);
        modelLightStruct_setDiffuseColor(renderHandle, 0xff, 0xff, 0xff, 0xff);
        modelLightChannels_reset(0);
        modelLightChannel_configure(2, 0, 0);
        GXSetChanAmbColor(GX_ALPHA0, *(GXColor*)&lbl_803DB470);
        GXSetChanMatColor(GX_ALPHA0, *(GXColor*)&lbl_803DB468);
        modelLightStruct_loadChannelLight(2, renderHandle, (GameObject*)obj);
        modelLightChannels_applyGXControls();
        ModelLightStruct_free(renderHandle);
    }
    GXSetTevKColor(GX_KCOLOR0, savedEnvColor);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
    newshadows_getShadowTextureTable4x8(&shadowTable, &shadowStride, &shadowParam);
    selectTexture(shadowTable[(gObjFuzzLayerIndex >> 2) + lbl_803DCC3D * shadowStride], 0);
    PSMTXScale((MtxPtr)mtx, 20.0f, 20.0f, 1.0f);
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



#include "main/dll/ppcwgpipe_struct.h"

extern PPCWGPipe GXWGFifo : (0xCC008000);

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
    PSMTXScale((MtxPtr)sm, 1.0f / ((GameObject*)obj)->anim.rootMotionScale,
               1.0f / ((GameObject*)obj)->anim.rootMotionScale, 1.0f);
    cm[3] = 0.0f;
    cm[7] = 0.0f;
    cm[11] = 0.0f;
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
        blk.scale = 1.0f;
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

extern s32 lbl_803DCC48;
extern f32 lbl_803DEA04;

#include "main/objprint_dolphin_internal.h"

extern u8 gObjGxPosMtxIdTable[12];

void modelMtxFn_8003be38(u8* def, int* model, f32* mtxA, f32* mtxB);

void modelLoadMtxsToGx(int obj, int* model, MtxBitStream* bs, f32* mtx)
{
    char* cache = (char*)getCache();
    if (lbl_803DCC48 == 1)
    {
        char* c2 = (char*)getCache();
        char* src;
        char* dst;
        int i;
        obj = ((ModelFileHeader*)obj)->jointCount + ((ModelFileHeader*)obj)->extraJointCount;
        src = c2 + 0x2700;
        dst = c2;
        cacheQueueWait(0);
        for (i = 0; i < obj; i++)
        {
            PSMTXConcat((MtxPtr)mtx, (MtxPtr)(f32*)src, (MtxPtr)(f32*)dst);
            src += 0x40;
            dst += 0x30;
        }
        lbl_803DCC48 = 2;
    }
    {
        u8* tbl[1];
        int i;
        int count;
        f32 tmp[12];
        {
            u32 w;
            int pos = bs->pos;
            int off = pos >> 3;
            u8* p;
            w = bs->data[off];
            p = (u8*)(off + (char*)bs->data);
            w |= p[1] << 8;
            w |= p[2] << 16;
            bs->pos = pos + 4;
            count = (w >> (pos & 7)) & 0xf;
        }
        i = 0;
        tbl[0] = gObjGxPosMtxIdTable;
        for (; i < count; i++)
        {
            int idx;
            {
                u32 w;
                int pos = bs->pos;
                int off = pos >> 3;
                u8* p = (u8*)(off + (char*)bs->data);
                w = p[0];
                w |= p[1] << 8;
                w |= p[2] << 16;
                bs->pos = pos + 8;
                idx = (w >> (pos & 7)) & 0xff;
            }
            if (lbl_803DCC48 == 2)
            {
                GXLoadPosMtxImm((const f32 (*)[4])(cache + idx * 0x30), *tbl[0]);
            }
            else
            {
                PSMTXConcat((MtxPtr)mtx, (MtxPtr)(f32*)ObjModel_GetJointMatrix((u8*)model, idx), (MtxPtr)tmp);
                GXLoadPosMtxImm((const f32 (*)[4])tmp, *tbl[0]);
            }
            tbl[0]++;
        }
    }
}

void renderOpMatrix(u8* hdr, int* model, MtxBitStream* bs, f32* m1, f32* mtx, u8 nrm, u8 tex, u8 skip)
{
    u8* tbl[1];
    char* cache;
    tbl[0] = gObjGxPosMtxIdTable;
    cache = (char*)getCache();
    if (lbl_803DCC48 == 1)
    {
        if (skip == 0)
        {
            modelMtxFn_8003be38(hdr, model, mtx, m1);
        }
        else
        {
            char* c2 = (char*)getCache();
            char* dst;
            int i;
            int total = hdr[0xf3] + hdr[0xf4];
            hdr = (u8*)(c2 + 0x2700);
            dst = c2;
            cacheQueueWait(0);
            for (i = 0; i < total; i++)
            {
                PSMTXConcat((MtxPtr)mtx, (MtxPtr)(f32*)hdr, (MtxPtr)(f32*)dst);
                hdr += 0x40;
                dst += 0x30;
            }
            lbl_803DCC48 = 2;
        }
    }
    {
        u8* tbl2;
        int i;
        int count;
        f32 tmp[12];
        {
            u32 w;
            int pos = bs->pos;
            int off = pos >> 3;
            u8* p;
            w = bs->data[off];
            p = (u8*)(off + (char*)bs->data);
            w |= p[1] << 8;
            w |= p[2] << 16;
            bs->pos = pos + 4;
            count = (w >> (pos & 7)) & 0xf;
        }
        if (count < 0 || count > 20)
        {
            OSReport((char*)&tbl[0][0x48], count);
        }
        i = 0;
        tbl2 = tbl[0] + 0xc;
        for (; i < count; i++)
        {
            int idx;
            {
                u32 w;
                int pos = bs->pos;
                int off = pos >> 3;
                u8* p = (u8*)(off + (char*)bs->data);
                w = p[0];
                w |= p[1] << 8;
                w |= p[2] << 16;
                bs->pos = pos + 8;
                idx = (w >> (pos & 7)) & 0xff;
            }
            if (lbl_803DCC48 == 2)
            {
                u8* pm = (u8*)(cache + idx * 0x30);
                u8* nm = pm + 0x12c0;
                GXLoadPosMtxImm((const f32 (*)[4])pm, *tbl[0]);
                if (skip == 0 && tex != 0)
                {
                    GXLoadTexMtxImm((const f32 (*)[4])nm, *tbl2, 0);
                }
                if (skip == 0 && nrm != 0)
                {
                    GXLoadNrmMtxImm((const f32 (*)[4])nm, *tbl[0]);
                }
            }
            else
            {
                PSMTXConcat((MtxPtr)mtx, (MtxPtr)(f32*)ObjModel_GetJointMatrix((u8*)model, idx), (MtxPtr)tmp);
                GXLoadPosMtxImm((const f32 (*)[4])tmp, *tbl[0]);
                if (skip == 0 && (nrm != 0 || tex != 0))
                {
                    tmp[3] = lbl_803DEA04;
                    tmp[7] = lbl_803DEA04;
                    tmp[11] = lbl_803DEA04;
                    PSMTXConcat((MtxPtr)tmp, (MtxPtr)m1, (MtxPtr)tmp);
                    if (tex != 0)
                    {
                        GXLoadTexMtxImm((const f32 (*)[4])tmp, *tbl2, 0);
                    }
                    if (nrm != 0)
                    {
                        GXLoadNrmMtxImm((const f32 (*)[4])tmp, *tbl[0]);
                    }
                }
            }
            tbl[0]++;
            tbl2++;
        }
    }
}

extern s32 lbl_803DCC48;
extern f32 lbl_803DEA04;
extern s32 gObjFuzzLayerIndex;
extern u8 lbl_803DCC3E;
extern u32 lbl_803DB468;
extern f32 lbl_803DEA28;
extern f32 lbl_803DEA1C;


extern f32 gObjPrintHalfPi;
extern f32 gObjPrintNegHalfPi;
extern const f32 gObjPrintAngleUnitScale;
extern const f32 gObjPrintTwoPi;

int objMatrixToRotation(f32* m, s16* outA, s16* outB, s16* outC);





void modelInitMtxs(ModelFileHeader* def, ObjModel* model);

#include "main/objprint_dolphin_internal.h"


extern u8 lbl_803DCC3D;
extern u32 lbl_803DB470;
extern int lbl_803DB498;
extern int lbl_803DB49C;
extern f32 lbl_803DEA38;

int modelRenderCb_8003c268(int obj, int* model, int ropIdx);

extern ObjPrintGXColor lbl_803DB494;
extern u8 lbl_803DCC35;
extern u8 lbl_803DCC36;
extern s32 lbl_803DCC5C;
extern u8 lbl_803DCC60;
extern int lbl_803DB48C;
extern int lbl_803DB490;


int shaderFuzzFn_8003cc1c(GameObject* obj, ObjModel* model, int ropIdx);

extern u8 gObjOverrideColor[3];
extern GXColor gObjCurChanColor;
extern f32 gObjShadowDist;
extern u8 gObjShadowNear;
extern s32 gObjFuzzStep;
extern f32 gObjFuzzPhase;
extern u8 lbl_803DCC34;
extern u32 gObjCachedModel;
extern u32 gObjCachedTexture;
extern u8 gObjRenderSetupDone;
extern u8 gObjRenderingShadowPass;
extern u8 gObjOverrideColorPending;
extern u32 curObjMtx;
extern u8 lbl_803DCC20;

extern u32 gObjGxDefaultChanColor;
extern u32 gObjGxVtxDescCache;
extern u8 gObjGxBlendModeCache;
extern u8 gObjGxZCompLocCache;
extern u32 gObjGxAlphaCompareCache;
extern u8 gObjGxZWriteCache;
extern u8 gObjGxZCompareCache;
extern u8 gObjGxCullModeCache;
extern u8 gObjGxKColorCache[4];
extern u8 gObjShadowColor[4];


void objRenderShadow2(int* obj, int* obj2, u8* m, int p4);
void modelDoRenderInstrs(int* obj, int* obj2, u8* m, u8 mode);
void objRenderChild(int* child, int* parent, u8 isShadow);

extern volatile int gAssetLoadInFlightFlags;
extern f32 lbl_803DEA4C;
extern f32 lbl_803DEA50;
extern f32 lbl_803DEA54;
extern f32 lbl_803DEA48;
extern s16 gDefragDelayFrames;
extern u32 gAssetLoadCompletedFlags;

int getLoadedFileFlags(int slot);
s32 mapCheckCurBlocks(int v);



#define OBJPRINT_MODEL_DEF(obj)         (((ObjAnimComponent*)(obj))->modelInstance)

void objRenderFuzzFn_8003d6f8(void* objArg);
extern f32 lbl_803DEA60;
extern f32 lbl_803DEA5C;
extern f32 lbl_803DEA64;
extern f32 lbl_803DEA68;
extern f32 lbl_803DEA6C;

#include "main/dll/ppcwgpipe_struct.h"


extern u8 gObjGxPosMtxIdTable[12];


void objFn_8003dc50(u8* obj, u8* model);
void modelLoadMtxsToGx(int obj, int* model, MtxBitStream* bs, f32* mtx);
void renderOpMatrix(u8* hdr, int* model, MtxBitStream* bs, f32* m1, f32* mtx, u8 nrm, u8 tex, u8 skip);
void ModelHeader_setupPosTexFmt(u8* hdr, int* model, MtxBitStream* bs, int p4)
{
    u32 flags = 0;
    if (hdr[0xf3] > 1)
    {
        flags |= 1;
    }
    {
        u32 w;
        int pos = bs->pos;
        int off = pos >> 3;
        u8* p;
        w = bs->data[off];
        p = (u8*)(off + (char*)bs->data);
        w |= p[1] << 8;
        w |= p[2] << 16;
        bs->pos = pos + 1;
        flags |= ((int)(w >> (pos & 7)) & 1) ? 2 : 0;
    }
    {
        u32 w;
        int pos = bs->pos;
        int off = pos >> 3;
        u8* p;
        w = bs->data[off];
        p = (u8*)(off + (char*)bs->data);
        w |= p[1] << 8;
        w |= p[2] << 16;
        bs->pos = pos + 1;
        flags |= ((int)(w >> (pos & 7)) & 1) ? 4 : 0;
    }
    if (gObjGxVtxDescCache != flags)
    {
        GXClearVtxDesc();
        if (flags & 1)
        {
            GXSetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
        }
        else
        {
            GXSetCurrentMtx(gObjGxPosMtxIdTable[0]);
        }
        GXSetVtxDesc(GX_VA_POS, (flags & 2) ? GX_INDEX16 : GX_INDEX8);
        GXSetVtxDesc(GX_VA_TEX0, (flags & 4) ? GX_INDEX16 : GX_INDEX8);
        gObjGxVtxDescCache = flags;
    }
}

void modelRenderFn_setVtxDescr(u8* hdr, u8* m, u32* p3, MtxBitStream* bs, u8 p5, u8* out1, u8* out2)
{
    int next;
    int back;
    GXClearVtxDesc();
    if (hdr[0xf3] > 1)
    {
        GXSetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
        next = 1;
        back = 8;
        if (p3[0] != 0 || p3[1] != 0)
        {
            if (*(u32*)&((ModelFileHeader*)m)->texCoords != 0)
            {
                GXSetVtxDesc(GX_VA_TEX0MTXIDX, GX_DIRECT);
                next = 3;
                GXSetVtxDesc(GX_VA_TEX1MTXIDX, GX_DIRECT);
            }
            GXSetVtxDesc(next++, GX_DIRECT);
        }
        {
            int i = 0;
            for (; i < ((ModelFileHeader*)hdr)->texMtxCount; i++)
            {
                u8 use;
                if (p5 == 4 && i == 0)
                {
                    int b;
                    int a;
                    if (lbl_803DCC5C != 0 &&
                        (modelLightStruct_getProjectionTevModes(lbl_803DCC64, &a, &b), a == 0))
                    {
                        use = 1;
                    }
                    else
                    {
                        use = 0;
                    }
                }
                else if (i < lbl_803DCC5C && p5 == 0)
                {
                    use = 1;
                }
                else
                {
                    use = 0;
                }
                if (use)
                {
                    GXSetVtxDesc(next++, GX_DIRECT);
                }
                else
                {
                    GXSetVtxDesc(back--, GX_DIRECT);
                }
            }
        }
        if (next > 1)
        {
            *out2 = 1;
        }
        else
        {
            *out2 = 0;
        }
    }
    else
    {
        GXSetCurrentMtx(0);
        *out2 = 1;
    }
    {
        u32 w;
        int pos = bs->pos;
        int off = pos >> 3;
        u8* p;
        w = bs->data[off];
        p = (u8*)(off + (char*)bs->data);
        w |= p[1] << 8;
        w |= p[2] << 16;
        bs->pos = pos + 1;
        GXSetVtxDesc(GX_VA_POS, (((int)(w >> (pos & 7)) & 1) ? GX_INDEX16 : GX_INDEX8));
    }
    if (m[0x40] & 1)
    {
        int b;
        {
            u32 w;
            int pos = bs->pos;
            int off = pos >> 3;
            u8* p;
            w = bs->data[off];
            p = (u8*)(off + (char*)bs->data);
            w |= p[1] << 8;
            w |= p[2] << 16;
            bs->pos = pos + 1;
            b = (w >> (pos & 7)) & 1;
        }
        if (hdr[0x24] & 8)
        {
            GXSetVtxDesc(GX_VA_NBT, b ? GX_INDEX16 : GX_INDEX8);
        }
        else
        {
            GXSetVtxDesc(GX_VA_NRM, b ? GX_INDEX16 : GX_INDEX8);
        }
        *out1 = 1;
    }
    else
    {
        *out1 = 0;
    }
    if (m[0x40] & 2)
    {
        u32 w;
        int pos = bs->pos;
        int off = pos >> 3;
        u8* p;
        w = bs->data[off];
        p = (u8*)(off + (char*)bs->data);
        w |= p[1] << 8;
        w |= p[2] << 16;
        bs->pos = pos + 1;
        GXSetVtxDesc(GX_VA_CLR0, (((int)(w >> (pos & 7)) & 1) ? GX_INDEX16 : GX_INDEX8));
    }
    {
        int b;
        int i;
        {
            u32 w;
            int pos = bs->pos;
            int off = pos >> 3;
            u8* p;
            w = bs->data[off];
            p = (u8*)(off + (char*)bs->data);
            w |= p[1] << 8;
            w |= p[2] << 16;
            bs->pos = pos + 1;
            b = (w >> (pos & 7)) & 1;
        }
        i = 0;
        for (; i < m[0x41]; i++)
        {
            GXSetVtxDesc(i + GX_VA_TEX0, b ? GX_INDEX16 : GX_INDEX8);
        }
    }
}
static inline void texSlotGetScroll(u8* obj, u32 jid, f32* txp, f32* typ)
{
    ObjTextureRuntimeSlot* slots = ((GameObject*)obj)->anim.textureSlots;
    ObjDef* modelDef = ((GameObject*)obj)->anim.modelInstance;
    ObjTextureSlotDef* q = modelDef->textureSlotDefs;
    int n = modelDef->textureSlotCount;
    int k;
    for (k = 0; k < n; k++)
    {
        if ((int)jid == q->materialIndex)
        {
            *txp = lbl_803DEA48 * slots[k].offsetS;
            *typ = lbl_803DEA48 * slots[k].offsetT;
            return;
        }
        q++;
    }
    *typ = *txp = lbl_803DEA04;
}
u8 modelRenderFn_8003e98c(u8* obj, u8* shader, u32* p3, int mask, int p5, int p6)
{
    u16 alpha;
    u8* colp;
    void* tex;
    u8* prev;
    u8* layer;
    u8 ok;
    int layerIdx;
    u8 color[4];
    f32 m[12];

    ok = 1;
    if (p3[0] != 0 || p3[1] != 0)
    {
        int i;
        u8 cnt;
        cnt = 0;
        for (i = 0; i < shader[0x41]; i++)
        {
            u8* l = Shader_getLayer(shader, i);
            if (l[4] & 0x80)
            {
                cnt++;
            }
        }
        if (cnt > 1)
        {
            ok = 0;
        }
    }
    layerIdx = 0;
    colp = &gObjCurChanColor.r;
    {
        for (; layerIdx < shader[0x41]; layerIdx++)
        {
            layer = Shader_getLayer(shader, layerIdx);
            if ((layer[4] & 0x80) == mask)
            {
                if ((((ObjModelRenderOp*)shader)->flags & SHADER_FLAG_DECAL_LAYER) && layerIdx == 1)
                {
                    u8 hasBaseTexture;
                    if (p3[0] != 0)
                    {
                        hasBaseTexture = 1;
                    }
                    else
                    {
                        hasBaseTexture = 0;
                    }
                    gxTextureFn_80050e28(hasBaseTexture);
                    return 1;
                }
                alpha = ((((GameObject*)obj)->anim.renderAlpha + 1) * shader[0xc]) >> 8;
                if (*(u32*)layer != 0)
                {
                    f32 (*mtxp)[4];
                    u8 fl;
                    tex = textureIdxToPtr(*(u32*)layer);
                    {
                        u32 jid = layer[5];
                        if (jid != 0)
                        {
                            ObjTextureRuntimeSlot* slots = ((GameObject*)obj)->anim.textureSlots;
                            ObjDef* modelDef = ((GameObject*)obj)->anim.modelInstance;
                            ObjTextureSlotDef* q = modelDef->textureSlotDefs;
                            int n = modelDef->textureSlotCount;
                            int k;
                            for (k = 0; k < n; k++)
                            {
                                if ((int)jid == q->materialIndex)
                                {
                                    tex = textureGetAnimationFrame(tex, slots[k].textureId);
                                    break;
                                }
                                q++;
                            }
                            {
                                f32 tx;
                                f32 ty;
                                texSlotGetScroll(obj, layer[5], &tx, &ty);
                                PSMTXTrans((MtxPtr)m, tx, ty, lbl_803DEA04);
                                mtxp = (f32 (*)[4])m;
                            }
                        }
                        else
                        {
                            mtxp = NULL;
                        }
                    }
                    if (layerIdx == 0)
                    {
                        if ((p3[0] != 0 || p3[1] != 0 || p6 != 0) && ok)
                        {
                            fl = 8;
                        }
                        else
                        {
                            fl = 0;
                        }
                        color[3] = alpha;
                    }
                    else
                    {
                        fl = prev[4] & 0x7f;
                        color[3] = 0xff;
                    }
                    color[0] = 0xff;
                    color[1] = 0xff;
                    color[2] = 0xff;
                    if (p3[0] != 0 || (shader[0] == 0xff && shader[1] == 0xff && shader[2] == 0xff))
                    {
                        addTexLayerStageSwizzled(tex, mtxp, (u8)fl, (GXColor*)color, *((u8*)p3 + 8), 1);
                    }
                    else if (p5 != 0)
                    {
                        colp[3] = color[3];
                        if (shader[0x40] & 0x10)
                        {
                            addTexLayerStageKColor(tex, mtxp, (u8)fl, &gObjCurChanColor);
                        }
                        else
                        {
                            addTexLayerStageSwizzled(tex, mtxp, (u8)fl, &gObjCurChanColor, *((u8*)p3 + 8), 1);
                        }
                    }
                    else
                    {
                        if (shader[0x40] & 0x10)
                        {
                            addTexLayerStage(tex, mtxp, (u8)fl);
                            if (color[3] < 0xff)
                            {
                                addKColorModulateStage((GXColor*)color);
                            }
                        }
                        else
                        {
                            addTexLayerStageKAlpha(tex, mtxp, (u8)fl, (GXColor*)color);
                        }
                    }
                }
                else
                {
                    color[0] = shader[4];
                    color[1] = shader[5];
                    color[2] = shader[6];
                    color[3] = alpha;
                    if (p3[0] != 0 || (shader[0] == 0xff && shader[1] == 0xff && shader[2] == 0xff))
                    {
                        addKColorModulateStage((GXColor*)color);
                    }
                    else if (p5 != 0)
                    {
                        colp[3] = alpha;
                        addKColorModulateStage(&gObjCurChanColor);
                    }
                    else
                    {
                        if (shader[0x40] & 0x10)
                        {
                            addVertexColorStage();
                            if (color[3] < 0xff)
                            {
                                addKColorModulateStage((GXColor*)color);
                            }
                        }
                        else
                        {
                            addVertexColorKAlphaStage((GXColor*)color);
                        }
                    }
                }
            }
            prev = layer;
        }
    }
    return ok;
}
u32 objRenderFn_8003edf4(u8* obj, u8* p2, int* am, MtxBitStream* bs)
{
    int* op;
    u32* refs;
    u32 idx;
    u8 shad;
    ModelLightStruct** lp;
    int envtex;
    int nlay;
    u8* sp;
    int i;
    ObjModelRenderCb cb;
    f32 m2[12];
    f32 t2[12];
    f32 wm[12];
    f32 t1[12];
    int a;
    int b;
    u8 color[4];
    u8 fogc[4];

    shad = 0;
    {
        u32 w;
        int pos = bs->pos;
        int off = pos >> 3;
        u8* p;
        w = bs->data[off];
        p = (u8*)(off + (char*)bs->data);
        w |= p[1] << 8;
        w |= p[2] << 16;
        bs->pos = pos + 6;
        idx = (w >> (pos & 7)) & 0x3f;
    }
    cb = (ObjModelRenderCb)ObjModel_GetRenderCallback((ObjModel*)am);
    if (cb != NULL && cb((int*)obj, am, idx) != 0)
    {
        return idx;
    }
    op = (int*)ObjModel_GetRenderOp((ModelFileHeader*)*am, idx);
    refs = (u32*)ObjModel_GetRenderOpTextureRefs((ObjModel*)am, idx);
    Rcp_ResetTextureStageState();
    envtex = 0;
    if ((refs[0] != 0 || refs[1] != 0) && ((ObjModelRenderOp*)op)->envTextureId != 0)
    {
        void* t = textureIdxToPtr(((ObjModelRenderOp*)op)->envTextureId);
        int nl = lbl_803DCC5C + 1;
        if (refs[0] != 0)
        {
            nl += 1;
        }
        if (refs[1] != 0)
        {
            nl += 1;
        }
        envtex = textureFn_80050ad8(t, nl, ((u8*)op)[0x42], ((ObjModelRenderOp*)op)->indirectTextureId);
        envtex &= 0xff;
    }
    if (refs[0] != 0)
    {
        textureFn_80051348((void*)refs[0], ((GameObject*)obj)->unkF1);
    }
    if (refs[1] != 0)
    {
        if (((ObjModelRenderOp*)op)->unk1C != 0)
        {
            color[0] = 0xff;
            color[1] = 0xff;
            color[2] = 0xff;
            color[3] = ((u8*)op)[0x22];
        }
        else
        {
            color[3] = 0;
        }
        GXSetTevColor(GX_TEVREG2, *(GXColor*)color);
        {
            u8 hasBaseTexture;
            if (refs[0] != 0)
            {
                hasBaseTexture = 1;
            }
            else
            {
                hasBaseTexture = 0;
            }
            addLightTexReg2Stage((void*)refs[1], hasBaseTexture, ((u8*)op)[0x20]);
        }
        if (color[3] != 0)
        {
            u8 hasBaseTexture;
            if (refs[0] != 0)
            {
                hasBaseTexture = 1;
            }
            else
            {
                hasBaseTexture = 0;
            }
            addAlphaLitColorReg2Stage(hasBaseTexture);
        }
    }
    else
    {
        GXSetTevColor(GX_TEVREG2, *(GXColor*)&gObjGxDefaultChanColor);
    }
    nlay = lbl_803DCC5C;
    if (gObjShadowNear != 0)
    {
        addShadowFalloffTevStages();
        shad = 1;
        nlay = 0;
    }
    else
    {
        int b4;
        f32* mx;
        u8 b5f = OBJPRINT_MODEL_DEF(obj)->renderFlags;
        b4 = b5f & 4;
        if (b4 && (mx = (f32*)((GameObject*)obj)->anim.modelState->shadowCastSlot) != NULL)
        {
            addCastShadowTevStages((u8*)mx);
            nlay = 0;
        }
        else if (b5f & 0x10)
        {
            addWavyCausticTevStage();
            nlay = 0;
        }
        else if (b4 == 0)
        {
            i = 0;
            lp = &lbl_803DCC64;
            sp = &lbl_803DCC60;
            for (; i < lbl_803DCC5C; i++)
            {
                u8* t = (u8*)modelLightStruct_getProjectionTexture(*lp);
                if (t != 0)
                {
                    modelLightStruct_getProjectionTevModes(*lp, &a, &b);
                    if (a == 2)
                    {
                        shad = 1;
                    }
                    {
                        f32* mtx = modelLightStruct_getProjectionTexMtx(*lp);
                        addProjectedLightTevStage(t, mtx, a, b, *sp);
                    }
                }
                lp++;
                sp++;
            }
        }
    }
    if (envtex != 0)
    {
        addEnvMapTexCoord(envtex);
    }
    {
        u32 t18;
        if ((t18 = ((ObjModelRenderOp*)op)->textureId) != 0 && ((ObjModelRenderOp*)op)->unk1C == 0 && refs[1] != 0)
        {
            textureIdxToPtr(t18);
            addTexModulateReg2Stage();
        }
    }
    {
        u8 hl;
        if (modelRenderFn_8003e98c(obj, (u8*)op, refs, 0x80, hl = ((((ModelFileHeader*)p2)->shaderFlags & 2) && !(p2[0x24] & 2)),
                                   nlay) == 0)
        {
            u8 hasBaseTexture;
            if (refs[0] != 0)
            {
                hasBaseTexture = 1;
            }
            else
            {
                hasBaseTexture = 0;
            }
            gxTextureFn_80050e28(hasBaseTexture);
        }
        if (((ObjModelRenderOp*)op)->flags & SHADER_FLAG_DECAL_LAYER)
        {
            u8* l1 = Shader_getLayer((u8*)op, 1);
            {
                f32 tx;
                f32 ty;
                texSlotGetScroll(obj, l1[5], &tx, &ty);
                PSMTXTrans((MtxPtr)m2, tx, ty, lbl_803DEA04);
            }
            textureFn_8004c330(textureIdxToPtr(*(u32*)l1), m2);
        }
        modelRenderFn_8003e98c(obj, (u8*)op, refs, 0, hl, nlay);
    }
    if (isHeavyFogEnabled() && !(((ModelFileHeader*)p2)->flags & 0x100))
    {
        getColor803dd01c(fogc);
        renderHeavyFog(fogc);
    }
    if (((ObjModelRenderOp*)op)->flags & SHADER_FLAG_PROJECTED_TEX_PASS)
    {
        f32* vm = Camera_GetViewMatrix();
        Obj_BuildWorldTransformMatrix((GameObject*)obj, wm, 0);
        PSMTXConcat((MtxPtr)vm, (MtxPtr)wm, (MtxPtr)t1);
        PSMTXConcat((MtxPtr)(f32*)lbl_803967F0, (MtxPtr)t1, (MtxPtr)t2);
        GXLoadTexMtxImm((const f32 (*)[4])t2, 0x24, 0);
        addSmallReflectionTevStage();
    }
    if (OBJPRINT_MODEL_DEF(obj)->renderFlags & OBJDEF_RENDERFLAG_DEFERRED_RENDER)
    {
        gxTextureFn_8004d5b4(op);
    }
    {
        u8 e5 = ((GameObject*)obj)->colorFadeFlags;
        if ((e5 & OBJ_COLOR_FADE_FLAG_ACTIVE) || (e5 & OBJ_COLOR_FADE_FLAG_OVERRIDE))
        {
            color[0] = ((GameObject*)obj)->colorFadeRed;
            color[1] = ((GameObject*)obj)->colorFadeGreen;
            color[2] = ((GameObject*)obj)->colorFadeBlue;
            color[3] = ((GameObject*)obj)->colorFadeAlpha;
            addColorFadeStage((GXColor*)color);
        }
    }
    if (((ObjModelRenderOp*)op)->flags & SHADER_FLAG_WATER_CAUSTIC)
    {
        AttractMovie_AddVideoTevStages();
    }
    Rcp_ApplyTextureStageCounts();
    {
        ObjModelRenderCb pcb = (ObjModelRenderCb)ObjModel_GetPostRenderCallback((ObjModel*)am);
        if (pcb != NULL)
        {
            pcb((int*)obj, am, idx);
        }
        else
        {
            u8 zon = 1;
            if (((GameObject*)obj)->anim.renderAlpha < 0xff || (((ObjModelRenderOp*)op)->flags & SHADER_FLAG_FORCE_BLEND) || shad)
            {
                u16 f2;
                GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
                f2 = ((ModelFileHeader*)p2)->flags;
                if (f2 & 0x400)
                {
                    gxSetZMode_(0, 3, 0);
                    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
                }
                else if (f2 & 0x2000)
                {
                    zon = 0;
                    gxSetZMode_(1, 3, 1);
                    GXSetAlphaCompare(GX_GREATER, gObjAlphaCompareThreshold, GX_AOP_AND, GX_GREATER, gObjAlphaCompareThreshold);
                }
                else
                {
                    gxSetZMode_(1, 3, 0);
                    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
                }
            }
            else if (((ObjModelRenderOp*)op)->flags & SHADER_FLAG_ALPHA_TEST_OPAQUE)
            {
                GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
                if (((ModelFileHeader*)p2)->flags & 0x400)
                {
                    gxSetZMode_(0, 3, 0);
                }
                else
                {
                    gxSetZMode_(1, 3, 1);
                }
                GXSetAlphaCompare(GX_GREATER, 0x40, GX_AOP_AND, GX_GREATER, 0x40);
            }
            else
            {
                GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
                if (((ModelFileHeader*)p2)->flags & 0x400)
                {
                    gxSetZMode_(0, 3, 0);
                }
                else
                {
                    gxSetZMode_(1, 3, 1);
                }
                GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
            }
            if (((ObjModelRenderOp*)op)->flags & SHADER_FLAG_ALPHA_TEST_OPAQUE)
            {
                zon = 0;
            }
            gxSetPeControl_ZCompLoc_(zon);
        }
    }
    if (((ObjModelRenderOp*)op)->flags & 8)
    {
        GXSetCullMode(GX_CULL_BACK);
    }
    else
    {
        GXSetCullMode(GX_CULL_NONE);
    }
    return idx;
}
void shaderSetGxFlags(u8* obj, u8* m, u8* shader)
{
    u8 blend;
    u8 zwrite;
    u8 zcmp;
    u8 zcomploc;
    u32 alpha;
    u8 cull;
    u32 sf;
    if (((GameObject*)obj)->anim.renderAlpha < 0xff || ((sf = ((ObjModelRenderOp*)shader)->flags) & SHADER_FLAG_FORCE_BLEND))
    {
        blend = 1;
        if (((ModelFileHeader*)m)->flags & 0x400)
        {
            zwrite = 0;
            zcmp = 0;
            zcomploc = 1;
            alpha = 0;
        }
        else if (((ModelFileHeader*)m)->flags & 0x2000)
        {
            zwrite = 1;
            zcmp = 1;
            zcomploc = 0;
            alpha = 0xdf;
        }
        else
        {
            zwrite = 1;
            zcmp = 0;
            zcomploc = 1;
            alpha = 0;
        }
    }
    else if (sf & SHADER_FLAG_ALPHA_TEST_OPAQUE)
    {
        blend = 0;
        if (((ModelFileHeader*)m)->flags & 0x400)
        {
            zwrite = 0;
            zcmp = 0;
        }
        else
        {
            zwrite = 1;
            zcmp = 1;
        }
        zcomploc = 0;
        alpha = 0x40;
    }
    else
    {
        blend = 0;
        if (((ModelFileHeader*)m)->flags & 0x400)
        {
            zwrite = 0;
            zcmp = 0;
        }
        else
        {
            zwrite = 1;
            zcmp = 1;
        }
        zcomploc = 1;
        alpha = 0;
    }
    if (((ObjModelRenderOp*)shader)->flags & SHADER_FLAG_BACKFACE_CULL)
    {
        cull = 1;
    }
    else
    {
        cull = 0;
    }
    if (gObjGxBlendModeCache != blend)
    {
        if (blend != 0)
        {
            GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        }
        else
        {
            GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
        }
        gObjGxBlendModeCache = blend;
    }
    if (gObjGxZWriteCache != zwrite || gObjGxZCompareCache != zcmp)
    {
        gxSetZMode_(zwrite, 3, zcmp);
        gObjGxZWriteCache = zwrite;
        gObjGxZCompareCache = zcmp;
    }
    if (gObjGxZCompLocCache != zcomploc)
    {
        gxSetPeControl_ZCompLoc_(zcomploc);
        gObjGxZCompLocCache = zcomploc;
    }
    if (gObjGxAlphaCompareCache != alpha)
    {
        gObjGxAlphaCompareCache = alpha;
        if (alpha != 0)
        {
            GXSetAlphaCompare(GX_GREATER, (u8)alpha, GX_AOP_AND, GX_GREATER, (u8)alpha);
        }
        else
        {
            GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
        }
    }
    if (cull != gObjGxCullModeCache)
    {
        gObjGxCullModeCache = cull;
        if (cull != 0)
        {
            GXSetCullMode(GX_CULL_BACK);
        }
        else
        {
            GXSetCullMode(GX_CULL_NONE);
        }
    }
}

extern f32 gObjJointMtxTemp[];
void modelDoAltRenderInstrs(int* obj, int* obj2, u8* m, int p4)
{
    f32 wm[16];
    f32 cm[12];
    MtxBitStream bs;
    u8 color[4];
    ObjModelRenderCb cb;
    int* am = (int*)Obj_GetActiveModel((GameObject*)obj);
    if (curObjMtx != 0)
    {
        PSMTXCopy((MtxPtr)(f32*)curObjMtx, (MtxPtr)wm);
        curObjMtx = 0;
    }
    else
    {
        Obj_BuildWorldTransformMatrix((GameObject*)obj, wm, 0);
    }
    PSMTXConcat((MtxPtr)Camera_GetViewMatrix(), (MtxPtr)wm, (MtxPtr)cm);
    if (!(((ObjModel*)am)->bufferFlags & 8))
    {
        ((ObjDef*)am)->hitboxStateIndex = 0;
        if (((ModelFileHeader*)m)->animationCount != 0 && !(((ModelFileHeader*)m)->flags & 2) &&
            ((ModelFileHeader*)m)->jointCount != 0)
        {
            if (gObjCachedModel != (u32)m)
            {
                ObjModel_UpdateAnimMatrices((ObjModel*)am, (ModelFileHeader*)m, (GameObject*)obj, gObjJointMtxTemp);
                modelInitMtxs((ModelFileHeader*)m, (ObjModel*)am);
            }
            else
            {
                lbl_803DCC48 = 1;
            }
        }
        else
        {
            ObjModel_ToggleMatrixBuffer((ObjModel*)am);
            PSMTXCopy((MtxPtr)gObjJointMtxTemp, (MtxPtr)(f32*)ObjModel_GetJointMatrix((u8*)am, 0));
            lbl_803DCC48 = 3;
        }
        {
            u8* att = *(u8**)&((GameObject*)obj)->anim.hitReactState;
            if (att != NULL)
            {
                att[0xaf]--;
                if (*(s8*)(*(char**)&((GameObject*)obj)->anim.hitReactState + 0xaf) < 0)
                {
                    *(u8*)(*(char**)&((GameObject*)obj)->anim.hitReactState + 0xaf) = 0;
                }
            }
        }
        ((ObjModel*)am)->bufferFlags |= 8;
    }
    modelRenderInstrsState_init((ModelRenderInstrsState*)&bs, ((ModelFileHeader*)m)->instrs,
                                ((ModelFileHeader*)m)->instrsBitLenWords << 3,
                                ((ModelFileHeader*)m)->instrsBitLenWords << 3);
    if (((ModelFileHeader*)m)->shaderFlags & MODEL_SHADERFLAGS_USE_OBJ_COLOR)
    {
        if (gObjOverrideColorPending != 0)
        {
            color[0] = gObjOverrideColor[0];
            color[1] = gObjOverrideColor[1];
            color[2] = gObjOverrideColor[2];
            gObjOverrideColorPending = 0;
        }
        else
        {
            objGetColor(((GameObject*)obj)->lightColorSlot, &color[0], &color[1], &color[2]);
        }
    }
    else
    {
        color[2] = 0xff;
        color[1] = 0xff;
        color[0] = 0xff;
    }
    color[3] = ((GameObject*)obj)->anim.renderAlpha;
    cb = (ObjModelRenderCb)ObjModel_GetRenderCallback((ObjModel*)am);
    if (gObjRenderSetupDone == 0 || cb != NULL)
    {
        Camera_RebuildProjectionMatrix();
        if (cb == NULL || cb(obj, am, 0) == 0)
        {
            _gxSetFogParams();
            Rcp_ResetTextureStageState();
            addTexLayerStageSwizzled(textureIdxToPtr(*(int*)(*(int*)&((ModelFileHeader*)m)->renderOps + 0x24)), NULL,
                          0, (GXColor*)color, 0, 0);
            if (isHeavyFogEnabled() != 0)
            {
                u8 c[4];
                getColor803dd01c(c);
                renderHeavyFog(c);
            }
            Rcp_ApplyTextureStageCounts();
            GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
            GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
            GXSetNumChans(0);
            gObjRenderSetupDone = 1;
            *(u32*)gObjGxKColorCache = *(u32*)color;
        }
    }
    else
    {
        void* tex = textureIdxToPtr(*(int*)(*(int*)&((ModelFileHeader*)m)->renderOps + 0x24));
        if (gObjCachedTexture != (u32)tex)
        {
            gObjCachedTexture = (u32)tex;
            selectTexture((Texture*)tex, 0);
        }
        if (gObjGxKColorCache[0] != color[0] || gObjGxKColorCache[1] != color[1] || gObjGxKColorCache[2] != color[2] ||
            gObjGxKColorCache[3] != color[3])
        {
            GXSetTevKColor(GX_KCOLOR0, *(GXColor*)color);
            *(u32*)gObjGxKColorCache = *(u32*)color;
        }
    }
    if (gObjCachedModel != (u32)m)
    {
        GXSetArray(GX_VA_POS,
                   (void*)((int*)((char*)am + 0x1c))[(((ObjModel*)am)->bufferFlags >> 1) & 1], 6);
        GXSetArray(GX_VA_TEX0, ((ModelFileHeader*)m)->texCoords, 4);
        gObjCachedModel = (u32)m;
    }
    shaderSetGxFlags((u8*)obj, m, (u8*)((ModelFileHeader*)m)->renderOps);
    bs.pos += 4;
    ModelHeader_setupPosTexFmt(m, (void*)((ModelFileHeader*)m)->renderOps, &bs, p4);
    bs.pos += 4;
    modelLoadMtxsToGx((int)m, am, &bs, cm);
    {
        u8* dl;
        int idx;
        {
            u32 w;
            int pos = (bs.pos += 4);
            int off = pos >> 3;
            u8* p;
            w = bs.data[off];
            p = (u8*)(off + (char*)bs.data);
            w |= p[1] << 8;
            w |= p[2] << 16;
            bs.pos = pos + 8;
            idx = (w >> (pos & 7)) & 0xff;
        }
        dl = modelFileGetDisplayList(m, idx);
        GXCallDisplayList(*(void**)dl, *(u16*)(dl + 4));
    }
}


void renderResetFn_8003fc60(void)
{
    gObjRenderSetupDone = 0;
    gObjCachedTexture = 0;
    gObjCachedModel = 0;
    lbl_803DCC34 = 0;
    gObjGxVtxDescCache = -1;
    gObjGxBlendModeCache = 0xff;
    gObjGxZCompLocCache = 0xff;
    gObjGxAlphaCompareCache = -1;
    gObjGxZWriteCache = 0xff;
    gObjGxZCompareCache = 0xff;
    gObjGxCullModeCache = 0xff;
    gObjGxKColorCache[3] = 0;
    gObjGxKColorCache[2] = 0;
    gObjGxKColorCache[1] = 0;
    gObjGxKColorCache[0] = 0;
}
typedef void (*ObjShadowCb)(int* obj, int* am, f32* wm);

extern f32 gObjBoneMtxBuffer[0xC00];



void objRenderShadow2(int* obj, int* obj2, u8* m, int p4)
{
    int done;
    f32 cm[16];
    f32 wm[16];
    f32 im[16];
    MtxBitStream bs;
    u8 color[4];
    int* am;
    f32* vm;
    u8 did;
    int* op;
    u32 sh;

    am = (int*)Obj_GetActiveModel((GameObject*)obj);
    vm = Camera_GetViewMatrix();
    if (curObjMtx != 0)
    {
        PSMTXCopy((MtxPtr)(f32*)curObjMtx, (MtxPtr)wm);
        curObjMtx = 0;
    }
    else
    {
        Obj_BuildWorldTransformMatrix((GameObject*)obj, wm, 0);
    }
    if (!(((ObjModel*)am)->bufferFlags & 8))
    {
        did = 0;
        ((ObjModel*)am)->vtxBufDirty = 0;
        ObjModel_ToggleVertexBuffer((ObjModel*)am);
        if (((ModelFileHeader*)m)->animationCount != 0 && !(((ModelFileHeader*)m)->flags & 2) &&
            ((ModelFileHeader*)m)->jointCount != 0)
        {
            if (*(u32*)&((ModelFileHeader*)m)->vertexAnimEntries != 0)
            {
                PSMTXIdentity((MtxPtr)im);
                ObjModel_UpdateAnimMatrices((ObjModel*)am, (ModelFileHeader*)m, (GameObject*)obj, im);
                modelInitBoneMtxs2((ObjModel*)am, wm, gObjBoneMtxBuffer);
                did = 1;
            }
            else
            {
                ObjModel_UpdateAnimMatrices((ObjModel*)am, (ModelFileHeader*)m, (GameObject*)obj, wm);
            }
            {
                ObjShadowCb cb = *(ObjShadowCb*)((char*)obj + 0x108);
                if (cb != NULL && obj2 == obj)
                {
                    cb(obj, am, wm);
                }
            }
        }
        else
        {
            ObjModel_ToggleMatrixBuffer((ObjModel*)am);
            PSMTXCopy((MtxPtr)wm, (MtxPtr)(f32*)ObjModel_GetJointMatrix((u8*)am, 0));
        }
        if (((ModelFileHeader*)m)->morphTargetCount != 0)
        {
            ObjModel_ApplyBlendChannels((ObjModel*)am);
        }
        if (did != 0)
        {
            u8* vtx;
            if (((ObjModel*)am)->vtxBufDirty != 0)
            {
                vtx = (u8*)((int*)((char*)am + 0x1c))[(((ObjModel*)am)->bufferFlags >> 1) & 1];
            }
            else
            {
                vtx = *(u8**)&((ModelFileHeader*)m)->vertices;
            }
            ObjModel_BlendVertexStream(
                (u8*)gObjBoneMtxBuffer, m + 0x88, vtx,
                (int*)*(int*)&((ModelFileHeader*)am)->jointBlendData,
                (u8*)((int*)((char*)am + 0x1c))[(((ObjModel*)am)->bufferFlags >> 1) & 1]);
            ObjModel_BlendNormalStream((u8*)gObjBoneMtxBuffer, m + 0xac,
                                       (u8*)*(int*)&((ModelFileHeader*)m)->normals,
                                       (u8**)((ObjModel*)am)->blendAnimData,
                                       ((ModelFileHeader*)m)->flags24 & 8);
        }
        if (((ModelFileHeader*)m)->hitSphereCount != 0)
        {
            objUpdateHitSpheres((u8*)am, m, (u8*)obj, NULL, (u8*)obj2);
        }
        else
        {
            u8* att = *(u8**)&((GameObject*)obj)->anim.hitReactState;
            if (att != NULL)
            {
                att[0xaf]--;
                if (*(s8*)(*(char**)&((GameObject*)obj)->anim.hitReactState + 0xaf) < 0)
                {
                    *(u8*)(*(char**)&((GameObject*)obj)->anim.hitReactState + 0xaf) = 0;
                }
            }
        }
        ((ObjModel*)am)->bufferFlags |= 8;
    }
    modelInitMtxs((ModelFileHeader*)m, (ObjModel*)am);
    modelRenderInstrsState_init((ModelRenderInstrsState*)&bs, ((ModelFileHeader*)m)->instrs,
                                ((ModelFileHeader*)m)->instrsBitLenWords << 3,
                                ((ModelFileHeader*)m)->instrsBitLenWords << 3);
    if (*(u32*)&((ModelFileHeader*)m)->vertexAnimEntries != 0)
    {
        PSMTXConcat((MtxPtr)vm, (MtxPtr)wm, (MtxPtr)cm);
        GXLoadPosMtxImm((const f32 (*)[4])cm, gObjGxPosMtxIdTable[9]);
    }
    {
        u8* o;
        u8* nxt;
        o = (u8*)obj;
        while ((nxt = *(u8**)&((GameObject*)o)->ownerObj) != NULL)
        {
            o = nxt;
        }
        sh = ((u8*)((GameObject*)o)->anim.modelState->shadowCastSlot)[0x65];
        if (sh == 0xff)
        {
            GXSetTevColor(GX_TEVREG2, *(GXColor*)&lbl_803DB468);
            GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
        }
        else
        {
            if (sh < 8)
            {
                color[0] = 1 << sh;
                color[1] = 0;
                color[2] = 0;
            }
            else
            {
                color[0] = 0;
                color[1] = 1 << (sh - 8);
                color[2] = 0;
            }
            color[3] = 0xff;
            GXSetTevColor(GX_TEVREG2, *(GXColor*)color);
            GXSetBlendMode(GX_BM_LOGIC, GX_BL_ONE, GX_BL_ZERO, GX_LO_OR);
        }
    }
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetNumIndStages(0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C2);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A2);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetFog(GX_FOG_NONE, 0.0f, 0.0f, 0.0f, 0.0f, *(GXColor*)&lbl_803DB468);
    gxSetPeControl_ZCompLoc_(1);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(1);
    if (OBJPRINT_MODEL_DEF(obj)->renderFlags & OBJDEF_RENDERFLAG_PROJECTED_SHADOW)
    {
        gxSetZMode_(1, 3, 1);
        GXSetCullMode(GX_CULL_FRONT);
    }
    else
    {
        gxSetZMode_(0, 3, 0);
        GXSetCullMode(GX_CULL_NONE);
    }
    GXSetArray(GX_VA_POS,
               (void*)((int*)((char*)am + 0x1c))[(((ObjModel*)am)->bufferFlags >> 1) & 1], 6);
    done = 0;
    while (!done)
    {
        u32 op4;
        {
            u32 w;
            int pos = bs.pos;
            u8* p = bs.data + (pos >> 3);
            w = p[0];
            w |= p[1] << 8;
            w |= p[2] << 16;
            bs.pos = pos + 4;
            op4 = (w >> (pos & 7)) & 0xf;
        }
        switch (op4)
        {
        case 3:
            GXClearVtxDesc();
            if (((ModelFileHeader*)m)->jointCount > 1)
            {
                GXSetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
            }
            {
                u32 w;
                int pos = bs.pos;
                u8* p = bs.data + (pos >> 3);
                w = p[0];
                w |= p[1] << 8;
                w |= p[2] << 16;
                bs.pos = pos + 1;
                GXSetVtxDesc(GX_VA_POS, (((int)(w >> (pos & 7)) & 1) ? GX_INDEX16 : GX_INDEX8));
            }
            if (((u8*)op)[0x40] & 1)
            {
                bs.pos += 1;
            }
            if (((u8*)op)[0x40] & 2)
            {
                bs.pos += 1;
            }
            GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
            bs.pos += 1;
            break;
        case 1:
        {
            u32 w;
            int pos = bs.pos;
            u8* p = bs.data + (pos >> 3);
            w = p[0];
            w |= p[1] << 8;
            w |= p[2] << 16;
            bs.pos = pos + 6;
            op = (int*)ObjModel_GetRenderOp((ModelFileHeader*)m, (w >> (pos & 7)) & 0x3f);
        }
        break;
        case 2:
        {
            u8* dl;
            u32 w;
            int pos = bs.pos;
            u8* p = (u8*)((pos >> 3) + bs.data);
            w = p[0];
            w |= p[1] << 8;
            w |= p[2] << 16;
            bs.pos = pos + 8;
            dl = modelFileGetDisplayList(
                m, ((ModelFileHeader*)m)->displayListCount + ((w >> (pos & 7)) & 0xff));
            GXCallDisplayList(*(void**)dl, *(u16*)(dl + 4));
        }
        break;
        case 4:
            modelLoadMtxsToGx((int)m, am, &bs, vm);
            break;
        case 5:
            done = 1;
            break;
        }
    }
}
extern u8 gObjGxTexMtxIdTable[12];

void modelDoRenderInstrs(int* obj, int* obj2, u8* m, u8 mode)
{
    int joff;
    f32 fm[16];
    f32 sm[16];
    f32 wm[16];
    f32 im[16];
    f32 tm[12];
    f32 t2m[12];
    MtxBitStream bs;
    u8 color[4];
    u8 o9;
    u8 o8;
    int* am;
    f32* vm;
    int mode8;
    int m4;
    int m2;
    int m1;
    u8 did;
    int* op;
    u32* refs;
    int done;
    f32 fade;

    gObjRenderSetupDone = 0;
    gObjCachedTexture = 0;
    gObjCachedModel = 0;
    lbl_803DCC34 = 0;
    gObjGxVtxDescCache = -1;
    gObjGxBlendModeCache = 0xff;
    gObjGxZCompLocCache = 0xff;
    gObjGxAlphaCompareCache = -1;
    gObjGxZWriteCache = 0xff;
    gObjGxZCompareCache = 0xff;
    gObjGxCullModeCache = 0xff;
    gObjGxKColorCache[3] = 0;
    gObjGxKColorCache[2] = 0;
    gObjGxKColorCache[1] = 0;
    gObjGxKColorCache[0] = 0;
    am = (int*)Obj_GetActiveModel((GameObject*)obj);
    vm = Camera_GetViewMatrix();
    if (curObjMtx != 0)
    {
        PSMTXCopy((MtxPtr)(f32*)curObjMtx, (MtxPtr)wm);
        curObjMtx = 0;
    }
    else
    {
        Obj_BuildWorldTransformMatrix((GameObject*)obj, wm, 0);
    }
    gObjShadowNear = 0;
    if (((ObjAnimComponent*)obj)->modelInstance->flags & 0x400)
    {
        GameObject* player = Obj_GetPlayerObject();
        int* cam = (int*)(*gCameraInterface)->getCamera();
        if (player != NULL && !(player->objectFlags & OBJECT_OBJFLAG_PARENT_SLACK) &&
            ((GameObject*)cam)->anim.targetObj == player)
        {
            f32 d = lbl_803DEA38 + (((GameObject*)obj)->anim.hitboxScale * ((GameObject*)obj)->anim.rootMotionScale +
                                    *(f32*)&((GameObject*)obj)->anim.targetObj);
            f32 dist = Camera_DistanceToCurrentViewPosition(player->anim.worldPosX, player->anim.worldPosY,
                                                            player->anim.worldPosZ);
            if (d > -dist)
            {
                gObjShadowNear = 1;
                gObjShadowDist = dist;
            }
        }
    }
    if (gObjOverrideColorPending != 0)
    {
        gObjCurChanColor.r = gObjOverrideColor[0];
        gObjCurChanColor.g = gObjOverrideColor[1];
        gObjCurChanColor.b = gObjOverrideColor[2];
        gObjOverrideColorPending = 0;
    }
    else
    {
        objGetColor(((GameObject*)obj)->lightColorSlot, &gObjCurChanColor.r, &gObjCurChanColor.g,
                    &gObjCurChanColor.b);
    }
    mode8 = mode;
    m4 = mode8 & 4;
    if (m4 || (mode8 & 8))
    {
        fade = lbl_803DEA4C;
    }
    else if (mode8 & 2)
    {
        fade = lbl_803DEA50;
    }
    did = 0;
    if (!(((ObjModel*)am)->bufferFlags & 8))
    {
        ((ObjModel*)am)->vtxBufDirty = 0;
        ObjModel_ToggleVertexBuffer((ObjModel*)am);
        if (((ModelFileHeader*)m)->animationCount != 0 && !(((ModelFileHeader*)m)->flags & 2) &&
            ((ModelFileHeader*)m)->jointCount != 0)
        {
            if (*(u32*)&((ModelFileHeader*)m)->vertexAnimEntries != 0)
            {
                PSMTXIdentity((MtxPtr)im);
                ObjModel_UpdateAnimMatrices((ObjModel*)am, (ModelFileHeader*)m, (GameObject*)obj, im);
                if (m4 == 0)
                {
                    modelInitBoneMtxs2((ObjModel*)am, wm, gObjBoneMtxBuffer);
                }
                else
                {
                    modelInitBoneMtxs((ObjModel*)am, gObjBoneMtxBuffer);
                }
                did = 1;
            }
            else
            {
                ObjModel_UpdateAnimMatrices((ObjModel*)am, (ModelFileHeader*)m, (GameObject*)obj, wm);
            }
            {
                ObjShadowCb cb = *(ObjShadowCb*)((char*)obj + 0x108);
                if (cb != NULL && obj2 == obj)
                {
                    cb(obj, am, wm);
                }
            }
        }
        else
        {
            ObjModel_ToggleMatrixBuffer((ObjModel*)am);
            PSMTXCopy((MtxPtr)wm, (MtxPtr)(f32*)ObjModel_GetJointMatrix((u8*)am, 0));
        }
        if ((m4 == 0 && (mode8 & 8) == 0) || gObjFuzzLayerIndex == 0)
        {
            if (((ModelFileHeader*)m)->morphTargetCount != 0)
            {
                ObjModel_ApplyBlendChannels((ObjModel*)am);
            }
            if (did != 0)
            {
                int vtx;
                vtx = ((ObjModel*)am)->vtxBufDirty != 0
                    ? ((int*)((char*)am + 0x1c))[(((ObjModel*)am)->bufferFlags >> 1) & 1]
                    : *(int*)&((ModelFileHeader*)m)->vertices;
                ObjModel_BlendVertexStream(
                    (u8*)gObjBoneMtxBuffer, m + 0x88, (u8*)vtx,
                    (int*)*(int*)&((ModelFileHeader*)am)->jointBlendData,
                    (u8*)((int*)((char*)am + 0x1c))[(((ObjModel*)am)->bufferFlags >> 1) & 1]);
                ObjModel_BlendNormalStream((u8*)gObjBoneMtxBuffer, m + 0xac,
                                           (u8*)*(int*)&((ModelFileHeader*)m)->normals,
                                           (u8**)((ObjModel*)am)->blendAnimData,
                                           ((ModelFileHeader*)m)->flags24 & 8);
            }
        }
        if (((ModelFileHeader*)m)->hitSphereCount != 0)
        {
            objUpdateHitSpheres((u8*)am, m, (u8*)obj, NULL, (u8*)obj2);
        }
        else
        {
            u8* att = *(u8**)&((GameObject*)obj)->anim.hitReactState;
            if (att != NULL)
            {
                att[0xaf]--;
                if (*(s8*)(*(char**)&((GameObject*)obj)->anim.hitReactState + 0xaf) < 0)
                {
                    *(u8*)(*(char**)&((GameObject*)obj)->anim.hitReactState + 0xaf) = 0;
                }
            }
        }
        ((ObjModel*)am)->bufferFlags |= 8;
    }
    m2 = mode8 & 2;
    if (m2 || m4 || (mode8 & 8))
    {
        int j;
        f32 one;
        j = 0;
        joff = 0;
        one = lbl_803DEA1C;
        for (; j < ((ModelFileHeader*)m)->jointCount; j++)
        {
            f32 sc =
                (f32)gObjFuzzStep * (fade / *(f32*)(((ModelFileHeader*)m)->jointBlendData + joff + 0xc)) + one;
            f32* jm = (f32*)ObjModel_GetJointMatrix((u8*)am, j);
            PSMTXScale((MtxPtr)sm, sc, sc, sc);
            if (lbl_803DCC35 == 0)
            {
                {
                    char* jp = (char*)((ModelFileHeader*)m)->jointBlendData + joff;
                    PSMTXTrans((MtxPtr)tm, -*(f32*)jp, -*(f32*)(jp + 4), -*(f32*)(jp + 8));
                }
                PSMTXConcat((MtxPtr)sm, (MtxPtr)tm, (MtxPtr)sm);
                {
                    char* jp = (char*)((ModelFileHeader*)m)->jointBlendData + joff;
                    PSMTXTrans((MtxPtr)tm, *(f32*)jp, *(f32*)(jp + 4), *(f32*)(jp + 8));
                }
                PSMTXConcat((MtxPtr)tm, (MtxPtr)sm, (MtxPtr)sm);
            }
            PSMTXConcat((MtxPtr)jm, (MtxPtr)sm, (MtxPtr)jm);
            joff += 0x10;
        }
        if (did != 0)
        {
            model_multMtxs((u8*)am, wm);
        }
    }
    modelInitMtxs((ModelFileHeader*)m, (ObjModel*)am);
    modelRenderInstrsState_init((ModelRenderInstrsState*)&bs, ((ModelFileHeader*)m)->instrs,
                                ((ModelFileHeader*)m)->instrsBitLenWords << 3,
                                ((ModelFileHeader*)m)->instrsBitLenWords << 3);
    {
        f32 inv = lbl_803DEA1C / ((GameObject*)obj)->anim.rootMotionScale;
        PSMTXScale((MtxPtr)sm, inv, inv, inv);
    }
    if (*(u32*)&((ModelFileHeader*)m)->vertexAnimEntries != 0)
    {
        if (m4 || m2 || (mode8 & 8))
        {
            f32 sc2 = lbl_803DEA1C + (lbl_803DEA54 * ((f32)(gObjFuzzLayerIndex + 1) * fade)) / *(f32*)(m + 0x50);
            PSMTXTrans((MtxPtr)tm, -*(f32*)(m + 0x44), -*(f32*)(m + 0x48), -*(f32*)(m + 0x4c));
            PSMTXScale((MtxPtr)sm, sc2, sc2, sc2);
            PSMTXConcat((MtxPtr)sm, (MtxPtr)tm, (MtxPtr)sm);
            PSMTXTrans((MtxPtr)tm, *(f32*)(m + 0x44), *(f32*)(m + 0x48), *(f32*)(m + 0x4c));
            PSMTXConcat((MtxPtr)tm, (MtxPtr)sm, (MtxPtr)sm);
            PSMTXConcat((MtxPtr)wm, (MtxPtr)sm, (MtxPtr)t2m);
            PSMTXConcat((MtxPtr)vm, (MtxPtr)t2m, (MtxPtr)fm);
        }
        else
        {
            PSMTXConcat((MtxPtr)vm, (MtxPtr)wm, (MtxPtr)fm);
        }
        {
            f32 z;
            GXLoadPosMtxImm((const f32 (*)[4])fm, gObjGxPosMtxIdTable[9]);
            z = lbl_803DEA04;
            fm[3] = z;
            fm[7] = z;
            fm[11] = z;
            PSMTXConcat((MtxPtr)fm, (MtxPtr)sm, (MtxPtr)fm);
            GXLoadNrmMtxImm((const f32 (*)[4])fm, gObjGxPosMtxIdTable[9]);
            GXLoadTexMtxImm((const f32 (*)[4])fm, gObjGxTexMtxIdTable[9], 0);
        }
    }
    m1 = mode8 & 1;
    if (m1 != 0)
    {
        GXSetNumTexGens(0);
        GXSetNumTevStages(1);
        GXSetNumIndStages(0);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        {
            u32 sh;
            u8* o;
            u8* nxt;
            o = (u8*)obj;
            while ((nxt = *(u8**)&((GameObject*)o)->ownerObj) != NULL)
            {
                o = nxt;
            }
            sh = ((u8*)((GameObject*)o)->anim.modelState->shadowCastSlot)[0x65];
            if (sh == 0xff)
            {
                GXSetTevColor(GX_TEVREG2, *(GXColor*)&lbl_803DB468);
                GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
            }
            else
            {
                if (sh < 8)
                {
                    color[0] = 1 << sh;
                    color[1] = 0;
                    color[2] = 0;
                }
                else
                {
                    color[0] = 0;
                    color[1] = 1 << (sh - 8);
                    color[2] = 0;
                }
                color[3] = 0xff;
                GXSetTevColor(GX_TEVREG2, *(GXColor*)color);
                GXSetBlendMode(GX_BM_LOGIC, GX_BL_ONE, GX_BL_ZERO, GX_LO_OR);
            }
        }
        GXSetTevDirect(GX_TEVSTAGE0);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C2);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A2);
        GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetFog(GX_FOG_NONE, 0.0f, 0.0f, 0.0f, 0.0f, *(GXColor*)&lbl_803DB468);
        gxSetPeControl_ZCompLoc_(1);
        GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
        GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
        GXSetNumChans(1);
        if (OBJPRINT_MODEL_DEF(obj)->renderFlags & OBJDEF_RENDERFLAG_PROJECTED_SHADOW)
        {
            gxSetZMode_(1, 3, 1);
            GXSetCullMode(GX_CULL_FRONT);
        }
        else
        {
            gxSetZMode_(0, 3, 0);
            GXSetCullMode(GX_CULL_NONE);
        }
    }
    else if (m2 != 0)
    {
        objRenderFuzzFn_8003d6f8(obj);
    }
    else
    {
        Camera_RebuildProjectionMatrix();
        objFn_8003dc50(m, (u8*)obj);
        if (((ModelFileHeader*)m)->flags & 0x100)
        {
            GXSetFog(GX_FOG_NONE, 0.0f, 0.0f, 0.0f, 0.0f, *(GXColor*)&lbl_803DB468);
        }
        else
        {
            _gxSetFogParams();
        }
    }
    GXSetArray(GX_VA_POS,
               (void*)((int*)((char*)am + 0x1c))[(((ObjModel*)am)->bufferFlags >> 1) & 1], 6);
    if (((ModelFileHeader*)m)->flags24 & 8)
    {
        GXSetArray(GX_VA_NRM, ((ObjModel*)am)->normalBuf, 9);
    }
    else
    {
        GXSetArray(GX_VA_NRM, ((ObjModel*)am)->normalBuf, 3);
    }
    GXSetArray(GX_VA_CLR0, ((ModelFileHeader*)m)->colors, 2);
    GXSetArray(GX_VA_TEX0, ((ModelFileHeader*)m)->texCoords, 4);
    GXSetArray(GX_VA_TEX1, ((ModelFileHeader*)m)->texCoords, 4);
    done = 0;
    while (!done)
    {
        u32 op4;
        {
            u32 w;
            int pos = bs.pos;
            u8* p = bs.data + (pos >> 3);
            w = p[0];
            w |= p[1] << 8;
            w |= p[2] << 16;
            bs.pos = pos + 4;
            op4 = (w >> (pos & 7)) & 0xf;
        }
        switch (op4)
        {
        case 3:
            modelRenderFn_setVtxDescr(m, (u8*)op, refs, &bs, mode, &o9, &o8);
            break;
        case 1:
            if (mode == 0 || mode == 4 || mode == 8)
            {
                u32 idx;
                if (lbl_803DCC20 == 0)
                {
                    idx = objRenderFn_8003edf4((u8*)obj, m, am, &bs);
                    op = (int*)ObjModel_GetRenderOp((ModelFileHeader*)m, idx);
                }
                else
                {
                    u32 w;
                    int pos = bs.pos;
                    u8* p = bs.data + (pos >> 3);
                    w = p[0];
                    w |= p[1] << 8;
                    w |= p[2] << 16;
                    bs.pos = pos + 6;
                    idx = (w >> (pos & 7)) & 0x3f;
                    op = (int*)ObjModel_GetRenderOp((ModelFileHeader*)m, idx);
                }
                refs = (u32*)ObjModel_GetRenderOpTextureRefs((ObjModel*)am, idx);
            }
            break;
        case 2:
            if ((mode != 4 && mode != 8) || lbl_803DCC3E != 0)
            {
                u8* dl;
                u32 w;
                int pos = bs.pos;
                u8* p = bs.data + (pos >> 3);
                w = p[0];
                w |= p[1] << 8;
                w |= p[2] << 16;
                bs.pos = pos + 8;
                dl = modelFileGetDisplayList(m, (w >> (pos & 7)) & 0xff);
                GXCallDisplayList(*(void**)dl, *(u16*)(dl + 4));
            }
            else
            {
                bs.pos += 8;
            }
            break;
        case 4:
            renderOpMatrix(m, am, &bs, sm, vm, o9, o8, m1);
            break;
        case 5:
            done = 1;
            break;
        }
    }
}


void objMtxFn_80041104(f32* mtx, f32* out, s16* in, int flag, int* obj, int e);

void objRenderFn_80041018(GameObject* obj)
{
    ObjDefHitVolume* p;
    ObjHitVolumeRuntimeTransform* q;
    int* model;
    ObjDefHitVolume* base;
    int i;
    base = obj->anim.modelInstance->hitVolumes;
    q = obj->anim.hitVolumeTransforms;
    if (!(*(u8*)&obj->anim.resetHitboxMode & 0x28))
    {
        model = (int*)Obj_GetActiveModel(obj);
        i = 0;
        p = base;
        for (; i < obj->anim.modelInstance->hitVolumeCount; i++)
        {
            int j = p->jointIndices[OBJPRINT_ACTIVE_BANK_INDEX(obj)];
            ObjModelJointMatrix* mtx;
            if (j >= 0)
            {
                mtx = ObjModel_GetJointMatrix((u8*)model, j);
            }
            else
            {
                mtx = NULL;
            }
            objMtxFn_80041104(NULL, &q->centerX, &p->posX, base->flags & 0x10, (int*)obj, 0);
            objMtxFn_80041104((f32*)mtx, &q->jointX, &p->jointOffsetX, base->flags & 0x10, (int*)obj, 1);
            p++;
            q++;
        }
    }
}


void objMtxFn_80041104(f32* mtx, f32* out, s16* in, int flag, int* obj, int e)
{
    f32 m[16];
    MatrixTransform blk;
    f32 v[3];
    f32 res[3];
    v[0] = in[0];
    v[1] = in[1];
    v[2] = in[2];
    if (e != 0)
    {
        v[0] *= 0.00390625f;
        v[1] *= 0.00390625f;
        v[2] *= 0.00390625f;
    }
    if (mtx != NULL)
    {
        if (flag != 0)
        {
            out[0] = mtx[3] + v[0];
            out[1] = mtx[7] + v[1];
            out[2] = mtx[11] + v[2];
        }
        else
        {
            PSMTXMultVec((MtxPtr)mtx, (Vec*)v, (Vec*)res);
            out[0] = res[0];
            out[1] = res[1];
            out[2] = res[2];
        }
        out[0] += playerMapOffsetX;
        out[2] += playerMapOffsetZ;
    }
    else
    {
        blk.x = ((GameObject*)obj)->anim.worldPosX;
        blk.y = ((GameObject*)obj)->anim.worldPosY;
        blk.z = ((GameObject*)obj)->anim.worldPosZ;
        if (flag != 0)
        {
            blk.rotX = 0;
            blk.rotY = 0;
            blk.rotZ = 0;
        }
        else
        {
            blk.rotX = ((GameObject*)obj)->anim.rotX;
            blk.rotY = ((GameObject*)obj)->anim.rotY;
            blk.rotZ = ((GameObject*)obj)->anim.rotZ;
        }
        blk.scale = lbl_803DEA1C;
        setMatrixFromObjectPos(m, &blk);
        Matrix_TransformPoint(m, v[0], v[1], v[2], &out[0], &out[1], &out[2]);
    }
}



void objSetOverrideColor(u8 r, u8 g, u8 b)
{
    gObjOverrideColorPending = 1;
    gObjOverrideColor[0] = r;
    gObjOverrideColor[1] = g;
    gObjOverrideColor[2] = b;
}

extern u32 gRomListLoadInFlight;
void objSetCurrentMatrix(u32 x)
{
    curObjMtx = x;
}

void objRenderFuzzShells(int* obj)
{
    int* model;
    u32 savedMtx;
    gObjFuzzStep = 1;
    model = (int*)Obj_GetActiveModel((GameObject*)obj);
    savedMtx = curObjMtx;
    lbl_803DCC3D = gObjFuzzPhase;
    ObjModel_SetRenderCallback((u8*)model, modelRenderCb_8003c268);
    for (gObjFuzzLayerIndex = 0; gObjFuzzLayerIndex < 16; gObjFuzzLayerIndex += gObjFuzzStep)
    {
        modelDoRenderInstrs(obj, ((GameObject*)obj)->ownerObj ? ((GameObject*)obj)->ownerObj : obj, (u8*)*model, 8);
        curObjMtx = savedMtx;
    }
    curObjMtx = 0;
    ObjModel_SetRenderCallback((u8*)model, NULL);
    gObjFuzzPhase += timeDelta;
    if (gObjFuzzPhase > lbl_803DEA60)
    {
        gObjFuzzPhase -= lbl_803DEA5C;
    }
}

void objRenderFn_800413d4(int* obj)
{
    int* model;
    u32 savedMtx;
    gObjFuzzStep = 4;
    model = (int*)Obj_GetActiveModel((GameObject*)obj);
    savedMtx = curObjMtx;
    lbl_803DCC3D = gObjFuzzPhase;
    for (gObjFuzzLayerIndex = 0; gObjFuzzLayerIndex < 16; gObjFuzzLayerIndex += gObjFuzzStep)
    {
        modelDoRenderInstrs(obj, ((GameObject*)obj)->ownerObj ? ((GameObject*)obj)->ownerObj : obj, (u8*)*model, 2);
        curObjMtx = savedMtx;
    }
    curObjMtx = 0;
    gObjFuzzPhase += timeDelta;
    if (gObjFuzzPhase > lbl_803DEA60)
    {
        gObjFuzzPhase -= lbl_803DEA5C;
    }
}

/* seqIds that always get the strong, high-segment-count fuzz (retail
   OBJECTS.bin names) */
#define OBJPRINT_SEQID_FRONT_FOX   0x77d /* "FrontFox" (DLL 0x2C0) */
#define OBJPRINT_SEQID_DIE_FOX     0x882 /* "DieFox" (DLL 0x10E) */
#define OBJPRINT_SEQID_DIE_KRYSTAL 0x887 /* "DieKrystal" (DLL 0x10E) */

void objRenderFuzz(int* obj)
{
    int n;
    u8 maxN;
    int cnt;
    int* model;
    u32 savedMtx;
    u8 strong;
    f32 dx, dy, dz, dist;
    CameraViewSlot* cam = Camera_GetCurrentViewSlot();
    if ((((GameObject*)obj)->objectFlags & OBJECT_OBJFLAG_PARENT_SLACK) ||
        ((GameObject*)obj)->anim.mapEventSlot == 0x3f ||
        ((GameObject*)obj)->anim.seqId == OBJPRINT_SEQID_DIE_FOX ||
        ((GameObject*)obj)->anim.seqId == OBJPRINT_SEQID_DIE_KRYSTAL)
    {
        strong = 1;
        if (((GameObject*)obj)->anim.classId == 1 ||
            ((GameObject*)obj)->anim.seqId == OBJPRINT_SEQID_FRONT_FOX ||
            ((GameObject*)obj)->anim.seqId == OBJPRINT_SEQID_DIE_FOX ||
            ((GameObject*)obj)->anim.seqId == OBJPRINT_SEQID_DIE_KRYSTAL)
        {
            maxN = 0xf;
        }
        else
        {
            maxN = 7;
        }
    }
    else
    {
        strong = 0;
        maxN = 3;
    }
    {
        u32 m = curObjMtx;
        if (m != 0)
        {
            dx = *(f32*)&((ModelFileHeader*)m)->dataSize - (cam->x - playerMapOffsetX);
            dy = *(f32*)&((ModelFileHeader*)m)->unk1C - cam->y;
            dz = *(f32*)&((ModelFileHeader*)m)->normals - (cam->z - playerMapOffsetZ);
        }
        else
        {
            dx = ((GameObject*)obj)->anim.worldPosX - cam->x;
            dy = ((GameObject*)obj)->anim.worldPosY - cam->y;
            dz = ((GameObject*)obj)->anim.worldPosZ - cam->z;
        }
    }
    dist = sqrtf(dx * dx + dy * dy + dz * dz);
    if (strong == 0)
    {
        cnt = (s32)((lbl_803DEA64 * (lbl_803DEA68 * dist)) /
                    (((GameObject*)obj)->anim.hitboxScale * ((GameObject*)obj)->anim.rootMotionScale));
        gObjFuzzStep = 2;
    }
    else
    {
        cnt = (s32)((lbl_803DEA68 * dist) /
                    (((GameObject*)obj)->anim.hitboxScale * ((GameObject*)obj)->anim.rootMotionScale));
        gObjFuzzStep = 1;
    }
    n = 16 - cnt;
    if (n > 0)
    {
        if (n > maxN)
        {
            n = maxN;
        }
        model = (int*)Obj_GetActiveModel((GameObject*)obj);
        savedMtx = curObjMtx;
        ObjModel_SetRenderCallback((u8*)model, shaderFuzzFn_8003cc1c);
        for (gObjFuzzLayerIndex = 0; gObjFuzzLayerIndex < n; gObjFuzzLayerIndex++)
        {
            modelDoRenderInstrs(obj, ((GameObject*)obj)->ownerObj ? ((GameObject*)obj)->ownerObj : obj, (u8*)*model, 4);
            curObjMtx = savedMtx;
        }
        curObjMtx = 0;
        ObjModel_SetRenderCallback((u8*)model, NULL);
    }
}

void objRenderShadow(void* obj)
{
    if (lbl_803DEA04 == ((GameObject*)obj)->anim.rootMotionScale)
    {
        curObjMtx = 0;
        return;
    }
    {
        int* m = *(int**)Obj_GetActiveModel((GameObject*)obj);
        if (((ModelFileHeader*)m)->shadowDisplayListCount != 0)
        {
            objRenderShadow2(obj, obj, (u8*)m, 1);
        }
        else
        {
            modelDoRenderInstrs(obj, obj, (u8*)m, 1);
        }
    }
    if (((GameObject*)obj)->anim.classId == 1)
    {
        u8* iter;
        int i = 0;
        iter = (u8*)obj;
        for (; i < ((GameObject*)obj)->childCount; i++)
        {
            int* child = *(int**)&((GameObject*)iter)->childObjs[0];
            if (child != NULL)
            {
                objRenderChild(child, obj, 1);
            }
            iter += 4;
        }
    }
}

void objRenderChild(int* child, int* parent, u8 isShadow)
{
    f32 res[3];
    MatrixTransform blk;
    f32 wm[16];
    f32 m2[16];
    f32 dx, dz;
    int off;
    f32* mtx;
    if (lbl_803DEA04 == ((GameObject*)child)->anim.rootMotionScale)
    {
        curObjMtx = 0;
        return;
    }
    Obj_GetActiveModel((GameObject*)child);
    {
        int* pmodel = (int*)Obj_GetActiveModel((GameObject*)parent);
        ChildEnt* ent;
        int j;
        u8* tbl = *(u8**)(*(int*)&((GameObject*)parent)->anim.modelInstance + 0x2c);
        off = (((GameObject*)child)->objectFlags & 7) * 0x18;
        ent = (ChildEnt*)(tbl + off);
        j = ent->joints[OBJPRINT_ACTIVE_BANK_INDEX(parent)];
        blk.x = *(f32*)(off + (char*)tbl);
        blk.y = ent->pos[1];
        blk.z = ent->pos[2];
        if (j == -1)
        {
            Obj_BuildWorldTransformMatrix((GameObject*)parent, wm, 0);
            mtx = wm;
        }
        else
        {
            mtx = (f32*)ObjModel_GetJointMatrix((u8*)pmodel, j);
        }
    }
    if (OBJPRINT_MODEL_DEF(child)->renderFlags & 8)
    {
        CameraViewSlot* cam = Camera_GetCurrentViewSlot();
        blk.scale = ((GameObject*)child)->anim.rootMotionScale;
        dx = ((GameObject*)child)->anim.localPosX - cam->x;
        dz = ((GameObject*)child)->anim.localPosZ - cam->z;
        blk.rotX = getAngle(dx, dz) + 0x8000;
        blk.rotY = getAngle(((GameObject*)child)->anim.localPosY - cam->y,
                              sqrtf(dx * dx + dz * dz));
        blk.rotZ = cam->roll;
        setMatrixFromObjectTransposed(&blk, m2);
        res[0] = m2[3];
        res[1] = m2[7];
        res[2] = m2[11];
        PSMTXMultVec((MtxPtr)mtx, (Vec*)res, (Vec*)res);
        m2[3] = res[0];
        m2[7] = res[1];
        m2[11] = res[2];
    }
    else
    {
        ChildEnt* pr;
        blk.scale = lbl_803DEA1C;
        pr = (ChildEnt*)(*(u8**)(*(int*)&((GameObject*)parent)->anim.modelInstance + 0x2c) + off);
        blk.rotX = pr->rot[0];
        blk.rotY = pr->rot[1];
        blk.rotZ = pr->rot[2];
        setMatrixFromObjectTransposed(&blk, m2);
        PSMTXConcat((MtxPtr)mtx, (MtxPtr)m2, (MtxPtr)m2);
    }
    if (isShadow == 0)
    {
        void* space;
        ((GameObject*)child)->anim.worldPosX = m2[3] + playerMapOffsetX;
        ((GameObject*)child)->anim.worldPosY = m2[7];
        ((GameObject*)child)->anim.worldPosZ = m2[11] + playerMapOffsetZ;
        space = ((GameObject*)child)->anim.parent;
        if (space != NULL)
        {
            Obj_TransformWorldPointToLocal(((GameObject*)child)->anim.worldPosX, ((GameObject*)child)->anim.worldPosY,
                                           ((GameObject*)child)->anim.worldPosZ, &((GameObject*)child)->anim.localPosX,
                                           &((GameObject*)child)->anim.localPosY, &((GameObject*)child)->anim.localPosZ,
                                           (u32)space);
        }
        else
        {
            ((GameObject*)child)->anim.localPosX = ((GameObject*)child)->anim.worldPosX;
            ((GameObject*)child)->anim.localPosY = ((GameObject*)child)->anim.worldPosY;
            ((GameObject*)child)->anim.localPosZ = ((GameObject*)child)->anim.worldPosZ;
        }
        objMatrixToRotation(m2, &((GameObject*)child)->anim.rotX, &((GameObject*)child)->anim.rotY,
                             &((GameObject*)child)->anim.rotZ);
    }
    ((GameObject*)child)->anim.renderAlpha =
        ((((GameObject*)child)->anim.alpha + 1) * ((GameObject*)parent)->anim.renderAlpha) >> 8;
    ((GameObject*)child)->unkF1 = ((GameObject*)parent)->unkF1;
    if (!(((GameObject*)child)->anim.flags & OBJANIM_FLAG_HIDDEN))
    {
        curObjMtx = (u32)m2;
        if (isShadow == 0)
        {
            ((GameObject*)child)->objectFlags |= OBJECT_OBJFLAG_RENDERED;
            objRenderModel((GameObject*)child);
        }
        else
        {
            objRenderShadow(child);
        }
    }
}


/*
 * Bit-cursor over the model's render-instruction stream
 * (ModelFileHeader.instrs, bit length at header +0xD8 * 8).  Every reader
 * fetches 3 bytes little-endian around the cursor and shifts by (pos & 7).
 * Stream grammar (4-bit opcodes):
 *   1 = bind render op: 6-bit renderOps index (shader state setup)
 *   2 = draw: 8-bit display-list index -> GXCallDisplayList
 *   3 = vertex descriptor block: 1-bit pos/nrm/clr/tex size selectors
 *   4 = load matrices: 4-bit count, then 8-bit joint-matrix indices
 *   5 = end of stream
 * The stream is walked through a MtxBitStream (data at +0, cursor at +0x10).
 */

void objRenderModel(GameObject* obj)
{
    u32 d1;
    f32 d2;
    int d3;
    int d4;
    f32 px;
    f32 py;
    f32 pz;
    s32 sx;
    s32 sy;
    s32 sz;
    u32 col;
    int* model = (int*)Obj_GetActiveModel(obj);
    if (lbl_803DEA04 == obj->anim.rootMotionScale)
    {
        curObjMtx = 0;
        return;
    }
    {
        int m0 = *model;
        if (((ModelFileHeader*)m0)->flags & 0x8000)
        {
            modelDoAltRenderInstrs((int*)obj, obj->ownerObj ? (int*)obj->ownerObj : (int*)obj, (u8*)m0, 0);
        }
        else
        {
            modelDoRenderInstrs((int*)obj, obj->ownerObj ? (int*)obj->ownerObj : (int*)obj, (u8*)m0, 0);
        }
    }
    {
        u8* iter;
        int i = 0;
        iter = (u8*)obj;
        for (; i < obj->childCount; i++)
        {
            int* child = *(int**)&((GameObject*)iter)->childObjs[0];
            if (child != NULL)
            {
                objRenderChild(child, (int*)obj, 0);
            }
            iter += 4;
        }
    }
    if (OBJPRINT_MODEL_DEF(obj)->shadowType != 4)
    {
        return;
    }
    if (gObjRenderingShadowPass != 0)
    {
        return;
    }
    {
        s16 seqId = obj->anim.seqId;
        if (seqId == 0x6a8)
            return;
        if (seqId == 0x6a9)
            return;
        if (seqId == 0x6aa)
            return;
        if (seqId == 0x6ab)
            return;
        if (seqId == 0x6ac)
            return;
        if (seqId == 0x752)
            return;
    }
    Camera_ProjectWorldPointWithOffset(
        obj->anim.localPosX - playerMapOffsetX, obj->anim.localPosY, obj->anim.localPosZ - playerMapOffsetZ,
        obj->anim.hitboxScale * obj->anim.rootMotionScale, &px, &py, &pz);
    Camera_NdcToScreen(px, py, pz, &sx, &sy, &sz);
    if (sz <= depthReadRequestPoll(sx, sy, obj))
    {
        obj->anim.modelState->shadowAlphaStep = 0x20;
    }
    else
    {
        obj->anim.modelState->shadowAlphaStep = -0x20;
    }
    {
        int a;
        a = obj->anim.modelState->shadowAlpha;
        a += obj->anim.modelState->shadowAlphaStep;
        if (a > 0xff)
        {
            obj->anim.modelState->shadowAlpha = 0xff;
        }
        else if (a < 0)
        {
            obj->anim.modelState->shadowAlpha = 0;
        }
        else
        {
            obj->anim.modelState->shadowAlpha = a;
        }
    }
    gObjShadowColor[3] = obj->anim.modelState->shadowAlpha;
    getObjectShadowDrawParams(obj, &d1, &d2, &d3, &d4);
    col = *(u32*)gObjShadowColor;
    hudDrawColored(d1, d3, d4, &col, (s32)(lbl_803DEA6C * d2), 1);
}

void objSetRenderingShadowPass(u8 x)
{
    gObjRenderingShadowPass = x;
}



extern int gPendingDvdReadCount;

void initLoadFileReadCb(s32 result, DVDFileInfo* fileInfo)
{
    if (result < 0)
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
    }
    else
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
        gPendingDvdReadCount--;
    }
}



// DVDGetCommandBlockStatus() command-block states (DVD_STATE_*)
#define DVD_STATE_FATAL_ERROR   -1
#define DVD_STATE_END           0
#define DVD_STATE_BUSY          1
#define DVD_STATE_WAITING       2
#define DVD_STATE_COVER_CLOSED  3
#define DVD_STATE_NO_DISK       4
#define DVD_STATE_COVER_OPEN    5
#define DVD_STATE_WRONG_DISK    6
#define DVD_STATE_MOTOR_STOPPED 7
#define DVD_STATE_PAUSING       8
#define DVD_STATE_IGNORED       9
#define DVD_STATE_CANCELED      10
#define DVD_STATE_RETRY         11

s32 ObjLoad_GetDvdCommandBlockStatus(DVDCommandBlock* block)
{
    s32 status;
    if (block == NULL)
    {
        return -1;
    }
    status = DVDGetCommandBlockStatus(block);
    switch (status)
    {
    case DVD_STATE_FATAL_ERROR:
        return status;
    case DVD_STATE_END:
        return status;
    case DVD_STATE_BUSY:
        return status;
    case DVD_STATE_WAITING:
        return status;
    case DVD_STATE_COVER_CLOSED:
        return status;
    case DVD_STATE_NO_DISK:
        return status;
    case DVD_STATE_COVER_OPEN:
        return status;
    case DVD_STATE_WRONG_DISK:
        return status;
    case DVD_STATE_MOTOR_STOPPED:
        return status;
    case DVD_STATE_PAUSING:
        return status;
    case DVD_STATE_IGNORED:
        return status;
    case DVD_STATE_CANCELED:
        return status;
    case DVD_STATE_RETRY:
        return status;
    }
    return 0;
}
u8 gObjGxPosMtxIdTable[12] = {0x00, 0x03, 0x06, 0x09, 0x0C, 0x0F, 0x12, 0x15, 0x18, 0x1B, 0x00, 0x00};
u8 gObjGxTexMtxIdTable[12] = {0x1E, 0x21, 0x24, 0x27, 0x2A, 0x2D, 0x30, 0x33, 0x36, 0x39, 0x00, 0x00};



extern u32 gForceLoadImmediately;
void clearForceLoadImmediately(void)
{
    gForceLoadImmediately = 0x0;
}
void setForceLoadImmediately(void)
{
    gForceLoadImmediately = 0x1;
}
static inline int loadedFileFlags(int slot)
{
    int s = OSDisableInterrupts();
    u32 v = gAssetLoadInFlightFlags;
    OSRestoreInterrupts(s);
    return v;
}
extern u8 gResourceFileTable[0x160];

void defragMemory(int mode)
{
    char* q1;
    char* q2;
    char* q3;
    char* q4;
    int i;
    int pass;
    int done;
    int d;
    u8* base = gResourceFileTable;
    done = 0;
    pass = 0;
    texFlagFn_80023cbc(2);
    if (loadedFileFlags(0) != 0)
    {
        return;
    }
    if (mode == 0 && gDefragDelayFrames == 0)
    {
        texRestructRefs(0);
        gDefragDelayFrames = 6;
        return;
    }
    if (mode != 0)
    {
        char* p1;
        char* p2;
        char* p3;
        char* p4;
        void* n;
        int i;
        testAndSet_onlyUseHeaps1and2(1);
        i = 0;
        {
            char* hi = (char*)base + 0x20000;
            p1 = hi - 0x6a28;
            p2 = hi - 0x68c8;
            p3 = hi - 0x6d68;
            p4 = hi - 0x6f20;
        }
        do
        {
            switch (i)
            {
            case 0xd:
            case 0x1b:
            case 0x23:
            case 0x25:
            case 0x2b:
            case 0x30:
            case 0x46:
            case 0x47:
            case 0x4a:
            case 0x4d:
            case 0x54:
            case 0x55:
            {
                if (*(void**)p1 == NULL)
                {
                    break;
                }
                if (*(s16*)p2 == -1)
                {
                    break;
                }
                if (mmGetRegionForPtr(*(void**)p1) != 0)
                {
                    break;
                }
                if (mode == 2)
                {
                    if (i == 0x20)
                        break;
                    if (i == 0x4b)
                        break;
                    if (i == 0x23)
                        break;
                    if (i == 0x4d)
                        break;
                }
                n = mmAlloc(*(int*)p3 + 0x20, 0x7d7d7d7d, 0);
                if (n == NULL)
                {
                    break;
                }
                memcpy(n, *(void**)p1, *(int*)p3);
                {
                    int d = mmSetFreeDelay(0);
                    mm_free(*(void**)p1);
                    *(int*)p1 = 0;
                    *(void**)p1 = n;
                    mmSetFreeDelay(d);
                }
                break;
            }
            }
            *(u8*)p4 = 0;
            p1 += 4;
            p2 += 2;
            p3 += 4;
            p4 += 1;
            i++;
        } while (i <= 0x57);
        testAndSet_onlyUseHeaps1and2(-1);
    }
    base = (u8*)((char*)base + 0x20000);
    while (done == 0 && pass < 10)
    {
        done = 1;
        i = 0;
        q1 = (char*)base - 0x6a28;
        q2 = (char*)base - 0x68c8;
        q3 = (char*)base - 0x6d68;
        q4 = (char*)base - 0x6f20;
        do
        {
            switch (i)
            {
            case 0xd:
            case 0x1b:
            case 0x23:
            case 0x25:
            case 0x2b:
            case 0x30:
            case 0x46:
            case 0x47:
            case 0x4a:
            case 0x4d:
            case 0x54:
            case 0x55:
            {
                void* n;
                if (*(void**)q1 != NULL && *(s16*)q2 != -1 && mmGetRegionForPtr(*(void**)q1) == 0)
                {
                    n = mmAlloc(*(int*)q3 + 0x20, 0x7d7d7d7d, 0);
                    if (n == NULL)
                    {
                        break;
                    }
                    if (*(int*)q3 >= 0x33450 && *(u32*)q1 < (u32)n)
                    {
                        int d = mmSetFreeDelay(0);
                        mm_free(n);
                        mmSetFreeDelay(d);
                    }
                    else if (*(int*)q3 < 0x33450 && *(u32*)q1 > (u32)n)
                    {
                        int d = mmSetFreeDelay(0);
                        mm_free(n);
                        mmSetFreeDelay(d);
                    }
                    else
                    {
                        int d;
                        memcpy(n, *(void**)q1, *(int*)q3);
                        d = mmSetFreeDelay(0);
                        mm_free(*(void**)q1);
                        *(int*)q1 = 0;
                        *(void**)q1 = n;
                        mmSetFreeDelay(d);
                        done = 0;
                    }
                }
                else
                {
                    if (mode == 2)
                        break;
                    if (pass == 0)
                        break;
                    if (*(void**)q1 == NULL)
                        break;
                    if (*(s16*)q2 == -1)
                        break;
                    if (mmGetRegionForPtr(*(void**)q1) != 1 && mmGetRegionForPtr(*(void**)q1) != 2)
                    {
                        break;
                    }
                    if (getHeapItemSize(*(void**)q1) < 0x3000)
                    {
                        break;
                    }
                    n = mmAlloc(*(int*)q3 + 0x20, 0x7d7d7d7d, 0);
                    if (n == NULL)
                    {
                        break;
                    }
                    if (mmGetRegionForPtr(n) != 0)
                    {
                        int d = mmSetFreeDelay(0);
                        mm_free(n);
                        mmSetFreeDelay(d);
                    }
                    else
                    {
                        memcpy(n, *(void**)q1, *(int*)q3);
                        d = mmSetFreeDelay(0);
                        mm_free(*(void**)q1);
                        *(int*)q1 = 0;
                        *(void**)q1 = n;
                        mmSetFreeDelay(d);
                        done = 0;
                    }
                }
                break;
            }
            }
            *(u8*)q4 = 0;
            q1 += 4;
            q2 += 2;
            q3 += 4;
            q4 += 1;
            i++;
        } while (i <= 0x57);
        pass++;
    }
    texFlagFn_80023cbc(0);
}

f32 gObjBoneMtxBuffer[0xC00];

extern u32 gObjBlockStatus[0x63F6];

void animCurvReadCb(s32 result, DVDFileInfo* fileInfo)
{
    if (result < 0)
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
    }
    else
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
        if (gAssetLoadInFlightFlags & 0x10000000)
        {
            gAssetLoadCompletedFlags |= 0x10000000;
            gObjBlockStatus[0x34 / 4] = 0;
        }
        else if (gAssetLoadInFlightFlags & 0x40000000)
        {
            gAssetLoadCompletedFlags |= 0x40000000;
            gObjBlockStatus[0x154 / 4] = 0;
        }
    }
}

u32 objRenderFn_8003edf4(u8* obj, u8* p2, int* am, MtxBitStream* bs);


void animCurvTabReadCb(s32 result, DVDFileInfo* fileInfo)
{
    if (result < 0)
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
    }
    else
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
        if (gAssetLoadInFlightFlags & 0x20000000)
        {
            gAssetLoadCompletedFlags |= 0x20000000;
            gObjBlockStatus[0x38 / 4] = 0;
        }
        else if (gAssetLoadInFlightFlags & 0x80000000)
        {
            gAssetLoadCompletedFlags |= 0x80000000;
            gObjBlockStatus[0x158 / 4] = 0;
        }
    }
}



void voxMapReadCb(s32 result, DVDFileInfo* fileInfo)
{
    if (result < 0)
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
    }
    else
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
        if (gAssetLoadInFlightFlags & 0x1000000)
        {
            gAssetLoadCompletedFlags |= 0x1000000;
            gObjBlockStatus[0x6c / 4] = 0;
        }
        else if (gAssetLoadInFlightFlags & 0x4000000)
        {
            gAssetLoadCompletedFlags |= 0x4000000;
            gObjBlockStatus[0x150 / 4] = 0;
        }
    }
}



void voxMapTabReadCb(s32 result, DVDFileInfo* fileInfo)
{
    if (result < 0)
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
    }
    else
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
        if (gAssetLoadInFlightFlags & 0x2000000)
        {
            gAssetLoadCompletedFlags |= 0x2000000;
            gObjBlockStatus[0x68 / 4] = 0;
        }
        else if (gAssetLoadInFlightFlags & 0x8000000)
        {
            gAssetLoadCompletedFlags |= 0x8000000;
            gObjBlockStatus[0x14c / 4] = 0;
        }
    }
}

u8 gResourceFileTable[0x160];


void blocksTabReadCb(s32 result, DVDFileInfo* fileInfo)
{
    if (result < 0)
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
    }
    else
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
        if (gAssetLoadInFlightFlags & 0x20000)
        {
            gAssetLoadCompletedFlags |= 0x20000;
            gObjBlockStatus[0x98 / 4] = 0;
        }
        else if (gAssetLoadInFlightFlags & 0x80000)
        {
            gAssetLoadCompletedFlags |= 0x80000;
            gObjBlockStatus[0x120 / 4] = 0;
        }
    }
}

void romListReadCb(s32 result, DVDFileInfo* fileInfo)
{
    gRomListLoadInFlight = 0;
    if (result < 0)
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
    }
    else
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
    }
}

extern u32 gResourceFileBuffers[];
u32 gObjBlockStatus[0x63F6];

void blocksReadCb(s32 result, DVDFileInfo* fileInfo)
{
    if (result < 0)
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
    }
    else
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
        if (gAssetLoadInFlightFlags & 0x10000)
        {
            gAssetLoadCompletedFlags |= 0x10000;
            gObjBlockStatus[0x94 / 4] = 0;
        }
        else if (gAssetLoadInFlightFlags & 0x40000)
        {
            gAssetLoadCompletedFlags |= 0x40000;
            gObjBlockStatus[0x11c / 4] = 0;
        }
    }
}

void tex1tab2readCb(s32 result, DVDFileInfo* fileInfo)
{
    if (result < 0)
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
        mm_free((void*)gResourceFileBuffers[78]);
        gResourceFileBuffers[78] = 0;
        gObjBlockStatus[78] = 0;
        if (gAssetLoadInFlightFlags & 0x8000)
        {
            gAssetLoadCompletedFlags |= 0x8000;
            gObjBlockStatus[76] = 0;
        }
    }
    else
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
        if (gAssetLoadInFlightFlags & 0x8000)
        {
            gAssetLoadCompletedFlags |= 0x8000;
            gObjBlockStatus[76] = 0;
        }
    }
}

void tex1tab1readCb(s32 result, DVDFileInfo* fileInfo)
{
    if (result < 0)
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
        mm_free((void*)gResourceFileBuffers[78]);
        gResourceFileBuffers[78] = 0;
        gObjBlockStatus[78] = 0;
        if (gAssetLoadInFlightFlags & 0x4000)
        {
            gAssetLoadCompletedFlags |= 0x4000;
            gObjBlockStatus[33] = 0;
        }
    }
    else
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
        if (gAssetLoadInFlightFlags & 0x4000)
        {
            gAssetLoadCompletedFlags |= 0x4000;
            gObjBlockStatus[33] = 0;
        }
    }
}

void tex1ReadCb(s32 result, DVDFileInfo* fileInfo)
{
    if (result < 0)
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
    }
    else
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
        if (gAssetLoadInFlightFlags & 0x1000)
        {
            gAssetLoadCompletedFlags |= 0x1000;
            gObjBlockStatus[0x80 / 4] = 0;
        }
        else if (gAssetLoadInFlightFlags & 0x2000)
        {
            gAssetLoadCompletedFlags |= 0x2000;
            gObjBlockStatus[0x12c / 4] = 0;
        }
    }
}

void tex0tab2readCb(s32 result, DVDFileInfo* fileInfo)
{
    if (result < 0)
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
        mm_free((void*)gResourceFileBuffers[78]);
        gResourceFileBuffers[78] = 0;
        gObjBlockStatus[78] = 0;
        if (gAssetLoadInFlightFlags & 0x800)
        {
            gAssetLoadCompletedFlags |= 0x800;
            gObjBlockStatus[78] = 0;
        }
    }
    else
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
        if (gAssetLoadInFlightFlags & 0x800)
        {
            gAssetLoadCompletedFlags |= 0x800;
            gObjBlockStatus[78] = 0;
        }
    }
}
void tex0tab1readCb(s32 result, DVDFileInfo* fileInfo)
{
    if (result < 0)
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
        mm_free((void*)gResourceFileBuffers[36]);
        gResourceFileBuffers[36] = 0;
        gObjBlockStatus[36] = 0;
        if (gAssetLoadInFlightFlags & 0x400)
        {
            gAssetLoadCompletedFlags |= 0x400;
            gObjBlockStatus[36] = 0;
        }
    }
    else
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
        if (gAssetLoadInFlightFlags & 0x400)
        {
            gAssetLoadCompletedFlags |= 0x400;
            gObjBlockStatus[36] = 0;
        }
    }
}

void tex0readCb(s32 result, DVDFileInfo* fileInfo)
{
    if (result < 0)
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
    }
    else
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
        if (gAssetLoadInFlightFlags & 0x100)
        {
            gAssetLoadCompletedFlags |= 0x100;
            gObjBlockStatus[0x8c / 4] = 0;
        }
        else if (gAssetLoadInFlightFlags & 0x200)
        {
            gAssetLoadCompletedFlags |= 0x200;
            gObjBlockStatus[0x134 / 4] = 0;
        }
    }
}

void animReadCb(s32 result, DVDFileInfo* fileInfo)
{
    if (result < 0)
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
    }
    else
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
        if (gAssetLoadInFlightFlags & 0x10)
        {
            gAssetLoadCompletedFlags |= 0x10;
            gObjBlockStatus[0xc0 / 4] = 0;
        }
        else if (gAssetLoadInFlightFlags & 0x20)
        {
            gAssetLoadCompletedFlags |= 0x20;
            gObjBlockStatus[0x128 / 4] = 0;
        }
    }
}

void modelsReadCb(s32 result, DVDFileInfo* fileInfo)
{
    if (result < 0)
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
    }
    else
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
        if (gAssetLoadInFlightFlags & 0x1)
        {
            gAssetLoadCompletedFlags |= 0x1;
            gObjBlockStatus[0xac / 4] = 0;
        }
        else if (gAssetLoadInFlightFlags & 0x2)
        {
            gAssetLoadCompletedFlags |= 0x2;
            gObjBlockStatus[0x118 / 4] = 0;
        }
    }
}

void animTabReadCb(s32 result, DVDFileInfo* fileInfo)
{
    if (result < 0)
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
    }
    else
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
        if (gAssetLoadInFlightFlags & 0x40)
        {
            gAssetLoadCompletedFlags |= 0x40;
            gObjBlockStatus[0xbc / 4] = 0;
        }
        else if (gAssetLoadInFlightFlags & 0x80)
        {
            gAssetLoadCompletedFlags |= 0x80;
            gObjBlockStatus[0x124 / 4] = 0;
        }
    }
}

void modelsTabReadCb(s32 result, DVDFileInfo* fileInfo)
{
    if (result < 0)
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
    }
    else
    {
        DVDClose(fileInfo);
        AtomicSList_Push(gDvdFileInfoPool, fileInfo);
        if (gAssetLoadInFlightFlags & 0x4)
        {
            gAssetLoadCompletedFlags |= 0x4;
            gObjBlockStatus[0xa8 / 4] = 0;
        }
        else if (gAssetLoadInFlightFlags & 0x8)
        {
            gAssetLoadCompletedFlags |= 0x8;
            gObjBlockStatus[0x114 / 4] = 0;
        }
    }
}

void mapLoadDataFiles(int mapIdx);

extern int sMapFileNameIndexRemapTable[];

static inline s32 mapCheckCurBlocksImpl(int v)
{
    if (((s16*)((char*)gObjMapBlockInfo + 0x4a))[0] == v)
        return 0;
    if (((s16*)((char*)gObjMapBlockInfo + 0x8e))[0] == v)
        return 1;
    return -1;
}

void mapLoadDataFiles(int mapIdx)
{
    if (sMapFileNameAdjacencyTable[mapIdx] != -1)
    {
        int* r = (int*)(*gMapEventInterface)->getCurCharPos();
        *(s8*)((char*)r + 0xe) = mapIdx;
    }
    mapLoadDataFile(mapIdx, MLDF_FILEID_TEX1_BIN_A);
    mapLoadDataFile(mapIdx, MLDF_FILEID_TEX1_TAB_A);
    mapLoadDataFile(mapIdx, MLDF_FILEID_TEX0_BIN_A);
    mapLoadDataFile(mapIdx, MLDF_FILEID_TEX0_TAB_A);
    mapLoadDataFile(mapIdx, MLDF_FILEID_ANIM_BIN_A);
    mapLoadDataFile(mapIdx, MLDF_FILEID_ANIM_TAB_A);
    mapLoadDataFile(mapIdx, MLDF_FILEID_MODELS_BIN_A);
    mapLoadDataFile(mapIdx, MLDF_FILEID_MODELS_TAB_A);
    mapLoadDataFile(mapIdx, MLDF_FILEID_BLOCKS_TAB_A);
    mapLoadDataFile(mapIdx, MLDF_FILEID_BLOCKS_BIN_A);
    mapLoadDataFile(mapIdx, MLDF_FILEID_VOXMAP_TAB_A);
    mapLoadDataFile(mapIdx, MLDF_FILEID_VOXMAP_BIN_A);
    mapLoadDataFile(mapIdx, MLDF_FILEID_ANIMCURV_TAB_A);
    mapLoadDataFile(mapIdx, MLDF_FILEID_ANIMCURV_BIN_A);
}


int loadMapAndParent(int mapId)
{
    int idx;
    int parent;
    if (mapId >= 0x4b)
    {
        idx = 5;
    }
    else
    {
        idx = sMapFileNameIndexRemapTable[mapId];
    }
    parent = sMapFileNameAdjacencyTable[idx];
    if (parent != -1 && mapCheckCurBlocksImpl(parent) == -1)
    {
        mapLoadDataFiles(parent);
        return parent;
    }
    mapLoadDataFiles(idx);
    return idx;
}

void clearLoadedFileFlags_blocks1(void)
{
    int s = OSDisableInterrupts();
    if (gAssetLoadInFlightFlags & 0x100000)
    {
        gAssetLoadInFlightFlags ^= 0x100000;
    }
    OSRestoreInterrupts(s);
}


void setLoadedFileFlags_blocks1(void)
{
    int s = OSDisableInterrupts();
    gAssetLoadInFlightFlags |= 0x100000;
    OSRestoreInterrupts(s);
}
int isRomListLoading(void)
{
    return gRomListLoadInFlight;
}

int getLoadedFileFlags(int slot)
{
    return loadedFileFlags(slot);
}


s32 gObjTableFileRequestFlags;
s16 gForceNextLoadSync;
u8 gLoadFilesInitDone;
void** gDvdFileInfoPool;
int gPendingDvdReadCount;
u32 gAssetLoadCompletedFlags;
volatile int gAssetLoadInFlightFlags;
int gModelsArchiveLoadCount;
s16 gDefragDelayFrames;
u32 gRomListLoadInFlight;
u32 gForceLoadImmediately;

u32 loadTableFiles(void)
{
    u8* base = gResourceFileTable;
    int s = OSDisableInterrupts();
    int flags = loadedFileFlags(0);
    int loadedFlags = gAssetLoadInFlightFlags;
    if ((gObjTableFileRequestFlags & 0x4) && !(flags & 0x4) && *(s32*)(base + 0x191e4) == -1)
    {
        mergeTableFiles((u32*)(base + 0x170e0), 0x2a, 0x45, 0x800);
    }
    if ((gObjTableFileRequestFlags & 0x8) && !(flags & 0x8) && *(s32*)(base + 0x19250) == -1)
    {
        mergeTableFiles((u32*)(base + 0x170e0), 0x2a, 0x45, 0x800);
    }
    if ((gObjTableFileRequestFlags & 0x40) && !(flags & 0x40) && *(s32*)(base + 0x191f8) == -1)
    {
        mergeTableFiles((u32*)(base + 0x14200), 0x2f, 0x49, 0xbb8);
    }
    if ((gObjTableFileRequestFlags & 0x80) && !(flags & 0x80) && *(s32*)(base + 0x19260) == -1)
    {
        mergeTableFiles((u32*)(base + 0x14200), 0x2f, 0x49, 0xbb8);
    }
    if ((gObjTableFileRequestFlags & 0x400) && !(flags & 0x400) && *(s32*)(base + 0x191c4) == -1)
    {
        mergeTableFiles((u32*)(base + 0x10200), 0x24, 0x4e, 0x1000);
    }
    if ((gObjTableFileRequestFlags & 0x800) && !(flags & 0x800) && *(s32*)(base + 0x1926c) == -1)
    {
        mergeTableFiles((u32*)(base + 0x10200), 0x24, 0x4e, 0x1000);
    }
    if ((gObjTableFileRequestFlags & 0x4000) && !(flags & 0x4000) && *(s32*)(base + 0x191b8) == -1)
    {
        mergeTableFiles((u32*)(base + 0xc200), 0x21, 0x4c, 0x1000);
    }
    if ((gObjTableFileRequestFlags & 0x8000) && !(flags & 0x8000) && *(s32*)(base + 0x19264) == -1)
    {
        mergeTableFiles((u32*)(base + 0xc200), 0x21, 0x4c, 0x1000);
    }
    if ((gObjTableFileRequestFlags & 0x20000) && !(flags & 0x20000) && *(s32*)(base + 0x191cc) == -1)
    {
        mergeTableFiles((u32*)(base + 0xa200), 0x26, 0x48, 0x800);
    }
    if ((gObjTableFileRequestFlags & 0x80000) && !(flags & 0x80000) && *(s32*)(base + 0x19254) == -1)
    {
        mergeTableFiles((u32*)(base + 0xa200), 0x26, 0x48, 0x800);
    }
    if ((gObjTableFileRequestFlags & 0x2000000) && !(flags & 0x2000000) && *(s32*)(base + 0x191a4) == -1)
    {
        mergeTableFiles((u32*)(base + 0x8200), 0x1a, 0x53, 0x800);
    }
    if ((gObjTableFileRequestFlags & 0x8000000) && !(flags & 0x8000000) && *(s32*)(base + 0x19288) == -1)
    {
        mergeTableFiles((u32*)(base + 0x8200), 0x1a, 0x53, 0x800);
    }
    if ((gObjTableFileRequestFlags & 0x20000000) && !(flags & 0x20000000) && *(s32*)(base + 0x1916c) == -1)
    {
        mergeTableFiles((u32*)(base + 0x2c0), 0xe, 0x56, 0x1fd0);
    }
    if ((gObjTableFileRequestFlags & 0x80000000) && !(flags & 0x80000000) && *(s32*)(base + 0x1928c) == -1)
    {
        mergeTableFiles((u32*)(base + 0x2c0), 0xe, 0x56, 0x1fd0);
    }
    gObjTableFileRequestFlags = flags;
    gAssetLoadInFlightFlags = gAssetLoadInFlightFlags ^ gAssetLoadCompletedFlags;
    gAssetLoadCompletedFlags = 0;
    OSRestoreInterrupts(s);
    return gAssetLoadInFlightFlags;
}

int unlockLevel(s32 val, int idx, int flag)
{
    s32 cur;
    if (flag == 1)
    {
        gObjLevelLockSlots[0] = -2;
        gObjLevelLockSlots[1] = -2;
        return -1;
    }
    cur = gObjLevelLockSlots[idx];
    if (val == cur || cur == -2)
    {
        gObjLevelLockSlots[idx] = -2;
        return -1;
    }
    return cur;
}


int lockLevel(s32 val, int idx)
{
    s32 cur = gObjLevelLockSlots[idx];
    if (cur == -2)
    {
        gObjLevelLockSlots[idx] = val;
        return -1;
    }
    return cur;
}

int getTableFileEntry(int fileId, int index, int* out)
{
    u8* base = gResourceFileTable;
    int count = 0;
    void* table = NULL;
    switch (fileId)
    {
    case 0x2a:
        count = 0x800;
        table = (u8*)(base + 0x10000) + 0x70e0;
        break;
    case 0x2f:
        count = 0xbb8;
        table = (u8*)(base + 0x10000) + 0x4200;
        break;
    case 0x24:
        count = 0x1000;
        table = (u8*)(base + 0x10000) + 0x200;
        break;
    case 0x21:
        count = 0x1000;
        table = (u8*)(base + 0x10000) - 0x3e00;
        break;
    case 0x50:
        table = *(void**)&base[0x19718];
        break;
    case 0x26:
        count = 0x800;
        table = (u8*)(base + 0x10000) - 0x5e00;
        break;
    case 0x1a:
        count = 0x800;
        table = (u8*)(base + 0x10000) - 0x7e00;
        break;
    case 0xe:
        count = 0x1fd0;
        table = &base[0x2c0];
        break;
    }
    if (index < 0 || index >= count)
    {
        debugPrintfxy(0x14, 0x28, sAssetIndexOverflowError);
        return 0;
    }
    if (table != NULL)
    {
        *out = ((int*)table)[index];
        return 1;
    }
    return 0;
}

struct MldfTables
{
    u8 pad0[0x160];
    int fileInfo[0x58];
    u8 mergeAnimCurv[0x7f40];
    u8 mergeVoxMap[0x2000];
    u8 mergeBlocks[0x2000];
    u8 mergeTex1[0x4000];
    u8 mergeTex0[0x4000];
    u8 mergeAnim[0x2ee0];
    u8 mergeModels[0x2000];
    u8 loadedFlags[0x58];
    int ids[0x58];
    int sizes[0x58];
    int romList[0x78];
    u32 ptrs[0x58];
    s16 owners[0x60];
};

#define MAPTBLP(idx)    (*(int**)(((idx) << 2) + ((u32)&((struct MldfTables*)base)->ptrs[0])))
#define MAPID_RT(s)    (*(int*)(((s) << 2) + ((u32) & tbl->ids[0])))
#define MAPPTR_RT(s)   (*(u32*)(((s) << 2) + ((u32) & tbl->ptrs[0])))
#define MAPOWNER_RT(s) (*(s16*)(((s) << 1) + ((u32) & tbl->owners[0])))


void* getCurrentDataFile(int id)
{
    u8* base = gResourceFileTable;
    switch (id)
    {
    case 42:
        return &base[0x170e0];
    case 47:
        return &base[0x14200];
    case 36:
        return &base[0x10200];
    case 33:
        return &base[0xc200];
    case 80:
        return *(void**)&base[0x19718];
    case 38:
        return &base[0xa200];
    case 26:
        return &base[0x8200];
    case 14:
        return &base[0x2c0];
    }
    return NULL;
}


int mapUnload(int mapId, int flags)
{
    struct MldfTables* tbl;
    int* e;
    int f20;
    int f10;
    u32 f80;
    int n;
    s32* lockp;
    u8 needWait;
    int i;
    int s;
    int j;
    int* st;

    tbl = (struct MldfTables*)gResourceFileTable;
    i = 0;
    needWait = 0;
    st = (int*)(*gMapEventInterface)->getCurCharPos();
    {
        int pairs[56] = {
            0x2b, 0x1,    0x2a, 0x2,    0x2f, 0x8,    0x30, 0x4,   0x46, 0x1,   0x45, 0x2,   0x49, 0x8,
            0x4a, 0x4,    0x24, 0x20,   0x23, 0x10,   0x4e, 0x20,  0x4d, 0x10,  0x21, 0x80,  0x20, 0x40,
            0x4c, 0x80,   0x4b, 0x40,   0x25, 0x100,  0x26, 0x200, 0x47, 0x100, 0x48, 0x200, 0x1b, 0x1000,
            0x1a, 0x2000, 0x54, 0x1000, 0x53, 0x2000, 0xd,  0x400, 0xe,  0x800, 0x55, 0x400, 0x56, 0x800,
        };

        while (s = OSDisableInterrupts(), n = gAssetLoadInFlightFlags, OSRestoreInterrupts(s), n != 0)
        {
            if (n == 0x100000)
            {
                break;
            }
            padUpdate();
            checkReset();
            if (needWait)
            {
                waitNextFrame();
            }
            loadDataFiles(0);
            dvdCheckError();
            if (needWait)
            {
                mmFreeTick(0);
                gameTextRun();
                GXFlush_(1, 0);
            }
            if (gDvdErrorPauseActive)
            {
                needWait = 1;
            }
        }

        st = (int*)(*gMapEventInterface)->getCurCharPos();
        {
            int v = *(s8*)((char*)st + 0xe);
            if (v != gObjLevelLockSlots[0] && v != gObjLevelLockSlots[1])
            {
                if ((flags & 0x10000000) && mapId != v)
                {
                    *((s8*)st + 0xe) = -1;
                }
                if ((flags & 0x20000000) && mapId == *((s8*)st + 0xe))
                {
                    *((s8*)st + 0xe) = -1;
                }
                if (flags & 0x80000000)
                {
                    *((s8*)st + 0xe) = -1;
                }
            }
        }

        e = pairs;
        f20 = flags & 0x20000000;
        f10 = flags & 0x10000000;
        f80 = flags & 0x80000000;
        lockp = gObjLevelLockSlots;
        for (; i < 0x38; i += 2)
        {
            if ((f20 && mapId == MAPID_RT(e[0])) || (f10 && mapId != MAPID_RT(e[0])) ||
                ((flags & e[1]) && mapId == MAPID_RT(e[0])))
            {
                MAPID_RT(e[0]) = -1;
            }
            {
                int idx = e[0];
                if (((int**)((char*)tbl + 0x20000 + -0x6A28))[idx] != NULL)
                {
                    s16 v;
                    if (f80 || ((flags & e[1]) && mapId == ((s16*)((char*)tbl + 0x20000 + -0x68C8))[idx]) ||
                        (f10 && mapId != MAPOWNER_RT(idx)) || (f20 && mapId == MAPOWNER_RT(idx)))
                    {
                        if (gObjLevelLockSlots[0] != (v = MAPOWNER_RT(idx)) && lockp[1] != v)
                        {
                            switch (idx)
                            {
                            case 0xe:
                            case 0x1a:
                            case 0x21:
                            case 0x24:
                            case 0x2a:
                            case 0x2b:
                            case 0x2f:
                            case 0x30:
                            case 0x45:
                            case 0x46:
                            case 0x49:
                            case 0x4a:
                            case 0x4c:
                            case 0x4e:
                            case 0x53:
                            case 0x56:
                                mmSetFreeDelay(0);
                                break;
                            case 0x20:
                            case 0x23:
                            case 0x4b:
                            case 0x4d:
                                mmSetFreeDelay(0);
                                break;
                            case 0x26:
                            case 0x48:
                                mmSetFreeDelay(0);
                                for (j = 0; j < 75; j++)
                                {
                                    if (sMapFileNameIndexRemapTable[j] == *(s16*)((u32)tbl + 0x20000 + (e[0] << 1) - 0x68C8))
                                    {
                                        break;
                                    }
                                }
                                if (j <= 0x50 && j != 0x49 && j != 0x43 && j != 5)
                                {
                                    u32* slot = (u32*)((j << 2) + ((u32)&tbl->romList[0]));
                                    mm_free((void*)*slot);
                                    *slot = 0;
                                }
                                break;
                            }
                            mm_free((void*)MAPPTR_RT(e[0]));
                            mmSetFreeDelay(2);
                            *(u32*)((e[0] << 2) + ((u32)tbl + 0x20000) - 0x6A28) = 0;
                            *(s16*)((e[0] << 1) + ((u32)tbl + 0x20000) - 0x68C8) = -1;
                            *(int*)((e[0] << 2) + ((u32)tbl + 0x20000) - 0x6D68) = 0;
                            switch (e[0])
                            {
                            case 0x2a:
                            case 0x45:
                                mergeTableFiles((u32*)tbl->mergeModels, 0x2a, 0x45, 0x800);
                                break;
                            case 0x2f:
                            case 0x49:
                                mergeTableFiles((u32*)tbl->mergeAnim, 0x2f, 0x49, 0xbb8);
                                break;
                            case 0x24:
                            case 0x4e:
                                mergeTableFiles((u32*)tbl->mergeTex0, 0x24, 0x4e, 0x1000);
                                break;
                            case 0x21:
                            case 0x4c:
                                mergeTableFiles((u32*)tbl->mergeTex1, 0x21, 0x4c, 0x1000);
                                break;
                            case 0x26:
                            case 0x48:
                                mergeTableFiles((u32*)tbl->mergeBlocks, 0x26, 0x48, 0x800);
                                break;
                            case 0x1a:
                            case 0x53:
                                mergeTableFiles((u32*)tbl->mergeVoxMap, 0x1a, 0x53, 0x800);
                                break;
                            case 0xe:
                            case 0x56:
                                mergeTableFiles((u32*)tbl->mergeAnimCurv, 0xe, 0x56, 0x1fd0);
                                break;
                            }
                        }
                    }
                }
            }
            e += 2;
        }
    }
    return 1;
}

int mergeTableFiles(void* table, int id, int idx, int count_)
{
    u32* tbl = table;
    u8* base = gResourceFileTable;
    int i = 0;
    int e1 = 0;
    int e2 = 0;
    int count = 0;
    int* p1;
    int* p2;
    int* src1 = MAPTBLP(id);
    if (src1 == NULL || MAPTBLP(idx) == NULL)
    {
        if (src1 == NULL)
        {
            e1 = 1;
        }
        if (MAPTBLP(idx) == NULL)
        {
            e2 = 1;
        }
    }
    p1 = (int*)(u32)src1;
    p2 = MAPTBLP(idx);
    if (tbl == (u32*)(base + 0x170e0))
    {
        count = 0x800;
    }
    else if (tbl == (u32*)(base + 0x14200))
    {
        count = 0xbb8;
    }
    else if (tbl == (u32*)(base + 0x10200))
    {
        count = 0x1000;
    }
    else if (tbl == (u32*)(base + 0xc200))
    {
        count = 0x1000;
    }
    else if (tbl == (u32*)(base + 0xa200))
    {
        count = 0x800;
    }
    else if (tbl == (u32*)(base + 0x8200))
    {
        count = 0x800;
    }
    else if (tbl == (u32*)(base + 0x2c0))
    {
        count = 0x1fd0;
    }
    if (tbl == (u32*)(base + 0x10200) || tbl == (u32*)(base + 0xc200))
    {
        int* w1 = p1;
        int* dst = (int*)tbl;
        int v;
        for (; count > 0; count--)
        {
            if (!e1 && *w1 == -1)
            {
                e1 = 1;
            }
            if (!e2 && *p2 == -1)
            {
                e2 = 1;
            }
            if (!e1 && (v = *w1, v != -1) && (v & 0x80000000))
            {
                *dst = v & 0x7fffffff;
                *dst = *dst | 0x40000000;
            }
            else if (!e2 && (v = *p2, v != -1) && (v & 0x80000000))
            {
                *dst = v;
            }
            else if (!e1 && *w1 != 0)
            {
                *dst = *w1;
            }
            else if (!e2 && *p2 != 0)
            {
                *dst = *p2;
            }
            else
            {
                *dst = 0;
            }
            w1++;
            p2++;
            dst++;
            i++;
        }
    }
    else if (tbl == (u32*)(base + 0xa200))
    {
        int* w1 = p1;
        int* dst = (int*)tbl;
        int* w2 = p2;
        int v;
        for (; count > 0; count--)
        {
            if (!e1 && (v = *w1, v != -1) && (v & 0x10000000))
            {
                *dst = v;
                if (p2 != NULL && *w2 == -1)
                {
                    e2 = 1;
                }
            }
            else if (!e2 && (v = *w2, v != -1) && (v & 0x10000000))
            {
                *dst = (v & 0xffffff) | 0x20000000;
                if (p1 != NULL && *w1 == -1)
                {
                    e1 = 1;
                }
            }
            else if (!e1 && *w1 == -1)
            {
                *dst = 0;
                e1 = 1;
            }
            else if (!e2 && *w2 == -1)
            {
                *dst = 0;
                e2 = 1;
            }
            else if (!e1 && *w1 != 0)
            {
                *dst = *w1;
            }
            else if (!e2 && *w2 != 0)
            {
                *dst = *w2;
            }
            else
            {
                *dst = 0;
            }
            w1++;
            dst++;
            w2++;
            i++;
        }
    }
    else if (tbl == (u32*)(base + 0x8200))
    {
        int* w1 = p1;
        int* dst = (int*)tbl;
        int v;
        for (; count > 0; count--)
        {
            if (!e1 && *w1 == -1)
            {
                *dst = 0;
                e1 = 1;
            }
            else if (!e2 && *p2 == -1)
            {
                *dst = 0;
                e2 = 1;
            }
            else if (!e1 && (v = *w1, v != -1) && (v & 0x80000000))
            {
                *dst = v;
            }
            else if (!e2 && (v = *p2, v != -1) && (v & 0x80000000))
            {
                *dst = (v & 0x7fffffff) | 0x20000000;
            }
            else if (!e1 && *w1 != 0)
            {
                *dst = *w1;
            }
            else if (!e2 && *p2 != 0)
            {
                *dst = *p2;
            }
            else
            {
                *dst = 0;
            }
            w1++;
            dst++;
            p2++;
            i++;
        }
    }
    else if (tbl == (u32*)(base + 0x2c0))
    {
        int* w1 = p1;
        int* dst = (int*)tbl;
        int v;
        for (; count > 0; count--)
        {
            if (!e1 && *w1 == -1)
            {
                *dst = 0;
                e1 = 1;
            }
            else if (!e2 && *p2 == -1)
            {
                *dst = 0;
                e2 = 1;
            }
            else if (!e1 && (v = *w1, v != -1) && (v & 0x80000000))
            {
                *dst = v;
            }
            else if (!e2 && (v = *p2, v != -1) && (v & 0x80000000))
            {
                *dst = (v & 0x7fffffff) | 0x20000000;
            }
            else if (!e1 && *w1 != 0)
            {
                *dst = *w1;
            }
            else if (!e2 && *p2 != 0)
            {
                *dst = *p2;
            }
            else
            {
                *dst = 0;
            }
            w1++;
            dst++;
            p2++;
            i++;
        }
    }
    else
    {
        int* w1 = p1;
        int* w2 = p2;
        int* dst = (int*)tbl;
        int v;
        for (; count > 0; count--)
        {
            if (!e1 && *w1 == -1)
            {
                e1 = 1;
            }
            if (!e2 && *w2 == -1)
            {
                e2 = 1;
            }
            if (!e1 && (v = *w1, v != -1) && (v & 0x10000000))
            {
                *dst = v;
            }
            else if (!e2 && (v = *w2, v != -1) && (v & 0x10000000))
            {
                *dst = (v & 0xffffff) | 0x20000000;
            }
            else if (!e1 && p1 != NULL)
            {
                *dst = *w1;
            }
            else if (!e2 && p2 != NULL)
            {
                *dst = *w2;
            }
            else
            {
                *dst = 0;
            }
            w1++;
            w2++;
            dst++;
            i++;
        }
    }
    {
        int last = i - 1;
        tbl[last] = 0xffffffff;
    }
    return 1;
}
#undef MAPTBLP

s32 mapCheckCurBlocks(int v)
{
    return mapCheckCurBlocksImpl(v);
}


f32 gObjJointMtxTemp[24] = {
    1.0f,         0.0f,           0.0f,           0.0f,           0.0f,           1.0f,
    0.0f,         0.0f,           0.0f,           0.0f,           1.0f,           0.0f,
    0.014794691f, 1.6930165e+22f, 2.5424896e+29f, 4.6243438e+30f, 1.6713787e-19f, 3.5253297e+09f,
    13.204376f,   1.8988991e+28f, 2.818281e+20f,  4.2326e+21f,    0.03909816f,    6.162976e-33f,
};
