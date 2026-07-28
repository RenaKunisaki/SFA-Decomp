/*
 * DLL 104 / 0x68 - a thin gameplay-effect DLL.
 *
 * Real exports (per the DLL's .text):
 *   dll_68_func00_nop / dll_68_func01_nop - empty entry-point stubs.
 *   dll_68_func03 - builds an 11-command Modgfx effect command list on the
 *     stack (textures/half-words sourced from lbl_803135C8) and submits it via
 *     gModgfxInterface->spawnEffect. When the caller's flags bit 0 is set the
 *     effect is positioned from the source object (offset 0x18..0x20) or, if
 *     none, from the PartFxSpawnParams pos fields.
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/partfx_interface.h"
#include "main/dll/modgfx_types.h"

/* effect id spawned by this DLL's modgfx emitter (spawnEffect textureAssetId arg). */
#define DLL68_EFFECT_ID 0x41

u32 lbl_803135C8[123] = {
    0x00000000, 0x03e80000, 0x00000362, 0x000001f4, 0x000b0000, 0x03620000, 0xfe0c0016, 0x00000000, 0x0000fc18,
    0x00200000, 0xfc9e0000, 0xfe0c002a, 0x0000fc9e, 0x000001f4, 0x00340000, 0x00000000, 0x03e8003f, 0x00000000,
    0x064003e8, 0x0000001f, 0x03620640, 0x01f4000b, 0x001f0362, 0x0640fe0c, 0x0016001f, 0x00000640, 0xfc180020,
    0x001ffc9e, 0x0640fe0c, 0x002a001f, 0xfc9e0640, 0x01f40034, 0x001f0000, 0x064003e8, 0x003f001f, 0x00001770,
    0x03e80000, 0x003f0362, 0x177001f4, 0x000b003f, 0x03621770, 0xfe0c0016, 0x003f0000, 0x1770fc18, 0x0020003f,
    0xfc9e1770, 0xfe0c002a, 0x003ffc9e, 0x177001f4, 0x0034003f, 0x00001770, 0x03e8003f, 0x003f0000, 0x00000001,
    0x00080000, 0x00080007, 0x00010002, 0x00090001, 0x00090008, 0x00020003, 0x000a0002, 0x000a0009, 0x00030004,
    0x000b0003, 0x000b000a, 0x00040005, 0x000c0004, 0x000c000b, 0x00050006, 0x000d0005, 0x000d000c, 0x00070008,
    0x000f0007, 0x000f000e, 0x00080009, 0x00100008, 0x0010000f, 0x0009000a, 0x00110009, 0x00110010, 0x000a000b,
    0x0012000a, 0x00120011, 0x000b000c, 0x0013000b, 0x00130012, 0x000c000d, 0x0014000c, 0x00140013, 0x00000001,
    0x00020003, 0x00040005, 0x00060000, 0x00070008, 0x0009000a, 0x000b000c, 0x000d0000, 0x000e000f, 0x00100011,
    0x00120013, 0x00140000, 0x00000001, 0x00020003, 0x00040005, 0x0006000e, 0x000f0010, 0x00110012, 0x00130014,
    0x00000001, 0x00020003, 0x00040005, 0x00060007, 0x00080009, 0x000a000b, 0x000c000d, 0x000e000f, 0x00100011,
    0x00120013, 0x00140000, 0x00000032, 0x00640032, 0x00000000, 0x00000000,
};

