/*
 * SC_Cloudrun (DLL 0x01B9) drives SC CloudRunner A sequence events and
 * manages the attached CmbSrc effect source.
 */

#include "dlls/objects/441_SC_Cloudrun.h"

#include "main/audio/sfx_trigger_ids.h"
#include "main/dll/dll_0004_dummy04.h"
#include "main/dll/dll_02B1_cmbsrc.h"
#include "main/obj_list.h"
#include "main/object_render.h"
#include "sys/objects.h"
#include "sys/objects/lifecycle.h"

#define SC_CLOUDRUNNER_A_EVENT_CREATE_CHILD      0
#define SC_CLOUDRUNNER_A_EVENT_DEACTIVATE_CHILD  1
#define SC_CLOUDRUNNER_A_EVENT_REMOVE_CHILD      2
#define SC_CLOUDRUNNER_A_CHILD_OBJECT_ID         CMBSRC_SEQ_DEFAULT
#define SC_CLOUDRUNNER_A_OBJECT_TYPE_ID          0xB
#define SC_CLOUDRUNNER_A_SEQUENCE_CLASS_ID       0x10
#define SC_CLOUDRUNNER_A_OBJECT_SLOT             0x64
#define SC_CLOUDRUNNER_A_CHILD_SETUP_FLAGS       5
#define SC_CLOUDRUNNER_A_SEQUENCE_PENDING        -2
#define SC_CLOUDRUNNER_A_SEQUENCE_NONE           -1
#define SC_CLOUDRUNNER_A_SEQUENCE_FLAGS          -1
#define SC_CLOUDRUNNER_A_CURVE_NONE              -1
#define SC_CLOUDRUNNER_A_ANIM_DATA_NONE          -1
#define SC_CLOUDRUNNER_A_DEFAULT_ANIM_DATA_INDEX 1
#define SC_CLOUDRUNNER_A_NO_OBJECT_INDEX         -1
#define SC_CLOUDRUNNER_A_CHILD_YAW_OFFSET        0xE38
#define SC_CLOUDRUNNER_A_CHILD_PITCH_OFFSET      -0x8000
#define SC_CLOUDRUNNER_A_SHADOW_TINT_A           0x64
#define SC_CLOUDRUNNER_A_SHADOW_TINT_B           0x96

extern u8 lbl_803DB411;

int sc_cloudrunnera_getExtraSize(void) {
    return sizeof(ScCloudrunnerAState);
}

int sc_cloudrunnera_getObjectTypeId(void) {
    return SC_CLOUDRUNNER_A_OBJECT_TYPE_ID;
}

void sc_cloudrunnera_free(GameObject* obj) {
    ObjSeqState* sequence = obj->extra;

    (*gObjectTriggerInterface)->freeState((u8*)sequence);
    gTitleMenuControlInterfaceCopy->vtable->func05(obj, 0xffff, 0, 0, 0);
}

void sc_cloudrunnera_render(GameObject* obj, int renderArg2, int renderArg3, int renderArg4, int renderArg5,
                            s8 visible) {
    s32 visibleValue = visible;

    if (visibleValue != 0) {
        objRenderModelAndHitVolumes(obj, renderArg2, renderArg3, renderArg4, renderArg5, 1.0f);
    }
}

void sc_cloudrunnera_hitDetect(void) {
}

