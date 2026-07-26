/*
 * dim2roofrub (DLL 0xC7) - DIM2 roof-rub object.
 * The dim2roofrub object is a GC-map interactive surface that triggers
 * animation sequences and particle effects when the player walks over it.
 */
#include "main/dll/partfx_interface.h"
#include "main/audio/sfx_stop_channel_api.h"
#include "main/shader_api.h"
#include "dolphin/mtx/mtx_legacy.h"
#include "main/objfx.h"
#include "main/objprint_render_api.h"
#include "main/dll/DIM/dll_00C7_dim2roofrub_api.h"
#include "main/frame_timing.h"
#include "main/object_render.h"

#include "game/objects/object.h"
#include "sys/objects.h"
#include "main/dll_000A_expgfx.h"
#include "main/camera_interface.h"
#include "main/objseq.h"
#include "main/obj_list.h"
#include "main/dll/dll_0004_dummy04.h"

#define DIM2ROOFRUB_OBJFLAG_RENDERED 0x800

typedef struct DIM2RoofRubPlacement
{
    u8 pad0[0x8 - 0x0];
    f32 posX;
    f32 posY;
    f32 posZ;
    s32 mapId;         /* 0x14: ObjPlacement-head map id (after posX/Y/Z) */
    s16 animDataIndex; /* 0x18 anim-data set selector (-1 = none); obj.userData1 = animDataIndex+1 */
    s16 gameBit; /* 0x1A: copied into ObjSeqState.gameBit at init */
    s16 unk1C;
    s16 unk1E;
    s16 unk20;
    u8 pad22[0x24 - 0x22];
    u8 dampingParam; /* 0x24: divisor source, dampingFactor = 1/(1+dampingParam) */
    u8 pad25[0x2C - 0x25];
    s16 unk2C;
    u8 pad2E[0x30 - 0x2E];
} DIM2RoofRubPlacement;

typedef struct Dim2FxRow
{
    f32 x;
    f32 y;
    f32 z;
    f32 w;
    u8 b1;
    u8 b2;
    u8 pad[2];
} Dim2FxRow;
typedef struct Dim2FxVec
{
    u8 pad[8];
    f32 fade;
    f32 x;
    f32 y;
    f32 z;
} Dim2FxVec;
typedef struct Dim2PartVec
{
    u8 pad[0xc];
    f32 x;
    f32 y;
    f32 z;
} Dim2PartVec;

#define DIM2ROOFRUB_SEQID_SLIDE 0xa8
#define DIM2ROOFRUB_SEQID_TREAD 0x451
#define DIM2ROOFRUB_EVENT_TOGGLE_LIGHT 1
#define DIM2ROOFRUB_EVENT_TOGGLE_HEAVY 2
#define DIM2ROOFRUB_EVENT_TOGGLE_FX    3
#define DIM2ROOFRUB_EVENT_SPAWN_DUST   4
/* dust particle spawned 3x on the SPAWN_DUST anim event */
#define DIM2ROOFRUB_PARTFX 2046
extern u32 lbl_80320768[];

