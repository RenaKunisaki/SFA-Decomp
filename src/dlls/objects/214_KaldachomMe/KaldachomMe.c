/*
 * KaldaChompMe object (DLL slot 214).
 *
 * Drives a linked mouth animation toward a requested progress and selects
 * linked mouth objects from the owning Kaldachom's placement ID.
 */
#include "dlls/objects/214_KaldachomMe.h"
#include "main/frame_timing.h"
#include "main/object_render.h"
#include "sys/objects.h"

ObjectDescriptor gKaldaChompMeObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)KaldaChompMe_initialise,
    (ObjectDescriptorCallback)KaldaChompMe_release,
    0,
    (ObjectDescriptorCallback)KaldaChompMe_init,
    (ObjectDescriptorCallback)KaldaChompMe_update,
    (ObjectDescriptorCallback)KaldaChompMe_hitDetect,
    (ObjectDescriptorCallback)KaldaChompMe_render,
    (ObjectDescriptorCallback)KaldaChompMe_free,
    (ObjectDescriptorCallback)KaldaChompMe_getObjectTypeId,
    KaldaChompMe_getExtraSize,
};

void kaldachompme_setLinkedMouthMode(GameObject* obj, KaldaChompMeLinkedMode mode) {
    KaldaChompMeState* state;
    GameObject* linkedObj;

    if (obj == NULL) {
        return;
    }
    switch (obj->anim.placement->mapId) {
    case 0x43d14:
        linkedObj = ObjList_FindObjectById(0x4b3b5);
        break;
    case 0x41be9:
        linkedObj = ObjList_FindObjectById(0x4b3f9);
        break;
    case 0x41cc4:
        linkedObj = ObjList_FindObjectById(0x4b402);
        break;
    case 0x41cc5:
        linkedObj = ObjList_FindObjectById(0x4b403);
        break;
    case 0x41cc6:
        linkedObj = ObjList_FindObjectById(0x4b404);
        break;
    case 0x41cc7:
        linkedObj = ObjList_FindObjectById(0x4b40b);
        break;
    case 0x41cc8:
        linkedObj = ObjList_FindObjectById(0x4b40c);
        break;
    case 0x41cc9:
        linkedObj = ObjList_FindObjectById(0x4b40f);
        break;
    case 0x41cd2:
        linkedObj = ObjList_FindObjectById(0x4b410);
        break;
    case 0x41ccc:
        linkedObj = ObjList_FindObjectById(0x4b411);
        break;
    case 0x41cd5:
        linkedObj = ObjList_FindObjectById(0x4b414);
        break;
    case 0x41cd6:
        linkedObj = ObjList_FindObjectById(0x4b415);
        break;
    case 0x41cd9:
        linkedObj = ObjList_FindObjectById(0x4b453);
        break;
    default:
        return;
    }
    state = linkedObj->extra;
    if (state != NULL) {
        switch (mode) {
        case KALDACHOMPME_LINKED_MODE_MOVE_0:
            state->targetProgress = 1.0f;
            state->progress = 0.0f;
            state->step = 0.025f;
            state->moveId = 0;
            break;
        case KALDACHOMPME_LINKED_MODE_MOVE_1:
            state->targetProgress = 1.0f;
            state->progress = 0.0f;
            state->step = 0.025f;
            state->moveId = 1;
            break;
        }
    }
}

int KaldaChompMe_getExtraSize(void) {
    return sizeof(KaldaChompMeState);
}

int KaldaChompMe_getObjectTypeId(void) {
    return 0;
}

void KaldaChompMe_free(GameObject* obj) {
    (void)obj;
}

void KaldaChompMe_render(GameObject* obj, int fwdArg2, int fwdArg3, int fwdArg4, int fwdArg5, s8 visible) {
    if (visible != 0) {
        objRenderModelAndHitVolumes(obj, fwdArg2, fwdArg3, fwdArg4, fwdArg5, 1.0f);
    }
}

void KaldaChompMe_hitDetect(GameObject* obj) {
    (void)obj;
}

void KaldaChompMe_update(GameObject* obj) {
    f32 target;
    f32 current;
    f32 step;
    KaldaChompMeState* state;

    state = obj->extra;
    current = state->progress;
    target = state->targetProgress;
    if (current != target) {
        step = state->step;
        if (step > 0.0f) {
            if (current < target) {
                state->progress = current + step * timeDelta;
            } else {
                state->progress = target;
            }
        } else {
            if (current > target) {
                state->progress = current + step * timeDelta;
            } else {
                state->progress = target;
            }
        }
    }
    ObjAnim_SetCurrentMove((int)obj, state->moveId, state->progress, 0);
}

void KaldaChompMe_init(GameObject* obj, KaldaChompMePlacement* placement) {
    obj->anim.rotZ = (s16)(placement->rotZByte << 8);
    obj->anim.rotY = (s16)(placement->rotYByte << 8);
    obj->anim.rotX = (s16)(placement->rotXByte << 8);
    obj->objectFlags |= OBJECT_OBJFLAG_HITDETECT_DISABLED;
    ObjAnim_SetCurrentMove((int)obj, 0, 0.0f, 0);
}

void KaldaChompMe_release(void) {
}

void KaldaChompMe_initialise(void) {
}
