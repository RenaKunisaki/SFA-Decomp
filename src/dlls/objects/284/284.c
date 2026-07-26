/*
 * DLL 0x11C implements staff-activated scenery and mechanisms. Placement
 * mode selects action, lift, and hit-reaction behaviours; the same TU also
 * contains the lift, game-bit mirror, and map-event debris helpers.
 */
#include "main/dll/partfx_interface.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_trig_api.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/dll/staffflags_struct.h"
#include "main/dll/player_api.h"
#include "game/objects/object.h"
#include "main/dll/CF/staffactivated_helpers.h"
#include "main/audio/sfx.h"
#include "main/audio/sfx_ids.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/dll/dll_80136a40.h"
#include "main/frame_timing.h"
#include "main/mapEventTypes.h"
#include "main/objseq.h"
#include "main/objhits.h"
#include "main/obj_group.h"
#include "main/pad.h"
#include "main/vecmath.h"
#include "sys/objects.h"
#include "sys/objects/lifecycle.h"
#include "main/object_render.h"
#include "main/gamebits.h"
#include "dlls/object_descriptor.h"
#include "main/dll/dll_011B_landedarwing.h"

STATIC_ASSERT(sizeof(StaffActivatedState) == 0x24);
STATIC_ASSERT(offsetof(StaffActivatedState, targetX) == 0x00);
STATIC_ASSERT(offsetof(StaffActivatedState, targetZ) == 0x04);
STATIC_ASSERT(offsetof(StaffActivatedState, liftVelocity) == 0x0c);
STATIC_ASSERT(offsetof(StaffActivatedState, previousLiftHeight) == 0x10);
STATIC_ASSERT(offsetof(StaffActivatedState, liftHeight) == 0x14);
STATIC_ASSERT(offsetof(StaffActivatedState, peakLiftHeight) == 0x18);
STATIC_ASSERT(offsetof(StaffActivatedState, liftReset) == 0x1c);
STATIC_ASSERT(offsetof(StaffActivatedState, flags) == 0x1d);
STATIC_ASSERT(offsetof(StaffActivatedState, hitCooldown) == 0x20);
STATIC_ASSERT(sizeof(StaffActivatedSetup) == 0x28);
STATIC_ASSERT(offsetof(StaffActivatedSetup, type) == 0x18);
STATIC_ASSERT(offsetof(StaffActivatedSetup, mode) == 0x1c);
STATIC_ASSERT(offsetof(StaffActivatedSetup, size) == 0x1d);
STATIC_ASSERT(offsetof(StaffActivatedSetup, debrisObjectSet) == 0x1e);
STATIC_ASSERT(offsetof(StaffActivatedSetup, debrisCount) == 0x1f);
STATIC_ASSERT(offsetof(StaffActivatedSetup, timedEventSeconds) == 0x20);
STATIC_ASSERT(offsetof(StaffActivatedSetup, activeGameBit) == 0x22);
STATIC_ASSERT(offsetof(StaffActivatedSetup, lockGameBit) == 0x24);

#define STAFFACTIVATED_OBJ_FLAG_HIT_TRIGGER 0x04
#define STAFFACTIVATED_OBJ_FLAG_LOCKED      0x08
#define STAFFACTIVATED_OBJ_FLAG_DISABLED    0x10

#define STAFFACTIVATED_MODE_ACTION        0
#define STAFFACTIVATED_MODE_LIFT          2
#define STAFFACTIVATED_MODE_HIT_REACTION  3
#define STAFFACTIVATED_MODE_DAMAGE_FIRST  4
#define STAFFACTIVATED_MODE_DAMAGE_SECOND 5

#define STAFFACTIVATED_TRIGGER_GAMEBIT 0xd2a
#define STAFFACTIVATED_ENABLE_GAMEBIT  0x957
#define STAFFACTIVATED_PARTICLE_ID     0x7c3

#define STAFFACTIVATED_OBJ_GROUP 0x41

extern const f32 lbl_803E3BBC;
extern const f32 lbl_803E3BC4;

s16 lbl_803DBDE0[4] = {0x3D3, 0x3D4, 0x3D5, 0x3D6};

typedef struct PrisonGuardStateFlags
{
    u8 pad[0x1d];
    u8 active : 1;
    u8 locked : 1;
    u8 mirror : 1;
} PrisonGuardStateFlags;

typedef struct PrisonGuardRotationWork
{
    s16 y;
    s16 x;
    s16 z;
    s16 pad;
    f32 scale;
    f32 tx;
    f32 ty;
    f32 tz;
} PrisonGuardRotationWork;

