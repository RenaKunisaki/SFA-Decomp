#ifndef MAIN_DLL_DLL_00C9_ENEMY_H_
#define MAIN_DLL_DLL_00C9_ENEMY_H_

#include "game/objects/object.h"
#include "ghidra_import.h"
#include "global.h"
#include "main/objanim_update.h"

struct ModelLightStruct;
struct ObjModelChain;

/*
 * obj+0xB8 extra record for the generic enemy family (DLL 0xC9). The block is
 * enemy_getExtraSize() = 0x370 bytes; it is neither a BaddieState (0x35C) nor a
 * TrickyState (0x83C), and the field set below is the one Baddie.c itself
 * reads and writes.
 */
typedef struct EnemyState {
    u8 unk0[0x4 - 0x0];
    u32 flags; /* head word of the embedded gPathControlInterface record at +4 */
    u8 unk8[0x1B8 - 0x8];
    f32 nearestSpecialDeltaY; /* signed dy to the nearest type-0xe special-surface floor hit */
    u8 unk1BC[0x25F - 0x1BC];
    s8 physicsActive; /* floor-response pass enables the per-frame ground snap / footstep audio */
    u8 unk260;
    u8 unk261; /* bbox trace filter handed to objBboxFn_800640cc */
    u8 unk262[0x264 - 0x262];
    s8 surfaceFlags; /* ENEMY_SURFACE_FLAG_* */
    u8 unk265[0x29C - 0x265];
    GameObject* trackedObj; /* current engagement target */
    u16 turnOctant; /* (u16 turnAngleDelta >> 13): which 1/8 sector the turn falls in */
    u16 turnAngleDelta; /* signed angle to trackedObj minus world rotX, normalized to +/-0x8000 */
    u16 targetDist; /* (s16) distance to trackedObj */
    u16 targetHeightDelta; /* (s16)(trackedObj.worldPosY - self.worldPosY) */
    f32 aggroRange; /* engagement range derived from placement data */
    f32 sightRange; /* patrol/detection range used by curve setup */
    u16 current;    /* numerator used by enemy_getHealthFraction */
    u16 max;        /* spawn-time denominator */
    s16 spawnedWeaponRomDefNo; /* romDefNo of the weapon child currently attached (-1 none) */
    s16 weaponRomDefNo; /* romDefNo of the weapon child that should be attached (-1 none) */
    f32 lookDirX; /* look/aim direction: yaw = getAngle(-X,-Z), pitch = getAngle(Y, hyp(X,Z)) */
    f32 lookDirY;
    f32 lookDirZ;
    u8 unk2C4[0x2D0 - 0x2C4];
    f32 freezeEffectTimer; /* counts down by timeDelta; on reaching 0 the ice shatter fx re-fires and it re-primes to 20 */
    f32 repeatHitCooldown; /* counts down by timeDelta; while >= 0 a repeat hit of kind 0x1a is ignored */
    f32 freezeRecoverTimer;
    u32 controlFlags;
    u32 prevControlFlags; /* controlFlags snapshot taken at the top of enemy_update; (cur & bit) && !(prev & bit) = bit raised this frame */
    u32 flags2E4;
    u32 flags2E8;
    u16 impactSfxId;
    u8 unk2EE[0x2EF - 0x2EE];
    u8 actionId; /* current action selector (0..5) */
    u8 prevActionId; /* previous frame's actionId */
    u8 flags2F1; /* decoded player-attack flags (baddie_decodePlayerAttackFlags) */
    u8 curveIndex;
    u8 curveParamA;
    u8 curveParamB;
    u8 spawnBits; /* reward-drop selector decoded from the player attack flags */
    u8 frozenFadeCounter : 5; /* countdown gating the frozen-shatter fade-in sfx */
    u8 unk2F6 : 3;
    u8 unk2F7[0x2F8 - 0x2F7];
    u16 animEventMask; /* per-frame bitmask OR'd from (1 << anim event index); fed to objAudioFn */
    u8 unk2FA[0x2FC - 0x2FA];
    f32 health;
    f32 gravity; /* fall acceleration: velocityY -= gravity*dt, posY -= 0.5*gravity*dt^2 */
    f32 drag; /* per-second velocity damping base: velocity *= powfBitEstimate(drag, dt) */
    f32 animPlaySpeed; /* play speed handed to ObjAnim_AdvanceCurrentMove */
    f32 particleScale;
    f32 unk310;
    f32 moveSpeedScale0; /* animPlaySpeed = 1 / (60 * scale) for moveId0 */
    f32 moveSpeedScale1; /* paired with moveId1 */
    f32 moveSpeedScale2; /* paired with moveId2 */
    u8 moveId0; /* ObjAnim_SetCurrentMove move id used when the baddie respawns */
    u8 moveId1; /* move id used by the defeat handler */
    u8 moveId2;
    u8 rootMotionFlags; /* which axes the current move drives from root motion: 1 Z, 2 X, 4 Y, 8 yaw */
    f32 unk324;
    f32 unk328;
    f32 unk32C;
    f32 unk330;
    f32 intervalTimer;
    s16 phaseAngle;
    u8 unk33A[0x340 - 0x33A];
    int lastHitObject;
    u8 unk344[0x368 - 0x344];
    struct ModelLightStruct* modelLight;
    struct ObjModelChain* tailSimHandle;
} EnemyState;

