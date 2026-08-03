/*
 * DLL 622 - the swinging chain/shackle that hangs from a
 * path point. drshackle_renderAtPathPoint orients the chain along the segment
 * between two model joints, drshackle_update binds the per-slot path
 * objects (ObjGroup 0x17) the chain rides, and drshackle_hitDetect plays
 * a distance-scaled footstep-style rattle when active.
 *
 * The 0x1A flag byte is a BitFlags8 whose b0 = "active" (chain visible
 * and rattling).
 */
#include "dolphin/mtx/vec.h"
#include "main/objtype.h"
#include "main/object_render.h"
#include "main/objseq.h"
#include "dlls/object_descriptor.h"
#include "dlls/objects/373_DFropenode.h"

#include "main/audio/sfx_trigger_ids.h"

#include "main/dll/DR/dll_026E_drshackle.h"
#include "main/dll/DR/dr_types.h"
#include "main/vecmath.h"
#include "main/audio/sfx_channel_query_api.h"
#include "main/audio/sfx_play_api.h"
#include "main/gamebits_api.h"
#include "main/model.h"
#include "main/obj_path.h"
#include "main/objprint_render_api.h"

int lbl_803DDD70;
int gDrShackleRotZOffset = -32768;

#define DRSHACKLE_OBJGROUP  0x37

static inline int* DrShackle_GetActiveModel(void* obj)
{
    ObjAnimComponent* objAnim = (ObjAnimComponent*)obj;
    return (int*)objAnim->banks[objAnim->bankIndex];
}

int drshackle_SeqFn(GameObject* obj, int unused, ObjSeqState* animUpdate)
{
    char* state = obj->extra;
    GameObject* pathObj = ((DrshackleState*)state)->pathSlots[0];
    int i;
    if (pathObj != 0)
    {
        pathObj->anim.localPosX = obj->anim.localPosX;
        pathObj->anim.localPosY = obj->anim.localPosY;
        pathObj->anim.localPosZ = obj->anim.localPosZ;
    }
    for (i = 0; i < animUpdate->eventCount; i++)
    {
        switch (animUpdate->eventIds[i])
        {
        case 1:
            ((BitFlags8*)(state + 0x1a))->b0 = 0;
            break;
        case 2:
            ((BitFlags8*)(state + 0x1a))->b0 = 1;
            break;
        }
    }
    return 0;
}

int drshackle_getAttachSlot(GameObject* obj)
{
    DrshacklePlacement* placement = (DrshacklePlacement*)obj->anim.placementData;
    return placement->attachSlot;
}

int drshackle_renderAtPathPoint(GameObject* obj, int a, int b, int c, int d, int e, int f)
{
    int* model;
    int* modelData;
    int joint1;
    u8* p = obj->extra;
    int* q = (int*)obj->anim.placementData;
    f32 jointPos[3];
    f32 parentPos[3];
    char* mdPtr;
    int i;
    BitFlags8* bf = (BitFlags8*)(p + 0x1a);
    DrshacklePlacement* placement;
    ObjAnimComponent* objAnim = &obj->anim;

    if (bf->b0 == 0)
    {
        return 1;
    }
    ((DrshackleState*)p)->savedPosX = obj->anim.localPosX;
    ((DrshackleState*)p)->savedPosY = obj->anim.localPosY;
    ((DrshackleState*)p)->savedPosZ = obj->anim.localPosZ;

    {
        s8* jp = (s8*)(*(int*)(*(int*)((char*)a + 0x50) + 0x2c) + b * 24);
        jp += objAnim->bankIndex;
        joint1 = jp[0x12];
    }
    model = DrShackle_GetActiveModel((void*)a);
    modelData = *(int**)model;
    mdPtr = (char*)modelData + 0x3c;

    obj->anim.rotZ = 0;
    obj->anim.rotY = 0;
    ObjModel_CopyJointTranslation((u8*)model, joint1, jointPos);
    ObjModel_CopyJointTranslation((u8*)model, *(s8*)(*(int*)mdPtr + joint1 * 28), parentPos);
    PSVECSubtract((Vec*)parentPos, (Vec*)jointPos, (Vec*)jointPos);

    if (((DrshacklePlacement*)q)->quarterTurns != 0)
    {
        obj->anim.rotZ =
            (s16)(((placement = (DrshacklePlacement*)q)->quarterTurns << 14) + getAngle(jointPos[2], jointPos[0]));
        obj->anim.rotY = (s16)getAngle(jointPos[2], jointPos[1]);
    }
    else
    {
        f32 savedY = jointPos[1];
        f32 mag;
        jointPos[1] = 0.0f;
        mag = PSVECMag((Vec*)jointPos);
        obj->anim.rotZ = (s16)(gDrShackleRotZOffset + getAngle(jointPos[0], jointPos[2]));
        obj->anim.rotY = (s16)(lbl_803DDD70 + getAngle(mag, savedY));
        objSetCurrentMatrix(ObjPath_GetPointModelMtx((GameObject*)a, b));
    }
    ObjPath_GetPointWorldPosition((GameObject*)a, b, &obj->anim.localPosX, &obj->anim.localPosY, &obj->anim.localPosZ,
                                  0);
    objRenderModelAndHitVolumes(obj, c, d, e, f, 1.0f);

    for (i = 0, a = (int)p; i < ((DrshackleState*)p)->slotCount; i++)
    {
        char* entry = *(char**)a;
        if (entry != NULL)
        {
            ObjPath_GetPointWorldPosition(obj, p[i + 0x1b], &((GameObject*)entry)->anim.localPosX,
                                          &((GameObject*)entry)->anim.localPosY, &((GameObject*)entry)->anim.localPosZ, 0);
        }
        a += 4;
    }
    return 0;
}