void staffactivated_updateLiftHeight(GameObject* obj, StaffActivatedState* state)
{
    u32 flags;
    s32 prevHeight;
    s32 rumbleStrength;

    flags = state->flags;
    if ((flags >> 7 & 1) == 0u || (flags >> 6 & 1) != 0u)
    {
        return;
    }
    if (state->liftReset == 0)
    {
        state->liftVelocity = (s32) - (4.0f * timeDelta - state->liftVelocity);
        state->liftHeight = (s32)((f32)state->liftVelocity * timeDelta + state->liftHeight);
        if (state->liftHeight > state->peakLiftHeight)
        {
            state->peakLiftHeight = state->liftHeight;
        }
        if (state->previousLiftHeight == 0x800 && state->liftHeight < 0x800)
        {
            Sfx_PlayFromObject((int)obj, SFXTRIG_mammoth_grunt);
        }
        if (state->liftHeight < 0)
        {
            if (state->previousLiftHeight > 0)
            {
                Sfx_PlayFromObject((int)obj, SFXTRIG_mammoth_grunt1);
                rumbleStrength = state->peakLiftHeight / 200;
                if (rumbleStrength > 0)
                {
                    doRumble((f32)rumbleStrength);
                }
            }
            state->liftVelocity = 0;
            state->liftHeight = 0;
        }
    }
    else
    {
        state->liftReset = 0;
        state->peakLiftHeight = 0;
    }

    prevHeight = state->previousLiftHeight;
    if ((prevHeight < 0x40 && state->liftHeight >= 0x40) || (prevHeight >= 0x40 && state->liftHeight < 0x40))
    {
        Sfx_PlayFromObject((int)obj, SFXTRIG_mammoth_grunt);
    }
    ObjHits_PollPriorityHitEffectWithCooldown(obj, 8, 0xb4, 0xf0, 0xff, 0x6f, &state->hitCooldown);
    state->previousLiftHeight = state->liftHeight;
    ObjAnim_SetMoveProgress((ObjAnimComponent*)obj, state->liftHeight / 2048.0f);
}

void cfPrisonGuard_setGameBitMirror(GameObject* obj, u8 flag)
{
    StaffActivatedSetup* setup = (StaffActivatedSetup*)obj->anim.placementData;
    StaffActivatedState* state = obj->extra;
    if (flag != 0)
    {
        mainSetBits(setup->lockGameBit, 1);
        ((PrisonGuardStateFlags*)state)->mirror = 1;
    }
    else
    {
        mainSetBits(setup->lockGameBit, 0);
        ((PrisonGuardStateFlags*)state)->mirror = 0;
    }
}

int cfPrisonGuard_isGameBitMirrorSet(GameObject* obj)
{
    return (((StaffActivatedState*)obj->extra)->flags >> 5) & 1;
}

