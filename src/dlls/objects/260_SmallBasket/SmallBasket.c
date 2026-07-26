/*
 * smallbasket (DLL 0x104) - a pick-up-and-throw basket/pot object whose
 * extra record is the shared CfperchState (obj+0xB8).
 *
 * SmallBasket_init acquires resource 0x5b, joins object group 0x10, seeds a
 * random idle timer, and picks the impact sfx from the spawn seqId
 * (0x3cf -> 0x60, 0x662 -> 0x37d, otherwise 0x4a).
 *
 * SmallBasket_update drives the lifecycle: a respawn countdown
 * (CfperchState.respawnTimer) that scatters basket contents and warps the object
 * back to its placement, fade-in via anim.alpha, the carry/throw state
 * machine on carryState/throwState (A-button grab, charged vs. normal throw via the
 * player query helpers isTrickyNear/fn_8029669C/fn_802966B4), in-flight
 * physics integration calling smallbasket_resolveCollision each step for swept-sphere
 * ground/wall collision, leash to the placement origin (leashRange), and
 * the periodic ambient sfx (0x6c/0x6d) keyed on the object subtype.
 *
 * smallbasket_spawnContents spawns the basket "contents" on break/throw: it dispatches on
 * the contents mode (data+0x1e, or a health-weighted random roll when 7),
 * allocating one of several object types (0x3d3/0x3d4/0x3d5 green/red/gold
 * scarab, 0xb energy egg, 0x3cd apple) and launching it with a randomized
 * outward velocity.
 *
 * objThrowFn_80182504 is the external entry the player code calls to launch
 * a held basket.
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/dll_0104_smallbasket.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/vecmath.h"
#include "main/vecmath_distance_api.h"
#include "game/objects/object_setup.h"
#include "game/objects/object.h"
#include "main/dll/player_api.h"
#include "main/dll/player_state.h"
#include "sys/objects/lifecycle.h"
#include "main/object_render.h"
#include "main/track_bbox_api.h"
#include "main/dll/dll_0105_largecrate.h"
#include "sys/objects.h"
#include "main/mapEvent.h"
#include "main/resource.h"
#include "main/sky_interface.h"
#include "main/shader_api.h"
#include "main/dll/player_status.h"
#include "main/objfx.h"
#include "main/objhits.h"
#include "main/obj_group.h"
#include "main/obj_message.h"
#include "main/obj_trigger.h"
#include "main/gamebits.h"
#include "main/pad.h"
#include "main/audio/sfx.h"
#include "main/audio/sfx_ids.h"
#include "main/audio/sfx_object_query_api.h"
#include "main/audio/sfx_play_api.h"
#include "main/dll/tricky_api.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/frame_timing.h"
#include "main/track_dolphin_api.h"

const f32 gSmallBasketPercentScale[1] = {100.0f};
const f32 gSmallBasketFxScale[1] = {0.014f};
const f32 gSmallBasketZero[1] = {0.0f};
const f32 gSmallBasketQuarter[1] = {0.25f};
const f32 gSmallBasketPctLow[1] = {50.0f};
const f32 gSmallBasketPctHigh[1] = {75.0f};
const f32 gSmallBasketBurstVelXZ[1] = {3.0f};
const f32 gSmallBasketBurstVelY[1] = {4.0f};
const f32 gSmallBasketOne[1] = {1.0f};
const f32 gSmallBasketVelStep[1] = {0.01f};
const f32 gSmallBasketThrowVelY[1] = {2.2f};

#define SMALLBASKET_HIT_VOLUME_SLOT 0xe

#define SMALLBASKET_OBJGROUP                   0x10
#define SMALLBASKET_MSG_PLAYER_GRAB            0x100010 /* tells player to grab/hold the basket */
#define PAD_BUTTON_A                           0x100

/* Contents objects spawned by smallbasket_spawnContents on break/throw (retail
   OBJECTS.bin names: GreenScarab/RedScarab/GoldScarab, EnergyEgg, Apple). */
#define SMALLBASKET_CHILD_OBJ_SCARAB_GREEN 0x3d3
#define SMALLBASKET_CHILD_OBJ_SCARAB_RED   0x3d4
#define SMALLBASKET_CHILD_OBJ_SCARAB_GOLD  0x3d5
#define SMALLBASKET_CHILD_OBJ_ENERGY_EGG   0xb
#define SMALLBASKET_CHILD_OBJ_APPLE        0x3cd
#define SMALLBASKET_RESOURCE_ID          0x5b /* basket object resource -> gSmallBasketResource */

typedef void (*ObjThrowInitFn)(void* obj, f32 vx, f32 vy, f32 vz);

typedef struct SmallBasketHitEffectPos
{
    u8 pad00[0xc];
    f32 x;
    f32 y;
    f32 z;
} SmallBasketHitEffectPos;

/* Spawn-setup record smallbasket hands to Obj_SetupObject for its thrown
   debris/pickup children. Embeds the common ObjPlacement head (pos + mapId)
   and carries the class-specific tuning fields from 0x18 on. */
typedef struct SmallBasketThrowSetup
{
    ObjPlacement head; /* 0x00 */
    s8 yawByte;        /* 0x18 */
    u8 pad19;
    s16 field1A; /* 0x1A */
    s16 field1C; /* 0x1C init -1 */
    u8 pad1E[6];
    s16 field24; /* 0x24 init -1 */
    u8 pad26[6];
    s16 field2C; /* 0x2C init -1 */
} SmallBasketThrowSetup;

typedef struct
{
    s16 h0;
    s16 h1;
    s16 h2;
    f32 fx;
    f32 fy;
    f32 fz;
    f32 fw;
} BasketMathArgs;

/* CfperchState's carryAttached/throwState fields are u8 in the header, but the
   sign-checked reads in SmallBasket_update treat them as signed (s8). */

int lbl_803DBDA0 = 1;
f32 lbl_803DBDA4 = 15.0f;
f32 lbl_803DBDA8 = 30.0f;
f32 gSmallBasketHitVelocity[4];
void* gSmallBasketResource;

