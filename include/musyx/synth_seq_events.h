#ifndef MUSYX_SYNTH_SEQ_EVENTS_H_
#define MUSYX_SYNTH_SEQ_EVENTS_H_

#include "src/musyx/runtime/synth_internal.h"

SynthSequenceEvent* synthGetNextChannelEvent(u8 channel);
void synthInsertChannelEvent(SynthSequenceQueue* queue, SynthSequenceEvent* event);

#endif /* MUSYX_SYNTH_SEQ_EVENTS_H_ */
