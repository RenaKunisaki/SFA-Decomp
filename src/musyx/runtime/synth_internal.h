#ifndef SFA_AUDIO_SYNTH_INTERNAL_H
#define SFA_AUDIO_SYNTH_INTERNAL_H

#ifndef SYNTH_INTERNAL_USE_PROJECT_TYPES
#include "ghidra_import.h"
#endif

#include "musyx/mcmd.h"
#include "musyx/snd_core.h"
#include "musyx/synth_queue.h"
#include "musyx/synth_master_fader.h"

#define SYNTH_CALLBACK_COUNT 0x100
#define SYNTH_SEQUENCE_TRACK_COUNT 0x40
#define SYNTH_STUDIO_CHANNEL_SCALE_STUDIO_COUNT 9
#define SYNTH_DELAY_BUCKET_COUNT 0x20
#define SYNTH_DELAY_BUCKET_INVALID 0xFF
#define SYNTH_HANDLE_INVALID 0xFFFFFFFF
#define SYNTH_HANDLE_ID_MASK 0x7FFFFFFF
#define SYNTH_HANDLE_QUEUED_FLAG 0x80000000
#define SND_CROSSFADE_STOP 0
#define SND_CROSSFADE_PAUSE 1
#define SND_CROSSFADE_CONTINUE 2
#define SND_CROSSFADE_START 0
#define SND_CROSSFADE_SYNC 4
#define SND_CROSSFADE_PAUSENEW 8
#define SND_CROSSFADE_TRACKMUTE 16
#define SND_CROSSFADE_SPEED 32
#define SND_CROSSFADE_MUTE 64
#define SND_CROSSFADE_MUTENEW 128
#define SYNTH_VARIABLE_PAIR_EXTENDED_FLAG 0x80
#define SYNTH_VARIABLE_PAIR_VALUE_MASK 0x7F
#define SYNTH_VARIABLE_PAIR_END_LOW 0x00
#define SYNTH_INVALID_LINK_ID 0xFFFFFFFF

typedef struct SynthCallbackLink
{
    struct SynthCallbackLink* next;
    struct SynthCallbackLink* prev;
    u32 callbackId;
    s32 triggerValue;
    u8 controllerIndex;
    u8 listIndex;
    u8 unk12[2];
} SynthCallbackLink;

typedef struct SynthDelayedNode
{
    struct SynthDelayedNode* next;
    struct SynthDelayedNode* prev;
    u8 voiceIndex;
    u8 bucketIndex;
    u8 pad[2];
} SynthDelayedNode;

typedef union SynthDelayedActionWord
{
    u32 word;

    struct
    {
        u8 action;
        u8 pad[3];
    } bytes;
} SynthDelayedActionWord;

typedef struct SynthDelayedEntry
{
    SynthDelayedNode nodes[3];
    u32 word0;
    u32 word1;
    SynthDelayedActionWord word2;
    u32 word3;
} SynthDelayedEntry;

typedef struct SynthDelayStorage
{
    u32 studioChannelScales[SYNTH_STUDIO_CHANNEL_SCALE_STUDIO_COUNT][SYNTH_VOICE_NOTE_COUNT];
    SynthDelayedNode* bucketHeads[SYNTH_DELAY_BUCKET_COUNT][3];
} SynthDelayStorage;

/* The 0x404 voice-slot record is the canonical McmdVoiceState (mcmd.h);
 * the former McmdVoiceState view is retired. */

typedef struct SynthPitchPoint
{
    u32 threshold;
    u32 value;
} SynthPitchPoint;

typedef struct SynthChannelState
{
    s32 eventActive;
    SynthPitchPoint* eventCursor;
    u32 currentValue;
    u8 unk0C[0x24 - 0x0C];
    u32 threshold0;
    u32 unk28;
    u32 threshold1;
    u8 thresholdIndex;
    u8 unk31[0x38 - 0x31];
} SynthChannelState;

typedef struct SynthTrackCursor
{
    u8* base;
    void* current;
} SynthTrackCursor;

typedef struct SynthSequenceState SynthSequenceState;

typedef struct SynthSequenceEvent
{
    struct SynthSequenceEvent* next;
    struct SynthSequenceEvent* prev;
    u32 time;
    void* data;
    SynthSequenceState* state;
    u8 type;
    u8 trackId;
    u8 pad16[2];
} SynthSequenceEvent;

typedef struct SynthSequenceStream
{
    u8* cursor;
    u16 value;
    s16 step;
    u32 nextTime;
} SynthSequenceStream;

struct SynthSequenceState
{
    u32 lastTime;
    u32 baseTime;
    u8* noteData;
    void* patternInfo;
    SynthSequenceStream pitchBend;
    SynthSequenceStream modulation;
    u8 midi;
    u8 pad29[3];
};

typedef struct SynthTimeWord
{
    u32 low;
    u32 high;
} SynthTimeWord;

typedef struct SynthMasterTrackEvent
{
    u32 time;
    u32 bpm;
} SynthMasterTrackEvent;