/* Handles SmallBasket hit effects, nearby-object damage, and content drops. */
void fn_801814D0(GameObject* obj, GameObject* arg, u8* state)
{
    int hitWork[4];
    SmallBasketHitEffectPos effectPos;
    int hitType;
    int* objects;
    int i;
    int* groupObjects;
    f32 dusterY;
    f32 candidateY;
    f32 launchVel;

    hitType = ObjHits_GetPriorityHitWithPosition(obj, &hitWork[3], &hitWork[2], (u32*)&hitWork[1],
                                                 &effectPos.x, &effectPos.y, &effectPos.z);
    if (hitType != 0)
    {
        if (hitType == 0x10)
        {
            Obj_StartModelFadeIn(obj, 0x12c);
        }
        else
        {
            effectPos.x += playerMapOffsetX;
            effectPos.z += playerMapOffsetZ;
            if (((CfperchState*)state)->disguiseGated != 0)
            {
                if (hitType != 5)
                {
                    objLightFn_8009a1dc((void*)obj, gSmallBasketFxScale[0], &effectPos, 4, 0);
                    if (Sfx_IsPlayingFromObject(0, SFXTRIG_staff_rocket_powerup) == 0)
                    {
                        Sfx_PlayFromObject((u32)obj, SFXTRIG_staff_rocket_powerup);
                    }
                    return;
                }
                groupObjects = (int*)ObjGroup_GetObjects(SMALLBASKET_OBJGROUP, &hitWork[0]);
                i = 0;
                objects = groupObjects;
                for (; i < hitWork[0]; i++)
                {
                    if (ObjHits_IsObjectEnabled((ObjAnimComponent*)*objects) != 0)
                    {
                        candidateY = ((GameObject*)*objects)->anim.localPosY;
                        dusterY = obj->anim.localPosY;
                        if (candidateY > dusterY && candidateY < dusterY + lbl_803DBDA8)
                        {
                            if (Vec_xzDistance((f32*)(*objects + 0x18), &obj->anim.worldPosX) < lbl_803DBDA4)
                            {
                                ObjHits_RecordObjectHit((GameObject*)*objects, (GameObject*)hitWork[3], 5, 1, 0);
                            }
                        }
                    }
                    objects++;
                }
            }
            objLightFn_8009a1dc((void*)obj, gSmallBasketFxScale[0], &effectPos, 1, 0);
            Obj_SetModelColorFadeRecursive(obj, 0xf, 0xc8, 0, 0, 1);
            if (Sfx_IsPlayingFromObject(0, (u16)((CfperchState*)state)->sfxId) == 0)
            {
                Sfx_PlayFromObject((u32)obj, (u16)((CfperchState*)state)->sfxId);
            }
            ((CfperchState*)state)->disableTimer = 0x32;
            ((CfperchState*)state)->throwState = 0;
            smallbasket_spawnContents(obj, arg, state);
            obj->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
            launchVel = gSmallBasketZero[0];
            obj->anim.velocityX = gSmallBasketZero[0];
            obj->anim.velocityZ = launchVel;
            ObjHits_ClearHitVolumes((ObjAnimComponent*)obj);
            if (lbl_803DBDA0 != 0)
            {
                ObjHits_DisableObject(obj);
            }
        }
    }
}

