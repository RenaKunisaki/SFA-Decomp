/*
 * dlla0func0 (DLL 160 / 0x00A0) - spawns a layered model-graphics (modgfx)
 * effect for a source object.
 *
 * dll_A0_func03 builds a fixed command list of GfxCmd descriptors plus a
 * spawn-context header on the stack and hands it to
 * (*gModgfxInterface)->spawnEffect. Each GfxCmd selects a texture sub-asset
 * (offset into lbl_803186B0) and a per-layer blend/draw mode with a
 * position/scale triple pulled from the lbl_803E14xx float table; the
 * `variant` argument swaps one descriptor's Y component. The base spawn
 * flags are SPAWN_FLAGS_BASE; when its low bit (SPAWN_FLAG_USE_POSITION)
 * survives the caller's `flags`, the effect origin is offset by the source
 * object's world position (or by posSource when sourceObj is NULL).
 *
 * func00/func01 are exported no-ops (other DLL entry slots).
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/modgfx_types.h"
#include "main/dll/partfx_interface.h"

/* base spawn flags; low bit positions the effect at the source object */
#define SPAWN_FLAGS_BASE        0xc010480
#define SPAWN_FLAG_USE_POSITION 1

/* effect id spawned by this DLL's modgfx emitter (spawnEffect textureAssetId arg). */
#define DLLA0_EFFECT_ID 0x1d9

extern u8 lbl_803186B0[]; /* texture/asset table */

void dll_A0_func03(GameObject* sourceObj, int variant, int posSource, u32 flags)
{
    ModgfxSpawnPacket buf;
    u8* tab = lbl_803186B0;
    GfxCmd* entry = buf.entries;
    GfxCmd* cmd;
    u32 fl;

    entry[0].layer = 0;
    entry[0].flags = 0x15;
    entry[0].tex = &tab[0x1b0];
    entry[0].mode = 4;
    entry[0].x = 0.0f;
    entry[0].y = 0.0f;
    entry[0].z = 0.0f;
    if (variant == 0)
    {
        entry[1].layer = 0;
        entry[1].flags = 0x15;
        entry[1].tex = &tab[0x1b0];
        entry[1].mode = 2;
        entry[1].x = 0.035f;
        entry[1].y = -0.45f;
        entry[1].z = 0.035f;
        cmd = entry + 2;
    }
    else
    {
        entry[1].layer = 0;
        entry[1].flags = 0x15;
        entry[1].tex = &tab[0x1b0];
        entry[1].mode = 2;
        entry[1].x = 0.035f;
        entry[1].y = 0.45f;
        entry[1].z = 0.035f;
        cmd = entry + 2;
    }
    cmd[0].layer = 1;
    cmd[0].flags = 0x15;
    cmd[0].tex = &tab[0x1b0];
    cmd[0].mode = 2;
    cmd[0].x = 10.0f;
    cmd[0].y = 10.0f;
    cmd[0].z = 10.0f;
    cmd[1].layer = 1;
    cmd[1].flags = 7;
    cmd[1].tex = &tab[0x174];
    cmd[1].mode = 4;
    cmd[1].x = 255.0f;
    cmd[1].y = 0.0f;
    cmd[1].z = 0.0f;
    cmd[2].layer = 1;
    cmd[2].flags = 0x15;
    cmd[2].tex = &tab[0x1b0];
    cmd[2].mode = 0x4000;
    cmd[2].x = -6.0f;
    cmd[2].y = 1.0f;
    cmd[2].z = 0.0f;
    cmd[3].layer = 2;
    cmd[3].flags = 7;
    cmd[3].tex = &tab[0x174];
    cmd[3].mode = 2;
    cmd[3].x = 2.0f;
    cmd[3].y = 1.0f;
    cmd[3].z = 2.0f;
    cmd[4].layer = 2;
    cmd[4].flags = 7;
    cmd[4].tex = &tab[0x184];
    cmd[4].mode = 2;
    cmd[4].x = 4.0f;
    cmd[4].y = 1.0f;
    cmd[4].z = 4.0f;
    cmd[5].layer = 2;
    cmd[5].flags = 0x15;
    cmd[5].tex = &tab[0x1b0];
    cmd[5].mode = 0x4000;
    cmd[5].x = -6.0f;
    cmd[5].y = 1.0f;
    cmd[5].z = 0.0f;
    cmd[6].layer = 3;
    cmd[6].flags = 7;
    cmd[6].tex = &tab[0x174];
    cmd[6].mode = 4;
    cmd[6].x = 0.0f;
    cmd[6].y = 0.0f;
    cmd[6].z = 0.0f;
    cmd[7].layer = 3;
    cmd[7].flags = 0x15;
    cmd[7].tex = &tab[0x1b0];
    cmd[7].mode = 0x4000;
    cmd[7].x = 6.0f;
    cmd[7].y = 1.0f;
    cmd[7].z = 0.0f;

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
    buf.count = &cmd[8] - entry;
    buf.hw[0] = *(s16*)&tab[0x1f8];
    buf.hw[1] = *(s16*)&tab[0x1fa];
    buf.hw[2] = *(s16*)&tab[0x1fc];
    buf.hw[3] = *(s16*)&tab[0x1fe];
    buf.hw[4] = *(s16*)&tab[0x200];
    buf.hw[5] = *(s16*)&tab[0x202];
    buf.hw[6] = *(s16*)&tab[0x204];
    buf.cmds = (GfxCmd*)((u8*)&buf + 0x60);
    fl = SPAWN_FLAGS_BASE;
    buf.flags = fl;
    fl |= flags;
    buf.flags = fl;
    if (fl & SPAWN_FLAG_USE_POSITION)
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
    (*gModgfxInterface)->spawnEffect(&buf, 0, 0x15, tab, 0x18, &tab[0xd4], DLLA0_EFFECT_ID, 0);
}

