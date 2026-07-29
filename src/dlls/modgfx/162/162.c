/*
 * dlla2func0 (DLL 162 / 0xA2) - particle/gfx spawn helper for a modgfx DLL.
 *
 * The unit owns three entry points:
 *   dll_A2_func00_nop - empty stub.
 *   dll_A2_func01_nop - empty stub.
 *   dll_A2_func03 - builds a 12-command graphics command list (GfxCmd[])
 *     on the stack from a layout table (lbl_80318B00) and a set of shared
 *     float constants, then submits it via gModgfxInterface->spawnEffect.
 *     Bit 0 of the spawn-context flags word (buf.flags) enables
 *     world-position override: a non-null sourceObj uses the GameObject
 *     world position, a null one uses posSource as a position packet.
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/modgfx_types.h"
#include "main/dll/partfx_interface.h"
#include "dlls/object_descriptor.h"

/* effect id spawned by this DLL's modgfx emitter (spawnEffect textureAssetId arg). */
#define DLLA2_EFFECT_ID 0x24

extern u8 lbl_80318B00[];

void dll_A2_func03(GameObject* sourceObj, int variant, u8* posSource, u32 flags)
{
    ModgfxSpawnPacket buf;
    u8* base = (u8*)(int)lbl_80318B00;
    GfxCmd* e = buf.entries;
    u32 fl;

    e[0].layer = 0;
    e[0].flags = 0x15;
    e[0].tex = &base[0x1b0];
    e[0].mode = 4;
    e[0].x = 0.0f;
    e[0].y = 0.0f;
    e[0].z = 0.0f;
    e[1].layer = 0;
    e[1].flags = 0x15;
    e[1].tex = &base[0x1b0];
    e[1].mode = 2;
    e[1].x = 0.05f;
    e[1].y = 0.05f;
    e[1].z = -0.35f;
    e[2].layer = 0;
    e[2].flags = 7;
    e[2].tex = &base[0x164];
    e[2].mode = 8;
    e[2].x = 255.0f;
    e[2].y = 0.0f;
    e[2].z = 0.0f;
    e[3].layer = 1;
    e[3].flags = 7;
    e[3].tex = &base[0x174];
    e[3].mode = 2;
    e[3].x = 4.0f;
    e[3].y = 4.0f;
    e[3].z = 8.0f;
    e[4].layer = 1;
    e[4].flags = 7;
    e[4].tex = &base[0x184];
    e[4].mode = 2;
    e[4].x = 8.0f;
    e[4].y = 8.0f;
    e[4].z = 10.0f;
    e[5].layer = 1;
    e[5].flags = 7;
    e[5].tex = &base[0x174];
    e[5].mode = 4;
    e[5].x = 255.0f;
    e[5].y = 0.0f;
    e[5].z = 0.0f;
    e[6].layer = 1;
    e[6].flags = 0x15;
    e[6].tex = &base[0x1b0];
    e[6].mode = 0x4000;
    e[6].x = 1.0f;
    e[6].y = -4.0f;
    e[6].z = 0.0f;
    e[7].layer = 2;
    e[7].flags = 7;
    e[7].tex = &base[0x174];
    e[7].mode = 2;
    e[7].x = 1.0f;
    e[7].y = 1.0f;
    e[7].z = 1.0f;
    e[8].layer = 2;
    e[8].flags = 7;
    e[8].tex = &base[0x184];
    e[8].mode = 2;
    e[8].x = 1.0f;
    e[8].y = 1.0f;
    e[8].z = 1.0f;
    e[9].layer = 2;
    e[9].flags = 0x15;
    e[9].tex = &base[0x1b0];
    e[9].mode = 0x4000;
    e[9].x = 1.0f;
    e[9].y = -4.0f;
    e[9].z = 0.0f;
    e[10].layer = 3;
    e[10].flags = 7;
    e[10].tex = &base[0x174];
    e[10].mode = 4;
    e[10].x = 0.0f;
    e[10].y = 0.0f;
    e[10].z = 0.0f;
    e[11].layer = 3;
    e[11].flags = 0x15;
    e[11].tex = &base[0x1b0];
    e[11].mode = 0x4000;
    e[11].x = 1.0f;
    e[11].y = -4.0f;
    e[11].z = 0.0f;

    buf.v58 = 0;
    buf.ctx = (int)sourceObj;
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
    buf.count = (GfxCmd*)((u8*)e + 0x120) - e; /* 12 entries * sizeof(GfxCmd)==0x18 */
    buf.hw[0] = *(s16*)&base[0x1f8];
    buf.hw[1] = *(s16*)&base[0x1fa];
    buf.hw[2] = *(s16*)&base[0x1fc];
    buf.hw[3] = *(s16*)&base[0x1fe];
    buf.hw[4] = *(s16*)&base[0x200];
    buf.hw[5] = *(s16*)&base[0x202];
    buf.hw[6] = *(s16*)&base[0x204];
    buf.cmds = e;
    buf.flags = 0xc010480;
    buf.flags |= flags;
    fl = buf.flags;
    if ((fl & 1) != 0)
    {
        if (sourceObj != NULL)
        {
            buf.pos[0] += (sourceObj)->anim.worldPosX;
            buf.pos[1] += (sourceObj)->anim.worldPosY;
            buf.pos[2] += (sourceObj)->anim.worldPosZ;
        }
        else
        {
            buf.pos[0] += ((PartFxSpawnParams*)posSource)->posX;
            buf.pos[1] += ((PartFxSpawnParams*)posSource)->posY;
            buf.pos[2] += ((PartFxSpawnParams*)posSource)->posZ;
        }
    }
    (*gModgfxInterface)->spawnEffect(&buf, 0, 0x15, (u8*)(int)lbl_80318B00, 0x18, &base[0xd4], DLLA2_EFFECT_ID, 0);
}

