#define TEX_SETSHADER_U8
#include "main/tex_dolphin.h"
#include "main/texture.h"
#include "main/shader_api.h"
#include "main/map_block.h"
#include "main/sky_api.h"
#include "main/game_object.h"
#include "main/rcp_dolphin_api.h"
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
#include "track/intersect_fog_api.h"
#include "main/model_light.h"
#include "main/pi_dolphin.h"
#include "main/newshadows.h"
#include "main/objseq_api.h"
#include "main/dll/FRONT/n_options.h"
#include "main/lightmap_render_queue_api.h"

u8 lbl_803DB638[4] = {0x20, 0x20, 0x20, 0};
int gTexShaderAmbColor = -1;
int gTexLightmapAmbColor = -1;
s8 gTexIndMtxScaleExp = -2;

extern f32 lbl_803DEBCC;
extern const f32 lbl_803DEBFC;
extern f32 lbl_803DEC20;
extern const f32 gTexIndMtxScale;
extern f32 lbl_803DEC28;
extern const f32 lbl_803DEC2C;
extern int lbl_803DEBB0;
extern ModelLightStruct* gTexDimmedLightList[2];
extern ModelLightStruct* gTexBlockLightList[2];
extern int lbl_803DCE30;
extern int* lbl_803DCE34;
extern int gTexIndMtxTable[];
extern u8 lbl_8037E0C0[];
extern int lbl_80382008[5];
#define FRUSTUM_PLANE_COUNT 5
extern FrustumPlane gViewFrustumPlanes[FRUSTUM_PLANE_COUNT];
extern int gTexShaderFogColor;
extern int gTexLightmapFogColor;

/*
 * TexShadowRow - 0x10-stride rows of the pending-shadow queue at the head of
 * lbl_8037E0C0 (indexed by lbl_803DCE30, bumped after each fn_8005D3B4 push).
 * mapBlockRender_callList writes type (4/5 = object shadow, 6 = indirect
 * lightmap) into the queued shadow row.
 */
typedef struct TexShadowRow
{
    int unk0;
    int unk4;
    int unk8;
    int type;
} TexShadowRow;

u8 mapBlockBounds_HasCornerPastDepthThreshold(MapBlockBoundsRec* bounds, float* xform)
{
    float v[3];
    u32 i;
    f32 fbset;
    f32 timing;

    i = 0;
    timing = lbl_803DEC20;
    fbset = lbl_803DEC28;
    while (1)
    {
        {
            switch (i)
            {
            case 0:
                v[0] = (f32)bounds->minX;
                v[1] = (f32)bounds->minY;
                v[2] = (f32)bounds->minZ;
                break;
            case 1:
                v[0] = (f32)bounds->maxX;
                v[1] = (f32)bounds->minY;
                v[2] = (f32)bounds->minZ;
                break;
            case 2:
                v[0] = (f32)bounds->minX;
                v[1] = (f32)bounds->maxY;
                v[2] = (f32)bounds->minZ;
                break;
            case 3:
                v[0] = (f32)bounds->maxX;
                v[1] = (f32)bounds->maxY;
                v[2] = (f32)bounds->minZ;
                break;
            case 4:
                v[0] = (f32)bounds->minX;
                v[1] = (f32)bounds->minY;
                v[2] = (f32)bounds->maxZ;
                break;
            case 5:
                v[0] = (f32)bounds->maxX;
                v[1] = (f32)bounds->minY;
                v[2] = (f32)bounds->maxZ;
                break;
            case 6:
                v[0] = (f32)bounds->minX;
                v[1] = (f32)bounds->maxY;
                v[2] = (f32)bounds->maxZ;
                break;
            case 7:
                v[0] = (f32)bounds->maxX;
                v[1] = (f32)bounds->maxY;
                v[2] = (f32)bounds->maxZ;
                break;
            }
        }
        v[0] = v[0] * timing;
        v[1] = v[1] * timing;
        v[2] = v[2] * timing;
        PSMTXMultVec((const float (*)[4])xform, (Vec*)v, (Vec*)v);
        if (v[2] >= fbset)
        {
            return 1;
        }
        i = i + 1;
        if ((int)i < 8)
        {
            continue;
        }
        return 0;
    }
}

