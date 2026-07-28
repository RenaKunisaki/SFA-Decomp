/* WM_seqobjec (DLL 0x01FA) */
#include "dlls/object_descriptor.h"
#include "game/objects/object.h"
#include "main/dll/WM/dll_01FA_wmseqobject.h"
#include "main/dll/wmseqobjectsetup_struct.h"
#include "main/gamebits.h"
#include "main/obj_group.h"
#include "main/objseq.h"
#include "main/object_render.h"
#include "main/screen_transition.h"

STATIC_ASSERT(offsetof(WMSeqObjectSetup, yawByte) == 0x18);
STATIC_ASSERT(offsetof(WMSeqObjectSetup, setupType) == 0x19);

#define OBJ_S16(obj, offset) (*(s16*)((u8*)(obj) + (offset)))
#define OBJ_S32(obj, offset) (*(s32*)((u8*)(obj) + (offset)))
#define OBJ_PTR(obj, offset) (*(void**)((u8*)(obj) + (offset)))

#define OBJECT_TRIGGER_REFRESH(eventId, obj, arg) (*gObjectTriggerInterface)->runSequence((eventId), (obj), (arg))
#define SCREEN_TRANSITION_START(kind, value)      (*gScreenTransitionInterface)->step((kind), (value))

u8 lbl_803DDC78;

int WM_seqobject_SeqFn(int obj, int unused, ObjAnimUpdateState* animUpdate)
{
    int i;

    for (i = 0; i < animUpdate->eventCount; i++)
    {
        if (animUpdate->eventIds[i] == 1)
        {
            lbl_803DDC78 = (u8)(1 - lbl_803DDC78);
        }
    }
    animUpdate->triggerCommand = 0;
    animUpdate->sequenceEventActive = 0;
    return 0;
}

int WM_seqobject_getExtraSize(void)
{
    return 1;
}
int WM_seqobject_getObjectTypeId(void)
{
    return 0;
}

void WM_seqobject_free(void)
{
}

void WM_seqobject_render(int obj, int p2, int p3, int p4, int p5, s8 visible)
{
    s32 v = visible;

    if (v != 0)
    {
        objRenderModelAndHitVolumes((GameObject*)obj, p2, p3, p4, p5, 1.0f);
    }
}

void WM_seqobject_hitDetect(void)
{
}

void WM_seqobject_update(int* obj)
{
    int count;
    int countdown;
    int* objects;
    int found;
    int i;
    int setupType;
    WMSeqObjectSetup* setup;

    setup = (WMSeqObjectSetup*)OBJ_PTR(obj, 0x4c);
    setupType = setup->setupType;
    switch (setupType)
    {
    case 8:
        break;
    case 0:
        if (OBJ_S32(obj, 0xf4) != 0)
        {
            return;
        }
        if (mainGetBit(GAMEBIT_WM_GalleonRelated00A4) != 0)
        {
            return;
        }
        if (mainGetBit(GAMEBIT_WM_Galleon_despawn) != 0)
        {
            return;
        }

        objects = (int*)ObjGroup_GetObjects(6, &count);
        found = 0;
        for (i = 0; i < count; i++)
        {
            if (OBJ_S16(*(int**)(objects + i), 0x46) == 0x139)
            {
                found = 1;
            }
        }

        if (found != 0)
        {
            if (OBJ_S32(obj, 0xf8) == 0)
            {
                OBJECT_TRIGGER_REFRESH(0, obj, -1);
                OBJ_S32(obj, 0xf4) = 1;
                mainSetBits(GAMEBIT_WM_GalleonRelated00A4, 1);
            }
            else
            {
                SCREEN_TRANSITION_START(0x50, 1);
            }
        }
        else
        {
            OBJ_S32(obj, 0xf8) = 0x14;
            SCREEN_TRANSITION_START(0x50, 1);
        }

        countdown = OBJ_S32(obj, 0xf8) - 1;
        OBJ_S32(obj, 0xf8) = countdown;
        if (countdown < 0)
        {
            OBJ_S32(obj, 0xf8) = 0;
        }
        break;
    }
}

void WM_seqobject_init(GameObject* obj, s8* def)
{
    s16 angle;
    WMSeqObjectSetup* setup = (WMSeqObjectSetup*)def;

    angle = (s16)((s32)setup->yawByte << 8);
    OBJ_S16(obj, 0) = angle;
    obj->animEventCallback = WM_seqobject_SeqFn;
    OBJ_S32(obj, 0xf8) = 0x14;
}

void WM_seqobject_release(void)
{
}

void WM_seqobject_initialise(void)
{
}

ObjectDescriptor gWM_seqobjectObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)WM_seqobject_initialise,
    (ObjectDescriptorCallback)WM_seqobject_release,
    0,
    (ObjectDescriptorCallback)WM_seqobject_init,
    (ObjectDescriptorCallback)WM_seqobject_update,
    (ObjectDescriptorCallback)WM_seqobject_hitDetect,
    (ObjectDescriptorCallback)WM_seqobject_render,
    (ObjectDescriptorCallback)WM_seqobject_free,
    (ObjectDescriptorCallback)WM_seqobject_getObjectTypeId,
    WM_seqobject_getExtraSize,
};
