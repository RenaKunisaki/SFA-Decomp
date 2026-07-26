/*
 * DLL 0xEA - Sideload.
 *
 * The only function this object contributes is sideload_update: a deferred
 * spawner placed in a map. Each tick, once the level has finished loading
 * (Obj_IsLoadingLocked), the player object exists, Tricky is absent, and the
 * placement's arming game bit (placement+0x18) is set, it allocates an object
 * setup (type 0x24), copies the spawner's position into it, hands it to
 * Obj_SetupObject, and seeds the new object's first field from placement
 * rotX byte.
 */
#include "game/objects/object.h"
#include "sys/objects.h"
#include "sys/objects/lifecycle.h"
#include "main/gamebits.h"
#include "main/dll/dll_00EA_sideload.h"

/* object id sideload_update defers into existence once its arming game bit is set */
#define SIDELOAD_TRICKY_OBJECT_ID 0x24

void sideload_update(GameObject* self)
{
    SideloadPlacement* placement;
    ObjPlacement* setup;
    GameObject* child;

    placement = (SideloadPlacement*)self->anim.placementData;
    if ((Obj_IsLoadingLocked() != 0) && (Obj_GetPlayerObject() != NULL) && (getTrickyObject() == NULL) &&
        (mainGetBit(placement->armGameBit) != 0))
    {
        setup = Obj_AllocObjectSetup(sizeof(ObjPlacement), SIDELOAD_TRICKY_OBJECT_ID);
        setup->color[0] = 2;
        setup->color[1] = 4;
        setup->color[3] = 0xff;
        setup->posX = self->anim.localPosX;
        setup->posY = self->anim.localPosY;
        setup->posZ = self->anim.localPosZ;
        child = Obj_SetupObject(setup, 5, -1, -1, NULL);
        child->anim.rotX = (s16)((u8)placement->rotX << 8);
    }
}

ObjectDescriptor gSideloadObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    0,
    0,
    0,
    0,
    (ObjectDescriptorCallback)sideload_update,
    0,
    0,
    0,
    0,
    0,
};