typedef struct SynthSequenceQueue
{
    u8* masterTrackBase;
    u8* masterTrackCursor;
    u32 bpm;
    SynthTimeWord tickDelta[2];
    SynthSequenceEvent* eventList;
    SynthTimeWord time[2];
    u8 timeIndex;
    u8 unk31;
    u16 speed;
    u16 loopCount;
    u8 loopDisable;
    u8 unk37;
} SynthSequenceQueue;

typedef struct SynthCallbackControllerState
{
    u8 listIndex;
    u8 unk01;
    u16 value16;
    u8 unk04[0x38 - 4];
} SynthCallbackControllerState;

typedef struct SynthTrackCommand
{
    u32 value0;
    u32 value1;
    u16 command;
    u16 arg;
} SynthTrackCommand;

typedef struct SynthStartRequest
{
    u32 seqId1;
    u16 time1;
    u8 pad06[2];
    u32 seqId2;
    u16 time2;
    u8 pad0E[2];
    u32 arr2;
    u16 gid2;
    u16 sid2;
    u8 vol2;
    u8 studio2;
    u8 pad1A[2];
    u32 trackMute2[2];
    u16 speed2;
    u8 flags;
    u8 pad27;
} SynthStartRequest;

typedef struct SynthKeyGroupState
{
    u8 unk00[0x36];
    u8 active;
    u8 pad37;
} SynthKeyGroupState;

/* The voice tail overlays sequence events, queue/keygroup state, and callback controller state. */
typedef struct SynthVoiceEventScratch
{
    u8 unk00[4];
    SynthSequenceEvent channelEvents[SYNTH_SEQUENCE_TRACK_COUNT];
    u8* keyGroupMap;
} SynthVoiceEventScratch;

typedef union SynthVoiceControllerOverlay
{
    SynthSequenceQueue sequenceQueues[SYNTH_VOICE_NOTE_COUNT];
    SynthKeyGroupState keyGroupStates[SYNTH_VOICE_NOTE_COUNT];
    SynthChannelState channelStates[SYNTH_VOICE_NOTE_COUNT];
    SynthCallbackControllerState callbackControllers[SYNTH_VOICE_NOTE_COUNT];
    u8 raw[0x380];
} SynthVoiceControllerOverlay;

typedef struct SynthVoiceScratch
{
    SynthVoiceEventScratch eventScratch;
    SynthVoiceControllerOverlay overlay;
} SynthVoiceScratch;

typedef struct SynthProgramState
{
    u16 macId;
    u8 priority;
    u8 maxVoices;
} SynthProgramState;

typedef struct SynthVoice
{
    struct SynthVoice* next;
    struct SynthVoice* prev;
    u8 state;
    u8 slotIndex;
    u16 groupId;
    u32 handle;
    SynthPage* normtab;
    u8 normTrans[0x80];
    SynthPage* drumtab;
    u8 drumTrans[0x80];
    u8* arrbase;
    u32 trackMute[2];
    u8 unk124[0x200];
    u8 trackVolumeGroup[0x40];
    u8 unk364[0xB00];
    SynthCallbackLink* callbackLists[3];
    SynthProgramState prgState[0x10];
    u8 defaultVolumeGroup;
    u8 unkEB1[3];
    SynthStartRequest syncCrossInfo;
    u32* syncSeqIdPtr;
    u8 syncActive;
    u8 defStudio;
    u8 keyOffCheck;
    u8 unkEE3;
    SynthSequenceEvent channelEvents[SYNTH_SEQUENCE_TRACK_COUNT];
    u8* keyGroupMap;
    SynthSequenceQueue section[SYNTH_VOICE_NOTE_COUNT];
} SynthVoice;

typedef struct SynthVoiceRuntime
{
    SynthCallbackLink callbacks[SYNTH_CALLBACK_COUNT];
    SynthVoice voices[SYNTH_MAX_VOICES];
    u16 voiceNotes[SYNTH_MAX_VOICES][SYNTH_VOICE_NOTE_COUNT];
} SynthVoiceRuntime;

#define SYNTH_VOICE_EVENT_SCRATCH(voice) ((SynthVoiceEventScratch*)((u8*)(voice) + 0xEE0))
#define SYNTH_VOICE_CONTROLLER_OVERLAY(voice) ((SynthVoiceControllerOverlay*)((u8*)(voice) + 0x14E8))
#define SYNTH_CHANNEL_STATE(voice, channel) \
    ((SynthChannelState*)((u8*)(voice) + 0x14E8 + (((channel) & 0xFF) * sizeof(SynthChannelState))))
#define SYNTH_CHANNEL_THRESHOLD(state, index) (*(u32*)((u8*)(state) + 0x24 + ((index) * 8)))
#define SYNTH_CHANNEL_EVENT(voice, channel) \
    ((SynthSequenceEvent*)((u8*)(voice) + 0xEE4 + ((channel) * 0x18)))