STATIC_ASSERT(sizeof(EnemyState) == 0x370);
STATIC_ASSERT(offsetof(EnemyState, nearestSpecialDeltaY) == 0x1B8);
STATIC_ASSERT(offsetof(EnemyState, physicsActive) == 0x25F);
STATIC_ASSERT(offsetof(EnemyState, surfaceFlags) == 0x264);
STATIC_ASSERT(offsetof(EnemyState, trackedObj) == 0x29C);
STATIC_ASSERT(offsetof(EnemyState, aggroRange) == 0x2A8);
STATIC_ASSERT(offsetof(EnemyState, lookDirX) == 0x2B8);
STATIC_ASSERT(offsetof(EnemyState, freezeEffectTimer) == 0x2D0);
STATIC_ASSERT(offsetof(EnemyState, controlFlags) == 0x2DC);
STATIC_ASSERT(offsetof(EnemyState, actionId) == 0x2EF);
STATIC_ASSERT(offsetof(EnemyState, spawnBits) == 0x2F5);
STATIC_ASSERT(offsetof(EnemyState, animEventMask) == 0x2F8);
STATIC_ASSERT(offsetof(EnemyState, gravity) == 0x300);
STATIC_ASSERT(offsetof(EnemyState, moveSpeedScale0) == 0x314);
STATIC_ASSERT(offsetof(EnemyState, rootMotionFlags) == 0x323);
STATIC_ASSERT(offsetof(EnemyState, modelLight) == 0x368);

typedef struct EnemyTargetSearchResult {
    GameObject* obj;
    u16 dist;
    u8 pad6[2];
} EnemyTargetSearchResult;

STATIC_ASSERT(sizeof(EnemyTargetSearchResult) == 8);

void enemyObjAnimUpdate(short* obj, int state);
void FUN_8014ab58(u64 param_1, double param_2, double param_3, double param_4, double param_5, double param_6,
                  double param_7, u64 param_8, u16* param_9, int* param_10, u32 param_11, u32 param_12, u32 param_13,
                  u32 param_14, u32 param_15, u32 param_16);
void FUN_8014c0b4(double param_1, double param_2, u64 param_3, u64 param_4, u64 param_5, u64 param_6, u64 param_7,
                  u64 param_8, int param_9, int param_10);
void FUN_8014c690(u64 param_1, double param_2, double param_3, u64 param_4, u64 param_5, u64 param_6, u64 param_7,
                  u64 param_8, u32 param_9, u32 param_10, int param_11);
void FUN_8014c694(u64 param_1, u64 param_2, u64 param_3, u64 param_4, u64 param_5, u64 param_6, u64 param_7,
                  u64 param_8, int param_9);
int enemy_SeqFn(GameObject* node, int unused, ObjAnimUpdateState* animUpdate);
int enemy_findNearbyEnemies(GameObject* obj, f32 radius, u8 flags, int maxCount, EnemyTargetSearchResult* results);
void tricky_handleDefeat(GameObject* obj, int state);
void baddie_updateWhileFrozen(GameObject* obj, u8* state, u8 fromHit);
int baddie_spawnRewardDrops(GameObject* obj, int state, int spawnBits, u32 useAltMode, u32 mode);
void baddieInstantiateWeapon(GameObject* obj, int state);
u8 baddie_canSeeTarget(GameObject* obj, EnemyState* state, void* from, void* to);
void baddie_updateSightQuadrants(GameObject* obj, EnemyState* state, f32 radius);
void enemy_setTrackedObj(GameObject* obj, GameObject* target);
void enemy_steerVelocityToward(GameObject* obj, void* state, f32* direction, f32 maxSpeed, f32 speedRange, f32 maxAngle,
                 u8 adjustGroundVelocity);
void baddieTurnTowardLookDir(GameObject* obj, void* state, int divisor, f32 rollScale, f32 pitchScale, u8 useScaledRoll);
void enemy_setHealthZero(GameObject* obj);
void enemy_trackPlayer(GameObject* obj);
u8 enemy_getFreezeRecoverSeconds(GameObject* obj);
void enemy_getCurveParams(GameObject* obj, int* outIdx, f32* outA, f32* outB);
void baddieTurnTowardPoint(GameObject* obj, int state, f32 targetX, f32 targetZ, int divisor, int angleBias);
f32 enemy_getHealthFraction(GameObject* obj);
f32 sidekickToy_accelerateTowardTarget3D(GameObject* obj, f32 tx, f32 ty, f32 tz, f32 accel, f32 speedScale,
                                         f32 maxVel, f32 drag);
f32 sidekickToy_accelerateTowardTargetXZ(GameObject* obj, f32 tx, f32 ty, f32 tz, f32 accel, f32 speedScale,
                                         f32 maxVel, f32 drag);
void baddieAfterUpdateBonesCb(GameObject* obj, int* bones);
int enemy_getExtraSize(void);
int enemy_getObjectTypeId(void);
void enemy_release(void);
void enemy_initialise(void);
void enemy_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible);
void enemy_hitDetect(GameObject* obj);
void enemy_free(GameObject* obj, int flag);
void enemy_update(GameObject* obj);
void enemy_init(GameObject* obj, u8* setup, int flag);
void FUN_8014ccb8(double param_1, double param_2, double param_3, int param_4, int param_5, float* param_6,
                  char param_7);
double FUN_8014cfac(double param_1, double param_2, double param_3, double param_4, double param_5, double param_6,
                    double param_7, int param_8);
double FUN_8014d2a4(double param_1, double param_2, double param_3, double param_4, double param_5, double param_6,
                    double param_7, int param_8);

#endif /* MAIN_DLL_DLL_00C9_ENEMY_H_ */
