/*
 * DLL 94 / 0x5E - a modgfx particle-sequence spawn stub DLL.
 *
 * dll_5E_func03 spawns a modgfx particle sequence from the embedded
 * gDll5EFunc03SequenceData block; the two tiny dll_5E entry stubs are
 * no-ops.
 */
#include "main/dll/modgfx_interface.h"
#include "game/objects/object.h"
#include "main/mapEventTypes.h"
#include "main/dll/dll_005E_dll5efunc0.h"

extern u8 gDll5EFunc03SequenceData[];

void dll_5E_func03(int sourceObj, int variant, u8* posSource, u32 flags)
{
    u8* base = (u8*)(int)gDll5EFunc03SequenceData;
    (*gModgfxInterface)->beginSequence((void*)sourceObj, (u8)variant, 0x12, 3, 9);
    (*gModgfxInterface)->setSequenceParams(&base[0x2cc]);
    (*gModgfxInterface)->addSequenceFlags(flags | 0x4004484);
    (*gModgfxInterface)->resetSequenceSpawns();
    (*gModgfxInterface)->addSequenceSpawn(2, 0.01f, 0.02f, 0.01f, 9, &base[0x1c8]);
    (*gModgfxInterface)->addSequenceSpawn(2, 0.015f, 0.02f, 0.014f, 9, &base[0x1dc]);
    (*gModgfxInterface)->addSequenceSpawn(2, 0.015f, 0.02f, 0.015f, 9, &base[0x1f0]);
    (*gModgfxInterface)->addSequenceSpawn(2, 0.015f, 0.02f, 0.015f, 9, &base[0x204]);
    (*gModgfxInterface)->addSequenceSpawn(4, 0.0f, 0.0f, 0.0f, 0x24, &base[0x260]);
    (*gModgfxInterface)->addSequenceSpawn(8, 175.0f, 165.0f, 40.0f, 0x24, &base[0x260]);
    (*gModgfxInterface)->nextSequenceParam();
    (*gModgfxInterface)->addSequenceSpawn(2, 37.0f, 11.0f, 37.0f, 0, NULL);
    (*gModgfxInterface)->addSequenceSpawn(0x4000, 0.0f, -4.0f, 0.0f, 0, NULL);
    (*gModgfxInterface)->addSequenceSpawn(0x1800000, 1.0f, 1.0f, 3.0f, 0x5e0, NULL);
    (*gModgfxInterface)->nextSequenceParam();
    (*gModgfxInterface)->addSequenceSpawn(4, 254.0f, 0.0f, 0.0f, 0x12, &base[0x2a8]);
    (*gModgfxInterface)->addSequenceSpawn(0x4000, 0.0f, -4.0f, 0.0f, 0x24, &base[0x260]);
    (*gModgfxInterface)->addSequenceSpawn(0x100, 0.0f, 0.0f, 1800.0f, 0, NULL);
    (*gModgfxInterface)->addSequenceSpawn(0x1800000, 1.0f, 1.0f, 3.0f, 0x5e0, NULL);
    (*gModgfxInterface)->nextSequenceParam();
    (*gModgfxInterface)->addSequenceSpawn(0x4000, 0.0f, -4.0f, 0.0f, 0x24, &base[0x260]);
    (*gModgfxInterface)->addSequenceSpawn(0x100, 0.0f, 0.0f, 1800.0f, 0, NULL);
    (*gModgfxInterface)->addSequenceSpawn(0x1800000, 1.0f, 1.0f, 3.0f, 0x5e0, NULL);
    (*gModgfxInterface)->nextSequenceParam();
    (*gModgfxInterface)->addSequenceSpawn(0x4000, 0.0f, -4.0f, 0.0f, 0x24, &base[0x260]);
    (*gModgfxInterface)->addSequenceSpawn(0x100, 0.0f, 0.0f, 1800.0f, 0, NULL);
    (*gModgfxInterface)->addSequenceSpawn(4, 0.0f, 0.0f, 0.0f, 0x24, &base[0x260]);
    (*gModgfxInterface)
        ->spawnSequence(posSource, (u8*)(int)gDll5EFunc03SequenceData, 0x24, &base[0x168], 0x10, 0x120, 0);
    (*gModgfxInterface)->getLastSpawnHandle();
}

void dll_5E_func01_nop(void)
{
}

void dll_5E_func00_nop(void)
{
}

