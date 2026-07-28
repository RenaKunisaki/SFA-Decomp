#ifndef MAIN_DLL_BADDIE_STATE_H_
#define MAIN_DLL_BADDIE_STATE_H_

#include "ghidra_import.h"
#include "global.h"
#include "main/objprint_character_api.h"
#include "main/voxmaps.h"

struct GameObject;
struct BaddieState;
typedef void (*BaddieStateExitFn)(struct GameObject* obj, struct BaddieState* state);

/*
 * BaddieState - the engine-wide actor-control record that lives at the
 * head of the obj+0xB8 extra block for objects driven through
 * gBaddieControlInterface / gPlayerInterface (the name follows the
 * gBaddieControlInterface linkage; ActorControlState was the considered
 * alternative since the player shares it - cheap to rename later).
 *
 * Shared layout evidence:
 * - scarab.c (dll_CA/CB/CE: extraSizes 0x458/0x41c/0x410 - this struct is
 *   the common 0x410 prefix; nothing past 0x40C is referenced there) and
 *   slot 202's iceBaddie implementation.
 * - player.c's "inner" is the SAME record (0x274 mode compares, 0x27A
 *   just-started flag, 0x346 latch, ...).
 * - dll_000F (engine/15) is the shared move/substate controller and is the
 *   only writer of most of the head.
 * - treasurechest.c / dlls/objects/437/437.c reference the same offsets.
 * - DR_CloudRunner's 0xBC8 extra block EMBEDS this record as its prefix
 *   (0x25F/0x28C/0x314/0x354 head + private tail from ~0x410).
 *
 * NOT evidence for this struct: the generic enemy DLL 0xC9 (slot 201 plus its
 * family handlers in slot 202). Its obj+0xB8 record is EnemyState
 * (dll_00C9_enemy.h, enemy_getExtraSize() = 0x370). The two records agree on
 * the engine-controlled head but diverge from roughly 0x25F up - 0x2B0 is
 * BaddieState scratch but EnemyState's health numerator, 0x2D0 is a target
 * pointer here and a countdown timer there, and 0x2F4-0x322 holds an unrelated
 * field set in each. Never read one through the other's names.
 *
 * Only fields with read/write evidence in the DLLs listed above are named;
 * everything else is padded.
 */
