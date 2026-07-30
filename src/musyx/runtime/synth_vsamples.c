#include "musyx/synth_virtual_sample.h"
#include "musyx/hw_sample.h"
#include "musyx/hw_samplemem.h"
#include "musyx/aram.h"
#include "musyx/mcmd.h"
#include "musyx/hw_stream.h"
#include "musyx/hw_break.h"


SynthVirtualSampleState synthVirtualSampleState;

/*
 * Reset the virtual sample stream buffer table.
 */
void synthInitVirtualSampleTable(void)
{
    int i;
    SynthVirtualSampleState* state = &synthVirtualSampleState;

    state->entryCount = 0;
    for (i = 0; i < SYNTH_VIRTUAL_SAMPLE_MAX_VOICES; i++)
    {
        state->voiceMap[i] = SYNTH_VIRTUAL_SAMPLE_FREE_SLOT;
    }
    state->nextId = 0;
    state->callback = 0;
}

static inline void vsFreeBuffer(u8 entryIndex)
{
    synthVirtualSampleState.entries[entryIndex].mode = SYNTH_VIRTUAL_SAMPLE_MODE_INACTIVE;
    synthVirtualSampleState.voiceMap[synthVirtualSampleState.entries[entryIndex].voice] =
        SYNTH_VIRTUAL_SAMPLE_FREE_SLOT;
}

static inline u8 vsAllocateBuffer(void)
{
    u8 i;

    for (i = 0; i < synthVirtualSampleState.entryCount; ++i)
    {
        if (synthVirtualSampleState.entries[i].mode != SYNTH_VIRTUAL_SAMPLE_MODE_INACTIVE)
        {
            continue;
        }
        synthVirtualSampleState.entries[i].mode = SYNTH_VIRTUAL_SAMPLE_MODE_ACTIVE;
        synthVirtualSampleState.entries[i].position = 0;
        return i;
    }

    return SYNTH_VIRTUAL_SAMPLE_FREE_SLOT;
}

static inline u16 vsNewInstanceID(void)
{
    u8 i;
    u16 instID;

    do
    {
        instID = synthVirtualSampleState.nextId++;
        for (i = 0; i < synthVirtualSampleState.entryCount; ++i)
        {
            if (synthVirtualSampleState.entries[i].mode != SYNTH_VIRTUAL_SAMPLE_MODE_INACTIVE &&
                synthVirtualSampleState.entries[i].callbackData.generation == instID)
            {
                break;
            }
        }
    } while (i != synthVirtualSampleState.entryCount);

    return instID;
}

/*
 * Allocate a stream buffer for the voice and set up its virtual sample
 * loop buffer.
 */
u32 synthClaimVirtualSampleSlot(u8 voiceID)
{
    u8 sb;
    u8 i;
    u32 addr;

    for (i = 0; i < synthVirtualSampleState.entryCount; ++i)
    {
        if (synthVirtualSampleState.entries[i].mode != SYNTH_VIRTUAL_SAMPLE_MODE_INACTIVE &&
            synthVirtualSampleState.entries[i].voice == voiceID)
        {
            vsFreeBuffer(i);
        }
    }

    sb = synthVirtualSampleState.voiceMap[voiceID] = vsAllocateBuffer();
    if (sb != SYNTH_VIRTUAL_SAMPLE_FREE_SLOT)
    {
        addr = aramGetStreamBufferAddress(synthVirtualSampleState.voiceMap[voiceID], 0);
        hwSetVirtualSampleLoopBuffer(voiceID, addr, synthVirtualSampleState.loopSize);
        synthVirtualSampleState.entries[sb].callbackData.sampleId = hwGetSampleID(voiceID);
        synthVirtualSampleState.entries[sb].callbackData.generation = vsNewInstanceID();
        synthVirtualSampleState.entries[sb].type = hwGetSampleType(voiceID);
        synthVirtualSampleState.entries[sb].voice = voiceID;
        if (synthVirtualSampleState.callback != 0)
        {
            synthVirtualSampleState.callback(SYNTH_VIRTUAL_SAMPLE_CLAIM_CALLBACK_KIND,
                                             &synthVirtualSampleState.entries[sb].callbackData);
            return (synthVirtualSampleState.entries[sb].callbackData.generation << 8) | (u8)voiceID;
        }
        hwSetVirtualSampleLoopBuffer(voiceID, 0, 0);
    }
    else
    {
        hwSetVirtualSampleLoopBuffer(voiceID, 0, 0);
    }

    return SYNTH_VIRTUAL_SAMPLE_INVALID_ID;
}