int smallbasket_spawnContents(GameObject* obj, GameObject* player, void* dataIn)
{
    GameObject* playerObj;
    s16 mode;
    u8* data;
    f32* vel;
    f32* hitVel;
    u8 slowMo;
    int sw;
    u8* setup;
    u8* spawned;
    int bit;
    int max;
    int ang;
    int diff;
    f32 ratio;
    f32 health;
    f32 maxHealth;
    f32 sc;
    f32 mag;
    struct
    {
        s16 f8;
        s16 fa;
        s16 fc;
        s16 pad_e;
        f32 f10;
        f32 f14;
        f32 f18;
        f32 f1c;
    } spread;

    data = dataIn;
    playerObj = player;
    slowMo = 0;
    bit = *(s16*)(data + 0x1c);
    if (bit != -1)
    {
        mainSetBits(bit, 1);
    }
    if (Obj_IsLoadingLocked() == 0)
    {
        return 0;
    }
    hitVel = gSmallBasketHitVelocity;
    if (hitVel[1] < gSmallBasketQuarter[0])
    {
        slowMo = 1;
    }
    if (data[0x1e] == 7)
    {
        health = (f32)(int)Player_GetCurrentHealth((int)player);
        ratio = health;
        maxHealth = (f32)(int)Player_GetMaxHealth((int)player);
        ratio = ratio / maxHealth;
        ratio = ratio * gSmallBasketPercentScale[0];
        if (ratio <= gSmallBasketPctLow[0])
        {
            mode = 6;
        }
        else if (ratio <= gSmallBasketPctHigh[0])
        {
            if ((int)randomGetRange(0, (s16)(int)(ratio - gSmallBasketPctLow[0])) < 7)
            {
                mode = 6;
                max = (s16)(maxHealth * gSmallBasketQuarter[0]);
                if (max < 1)
                {
                    max = 1;
                }
                randomGetRange(1, max);
            }
            else
            {
                mode = 1;
                randomGetRange(1, 4);
            }
        }
        else
        {
            return 1;
        }
    }
    else
    {
        mode = data[0x1e];
    }

    sw = mode;
    vel = gSmallBasketHitVelocity;
    switch (sw)
    {
    case 1:
        setup = (u8*)Obj_AllocObjectSetup(0x24, SMALLBASKET_CHILD_OBJ_SCARAB_GREEN);
        ((ObjPlacement*)setup)->posX = obj->anim.localPosX;
        ((ObjPlacement*)setup)->posY = obj->anim.localPosY;
        ((ObjPlacement*)setup)->posZ = obj->anim.localPosZ;
        ((SmallBasketThrowSetup*)setup)->field1A = 0x190;
        spawned = (u8*)Obj_SetupObject((ObjPlacement*)setup, 5, obj->anim.mapEventSlot, -1, obj->anim.parent);
        if (slowMo)
        {
            sc = gSmallBasketBurstVelXZ[0];
            ((GameObject*)spawned)->anim.velocityX = sc * gSmallBasketHitVelocity[0];
            ((GameObject*)spawned)->anim.velocityY = gSmallBasketBurstVelY[0] * hitVel[1];
            ((GameObject*)spawned)->anim.velocityZ = sc * vel[2];
        }
        else
        {
            ((GameObject*)spawned)->anim.velocityX = obj->anim.localPosX - playerObj->anim.localPosX;
            ((GameObject*)spawned)->anim.velocityZ = obj->anim.localPosZ - playerObj->anim.localPosZ;
        }
        mag = ((GameObject*)spawned)->anim.velocityX * ((GameObject*)spawned)->anim.velocityX;
        mag += ((GameObject*)spawned)->anim.velocityZ * ((GameObject*)spawned)->anim.velocityZ;
        if (mag != gSmallBasketZero[0])
        {
            mag = sqrtf(mag);
            ((GameObject*)spawned)->anim.velocityX = ((GameObject*)spawned)->anim.velocityX / mag;
            ((GameObject*)spawned)->anim.velocityZ = ((GameObject*)spawned)->anim.velocityZ / mag;
        }
        ((GameObject*)spawned)->anim.velocityX =
            ((GameObject*)spawned)->anim.velocityX * -(gSmallBasketVelStep[0] * (f32)(int)randomGetRange(0, 0x19) - gSmallBasketOne[0]);
        ((GameObject*)spawned)->anim.velocityZ =
            ((GameObject*)spawned)->anim.velocityZ * -(gSmallBasketVelStep[0] * (f32)(int)randomGetRange(0, 0x19) - gSmallBasketOne[0]);
        ((GameObject*)spawned)->anim.velocityY = gSmallBasketThrowVelY[0];
        spread.f14 = gSmallBasketZero[0];
        spread.f18 = gSmallBasketZero[0];
        spread.f1c = gSmallBasketZero[0];
        spread.f10 = gSmallBasketOne[0];
        spread.fc = 0;
        spread.fa = 0;
        spread.f8 = randomGetRange(-10000, 10000);
        vecRotateZXY(&spread.f8, (f32*)(spawned + 0x24));
        ang = (u16)(s16)getAngle(((GameObject*)spawned)->anim.velocityX, -((GameObject*)spawned)->anim.velocityZ);
        diff = ((GameObject*)spawned)->anim.rotX - ang;
        if (diff > 0x8000)
        {
            diff -= 0xffff;
        }
        if (diff < -0x8000)
        {
            diff += 0xffff;
        }
        ((GameObject*)spawned)->anim.rotX = diff;
        break;
    case 2:
        setup = (u8*)Obj_AllocObjectSetup(0x24, SMALLBASKET_CHILD_OBJ_SCARAB_RED);
        ((SmallBasketThrowSetup*)setup)->yawByte = randomGetRange(-0x7f, 0x7e);
        ((ObjPlacement*)setup)->posX = obj->anim.localPosX;
        ((ObjPlacement*)setup)->posY = obj->anim.localPosY;
        ((ObjPlacement*)setup)->posZ = obj->anim.localPosZ;
        ((SmallBasketThrowSetup*)setup)->field1A = 0x190;
        spawned = (u8*)Obj_SetupObject((ObjPlacement*)setup, 5, obj->anim.mapEventSlot, -1, obj->anim.parent);
        if (slowMo)
        {
            sc = gSmallBasketBurstVelXZ[0];
            ((GameObject*)spawned)->anim.velocityX = sc * gSmallBasketHitVelocity[0];
            ((GameObject*)spawned)->anim.velocityY = gSmallBasketBurstVelY[0] * hitVel[1];
            ((GameObject*)spawned)->anim.velocityZ = sc * vel[2];
        }
        else
        {
            ((GameObject*)spawned)->anim.velocityX = obj->anim.localPosX - playerObj->anim.localPosX;
            ((GameObject*)spawned)->anim.velocityZ = obj->anim.localPosZ - playerObj->anim.localPosZ;
        }
        mag = ((GameObject*)spawned)->anim.velocityX * ((GameObject*)spawned)->anim.velocityX;
        mag += ((GameObject*)spawned)->anim.velocityZ * ((GameObject*)spawned)->anim.velocityZ;
        if (mag != gSmallBasketZero[0])
        {
            mag = sqrtf(mag);
            ((GameObject*)spawned)->anim.velocityX = ((GameObject*)spawned)->anim.velocityX / mag;
            ((GameObject*)spawned)->anim.velocityZ = ((GameObject*)spawned)->anim.velocityZ / mag;
        }
        ((GameObject*)spawned)->anim.velocityX =
            ((GameObject*)spawned)->anim.velocityX * -(gSmallBasketVelStep[0] * (f32)(int)randomGetRange(0, 0x19) - gSmallBasketOne[0]);
        ((GameObject*)spawned)->anim.velocityZ =
            ((GameObject*)spawned)->anim.velocityZ * -(gSmallBasketVelStep[0] * (f32)(int)randomGetRange(0, 0x19) - gSmallBasketOne[0]);
        ((GameObject*)spawned)->anim.velocityY = gSmallBasketThrowVelY[0];
        spread.f14 = gSmallBasketZero[0];
        spread.f18 = gSmallBasketZero[0];
        spread.f1c = gSmallBasketZero[0];
        spread.f10 = gSmallBasketOne[0];
        spread.fc = 0;
        spread.fa = 0;
        spread.f8 = randomGetRange(-10000, 10000);
        vecRotateZXY(&spread.f8, (f32*)(spawned + 0x24));
        ang = (u16)(s16)getAngle(((GameObject*)spawned)->anim.velocityX, -((GameObject*)spawned)->anim.velocityZ);
        diff = ((GameObject*)spawned)->anim.rotX - ang;
        if (diff > 0x8000)
        {
            diff -= 0xffff;
        }
        if (diff < -0x8000)
        {
            diff += 0xffff;
        }
        ((GameObject*)spawned)->anim.rotX = diff;
        break;
    case 3:
        setup = (u8*)Obj_AllocObjectSetup(0x24, SMALLBASKET_CHILD_OBJ_SCARAB_GOLD);
        ((SmallBasketThrowSetup*)setup)->yawByte = randomGetRange(-0x7f, 0x7e);
        ((ObjPlacement*)setup)->posX = obj->anim.localPosX;
        ((ObjPlacement*)setup)->posY = obj->anim.localPosY;
        ((ObjPlacement*)setup)->posZ = obj->anim.localPosZ;
        ((SmallBasketThrowSetup*)setup)->field1A = 0x7d0;
        spawned = (u8*)Obj_SetupObject((ObjPlacement*)setup, 5, obj->anim.mapEventSlot, -1, obj->anim.parent);
        if (slowMo)
        {
            sc = gSmallBasketBurstVelXZ[0];
            ((GameObject*)spawned)->anim.velocityX = sc * gSmallBasketHitVelocity[0];
            ((GameObject*)spawned)->anim.velocityY = gSmallBasketBurstVelY[0] * hitVel[1];
            ((GameObject*)spawned)->anim.velocityZ = sc * vel[2];
        }
        else
        {
            ((GameObject*)spawned)->anim.velocityX = obj->anim.localPosX - playerObj->anim.localPosX;
            ((GameObject*)spawned)->anim.velocityZ = obj->anim.localPosZ - playerObj->anim.localPosZ;
        }
        mag = ((GameObject*)spawned)->anim.velocityX * ((GameObject*)spawned)->anim.velocityX;
        mag += ((GameObject*)spawned)->anim.velocityZ * ((GameObject*)spawned)->anim.velocityZ;
        if (mag != gSmallBasketZero[0])
        {
            mag = sqrtf(mag);
            ((GameObject*)spawned)->anim.velocityX = ((GameObject*)spawned)->anim.velocityX / mag;
            ((GameObject*)spawned)->anim.velocityZ = ((GameObject*)spawned)->anim.velocityZ / mag;
        }
        ((GameObject*)spawned)->anim.velocityX =
            ((GameObject*)spawned)->anim.velocityX * -(gSmallBasketVelStep[0] * (f32)(int)randomGetRange(0, 0x19) - gSmallBasketOne[0]);
        ((GameObject*)spawned)->anim.velocityZ =
            ((GameObject*)spawned)->anim.velocityZ * -(gSmallBasketVelStep[0] * (f32)(int)randomGetRange(0, 0x19) - gSmallBasketOne[0]);
        ((GameObject*)spawned)->anim.velocityY = gSmallBasketThrowVelY[0];
        spread.f14 = gSmallBasketZero[0];
        spread.f18 = gSmallBasketZero[0];
        spread.f1c = gSmallBasketZero[0];
        spread.f10 = gSmallBasketOne[0];
        spread.fc = 0;
        spread.fa = 0;
        spread.f8 = randomGetRange(-10000, 10000);
        vecRotateZXY(&spread.f8, (f32*)(spawned + 0x24));
        ang = (u16)(s16)getAngle(((GameObject*)spawned)->anim.velocityX, -((GameObject*)spawned)->anim.velocityZ);
        diff = ((GameObject*)spawned)->anim.rotX - ang;
        if (diff > 0x8000)
        {
            diff -= 0xffff;
        }
        if (diff < -0x8000)
        {
            diff += 0xffff;
        }
        ((GameObject*)spawned)->anim.rotX = diff;
        break;
    case 5:
    case 6:
        if (data[0x1e] == 5)
        {
            setup = (u8*)Obj_AllocObjectSetup(0x30, SMALLBASKET_CHILD_OBJ_ENERGY_EGG);
        }
        else
        {
            setup = (u8*)Obj_AllocObjectSetup(0x30, SMALLBASKET_CHILD_OBJ_APPLE);
        }
        setup[0x1a] = 0x14;
        ((SmallBasketThrowSetup*)setup)->field2C = -1;
        ((SmallBasketThrowSetup*)setup)->field1C = -1;
        if ((s8)data[9] != 0)
        {
            ((ObjPlacement*)setup)->posX = obj->anim.localPosX + (f32)(int)randomGetRange(-0xf, 0xf);
            ((ObjPlacement*)setup)->posY = (15.0f) + obj->anim.localPosY;
            ((ObjPlacement*)setup)->posZ = obj->anim.localPosZ + (f32)(int)randomGetRange(-0xf, 0xf);
        }
        else
        {
            ((ObjPlacement*)setup)->posX = obj->anim.localPosX;
            ((ObjPlacement*)setup)->posY = (5.0f) + obj->anim.localPosY;
            ((ObjPlacement*)setup)->posZ = obj->anim.localPosZ;
        }
        ((SmallBasketThrowSetup*)setup)->field24 = -1;
        spawned = (u8*)Obj_SetupObject((ObjPlacement*)setup, 5, obj->anim.mapEventSlot, -1, obj->anim.parent);
        if (slowMo)
        {
            sc = gSmallBasketBurstVelXZ[0];
            ((GameObject*)spawned)->anim.velocityX = sc * gSmallBasketHitVelocity[0];
            ((GameObject*)spawned)->anim.velocityY = gSmallBasketBurstVelY[0] * hitVel[1];
            ((GameObject*)spawned)->anim.velocityZ = sc * vel[2];
        }
        mag = ((GameObject*)spawned)->anim.velocityX * ((GameObject*)spawned)->anim.velocityX;
        mag += ((GameObject*)spawned)->anim.velocityZ * ((GameObject*)spawned)->anim.velocityZ;
        if (mag != gSmallBasketZero[0])
        {
            mag = sqrtf(mag);
            ((GameObject*)spawned)->anim.velocityX =
                ((GameObject*)spawned)->anim.velocityX / (mag = (2.0f) * mag);
            ((GameObject*)spawned)->anim.velocityZ = ((GameObject*)spawned)->anim.velocityZ / mag;
        }
        ((GameObject*)spawned)->anim.velocityX =
            ((GameObject*)spawned)->anim.velocityX * -(gSmallBasketVelStep[0] * (f32)(int)randomGetRange(0, 0x19) - gSmallBasketOne[0]);
        ((GameObject*)spawned)->anim.velocityZ =
            ((GameObject*)spawned)->anim.velocityZ * -(gSmallBasketVelStep[0] * (f32)(int)randomGetRange(0, 0x19) - gSmallBasketOne[0]);
        ((GameObject*)spawned)->anim.velocityY = gSmallBasketThrowVelY[0];
        (*(ObjThrowInitFn*)(*(int*)*(int*)&((GameObject*)spawned)->anim.dll + 0x2c))(
            spawned, ((GameObject*)spawned)->anim.velocityX, ((GameObject*)spawned)->anim.velocityY,
            ((GameObject*)spawned)->anim.velocityZ);
        spread.f14 = gSmallBasketZero[0];
        spread.f18 = gSmallBasketZero[0];
        spread.f1c = gSmallBasketZero[0];
        spread.f10 = gSmallBasketOne[0];
        spread.fc = 0;
        spread.fa = 0;
        spread.f8 = randomGetRange(-10000, 10000);
        vecRotateZXY(&spread.f8, (f32*)(spawned + 0x24));
        ang = (u16)(s16)getAngle(((GameObject*)spawned)->anim.velocityX, -((GameObject*)spawned)->anim.velocityZ);
        diff = ((GameObject*)spawned)->anim.rotX - ang;
        if (diff > 0x8000)
        {
            diff -= 0xffff;
        }
        if (diff < -0x8000)
        {
            diff += 0xffff;
        }
        ((GameObject*)spawned)->anim.rotX = diff;
        break;
    }
    return 1;
}

