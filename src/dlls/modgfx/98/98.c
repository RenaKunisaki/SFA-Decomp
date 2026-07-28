/*
 * DLL 98 / 0x62 - a modgfx effect spawner.
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/partfx_interface.h"
#include "main/dll/modgfx_types.h"
#include "main/dll/dll_0062_dll62func0.h"

/* effect id spawned by this DLL's modgfx emitter (spawnEffect textureAssetId arg). */
#define DLL62_EFFECT_ID 0x5e0

u32 lbl_803129C8[123] = {
    0x00000000, 0x03e80000, 0x00000362, 0x000001f4, 0x002c0000, 0x03620000, 0xfe0c0058, 0x00000000, 0x0000fc18,
    0x00800000, 0xfc9e0000, 0xfe0c00a8, 0x0000fc9e, 0x000001f4, 0x00d00000, 0x00000000, 0x03e80100, 0x00000000,
    0x064003e8, 0x00000020, 0x03620640, 0x01f4002c, 0x00200362, 0x0640fe0c, 0x00580020, 0x00000640, 0xfc180080,
    0x0020fc9e, 0x0640fe0c, 0x00a80020, 0xfc9e0640, 0x01f400d0, 0x00200000, 0x064003e8, 0x01000020, 0x00001770,
    0x03e80000, 0x00400362, 0x177001f4, 0x002c0040, 0x03621770, 0xfe0c0058, 0x00400000, 0x1770fc18, 0x00800040,
    0xfc9e1770, 0xfe0c00a8, 0x0040fc9e, 0x177001f4, 0x00d00040, 0x00001770, 0x03e80100, 0x00400000, 0x00000001,
    0x00080000, 0x00080007, 0x00010002, 0x00090001, 0x00090008, 0x00020003, 0x000a0002, 0x000a0009, 0x00030004,
    0x000b0003, 0x000b000a, 0x00040005, 0x000c0004, 0x000c000b, 0x00050006, 0x000d0005, 0x000d000c, 0x00070008,
    0x000f0007, 0x000f000e, 0x00080009, 0x00100008, 0x0010000f, 0x0009000a, 0x00110009, 0x00110010, 0x000a000b,
    0x0012000a, 0x00120011, 0x000b000c, 0x0013000b, 0x00130012, 0x000c000d, 0x0014000c, 0x00140013, 0x00000001,
    0x00020003, 0x00040005, 0x00060000, 0x00070008, 0x0009000a, 0x000b000c, 0x000d0000, 0x000e000f, 0x00100011,
    0x00120013, 0x00140000, 0x00000001, 0x00020003, 0x00040005, 0x0006000e, 0x000f0010, 0x00110012, 0x00130014,
    0x00000001, 0x00020003, 0x00040005, 0x00060007, 0x00080009, 0x000a000b, 0x000c000d, 0x000e000f, 0x00100011,
    0x00120013, 0x00140000, 0x00000104, 0x003c003c, 0x00010104, 0x00000000,
};

