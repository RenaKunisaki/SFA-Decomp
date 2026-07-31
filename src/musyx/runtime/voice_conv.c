#include "musyx/voice_manage.h"
#include "musyx/voice_conv.h"

void voiceInitLastStarted(void)
{
    int channel;
    int key;

    for (channel = 0; channel < SYNTH_VOICE_MIDI_CHANNEL_COUNT; channel++)
    {
        for (key = 0; key < SYNTH_VOICE_MIDI_KEY_COUNT; key++)
        {
            synth_last_started[channel][key] = SYNTH_VOICE_REGISTRATION_FREE;
        }
    }
    synth_last_fxstarted[0] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[1] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[2] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[3] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[4] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[5] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[6] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[7] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[8] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[9] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[10] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[11] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[12] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[13] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[14] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[15] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[16] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[17] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[18] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[19] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[20] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[21] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[22] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[23] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[24] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[25] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[26] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[27] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[28] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[29] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[30] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[31] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[32] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[33] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[34] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[35] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[36] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[37] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[38] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[39] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[40] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[41] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[42] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[43] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[44] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[45] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[46] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[47] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[48] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[49] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[50] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[51] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[52] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[53] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[54] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[55] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[56] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[57] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[58] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[59] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[60] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[61] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[62] = SYNTH_VOICE_REGISTRATION_FREE;
    synth_last_fxstarted[63] = SYNTH_VOICE_REGISTRATION_FREE;
}
