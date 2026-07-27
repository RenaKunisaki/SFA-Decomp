/*
 * dimwooddoor - spinning wooden-door hazard found in DIM (Dinosaur
 * InfernoMountain).  Each door tracks the player and flings debris
 * shards (type 0x1d6) at them.  DIMwooddoor_updateFallingDebris
 * integrates a shard under gravity and explodes it on contact;
 * DIMwooddoor_spawnShard creates one shard projectile per trigger;
 * DIMwooddoor_updateShardAim computes the launch angle and speed from
 * the door's current aim state.
 */
#include "main/audio/sfx_ids.h"
#include "main/vecmath.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/audio/sfx.h"
#include "game/objects/object_setup.h"
#include "game/objects/object.h"
#include "sys/objects/lifecycle.h"
#include "main/objprint_api.h"
#include "sys/objects.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/dll/DIM/DIMwooddoor.h"
#include "main/objfx.h"
#include "main/gamebits.h"
#include "main/objhits.h"
#include "main/frame_timing.h"
#include "main/gamebit_ids.h"

#define DIMWOODDOOR_OBJFLAG_PARENT_SLACK 0x1000
#define DIMWOODDOOR_CHILD_OBJ_SHARD      0x1d6 /* debris shard projectile spawned by DIMwooddoor_spawnShard */

typedef struct DIMWoodDoorConfig
{
    u8 pad00[0x4];
    u8 setup04;
    u8 setup05;
    u8 setup06;
    u8 setup07;
    u8 pad08[0x20];
    s8 angleBias;
    u8 delayMin;
    u8 delayMax;
    u8 targetRadius;
} DIMWoodDoorConfig;

typedef struct DIMWoodDoorState
{
    u8 pad00[0x4];
    f32 posX;
    f32 posY;
    f32 posZ;
    u8 pad10[0x7c];
    f32 targetX;
    f32 targetY;
    f32 targetZ;
    f32 launchSpeed;
    u8 pad9c[0x8];
    s16 launchDelay;
    s16 cooldown;
    u8 padA8[0x4];
    u8 setupId;
    u8 shouldSpawnShard;
} DIMWoodDoorState;

typedef struct DIMWoodDoorShardState
{
    GameObject* parent;
    u8 variant;
    u8 lifetime;
    u8 hitRadius;
} DIMWoodDoorShardState;

extern s16 lbl_803DBF02;
extern s16 lbl_803DBF04;
extern f32 lbl_803DBEF0;
extern f32 lbl_803DBF14;

enum DIMwooddoorDebrisState
{
    DIMWOODDOOR_DEBRIS_STATE_FALLING = 0, /* fall under gravity, spin, watch for impact */
    DIMWOODDOOR_DEBRIS_STATE_EXPLODED = 1 /* exploded + hidden, awaiting despawn timer   */
};

typedef struct DIMwooddoorUpdateFallingDebrisState
{
    u8 pad0[0x1 - 0x0];
    u8 unk1;
    s16 unk2;
    u8 pad4[0x5 - 0x4];
    u8 hitboxRadius;
    u8 hitVolumeSlot;
    u8 unk7;
    u8 state;
    s8 rotZRate;
    s8 rotYRate;
    s8 rotXRate;
    u8 padC[0x10 - 0xC];
} DIMwooddoorUpdateFallingDebrisState;

#define DLL801B1D84_HIT_VOLUME_SLOT 5

/* DIMwooddoor_updateFallingDebris: integrate the falling debris under gravity, spin it, and on
 * contact (or scripted trigger) fire the explosion and start the despawn timer. */