void staffactivated_spawnMapEventDebris(GameObject* obj)
{
    int i;
    StaffActivatedSetup* setup;
    int player;
    u32 tricky;
    StaffActivatedState* state;
    int spawnedSetup;
    int spawnedObj;
    ObjPlacement* spawnedPlacement;
    f32 zero;
    f32 lenSq;
    f32 len;
    s32 yawDelta;
    PrisonGuardRotationWork rotate;

    setup = (StaffActivatedSetup*)obj->anim.placementData;
    player = (int)Obj_GetPlayerObject();
    tricky = (u32)getTrickyObject();
    state = obj->extra;

    if ((*gMapEventInterface)->shouldNotSaveTime(setup->base.mapId) != 0 && Obj_IsLoadingLocked() != 0)
    {
        (*gMapEventInterface)->addTime(setup->base.mapId, 60.0f * setup->timedEventSeconds);
        if (tricky != 0)
        {
            trickyImpress((GameObject*)tricky);
        }

        zero = 0.0f;
        i = 0;
        while (i < setup->debrisCount)
        {
            spawnedSetup = (int)Obj_AllocObjectSetup(0x24, lbl_803DBDE0[setup->debrisObjectSet]);
            spawnedPlacement = (ObjPlacement*)spawnedSetup;
            spawnedPlacement->posX = state->targetX;
            spawnedPlacement->posY = obj->anim.localPosY;
            spawnedPlacement->posZ = state->targetZ;
            *(s16*)((StaffActivatedSetup*)spawnedPlacement)->pad1A = 0x190;

            spawnedObj = (int)Obj_SetupObject((ObjPlacement*)spawnedSetup, 5, obj->anim.mapEventSlot, -1,
                                              *(void**)&obj->anim.parent);
            ((GameObject*)spawnedObj)->anim.velocityX = obj->anim.localPosX - *(f32*)(player + 0xc);
            ((GameObject*)spawnedObj)->anim.velocityZ = obj->anim.localPosZ - *(f32*)(player + 0x14);

            lenSq = (((GameObject*)spawnedObj)->anim.velocityX * ((GameObject*)spawnedObj)->anim.velocityX) +
                    (((GameObject*)spawnedObj)->anim.velocityZ * ((GameObject*)spawnedObj)->anim.velocityZ);
            if (lenSq != zero)
            {
                len = sqrtf(lenSq);
                ((GameObject*)spawnedObj)->anim.velocityX = ((GameObject*)spawnedObj)->anim.velocityX / len;
                ((GameObject*)spawnedObj)->anim.velocityZ = ((GameObject*)spawnedObj)->anim.velocityZ / len;
            }

            ((GameObject*)spawnedObj)->anim.velocityX =
                ((GameObject*)spawnedObj)->anim.velocityX *
                (lbl_803E3BBC - (lbl_803E3BC4 * (f32)(int)randomGetRange(0, 0x19)));
            ((GameObject*)spawnedObj)->anim.velocityZ =
                ((GameObject*)spawnedObj)->anim.velocityZ *
                (lbl_803E3BBC - (lbl_803E3BC4 * (f32)(int)randomGetRange(0, 0x19)));
            ((GameObject*)spawnedObj)->anim.velocityY = 2.2f;

            rotate.tx = zero;
            rotate.ty = zero;
            rotate.tz = zero;
            rotate.scale = lbl_803E3BBC;
            rotate.z = 0;
            rotate.x = 0;
            rotate.y = randomGetRange(-10000, 10000);
            vecRotateZXY((s16*)&rotate, (f32*)(spawnedObj + 0x24));

            yawDelta = ((GameObject*)spawnedObj)->anim.rotX -
                       (u16)getAngle(((GameObject*)spawnedObj)->anim.velocityX,
                                     -((GameObject*)spawnedObj)->anim.velocityZ);
            if (yawDelta > 0x8000)
            {
                yawDelta -= 0xffff;
            }
            if (yawDelta < -0x8000)
            {
                yawDelta += 0xffff;
            }
            ((GameObject*)spawnedObj)->anim.rotX = yawDelta;
            i++;
        }
    }
}

u32 cfPrisonGuard_getPullRateMode(GameObject* obj)
{
    u32 mode;
    mode = ((StaffActivatedSetup*)obj->anim.placementData)->size;
    if (mode > 2)
        mode = 2;
    return mode;
}

void staffactivated_calcInteractionTargetXZ(GameObject* obj, f32* outX, f32* outZ)
{
    int mode;
    StaffActivatedState* state;
    GameObject* gobj;

    gobj = obj;
    state = gobj->extra;
    mode = ((StaffActivatedSetup*)gobj->anim.placementData)->mode;

    switch (mode)
    {
    case STAFFACTIVATED_MODE_LIFT:
        *outX = -(20.0f * mathSinf(3.1415927f * (f32)(gobj->anim.rotX) / 32768.0f) -
                  state->targetX);
        *outZ = -(20.0f * mathCosf(3.1415927f * (f32)(gobj->anim.rotX) / 32768.0f) -
                  state->targetZ);
        break;
    case STAFFACTIVATED_MODE_HIT_REACTION:
        *outX = 20.0f * mathSinf(3.1415927f * (f32)(gobj->anim.rotX) / 32768.0f) +
                state->targetX;
        *outZ = 20.0f * mathCosf(3.1415927f * (f32)(gobj->anim.rotX) / 32768.0f) +
                state->targetZ;
        break;
    case STAFFACTIVATED_MODE_ACTION:
        *outX = 18.0f * mathSinf(3.1415927f * (f32)(gobj->anim.rotX) / 32768.0f) +
                gobj->anim.localPosX;
        *outZ = 18.0f * mathCosf(3.1415927f * (f32)(gobj->anim.rotX) / 32768.0f) +
                gobj->anim.localPosZ;
        break;
    default:
        *outX = 20.0f * mathSinf(3.1415927f * (f32)(gobj->anim.rotX) / 32768.0f) +
                gobj->anim.localPosX;
        *outZ = 20.0f * mathCosf(3.1415927f * (f32)(gobj->anim.rotX) / 32768.0f) +
                gobj->anim.localPosZ;
        break;
    }
}

