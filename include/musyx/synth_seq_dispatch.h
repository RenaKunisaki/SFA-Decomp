#ifndef MUSYX_SYNTH_SEQ_DISPATCH_H_
#define MUSYX_SYNTH_SEQ_DISPATCH_H_

#include "ghidra_import.h"

typedef struct SynthSequenceEvent SynthSequenceEvent;

SynthSequenceEvent* synthHandleSequenceEvent(SynthSequenceEvent* event, u8 groupIndex, u32* loopFlag);
void synthQueueAllChannelEvents(void);
u32 synthProcessChannelEventQueue(u8 groupIndex, u32 deltaTime);

#endif /* MUSYX_SYNTH_SEQ_DISPATCH_H_ */
