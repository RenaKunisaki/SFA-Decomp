/*
 * DLL 88 / 0x58 - a no-op DLL.
 */
#include "types.h"

int Dummy58_func03_ret_0(void)
{
    return 0x0;
}

void Dummy58_release(void)
{
}

void Dummy58_initialise(void)
{
}

u32 lbl_803137D8[8] = {
    0x00000000,
    0x00000000,
    0x00000000,
    0x00030000,
    (u32)Dummy58_initialise,
    (u32)Dummy58_release,
    0x00000000,
    (u32)Dummy58_func03_ret_0,
};