void dll_68_func03(int sourceObj, int variant, int posSource, u32 flags)
{
    ModgfxSpawnPacket buf;
    u8* base = (u8*)(int)lbl_803135C8;
    int ctx;
    buf.entries[0].layer = 0;
    buf.entries[0].flags = 0x15;
    buf.entries[0].tex = &base[432];
    buf.entries[0].mode = 4;
    buf.entries[0].x = 0.0f;
    buf.entries[0].y = 0.0f;
    buf.entries[0].z = 0.0f;
    buf.entries[1].layer = 0;
    buf.entries[1].flags = 0x15;
    buf.entries[1].tex = &base[432];
    buf.entries[1].mode = 2;
    buf.entries[1].x = 2.1f;
    buf.entries[1].y = 2.0f;
    buf.entries[1].z = 2.1f;
    buf.entries[2].layer = 1;
    buf.entries[2].flags = 7;
    buf.entries[2].tex = &base[356];
    buf.entries[2].mode = 2;
    buf.entries[2].x = 2.0f;
    buf.entries[2].y = 1.0f;
    buf.entries[2].z = 2.0f;
    buf.entries[3].layer = 1;
    buf.entries[3].flags = 7;
    buf.entries[3].tex = &base[372];
    buf.entries[3].mode = 4;
    buf.entries[3].x = 255.0f;
    buf.entries[3].y = 0.0f;
    buf.entries[3].z = 0.0f;
    buf.entries[4].layer = 1;
    buf.entries[4].flags = 0x15;
    buf.entries[4].tex = &base[432];
    buf.entries[4].mode = 0x4000;
    buf.entries[4].x = 0.0f;
    buf.entries[4].y = -2.0f;
    buf.entries[4].z = 0.0f;
    buf.entries[5].layer = 1;
    buf.entries[5].flags = 0;
    buf.entries[5].tex = 0;
    buf.entries[5].mode = 0x100;
    buf.entries[5].x = 0.0f;
    buf.entries[5].y = 0.0f;
    buf.entries[5].z = 150.0f;
    buf.entries[6].layer = 2;
    buf.entries[6].flags = 0x15;
    buf.entries[6].tex = &base[432];
    buf.entries[6].mode = 0x4000;
    buf.entries[6].x = 0.0f;
    buf.entries[6].y = -2.0f;
    buf.entries[6].z = 0.0f;
    buf.entries[7].layer = 2;
    buf.entries[7].flags = 0;
    buf.entries[7].tex = 0;
    buf.entries[7].mode = 0x100;
    buf.entries[7].x = 0.0f;
    buf.entries[7].y = 0.0f;
    buf.entries[7].z = 150.0f;
    buf.entries[8].layer = 3;
    buf.entries[8].flags = 7;
    buf.entries[8].tex = &base[372];
    buf.entries[8].mode = 4;
    buf.entries[8].x = 0.0f;
    buf.entries[8].y = 0.0f;
    buf.entries[8].z = 0.0f;
    buf.entries[9].layer = 3;
    buf.entries[9].flags = 0x15;
    buf.entries[9].tex = &base[432];
    buf.entries[9].mode = 0x4000;
    buf.entries[9].x = 0.0f;
    buf.entries[9].y = -2.0f;
    buf.entries[9].z = 0.0f;
    buf.entries[10].layer = 3;
    buf.entries[10].flags = 0;
    buf.entries[10].tex = 0;
    buf.entries[10].mode = 0x100;
    buf.entries[10].x = 0.0f;
    buf.entries[10].y = 0.0f;
    buf.entries[10].z = 150.0f;
    buf.v58 = 0;
    ctx = sourceObj;
    buf.ctx = ctx;
    buf.v44 = variant;
    buf.pos[0] = 0.0f;
    buf.pos[1] = 0.0f;
    buf.pos[2] = 0.0f;
    buf.col[0] = 0.0f;
    buf.col[1] = 0.0f;
    buf.col[2] = 0.0f;
    buf.scale = 1.0f;
    buf.v40 = 2;
    buf.v3c = 7;
    buf.v59 = 0xe;
    buf.v5a = 0;
    buf.v5b = 0x1e;
    buf.count = 11;
    buf.hw[0] = *(s16*)&base[476];
    buf.hw[1] = *(s16*)&base[478];
    buf.hw[2] = *(s16*)&base[480];
    buf.hw[3] = *(s16*)&base[482];
    buf.hw[4] = *(s16*)&base[484];
    buf.hw[5] = *(s16*)&base[486];
    buf.hw[6] = *(s16*)&base[488];
    buf.cmds = (GfxCmd*)((u8*)&buf + 0x60);
    buf.flags = 0xc0100c0;
    buf.flags |= flags;
    if ((buf.flags & 1) != 0)
    {
        if (*(void**)&buf.ctx != 0)
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
    (*gModgfxInterface)->spawnEffect(&buf, 0, 0x15, (u8*)(int)lbl_803135C8, 0x18, &base[212], DLL68_EFFECT_ID, 0);
}

void dll_68_func01_nop(void)
{
}

void dll_68_func00_nop(void)
{
}

u32 lbl_803137B4[9] = {
    0x00000000, 0x00000000,         0x00000000, 0x00030000, (u32)dll_68_func00_nop, (u32)dll_68_func01_nop,
    0x00000000, (u32)dll_68_func03, 0x00000000,
};
