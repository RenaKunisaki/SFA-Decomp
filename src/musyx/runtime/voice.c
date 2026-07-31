#include "global.h"
#include "ghidra_import.h"
#include "musyx/mcmd.h"
#include "musyx/hw_init.h"
#include "musyx/synth_jobs.h"
#include "musyx/synth_config.h"
#include "musyx/mcmd_exec.h"
#include "musyx/snd_core.h"
#include "musyx/vidlisttables.h"
#include "musyx/vid_get.h"
#include "musyx/voice_id.h"
#include "musyx/voice_prio.h"
#include "musyx/voice_alloc.h"
#include "musyx/voice_manage.h"
#include "musyx/voice_unregister.h"
#include "musyx/hw_break.h"
#include "musyx/hw_break.h"

u8 voiceFreeListRoot;
u8 voiceFreeListTail;
u8 voiceFxRunning;
u8 voiceMusicRunning;
u16 voicePrioSortedRoot;

static McmdVidListNode vidListNodes[128];
static u8 voiceMidiKeySlots[SYNTH_VOICE_MIDI_CHANNEL_COUNT][SYNTH_VOICE_MIDI_KEY_COUNT];
static u8 voiceDirectSlots[SYNTH_VOICE_DIRECT_SLOT_COUNT];
static SynthVoiceListNode voicePriorityLinks[0x40];
static u8 voicePriorityGroupHeads[0x100];
static SynthRootListNode voicePrioritySortLinks[0x100];
static SynthVoiceListNode voiceFreeListSlots[64];

#define VB_PRIO_HEAD(vb, p)      (*(u8*)((u8*)&(vb)->priorityGroupHeads[0] + (p)))
#define VB_PRIO_LINK(vb, i)      ((SynthVoiceListNode*)((u8*)&(vb)->priorityLinks[0] + (i) * sizeof(SynthVoiceListNode)))
#define VB_PRIO_SORT_NEXT(vb, p) (((SynthRootListNode*)&(vb)->prioritySortLinks[0])[p].next)
#define VB_PRIO_SORT_PREV(vb, p) \
    (((SynthRootListNode*)((u8*)&(vb)->prioritySortLinks[0] + offsetof(SynthRootListNode, prev)))[p].next)

/*
 * Insert the voice into the new priority group's list and keep the global
 * group list sorted by priority.
 */
void voiceSetPriority(McmdVoiceState* svoice, u8 prio)
{
    u32 voiceIdx;
    VidListTables* vb;
    u16 li;
    SynthVoiceListNode* vps;
    u16 root;
    u16 i;

    voiceIdx = (u8)svoice->voiceHandle;
    vb = (VidListTables*)vidListNodes;
    vps = VB_PRIO_LINK(vb, voiceIdx);
    if (vps->user == 1)
    {
        if (svoice->priorityGroup == prio)
        {
            return;
        }

        voiceRemovePriority(svoice);
    }

    vps->user = 1;
    vps->prev = 0xff;
    if ((vps->next = VB_PRIO_HEAD(vb, prio)) != 0xFF)
    {
        VB_PRIO_LINK(vb, VB_PRIO_HEAD(vb, prio))->prev = voiceIdx;
    }
    else if (root = voicePrioSortedRoot, root != 0xFFFF)
    {
        if (prio >= root)
        {
            for (i = root; i != 0xFFFF; i = VB_PRIO_SORT_NEXT(vb, i))
            {
                if (i > prio)
                {
                    break;
                }
                li = i;
            }

            VB_PRIO_SORT_NEXT(vb, li) = prio;
            VB_PRIO_SORT_PREV(vb, prio) = li;
            VB_PRIO_SORT_NEXT(vb, prio) = i;
            if (i != 0xFFFF)
            {
                VB_PRIO_SORT_PREV(vb, i) = prio;
            }
        }
        else
        {
            VB_PRIO_SORT_NEXT(vb, prio) = root;
            VB_PRIO_SORT_PREV(vb, prio) = 0xFFFF;
            VB_PRIO_SORT_PREV(vb, root) = prio;
            voicePrioSortedRoot = prio;
        }
    }
    else
    {
        VB_PRIO_SORT_NEXT(vb, prio) = 0xFFFF;
        VB_PRIO_SORT_PREV(vb, prio) = 0xFFFF;
        voicePrioSortedRoot = prio;
    }

    VB_PRIO_HEAD(vb, prio) = voiceIdx;
    svoice->priorityGroup = prio;
    hwSetPriority(svoice->voiceHandle & 0xFF, ((u32)prio << 24) | (svoice->priorityValue >> 15));
}

