/*
 * DLL 122 / 0x7A - a model-fx command-list spawner stub DLL.
 *
 * dll_7A_func03 builds a GfxCmd array on the stack from the
 * lbl_80314BD0 resource block and hands it to
 * gModgfxInterface->spawnEffect; the two tiny dll_7A entry stubs are
 * no-ops.
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/partfx_interface.h"
#include "main/mapEventTypes.h"
#include "main/dll/modgfx_types.h"
#include "dlls/object_descriptor.h"

/* spawnEffect effect ids per variant (textureAssetId arg). */
#define DLL7A_EFFECT_ID_VARIANT0 0x156
#define DLL7A_EFFECT_ID_VARIANT1 0xc0d

u32 lbl_80314BD0[48] = {0x03e80000, 0x0190001f, 0x001f02c3, 0xfd3d0190, 0x0000001f, 0x0000fc18, 0x0190001f, 0x001ffd3d,
                        0xfd3d0190, 0x0000001f, 0xfc180000, 0x0190001f, 0x001ffd3d, 0x02c30190, 0x0000001f, 0x000003e8,
                        0x0190001f, 0x001f02c3, 0x02c30190, 0x0000001f, 0x00000000, 0x0000000f, 0x00000000, 0x00000001,
                        0x00080001, 0x00020008, 0x00020003, 0x00080003, 0x00040008, 0x00040005, 0x00080005, 0x00060008,
                        0x00060007, 0x00080007, 0x00000008, 0x00000001, 0x00020003, 0x00040005, 0x00060007, 0x00080000,
                        0x00000001, 0x00020003, 0x00040005, 0x00060007, 0x00000064, 0x00000000, 0x00000000, 0x00000000};

s16 dll_7A_func03(u8* sourceObj, int variant, u8* posSource, u32 flags)
{
    ModgfxPointerSpawnPacket buf;
    u8* base = (u8*)(int)lbl_80314BD0;
    GfxCmd* entries;
    GfxCmd* e;
    s16 ret;
    ret = 0;
    entries = buf.entries;
    entries[0].layer = 0;
    entries[0].flags = 8;
    entries[0].tex = &base[0xa0];
    entries[0].mode = 4;
    entries[0].x = 0.0f;
    entries[0].y = 0.0f;
    entries[0].z = 0.0f;
    entries[1].layer = 0;
    entries[1].flags = 8;
    entries[1].tex = &base[0x8c];
    entries[1].mode = 2;
    entries[1].x = 0.4f * (f32)randomGetRange(10, 15);
    entries[1].y = 0.4f * (f32)randomGetRange(10, 15);
    entries[1].z = 0.8f * (f32)randomGetRange(10, 15);
    entries[2].layer = 0;
    entries[2].flags = 9;
    entries[2].tex = &base[0x8c];
    entries[2].mode = 0x80;
    entries[2].x = 0.0f;
    entries[2].y = 0.0f;
    entries[2].z = -16383.0f;
    entries[3].layer = 1;
    entries[3].flags = 0x9c;
    entries[3].tex = 0;
    entries[3].mode = 0x800000;
    entries[3].x = 2.0f;
    entries[3].y = 1.0f;
    entries[3].z = 0.0f;
    entries[4].layer = 1;
    entries[4].flags = 0;
    entries[4].tex = 0;
    entries[4].mode = 0x400000;
    entries[4].x = (f32)randomGetRange(-2000, 200);
    entries[4].y = (f32)randomGetRange(-200, 200);
    entries[4].z = (f32)randomGetRange(-200, 200);
    entries[5].layer = 1;
    entries[5].flags = 9;
    entries[5].tex = &base[0x8c];
    entries[5].mode = 4;
    entries[5].x = 0.0f;
    entries[5].y = 0.0f;
    entries[5].z = 0.0f;
    e = &entries[6];
    if (variant == 0)
    {
        e->layer = 3;
        e->flags = 0;
        e->tex = 0;
        e->mode = 0x20000000;
        e->x = 999.0f;
        e->y = 94.0f;
        e->z = 95.0f;
        e++;
    }
    buf.ctx = sourceObj;
    buf.v44 = variant;
    if (variant == 0)
    {
        buf.pos[0] = 0.0f;
        buf.pos[1] = 0.0f;
        buf.pos[2] = 0.0f;
    }
    else
    {
        buf.pos[0] = 0.0f;
        buf.pos[1] = 135.0f;
        buf.pos[2] = 0.0f;
    }
    buf.col[0] = 0.0f;
    buf.col[1] = 0.0f;
    buf.col[2] = 0.0f;
    buf.scale = 1.0f;
    buf.v40 = 1;
    buf.v3c = 0;
    buf.v59 = 9;
    buf.v5a = 0;
    buf.v5b = 0;
    buf.count = e - entries;
    buf.hw[0] = *(s16*)&base[0xb0];
    buf.hw[1] = *(s16*)&base[0xb2];
    buf.hw[2] = *(s16*)&base[0xb4];
    buf.hw[3] = *(s16*)&base[0xb6];
    buf.hw[4] = *(s16*)&base[0xb8];
    buf.hw[5] = *(s16*)&base[0xba];
    buf.hw[6] = *(s16*)&base[0xbc];
    buf.cmds = (GfxCmd*)((u8*)&buf + 0x60);
    buf.flags = 0x4000000;
    buf.flags |= flags;
    if ((buf.flags & 1) != 0)
    {
        if (buf.ctx != 0)
        {
            buf.pos[0] += ((GameObject*)(buf.ctx))->anim.worldPosX;
            buf.pos[1] += ((GameObject*)(buf.ctx))->anim.worldPosY;
            buf.pos[2] += ((GameObject*)(buf.ctx))->anim.worldPosZ;
        }
        else
        {
            buf.pos[0] += ((PartFxSpawnParams*)posSource)->posX;
            buf.pos[1] += ((PartFxSpawnParams*)posSource)->posY;
            buf.pos[2] += ((PartFxSpawnParams*)posSource)->posZ;
        }
    }
    if (variant == 0)
    {
        buf.v58 = 0;
        ret = (*gModgfxInterface)
                  ->spawnEffect(&buf, 0, 9, (u8*)(int)lbl_80314BD0, 8, &base[0x5c], DLL7A_EFFECT_ID_VARIANT0, 0);
    }
    else if (variant == 1)
    {
        buf.v58 = 0;
        ret = (*gModgfxInterface)
                  ->spawnEffect(&buf, 0, 9, (u8*)(int)lbl_80314BD0, 8, &base[0x5c], DLL7A_EFFECT_ID_VARIANT1, 0);
    }
    return ret;
}

void dll_7A_func01_nop(void)
{
}

void dll_7A_func00_nop(void)
{
}

ObjectDescriptor4 dll_7A_funcs = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_4_SLOTS,
    (ObjectDescriptorCallback)dll_7A_func00_nop,
    (ObjectDescriptorCallback)dll_7A_func01_nop,
    0,
    (ObjectDescriptorCallback)dll_7A_func03,
};
