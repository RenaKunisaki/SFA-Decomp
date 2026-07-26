#include "main/dll/partfx_interface.h"
#include "game/objects/object.h"
#include "main/dll/CF/laser.h"
#include "main/object_render.h"
#include "main/objprint_render_api.h"
#include "sys/objects/lifecycle.h"
#include "main/audio/sfx.h"
#include "main/dll_000A_expgfx.h"
#include "main/dll/modgfx_interface.h"
#include "main/gamebits.h"
#include "main/main_internal.h"
#include "main/objprint_api.h"
#include "main/objtexture.h"
#include "main/obj_group.h"
#include "main/resource.h"
#include "main/vecmath.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/texture.h"
#include "main/frame_timing.h"
#include "main/audio/sfx_ids.h"
#include "main/audio/sfx_trigger_ids.h"
#include "dlls/object_descriptor.h"

typedef struct VfpLavaStarState
{
    f32 verticalVelocity;
    f32 delayRangeMin;
    f32 delayRangeMax;
    s16 gameBit;
    s16 effectTimer;
    u8 particleToggle;
    u8 pad11[3];
} VfpLavaStarState;

typedef struct VfpLavaStarMapData
{
    ObjPlacement base;
    u8 pad18[2];
    s16 heightOffset;
    u8 pad1C[2];
    s16 gameBit;
} VfpLavaStarMapData;

STATIC_ASSERT(sizeof(VfpLavaStarState) == 0x14);
STATIC_ASSERT(offsetof(VfpLavaStarState, verticalVelocity) == 0x00);
STATIC_ASSERT(offsetof(VfpLavaStarState, delayRangeMin) == 0x04);
STATIC_ASSERT(offsetof(VfpLavaStarState, delayRangeMax) == 0x08);
STATIC_ASSERT(offsetof(VfpLavaStarState, gameBit) == 0x0C);
STATIC_ASSERT(offsetof(VfpLavaStarState, effectTimer) == 0x0E);
STATIC_ASSERT(offsetof(VfpLavaStarState, particleToggle) == 0x10);
STATIC_ASSERT(offsetof(VfpLavaStarMapData, heightOffset) == 0x1A);
STATIC_ASSERT(offsetof(VfpLavaStarMapData, gameBit) == 0x1E);


#define MAIN_OBJFLAG_HITDETECT_DISABLED 0x2000

#define MAIN_LAVAPOOL_RESOURCE_ID 0xa6

#define MAIN_LAVASTAR_PARTFX 0x3a4

extern void* gVfpLavaPoolEffectResource;
extern f32 lbl_803E61B0;
extern f32 lbl_803E61B4;

int VFP_lavastar_getExtraSize(void)
{
    return sizeof(VfpLavaStarState);
}
int VFP_lavastar_getObjectTypeId(void)
{
    return 0x0;
}

void VFP_lavastar_free(int obj)
{
    (*gExpgfxInterface)->freeSource2((u32)obj);
    (*gModgfxInterface)->freeSourceEffects((void*)obj);
}

void VFP_lavastar_render(void)
{
}

void VFP_lavastar_hitDetect(void)
{
}

void VFP_lavastar_update(GameObject* obj)
{
    VfpLavaStarMapData* mapData;
    VfpLavaStarState* state;

    mapData = (VfpLavaStarMapData*)obj->anim.placementData;
    state = obj->extra;
    obj->anim.localPosY += timeDelta * state->verticalVelocity;
    if (obj->anim.localPosY > lbl_803E61B0 + mapData->base.posY)
    {
        state->verticalVelocity = lbl_803E61B4 * (f32)(int)randomGetRange(5, 0x14);
        obj->anim.localPosY = mapData->base.posY;
    }
    state->effectTimer += (s16)timeDelta;
    if (gVfpLavaPoolEffectResource != 0 && state->effectTimer >= 0x28)
    {
        (*(void (*)(int, int, int, int, int, int)) * (int*)(*(int*)gVfpLavaPoolEffectResource + 4))((int)obj, 0, 0, 4,
                                                                                                    -1, 0);
        state->effectTimer = 0;
    }
    if (state->particleToggle == 0)
    {
        (*gPartfxInterface)->spawnObject((void*)obj, MAIN_LAVASTAR_PARTFX, NULL, 2, -1, NULL);
    }
    state->particleToggle ^= 1;
}

void VFP_lavastar_init(GameObject* obj, int def)
{
    VfpLavaStarState* state;
    VfpLavaStarMapData* mapData;

    mapData = (VfpLavaStarMapData*)def;
    state = obj->extra;
    state->gameBit = mapData->gameBit;
    state->verticalVelocity = lbl_803E61B4 * (f32)(int)randomGetRange(10, 0x19);
    state->effectTimer = 0x14;
    obj->anim.localPosY = mapData->base.posY + (f32)(int)mapData->heightOffset;
    obj->objectFlags |= MAIN_OBJFLAG_HITDETECT_DISABLED;
    state->delayRangeMin = (f32)(int)randomGetRange(0x1e, 0x3c);
    state->delayRangeMax = (f32)(int)randomGetRange(100, 200);
}
void VFP_lavastar_release(void)
{
    Resource_Release(gVfpLavaPoolEffectResource);
    gVfpLavaPoolEffectResource = NULL;
}

