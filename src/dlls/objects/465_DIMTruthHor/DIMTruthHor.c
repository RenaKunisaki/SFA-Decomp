/*
 * DIMTruthHor (DLL 0x1D1) - a breakable ice target in Snowhorn Wastes 2.
 * Hit-count tracked in extra->hitsLeft; when depleted sets gameBit and starts a
 * particle-burst death animation (spawn loop in phase 1, freeze-hide in phase 2).
 * Tricky can deliver fire hits via vtable dispatch (slot 0x28 of Tricky's type at
 * offset 0x68). Also contains dimtruthhornice_countdownCallback, a generic
 * byte-damage helper.
 */
#include "main/dll/partfx_interface.h"
#include "main/dll/DIM/dll_01D1_dimtruthhornice.h"
#include "sys/objects/lifecycle.h"
#include "main/dll/truthhornicestate_struct.h"
#include "dlls/object_descriptor.h"
#include "game/objects/object.h"
#include "main/gamebits.h"
#include "main/frame_timing.h"
#include "main/vecmath.h"
#include "main/objhits.h"
#include "main/audio/sfx.h"
#include "main/audio/sfx_trigger_ids.h"

#define DIMTRUTHHORNICE_OBJFLAG_HIDDEN 0x4000

typedef struct DimtruthhorniceObjectDef
{
    u8 pad0[0x1A - 0x0];
    s16 hitsLeft;
    s16 unk1C;
    s16 gameBit;
} DimtruthhorniceObjectDef;

typedef enum TruthHornIcePhase
{
    TRUTHHORNICE_PHASE_INTACT = 0,     /* takes hits; on break sets game bit, disables hits */
    TRUTHHORNICE_PHASE_SHATTERING = 1, /* delay timer, then spawns the ice-shard burst */
    TRUTHHORNICE_PHASE_SHATTERED = 2,  /* hidden */
} TruthHornIcePhase;

STATIC_ASSERT(sizeof(TruthHornIceState) == 0x8);

int dimtruthhornice_countdownCallback(GameObject* obj, int damage)
{
    u8* state = obj->extra;
    *(s8*)(state + 2) = (s8)(state[2] - damage);
    return *(s8*)(state + 2) <= 0;
}

int dimtruthhornice_getExtraSize(void)
{
    return 0x8;
}

void dimtruthhornice_update(GameObject* obj)
{
    TruthHornIceState* extra = obj->extra;
    *(u8*)&obj->anim.resetHitboxMode |= INTERACT_FLAG_DISABLED;
    switch (extra->phase)
    {
    case TRUTHHORNICE_PHASE_INTACT:
        if (extra->hitsLeft <= 0)
        {
            if (extra->gameBit != -1)
            {
                mainSetBits(extra->gameBit, 1);
                ObjHits_DisableObject(obj);
                extra->phase = TRUTHHORNICE_PHASE_SHATTERING;
                extra->timer = 0.0f;
            }
        }
        else
        {
            int* tricky = (int*)getTrickyObject();
            if (tricky != NULL)
            {
                if ((*(u8*)&obj->anim.resetHitboxMode & INTERACT_FLAG_IN_RANGE) != 0)
                {
                    (*(void (**)(int*, GameObject*, int, int))(**(int**)((char*)tricky + 0x68) + 0x28))(tricky, obj, 1, 4);
                }
                *(u8*)&obj->anim.resetHitboxMode &= ~INTERACT_FLAG_DISABLED;
            }
        }
        break;
    case TRUTHHORNICE_PHASE_SHATTERING:
        {
            PartFxSpawnParams desc;
            extra->timer = extra->timer + timeDelta;
            if (extra->timer > 20.0f)
            {
                int i;
                extra->phase = TRUTHHORNICE_PHASE_SHATTERED;
                Sfx_PlayFromObject(0, SFXTRIG_menuups16k);
                Sfx_PlayFromObject((int)obj, SFXTRIG_barrel_bounce1);
                for (i = 30; i != 0; i--)
                {
                    desc.posX = 0.1f * (f32)(int)randomGetRange(-100, 100);
                    desc.posY = 0.1f * (f32)(int)randomGetRange(0, 350);
                    desc.posZ = 0.1f * (f32)(int)randomGetRange(-100, 100);
                    desc.scale = 1.0f;
                    (*gPartfxInterface)->spawnObject(obj, 2043, &desc, 2, -1, NULL);
                    (*gPartfxInterface)->spawnObject(obj, 2044, &desc, 2, -1, NULL);
                }
            }
            desc.posX = 0.1f * (f32)(int)randomGetRange(-100, 100);
            desc.posY = 0.1f * (f32)(int)randomGetRange(0, 350);
            desc.posZ = 0.1f * (f32)(int)randomGetRange(-100, 100);
            desc.scale = 1.0f;
            (*gPartfxInterface)->spawnObject(obj, 2044, &desc, 2, -1, NULL);
            break;
        }
    case TRUTHHORNICE_PHASE_SHATTERED:
        obj->anim.flags |= OBJANIM_FLAG_HIDDEN;
        break;
    }
}

void dimtruthhornice_init(GameObject* obj, DimtruthhorniceObjectDef* def)
{
    TruthHornIceState* state = obj->extra;
    state->hitsLeft = (s8)def->hitsLeft;
    state->gameBit = def->gameBit;
    obj->objectFlags = (u16)(obj->objectFlags | DIMTRUTHHORNICE_OBJFLAG_HIDDEN);
    {
        s16 slot = state->gameBit;
        if (slot != -1 && mainGetBit(slot) != 0u)
        {
            ObjHits_DisableObject(obj);
            state->phase = TRUTHHORNICE_PHASE_SHATTERED;
            obj->anim.flags = (s16)(obj->anim.flags | OBJANIM_FLAG_HIDDEN);
        }
    }
}

ObjectDescriptor gDIMTruthHornIceObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    0,
    0,
    0,
    (ObjectDescriptorCallback)dimtruthhornice_init,
    (ObjectDescriptorCallback)dimtruthhornice_update,
    0,
    0,
    0,
    0,
    (ObjectDescriptorExtraSizeCallback)dimtruthhornice_getExtraSize,
};
