/* SC_Cloudrun (DLL 0x01B9) - SC CloudRunner A level control. */
#include "dlls/object_descriptor.h"
#include "main/dll/SC/dll_01B9_sccloudrunnera.h"
#include "game/objects/object.h"
#include "main/audio/sfx_play_legacy_api.h"
#include "sys/objects/lifecycle.h"
#include "sys/objects.h"
#include "main/object_render.h"
#include "main/obj_link.h"
#include "main/obj_list.h"
#include "main/objseq.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/dll/dll_02B1_cmbsrc.h"
#include "main/dll/dll_0004_dummy04.h"
#include "game/objects/object_setup.h"

extern u8 lbl_803DB411;    /* trigger-interface update parameter */

/* Child object spawned in sc_cloudrunnera_update case 0, cached in childObjs[0]
 * and attached via ObjLink_AttachChild. */
#define SCCLOUDRUNNERA_CHILD_OBJ CMBSRC_SEQ_DEFAULT

typedef struct ScCloudrunneraPlacement
{
    ObjPlacement base;
    s16 animDataIndex; /* anim-data set selector (-1 = none); obj.userData1 = animDataIndex+1 */
    s16 gameBit; /* GameBit id -> seq->gameBit */
    u8 pad1C[0x24 - 0x1C];
    u8 posOffsetDecayFactor; /* 0x24: decay input; posOffsetDecay = base/(base + this) */
    u8 pad25[0x28 - 0x25];
} ScCloudrunneraPlacement;

/* Obj_AllocObjectSetup buffer filled in sc_cloudrunnera_update (case 0).
 * File-local layout recovered from constant-offset stores. */
int sc_cloudrunnera_getExtraSize(void) { return 0x140; }
int sc_cloudrunnera_getObjectTypeId(void) { return 0xb; }

void sc_cloudrunnera_free(GameObject* obj)
{
    void* inner = obj->extra;
    (*gObjectTriggerInterface)->freeState(inner);
    gTitleMenuControlInterfaceCopy->vtable->func05(obj, 0xffff, 0, 0, 0);
}

void sc_cloudrunnera_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible)
{
    s32 v = visible;
    if (v != 0) objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, 1.0f);
}

void sc_cloudrunnera_hitDetect(void)
{
}