typedef struct IndMtxCopy
{
    int w[6];
} IndMtxCopy;

#define SHADER_FLAGS(s) ((s)->flags)

void mapBlockRender_drawLightmapIndirectPasses(struct MapBlockData* blockData, MapShader* shader,
                                               ModelRenderInstrsState* state, Mtx viewMtx)
{
    Mtx passMtx;
    float indMtx[2][3];
    int noiseFrameCount;
    Texture** noiseTextures;
    MapBlockBoundsRec* bounds;
    u8 passCount;
    int byteBase;
    u32 bits;
    int bitPos;
    u32 flags;
    u8* mtxSrc;
    int i;

    bitPos = state->bit;
    {
        int off = bitPos >> 3;
        byteBase = (int)state->instrs;
        bits = *(u8*)(byteBase + off);
        byteBase += off;
        bits = bits | (u32)(*(u8*)(byteBase + 1) << 8);
        bits = bits | (u32)(*(u8*)(byteBase + 2) << 16);
    }
    state->bit = bitPos + 8;
    /* extract this cursor's 8-bit field (LSB-first: shift out the bits already
     * consumed within the byte, then mask the width) -> bounds-record index */
    bounds = &blockData->displayLists[(bits >> (bitPos & 7)) & 0xff];
    flags = SHADER_FLAGS(shader);
    if ((flags & 0x4000) != 0)
    {
        passCount = 4;
    }
    else if ((flags & 0x8000) != 0)
    {
        passCount = 8;
    }
    else if ((flags & 0x10000) != 0)
    {
        passCount = 0x10;
    }
    else
    {
        return;
    }
    i = 0;
    for (; i < passCount; i = i + 1)
    {
        PSMTXTrans(passMtx, 0.0f, lbl_803DEC2C * (f32)(i + 1), 0.0f);
        PSMTXConcat(viewMtx, passMtx, passMtx);
        GXLoadPosMtxImm(passMtx, GX_PNMTX0);
        mtxSrc = (u8*)gTexIndMtxTable;
        *(IndMtxCopy*)indMtx = *(IndMtxCopy*)mtxSrc;
        getNewShadowNoiseTextureFrames(&noiseTextures, &noiseFrameCount);
        selectTexture(noiseTextures[(u8)i], 1);
        {
            f32 s = (f32)((i & 0xff) + 1) * gTexIndMtxScale;
            indMtx[0][0] = s * lbl_803DEBFC;
        }
        indMtx[1][1] = indMtx[0][0];
        GXSetIndTexMtx(GX_ITM_0, (const float (*)[3])indMtx, gTexIndMtxScaleExp);
        GXCallDisplayList(bounds->dlist, bounds->dlistSize);
    }
}