void sc_cloudrunnera_update(int obj) {
    int eventIndex;
    ObjSeqState* sequence = ((GameObject*)obj)->extra;
    ScCloudrunnerAPlacement* placement;
    int objectIndex, objectCount;

    placement = (ScCloudrunnerAPlacement*)((GameObject*)obj)->anim.placementData;
    if (placement == NULL) {
        return;
    }
    if (placement->animDataIndex == SC_CLOUDRUNNER_A_ANIM_DATA_NONE) {
        return;
    }
    objectIndex = (*gObjectTriggerInterface)->update((u8*)obj, (f32)(u32)lbl_803DB411);
    if (objectIndex != 0 && ((GameObject*)obj)->seqIndex == SC_CLOUDRUNNER_A_SEQUENCE_PENDING) {
        int sequenceOwner;
        register s32 slot = *(s8*)&sequence->slot;
        int* objects;
        int participantLimit;
        int sequenceSlotCopy;
        int participantCount;

        sequenceOwner = 0;
        objects = ObjList_GetObjects(&objectIndex, &objectCount);
        participantCount = 0;
        objectIndex = 0;
        sequenceSlotCopy = slot;
        participantLimit = objectCount;
        for (; objectIndex < participantLimit; objectIndex++) {
            int otherObject = *objects;
            s16 sequenceIndex = ((GameObject*)otherObject)->seqIndex;

            if (sequenceIndex == slot) {
                sequenceOwner = otherObject;
            }
            if (sequenceIndex == SC_CLOUDRUNNER_A_SEQUENCE_PENDING &&
                ((GameObject*)otherObject)->anim.classId == SC_CLOUDRUNNER_A_SEQUENCE_CLASS_ID) {
                sequence = *(ObjSeqState**)&((GameObject*)otherObject)->extra;
                if (sequenceSlotCopy == sequence->slot) {
                    participantCount++;
                }
            }
            objects++;
        }
        if (participantCount <= 1 && (u32)sequenceOwner != 0 &&
            ((GameObject*)sequenceOwner)->seqIndex != SC_CLOUDRUNNER_A_SEQUENCE_NONE) {
            ((GameObject*)sequenceOwner)->seqIndex = SC_CLOUDRUNNER_A_SEQUENCE_NONE;
            (*gObjectTriggerInterface)->endSequence(sequenceSlotCopy);
        }
        ((GameObject*)obj)->seqIndex = SC_CLOUDRUNNER_A_SEQUENCE_NONE;
    }

    for (eventIndex = 0; eventIndex < sequence->eventCount; eventIndex++) {
        switch (sequence->eventIds[eventIndex]) {
        case SC_CLOUDRUNNER_A_EVENT_CREATE_CHILD: {
            CmbSrcMapData* setup;
            GameObject* child;

            if (((GameObject*)obj)->childObjs[0] != NULL) {
                break;
            }
            if (Obj_IsLoadingLocked() == 0) {
                break;
            }
            setup = (CmbSrcMapData*)Obj_AllocObjectSetup(CMBSRC_PLACEMENT_BYTES, SC_CLOUDRUNNER_A_CHILD_OBJECT_ID);
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
            setup->flags = CMBSRC_MAP_START_ACTIVE;
            setup->behaviorFlags = 0;
            child =
                Obj_SetupObject(&setup->base, SC_CLOUDRUNNER_A_CHILD_SETUP_FLAGS, ((GameObject*)obj)->anim.mapEventSlot,
                                SC_CLOUDRUNNER_A_NO_OBJECT_INDEX, ((GameObject*)obj)->anim.parent);
            child->anim.flags = (s16)(child->anim.flags | OBJANIM_FLAG_HIDDEN);
            ObjLink_AttachChild((GameObject*)obj, child, 0);
            Sfx_PlayFromObject(obj, SFXTRIG_en_cvdrip1c);
            break;
        }
        case SC_CLOUDRUNNER_A_EVENT_DEACTIVATE_CHILD: {
            if (((GameObject*)obj)->childObjs[0] != NULL) {
                cmbsrc_setExternalActive((GameObject*)((GameObject*)obj)->childObjs[0], 0);
            }
            break;
        }
        case SC_CLOUDRUNNER_A_EVENT_REMOVE_CHILD: {
            GameObject* child = ((GameObject*)obj)->childObjs[0];

            if (child != NULL) {
                ObjLink_DetachChild((GameObject*)obj, child);
                Obj_FreeObject(child);
            }
            break;
        }
        }
    }
    {
        GameObject* child = ((GameObject*)obj)->childObjs[0];

        if (child != NULL) {
            child->anim.rotZ = ((GameObject*)obj)->anim.rotZ;
            ((GameObject*)((GameObject*)obj)->childObjs[0])->anim.rotY =
                (s16)(((GameObject*)obj)->anim.rotY + SC_CLOUDRUNNER_A_CHILD_YAW_OFFSET);
            ((GameObject*)((GameObject*)obj)->childObjs[0])->anim.rotX =
                (s16)(((GameObject*)obj)->anim.rotX + SC_CLOUDRUNNER_A_CHILD_PITCH_OFFSET);
        }
    }
}

void sc_cloudrunnera_init(GameObject* obj, const ScCloudrunnerAPlacement* placement) {
    ObjSeqState* sequence;
    f32 one;
    s32 cachedAnimDataIndexPlusOne;

    objSetSlot(obj, SC_CLOUDRUNNER_A_OBJECT_SLOT);
    sequence = obj->extra;
    sequence->gameBit = placement->sequenceGameBit;
    sequence->flags = SC_CLOUDRUNNER_A_SEQUENCE_FLAGS;
    one = 1.0f;
    sequence->posOffsetDecay = one / (one + (f32)(u32)placement->positionDamping);
    sequence->curveId = SC_CLOUDRUNNER_A_CURVE_NONE;
    obj->userData2 = 0;

    cachedAnimDataIndexPlusOne = obj->userData1;
    if (cachedAnimDataIndexPlusOne == 0 && placement->animDataIndex != SC_CLOUDRUNNER_A_DEFAULT_ANIM_DATA_INDEX) {
        (*gObjectTriggerInterface)->loadAnimData((u8*)sequence, (u8*)placement);
        obj->userData1 = placement->animDataIndex + 1;
    } else if (cachedAnimDataIndexPlusOne != 0 && placement->animDataIndex != cachedAnimDataIndexPlusOne - 1) {
        (*gObjectTriggerInterface)->freeState((u8*)sequence);
        if (placement->animDataIndex != SC_CLOUDRUNNER_A_ANIM_DATA_NONE) {
            (*gObjectTriggerInterface)->loadAnimData((u8*)sequence, (u8*)placement);
        }
        obj->userData1 = placement->animDataIndex + 1;
    }
    if (obj->anim.modelState != NULL) {
        obj->anim.modelState->shadowTintA = SC_CLOUDRUNNER_A_SHADOW_TINT_A;
        obj->anim.modelState->shadowTintB = SC_CLOUDRUNNER_A_SHADOW_TINT_B;
    }
}

void sc_cloudrunnera_release(void) {
}

void sc_cloudrunnera_initialise(void) {
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
