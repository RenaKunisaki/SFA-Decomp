/*
 * DLL 116 / 0x74 - modgfx effect spawner.
 *
 * dll_74_func03 builds the command list for a two-variant spirit/aura
 * effect and submits it through gModgfxInterface. func00/func01 are
 * empty lifecycle slots.
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/partfx_interface.h"
#include "main/mapEventTypes.h"
#include "main/dll/modgfx_types.h"

/* spawnEffect effect ids for the func03 "0/non-0 variant pair" (docblock). */
#define DLL74_EFFECT_ID_VARIANT0 0x2e
#define DLL74_EFFECT_ID_DEFAULT  0xd9

u32 lbl_803146D8[137] = {
    0x00000000, 0x03e80000, 0x00000362, 0x000001f4, 0x000b0000, 0x03620000, 0xfe0c0016, 0x00000000, 0x0000fc18,
    0x00200000, 0xfc9e0000, 0xfe0c0016, 0x0000fc9e, 0x000001f4, 0x000b0000, 0x00000000, 0x03e80000, 0x00000000,
    0x157c03e8, 0x0000003b, 0x0362157c, 0x01f4000b, 0x003b0362, 0x157cfe0c, 0x0016003b, 0x0000157c, 0xfc180020,
    0x003bfc9e, 0x157cfe0c, 0x0016003b, 0xfc9e157c, 0x01f4000b, 0x003b0000, 0x157c03e8, 0x0000003b, 0x00001770,
    0x03e80000, 0x003f0362, 0x177001f4, 0x000b003f, 0x03621770, 0xfe0c0016, 0x003f0000, 0x1770fc18, 0x0020003f,
    0xfc9e1770, 0xfe0c0016, 0x003ffc9e, 0x177001f4, 0x000b003f, 0x00001770, 0x03e80000, 0x003f0000, 0x00000001,
    0x00080000, 0x00080007, 0x00010002, 0x00090001, 0x00090008, 0x00020003, 0x000a0002, 0x000a0009, 0x00030004,
    0x000b0003, 0x000b000a, 0x00040005, 0x000c0004, 0x000c000b, 0x00050006, 0x000d0005, 0x000d000c, 0x00070008,
    0x000f0007, 0x000f000e, 0x00080009, 0x00100008, 0x0010000f, 0x0009000a, 0x00110009, 0x00110010, 0x000a000b,
    0x0012000a, 0x00120011, 0x000b000c, 0x0013000b, 0x00130012, 0x000c000d, 0x0014000c, 0x00140013, 0x00000001,
    0x00020003, 0x00040005, 0x00060000, 0x00070008, 0x0009000a, 0x000b000c, 0x000d0000, 0x000e000f, 0x00100011,
    0x00120013, 0x00140000, 0x00000001, 0x00020003, 0x00040005, 0x0006000e, 0x000f0010, 0x00110012, 0x00130014,
    0x00000001, 0x00020003, 0x00040005, 0x00060007, 0x00080009, 0x000a000b, 0x000c000d, 0x000e000f, 0x00100011,
    0x00120013, 0x00140000, 0x00070008, 0x0009000a, 0x000b000c, 0x000d000e, 0x000f0010, 0x00110012, 0x00130014,
    0x00000001, 0x00020003, 0x00040005, 0x00060007, 0x00080009, 0x000a000b, 0x000c000d, 0x0000000a, 0x00b40028,
    0x00000000, 0x00000000};

