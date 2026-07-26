/*
 * shemptytumblew (DLL 0x1B4) - the empty (non-rolling) tumbleweed bush.
 *
 * init orients the bush from its placement bytes and sizes a capsule hit
 * volume scaled by the model's root-motion scale; update just polls the
 * shared priority hit-effect handler each frame.
 */
#include "dlls/object_descriptor.h"
#include "game/objects/object.h"
#include "game/objects/object_setup.h"
#include "main/objhits.h"
#include "main/dll/SH/dll_01B4_shemptytumblew.h"

#define SHEMPTYTUMBLEW_OBJFLAG_HIDDEN 0x4000

f32 lbl_803DDC00;


void SH_EmptyTumbleW_update(GameObject* obj)
{
    ObjHits_PollPriorityHitEffectWithCooldown(obj, 8, 0xff, 0xff, 0x78, 0x280, &lbl_803DDC00);
}

void SH_EmptyTumbleW_init(s16* obj, ShEmptyTumblewPlacement* def)
{
    f32 scale;

    ((GameObject*)obj)->anim.rotZ = (def->rotZByte - 0x7f) * 0x80;
    ((GameObject*)obj)->anim.rotY = (def->rotYByte - 0x7f) * 0x80;
    ((GameObject*)obj)->anim.rotX = def->rotXByte << 8;
    ((GameObject*)obj)->anim.rootMotionScale = def->scale;
    scale = ((GameObject*)obj)->anim.rootMotionScale;
    ObjHitbox_SetCapsuleBounds((ObjAnimComponent*)obj, (int)(15.0f * scale), (int)(-5.0f * scale),
                               (int)(100.0f * scale));
    ((GameObject*)obj)->objectFlags |= SHEMPTYTUMBLEW_OBJFLAG_HIDDEN;
}

ObjectDescriptor gSH_EmptyTumbleWObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    0,
    0,
    0,
    (ObjectDescriptorCallback)SH_EmptyTumbleW_init,
    (ObjectDescriptorCallback)SH_EmptyTumbleW_update,
    0,
    0,
    0,
    0,
    0,
};
