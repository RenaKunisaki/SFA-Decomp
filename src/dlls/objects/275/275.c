/*
 * DLL 0x113 implements sequence-triggered placed objects. SeqObj2_seqFn
 * handles in-sequence events that clear the trigger bit or set the open bit;
 * update then preempts or starts the placement's sequence as its game bits
 * change. The game-bit IDs are placement data, with -1 as the unused sentinel.
 */
#include "main/dll/dll_0112_seqobject.h"
#include "main/dll/dll_0113_seqobj2.h"
#include "game/objects/object.h"
#include "main/gamebits.h"
#include "main/obj_group.h"
#include "main/objseq.h"
#include "dolphin/os.h"

STATIC_ASSERT(sizeof(SeqObj2State) == 0x1);

/* object group this object joins while active */
#define SEQOBJ2_OBJGROUP 0xf

#define SEQOBJECT_STATE_OPEN             0x01
#define SEQOBJECT_STATE_TRIGGER_SEQUENCE 0x02

#define SEQOBJECT_FLAG_LATCH_SOURCE_CLEAR     0x01
#define SEQOBJECT_FLAG_SET_SOURCE_ON_SEQUENCE 0x02
#define SEQOBJECT_FLAG_CLEAR_TARGET_ON_DONE   0x04
#define SEQOBJECT_FLAG_SET_SOURCE_ON_DONE     0x08
#define SEQOBJECT_FLAG_USE_TRIGGER_PARAM      0x10
#define SEQOBJECT_FLAG_UNUSED_20              0x20

#define SEQOBJECT_OBJFLAG_HIDDEN             0x4000
#define SEQOBJECT_OBJFLAG_HITDETECT_DISABLED 0x2000

extern const char sSeqObjNeedBitClearDuringSequenceFormat[];
extern const char lbl_80321208[];
extern const char sSeqObjNeedBitUsedBitFormat[];

int SeqObj2_seqFn(int* obj, int* anim, ObjAnimUpdateState* animUpdate)
{
    SeqObjectPlacement* def = (SeqObjectPlacement*)((GameObject*)obj)->anim.placementData;
    SeqObj2State* state = ((GameObject*)obj)->extra;
    int i;
    enum
    {
        SEQOBJ2_SEQEV_CLEAR_TRIGGER = 0,
        SEQOBJ2_SEQEV_SET_OPEN = 1
    };
    for (i = 0; i < animUpdate->eventCount; i++)
    {
        int op = animUpdate->eventIds[i];
        switch (op)
        {
        case SEQOBJ2_SEQEV_CLEAR_TRIGGER:
            mainSetBits(def->triggerGameBit, 0);
            OSReport(sSeqObjNeedBitClearDuringSequenceFormat, def->base.mapId);
            break;
        case SEQOBJ2_SEQEV_SET_OPEN:
            mainSetBits(def->openGameBit, 1);
            OSReport(lbl_80321208, def->base.mapId);
            break;
        }
    }
    state->flags = (u8)(state->flags | SEQOBJECT_STATE_TRIGGER_SEQUENCE);
    return 0;
}

int SeqObj2_getExtraSize(void)
{
    return 0x1;
}
int SeqObj2_getObjectTypeId(void)
{
    return 0x0;
}

void SeqObj2_free(int obj)
{
    ObjGroup_RemoveObject(obj, SEQOBJ2_OBJGROUP);
}

void SeqObj2_render(void)
{
}

void SeqObj2_hitDetect(void)
{
}

