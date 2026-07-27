/*
 * DIMCannon (DLL 0x1C6) - DIM lava cannon; a stationary turret that tracks
 * and fires cannonballs at the player, with a manned-control mode (fireState 3)
 * in which the player aims with the stick, charges with A, and fires on release.
 * The 0x1D6 sub-variant is a falling-debris prop shared with DIMwooddoor.
 */
#include "main/dll/DIM/dimcannon_state.h"
#include "main/frame_timing.h"
#include "main/vecmath.h"
#include "sys/objects.h"
#include "main/dll/player_api.h"
#include "main/camera_interface.h"
#include "main/game_ui_interface.h"
#include "game/objects/object.h"
#include "dlls/object_descriptor.h"
#include "main/object_render.h"
#include "main/objprint_api.h"
#include "main/audio/sfx.h"
#include "main/gamebits.h"
#include "main/dll/DIM/DIMlevcontrol.h"
#include "main/dll/player_status.h"
#include "main/objseq.h"
#include "main/resource.h"
#include "main/pad.h"
#include "main/dll/DIM/dll_01C6_dimcannon.h"
#include "main/dll/DIM/DIMwooddoor.h"
#include "main/dll/tricky_api.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/obj_group.h"
#include "main/obj_message.h"
#include "main/obj_path.h"
#include "game/objects/object_setup.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/audio/sfx_ids.h"
#include "main/gamebit_ids.h"
#include "main/objfx.h"
#include "main/objhits.h"
#include "sys/objects/lifecycle.h"

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


f32 lbl_803DBEF0 = 6.0f;
f32 gDimCannonAnimAdvanceSpeedCur = 0.025f;
f32 lbl_803DBEF8 = 1.0f;
f32 lbl_803DBEFC = 0.04f;
u8 gDimCannonMaxCharge = 100;
s16 lbl_803DBF02 = 14000;
s16 lbl_803DBF04 = 1000;
f32 gDimCannonAimStickScale = 2.5f;
int lbl_803DBF0C = 164025;
int lbl_803DBF10 = 152100;
f32 lbl_803DBF14 = -300.0f;

#define PAD_BUTTON_A                         0x100
#define PAD_BUTTON_B                         0x200
#define DIMCANNON_OBJGROUP                   3
#define DIMCANNON_BALL_OBJ                   0x1d6
#define DIMCANNON_AIRMETER_BGTEXTURE         0x5d5 /* HUD air-meter background texture id */
#define DIMCANNON_OBJFLAG_HIDDEN             0x4000
#define DIMCANNON_OBJFLAG_HITDETECT_DISABLED 0x2000

/* Camera mode ids passed to setMode() (== the target camera-mode DLL number). */
#define CAMMODE_CANNON  0x51 /* dll_0051_cameramodecannon */
#define CAMMODE_DEFAULT 0x42 /* dll_0042 - default/release camera */

extern f32 lbl_803E48E8;
STATIC_ASSERT(sizeof(DimCannonState) == 0xb4);
void* lbl_803DDB50;

extern f32 lbl_803E48B8;

extern f32 lbl_803E48EC;
extern f32 gDimCannonAnimAdvanceSpeed;
typedef struct DimcannonPlacement
{
    ObjPlacement base;
    u8 pad18[0x1A - 0x18];
    s16 resetGameBit;
    s16 armGameBit;
    s16 holdGameBit;
    u8 pad20[0x26 - 0x20];
    s16 triggerRange;
    s8 unk28;
    u8 pad29[0x30 - 0x29];
} DimcannonPlacement;
typedef struct DimcannonState
{
    u8 pad0[0x7 - 0x0];
    u8 unk7;
    u8 pad8[0x9 - 0x8];
    s8 rotZRate;
    s8 rotYRate;
    s8 rotXRate;
    u8 padC[0x10 - 0xC];
} DimcannonState;
#define DIMCANNON_MAP_EVENT_SLOT_PLAYER_OPERATED 0x13

