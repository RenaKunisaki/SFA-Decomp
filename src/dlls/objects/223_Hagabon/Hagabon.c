/*
 * Hagabon (DLL 0xDF) - a flying baddie that patrols a rom curve path and,
 * when the player closes in, breaks off to chase. Its curve walker tracks the
 * last-seen point in a TU-local cache.
 *
 * fn_8014E1DC is the per-frame motion integrator: it advances the curve walker
 * (relinking via gRomCurveInterface when a point is exhausted), drives the
 * yaw/pitch/roll body wobble from three sine-wave phase accumulators, steers
 * the velocity toward either the player (HAGABON_FLAG_CHASE) or the curve
 * target, clamps + damps the velocity, moves the object, and turns it to face
 * the player.
 *
 * Hagabon_update handles the dormant-until-armed state (userData1): while waiting it
 * polls its placement game bit / the map-event save-time gate, then fades in.
 * Once active it fades out on a priority hit, plays the swipe/lock/creak sfx,
 * adds map time and sets the placement game bit, and re-evaluates chase state.
 */
#include "main/audio/sfx_ids.h"
#include "main/dll/objfx_api.h"
#include "main/frame_timing.h"
#include "main/vecmath_distance_api.h"
#include "main/audio/sfx.h"
#include "main/shader_api.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/dll/dll_00DF_hagabon.h"
#include "main/dll/rom_curve_interface.h"
#include "main/mapEvent.h"
#include "game/objects/object.h"
#include "sys/objects.h"
#include "main/object_render.h"
#include "main/objfx.h"
#include "main/objhits.h"
#include "main/gamebits.h"
#include "main/obj_group.h"
#include "main/mm.h"
#include "string.h"
#include "main/curve.h"
#include "main/dll/curve_walker.h"

int lbl_803DBC70[2] = {2, 3};
int gHagabonLastCurvePoint;

#define HAGABON_HIT_VOLUME_SLOT 10

/* object group this object belongs to */
#define HAGABON_OBJGROUP 3

typedef struct HagabonPlacement
{
    u8 pad0[0x14 - 0x0];
    s32 mapEventId; /* 0x14: map-event id for save-time gating / addTime */
    u8 pad18[0x19 - 0x18];
    s8 chaseRadiusScale; /* 0x19 */
    s16 curveStepRaw;    /* 0x1A */
    s16 timeReward;      /* 0x1C: minutes added to the map timer on a hit */
    s16 startInactive;   /* 0x1E: when nonzero the baddie never auto-chases */
    s16 armGameBit;      /* 0x20: -1 = none; bit that arms/latches the spawn */
    u8 pad22[0x28 - 0x22];
} HagabonPlacement;

/* HagabonState.flags */
#define HAGABON_FLAG_PATH_NEEDS_LINK 0x01
#define HAGABON_FLAG_CHASE           0x02
#define HAGABON_FLAG_PATH_RETURN     0x04
#define HAGABON_FLAG_FADE_IN         0x08
#define HAGABON_FLAG_FADE_OUT        0x10

#define HAGABON_ALPHA_MAX 255.0f
STATIC_ASSERT(sizeof(HagabonState) == 0x28);
STATIC_ASSERT(offsetof(HagabonState, wavePhaseA) == 0x20);
STATIC_ASSERT(offsetof(HagabonState, flags) == 0x26);

