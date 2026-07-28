/*
 * dll_00A1 func0 - pickup/collectible visual effect spawner (DLL 161 / 0xA1).
 *
 * dll_A1_func03 is the per-pickup effect builder: it fills a stack-resident
 * ModgfxInterface spawn request with 14 layered GfxCmd draw entries (the
 * sparkle/glint sprite stack) plus a header block (colour, scale, position,
 * flag word), then hands it to (*gModgfxInterface)->spawnEffect. When request
 * flag bit 0 is set the effect is anchored to a world position taken either
 * from sourceObj (+0x18 vector) or, when sourceObj is null, from posSource
 * (+0x0c vector). func00/func01 are empty DLL entry-point stubs.
 *
 * Draw geometry/colour constants come from .sdata2 (lbl_803E14xx), while
 * this unit owns the sprite asset table lbl_803188D8 in .data.
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/modgfx_types.h"
#include "main/dll/partfx_interface.h"
#include "game/objects/object.h"
#include "main/dll/pickup.h"
#include "main/dll/dll_00A1_dlla1func0.h"

/* base spawn flags; low bit positions the effect at the source object */
#define SPAWN_FLAGS_BASE        0xc0104c0
#define SPAWN_FLAG_USE_POSITION 1

/* effect id spawned by this DLL's modgfx emitter (spawnEffect textureAssetId arg). */
#define DLLA1_EFFECT_ID 0x203

extern u8 lbl_803188D8[];

void dll_A1_func03(GameObject* sourceObj, int variant, u8* posSource, u32 flags)
{
    ModgfxSpawnPacket buf;
    u8* assets = (u8*)(int)lbl_803188D8;
    GfxCmd* cmd = buf.entries;

    cmd[0].layer = 0;
    cmd[0].flags = 0x15;
    cmd[0].tex = &assets[0x1b0];
    cmd[0].mode = 4;
    cmd[0].x = 0.0f;
    cmd[0].y = 0.0f;
    cmd[0].z = 0.0f;
    cmd[1].layer = 0;
    cmd[1].flags = 0x15;
    cmd[1].tex = &assets[0x1b0];
    cmd[1].mode = 2;
    cmd[1].x = 0.01f;
    cmd[1].y = 0.01f;
    cmd[1].z = 0.05f;
    cmd[2].layer = 1;
    cmd[2].flags = 0x15;
    cmd[2].tex = &assets[0x1b0];
    cmd[2].mode = 4;
    cmd[2].x = 255.0f;
    cmd[2].y = 0.0f;
    cmd[2].z = 0.0f;
    cmd[3].layer = 1;
    cmd[3].flags = 0x15;
    cmd[3].tex = &assets[0x1b0];
    cmd[3].mode = 0x4000;
    cmd[3].x = 1.0f;
    cmd[3].y = -6.0f;
    cmd[3].z = 0.0f;
    cmd[4].layer = 1;
    cmd[4].flags = 0x15;
    cmd[4].tex = &assets[0x1b0];
    cmd[4].mode = 2;
    cmd[4].x = 5.0f;
    cmd[4].y = 5.0f;
    cmd[4].z = 500.0f;
    cmd[5].layer = 2;
    cmd[5].flags = 0x15;
    cmd[5].tex = &assets[0x1b0];
    cmd[5].mode = 0x4000;
    cmd[5].x = 1.0f;
    cmd[5].y = -6.0f;
    cmd[5].z = 0.0f;
    cmd[6].layer = 3;
    cmd[6].flags = 1;
    cmd[6].tex = NULL;
    cmd[6].mode = 0x2000;
    cmd[6].x = 0.0f;
    cmd[6].y = 0.0f;
    cmd[6].z = 0.0f;
    cmd[7].layer = 4;
    cmd[7].flags = 0x15;
    cmd[7].tex = &assets[0x1b0];
    cmd[7].mode = 2;
    cmd[7].x = 3.5f;
    cmd[7].y = 3.5f;
    cmd[7].z = 1.0f;
    cmd[8].layer = 4;
    cmd[8].flags = 0x15;
    cmd[8].tex = &assets[0x1b0];
    cmd[8].mode = 0x4000;
    cmd[8].x = 1.0f;
    cmd[8].y = -6.0f;
    cmd[8].z = 0.0f;
    cmd[9].layer = 4;
    cmd[9].flags = 0x6dd;
    cmd[9].tex = NULL;
    cmd[9].mode = 0x800000;
    cmd[9].x = 1.0f;
    cmd[9].y = 0.0f;
    cmd[9].z = 0.0f;
    cmd[10].layer = 5;
    cmd[10].flags = 0x15;
    cmd[10].tex = &assets[0x1b0];
    cmd[10].mode = 0x4000;
    cmd[10].x = 1.0f;
    cmd[10].y = -6.0f;
    cmd[10].z = 0.0f;
    cmd[11].layer = 5;
    cmd[11].flags = 0x6de;
    cmd[11].tex = NULL;
    cmd[11].mode = 0x800000;
    cmd[11].x = 5.0f;
    cmd[11].y = 0.0f;
    cmd[11].z = 0.0f;
    cmd[12].layer = 5;
    cmd[12].flags = 0x6dd;
    cmd[12].tex = NULL;
    cmd[12].mode = 0x800000;
    cmd[12].x = 1.0f;
    cmd[12].y = 0.0f;
    cmd[12].z = 0.0f;
    cmd[13].layer = 6;
    cmd[13].flags = 4;
    cmd[13].tex = NULL;
    cmd[13].mode = 0x2000;
    cmd[13].x = 0.0f;
    cmd[13].y = 0.0f;
    cmd[13].z = 0.0f;

    buf.v58 = 0;
    buf.ctx = (int)sourceObj;
    buf.v44 = variant;
    buf.pos[0] = 0.0f;
    buf.pos[1] = 0.0f;
    buf.pos[2] = 0.0f;
    buf.col[0] = 0.0f;
    buf.col[1] = 0.0f;
    buf.col[2] = 0.0f;
    buf.scale = 2.0f;
    buf.v40 = 2;
    buf.v3c = 7;
    buf.v59 = 0xe;
    buf.v5a = 0;
    buf.v5b = 0x1e;
    buf.count = (GfxCmd*)((u8*)cmd + 0x150) - cmd;
    buf.hw[0] = *(s16*)&assets[0x1f8];
    buf.hw[1] = *(s16*)&assets[0x1fa];
    buf.hw[2] = *(s16*)&assets[0x1fc];
    buf.hw[3] = *(s16*)&assets[0x1fe];
    buf.hw[4] = *(s16*)&assets[0x200];
    buf.hw[5] = *(s16*)&assets[0x202];
    buf.hw[6] = *(s16*)&assets[0x204];
    buf.cmds = cmd;
    buf.flags = SPAWN_FLAGS_BASE;
    buf.flags |= flags;
    if (buf.flags & SPAWN_FLAG_USE_POSITION)
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
    (*gModgfxInterface)->spawnEffect(&buf, 0, 0x15, (u8*)(int)lbl_803188D8, 0x18, &assets[0xd4], DLLA1_EFFECT_ID, 0);
}

