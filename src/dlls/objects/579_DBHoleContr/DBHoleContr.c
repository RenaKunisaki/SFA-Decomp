/* DBHoleContr (DLL 0x243) */
#include "game/objects/object.h"
#include "main/object_update_list.h"
#include "main/obj_group.h"
#include "main/object_render.h"
#include "sys/objects/lifecycle.h"
#include "main/dll/dbholecontrol1state_struct.h"
#include "main/objseq.h"
#include "main/gamebits.h"
#include "main/obj_message.h"
#include "sys/objects.h"
#include "main/dll/dll_0243_dbholecontrol1.h"
#include "string.h"
#include "main/lightmap.h"

STATIC_ASSERT(sizeof(DbHoleControl1State) == 0xC);

#define DBHOLECONTROL1_OBJGROUP  0x1e
#define DBEGG_OBJGROUP           0x24
#define DBHOLECONTROL1_CHILD_OBJ 1337

int lbl_803DDCE0;

int dbholecontrol1_SeqFn(GameObject* obj, int unused, ObjAnimUpdateState* animUpdate)
{
    int newObj;
    void* res;
    int* objs;
    int count;
    int data = *(int*)&obj->anim.placementData;
    int i;

    for (i = 0; i < animUpdate->eventCount; i++)
    {
        switch (animUpdate->eventIds[i])
        {
        case 1:
            if (mainGetBit((s32)((Dbholecontrol1Placement*)data)->triggerSeqId + 2601) != 0)
                continue;
            if (Obj_IsLoadingLocked() == 0)
                continue;
            res = mapRomListFindItem(0x4658A, 0, 0, 0, 0);
            if (res == NULL)
                continue;
            newObj = (int)Obj_AllocObjectSetup(56, DBHOLECONTROL1_CHILD_OBJ);
            memcpy((void*)newObj, res, 56);
            ((GameObject*)newObj)->anim.rootMotionScale = obj->anim.localPosX;
            ((GameObject*)newObj)->anim.localPosX = obj->anim.localPosY;
            ((GameObject*)newObj)->anim.localPosY = obj->anim.localPosZ;
            *(int*)&((GameObject*)newObj)->anim.localPosZ = -1;
            *(s16*)(newObj + 26) = 149;
            loadObjectAtObject(obj, (ObjPlacement*)newObj);
            break;
        }
    }

    if (mainGetBit(((Dbholecontrol1Placement*)data)->hideGameBit) != 0 || lbl_803DDCE0 != 0)
    {
        objs = (int*)ObjGroup_GetObjects(DBEGG_OBJGROUP, &count);
        ObjMsg_SendToObjects(0, 3, obj, 17, 0);
        while (count-- != 0)
        {
            ObjGroup_RemoveObject(*objs++, DBEGG_OBJGROUP);
        }
        return 4;
    }
    return 0;
}

int dbholecontrol1_getExtraSize(void)
{
    return 0xc;
}
int dbholecontrol1_getObjectTypeId(void)
{
    return 0x0;
}

void dbholecontrol1_free(int obj)
{
    ObjGroup_RemoveObject(obj, DBHOLECONTROL1_OBJGROUP);
}

void dbholecontrol1_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible)
{
    s32 enabled = visible;
    if (enabled != 0)
        objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, (1.0f));
}

void dbholecontrol1_hitDetect(void)
{
}

void dbholecontrol1_update(GameObject* obj)
{

    u8* def;
    def = *(u8**)&obj->anim.placementData;
    if (mainGetBit(((Dbholecontrol1Placement*)def)->hideGameBit) != 0)
    {
        Obj_RemoveFromUpdateList(obj);
        obj->anim.flags = (s16)(obj->anim.flags | OBJANIM_FLAG_HIDDEN);
    }
    else if (mainGetBit(((Dbholecontrol1Placement*)def)->triggerGameBit) != 0)
    {
        (*gObjectTriggerInterface)->runSequence(((Dbholecontrol1Placement*)def)->triggerSeqId, obj, -1);
    }
}

void dbholecontrol1_init(GameObject* obj, u8* params)
{
    DbHoleControl1State* state = obj->extra;
    ObjGroup_AddObject((int)obj, DBHOLECONTROL1_OBJGROUP);
    *(s16*)obj = (s16)((s8)params[0x18] << 8);
    obj->animEventCallback = dbholecontrol1_SeqFn;
    state->gameBitA = ((Dbholecontrol1Placement*)params)->gameBitA;
    state->gameBitB = ((Dbholecontrol1Placement*)params)->gameBitB;
}

void dbholecontrol1_release(void)
{
}

void dbholecontrol1_initialise(void)
{
}

ObjectDescriptor gDBHoleControl1ObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)dbholecontrol1_initialise,
    (ObjectDescriptorCallback)dbholecontrol1_release,
    0,
    (ObjectDescriptorCallback)dbholecontrol1_init,
    (ObjectDescriptorCallback)dbholecontrol1_update,
    (ObjectDescriptorCallback)dbholecontrol1_hitDetect,
    (ObjectDescriptorCallback)dbholecontrol1_render,
    (ObjectDescriptorCallback)dbholecontrol1_free,
    (ObjectDescriptorCallback)dbholecontrol1_getObjectTypeId,
    dbholecontrol1_getExtraSize,
};