/*
 * Sample-completion handler: if the packed (slotIdx, sampleId)
 * still matches the active sample, fire the global "done" callback
 * with kind=2, then clear the entry's mode and free the slot back to
 * the index pool.
 */
void synthHandleVirtualSampleDone(u32 packed)
{
    SynthVirtualSampleState* state;
    SynthVirtualSampleEntry* entry;
    u32 entryOffset;
    u8* slots;
    u8 vid;
    u32 generation;

    state = &synthVirtualSampleState;
    if (packed == SYNTH_VIRTUAL_SAMPLE_INVALID_ID)
    {
        return;
    }
    vid = (slots = state->voiceMap)[(u8)packed];
    if (vid == SYNTH_VIRTUAL_SAMPLE_FREE_SLOT)
    {
        return;
    }
    entryOffset = vid * SYNTH_VIRTUAL_SAMPLE_ENTRY_SIZE;
    generation = (packed >> 8) & 0xffff;
    if (state->entries[vid].callbackData.generation != generation)
    {
        return;
    }
    if (state->callback != NULL)
    {
        state->callback(SYNTH_VIRTUAL_SAMPLE_DONE_CALLBACK_KIND, &state->entries[vid].callbackData);
    }
    entry = (SynthVirtualSampleEntry*)((u8*)state->entries + entryOffset);
    entry->mode = SYNTH_VIRTUAL_SAMPLE_MODE_INACTIVE;
    slots[entry->voice] = SYNTH_VIRTUAL_SAMPLE_FREE_SLOT;
}

static void synthAdvanceVirtualSampleEntry(void* entry, u32 elapsed)
{
    SynthVirtualSampleState* state;
    SynthVirtualSampleEntry* sample;
    u32* loopSizePtr;
    struct
    {
        u32 len, off;
    } d; /* struct-typed pair claims target frame slot */

    state = &synthVirtualSampleState;
    sample = entry;
    if (sample->position == elapsed)
    {
        return;
    }
    if ((s32)sample->position < elapsed)
    {
        switch (sample->type)
        {
        case SYNTH_VIRTUAL_SAMPLE_STREAM_TYPE:
            sample->callbackData.start =
                (sample->position / SYNTH_VIRTUAL_SAMPLE_ADPCM_FRAME_SAMPLES) * SYNTH_VIRTUAL_SAMPLE_ADPCM_FRAME_BYTES;
            sample->callbackData.size = elapsed - sample->position;
            sample->callbackData.wrapA = 0;
            sample->callbackData.wrapB = 0;
            if ((d.len = state->callback(SYNTH_VIRTUAL_SAMPLE_STREAM_CALLBACK_KIND, &sample->callbackData)) != 0)
            {
                d.off = sample->position + d.len;
                sample->position = d.off % state->loopSize;
            }
            break;
        default:
            break;
        }
    }
    else if (elapsed == 0)
    {
        switch (sample->type)
        {
        case SYNTH_VIRTUAL_SAMPLE_STREAM_TYPE:
            sample->callbackData.start =
                (sample->position / SYNTH_VIRTUAL_SAMPLE_ADPCM_FRAME_SAMPLES) * SYNTH_VIRTUAL_SAMPLE_ADPCM_FRAME_BYTES;
            loopSizePtr = &state->loopSize;
            sample->callbackData.size = *loopSizePtr - sample->position;
            sample->callbackData.wrapA = 0;
            sample->callbackData.wrapB = 0;
            if ((d.len = state->callback(SYNTH_VIRTUAL_SAMPLE_STREAM_CALLBACK_KIND, &sample->callbackData)) != 0)
            {
                d.off = sample->position + d.len;
                sample->position = d.off % *loopSizePtr;
            }
            break;
        default:
            break;
        }
    }
    else
    {
        switch (sample->type)
        {
        case SYNTH_VIRTUAL_SAMPLE_STREAM_TYPE:
            sample->callbackData.start =
                (sample->position / SYNTH_VIRTUAL_SAMPLE_ADPCM_FRAME_SAMPLES) * SYNTH_VIRTUAL_SAMPLE_ADPCM_FRAME_BYTES;
            loopSizePtr = &state->loopSize;
            sample->callbackData.size = *loopSizePtr - sample->position;
            sample->callbackData.wrapA = 0;
            sample->callbackData.wrapB = elapsed;
            if ((d.len = state->callback(SYNTH_VIRTUAL_SAMPLE_STREAM_CALLBACK_KIND, &sample->callbackData)) != 0)
            {
                d.off = sample->position + d.len;
                sample->position = d.off % *loopSizePtr;
            }
            break;
        default:
            break;
        }
    }
}

