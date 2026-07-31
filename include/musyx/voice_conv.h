#ifndef MUSYX_VOICE_CONV_H_
#define MUSYX_VOICE_CONV_H_

#include "ghidra_import.h"
#include "musyx/voice_manage.h"

extern u8 synth_last_started[SYNTH_VOICE_MIDI_CHANNEL_COUNT][SYNTH_VOICE_MIDI_KEY_COUNT];
extern u8 synth_last_fxstarted[SYNTH_VOICE_DIRECT_SLOT_COUNT];

void voiceInitLastStarted(void);
int voiceScaleSampleRate(u16 x);
u32 voiceGetPitchRatio(u8 noteIn, u32 packed);
u32 voiceConvertDbToLinear(u32 dbCents);

#endif /* MUSYX_VOICE_CONV_H_ */