u32 cfPrisonGuard_getLiftHeight(GameObject* obj)
{
    StaffActivatedState* state = obj->extra;
    return state->liftHeight;
}

void cfPrisonGuard_setLiftHeight(GameObject* obj, int height)
{
    StaffActivatedState* state = obj->extra;
    state->liftHeight = height;
    state->liftReset = 1;
}

u8 objGetByteParam1C(GameObject* obj)
{
    StaffActivatedSetup* setup = (StaffActivatedSetup*)obj->anim.placementData;
    return setup->mode;
}

int staffactivated_getExtraSize(void)
{
    return sizeof(StaffActivatedState);
}

int staffactivated_getObjectTypeId(void)
{
    return 0x40;
}

void staffactivated_free(int obj)
{
    ObjGroup_RemoveObject(obj, STAFFACTIVATED_OBJ_GROUP);
}

void staffactivated_render(int p1, int p2, int p3, int p4, int p5, s8 visible)
{
    objRenderModelAndHitVolumes((GameObject*)p1, p2, p3, p4, p5, lbl_803E3BBC);
}

void staffactivated_update(GameObject* obj)
{
    struct PartfxParams
    {
        int pad;
        s16 life;
        s16 extra;
        f32 scale;
        f32 posX;
        f32 posY;
        f32 posZ;
    } stk;
    StaffActivatedSetup* setup = (StaffActivatedSetup*)obj->anim.placementData;
    StaffActivatedState* state = obj->extra;
    GameObject* player;
    int isSet;
    int gameBit;

    player = Obj_GetPlayerObject();

    if (((StaffFlags*)&state->flags)->b6)
    {
        obj->anim.resetHitboxFlags |= STAFFACTIVATED_OBJ_FLAG_LOCKED;
    }
    else
    {
        obj->anim.resetHitboxFlags &= ~STAFFACTIVATED_OBJ_FLAG_LOCKED;
    }

    if (((StaffFlags*)&state->flags)->b7 == 0 || playerIsPathFollowing(player) == 0)
    {
        obj->anim.resetHitboxFlags |= STAFFACTIVATED_OBJ_FLAG_DISABLED;
    }
    else
    {
        obj->anim.resetHitboxFlags &= ~STAFFACTIVATED_OBJ_FLAG_DISABLED;
    }

    switch (setup->mode)
    {
    case STAFFACTIVATED_MODE_LIFT:
        staffactivated_updateLiftHeight((GameObject*)obj, state);
        break;
    case STAFFACTIVATED_MODE_HIT_REACTION:
        landed_arwing_updateHitReaction(obj, (LandedArwingObjectState*)state);
        break;
    case STAFFACTIVATED_MODE_DAMAGE_FIRST:
    case STAFFACTIVATED_MODE_DAMAGE_SECOND:
        landed_arwing_updateDamageTexture(obj, (LandedArwingObjectState*)state);
        break;
    case STAFFACTIVATED_MODE_ACTION:
        if (obj->anim.resetHitboxFlags & STAFFACTIVATED_OBJ_FLAG_HIT_TRIGGER)
        {
            if (mainGetBit(STAFFACTIVATED_TRIGGER_GAMEBIT) == 0)
            {
                (*gObjectTriggerInterface)->runSequence(0, (void*)obj, -1);
                mainSetBits(STAFFACTIVATED_TRIGGER_GAMEBIT, 1);
            }
        }
        if (mainGetBit(STAFFACTIVATED_ENABLE_GAMEBIT) == 0)
        {
            obj->anim.resetHitboxFlags |= STAFFACTIVATED_OBJ_FLAG_DISABLED;
        }
        isSet = 0;
        gameBit = setup->activeGameBit;
        if (gameBit == -1 || mainGetBit(gameBit) != 0)
        {
            isSet = 1;
        }
        ((StaffFlags*)&state->flags)->b7 = isSet;
        if (((StaffFlags*)&state->flags)->b7)
        {
            stk.posX = 2.8f;
            stk.posY = 1.7f;
            stk.posZ = 0.0f;
            stk.scale = lbl_803E3BBC;
            stk.extra = 0;
            stk.life = 0x64;
            (*gPartfxInterface)->spawnObject((void*)obj, STAFFACTIVATED_PARTICLE_ID, &stk, 2, -1, NULL);
            stk.posX = 2.8f;
            stk.posY = 1.7f;
            stk.posZ = 0.0f;
            stk.scale = lbl_803E3BBC;
            stk.extra = 5;
            stk.life = 0xa;
            (*gPartfxInterface)->spawnObject((void*)obj, STAFFACTIVATED_PARTICLE_ID, &stk, 2, -1, NULL);
        }
        break;
    default:
        isSet = 0;
        gameBit = setup->activeGameBit;
        if (gameBit == -1 || mainGetBit(gameBit) != 0)
        {
            isSet = 1;
        }
        ((StaffFlags*)&state->flags)->b7 = isSet;
        break;
    }
}