int DIMCannon_SeqFn(GameObject* obj, int unused, ObjAnimUpdateState* animUpdate)
{
    DimCannonState* state;
    DimcannonPlacement* src = *(DimcannonPlacement**)&obj->anim.placementData;
    int delta;
    u8 done = 0;
    int camMode;

    animUpdate->sequenceEventActive = 0;
    animUpdate->hitVolumePair &= ~0x608;
    state = obj->extra;

    if (state->fireState == 0x3)
    {
        s16* vec;
        s8 timer;
        GameObject* player;

        player = Obj_GetPlayerObject();
        setAButtonIcon(0x16);
        setBButtonIcon(0x17);
        hudFn_8011f38c(1);
        camMode = (*gCameraInterface)->getMode();
        if (camMode != 0x51 && camMode != 0x4c)
        {
            GameObject* focusObj = obj;
            (*gCameraInterface)->setMode(CAMMODE_CANNON, 1, 0, 4, &focusObj, 0x32, 0xff);
        }
        if (camMode != 0x51)
        {
            return 0;
        }
        vec = objModelGetVecFn_800395d8(obj, 0);
        timer = state->chargeTimer;
        if (timer > 0)
        {
            state->chargeTimer = (s8)(timer - framesThisStep);
            if (state->chargeTimer <= 0)
            {
                (*gGameUIInterface)->initAirMeter(gDimCannonMaxCharge, DIMCANNON_AIRMETER_BGTEXTURE);
            }
        }
        else
        {
            if (!mainGetBit(0xdb))
            {
                (*gGameUIInterface)->showNpcDialogue(0x4b9, 0x14, 0x8c, 1);
                mainSetBits(0xdb, 1);
            }
            delta = (int)(-gDimCannonAimStickScale * padGetStickX(0));
            if (delta != 0)
            {
                s16 mag = *(s16*)((char*)vec + 0x2) < 0 ? -*(s16*)((char*)vec + 0x2) : *(s16*)((char*)vec + 0x2);
                if (mag > lbl_803DBF02 - lbl_803DBF04)
                {
                    int sc, sd;
                    sd = delta < 0 ? -1 : (delta > 0 ? 1 : 0);
                    sc = *(s16*)((char*)vec + 0x2) < 0 ? -1 : (*(s16*)((char*)vec + 0x2) > 0 ? 1 : 0);
                    if (sc == sd)
                    {
                        delta = delta * (lbl_803DBF02 - mag);
                        delta = delta / lbl_803DBF04;
                    }
                }
                *(s16*)((int)vec + 0x2) = (s16)(*(s16*)((int)vec + 0x2) + delta);
                Sfx_KeepAliveLoopedObjectSound((u32)obj, SFXTRIG_gal_sailflap2);
            }
            else
            {
                if (state->prevAimDelta != 0)
                {
                    Sfx_PlayFromObject((u32)obj, SFXTRIG_cnplarlp);
                }
            }
            state->prevAimDelta = delta;
            if (state->aimYaw > 0)
            {
                state->aimYaw -= framesThisStep;
            }
            if (state->aimPitch > 0)
            {
                state->aimPitch -= framesThisStep;
            }
            if ((getButtonsHeld(0) & PAD_BUTTON_A) && state->aimYaw <= 0)
            {
                buttonDisable(0, PAD_BUTTON_A);
                if (Player_GetCurrentMagic((int)player) >= 1)
                {
                    state->airMeterCharge += framesThisStep;
                    if (Sfx_IsPlayingFromObjectChannel((u32)obj, 2) == 0)
                    {
                        Sfx_PlayFromObject((u32)obj, SFXTRIG_gal_sailflap1);
                        Sfx_PlayFromObject((u32)obj, SFXTRIG_tr_cnplarlp);
                    }
                }
                else
                {
                    Sfx_PlayFromObject((u32)obj, SFXTRIG_staff_swipes_long);
                }
            }
            else
            {
                Sfx_StopObjectChannel((u32)obj, 2);
            }
            if (state->airMeterCharge > gDimCannonMaxCharge)
            {
                state->airMeterCharge = gDimCannonMaxCharge;
            }
            (*gGameUIInterface)->runAirMeter(state->airMeterCharge);
            state->unk98 =
                (f32)state->airMeterCharge * lbl_803DBEFC + lbl_803DBEF8;
            if ((getButtonsJustPressedIfNotBusy(0) & PAD_BUTTON_A) ||
                state->airMeterCharge == gDimCannonMaxCharge)
            {
                if (state->aimYaw <= 0 && Player_GetCurrentMagic((int)player) >= 1)
                {
                    buttonDisable(0, PAD_BUTTON_A);
                    playerAddRemoveMagic(player, -1);
                    state->fireRequested = 1;
                    state->airMeterCharge = 0;
                }
            }
            DIMwooddoor_spawnShard(obj, 1);
            if (obj->anim.mapEventSlot == DIMCANNON_MAP_EVENT_SLOT_PLAYER_OPERATED &&
                state->hasActivated == 0 && mainGetBit(GAMEBIT_DIM_CannonRelated0C17) &&
                mainGetBit(GAMEBIT_DIM_CannonRelated0A21))
            {
                state->hasActivated = 1;
                state->shutdownTimer = 1;
            }
            {
                u8 b1 = state->shutdownTimer;
                if (b1 != 0)
                {
                    state->shutdownTimer += framesThisStep;
                    if (state->shutdownTimer > 0x3c)
                    {
                        done = 1;
                    }
                }
            }
            if (done != 0 || (getButtonsJustPressed(0) & PAD_BUTTON_B))
            {
                buttonDisable(0, PAD_BUTTON_B);
                hudFn_8011f38c(0);
                (*gGameUIInterface)->airMeterSetShutdown();
                (*gCameraInterface)->setMode(CAMMODE_DEFAULT, 0, 1, 0, NULL, 0, 0xff);
                state->fireState = 5;
                *(u8*)&state->chargeTimer = 0x3c;
                animUpdate->sequenceControlFlags |= OBJSEQ_CONTROL_SET_LATCH_A;
                *(u8*)&obj->anim.resetHitboxMode =
                    (u8)(*(u8*)&obj->anim.resetHitboxMode & ~INTERACT_FLAG_DISABLED);
                if (Sfx_IsPlayingFromObjectChannel((u32)obj, 8) != 0)
                {
                    Sfx_IsPlayingFromObjectChannel((u32)obj, 0);
                }
                Sfx_StopObjectChannel((u32)obj, 2);
            }
            ObjAnim_AdvanceCurrentMove((int)obj, gDimCannonAnimAdvanceSpeedCur,
                                                                        timeDelta, NULL);
        }
    }
    else
    {
        s16* vec2;
        obj->anim.flags = (s16)(obj->anim.flags & ~OBJANIM_FLAG_HIDDEN);
        vec2 = objModelGetVecFn_800395d8(obj, 0);
        *(s16*)((char*)vec2 + 0x2) = (s16)(obj->anim.rotX - (src->unk28 << 8));
        obj->anim.rotX = (s16)(src->unk28 << 8);
        state->fireState = 4;
    }

    return 0;
}