u8 gDll5EFunc03SequenceData[748] = {
    4,   76,  0,   0,   0,   0,   0,   0,   0,   0,   3,   39,  0,   0,   253, 61,  0,   15,  0,   0,   0,   0,   0,
    0,   252, 24,  0,   31,  0,   0,   253, 161, 0,   0,   253, 61,  0,   47,  0,   0,   252, 124, 0,   0,   0,   0,
    0,   63,  0,   0,   253, 161, 0,   0,   2,   195, 0,   79,  0,   0,   0,   0,   0,   0,   3,   232, 0,   95,  0,
    0,   3,   39,  0,   0,   2,   195, 0,   111, 0,   0,   4,   76,  0,   0,   0,   0,   0,   127, 0,   0,   4,   176,
    7,   208, 0,   100, 0,   0,   0,   31,  3,   39,  7,   208, 253, 161, 0,   15,  0,   31,  0,   100, 7,   208, 252,
    124, 0,   31,  0,   31,  253, 161, 7,   208, 253, 161, 0,   47,  0,   31,  252, 124, 7,   208, 0,   100, 0,   63,
    0,   31,  253, 161, 7,   208, 3,   39,  0,   79,  0,   31,  0,   0,   7,   208, 4,   76,  0,   95,  0,   31,  3,
    39,  7,   208, 3,   39,  0,   111, 0,   31,  4,   176, 7,   208, 0,   100, 0,   127, 0,   31,  3,   132, 15,  160,
    0,   100, 0,   0,   0,   63,  2,   95,  15,  160, 253, 161, 0,   15,  0,   63,  255, 156, 15,  160, 252, 124, 0,
    31,  0,   63,  252, 217, 15,  160, 253, 161, 0,   47,  0,   63,  251, 180, 15,  160, 0,   100, 0,   63,  0,   63,
    252, 217, 15,  160, 3,   39,  0,   79,  0,   63,  0,   100, 15,  160, 4,   76,  0,   95,  0,   63,  2,   95,  15,
    160, 3,   39,  0,   111, 0,   63,  3,   132, 15,  160, 0,   100, 0,   127, 0,   63,  3,   232, 23,  112, 255, 156,
    0,   0,   0,   94,  2,   195, 23,  112, 252, 217, 0,   15,  0,   94,  0,   0,   23,  112, 251, 180, 0,   31,  0,
    94,  253, 61,  23,  112, 252, 217, 0,   47,  0,   94,  252, 24,  23,  112, 255, 156, 0,   63,  0,   94,  253, 61,
    23,  112, 2,   95,  0,   79,  0,   94,  0,   0,   23,  112, 3,   132, 0,   95,  0,   94,  2,   195, 23,  112, 2,
    95,  0,   111, 0,   94,  3,   232, 23,  112, 255, 156, 0,   127, 0,   94,  0,   0,   0,   1,   0,   10,  0,   0,
    0,   10,  0,   9,   0,   1,   0,   2,   0,   11,  0,   1,   0,   11,  0,   10,  0,   2,   0,   3,   0,   12,  0,
    2,   0,   12,  0,   11,  0,   3,   0,   4,   0,   13,  0,   3,   0,   13,  0,   12,  0,   4,   0,   5,   0,   14,
    0,   4,   0,   14,  0,   13,  0,   5,   0,   6,   0,   15,  0,   5,   0,   15,  0,   14,  0,   6,   0,   7,   0,
    16,  0,   6,   0,   16,  0,   15,  0,   7,   0,   8,   0,   17,  0,   7,   0,   17,  0,   16,  0,   0,   0,   1,
    0,   2,   0,   3,   0,   4,   0,   5,   0,   6,   0,   7,   0,   8,   0,   0,   0,   9,   0,   10,  0,   11,  0,
    12,  0,   13,  0,   14,  0,   15,  0,   16,  0,   17,  0,   0,   0,   18,  0,   19,  0,   20,  0,   21,  0,   22,
    0,   23,  0,   24,  0,   25,  0,   26,  0,   0,   0,   27,  0,   28,  0,   29,  0,   30,  0,   31,  0,   32,  0,
    33,  0,   34,  0,   35,  0,   0,   0,   0,   0,   1,   0,   2,   0,   3,   0,   4,   0,   5,   0,   6,   0,   7,
    0,   8,   0,   9,   0,   10,  0,   11,  0,   12,  0,   13,  0,   14,  0,   15,  0,   16,  0,   17,  0,   18,  0,
    19,  0,   20,  0,   21,  0,   22,  0,   23,  0,   24,  0,   25,  0,   26,  0,   27,  0,   28,  0,   29,  0,   30,
    0,   31,  0,   32,  0,   33,  0,   34,  0,   35,  0,   0,   0,   1,   0,   2,   0,   3,   0,   4,   0,   5,   0,
    6,   0,   7,   0,   8,   0,   9,   0,   10,  0,   11,  0,   12,  0,   13,  0,   14,  0,   15,  0,   16,  0,   17,
    0,   18,  0,   19,  0,   20,  0,   21,  0,   22,  0,   23,  0,   24,  0,   25,  0,   26,  0,   27,  0,   28,  0,
    29,  0,   30,  0,   31,  0,   32,  0,   33,  0,   34,  0,   35,  0,   9,   0,   10,  0,   11,  0,   12,  0,   13,
    0,   14,  0,   15,  0,   16,  0,   17,  0,   18,  0,   19,  0,   20,  0,   21,  0,   22,  0,   23,  0,   24,  0,
    25,  0,   26,  0,   0,   0,   10,  0,   120, 0,   80,  0,   10,  0,   0,   0,   0,   0,   0,   0,   0,   1,   217,
    0,   0,   1,   253, 0,   0,   2,   1,   0,   0,   2,   3,
};

u32 lbl_8031262C[9] = {
    0x00000000, 0x00000000,         0x00000000, 0x00030000, (u32)dll_5E_func00_nop, (u32)dll_5E_func01_nop,
    0x00000000, (u32)dll_5E_func03, 0x00000000};