void dll_A1_func01_nop(void)
{
}

void dll_A1_func00_nop(void)
{
}

u8 lbl_803188D8[520] = {
    0,   0,   3,   232, 0,   0,   0,   0,   0,   0,   3,   98,  1,  244, 0,   0,   0,   11,  0,   0,   3,   98,  254,
    12,  0,   0,   0,   22,  0,   0,   0,   0,   252, 24,  0,   0,  0,   32,  0,   0,   252, 158, 254, 12,  0,   0,
    0,   22,  0,   0,   252, 158, 1,   244, 0,   0,   0,   11,  0,  0,   0,   0,   3,   232, 0,   0,   0,   0,   0,
    0,   0,   0,   3,   232, 1,   244, 0,   0,   0,   120, 3,   98, 1,   244, 1,   244, 0,   11,  0,   120, 3,   98,
    254, 12,  1,   244, 0,   22,  0,   120, 0,   0,   252, 24,  1,  244, 0,   32,  0,   120, 252, 158, 254, 12,  1,
    244, 0,   22,  0,   120, 252, 158, 1,   244, 1,   244, 0,   11, 0,   120, 0,   0,   3,   232, 1,   244, 0,   0,
    0,   120, 0,   0,   3,   232, 3,   232, 0,   0,   0,   248, 3,  98,  1,   244, 3,   232, 0,   11,  0,   248, 3,
    98,  254, 12,  3,   232, 0,   22,  0,   248, 0,   0,   252, 24, 3,   232, 0,   32,  0,   248, 252, 158, 254, 12,
    3,   232, 0,   22,  0,   248, 252, 158, 1,   244, 3,   232, 0,  11,  0,   248, 0,   0,   3,   232, 3,   232, 0,
    0,   0,   248, 0,   0,   0,   0,   0,   1,   0,   8,   0,   0,  0,   8,   0,   7,   0,   1,   0,   2,   0,   9,
    0,   1,   0,   9,   0,   8,   0,   2,   0,   3,   0,   10,  0,  2,   0,   10,  0,   9,   0,   3,   0,   4,   0,
    11,  0,   3,   0,   11,  0,   10,  0,   4,   0,   5,   0,   12, 0,   4,   0,   12,  0,   11,  0,   5,   0,   6,
    0,   13,  0,   5,   0,   13,  0,   12,  0,   7,   0,   8,   0,  15,  0,   7,   0,   15,  0,   14,  0,   8,   0,
    9,   0,   16,  0,   8,   0,   16,  0,   15,  0,   9,   0,   10, 0,   17,  0,   9,   0,   17,  0,   16,  0,   10,
    0,   11,  0,   18,  0,   10,  0,   18,  0,   17,  0,   11,  0,  12,  0,   19,  0,   11,  0,   19,  0,   18,  0,
    12,  0,   13,  0,   20,  0,   12,  0,   20,  0,   19,  0,   0,  0,   1,   0,   2,   0,   3,   0,   4,   0,   5,
    0,   6,   0,   0,   0,   7,   0,   8,   0,   9,   0,   10,  0,  11,  0,   12,  0,   13,  0,   0,   0,   14,  0,
    15,  0,   16,  0,   17,  0,   18,  0,   19,  0,   20,  0,   0,  0,   0,   0,   1,   0,   2,   0,   3,   0,   4,
    0,   5,   0,   6,   0,   14,  0,   15,  0,   16,  0,   17,  0,  18,  0,   19,  0,   20,  0,   0,   0,   1,   0,
    2,   0,   3,   0,   4,   0,   5,   0,   6,   0,   7,   0,   8,  0,   9,   0,   10,  0,   11,  0,   12,  0,   13,
    0,   14,  0,   15,  0,   16,  0,   17,  0,   18,  0,   19,  0,  20,  0,   0,   0,   7,   0,   8,   0,   9,   0,
    10,  0,   11,  0,   12,  0,   13,  0,   14,  0,   15,  0,   16, 0,   17,  0,   18,  0,   19,  0,   20,  0,   0,
    0,   60,  0,   10,  0,   1,   0,   60,  0,   10,  0,   1,   0,  0};
void* lbl_80318AE0[8] = {(void*)0x00000000, (void*)0x00000000, (void*)0x00000000, (void*)0x00030000,
                         dll_A1_func00_nop, dll_A1_func01_nop, (void*)0x00000000, dll_A1_func03};