typedef union SynthVoiceFlags
{
    u64 combined;
    struct
    {
        u32 input;
        u32 output;
    } channel;
} SynthVoiceFlags;

#define VOICE_CFLAGS(i) (((SynthVoiceFlags*)&synthVoice[i].inputFlags)->combined)

#define VB_PRIO_LINK_NEXT(vb, i) (((SynthVoiceListNode*)((u8*)&(vb)->priorityLinks[0] + (i) * 4))->next)
#define VOICE_PRIORITY_NONE      0xFFFF

/*
 * Allocate a voice id, preferring a free slot but stealing the lowest-priority
 * compatible active voice when limits are exceeded. (musyx synthvoice.c
 * voiceAllocate, pre-2.0.1 variant.)
 */
u32 voiceAllocate(u8 priority, u8 maxInstances, u16 allocId, u8 streamKind)
{
    s32 i;
    s32 allocationCount;
    s32 selectedVoice;
    u16 priorityGroup;
    u32 restrictToStreamKind;
    u16 priorityGroupIndex;
    SynthVoiceListNode* freeSlot;
    SynthVoiceListNode* slotBase;
    VidListTables* voiceLists = (VidListTables*)vidListNodes;

    if (!synthIdleWaitActive)
    {
        if (streamKind)
        {
            restrictToStreamKind = (voiceFxRunning >= SYNTH_CONFIGURATION->fxVoiceCount &&
                                    SYNTH_CONFIGURATION->voiceCount > SYNTH_CONFIGURATION->fxVoiceCount);

            if (SYNTH_CONFIGURATION->fxVoiceCount <= maxInstances)
            {
                goto steal;
            }
        }
        else
        {
            restrictToStreamKind = (voiceMusicRunning >= SYNTH_CONFIGURATION->musicVoiceCount &&
                                    SYNTH_CONFIGURATION->voiceCount > SYNTH_CONFIGURATION->musicVoiceCount);

            if (SYNTH_CONFIGURATION->musicVoiceCount <= maxInstances)
            {
                goto steal;
            }
        }

        {
            allocationCount = 0;
            selectedVoice = -1;

            priorityGroup = voicePrioSortedRoot;
            while (priorityGroup != VOICE_PRIORITY_NONE && priority >= priorityGroup && selectedVoice == -1)
            {
                u32 group = priorityGroup;
                for (i = VB_PRIO_HEAD(voiceLists, group); i != SYNTH_INVALID_VOICE_U8;
                     i = VB_PRIO_LINK_NEXT(voiceLists, i))
                {
                    if (allocId != synthVoice[i].baseSample)
                        continue;
                    ++allocationCount;
                    if (synthVoice[i].block)
                        continue;

                    if (!restrictToStreamKind || streamKind == synthVoice[i].streamKind)
                    {
                        if (VOICE_CFLAGS(i) & 2)
                            continue;
                        if (selectedVoice != -1)
                        {
                            if (synthVoice[i].priorityValue < synthVoice[selectedVoice].priorityValue)
                                selectedVoice = i;
                        }
                        else
                            selectedVoice = i;
                    }
                }

                priorityGroup = VB_PRIO_SORT_NEXT(voiceLists, group);
            }

            if (allocationCount >= maxInstances)
            {
                goto have_voice;
            }

            while (priorityGroup != VOICE_PRIORITY_NONE && allocationCount < maxInstances)
            {
                u32 group = priorityGroup;
                i = VB_PRIO_HEAD(voiceLists, group);
                while (i != SYNTH_INVALID_VOICE_U8)
                {
                    if (allocId == synthVoice[i].baseSample)
                    {
                        allocationCount++;
                    }

                    i = VB_PRIO_LINK_NEXT(voiceLists, i);
                }

                priorityGroup = VB_PRIO_SORT_NEXT(voiceLists, group);
            }

            if (allocationCount >= maxInstances)
            {
                goto have_voice;
            }
        }

    steal:
        {
            selectedVoice = -1;
            if (voiceFreeListRoot != SYNTH_INVALID_VOICE_U8 && restrictToStreamKind == 0)
            {
                selectedVoice = voiceFreeListRoot;
            }
            else
            {
                if (priority < voicePrioSortedRoot)
                {
                    return SYNTH_INVALID_VOICE;
                }

                priorityGroup = voicePrioSortedRoot;

                while (priorityGroup != VOICE_PRIORITY_NONE && priority >= priorityGroup && selectedVoice == -1)
                {
                    priorityGroupIndex = priorityGroup;
                    for (i = VB_PRIO_HEAD(voiceLists, priorityGroupIndex); i != SYNTH_INVALID_VOICE_U8;
                         i = VB_PRIO_LINK_NEXT(voiceLists, i))
                    {
                        if ((synthVoice[i].block == 0) &&
                            (!restrictToStreamKind || streamKind == synthVoice[i].streamKind))
                        {
                            if ((VOICE_CFLAGS(i) & 2) == 0)
                            {
                                if (selectedVoice != -1)
                                {
                                    if (synthVoice[selectedVoice].priorityValue > synthVoice[i].priorityValue)
                                        selectedVoice = i;
                                }
                                else
                                {
                                    selectedVoice = i;
                                }
                            }
                        }
                    }
                    priorityGroup = VB_PRIO_SORT_NEXT(voiceLists, priorityGroupIndex);
                }

                if (selectedVoice == -1)
                {
                    return SYNTH_INVALID_VOICE;
                }
            }

            if (synthVoice[selectedVoice].priorityGroup > priority)
            {
                goto ret_invalid;
            }
        }

    have_voice:
        if (selectedVoice == -1)
        {
            goto ret_invalid;
        }

        slotBase = (SynthVoiceListNode*)((u8*)voiceLists + selectedVoice * 4);
        if (slotBase[offsetof(VidListTables, freeList) / sizeof(*slotBase)].user == 1)
        {
            freeSlot = (SynthVoiceListNode*)((u8*)slotBase + offsetof(VidListTables, freeList));
            i = freeSlot->prev;

            if (i != SYNTH_INVALID_VOICE_U8)
            {
                voiceLists->freeList[i].next = freeSlot->next;
            }
            else
            {
                voiceFreeListRoot = freeSlot->next;
            }

            i = freeSlot->next;
            if (i != SYNTH_INVALID_VOICE_U8)
            {
                voiceLists->freeList[i].prev = freeSlot->prev;
            }

            if (selectedVoice == voiceFreeListTail)
            {
                voiceFreeListTail = freeSlot->prev;
            }

            freeSlot->user = 0;
        }
        else if (synthVoice[selectedVoice].streamKind)
        {
            voiceFxRunning--;
        }
        else
        {
            voiceMusicRunning--;
        }
        if (streamKind != 0)
        {
            ++voiceFxRunning;
        }
        else
        {
            ++voiceMusicRunning;
        }
        return selectedVoice;
    }

ret_invalid:
    return SYNTH_INVALID_VOICE;
}

