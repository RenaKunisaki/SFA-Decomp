#include "main/texture.h"
#include "main/model.h"
#include "dolphin/mtx.h"
#include "dolphin/gx/GXGeometry.h"
#include "dolphin/os/OSReport.h"
#include "main/gameloop_api.h"
#include "main/pi_dolphin.h"
#include "main/sky_api.h"
#include "main/textrender_api.h"
#include "main/camera_interface.h"
#include "main/mldf_fileid.h"
#include "main/model_runtime_api.h"
#include "main/map_load.h"
#include "main/objprint_load_api.h"
#include "main/vecmath.h"
#include "main/dll/FRONT/n_options.h"
#include "track/intersect_texture_api.h"
#include "dolphin/os.h"
#include "string.h"

extern s32 lbl_803DCC48;
extern f32 lbl_803DEA04;

typedef struct
{
    u8* data;
    int pad[3];
    int pos;
} MtxBitStream;

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