int DIMCannon_getExtraSize(GameObject* obj)
{
    if (obj->anim.seqId == DIMCANNON_BALL_OBJ)
        return 0xc;
    return 0xb4;
}

int DIMCannon_getObjectTypeId(GameObject* obj)
{
    if (obj->anim.seqId == DIMCANNON_BALL_OBJ)
        return 0x0;
    return 0x0;
}

void DIMCannon_free(GameObject* obj)
{
    if (obj->anim.seqId != DIMCANNON_BALL_OBJ)
    {
        ((void (*)(void))((int**)*gGameUIInterface)[0x18])();
        Resource_Release(lbl_803DDB50);
        lbl_803DDB50 = NULL;
    }
    ObjGroup_RemoveObject((int)obj, DIMCANNON_OBJGROUP);
}

void DIMCannon_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible)
{
    u8* def;
    DimCannonState* sub;
    s16 saved;

    def = *(u8**)&obj->anim.placementData;
    if (obj->anim.seqId != DIMCANNON_BALL_OBJ)
    {
        sub = obj->extra;
        saved = obj->anim.rotX;
        obj->anim.rotX = (s16)((s8)def[0x28] << 8);
        objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, lbl_803E48E8);
        obj->anim.rotX = saved;
        ObjPath_GetPointWorldPosition(obj, 0, &sub->posX, &sub->posY,
                                      &sub->posZ, 0);
    }
    else
    {
        objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, lbl_803E48E8);
    }
}

void DIMCannon_hitDetect(void)
{
}

