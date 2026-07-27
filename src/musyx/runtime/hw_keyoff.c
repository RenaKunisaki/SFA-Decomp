#include "ghidra_import.h"
#include "musyx/hw_keyoff.h"
#include "musyx/dsp_voice_state.h"

extern u8 salTimeOffset;

void hwKeyOff(int slot)
{
    dspVoice[slot].changed[salTimeOffset] |= DSP_VOICE_CHANGE_KEYOFF;
}
