/* Volcano Force Point object DLLs. */
#include "main/gamebits.h"
#include "main/mapEventTypes.h"
#include "main/object_render_legacy.h"
#include "dlls/object_descriptor.h"
#include "main/dll/expgfx_interface.h"
#include "main/objprint_render_api.h"
#include "main/vecmath.h"
#include "main/gamebit_ids.h"
#include "game/objects/object.h"
#include "sys/objects.h"
#include "game/objects/object_setup.h"
#include "main/audio/sfx_ids.h"
#include "main/game_ui_interface.h"
#include "main/light_internal.h"

#define LIGHT_OBJFLAG_HITDETECT_DISABLED 0x2000

extern u32 gSpellStoneEventId;
extern f32 lbl_803E6144;
extern f32 lbl_803E6148;
extern f32 lbl_803E6150;
extern f32 lbl_803E6140;
/* Per-object extra state for the VFP platform family (vfpplatform/vfpblock1/
 * vfpcoreplat). VFP_Platform_getExtraSize == 0x6. */
typedef struct VfpPlatformState
{
    s16 gameBitId; /* drives the open/close state machine */
    u8 state;      /* state-machine mode (cases 0-6) */
    u8 axisMode;   /* 0/3 = move axis, 10 = trigger-on-bit, 99/0x63 = inert */
    s16 timer;     /* dwell countdown */
} VfpPlatformState;
STATIC_ASSERT(sizeof(VfpPlatformState) == 0x6);

typedef struct SpellStoneUseState
{
    s16 completeGameBit;
    s16 requiredGameBit;
    u8 used;
} SpellStoneUseState;
typedef struct VfpPlatformPlacement
{
    u8 pad00[0x18];
    s8 rotXByte;   /* 0x18 */
    u8 axisMode;   /* 0x19 */
    u8 pad1A[6];
    s16 gameBitId; /* 0x20 */
} VfpPlatformPlacement;
typedef struct SpellStonePlacement
{
    u8 pad00[0x18];
    s8 rotXByte;         /* 0x18 */
    u8 pad19[5];
    s16 completeGameBit; /* 0x1e */
    s16 requiredGameBit; /* 0x20 */
} SpellStonePlacement;

int return0_801FD13C(void)
{
    return 0x0;
}

int VFP_coreplat_getExtraSize(void)
{
    return 0x4;
}

int VFP_coreplat_getObjectTypeId(void)
{
    return 0x0;
}

void VFP_coreplat_free(int obj)
{
    (*gExpgfxInterface)->freeSource2(obj);
}

void VFP_coreplat_render(int p1, int p2, int p3, int p4, int p5, s8 visible)
{
    objRenderModelAndHitVolumes(p1, p2, p3, p4, p5, lbl_803E6140);
}

void VFP_coreplat_hitDetect(void)
{
}

void VFP_coreplat_update(void)
{
}

void VFP_coreplat_init(GameObject* obj, int data)
{
    VfpPlatformPlacement* def = (VfpPlatformPlacement*)data;
    VfpPlatformState* state = obj->extra;
    obj->anim.rotX = (((s32)def->rotXByte) << 8);
    state->gameBitId = def->gameBitId;
    obj->animEventCallback = return0_801FD13C;
    if (obj->anim.seqId == 0x3cb)
    {
        if (mainGetBit(GAMEBIT_ITEM_SpellStone1_Used) != 0)
        {
            obj->anim.rootMotionScale = lbl_803E6144 * obj->anim.modelInstance->rootMotionScaleBase;
        }
        if (mainGetBit(GAMEBIT_ITEM_SpellStone3_Got) != 0)
        {
            obj->anim.rootMotionScale = lbl_803E6148 * obj->anim.modelInstance->rootMotionScaleBase;
        }
    }
    obj->objectFlags |= LIGHT_OBJFLAG_HITDETECT_DISABLED;
}

void VFP_coreplat_release(void)
{
}

void VFP_coreplat_initialise(void)
{
}

