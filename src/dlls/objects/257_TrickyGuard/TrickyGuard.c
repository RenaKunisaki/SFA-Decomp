/*
 * DLL 0x101 - TrickyGuard.
 *
 * Directs an available Tricky to guard this object's position when its
 * optional game-bit condition is met and the player enters range.
 */
#include "dlls/objects/257_TrickyGuard.h"
#include "game/objects/object.h"
#include "main/dll/dll_0120_trickyguardspot.h"
#include "main/gamebits.h"
#include "main/objprint_render_api.h"
#include "sys/objects/lifecycle.h"

#define TRICKYGUARD_GAMEBIT_NONE   -1
#define TRICKYGUARD_VTABLE(tricky) (*(TrickyGuardSpotInterfaceVTable**)((tricky)->anim.dll))

void TrickyGuard_update(GameObject* obj) {
    GameObject* tricky;
    TrickyGuardPlacement* placement = (TrickyGuardPlacement*)obj->anim.placementData;

    obj->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
    if (placement->armingGameBit != TRICKYGUARD_GAMEBIT_NONE) {
        if ((u32)mainGetBit(placement->armingGameBit) == 0) {
            return;
        }
    }
    tricky = getTrickyObject();
    if (tricky == NULL) {
        return;
    }
    if ((u8)TRICKYGUARD_VTABLE(tricky)->isBusy(&tricky->anim) != 0) {
        return;
    }
    if ((obj->anim.resetHitboxFlags & INTERACT_FLAG_IN_RANGE) != 0) {
        TRICKYGUARD_VTABLE(tricky)->setGuardSpotAction(&tricky->anim, (TrickyGuardSpotObject*)obj,
                                                       TRICKY_GUARD_SPOT_ACTION, TRICKY_GUARD_SPOT_ACTION_PARAM);
    }
    obj->anim.resetHitboxFlags = (u8)(obj->anim.resetHitboxFlags & ~INTERACT_FLAG_DISABLED);
    objRenderFn_80041018(obj);
}

void TrickyGuard_init(GameObject* obj, TrickyGuardPlacement* placement) {
    u32 flags;

    obj->anim.rotX = (s16)((u32)placement->rotXByte << 8);
    flags = obj->objectFlags;
    flags |= OBJECT_OBJFLAG_HIDDEN;
    obj->objectFlags = flags;
}

ObjectDescriptor gTrickyGuardObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    0,
    0,
    0,
    (ObjectDescriptorCallback)TrickyGuard_init,
    (ObjectDescriptorCallback)TrickyGuard_update,
    0,
    0,
    0,
    0,
    0,
};