void dim2roofrub_spawnEffects(GameObject* obj)
{
    Dim2FxVec v;
    int flags;

    if ((obj->userData2 & 4) != 0)
    {
        u8 i = 0;
        f32 scale = (0.64f);
        Dim2FxRow* tbl = (Dim2FxRow*)lbl_80320768;
        for (; i < 10; i++)
        {
            f32 f = obj->anim.rootMotionScale;
            Dim2FxRow* row = &tbl[i];
            v.x = scale * (f * row->x);
            v.y = scale * (f * row->y);
            v.z = scale * (f * row->z);
            objfx_spawnMaskedHitEffect(obj, f * row->w, 3, row->b1, row->b2, &v);
        }
    }
    v.fade = (-1.0f);
    flags = obj->userData2;
    if ((flags & 1) != 0)
    {
        int count;
        if ((flags & 2) != 0)
        {
            count = 6;
        }
        else
        {
            count = 3;
        }
        v.x = (0.64f) * ((-0.8230000138282776f) * obj->anim.rootMotionScale);
        v.y = (0.64f) * ((-0.08399999886751175f) * obj->anim.rootMotionScale);
        v.z = (0.64f) * ((-2.5999999046325684f) * obj->anim.rootMotionScale);
        objfx_spawnLightPulse(obj, (0.02500000037252903f) * obj->anim.rootMotionScale,
                              1, 0, count, (0.699999988079071f), &v);
        v.x = (0.0f);
        v.y = (0.64f) * ((0.20900000631809235f) * obj->anim.rootMotionScale);
        v.z = (0.64f) * ((-3.5999999046325684f) * obj->anim.rootMotionScale);
        objfx_spawnLightPulse(obj, (0.02500000037252903f) * obj->anim.rootMotionScale,
                              1, 0, count, (0.5f), &v);
        v.x = (0.64f) * ((0.8230000138282776f) * obj->anim.rootMotionScale);
        v.y = (0.64f) * ((-0.08399999886751175f) * obj->anim.rootMotionScale);
        v.z = (0.64f) * ((-2.5999999046325684f) * obj->anim.rootMotionScale);
        objfx_spawnLightPulse(obj, (0.02500000037252903f) * obj->anim.rootMotionScale,
                              1, 0, count, (0.699999988079071f), &v);
    }
    if (obj->anim.seqId == DIM2ROOFRUB_SEQID_SLIDE)
    {
        objfx_spawnDirectionalBurst(obj, 7, (1.0f), 5, 1, 10, (6.0f), NULL, 0x20000000);
    }
    else if (obj->anim.seqId == DIM2ROOFRUB_SEQID_TREAD)
    {
        ObjModel* model = Obj_GetActiveModel(obj);
        *(u8*)((char*)*(int**)((char*)model + 0x34) + 8) = 2;
        if ((obj->objectFlags & DIM2ROOFRUB_OBJFLAG_RENDERED) != 0)
        {
            objfx_spawnDirectionalBurst(obj, 5, (1.0f), 2, 1, 20, (2.5f), NULL, 0);
        }
    }
}

int dim2roofrub_getExtraSize(void)
{
    return 0x140;
}

u32 lbl_80320768[] = {
    0x00000000, 0x3FD5A1CB, 0xC0253F7D, 0x3C23D70A, 0x06100000, 0x402F3B64, 0x3F4B020C, 0xBFFA1CAC, 0x3C23D70A,
    0x09200000, 0x402EB852, 0x3F476C8B, 0xBF73B646, 0x3C23D70A, 0x07200000, 0x4032E148, 0xBF795810, 0xBFF8F5C3,
    0x3C23D70A, 0x09200000, 0x4033F7CF, 0xBF810625, 0xBF747AE1, 0x3C23D70A, 0x07200000, 0xC02F3B64, 0x3F4B020C,
    0xBFFC28F6, 0x3C23D70A, 0x09200000, 0xC02EB852, 0x3F476C8B, 0xBF73B646, 0x3C23D70A, 0x07200000, 0xC032E148,
    0xBF795810, 0xBFFC49BA, 0x3C23D70A, 0x09200000, 0xC033F7CF, 0xBF810625, 0xBF747AE1, 0x3C23D70A, 0x07200000,
    0x00000000, 0x3ECF5C29, 0x403CED91, 0x3C23D70A, 0x08400000,
};

ObjectDescriptor gDIM2RoofRubObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    0,
    0,
    0,
    (ObjectDescriptorCallback)dim2roofrub_init,
    (ObjectDescriptorCallback)dim2roofrub_update,
    0,
    (ObjectDescriptorCallback)dim2roofrub_render,
    (ObjectDescriptorCallback)dim2roofrub_free,
    0,
    dim2roofrub_getExtraSize,
};
void dim2roofrub_free(GameObject* obj)
{
    (*gObjectTriggerInterface)->freeState(obj->extra);
    gTitleMenuControlInterfaceCopy->vtable->func05(obj, 0xffff, 0, 0, 0);
    Sfx_StopObjectChannel((int)obj, 0x7f);
}

