/* DLL 0x01FB */
#include "dlls/object_descriptor.h"
#include "main/dll/dll1fbsetup_struct.h"
#include "main/dll/dll1fbstate_struct.h"
#include "main/frame_timing.h"
#include "main/gamebits.h"
#include "main/objanim_update.h"
#include "main/objseq.h"
#include "main/object_render.h"

#define PAD_BUTTON_A 0x100

STATIC_ASSERT(sizeof(Dll1FBState) == 0xc);
STATIC_ASSERT(offsetof(Dll1FBState, baseMove) == 0x04);
STATIC_ASSERT(offsetof(Dll1FBState, triggerMode) == 0x06);
STATIC_ASSERT(offsetof(Dll1FBState, hideModel) == 0x09);
STATIC_ASSERT(offsetof(Dll1FBSetup, yawByte) == 0x18);
STATIC_ASSERT(offsetof(Dll1FBSetup, baseMove) == 0x19);
STATIC_ASSERT(offsetof(Dll1FBSetup, triggerMode) == 0x1a);
STATIC_ASSERT(offsetof(Dll1FBSetup, objectParam) == 0x1c);

int dll_1FB_SeqFn(GameObject* obj, int unused, ObjAnimUpdateState* animUpdate)
{
    Dll1FBState* state = obj->extra;
    s16 mode = state->triggerMode;
    u8 flags;

    if ((mode == 1) || (mode == 2))
    {
        flags = (u8)(*(u8*)&obj->anim.resetHitboxMode | INTERACT_FLAG_DISABLED);
        *(u8*)&obj->anim.resetHitboxMode = flags;
    }
    animUpdate->activeHitVolumePair = -1;
    animUpdate->sequenceEventActive = 0;
    return 0;
}

int dll_1FB_getExtraSize_ret_12(void) { return 0xc; }
int dll_1FB_getObjectTypeId(void) { return 0; }

void dll_1FB_free_nop(void)
{
}

void dll_1FB_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible)
{
    Dll1FBState* state = obj->extra;

    if (visible == 0 || state->hideModel != 0u)
    {
        return;
    }
    objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, 1.0f);
}

void dll_1FB_hitDetect_nop(void)
{
}

void dll_1FB_update(GameObject* obj)
{
    Dll1FBState* state = obj->extra;

    if (((*(u8*)&obj->anim.resetHitboxMode & INTERACT_FLAG_ACTIVATED) != 0) && (state->triggerMode == 2) &&
        (mainGetBit(GAMEBIT_K1_SHRINE_DOOR_DIALOGUE_DONE) == 0))
    {
        (*gObjectTriggerInterface)->runSequence(4, obj, -1);
        buttonDisable(0, PAD_BUTTON_A);
        mainSetBits(GAMEBIT_K1_SHRINE_DOOR_DIALOGUE_DONE, 1);
    }
    ObjAnim_AdvanceCurrentMove((int)obj, 0.01f, timeDelta, NULL);
}

void dll_1FB_init(GameObject* obj, u8* def)
{
    Dll1FBState* state;
    Dll1FBSetup* setup;

    state = obj->extra;
    setup = (Dll1FBSetup*)def;
    ObjMsg_AllocQueue(obj, 4);
    obj->animEventCallback = dll_1FB_SeqFn;
    obj->anim.rotX = (s16)(setup->yawByte << 8);
    obj->anim.rotY = setup->objectParam;
    state->baseMove = setup->baseMove;
    state->triggerMode = setup->triggerMode;
    ObjAnim_SetCurrentMove((int)obj, state->baseMove + 0x100, 0.0f, 0);
}

void dll_1FB_release_nop(void)
{
}

void dll_1FB_initialise_nop(void)
{
}

ObjectDescriptor dll_1FB = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)dll_1FB_initialise_nop,
    (ObjectDescriptorCallback)dll_1FB_release_nop,
    0,
    (ObjectDescriptorCallback)dll_1FB_init,
    (ObjectDescriptorCallback)dll_1FB_update,
    (ObjectDescriptorCallback)dll_1FB_hitDetect_nop,
    (ObjectDescriptorCallback)dll_1FB_render,
    (ObjectDescriptorCallback)dll_1FB_free_nop,
    (ObjectDescriptorCallback)dll_1FB_getObjectTypeId,
    dll_1FB_getExtraSize_ret_12,
};
