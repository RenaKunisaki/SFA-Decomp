#ifndef MUSYX_HW_INIT_H_
#define MUSYX_HW_INIT_H_

#include "ghidra_import.h"
#include "musyx/dsp_voice.h"

typedef u32 (*SndMessageCallback)(u32 message, u32 userValue);

extern SndMessageCallback salMessageCallback;

void snd_handle_irq(void);
int hwInit(u32 *sampleRate, u16 numVoices, u16 numStudios, u32 flags);
void hwExit(void);
void hwSetTimeOffset(int value);
u8 hwGetTimeOffset(void);
u32 hwIsActive(u32 voiceIndex);
void hwSetMesgCallback(SndMessageCallback callback);
void hwSetPriority(int voiceIndex, u32 priority);
void hwInitSamplePlayback(u32 voiceIndex, u16 sampleId, SAMPLE_INFO *sampleInfo,
                          u32 resetAdsr, u32 priority, u32 callbackUserValue,
                          u32 resetSrc, u32 itdMode);

extern u8 salMaxStudioNum;
extern u8 salNumVoices;
extern u8 salAuxFrame;
extern u8 salFrame;
extern u8 salTimeOffset;

#endif /* MUSYX_HW_INIT_H_ */
