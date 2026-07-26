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

extern int gNewShadowHeavyFogTexture;
extern u8 lbl_803DCF80;
extern f32 lbl_803DED08;

#define READ_TEXTURE_U16(address) (*(u16*)(address))
#define WRITE_TEXTURE_U16(address, value) (*(u16*)(address) = (value))

void blendTextures(Texture* src1, Texture* src2, f32 blend, Texture* dst)
{
    u32 fmt;
    u32 w;
    u32 h;
    u32 wA;
    u32 wB;
    int pixelB;
    int pixelA;
    int redB;
    int redA;
    int blue;
    int red;
    int green;
    u16 outputPixel;

    if (src1 == NULL)
        return;
    if (src2 == NULL)
        return;
    if (dst == NULL)
        return;
    fmt = src1->format;
    if (fmt != GX_TF_RGB565 && fmt != GX_TF_RGBA8)
        return;
    if (src2->format != fmt)
        return;
    if (dst->format != fmt)
        return;
    w = src1->width;
    if (w != src2->width)
        return;
    h = src1->height;
    if (h != src2->height)
        return;
    if (w != dst->width || h != dst->height)
    {
        return;
    }
    {
        wA = (int)(lbl_803DED08 * blend) & 0xff;
        wB = (0xff - wA) & 0xff;
        if (fmt == GX_TF_RGB565)
        {
            int i, j;
            for (i = 0; i < src1->height; i++)
            {
                u8 *pa, *pb, *pc;
                u32 wd;
                int rowDataOffset;
                int tileColumnOffset;
                int pixelColumnOffset;
                j = 0;
                w = i & 0xfffffffc;
                h = (i & 3) * 8;
                for (; j < (int)(wd = src1->width); j++)
                {
                    pixelColumnOffset = (j & 3) * 2;
                    pa = (u8*)src1 + pixelColumnOffset;
                    tileColumnOffset = (j >> 2) * 0x20;
                    pa += tileColumnOffset;
                    pa += h;
                    rowDataOffset = (int)wd * w * 2;
                    pa += rowDataOffset;
                    pixelA = READ_TEXTURE_U16(pa + 0x60);
                    redA = (pixelA & 0xf800) >> 8;
                    redA = (u8)(redA | ((pixelA & 0xe000) >> 13));
                    pb = (u8*)src2 + pixelColumnOffset;
                    pb += tileColumnOffset;
                    pb += h;
                    pb += rowDataOffset;
                    pixelB = READ_TEXTURE_U16(pb + 0x60);
                    redB = (pixelB & 0xf800) >> 8;
                    redB = (u8)(redB | ((pixelB & 0xe000) >> 13));
                    blue = ((u8)(((int)(wA * (u8)(((pixelA & 0x1f) << 3) | ((pixelA & 0x1c) >> 2))) >> 8) +
                               ((int)(wB * (u8)(((pixelB & 0x1f) << 3) | ((pixelB & 0x1c) >> 2))) >> 8)) &
                           0xf8) >>
                          3;
                    red = ((u8)(((int)(redA * wA) >> 8) + ((int)(redB * wB) >> 8)) & 0xf8) << 8;
                    green = ((u8)(((int)(wA * (u8)(((pixelA & 0x7e0) >> 3) | ((pixelA & 0x600) >> 9))) >> 8) +
                               ((int)(wB * (u8)(((pixelB & 0x7e0) >> 3) | ((pixelB & 0x600) >> 9))) >> 8)) &
                            0xfc)
                           << 3;
                    outputPixel = blue | (red | green);
                    pc = (u8*)dst + pixelColumnOffset + tileColumnOffset + h + rowDataOffset;
                    WRITE_TEXTURE_U16(pc + 0x60, outputPixel);
                }
            }
        }
        else
        {
            int i, j;
            for (i = 0; i < src1->height; i++)
            {
                int tileRowGroupOffset, tileRowOffset;
                u32 wd;
                j = 0;
                tileRowGroupOffset = (i >> 2) * 8;
                tileRowOffset = (i & 3) * 8;
                for (; j < (int)(wd = src1->width); j++)
                {
                    int rowDataOffset;
                    int pixelColumnOffset = (j & 3) * 2;
                    int tileColumnOffset;
                    u8 *ad, *bd, *ct, *cd;
                    int aLo, bLo, aHi, bHi;
                    ad = (u8*)src1 + pixelColumnOffset;
                    tileColumnOffset = (j >> 2) * 0x40;
                    ad += tileColumnOffset;
                    ad += tileRowOffset;
                    rowDataOffset = (int)wd * tileRowGroupOffset * 2;
                    ad += rowDataOffset;
                    bd = (u8*)src2 + pixelColumnOffset;
                    bd += tileColumnOffset;
                    bd += tileRowOffset;
                    bd += rowDataOffset;
                    aLo = READ_TEXTURE_U16(ad + 0x60);
                    aLo = (u8)aLo;
                    bLo = READ_TEXTURE_U16(bd + 0x60);
                    bLo = (u8)bLo;
                    pixelA = READ_TEXTURE_U16(ad + 0x80);
                    aHi = (pixelA & 0xff00) >> 8;
                    aHi = (u8)aHi;
                    pixelB = READ_TEXTURE_U16(bd + 0x80);
                    bHi = (pixelB & 0xff00) >> 8;
                    bHi = (u8)bHi;
                    ct = (u8*)dst + pixelColumnOffset + 0x60;
                    cd = ct + tileColumnOffset;
                    cd += tileRowOffset;
                    WRITE_TEXTURE_U16(cd + rowDataOffset,
                                      (u8)(((int)(aLo * wA) >> 8) + ((int)(bLo * wB) >> 8)));
                    WRITE_TEXTURE_U16(cd + src1->width * tileRowGroupOffset * 2 + 0x20,
                                      ((u8)(((int)(aHi * wA) >> 8) + ((int)(bHi * wB) >> 8)) << 8) |
                                          (u8)(((int)(wA * (u8)pixelA) >> 8) +
                                               ((int)(wB * (u8)pixelB) >> 8)));
                }
            }
        }
        DCStoreRange((u8*)dst + sizeof(Texture), dst->dataSize);
    }
}

#undef READ_TEXTURE_U16
#undef WRITE_TEXTURE_U16

void updateHeavyFogTexture(int intensity)
{
    u8* cache;
    u32 hi, mid;
    u32 scaled;
    u32 j;
    int row;

    cache = getCache();
    for (row = 0; (u32)row < 0x40; row++)
    {
        j = 0;
        hi = ((u32)row >> 2) << 8;
        mid = (row & 3) << 3;
        scaled = (row + intensity) * 0xff;
        for (; j < 0x40; j++)
        {
            int idx;
            u32 s;
            s = (j & 7) + ((j >> 3) << 5);
            s += mid;
            idx = s + hi;
            s = scaled;
            if (s > 0x3fc0)
            {
                s = 0x3fc0;
            }
            cache[idx] = (s * j) >> 12;
        }
    }
    memcpyToCache((void*)(gNewShadowHeavyFogTexture + 0x60), cache, 0);
    lbl_803DCF80 = intensity;
}
