/*
 * SB_ShipGunB (DLL 0x01F7) - the wrecked variant of the galleon's deck
 * gun (SB_ShipGun) in the ShipBattle prologue (SB = the retail "ShipBattle"
 * map), shown after the gun has been shot out.
 *
 * It is purely cosmetic: a static prop that is only rendered (and plays a
 * looping electrical-damage sfx) while a placement-supplied GameBit is set.
 * That GameBit is the gun's "destroyed" flag - its index is stored in the
 * placement record at offset 0x1E.
 */
#include "main/object_render.h"
#include "game/objects/object.h"
#include "main/audio/sfx.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/gamebits.h"
#include "main/dll/SB/dll_01F7_sbshipgunbroke.h"
#include "dlls/object_descriptor.h"

int SB_ShipGunBroke_getExtraSize(void)
{
    return 0x1;
}
int SB_ShipGunBroke_getObjectTypeId(void)
{
    return 0x0;
}

void SB_ShipGunBroke_free(void)
{
}

void SB_ShipGunBroke_render(GameObject* obj, int p2, int p3, int p4, int p5)
{
    SBShipGunBrokePlacement* placement = (SBShipGunBrokePlacement*)obj->anim.placementData;
    if (mainGetBit(placement->destroyedGameBit) != 0u)
    {
        objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, 1.0f);
    }
}

void SB_ShipGunBroke_hitDetect(void)
{
}

void SB_ShipGunBroke_update(GameObject* obj)
{
    SBShipGunBrokePlacement* placement = (SBShipGunBrokePlacement*)obj->anim.placementData;
    if (mainGetBit(placement->destroyedGameBit) != 0u)
    {
        Sfx_PlayFromObject((u32)obj, SFXTRIG_en_trpopn_c);
    }
}

void SB_ShipGunBroke_init(void)
{
}

void SB_ShipGunBroke_release(void)
{
}

void SB_ShipGunBroke_initialise(void)
{
}

ObjectDescriptor gSB_ShipGunBrokeObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)SB_ShipGunBroke_initialise,
    (ObjectDescriptorCallback)SB_ShipGunBroke_release,
    0,
    (ObjectDescriptorCallback)SB_ShipGunBroke_init,
    (ObjectDescriptorCallback)SB_ShipGunBroke_update,
    (ObjectDescriptorCallback)SB_ShipGunBroke_hitDetect,
    (ObjectDescriptorCallback)SB_ShipGunBroke_render,
    (ObjectDescriptorCallback)SB_ShipGunBroke_free,
    (ObjectDescriptorCallback)SB_ShipGunBroke_getObjectTypeId,
    SB_ShipGunBroke_getExtraSize,
};
