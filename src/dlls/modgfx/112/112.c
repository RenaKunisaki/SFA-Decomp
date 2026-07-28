/*
 * DLL 112 / 0x70 - particle/gfx spawn helper.
 *
 * The unit owns three entry points:
 *   dll_70_func00_nop / dll_70_func01_nop - empty stubs.
 *   dll_70_func03 - builds a 27-command graphics command list (GfxCmd[])
 *     on the stack from a layout table (gDll70Func03GfxLayoutTable) and a set of shared
 *     float constants, then submits it via gModgfxInterface->spawnEffect.
 *     Bit 0 of the spawn-context flags word (buf.flags) enables
 *     world-position override: a non-null sourceObj uses the GameObject
 *     world position, a null one uses posSource as a PartFxSpawnParams packet.
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/partfx_interface.h"
#include "main/dll/modgfx_types.h"

u8 lbl_803DB8D0[8] = {0, 0x0A, 0, 0x0C, 0, 0x0E, 0, 0x10};

/* effect id spawned by this DLL's modgfx emitter (spawnEffect textureAssetId arg). */
#define DLL70_EFFECT_ID 0x45

extern u8 gDll70Func03GfxLayoutTable[];

void dll_70_func03(int sourceObj, int variant, int posSource, u32 flags)
{
    ModgfxSpawnPacket buf;
    u8* base = (u8*)(int)gDll70Func03GfxLayoutTable;
    GfxCmd* e = buf.entries;
    e[0].layer = 0;
    e[0].flags = 0x12;
    e[0].tex = &base[336];
    e[0].mode = 4;
    e[0].x = 0.0f;
    e[0].y = 0.0f;
    e[0].z = 0.0f;
    e[1].layer = 0;
    e[1].flags = 9;
    e[1].tex = &base[276];
    e[1].mode = 8;
    e[1].x = 255.0f;
    e[1].y = 255.0f;
    e[1].z = 0.0f;
    e[2].layer = 0;
    e[2].flags = 9;
    e[2].tex = &base[296];
    e[2].mode = 2;
    e[2].x = 1.0f;
    e[2].y = 0.01f;
    e[2].z = 1.0f;
    e[3].layer = 0;
    e[3].flags = 0x12;
    e[3].tex = &base[336];
    e[3].mode = 2;
    e[3].x = 3.5f;
    e[3].y = 1.0f;
    e[3].z = 3.5f;
    e[4].layer = 0;
    e[4].flags = 9;
    e[4].tex = &base[296];
    e[4].mode = 8;
    e[4].x = 205.0f;
    e[4].y = 0.0f;
    e[4].z = 0.0f;
    e[5].layer = 0;
    e[5].flags = 1;
    e[5].tex = NULL;
    e[5].mode = 0x8000;
    e[5].x = 255.0f;
    e[5].y = 125.0f;
    e[5].z = 0.0f;
    e[6].layer = 0;
    e[6].flags = 0;
    e[6].tex = NULL;
    e[6].mode = 0x80000;
    e[6].x = 0.0f;
    e[6].y = 10.0f;
    e[6].z = 0.0f;
    e[7].layer = 1;
    e[7].flags = 0x12;
    e[7].tex = &base[336];
    e[7].mode = 4;
    e[7].x = 255.0f;
    e[7].y = 0.0f;
    e[7].z = 0.0f;
    e[8].layer = 1;
    e[8].flags = 9;
    e[8].tex = &base[296];
    e[8].mode = 2;
    e[8].x = 1.0f;
    e[8].y = 150.0f;
    e[8].z = 1.0f;
    e[9].layer = 1;
    e[9].flags = 0x7a;
    e[9].tex = NULL;
    e[9].mode = 0x10000;
    e[9].x = 0.0f;
    e[9].y = 0.0f;
    e[9].z = 0.0f;
    e[10].layer = 1;
    e[10].flags = 0;
    e[10].tex = NULL;
    e[10].mode = 0x80000;
    e[10].x = 0.0f;
    e[10].y = 10.0f;
    e[10].z = 0.0f;
    e[11].layer = 2;
    e[11].flags = 0x9d;
    e[11].tex = NULL;
    e[11].mode = 0x20000;
    e[11].x = 0.0f;
    e[11].y = 0.0f;
    e[11].z = 0.0f;
    e[12].layer = 3;
    e[12].flags = 9;
    e[12].tex = &base[276];
    e[12].mode = 8;
    e[12].x = 255.0f;
    e[12].y = 155.0f;
    e[12].z = 0.0f;
    e[13].layer = 3;
    e[13].flags = 0x12;
    e[13].tex = &base[336];
    e[13].mode = 0x100;
    e[13].x = 0.0f;
    e[13].y = 0.0f;
    e[13].z = -10.0f;
    e[14].layer = 3;
    e[14].flags = 5;
    e[14].tex = &base[392];
    e[14].mode = 2;
    e[14].x = 0.98f;
    e[14].y = 1.0f;
    e[14].z = 0.98f;
    e[15].layer = 3;
    e[15].flags = 4;
    e[15].tex = lbl_803DB8D0;
    e[15].mode = 2;
    e[15].x = 1.02f;
    e[15].y = 1.0f;
    e[15].z = 1.02f;
    e[16].layer = 3;
    e[16].flags = 0;
    e[16].tex = NULL;
    e[16].mode = 0x80000;
    e[16].x = 0.0f;
    e[16].y = -30.0f;
    e[16].z = 0.0f;
    e[17].layer = 4;
    e[17].flags = 9;
    e[17].tex = &base[276];
    e[17].mode = 8;
    e[17].x = 255.0f;
    e[17].y = 255.0f;
    e[17].z = 0.0f;
    e[18].layer = 4;
    e[18].flags = 0x12;
    e[18].tex = &base[336];
    e[18].mode = 0x100;
    e[18].x = 0.0f;
    e[18].y = 0.0f;
    e[18].z = -10.0f;
    e[19].layer = 4;
    e[19].flags = 5;
    e[19].tex = &base[392];
    e[19].mode = 2;
    e[19].x = 1.02f;
    e[19].y = 1.0f;
    e[19].z = 1.02f;
    e[20].layer = 4;
    e[20].flags = 4;
    e[20].tex = lbl_803DB8D0;
    e[20].mode = 2;
    e[20].x = 0.98f;
    e[20].y = 1.0f;
    e[20].z = 0.98f;
    e[21].layer = 5;
    e[21].flags = 2;
    e[21].tex = NULL;
    e[21].mode = 0x1000;
    e[21].x = 1.0f;
    e[21].y = 0.0f;
    e[21].z = 0.0f;
    e[22].layer = 6;
    e[22].flags = 0x9d;
    e[22].tex = NULL;
    e[22].mode = 0x20000;
    e[22].x = 0.0f;
    e[22].y = 0.0f;
    e[22].z = 0.0f;
    e[23].layer = 6;
    e[23].flags = 0x9b;
    e[23].tex = NULL;
    e[23].mode = 0x10000;
    e[23].x = 0.0f;
    e[23].y = 0.0f;
    e[23].z = 0.0f;
    e[24].layer = 6;
    e[24].flags = 0x12;
    e[24].tex = &base[336];
    e[24].mode = 4;
    e[24].x = 0.0f;
    e[24].y = 0.0f;
    e[24].z = 0.0f;
    e[25].layer = 6;
    e[25].flags = 0x12;
    e[25].tex = &base[336];
    e[25].mode = 2;
    e[25].x = 2.0f;
    e[25].y = 1.0f;
    e[25].z = 2.0f;
    e[26].layer = 6;
    e[26].flags = 0;
    e[26].tex = NULL;
    e[26].mode = 0x80000;
    e[26].x = 0.0f;
    e[26].y = -30.0f;
    e[26].z = 0.0f;
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
    buf.count = 27;
    buf.hw[0] = *(s16*)&base[404];
    buf.hw[1] = *(s16*)&base[406];
    buf.hw[2] = *(s16*)&base[408];
    buf.hw[3] = *(s16*)&base[410];
    buf.hw[4] = *(s16*)&base[412];
    buf.hw[5] = *(s16*)&base[414];
    buf.hw[6] = *(s16*)&base[416];
    buf.cmds = (GfxCmd*)((u8*)&buf.col[0] + 0x40);
    buf.flags |= flags;
    if ((buf.flags & 1) != 0)
    {
        if ((void*)sourceObj != NULL)
        {
            buf.pos[0] += ((GameObject*)(sourceObj))->anim.worldPosX;
            buf.pos[1] += ((GameObject*)(sourceObj))->anim.worldPosY;
            buf.pos[2] += ((GameObject*)(sourceObj))->anim.worldPosZ;
        }
        else
        {
            buf.pos[0] += ((PartFxSpawnParams*)posSource)->posX;
            buf.pos[1] += ((PartFxSpawnParams*)posSource)->posY;
            buf.pos[2] += ((PartFxSpawnParams*)posSource)->posZ;
        }
    }
    (*gModgfxInterface)
        ->spawnEffect(&buf, 0, 0x12, (u8*)(int)gDll70Func03GfxLayoutTable, 0x10, &base[180], DLL70_EFFECT_ID, 0);
}

void dll_70_func01_nop(void)
{
}

void dll_70_func00_nop(void)
{
}

u8 gDll70Func03GfxLayoutTable[420] = {
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

u32 lbl_8031403C[9] = {
    0x00000000, 0x00000000,         0x00000000, 0x00030000, (u32)dll_70_func00_nop, (u32)dll_70_func01_nop,
    0x00000000, (u32)dll_70_func03, 0x00000000};