void fn_8014E1DC(GameObject* obj, HagabonState* state)
{
    int curve;
    GameObject* player;
    int angleDelta;
    int angle;
    u8* flags;
    char animEvents[32];
    f32 waveA;
    f32 waveB;
    f32 damp;
    f32 maxSpeed;
    f32 minSpeed;

    curve = state->curve;
    flags = &state->flags;

    if (((Curve_AdvanceAlongPath((Curve*)curve, state->curveStep) != 0) || (((RomCurveWalker*)curve)->atSegmentEnd != gHagabonLastCurvePoint)) &&
        ((*gRomCurveInterface)->goNextPoint((void*)curve) != 0) &&
        ((*gRomCurveInterface)->initCurve((void*)state->curve, (void*)obj, (400.0f), lbl_803DBC70, -1) != 0))
    {
        *flags &= ~HAGABON_FLAG_PATH_NEEDS_LINK;
    }

    gHagabonLastCurvePoint = ((RomCurveWalker*)curve)->atSegmentEnd;

    *(u16*)&state->wavePhaseA += (u16)((128.0f) * timeDelta);
    *(u16*)&state->wavePhaseB += (u16)((256.0f) * timeDelta);
    *(u16*)&state->wavePhaseC += (u16)((512.0f) * timeDelta);

    obj->anim.rotZ = (1000.0f) * (mathSinf(((3.1415927f) * (f32)(u32)state->wavePhaseA) / (32768.0f)) +
                                  mathSinf(((3.1415927f) * (f32)(u32)state->wavePhaseB) / (32768.0f)));

    obj->anim.rotY = (1000.0f) * (mathSinf(((3.1415927f) * (f32)(u32)state->wavePhaseA) / (32768.0f)) +
                                  mathSinf(((3.1415927f) * (f32)(u32)state->wavePhaseC) / (32768.0f)));

    if ((*flags & HAGABON_FLAG_CHASE) != 0)
    {
        obj->anim.velocityX +=
            (0.001f) * (state->player->anim.localPosX - obj->anim.localPosX);
        obj->anim.velocityY +=
            (0.001f) * ((60.0f + state->player->anim.localPosY) - obj->anim.localPosY);
        obj->anim.velocityZ +=
            (0.001f) * (state->player->anim.localPosZ - obj->anim.localPosZ);
    }
    else if ((*flags & HAGABON_FLAG_PATH_RETURN) != 0)
    {
        obj->anim.velocityX +=
            (0.001f) * (((RomCurveWalker*)curve)->posX - obj->anim.localPosX);
        obj->anim.velocityY +=
            (0.001f) * (((RomCurveWalker*)curve)->posY - obj->anim.localPosY);
        obj->anim.velocityZ +=
            (0.001f) * (((RomCurveWalker*)curve)->posZ - obj->anim.localPosZ);
    }
    else
    {
        obj->anim.velocityX +=
            (0.001f) * (((RomCurveWalker*)curve)->posX - obj->anim.localPosX);
        waveA = mathSinf(((3.1415927f) * (f32)(u32)state->wavePhaseB) / (32768.0f));
        waveB = mathSinf(((3.1415927f) * (f32)(u32)state->wavePhaseA) / (32768.0f));
        waveA = waveB + waveA;
        waveA = ((10.0f * waveA) + ((RomCurveWalker*)curve)->posY) - obj->anim.localPosY;
        obj->anim.velocityY += (0.001f) * waveA;
        obj->anim.velocityZ +=
            (0.001f) * (((RomCurveWalker*)curve)->posZ - obj->anim.localPosZ);
    }

    obj->anim.velocityX *= (damp = 0.9f);
    obj->anim.velocityY *= damp;
    obj->anim.velocityZ *= damp;

    if (obj->anim.velocityX > 0.5f)
    {
        obj->anim.velocityX = 0.5f;
    }
    if (obj->anim.velocityY > 0.5f)
    {
        obj->anim.velocityY = 0.5f;
    }
    if (obj->anim.velocityZ > 0.5f)
    {
        obj->anim.velocityZ = 0.5f;
    }

    if (obj->anim.velocityX < -0.5f)
    {
        obj->anim.velocityX = -0.5f;
    }
    if (obj->anim.velocityY < -0.5f)
    {
        obj->anim.velocityY = -0.5f;
    }
    if (obj->anim.velocityZ < -0.5f)
    {
        obj->anim.velocityZ = -0.5f;
    }

    objMove(obj, obj->anim.velocityX * timeDelta, obj->anim.velocityY * timeDelta,
            obj->anim.velocityZ * timeDelta);
    ObjAnim_AdvanceCurrentMove((int)obj, state->animSpeed, timeDelta,
                                                                (ObjAnimEventList*)animEvents);

    player = state->player;
    angle = (u16)getAngle(obj->anim.worldPosX - player->anim.worldPosX,
                          obj->anim.worldPosZ - player->anim.worldPosZ);
    angleDelta = angle - ((int)obj->anim.rotX & 0xffff);
    if (angleDelta > 0x8000)
    {
        angleDelta -= 0xffff;
    }
    if (angleDelta < -0x8000)
    {
        angleDelta += 0xffff;
    }

    obj->anim.rotX += (s32)(((f32)angleDelta * timeDelta) / 12.0f);
}

int Hagabon_getExtraSize(void)
{
    return 0x28;
}

int Hagabon_getObjectTypeId(void)
{
    return 0xb;
}

void Hagabon_free(int obj)
{
    void** state = ((GameObject*)obj)->extra;
    ObjGroup_RemoveObject(obj, HAGABON_OBJGROUP);
    Sfx_StopFromObject(obj, SFXTRIG_en_twiggysnap11);
    if (*state != NULL)
    {
        mm_free(*state);
        *state = NULL;
    }
}