void DIMCannon_update(GameObject* obj)
{
    DimCannonState* state;
    GameObject* player;
    DimcannonPlacement* src = *(DimcannonPlacement**)&obj->anim.placementData;

    if (obj->anim.seqId == DIMCANNON_BALL_OBJ)
    {
        DIMwooddoor_updateFallingDebris(obj);
        return;
    }

    if ((*(u8*)&obj->anim.resetHitboxMode & INTERACT_FLAG_DISABLED) &&
        mainGetBit(src->resetGameBit))
    {
        *(u8*)&obj->anim.resetHitboxMode =
            (u8)(*(u8*)&obj->anim.resetHitboxMode & ~INTERACT_FLAG_DISABLED);
    }

    state = obj->extra;
    player = Obj_GetPlayerObject();
    if (playerGetFocusObject(player) != NULL)
    {
        *(int*)&state->targetPlayer = 0;
    }
    else
    {
        state->targetPlayer = player;
    }

    obj->anim.flags = (s16)(obj->anim.flags & ~OBJANIM_FLAG_HIDDEN);

    switch (state->fireState)
    {
    case 0:
        if (mainGetBit(src->armGameBit))
        {
            state->fireState = 4;
        }
        break;
    case 5:
    {
        s8 t = state->chargeTimer;
        if (t > 0)
        {
            state->chargeTimer = (s8)(t - framesThisStep);
        }
        else if (*(u8*)&obj->anim.resetHitboxMode & INTERACT_FLAG_ACTIVATED)
        {
            GameObject* focusObj;
            state->airMeterCharge = 0;
            state->shutdownTimer = 0;
            focusObj = obj;
            (*gCameraInterface)->setMode(CAMMODE_CANNON, 1, 0, 4, &focusObj, 0x32, 0xff);
            buttonDisable(0, PAD_BUTTON_A);
            state->fireState = 3;
            (*gObjectTriggerInterface)->runSequence(0, obj, -1);
            *(u8*)&state->chargeTimer = 0x3c;
            *(u8*)&obj->anim.resetHitboxMode |= INTERACT_FLAG_DISABLED;
        }
        state->fireRequested = 0;
        state->aimYaw = 0;
        state->aimPitch = 0;
        break;
    }
    case 4:
        DIMwooddoor_updateShardAim(obj, *(f32*)&state->aimTargetX,
                                   *(f32*)&state->aimTargetY, state->aimTargetZ,
                                   state->distance);
        if (mainGetBit(src->resetGameBit))
        {
            state->fireState = 5;
        }
        else if (state->targetPlayer != 0 && !mainGetBit(src->holdGameBit))
        {
            f32 d = getXZDistance(&obj->anim.worldPosX,
                                  &((GameObject*)state->targetPlayer)->anim.worldPosX);
            int v = src->triggerRange * lbl_803DBF10;
            if (d < v / lbl_803E48EC)
            {
                state->fireState = 1;
            }
        }
        state->fireRequested = 0;
        state->aimYaw = 0;
        state->aimPitch = 0;
        break;
    case 1:
        if (mainGetBit(src->resetGameBit))
        {
            state->fireState = 5;
            break;
        }
        if (mainGetBit(src->holdGameBit))
        {
            state->fireState = 4;
            break;
        }
        if (state->targetPlayer != 0)
        {
            state->refreshTimer += framesThisStep;
            if (state->refreshTimer > 0xa)
            {
                u8 j;
                state->refreshTimer = 0;
                for (j = 0; j < 9; j++)
                {
                    state->aimHistX[j] = state->aimHistX[j + 1];
                    state->aimHistY[j] = state->aimHistY[j + 1];
                    state->aimHistZ[j] = state->aimHistZ[j + 1];
                    if (j == 0 || state->aimHistY[j] > *(f32*)&state->aimTargetY)
                    {
                        *(f32*)&state->aimTargetY = state->aimHistY[j];
                    }
                }
                state->aimHistX[9] =
                    ((GameObject*)state->targetPlayer)->anim.localPosX;
                state->aimHistY[9] =
                    ((GameObject*)state->targetPlayer)->anim.localPosY;
                state->aimHistZ[9] =
                    ((GameObject*)state->targetPlayer)->anim.localPosZ;
                *(f32*)&state->aimTargetX = state->aimHistX[0];
                state->aimTargetZ = state->aimHistZ[0];
            }
            if (state->aimYaw > 0)
            {
                state->aimYaw -= framesThisStep;
            }
            if (state->aimPitch > 0)
            {
                state->aimPitch -= framesThisStep;
            }
            state->distance =
                getXZDistance(&obj->anim.worldPosX,
                              &((GameObject*)state->targetPlayer)->anim.worldPosX);
            DIMwooddoor_updateShardAim(obj, *(f32*)&state->aimTargetX,
                                       *(f32*)&state->aimTargetY,
                                       state->aimTargetZ, state->distance);
            DIMwooddoor_spawnShard(obj, 0);
            {
                f32 d2 = state->distance;
                int v = src->triggerRange * lbl_803DBF0C;
                if (d2 > v / lbl_803E48EC)
                {
                    state->fireState = 4;
                }
            }
        }
        else
        {
            state->fireState = 4;
        }
        break;
    }

    gDimCannonAnimAdvanceSpeedCur = gDimCannonAnimAdvanceSpeed;
    ObjAnim_AdvanceCurrentMove((int)obj, gDimCannonAnimAdvanceSpeed, timeDelta, NULL);
}