int smallbasket_resolveCollision(GameObject* obj)
{
    typedef struct
    {
        f32 hitInfo[4][4];
        f32 radii[4];
        s8 axes[12];
        u32 solidFlags[4];
    } HitDetectResults;

    u8* st;
    s8* axes;
    f32* endY;
    f32* endZ;
    int idx;
    u8 hit;
    f32 fz;
    HitDetectResults hitResults;
    f32 endPoints[12];
    f32 startPoints[12];
    TrackQueryBounds sweptBounds;

    st = *(u8**)&obj->anim.hitReactState;
    if (objBboxFn_800640cc(&obj->anim.previousLocalPosX, &obj->anim.localPosX, (0.1f), 1, NULL,
                           obj, 1, -1, 0xff, 0) != 0)
    {
        ((ObjHitsPriorityState*)st)->contactFlags |= OBJHITS_CONTACT_FLAG_KIND0;
        ((ObjHitsPriorityState*)st)->localPosX = obj->anim.previousLocalPosX;
        ((ObjHitsPriorityState*)st)->localPosY = obj->anim.previousLocalPosY;
        ((ObjHitsPriorityState*)st)->localPosZ = obj->anim.previousLocalPosZ;
        fz = gSmallBasketZero[0];
        obj->anim.velocityX = fz;
        obj->anim.velocityY = fz;
        obj->anim.velocityZ = fz;
        return 1;
    }

    if ((int)(((ObjHitsPriorityState*)st)->objectHitMask >> 4) != 0 && (s8)st[0x70] == 0)
    {
        endPoints[0] = obj->anim.localPosX;
        *(endY = &endPoints[1]) = obj->anim.localPosY;
        *(endZ = &endPoints[2]) = obj->anim.localPosZ;
        startPoints[0] = obj->anim.previousLocalPosX;
        startPoints[1] = obj->anim.previousLocalPosY;
        startPoints[2] = obj->anim.previousLocalPosZ;
        hitResults.radii[0] = (f32)((ObjHitsPriorityState*)st)->primaryRadius;
        *(axes = hitResults.axes) = -1;
        axes[4] = 3;
    }
    else
    {
        return 0;
    }

    hitDetect_calcSweptSphereBounds(&sweptBounds, startPoints, endPoints, hitResults.radii, 1);
    hitDetectFn_800691c0(obj, &sweptBounds, ((ObjHitsPriorityState*)st)->trackContactMask, 1);
    hit = hitDetectFn_80067958(obj, startPoints, endPoints, 1, &hitResults, 0);
    if (hit != 0)
    {
        if (hit & 1)
        {
            idx = 0;
        }
        else if (hit & 2)
        {
            idx = 1;
        }
        else if (hit & 4)
        {
            idx = 2;
        }
        else
        {
            idx = 3;
        }
        st[0xac] = axes[idx];
        ((ObjHitsPriorityState*)st)->contactPosX = endPoints[idx * 3];
        ((ObjHitsPriorityState*)st)->contactPosY = endY[idx * 3];
        ((ObjHitsPriorityState*)st)->contactPosZ = endZ[idx * 3];
        gSmallBasketHitVelocity[0] = hitResults.hitInfo[idx][0];
        gSmallBasketHitVelocity[1] = hitResults.hitInfo[idx][1];
        gSmallBasketHitVelocity[2] = hitResults.hitInfo[idx][2];
        gSmallBasketHitVelocity[3] = hitResults.hitInfo[idx][3];
        if (hitResults.solidFlags[idx] != 0)
        {
            ((ObjHitsPriorityState*)st)->contactFlags |= OBJHITS_CONTACT_FLAG_KIND_NONZERO;
            obj->anim.localPosX = ((ObjHitsPriorityState*)st)->contactPosX;
            obj->anim.localPosY = ((ObjHitsPriorityState*)st)->contactPosY;
            obj->anim.localPosZ = ((ObjHitsPriorityState*)st)->contactPosZ;
            ((ObjHitsPriorityState*)st)->localPosX = obj->anim.previousLocalPosX;
            ((ObjHitsPriorityState*)st)->localPosY = obj->anim.previousLocalPosY;
            ((ObjHitsPriorityState*)st)->localPosZ = obj->anim.previousLocalPosZ;
            fz = gSmallBasketZero[0];
            obj->anim.velocityX = fz;
            obj->anim.velocityY = fz;
            obj->anim.velocityZ = fz;
            return 1;
        }
        else
        {
            ((ObjHitsPriorityState*)st)->contactFlags |= OBJHITS_CONTACT_FLAG_KIND0;
            obj->anim.localPosX = ((ObjHitsPriorityState*)st)->contactPosX;
            obj->anim.localPosY = ((ObjHitsPriorityState*)st)->contactPosY;
            obj->anim.localPosZ = ((ObjHitsPriorityState*)st)->contactPosZ;
            ((ObjHitsPriorityState*)st)->localPosX = obj->anim.previousLocalPosX;
            ((ObjHitsPriorityState*)st)->localPosY = obj->anim.previousLocalPosY;
            ((ObjHitsPriorityState*)st)->localPosZ = obj->anim.previousLocalPosZ;
            fz = gSmallBasketZero[0];
            obj->anim.velocityX = fz;
            obj->anim.velocityY = fz;
            obj->anim.velocityZ = fz;
            return 1;
        }
    }
    return 0;
}

