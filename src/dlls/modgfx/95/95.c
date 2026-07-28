/*
 * DLL 95 / 0x5F - a thin gameplay-effect DLL.
 *
 * Real exports (per the DLL's .text):
 *   dll_5F_func00_nop / dll_5F_func01_nop - empty entry-point stubs.
 *   dll_5F_func03 - builds a 13-command Modgfx effect command list on the
 *     stack (textures/half-words sourced from lbl_80312650, colours/positions
 *     from the lbl_803E08xx float pool) and submits it via
 *     gModgfxInterface->spawnEffect. When the caller's flags bit 0 is set the
 *     effect is positioned from the source object or, if none, from the
 *     PartFxSpawnParams pos fields.
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/partfx_interface.h"
#include "game/objects/object.h"
#include "main/mapEventTypes.h"
#include "main/dll/modgfx_types.h"
#include "main/dll/dll_005F_dll5ffunc0.h"

/* effect id spawned by this DLL's modgfx emitter (spawnEffect textureAssetId arg). */
#define DLL5F_EFFECT_ID 0x48

u32 lbl_80312650[72] = {
    0x00000000, 0x03e80000, 0x00000362, 0x000001f4, 0x001f0000, 0x03620000, 0xfe0c003f, 0x00000000,
    0x0000fc18, 0x005f0000, 0xfc9e0000, 0xfe0c007f, 0x0000fc9e, 0x000001f4, 0x009f0000, 0x00000000,
    0x03e800bf, 0x00000000, 0x177003e8, 0x0000003f, 0x03621770, 0x01f4001f, 0x003f0362, 0x1770fe0c,
    0x003f003f, 0x00001770, 0xfc18005f, 0x003ffc9e, 0x1770fe0c, 0x007f003f, 0xfc9e1770, 0x01f4009f,
    0x003f0000, 0x177003e8, 0x00bf003f, 0x00000001, 0x00080000, 0x00080007, 0x00010002, 0x00090001,
    0x00090008, 0x00020003, 0x000a0002, 0x000a0009, 0x00030004, 0x000b0003, 0x000b000a, 0x00040005,
    0x000c0004, 0x000c000b, 0x00050006, 0x000d0005, 0x000d000c, 0x00000001, 0x00020003, 0x00040005,
    0x00060007, 0x00080009, 0x000a000b, 0x000c000d, 0x00000001, 0x00020003, 0x00040005, 0x00060000,
    0x00070008, 0x0009000a, 0x000b000c, 0x000d0000, 0x00000014, 0x00aa0000, 0x00000000, 0x00000000,
};