#define SYNTH_VIRTUAL_SAMPLE_VOICE_STRIDE         0x404
#define SYNTH_VIRTUAL_SAMPLE_VOICE_RELEASE_OFFSET 0x206
#define SYNTH_VIRTUAL_SAMPLE_RELEASE_SCALE        0xa0
#define SYNTH_VIRTUAL_SAMPLE_RELEASE_ROUND        0xfff
#define SYNTH_VIRTUAL_SAMPLE_RELEASE_SHIFT        0x1000

extern s16 synthLoadedGroupCount;

/*
 * Periodic virtual-sample tick processor: walks 64 active voices, computes
 * elapsed tick for each, and either advances the envelope (mode 1)
 * or runs sample-completion logic (mode 2 - checks current sample
 * id matches expected and triggers a stop+vacate when threshold
 * elapsed).
 */
void synthUpdateVirtualSamples(void)
{
    SynthVirtualSampleState* state;
    u8* slotMap;
    u32 i;
    u32 currentTick;
    u32 elapsed;
    SynthVirtualSampleEntry* entry;
    u8 vid;

    if (synthVirtualSampleState.callback != 0)
    {
        state = &synthVirtualSampleState;
        slotMap = (u8*)state;
        for (i = 0; i < SYNTH_VIRTUAL_SAMPLE_MAX_VOICES; i++, slotMap++)
        {
            vid = slotMap[SYNTH_VIRTUAL_SAMPLE_VOICE_MAP_OFFSET];
            if (vid == SYNTH_VIRTUAL_SAMPLE_FREE_SLOT)
            {
                continue;
            }
            if (hwGetVirtualSampleState(i) == 0)
            {
                continue;
            }
            vid = slotMap[SYNTH_VIRTUAL_SAMPLE_VOICE_MAP_OFFSET];
            entry = &state->entries[vid];

            currentTick = hwChangeStudio(i);
            if (entry->type == SYNTH_VIRTUAL_SAMPLE_STREAM_TYPE)
            {
                elapsed =
                    (currentTick / SYNTH_VIRTUAL_SAMPLE_ADPCM_FRAME_SAMPLES) * SYNTH_VIRTUAL_SAMPLE_ADPCM_FRAME_SAMPLES;
            }
            else
            {
                elapsed = currentTick;
            }

            switch (entry->mode)
            {
            case SYNTH_VIRTUAL_SAMPLE_MODE_ACTIVE:
                synthAdvanceVirtualSampleEntry(entry, elapsed);
                break;
            case SYNTH_VIRTUAL_SAMPLE_MODE_DONE_WAIT:
            {
                u32 sampleId = hwGetVirtualSampleID(entry->voice);
                u32 expected = ((u32)entry->callbackData.generation << 8) | entry->voice;

                if (expected == sampleId)
                {
                    u32 prev;

                    synthAdvanceVirtualSampleEntry(entry, elapsed);
                    prev = entry->lastTick;
                    if (currentTick >= prev)
                    {
                        entry->remaining -= (currentTick - prev);
                    }
                    else
                    {
                        entry->remaining -= state->loopSize - (prev - currentTick);
                    }
                    entry->lastTick = currentTick;

                    if ((s32)(u32)((s32)(*(u16*)((u8*)synthVoice +
                                                 entry->voice * SYNTH_VIRTUAL_SAMPLE_VOICE_STRIDE +
                                                 SYNTH_VIRTUAL_SAMPLE_VOICE_RELEASE_OFFSET) *
                                             SYNTH_VIRTUAL_SAMPLE_RELEASE_SCALE +
                                         SYNTH_VIRTUAL_SAMPLE_RELEASE_ROUND) /
                                   SYNTH_VIRTUAL_SAMPLE_RELEASE_SHIFT) >
                        (s32)entry->remaining)
                    {
                        if (hwVoiceInStartup(entry->voice) == 0)
                        {
                            hwBreak(entry->voice);
                        }
                        entry->mode = SYNTH_VIRTUAL_SAMPLE_MODE_INACTIVE;
                        state->voiceMap[entry->voice] = SYNTH_VIRTUAL_SAMPLE_FREE_SLOT;
                    }
                }
                else
                {
                    entry->mode = SYNTH_VIRTUAL_SAMPLE_MODE_INACTIVE;
                    state->voiceMap[entry->voice] = SYNTH_VIRTUAL_SAMPLE_FREE_SLOT;
                }
            }
            break;
            }
        }
    }
}

/*
 * Reset the loaded sound-group table count.
 */
void synthResetLoadedGroupCount(void)
{
    synthLoadedGroupCount = 0;
}