void dll_A2_func01_nop(void)
{
}

void dll_A2_func00_nop(void)
{
}

u8 lbl_80318B00[520] = {
    0,   0,   3,   232, 0,   0,   0,   0,   0,   0,   3, 98,  1,   244, 0,   0,   0,   11,  0,   0,   3,   98, 254, 12,
    0,   0,   0,   22,  0,   0,   0,   0,   252, 24,  0, 0,   0,   31,  0,   0,   252, 158, 254, 12,  0,   0,  0,   22,
    0,   0,   252, 158, 1,   244, 0,   0,   0,   11,  0, 0,   0,   0,   3,   232, 0,   0,   0,   0,   0,   0,  0,   0,
    3,   232, 1,   244, 0,   0,   0,   15,  3,   98,  1, 244, 1,   44,  0,   11,  0,   15,  3,   98,  254, 12, 1,   244,
    0,   22,  0,   15,  0,   0,   252, 24,  1,   44,  0, 31,  0,   15,  252, 158, 254, 12,  1,   244, 0,   22, 0,   15,
    252, 158, 1,   244, 1,   44,  0,   11,  0,   15,  0, 0,   3,   232, 1,   244, 0,   0,   0,   15,  0,   0,  3,   232,
    3,   32,  0,   0,   0,   31,  3,   98,  1,   244, 3, 232, 0,   11,  0,   31,  3,   98,  254, 12,  3,   32, 0,   22,
    0,   31,  0,   0,   252, 24,  3,   232, 0,   31,  0, 31,  252, 158, 254, 12,  3,   32,  0,   22,  0,   31, 252, 158,
    1,   244, 3,   232, 0,   11,  0,   31,  0,   0,   3, 232, 3,   32,  0,   0,   0,   31,  0,   0,   0,   0,  0,   1,
    0,   8,   0,   0,   0,   8,   0,   7,   0,   1,   0, 2,   0,   9,   0,   1,   0,   9,   0,   8,   0,   2,  0,   3,
    0,   10,  0,   2,   0,   10,  0,   9,   0,   3,   0, 4,   0,   11,  0,   3,   0,   11,  0,   10,  0,   4,  0,   5,
    0,   12,  0,   4,   0,   12,  0,   11,  0,   5,   0, 6,   0,   13,  0,   5,   0,   13,  0,   12,  0,   7,  0,   8,
    0,   15,  0,   7,   0,   15,  0,   14,  0,   8,   0, 9,   0,   16,  0,   8,   0,   16,  0,   15,  0,   9,  0,   10,
    0,   17,  0,   9,   0,   17,  0,   16,  0,   10,  0, 11,  0,   18,  0,   10,  0,   18,  0,   17,  0,   11, 0,   12,
    0,   19,  0,   11,  0,   19,  0,   18,  0,   12,  0, 13,  0,   20,  0,   12,  0,   20,  0,   19,  0,   0,  0,   1,
    0,   2,   0,   3,   0,   4,   0,   5,   0,   6,   0, 0,   0,   7,   0,   8,   0,   9,   0,   10,  0,   11, 0,   12,
    0,   13,  0,   0,   0,   14,  0,   15,  0,   16,  0, 17,  0,   18,  0,   19,  0,   20,  0,   0,   0,   0,  0,   1,
    0,   2,   0,   3,   0,   4,   0,   5,   0,   6,   0, 14,  0,   15,  0,   16,  0,   17,  0,   18,  0,   19, 0,   20,
    0,   0,   0,   1,   0,   2,   0,   3,   0,   4,   0, 5,   0,   6,   0,   7,   0,   8,   0,   9,   0,   10, 0,   11,
    0,   12,  0,   13,  0,   14,  0,   15,  0,   16,  0, 17,  0,   18,  0,   19,  0,   20,  0,   0,   0,   7,  0,   8,
    0,   9,   0,   10,  0,   11,  0,   12,  0,   13,  0, 14,  0,   15,  0,   16,  0,   17,  0,   18,  0,   19, 0,   20,
    0,   0,   0,   35,  0,   6,   0,   35,  0,   0,   0, 0,   0,   0,   0,   0};
ObjectDescriptor4 dll_A2_funcs = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_4_SLOTS,
    (ObjectDescriptorCallback)dll_A2_func00_nop,
    (ObjectDescriptorCallback)dll_A2_func01_nop,
    0,
    (ObjectDescriptorCallback)dll_A2_func03,
};
