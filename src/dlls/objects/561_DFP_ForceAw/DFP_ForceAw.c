/*
 * tesla / "Tricky curve" trigger object.
 *
 * A box-shaped trigger volume centred on the object that watches the
 * player's offset from the object on each axis. When the player is inside
 * the half-extents on all three axes (insideAxes == 3) the object reacts:
 *  - TrickyCurve_updateCooldownHit is the cooldown variant: throttled by state->cooldown
 *    (decremented by timeDelta, reset to TRICKY_CURVE_COOLDOWN_TICKS after
 *    a hit). A sliding player (player anim state ==
 *    TRICKY_CURVE_PLAYER_ANIM_SLIDE) sets the hit game bit and spawns the
 *    cooldown partfx; otherwise the player takes a recorded hit. Either
 *    way a sfx plays.
 *  - TrickyCurve_updateBurstHit is the burst variant: spawns a directional burst partfx
 *    (TrickyCurveBurstPartfxArgs carries the player-relative deltas and an
 *    x-rotation flip when the player crosses the x midline) and, off the
 *    slide path, messages the player and plays the burst sfx. The
 *    gTrickyCurveBurstCounter gates the bit/sfx to once every
 *    TRICKY_CURVE_BURST_LIMIT ticks while sliding.
 *
 * Both variants cache the entry side per axis (xSide/ySide/zSide) in the
 * trigger state so the burst variant can detect a midline crossing.
 */
#include "main/dll/partfx_interface.h"
#include "main/gamebits.h"
#include "main/audio/sfx.h"
#include "sys/objects.h"
#include "game/objects/object.h"
#include "main/objhits.h"
#include "main/dll/player_api.h"
#include "main/obj_message.h"
#include "main/frame_timing.h"
#include "main/dll/tesla.h"
#include "main/dll/infopoint.h"
#include "main/audio/sfx_ids.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/dll_000A_expgfx.h"
#include "game/objects/object_setup.h"
#include "main/dll/trickycurve_state.h"
#include "main/dll/TrickyCurve.h"
#include "main/mapEvent.h"
#include "main/dll/sfxplayer.h"
#include "main/gamebit_ids.h"
#include "dlls/object_descriptor.h"
#include "main/vecmath.h"

#define TRICKY_CURVE_GAMEBIT_HIT       0x468
#define TRICKY_CURVE_PLAYER_ANIM_SLIDE 0x1d7
#define TRICKY_CURVE_COOLDOWN_TICKS    200
#define TRICKY_CURVE_BURST_LIMIT       0x14
#define TRICKY_CURVE_HIT_PRIORITY      0x14
#define TRICKY_CURVE_MESSAGE_BURST     0x60004
#define TRICKY_CURVE_PARTFX_COOLDOWN   0x397
#define TRICKY_CURVE_PARTFX_BURST      0x399
#define TRICKY_CURVE_SFX_BURST         0x1c9
#define TRICKY_CURVE_SFX_COOLDOWN      0x1ca

typedef struct TrickyCurveTriggerState
{
    s16 xExtent;  /* 0x00: half-extent of the trigger box on each axis */
    s16 zExtent;  /* 0x02 */
    s16 yExtent;  /* 0x04 */
    s16 cooldown; /* 0x06: ticks until the cooldown variant can hit again */
    u8 unk8[8];   /* 0x08: unknown */
    u8 xSide;     /* 0x10: which side of the midline the player entered on */
    u8 ySide;     /* 0x11 */
    u8 zSide;     /* 0x12 */
} TrickyCurveTriggerState;

typedef struct TrickyCurveBurstPartfxArgs
{
    s16 xRot;
    s16 yRot;
    s16 zRot;
    f32 scale;
    f32 xDelta;
    f32 yDelta;
    f32 zDelta;
} TrickyCurveBurstPartfxArgs;

STATIC_ASSERT(offsetof(TrickyCurveTriggerState, cooldown) == 0x06);
STATIC_ASSERT(offsetof(TrickyCurveTriggerState, xSide) == 0x10);
STATIC_ASSERT(sizeof(TrickyCurveTriggerState) == 0x14);
STATIC_ASSERT(offsetof(TrickyCurveBurstPartfxArgs, scale) == 0x08);
STATIC_ASSERT(offsetof(TrickyCurveBurstPartfxArgs, xDelta) == 0x0C);


