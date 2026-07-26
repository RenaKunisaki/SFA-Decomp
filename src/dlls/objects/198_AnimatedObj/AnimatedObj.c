#include "main/object_render.h"
#include "dolphin/mtx/mtx_legacy.h"
#include "main/shader_api.h"
#include "main/objprint_render_api.h"
#include "main/dll/dll_00C6_animatedobj_api.h"
#include "main/frame_timing.h"
#include "game/objects/object.h"
#include "main/obj_list.h"
#include "main/obj_link.h"
#include "sys/objects/lifecycle.h"
#include "sys/objects.h"
#include "main/audio/sfx_looped_object_api.h"
#include "main/audio/sfx_stop_channel_api.h"
#include "main/camera_interface.h"
#include "main/maketex_sequence_api.h"
#include "main/objseq.h"
#include "main/dll/dll_0004_dummy04.h"

#define ANIMATEDOBJ_OBJFLAG_UPDATE_DISABLED 0x8000
/* Child object spawned and attached on sequence event 0xa. */
#define ANIMATEDOBJ_CHILD_OBJ 0x69
#define ANIMATEDOBJ_KRYSTAL_OBJ 0x774

typedef struct AnimatedObjPlacement
{
    u8 pad0[0x8 - 0x0];
    f32 posX;
    f32 posY;
    f32 posZ;
    s32 mapId; /* 0x14: ObjPlacement map id */
    s16 loadKey;
    s16 gameBit; /* 0x1A: copied into ObjSeqState.gameBit at init */
    s16 unk1C;
    s16 unk1E;
    s16 unk20;
    u8 pad22[0x24 - 0x22];
    u8 posOffsetDecayTime;
    u8 pad25[0x2C - 0x25];
    s16 unk2C;
    u8 pad2E[0x30 - 0x2E];
} AnimatedObjPlacement;

int animatedobj_getExtraSize(void)
{
    return 0x140;
}

ObjectDescriptor gAnimatedObjDescriptor = {
    0, 0, 0, OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    0,
    0,
    0,
    (ObjectDescriptorCallback)animatedobj_init,
    (ObjectDescriptorCallback)animatedobj_update,
    0,
    (ObjectDescriptorCallback)animatedobj_render,
    (ObjectDescriptorCallback)animatedobj_free,
    0,
    animatedobj_getExtraSize,
};

void animatedobj_free(GameObject* obj, int seqFlag)
{
    (*gObjectTriggerInterface)
        ->freeState(obj->extra);
    gTitleMenuControlInterfaceCopy->vtable->func05(obj, 0xffff, 0, 0, 0);
    Sfx_RemoveLoopedObjectSoundForObject((u32)obj);
    Sfx_StopObjectChannel((int)obj, 0x7f);
    if (obj->anim.seqId == ANIMATEDOBJ_KRYSTAL_OBJ && obj->childCount != 0)
    {
        Obj_FreeObject(obj->childObjs[0]);
        ObjLink_DetachChild(obj, obj->childObjs[0]);
    }
    if (seqFlag != 0)
    {
        clearCurSeqNo();
    }
}

void animatedobj_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible)
{
    f32 mWorld[12];
    f32 mTransPlayer[12];
    f32 mWorldCombined[12];
    f32 mTransNeg[12];
    f32 mRotY[12];
    f32 mRotZ[12];
    f32 mTransPos[12];
    f32 mCam[12];
    f32 mA[12];
    f32 mB[12];
    f32 mC[12];
    f32 mD[12];
    f32 mFinal[12];

    ObjSeqState* seq = obj->extra;
    if ((seq->stateFlags & 4) != 0)
    {
        AnimatedObjPlacement* params;
        GameObject* camera;
        Obj_BuildWorldTransformMatrix(obj, mWorld, 0);
        params = (AnimatedObjPlacement*)obj->anim.placementData;
        PSMTXTrans(mTransPlayer, -(params->posX - playerMapOffsetX),
                   -params->posY,
                   -(params->posZ - playerMapOffsetZ));
        PSMTXConcat(mTransPlayer, mWorld, mWorldCombined);
        camera = (GameObject*)(*gCameraInterface)->getCamera();
        camera->anim.rotY += 0x8000;
        camera->anim.rootMotionScale = 1.0f;
        Obj_BuildWorldTransformMatrix(camera, mCam, 0);
        camera->anim.rotY += 0x8000;
        camera->anim.rootMotionScale = 0.0f;
        PSMTXTrans(mTransNeg, -mCam[3], -mCam[7], -mCam[11]);
        PSMTXRotRad(mRotY, 'y', 3.1415927f);
        PSMTXRotRad(mRotZ, 'z', 3.1415927f);
        PSMTXTrans(mTransPos, mCam[3], mCam[7], mCam[11]);
        PSMTXConcat(mTransNeg, mCam, mA);
        PSMTXConcat(mRotY, mA, mB);
        PSMTXConcat(mRotZ, mB, mC);
        PSMTXConcat(mTransPos, mC, mD);
        PSMTXConcat(mD, mWorldCombined, mFinal);
        objSetMtxFn_800412d4((u32)mFinal);
        objRenderModel(obj);
    }
    else
    {
        objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, 1.0f);
    }
}


