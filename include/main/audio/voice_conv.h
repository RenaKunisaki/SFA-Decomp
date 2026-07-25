#ifndef MAIN_AUDIO_VOICE_CONV_H_
#define MAIN_AUDIO_VOICE_CONV_H_

#include "ghidra_import.h"
#include "main/audio/voice_manage.h"

extern u8 voiceMidiKeySlots[SYNTH_VOICE_MIDI_CHANNEL_COUNT][SYNTH_VOICE_MIDI_KEY_COUNT];
extern u8 voiceDirectSlots[SYNTH_VOICE_DIRECT_SLOT_COUNT];

void voiceInitRegistrationTables(void);
int voiceScaleSampleRate(u16 x);
u32 voiceGetPitchRatio(u8 noteIn, u32 packed);
u32 voiceConvertDbToLinear(u32 dbCents);

#endif /* MAIN_AUDIO_VOICE_CONV_H_ */