void dll_74_func03(GameObject* sourceObj, int variant, u8* posSource, u32 flags)
{
    ModgfxPointerSpawnPacket buf;
    GfxCmd* e;
    u8* base = (u8*)(int)lbl_803146D8;
    GfxCmd* entries = buf.entries;
    entries[0].layer = 0;
    entries[0].flags = 0x15;
    entries[0].tex = &base[0x1b0];
    entries[0].mode = 4;
    entries[0].x = 0.0f;
    entries[0].y = 0.0f;
    entries[0].z = 0.0f;
    if (variant == 0)
    {
        entries[1].layer = 0;
        entries[1].flags = 0x15;
        entries[1].tex = &base[0x1b0];
        entries[1].mode = 2;
        entries[1].x = 0.01f;
        entries[1].y = 1.0f;
        entries[1].z = 0.01f;
        e = &entries[2];
    }
    else
    {
        entries[1].layer = 0;
        entries[1].flags = 0x15;
        entries[1].tex = &base[0x1b0];
        entries[1].mode = 2;
        entries[1].x = 0.01f;
        entries[1].y = 3.0f;
        entries[1].z = 0.01f;
        e = &entries[2];
    }
    if (variant == 0)
    {
        e->layer = 0;
        e->flags = 0;
        e->tex = NULL;
        e->mode = 0x400000;
        e->x = 0.0f;
        e->y = -90.0f;
        e->z = 0.0f;
        e++;
    }
    else
    {
        e->layer = 0;
        e->flags = 0;
        e->tex = NULL;
        e->mode = 0x400000;
        e->x = 0.0f;
        e->y = -290.0f;
        e->z = 0.0f;
        e++;
    }
    e[0].layer = 1;
    e[0].flags = 0x15;
    e[0].tex = &base[0x1b0];
    e[0].mode = 2;
    e[0].x = 70.0f;
    e[0].y = 1.5f;
    e[0].z = 70.0f;
    e[1].layer = 1;
    e[1].flags = 0xe;
    e[1].tex = &base[0x1f8];
    e[1].mode = 4;
    e[1].x = 255.0f;
    e[1].y = 0.0f;
    e[1].z = 0.0f;
    if (variant == 0)
    {
        e[2].layer = 1;
        e[2].flags = 0x15;
        e[2].tex = &base[0x1b0];
        e[2].mode = 0x4000;
        e[2].x = 0.0f;
        e[2].y = 4.0f;
        e[2].z = 0.0f;
        e += 3;
    }
    else
    {
        e[2].layer = 1;
        e[2].flags = 0x15;
        e[2].tex = &base[0x1b0];
        e[2].mode = 0x4000;
        e[2].x = 0.0f;
        e[2].y = -4.0f;
        e[2].z = 0.0f;
        e += 3;
    }
    e[0].layer = 2;
    e[0].flags = 7;
    e[0].tex = &base[0x164];
    e[0].mode = 2;
    e[0].x = 17.0f;
    e[0].y = 1.0f;
    e[0].z = 17.0f;
    e[1].layer = 2;
    e[1].flags = 7;
    e[1].tex = &base[0x174];
    e[1].mode = 2;
    e[1].x = 1.5f;
    e[1].y = 1.0f;
    e[1].z = 1.5f;
    if (variant == 0)
    {
        e[2].layer = 2;
        e[2].flags = 0x15;
        e[2].tex = &base[0x1b0];
        e[2].mode = 0x4000;
        e[2].x = 0.0f;
        e[2].y = 4.0f;
        e[2].z = 0.0f;
        e += 3;
    }
    else
    {
        e[2].layer = 2;
        e[2].flags = 0x15;
        e[2].tex = &base[0x1b0];
        e[2].mode = 0x4000;
        e[2].x = 0.0f;
        e[2].y = -4.0f;
        e[2].z = 0.0f;
        e += 3;
    }
    e[0].layer = 2;
    e[0].flags = 0xe;
    e[0].tex = &base[0x1f8];
    e[0].mode = 4;
    e[0].x = 0.0f;
    e[0].y = 0.0f;
    e[0].z = 0.0f;
    buf.v58 = 0;
    buf.ctx = sourceObj;
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
    buf.count = (e + 1) - entries;
    buf.hw[0] = *(s16*)&base[0x214];
    buf.hw[1] = *(s16*)&base[0x216];
    buf.hw[2] = *(s16*)&base[0x218];
    buf.hw[3] = *(s16*)&base[0x21a];
    buf.hw[4] = *(s16*)&base[0x21c];
    buf.hw[5] = *(s16*)&base[0x21e];
    buf.hw[6] = *(s16*)&base[0x220];
    buf.cmds = (GfxCmd*)((u8*)&buf + 0x60);
    buf.flags = 0xc0104c0;
    buf.flags |= flags;
    if ((buf.flags & 1) != 0)
    {
        if (sourceObj != NULL)
        {
            buf.pos[0] += (sourceObj)->anim.localPosX;
            buf.pos[1] += (sourceObj)->anim.localPosY;
            buf.pos[2] += (sourceObj)->anim.localPosZ;
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
        (*gModgfxInterface)
            ->spawnEffect(&buf, 0, 0x15, (u8*)(int)lbl_803146D8, 0x18, &base[0xd4], DLL74_EFFECT_ID_VARIANT0, 0);
    }
    else
    {
        (*gModgfxInterface)
            ->spawnEffect(&buf, 0, 0x15, (u8*)(int)lbl_803146D8, 0x18, &base[0xd4], DLL74_EFFECT_ID_DEFAULT, 0);
    }
}

void dll_74_func01_nop(void)
{
}

void dll_74_func00_nop(void)
{
}

u32 lbl_803148FC[9] = {
    0x00000000, 0x00000000,         0x00000000, 0x00030000, (u32)dll_74_func00_nop, (u32)dll_74_func01_nop,
    0x00000000, (u32)dll_74_func03, 0x00000000};
