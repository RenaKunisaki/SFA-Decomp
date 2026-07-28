/*
 * DLL 145 / 0x91. Holds the func00/func01 no-op slots plus
 * dll_91_func03, which assembles a fixed 19-entry modgfx command list (one
 * GfxCmd per sub-effect: per-layer texture, draw mode, and a position scale
 * triple) into a stack ModgfxSpawnPacket, optionally biases the spawn position by the
 * source object's world position (flags bit 0), then hands the buffer to
 * (*gModgfxInterface)->spawnEffect. The texture pointers index a shared
 * resource blob (gDll91Func0ResourceBlob); the position/scale constants live in a
 * shared float pool (0.0f..2.0f).
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/modgfx_types.h"
#include "main/dll/partfx_interface.h"

/* effect id spawned by this DLL's modgfx emitter (spawnEffect textureAssetId arg). */
#define DLL91_EFFECT_ID 0x45

extern u8 gDll91Func0ResourceBlob[];

u8 gDll91Func0Tex[8] = {0, 0x0A, 0, 0x0C, 0, 0x0E, 0, 0x10};

void dll_91_func03(int sourceObj, int variant, int posSource, u32 flags)
{
    ModgfxSpawnPacket buf;
    u8* base = (u8*)(int)gDll91Func0ResourceBlob;
    GfxCmd* entry = buf.entries;

    entry[0].layer = 0;
    entry[0].flags = 0x12;
    entry[0].tex = base + 0x150;
    entry[0].mode = 4;
    entry[0].x = 0.0f;
    entry[0].y = 0.0f;
    entry[0].z = 0.0f;
    entry[1].layer = 0;
    entry[1].flags = 9;
    entry[1].tex = base + 0x114;
    entry[1].mode = 8;
    entry[1].x = 0.0f;
    entry[1].y = 0.0f;
    entry[1].z = 255.0f;
    entry[2].layer = 0;
    entry[2].flags = 9;
    entry[2].tex = base + 0x128;
    entry[2].mode = 2;
    entry[2].x = 3.0f;
    entry[2].y = 0.03f;
    entry[2].z = 3.0f;
    entry[3].layer = 0;
    entry[3].flags = 0x12;
    entry[3].tex = base + 0x150;
    entry[3].mode = 2;
    entry[3].x = 1.75f;
    entry[3].y = 0.5f;
    entry[3].z = 1.75f;
    entry[4].layer = 0;
    entry[4].flags = 9;
    entry[4].tex = base + 0x128;
    entry[4].mode = 8;
    entry[4].x = 255.0f;
    entry[4].y = 0.0f;
    entry[4].z = 255.0f;
    entry[5].layer = 1;
    entry[5].flags = 0x12;
    entry[5].tex = base + 0x150;
    entry[5].mode = 4;
    entry[5].x = 255.0f;
    entry[5].y = 0.0f;
    entry[5].z = 0.0f;
    entry[6].layer = 1;
    entry[6].flags = 9;
    entry[6].tex = base + 0x128;
    entry[6].mode = 2;
    entry[6].x = 1.0f;
    entry[6].y = 150.0f;
    entry[6].z = 1.0f;
    entry[7].layer = 2;
    entry[7].flags = 0;
    entry[7].tex = NULL;
    entry[7].mode = 0x20;
    entry[7].x = 0.0f;
    entry[7].y = 0.0f;
    entry[7].z = 0.0f;
    entry[8].layer = 3;
    entry[8].flags = 9;
    entry[8].tex = base + 0x114;
    entry[8].mode = 8;
    entry[8].x = 255.0f;
    entry[8].y = 155.0f;
    entry[8].z = 0.0f;
    entry[9].layer = 3;
    entry[9].flags = 0x12;
    entry[9].tex = base + 0x150;
    entry[9].mode = 0x100;
    entry[9].x = 0.0f;
    entry[9].y = 0.0f;
    entry[9].z = -10.0f;
    entry[10].layer = 3;
    entry[10].flags = 5;
    entry[10].tex = base + 0x188;
    entry[10].mode = 2;
    entry[10].x = 0.98f;
    entry[10].y = 1.0f;
    entry[10].z = 0.98f;
    entry[11].layer = 3;
    entry[11].flags = 4;
    entry[11].tex = gDll91Func0Tex;
    entry[11].mode = 2;
    entry[11].x = 1.02f;
    entry[11].y = 1.0f;
    entry[11].z = 1.02f;
    entry[12].layer = 4;
    entry[12].flags = 9;
    entry[12].tex = base + 0x114;
    entry[12].mode = 8;
    entry[12].x = 255.0f;
    entry[12].y = 0.0f;
    entry[12].z = 255.0f;
    entry[13].layer = 4;
    entry[13].flags = 0x12;
    entry[13].tex = base + 0x150;
    entry[13].mode = 0x100;
    entry[13].x = 0.0f;
    entry[13].y = 0.0f;
    entry[13].z = -10.0f;
    entry[14].layer = 4;
    entry[14].flags = 5;
    entry[14].tex = base + 0x188;
    entry[14].mode = 2;
    entry[14].x = 1.02f;
    entry[14].y = 1.0f;
    entry[14].z = 1.02f;
    entry[15].layer = 4;
    entry[15].flags = 4;
    entry[15].tex = gDll91Func0Tex;
    entry[15].mode = 2;
    entry[15].x = 0.98f;
    entry[15].y = 1.0f;
    entry[15].z = 0.98f;
    entry[16].layer = 5;
    entry[16].flags = 2;
    entry[16].tex = NULL;
    entry[16].mode = 0x1000;
    entry[16].x = 1.0f;
    entry[16].y = 0.0f;
    entry[16].z = 0.0f;
    entry[17].layer = 6;
    entry[17].flags = 0x12;
    entry[17].tex = base + 0x150;
    entry[17].mode = 4;
    entry[17].x = 0.0f;
    entry[17].y = 0.0f;
    entry[17].z = 0.0f;
    entry[18].layer = 6;
    entry[18].flags = 0x12;
    entry[18].tex = base + 0x150;
    entry[18].mode = 2;
    entry[18].x = 2.0f;
    entry[18].y = 1.0f;
    entry[18].z = 2.0f;
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
    buf.v40 = 1;
    buf.v3c = 0;
    buf.v59 = 0x12;
    buf.v5a = 0;
    buf.v5b = 0xc;
    buf.flags = 0x1000082;
    buf.count = (GfxCmd*)((u8*)entry + 0x1c8) - entry;
    buf.hw[0] = *(s16*)(base + 0x194);
    buf.hw[1] = *(s16*)(base + 0x196);
    buf.hw[2] = *(s16*)(base + 0x198);
    buf.hw[3] = *(s16*)(base + 0x19a);
    buf.hw[4] = *(s16*)(base + 0x19c);
    buf.hw[5] = *(s16*)(base + 0x19e);
    buf.hw[6] = *(s16*)(base + 0x1a0);
    buf.cmds = entry;
    buf.flags |= flags;
    if ((buf.flags & 1) != 0)
    {
        if ((u32)sourceObj != 0)
        {
            GameObject* obj = (GameObject*)sourceObj;
            buf.pos[0] += obj->anim.worldPosX;
            buf.pos[1] += obj->anim.worldPosY;
            buf.pos[2] += obj->anim.worldPosZ;
        }
        else
        {
            PartFxSpawnParams* params = (PartFxSpawnParams*)posSource;
            buf.pos[0] += params->posX;
            buf.pos[1] += params->posY;
            buf.pos[2] += params->posZ;
        }
    }
    (*gModgfxInterface)
        ->spawnEffect(&buf, 0, 0x12, (u8*)(int)gDll91Func0ResourceBlob, 0x10, base + 0xb4, DLL91_EFFECT_ID, 0);
}