u8
    gTrickyCurveBurstCounter; /* inter-frame burst-fire counter; reset to 0 after TRICKY_CURVE_BURST_LIMIT ticks */

void TrickyCurve_updateCooldownHit(GameObject* obj)
{
    u8 insideAxes;
    TrickyCurveTriggerState* state;
    GameObject* player;
    u8 xSide;
    u8 ySide;
    u8 zSide;
    f32 xDelta;
    f32 zDelta;
    f32 yDelta;

    state = (TrickyCurveTriggerState*)obj->extra;
    player = (GameObject*)Obj_GetPlayerObject();
    insideAxes = 0;
    xSide = 0;
    ySide = 0;
    zSide = 0;

    xDelta = player->anim.localPosX - obj->anim.localPosX;
    yDelta = player->anim.localPosY - obj->anim.localPosY;
    zDelta = player->anim.localPosZ - obj->anim.localPosZ;

    if (xDelta <= 0.0f)
    {
        if (xDelta > -(f32)state->xExtent)
        {
            insideAxes = 1;
            xSide = 1;
        }
    }
    if (xDelta > 0.0f)
    {
        if (xDelta < state->xExtent)
        {
            insideAxes++;
            xSide--;
        }
    }
    if (zDelta <= 0.0f)
    {
        if (zDelta > -(f32)state->zExtent)
        {
            insideAxes++;
            zSide = 1;
        }
    }
    if (zDelta > 0.0f)
    {
        if (zDelta < state->zExtent)
        {
            insideAxes++;
            zSide--;
        }
    }
    if (yDelta <= 0.0f)
    {
        if (yDelta > -(f32)state->yExtent)
        {
            insideAxes++;
            ySide = 1;
        }
    }
    if (yDelta > 0.0f)
    {
        if (yDelta < state->yExtent)
        {
            insideAxes++;
            ySide--;
        }
    }

    if (state->cooldown >= 0)
    {
        state->cooldown -= (s16)timeDelta;
    }
    if (insideAxes == 3 && state->cooldown <= 0)
    {
        if (objGetAnimState80A((GameObject*)player) == TRICKY_CURVE_PLAYER_ANIM_SLIDE)
        {
            mainSetBits(TRICKY_CURVE_GAMEBIT_HIT, 1);
            (*gPartfxInterface)->spawnObject(player, TRICKY_CURVE_PARTFX_COOLDOWN, NULL, 2, -1, NULL);
        }
        else
        {
            ObjHits_RecordObjectHit(player, NULL, TRICKY_CURVE_HIT_PRIORITY, 2, 0);
        }
        Sfx_PlayFromObject((u32)player, TRICKY_CURVE_SFX_COOLDOWN);
        state->cooldown = TRICKY_CURVE_COOLDOWN_TICKS;
    }

    state->xSide = xSide;
    state->ySide = ySide;
    state->zSide = zSide;
}