void dll_5F_func03(int sourceObj, int variant, int posSource, u32 flags)
{
    ModgfxSpawnPacket buf;
    u8* base = (u8*)(int)lbl_80312650;
    int ctx;
    f32 originOffset = 0.0f;
    buf.entries[0].layer = 0;
    buf.entries[0].flags = 0x32;
    buf.entries[0].tex = 0;
    buf.entries[0].mode = 0x800000;
    buf.entries[0].x = 1.0f;
    buf.entries[0].y = originOffset;
    buf.entries[0].z = originOffset;
    buf.entries[1].layer = 0;
    buf.entries[1].flags = 0x7a;
    buf.entries[1].tex = 0;
    buf.entries[1].mode = 0x10000;
    buf.entries[1].x = originOffset;
    buf.entries[1].y = originOffset;
    buf.entries[1].z = originOffset;
    buf.entries[2].layer = 0;
    buf.entries[2].flags = 7;
    buf.entries[2].tex = &base[256];
    buf.entries[2].mode = 4;
    buf.entries[2].x = originOffset;
    buf.entries[2].y = originOffset;
    buf.entries[2].z = originOffset;
    buf.entries[3].layer = 0;
    buf.entries[3].flags = 7;
    buf.entries[3].tex = &base[240];
    buf.entries[3].mode = 2;
    buf.entries[3].x = 0.7f;
    buf.entries[3].y = 1.0f;
    buf.entries[3].z = 0.7f;
    buf.entries[4].layer = 0;
    buf.entries[4].flags = 7;
    buf.entries[4].tex = &base[256];
    buf.entries[4].mode = 2;
    buf.entries[4].x = 1.2f;
    buf.entries[4].y = -1.0f;
    buf.entries[4].z = 1.2f;
    buf.entries[5].layer = 0;
    buf.entries[5].flags = 7;
    buf.entries[5].tex = &base[240];
    buf.entries[5].mode = 8;
    buf.entries[5].x = originOffset;
    buf.entries[5].y = 160.0f;
    buf.entries[5].z = 115.0f;
    buf.entries[6].layer = 0;
    buf.entries[6].flags = 7;
    buf.entries[6].tex = &base[256];
    buf.entries[6].mode = 8;
    buf.entries[6].x = 255.0f;
    buf.entries[6].y = 255.0f;
    buf.entries[6].z = 115.0f;
    buf.entries[7].layer = 0;
    buf.entries[7].flags = 1;
    buf.entries[7].tex = 0;
    buf.entries[7].mode = 0x8000;
    buf.entries[7].x = originOffset;
    buf.entries[7].y = 255.0f;
    buf.entries[7].z = originOffset;
    buf.entries[8].layer = 0;
    buf.entries[8].flags = 1;
    buf.entries[8].tex = 0;
    buf.entries[8].mode = 0x80000;
    buf.entries[8].x = originOffset;
    buf.entries[8].y = -130.0f;
    buf.entries[8].z = originOffset;
    buf.entries[9].layer = 1;
    buf.entries[9].flags = 1;
    buf.entries[9].tex = 0;
    buf.entries[9].mode = 0x80000;
    buf.entries[9].x = originOffset;
    buf.entries[9].y = originOffset;
    buf.entries[9].z = originOffset;
    buf.entries[10].layer = 2;
    buf.entries[10].flags = 0xe;
    buf.entries[10].tex = &base[212];
    buf.entries[10].mode = 0x4000;
    buf.entries[10].x = originOffset;
    buf.entries[10].y = -4.0f;
    buf.entries[10].z = originOffset;
    buf.entries[11].layer = 2;
    buf.entries[11].flags = 7;
    buf.entries[11].tex = &base[240];
    buf.entries[11].mode = 4;
    buf.entries[11].x = originOffset;
    buf.entries[11].y = originOffset;
    buf.entries[11].z = originOffset;
    buf.entries[12].layer = 2;
    buf.entries[12].flags = 1;
    buf.entries[12].tex = 0;
    buf.entries[12].mode = 0x80000;
    buf.entries[12].x = originOffset;
    buf.entries[12].y = 90.0f;
    buf.entries[12].z = originOffset;
    buf.v58 = 0;
    ctx = sourceObj;
    buf.ctx = ctx;
    buf.v44 = variant;
    buf.pos[0] = originOffset;
    buf.pos[1] = originOffset;
    buf.pos[2] = originOffset;
    buf.col[0] = originOffset;
    buf.col[1] = originOffset;
    buf.col[2] = originOffset;
    buf.scale = 1.0f;
    buf.v40 = 1;
    buf.v3c = 0;
    buf.v59 = 0xe;
    buf.v5a = 0;
    buf.v5b = 0x10;
    buf.count = 0;
    buf.hw[0] = *(s16*)&base[272];
    buf.hw[1] = *(s16*)&base[274];
    buf.hw[2] = *(s16*)&base[276];
    buf.hw[3] = *(s16*)&base[278];
    buf.hw[4] = *(s16*)&base[280];
    buf.hw[5] = *(s16*)&base[282];
    buf.hw[6] = *(s16*)&base[284];
    buf.cmds = (GfxCmd*)((u8*)&buf + 0x60);
    buf.flags = 0x4000002;
    buf.flags |= flags;
    if ((buf.flags & 1) != 0)
    {
        if ((void*)ctx != NULL)
        {
            buf.pos[0] = originOffset + ((GameObject*)ctx)->anim.worldPosX;
            buf.pos[1] = originOffset + ((GameObject*)ctx)->anim.worldPosY;
            buf.pos[2] = originOffset + ((GameObject*)ctx)->anim.worldPosZ;
        }
        else
        {
            buf.pos[0] = originOffset + ((PartFxSpawnParams*)posSource)->posX;
            buf.pos[1] = originOffset + ((PartFxSpawnParams*)posSource)->posY;
            buf.pos[2] = originOffset + ((PartFxSpawnParams*)posSource)->posZ;
        }
    }
    (*gModgfxInterface)->spawnEffect(&buf, 0, 0xe, (u8*)(int)lbl_80312650, 0xc, &base[140], DLL5F_EFFECT_ID, 0);
}

void dll_5F_func01_nop(void)
{
}

void dll_5F_func00_nop(void)
{
}

u32 lbl_80312770[8] = {
    0x00000000, 0x00000000,        0x00000000, 0x00030000, (u32)dll_5F_func00_nop, (u32)dll_5F_func01_nop,
    0x00000000, (u32)dll_5F_func03,
};