/*
 * Release a voice slot: clear voice flags, unlink from id table,
 * decrement counter, and mark id slot as free (-1).
 */
void voiceFree(McmdVoiceState* voice)
{
    macMakeInactive(voice, 2);
    voiceRemovePriority(voice);
    voice->macroBase = NULL;
    voice->priorityGroup = 0;
    {
        u32 voiceId = voice->voiceHandle;
        u32 v = voiceId & 0xff;
        SynthVoiceListNode* slot = &voiceFreeListSlots[v];
        if (slot->user == 0)
        {
            slot->user = 1;
            if (voiceFreeListRoot != 0xff)
            {
                slot->next = 0xff;
                slot->prev = voiceFreeListTail;
                voiceFreeListSlots[voiceFreeListTail].next = v;
            }
            else
            {
                slot->next = 0xff;
                slot->prev = 0xff;
                voiceFreeListRoot = v;
            }
            voiceFreeListTail = v;
            if (voice->streamKind != 0)
            {
                voiceFxRunning--;
            }
            else
            {
                voiceMusicRunning--;
            }
        }
    }
    *(int*)&voice->voiceHandle = -1;
}

#define SYNTH_VOICE_STATE(voice) (&synthVoice[voice])

static inline void voiceInitFreeList(void)
{
    u32 i;

    for (i = 0; i < synthInfo.voiceCount; i++)
    {
        voiceFreeListSlots[i].prev = i - 1;
        voiceFreeListSlots[i].next = i + 1;
        voiceFreeListSlots[i].user = 1;
    }
    voiceFreeListSlots[0].prev = 0xff;
    voiceFreeListSlots[synthInfo.voiceCount - 1].next = 0xff;
    voiceFreeListRoot = 0;
    voiceFreeListTail = synthInfo.voiceCount - 1;
}

static inline void voiceInitPrioSort(void)
{
    u32 i;

    for (i = 0; i < synthInfo.voiceCount; i++)
    {
        voicePriorityLinks[i].user = 0;
    }
    for (i = 0; i < 0x100; i++)
    {
        voicePriorityGroupHeads[i] = 0xff;
    }
    voicePrioSortedRoot = 0xffff;
}

/*
 * Initialize the voice priority and group linked-list tables.
 */
void voiceInitPriorityTables(void)
{
    voiceInitFreeList();
    voiceInitPrioSort();
    voiceFxRunning = 0;
    voiceMusicRunning = 0;
}

