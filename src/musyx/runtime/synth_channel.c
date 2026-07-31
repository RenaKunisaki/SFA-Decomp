#include "src/musyx/runtime/synth_internal.h"

void seqMute(u32 seqId, u32 mask1, u32 mask2)
{
    SynthVoiceRuntime* runtime;
    u32 slot;

    runtime = SYNTH_VOICE_RUNTIME();
    slot = seqGetPrivateIdInline(seqId);

    if (slot == SYNTH_HANDLE_INVALID)
    {
        return;
    }

    if ((slot & SYNTH_HANDLE_QUEUED_FLAG) == 0)
    {
        runtime->voices[slot].trackMute[0] = mask1;
        runtime->voices[slot].trackMute[1] = mask2;
    }
    else
    {
        runtime->voices[slot & SYNTH_HANDLE_ID_MASK].pendingUpdate.flags |= SYNTH_PENDING_FLAG_MIX_DATA;
        runtime->voices[slot & SYNTH_HANDLE_ID_MASK].pendingUpdate.mixValue0 = mask1;
        runtime->voices[slot & SYNTH_HANDLE_ID_MASK].pendingUpdate.mixValue1 = mask2;
    }
}