void DIMwooddoor_updateFallingDebris(GameObject* obj)
{
    DIMwooddoorUpdateFallingDebrisState* extra = obj->extra;
    switch (extra->state)
    {
    case DIMWOODDOOR_DEBRIS_STATE_FALLING:
    {
        f32 oldvy = obj->anim.velocityY;
        f32 grav = 0.01f * -lbl_803DBEF0;
        f32 midVel;
        ObjHitsPriorityState* hitState;
        obj->anim.velocityY = grav * timeDelta + oldvy;
        midVel = 0.5f * (oldvy + obj->anim.velocityY);
        objMove(obj, obj->anim.velocityX * timeDelta, midVel * timeDelta,
                obj->anim.velocityZ * timeDelta);
        obj->anim.rotZ =
            obj->anim.rotZ + extra->rotZRate * 10;
        obj->anim.rotY =
            obj->anim.rotY + extra->rotYRate * 10;
        obj->anim.rotX = obj->anim.rotX + extra->rotXRate * 10;
        hitState = (ObjHitsPriorityState*)obj->anim.hitReactState;
        if (hitState != NULL)
        {
            int* vol;
            ObjHits_SetHitVolumeSlot(&obj->anim, DLL801B1D84_HIT_VOLUME_SLOT,
                                     extra->hitVolumeSlot, 0);
            vol = (int*)hitState->lastHitObject;
            if (vol != NULL && vol != *(int**)extra)
            {
                ObjHitbox_SetSphereRadius(&obj->anim,
                                          extra->hitboxRadius);
                spawnExplosion(obj, 50.0f, 2, 1, 0, 1, 1, 1, 0);
                obj->userData1 = 1180;
                *(s8*)&extra->state = DIMWOODDOOR_DEBRIS_STATE_EXPLODED;
                obj->anim.flags |= OBJANIM_FLAG_HIDDEN;
            }
        }
        if ((mainGetBit(GAMEBIT_DIM2_CannonRelated085E) != 0 && mainGetBit(GAMEBIT_CannonRelated0C2D) == 0) ||
            (mainGetBit(GAMEBIT_DIM2_CannonRelated0874) != 0 && mainGetBit(GAMEBIT_CannonRelated0C2E) == 0))
        {
            obj->userData1 = 1200;
        }
        if (((ObjHitsPriorityState*)obj->anim.hitReactState)->contactFlags != 0)
        {
            ObjHitbox_SetSphereRadius(&obj->anim,
                                      extra->hitboxRadius);
            spawnExplosion(obj, 50.0f, 2, 1, 0, 1, 1, 1, 0);
            obj->userData1 = 1180;
            *(s8*)&extra->state = DIMWOODDOOR_DEBRIS_STATE_EXPLODED;
            obj->anim.flags |= OBJANIM_FLAG_HIDDEN;
        }
        break;
    }
    case DIMWOODDOOR_DEBRIS_STATE_EXPLODED:
        break;
    }
    obj->userData1 = obj->userData1 + framesThisStep;
    if (obj->userData1 > 1200)
    {
        Obj_FreeObject(obj);
    }
    else if (extra->unk7 != 0)
    {
        *(s8*)&extra->unk7 = 0;
    }
}

void DIMwooddoor_spawnShard(GameObject* obj, u8 variant)
{
    DIMWoodDoorConfig* config;
    DIMWoodDoorState* state;
    DIMWoodDoorShardState* shardState;
    s16* modelVec;
    u8* setup;
    GameObject* shard;
    f32 launchSpeed;
    f32 launchScale;
    f32 angle;
    int objHandle = (int)obj;

    config = *(DIMWoodDoorConfig**)&obj->anim.placementData;
    if (Obj_IsLoadingLocked() == 0 || (state = obj->extra)->shouldSpawnShard == 0 ||
        state->launchDelay > 0)
    {
        return;
    }

    modelVec = objModelGetVecFn_800395d8(obj, 0);
    setup = (u8*)Obj_AllocObjectSetup(0x24, DIMWOODDOOR_CHILD_OBJ_SHARD);
    setup[4] = config->setup04;
    setup[6] = config->setup06;
    setup[5] = config->setup05;
    setup[7] = config->setup07;
    ((ObjPlacement*)setup)->posX = state->targetX;
    ((ObjPlacement*)setup)->posY = state->targetY;
    ((ObjPlacement*)setup)->posZ = state->targetZ;

    shard = Obj_SetupObject((ObjPlacement*)setup, 5, obj->anim.mapEventSlot, -1, 0);
    shardState = shard->extra;
    shardState->parent = obj;
    shardState->variant = variant;
    if (variant != 0)
    {
        if (obj->anim.mapEventSlot == 0x1b)
        {
            shardState->lifetime = 100;
        }
        else
        {
            shardState->lifetime = 60;
        }
        shardState->hitRadius = 100;
    }
    else
    {
        shardState->lifetime = 20;
        shardState->hitRadius = 1;
    }

    launchSpeed = state->launchSpeed;
    launchScale = 2.0f * launchSpeed;
    shard->anim.rotX = obj->anim.rotX + modelVec[1];
    angle = (3.1415927f * (f32)(s32) * (s16*)shard) / 32768.0f;
    shard->anim.velocityX = launchScale * -mathSinf(angle);
    shard->anim.velocityY = launchSpeed;
    angle = (3.1415927f * (f32)(s32) * (s16*)shard) / 32768.0f;
    shard->anim.velocityZ = launchScale * -mathCosf(angle);

    state->shouldSpawnShard = 0;
    state->cooldown = 50;
    if (state->setupId == 3)
    {
        state->launchDelay = 50;
    }
    else
    {
        state->launchDelay = (s16)(randomGetRange(config->delayMin, config->delayMax) << 2);
    }

    ObjAnim_SetCurrentMove(objHandle, 0, 0.0f, 0);
    Sfx_PlayFromObject(objHandle, SFXTRIG_tr_jrumbalp);
}