void animatedobj_update(GameObject* obj)
{
    AnimatedObjPlacement* params;
    int res;
    int count;
    int objectSeqIndex;
    GameObject* match;
    ObjSeqState* seq;
    GameObject** list;
    int sequenceSlot;
    int slotObjectCount;
    GameObject* other;
    int i;
    GameObject* child;

    seq = obj->extra;
    params = (AnimatedObjPlacement*)obj->anim.placementData;
    if (params != NULL && params->loadKey != -1)
    {
        res = (*gObjectTriggerInterface)->update((u8*)obj, timeDelta);
        if (res != 0 && obj->seqIndex == -2)
        {
            objectSeqIndex = seq->slot;
            match = NULL;
            list = ObjList_GetObjects(&res, &count);
            slotObjectCount = 0;
            res = 0;
            sequenceSlot = (s8)objectSeqIndex;
            while (res < count)
            {
                other = *list;
                if (other->seqIndex == objectSeqIndex)
                {
                    match = *list;
                }
                if (other->seqIndex == -2 && other->anim.classId == 0x10)
                {
                    seq = other->extra;
                    if (sequenceSlot == seq->slot)
                    {
                        slotObjectCount++;
                    }
                }
                list++;
                res++;
            }
            if (slotObjectCount <= 1 && match != NULL && match->seqIndex != -1)
            {
                match->seqIndex = -1;
                (*gObjectTriggerInterface)->endSequence(sequenceSlot);
            }
            obj->seqIndex = -1;
            obj->objectFlags |= ANIMATEDOBJ_OBJFLAG_UPDATE_DISABLED;
            obj->anim.flags |= OBJANIM_FLAG_HIDDEN;
        }
        switch (obj->anim.seqId)
        {
        case ANIMATEDOBJ_KRYSTAL_OBJ:
        {
            for (i = 0; i < seq->eventCount; i++)
            {
                switch (seq->eventIds[i])
                {
                case 0xa:
                    if ((u8)Obj_IsLoadingLocked() != 0)
                    {
                        child = Obj_SetupObject(
                            Obj_AllocObjectSetup(0x18, ANIMATEDOBJ_CHILD_OBJ),
                            4, -1, -1, 0);
                        ObjLink_AttachChild(obj, child, 0);
                        ObjAnim_SetCurrentMove((int)child, 0, 0.0f, 0);
                        ObjAnim_AdvanceCurrentMove(
                            (int)child, 1.0f, timeDelta, NULL);
                    }
                    break;
                case 0xb:
                    if (obj->childCount != 0)
                    {
                        Obj_FreeObject(obj->childObjs[0]);
                        ObjLink_DetachChild(obj, obj->childObjs[0]);
                    }
                    break;
                }
            }
            break;
        }
        }
    }
}


void animatedobj_init(GameObject* obj, AnimatedObjPlacement* params)
{
    ObjSeqState* seq;
    int f4;
    objSetSlot(obj, 0x64);
    seq = obj->extra;
    seq->gameBit = params->gameBit;
    seq->flags = -1;
    {
        f32 d = 1.0f;
        seq->posOffsetDecay = d / (d + (f32)(u32)params->posOffsetDecayTime);
    }
    seq->curveId = -1;
    seq->animEntries = NULL;
    seq->cmds = NULL;
    seq->baseRotX = 0;
    seq->baseRotY = 0;
    seq->freeCallback = NULL;
    f4 = obj->userData1;
    if (f4 == 0 && params->loadKey != 1)
    {
        (*gObjectTriggerInterface)
            ->loadAnimData((u8*)seq, (u8*)params);
        obj->userData1 = params->loadKey + 1;
    }
    else if (f4 != 0 && params->loadKey != f4 - 1)
    {
        (*gObjectTriggerInterface)->freeState((u8*)seq);
        if (params->loadKey != -1)
        {
            (*gObjectTriggerInterface)
                ->loadAnimData((u8*)seq, (u8*)params);
        }
        obj->userData1 = params->loadKey + 1;
    }
    {
        ObjModelState* modelState = obj->anim.modelState;
        if (modelState != NULL)
        {
            modelState->shadowTintA = 0x64;
            obj->anim.modelState->shadowTintB = 0x96;
        }
    }
    Obj_SetModelRenderOpAlpha(obj, 0xff);
}