void VFP_lavastar_initialise(void)
{
    gVfpLavaPoolEffectResource = NULL;
    gVfpLavaPoolEffectResource = Resource_Acquire(MAIN_LAVAPOOL_RESOURCE_ID, 1);
}

int VFP_SpellPlace_getExtraSize(void)
{
    return sizeof(LaserState);
}
int VFP_SpellPlace_getObjectTypeId(void)
{
    return 0x0;
}

void VFP_SpellPlace_free(void)
{
}

void VFP_SpellPlace_render(void)
{
}

void VFP_SpellPlace_hitDetect(void)
{
}

void VFP_SpellPlace_update(int obj)
{
    LaserObject* spellPlace;
    LaserState* state;
    u8 mode;

    spellPlace = (LaserObject*)obj;
    if (spellPlace->state->completionLatched == 0 && mainGetBit((int)spellPlace->state->activationGameBit) != 0)
    {
        spellPlace->statusFlags &= ~LASER_OBJECT_STATUS_DISABLED;
    }
    else
    {
        spellPlace->statusFlags |= LASER_OBJECT_STATUS_DISABLED;
    }
    objRenderFn_80041018((GameObject*)obj);
    if (spellPlace->statusFlags & LASER_OBJECT_STATUS_ACTIVE)
    {
        mode = (*gMapEventInterface)->getMapAct((int)spellPlace->mapEventSlot);
        switch (mode)
        {
        case LASEROBJ_MODE_SEQUENCE_A:
            state = spellPlace->state;
            if ((*gGameUIInterface)->isEventReady(LASEROBJ_MAIN_SEQUENCE_A_EVENT) != 0)
            {
                mainSetBits(state->completionGameBit, 1);
                mainSetBits(state->activationGameBit, 0);
                state->completionLatched = 1;
                spellPlace->statusFlags |= LASER_OBJECT_STATUS_DISABLED;
            }
            break;
        case LASEROBJ_MODE_SEQUENCE_B:
            state = spellPlace->state;
            if ((*gGameUIInterface)->isEventReady(LASEROBJ_MAIN_SEQUENCE_B_EVENT) != 0)
            {
                mainSetBits(state->completionGameBit, 1);
                mainSetBits(state->activationGameBit, 0);
                state->completionLatched = 1;
                spellPlace->statusFlags |= LASER_OBJECT_STATUS_DISABLED;
            }
            break;
        }
    }
}

void VFP_SpellPlace_init(int obj, s8* def)
{
    LaserObject* spellPlace;
    LaserObjectMapData* mapData;
    LaserState* state;

    spellPlace = (LaserObject*)obj;
    mapData = (LaserObjectMapData*)def;
    state = spellPlace->state;
    state->completionGameBit = mapData->completionGameBit;
    state->activationGameBit = mapData->activationGameBit;
    state->completionLatched = 0;
    spellPlace->modeWord = (s16)(mapData->mapEventSlot << LASEROBJ_MODE_WORD_SHIFT);
    if (mainGetBit(state->completionGameBit) != 0)
    {
        state->completionLatched = 1;
        spellPlace->statusFlags |= LASER_OBJECT_STATUS_DISABLED;
    }
    spellPlace->objectFlags |= LASER_OBJECT_FLAGS_SEQUENCE_CONTROL;
}

void VFP_SpellPlace_release(void)
{
}

void VFP_SpellPlace_initialise(void)
{
}

ObjectDescriptor gVFP_lavastarObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)VFP_lavastar_initialise,
    (ObjectDescriptorCallback)VFP_lavastar_release,
    0,
    (ObjectDescriptorCallback)VFP_lavastar_init,
    (ObjectDescriptorCallback)VFP_lavastar_update,
    (ObjectDescriptorCallback)VFP_lavastar_hitDetect,
    (ObjectDescriptorCallback)VFP_lavastar_render,
    (ObjectDescriptorCallback)VFP_lavastar_free,
    (ObjectDescriptorCallback)VFP_lavastar_getObjectTypeId,
    (ObjectDescriptorExtraSizeCallback)VFP_lavastar_getExtraSize,
};

ObjectDescriptor gVFP_SpellPlaceObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)VFP_SpellPlace_initialise,
    (ObjectDescriptorCallback)VFP_SpellPlace_release,
    0,
    (ObjectDescriptorCallback)VFP_SpellPlace_init,
    (ObjectDescriptorCallback)VFP_SpellPlace_update,
    (ObjectDescriptorCallback)VFP_SpellPlace_hitDetect,
    (ObjectDescriptorCallback)VFP_SpellPlace_render,
    (ObjectDescriptorCallback)VFP_SpellPlace_free,
    (ObjectDescriptorCallback)VFP_SpellPlace_getObjectTypeId,
    (ObjectDescriptorExtraSizeCallback)VFP_SpellPlace_getExtraSize,
};