void dll_A0_func01_nop(void)
{
}

void dll_A0_func00_nop(void)
{
}

u8 lbl_803186B0[520] = {
    0,   0,   0,   0,   3,   232, 0,   0,   0,   0,   3,   98,  0,   0,   1,   244, 0,   0,   0,   11,  3,   98,  0,
    0,   254, 12,  0,   0,   0,   22,  0,   0,   0,   0,   252, 24,  0,   0,   0,   32,  252, 158, 0,   0,   254, 12,
    0,   0,   0,   22,  252, 158, 0,   0,   1,   244, 0,   0,   0,   11,  0,   0,   0,   0,   3,   232, 0,   0,   0,
    0,   0,   0,   1,   244, 3,   232, 0,   15,  0,   0,   3,   98,  1,   244, 1,   244, 0,   15,  0,   11,  3,   98,
    1,   244, 254, 12,  0,   15,  0,   22,  0,   0,   1,   244, 252, 24,  0,   15,  0,   32,  252, 158, 1,   244, 254,
    12,  0,   15,  0,   22,  252, 158, 1,   244, 1,   244, 0,   15,  0,   11,  0,   0,   1,   244, 3,   232, 0,   15,
    0,   0,   0,   0,   3,   232, 3,   232, 0,   31,  0,   0,   3,   98,  3,   232, 1,   244, 0,   31,  0,   11,  3,
    98,  3,   232, 254, 12,  0,   31,  0,   22,  0,   0,   3,   232, 252, 24,  0,   31,  0,   32,  252, 158, 3,   232,
    254, 12,  0,   31,  0,   22,  252, 158, 3,   232, 1,   244, 0,   31,  0,   11,  0,   0,   3,   232, 3,   232, 0,
    31,  0,   0,   0,   0,   0,   0,   0,   1,   0,   8,   0,   0,   0,   8,   0,   7,   0,   1,   0,   2,   0,   9,
    0,   1,   0,   9,   0,   8,   0,   2,   0,   3,   0,   10,  0,   2,   0,   10,  0,   9,   0,   3,   0,   4,   0,
    11,  0,   3,   0,   11,  0,   10,  0,   4,   0,   5,   0,   12,  0,   4,   0,   12,  0,   11,  0,   5,   0,   6,
    0,   13,  0,   5,   0,   13,  0,   12,  0,   7,   0,   8,   0,   15,  0,   7,   0,   15,  0,   14,  0,   8,   0,
    9,   0,   16,  0,   8,   0,   16,  0,   15,  0,   9,   0,   10,  0,   17,  0,   9,   0,   17,  0,   16,  0,   10,
    0,   11,  0,   18,  0,   10,  0,   18,  0,   17,  0,   11,  0,   12,  0,   19,  0,   11,  0,   19,  0,   18,  0,
    12,  0,   13,  0,   20,  0,   12,  0,   20,  0,   19,  0,   0,   0,   1,   0,   2,   0,   3,   0,   4,   0,   5,
    0,   6,   0,   0,   0,   7,   0,   8,   0,   9,   0,   10,  0,   11,  0,   12,  0,   13,  0,   0,   0,   14,  0,
    15,  0,   16,  0,   17,  0,   18,  0,   19,  0,   20,  0,   0,   0,   0,   0,   1,   0,   2,   0,   3,   0,   4,
    0,   5,   0,   6,   0,   14,  0,   15,  0,   16,  0,   17,  0,   18,  0,   19,  0,   20,  0,   0,   0,   1,   0,
    2,   0,   3,   0,   4,   0,   5,   0,   6,   0,   7,   0,   8,   0,   9,   0,   10,  0,   11,  0,   12,  0,   13,
    0,   14,  0,   15,  0,   16,  0,   17,  0,   18,  0,   19,  0,   20,  0,   0,   0,   7,   0,   8,   0,   9,   0,
    10,  0,   11,  0,   12,  0,   13,  0,   14,  0,   15,  0,   16,  0,   17,  0,   18,  0,   19,  0,   20,  0,   0,
    0,   35,  0,   6,   0,   35,  0,   0,   0,   0,   0,   0,   0,   0};
void* lbl_803188B8[8] = {(void*)0x00000000, (void*)0x00000000, (void*)0x00000000, (void*)0x00030000,
                         dll_A0_func00_nop, dll_A0_func01_nop, (void*)0x00000000, dll_A0_func03};
