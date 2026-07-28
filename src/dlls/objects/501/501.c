/*
 * DLL 0x1F5 (slot 501) provides shared trigger-sequence behavior for several
 * object definitions. The retail source basename is not yet recovered, so its
 * symbols remain in the numbered DLL namespace.
 */
#include "dlls/objects/501.h"

#include "main/dll/dll_0004_dummy04.h"
#include "main/dll/objfx.h"
#include "main/model_light.h"
#include "main/obj_list.h"
#include "main/object_render.h"

#define DLL501_OBJECT_TYPE_ID           0xB
#define DLL501_FIRE_SEQ_ID              0x171
#define DLL501_SEQ_INDEX_PENDING        -2
#define DLL501_SEQUENCE_OBJECT_CLASS_ID 0x10
#define DLL501_TRAIL_MODE               4
#define DLL501_TRAIL_EFFECT_ID          389
#define DLL501_TRAIL_EFFECT_PARAM       5

extern u8 lbl_803DB411;

typedef struct Dll501TrackedState {
    f32 value;
    u8 flag;
    u8 unknown05[3];
} Dll501TrackedState;

STATIC_ASSERT(offsetof(Dll501TrackedState, value) == 0x00);
STATIC_ASSERT(offsetof(Dll501TrackedState, flag) == 0x04);
STATIC_ASSERT(offsetof(Dll501TrackedState, unknown05) == 0x05);
STATIC_ASSERT(sizeof(Dll501TrackedState) == 0x08);

Dll501TrackedState lbl_803DDC50;

static void dll501_resetTrackedState(void) {
    lbl_803DDC50.value = 0.0f;
    lbl_803DDC50.flag = 0;
}

int dll501_getExtraSize(void) {
    return sizeof(Dll501State);
}

int dll501_getObjectTypeId(void) {
    return DLL501_OBJECT_TYPE_ID;
}

void dll501_free(GameObject* obj) {
    Dll501State* state = obj->extra;
    int light;

    (*gObjectTriggerInterface)->freeState((u8*)state);
    gTitleMenuControlInterfaceCopy->vtable->func05(obj, 0xffff, 0, 0, 0);
    light = obj->userData2;
    if (light != 0) {
        ModelLightStruct_free((ModelLightStruct*)light);
    }
}

void dll501_render(GameObject* obj, int renderArg2, int renderArg3, int renderArg4, int renderArg5, s8 unusedVisible) {
    objRenderModelAndHitVolumes(obj, renderArg2, renderArg3, renderArg4, renderArg5, 1.0f);
    if (obj->anim.seqId == DLL501_FIRE_SEQ_ID) {
        objfx_spawnFlaggedTrailBurst(obj, 0.11f, DLL501_TRAIL_MODE, DLL501_TRAIL_EFFECT_ID, DLL501_TRAIL_EFFECT_PARAM,
                                     NULL);
    }
}

void dll501_hitDetect(void) {
}

void dll501_update(GameObject* obj) {
    int groupId;
    int* objects;
    int i;
    int objectCount;
    int current;
    int linkedObject;
    int groupId2;
    int sameGroupCount;

    if (obj->anim.placementData == NULL) {
        return;
    }
    if (((Dll501PlacementView*)obj->anim.placementData)->segmentIndex == -1) {
        return;
    }

    i = (*gObjectTriggerInterface)->update((u8*)obj, lbl_803DB411);
    if (i == 0) {
        return;
    }
    if (obj->seqIndex != DLL501_SEQ_INDEX_PENDING) {
        return;
    }

    groupId = ((Dll501State*)obj->extra)->sequence.slot;
    linkedObject = 0;
    objects = ObjList_GetObjects(&i, &objectCount);
    sameGroupCount = 0;
    i = 0;
    groupId2 = groupId;
    groupId2 |= groupId;
    while (i < objectCount) {
        current = objects[i];
        if (((GameObject*)current)->seqIndex == groupId) {
            linkedObject = current;
        }
        if (((GameObject*)current)->seqIndex == DLL501_SEQ_INDEX_PENDING &&
            ((GameObject*)current)->anim.classId == DLL501_SEQUENCE_OBJECT_CLASS_ID &&
            groupId2 == ((Dll501State*)((GameObject*)current)->extra)->sequence.slot) {
            sameGroupCount++;
        }
        i++;
    }

    if (sameGroupCount <= 1 && (void*)linkedObject != NULL && ((GameObject*)linkedObject)->seqIndex != -1) {
        ((GameObject*)linkedObject)->seqIndex = -1;
        (*gObjectTriggerInterface)->endSequence(groupId2);
    }
    obj->seqIndex = -1;
    Obj_FreeObject(obj);
}

void dll501_init(GameObject* obj, int placement) {
    Dll501State* state;
    int light;
    int chainIndex;

    state = obj->extra;
    state->sequence.gameBit = ((Dll501PlacementView*)placement)->gameBit;
    state->sequence.flags = -1;
    state->sequence.posOffsetDecay = 1.0f / (1.0f + (f32)((Dll501PlacementView*)placement)->dampingDivisor);
    state->sequence.curveId = -1;

    chainIndex = obj->userData1;
    if (chainIndex == 0 && ((Dll501PlacementView*)placement)->segmentIndex != 1) {
        (*gObjectTriggerInterface)->loadAnimData((u8*)state, (u8*)placement);
        obj->userData1 = ((Dll501PlacementView*)placement)->segmentIndex + 1;
    } else if (chainIndex != 0 && ((Dll501PlacementView*)placement)->segmentIndex != chainIndex - 1) {
        (*gObjectTriggerInterface)->freeState((u8*)state);
        if (((Dll501PlacementView*)placement)->segmentIndex != -1) {
            (*gObjectTriggerInterface)->loadAnimData((u8*)state, (u8*)placement);
        }
        obj->userData1 = ((Dll501PlacementView*)placement)->segmentIndex + 1;
    }

    if (obj->anim.seqId == DLL501_FIRE_SEQ_ID) {
        light = (int)objCreateLight(obj, 1);
        if ((u32)light != 0) {
            modelLightStruct_setLightKind((ModelLightStruct*)light, MODEL_LIGHT_KIND_POINT);
            modelLightStruct_setDiffuseColor((ModelLightStruct*)light, 200, 60, 0, 0);
            modelLightStruct_setDistanceAttenuation((ModelLightStruct*)light, 30.0f, 80.0f);
        }
        obj->userData2 = light;
    }

    dll501_resetTrackedState();
}

void dll501_release(void) {
}

void dll501_initialise(void) {
}

ObjectDescriptor gDll501ObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    dll501_initialise,
    dll501_release,
    0,
    (ObjectDescriptorCallback)dll501_init,
    (ObjectDescriptorCallback)dll501_update,
    dll501_hitDetect,
    (ObjectDescriptorCallback)dll501_render,
    (ObjectDescriptorCallback)dll501_free,
    (ObjectDescriptorCallback)dll501_getObjectTypeId,
    dll501_getExtraSize,
};