void DIMwooddoor_updateShardAim(GameObject* obj, f32 targetX, f32 targetY, f32 targetZ, f32 distance)
{
    DIMWoodDoorState* state;
    DIMWoodDoorConfig* config;
    s16* modelVec;
    int player;
    f32 dx;
    f32 dz;
    f32 distSq;
    f32 dist;
    f32 heightDelta;
    f32 radiusSq;
    f32 accel;
    f32 accelDenom;
    register int facingAngle;
    int angleDelta;
    int pitchSign;
    int turnSign;
    s16 pitch;
    int turnStep;
    s16 absPitch;

    config = *(DIMWoodDoorConfig**)&(obj)->anim.placementData;
    player = (int)Obj_GetPlayerObject();
    state = (obj)->extra;
    if (state->cooldown <= 0)
    {
        modelVec = objModelGetVecFn_800395d8(obj, 0);
        facingAngle = modelVec[1] + ((s32)config->angleBias << 8);
        targetX -= (obj)->anim.localPosX;
        targetZ -= (obj)->anim.localPosZ;
        angleDelta = ((u16)getAngle(targetX, targetZ) + 0x8000);
        angleDelta = angleDelta - (u16)facingAngle;
        if (angleDelta > 0x8000)
        {
            angleDelta -= 0xffff;
        }
        if (angleDelta < -0x8000)
        {
            angleDelta += 0xffff;
        }
        if ((angleDelta < 0x1200) && (angleDelta > -0x1200))
        {
            state->shouldSpawnShard = 1;
        }
        if (angleDelta > 0x800)
        {
            angleDelta = 0x800;
        }
        if (angleDelta < -0x800)
        {
            angleDelta = -0x800;
        }
        turnStep = angleDelta >> 3;
        if (turnStep != 0)
        {
            pitch = modelVec[1];
            absPitch = (pitch < 0) ? -pitch : pitch;
            if ((s32)absPitch > (s32)lbl_803DBF02 - lbl_803DBF04)
            {
                turnSign = (turnStep < 0) ? -1 : ((turnStep > 0) ? 1 : 0);
                pitchSign = (modelVec[1] < 0) ? -1 : ((modelVec[1] > 0) ? 1 : 0);
                if (pitchSign == turnSign)
                {
                    turnStep *= lbl_803DBF02 - (s32)absPitch;
                    turnStep /= lbl_803DBF04;
                }
            }
            modelVec[1] = (s16)(*(s16*)((char*)modelVec + 2) + turnStep);
        }

        dx = state->targetX - state->posX;
        dz = state->targetZ - state->posZ;
        distSq = dx * dx + dz * dz;
        dist = sqrtf(distSq);
        heightDelta = (10.0f + state->posY) - state->targetY;
        distSq = (distSq < 10.0f) ? 10.0f : distSq;
        if ((distSq < (f32)((s32)(config->targetRadius * 2) * (s32)(config->targetRadius * 2))) ||
            (heightDelta < lbl_803DBF14) ||
            ((((GameObject*)player)->objectFlags & DIMWOODDOOR_OBJFLAG_PARENT_SLACK) != 0))
        {
            state->shouldSpawnShard = 0;
        }
        distSq = (distSq > (f32)((s32)(config->targetRadius * 2) * (s32)(config->targetRadius * 2)))
                     ? distSq
                     : (f32)((s32)(config->targetRadius * 2) * (s32)(config->targetRadius * 2));

        accel = (0.01f * -lbl_803DBEF0) * distSq;
        accelDenom = 8.0f * heightDelta - 4.0f * dist;
        accel = accel / ((accelDenom < -1.0f) ? accelDenom : -1.0f);
        accel = (0.0f > accel) ? 0.0f : accel;
        accel = sqrtf(accel);
        state->launchSpeed += (accel - state->launchSpeed) / 80.0f;
    }
}