MapShader* mapBlockRender_setLightmapShader(struct MapBlockData* blockData, ModelRenderInstrsState* state)
{
    MapShader* shader;
    u32 shaderIdx;
    int byteBase;
    int fogColor;
    u32 bits;
    u32 bitPos;
    u8 ambColor[3];

    fogColor = gTexLightmapFogColor;
    bitPos = state->bit;
    {
        int off = (int)bitPos >> 3;
        byteBase = (int)state->instrs;
        bits = *(u8*)(byteBase + off);
        byteBase += off;
        bits |= (u32) * (u8*)(byteBase + 1) << 8;
        bits |= (u32) * (u8*)(byteBase + 2) << 16;
        state->bit = bitPos + 6;
        shaderIdx = (bits >> (bitPos & 7)) & 0x3f;
        shader = &blockData->shaders[shaderIdx];
    }
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
    selectTexture(((MapShaderLayer*)Shader_getLayer(shader, 0))->texture, 0);
    if ((SHADER_FLAGS(shader) & 4) != 0)
    {
        _gxSetFogParams();
    }
    else
    {
        GXSetFog(GX_FOG_NONE, 0.0f, 0.0f, 0.0f, 0.0f, *(GXColor*)&fogColor);
    }
    if ((SHADER_FLAGS(shader) & 1) != 0 || (SHADER_FLAGS(shader) & 0x40000) != 0 ||
        (SHADER_FLAGS(shader) & 0x800) != 0 || (SHADER_FLAGS(shader) & 0x1000) != 0)
    {
        GXSetChanAmbColor(GX_COLOR0, *(GXColor*)&gTexLightmapAmbColor);
        if ((SHADER_FLAGS(shader) & 0x40000) != 0)
        {
            GXSetChanCtrl(GX_COLOR0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        }
        else
        {
            GXSetChanCtrl(GX_COLOR0, GX_ENABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        }
    }
    else
    {
        objGetColor(0, &ambColor[0], &ambColor[1], &ambColor[2]);
        GXSetChanCtrl(GX_COLOR0, GX_ENABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        GXSetChanAmbColor(GX_COLOR0, *(GXColor*)&ambColor[0]);
    }
    return shader;
}

void mapBlockRender_drawDimmedAabbLights(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx)
{
    ModelLightStruct** lightPtr;
    f32 posZ;
    f32 posY;
    f32 posX;
    int lightCount;
    u8 colorA;
    u8 colorB;
    u8 colorG;
    u8 colorR;
    int i;

    {
        f32 fz = *(f32*)&playerMapOffsetZ;
        f32 fldZ = block->transform[2][3];
        f32 fldY = block->transform[1][3];
        f32 fx = *(f32*)&playerMapOffsetX;
        f32 fldX = block->transform[0][3];
        f32 ax0 = (f32)(bounds->minX >> 3) + fldX;
        f32 az0 = (f32)(bounds->minZ >> 3) + fldZ;
        f32 ax1 = (f32)(bounds->maxX >> 3) + fldX;
        f32 az1 = (f32)(bounds->maxZ >> 3) + fldZ;
        modelLightStruct_selectBrightestAabbLights(ax0 + fx, (f32)(bounds->minY >> 3) + fldY, az0 + fz, ax1 + fx,
                                                   (f32)(bounds->maxY >> 3) + fldY, az1 + fz,
                                                   gTexDimmedLightList, 2, &lightCount);
    }
    Rcp_ResetTextureStageState();
    fn_8004CE0C(viewMtx);
    i = 0;
    lightPtr = gTexDimmedLightList;
    {
        u8* pColorA = &colorA;
        u8* pColorB = &colorB;
        u8* pColorG = &colorG;
        f32* pPosZ = &posZ;
        f32* pPosY = &posY;
        for (; i < lightCount; lightPtr = lightPtr + 1, i = i + 1)
        {
            modelLightStruct_getDiffuseColor(*lightPtr, &colorR, pColorG, pColorB, pColorA);
            colorR = ((int)colorR >> 1) + ((int)colorR >> 2);
            colorG = ((int)colorG >> 1) + ((int)colorG >> 2);
            colorB = ((int)colorB >> 1) + ((int)colorB >> 2);
            modelLightStruct_getPosition(*lightPtr, &posX, pPosY, pPosZ);
            fn_8004FA30(modelLightStruct_getRadius(*lightPtr), (int*)&colorR, &posX);
        }
    }
    Rcp_ApplyTextureStageCounts();
    GXSetNumChans(1);
    GXSetCullMode(GX_CULL_BACK);
    gxSetZMode_(1, GX_LEQUAL, 0);
    gxSetPeControl_ZCompLoc_(1);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    return;
}

u32 frustumTestAabbWithPlaneOffsets(f32 minX, f32 maxX, f32 minY, f32 maxY, f32 minZ, f32 maxZ, f32* planeOffsets)
{
    FrustumPlane* plane;
    int cornerIndex;
    int i;
    float nearX;
    float nearY;
    float nearZ;
    float farX;
    float farY;
    float farZ;

    plane = gViewFrustumPlanes;
    for (i = 0; i < FRUSTUM_PLANE_COUNT; i++)
    {
        cornerIndex = plane[i].aabbCornerIndex;
        if ((cornerIndex & 1) != 0)
        {
            nearX = maxX;
            farX = minX;
        }
        else
        {
            nearX = minX;
            farX = maxX;
        }
        if ((cornerIndex & 2) != 0)
        {
            nearY = maxY;
            farY = minY;
        }
        else
        {
            nearY = minY;
            farY = maxY;
        }
        if ((cornerIndex & 4) != 0)
        {
            nearZ = maxZ;
            farZ = minZ;
        }
        else
        {
            nearZ = minZ;
            farZ = maxZ;
        }
        if ((nearX * plane[i].normalX + nearY * plane[i].normalY + nearZ * plane[i].normalZ + plane[i].distance +
                 planeOffsets[i] <
             lbl_803DEBCC) &&
            (farX * plane[i].normalX + farY * plane[i].normalY + farZ * plane[i].normalZ + plane[i].distance +
                 planeOffsets[i] <
             lbl_803DEBCC))
            return 0;
    }
    return 1;
}

u8 mapBlockBounds_ComputeAndTestPlanes(MapBlockBoundsRec* bounds, struct MapBlockData* block,
                                       FrustumPlane* planes, int planeCount, f32* minX, f32* minY, f32* minZ,
                                       f32* maxX, f32* maxY, f32* maxZ)
{
    u8 cornerIndex;
    float nearX;
    float nearY;
    float nearZ;
    float farX;
    float farY;
    float farZ;
    int i;
    *maxX = (f32)(bounds->maxX >> 3) + block->transform[0][3];
    *minX = (f32)(bounds->minX >> 3) + block->transform[0][3];
    *maxY = (f32)(bounds->maxY >> 3) + block->transform[1][3];
    *minY = (f32)(bounds->minY >> 3) + block->transform[1][3];
    *maxZ = (f32)(bounds->maxZ >> 3) + block->transform[2][3];
    *minZ = (f32)(bounds->minZ >> 3) + block->transform[2][3];
    for (i = 0; i < planeCount; i = i + 1)
    {
        cornerIndex = planes->aabbCornerIndex;
        if ((cornerIndex & 1) != 0)
        {
            nearX = *maxX;
            farX = *minX;
        }
        else
        {
            nearX = *minX;
            farX = *maxX;
        }
        if ((cornerIndex & 2) != 0)
        {
            nearY = *maxY;
            farY = *minY;
        }
        else
        {
            nearY = *minY;
            farY = *maxY;
        }
        if ((cornerIndex & 4) != 0)
        {
            nearZ = *maxZ;
            farZ = *minZ;
        }
        else
        {
            nearZ = *minZ;
            farZ = *maxZ;
        }
        if ((planes->distance + (nearX * planes->normalX + nearY * planes->normalY + nearZ * planes->normalZ) <
             lbl_803DEBCC) &&
            (planes->distance + (farX * planes->normalX + farY * planes->normalY + farZ * planes->normalZ) <
             lbl_803DEBCC))
        {
            return 0;
        }
        planes++;
    }
    return 1;
}

void mapBlockRender_callList(u32 passSelect, u32 visArg, MapBlockData* block, MapShader* shader,
                             ModelRenderInstrsState* state, float* mtx)
{
    int lightPos[3];
    int count;
    float minX;
    float minY;
    float minZ;
    float maxX;
    float maxY;
    float maxZ;
    u8 lightColor[4];
    GXColor chanColor;
    int i;
    u32 visible;
    u32 flags;
    u32 bits;
    int bitPos;
    int byteBase;

    {
        u8* texGlobals;
        MapBlockBoundsRec* bounds;

        texGlobals = lbl_8037E0C0;
        bitPos = state->bit;
        {
            int off = bitPos >> 3;
            byteBase = (int)state->instrs;
            bits = *(u8*)(byteBase + off);
            byteBase += off;
            bits = bits | (u32)(*(u8*)(byteBase + 1) << 8);
            bits = bits | (u32)(*(u8*)(byteBase + 2) << 16);
        }
        state->bit = bitPos + 8;
        bounds = &block->displayLists[(bits >> (bitPos & 7)) & 0xff];
        if ((shader != NULL) && ((SHADER_FLAGS(shader) & 2) != 0))
        {
            return;
        }
        if (mapBlockBounds_ComputeAndTestPlanes(bounds, block, (FrustumPlane*)(texGlobals + 0x987c),
                                                FRUSTUM_PLANE_COUNT, &minX, &minY, &minZ, &maxX, &maxY, &maxZ) == 0)
        {
            return;
        }
        if ((u8)passSelect == 0)
        {
            flags = SHADER_FLAGS(shader);
            if ((flags & 0x80000000) != 0)
            {
                fn_8005D3B4(bounds, block, bounds->selector);
                ((TexShadowRow*)texGlobals)[lbl_803DCE30].type = 5;
                lbl_803DCE30 = lbl_803DCE30 + 1;
            }
            else if (((flags & 0x40000000) != 0) || ((flags & 0x2000) != 0))
            {
                fn_8005D3B4(bounds, block, bounds->selector);
                ((TexShadowRow*)texGlobals)[lbl_803DCE30].type = 4;
                lbl_803DCE30 = lbl_803DCE30 + 1;
            }
        }
        else
        {
            if (shader != NULL)
            {
                flags = SHADER_FLAGS(shader);
                if (((flags & 0x80000000) == 0) && ((flags & 0x20000) == 0))
                {
                    if ((shader != NULL) && ((flags & 0x80000) != 0))
                    {
                        count = 0;
                    }
                    else
                    {
                        modelLightStruct_selectBrightestAabbLights(
                            minX + playerMapOffsetX, minY, minZ + playerMapOffsetZ, maxX + playerMapOffsetX, maxY,
                            maxZ + playerMapOffsetZ, gTexBlockLightList, 2, &count);
                    }
                    if ((shader != NULL) &&
                        (((SHADER_FLAGS(shader) & 0x800) != 0 || ((SHADER_FLAGS(shader) & 0x1000) != 0))))
                    {
                        ObjSeq_copyDefaultColor(&chanColor);
                        chanColor.a = 0;
                        chanColor.b = 0;
                        chanColor.g = 0;
                        chanColor.r = 0;
                        if (count == 0)
                        {
                            if ((shader != NULL) && ((SHADER_FLAGS(shader) & 0x800) != 0))
                            {
                                fn_8004EF9C((int*)&chanColor);
                            }
                            else
                            {
                                fn_8004EECC((u8*)&chanColor);
                            }
                        }
                        else
                        {
                            modelLightStruct_getDiffuseColor(gTexBlockLightList[0], &lightColor[0], &lightColor[1],
                                                             &lightColor[2], &lightColor[3]);
                            modelLightStruct_getPosition(gTexBlockLightList[0], (f32*)&lightPos[0],
                                                         (f32*)&lightPos[1], (f32*)&lightPos[2]);
                            fn_8004F6D8(modelLightStruct_getRadius(gTexBlockLightList[0]),
                                        (int*)lightColor, (f32*)&lightPos[0], (u8*)&chanColor);
                            for (i = 1; i < count; i = i + 1)
                            {
                                modelLightStruct_getDiffuseColor(gTexBlockLightList[i], &lightColor[0],
                                                                 &lightColor[1], &lightColor[2], &lightColor[3]);
                                modelLightStruct_getPosition(gTexBlockLightList[i], (f32*)&lightPos[0],
                                                             (f32*)&lightPos[1], (f32*)&lightPos[2]);
                                fn_8004F380(modelLightStruct_getRadius(gTexBlockLightList[i]),
                                            (int*)lightColor, (f32*)&lightPos[0]);
                            }
                            if ((shader != NULL) && ((SHADER_FLAGS(shader) & 0x800) != 0))
                            {
                                fn_8004F2B0();
                            }
                            else
                            {
                                fn_8004F080();
                            }
                        }
                    }
                    else
                    {
                        for (i = 0; i < count; i = i + 1)
                        {
                            modelLightStruct_getDiffuseColor(gTexBlockLightList[i], &lightColor[0],
                                                             &lightColor[1], &lightColor[2], &lightColor[3]);
                            modelLightStruct_getPosition(gTexBlockLightList[i], (f32*)&lightPos[0],
                                                         (f32*)&lightPos[1], (f32*)&lightPos[2]);
                            fn_8004FA30(modelLightStruct_getRadius(gTexBlockLightList[i]),
                                        (int*)lightColor, (f32*)&lightPos[0]);
                        }
                    }
                    if ((shader != NULL) && ((SHADER_FLAGS(shader) & 0x2000) != 0))
                    {
                        if ((shader != NULL) && ((SHADER_FLAGS(shader) & 0x40000000) != 0))
                        {
                            visible = visArg;
                        }
                        else
                        {
                            u8 mirrorVisible = mapBlockBounds_ComputeAndTestPlanes(
                                bounds, block, (FrustumPlane*)(texGlobals + 0x9818), FRUSTUM_PLANE_COUNT, &minX, &minY,
                                &minZ, &maxX, &maxY, &maxZ);
                            if ((mirrorVisible != 0 && (u8)visArg != 0) || (mirrorVisible == 0 && (u8)visArg == 0))
                            {
                                visible = 1;
                            }
                            else
                            {
                                visible = 0;
                            }
                            if ((u8)visArg != 0)
                            {
                                GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
                                gxSetZMode_(1, GX_LEQUAL, 0);
                                gxSetPeControl_ZCompLoc_(1);
                                GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
                            }
                        }
                        if ((u8)visible == 0)
                        {
                            return;
                        }
                        fn_8004D230();
                    }
                    Rcp_ApplyTextureStageCounts();
                }
            }
            GXCallDisplayList(bounds->dlist, bounds->dlistSize);
            flags = SHADER_FLAGS(shader);
            if ((((flags & 0x4000) != 0) || ((flags & 0x8000) != 0) || ((flags & 0x10000) != 0)) &&
                (mapBlockBounds_HasCornerPastDepthThreshold(bounds, mtx) != 0))
            {
                fn_8005D3B4(bounds, block, 0x17);
                ((TexShadowRow*)texGlobals)[lbl_803DCE30].type = 6;
                lbl_803DCE30 = lbl_803DCE30 + 1;
            }
        }
    }
}

void mapBlockRender_setupShaderTextures(MapShader* shader, int mode)
{
    int layerIdx;
    MapShaderLayer* layer;
    Texture* texture;
    MtxPtr texMtx;
    int overrideIdx;
    int remain;
    MapTextureOverride* overrideEntry;
    u8 layerByte;
    u32 kColor;
    f32 tx;
    Mtx texMatrix;

    kColor = lbl_803DEBB0;
    if (shader->layerCount == 2 &&
        (((layer = Shader_getLayer(shader, 1))->typeBits & 0x7f) == 9))
    {
        layer = Shader_getLayer(shader, 0);
        {
            u8 overrideType;
            if ((overrideType = layer->overrideType) != 0)
            {
                Texture* layerTexture = layer->texture;
                MapTextureOverride* overrides;
                overrideIdx = 0;
                overrides = lbl_803DCE6C;
                overrideEntry = overrides;
                for (remain = 80; remain != 0 || (texture = layerTexture, 0); remain--)
                {
                    if (overrideEntry->refCount > 0 && overrideEntry->texture == layerTexture &&
                        overrideEntry->type == overrideType)
                    {
                        texture = textureGetAnimationFrame(layerTexture, overrides[overrideIdx].frame);
                        break;
                    }
                    overrideEntry++;
                    overrideIdx++;
                }
            }
            else
            {
                texture = layer->texture;
            }
        }
        if (layer->scrollMtx != 0xff)
        {
            tx = lbl_803DCE68[layer->scrollMtx].offsetX / 1048576.0f;
            PSMTXTrans(texMatrix, tx, lbl_803DCE68[layer->scrollMtx].offsetY / 1048576.0f,
                       lbl_803DEBCC);
            texMtx = texMatrix;
        }
        else
        {
            texMtx = NULL;
        }
        fn_80051B00(texture, texMtx, 0, (GXColor*)&kColor);
        if (shader->flags & 0x100)
        {
            fn_8004D928();
        }
        layer = Shader_getLayer(shader, 1);
        {
            u8 overrideType;
            if ((overrideType = layer->overrideType) != 0)
            {
                Texture* layerTexture = layer->texture;
                MapTextureOverride* overrides;
                overrideIdx = 0;
                overrides = lbl_803DCE6C;
                overrideEntry = overrides;
                for (remain = 80; remain != 0 || (texture = layerTexture, 0); remain--)
                {
                    if (overrideEntry->refCount > 0 && overrideEntry->texture == layerTexture &&
                        overrideEntry->type == overrideType)
                    {
                        texture = textureGetAnimationFrame(layerTexture, overrides[overrideIdx].frame);
                        break;
                    }
                    overrideEntry++;
                    overrideIdx++;
                }
            }
            else
            {
                texture = layer->texture;
            }
        }
        if (layer->scrollMtx != 0xff)
        {
            tx = lbl_803DCE68[layer->scrollMtx].offsetX / 1048576.0f;
            PSMTXTrans(texMatrix, tx, lbl_803DCE68[layer->scrollMtx].offsetY / 1048576.0f,
                       lbl_803DEBCC);
            texMtx = texMatrix;
        }
        else
        {
            texMtx = NULL;
        }
        fn_80051868(texture, texMtx, 9);
        textureFn_800524ec((GXColor*)&kColor);
    }
    else
    {
        for (layerIdx = 0; layerIdx < (int)(u32)shader->layerCount; layerIdx = layerIdx + 1)
        {
            layer = Shader_getLayer(shader, layerIdx);
            texture = layer->texture;
            if (texture != NULL)
            {
                {
                    u8 overrideType;
                    if ((overrideType = layer->overrideType) != 0)
                    {
                        Texture* layerTexture = layer->texture;
                        MapTextureOverride* overrides;
                        overrideIdx = 0;
                        overrides = lbl_803DCE6C;
                        overrideEntry = overrides;
                        for (remain = 80; remain != 0 || (texture = layerTexture, 0); remain--)
                        {
                            if (overrideEntry->refCount > 0 && overrideEntry->texture == layerTexture &&
                                overrideEntry->type == overrideType)
                            {
                                texture = textureGetAnimationFrame(layerTexture, overrides[overrideIdx].frame);
                                break;
                            }
                            overrideEntry++;
                            overrideIdx++;
                        }
                    }
                    if (layer->scrollMtx != 0xff)
                    {
                        tx = lbl_803DCE68[layer->scrollMtx].offsetX / 1048576.0f;
                        PSMTXTrans(texMatrix, tx, lbl_803DCE68[layer->scrollMtx].offsetY / 1048576.0f,
                                   lbl_803DEBCC);
                        texMtx = texMatrix;
                    }
                    else
                    {
                        texMtx = NULL;
                    }
                    layerByte = layer->typeBits & 0x7f;
                    if (shader->flags & 0x40000)
                    {
                        fn_80051528(texture, texMtx);
                    }
                    else
                    {
                        fn_80051868(texture, texMtx, layerByte);
                    }
                }
            }
            else
            {
                gxColorFn_800523d0();
            }
        }
        if (shader->flags & 0x100)
        {
            fn_8004D928();
        }
    }
}

MapShader* mapBlockRender_setShader(u8 doSetup, MapBlockData* blockData, ModelRenderInstrsState* state)
{
    MapShader* shader;
    u32 shaderIdx;
    int fogColor;
    int byteBase;
    u32 flags;
    int* lightList;
    u8 ambColor[3];
    u8 fogRgba[4];
    u32 bits;
    u32 bitPos;

    fogColor = gTexShaderFogColor;
    bitPos = state->bit;
    {
        int off = (int)bitPos >> 3;
        byteBase = (int)state->instrs;
        bits = *(u8*)(byteBase + off);
        byteBase += off;
        bits |= (u32) * (u8*)(byteBase + 1) << 8;
        bits |= (u32) * (u8*)(byteBase + 2) << 16;
        state->bit = bitPos + 6;
        shaderIdx = (bits >> (bitPos & 7)) & 0x3f;
        shader = &blockData->shaders[shaderIdx];
    }

    if (doSetup == 0)
    {
        return shader;
    }

    if ((SHADER_FLAGS(shader) & 4) != 0)
    {
        _gxSetFogParams();
    }
    else
    {
        GXSetFog(GX_FOG_NONE, 0.0f, 0.0f, 0.0f, 0.0f, *(GXColor*)&fogColor);
    }
    if ((shader != 0) && ((SHADER_FLAGS(shader) & 0x80000000) != 0))
    {
        return shader;
    }
    if ((shader != 0) && ((SHADER_FLAGS(shader) & 0x20000) != 0))
    {
        u32 res;
        res = AttractMovie_DrawTextureCallback(0, 0, 0);
        if ((res & 0xff) != 0)
        {
            return shader;
        }
    }
    Rcp_ResetTextureStageState();
    if ((SHADER_FLAGS(shader) & 0x80) != 0)
    {
        fn_8004DA54((char*)shader);
    }
    else
    {
        mapBlockRender_setupShaderTextures(shader, 0x80);
    }
    flags = SHADER_FLAGS(shader);
    if ((flags & 0x20) != 0 && (lightList = lbl_803DCE34) != 0)
    {
        fn_8004FDA0((u8*)lightList, &lbl_80382008, lbl_803DB638);
    }
    else if ((flags & 0x40) != 0)
    {
        fn_8004E0FC();
    }
    else if (isHeavyFogEnabled())
    {
        getColor803dd01c(fogRgba);
        renderHeavyFog(fogRgba);
    }
    if (((SHADER_FLAGS(shader) & 0x40000000) != 0) || ((SHADER_FLAGS(shader) & 0x20000000) != 0))
    {
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        gxSetZMode_(1, GX_LEQUAL, 0);
        gxSetPeControl_ZCompLoc_(1);
        GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    }
    else if ((SHADER_FLAGS(shader) & 0x400) != 0 && (SHADER_FLAGS(shader) & 0x80) == 0)
    {
        GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
        gxSetZMode_(1, GX_LEQUAL, 1);
        gxSetPeControl_ZCompLoc_(0);
        GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_GREATER, 0);
    }
    else
    {
        GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
        gxSetZMode_(1, GX_LEQUAL, 1);
        gxSetPeControl_ZCompLoc_(1);
        GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    }
    if ((SHADER_FLAGS(shader) & 1) != 0 || (SHADER_FLAGS(shader) & 0x40000) != 0 ||
        (SHADER_FLAGS(shader) & 0x800) != 0 || (SHADER_FLAGS(shader) & 0x1000) != 0)
    {
        GXSetChanAmbColor(GX_COLOR0, *(GXColor*)&gTexShaderAmbColor);
        if ((SHADER_FLAGS(shader) & 0x40000) != 0)
        {
            GXSetChanCtrl(GX_COLOR0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        }
        else
        {
            GXSetChanCtrl(GX_COLOR0, GX_ENABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        }
    }
    else
    {
        objGetColor(0, &ambColor[0], &ambColor[1], &ambColor[2]);
        GXSetChanCtrl(GX_COLOR0, GX_ENABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        GXSetChanAmbColor(GX_COLOR0, *(GXColor*)&ambColor[0]);
    }
    if ((SHADER_FLAGS(shader) & 0x8) != 0)
    {
        GXSetCullMode(GX_CULL_BACK);
    }
    else
    {
        GXSetCullMode(GX_CULL_NONE);
    }
    return shader;
}
