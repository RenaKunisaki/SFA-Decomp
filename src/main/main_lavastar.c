#include "game/objects/object.h"
#include "main/dll/CF/laser.h"
#include "main/objprint_render_api.h"
#include "main/gamebits.h"
#include "dlls/object_descriptor.h"

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