typedef struct BaddieState {
    u32 flags0; /* actor-state flags; player climbing sets bit 0x200000 */
    u32 flags4; /* secondary actor-state flags; player climbing sets bits 0x100000/0x8000000 */
    u8 unk08[0x14 - 0x8];
    f32 posX; /* copied into spawned contact objects as position */
    f32 posY;
    f32 posZ;
    u8 unk20[0x38 - 0x20];
    f32 velX; /* copied into spawned contact objects as velocity */
    f32 velY;
    f32 velZ;
    u8 unk44[0xB8 - 0x44];
    s8 surfaceSoundIndex; /* 0..0x22 index into the per-type contact-sfx tables (intersect.c objAudioFn_8006ef38) */
    u8 padB9[0xBC - 0xB9];
    u8 paletteSlot; /* indexes the palette table (paletteIndex = gIceBaddiePaletteIndexTable[slot]) */
    u8 unkBD[0xC4 - 0xBD];
    void *contactObj; /* GameObject*; its anim.romDefNo (0x5d/0x99/0x1db/0x223) switches a sfx override (intersect.c) */
    u8 unkC8[0x19C - 0xC8];
    s16 spawnRotY; /* pair copied into the spawn-setup shorts; restored into anim.rotY */
    s16 spawnRotZ; /* restored into anim.rotZ */
    u8 unk1A0[0x1B4 - 0x1A0];
    f32 waterDepth; /* compared > threshold to fire the waterfx splash path (intersect.c) */
    u8 unk1B8[0x25B - 0x1B8];
    s8 contactSfxMuted; /* nonzero suppresses contact sfx unless contactSfxFlags bit 0x10 (intersect.c) */
    u8 unk25C[0x25F - 0x25C];
    s8 physicsActive; /* enables the free-fall physics path: gravity integration (velY -= g*dt), floor bounce response; set when thrown/spat */
    s8 contactSfxFlags; /* bit 0x10 allows contact sfx while contactSfxMuted is set (intersect.c) */
    u8 unk261[0x270 - 0x261];
    s16 substate; /* CA-family substate 0..5; gates the map-event re-register when != 3 */
    s16 prevSubstate; /* latched from substate for change detection (prevSubstate = startState in objseq) */
    s16 controlMode; /* current control move/mode; gPlayerInterface[5](obj,state,N) requests N */
    s16 prevControlMode; /* latched from controlMode for change detection (parallels prevSubstate/substate): controlMode != prevControlMode arms moveJustStartedA, then prevControlMode = controlMode; consumers compare it to prior mode ids (dll_000F/icebaddie/player) */
    s16 stateId; /* active player/control state id, written when a state handler starts */
    s8 moveJustStartedA; /* one-shot, tested at SeqFn entry */
    s8 moveJustStartedB; /* one-shot, secondary channel (death/cleanup handlers) */
    u8 unk27C[0x280 - 0x27C];
    f32 animSpeedA; /* anim blend speed pair */
    f32 animSpeedB;
    u8 unk288[4];
    f32 moveInputZ;
    f32 moveInputX;
    f32 animSpeedC; /* third of the animSpeed family - stored in lockstep with animSpeedB (z = K; animSpeedC = z; animSpeedB = z), scaled with animSpeedA and obj+0x28 */
    f32 inputMagnitude;
    void *trackedObj; /* current target/player object */
    f32 moveSpeed; /* per-mode movement speed */
    f32 gravity; /* fall acceleration: velocityY -= gravity * timeDelta (dll_000F player_applyGravity) */
    /* 0x2A8/0x2AC are two independent 0..1 ramp progresses for the deferred
     * "nudge" the shared controller applies over several frames. dll_000F
     * player_render2 steps 0x2A8 by f1*f2, clamps it at 1.0 and adds
     * nudgeYaw * (that frame's increment) straight into anim.rotX;
     * player_modelMtxFn steps 0x2AC the same way and adds
     * nudgePos{X,Y,Z} * increment into the model matrix translation. */
    f32 nudgeYawProgress;
    f32 nudgePosProgress;
    u8 pad2B0[0x2B8 - 0x2B0];
    f32 velSmoothTime; /* first-order velocity smoothing divisor: vel += t * (target - vel) / velSmoothTime */
    u8 pad2BC[0x2C0 - 0x2BC];
    f32 targetDistance; /* sqrtf dist to targetObj (scarab/campfire/anim/iceBaddie); also (s32)-compared */
    u8 unk2C4[0x2D0 - 0x2C4];
    void *targetObj; /* current attack/aggro target */
    u8 pad2D4[0x2F4 - 0x2D4];
    f32 nudgePosX; /* translation added to the model matrix as nudgePosProgress ramps */
    f32 nudgePosY;
    f32 nudgePosZ;
    f32 nudgeYaw; /* anim.rotX delta added as nudgeYawProgress ramps */
    union {
        f32 unk304;
        BaddieStateExitFn stateExitFn;
    };
    union {
        f32 unk308;
        int stateHandler; /* player state callback address */
        BaddieStateExitFn nextStateExitFn;
    };
    u8 unk30C[8];
/* eventFlags bit: anim-event footstep - the anim/event stream latches it, and
 * the per-family update readers test-then-clear it to fire the footstep/climb
 * contact SFX. */
#define BADDIE_EVENT_FOOTSTEP 0x1
/* eventFlags bit: anim-event landing/impact - latched on a landing anim event,
 * test-then-cleared by the readers to fire the land sound / rumble / waterfx
 * splash. */
#define BADDIE_EVENT_LANDING 0x200
    s32 eventFlags; /* bits 1/0x200 observed */
    f32 unk318;
    f32 unk31C;
    u8 unk320[0x32E - 0x320];
    s16 stateTimer; /* count-up dt-accumulating timer, gated > 0x78, reset to 0 on state entry */
    s16 cameraYaw;
    u8 unk332[4];
    s16 turnRate; /* s16 angle units/sec: *yaw += k * (turnRate * timeDelta / speed) */
    s16 controlTimer; /* primary control-state timer; reset on mode entry and accumulated each update */
    u8 unk33A[0x346 - 0x33A];
    s8 moveDone; /* set when the current move completes; SeqFns chain the next mode off it */
    u8 unk347[2];
    u8 hasTarget; /* cleared with death/reset */
    u8 unk34A[2];
    s8 movementFlags; /* root-motion / velocity handling flags for the shared player controller */
    s8 stateTag; /* per-tick state/mode index (written each tick; compared ==1/==3 across the baddie cluster + player) */
    u8 unk34E;
    s8 lastHitPriority;
    u8 unk350[4];
    s8 hitPoints; /* remaining hit points; decremented on hit, < 1 = dead */
    u8 unk355;
    u8 moveEventFlags; /* one-shot move-progress event latches (bit1/bit2: SFX fired once past a progress threshold) */
    u8 unk357[0x35C - 0x357];
} BaddieState;