void TrickyCurve_updateBurstHit(GameObject* obj)
{
    u8 insideAxes;
    TrickyCurveTriggerState* state;
    GameObject* player;
    u8 xSide;
    u8 ySide;
    u8 zSide;
    f32 xDelta;
    f32 zDelta;
    f32 yDelta;
    TrickyCurveBurstPartfxArgs partfxArgs;

    state = (TrickyCurveTriggerState*)obj->extra;
    player = (GameObject*)Obj_GetPlayerObject();
    insideAxes = 0;
    xSide = 0;
    ySide = 0;
    zSide = 0;

    xDelta = player->anim.localPosX - obj->anim.localPosX;
    yDelta = player->anim.localPosY - obj->anim.localPosY;
    zDelta = player->anim.localPosZ - obj->anim.localPosZ;
    gTrickyCurveBurstCounter++;

    if (xDelta <= 0.0f)
    {
        if (xDelta > -(f32)state->xExtent)
        {
            insideAxes = 1;
            xSide = 1;
        }
    }
    if (xDelta > 0.0f)
    {
        if (xDelta < state->xExtent)
        {
            insideAxes++;
            xSide--;
        }
    }
    if (zDelta <= 0.0f)
    {
        if (zDelta > -(f32)state->zExtent)
        {
            insideAxes++;
            zSide = 1;
        }
    }
    if (zDelta > 0.0f)
    {
        if (zDelta < state->zExtent)
        {
            insideAxes++;
            zSide--;
        }
    }
    if (yDelta <= 0.0f)
    {
        if (yDelta > -(f32)state->yExtent)
        {
            insideAxes++;
            ySide = 1;
        }
    }
    if (yDelta > 0.0f)
    {
        if (yDelta < state->yExtent)
        {
            insideAxes++;
            ySide--;
        }
    }

    if (insideAxes == 3)
    {
        partfxArgs.xDelta = xDelta;
        partfxArgs.yDelta = yDelta;
        partfxArgs.zDelta = zDelta;
        partfxArgs.scale = 1.0f;
        partfxArgs.zRot = 0;
        partfxArgs.yRot = 0;
        partfxArgs.xRot = 0;
        if (xSide != state->xSide)
        {
            partfxArgs.xRot = 0x3fff;
        }

        if (objGetAnimState80A((GameObject*)player) == TRICKY_CURVE_PLAYER_ANIM_SLIDE)
        {
            if (gTrickyCurveBurstCounter > TRICKY_CURVE_BURST_LIMIT)
            {
                gTrickyCurveBurstCounter = 0;
                mainSetBits(TRICKY_CURVE_GAMEBIT_HIT, 1);
                Sfx_PlayFromObject((u32)obj, TRICKY_CURVE_SFX_BURST);
            }
            (*gPartfxInterface)->spawnObject(player, TRICKY_CURVE_PARTFX_COOLDOWN, NULL, 2, -1, NULL);
        }
        else
        {
            mainSetBits(TRICKY_CURVE_GAMEBIT_HIT, 1);
            ObjMsg_SendToObject(player, TRICKY_CURVE_MESSAGE_BURST, obj, 2);
            (*gPartfxInterface)->spawnObject(obj, TRICKY_CURVE_PARTFX_BURST, &partfxArgs, 2, -1, NULL);
            Sfx_PlayFromObject((u32)obj, TRICKY_CURVE_SFX_BURST);
        }
    }

    state->xSide = xSide;
    state->ySide = ySide;
    state->zSide = zSide;
}

/*
 * TrickyCurve (DLL 0xEC) - axis-aligned box trigger that applies a random
 * velocity kick to the player when they enter the volume. Used for the
 * "Tricky curve" current-like push zone. The box half-extents are stored in
 * TrickyCurveState; the object's world position is the centre. On each update
 * the function counts how many of the three axis intervals contain the player
 * (requires all three = axisCount 3) then fires a random horizontal nudge.
 */
typedef struct TrickyCurveState
{
    s16 halfWidthX;
    s16 halfWidthZ;
    s16 halfHeightY;
} TrickyCurveState;

void TrickyCurve_updateCooldownTrigger(int obj)
{
    GameObject* curve;
    TrickyCurveState* state;
    GameObject* player;
    int axisCount;
    f32 deltaX;
    f32 deltaZ;
    f32 deltaY;
    f32 bound;
    f32 randomX;
    f32 randomZ;

    curve = (GameObject*)obj;
    state = (TrickyCurveState*)curve->extra;
    player = Obj_GetPlayerObject();
    axisCount = 0;
    deltaX = player->anim.localPosX - curve->anim.localPosX;
    deltaY = player->anim.localPosY - curve->anim.localPosY;
    deltaZ = player->anim.localPosZ - curve->anim.localPosZ;

    if (deltaX <= 0.0f)
    {
        bound = state->halfWidthX;
        if (deltaX > -bound)
        {
            axisCount = 1;
        }
    }
    if (deltaX > 0.0f)
    {
        bound = state->halfWidthX;
        if (deltaX < bound)
        {
            axisCount = axisCount + 1;
        }
    }

    if (deltaZ <= 0.0f)
    {
        bound = state->halfWidthZ;
        if (deltaZ > -bound)
        {
            axisCount = axisCount + 1;
        }
    }
    if (deltaZ > 0.0f)
    {
        bound = state->halfWidthZ;
        if (deltaZ < bound)
        {
            axisCount = axisCount + 1;
        }
    }

    if (deltaY <= 0.0f)
    {
        bound = state->halfHeightY;
        if (deltaY > -bound)
        {
            axisCount = axisCount + 1;
        }
    }
    if (deltaY > 0.0f)
    {
        bound = state->halfHeightY;
        if (deltaY < bound)
        {
            axisCount = axisCount + 1;
        }
    }

    if ((u8)axisCount == 3)
    {
        randomX = 0.01f * randomGetRange(-0x17, 0x17);
        randomZ = 0.01f * randomGetRange(-0x17, 0x17);
        fn_802960E4((int)player, randomX, randomZ);
    }
    return;
}