#define SYNTH_KEYGROUP_MAP(voice) (*(u8**)((u8*)(voice) + 0x14E4))
#define SYNTH_SEQUENCE_QUEUE(voice, index) \
    ((SynthSequenceQueue*)((u8*)(voice) + 0x14E8 + (((index) & 0xFF) * sizeof(SynthSequenceQueue))))
#define SYNTH_KEYGROUP_STATE(voice, index) \
    ((SynthKeyGroupState*)((u8*)(voice) + 0x14E8 + ((index) * 0x38)))
#define SYNTH_SEQUENCE_STATE(voice, channel) ((SynthSequenceState*)&(voice)->unk364[(channel) * 0x2C])
#define SYNTH_TRACK_CURSOR(voice, channel) ((SynthTrackCursor*)&(voice)->unk124[(channel) * 8])
#define SYNTH_CHANNEL_SPEED_VALUE(voice, channel) (*(u16*)((u8*)(voice) + 0x151A + ((channel) * 0x38)))
#define SYNTH_RUNTIME_CHANNEL_SPEED_VALUE(runtime, voiceIndex, channel) \
    (*(u16*)((u8*)(runtime) + 0x291A + ((voiceIndex) * sizeof(SynthVoice)) + ((channel) * 0x38)))
#define SYNTH_RUNTIME_PENDING_VALUE16(runtime, voiceIndex) \
    (*(u16*)((u8*)(runtime) + 0x22D8 + ((voiceIndex) * sizeof(SynthVoice))))
#define SYNTH_RUNTIME_PENDING_FLAGS(runtime, voiceIndex) \
    (*(u8*)((u8*)(runtime) + 0x22DA + ((voiceIndex) * sizeof(SynthVoice))))

extern SynthCallbackLink seqNote[SYNTH_CALLBACK_COUNT];
extern u8 gSynthDelayBucketCursor;
extern SynthCallbackLink* noteFree;
extern SynthVoice* cseq;
extern u32 curSeqId;
extern u8 curFadeOutState;

extern SynthVoice seqInstance[SYNTH_MAX_VOICES];
extern u8 synthTrackVolume[64];
extern SynthVoice* seqFreeRoot;
extern SynthVoice* seqActiveRoot;
extern SynthVoice* seqPausedRoot;
extern u32 seq_next_id;

#define SYNTH_VOICE_RUNTIME() ((SynthVoiceRuntime*)(void*)seqNote)
#define SYNTH_VOICE_SLOT_FLAGS64(slot) (*(u64*)&(slot)->inputFlags)


void synthSetBpm(int bpm, u8 set, u8 section);
int synthGetTicksPerSecond(McmdVoiceState *slot);
SynthSequenceEvent* GenerateNextTrackEvent(u8 channel);
void InsertGlobalEvent(SynthSequenceQueue* queue, SynthSequenceEvent* event);
SynthSequenceEvent* HandleEvent(SynthSequenceEvent* event, u8 groupIndex, u32* output);
void synthInitChannelEventQueues(void);
void synthRefreshChannelEventQueue(u8 groupIndex);
u32 HandleTrackEvents(u8 groupIndex, u32 delta);
void ResetNotes(SynthVoice* voice);
SynthCallbackLink* AllocateNote(s32 triggerValue, u8 controllerIndex);
s32 HandleNotes(void);
void KeyOffNotes(void);
void seqFreeKeyOffNote(SynthCallbackLink* callback);
u32 GetPublicId(s32 voiceIndex);
u32 seqGetPrivateId(u32 seqId);

static inline u32 seqGetPrivateIdInline(u32 handle)
{
    u32 resolvedHandle;
    SynthVoice* walker;

    resolvedHandle = handle & SYNTH_HANDLE_ID_MASK;

    for (walker = seqActiveRoot; walker != 0; walker = walker->next)
    {
        if (walker->handle == resolvedHandle)
        {
            return walker->slotIndex | (handle & SYNTH_HANDLE_QUEUED_FLAG);
        }
    }

    for (walker = seqPausedRoot; walker != 0; walker = walker->next)
    {
        if (walker->handle == resolvedHandle)
        {
            return walker->slotIndex | (handle & SYNTH_HANDLE_QUEUED_FLAG);
        }
    }

    return SYNTH_HANDLE_INVALID;
}
void seqPause(u32 seqId);
void seqStop(u32 seqId);
void seqSpeed(u32 seqId, u16 speed);
void seqContinue(u32 seqId);
void seqMute(u32 seqId, u32 mask1, u32 mask2);
u32 synthFXSetCtrl(u32 handle, u8 controller, u8 value);
u32 synthFXSetCtrl14(u32 handle, u8 controller, u16 value);
void synthFXCloneMidiSetup(McmdVoiceState *dstVoice, McmdVoiceState *srcVoice);
u32 synthSendKeyOff(u32 handle);
void seqVolume(u8 volume, u16 time, u32 seqId, u8 mode);

#endif