void DIMCannon_init(GameObject* obj, DimcannonPlacement* arg)
{
    ObjMsg_AllocQueue(obj, 4);

    if (obj->anim.seqId == DIMCANNON_BALL_OBJ)
    {
        DimcannonState* state;
        int* p;
        obj->userData1 = 0;
        p = *(int**)&obj->anim.modelState;
        if (p != 0)
        {
            *(int*)&((ObjHitsPriorityState*)p)->secondaryRadiusY |= 0xc10;
            p = *(int**)&obj->anim.modelState;
            *(u32*)&((ObjHitsPriorityState*)p)->secondaryRadiusY |= 0x8000LL;
        }
        state = obj->extra;
        state->rotZRate = randomGetRange(-0x64, 0x64);
        state->rotYRate = randomGetRange(-0x64, 0x64);
        state->rotXRate = randomGetRange(-0x64, 0x64);
        state->unk7 = 1;
        p = *(int**)&obj->anim.hitReactState;
        if (p != 0)
        {
            *(s16*)&((ObjHitsPriorityState*)p)->trackContactMask = 1;
        }
        obj->objectFlags |= DIMCANNON_OBJFLAG_HIDDEN;
    }
    else
    {
        DimCannonState* state = obj->extra;
        u8 i;

        if (obj->anim.mapEventSlot == DIMCANNON_MAP_EVENT_SLOT_PLAYER_OPERATED)
        {
            int v = 0;
            if (mainGetBit(GAMEBIT_DIM_CannonRelated0C17) && mainGetBit(GAMEBIT_DIM_CannonRelated0A21))
            {
                v = 1;
            }
            state->hasActivated = v;
        }

        for (i = 0; i < 0xa; i += 5)
        {
            state->aimHistX[i + 0] = obj->anim.localPosX;
            state->aimHistY[i + 0] = obj->anim.localPosY;
            state->aimHistZ[i + 0] = obj->anim.localPosZ;
            state->aimHistX[i + 1] = obj->anim.localPosX;
            state->aimHistY[i + 1] = obj->anim.localPosY;
            state->aimHistZ[i + 1] = obj->anim.localPosZ;
            state->aimHistX[i + 2] = obj->anim.localPosX;
            state->aimHistY[i + 2] = obj->anim.localPosY;
            state->aimHistZ[i + 2] = obj->anim.localPosZ;
            state->aimHistX[i + 3] = obj->anim.localPosX;
            state->aimHistY[i + 3] = obj->anim.localPosY;
            state->aimHistZ[i + 3] = obj->anim.localPosZ;
            state->aimHistX[i + 4] = obj->anim.localPosX;
            state->aimHistY[i + 4] = obj->anim.localPosY;
            state->aimHistZ[i + 4] = obj->anim.localPosZ;
        }

        state->refreshTimer = 0x80;
        state->unk98 = lbl_803E48B8;
        *(u8*)&obj->anim.resetHitboxMode |= INTERACT_FLAG_DISABLED;
        obj->animEventCallback = DIMCannon_SeqFn;
        obj->anim.rotX = (s16)(arg->unk28 << 8);
        lbl_803DDB50 = Resource_Acquire(0x79, 1);
        if (mainGetBit(arg->resetGameBit))
        {
            *(u8*)&state->chargeTimer = 0x3c;
            state->fireState = 5;
        }
        state->posX = obj->anim.localPosX;
        state->posY = obj->anim.localPosY;
        state->posZ = obj->anim.localPosZ;
    }

    obj->objectFlags |= DIMCANNON_OBJFLAG_HITDETECT_DISABLED;
}

void DIMCannon_release(void)
{
}

void DIMCannon_initialise(void)
{
}

ObjectDescriptor gDIMCannonObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)DIMCannon_initialise,
    (ObjectDescriptorCallback)DIMCannon_release,
    0,
    (ObjectDescriptorCallback)DIMCannon_init,
    (ObjectDescriptorCallback)DIMCannon_update,
    (ObjectDescriptorCallback)DIMCannon_hitDetect,
    (ObjectDescriptorCallback)DIMCannon_render,
    (ObjectDescriptorCallback)DIMCannon_free,
    (ObjectDescriptorCallback)DIMCannon_getObjectTypeId,
    (ObjectDescriptorExtraSizeCallback)DIMCannon_getExtraSize,
};