void sc_cloudrunnera_update(int obj)
{
    int i;
    ObjSeqState* seq = ((GameObject*)obj)->extra;
    ScCloudrunneraPlacement* sub;
    int idx, count;

    sub = (ScCloudrunneraPlacement*)((GameObject*)obj)->anim.placementData;
    if (sub == NULL) return;
    if (sub->animDataIndex == -1) return;
    idx = (*gObjectTriggerInterface)->update((u8*)obj, (f32)(u32)lbl_803DB411);
    if (idx != 0 && ((GameObject*)obj)->seqIndex == -2)
    {
        int found;
        register s32 mark = *(s8*)&seq->slot;
        int* arr;
        int n;
        int markCopy;
        int matchCount;

        found = 0;
        arr = ObjList_GetObjects(&idx, &count);
        matchCount = 0;
        idx = 0;
        markCopy = mark;
        n = count;
        for (; idx < n; idx++)
        {
            int o = *arr;
            s16 t = ((GameObject*)o)->seqIndex;
            if (t == mark)
            {
                found = o;
            }
            if (t == -2 && ((GameObject*)o)->anim.classId == 0x10)
            {
                seq = *(ObjSeqState**)&((GameObject*)o)->extra;
                if (markCopy == (s8)seq->slot)
                {
                    matchCount++;
                }
            }
            arr++;
        }
        if (matchCount <= 1 && (u32)found != 0 && ((GameObject*)found)->seqIndex != -1)
        {
            ((GameObject*)found)->seqIndex = -1;
            (*gObjectTriggerInterface)->endSequence(markCopy);
        }
        ((GameObject*)obj)->seqIndex = -1;
    }

    for (i = 0; i < seq->eventCount; i++)
    {
        switch (seq->eventIds[i])
        {
        case 0:
            {
                CmbSrcMapData* setup;
                GameObject* newObj;
                if (((GameObject*)obj)->childObjs[0] != NULL)
                {
                    break;
                }
                if (Obj_IsLoadingLocked() == 0)
                {
                    break;
                }
                setup = (CmbSrcMapData*)Obj_AllocObjectSetup(CMBSRC_PLACEMENT_BYTES, SCCLOUDRUNNERA_CHILD_OBJ);
                setup->colorIndex = 0x9;
                setup->effectMode = 0;
                setup->pulseSubMode = 0;
                setup->radius = 1.0f;
                setup->colorDistance = 0xff;
                setup->effectDistance = 0xff;
                setup->pulseDistance = 0xff;
                setup->gameBit = -1;
                setup->base.color[0] = 2;
                setup->base.color[1] = 1;
                setup->base.color[2] = 0xff;
                setup->base.color[3] = 0xff;
                setup->flags = 1;
                setup->behaviorFlags = 0;
                newObj = Obj_SetupObject(&setup->base, 5, ((GameObject*)obj)->anim.mapEventSlot, -1,
                                         ((GameObject*)obj)->anim.parent);
                newObj->anim.flags = (s16)(newObj->anim.flags | OBJANIM_FLAG_HIDDEN);
                ObjLink_AttachChild((GameObject*)obj, newObj, 0);
                Sfx_PlayFromObject(obj, SFXTRIG_en_cvdrip1c);
                break;
            }
        case 1:
            {
                if (((GameObject*)obj)->childObjs[0] != NULL)
                {
                    cmbsrc_setExternalActive(((GameObject*)obj)->childObjs[0], 0);
                }
                break;
            }
        case 2:
            {
                GameObject* innerSlot = ((GameObject*)obj)->childObjs[0];
                if (innerSlot != NULL)
                {
                    ObjLink_DetachChild((GameObject*)obj, innerSlot);
                    Obj_FreeObject(innerSlot);
                }
                break;
            }
        }
    }
    {
        GameObject* t = ((GameObject*)obj)->childObjs[0];
        if (t != NULL)
        {
            t->anim.rotZ = ((GameObject*)obj)->anim.rotZ;
            ((GameObject*)((GameObject*)obj)->childObjs[0])->anim.rotY = (s16)(((GameObject*)obj)->anim.rotY + 0xe38);
            ((GameObject*)((GameObject*)obj)->childObjs[0])->anim.rotX = (s16)(((GameObject*)obj)->anim.rotX + -0x8000);
        }
    }
}

void sc_cloudrunnera_init(GameObject *obj, int def)
{
    ObjSeqState* seq;
    f32 base;
    s32 objF4;
    ScCloudrunneraPlacement* place = (ScCloudrunneraPlacement*)def;

    objSetSlot(obj, 0x64);
    seq = (obj)->extra;
    seq->gameBit = place->gameBit;
    seq->flags = -1;
    base = 1.0f;
    seq->posOffsetDecay = base / (base + (f32)(u32)place->posOffsetDecayFactor);
    seq->curveId = -1;
    (obj)->userData2 = 0;

    objF4 = (obj)->userData1;
    if (objF4 == 0 && place->animDataIndex != 1)
    {
        (*gObjectTriggerInterface)
            ->loadAnimData((u8*)seq, (u8*)def);
        (obj)->userData1 = place->animDataIndex + 1;
    }
    else if (objF4 != 0 && place->animDataIndex != objF4 - 1)
    {
        (*gObjectTriggerInterface)->freeState((u8*)seq);
        if (place->animDataIndex != -1)
        {
            (*gObjectTriggerInterface)
                ->loadAnimData((u8*)seq, (u8*)def);
        }
        (obj)->userData1 = place->animDataIndex + 1;
    }
    if ((obj)->anim.modelState != NULL)
    {
        (obj)->anim.modelState->shadowTintA = 0x64;
        (obj)->anim.modelState->shadowTintB = 0x96;
    }
}

void sc_cloudrunnera_release(void)
{
}

void sc_cloudrunnera_initialise(void)
{
}

ObjectDescriptor gSC_CloudrunnerAObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)sc_cloudrunnera_initialise,
    (ObjectDescriptorCallback)sc_cloudrunnera_release,
    0,
    (ObjectDescriptorCallback)sc_cloudrunnera_init,
    (ObjectDescriptorCallback)sc_cloudrunnera_update,
    (ObjectDescriptorCallback)sc_cloudrunnera_hitDetect,
    (ObjectDescriptorCallback)sc_cloudrunnera_render,
    (ObjectDescriptorCallback)sc_cloudrunnera_free,
    (ObjectDescriptorCallback)sc_cloudrunnera_getObjectTypeId,
    sc_cloudrunnera_getExtraSize,
};