void SeqObj2_update(int* obj)
{
    SeqObj2State* state;
    SeqObjectPlacement* def;
    char* strBase;
    u32 bitValue;

    strBase = (char*)&gSeqObj2ObjDescriptor;
    state = ((GameObject*)obj)->extra;
    def = (SeqObjectPlacement*)((GameObject*)obj)->anim.placementData;

    if ((state->flags & SEQOBJECT_STATE_OPEN) != 0)
    {
        if ((def->flags & SEQOBJECT_FLAG_LATCH_SOURCE_CLEAR) != 0)
        {
            mainSetBits(def->triggerGameBit, 0);
            OSReport(strBase + 0x94, def->base.mapId);
        }
        if ((def->flags & SEQOBJECT_FLAG_SET_SOURCE_ON_DONE) != 0)
        {
            mainSetBits(def->openGameBit, 1);
            OSReport(strBase + 0xd0, def->base.mapId);
        }
        OSReport(strBase + 0x108, def->base.mapId, def->sequenceParam);
        (*gObjectTriggerInterface)->preempt((int)obj, def->preemptSequenceId);
        bitValue = def->sequenceParam;
        (*gObjectTriggerInterface)->runSequence(def->triggerId, obj, bitValue);
        state->flags = (u8)(state->flags & ~SEQOBJECT_STATE_OPEN);
    }
    else if ((state->flags & SEQOBJECT_STATE_TRIGGER_SEQUENCE) != 0)
    {
        if ((def->flags & SEQOBJECT_FLAG_SET_SOURCE_ON_SEQUENCE) != 0)
        {
            mainSetBits(def->triggerGameBit, 0);
            OSReport(strBase + 0x140, def->base.mapId);
        }
        if ((def->flags & SEQOBJECT_FLAG_USE_TRIGGER_PARAM) != 0)
        {
            mainSetBits(def->openGameBit, 1);
            OSReport(strBase + 0x170, def->base.mapId);
        }
        state->flags = (u8)(state->flags & ~SEQOBJECT_STATE_TRIGGER_SEQUENCE);
    }
    else
    {
        if ((def->triggerGameBit == -1 || mainGetBit(def->triggerGameBit) != 0) &&
            (def->openGameBit == -1 || mainGetBit(def->openGameBit) == 0))
        {
            if ((def->flags & SEQOBJECT_FLAG_CLEAR_TARGET_ON_DONE) != 0)
            {
                mainSetBits(def->triggerGameBit, 0);
                OSReport(strBase + 0x19c, def->base.mapId);
            }
            if ((def->flags & SEQOBJECT_FLAG_UNUSED_20) != 0)
            {
                mainSetBits(def->openGameBit, 1);
                OSReport(strBase + 0x1cc, def->base.mapId);
            }
            OSReport(strBase + 0x1f8, def->base.mapId);
            (*gObjectTriggerInterface)->runSequence(def->triggerId, obj, -1);
        }
    }
}

void SeqObj2_init(int* obj, SeqObjectPlacement* def)
{
    SeqObj2State* state = ((GameObject*)obj)->extra;
    OSReport(sSeqObjNeedBitUsedBitFormat, def->base.mapId, def->triggerGameBit, def->openGameBit);
    ((GameObject*)obj)->anim.rotX = (s16)((u32)def->initialYaw << 8);
    ((GameObject*)obj)->animEventCallback = SeqObj2_seqFn;
    if (def->preemptSequenceId > -1)
    {
        s16 slot = def->openGameBit;
        if (slot != -1 && mainGetBit(slot) != 0u)
        {
            state->flags = (u8)(state->flags | SEQOBJECT_STATE_OPEN);
        }
    }
    ObjGroup_AddObject((u32)obj, SEQOBJ2_OBJGROUP);
    ((GameObject*)obj)->objectFlags =
        (u16)(((GameObject*)obj)->objectFlags | (SEQOBJECT_OBJFLAG_HIDDEN | SEQOBJECT_OBJFLAG_HITDETECT_DISABLED));
}

void SeqObj2_release(void)
{
}

void SeqObj2_initialise(void)
{
}

ObjectDescriptor gSeqObj2ObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    SeqObj2_initialise,
    SeqObj2_release,
    0,
    (ObjectDescriptorCallback)SeqObj2_init,
    (ObjectDescriptorCallback)SeqObj2_update,
    SeqObj2_hitDetect,
    SeqObj2_render,
    (ObjectDescriptorCallback)SeqObj2_free,
    (ObjectDescriptorCallback)SeqObj2_getObjectTypeId,
    SeqObj2_getExtraSize,
};

const char sSeqObjNeedBitClearDuringSequenceFormat[] = "newseqobj %d: need bit clear during sequence\n";

const char lbl_80321208[444] =
    "newseqobj %d: used bit set during sequence\n\000newseqobj %d: need bit clear before preempting "
    "sequence\n\000\000\000\000newseqobj %d: used bit set before preempting sequence\n\000\000newseqobj %d: about to "
    "prempt the sequence - objs %d\n\000\000\000newseqobj %d: need bit clear after sequence\n\000\000\000\000newseqobj "
    "%d: used bit set after sequence\n\000\000newseqobj %d: need bit clear before sequence\n\000\000\000newseqobj %d: "
    "used bit set before sequence\n\000newseqobj %d: about to start the sequence\n\000\000";
const char sSeqObjNeedBitUsedBitFormat[40] = "newseqobj %d: Need Bit %d, Used Bit %d\n\000";
