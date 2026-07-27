/*
 * mikaladon - the firefly-hover update handler (mikaladon_update) plus spawn-time
 * setup for the mikaladon enemy, dispatched by object seqId from the tricky
 * (DLL 0x00C4) and enemy (DLL 0x00C9) object DLLs. mikaladon_update drives a
 * circular drift, bobs between two heights, periodically drops a spawned
 * object and runs ambient sfx timers; mikaladon_init seeds the per-instance
 * speed/anim scales and the curve-path step, then places the actor at its
 * initial position along the path.
 */
#include "main/dll/partfx_interface.h"
#include "main/audio/sfx_ids.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/trig_float_helpers.h"
#include "dolphin/mtx/mtx_legacy.h"
#include "main/audio/sfx.h"
#include "main/dll/baddie_state.h"
#include "main/dll/dll_00C9_enemy.h"
#include "game/objects/object.h"
#include "sys/objects/lifecycle.h"
#include "sys/objects.h"
#include "game/objects/object_setup.h"
#include "main/vecmath.h"
#include "main/dll/curve_walker.h"
#include "main/dll/rom_curve_interface.h"
#include "main/objhits.h"
#include "main/dll/objfsa.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/frame_timing.h"
#include "main/voxmaps.h"
#include "main/dll/mikaladon.h"

#define MAGICPLANT_OBJFLAG_PARENT_SLACK 0x1000

/* Spit projectile spawned by kooshy_spawnProjectile; retail OBJECTS.bin name
   "KaldachomSp" (DLL 0xD7 kaldachompspit), shared with the snowworm spitter. */
#define KALDACHOM_SPIT_OBJ 0x51b

/* The magic-plant's one particle-fx effect (spawned per hit-count in the
   attack handler). */
#define MAGICPLANT_PARTFX          0x802
#define MAGICPLANT_HIT_VOLUME_SLOT 0xe

/* gcRobotPatrol (mikaladon_update): periodically dropped object; parented back to
 * the dropper via +0xC4 and announced with SFX 0x249. */
#define SEQOBJ11E_GCROBOT_DROP_OBJ 0x6b5

enum MikaladonVerticalPhase
{
    MIKALADON_PHASE_ORBIT = 0,
    MIKALADON_PHASE_DESCEND = 1,
    MIKALADON_PHASE_ASCEND = 2
};

#define MIKALADON_ORBIT_ANGLE_SPEED     75.0f
#define MIKALADON_TRIGGER_RADIUS_SCALE  1.3f
#define MIKALADON_DESCENT_SPEED         0.5f
#define MIKALADON_DESCENT_DISTANCE      500.0f
#define MIKALADON_ASCENT_SPEED          1.5f
#define MIKALADON_DROP_INTERVAL         100
#define MIKALADON_DROP_HEIGHT_OFFSET    5.0f
#define MIKALADON_AMBIENT_SFX_MIN_DELAY 60
#define MIKALADON_AMBIENT_SFX_MAX_DELAY 120

static f32 mikaladon_randomAmbientSfxDelay(void)
{
    return (f32)(int)randomGetRange(MIKALADON_AMBIENT_SFX_MIN_DELAY, MIKALADON_AMBIENT_SFX_MAX_DELAY);
}

static void mikaladon_spawnDrop(GameObject* obj)
{
    MikaladonDropSetup* setup;
    GameObject* spawned;

    setup = (MikaladonDropSetup*)Obj_AllocObjectSetup(sizeof(MikaladonDropSetup), SEQOBJ11E_GCROBOT_DROP_OBJ);
    setup->base.posX = obj->anim.localPosX;
    setup->base.posY = MIKALADON_DROP_HEIGHT_OFFSET + obj->anim.localPosY;
    setup->base.posZ = obj->anim.localPosZ;
    setup->base.color[0] = 1;
    setup->base.color[1] = 1;
    setup->base.color[2] = 0xff;
    setup->base.color[3] = 0xff;
    spawned = loadObjectAtObject(obj, &setup->base);
    if (spawned != NULL)
    {
        spawned->ownerObj = obj;
        Sfx_PlayFromObject((u32)obj, SFXTRIG_id_249);
    }
}

/* mikaladon_update: firefly hover update: circle drift, bob between heights,
 * periodically drop a spawned object, ambient sfx timers. */
