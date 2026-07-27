#include "dolphin/types.h"
#include "main/dll/dll_003E_dummy3e.h"

int Dummy3E_func05_ret_1(void)
{
    return 1;
}

void Dummy3E_func04_nop(void)
{
}

int Dummy3E_func03_ret_0(void)
{
    return 0;
}

void Dummy3E_release(void)
{
}

void Dummy3E_initialise(void)
{
}

u32 lbl_8031C300[10] = {0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00050000,
                        (u32)Dummy3E_initialise,
                        (u32)Dummy3E_release,
                        0x00000000,
                        (u32)Dummy3E_func03_ret_0,
                        (u32)Dummy3E_func04_nop,
                        (u32)Dummy3E_func05_ret_1};
