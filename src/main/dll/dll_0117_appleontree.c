/* DLL 0x0117 - appleontree / groundAnimator group. TU: 0x8017D818-0x8017D854. */
#include "main/dll/partfx_interface.h"
#include "main/audio/sfx_ids.h"
#include "main/vecmath_distance_api.h"
#include "main/audio/sfx_play_api.h"
#include "main/objfx.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/vecmath.h"
#include "main/dll/groundAnimator.h"
#include "main/dll_000A_expgfx.h"
#include "main/dll/waterfx_interface.h"
#include "main/game_object.h"
#include "main/obj_placement.h"
#include "main/obj_message.h"
#include "main/object_api.h"
#include "main/object.h"
#include "main/objseq.h"
#include "main/objtexture.h"
#include "main/dll/baddie_state.h"
#include "main/sky_interface.h"
#include "main/gamebits.h"
#include "main/frame_timing.h"
#include "main/track_dolphin_api.h"
#include "main/objhits.h"
#include "main/dll/dll_00FC_babycloudrunner.h"
#include "main/dll/dll_0117_appleontree.h"
#include "main/dll/player_api.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/object_render.h"
#include "main/dll/dll_0117_appleontree_internal.h"

ObjectDescriptor13 gAppleOnTreeObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_13_SLOTS,
    0,
    0,
    0,
    (ObjectDescriptorCallback)AppleOnTree_init,
    (ObjectDescriptorCallback)AppleOnTree_update,
    0,
    (ObjectDescriptorCallback)AppleOnTree_render,
    (ObjectDescriptorCallback)AppleOnTree_free,
    0,
    AppleOnTree_getExtraSize,
    (ObjectDescriptorCallback)AppleOnTree_setScale,
    (ObjectDescriptorCallback)AppleOnTree_setPosition,
    (ObjectDescriptorCallback)AppleOnTree_modelMtxFn,
};

void AppleOnTree_setPosition(GameObject* obj, float* pos)
{
    AppleOnTreeState* state = obj->extra;

    if (state->animState == APPLEONTREE_STATE_KNOCKED)
    {
        return;
    }
    if (state->animState == APPLEONTREE_STATE_BURST)
    {
        return;
    }
    if (state->animState == APPLEONTREE_STATE_FADEOUT)
    {
        return;
    }
    obj->anim.localPosX = pos[0];
    obj->anim.localPosY = pos[1];
    obj->anim.localPosZ = pos[2];
}