STATIC_ASSERT(sizeof(BaddieState) == 0x35C);
STATIC_ASSERT(offsetof(BaddieState, controlMode) == 0x274);
STATIC_ASSERT(offsetof(BaddieState, moveJustStartedB) == 0x27B);
STATIC_ASSERT(offsetof(BaddieState, trackedObj) == 0x29C);
STATIC_ASSERT(offsetof(BaddieState, moveSpeed) == 0x2A0);
STATIC_ASSERT(offsetof(BaddieState, targetObj) == 0x2D0);
STATIC_ASSERT(offsetof(BaddieState, nudgePosX) == 0x2F4);
STATIC_ASSERT(offsetof(BaddieState, nudgeYaw) == 0x300);
STATIC_ASSERT(offsetof(BaddieState, eventFlags) == 0x314);
STATIC_ASSERT(offsetof(BaddieState, stateTimer) == 0x32E);
STATIC_ASSERT(offsetof(BaddieState, controlTimer) == 0x338);
STATIC_ASSERT(offsetof(BaddieState, moveDone) == 0x346);
STATIC_ASSERT(offsetof(BaddieState, hitPoints) == 0x354);

/*
 * GroundBaddieState - BaddieState plus the route/config tail shared by the
 * ground-bug baddie cluster (scarab dll_CA/CB/CE, iceBaddie; treasurechest
 * and lightfoot reference the same tail offsets). The 0x35C+ region is
 * PER-FAMILY in general: the dll_2E look-controller block sits at 0x35C for
 * DRpushcart/DIMSnowHorn1, 0x3EC for hightop, 0x4C4 for DR_CloudRunner -
 * which is why it is not part of BaddieState itself.
 */
typedef struct GroundBaddieState {
    BaddieState baddie;
    RouteNav routeNav; /* 0x35c: route destination/current/target and update budget */
    RouteState routeState; /* 0x384: allocated route nodes, heap, and recovered path */
    CharacterEyeAnimState eyeAnimState; /* 0x3ac: head-aim / eye-blink record (characterDoEyeAnims / characterSetHeadYawToTarget) */
    u8 pad3D4[0x3DC - 0x3D4];
    void *path; /* rom-curve/path record */
    void *savedPendingParentObj; /* obj+0xC0 swap slot around the player-interface update */
    u8 unk3E4[4];
    f32 glowAlpha; /* 0x3e8: alpha of the red glow tint RGBA(200,0,0,glowAlpha), passed to objSetGlowColor + objParticleFn alpha arg in baddie render */
    f32 glowRate; /* 0x3ec: per-frame delta added to glowAlpha; negated when the alpha ramp reaches its ceiling (ktrex) */
    s16 triggerId; /* config-sourced id (loaded from config+0x22) handed to BaddieControlInterface.spawnChild when a move/landing event fires */
    s16 gameBitA; /* set 1 on trigger */
    s16 gameBitB; /* set 1 / cleared 0; also passed to interface[10] */
    s16 gameBitC; /* gate; checked != -1 + mainGetBit */
    u8 unk3F8[2];
    s16 soundIdA; /* config-sourced sound-id (config+32); played via interface[+8]
        on the stop/cleanup path (dll19func12) and passed with soundIdB to the
        route/move interface [+0x3c] (dllcb/dllce/icebaddie) */
    s16 soundIdB; /* config-sourced sound-id (config+30); paired with soundIdA */
    u16 aggroRange; /* target-acquire radius passed to interface+0x48; (f32)(u32) conversions */
/* flags400 bit: baddie is advancing along its ROM curve path. Set once the
 * RomCurveWalker is successfully initialised (dll19func0), then each update
 * step calls Curve_AdvanceAlongPath while it is set and clears it at the end
 * of the path (dllcb). u16 field - no LL form needed. */
#define BADDIE_FLAG400_PATH_ACTIVE 0x8
    u16 flags400; /* bit flags 2/8/0x100; &flags400 also passed as a buffer base */
    s16 targetState; /* 0 = no target; tryAcquireTarget vs updateTargetMotion */
    u8 configFlags; /* bits 1/2/0x10 */
    u8 subMode; /* sub-state-machine index 0/1/2 (switch/==-tested; &subMode handed to BaddieControlInterface.processMessages as the route-phase out-param) */
    u8 aggression; /* percent-ish; randomGetRange(0, x), > 50 compares */
    u8 unk407[0x40C - 0x407];
    void *control; /* per-family control/extra record (engine-allocated; treasurechest casts its slot to LandedArwingState*) */
} GroundBaddieState;

STATIC_ASSERT(sizeof(GroundBaddieState) == 0x410);
STATIC_ASSERT(offsetof(GroundBaddieState, routeNav) == 0x35C);
STATIC_ASSERT(offsetof(GroundBaddieState, routeState) == 0x384);
STATIC_ASSERT(offsetof(GroundBaddieState, eyeAnimState) == 0x3AC);
STATIC_ASSERT(offsetof(GroundBaddieState, targetState) == 0x402);
STATIC_ASSERT(offsetof(GroundBaddieState, control) == 0x40C);

#endif /* MAIN_DLL_BADDIE_STATE_H_ */
