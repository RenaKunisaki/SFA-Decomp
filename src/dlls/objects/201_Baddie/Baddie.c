/*
 * DLL 0xC9 (Baddie) - the generic enemy/baddie controller. It runs several romlist
 * enemy types, including GCRobotPatrol ("GCRobotPatr[ol]"), the floating
 * patrol robot of CloudRunner Fortress (placed in fortress.romlist).
 * GCRobotPatrol carries the GCRobotLightBeam searchlight (DLL 0x150) as
 * childObjs[0] and reads that child's "player caught" flag to react; the
 * SharpClaw disguise fools the beam.
 */
#include "dlls/objects/237.h"
#include "dlls/object_descriptor.h"
#include "main/camera_interface.h"
#include "main/dll/objfx_api.h"
#include "main/objfx.h"
#include "main/newshadows_audio_api.h"
#include "main/dll/dll_005A_staffcollisionfunc03.h"
#include "main/object_render.h"
#include "main/track_bbox_api.h"
#include "main/track_dolphin_api.h"
#include "main/objanim.h"
#include "game/objects/object.h"
#include "main/dll/player_api.h"
#include "main/dll/baddie_placement.h"
#include "main/dll/baddie_setmove.h"
#include "main/dll/boneparticleeffect_interface.h"
#include "main/obj_group.h"
#include "main/obj_link.h"
#include "main/objprint_character_api.h"
#include "sys/objects/lifecycle.h"
#include "sys/objects.h"
#include "main/model.h"
#include "main/mm.h"
#include "main/objseq.h"
#include "main/dll/rom_curve_interface.h"
#include "main/dll/dll_00C9_enemy.h"
#include "main/dll/tricky_state.h"
#include "main/audio/sfx_ids.h"
#include "main/audio/sfx_keep_alive_api.h"
#include "main/audio/sfx_trigger_ids.h"
#include "game/objects/object_setup.h"
#include "main/objhits.h"
#include "main/dll_000A_expgfx.h"
#include "main/dll/path_control_interface.h"
#include "main/mapEventTypes.h"
#include "main/resource.h"
#include "main/vecmath.h"
#include "main/dll/duster.h"
#include "main/gamebits.h"
#include "main/dll/tricky_api.h"
#include "main/lightmap_api.h"
#include "main/shader_api.h"
#include "main/frame_timing.h"
#include "main/model_engine.h"
#include "main/model_light.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "dolphin/mtx/mtx_legacy.h"
#include "main/dll/hagabon_mk2.h"
#include "main/dll/duster_wb.h"
#include "main/dll/weevil.h"
#include "main/dll/hoodedzyck.h"
#include "main/dll/snowworm.h"
#include "main/dll/kooshy.h"
#include "main/dll/mikaladon.h"
#include "main/dll/baddiewhirlpool.h"
#include "main/dll/newseqobj_baddie.h"
#include "main/dll/fireflyLantern.h"
#include "main/dll/firecrawler_baddie.h"
#include "main/dll/seqobj11e_baddie.h"
#include "main/dll/wispbaddie_baddie.h"
#include "main/dll/seqobj11d_baddie.h"
#include "main/dll/magicPlant.h"
#include "main/dll/seqObj11D.h"
#include "main/dll/dll_00C4_tricky.h"
#include "main/dll/fall_ladders.h"
#include "string.h"
#include "main/gameloop_gamebit_api.h"
#include "main/dll/dll_80136a40.h"
#include "main/obj_path.h"

u8 lbl_8031DBD8[12] = {0};
u8 lbl_8031DBE4[12] = {0};

int lbl_803DBC58[2] = {2, 3};
f32 lbl_803DBC60 = 20.0f;
f32 lbl_803DBC64 = 20.0f;
f32 lbl_803DBC68 = 2.3509887e-38f;

typedef struct BaddieAfterUpdateBonesCbState
{
    u8 pad0[0x2B0 - 0x0];
    s16 unk2B0;
    u16 unk2B2;
    u8 pad2B4[0x2D8 - 0x2B4];
    f32 freezeRecoverTimer;
    u32 unk2DC;
    u8 pad2E0[0x2F2 - 0x2E0];
    u8 unk2F2;
    u8 unk2F3;
    u8 unk2F4;
    u8 pad2F5[0x36C - 0x2F5];
    s32 tailBoneChain; /* 0x36C: bone chain passed to ObjModelChain_Update for tail sim */
} BaddieAfterUpdateBonesCbState;

typedef struct
{
    f32 dx, dy, dz;
    u8 pad0[2];
    s16 dAngle;
    u8 pad1[3];
    s8 events[8];
    s8 eventCount;
} TrickyMoveResult;

struct TrickyCommandSpawnPair
{
    u32 a;
    u32 b;
};

typedef struct
{
    s16 rot[3];
    f32 scale;
    Vec pos;
} FrozenFxParams;

typedef struct
{
    u8 fadeCounter : 5;
    u8 low : 3;
} FrozenByte2F6;

typedef struct BaddieInstantiateWeaponPlacement
{
    u8 pad0[0x4 - 0x0];
    u8 unk4;
    u8 unk5;
    u8 unk6;
    u8 unk7;
    f32 unk8;
    f32 unkC;
    f32 unk10;
    u8 pad14[0x18 - 0x14];
} BaddieInstantiateWeaponPlacement;

struct VisBits16
{
    u32 w[4];
};

STATIC_ASSERT(sizeof(struct VisBits16) == 0x10);

const struct VisBits16 gTrickyVisibilityBitsInit = {{0x10000, 0x20000, 0x40000, 0x80000}};
const StaffCollisionColorArgs gTrickyFrozenFxColors = {0x08, 0xFF, 0xFF, 0x78};

int gTrickyNearestObject;
StaffCollisionInterface** gBaddieStaffCollisionInterface;

#define ENEMY_OBJFLAG_PARENT_SLACK 0x1000
#define ENEMY_OBJFLAG_FREED        0x40

/* object groups: the enemy's own group / secondary group left on a message */
#define ENEMY_OBJGROUP           3
#define ENEMY_OBJGROUP_SECONDARY 0x50

/* camera mode DLL 0x49 = dll_0049_cameramodecombat */
#define ENEMY_CAMMODE_COMBAT 0x49

/* enemy defNos (anim.seqId) - names read from retail OBJECTS.bin at def+0x91;
   every id below gates to this file's own DLL 0xC9 */
#define ENEMY_SHARPCLAW_GR_OBJ     0x11
#define ENEMY_GUARDCLAW_OBJ        0xd8
#define ENEMY_SHARPCLAW_SN_OBJ     0x13a
#define ENEMY_PINPON_OBJ           0x251
#define ENEMY_RACHNOP_OBJ          0x25d
#define ENEMY_WEEVIL_OBJ           0x369
#define ENEMY_VAMBAT_OBJ           0x3fe
#define ENEMY_BATTLEDROID_OBJ      0x427
#define ENEMY_SPITTINGEBA_OBJ      0x457
#define ENEMY_MUTATEDEBA_OBJ       0x458
#define ENEMY_HOODEDZYCK_OBJ       0x4ac
#define ENEMY_WB_OBJ               0x4d7
#define ENEMY_KOOSHY_OBJ           0x58b
#define ENEMY_SHARPCLAW_CO_OBJ     0x5b7
#define ENEMY_SHARPCLAW_AS_OBJ     0x5b8
#define ENEMY_SHARPCLAW_SH_OBJ     0x5b9
#define ENEMY_SHARPCLAW_SO_OBJ     0x5e1
#define ENEMY_GCROBOTPATROL_OBJ    0x613
#define ENEMY_MIKALADON_OBJ        0x642
#define ENEMY_FIRECRAWLER_OBJ      0x6a2
#define ENEMY_REDEYE_OBJ           0x6a3
#define ENEMY_SHADOWHUNTER_OBJ     0x6a4
#define ENEMY_SWAMPSTRIDER_OBJ     0x6a5
#define ENEMY_BOSSGENERAL_OBJ      0x7a6
#define ENEMY_FIREBAT_OBJ          0x7c6
#define ENEMY_HAGABONMK2_OBJ       0x7c8
#define ENEMY_SNOWWORM_OBJ         0x842
#define ENEMY_SNOWWORM_BABY_OBJ    0x84b
#define ENEMY_WHIRLPOOL_OBJ        0x851

#define TRICKY_CHILD_OBJ_MAGIC_DUST 0x2cd /* "MagicDustMi..." (DLL 0xFF magicgem) */
#define TRICKY_CHILD_OBJ_ENERGY_EGG 0xb   /* "EnergyEgg" (DLL 0xED) */
#define TRICKY_OBJ_APPLE            0x3cd /* "Apple" (DLL 0xED) */

#define TRICKY_CONTROL_FLAG_BBOX_BLOCKS_SIGHT   0x00000008
#define TRICKY_CONTROL_FLAG_USE_SPECIAL_FLOOR_Y 0x08000000
#define TRICKY_CONTROL_FLAG_OFFSET_FLOOR_Y      0x20000000
#define TRICKY_CONTROL_FLAG_FLOOR_RESPONSE_MASK 0x28000002
#define TRICKY_SURFACE_FLAG_HAS_NEARBY_FLOOR    0x10

/* flags2DC status bits set by the floor-response pass (Tricky_applyFloorResponse /
 * Tricky_findNearbyFloorHeights) to record what floor correction ran this frame. */
#define TRICKY_STATE2DC_FLAG_FLOOR_OFFSET_APPLIED 0x08000000LL /* offset-floor-Y push applied */
#define TRICKY_STATE2DC_FLAG_FLOOR_SNAP_APPLIED   0x00100000LL /* snap-to-floor velocity applied */
#define TRICKY_STATE2DC_FLAG_SPECIAL_FLOOR_FOUND  0x10000000LL /* a nearby type-0xe special floor was found */

/* ObjPlacement offsets read by the defeat handler to fire the baddie's
 * death gamebits. */
#define BADDIE_PLACEMENT_DEATH_GAMEBIT          0x18 /* s16: gamebit incremented on defeat */
#define BADDIE_PLACEMENT_CLEAR_ON_DEATH_GAMEBIT 0x1a /* s16: gamebit cleared on defeat */

static const u16 lbl_803E2558[4] = { 0x2C4, 0x2CD, 0x2CE, 0x2CF };
static const u16 lbl_803E2560[2] = { 0x3CD, 0xB };
static const u16 lbl_803E2564[2] = { 0x3CD, 0x2C4 };
static const u16 lbl_803E2568[1] = { 0xB };

void baddie_updateEngagementState(GameObject* obj, TrickyState* sub);
void baddieTurnTowardTarget(GameObject* node, TrickyState* sub);
void baddie_decodePlayerAttackFlags(TrickyState* state, u32 flags, f32 f, u16 val);
void Tricky_findNearbyFloorHeights(GameObject* obj, int state, f32* nearestFloorY, f32* nearestSpecialY);
typedef struct
{
    f32 x, y, z;
} TrickyVec3;

extern u8 lbl_8031DBD8[];
extern u8 lbl_8031DBE4[];

void Tricky_resumeAfterCommand(GameObject* obj, int state)
{
    ObjHitsPriorityState* hitState;
    u8 moveId;

    ((TrickyState*)state)->actionId = 1;
    if (((((TrickyState*)state)->flags2DC & 0x1000) != 0) && ((((TrickyState*)state)->flags2E0 & 0x1000) == 0))
    {
        (obj)->anim.flags = (obj)->anim.flags & ~OBJANIM_FLAG_HIDDEN;
        moveId = ((TrickyState*)state)->moveId0;
        ((TrickyState*)state)->animPlaySpeed = 1.0f / (60.0f * ((TrickyState*)state)->moveSpeedScale0);
        ((TrickyState*)state)->flags323 = 1;
        ObjAnim_SetCurrentMove((int)obj, moveId, 0.0f, OBJANIM_MOVE_CONTROL_SKIP_EVENT_COUNTDOWN);
        if ((obj)->anim.hitReactState != NULL)
        {
            hitState = (ObjHitsPriorityState*)(obj)->anim.hitReactState;
            hitState->suppressOutgoingHits = 0;
        }
        ((TrickyState*)state)->flags2E8 = ((TrickyState*)state)->flags2E8 | 4;
        Sfx_PlayFromObjectLimited((int)obj, SFXTRIG_holorays16, 2);
        ObjHits_EnableObject(obj);
    }
    if ((((TrickyState*)state)->flags2DC & 0x40000000) != 0)
    {
        ((TrickyState*)state)->animPlaySpeed = 0.0055555557f;
        ((TrickyState*)state)->flags323 = 0;
        ObjAnim_SetCurrentMove((int)obj, 0, 0.0f, 0);
        if ((obj)->anim.hitReactState != NULL)
        {
            hitState = (ObjHitsPriorityState*)(obj)->anim.hitReactState;
            hitState->suppressOutgoingHits = 0;
        }
        ((TrickyState*)state)->flags2DC = ((TrickyState*)state)->flags2DC & 0xffffef7f;
        ((TrickyState*)state)->flags2E8 = ((TrickyState*)state)->flags2E8 & ~(u64)0x4;
        ((TrickyState*)state)->currentMoveProgress = 0.0f;
        (obj)->anim.alpha = 0xff;
    }
    else
    {
        (obj)->anim.alpha = (int)(255.0f * (obj)->anim.currentMoveProgress);
        ((TrickyState*)state)->currentMoveProgress = (obj)->anim.currentMoveProgress;
    }
}