void Hagabon_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible)
{
    HagabonState* state = *(HagabonState**)&obj->extra;
    s32 v = visible;
    if (v != 0)
    {
        switch (obj->userData1)
        {
        case 0:
            objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, 1.0f);
            if ((state->flags & HAGABON_FLAG_FADE_OUT) != 0)
            {
                objParticleFn_80099d84((GameObject*)obj, 1.0f, 3,
                                       (f32)(u32)obj->anim.alpha / HAGABON_ALPHA_MAX, 0);
            }
            if ((state->flags & HAGABON_FLAG_FADE_IN) != 0)
            {
                objParticleFn_80099d84((GameObject*)obj, 1.0f, 4,
                                       (f32)(u32)obj->anim.alpha / HAGABON_ALPHA_MAX, 0);
            }
            break;
        }
    }
}

void Hagabon_hitDetect(GameObject* obj)
{
    ObjHitsPriorityState* hitState;

    hitState = (ObjHitsPriorityState*)(obj)->anim.hitReactState;
    if (hitState->lastHitObject != 0)
    {
        Sfx_PlayFromObject((int)obj, SFXTRIG_dn_boar1_c_32b);
    }
}

void Hagabon_update(GameObject* obj)
{
    GameObject* player;
    HagabonState* state;
    int oldCurve;
    HagabonPlacement* data;
    f32 lightPos[3];
    f32 effectPos[3];
    f32 d[3];
    f32 dist;
    int hitObject;
    int hitSphereIndex;
    u32 hitVolume;
    u8 flags;

    state = (HagabonState*)obj->extra;
    oldCurve = state->curve;
    data = (HagabonPlacement*)obj->anim.placementData;

    if (obj->userData1 != 0)
    {
        if ((data->armGameBit != -1) && (mainGetBit(data->armGameBit) != 0))
        {
            return;
        }
        if ((*gMapEventInterface)->shouldNotSaveTime(data->mapEventId) == 0)
        {
            return;
        }
        obj->userData1 = 0;
        obj->anim.alpha = 1;
        state->flags |= HAGABON_FLAG_FADE_IN;
        Sfx_PlayFromObject((int)obj, SFXTRIG_dn_seal4_c);
        return;
    }

    player = Obj_GetPlayerObject();
    dist = Vec_distance(&obj->anim.worldPosX, &player->anim.worldPosX);
    if (dist < 300.0f)
    {
        Sfx_PlayFromObject((int)obj, SFXTRIG_en_twiggysnap11);
    }
    else if (dist > 350.0f)
    {
        Sfx_StopFromObject((int)obj, SFXTRIG_en_twiggysnap11);
    }

    if ((obj->anim.alpha != 0) &&
        (((flags = state->flags) & (HAGABON_FLAG_FADE_IN | HAGABON_FLAG_FADE_OUT)) != 0))
    {
        if ((flags & HAGABON_FLAG_FADE_OUT) != 0)
        {
            obj->anim.alpha = (f32)(u32)obj->anim.alpha - timeDelta;
            if (obj->anim.alpha <= 6)
            {
                obj->userData1 = 1;
                obj->anim.alpha = 0;
                state->flags &= ~HAGABON_FLAG_FADE_OUT;
                Sfx_StopFromObject((int)obj, SFXTRIG_en_twiggysnap11);
            }
            ObjHits_DisableObject(obj);
        }
        if ((state->flags & HAGABON_FLAG_FADE_IN) != 0)
        {
            obj->anim.alpha = (f32)(u32)obj->anim.alpha + timeDelta;
            if (obj->anim.alpha >= 0xf9)
            {
                obj->anim.alpha = 0xff;
                state->flags &= ~HAGABON_FLAG_FADE_IN;
            }
        }
    }
    else
    {
        if (ObjHits_GetPriorityHitWithPosition(obj, &hitObject, &hitSphereIndex, &hitVolume,
                                               &lightPos[0], &lightPos[1], &lightPos[2]) != 0)
        {
            Sfx_StopObjectChannel((int)obj, 0x7f);
            state->flags |= HAGABON_FLAG_FADE_OUT;
            Sfx_PlayFromObject((int)obj, SFXTRIG_en_rfall5_c);
            Sfx_PlayFromObject((int)obj, SFXTRIG_wp_iceywindlp16_233);
            Sfx_PlayFromObject((int)obj, SFXTRIG_dn_boar1_c_238);
            Sfx_PlayFromObject((int)obj, SFXTRIG_wp_stftest122_1f2);
            lightPos[0] += playerMapOffsetX;
            lightPos[2] += playerMapOffsetZ;
            objLightFn_8009a1dc((void*)obj, 0.014f, effectPos, 3, 0);
            (*gMapEventInterface)
                ->addTime(data->mapEventId,
                          (f32)(s32)(data->timeReward * 0x3c));
            if (data->armGameBit != -1)
            {
                mainSetBits(data->armGameBit, 1);
            }
        }
        ObjHits_SetHitVolumeSlot(&obj->anim, HAGABON_HIT_VOLUME_SLOT, 1, 0);
        ObjHits_EnableObject(obj);
    }

    state->player = Obj_GetPlayerObject();
    player = state->player;
    if (player != 0)
    {
        f32* dp = d;
        dp[0] = player->anim.worldPosX - obj->anim.worldPosX;
        dp[1] = player->anim.worldPosY - obj->anim.worldPosY;
        dp[2] = player->anim.worldPosZ - obj->anim.worldPosZ;
        state->playerDistance = sqrtf(dp[2] * dp[2] + (dp[0] * dp[0] + dp[1] * dp[1]));
    }
    if ((void*)oldCurve != NULL)
    {
        f32* dp = d;
        dp[0] = ((RomCurveWalker*)oldCurve)->posX - obj->anim.worldPosX;
        dp[1] = ((RomCurveWalker*)oldCurve)->posY - obj->anim.worldPosY;
        dp[2] = ((RomCurveWalker*)oldCurve)->posZ - obj->anim.worldPosZ;
        state->pathDistance = sqrtf(dp[2] * dp[2] + (dp[0] * dp[0] + dp[1] * dp[1]));
    }
    if (((state->flags & HAGABON_FLAG_CHASE) != 0) && (state->pathDistance > 250.0f))
    {
        state->flags &= ~HAGABON_FLAG_CHASE;
        state->flags |= HAGABON_FLAG_PATH_RETURN;
    }
    if (((state->flags & HAGABON_FLAG_PATH_RETURN) != 0) && (state->pathDistance < 30.0f))
    {
        state->flags &= ~HAGABON_FLAG_PATH_RETURN;
    }
    if (((state->flags & (HAGABON_FLAG_CHASE | HAGABON_FLAG_PATH_RETURN)) == 0) &&
        (data->startInactive == 0) && (state->player != 0) &&
        (state->playerDistance < state->chaseRadius))
    {
        state->flags |= HAGABON_FLAG_CHASE;
    }
    fn_8014E1DC(obj, state);
}

