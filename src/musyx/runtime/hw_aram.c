#include "musyx/hw_aram.h"
#include "musyx/synth_config.h"
#include "musyx/aram.h"
#include "PowerPC_EABI_Support/Runtime/runtime.h"


extern f32 lbl_803E78E8;

u32 hwExitStream(u32 value)
{
    return __cvt_fp2unsigned((double)((lbl_803E78E8 * (f32)value) / (f32)SYNTH_CONFIGURATION->sampleRate));
}

void hwInitSampleMem(u32 baseAddr, u32 length)
{
    aramInit(length);
}

void hwExitSampleMem(void)
{
    aramExit();
}