int drshackle_getExtraSize(void)
{
    return 0x20;
}

int drshackle_getObjectTypeId(void)
{
    return 0x0;
}

void drshackle_free(int obj)
{
    objFreeObjectType(obj, DRSHACKLE_OBJGROUP);
}

void drshackle_render(GameObject* obj, u32 p2, u32 p3, u32 p4, u32 p5, char visible)
{
    u8* state = obj->extra;
    int i;
    if (((BitFlags8*)(state + 0x1a))->b0 == 0 && visible != 0)
    {
        objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, 1.0f);
        for (i = 0; i < ((DrshackleState*)state)->slotCount; i++)
        {
            int* entry = ((int**)state)[i];
            if (entry != 0)
            {
                ObjPath_GetPointWorldPosition(obj, state[i + 0x1b], &((GameObject*)entry)->anim.localPosX,
                                              &((GameObject*)entry)->anim.localPosY,
                                              &((GameObject*)entry)->anim.localPosZ, 0);
            }
        }
    }
}

void drshackle_hitDetect(unsigned long obj)
{
    char* state = ((GameObject*)obj)->extra;
    if (Sfx_IsPlayingFromObjectChannel((GameObject*)obj, 1) == 0 && ((BitFlags8*)(state + 0x1a))->b0 != 0)
    {
        Vec vec;
        int n;
        PSVECSubtract(&((GameObject*)obj)->anim.localPos, &((DrshackleState*)state)->savedPos, &vec);
        n = 0xc8 - (int)(30.0f * PSVECMag(&vec));
        if (randomGetRange(0, (n < 1) ? 1 : ((n > 0xc8) ? 0xc8 : n)) == 0)
        {
            Sfx_PlayFromObject((GameObject*)obj, SFXTRIG_dn_boar1_c_1b3);
        }
    }
}

void drshackle_update(GameObject* obj)
{
    char* state = obj->extra;
    DrshacklePlacement* placement = (DrshacklePlacement*)obj->anim.placementData;
    int count;
    int sub;
    int j;
    u32* list;
    if (placement->pathObjGroupBase != 0 && *(void**)state == 0)
    {
        list = (u32*)objGetAllOfType(DFROPENODE_OBJECT_GROUP, &count);
        while (count-- != 0)
        {
            sub = (int)((GameObject*)*list)->anim.placementData;
            for (j = 0; j < ((DrshackleState*)state)->slotCount; j++)
            {
                if (*(u8*)(sub + 0x18) == placement->pathObjGroupBase + j * 4)
                {
                    ((DrshackleState*)state)->pathSlots[j] = (GameObject*)*list;
                    (*gObjectTriggerInterface)->runSequence(0, ((DrshackleState*)state)->pathSlots[j], -1);
                }
            }
            list++;
        }
    }
    if (((BitFlags8*)(state + 0x1a))->b0 != 0)
    {
        ((BitFlags8*)(state + 0x1a))->b0 = (mainGetBit(placement->activeGameBit) == 0);
    }
}

void drshackle_init(GameObject* obj, char* arg)
{
    char* state = (obj)->extra;
    objAddObjectType((int)obj, DRSHACKLE_OBJGROUP);
    ((BitFlags8*)(state + 0x1a))->b0 = (mainGetBit(((DrshacklePlacement*)arg)->activeGameBit) == 0);
    ((DrshackleState*)state)->pathPointA = ((DrshacklePlacement*)arg)->startPathPoint % 2;
    (obj)->animEventCallback = drshackle_SeqFn;
    if (((DrshacklePlacement*)arg)->quarterTurns == 1)
    {
        ((DrshackleState*)state)->slotCount = 2;
        ((DrshackleState*)state)->pathPointB = 1 - ((DrshackleState*)state)->pathPointA;
    }
    else
    {
        ((DrshackleState*)state)->slotCount = 1;
    }
}

void drshackle_release(void)
{
}

void drshackle_initialise(void)
{
}

ObjectDescriptor12 gDrShackleObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_12_SLOTS,
    (ObjectDescriptorCallback)drshackle_initialise,
    (ObjectDescriptorCallback)drshackle_release,
    0,
    (ObjectDescriptorCallback)drshackle_init,
    (ObjectDescriptorCallback)drshackle_update,
    (ObjectDescriptorCallback)drshackle_hitDetect,
    (ObjectDescriptorCallback)drshackle_render,
    (ObjectDescriptorCallback)drshackle_free,
    (ObjectDescriptorCallback)drshackle_getObjectTypeId,
    (ObjectDescriptorExtraSizeCallback)drshackle_getExtraSize,
    (ObjectDescriptorCallback)drshackle_renderAtPathPoint,
    (ObjectDescriptorCallback)drshackle_getAttachSlot,
};
