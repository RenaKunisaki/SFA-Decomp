#include "global.h"
#include "musyx/voice_id.h"
#include "musyx/mcmd.h"
#include "musyx/voice_unregister.h"
#include "musyx/vid_init.h"
#include "musyx/voice_manage.h"
#include "musyx/vidlisttables.h"

#define voicePriorityLinks ((u8*)vidList + offsetof(VidListTables, priorityLinks))

#define VID_UNLINK(field)                                                                                              \
    if (s->field->prev != 0)                                                                                           \
    {                                                                                                                  \
        s->field->prev->next = s->field->next;                                                                         \
    }                                                                                                                  \
    else                                                                                                               \
    {                                                                                                                  \
        vidRoot = s->field->next;                                                                                      \
    }                                                                                                                  \
    if (s->field->next != 0)                                                                                           \
    {                                                                                                                  \
        s->field->next->prev = s->field->prev;                                                                         \
    }                                                                                                                  \
    s->field->next = vidFree;                                                                                          \
    if (vidFree != 0)                                                                                                  \
    {                                                                                                                  \
        vidFree->prev = s->field;                                                                                      \
    }                                                                                                                  \
    s->field->prev = 0;                                                                                                \
    vidFree = s->field

void vidRemoveVoiceReferences(McmdVoiceState* state)
{
    McmdVoiceState* s = state;
    if (s->id != 0xffffffff)
    {
        voiceResetLastStarted(state);
        if (s->parent != 0xffffffff)
        {
            synthVoice[s->parent & 0xff].child = s->child;
            if (s->child != 0xffffffff)
            {
                synthVoice[s->child & 0xff].parent = s->parent;
            }
            VID_UNLINK(vidList);
            s->vidList = 0;
        }
        else if (s->child != 0xffffffff)
        {
            s->vidList->root = s->child;
            synthVoice[s->child & 0xff].parent = 0xffffffff;
            synthVoice[s->child & 0xff].vidMasterList = s->vidMasterList;
            if (s->vidList != s->vidMasterList)
            {
                VID_UNLINK(vidList);
                s->vidList = 0;
            }
            s->vidList = 0;
            s->vidMasterList = 0;
        }
        else if (s->vidList != s->vidMasterList)
        {
            VID_UNLINK(vidList);
            s->vidList = 0;
            VID_UNLINK(vidMasterList);
            s->vidMasterList = 0;
        }
        else
        {
            VID_UNLINK(vidList);
            s->vidList = 0;
            s->vidMasterList = 0;
        }
    }
}

/*
 * Snapshot the current entry's `next` pointer (state->[0xf8]) into the
 * cached field (state->[0xfc]) and return that next entry's id field.
 */
u32 vidMakeRoot(McmdVoiceState* state)
{
    McmdVoiceState* s = state;
    s->vidMasterList = s->vidList;
    return s->vidList->vid;
}

/*
 * Allocate the next unique id from the global counter, walking the
 * sorted-by-id list to skip any already-in-use ids. Used to assign
 * fresh handles to dynamically-allocated voices.
 */
u32 vidMakeNew(McmdVoiceState* state, int returnNewId)
{
    McmdVoiceState* s = state;
    u32 nextId;
    VID_LIST* cursor;
    VID_LIST* node;
    VID_LIST* prev;
    VID_LIST* freeNode;

    do
    {
        nextId = vidCurrentId;
        vidCurrentId = nextId + 1;
    } while (nextId == 0xffffffffU);

    cursor = vidRoot;
    prev = 0;
    while ((node = cursor) != 0)
    {
        if (node->vid > nextId)
        {
            break;
        }
        if (node->vid == nextId)
        {
            do
            {
                nextId = vidCurrentId;
                vidCurrentId = nextId + 1;
            } while (nextId == 0xffffffffU);
        }
        prev = node;
        cursor = node->next;
    }

    if ((freeNode = vidFree) == 0)
    {
        return 0xffffffffU;
    }
    if ((vidFree = vidFree->next) != 0)
    {
        vidFree->prev = NULL;
    }
    if (prev == 0)
    {
        vidRoot = freeNode;
    }
    else
    {
        prev->next = freeNode;
    }
    freeNode->prev = prev;
    freeNode->next = node;
    if (node != 0)
    {
        node->prev = freeNode;
    }
    freeNode->vid = nextId;
    freeNode->root = s->id;
    s->vidMasterList = ((u32)returnNewId != 0) ? freeNode : NULL;
    s->vidList = freeNode;
    if ((u32)returnNewId != 0)
    {
        return nextId;
    }
    return s->id;
}

/*
 * Look up a voice handle's slot via the sorted linked list.
 * Returns -1 for the sentinel id 0xFFFFFFFF or if not found.
 */
static inline VID_LIST* get_vidlist(u32 id)
{
    VID_LIST* node;
    node = vidRoot;
    while (node != NULL)
    {
        if (node->vid == id)
            return node;
        if (node->vid > id)
            break;
        node = node->next;
    }
    return NULL;
}

int vidGetInternalId(u32 id)
{
    VID_LIST* node;

    if (id != 0xffffffffU)
    {
        if ((node = get_vidlist(id)) != NULL)
        {
            return node->root;
        }
    }
    return -1;
}

/*
 * voiceRemovePriority - voice priority-queue removal. Removes the active
 * voice from its group's linked list and from the sorted priority list.
 */
void voiceRemovePriority(McmdVoiceState* state)
{
    McmdVoiceState* s = state;
    VidListTables* vb;
    SynthVoiceListNode* vps;
    SynthRootListNode* pr;

    vb = (VidListTables*)vidList;
    vps = (SynthVoiceListNode*)&((u8*)voicePriorityLinks)[(s->id & 0xff) << 2];
    if (vps->user != 1)
    {
        return;
    }
    if (vps->prev != 0xff)
    {
        vb->priorityLinks[vps->prev].next = vps->next;
    }
    else
    {
        vb->priorityGroupHeads[s->prio] = vps->next;
    }
    if (vps->next != 0xff)
    {
        vb->priorityLinks[vps->next].prev = vps->prev;
    }
    else if (vps->prev == 0xff)
    {
        u32 prevv;
        pr = (SynthRootListNode*)((u8*)vb + ((u32)s->prio << 2));
        prevv = *(u16*)((u8*)pr + offsetof(VidListTables, prioritySortLinks) +
                       offsetof(SynthRootListNode, prev));
        pr = (SynthRootListNode*)((u8*)pr + offsetof(VidListTables, prioritySortLinks));
        if (prevv != 0xffff)
        {
            vb->prioritySortLinks[prevv].next = pr->next;
        }
        else
        {
            voicePrioSortedRoot = pr->next;
        }
        if (pr->next != 0xffff)
        {
            vb->prioritySortLinks[pr->next].prev = pr->prev;
        }
    }
    vps->user = 0;
}