void objThrowFn_80182504(GameObject* obj)
{
    struct LocalArgs
    {
        short f8;
        short fa;
        short fc;
        short pad_e;
        float f10;
        float f14;
        float f18;
        float f1c;
    } local;
    int extra;
    short* player;
    extra = *(int*)&obj->extra;
    player = (short*)Obj_GetPlayerObject();
    ((CfperchState*)extra)->carryAttached = 0;
    ((CfperchState*)extra)->carryState = 0;
    ((CfperchState*)extra)->throwState = 1;
    obj->anim.velocityY = gSmallBasketThrowVelY[0];
    obj->anim.velocityZ = (-2.2f);
    local.f14 = gSmallBasketZero[0];
    local.f18 = gSmallBasketZero[0];
    local.f1c = gSmallBasketZero[0];
    local.f10 = gSmallBasketOne[0];
    local.fc = 0;
    local.fa = 0;
    local.f8 = *player;
    vecRotateZXY(&local.f8, &obj->anim.velocityX);
}

int SmallBasket_getExtraSize(void)
{
    return 0x24;
}

void SmallBasket_free(GameObject* obj)
{
    (*gModgfxInterface)->detachSource(obj);
    Resource_Release(gSmallBasketResource);
    ObjGroup_RemoveObject((int)obj, SMALLBASKET_OBJGROUP);
}