void tricky_handleDefeat(GameObject* obj, int state)
{
    ObjHitsPriorityState* hitState;
    int setup;
    int alpha;
    void* tricky;
    int spawnBits;
    u8 moveId;

    setup = *(int*)&(obj)->anim.placementData;
    ((TrickyState*)state)->actionId = 0;
    if (((((TrickyState*)state)->flags2DC & 0x800) != 0) && ((((TrickyState*)state)->flags2E0 & 0x800) == 0))
    {
        tricky = (void*)getTrickyObject();
        if (tricky != NULL)
        {
            trickyImpress((GameObject*)tricky);
        }
        /* Skip the death gamebits when the baddie is sequence-driven so
         * scripted/cutscene deaths don't count. */
        if ((((TrickyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) == 0)
        {
            if (*(s16*)(setup + BADDIE_PLACEMENT_DEATH_GAMEBIT) != -1)
            {
                gameBitIncrement(*(s16*)(setup + BADDIE_PLACEMENT_DEATH_GAMEBIT));
            }
            if (*(s16*)(setup + BADDIE_PLACEMENT_CLEAR_ON_DEATH_GAMEBIT) != -1)
            {
                mainSetBits(*(s16*)(setup + BADDIE_PLACEMENT_CLEAR_ON_DEATH_GAMEBIT), 0);
            }
        }
        ((TrickyState*)state)->actionTargetObj = NULL;
        ObjHits_DisableObject(obj);
        *(u8*)&(obj)->anim.resetHitboxMode = *(u8*)&(obj)->anim.resetHitboxMode | INTERACT_FLAG_DISABLED;
        moveId = ((TrickyState*)state)->moveId1;
        ((TrickyState*)state)->animPlaySpeed = 1.0f / (60.0f * ((TrickyState*)state)->moveSpeedScale1);
        ((TrickyState*)state)->flags323 = 1;
        ObjAnim_SetCurrentMove((int)obj, moveId, 0.0f, 0);
        if (*(void**)&(obj)->anim.hitReactState != NULL)
        {
            hitState = (ObjHitsPriorityState*)(obj)->anim.hitReactState;
            hitState->suppressOutgoingHits = 0;
        }
        ((TrickyState*)state)->flags2E8 = ((TrickyState*)state)->flags2E8 | 1;
        Sfx_PlayFromObject((u32)obj, SFXTRIG_wp_iceywindlp16_233);
        if (randomGetRange(0, 100) > 50)
        {
            if ((((TrickyState*)state)->controlFlags & 0x100000) != 0)
            {
                baddie_spawnRewardDrops(obj, state, ((TrickyState*)state)->spawnBits, 0, 4);
            }
            else
            {
                spawnBits = *(s16*)(setup + 0x22) & 0xf00;
                if (spawnBits != 0)
                {
                    baddie_spawnRewardDrops(obj, state, spawnBits, 0, 1);
                }
                spawnBits = *(s16*)(setup + 0x22) & 0xf000;
                if (spawnBits != 0)
                {
                    baddie_spawnRewardDrops(obj, state, spawnBits, 0, 2);
                }
                spawnBits = *(s16*)(setup + 0x22) & 0xff;
                if (spawnBits != 0)
                {
                    baddie_spawnRewardDrops(obj, state, spawnBits, 0, 3);
                }
            }
        }
    }
    alpha = 0xff - (int)(255.0f * (obj)->anim.currentMoveProgress);
    alpha = (alpha < 0) ? 0 : ((alpha > 0xff) ? 0xff : alpha);
    (obj)->anim.alpha = alpha;
    ((TrickyState*)state)->currentMoveProgress = 1.0f + (f32)(0xff - (obj)->anim.alpha) / 255.0f;
    if ((obj)->anim.alpha < 5)
    {
        /* Fire the death gamebits for the sequence-driven path (the
         * faded-out branch). */
        if ((((TrickyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
        {
            if (*(s16*)(setup + BADDIE_PLACEMENT_DEATH_GAMEBIT) != -1)
            {
                gameBitIncrement(*(s16*)(setup + BADDIE_PLACEMENT_DEATH_GAMEBIT));
            }
            if (*(s16*)(setup + BADDIE_PLACEMENT_CLEAR_ON_DEATH_GAMEBIT) != -1)
            {
                mainSetBits(*(s16*)(setup + BADDIE_PLACEMENT_CLEAR_ON_DEATH_GAMEBIT), 0);
            }
        }
        ((TrickyState*)state)->currentMoveProgress = 0.0f;
        ((TrickyState*)state)->flags2DC = 0;
        (obj)->anim.flags = (obj)->anim.flags | OBJANIM_FLAG_HIDDEN;
        (obj)->anim.alpha = 0;
        *(u32*)&(obj)->userData1 = 1;
        if ((u32)((ObjPlacement*)setup)->mapId == 0xFFFFFFFF)
        {
            Obj_FreeObject(obj);
        }
        else
        {
            if (*(s16*)(setup + 0x2c) != 0)
            {
                (*gMapEventInterface)
                    ->addTime(((ObjPlacement*)setup)->mapId, 60.0f * (f32) * (s16*)(setup + 0x2c));
            }
            ((TrickyState*)state)->flags2DC = ((TrickyState*)state)->flags2DC & ~(u64)0x800;
            ((TrickyState*)state)->flags2E8 = ((TrickyState*)state)->flags2E8 & ~3LL;
        }
    }
}

/* Shared frozen-state update + per-baddie reaction dispatch. */
void baddie_updateWhileFrozen(GameObject* obj, u8* state, u8 fromHit)
{
    int player;
    int hit;
    int result;
    u16 sector;
    int diff;
    f32 hDist;
    f32 vDist;
    GameObject* proj;
    f32* dp;
    f32 zero;
    FrozenFxParams params;
    Vec hitPos;
    f32 delta[3];
    StaffCollisionColorArgs colors;
    int attacker;
    f32 fxA;
    f32 fxB;
    f32 fxC;
    int hitArg;
    u32 hitCount;
    u32 hitEffects;
    u16 impactSfx;

    player = (int)Obj_GetPlayerObject();
    colors = gTrickyFrozenFxColors;
    result = 2;
    if ((((TrickyState*)state)->flags2DC & 0x1800) == 0)
    {
        if ((((TrickyState*)state)->controlFlags & 1) != 0)
        {
            ObjHits_EnableObject(obj);
        }
        else
        {
            ObjHits_DisableObject(obj);
        }
        hit = ObjHits_GetPriorityHitWithPosition(obj, &attacker, &hitArg, &hitCount, &hitPos.x, &hitPos.y, &hitPos.z);
        hitPos.x += playerMapOffsetX;
        hitPos.z += playerMapOffsetZ;
        ((TrickyState*)state)->freezeStunTimer -= timeDelta;
        if (hit == 0x1a)
        {
            if (((TrickyState*)state)->freezeStunTimer >= 0.0f)
            {
                hit = 0;
            }
            else
            {
                ((TrickyState*)state)->freezeStunTimer = 5.0f;
            }
        }
        ((TrickyState*)state)->flags2DC = ((TrickyState*)state)->flags2DC & ~0x30LL;
        ((TrickyState*)state)->freezeRecoverTimer -= timeDelta;
        if (((TrickyState*)state)->freezeRecoverTimer < 0.0f)
        {
            ((TrickyState*)state)->freezeRecoverTimer = 0.0f;
        }
        fn_802972B4((GameObject*)(player), &hitEffects, &fxA, &fxB, &fxC, &impactSfx);
        baddie_decodePlayerAttackFlags((TrickyState*)state, hitEffects, fxA, impactSfx);
        if (hit != 0)
        {
            if (fromHit)
            {
                if (hit != 0x10)
                {
                    params.scale = 2.0f;
                    (*gBoneParticleEffectInterface)->spawnEffect((void*)obj, 0x7fb, NULL, 0x64, &params);
                    (*gBoneParticleEffectInterface)->spawnEffect((void*)obj, 0x7fc, NULL, 0x32, NULL);
                    Obj_Shatter(obj);
                    *(u16*)&((TrickyState*)state)->eventTime = 0;
                    ((TrickyState*)state)->flags2E8 = ((TrickyState*)state)->flags2E8 & ~0x20LL;
                    ((TrickyState*)state)->flags2E8 = ((TrickyState*)state)->flags2E8 | 0x200;
                    Sfx_PlayFromObject((u32)obj, SFXTRIG_barrel_bounce1);
                }
                else
                {
                    ((TrickyState*)state)->flags2E8 = ((TrickyState*)state)->flags2E8 | 0x10;
                }
            }
            else
            {
                if (hitEffects != 0)
                {
                    if (((GameObject*)attacker)->anim.classId == 1 || ((GameObject*)attacker)->anim.classId == 0x2d)
                    {
                        if ((((TrickyState*)state)->controlFlags & 0x200) != 0)
                        {
                            if (fxC >= 0.1f && fxC <= 1.0f)
                            {
                                ((TrickyState*)state)->base = fxC;
                            }
                            zero = 0.0f;
                            (obj)->anim.velocityX = zero;
                            (obj)->anim.velocityY = zero;
                            if ((((TrickyState*)state)->flags2DC & 0x40) != 0)
                            {
                                (obj)->anim.velocityZ = 0.3f * fxB;
                            }
                            else
                            {
                                (obj)->anim.velocityZ = fxB;
                            }
                            vecRotateZXY(&obj->anim.rotX, &obj->anim.velocityX);
                        }
                    }
                }
                ((TrickyState*)state)->freezeRecoverTimer += 30.0f * (f32)(int)hitCount;
                if ((((TrickyState*)state)->flags2DC & 0x4000) != 0)
                {
                    ((TrickyState*)state)->flags2DC = ((TrickyState*)state)->flags2DC | 0x10;
                }
                if ((((TrickyState*)state)->flags2DC & 0x40) == 0)
                {
                    ((TrickyState*)state)->flags2DC = ((TrickyState*)state)->flags2DC | 0x4000;
                }
                ((TrickyState*)state)->flags2DC = ((TrickyState*)state)->flags2DC | 0x20;
                dp = delta;
                dp[0] = (obj)->anim.worldPosX - hitPos.x;
                dp[1] = (obj)->anim.worldPosY - hitPos.y;
                dp[2] = (obj)->anim.worldPosZ - hitPos.z;
                diff = (u16)getAngle(-dp[0], -dp[2]) - (u16)(obj)->anim.rotX;
                if (diff > 0x8000)
                {
                    diff -= 0xffff;
                }
                if (diff < -0x8000)
                {
                    diff += 0xffff;
                }
                sector = (u32)(u16)diff >> 13;
                hDist = sqrtf(dp[0] * dp[0] + dp[2] * dp[2]);
                vDist = sqrtf(dp[1] * dp[1]);
                switch ((obj)->anim.seqId)
                {
                case 0x11:
                case 0x13a:
                case 0x5b7:
                case 0x5b8:
                case 0x5b9:
                case 0x5e1:
                case 0x7a6:
                    result = sharpClawHandleHitMessage(obj, state, (GameObject*)attacker, hit, hitArg, hitCount,
                                                          &hitPos, sector, hDist, vDist);
                    break;
                case 0xd8:
                case 0x281:
                    guardClawUpdateWhileFrozen((GameObject*)(obj), state, attacker, hit, hitArg, hitCount, &hitPos, sector);
                    break;
                case 0x613:
                    gcRobotPatrol_updateWhileFrozen((int)obj, state, attacker, hit, hitArg, hitCount, &hitPos, sector);
                    break;
                case 0x642:
                    mikaladon_updateWhileFrozen((int)obj, state, attacker, hit, hitArg, hitCount, &hitPos, sector);
                    break;
                case 0x3fe:
                case 0x7c6:
                    vambat_updateWhileFrozen((int)obj, state, attacker, hit, hitArg, hitCount, &hitPos, sector);
                    break;
                case 0x58b:
                    kooshy_updateWhileFrozen(obj, state, attacker, hit, hitArg, hitCount, &hitPos, sector);
                    break;
                case 0x369:
                    weevil_updateWhileFrozen(obj, state, attacker, hit, hitArg, hitCount, &hitPos, sector);
                    break;
                case 0x251:
                    pinPon_updateWhileFrozen(obj, (BaddieState*)state, attacker, hit, hitArg, hitCount, &hitPos, sector);
                    break;
                case 0x25d:
                    rachnopUpdateWhileFrozen((int)obj, state, attacker, hit, hitArg, hitCount, &hitPos, sector);
                    break;
                case 0x4d7:
                    wbUpdateWhileFrozen((int)obj, state, attacker, hit, hitArg, hitCount, &hitPos, sector);
                    break;
                case 0x457:
                    spittingEbaUpdateWhileFrozen((int)obj, state, attacker, hit, hitArg, hitCount, &hitPos, sector);
                    break;
                case 0x458:
                    mutatedEbaUpdateWhileFrozen((int)obj, state, attacker, hit, hitArg, hitCount, &hitPos, sector);
                    break;
                case 0x851:
                    whirlpool_updateWhileFrozen((int)obj, state, attacker, hit, hitArg, hitCount, &hitPos, sector);
                    break;
                case 0x842:
                case 0x84b:
                    snowworm_updateWhileFrozen((int)obj, state, attacker, hit, hitArg, hitCount, &hitPos, sector);
                    break;
                case 0x4ac:
                    hoodedZyckUpdateWhileFrozen((int)obj, state, attacker, hit, hitArg, hitCount, &hitPos, sector);
                    break;
                case 0x427:
                    battleDroidUpdateWhileFrozen((int)obj, state, attacker, hit, hitArg, hitCount, &hitPos, sector);
                    break;
                case 0x6a2:
                case 0x6a3:
                case 0x6a4:
                case 0x6a5:
                    crawler_onHit(obj, state, (GameObject*)attacker, hit, hitArg, hitCount, &hitPos, sector);
                    break;
                case 0x7c8:
                    hagabonMK2_updateWhileFrozen((int)obj, state, attacker, hit, hitArg, hitCount, &hitPos, sector);
                    break;
                default:
                    battleDroidUpdateWhileFrozen((int)obj, state, attacker, hit, hitArg, hitCount, &hitPos, sector);
                    break;
                }
            }
        }
        else
        {
            if ((((TrickyState*)state)->flags2DC & 0x40000000) != 0)
            {
                ((TrickyState*)state)->flags2DC = ((TrickyState*)state)->flags2DC & ~0x4000LL;
            }
        }
        if ((((TrickyState*)state)->flags2E8 & 0x208) != 0)
        {
            params.pos.x = hitPos.x;
            params.pos.y = hitPos.y;
            params.pos.z = hitPos.z;
            if (*(void**)&((TrickyState*)state)->light == NULL)
            {
                ((TrickyState*)state)->light = (int)objCreateLight(NULL, 1);
            }
            if ((((TrickyState*)state)->flags2E8 & 0x200) != 0)
            {
                objLightFn_8009a1dc((void*)obj, 0.014f, &params, 1, (void*)((TrickyState*)state)->light);
            }
            else if ((((TrickyState*)state)->flags2F1 & 0x10) != 0)
            {
                objLightFn_8009a1dc((void*)obj, 0.014f, &params, 3, (void*)((TrickyState*)state)->light);
            }
            else if ((((TrickyState*)state)->flags2F1 & 8) != 0)
            {
                objLightFn_8009a1dc((void*)obj, 0.014f, &params, 2, (void*)((TrickyState*)state)->light);
            }
            else
            {
                objLightFn_8009a1dc((void*)obj, 0.014f, &params, 1, (void*)((TrickyState*)state)->light);
            }
            Obj_SetModelColorFadeRecursive(obj, 0xf, 0xc8, 0, 0, 1);
        }
        ((TrickyState*)state)->freezeEffectTimer -= timeDelta;
        if (((TrickyState*)state)->freezeEffectTimer < 0.0f)
        {
            ((TrickyState*)state)->freezeEffectTimer = 0.0f;
        }
        if ((((TrickyState*)state)->flags2E8 & 0x10) != 0)
        {
            if (((TrickyState*)state)->freezeEffectTimer <= 0.0f)
            {
                params.pos.x = hitPos.x;
                params.pos.y = hitPos.y;
                params.pos.z = hitPos.z;
                params.scale = 1.0f;
                params.rot[2] = 0;
                params.rot[1] = 0;
                params.rot[0] = 0;
                if (gBaddieStaffCollisionInterface != NULL)
                {
                    (*gBaddieStaffCollisionInterface)->spawn(NULL, 1, (PartFxSpawnParams*)&params, 0x401, -1, &colors);
                }
                ((TrickyState*)state)->freezeEffectTimer = 20.0f;
                if (*(void**)&((TrickyState*)state)->light == NULL)
                {
                    ((TrickyState*)state)->light = (int)objCreateLight(NULL, 1);
                }
                objLightFn_8009a1dc((void*)obj, 0.014f, &params, 4, (void*)((TrickyState*)state)->light);
            }
            proj = ((TrickyState*)state)->actionTargetObj;
            if (proj != NULL && proj->anim.classId == 1)
            {
                fn_802961FC(proj, result);
            }
        }
        else if ((((TrickyState*)state)->flags2E8 & 0x20) != 0)
        {
            if (((FrozenByte2F6*)((TrickyState*)state)->pad2F6)->fadeCounter == 0)
            {
                Sfx_PlayFromObject((u32)obj, SFXTRIG_fox_kick2);
                ((FrozenByte2F6*)((TrickyState*)state)->pad2F6)->fadeCounter = 0x1f;
            }
            Obj_StartModelFadeIn(obj, 0x12c);
        }
        else
        {
            if (((FrozenByte2F6*)((TrickyState*)state)->pad2F6)->fadeCounter != 0)
            {
                ((FrozenByte2F6*)((TrickyState*)state)->pad2F6)->fadeCounter--;
            }
        }
        ((TrickyState*)state)->flags2E8 = ((TrickyState*)state)->flags2E8 & 0xfffffdc7;
    }
}

void baddie_decodePlayerAttackFlags(TrickyState* state, u32 flags, f32 f, u16 val)
{
    state->flags2F1 = 0;
    if ((flags & 0x2) != 0)
    {
        state->flags2F1 = (u8)(state->flags2F1 | 0x20);
    }
    if ((flags & 0x1) != 0)
    {
        state->flags2F1 = (u8)(state->flags2F1 | 0x40);
    }
    if ((flags & 0x4) != 0)
    {
        state->flags2F1 = (u8)(state->flags2F1 | 0x1);
    }
    if ((flags & 0x8) != 0)
    {
        state->flags2F1 = (u8)(state->flags2F1 | 0x2);
    }
    if ((flags & 0x10) != 0)
    {
        state->flags2F1 = (u8)(state->flags2F1 | 0x4);
    }
    if (0.2f == f)
    {
        state->flags2F1 = (u8)(state->flags2F1 | 0x8);
    }
    else if (0.3f == f)
    {
        state->flags2F1 = (u8)(state->flags2F1 | 0x10);
    }
    if ((flags & 0x80) != 0)
    {
        state->flags2F1 = (u8)(state->flags2F1 | 0x80);
    }
    if ((flags & 0x100) != 0)
    {
        state->spawnBits = 1;
    }
    else if ((flags & 0x200) != 0)
    {
        state->spawnBits = 2;
    }
    else if ((flags & 0x400) != 0)
    {
        state->spawnBits = 3;
    }
    state->impactSfxId = val;
}

int baddie_spawnRewardDrops(GameObject* obj, int state, int spawnBits, u32 useAltMode, u32 mode)
{
    u32 commandSpawnIds[2];
    struct TrickyRewardSpawnTail
    {
        u32 pair;
        u16 single;
    } rewardTail;
    f32 nearestDistance;
    u32 rewardSpawnIds0;
    int nearest;
    int parentSetup;
    int setup;
    int index;
    f32 savedX;
    f32 savedY;
    f32 savedZ;
    f32 v;

    (void)state;
    parentSetup = *(int*)&(obj)->anim.placementData;
    *(struct TrickyCommandSpawnPair*)commandSpawnIds = *(struct TrickyCommandSpawnPair*)lbl_803E2558;
    rewardSpawnIds0 = *(u32*)lbl_803E2560;
    rewardTail.pair = *(u32*)lbl_803E2564;
    rewardTail.single = lbl_803E2568[0];
    if (spawnBits == 0)
    {
        return 0;
    }
    if (Obj_IsLoadingLocked() == 0)
    {
        return 0;
    }
    mode = (u8)mode;
    if (mode == 1)
    {
        index = ((spawnBits & 0xf00) >> 8) - 1;
        if (index > 3)
        {
            index = 3;
        }
        setup = (int)Obj_AllocObjectSetup(0x30, *(u16*)((int)commandSpawnIds + index * 2));
    }
    else if (mode == 2)
    {
        index = ((spawnBits & 0xf000) >> 0xc) - 1;
        if (index > 1)
        {
            index = 1;
        }
        setup = (int)Obj_AllocObjectSetup(0x30, *(u16*)((int)&rewardSpawnIds0 + index * 2));
    }
    else if (mode == 3)
    {
        switch (spawnBits)
        {
        case 1:
            setup = (int)Obj_AllocObjectSetup(0x30, TRICKY_CHILD_OBJ_MAGIC_DUST);
            break;
        case 3:
            setup = (int)Obj_AllocObjectSetup(0x30, TRICKY_CHILD_OBJ_ENERGY_EGG);
            break;
        case 4:
            setup = (int)Obj_AllocObjectSetup(0x30, TRICKY_CHILD_OBJ_MAGIC_DUST);
            break;
        case 5:
            savedX = (obj)->anim.worldPosX;
            savedY = (obj)->anim.worldPosY;
            savedZ = (obj)->anim.worldPosZ;
            parentSetup = *(int*)&(obj)->anim.placementData;
            if ((void*)parentSetup != NULL)
            {
                (obj)->anim.worldPosX = ((ObjPlacement*)parentSetup)->posX;
                (obj)->anim.worldPosY = ((ObjPlacement*)parentSetup)->posY;
                (obj)->anim.worldPosZ = ((ObjPlacement*)parentSetup)->posZ;
            }
            nearestDistance = 750.0f;
            gTrickyNearestObject = ObjGroup_FindNearestObject(COLLECTIBLE_OBJECT_GROUP, obj, &nearestDistance);
            (obj)->anim.worldPosX = savedX;
            (obj)->anim.worldPosY = savedY;
            (obj)->anim.worldPosZ = savedZ;
            if ((void*)gTrickyNearestObject != NULL)
            {
                v = (obj)->anim.localPosX;
                ((GameObject*)gTrickyNearestObject)->anim.worldPosX = v;
                ((GameObject*)gTrickyNearestObject)->anim.localPosX = v;
                v = 15.0f + (obj)->anim.localPosY;
                ((GameObject*)gTrickyNearestObject)->anim.worldPosY = v;
                ((GameObject*)gTrickyNearestObject)->anim.localPosY = v;
                v = (obj)->anim.localPosZ;
                ((GameObject*)gTrickyNearestObject)->anim.worldPosZ = v;
                ((GameObject*)gTrickyNearestObject)->anim.localPosZ = v;
            }
            return gTrickyNearestObject;
        default:
            return 0;
        }
    }
    else if (mode == 4)
    {
        index = spawnBits;
        if (index > 3)
        {
            index = 3;
        }
        if (index <= 0)
        {
            return 0;
        }
        setup = (int)Obj_AllocObjectSetup(0x30, ((u16*)((u8*)&rewardTail.pair - 2))[index]);
    }
    *(u8*)(setup + 0x1a) = 0x14;
    *(s16*)(setup + 0x2c) = -1;
    *(s16*)(setup + 0x1c) = -1;
    *(s16*)(setup + 0x24) = -1;
    ((ObjPlacement*)setup)->posX = (obj)->anim.localPosX;
    ((ObjPlacement*)setup)->posY = 30.0f + (obj)->anim.localPosY;
    ((ObjPlacement*)setup)->posZ = (obj)->anim.localPosZ;
    if ((useAltMode & 0xff) != 0)
    {
        *(s16*)(setup + 0x2e) = 2;
    }
    else
    {
        *(s16*)(setup + 0x2e) = 1;
    }
    ((ObjPlacement*)setup)->color[0] = ((ObjPlacement*)parentSetup)->color[0];
    ((ObjPlacement*)setup)->color[2] = ((ObjPlacement*)parentSetup)->color[2];
    ((ObjPlacement*)setup)->color[1] = ((ObjPlacement*)parentSetup)->color[1];
    ((ObjPlacement*)setup)->color[3] = ((ObjPlacement*)parentSetup)->color[3];
    nearest = (int)Obj_SetupObject((ObjPlacement*)setup, 5, (obj)->anim.mapEventSlot, -1, (obj)->anim.parent);
    gTrickyNearestObject = nearest;
    if ((((GameObject*)nearest)->anim.seqId == TRICKY_OBJ_APPLE) || (((GameObject*)nearest)->anim.seqId == TRICKY_CHILD_OBJ_ENERGY_EGG))
    {
        (*(void (**)(int, f32, f32, f32))(*(int*)(*(int*)&((GameObject*)nearest)->anim.dll) + 0x2c))(
            nearest, 0.0f, 1.0f, 0.0f);
    }
    return gTrickyNearestObject;
}

void baddieInstantiateWeapon(GameObject* obj, int state)
{
    int parentSetup;
    void* child;
    int setup;

    parentSetup = *(int*)&(obj)->anim.placementData;
    if ((*(s16*)&((TrickyState*)state)->currentTime != *(s16*)(state + 0x2b6)) && ((obj)->anim.alpha != 0))
    {
        if ((obj)->childObjs[0] != NULL)
        {
            child = (obj)->childObjs[0];
            ObjLink_DetachChild(obj, child);
            Obj_FreeObject((GameObject*)child);
        }
        if (Obj_IsLoadingLocked() != 0)
        {
            if (*(s16*)(state + 0x2b6) > 0)
            {
                setup = (int)Obj_AllocObjectSetup(0x20, *(s16*)(state + 0x2b6));
                *(u8*)(setup + 5) = *(u8*)(setup + 5) | (((BaddieInstantiateWeaponPlacement*)parentSetup)->unk5 & 0x18);
                child = Obj_SetupObject((ObjPlacement*)setup, 4, (obj)->anim.mapEventSlot, -1, (obj)->anim.parent);
                ObjLink_AttachChild(obj, child, 0);
                *(s16*)&((TrickyState*)state)->currentTime = *(s16*)(state + 0x2b6);
            }
        }
        else
        {
            *(s16*)&((TrickyState*)state)->currentTime = 0;
        }
    }
}


u8 baddie_canSeeTarget(GameObject* obj, int state, void* from, void* to)
{
    u8 traceHit[4];
    s16 toGrid[4];
    s16 fromGrid[4];
    Vec probe;
    Vec delta;
    TrackBBoxHit bboxHit;
    s16 setupId;
    u8 visible;
    int keepGroundOffset;

    traceHit[0] = 0;
    visible = 0;
    if (((TrickyState*)state)->actionTargetObj != NULL)
    {
        probe.x = *(f32*)((int)from + 0);
        probe.y = *(f32*)((int)from + 4);
        probe.z = *(f32*)((int)from + 8);
        keepGroundOffset = 1;
        setupId = (obj)->anim.seqId;
        if (((((setupId != 0x613) && (setupId != 0x642)) && (setupId != 0x3fe)) &&
             ((setupId != 0x7c6) && (setupId != 0x7c8))) &&
            ((setupId != 0x251) && (setupId != 0x851)))
        {
            probe.y += 20.0f;
            keepGroundOffset = 0;
        }
        voxmaps_worldToGrid((f32*)&probe, fromGrid);
        probe.x = *(f32*)((int)to + 0);
        probe.y = 20.0f + *(f32*)((int)to + 4);
        probe.z = *(f32*)((int)to + 8);
        voxmaps_worldToGrid((f32*)&probe, toGrid);
        PSVECSubtract((f32*)from, (f32*)&probe, (f32*)&delta);
        if (PSVECMag((f32*)&delta) < 1905.0f)
        {
            if (*(u32*)&(obj)->anim.parent == 0)
            {
                visible = voxmaps_traceLine((VoxPos*)toGrid, (VoxPos*)fromGrid, NULL, traceHit, 0);
            }
            if ((keepGroundOffset == 0) && (traceHit[0] == 1))
            {
                visible = 1;
            }
        }
    }
    if ((visible != 0) && ((((TrickyState*)state)->controlFlags & TRICKY_CONTROL_FLAG_BBOX_BLOCKS_SIGHT) != 0))
    {
        if (objBboxFn_800640cc((f32*)from, (f32*)&probe, 1.0f, 0, &bboxHit, obj,
                               ((TrickyState*)state)->unk261, -1, 0, 0) != 0)
        {
            visible = 0;
        }
    }
    return visible;
}

void baddie_updateSightQuadrants(GameObject* obj, TrickyState* state, f32 radius)
{
    u8 traceHit[4];
    s16 probeGrid[4];
    s16 baseGrid[4];
    Vec probe;
    struct VisBits16 visibilityBits;
    Vec delta;
    TrackBBoxHit bboxHit;
    s16 baseAngle;
    u16 i;
    u8 visible;
    s16 setupId;
    f32 angle;

    visibilityBits = gTrickyVisibilityBitsInit;
    probe.x = obj->anim.localPosX;
    probe.y = 20.0f + obj->anim.localPosY;
    probe.z = obj->anim.localPosZ;
    voxmaps_worldToGrid((f32*)&probe, baseGrid);
    if (*(u32*)&obj->anim.parent != 0)
    {
        baseAngle = obj->anim.rotX + **(s16**)&obj->anim.parent;
    }
    else
    {
        baseAngle = obj->anim.rotX;
    }
    i = 0;
    for (; i < 4; i++)
    {
        angle = (3.1415927f * (f32)(s32)((s32)baseAngle + ((u32)(u16)i << 0xe))) / 32768.0f;
        probe.x = obj->anim.worldPosX - (radius * mathSinf(angle));
        probe.y = obj->anim.worldPosY;
        probe.z = obj->anim.worldPosZ - (radius * mathCosf(angle));
        setupId = obj->anim.seqId;
        if (((((setupId != 0x613) && (setupId != 0x642)) && (setupId != 0x3fe)) &&
             ((setupId != 0x7c6) && (setupId != 0x7c8))) &&
            ((setupId != 0x251) && (setupId != 0x851)))
        {
            probe.y += 20.0f;
        }
        voxmaps_worldToGrid((f32*)&probe, probeGrid);
        PSVECSubtract(&obj->anim.worldPosX, (f32*)&probe, (f32*)&delta);
        if (PSVECMag((f32*)&delta) < 1905.0f)
        {
            if (*(u32*)&obj->anim.parent != 0)
            {
                visible = 1;
            }
            else
            {
                visible = voxmaps_traceLine((VoxPos*)probeGrid, (VoxPos*)baseGrid, NULL, traceHit, 0);
                if (traceHit[0] == 1)
                {
                    visible = 1;
                }
            }
        }
        else
        {
            visible = 0;
        }
        if ((visible != 0) && ((state->controlFlags & TRICKY_CONTROL_FLAG_BBOX_BLOCKS_SIGHT) != 0))
        {
            if (objBboxFn_800640cc(&obj->anim.worldPosX, (f32*)&probe, 1.0f, 0, &bboxHit,
                                   obj,
                                   state->unk261, -1, 0, 0) != 0)
            {
                visible = 0;
            }
        }
        if (visible != 0)
        {
            state->flags2DC |= visibilityBits.w[i];
        }
        else
        {
            state->flags2DC &= ~visibilityBits.w[i];
        }
    }
}

void Tricky_applyFloorResponse(GameObject* obj, int state)
{
    f32 nearestFloorY;
    f32 nearestSpecialY;
    f32 points[6];
    u32 flags;
    f32 dy;

    ((TrickyState*)state)->flags2DC &= 0xf7efffff;
    flags = ((TrickyState*)state)->controlFlags;
    if ((flags & TRICKY_CONTROL_FLAG_FLOOR_RESPONSE_MASK) != 0)
    {
        Tricky_findNearbyFloorHeights(obj, state, &nearestFloorY, &nearestSpecialY);
        flags = ((TrickyState*)state)->controlFlags;
        if ((flags & TRICKY_CONTROL_FLAG_USE_SPECIAL_FLOOR_Y) != 0)
        {
            f32 sd = nearestSpecialY - (obj)->anim.localPosY;
            (obj)->anim.velocityY = sd * oneOverTimeDelta;
        }
        else if ((flags & TRICKY_CONTROL_FLAG_OFFSET_FLOOR_Y) != 0)
        {
            f32 dy = nearestFloorY - (obj)->anim.localPosY;
            if ((dy > -20.0f) && (dy < 20.0f))
            {
                f32 od = 25.0f + dy;
                (obj)->anim.velocityY = od * oneOverTimeDelta;
                ((TrickyState*)state)->flags2DC |= TRICKY_STATE2DC_FLAG_FLOOR_OFFSET_APPLIED;
            }
        }
        else
        {
            f32 dy = nearestFloorY - (obj)->anim.localPosY;
            if ((dy > -20.0f) && (dy < 20.0f))
            {
                (obj)->anim.velocityY = dy * oneOverTimeDelta;
                ((TrickyState*)state)->flags2DC |= TRICKY_STATE2DC_FLAG_FLOOR_SNAP_APPLIED;
            }
        }
        if ((((TrickyState*)state)->controlFlags & TRICKY_CONTROL_FLAG_BBOX_BLOCKS_SIGHT) == 0)
        {
            ((TrickyState*)state)->physicsActive = 0;
        }
    }
    else
    {
        if ((flags & 0xc) != 0)
        {
            ((TrickyState*)state)->physicsActive = 1;
        }
        else
        {
            ((TrickyState*)state)->physicsActive = 0;
        }
    }

    (*gPathControlInterface)->update((void*)obj, (void*)(state + 4), timeDelta);
    if ((((TrickyState*)state)->controlFlags & 4) != 0)
    {
        (*gPathControlInterface)->apply((void*)obj, (void*)(state + 4));
    }
    (*gPathControlInterface)->advance((void*)obj, (void*)(state + 4), timeDelta);

    if (((*(s8*)&((TrickyState*)state)->physicsActive != 0) &&
         ((((TrickyState*)state)->controlFlags & TRICKY_CONTROL_FLAG_FLOOR_RESPONSE_MASK) == 0)) &&
        ((*(s8*)&((TrickyState*)state)->surfaceFlags & TRICKY_SURFACE_FLAG_HAS_NEARBY_FLOOR) != 0))
    {
        (obj)->anim.velocityY = 0.0f;
        ((TrickyState*)state)->flags2DC |= TRICKY_STATE2DC_FLAG_FLOOR_SNAP_APPLIED;
    }
    if ((((TrickyState*)state)->controlFlags & 0x00200000) != 0)
    {
        ObjPath_GetPointWorldPositionArray(obj, 2, 2, points);
        objAudioFn_8006edcc(obj, ((TrickyState*)state)->animEventMask, 7, points, (void*)(state + 4),
                            ((TrickyState*)state)->unk310, 1.0f);
    }
}

void Tricky_findNearbyFloorHeights(GameObject* obj, int state, f32* nearestFloorY, f32* nearestSpecialY)
{
    TrackGroundHit** hitList[2];
    u16 hitCount;
    u16 i;
    TrackGroundHit* hit;
    f32 hitY;
    f32 zero;
    f32 nearestSpecialDelta;
    f32 nearestFloorDelta;
    f32 dy;
    f32 absDy;
    f32 defaultY;

    defaultY = -1.0f;
    *nearestFloorY = defaultY;
    *nearestSpecialY = defaultY;
    hitCount = (u16)hitDetectFn_80065e50(obj, (obj)->anim.localPosX, (obj)->anim.localPosY,
                                         (obj)->anim.localPosZ, hitList, 0, 0);
    *nearestFloorY = (obj)->anim.localPosY;
    *nearestSpecialY = (obj)->anim.localPosY;
    nearestSpecialDelta = nearestFloorDelta = 99999.0f;
    i = 0;
    ((TrickyState*)state)->flags2DC &= ~TRICKY_STATE2DC_FLAG_SPECIAL_FLOOR_FOUND;
    zero = 0.0f;
    ((TrickyState*)state)->nearestSpecialDeltaY = zero;
    *(s8*)&((TrickyState*)state)->surfaceFlags &= ~TRICKY_SURFACE_FLAG_HAS_NEARBY_FLOOR;
    for (; i < hitCount; i++)
    {
        hit = hitList[0][i];
        hitY = hit->height;
        dy = hitY - (obj)->anim.localPosY;
        absDy = dy;
        if (dy < zero)
        {
            absDy = -dy;
        }
        if ((s8)hit->surfaceType == 0xe)
        {
            if (absDy < nearestSpecialDelta)
            {
                ((TrickyState*)state)->nearestSpecialDeltaY = dy;
                *(s8*)&((TrickyState*)state)->surfaceFlags |= TRICKY_SURFACE_FLAG_HAS_NEARBY_FLOOR;
                nearestSpecialDelta = absDy;
                *nearestSpecialY = hitList[0][i]->height;
                if (((TrickyState*)state)->nearestSpecialDeltaY > 20.0f)
                {
                    ((TrickyState*)state)->flags2DC |=
                        (TRICKY_STATE2DC_FLAG_SPECIAL_FLOOR_FOUND | TRICKY_STATE2DC_FLAG_FLOOR_SNAP_APPLIED);
                }
            }
        }
        else if (absDy < nearestFloorDelta)
        {
            *nearestFloorY = hitY;
            *(s8*)&((TrickyState*)state)->surfaceFlags |= TRICKY_SURFACE_FLAG_HAS_NEARBY_FLOOR;
            nearestFloorDelta = absDy;
        }
    }
}


void enemyObjAnimUpdate(short* obj, int state)
{
    f32 vy;
    f32 dz;
    f32 dx;
    f32 dy;
    u32 flags;
    int mode;
    int i;
    f32 vel;
    f32 c;
    f32 phase;
    f32 outY;
    TrickyMoveResult res;
    MatrixTransform rec;
    f32 mtx[16];

    memcpy((void*)(state + 0x2c4), (void*)(state + 0x2b8), 0xc);
    memcpy((void*)(state + 0x2b8), obj + 0x12, 0xc);
    if ((((TrickyState*)state)->controlFlags & 0x400) != 0)
    {
        characterDoEyeAnims((GameObject*)obj, (void*)(state + 0x26c));
    }
    if ((((TrickyState*)state)->actionTargetObj != NULL) && ((((TrickyState*)state)->controlFlags & 0x800) != 0))
    {
        characterSetHeadYawToTarget((GameObject*)obj, ((TrickyState*)state)->actionTargetObj,
                    (CharacterEyeAnimState*)(state + 0x26c), 0x19);
    }
    ((TrickyState*)state)->prevActionId = ((TrickyState*)state)->actionId;
    flags = ((TrickyState*)state)->flags2DC;
    if ((flags & 0x800) != 0)
    {
        tricky_handleDefeat((GameObject*)(obj), state);
    }
    else if ((flags & 0x1000) != 0)
    {
        Tricky_resumeAfterCommand((GameObject*)(obj), state);
    }
    else if ((flags & 0x20000000) != 0)
    {
        if ((flags & 0x400) != 0)
        {
            ((TrickyState*)state)->actionId = 3;
            switch (((GameObject*)obj)->anim.seqId)
            {
            case ENEMY_SHARPCLAW_GR_OBJ:
            case ENEMY_SHARPCLAW_SN_OBJ:
            case ENEMY_SHARPCLAW_CO_OBJ:
            case ENEMY_SHARPCLAW_AS_OBJ:
            case ENEMY_SHARPCLAW_SH_OBJ:
            case ENEMY_SHARPCLAW_SO_OBJ:
            case ENEMY_BOSSGENERAL_OBJ:
                sharpClawUpdateAttack((GameObject*)(obj), (u8*)state);
                break;
            case ENEMY_GUARDCLAW_OBJ:
            case 0x281:
                guardClaw_update((GameObject*)obj, (u8*)state);
                break;
            case ENEMY_GCROBOTPATROL_OBJ:
                gcRobotPatrol_update((GameObject*)obj, (u8*)state);
                break;
            case ENEMY_MIKALADON_OBJ:
                mikaladon_update((GameObject*)obj, (MikaladonState*)state);
                break;
            case ENEMY_VAMBAT_OBJ:
            case ENEMY_FIREBAT_OBJ:
                vambat_updateEngaged((GameObject*)(obj), state);
                break;
            case ENEMY_KOOSHY_OBJ:
                kooshy_updateEngaged((GameObject*)(obj), state);
                break;
            case ENEMY_WEEVIL_OBJ:
                weevil_updateEngaged((GameObject*)(obj), state);
                break;
            case ENEMY_PINPON_OBJ:
                pinPon_updateEngaged((GameObject*)(obj), (int*)state);
                break;
            case ENEMY_RACHNOP_OBJ:
                rachnopUpdateAttack((int*)obj, state);
                break;
            case ENEMY_SPITTINGEBA_OBJ:
                spittingEbaUpdateEngaged((GameObject*)(obj), state);
                break;
            case ENEMY_WB_OBJ:
                wbUpdateEngaged((u32)obj, state);
                break;
            case ENEMY_MUTATEDEBA_OBJ:
                mutatedEbaUpdateEngaged((u32)obj, state);
                break;
            case ENEMY_WHIRLPOOL_OBJ:
                iceBaddie_enterWhirlpoolGroup((GameObject*)obj, (GroundBaddieState*)state);
                break;
            case ENEMY_SNOWWORM_OBJ:
            case ENEMY_SNOWWORM_BABY_OBJ:
                snowworm_update((GameObject*)obj, (u8*)state);
                break;
            case ENEMY_HOODEDZYCK_OBJ:
                hoodedZyck_update((GameObject*)obj, (u8*)state);
                break;
            case ENEMY_BATTLEDROID_OBJ:
                battleDroidUpdateAttack((int)obj, state);
                break;
            case ENEMY_FIRECRAWLER_OBJ:
            case ENEMY_REDEYE_OBJ:
            case ENEMY_SHADOWHUNTER_OBJ:
            case ENEMY_SWAMPSTRIDER_OBJ:
                crawler_update((GameObject*)obj, (u8*)state);
                break;
            case ENEMY_HAGABONMK2_OBJ:
                hagabonMK2_updateB((GameObject*)obj, (u8*)state);
                break;
            case 0x7c7:
            default:
                battleDroidUpdateAttack((int)obj, state);
                break;
            }
        }
        else
        {
            ((TrickyState*)state)->actionId = 4;
            switch (((GameObject*)obj)->anim.seqId)
            {
            case ENEMY_SHARPCLAW_GR_OBJ:
            case ENEMY_SHARPCLAW_SN_OBJ:
            case ENEMY_SHARPCLAW_CO_OBJ:
            case ENEMY_SHARPCLAW_AS_OBJ:
            case ENEMY_SHARPCLAW_SH_OBJ:
            case ENEMY_SHARPCLAW_SO_OBJ:
            case ENEMY_BOSSGENERAL_OBJ:
                sharpClawUpdateApproach((GameObject*)(obj), (void*)state);
                break;
            case ENEMY_GUARDCLAW_OBJ:
            case 0x281:
                guardClaw_update((GameObject*)obj, (u8*)state);
                break;
            case ENEMY_GCROBOTPATROL_OBJ:
                gcRobotPatrol_update((GameObject*)obj, (u8*)state);
                break;
            case ENEMY_MIKALADON_OBJ:
                mikaladon_update((GameObject*)obj, (MikaladonState*)state);
                break;
            case ENEMY_VAMBAT_OBJ:
            case ENEMY_FIREBAT_OBJ:
                vambat_updateEngaged((GameObject*)(obj), state);
                break;
            case ENEMY_KOOSHY_OBJ:
                kooshy_updateEngaged((GameObject*)(obj), state);
                break;
            case ENEMY_WEEVIL_OBJ:
                weevil_updateEngaged((GameObject*)(obj), state);
                break;
            case ENEMY_PINPON_OBJ:
                pinPon_updateEngaged((GameObject*)(obj), (int*)state);
                break;
            case ENEMY_RACHNOP_OBJ:
                rachnopUpdateApproach((int*)obj, state);
                break;
            case ENEMY_SPITTINGEBA_OBJ:
                spittingEbaUpdateEngaged((GameObject*)(obj), state);
                break;
            case ENEMY_WB_OBJ:
                wbUpdateEngaged((u32)obj, state);
                break;
            case ENEMY_MUTATEDEBA_OBJ:
                mutatedEbaUpdateEngaged((u32)obj, state);
                break;
            case ENEMY_WHIRLPOOL_OBJ:
                iceBaddie_enterWhirlpoolGroup((GameObject*)obj, (GroundBaddieState*)state);
                break;
            case ENEMY_SNOWWORM_OBJ:
            case ENEMY_SNOWWORM_BABY_OBJ:
                snowworm_update((GameObject*)obj, (u8*)state);
                break;
            case ENEMY_HOODEDZYCK_OBJ:
                hoodedZyck_updateB((GameObject*)obj, (u8*)state);
                break;
            case ENEMY_BATTLEDROID_OBJ:
                battleDroidUpdate((int)obj, state);
                break;
            case ENEMY_FIRECRAWLER_OBJ:
            case ENEMY_REDEYE_OBJ:
            case ENEMY_SHADOWHUNTER_OBJ:
            case ENEMY_SWAMPSTRIDER_OBJ:
                crawler_updateB((GameObject*)obj, (u8*)state);
                break;
            case ENEMY_HAGABONMK2_OBJ:
                hagabonMK2_update((GameObject*)obj, (u8*)state);
                break;
            case 0x7c7:
            default:
                battleDroidUpdate((int)obj, state);
                break;
            }
        }
    }
    else if ((flags & 0x100) != 0)
    {
        ((TrickyState*)state)->actionId = 2;
        if (((((TrickyState*)state)->flags2DC & 0x100) != 0) && ((((TrickyState*)state)->flags2E0 & 0x100) == 0))
        {
            int moveId = ((TrickyState*)state)->moveId2;
            ((TrickyState*)state)->animPlaySpeed =
                1.0f / (60.0f * ((TrickyState*)state)->moveSpeedScale2);
            ((TrickyState*)state)->flags323 = 1;
            ObjAnim_SetCurrentMove((int)obj, moveId, 0.0f, OBJANIM_MOVE_CONTROL_SKIP_EVENT_COUNTDOWN);
            if (*(void**)(obj + 0x2a) != 0)
            {
                *(u8*)(*(int*)&((GameObject*)obj)->anim.hitReactState + 0x70) = 0;
            }
        }
        if ((((TrickyState*)state)->flags2DC & 0x40000000) != 0)
        {
            ((TrickyState*)state)->animPlaySpeed = 0.0055555557f;
            ((TrickyState*)state)->flags323 = 0;
            ObjAnim_SetCurrentMove((int)obj, 0, 0.0f, 0);
            if (*(void**)(obj + 0x2a) != 0)
            {
                *(u8*)(*(int*)&((GameObject*)obj)->anim.hitReactState + 0x70) = 0;
            }
            ((TrickyState*)state)->flags2DC &= ~0x100LL;
            ((GameObject*)obj)->anim.alpha = 0xff;
        }
        else
        {
            ((GameObject*)obj)->anim.alpha = (u8)(int)(255.0f * ((GameObject*)obj)->anim.currentMoveProgress);
            ((GameObject*)obj)->anim.flags = ((GameObject*)obj)->anim.flags & ~OBJANIM_FLAG_HIDDEN;
        }
    }
    else
    {
        ((TrickyState*)state)->actionId = 5;
        switch (((GameObject*)obj)->anim.seqId)
        {
        case ENEMY_SHARPCLAW_GR_OBJ:
        case ENEMY_SHARPCLAW_SN_OBJ:
        case ENEMY_SHARPCLAW_CO_OBJ:
        case ENEMY_SHARPCLAW_AS_OBJ:
        case ENEMY_SHARPCLAW_SH_OBJ:
        case ENEMY_SHARPCLAW_SO_OBJ:
        case ENEMY_BOSSGENERAL_OBJ:
            sharpClawUpdateIdle((GameObject*)obj, (u8*)state);
            break;
        case ENEMY_GUARDCLAW_OBJ:
        case 0x281:
            guardClaw_update((GameObject*)obj, (u8*)state);
            break;
        case ENEMY_GCROBOTPATROL_OBJ:
            gcRobotPatrol_update((GameObject*)obj, (u8*)state);
            break;
        case ENEMY_MIKALADON_OBJ:
            mikaladon_update((GameObject*)obj, (MikaladonState*)state);
            break;
        case ENEMY_VAMBAT_OBJ:
        case ENEMY_FIREBAT_OBJ:
            vambat_updateIdle((GameObject*)(obj), state);
            break;
        case ENEMY_KOOSHY_OBJ:
            kooshy_updateIdle((GameObject*)(obj), state);
            break;
        case ENEMY_WEEVIL_OBJ:
            weevil_updateIdle((GameObject*)(obj), state);
            break;
        case ENEMY_PINPON_OBJ:
            pinPon_updateIdle((GameObject*)(obj), state);
            break;
        case ENEMY_RACHNOP_OBJ:
            rachnopUpdateIdle((int*)obj, state);
            break;
        case ENEMY_SPITTINGEBA_OBJ:
            spittingEbaUpdateIdle((GameObject*)(obj), state);
            break;
        case ENEMY_WB_OBJ:
            wbUpdateIdle((u32)obj, state);
            break;
        case ENEMY_MUTATEDEBA_OBJ:
            mutatedEbaUpdateIdle((u32)obj, state);
            break;
        case ENEMY_WHIRLPOOL_OBJ:
            iceBaddie_leaveWhirlpoolGroup((GameObject*)obj, (GroundBaddieState*)state);
            break;
        case ENEMY_SNOWWORM_OBJ:
        case ENEMY_SNOWWORM_BABY_OBJ:
            snowworm_applyReactionState((GameObject*)obj, (int*)state);
            break;
        case ENEMY_HOODEDZYCK_OBJ:
            hoodedZyck_updateIdle((GameObject*)(obj), state);
            break;
        case ENEMY_BATTLEDROID_OBJ:
            battleDroidUpdate((int)obj, state);
            break;
        case ENEMY_FIRECRAWLER_OBJ:
        case ENEMY_REDEYE_OBJ:
        case ENEMY_SHADOWHUNTER_OBJ:
        case ENEMY_SWAMPSTRIDER_OBJ:
            crawler_updateC((GameObject*)obj, (u8*)state);
            break;
        case ENEMY_HAGABONMK2_OBJ:
            hagabonMK2_updateB((GameObject*)obj, (u8*)state);
            break;
        case 0x7c7:
        default:
            battleDroidUpdate((int)obj, state);
            break;
        }
    }
    if (((TrickyState*)state)->actionId != ((TrickyState*)state)->prevActionId)
    {
        ((TrickyState*)state)->flags2DC = ((TrickyState*)state)->flags2DC | 0x80000000;
    }
    else
    {
        ((TrickyState*)state)->flags2DC = ((TrickyState*)state)->flags2DC & 0x7fffffff;
    }
    res.eventCount = 0;
    if (ObjAnim_AdvanceCurrentMove((int)obj, ((TrickyState*)state)->animPlaySpeed,
                                                                    timeDelta, (ObjAnimEventList*)&res) != 0)
    {
        ((TrickyState*)state)->flags2DC |= 0x40000000LL;
    }
    else
    {
        ((TrickyState*)state)->flags2DC &= ~0x40000000LL;
    }
    ((TrickyState*)state)->animEventMask = 0;
    for (i = 0; i < res.eventCount; i++)
    {
        ((TrickyState*)state)->animEventMask |= 1 << res.events[i];
    }
    vy = 0.0f;
    if ((((((TrickyState*)state)->controlFlags & 0x20) != 0) &&
         ((((TrickyState*)state)->controlFlags & 0x400000) == 0)) &&
        (((((TrickyState*)state)->flags2DC & 0x1800) == 0) && ((((TrickyState*)state)->flags323 & 4) == 0)))
    {
        vy = -(((TrickyState*)state)->gravity * timeDelta - ((GameObject*)obj)->anim.velocityY);
    }
    vel = ((GameObject*)obj)->anim.velocityX;
    ((GameObject*)obj)->anim.velocityX =
        (vel < -10.0f) ? -10.0f : ((vel > 10.0f) ? 10.0f : vel);
    vel = ((GameObject*)obj)->anim.velocityY;
    ((GameObject*)obj)->anim.velocityY =
        (vel < -10.0f) ? -10.0f : ((vel > 10.0f) ? 10.0f : vel);
    vel = ((GameObject*)obj)->anim.velocityZ;
    ((GameObject*)obj)->anim.velocityZ =
        (vel < -10.0f) ? -10.0f : ((vel > 10.0f) ? 10.0f : vel);
    mode = 0;
    if (((((TrickyState*)state)->controlFlags & 0x80) != 0) && (((TrickyState*)state)->flags323 != 0))
    {
        mode = 1;
    }
    else if ((((TrickyState*)state)->controlFlags & 0x100) != 0)
    {
        mode = 2;
    }
    else if ((((TrickyState*)state)->controlFlags & 0x10) != 0)
    {
        mode = 3;
    }
    if (((((TrickyState*)state)->controlFlags & 0x200) != 0) && ((((TrickyState*)state)->flags2DC & 0x4010) != 0))
    {
        mode = 3;
    }
    if (mode == 1)
    {
        f32 zero;
        dx = (dz = 0.0f);
        dy = dz;
        if ((((TrickyState*)state)->flags323 & 2) != 0)
        {
            dx = res.dx * oneOverTimeDelta;
        }
        if ((((TrickyState*)state)->flags323 & 4) != 0)
        {
            dy = res.dy * oneOverTimeDelta;
        }
        if ((((TrickyState*)state)->flags323 & 1) != 0)
        {
            dz = -res.dz * oneOverTimeDelta;
        }
        if ((((TrickyState*)state)->flags323 & 8) != 0)
        {
            ((GameObject*)obj)->anim.rotX += res.dAngle;
        }
        rec.rotX = ((GameObject*)obj)->anim.rotX;
        rec.rotY = ((GameObject*)obj)->anim.rotY;
        rec.rotZ = ((GameObject*)obj)->anim.rotZ;
        rec.scale = 1.0f;
        zero = 0.0f;
        rec.x = zero;
        rec.y = zero;
        rec.z = zero;
        setMatrixFromObjectPos(mtx, &rec);
        if ((((TrickyState*)state)->flags323 & 4) != 0)
        {
            Matrix_TransformPoint(mtx, dx, dy, -dz, (f32*)(obj + 0x12), (f32*)(obj + 0x14), (f32*)(obj + 0x16));
        }
        else
        {
            Matrix_TransformPoint(mtx, dx, 0.0f, -dz, (f32*)(obj + 0x12), &outY, (f32*)(obj + 0x16));
        }
    }
    else if (mode == 2)
    {
        if (ObjAnim_SampleRootCurvePhase((ObjAnimComponent*)obj,
                                         sqrtf(((GameObject*)obj)->anim.velocityX * ((GameObject*)obj)->anim.velocityX +
                                               ((GameObject*)obj)->anim.velocityZ * ((GameObject*)obj)->anim.velocityZ),
                                         &phase) != 0)
        {
            ((TrickyState*)state)->animPlaySpeed = phase;
        }
    }
    else if (mode == 3)
    {
        if ((((TrickyState*)state)->flags2F1 & 0x80) == 0)
        {
            ((GameObject*)obj)->anim.velocityX =
                ((GameObject*)obj)->anim.velocityX * powfBitEstimate(((TrickyState*)state)->base, timeDelta);
            ((GameObject*)obj)->anim.velocityY =
                ((GameObject*)obj)->anim.velocityY * powfBitEstimate(((TrickyState*)state)->base, timeDelta);
            ((GameObject*)obj)->anim.velocityZ =
                ((GameObject*)obj)->anim.velocityZ * powfBitEstimate(((TrickyState*)state)->base, timeDelta);
        }
    }
    Tricky_applyFloorResponse((GameObject*)(obj), state);
    if (((((TrickyState*)state)->controlFlags & 0x400000) != 0) || ((((TrickyState*)state)->flags2DC & 0x8100000) != 0))
    {
        if ((((TrickyState*)state)->flags2F1 & 0x80) == 0)
        {
            objMove((GameObject*)obj, ((GameObject*)obj)->anim.velocityX * timeDelta, ((GameObject*)obj)->anim.velocityY * timeDelta,
                    ((GameObject*)obj)->anim.velocityZ * timeDelta);
        }
    }
    else if ((((TrickyState*)state)->controlFlags & 0x20) != 0)
    {
        f32 newY = (((GameObject*)obj)->anim.velocityY * timeDelta + ((GameObject*)obj)->anim.localPosY) -
                   0.5f * (((TrickyState*)state)->gravity * (timeDelta * timeDelta));
        if ((((TrickyState*)state)->flags2F1 & 0x80) == 0)
        {
            objMove((GameObject*)obj, ((GameObject*)obj)->anim.velocityX * timeDelta, newY - ((GameObject*)obj)->anim.localPosY,
                    ((GameObject*)obj)->anim.velocityZ * timeDelta);
            ((GameObject*)obj)->anim.velocityY = vy;
        }
    }
    else if ((((TrickyState*)state)->flags2F1 & 0x80) == 0)
    {
        objMove((GameObject*)obj, ((GameObject*)obj)->anim.velocityX * timeDelta, ((GameObject*)obj)->anim.velocityY * timeDelta,
                ((GameObject*)obj)->anim.velocityZ * timeDelta);
    }
}

void baddie_updateEngagementState(GameObject* obj, TrickyState* sub)
{
    GameObject* player;
    int* tricky;
    GameObject* target;
    GameObject* camTarget;

    player = Obj_GetPlayerObject();
    tricky = (int*)getTrickyObject();
    target = sub->actionTargetObj;
    if (target != NULL && (sub->controlFlags & 0x10000) == 0 &&
        (target != player || (player->objectFlags & ENEMY_OBJFLAG_PARENT_SLACK) == 0))
    {
        sub->flags2DC &= ~0x800000LL;
        camTarget = (GameObject*)(*gCameraInterface)->getOverrideTarget();
        if (camTarget == obj)
        {
            sub->flags2DC |= 0x800200LL;
        }
        {
            u16 dist = sub->targetDist;
            u16 near = (u16)(int)sub->waterLevel;
            if (dist < near)
            {
                sub->flags2DC |= 0x400LL;
                sub->flags2DC &= ~0x200LL;
            }
            else
            {
                f32 midf = ((BaddieState*)sub)->unk2A8;
                u16 mid = (u16)(int)midf;
                if (dist < mid)
                {
                    sub->flags2DC |= 0x200LL;
                    sub->flags2DC &= ~0x400LL;
                }
                else
                {
                    u16 far = (u16)(int)(1.39f * midf);
                    if (dist > far)
                    {
                        sub->flags2DC &= ~0x20000600LL;
                    }
                }
            }
        }
    }
    else
    {
        sub->flags2DC &= ~0x800600LL;
        if ((sub->controlFlags & 0x10000) != 0 ||
            (sub->actionTargetObj == player &&
             (player->objectFlags & ENEMY_OBJFLAG_PARENT_SLACK) != 0))
        {
            sub->flags2DC &= ~0x20000000LL;
        }
    }
    sub->flags2DC &= ~0x76f0008LL;
    if (tricky != NULL)
    {
        u8 r = (*(u8(**)(int*))(*(int*)*(int*)((char*)tricky + 0x68) + 0x40))(tricky);
        if (r != 0)
            sub->flags2DC |= 0x200000LL;
    }
    if (sub->actionTargetObj == player)
    {
        if (playerIsDisguised(player) != 0)
        {
            sub->flags2DC |= 8LL;
            if ((sub->controlFlags & BADDIE_CONTROL_PATH_FOLLOW) != 0)
            {
                sub->flags2DC &= ~0x800600LL;
            }
        }
    }
    if ((sub->flags2DC & 0x20000600) != 0)
    {
        if ((sub->controlFlags & 0x1000) != 0)
        {
            u8 r = baddie_canSeeTarget(obj, (int)sub, &obj->anim.worldPosX,
                                           (u8*)sub->actionTargetObj + 0x18);
            if (r != 0)
                sub->flags2DC |= 0x1000000LL;
            if ((sub->flags2DC & 0x1000000) == 0)
            {
                sub->flags2DC &= ~0x20000000LL;
            }
        }
        else
        {
            sub->flags2DC |= 0x1000000LL;
        }
        {
            u16 mode = sub->turnOctant;
            if (mode < 2 || mode > 5)
            {
                sub->flags2DC |= 0x400000LL;
            }
            else if ((sub->flags2DC & 0x1000000) != 0)
            {
                sub->flags2DC |= 0x2000000LL;
            }
        }
        if ((sub->controlFlags & 0x4000) == 0)
        {
            f32* t = (f32*)sub->actionTargetObj;
            f32 mag = sqrtf(t[11] * t[11] + (t[9] * t[9] + t[10] * t[10]));
            if (mag > 0.5f)
                sub->flags2DC |= 0x4000000LL;
        }
        if ((sub->flags2DC & 0x600) != 0 && (sub->flags2DC & 0x6800000) != 0 &&
            (sub->flags2DC & 0x1000000) != 0)
        {
            sub->flags2DC |= 0x20000000LL;
        }
        if ((sub->flags2DC & 0x20000000) != 0)
        {
            if ((sub->controlFlags & 0x40) != 0)
            {
                baddie_updateSightQuadrants(obj, sub, sub->waterLevel);
            }
            else
            {
                sub->flags2DC |= 0xf0000LL;
            }
        }
    }
    if (((BaddieState*)sub)->hitCounter == 0)
    {
        sub->flags2DC |= 0x800LL;
    }
}
void baddieTurnTowardTarget(GameObject* node, TrickyState* sub)
{
    GameObject* target = sub->actionTargetObj;
    if (target != NULL)
    {
        f32 d[3];
        f32* dp = d;
        int raw;
        s32 delta;
        f32 dist;
        u16 ua;

        if ((sub->controlFlags & 0x8000) != 0)
        {
            dp[0] = node->anim.worldPosX - target->anim.worldPosX;
            dp[1] = 0.0f;
            dp[2] = node->anim.worldPosZ - target->anim.worldPosZ;
        }
        else
        {
            dp[0] = node->anim.worldPosX - target->anim.worldPosX;
            dp[1] = node->anim.worldPosY - target->anim.worldPosY;
            dp[2] = node->anim.worldPosZ - target->anim.worldPosZ;
        }
        ua = getAngle(-dp[0], -dp[2]);
        if (*(int**)&node->anim.parent != NULL)
        {
            raw = (s16)(node->anim.rotX + **(s16**)&node->anim.parent);
        }
        else
        {
            raw = node->anim.rotX;
        }
        delta = ua - (u16)(s16)raw;
        if (delta > 0x8000)
            delta -= 0xFFFF;
        if (delta < -0x8000)
            delta += 0xFFFF;
        sub->turnAngleDelta = delta;
        sub->turnOctant = (u32)(u16)delta >> 13;

        {
            f32 sqX;
            f32 sqZ;
            f32 sqY;
            f32 t;
            t = dp[2];
            sqZ = t * t;
            t = dp[0];
            sqX = t * t;
            t = dp[1];
            sqY = t * t;
            dist = sqrtf(sqZ + (sqX + sqY));
        }
        *(s16*)&sub->targetDist = (s16)dist;

        {
            GameObject* targetObj = sub->actionTargetObj;
            *(s16*)&sub->targetHeightDelta =
                (s16)(targetObj->anim.worldPosY - node->anim.worldPosY);
        }
    }
}

u32 gEnemySelfAngleFlagClearMask[] = {
    0x40000, 0x80000, 0x80000, 0x10000, 0x10000, 0x20000, 0x20000, 0x40000,
};

u32 gEnemyTargetAngleFlagClearMask[] = {
    0x10000, 0x20000, 0x20000, 0x40000, 0x40000, 0x80000, 0x80000, 0x10000,
};

ObjectDescriptor gBaddieObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)enemy_initialise,
    (ObjectDescriptorCallback)enemy_release,
    0,
    (ObjectDescriptorCallback)enemy_init,
    (ObjectDescriptorCallback)enemy_update,
    (ObjectDescriptorCallback)enemy_hitDetect,
    (ObjectDescriptorCallback)enemy_render,
    (ObjectDescriptorCallback)enemy_free,
    (ObjectDescriptorCallback)enemy_getObjectTypeId,
    enemy_getExtraSize,
};

int enemy_SeqFn(GameObject* node, int unused, ObjAnimUpdateState* animUpdate)
{
    char* sub = *(char**)&node->extra;
    s8* n29 = *(s8**)&node->anim.placementData;
    int i;
    GameObject* obj;

    if (node->userData1 != 0)
        return 0;
    ((TrickyState*)sub)->flags2DC |= 0x8000LL;
    memcpy(sub + 0x2c4, sub + 0x2b8, 0xc);
    memcpy(sub + 0x2b8, (char*)node + 0x24, 0xc);
    for (i = 0; i < animUpdate->eventCount; i++)
    {
        switch (animUpdate->eventIds[i])
        {
        case 1:
            obj = getTrickyObject();
            if (obj != NULL)
            {
                (*(void (*)(GameObject*, int, GameObject*))(*(int*)(*(int*)(*(int*)&obj->anim.dll) + 0x34)))(
                    obj, 1, node);
                ((TrickyState*)sub)->flags2DC |= 0x200000LL;
                ((TrickyState*)sub)->actionTargetObj = obj;
            }
            break;
        case 4:
            obj = Obj_GetPlayerObject();
            if (obj != NULL)
            {
                ((TrickyState*)sub)->flags2DC &= ~0x200000LL;
                ((TrickyState*)sub)->actionTargetObj = obj;
            }
            break;
        case 2:
            if (node->anim.seqId == ENEMY_BOSSGENERAL_OBJ)
                *(u16*)(sub + 0x2b6) = 0x7a5;
            else
                *(u16*)(sub + 0x2b6) = 0x33;
            break;
        case 3:
            (*gObjectTriggerInterface)->setCamVars(ENEMY_CAMMODE_COMBAT, 4, (int)node, 0x3c);
            break;
        case 6:
            if (*(int**)&((TrickyState*)sub)->modelChain != NULL)
                ObjModelChain_SetEnabled(*(ObjModelChain**)&((TrickyState*)sub)->modelChain, 1);
            break;
        case 7:
            if (*(int**)&((TrickyState*)sub)->modelChain != NULL)
                ObjModelChain_SetEnabled(*(ObjModelChain**)&((TrickyState*)sub)->modelChain, 0);
            break;
        }
    }
    baddieInstantiateWeapon(node, (int)sub);
    if (node->seqIndex == -1)
    {
        ((TrickyState*)sub)->flags2E8 &= ~3LL;
        ObjHits_DisableObject(node);
        return 0;
    }
    if ((((TrickyState*)sub)->flags2DC & 0x1800) == 0)
    {
        baddieTurnTowardTarget(node, (TrickyState*)sub);
        baddie_updateEngagementState(node, (TrickyState*)sub);
    }
    if (n29[0x2e] != -1)
    {
        if ((((TrickyState*)sub)->flags2DC & 0x600) != 0)
        {
            if (animUpdate->sequenceSlot == node->seqIndex)
                return 4;
        }
    }
    return 0;
}

/* sidekickToy_updateCurveTargetLatch: pre-curve probe + state-bit gate. If state's 0x2000 bit is
 * set, ask baddie_canSeeTarget whether the target is locked on; on hit,
 * leave state[0x2dc] alone. Otherwise initialise the rom-curve walker with
 * (data, obj, 700.0f, &lbl_803DBC58, -1) and toggle
 * the 0x2000 bit based on the u8 result. */
void sidekickToy_updateCurveTargetLatch(GameObject* obj)
{
    u8* state = (obj)->extra;
    u8* data = *(u8**)state;
    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_PATH_FOLLOW) != 0)
    {
        if ((u8)baddie_canSeeTarget(obj, (int)state, &(obj)->anim.worldPosX, data + 0x68) != 0)
        {
            return;
        }
    }
    if ((*gRomCurveInterface)->initCurve(*(u8**)state, (void*)obj, 700.0f, (int*)&lbl_803DBC58, -1) != 0)
    {
        ((EnemyState*)state)->controlFlags &= ~(u64)BADDIE_CONTROL_PATH_FOLLOW;
    }
    else
    {
        ((EnemyState*)state)->controlFlags = ((EnemyState*)state)->controlFlags | BADDIE_CONTROL_PATH_FOLLOW;
    }
}

int enemy_findNearbyEnemies(GameObject* obj, f32 radius, u8 flags, int max, EnemyTargetSearchResult* out)
{
    EnemyTargetSearchResult* cur[1];
    int state;
    int resultCount;
    GameObject** arr;
    short ang;
    GameObject* tgt;
    u32 diff;
    int i;
    f32 distSquared;
    int count;
    TrickyVec3 d;
    void* dp = &d;

    cur[0] = 0;
    state = *(int*)&obj->extra;
    count = 0;
    resultCount = 0;
    if ((flags & 1) != 0)
    {
        tgt = (GameObject*)ObjGroup_FindNearestObject(ENEMY_OBJGROUP, obj, &radius);
        out->obj = tgt;
        if (tgt != 0)
        {
            out->dist = radius;
            resultCount = 1;
            if ((flags & 2) != 0)
            {
                if ((((TrickyState*)state)->controlFlags & 0x8000) != 0)
                {
                    d.x = obj->anim.worldPosX - out->obj->anim.worldPosX;
                    d.y = 0.0f;
                    d.z = obj->anim.worldPosZ - out->obj->anim.worldPosZ;
                }
                else
                {
                    d.x = obj->anim.worldPosX - out->obj->anim.worldPosX;
                    d.y = obj->anim.worldPosY - out->obj->anim.worldPosY;
                    d.z = obj->anim.worldPosZ - out->obj->anim.worldPosZ;
                }
                diff = getAngle(-d.x, -d.z) & 0xffff;
                if (obj->anim.parent != 0)
                {
                    ang = (s16)(obj->anim.rotX + *(s16*)obj->anim.parent);
                }
                else
                {
                    ang = obj->anim.rotX;
                }
                diff = diff - ((int)ang & 0xffffU);
                if (0x8000 < (int)diff)
                {
                    diff = diff - 0xffff;
                }
                if ((int)diff < -0x8000)
                {
                    diff = diff + 0xffff;
                }
                ang = (short)((diff & 0xffff) >> 0xd);
                ((TrickyState*)state)->flags2DC = ((TrickyState*)state)->flags2DC & ~gEnemySelfAngleFlagClearMask[ang];
                if ((flags & 4) != 0)
                {
                    ((TrickyState*)out->obj->extra)->flags2DC &= ~gEnemyTargetAngleFlagClearMask[ang];
                }
            }
        }
    }
    else
    {
        radius = radius * radius;
        arr = (GameObject**)ObjGroup_GetObjects(ENEMY_OBJGROUP, &count);
        if (count != 0)
        {
            i = 0;
            cur[0] = out;
            for (; i < count; i++)
            {
                distSquared = vec3f_distanceSquared(&obj->anim.worldPosX, &arr[i]->anim.worldPosX);
                if ((distSquared < radius) && (arr[i] != obj))
                {
                    cur[0]->obj = arr[i];
                    cur[0]->dist = sqrtf(distSquared);
                    if ((flags & 2) != 0)
                    {
                        if ((((TrickyState*)state)->controlFlags & 0x8000) != 0)
                        {
                            d.x = obj->anim.worldPosX - cur[0]->obj->anim.worldPosX;
                            d.y = 0.0f;
                            d.z = obj->anim.worldPosZ - cur[0]->obj->anim.worldPosZ;
                        }
                        else
                        {
                            d.x = obj->anim.worldPosX - cur[0]->obj->anim.worldPosX;
                            d.y = obj->anim.worldPosY - cur[0]->obj->anim.worldPosY;
                            d.z = obj->anim.worldPosZ - cur[0]->obj->anim.worldPosZ;
                        }
                        diff = getAngle(-d.x, -d.z) & 0xffff;
                        if (obj->anim.parent != 0)
                        {
                            ang = (s16)(obj->anim.rotX + *(s16*)obj->anim.parent);
                        }
                        else
                        {
                            ang = obj->anim.rotX;
                        }
                        diff = diff - ((int)ang & 0xffffU);
                        if (0x8000 < (int)diff)
                        {
                            diff = diff - 0xffff;
                        }
                        if ((int)diff < -0x8000)
                        {
                            diff = diff + 0xffff;
                        }
                        ang = (short)((diff & 0xffff) >> 0xd);
                        ((TrickyState*)state)->flags2DC =
                            ((TrickyState*)state)->flags2DC & ~gEnemySelfAngleFlagClearMask[ang];
                        if ((flags & 4) != 0)
                        {
                            ((TrickyState*)cur[0]->obj->extra)->flags2DC &= ~gEnemyTargetAngleFlagClearMask[ang];
                        }
                    }
                    cur[0]++;
                    resultCount++;
                    if (resultCount >= max)
                    {
                        i = count;
                    }
                }
            }
        }
    }
    return resultCount;
}

u8 enemy_getFreezeRecoverSeconds(GameObject* obj)
{
    int* state;
    f32 freezeRecoverTimer;
    f32 zero;
    if (obj != NULL)
    {
        state = obj->extra;
    }
    else
    {
        return 0;
    }
    if (state != NULL)
    {
        freezeRecoverTimer = ((EnemyState*)state)->freezeRecoverTimer;
        zero = 0.0f;
        if (freezeRecoverTimer != zero)
        {
            return (u8)((s32)(freezeRecoverTimer / 30.0f) + 1);
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

void enemy_getCurveParams(GameObject* obj, int* outIdx, f32* outA, f32* outB)
{
    int* state;
    f32 fz;
    if (obj != NULL)
    {
        state = obj->extra;
        if (state != NULL)
        {
            *outA = (f32)(u32)((EnemyState*)state)->curveParamA / 255.0f;
            *outB = (f32)(u32)((EnemyState*)state)->curveParamB;
            *outIdx = ((EnemyState*)state)->curveIndex;
            return;
        }
    }
    fz = 0.0f;
    *outA = fz;
    *outB = fz;
    *outIdx = 0;
}
void enemy_setHealthZero(GameObject* obj)
{
    EnemyState* state = obj->extra;
    state->current = 0;
}

f32 enemy_getHealthFraction(register GameObject* obj)
{
    register u16 maxHealth;
    register EnemyState* state;
    u16 curHealth;
    state = obj->extra;
    if (state == NULL)
        return 0.0f;
    maxHealth = state->max;
    if (maxHealth != 0)
    {
        curHealth = *(u16*)&state->current;
        if (curHealth != 0)
        {
            return (f32)(u32)curHealth / (f32)(u32)maxHealth;
        }
    }
    return 0.0f;
}

void enemy_trackPlayer(GameObject* obj)
{
    EnemyState* state = obj->extra;
    state->trackedObj = Obj_GetPlayerObject();
}

void enemy_setTrackedObj(GameObject* obj, GameObject* target)
{
    ((EnemyState*)obj->extra)->trackedObj = target;
}

void enemy_steerVelocityToward(GameObject* obj, void* state, f32* desiredVec, f32 maxSpeed, f32 speedBand, f32 maxTurnRad, u8 clampToGround)
{
    f32 curMag, targetMag, axisMag, speed, speedScale;
    f32 curDir[3];
    f32 targetDir[3];
    f32 turnAxis[3];
    f32 rotMtx[12];

    curMag = PSVECMag((f32*)((int)state + 0x2b8));
    if (curMag > 0.0f)
    {
        f32 inv = 1.0f / curMag;
        curDir[0] = ((f32*)state)[174] * inv;
        curDir[1] = ((f32*)state)[175] * inv;
        curDir[2] = ((f32*)state)[176] * inv;
        PSVECNormalize(curDir, curDir);
    }
    else
    {
        curDir[0] = 0.0f;
        curDir[1] = 0.0f;
        curDir[2] = 0.0f;
    }

    targetMag = PSVECMag(desiredVec);
    if (targetMag > 0.0f)
    {
        f32 inv = 1.0f / targetMag;
        targetDir[0] = desiredVec[0] * inv;
        targetDir[1] = desiredVec[1] * inv;
        targetDir[2] = desiredVec[2] * inv;
    }
    else
    {
        targetDir[0] = 0.0f;
        targetDir[1] = 0.0f;
        targetDir[2] = 0.0f;
    }

    PSVECCrossProduct(curDir, targetDir, turnAxis);
    axisMag = PSVECMag(turnAxis);
    if (axisMag > 0.0f)
    {
        f32 angle;
        int gt;
        f64 gtf;
        f32 zero;
        angle = acosf_fast(PSVECDotProduct(curDir, targetDir));
        gt = (angle > maxTurnRad);
        zero = 0.0f;
        gtf = __fabs((f32)gt);
        if (gtf != zero)
        {
            f32 rot = maxTurnRad * ((angle > 0.0f) ? 1.0f : -1.0f);
            PSMTXRotAxisRad(rotMtx, turnAxis, rot);
            PSMTXMultVecSR(rotMtx, curDir, targetDir);
        }
    }

    speedScale = 0.075f;
    speed = targetMag * speedScale;
    {
        f32 cap_high = curMag + speedBand;
        if (speed > cap_high)
        {
            speed = cap_high;
        }
        else
        {
            f32 cap_low = curMag - speedBand;
            if (speed < cap_low)
                speed = cap_low;
        }
        if (speed > maxSpeed)
            speed = maxSpeed;
    }

    obj->anim.velocityX = targetDir[0] * speed;
    obj->anim.velocityY = targetDir[1] * speed;
    obj->anim.velocityZ = targetDir[2] * speed;

    if (clampToGround != 0)
    {
        f32 y = obj->anim.velocityY;
        if (y < 0.0f)
        {
            f32 floor_height = obj->anim.localPosY;
            GameObject* target = *(GameObject**)((char*)state + 0x29c);
            f32 ground = 10.0f + target->anim.localPosY;
            if (floor_height < ground)
            {
                f32 t = (ground - floor_height) / 10.0f;
                obj->anim.velocityY = y * (1.0f - t);
            }
        }
    }
}

/* sidekickToy_accelerateTowardTarget3D: 3D physics step toward a target. Variant of sidekickToy_accelerateTowardTargetXZ that
 * uses the full 3D distance (xyz) instead of planar (xz), and also nudges
 * the y-axis velocity at obj+0x28. Returns the y-delta. */

f32 sidekickToy_accelerateTowardTarget3D(GameObject* obj, f32 tx, f32 ty, f32 tz, f32 accel, f32 speedScale, f32 maxVel,
                                         f32 drag)
{
    f32 dx = tx - obj->anim.worldPosX;
    f32 dy = ty - obj->anim.worldPosY;
    f32 dz = tz - obj->anim.worldPosZ;
    f32 dist = sqrtf(dx * dx + dy * dy + dz * dz);
    if (dist > accel)
    {
        obj->anim.velocityX = obj->anim.velocityX + timeDelta * (speedScale * (dx / dist));
        obj->anim.velocityY = obj->anim.velocityY + timeDelta * (speedScale * (dy / dist));
        obj->anim.velocityZ = obj->anim.velocityZ + timeDelta * (speedScale * (dz / dist));
    }
    else if (dist > 0.0f)
    {
        obj->anim.velocityX = obj->anim.velocityX + timeDelta * (speedScale * (dx / accel));
        obj->anim.velocityY = obj->anim.velocityY + timeDelta * (speedScale * (dy / accel));
        obj->anim.velocityZ = obj->anim.velocityZ + timeDelta * (speedScale * (dz / accel));
    }
    if (obj->anim.velocityX < -maxVel)
    {
        obj->anim.velocityX = -maxVel;
    }
    else if (obj->anim.velocityX > maxVel)
    {
        obj->anim.velocityX = maxVel;
    }
    if (obj->anim.velocityY < -maxVel)
    {
        obj->anim.velocityY = -maxVel;
    }
    else if (obj->anim.velocityY > maxVel)
    {
        obj->anim.velocityY = maxVel;
    }
    if (obj->anim.velocityZ < -maxVel)
    {
        obj->anim.velocityZ = -maxVel;
    }
    else if (obj->anim.velocityZ > maxVel)
    {
        obj->anim.velocityZ = maxVel;
    }
    if (0.0f != drag)
    {
        obj->anim.velocityX = obj->anim.velocityX * powfBitEstimate(drag, timeDelta);
        obj->anim.velocityY = obj->anim.velocityY * powfBitEstimate(drag, timeDelta);
        obj->anim.velocityZ = obj->anim.velocityZ * powfBitEstimate(drag, timeDelta);
    }
    return dy;
}

/* sidekickToy_accelerateTowardTargetXZ: xz-plane physics step toward a target. Computes the planar
 * distance to (tx,ty,tz), then nudges the obj's xz velocity (offsets 0x24,
 * 0x2c) by timeDelta * speedScale * unitDir, clamped at +/-maxVel, with an
 * optional drag pass. Returns the y-delta. */
f32 sidekickToy_accelerateTowardTargetXZ(GameObject* obj, f32 tx, f32 ty, f32 tz, f32 accel, f32 speedScale, f32 maxVel,
                                         f32 drag)
{
    f32 dx = tx - obj->anim.worldPosX;
    f32 dy = ty - obj->anim.worldPosY;
    f32 dz = tz - obj->anim.worldPosZ;
    f32 dist = sqrtf(dx * dx + dz * dz);
    if (dist > accel)
    {
        obj->anim.velocityX = obj->anim.velocityX + timeDelta * (speedScale * (dx / dist));
        obj->anim.velocityZ = obj->anim.velocityZ + timeDelta * (speedScale * (dz / dist));
    }
    else if (dist > 0.0f)
    {
        obj->anim.velocityX = obj->anim.velocityX + timeDelta * (speedScale * (dx / accel));
        obj->anim.velocityZ = obj->anim.velocityZ + timeDelta * (speedScale * (dz / accel));
    }
    if (obj->anim.velocityX < -maxVel)
    {
        obj->anim.velocityX = -maxVel;
    }
    else if (obj->anim.velocityX > maxVel)
    {
        obj->anim.velocityX = maxVel;
    }
    if (obj->anim.velocityZ < -maxVel)
    {
        obj->anim.velocityZ = -maxVel;
    }
    else if (obj->anim.velocityZ > maxVel)
    {
        obj->anim.velocityZ = maxVel;
    }
    if (0.0f != drag)
    {
        obj->anim.velocityX = obj->anim.velocityX * powfBitEstimate(drag, timeDelta);
        obj->anim.velocityZ = obj->anim.velocityZ * powfBitEstimate(drag, timeDelta);
    }
    return dy;
}

void baddieTurnTowardLookDir(GameObject* node, void* sub, int divisor, f32 fa, f32 fb, u8 useScaledRoll)
{
    f32 dt;
    int angle;
    s32 delta;
    f32 delta_f;
    s16 newVal;
    f32 zero;

    dt = timeDelta / (f32)(u32)(u16)divisor;
    if (dt > 1.0f)
        dt = 1.0f;

    angle = (u16)getAngle(-((TrickyState*)sub)->lookDirX, -((TrickyState*)sub)->lookDirZ);
    delta = angle - (u16)node->anim.rotX;
    delta_f = delta;
    if (delta_f > 32768.0f)
        delta_f = -65535.0f + delta_f;
    if (delta_f < -32768.0f)
        delta_f = 65535.0f + delta_f;
    delta_f *= dt;
    newVal = (s16)(*(s16*)(int)node + (s32)delta_f);
    node->anim.rotX = newVal;

    zero = 0.0f;
    if (fa != zero)
    {
        if (useScaledRoll != 0)
        {
            node->anim.rotZ = (s16)(node->anim.rotZ + (s32)(fa * (delta_f * dt)));
        }
        else
        {
            node->anim.rotZ = (s16)(oneOverTimeDelta * (delta_f * fa));
            {
                s16 v = node->anim.rotZ;
                if (v > 0x2000)
                    node->anim.rotZ = 0x2000;
                else if (v < -0x2000)
                    node->anim.rotZ = -0x2000;
            }
        }
    }

    if (0.0f != fb)
    {
        f32 dz2 = ((TrickyState*)sub)->lookDirZ * ((TrickyState*)sub)->lookDirZ;
        f32 dx2 = ((TrickyState*)sub)->lookDirX * ((TrickyState*)sub)->lookDirX;
        f32 hyp = sqrtf(dz2 + dx2);
        int angle2 = (u16)getAngle(((TrickyState*)sub)->lookDirY * fb, hyp);
        s32 d2 = angle2 - (u16)node->anim.rotY;
        f32 d2f = d2;
        s16 newVal2;
        if (d2f > 32768.0f)
            d2f = -65535.0f + d2f;
        if (d2f < -32768.0f)
            d2f = 65535.0f + d2f;
        newVal2 = (s16)(*(s16*)((int)node + 2) + (s32)(d2f * dt));
        node->anim.rotY = newVal2;
    }
}

void baddieTurnTowardPoint(GameObject* node, int state, f32 targetX, f32 targetZ, int divisor, int angleBias)
{
    s32 delta;
    f32 dt;
    s16 newVal;
    f32 t0 = node->anim.localPosX - targetX;
    f32 t1 = node->anim.localPosZ - targetZ;
    delta = getAngle(t0, t1);
    delta = (s16)(delta - (u16)node->anim.rotX);
    if (delta > 0x8000)
        delta = (s16)(delta - 0xFFFF);
    if ((s16)delta < -0x8000)
        delta = (s16)(delta + 0xFFFF);
    delta += angleBias;
    dt = timeDelta / (f32)(u32)(u16)divisor;
    if (dt > 1.0f)
        dt = 1.0f;
    newVal = (s16)(*(s16*)node + (s32)((f32)(s16)delta * dt));
    node->anim.rotX = newVal;
}

void baddieSetMove(GameObject* obj, int state, u8 moveId, f32 rateScale, int moveControlFlags, u8 stateByte)
{
    ObjHitsPriorityState* hitState;

    ((BaddieState*)state)->unk308 = 1.0f / (60.0f * rateScale);
    *(u8*)(state + 0x323) = stateByte;
    ObjAnim_SetCurrentMove((int)obj, moveId, 0.0f, moveControlFlags);
    hitState = (ObjHitsPriorityState*)(obj)->anim.hitReactState;
    if (hitState != NULL)
    {
        hitState->suppressOutgoingHits = 0;
    }
}

void baddieAfterUpdateBonesCb(GameObject* obj, int* bones)
{
    BaddieAfterUpdateBonesCbState* state = obj->extra;
    int v = *bones;
    switch (obj->anim.seqId)
    {
    case ENEMY_HAGABONMK2_OBJ:
        ObjModelChain_Update(bones, v, (ObjModelChain*)state->tailBoneChain, crawler_rotateVectorYaw);
        break;
    default:
        ObjModelChain_Update(bones, v, (ObjModelChain*)state->tailBoneChain, NULL);
        break;
    }
}

int enemy_getExtraSize(void)
{
    return 0x370;
}
int enemy_getObjectTypeId(void)
{
    return 0x14b;
}

void enemy_free(GameObject* obj, int flag)
{
    u8* child;
    int i;
    int n;
    u8* state;

    state = (obj)->extra;

    if (*(void**)&((EnemyState*)state)->tailSimHandle != NULL)
    {
        ObjModelChain_Free((ObjModelChain*)((EnemyState*)state)->tailSimHandle);
    }
    if (((EnemyState*)state)->modelLight != NULL)
    {
        ModelLightStruct_free(((EnemyState*)state)->modelLight);
        ((EnemyState*)state)->modelLight = NULL;
    }
    if (*(void**)state != NULL)
    {
        mm_free((void*)*(int*)state);
        *(int*)state = 0;
    }
    switch ((obj)->anim.seqId)
    {
    case ENEMY_HAGABONMK2_OBJ:
        hagabonMK2_stopLoopSfx((int)obj, state);
        break;
    case ENEMY_WHIRLPOOL_OBJ:
        if ((int)ObjGroup_ContainsObject((u32)obj, ENEMY_OBJGROUP_SECONDARY) != 0)
        {
            ObjGroup_RemoveObject((int)obj, ENEMY_OBJGROUP_SECONDARY);
        }
        break;
    }
    n = (obj)->childCount;
    for (i = 0; i < n; i++)
    {
        child = (obj)->childObjs[0];
        if (child != NULL)
        {
            ObjLink_DetachChild(obj, (GameObject*)child);
            if (flag == 0 || (((GameObject*)child)->objectFlags & 0x10) == 0)
            {
                Obj_FreeObject((GameObject*)child);
            }
        }
    }
    (*gExpgfxInterface)->freeSource((int)obj);
    ObjGroup_RemoveObject((int)obj, ENEMY_OBJGROUP);
}

void enemy_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible)
{
    EnemyState* state = obj->extra;
    if (visible != 0)
    {
        switch (obj->userData1)
        {
        case 0:
            objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, 1.0f);
            {
                u32 flags = *(u32*)&state->flags2E8;
                if ((flags & 3) != 0)
                {
                    if ((flags & 1) != 0)
                    {
                        *(u32*)&state->flags2E8 = flags & ~1LL;
                        *(u32*)&state->flags2E8 = *(u32*)&state->flags2E8 | 2;
                    }
                    if (state->modelLight == NULL)
                    {
                        state->modelLight = objCreateLight(0, 1);
                    }
                    objParticleFn_80099d84(obj, 1.0f, 3, state->particleScale,
                                           state->modelLight);
                }
            }
            if ((*(u32*)&state->flags2E8 & 4) != 0)
            {
                if (state->modelLight == NULL)
                {
                    state->modelLight = objCreateLight(0, 1);
                }
                objParticleFn_80099d84(obj, 1.0f, 4, state->particleScale,
                                       state->modelLight);
            }
            if ((*(u32*)&state->flags2E8 & 0x40) != 0)
            {
                Sfx_KeepAliveLoopedObjectSound((int)obj, SFXTRIG_forcecryslp11);
                objParticleFn_80099d84(obj, 1.0f, 5, state->particleScale, 0);
            }
            if ((*(u32*)&state->flags2E8 & 0x80) != 0)
            {
                Sfx_KeepAliveLoopedObjectSound((int)obj, SFXTRIG_forcecryslp11);
                objParticleFn_80099d84(obj, 1.5f, 6, state->particleScale, 0);
            }
            if ((*(u32*)&state->flags2E8 & 0x100) != 0)
            {
                objParticleFn_80099d84(obj, 0.75f, 7, state->particleScale, 0);
            }
            break;
        }
    }
}


void enemy_hitDetect(GameObject* obj)
{
    u8* state = obj->extra;
    ObjHitsPriorityState* childHitState;

    if (((EnemyState*)state)->modelLight != NULL &&
        modelLightStruct_getActiveState(((EnemyState*)state)->modelLight) == 0)
    {
        ModelLightStruct_free(((EnemyState*)state)->modelLight);
        ((EnemyState*)state)->modelLight = NULL;
    }
    ((EnemyState*)state)->lastHitObject = ((ObjHitsPriorityState*)obj->anim.hitReactState)->lastHitObject;
    if (((ObjHitsPriorityState*)obj->anim.hitReactState)->lastHitObject != 0)
    {
        ((ObjHitsPriorityState*)obj->anim.hitReactState)->suppressOutgoingHits = 1;
    }
    if (obj->childObjs[0] != NULL && *(void**)(*(int*)&obj->childObjs[0] + 0x54) != NULL &&
        (childHitState = *(ObjHitsPriorityState**)(*(int*)&obj->childObjs[0] + 0x54))->lastHitObject != 0)
    {
        ((ObjHitsPriorityState*)obj->anim.hitReactState)->suppressOutgoingHits = 1;
    }
    if (*(void**)&((EnemyState*)state)->tailSimHandle != NULL)
    {
        ObjModelChain_AdvancePhase((ObjModelChain*)((EnemyState*)state)->tailSimHandle);
    }
}

void enemy_update(GameObject* obj)
{
    u8* player;
    u8* state;
    u8* setup;
    u8* tricky;
    u32 flags;
    u8* s2;
    f32 fz;

    state = obj->extra;
    setup = *(u8**)&obj->anim.placementData;
    tricky = (u8*)getTrickyObject();
    if (getCurUiDll() == 4)
    {
        return;
    }
    if ((((EnemyState*)state)->flags2E4 & 0x8000006) != 0)
    {
        if (objPosToMapBlockIdx(obj->anim.localPosX, obj->anim.localPosY,
                                obj->anim.localPosZ) == -1)
        {
            return;
        }
    }
    else
    {
        if (isInBounds(obj->anim.localPosX, obj->anim.localPosZ) == 0)
        {
            return;
        }
    }
    if (objIsFrozen((u8*)obj) != 0)
    {
        baddie_updateWhileFrozen(obj, state, 1);
        return;
    }
    if (((EnemyState*)state)->trackedObj == NULL)
    {
        ((EnemyState*)state)->trackedObj = Obj_GetPlayerObject();
    }
    else if ((((GameObject*)((EnemyState*)state)->trackedObj)->objectFlags & ENEMY_OBJFLAG_FREED) != 0)
    {
        ((EnemyState*)state)->trackedObj = Obj_GetPlayerObject();
    }
    ((EnemyState*)state)->initialFlags = *(int*)&((EnemyState*)state)->controlFlags;
    baddieInstantiateWeapon(obj, (int)state);
    flags = ((EnemyState*)state)->controlFlags;
    if ((flags & 1) != 0 && (flags & 2) == 0)
    {
        if (((EnemyPlacement*)setup)->triggerSequenceId == -1)
        {
            return;
        }
        if (setup != NULL && (((EnemyPlacement*)setup)->flags & 8) != 0)
        {
            obj->anim.localPosX = ((ObjPlacement*)setup)->posX;
            obj->anim.localPosY = ((ObjPlacement*)setup)->posY;
            obj->anim.localPosZ = ((ObjPlacement*)setup)->posZ;
        }
        (*gObjectTriggerInterface)->runSequence(((EnemyPlacement*)setup)->triggerSequenceId, obj, -1);
        ((EnemyState*)state)->controlFlags |= 2;
        *(u32*)&((EnemyState*)state)->controlFlags = *(u32*)&((EnemyState*)state)->controlFlags & ~1LL;
        return;
    }
    if (obj->userData1 != 0)
    {
        if (((EnemyPlacement*)setup)->gameBit2 != -1)
        {
            if (mainGetBit(((EnemyPlacement*)setup)->gameBit2) == 0)
            {
                return;
            }
            if ((((EnemyState*)state)->controlFlags & 0x800) != 0)
            {
                return;
            }
            if ((((EnemyState*)state)->controlFlags & 0x1000) == 0)
            {
                return;
            }
            player = (u8*)Obj_GetPlayerObject();
            if (((EnemyPlacement*)setup)->gameBit != -1)
            {
                if (mainGetBit(((EnemyPlacement*)setup)->gameBit) != 0)
                {
                    return;
                }
            }
            if (player != NULL)
            {
                if (vec3f_distanceSquared((f32*)(player + 0x18), &((EnemyPlacement*)setup)->base.posX) >
                    1600.0f)
                {
                    enemy_init(obj, setup, 0);
                    ((EnemyState*)state)->controlFlags |= 0x1000;
                    *(u32*)&((EnemyState*)state)->initialFlags &= ~0x1000LL;
                }
                else
                {
                    return;
                }
            }
            else
            {
                return;
            }
        }
        else if (((EnemyPlacement*)setup)->gameBit != -1)
        {
            if (mainGetBit(((EnemyPlacement*)setup)->gameBit) != 0)
            {
                return;
            }
            if ((((EnemyState*)state)->controlFlags & 0x800) != 0)
            {
                return;
            }
            player = (u8*)Obj_GetPlayerObject();
            if (player != NULL)
            {
                if (vec3f_distanceSquared((f32*)(player + 0x18), &((EnemyPlacement*)setup)->base.posX) >
                    1600.0f)
                {
                    enemy_init(obj, setup, 0);
                    ((EnemyState*)state)->controlFlags |= 0x1000;
                    *(u32*)&((EnemyState*)state)->initialFlags &= ~0x1000LL;
                }
                else
                {
                    return;
                }
            }
            else
            {
                return;
            }
        }
        else
        {
            if (*(u32*)&((ObjPlacement*)setup)->mapId == 0xFFFFFFFF)
            {
                return;
            }
            if (((EnemyPlacement*)setup)->respawnEnabled == 0)
            {
                return;
            }
            if ((*gMapEventInterface)->shouldNotSaveTime(((ObjPlacement*)setup)->mapId) != 0)
            {
                if ((((EnemyState*)state)->controlFlags & 0x800) == 0)
                {
                    player = (u8*)Obj_GetPlayerObject();
                    if (player != NULL)
                    {
                        if (vec3f_distanceSquared((f32*)(player + 0x18), &((EnemyPlacement*)setup)->base.posX) >
                            1600.0f)
                        {
                            enemy_init(obj, setup, 0);
                            ((EnemyState*)state)->controlFlags |= 0x1000;
                            *(u32*)&((EnemyState*)state)->initialFlags &= ~0x1000LL;
                        }
                        else
                        {
                            return;
                        }
                    }
                    else
                    {
                        return;
                    }
                }
                else
                {
                    return;
                }
            }
            else
            {
                return;
            }
        }
    }
    if ((((EnemyState*)state)->controlFlags & 0x8000) != 0)
    {
        hudFn_8011f38c(0);
        (*gPathControlInterface)->attachObject(obj, state + 4);
        ((EnemyState*)state)->controlFlags &= ~0x8003;
        if ((((EnemyState*)state)->flags2E4 & 0x20000) != 0)
        {
            s2 = *(u8**)&obj->anim.placementData;
            obj->anim.localPosX = ((EnemyPlacement*)s2)->base.posX;
            obj->anim.localPosY = ((EnemyPlacement*)s2)->base.posY;
            obj->anim.localPosZ = ((EnemyPlacement*)s2)->base.posZ;
            obj->anim.rotZ = 0;
            obj->anim.rotY = 0;
            obj->anim.rotX = ((EnemyPlacement*)s2)->initialYaw << 8;
            fz = 0.0f;
            obj->anim.velocityX = fz;
            obj->anim.velocityY = fz;
            obj->anim.velocityZ = fz;
        }
    }
    if ((((EnemyState*)state)->flags2E4 & 0x80000) != 0)
    {
        if (tricky != NULL && mainGetBit(0x9e) != 0)
        {
            *(u8*)&obj->anim.resetHitboxMode &= ~INTERACT_FLAG_PROMPT_SUPPRESSED;
        }
        else
        {
            *(u8*)&obj->anim.resetHitboxMode |= INTERACT_FLAG_PROMPT_SUPPRESSED;
        }
        if (tricky != NULL && (*(u8*)&obj->anim.resetHitboxMode & INTERACT_FLAG_IN_RANGE) != 0)
        {
            (**(void (**)(u8*, int, int, int))(*(int*)(*(int*)(tricky + 0x68)) + 0x28))(tricky, (int)obj, 1, 2);
        }
    }
    baddie_updateWhileFrozen(obj, state, 0);
    if ((((EnemyState*)state)->controlFlags & 0x1800) == 0)
    {
        baddieTurnTowardTarget(obj, (TrickyState*)state);
        baddie_updateEngagementState(obj, (TrickyState*)state);
    }
    enemyObjAnimUpdate((short*)obj, (int)state);
}

void enemy_init(GameObject* obj, u8* setup, int flag)
{
    u8* state = (obj)->extra;
    f32 fz;

    (obj)->userData1 = 0;
    if (flag == 0)
    {
        if (((EnemyPlacement*)setup)->gameBit2 != -1)
        {
            if (((EnemyPlacement*)setup)->gameBit != -1)
            {
                if (mainGetBit(((EnemyPlacement*)setup)->gameBit) == 0)
                {
                    (obj)->userData1 = mainGetBit(((EnemyPlacement*)setup)->gameBit2) == 0;
                }
            }
            else
            {
                (obj)->userData1 = mainGetBit(((EnemyPlacement*)setup)->gameBit2) == 0;
            }
        }
        if (*(u32*)&((ObjPlacement*)setup)->mapId != 0xFFFFFFFF)
        {
            if ((obj)->userData1 == 0)
            {
                if (((EnemyPlacement*)setup)->gameBit != -1)
                {
                    (obj)->userData1 = mainGetBit(((EnemyPlacement*)setup)->gameBit);
                }
                if ((obj)->userData1 == 0)
                {
                    if (((EnemyPlacement*)setup)->respawnEnabled != 0)
                    {
                        if ((*gMapEventInterface)->shouldNotSaveTime(((ObjPlacement*)setup)->mapId) == 0)
                        {
                            (obj)->userData1 = 1;
                        }
                    }
                }
            }
        }
    }
    if ((obj)->userData1 != 0)
    {
        (obj)->anim.flags |= OBJANIM_FLAG_HIDDEN;
        (obj)->anim.alpha = 0;
    }
    else
    {
        (obj)->anim.flags &= ~OBJANIM_FLAG_HIDDEN;
        (obj)->anim.alpha = 255;
    }
    ((EnemyState*)state)->health = ((EnemyPlacement*)setup)->healthByte / 255.0f;
    ((EnemyState*)state)->aggroRange = (f32)(u32)(((EnemyPlacement*)setup)->aggroRangeByte << 3);
    *(int*)&((EnemyState*)state)->controlFlags = 0;
    ((EnemyState*)state)->initialFlags = *(int*)&((EnemyState*)state)->controlFlags;
    (obj)->anim.rotX = ((EnemyPlacement*)setup)->initialYaw << 8;
    (obj)->anim.localPosX = ((ObjPlacement*)setup)->posX;
    (obj)->anim.localPosY = ((ObjPlacement*)setup)->posY;
    (obj)->anim.localPosZ = ((ObjPlacement*)setup)->posZ;
    *(u8*)&(obj)->anim.resetHitboxMode &= ~INTERACT_FLAG_DISABLED;
    if (flag == 0)
    {
        *(int*)&((EnemyState*)state)->flags2E4 = 0;
        ((EnemyState*)state)->flags2E8 = 0;
        state[0x2f1] = 0;
        state[0x2f2] = 0;
        ((EnemyState*)state)->unk2EC = 0;
        state[0x2f5] = 0;
        fz = 0.0f;
        ((EnemyState*)state)->animDeltaScale = fz;
        ((EnemyState*)state)->unk304 = fz;
        ((EnemyState*)state)->unk308 = fz;
        ((EnemyState*)state)->particleScale = fz;
        state[0x323] = 0;
        ((EnemyState*)state)->unk310 = fz;
        ((EnemyState*)state)->unk2F8 = 0;
        state[0x33a] = 0;
        state[0x33b] = 0;
        ((EnemyState*)state)->phaseAngle = 0;
        state[0x33c] = 0;
        state[0x33d] = 0;
        ((EnemyState*)state)->unk324 = fz;
        ((EnemyState*)state)->unk328 = fz;
        ((EnemyState*)state)->unk32C = fz;
        ((EnemyState*)state)->unk330 = fz;
        ((EnemyState*)state)->intervalTimer = fz;
        ((EnemyState*)state)->unk2B4 = -1;
        ((EnemyState*)state)->unk2B6 = ((EnemyState*)state)->unk2B4;
        (obj)->objectFlags |= ((EnemyPlacement*)setup)->objectFlagBits & 7;
        ((EnemyState*)state)->current = ((EnemyPlacement*)setup)->hitPoints;
        (obj)->animEventCallback = enemy_SeqFn;
        switch ((obj)->anim.seqId)
        {
        case ENEMY_SHARPCLAW_GR_OBJ:
        case ENEMY_SHARPCLAW_SN_OBJ:
        case ENEMY_SHARPCLAW_CO_OBJ:
        case ENEMY_SHARPCLAW_AS_OBJ:
        case ENEMY_SHARPCLAW_SH_OBJ:
        case ENEMY_SHARPCLAW_SO_OBJ:
        case ENEMY_BOSSGENERAL_OBJ:
            sharpClawInit((int)obj, state);
            break;
        case ENEMY_GUARDCLAW_OBJ:
        case 641:
            guardClaw_init((GameObject*)obj, state);
            break;
        case ENEMY_GCROBOTPATROL_OBJ:
            gcRobotPatrol_init(obj, (int)state);
            break;
        case ENEMY_MIKALADON_OBJ:
            mikaladon_init(obj, (MikaladonState*)state);
            break;
        case ENEMY_VAMBAT_OBJ:
        case ENEMY_FIREBAT_OBJ:
            vambat_init(obj, (int)state);
            break;
        case ENEMY_KOOSHY_OBJ:
            kooshy_init((int)obj, (int)state);
            break;
        case ENEMY_WEEVIL_OBJ:
            weevil_init((int)obj, state);
            break;
        case ENEMY_PINPON_OBJ:
            pinPon_init(obj, state);
            break;
        case ENEMY_RACHNOP_OBJ:
            rachnopInit((int)obj, (int)state);
            break;
        case ENEMY_SPITTINGEBA_OBJ:
            spittingEbaInit((int)obj, (int)state);
            break;
        case ENEMY_WB_OBJ:
            wbInit((int)obj, (int)state);
            break;
        case ENEMY_MUTATEDEBA_OBJ:
            mutatedEbaInit((u32)obj, (int)state);
            break;
        case ENEMY_WHIRLPOOL_OBJ:
            baddie_initWhirlpoolState((int*)obj, (GroundBaddieState*)state);
            break;
        case ENEMY_SNOWWORM_OBJ:
        case ENEMY_SNOWWORM_BABY_OBJ:
            snowworm_init(obj, (int*)state);
            break;
        case ENEMY_HOODEDZYCK_OBJ:
            hoodedZyck_init(obj, (struct BaddieState*)state);
            break;
        case ENEMY_BATTLEDROID_OBJ:
            battleDroidInit((int)obj, (char*)state);
            break;
        case ENEMY_FIRECRAWLER_OBJ:
        case ENEMY_REDEYE_OBJ:
        case ENEMY_SHADOWHUNTER_OBJ:
        case ENEMY_SWAMPSTRIDER_OBJ:
            crawler_initModelVariant(obj, state);
            break;
        case ENEMY_HAGABONMK2_OBJ:
            hagabonMK2_init(obj, (struct BaddieState*)state);
            break;
        default:
            battleDroidInit((int)obj, (char*)state);
            break;
        }
        ((EnemyState*)state)->max = *(u16*)&((EnemyState*)state)->current;
        if (((EnemyPlacement*)setup)->unk34 != 0)
        {
            *(int*)&((EnemyState*)state)->flags2E4 = *(int*)&((EnemyState*)state)->flags2E4 & -39;
        }
        ObjGroup_AddObject((int)obj, ENEMY_OBJGROUP);
        state[0x2f0] = 7;
        state[0x2ef] = 2;
        if (*(void**)state == NULL)
        {
            *(int*)state = (int)mmAlloc(264, 26, 0);
        }
        if (*(void**)state != NULL)
        {
            memset(*(void**)state, 0, 264);
        }
        if ((*gRomCurveInterface)
                ->initCurve(*(void**)state, (void*)obj, ((EnemyState*)state)->sightRange, (int*)&lbl_803DBC58, -1) == 0)
        {
            ((EnemyState*)state)->controlFlags |= BADDIE_CONTROL_PATH_FOLLOW;
        }
        (*gPathControlInterface)->init(state + 4, 0, 422, 1);
        if ((((EnemyState*)state)->flags2E4 & 8) != 0)
        {
            (*gPathControlInterface)->setLocalPointCollision(state + 4, 1, lbl_8031DBE4, &lbl_803DBC64, 4);
        }
        if ((((EnemyState*)state)->flags2E4 & 4) != 0)
        {
            (*gPathControlInterface)->setup(state + 4, 1, lbl_8031DBD8, &lbl_803DBC60, &lbl_803DBC68);
        }
        (*gPathControlInterface)->attachObject(obj, state + 4);
        if ((((EnemyState*)state)->flags2E4 & 0xc) != 0)
        {
            state[0x25f] = 1;
        }
        if ((((EnemyState*)state)->flags2E4 & 0x8000022) != 0 || ((EnemyPlacement*)setup)->unk34 != 0 ||
            (obj)->anim.seqId == ENEMY_VAMBAT_OBJ || (obj)->anim.seqId == ENEMY_FIREBAT_OBJ)
        {
            ((EnemyState*)state)->flags |= 0x40000;
        }
        else
        {
            ((EnemyState*)state)->flags &= ~0x40000;
        }
        if ((((EnemyState*)state)->flags2E4 & 4) == 0 && (((EnemyState*)state)->flags2E4 & 8) != 0)
        {
            ((EnemyState*)state)->flags &= ~0x3800;
        }
        if ((obj)->userData1 != 0)
        {
            ((EnemyState*)state)->controlFlags |= 0x1000;
            *(u32*)&((EnemyState*)state)->initialFlags = *(u32*)&((EnemyState*)state)->initialFlags & ~0x1000LL;
            ObjHits_DisableObject(obj);
        }
        else if ((((EnemyState*)state)->flags2E4 & 1) != 0)
        {
            ObjHits_EnableObject(obj);
        }
    }
    ((EnemyState*)state)->freezeRecoverTimer = 0.0f;
    if (((EnemyState*)state)->aggroRange > 1905.0f)
    {
        ((EnemyState*)state)->aggroRange = 1905.0f;
    }
    if (((EnemyState*)state)->sightRange > 1905.0f)
    {
        ((EnemyState*)state)->sightRange = 1905.0f;
    }
}

void enemy_release(void)
{
    if (gBaddieStaffCollisionInterface != NULL)
    {
        Resource_Release(gBaddieStaffCollisionInterface);
        gBaddieStaffCollisionInterface = NULL;
    }
}

void enemy_initialise(void)
{
    if (gBaddieStaffCollisionInterface == NULL)
        gBaddieStaffCollisionInterface = Resource_Acquire(0x5a, 1);
}

const f32 lbl_803E2604 = 0.0f;