void dll_62_func03(int sourceObj, int variant, int posSource, u32 flags)
{
    ModgfxSpawnPacket buf;
    GfxCmd* e = buf.entries;
    u8* base = (u8*)(int)lbl_803129C8;
    u8 cnt;
    f32 posX;
    posX = 1.0f;
    cnt = *(u8*)(*(u8**)&((GameObject*)sourceObj)->anim.placementData + 0x1a);
    if (variant == 1)
    {
        *(s16*)&base[478] = 0;
        posX = 4.0f;
    }
    else if (variant == 2)
    {
        posX = 0.0f;
        cnt = 6;
    }
    e[0].layer = 0;
    e[0].flags = 0x15;
    e[0].tex = &base[432];
    e[0].mode = 4;
    e[0].x = 0.0f;
    e[0].y = 0.0f;
    e[0].z = 0.0f;
    e[1].layer = 0;
    e[1].flags = 0xe;
    e[1].tex = &base[404];
    e[1].mode = 2;
    e[1].x = 0.95f;
    e[1].y = 0.4f;
    e[1].z = 0.95f;
    e[2].layer = 0;
    e[2].flags = 7;
    e[2].tex = &base[372];
    e[2].mode = 2;
    e[2].x = 0.95f;
    e[2].y = 0.4f;
    e[2].z = 0.95f;
    e[3].layer = 1;
    e[3].flags = 7;
    e[3].tex = &base[372];
    e[3].mode = 4;
    e[3].x = 255.0f;
    e[3].y = 0.0f;
    e[3].z = 0.0f;
    e[4].layer = 1;
    e[4].flags = 7;
    e[4].tex = &base[388];
    e[4].mode = 4;
    e[4].x = 255.0f;
    e[4].y = 0.0f;
    e[4].z = 0.0f;
    e[5].layer = 1;
    e[5].flags = 0x15;
    e[5].tex = &base[432];
    e[5].mode = 0x100;
    e[5].x = 0.0f;
    e[5].y = 0.0f;
    e[5].z = 10.0f;
    e[6].layer = 2;
    e[6].flags = 0x3a;
    e[6].tex = 0;
    e[6].mode = 0x1800000;
    e[6].x = posX;
    e[6].y = 0.0f;
    e[6].z = 5.0f;
    e[7].layer = 2;
    e[7].flags = 0x15;
    e[7].tex = &base[432];
    e[7].mode = 0x100;
    e[7].x = 0.0f;
    e[7].y = 0.0f;
    e[7].z = 10.0f;
    e[8].layer = 3;
    e[8].flags = 0x3a;
    e[8].tex = 0;
    e[8].mode = 0x1800000;
    e[8].x = posX;
    e[8].y = 0.0f;
    e[8].z = 5.0f;
    e[9].layer = 3;
    e[9].flags = 0x15;
    e[9].tex = &base[432];
    e[9].mode = 0x100;
    e[9].x = 0.0f;
    e[9].y = 0.0f;
    e[9].z = 10.0f;
    e[10].layer = 4;
    e[10].flags = 2;
    e[10].tex = 0;
    e[10].mode = 0x2000;
    e[10].x = 0.0f;
    e[10].y = 0.0f;
    e[10].z = 0.0f;
    e[11].layer = 5;
    e[11].flags = 7;
    e[11].tex = &base[372];
    e[11].mode = 4;
    e[11].x = 0.0f;
    e[11].y = 0.0f;
    e[11].z = 0.0f;
    e[12].layer = 5;
    e[12].flags = 7;
    e[12].tex = &base[388];
    e[12].mode = 4;
    e[12].x = 0.0f;
    e[12].y = 0.0f;
    e[12].z = 0.0f;
    e[13].layer = 5;
    e[13].flags = 0x15;
    e[13].tex = &base[432];
    e[13].mode = 0x100;
    e[13].x = 0.0f;
    e[13].y = 0.0f;
    e[13].z = 10.0f;
    buf.v58 = 0;
    buf.ctx = sourceObj;
    buf.v44 = variant;
    buf.pos[0] = 0.0f;
    buf.pos[1] = 0.0f;
    buf.pos[2] = 0.0f;
    buf.col[0] = 0.0f;
    buf.col[1] = 0.0f;
    buf.col[2] = 0.0f;
    if (cnt != 0)
    {
        buf.scale = 0.1f * (f32)(u32)cnt;
    }
    else
    {
        buf.scale = 1.0f;
    }
    buf.v40 = 2;
    buf.v3c = 7;
    buf.v59 = 0xe;
    buf.v5a = 0;
    buf.v5b = 0x1e;
    buf.count = 14;
    buf.hw[0] = *(s16*)&base[476];
    buf.hw[1] = *(s16*)&base[478];
    buf.hw[2] = *(s16*)&base[480];
    buf.hw[3] = *(s16*)&base[482];
    buf.hw[4] = *(s16*)&base[484];
    buf.hw[5] = *(s16*)&base[486];
    buf.hw[6] = *(s16*)&base[488];
    buf.cmds = (GfxCmd*)((u8*)&buf + 0x60);
    buf.flags = 0xc0400c0;
    buf.flags |= flags;
    if ((buf.flags & 1) != 0)
    {
        if ((void*)buf.ctx != NULL)
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
    (*gModgfxInterface)->spawnEffect(&buf, 0, 0x15, (u8*)(int)lbl_803129C8, 0x18, &base[212], DLL62_EFFECT_ID, 0);
}

void dll_62_func01_nop(void)
{
}

void dll_62_func00_nop(void)
{
}

u32 lbl_80312BB4[9] = {
    0x00000000, 0x00000000,         0x00000000, 0x00030000, (u32)dll_62_func00_nop, (u32)dll_62_func01_nop,
    0x00000000, (u32)dll_62_func03, 0x00000000,
};