void SmallBasket_render(GameObject* obj, int p2, int p3, int p4, int p5, char visible)
{
    int extra;
    int result;
    short field_a;
    extra = *(int*)&obj->extra;
    result = (*gMapEventInterface)->shouldNotSaveTime(*(int*)(*(int*)&obj->anim.placementData + 0x14));
    if (result == 0)
    {
        obj->anim.flags = obj->anim.flags | OBJANIM_FLAG_HIDDEN;
    }
    else
    {
        field_a = ((CfperchState*)extra)->disableTimer;
        if ((field_a != 0 && field_a <= 0x32) || ((CfperchState*)extra)->hiddenTimer != 0)
        {
            obj->anim.flags = obj->anim.flags | OBJANIM_FLAG_HIDDEN;
        }
        else if (obj->userData2 != 0 && visible != -1)
        {
            obj->anim.flags = obj->anim.flags | OBJANIM_FLAG_HIDDEN;
        }
        else
        {
            objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, gSmallBasketOne[0]);
        }
    }
}

ObjectDescriptor gSmallBasketObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    0,
    0,
    0,
    (ObjectDescriptorCallback)SmallBasket_init,
    (ObjectDescriptorCallback)SmallBasket_update,
    0,
    (ObjectDescriptorCallback)SmallBasket_render,
    (ObjectDescriptorCallback)SmallBasket_free,
    0,
    SmallBasket_getExtraSize,
};
void SmallBasket_update(GameObject* obj)
{
    GameObject* player;
    int def;
    CfperchState* state;
    PlayerState* playerState;
    int flag[1];
    s8 contactFlags;
    u8 subtype;
    int level;
    f32 zf;
    f32 animSpeed;
    BasketMathArgs blk;

    player = Obj_GetPlayerObject();
    def = *(int*)&(obj)->anim.placementData;
    animSpeed = gSmallBasketOne[0];
    (*gSkyInterface)->getClockTime(&animSpeed);
    state = (obj)->extra;
    if ((*gMapEventInterface)->shouldNotSaveTime(((ObjPlacement*)def)->mapId) == 0)
    {
        return;
    }
    playerState = player->extra;
    if (state->respawnTimer <= 0)
    {
        state->respawnTimer = 800;
        state->disableTimer = 1;
        state->throwState = 0;
        *(u8*)&(obj)->anim.resetHitboxMode |= INTERACT_FLAG_DISABLED;
        smallbasket_spawnContents(obj, player, state);
        zf = gSmallBasketZero[0];
        (obj)->anim.velocityX = zf;
        (obj)->anim.velocityZ = zf;
    }
    if (state->hiddenTimer != 0)
    {
        flag[0] = 0;
        (obj)->anim.alpha = flag[0];
        state->hiddenTimer -= (s16)(int)(timeDelta * animSpeed);
        if (state->hiddenTimer <= 0)
        {
            if ((Vec_distance(&(obj)->anim.worldPosX, &((GameObject*)Obj_GetPlayerObject())->anim.worldPosX) >
                 gSmallBasketPercentScale[0]) &&
                (state->enableGameBit == -1))
            {
                flag[0] = 1;
            }
            if (flag[0] == 0)
            {
                state->hiddenTimer = 1;
            }
            else
            {
                state->hiddenTimer = 0;
                state->disableTimer = 0;
                ObjHits_EnableObject(obj);
                ObjHits_SyncObjectPositionIfDirty(obj);
                *(u8*)&(obj)->anim.resetHitboxMode &= ~INTERACT_FLAG_DISABLED;
                (obj)->anim.flags &= ~OBJANIM_FLAG_HIDDEN;
            }
        }
    }
    else
    {
        if (state->carryState != 2)
        {
            level = (int)((8.0f) * timeDelta + (f32)(u32)(obj)->anim.alpha);
            if (level > 0xff)
            {
                level = 0xff;
            }
            (obj)->anim.alpha = level;
        }
        if (state->disableTimer != 0)
        {
            ObjHits_DisableObject(obj);
            state->disableTimer -= framesThisStep;
            if (state->disableTimer <= 0)
            {
                if (state->respawnDelay != 0)
                {
                    state->hiddenTimer = state->respawnDelay;
                }
                else
                {
                    state->hiddenTimer = 1;
                }
                (*gMapEventInterface)->addTime(((ObjPlacement*)def)->mapId, (f32)state->respawnDelay);
                (obj)->anim.localPosX = ((ObjPlacement*)def)->posX;
                (obj)->anim.localPosY = ((ObjPlacement*)def)->posY;
                (obj)->anim.localPosZ = ((ObjPlacement*)def)->posZ;
                (obj)->anim.previousLocalPosX = ((ObjPlacement*)def)->posX;
                (obj)->anim.previousLocalPosY = ((ObjPlacement*)def)->posY;
                (obj)->anim.previousLocalPosZ = ((ObjPlacement*)def)->posZ;
                zf = gSmallBasketZero[0];
                (obj)->anim.velocityX = zf;
                (obj)->anim.velocityY = zf;
                (obj)->anim.velocityZ = zf;
            }
            if (state->disableTimer <= 0x32)
            {
                return;
            }
        }
        if (*(s8*)&state->throwState != 1)
        {
            if (state->carryState == 0)
            {
                flag[0] = 0;
                if (((buttonGetDisabled(0) & PAD_BUTTON_A) == 0) && ((obj)->userData2 == 0) &&
                    (ObjTrigger_IsSet((int)obj) != 0))
                {
                    state->carryAngle = -0x8000;
                    state->carryParam = 0;
                    ObjHits_DisableObject(obj);
                    flag[0] = 1;
                }
                state->carryState = flag[0];
                if (state->carryState != 0)
                {
                    state->carryAttached = 1;
                }
                if ((obj)->userData2 == 0)
                {
                    ObjHits_EnableObject(obj);
                    if ((state->disguiseGated != 0) && (playerIsDisguised(player) == 0))
                    {
                        *(u8*)&(obj)->anim.resetHitboxMode |= INTERACT_FLAG_PROMPT_SUPPRESSED;
                    }
                    else
                    {
                        *(u8*)&(obj)->anim.resetHitboxMode &= ~INTERACT_FLAG_PROMPT_SUPPRESSED;
                    }
                }
                (obj)->anim.previousLocalPosX = (obj)->anim.localPosX;
                (obj)->anim.previousLocalPosY = (obj)->anim.localPosZ;
                (obj)->anim.previousLocalPosZ = (obj)->anim.localPosZ;
            }
            else
            {
                ObjHits_DisableObject(obj);
                *(u8*)&(obj)->anim.resetHitboxMode |= INTERACT_FLAG_DISABLED;
                if ((playerGetStateFlag310(player) & 0x4000) != 0)
                {
                    setAButtonIcon(5);
                }
                else
                {
                    setAButtonIcon(4);
                }
                if ((getButtonsJustPressed(0) & PAD_BUTTON_A) != 0)
                {
                    if (isTrickyNear(player) != 0)
                    {
                        state->carryAttached = 0;
                        buttonDisable(0, PAD_BUTTON_A);
                    }
                    else
                    {
                        Sfx_PlayFromObject(0, SFXTRIG_id_10a);
                    }
                }
                if ((obj)->userData2 == 1)
                {
                    state->carryState = 2;
                }
                if (((state->carryState == 2) && ((obj)->userData2 == 0)) ||
                    ((state->disguiseGated != 0) && (playerIsDisguised(player) == 0)))
                {
                    if (fn_8029669C(player) != 0)
                    {
                        state->carryState = 0;
                        state->throwState = 1;
                        (obj)->anim.velocityY = (0.75f) * playerState->baddie.inputMagnitude + gSmallBasketThrowVelY[0];
                        (obj)->anim.velocityZ = (-0.75f) * playerState->baddie.inputMagnitude + (-2.2f);
                        blk.fy = gSmallBasketZero[0];
                        blk.fz = gSmallBasketZero[0];
                        blk.fw = gSmallBasketZero[0];
                        blk.fx = gSmallBasketOne[0];
                        blk.h2 = 0;
                        blk.h1 = 0;
                        blk.h0 = player->anim.rotX;
                        if (player->anim.parent != NULL)
                        {
                            blk.h0 = blk.h0 + ((ObjAnimComponent*)player->anim.parent)->rotX;
                        }
                        vecRotateZXY((s16*)&blk, &(obj)->anim.velocityX);
                        Sfx_PlayFromObject((int)obj, SFXTRIG_barrel_throw);
                    }
                    else if (fn_802966B4(player) != 0)
                    {
                        state->carryState = 0;
                        state->throwState = 2;
                        zf = gSmallBasketZero[0];
                        (obj)->anim.velocityX = zf;
                        (obj)->anim.velocityY = zf;
                        (obj)->anim.velocityZ = zf;
                        ObjHits_EnableObject(obj);
                        *(u8*)&(obj)->anim.resetHitboxMode &= ~INTERACT_FLAG_DISABLED;
                        ObjHits_ClearHitVolumes((ObjAnimComponent*)obj);
                    }
                    else
                    {
                        state->carryState = 0;
                        state->throwState = 1;
                        (obj)->anim.velocityY = (0.35f) * playerState->baddie.inputMagnitude + (1.2f);
                        (obj)->anim.velocityZ = (-0.35f) * playerState->baddie.inputMagnitude + (-1.2f);
                        blk.fy = gSmallBasketZero[0];
                        blk.fz = gSmallBasketZero[0];
                        blk.fw = gSmallBasketZero[0];
                        blk.fx = gSmallBasketOne[0];
                        blk.h2 = 0;
                        blk.h1 = 0;
                        blk.h0 = player->anim.rotX;
                        vecRotateZXY((s16*)&blk, &(obj)->anim.velocityX);
                        Sfx_PlayFromObject((int)obj, SFXTRIG_barrel_throw);
                        state->carryAttached = 0;
                        *(u8*)&(obj)->anim.resetHitboxMode |= INTERACT_FLAG_DISABLED;
                    }
                }
                if (*(s8*)&state->carryAttached != 0)
                {
                    state->disableTimer = 0;
                    state->hiddenTimer = 0;
                    ObjMsg_SendToObject(player, SMALLBASKET_MSG_PLAYER_GRAB, obj,
                                        (state->carryParam << 16) | ((u16)state->carryAngle));
                }
            }
        }
        else if (*(s8*)&state->throwState != 0)
        {
            state->respawnTimer -= framesThisStep;
            if (*(s8*)&state->throwState == 1)
            {
                ObjHits_SetHitVolumeSlot((ObjAnimComponent*)obj, SMALLBASKET_HIT_VOLUME_SLOT, 1, 0);
                if ((obj)->anim.velocityY > (-10.0f))
                {
                    (obj)->anim.velocityY = -0.12f * timeDelta + (obj)->anim.velocityY;
                }
                ObjHits_EnableObject(obj);
            }
            (obj)->anim.localPosX = (obj)->anim.velocityX * timeDelta + (obj)->anim.localPosX;
            (obj)->anim.localPosY = (obj)->anim.velocityY * timeDelta + (obj)->anim.localPosY;
            (obj)->anim.localPosZ = (obj)->anim.velocityZ * timeDelta + (obj)->anim.localPosZ;
            smallbasket_resolveCollision(obj);
            contactFlags = (*(ObjHitsPriorityState**)&(obj)->anim.hitReactState)->contactFlags;
            if ((contactFlags != 0) && (*(s8*)&state->throwState == 1))
            {
                blk.fy = (obj)->anim.localPosX;
                blk.fz = (obj)->anim.localPosY;
                blk.fw = (obj)->anim.localPosZ;
                objLightFn_8009a1dc((void*)obj, gSmallBasketFxScale[0], &blk, 1, 0);
                (**(void (**)(int, int, int, int, int, int))(*(int*)gSmallBasketResource + 0x4))((int)obj, 1, 0, 2, -1,
                                                                                                 0);
                Sfx_PlayFromObject((int)obj, (u16)state->sfxId);
                state->disableTimer = 0x32;
                state->throwState = 0;
                *(u8*)&(obj)->anim.resetHitboxMode |= INTERACT_FLAG_DISABLED;
                smallbasket_spawnContents(obj, player, state);
                zf = gSmallBasketZero[0];
                (obj)->anim.velocityX = zf;
                (obj)->anim.velocityZ = zf;
                ObjHits_ClearHitVolumes((ObjAnimComponent*)obj);
            }
            else if ((contactFlags != 0) && (*(s8*)&state->throwState == 2))
            {
                zf = gSmallBasketZero[0];
                (obj)->anim.velocityX = zf;
                (obj)->anim.velocityZ = zf;
                state->disableTimer = 500;
                state->throwState = 0;
                (obj)->userData2 = 0;
                ObjHits_EnableObject(obj);
                *(u8*)&(obj)->anim.resetHitboxMode &= ~INTERACT_FLAG_DISABLED;
                ObjHits_ClearHitVolumes((ObjAnimComponent*)obj);
            }
        }
        state->randomTimer -= framesThisStep;
        if (state->carryState != 0)
        {
            if (getXZDistance(&(obj)->anim.worldPosX, &((ObjPlacement*)def)->posX) >=
                (f32)(state->leashRange * state->leashRange))
            {
                zf = gSmallBasketZero[0];
                (obj)->anim.velocityX = zf;
                (obj)->anim.velocityZ = zf;
                state->disableTimer = 500;
                state->throwState = 0;
                (obj)->userData2 = 0;
                ObjHits_EnableObject(obj);
                *(u8*)&(obj)->anim.resetHitboxMode &= ~INTERACT_FLAG_DISABLED;
                ObjHits_ClearHitVolumes((ObjAnimComponent*)obj);
            }
        }
        else
        {
            fn_801814D0(obj, player, (u8*)state);
        }
        if ((state->randomTimer <= 0) && (state->carryState != 0))
        {
            subtype = state->subtype;
            if ((subtype == 5) || (subtype == 6))
            {
                Sfx_PlayFromObject((int)obj, SFXTRIG_id_6c);
                state->randomTimer = (s16)(randomGetRange(0, 100) + 0x12c);
            }
            else if (((u8)(subtype - 1) <= 1) || (subtype == 3))
            {
                Sfx_PlayFromObject((int)obj, SFXTRIG_vineclimb116);
                state->randomTimer = (s16)(randomGetRange(0, 100) + 0x12c);
            }
        }
        if ((obj)->userData2 == 0)
        {
            (obj)->anim.flags &= ~OBJANIM_FLAG_HIDDEN;
        }
    }
}

