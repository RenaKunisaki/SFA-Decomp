/*
 * DLL 89 / 0x59 - a modgfx particle-sequence spawn DLL.
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/partfx_interface.h"
#include "main/dll/modgfx_types.h"
#include "dlls/object_descriptor.h"

/* effect id spawned by this DLL's modgfx emitter (spawnEffect textureAssetId arg). */
#define DLL59_EFFECT_ID 0xc0d

extern u8 lbl_80311C58[];

void dll_59_func03(int sourceObj, int variant, int posSource, u32 flags)
{
    ModgfxSpawnPacket buf;
    u8* base = (u8*)(int)lbl_80311C58;
    GfxCmd* e = buf.entries;
    int ctx;
    f32 one;
    f32 zero;
    e[0].layer = 1;
    e[0].flags = 0x11;
    e[0].tex = &base[220];
    e[0].mode = 0x4000;
    e[0].x = (zero = 0.0f);
    e[0].y = -3.0f;
    e[0].z = zero;
    e[1].layer = 1;
    e[1].flags = 0x10;
    e[1].tex = &base[256];
    e[1].mode = 2;
    e[1].x = 35.0f;
    e[1].y = 35.0f;
    e[1].z = 35.0f;
    e[2].layer = 1;
    e[2].flags = 0x11;
    e[2].tex = &base[220];
    e[2].mode = 0x100;
    e[2].x = zero;
    e[2].y = zero;
    e[2].z = 1500.0f;
    e[3].layer = 1;
    e[3].flags = 2;
    e[3].tex = NULL;
    e[3].mode = 0x4000000;
    e[3].x = (one = 1.0f);
    e[3].y = zero;
    e[3].z = zero;
    e[4].layer = 2;
    e[4].flags = 2;
    e[4].tex = NULL;
    e[4].mode = 0x4000000;
    e[4].x = one;
    e[4].y = zero;
    e[4].z = zero;
    e[5].layer = 2;
    e[5].flags = 0x11;
    e[5].tex = &base[220];
    e[5].mode = 0x4000;
    e[5].x = zero;
    e[5].y = -3.0f;
    e[5].z = zero;
    e[6].layer = 2;
    e[6].flags = 0x11;
    e[6].tex = &base[220];
    e[6].mode = 4;
    e[6].x = zero;
    e[6].y = zero;
    e[6].z = zero;
    e[7].layer = 2;
    e[7].flags = 0x11;
    e[7].tex = &base[220];
    e[7].mode = 0x100;
    e[7].x = zero;
    e[7].y = zero;
    e[7].z = -1000.0f;
    e[8].layer = 2;
    e[8].flags = 0x10;
    e[8].tex = &base[256];
    e[8].mode = 2;
    e[8].x = 2.0f;
    e[8].y = 2.0f;
    e[8].z = 2.0f;
    buf.v58 = 0;
    ctx = sourceObj;
    buf.ctx = ctx;
    buf.v44 = variant;
    buf.pos[0] = zero;
    buf.pos[1] = 135.0f;
    buf.pos[2] = zero;
    buf.col[0] = zero;
    buf.col[1] = zero;
    buf.col[2] = zero;
    buf.scale = one;
    buf.v40 = 1;
    buf.v3c = 0;
    buf.v59 = 0x11;
    buf.v5a = 0;
    buf.v5b = 0x10;
    buf.count = (e + 9) - buf.entries;
    buf.hw[0] = *(s16*)&base[288];
    buf.hw[1] = *(s16*)&base[290];
    buf.hw[2] = *(s16*)&base[292];
    buf.hw[3] = *(s16*)&base[294];
    buf.hw[4] = *(s16*)&base[296];
    buf.hw[5] = *(s16*)&base[298];
    buf.hw[6] = *(s16*)&base[300];
    buf.cmds = buf.entries;
    buf.flags = 0x4000000;
    buf.flags |= flags;
    if ((buf.flags & 1) != 0)
    {
        if ((void*)ctx != NULL)
        {
            buf.pos[0] = zero + ((GameObject*)ctx)->anim.worldPosX;
            buf.pos[1] = 135.0f + ((GameObject*)ctx)->anim.worldPosY;
            buf.pos[2] = zero + ((GameObject*)ctx)->anim.worldPosZ;
        }
        else
        {
            buf.pos[0] = zero + ((PartFxSpawnParams*)posSource)->posX;
            buf.pos[1] = 135.0f + ((PartFxSpawnParams*)posSource)->posY;
            buf.pos[2] = zero + ((PartFxSpawnParams*)posSource)->posZ;
        }
    }
    (*gModgfxInterface)->spawnEffect(&buf, 0, 0x11, (u8*)(int)lbl_80311C58, 8, &base[172], DLL59_EFFECT_ID, 0);
}

void dll_59_func01_nop(void)
{
}

void dll_59_func00_nop(void)
{
}

u8 lbl_80311C58[304] = {
    0,   0,   0, 0,   0, 0,   0,   15,  0,   0,   0,   150, 1,   144, 3,   132, 0,   0,   0, 127, 255, 206, 1,   144,
    3,   232, 0, 31,  0, 127, 0,   50,  2,   18,  252, 24,  0,   0,   0,   127, 255, 106, 2, 18,  252, 174, 0,   31,
    0,   127, 3, 232, 0, 100, 0,   150, 0,   0,   0,   127, 4,   176, 0,   100, 255, 206, 0, 31,  0,   127, 252, 24,
    1,   14,  0, 50,  0, 0,   0,   127, 252, 24,  1,   14,  255, 206, 0,   31,  0,   127, 2, 108, 2,   38,  3,   12,
    0,   0,   0, 127, 3, 12,  2,   38,  3,   152, 0,   31,  0,   127, 252, 204, 0,   210, 3, 12,  0,   0,   0,   127,
    253, 188, 0, 210, 3, 52,  0,   31,  0,   127, 3,   52,  0,   100, 252, 244, 0,   0,   0, 127, 3,   12,  0,   100,
    253, 148, 0, 31,  0, 127, 252, 104, 1,   214, 252, 244, 0,   0,   0,   127, 252, 244, 1, 214, 252, 204, 0,   31,
    0,   127, 0, 0,   0, 0,   0,   1,   0,   2,   0,   0,   0,   3,   0,   4,   0,   0,   0, 5,   0,   6,   0,   0,
    0,   7,   0, 8,   0, 0,   0,   9,   0,   10,  0,   0,   0,   11,  0,   12,  0,   0,   0, 13,  0,   14,  0,   0,
    0,   15,  0, 16,  0, 0,   0,   1,   0,   2,   0,   3,   0,   4,   0,   5,   0,   6,   0, 7,   0,   8,   0,   9,
    0,   10,  0, 11,  0, 12,  0,   13,  0,   14,  0,   15,  0,   16,  0,   0,   0,   1,   0, 2,   0,   3,   0,   4,
    0,   5,   0, 6,   0, 7,   0,   8,   0,   9,   0,   10,  0,   11,  0,   12,  0,   13,  0, 14,  0,   15,  0,   16,
    0,   0,   0, 90,  0, 50,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0};

ObjectDescriptor4 dll_59_funcs = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_4_SLOTS,
    (ObjectDescriptorCallback)dll_59_func00_nop,
    (ObjectDescriptorCallback)dll_59_func01_nop,
    0,
    (ObjectDescriptorCallback)dll_59_func03,
};
