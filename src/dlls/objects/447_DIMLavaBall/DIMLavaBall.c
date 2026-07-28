/*
 * DIMLavaBall (DLL 0x1BF) - DIM lava-ball cannon proxy; manages the spawned
 * 0x18D lava-ball sub-object, controls its fire period and game-bit gate,
 * and relaunches it on each fire cycle.
 */
#include "dlls/objects/446.h"
#include "main/dll/lavaball1bfstate_struct.h"
#include "main/frame_timing.h"
#include "main/objseq.h"
#include "sys/objects/lifecycle.h"
#include "main/object_render.h"
#include "dlls/object_descriptor.h"
#include "game/objects/object_setup.h"
#include "game/objects/object.h"
#include "sys/objects.h"
#include "main/gamebits.h"
#include "main/vecmath.h"

#define DIMLAVABALL_OBJFLAG_HITDETECT_DISABLED 0x2000
#define DIMLAVABALL_OBJFLAG_HIDDEN             0x4000

typedef struct Lavaball1bfPlacement
{
    ObjPlacement head; /* 0x00 */
    s8 firePeriod; /* 0x18 read raw as s16 (p+0x18) into state.firePeriod */
    u8 pad19[0x1E - 0x19];
    s16 triggerGameBit;
    u8 pad20[0x24 - 0x20];
    s16 stateGameBit;
    u8 pad26[0x28 - 0x26];
} Lavaball1bfPlacement;

STATIC_ASSERT(sizeof(Lavaball1bfState) == 0x1C);
void lavaball1bf_clearPending(GameObject* obj)
{
    Lavaball1bfState* p = (Lavaball1bfState*)(int*)obj->extra;
    if (p->gateA == 0)
        return;
    if (p->pending == 0)
        return;
    p->pending = 0;
}

int lavaball1bf_trySetPending(int* obj)
{
    Lavaball1bfState* p;
    obj = (int*)(int*)((GameObject*)obj)->extra;
    p = (Lavaball1bfState*)obj;
    if (p->gateA == 0)
        return 0;
    if (p->pending == 0)
    {
        p->pending = 1;
        return 1;
    }
    return 0;
}

int lavaball1bf_getExtraSize(void)
{
    return 0x1c;
}
int lavaball1bf_getObjectTypeId(void)
{
    return 0x0;
}

void lavaball1bf_free(GameObject* obj, int mode)
{
    Lavaball1bfState* inner = obj->extra;
    if (mode == 0 && inner->spawnedObj != 0)
    {
        Obj_FreeObject((GameObject*)inner->spawnedObj);
    }
}

void lavaball1bf_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible)
{
    s32 v = visible;
    if (v != 0)
        objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, 1.0f);
}

void lavaball1bf_hitDetect(void)
{
}

void lavaball1bf_update(GameObject* obj)
{
    u8* setup;
    Lavaball1bfState* state;
    int* spawned;
    f32 timer;

    state = obj->extra;
    setup = *(u8**)&obj->anim.placementData;
    state->gbState = mainGetBit(((Lavaball1bfPlacement*)setup)->stateGameBit);
    if (state->soloLatch != 0)
    {
        if (mainGetBit(((Lavaball1bfPlacement*)setup)->triggerGameBit) != 0)
        {
            state->gbState = 1;
            state->soloLatch = 0;
            state->fireTimer = 0.0f;
        }
        else
        {
            state->gbState = 0;
        }
    }
    if (*(void**)&state->spawnedObj == NULL && Obj_IsLoadingLocked() != 0)
    {
        int s = (int)Obj_AllocObjectSetup(sizeof(DimLavaProjectilePlacement), DIM_LAVA_PROJECTILE_SEQUENCE_ID);
        DimLavaProjectilePlacement* sp = (DimLavaProjectilePlacement*)s;
        sp->base.size = 9;
        sp->base.color[0] = 2;
        sp->base.color[2] = 0xff;
        sp->base.color[1] = 4;
        sp->base.color[3] = 0x50;
        sp->base.posX = obj->anim.localPosX;
        sp->base.posY = obj->anim.localPosY;
        sp->base.posZ = obj->anim.localPosZ;
        sp->launchYaw = setup[0x1c];
        sp->verticalSpeed = setup[0x1a];
        sp->horizontalSpeed = setup[0x1b];
        sp->base.mapId = ((ObjPlacement*)setup)->mapId;
        *(int*)&state->spawnedObj =
            (int)Obj_SetupObject((ObjPlacement*)s, 5, obj->anim.mapEventSlot, -1, 0);
    }
    spawned = state->spawnedObj;
    timer = state->fireTimer - timeDelta;
    state->fireTimer = timer;
    if (timer <= 0.0f &&
        ((int (*)(int*))((void**)*(void**)*(int*)&((GameObject*)spawned)->anim.dll)
             [DIM_LAVA_PROJECTILE_IS_INACTIVE_VTABLE_OFFSET / sizeof(void*)])(spawned) != 0)
    {
        if (state->gbState != 0)
        {
            int rot;
            if (mainGetBit(((Lavaball1bfPlacement*)setup)->triggerGameBit) != 0 && state->gateB == 0)
            {
                rot = setup[0x20];
                state->gateB = 1;
            }
            else
            {
                rot = setup[0x1a];
            }
            ((void (*)(int*, int, int))((void**)*(void**)*(int*)&((GameObject*)spawned)->anim.dll)
                 [DIM_LAVA_PROJECTILE_RELAUNCH_VTABLE_OFFSET / sizeof(void*)])(spawned, rot, setup[0x1b]);
        }
        state->fireTimer = state->firePeriod + (f32)(int)randomGetRange(0, 0x3c);
    }
}

void lavaball1bf_init(GameObject* obj, u8* p)
{
    Lavaball1bfState* inner;
    obj->anim.rotX = (s16)((s32)p[0x1c] << 8);
    inner = obj->extra;
    inner->firePeriod = (f32) * (s16*)(p + 0x18);
    inner->fireTimer = 0.0f;
    inner->gateA = p[0x1d];
    inner->gateB = mainGetBit((int)*(s16*)(p + 0x22));
    if (*(s16*)(p + 0x24) == -1 && inner->gateB == 0)
    {
        inner->soloLatch = 1;
    }
    obj->objectFlags |= (DIMLAVABALL_OBJFLAG_HIDDEN | DIMLAVABALL_OBJFLAG_HITDETECT_DISABLED);
}

void lavaball1bf_release(void)
{
}

void lavaball1bf_initialise(void)
{
}

ObjectDescriptor12 gLavaBall1BFObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_12_SLOTS,
    (ObjectDescriptorCallback)lavaball1bf_initialise,
    (ObjectDescriptorCallback)lavaball1bf_release,
    0,
    (ObjectDescriptorCallback)lavaball1bf_init,
    (ObjectDescriptorCallback)lavaball1bf_update,
    (ObjectDescriptorCallback)lavaball1bf_hitDetect,
    (ObjectDescriptorCallback)lavaball1bf_render,
    (ObjectDescriptorCallback)lavaball1bf_free,
    (ObjectDescriptorCallback)lavaball1bf_getObjectTypeId,
    lavaball1bf_getExtraSize,
    (ObjectDescriptorCallback)lavaball1bf_trySetPending,
    (ObjectDescriptorCallback)lavaball1bf_clearPending,
};