/*
 * Voice cleanup: if voice handle is valid, break the active voice and
 * reset its id slot.
 */
void voiceBreakAndFree(u32 voice)
{
    if (voice == SYNTH_INVALID_VOICE)
        return;
    if (hwIsActive(voice) != 0)
    {
        hwBreak(voice);
    }
    synthVoice[voice].handle = voice;
    voiceFree(&synthVoice[voice]);
    synthVoice[voice].callbackActive = 0;
}

/*
 * Voice teardown: clears state flags then breaks the voice.
 */
void voiceKill(u32 voice)
{
    McmdVoiceState* voiceState = SYNTH_VOICE_STATE(voice);

    if (voiceState->activeHandle != 0)
    {
        vidRemoveVoice(voiceState);
        *(u64*)&voiceState->inputFlags &= ~3;
        voiceState->priorityTick = 0;
        voiceFree(voiceState);
    }
    if (voiceState->callbackActive != 0)
    {
        streamKill(voice);
    }
    hwBreak(voice);
}

/*
 * Walk the synth's voice list for the given id, breaking each match.
 * Returns 0 if at least one match was broken, else -1.
 */
int voiceKillById(u32 id)
{
    int result = -1;
    u32 nextHandle;
    u32 i;

    if (gSynthInitialized != 0)
    {
        McmdVidListNode* listEntry;
        if ((id != SYNTH_INVALID_VOICE) && ((listEntry = get_vidlist(id)) != 0))
        {
            id = listEntry->internalId;
        }
        else
        {
            id = SYNTH_INVALID_VOICE;
        }

        for (; id != SYNTH_INVALID_VOICE; id = nextHandle)
        {
            i = (u8)id;
            nextHandle = SYNTH_VOICE_STATE(i)->nextHandle;
            if (id == SYNTH_VOICE_STATE(i)->handle)
            {
                voiceKill(i);
                result = 0;
            }
        }
    }

    return result;
}

/*
 * Returns 1 if state's voice id is currently registered in the
 * appropriate slot table, else 0.
 */
u32 voiceIsRegistered(McmdVoiceState* state)
{
    McmdVoiceState* voiceState = state;
    u32 voice = voiceState->handle;
    u8 slot;
    u8 channel;
    u8 voiceIdx;
    if (voice != SYNTH_INVALID_VOICE)
    {
        slot = voiceState->midiSlot;
        if (slot != SYNTH_INVALID_VOICE_U8)
        {
            channel = voiceState->midiChannel;
            voiceIdx = voice;
            if (channel == SYNTH_INVALID_VOICE_U8)
            {
                if (voiceDirectSlots[voiceIdx] == voiceIdx)
                    return 1;
            }
            else if (voiceIdx == voiceMidiKeySlots[channel][slot])
            {
                return 1;
            }
        }
    }
    return 0;
}

/*
 * Register the state's voice id in either the 1D or 2D slot table.
 */
void voiceRegister(McmdVoiceState* state)
{
    McmdVoiceState* voiceState = state;
    u32 voice = voiceState->handle;
    u8 slot;
    u8 channel;
    u8 voiceIdx;
    if (voice == SYNTH_INVALID_VOICE)
        return;
    slot = voiceState->midiSlot;
    if (slot == SYNTH_INVALID_VOICE_U8)
        return;
    channel = voiceState->midiChannel;
    voiceIdx = voice;
    if (channel == SYNTH_INVALID_VOICE_U8)
    {
        voiceDirectSlots[voiceIdx] = voiceIdx;
    }
    else
    {
        voiceMidiKeySlots[channel][slot] = voiceIdx;
    }
}

void voiceUnregister(McmdVoiceState* voice)
{
    u32 voiceId;
    u32 midiSlot;
    u32 midiChannel;
    u32 vid8;
    u8* slot;

    voiceId = voice->handle;
    if (voiceId == SYNTH_INVALID_VOICE)
        return;
    midiSlot = voice->midiSlot;
    if (midiSlot == SYNTH_INVALID_VOICE_U8)
        return;
    midiChannel = voice->midiChannel;
    vid8 = voiceId & 0xff;
    if (midiChannel == SYNTH_INVALID_VOICE_U8)
    {
        slot = &voiceDirectSlots[vid8];
        if (*slot != vid8)
            return;
        *slot = SYNTH_INVALID_VOICE_U8;
    }
    else
    {
        slot = &voiceMidiKeySlots[midiChannel][midiSlot];
        if (vid8 != *slot)
            return;
        *slot = SYNTH_INVALID_VOICE_U8;
    }
}
