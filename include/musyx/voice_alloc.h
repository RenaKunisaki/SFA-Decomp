#ifndef MUSYX_VOICE_ALLOC_H_
#define MUSYX_VOICE_ALLOC_H_

#include "ghidra_import.h"
#include "musyx/mcmd.h"

u32 voiceAllocate(u8 priority, u8 maxInstances, u16 allocId, u8 fxFlag);
void voiceFree(McmdVoiceState *voice);

#endif /* MUSYX_VOICE_ALLOC_H_ */