/*
 * DragonRock Palace force-field object (DLL 0x231; "DFP_ForceAw"),
 * implemented on the shared TrickyCurve state machine and sfxplayer: a
 * curve-driven hazard/barrier with per-state update handlers.
 */
typedef struct TrickyCurveObjectDef
{
    ObjPlacement head; /* 0x00 */
    s8 rangeYRaw; /* 0x18 << 2 -> state.rangeY */
    u8 pad19[0x1A - 0x19];
    s16 rangeX;         /* 0x1A -> state.rangeX (X-axis half-extent) */
    s16 rangeZ;         /* 0x1C -> state.rangeZ */
    s16 triggerGameBit; /* 0x1E -> state.triggerGameBit */
    s16 gateGameBit;    /* 0x20 -> state.gateGameBit */
    u8 pad22[0x28 - 0x22];
} TrickyCurveObjectDef;

typedef struct TrickyCurveBurstFxParams
{
    s16 rotX;
    s16 rotY;
    s16 rotZ;
    s16 pad;
    f32 scale;
    f32 xOffset;
    f32 yOffset;
    f32 zOffset;
} TrickyCurveBurstFxParams;

#define DFPFORCEAW_OBJFLAG_HITDETECT_DISABLED 0x2000
#define DFPFORCEAW_MSG_PLAYER_BURST           0x60004 /* knock the player back with a burst hit */

/* partfx ids spawned on the player-burst trigger: single burst flash plus a
 * 10-count spray of burst particles (same shape in both mainGetBit(0x1d9) arms) */
#define DFPFORCEAW_PARTFX_BURST          0x5ed /* spawned once */
#define DFPFORCEAW_PARTFX_BURST_PARTICLE 0x5fd /* spawned 10x */

void TrickyCurve_updateBurstTrigger(GameObject* obj)
{
    u8* state;
    int player;
    f32 dx;
    f32 dz;
    f32 dy;
    u8 insideCount;
    u8 xSide;
    u8 ySide;
    u8 zSide;
    TrickyCurveBurstFxParams fxParams;
    int burstParticles;

    state = (obj)->extra;
    player = (int)Obj_GetPlayerObject();
    insideCount = 0;
    xSide = 0;
    ySide = 0;
    zSide = 0;
    dx = ((GameObject*)player)->anim.localPosX - (obj)->anim.localPosX;
    dy = ((GameObject*)player)->anim.localPosY - (obj)->anim.localPosY;
    dz = ((GameObject*)player)->anim.localPosZ - (obj)->anim.localPosZ;

    if ((((TrickyCurveObjState*)state)->gateGameBit != -1) &&
        (mainGetBit(((TrickyCurveObjState*)state)->gateGameBit) != 0))
    {
        return;
    }

    if (mainGetBit(((TrickyCurveObjState*)state)->triggerGameBit) != 0)
    {
        mainSetBits(((TrickyCurveObjState*)state)->triggerGameBit, 0);
    }

    if (dx <= 0.0f)
    {
        if (dx > -(f32) * (s16*)state)
        {
            insideCount = 1;
            xSide = 1;
        }
    }
    if (dx > 0.0f)
    {
        if (dx < (f32) * (s16*)state)
        {
            insideCount++;
            xSide--;
        }
    }
    if (dz <= 0.0f)
    {
        if (dz > -(f32)((TrickyCurveObjState*)state)->rangeZ)
        {
            insideCount++;
            zSide = 1;
        }
    }
    if (dz > 0.0f)
    {
        if (dz < (f32)((TrickyCurveObjState*)state)->rangeZ)
        {
            insideCount++;
            zSide--;
        }
    }
    if (dy <= 0.0f)
    {
        if (dy > -(f32)((TrickyCurveObjState*)state)->rangeY)
        {
            insideCount++;
            ySide = 1;
        }
    }
    if (dy > 0.0f)
    {
        if (dy < (f32)((TrickyCurveObjState*)state)->rangeY)
        {
            insideCount++;
            ySide--;
        }
    }

    if (insideCount == 3)
    {
        fxParams.xOffset = dx;
        fxParams.yOffset = dy;
        fxParams.zOffset = dz;
        fxParams.scale = 1.0f;
        fxParams.rotZ = 0;
        fxParams.rotY = 0;
        fxParams.rotX = 0;
        if (xSide != state[0x10])
        {
            fxParams.rotX = 0x3fff;
        }

        if (mainGetBit(0x1d9) != 0)
        {
            mainSetBits(GAMEBIT_TRICKYCURVE_PLAYER_HIT, 1);
            ObjMsg_SendToObject((void*)player, DFPFORCEAW_MSG_PLAYER_BURST, obj, 0);
            (*gPartfxInterface)->spawnObject((void*)obj, DFPFORCEAW_PARTFX_BURST, &fxParams, 2, -1, NULL);
            burstParticles = 9;
            do
            {
                (*gPartfxInterface)->spawnObject((void*)obj, DFPFORCEAW_PARTFX_BURST_PARTICLE, &fxParams, 2, -1, NULL);
            } while (burstParticles-- != 0);
        }
        else
        {
            ObjMsg_SendToObject((void*)player, DFPFORCEAW_MSG_PLAYER_BURST, obj, 1);
            (*gPartfxInterface)->spawnObject((void*)obj, DFPFORCEAW_PARTFX_BURST, &fxParams, 2, -1, NULL);
            burstParticles = 9;
            do
            {
                (*gPartfxInterface)->spawnObject((void*)obj, DFPFORCEAW_PARTFX_BURST_PARTICLE, &fxParams, 2, -1, NULL);
            } while (burstParticles-- != 0);
        }
        mainSetBits(((TrickyCurveObjState*)state)->triggerGameBit, 1);
        Sfx_PlayFromObject((int)obj, SFXTRIG_wp_fball2_c_1c9);
    }

    state[0x10] = xSide;
    state[0x11] = ySide;
    state[0x12] = zSide;
}

