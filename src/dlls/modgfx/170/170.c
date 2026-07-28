/*
 * dllaafunc0 (DLL 170 / 0xAA) - a modgfx sequence-effect spawner.
 *
 * dll_AA_func03 drives gModgfxInterface to build and emit a multi-spawn
 * particle "sequence" (sequence id 0x15): it begins the sequence on the
 * source object, applies the caller's seqFlags, then queues several
 * addSequenceSpawn layers - their position/scale/lifetime triples come
 * from f32 literals (pooled by the compiler at lbl_803E1600..lbl_803E1624)
 * and a shared asset table (lbl_80319168). When posSource is supplied, one
 * spawn's scale is taken from the spawn-param packet (posSource + 8)
 * divided by 5. func00/func01 are the DLL's unused entry-point stubs.
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/partfx_interface.h"

extern u8 lbl_80319168[];

void dll_AA_func03(int sourceObj, int variant, u8* posSource, u32 seqFlags)
{
    u8* tab = (u8*)(int)lbl_80319168;
    f32 scale;

    scale = 8.0f;
    if (posSource != 0)
    {
        scale = ((PartFxSpawnParams*)posSource)->scale / 5.0f;
    }
    (*gModgfxInterface)->beginSequence((void*)sourceObj, (u8)variant, 0x15, 1, 0);
    (*gModgfxInterface)->setSequenceParams(&tab[0x1dc]);
    (*gModgfxInterface)->addSequenceFlags(seqFlags);
    (*gModgfxInterface)->resetSequenceSpawns();
    (*gModgfxInterface)->addSequenceSpawn(4, 0.65f, 0.0f, 0.0f, 0x15, &tab[0x1b0]);
    (*gModgfxInterface)->addSequenceSpawn(2, 0.5f, 1.0f, 0.5f, 0x15, &tab[0x1b0]);
    (*gModgfxInterface)->addSequenceSpawn(0x400000, 0.0f, -100.0f, 0.0f, 0, NULL);
    (*gModgfxInterface)->nextSequenceParam();
    (*gModgfxInterface)->addSequenceSpawn(4, 160.0f, 0.0f, 0.0f, 7, &tab[0x174]);
    (*gModgfxInterface)->nextSequenceParam();
    (*gModgfxInterface)->addSequenceSpawn(4, 255.0f, 0.0f, 0.0f, 7, &tab[0x174]);
    (*gModgfxInterface)->addSequenceSpawn(2, scale, 3.0f, scale, 0x15, &tab[0x1b0]);
    (*gModgfxInterface)->nextSequenceParam();
    (*gModgfxInterface)->addSequenceSpawn(4, 0.0f, 0.0f, 0.0f, 7, &tab[0x174]);
    (*gModgfxInterface)->spawnSequence(posSource, (u8*)(int)lbl_80319168, 0x15, &tab[0xd4], 0x18, 0x3e9, 0);
    (*gModgfxInterface)->getLastSpawnHandle();
}

void dll_AA_func01_nop(void)
{
}

void dll_AA_func00_nop(void)
{
}

u8 lbl_80319168[492] = {
    0,   0,   0,   0,   3,   232, 0,   0,   0,   0,   3,   98,  0,   0,   1,   244, 0,   11,  0,   0,   3,   98,  0,
    0,   254, 12,  0,   22,  0,   0,   0,   0,   0,   0,   252, 24,  0,   32,  0,   0,   252, 158, 0,   0,   254, 12,
    0,   42,  0,   0,   252, 158, 0,   0,   1,   244, 0,   52,  0,   0,   0,   0,   0,   0,   3,   232, 0,   63,  0,
    0,   0,   0,   11,  184, 3,   232, 0,   0,   0,   31,  3,   98,  11,  184, 1,   244, 0,   11,  0,   31,  3,   98,
    11,  184, 254, 12,  0,   22,  0,   31,  0,   0,   11,  184, 252, 24,  0,   32,  0,   31,  252, 158, 11,  184, 254,
    12,  0,   42,  0,   31,  252, 158, 11,  184, 1,   244, 0,   52,  0,   31,  0,   0,   11,  184, 3,   232, 0,   63,
    0,   31,  0,   0,   23,  112, 3,   232, 0,   0,   0,   63,  3,   98,  23,  112, 1,   244, 0,   11,  0,   63,  3,
    98,  23,  112, 254, 12,  0,   22,  0,   63,  0,   0,   23,  112, 252, 24,  0,   32,  0,   63,  252, 158, 23,  112,
    254, 12,  0,   42,  0,   63,  252, 158, 23,  112, 1,   244, 0,   52,  0,   63,  0,   0,   23,  112, 3,   232, 0,
    63,  0,   63,  0,   0,   0,   0,   0,   1,   0,   8,   0,   0,   0,   8,   0,   7,   0,   1,   0,   2,   0,   9,
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
    0,   14,  0,   15,  0,   16,  0,   17,  0,   18,  0,   19,  0,   20,  0,   0,   0,   0,   0,   5,   0,   30,  0,
    40,  0,   0,   0,   0,   0,   0,   0,   0};
void* lbl_80319354[9] = {(void*)0x00000000, (void*)0x00000000, (void*)0x00000000, (void*)0x00030000, dll_AA_func00_nop,
                         dll_AA_func01_nop, (void*)0x00000000, dll_AA_func03,     (void*)0x00000000};