void SmallBasket_init(GameObject* obj, SmallBasketPlacement* placement)
{
    CfperchState* state;
    s16 v1c;
    s16 mode;

    state = (obj)->extra;
    ObjHits_DisableObject(obj);
    ObjGroup_AddObject((int)obj, SMALLBASKET_OBJGROUP);

    v1c = placement->respawnMinutes;
    if (v1c == 0)
    {
        state->respawnDelay = 0;
    }
    else
    {
        state->respawnDelay = v1c * 0x3c;
    }

    gSmallBasketResource = Resource_Acquire(SMALLBASKET_RESOURCE_ID, 1);
    state->randomTimer = (s16)(randomGetRange(0, 0x64) + 0x12c);
    state->unk1F = (u8)placement->unk1A;
    (obj)->anim.rotX = (s16)(placement->rotX << 8);
    state->enableGameBit = placement->enableGameBit;
    state->leashRange = placement->leashRange;
    if (state->leashRange == 0)
    {
        state->leashRange = 0x14;
    }
    state->respawnTimer = 0x320;
    (obj)->objectFlags |= OBJECT_OBJFLAG_HITDETECT_DISABLED;
    state->subtype = placement->subtype;
    (obj)->anim.previousLocalPosX = (obj)->anim.localPosX;
    (obj)->anim.previousLocalPosY = (obj)->anim.localPosY;
    (obj)->anim.previousLocalPosX = (obj)->anim.localPosZ;

    if ((u32)mainGetBit(state->enableGameBit) != 0)
    {
        state->hiddenTimer = 1;
        ObjHits_DisableObject(obj);
    }

    mode = (obj)->anim.seqId;
    if (mode == 0x3cf)
    {
        state->sfxId = 0x60;
    }
    else if (mode == 0x662)
    {
        state->disguiseGated = 1;
        state->sfxId = 0x37d;
    }
    else
    {
        state->sfxId = 0x4a;
    }
}