int TrickyCurve_getExtraSize(void)
{
    return 0x14;
}
int TrickyCurve_getObjectTypeId(void)
{
    return 0x0;
}

void TrickyCurve_free(GameObject* obj)
{
    (*gExpgfxInterface)->freeSource2((u32)obj);
}

void TrickyCurve_render(void)
{
}

void TrickyCurve_hitDetect(void)
{
}

void TrickyCurve_update(GameObject* obj)
{
    u8* inner = obj->extra;
    u32 state = inner[0xe];
    if (state == 0)
    {
        TrickyCurve_updateBurstTrigger(obj);
    }
    else if (state == 1)
    {
        TrickyCurve_updateCooldownTrigger((int)obj);
    }
    else if (state == 2)
    {
        TrickyCurve_updateBurstHit(obj);
    }
    else if (state == 3)
    {
        TrickyCurve_updateCooldownHit(obj);
    }
}

void TrickyCurve_init(GameObject* obj, u8* def)
{
    u8* state = obj->extra;
    state[0xc] = def[0x19];
    ((TrickyCurveObjState*)state)->rangeY = (s16)((s32)((TrickyCurveObjectDef*)def)->rangeYRaw << 2);
    *(s16*)state = ((TrickyCurveObjectDef*)def)->rangeX;
    ((TrickyCurveObjState*)state)->rangeZ = ((TrickyCurveObjectDef*)def)->rangeZ;
    state[0xe] = def[0x19];
    state[0x10] = 0;
    state[0x11] = 0;
    state[0x12] = 0;
    ((TrickyCurveObjState*)state)->gateGameBit = ((TrickyCurveObjectDef*)def)->gateGameBit;
    ((TrickyCurveObjState*)state)->triggerGameBit = ((TrickyCurveObjectDef*)def)->triggerGameBit;
    ((TrickyCurveObjState*)state)->cooldown = 0;
    obj->objectFlags = (u16)(obj->objectFlags | DFPFORCEAW_OBJFLAG_HITDETECT_DISABLED);
}

void TrickyCurve_release(void)
{
}

void TrickyCurve_initialise(void)
{
}

ObjectDescriptor gTrickyCurveObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)TrickyCurve_initialise,
    (ObjectDescriptorCallback)TrickyCurve_release,
    0,
    (ObjectDescriptorCallback)TrickyCurve_init,
    (ObjectDescriptorCallback)TrickyCurve_update,
    (ObjectDescriptorCallback)TrickyCurve_hitDetect,
    (ObjectDescriptorCallback)TrickyCurve_render,
    (ObjectDescriptorCallback)TrickyCurve_free,
    (ObjectDescriptorCallback)TrickyCurve_getObjectTypeId,
    TrickyCurve_getExtraSize,
};