void Hagabon_init(GameObject* obj, int data, int skip_alloc)
{
    HagabonState* state = obj->extra;
    HagabonPlacement* placement = (HagabonPlacement*)data;
    state->curveStep = (f32)(s32)placement->curveStepRaw / 100.0f;
    state->animSpeed = 0.005f;
    state->chaseRadius = 4.0f * (f32)(s32)placement->chaseRadiusScale;
    if (skip_alloc == 0)
    {
        *(void**)&state->curve = mmAlloc(0x108, 0x1A, 0);
        if (*(void**)&state->curve != NULL)
        {
            memset(*(void**)&state->curve, 0, 0x108);
        }
        if ((*gRomCurveInterface)->initCurve((void*)state->curve, (void*)obj, state->chaseRadius, lbl_803DBC70, -1) ==
            0)
        {
            state->flags |= HAGABON_FLAG_PATH_NEEDS_LINK;
        }
    }
    if (placement->armGameBit != -1)
    {
        if (mainGetBit(placement->armGameBit) != 0)
        {
            obj->userData1 = 1;
        }
    }
}

void Hagabon_release(void)
{
}

void Hagabon_initialise(void)
{
}

ObjectDescriptor gHagabonObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)Hagabon_initialise,
    (ObjectDescriptorCallback)Hagabon_release,
    0,
    (ObjectDescriptorCallback)Hagabon_init,
    (ObjectDescriptorCallback)Hagabon_update,
    (ObjectDescriptorCallback)Hagabon_hitDetect,
    (ObjectDescriptorCallback)Hagabon_render,
    (ObjectDescriptorCallback)Hagabon_free,
    (ObjectDescriptorCallback)Hagabon_getObjectTypeId,
    Hagabon_getExtraSize,
};