void dll_91_func01_nop(void)
{
}

void dll_91_func00_nop(void)
{
}

u8 gDll91Func0ResourceBlob[420] = {
    3,   232, 0,   0,   0,   0,   0,   0,   0,   0,   2,   195, 0, 0,   253, 61,  0,   15,  0,   0,   0,   0,   0, 0,
    252, 24,  0,   31,  0,   0,   253, 61,  0,   0,   253, 61,  0, 47,  0,   0,   252, 24,  0,   0,   0,   0,   0, 63,
    0,   0,   253, 61,  0,   0,   2,   195, 0,   79,  0,   0,   0, 0,   0,   0,   3,   232, 0,   95,  0,   0,   2, 195,
    0,   0,   2,   195, 0,   111, 0,   0,   3,   232, 0,   0,   0, 0,   0,   127, 0,   0,   3,   232, 7,   208, 0, 0,
    0,   0,   0,   31,  2,   195, 7,   208, 253, 61,  0,   15,  0, 31,  0,   0,   7,   208, 252, 24,  0,   31,  0, 31,
    253, 61,  7,   208, 253, 61,  0,   47,  0,   31,  252, 24,  7, 208, 0,   0,   0,   63,  0,   31,  253, 61,  7, 208,
    2,   195, 0,   79,  0,   31,  0,   0,   7,   208, 3,   232, 0, 95,  0,   31,  2,   195, 7,   208, 2,   195, 0, 111,
    0,   31,  3,   232, 7,   208, 0,   0,   0,   127, 0,   31,  0, 0,   0,   1,   0,   10,  0,   0,   0,   10,  0, 9,
    0,   1,   0,   2,   0,   11,  0,   1,   0,   11,  0,   10,  0, 2,   0,   3,   0,   12,  0,   2,   0,   12,  0, 11,
    0,   3,   0,   4,   0,   13,  0,   3,   0,   13,  0,   12,  0, 4,   0,   5,   0,   14,  0,   4,   0,   14,  0, 13,
    0,   5,   0,   6,   0,   15,  0,   5,   0,   15,  0,   14,  0, 6,   0,   7,   0,   16,  0,   6,   0,   16,  0, 15,
    0,   7,   0,   8,   0,   17,  0,   7,   0,   17,  0,   16,  0, 0,   0,   1,   0,   2,   0,   3,   0,   4,   0, 5,
    0,   6,   0,   7,   0,   8,   0,   0,   0,   9,   0,   10,  0, 11,  0,   12,  0,   13,  0,   14,  0,   15,  0, 16,
    0,   17,  0,   0,   0,   18,  0,   19,  0,   20,  0,   21,  0, 22,  0,   23,  0,   24,  0,   25,  0,   26,  0, 0,
    0,   0,   0,   1,   0,   2,   0,   3,   0,   4,   0,   5,   0, 6,   0,   7,   0,   8,   0,   9,   0,   10,  0, 11,
    0,   12,  0,   13,  0,   14,  0,   15,  0,   16,  0,   17,  0, 0,   0,   2,   0,   4,   0,   6,   0,   8,   0, 10,
    0,   12,  0,   14,  0,   16,  0,   0,   0,   9,   0,   11,  0, 13,  0,   15,  0,   17,  0,   0,   0,   0,   0, 45,
    0,   0,   0,   18,  0,   18,  0,   0,   0,   30,  0,   0,
};
u32 lbl_8031719C[9] = {
    0x00000000, 0x00000000,         0x00000000, 0x00030000, (u32)dll_91_func00_nop, (u32)dll_91_func01_nop,
    0x00000000, (u32)dll_91_func03, 0x00000000};