void dim2roofrub_render(GameObject* obj, int p2, int p3, int p4, int p5)
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

    dim2roofrub_spawnEffects(obj);
    if ((((ObjSeqState*)obj->extra)->stateFlags & 4) != 0)
    {
        DIM2RoofRubPlacement* prm;
        GameObject* cam;
        Obj_BuildWorldTransformMatrix(obj, mWorld, 0);
        prm = *(DIM2RoofRubPlacement**)&obj->anim.placementData;
        PSMTXTrans(mTransPlayer, -(prm->posX - playerMapOffsetX),
                   -prm->posY, -(prm->posZ - playerMapOffsetZ));
        PSMTXConcat(mTransPlayer, mWorld, mWorldCombined);
        cam = (GameObject*)(*gCameraInterface)->getCamera();
        cam->anim.rotY += 0x8000;
        cam->anim.rootMotionScale = (1.0f);
        Obj_BuildWorldTransformMatrix(cam, mCam, 0);
        cam->anim.rotY += 0x8000;
        cam->anim.rootMotionScale = (0.0f);
        PSMTXTrans(mTransNeg, -mCam[3], -mCam[7], -mCam[11]);
        PSMTXRotRad(mRotY, 'y', (3.1415927410125732f));
        PSMTXRotRad(mRotZ, 'z', (3.1415927410125732f));
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

void dim2roofrub_update(GameObject* obj)
{
    ObjSeqState* seq = obj->extra;
    DIM2RoofRubPlacement* params = *(DIM2RoofRubPlacement**)&obj->anim.placementData;

    if (params != NULL && params->animDataIndex != -1)
    {
        Dim2PartVec v;
        int count;
        int res;
        for (res = 0; res < seq->eventCount; res++)
        {
            int b = seq->eventIds[res];
            switch (b)
            {
            case DIM2ROOFRUB_EVENT_TOGGLE_LIGHT:
                obj->userData2 ^= 1;
                break;
            case DIM2ROOFRUB_EVENT_TOGGLE_HEAVY:
                obj->userData2 ^= 2;
                break;
            case DIM2ROOFRUB_EVENT_TOGGLE_FX:
                obj->userData2 ^= 4;
                break;
            case DIM2ROOFRUB_EVENT_SPAWN_DUST:
            {
                int k;
                v.x = obj->anim.localPosX;
                v.y = obj->anim.localPosY;
                v.z = obj->anim.localPosZ;
                for (k = 3; k != 0; k--)
                {
                    (*gPartfxInterface)->spawnObject(obj, DIM2ROOFRUB_PARTFX, &v, 0x200001, -1, NULL);
                }
                break;
            }
            }
        }
        res = (*gObjectTriggerInterface)->update((u8*)obj, timeDelta);
        if (res != 0 && obj->seqIndex == -2)
        {
            int slot8 = *(s8*)&seq->slot;
            int* list;
            int slot;
            int cnt;
            GameObject* match = NULL;
            list = ObjList_GetObjects(&res, &count);
            res = cnt = 0;
            slot = slot8;
            for (; res < count; res++)
            {
                GameObject* other = (GameObject*)*list;
                if (other->seqIndex == slot8)
                {
                    match = (GameObject*)*list;
                }
                if (other->seqIndex == -2 && other->anim.classId == 0x10)
                {
                    ObjSeqState* otherSeq = *(ObjSeqState**)&other->extra;
                    if (slot == (s8)otherSeq->slot)
                    {
                        cnt++;
                    }
                }
                list++;
            }
            if (cnt <= 1 && match != NULL && match->seqIndex != -1)
            {
                match->seqIndex = -1;
                (*gObjectTriggerInterface)->endSequence(slot);
            }
            obj->seqIndex = -1;
        }
    }
}

void dim2roofrub_init(GameObject* obj, DIM2RoofRubPlacement* params)
{
    ObjSeqState* seq;
    int f4;
    objSetSlot(obj, 0x64);
    seq = obj->extra;
    seq->gameBit = params->gameBit;
    seq->flags = -1;
    {
        f32 d = (1.0f);
        seq->posOffsetDecay =
            d / (d + (f32)(u32)params->dampingParam);
    }
    seq->curveId = -1;
    seq->animEntries = NULL;
    seq->cmds = NULL;
    seq->baseRotX = 0;
    seq->baseRotY = 0;
    obj->userData2 = 0;
    f4 = obj->userData1;
    if (f4 == 0 && params->animDataIndex != 1)
    {
        (*gObjectTriggerInterface)->loadAnimData((u8*)seq, (u8*)params);
        obj->userData1 = params->animDataIndex + 1;
    }
    else if (f4 != 0 && params->animDataIndex != f4 - 1)
    {
        (*gObjectTriggerInterface)->freeState((u8*)seq);
        if (params->animDataIndex != -1)
        {
            (*gObjectTriggerInterface)->loadAnimData((u8*)seq, (u8*)params);
        }
        obj->userData1 = params->animDataIndex + 1;
    }
    {
        ObjModelState* modelState = obj->anim.modelState;
        if (modelState != NULL)
        {
            modelState->shadowTintA = 0x64;
            obj->anim.modelState->shadowTintB = 0x96;
        }
    }
}