void spellStoneUseFn_801fd270(GameObject* obj)
{
    SpellStoneUseState* state = obj->extra;
    s16 cond = 1;
    GameObject* player = Obj_GetPlayerObject();
    if (player == NULL)
        return;
    if (state->requiredGameBit != -1)
    {
        cond = mainGetBit(state->requiredGameBit);
    }
    if ((s16)mainGetBit(state->completeGameBit) != 0 || state->used != 0)
        return;
    if (cond == 0)
        return;
    *(u8*)&obj->anim.resetHitboxMode &= ~INTERACT_FLAG_DISABLED;
    if ((*gGameUIInterface)->isEventReady(gSpellStoneEventId) != 0)
    {
        if (Vec_distance(&obj->anim.worldPosX, &player->anim.worldPosX) < lbl_803E6150)
        {
            mainSetBits(state->completeGameBit, 1);
            state->used = 1;
            *(u8*)&obj->anim.resetHitboxMode |= INTERACT_FLAG_DISABLED;
        }
    }
}

int dll_224_getExtraSize_ret_6(void)
{
    return 0x6;
}

int dll_224_getObjectTypeId(void)
{
    return 0x0;
}

void dll_224_free_nop(void)
{
}

void dll_224_render(int p1, int p2, int p3, int p4, int p5, s8 visible)
{
    if (visible == 0)
        return;
}

void dll_224_hitDetect(GameObject* obj)
{
    if (obj->anim.hitVolumeTransforms != NULL)
    {
        objRenderFn_80041018(obj);
    }
}

void dll_224_update(GameObject* obj)
{
    int mapAct;
    mapAct = (*gMapEventInterface)->getMapAct((obj)->anim.mapEventSlot);
    switch (mapAct)
    {
    case 1:
        gSpellStoneEventId = 0x123;
        break;
    case 2:
        gSpellStoneEventId = 0x83b;
        break;
    case 3:
        gSpellStoneEventId = 0x83c;
        break;
    default:
        gSpellStoneEventId = 0x123;
        break;
    }
    ((void (*)(GameObject*))spellStoneUseFn_801fd270)(obj);
}

void dll_224_init(GameObject* obj, void* other)
{
    SpellStoneUseState* extra = obj->extra;
    SpellStonePlacement* def = (SpellStonePlacement*)other;
    s16 rotX = ((s8)def->rotXByte << 8);
    u8 hitboxFlags;
    obj->anim.rotX = rotX;
    extra->completeGameBit = def->completeGameBit;
    extra->requiredGameBit = def->requiredGameBit;
    hitboxFlags = (*&obj->anim.resetHitboxMode | INTERACT_FLAG_DISABLED);
    *(u8*)&obj->anim.resetHitboxMode = hitboxFlags;
}

void dll_224_release_nop(void)
{
}

void dll_224_initialise_nop(void)
{
}

ObjectDescriptor gVFP_coreplatObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)VFP_coreplat_initialise,
    (ObjectDescriptorCallback)VFP_coreplat_release,
    0,
    (ObjectDescriptorCallback)VFP_coreplat_init,
    (ObjectDescriptorCallback)VFP_coreplat_update,
    (ObjectDescriptorCallback)VFP_coreplat_hitDetect,
    (ObjectDescriptorCallback)VFP_coreplat_render,
    (ObjectDescriptorCallback)VFP_coreplat_free,
    (ObjectDescriptorCallback)VFP_coreplat_getObjectTypeId,
    (ObjectDescriptorExtraSizeCallback)VFP_coreplat_getExtraSize,
};

ObjectDescriptor dll_224 = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)dll_224_initialise_nop,
    (ObjectDescriptorCallback)dll_224_release_nop,
    0,
    (ObjectDescriptorCallback)dll_224_init,
    (ObjectDescriptorCallback)dll_224_update,
    (ObjectDescriptorCallback)dll_224_hitDetect,
    (ObjectDescriptorCallback)dll_224_render,
    (ObjectDescriptorCallback)dll_224_free_nop,
    (ObjectDescriptorCallback)dll_224_getObjectTypeId,
    (ObjectDescriptorExtraSizeCallback)dll_224_getExtraSize_ret_6,
};