void mikaladon_update(GameObject* obj, MikaladonState* state)
{
    f32 y;
    f32 sinOut;
    f32 cosOut;

    state->actor.orbitAngle =
        MIKALADON_ORBIT_ANGLE_SPEED * timeDelta + (f32)(u32)state->actor.orbitAngle;
    fn_80293018(state->actor.orbitAngle, &sinOut, &cosOut);
    sinOut = sinOut * ((BaddieState*)state)->unk2A8 + state->actor.orbitCenterX;
    cosOut = cosOut * ((BaddieState*)state)->unk2A8 + state->actor.orbitCenterZ;
    if (state->actor.verticalPhase == MIKALADON_PHASE_ORBIT)
    {
        f32 dx;
        f32 dz;

        y = obj->anim.localPosY;
        dx = state->actor.orbitCenterX -
             ((GameObject*)((BaddieState*)state)->trackedObj)->anim.localPosX;
        dz = state->actor.orbitCenterZ -
             ((GameObject*)((BaddieState*)state)->trackedObj)->anim.localPosZ;
        if (sqrtf(dx * dx + dz * dz) <= MIKALADON_TRIGGER_RADIUS_SCALE * ((BaddieState*)state)->unk2A8)
        {
            state->actor.verticalPhase = MIKALADON_PHASE_DESCEND;
            state->actor.dropTimer = 0;
        }
    }
    else if (state->actor.verticalPhase == MIKALADON_PHASE_DESCEND)
    {
        y = obj->anim.localPosY - MIKALADON_DESCENT_SPEED * timeDelta;
        if (y <= state->actor.homeY - MIKALADON_DESCENT_DISTANCE)
        {
            state->actor.verticalPhase = MIKALADON_PHASE_ASCEND;
        }
        else
        {
            state->actor.dropTimer =
                (f32)(u32)state->actor.dropTimer + timeDelta;
            if (state->actor.dropTimer > MIKALADON_DROP_INTERVAL)
            {
                state->actor.dropTimer = 0;
                if (Obj_IsLoadingLocked() != 0)
                {
                    MikaladonDropSetup* setup;
                    GameObject* spawned;

                    setup = (MikaladonDropSetup*)Obj_AllocObjectSetup(sizeof(MikaladonDropSetup),
                                                                     SEQOBJ11E_GCROBOT_DROP_OBJ);
                    setup->base.posX = obj->anim.localPosX;
                    setup->base.posY = MIKALADON_DROP_HEIGHT_OFFSET + obj->anim.localPosY;
                    setup->base.posZ = obj->anim.localPosZ;
                    setup->base.color[0] = 1;
                    setup->base.color[1] = 1;
                    setup->base.color[2] = 0xff;
                    setup->base.color[3] = 0xff;
                    spawned = loadObjectAtObject(obj, &setup->base);
                    if (spawned != NULL)
                    {
                        spawned->ownerObj = obj;
                        Sfx_PlayFromObject((u32)obj, SFXTRIG_id_249);
                    }
                }
            }
        }
    }
    else
    {
        y = MIKALADON_ASCENT_SPEED * timeDelta + obj->anim.localPosY;
        if (y >= state->actor.homeY)
        {
            state->actor.verticalPhase = MIKALADON_PHASE_ORBIT;
        }
    }
    obj->anim.velocityX = oneOverTimeDelta * (sinOut - obj->anim.localPosX);
    obj->anim.velocityY = oneOverTimeDelta * (y - obj->anim.localPosY);
    obj->anim.velocityZ = oneOverTimeDelta * (cosOut - obj->anim.localPosZ);
    baddieTurnTowardLookDir(obj, state, 0xf, 7.5f, 1.0f, 0);
    state->actor.ambientSfxTimer -= timeDelta;
    if (state->actor.ambientSfxTimer <= gMikaladonZero)
    {
        state->actor.ambientSfxTimer =
            (f32)(int)randomGetRange(MIKALADON_AMBIENT_SFX_MIN_DELAY, MIKALADON_AMBIENT_SFX_MAX_DELAY);
        Sfx_PlayFromObject((u32)obj, SFXTRIG_id_31);
    }
    state->actor.loopSfxTimer -= timeDelta;
    if (state->actor.loopSfxTimer <= gMikaladonZero)
    {
        state->actor.loopSfxTimer = gMikaladonDefaultPeriod;
        Sfx_PlayFromObject((u32)obj, SFXTRIG_id_24a);
    }
}

void mikaladon_init(GameObject* obj, MikaladonState* state)
{
    f32 zero;
    f32 lblA;
    f32 a, b;

    zero = gMikaladonDefaultPeriod;
    ((BaddieState*)state)->speedScale = zero;
    ((BaddieState*)state)->unk2E4 = 1;
    ((BaddieState*)state)->unk308 = 0.01f;
    ((BaddieState*)state)->animDeltaScale = 0.006f;
    lblA = 1.0f;
    ((BaddieState*)state)->unk304 = lblA;
    ((BaddieState*)state)->unk320 = 1;
    *(f32*)&((BaddieState*)state)->eventFlags = lblA;
    ((BaddieState*)state)->unk321 = 3;
    ((BaddieState*)state)->unk318 = lblA;
    ((BaddieState*)state)->unk322 = 1;
    ((BaddieState*)state)->unk31C = lblA;
    state->actor.orbitCenterX = obj->anim.localPosX;
    state->actor.homeY = obj->anim.localPosY;
    state->actor.orbitCenterZ = obj->anim.localPosZ;
    state->actor.verticalPhase = MIKALADON_PHASE_ORBIT;
    state->actor.dropTimer = 0;
    state->actor.orbitAngle = 0;
    state->actor.loopSfxTimer = zero;
    state->actor.ambientSfxTimer = zero;
    ((BaddieState*)state)->pathStep = 8.0f;

    fn_80293018(state->actor.orbitAngle, &a, &b);
    obj->anim.localPosX =
        a * ((BaddieState*)state)->unk2A8 + state->actor.orbitCenterX;
    obj->anim.localPosZ =
        b * ((BaddieState*)state)->unk2A8 + state->actor.orbitCenterZ;
}