void staffactivated_init(GameObject* obj, StaffActivatedSetup* setupData)
{
    StaffActivatedState* state;
    int sizeIndex;
    int modelVariant;
    f32 scale;
    StaffFlags* flags;

    state = obj->extra;
    ObjGroup_AddObject((u32)obj, STAFFACTIVATED_OBJ_GROUP);
    obj->anim.rotX = (s16)((s32)setupData->type << 8);

    sizeIndex = setupData->size;
    if (sizeIndex > 2)
    {
        sizeIndex = 2;
    }

    if (setupData->mode == STAFFACTIVATED_MODE_LIFT)
    {
        switch (sizeIndex)
        {
        case 2:
            modelVariant = 2;
            scale = 1.25f;
            break;
        default:
            modelVariant = 1;
            scale = lbl_803E3BBC;
            break;
        case 0:
            modelVariant = 0;
            scale = 0.75f;
            break;
        }
    }
    else
    {
        scale = lbl_803E3BBC;
    }

    if (obj->anim.hitReactState != NULL)
    {
        ObjHitbox_SetSphereRadius((ObjAnimComponent*)obj,
                                  (int)((f32)((ObjHitsPriorityState*)obj->anim.hitReactState)->primaryRadius * scale));
    }

    obj->anim.rootMotionScale = obj->anim.modelInstance->rootMotionScaleBase * scale;
    if (obj->anim.rootMotionScale < 0.1f)
    {
        obj->anim.rootMotionScale = 0.1f;
    }

    switch (setupData->mode)
    {
    case STAFFACTIVATED_MODE_LIFT:
        obj->hitVolumeIndex = modelVariant;
        state->targetX = -(0.5f * (obj->anim.rootMotionScale *
                                           (10.0f * mathSinf((3.1415927f * (f32)obj->anim.rotX) /
                                                                    32768.0f))) -
                           obj->anim.localPosX);
        state->targetZ = -(0.5f * (obj->anim.rootMotionScale *
                                           (10.0f * mathCosf((3.1415927f * (f32)obj->anim.rotX) /
                                                                    32768.0f))) -
                           obj->anim.localPosZ);
        break;
    case STAFFACTIVATED_MODE_HIT_REACTION:
        state->targetX =
            0.5f *
                (obj->anim.rootMotionScale *
                 (10.0f * mathSinf((3.1415927f * (f32)obj->anim.rotX) / 32768.0f))) +
            obj->anim.localPosX;
        state->targetZ =
            0.5f *
                (obj->anim.rootMotionScale *
                 (10.0f * mathCosf((3.1415927f * (f32)obj->anim.rotX) / 32768.0f))) +
            obj->anim.localPosZ;
        break;
    default:
        state->targetX = obj->anim.localPosX;
        state->targetZ = obj->anim.localPosZ;
        break;
    }

    flags = (StaffFlags*)&state->flags;
    if (setupData->activeGameBit > 0)
    {
        flags->b7 = mainGetBit(setupData->activeGameBit);
    }
    else
    {
        flags->b7 = 1;
    }
    flags->b4 = 0;

    if (setupData->lockGameBit > 0)
    {
        if ((flags->b6 = mainGetBit(setupData->lockGameBit)) != 0)
        {
            switch (setupData->mode)
            {
            case STAFFACTIVATED_MODE_HIT_REACTION:
                ObjAnim_SetMoveProgress((ObjAnimComponent*)obj, lbl_803E3BBC);
                break;
            case STAFFACTIVATED_MODE_DAMAGE_FIRST:
                flags->b6 = 0;
                break;
            case STAFFACTIVATED_MODE_LIFT:
                break;
            case STAFFACTIVATED_MODE_DAMAGE_SECOND:
                break;
            }
        }
    }
}

ObjectDescriptor gStaffActivatedObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    0,
    0,
    0,
    (ObjectDescriptorCallback)staffactivated_init,
    (ObjectDescriptorCallback)staffactivated_update,
    0,
    (ObjectDescriptorCallback)staffactivated_render,
    (ObjectDescriptorCallback)staffactivated_free,
    (ObjectDescriptorCallback)staffactivated_getObjectTypeId,
    staffactivated_getExtraSize,
};
