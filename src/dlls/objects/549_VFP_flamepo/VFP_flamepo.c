/* VFP_flamepo (DLL 0x0225) */
#include "dlls/object_descriptor.h"
#include "game/objects/object.h"
#include "game/objects/object_setup.h"
#include "main/gamebits.h"
#include "main/obj_group.h"
#include "main/objprint_render_api.h"
#include "sys/objects/lifecycle.h"

#define VFP_FLAMEPOINT_OBJFLAG_HIDDEN             0x4000
#define VFP_FLAMEPOINT_OBJFLAG_HITDETECT_DISABLED 0x2000

typedef struct VfpFlamePointTrickyInterface
{
    void* unknown00[10];
    void (*sideCommandEnable)(GameObject* tricky, GameObject* target, int commandKind, int commandType);
} VfpFlamePointTrickyInterface;

STATIC_ASSERT(offsetof(VfpFlamePointTrickyInterface, sideCommandEnable) == 0x28);

#define VFP_FLAMEPOINT_TRICKY_INTERFACE(tricky) ((VfpFlamePointTrickyInterface*)*(tricky)->anim.dll)

static const f32 lbl_803E6158 = 35.0f;

typedef struct VfpFlamePointData
{
    s16 showGameBit;  /* 0x0 */
    s16 checkGameBit; /* 0x2 */
    s8 counter;       /* 0x4 */
    u8 done : 1;      /* 0x5 bit 7 */
    u8 noCheck : 1;   /* 0x5 bit 6 */
    u8 pad06[2];
} VfpFlamePointData;

typedef struct VfpFlamePointMapData
{
    ObjPlacement base;
    u8 pad18[2];
    s16 counterInit;  /* 0x1a */
    s16 noCheck;      /* 0x1c */
    s16 showGameBit;  /* 0x1e */
    s16 checkGameBit; /* 0x20 */
} VfpFlamePointMapData;

STATIC_ASSERT(sizeof(VfpFlamePointData) == 0x08);
STATIC_ASSERT(offsetof(VfpFlamePointData, showGameBit) == 0x00);
STATIC_ASSERT(offsetof(VfpFlamePointData, checkGameBit) == 0x02);
STATIC_ASSERT(offsetof(VfpFlamePointData, counter) == 0x04);
STATIC_ASSERT(offsetof(VfpFlamePointMapData, counterInit) == 0x1A);
STATIC_ASSERT(offsetof(VfpFlamePointMapData, noCheck) == 0x1C);
STATIC_ASSERT(offsetof(VfpFlamePointMapData, showGameBit) == 0x1E);
STATIC_ASSERT(offsetof(VfpFlamePointMapData, checkGameBit) == 0x20);

int vfpflamepoint_countdownCallback(GameObject* obj, int x)
{
    VfpFlamePointData* extra = obj->extra;

    if (extra != NULL)
    {
        extra->counter -= x;
        return extra->counter <= 0;
    }
    return 0;
}

int VFP_flamepoint_getExtraSize(void)
{
    return sizeof(VfpFlamePointData);
}

void VFP_flamepoint_update(GameObject* obj)
{
    VfpFlamePointData* d;
    void* tricky;

    d = obj->extra;
    *(u8*)&obj->anim.resetHitboxMode |= INTERACT_FLAG_DISABLED;
    if (!d->done && (d->checkGameBit == -1 || mainGetBit(d->checkGameBit) != 0))
    {
        if (d->counter <= 0 && !d->done)
        {
            if (d->showGameBit != -1)
            {
                mainSetBits(d->showGameBit, 1);
                d->done = 1;
            }
        }
        else
        {
            tricky = getTrickyObject();
            if (tricky != NULL)
            {
                f32 dist = lbl_803E6158;

                if (d->noCheck || (void*)ObjGroup_FindNearestObject(5, obj, &dist) == NULL)
                {
                    if (*(u8*)&obj->anim.resetHitboxMode & INTERACT_FLAG_IN_RANGE)
                    {
                        VFP_FLAMEPOINT_TRICKY_INTERFACE((GameObject*)tricky)
                            ->sideCommandEnable((GameObject*)tricky, obj, 1, 4);
                    }
                    *(u8*)&obj->anim.resetHitboxMode &= ~INTERACT_FLAG_DISABLED;
                    objRenderFn_80041018(obj);
                }
            }
        }
    }
    else
    {
        u8 v = mainGetBit(d->showGameBit);

        if (!(d->done = v))
        {
            d->counter = (s8) * (s16*)(*(int*)&obj->anim.placementData + 0x1a);
        }
    }
}

void VFP_flamepoint_init(int* obj, s8* def)
{
    VfpFlamePointData* d = (VfpFlamePointData*)((GameObject*)obj)->extra;
    VfpFlamePointMapData* mapData = (VfpFlamePointMapData*)def;

    d->counter = (s8)mapData->counterInit;
    d->noCheck = (u8)mapData->noCheck;
    d->showGameBit = mapData->showGameBit;
    d->checkGameBit = mapData->checkGameBit;
    ((GameObject*)obj)->objectFlags |=
        (VFP_FLAMEPOINT_OBJFLAG_HIDDEN | VFP_FLAMEPOINT_OBJFLAG_HITDETECT_DISABLED);
}

ObjectDescriptor gVFP_flamepointObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    0,
    0,
    0,
    (ObjectDescriptorCallback)VFP_flamepoint_init,
    (ObjectDescriptorCallback)VFP_flamepoint_update,
    0,
    0,
    0,
    0,
    (ObjectDescriptorExtraSizeCallback)VFP_flamepoint_getExtraSize,
};
