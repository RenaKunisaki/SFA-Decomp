/*
 * DIMSnowBall (DLL 0x1C2) - timed snowball spawner for Dinosaur Island
 * Mission.  On each timer expiry, if loading is not locked and the player
 * is clear, allocates a 36-byte setup for rolling-snowball object 406,
 * seeds it from the placement params, and resets the spawn countdown.
 */
#include "dlls/objects/449_DIMSnowBall.h"

#include "main/frame_timing.h"
#include "main/dll/player_api.h"
#include "main/vecmath.h"
#include "game/objects/object.h"
#include "main/object_render.h"
#include "sys/objects/lifecycle.h"
#include "sys/objects.h"
#include "dlls/object_descriptor.h"
#include "game/objects/object_setup.h"
/* Child object periodically spawned by dimsnowball1c2_update. */
#define DIMSNOWBALL1C2_CHILD_OBJ 406

typedef struct Dimsnowball1c2State
{
    s16 countdown;
    s16 spawnPeriod;
} Dimsnowball1c2State;

typedef struct Dimsnowball1c2Placement
{
    ObjPlacement head;
    s16 initialCountdown; /* init: copied to extra (spawnPeriod + countdown) */
    u8 childRot; /* copied to spawned child placement 0x1A (rotation) */
    u8 childZOffset; /* base for spawned child placement 0x1C (+random) */
    union {
        s8 rotX; /* copied to child placement 0x18 */
        u8 rotXUnsigned; /* shifted into the parent's anim.rotX */
    };
    u8 pad1D[0x1E - 0x1D];
    s16 unk1E;
} Dimsnowball1c2Placement;

STATIC_ASSERT(sizeof(Dimsnowball1c2State) == 0x4);
STATIC_ASSERT(offsetof(Dimsnowball1c2Placement, initialCountdown) == 0x18);
STATIC_ASSERT(offsetof(Dimsnowball1c2Placement, childRot) == 0x1A);
STATIC_ASSERT(offsetof(Dimsnowball1c2Placement, childZOffset) == 0x1B);
STATIC_ASSERT(offsetof(Dimsnowball1c2Placement, rotX) == 0x1C);
STATIC_ASSERT(offsetof(Dimsnowball1c2Placement, unk1E) == 0x1E);
int dimsnowball1c2_getExtraSize(void)
{
    return sizeof(Dimsnowball1c2State);
}

int dimsnowball1c2_getObjectTypeId(void)
{
    return 0x0;
}

void dimsnowball1c2_free(void)
{
}

void dimsnowball1c2_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible)
{
    s32 v = visible;
    if (v != 0)
    {
        objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, 1.0f);
    }
}

void dimsnowball1c2_hitDetect(void)
{
}

void dimsnowball1c2_update(GameObject* obj)
{
    if (Obj_IsLoadingLocked())
    {
        Dimsnowball1c2State* state = obj->extra;
        if ((state->countdown -= framesThisStep) <= 0)
        {
            if (playerGetFocusObject(Obj_GetPlayerObject()) == NULL)
            {
                DimSnowBallPlacement* setup;
                Dimsnowball1c2Placement* placement;
                placement = (Dimsnowball1c2Placement*)obj->anim.placementData;
                setup =
                    (DimSnowBallPlacement*)Obj_AllocObjectSetup(sizeof(DimSnowBallPlacement), DIMSNOWBALL1C2_CHILD_OBJ);
                setup->base.color[0] = placement->head.color[0];
                setup->base.color[2] = placement->head.color[2];
                setup->base.color[1] = placement->head.color[1];
                setup->base.color[3] = placement->head.color[3];
                setup->base.posX = obj->anim.localPosX;
                setup->base.posY = obj->anim.localPosY;
                setup->base.posZ = obj->anim.localPosZ;
                setup->base.mapId = placement->head.mapId;
                {
                    int rotX = placement->rotX;
                    setup->rotationXByte = rotX;
                }
                setup->rotationParam1A = placement->childRot;
                setup->rotationParam1C =
                    (f32)(u32)placement->childZOffset +
                    (f32)(int)randomGetRange(0, 100) / 100.0f;
                Obj_SetupObject(&setup->base, 5, obj->anim.mapEventSlot, -1, 0);
                state->countdown = state->spawnPeriod;
            }
        }
    }
}

void dimsnowball1c2_init(GameObject* obj, Dimsnowball1c2Placement* placement)
{
    Dimsnowball1c2State* state;
    obj->anim.rotX = (s16)((u32)placement->rotXUnsigned << 8);
    state = obj->extra;
    state->spawnPeriod = placement->initialCountdown;
    state->countdown = placement->initialCountdown;
    obj->objectFlags |= (OBJECT_OBJFLAG_HIDDEN | OBJECT_OBJFLAG_HITDETECT_DISABLED);
}

void dimsnowball1c2_release(void)
{
}

void dimsnowball1c2_initialise(void)
{
}

ObjectDescriptor gDIMSnowBall1C2ObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)dimsnowball1c2_initialise,
    (ObjectDescriptorCallback)dimsnowball1c2_release,
    0,
    (ObjectDescriptorCallback)dimsnowball1c2_init,
    (ObjectDescriptorCallback)dimsnowball1c2_update,
    (ObjectDescriptorCallback)dimsnowball1c2_hitDetect,
    (ObjectDescriptorCallback)dimsnowball1c2_render,
    (ObjectDescriptorCallback)dimsnowball1c2_free,
    (ObjectDescriptorCallback)dimsnowball1c2_getObjectTypeId,
    dimsnowball1c2_getExtraSize,
};
