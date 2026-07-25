#define BADDIE_MOVE_STATUS_SIGNED

#include "main/dll/modgfx_interface.h"
#include "main/dll/partfx_interface.h"
#include "game/objects/object_setup.h"
#include "main/model_engine.h"
#include "main/model_engine_ui_api.h"
#include "sys/objects/lifecycle.h"
#include "main/dll/dll_80136a40.h"
#include "main/debug.h"
#include "main/render_envfx_api.h"
#include "game/objects/object.h"
#include "main/model.h"
#include "main/maketex_api.h"
#include "main/objprint_api.h"
#include "main/objprint_character_api.h"
#include "main/objprint_render_api.h"
#include "main/dll/objfx_api.h"
#include "main/dll/player_api.h"
#include "main/dll/player_spirit_api.h"
#include "main/dll/player_state_api.h"
#include "main/dll/player_motion_api.h"
#include "main/dll/dll_00E5_shield_api.h"
#include "main/dll/dll_000D_playershadow.h"
#include "main/dll/dll_01B5_lightfoot.h"
#include "main/dll/DB/DBprotection.h"
#include "main/dll/SB/dll_01E8_sbgalleon.h"
#include "main/dll/dll_00E2_staff_api.h"
#include "main/dll/CF/staffactivated_helpers.h"
#include "main/dll/viewfinder.h"
#include "main/sky_api.h"
#include "main/object_render.h"
#include "main/dll/dll_0015_curves.h"
#include "track/intersect_api.h"
#include "main/track_dolphin_api.h"
#include "main/track_bbox_api.h"
#include "main/vecmath_distance_api.h"

#include "sys/objects.h"
#include "main/curve_eval.h"
#include "main/objhits.h"
#include "main/audio/sfx_ids.h"
#include "main/audio/stream_api.h"
#include "main/audio/sfx_keep_alive_api.h"
#include "main/audio/sfx_object_query_api.h"
#include "main/audio/sfx_play_legacy_api.h"
#include "main/audio/sfx_position_api.h"
#include "main/audio/sfx_stop_object_api.h"
#include "main/audio/music_api.h"
#include "main/gameloop_api.h"
#include "main/dll/dll_0000_gameui_api.h"
#include "main/lightmap_api.h"
#include "main/newshadows_audio_api.h"
#include "main/objfx.h"
#include "main/screen_transition.h"
#include "main/object_transform.h"
#include "ghidra_import.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/objseq_api.h"
#include "main/shader_api.h"
#include "main/pi_dolphin_api.h"
#include "main/dll/player_state.h"
#include "main/dll/baddie_control_interface.h"
#include "main/dll/boneparticleeffect_interface.h"
#include "main/camera_interface.h"
#include "main/camera.h"
#include "main/dll/rom_curve_interface.h"
#include "main/dll/waterfx_interface.h"

#include "main/game_ui_interface.h"
#include "main/mapEventTypes.h"
#include "main/mm.h"
#include "main/objanim.h"
#include "main/objanim_update.h"
#include "main/objtexture.h"
#include "main/objseq.h"
#include "main/dll/player_motion.h"
#include "main/dll/player_objects.h"
#include "main/dll/player_status.h"
#include "main/dll/player_target.h"
#include "main/resource.h"
#include "main/sky_interface.h"
#include "main/vecmath.h"
#include "main/dll/path_control_interface.h"
#include "main/frame_timing.h"
#include "main/byte_flags.h"
#include "main/pad.h"
#include "dolphin/mtx/mtx_legacy.h"
#include "dolphin/gx/GXPixel.h"
#include "dolphin/gx/GXTransform.h"
#include "string.h"
#include "main/dll/dll_002F_carryable.h"
#include "main/dll/dll_0104_smallbasket.h"
#define FEAR_TEST_METER_POSITION_INT
#include "main/dll/dll_0000_gameui.h"
#undef FEAR_TEST_METER_POSITION_INT
#include "main/dll/dll_00C9_enemy.h"
#include "main/obj_group.h"
#include "main/obj_link.h"
#include "main/obj_message.h"
#include "main/obj_path.h"
#include "main/obj_query.h"
#include "main/player_eye_anim.h"
#include "main/dll/dll_029B_arwingandrossstuff.h"
#include "main/dll/player.h"
#include "main/dll/tricky_api.h"
#include "main/gamebits.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/audio/music_trigger_ids.h"
#include "main/gamebit_ids.h"
#include "main/player_control_interface.h"
#include "main/sky.h"

#undef BADDIE_MOVE_STATUS_SIGNED

extern int lbl_80332EC0[5];
extern void* gPlayerSpawnedObjects[7];

extern s16 lbl_80332EF0[30];

extern s16 lbl_80332F2C[14];

extern s16 lbl_80332F48[24];

extern s16 lbl_80332F78[8];

extern s16 lbl_80332F88[28];

extern const u8 lbl_802C2B30[12][16];

extern const f32 lbl_802C2BF0[24];

extern const int lbl_802C2C50[6];
extern const int lbl_802C2C68[4];
extern const u8 lbl_802C2C78[16];
extern const u16 lbl_802C2C88[16];

void playerUpdateTail(int unused1, int* unused2, f32* vec, int unused3, int mode, f32 angle);
void playerDoTailAnims(int obj, void* statep);
void playerUpdatePathEffectCountdown(GameObject* obj, int inner);
int playerStopRidingObject(GameObject* obj);
void playerSetHaveSpell(GameObject* obj, int spell, int set);
int fn_80297498(void);
int playerState41(GameObject* obj, int state, f32 fv);
int playerState40(int p1, int obj);
int playerState3F(int obj, int state);
int playerStateNop3E(void);
void fn_8029782C(GameObject* obj);
int playerState3D(int obj, int state, f32 fv);
int playerState3C(GameObject* obj, int state, f32 fv);
int playerState3B(GameObject* obj, int state, f32 fv);
int playerState3A(GameObject* obj, int state, f32 fv);
int playerState39(GameObject* obj, int state, f32 fv);
int playerState38(GameObject* obj, int state, f32 fv);
int playerState37(GameObject* obj, int state);
void fn_802985AC(GameObject* obj);
int playerStateSuperQuake(GameObject* obj, int state, f32 fv);
void fn_80298924(int obj);
int playerState35(GameObject* obj, int state);
int playerState34(GameObject* obj, int state);
int playerStateStaffLiftRock(int obj, int state, f32 fv);
void fn_802994A4(GameObject* obj);
int playerStateStaffBoost(GameObject* obj, int state, f32 fv);
int playerState31(GameObject* obj, int p2);
int playerState30(GameObject* obj, int state, f32 fv);
void fn_8029A420(GameObject* obj);
void fn_8029A4A8(GameObject* obj, int p2);
int playerStateFireLaser(int obj, int state, f32 fv);
int playerStateShootFireball(GameObject* obj, int state, f32 fv);
int playerStateTryCastSpell(GameObject* obj, int state, f32 fv);
int playerStateStopAimStaff(int obj, int state, f32 fv);
int playerStateStartAimStaff(GameObject* obj, int state, f32 fv);
int playerState29(GameObject* obj, int state);
int playerState28(GameObject* obj, int state, f32 fv);
void fn_8029BC08(GameObject* obj);
int playerState27(GameObject* obj, int state, f32 fv);
void fn_8029C8C8(GameObject* obj, int p2);
int playerState25(int obj, int state, f32 fv);
int playerState24(GameObject* obj, int state, f32 fv);
int playerState23(GameObject* obj, int state, f32 fv);
int playerState22(GameObject* obj, int state);
int playerState21(int obj, int state, f32 fv);
int playerState20(GameObject* obj, int state, f32 fv);
int playerState1F(GameObject* obj, int state, f32 fv);
int playerState1E(int obj, int state, f32 fv);
void fn_8029DAE0(GameObject* obj, int* p2);
int playerState1C(GameObject* obj, int state);
int playerState1B(GameObject* obj, int state, f32 fv);
int playerStateOnCloudRunner(GameObject* obj, int state);
int playerState19(GameObject* obj, int state);
void fn_8029F67C(GameObject* obj);
int playerStateOnBike(GameObject* obj, int state);
int playerState17(int p1, int state);
int playerStateMountBike(GameObject* obj, int state, f32 fv);
void fn_8029FFD0(GameObject* obj, int p2);
void objUpdateHitboxPos(int obj);
int playerStateClimbDownFromWall(GameObject* obj, int state);
int playerStateClimbUpFromWall(GameObject* obj, int state);
int playerStateClimbOntoWall(GameObject* obj, int state);
void playerPlayClimbingSound(GameObject* obj, int p2);
int playerState11(GameObject* obj, int state);
int playerStateSlideDownLadder(GameObject* obj, int state, f32 fv);
int playerStateClimbOntoLadder(GameObject* obj, int state, f32 fv);
int playerState0D(GameObject* obj, int p2);
int playerState0B(GameObject* obj, int state);
int playerStateGrabLedge(GameObject* obj, int state);
int playerState09(GameObject* obj, int state);
void fn_802A49A8(GameObject* obj);
int playerStateThrowing(GameObject* obj, int state);
void fn_802A4B4C(GameObject* obj);
int playerState06(GameObject* obj, int state);
int playerState05(GameObject* obj, int state);
int playerState04(int obj, int state, f32 fv);
int playerStateIceSpell(int obj, int state, f32 fv);
void fn_802A514C(GameObject* obj, int state);
int playerState00(int obj, int state);
s16 fn_802A71E0(int obj, int baseMoveId, int blendMoveId, int* blendAnchor, int* blendPlane,
                f32 samplePhase, f32 moveStepScale, int axis, int flags);
void fn_802A81B8(GameObject* obj, int state, f32* out);
int fn_802A8680(int p1, int p2, void* src, f32* vec, int out, int flag);
int fn_802A8EE4(int a, int b, void* c, int d, f32* e, f32 distance);
void fn_802A93F4(GameObject* obj, int p2, int p3);
void playerCastIceSpell(GameObject* unused);
int fn_802A97D0(GameObject* obj, int p2);
int playerCanCastPortalOpenSpell(GameObject* obj, int p2);
int playerCanCastQuakeSpell(GameObject* obj, int p2);
int playerCanCastBlasterSpell(GameObject* obj, int p2, int p3);
int playerIsBlasterSpellAvailable(GameObject* obj, int p2, int p3);
void fn_802A9D0C(int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8);
void fn_802AA014(GameObject* obj, int state, f32 aimInputZ, f32 zero);
void fn_802AA2B0(int obj, int state, f32 unused, f32 yoff);
void staffShootFireball(GameObject* obj, int p2, f32 unused);
void objDoTeleportAnim(GameObject* obj);
void playerDie(GameObject* obj);
void fn_802AABE4(int obj);
void playerDrawTeleportAnim(GameObject* obj);
void fn_802AAF80(GameObject* obj, int inner, int a, int b, int c);
int fn_802AB1D0(GameObject* obj);
void playerCastSpell(int a, int b, int c);
void fn_802AB5A4(GameObject* obj, int p2, int flags);
void playerCalcWaterCurrent(f32* outX, f32* outZ, f32 p3, int player);
int fn_802ABAE8(GameObject* obj, int state, int inner, f32 fv);
void fn_802ABFBC(GameObject* obj, int state, PlayerState* inner);
void fn_802AC32C(int p1, int p2, int p3);
void playerSetMovingAnims(int p1, int obj);
int fn_802ADC08(GameObject* obj, int inner, int p3);
void fn_802ADE80(GameObject* obj, int inner, int state);
int fn_802AE480(GameObject* obj, int inner, int state);
void fn_802AE650(GameObject* obj, int state, int p3);
void fn_802AE83C(int obj, int inner, int state);
void fn_802AE9C8(GameObject* obj, int inner, int state);
void fn_802AED2C(GameObject* obj, int state, int p3);
void staffAnimate(int obj, void* state, f32 dt);
void playerProcessQueuedItemCommand(GameObject* obj, int state);
void playerRunActiveSpells(GameObject* obj, int state);
void fn_802B066C(GameObject* obj, int state);
void playerStaffInit(GameObject* obj, int state);
void playerDoEyeAnims(GameObject* obj, int state);
int fn_80295A04(GameObject* obj, int sel);
void fn_802B18BC(GameObject* obj, int state, f32 fv);
void playerDoControls(GameObject* obj, int state, f32 fv);
void fn_802B1E5C(GameObject* obj, int state, int cfg, f32 dt);
void fn_802B4A9C(GameObject* obj, int inner, int inner2);
void playerAnimate(GameObject* obj, int state, f32 fv);
void fn_802B4ED8(GameObject* obj, int p2, int mode);
void playerInitFuncPtrs(void);
#define LANTERNFIREFLY_OBJGROUP  0x30 /* DLL 0x10C lanternfirefly */
typedef struct
{
    int a;
    int b;
} IntPair2;

int playerCheckIfClimbingOntoWall(int obj, int state, int state2, void* out, f32 fv, u32 mask);
int fn_802AD2F4(GameObject* obj, int inner, int state);
int fn_802AC7DC(int obj, int state, int inner, f32 fv);


void playerUpdateTail(int unused1, int* unused2, f32* vec, int unused3, int mode, f32 angle);

void playerDoTailAnims(int obj, void* statep);
static inline ObjModel* Player_GetActiveModel(int obj)
{
    ObjAnimComponent* objAnim = (ObjAnimComponent*)obj;
    return (ObjModel*)objAnim->banks[objAnim->bankIndex];
}

static inline ObjHitsPriorityState* Player_GetObjHitsState(GameObject* obj)
{
    return (ObjHitsPriorityState*)obj->anim.hitReactState;
}
/* the player object's own group (joined at init, left on free) */
#define PLAYER_OBJGROUP 0x25
/* groups owned by other DLLs the player queries */
#define CFGUARDIAN_OBJGROUP      0x16 /* DLL 0x148 cfguardian */
#define BABYCLOUDRUNNER_OBJGROUP 0x20 /* DLL 0x14C babycloudrunner (secondary) */
#define STAFFACTIVATED_OBJ_GROUP 0x41 /* DLL 0x11C staffactivated */
#define MAGICPLANT_OBJGROUP_B    0x3e /* DLL 0xFE magicplant (group B) */

/* GameCube controller button masks (tested against PlayerState.buttons* fields) */
#define PAD_BUTTON_A  0x100
#define PAD_BUTTON_B  0x200
#define PAD_BUTTON_X  0x400
#define PAD_BUTTON_Y  0x800
#define PAD_TRIGGER_L 0x40

extern f32 lbl_803E8064;
extern f32 lbl_803E8074;
extern f32 lbl_803E8030;
extern f32 lbl_803E8078;
extern f32 lbl_803E807C;
extern f32 lbl_803E8080;


extern f32 lbl_803E8020;


typedef struct
{
    u8 pad0[0xc];
    f32 fz0;
    f32 fz1;
    u8 pad1[8];
    f32 nx;
    f32 ny;
    f32 nz;
    f32 nw;
    u8 pad2[0x10];
    f32 ga;
    f32 gb;
    u8 pad3[4];
    f32 gt;
    u8 pad4[6];
    s8 flags;
    u8 pad5;
} WallHit;



extern f32 lbl_803E8090;
extern f32 lbl_803E8094;
extern f32 lbl_803E8098;
extern f32 lbl_803E809C;
extern f32 lbl_803E80A0;
extern char sNotOnGroundFailureMessage[23];

int fn_802A87CC(GameObject* obj, char* cam, f32* out, f32* vec, f32 fa, f32 fb);
int player_probeClimbable(GameObject* obj, int p4, void* src, int dst, int flag);

static inline void playerFreeSpawnedObjects(void** p, int i, int hi)
{
    do
    {
        if (*p != NULL)
        {
            Obj_FreeObject((GameObject*)*p);
            *p = (void*)hi;
        }
        p++;
        i++;
    } while (i < 7);
}

typedef struct
{
    u8 pad[0x1ba8];
    int moveA[4];
    int moveB[4];
    int moveC[4];
    f32 spdD[4];
    f32 spdE[4];
} HeadMoveTable;

typedef struct
{
    int a;
    int b;
} ColPair;

extern ColPair lbl_803E7E78;
extern f32 lbl_803E7FE4;

extern u8 lbl_803DC6A8[8];
extern u8 lbl_803DC6B0[2];
extern f32 lbl_803E8164;

typedef struct
{
    int a[6];
} UiMsgBlock;

static inline u32 playerLoadPendingHitBits(char* p)
{
    return *(u32*)p;
}

typedef struct
{
    u8 pad[0x88];
    u8 flags;
    u8 pad2[0x1f];
    u8 valsA[3];
    u8 valsB[5];
} HitDesc;

extern f32 lbl_803E8160;


typedef struct
{
    s16 rx, ry, rz;
    f32 scale;
    f32 x, y, z;
} HitFxDesc;

typedef struct
{
    u8 knock : 3;
    u8 low : 5;
} KnockBits;

typedef struct
{
    f32 x, y, z;
} VecXYZ;

static inline void Player_ApplyStatusDamage(GameObject* obj, int param)
{
    PlayerStatus* pc;
    PlayerState* in2;
    int v;

    in2 = obj->extra;
    pc = (PlayerStatus*)in2->playerStatus;
    v = pc->health;
    v -= param;
    if (v < 0)
    {
        v = 0;
    }
    else if (v > pc->maxHealth)
    {
        v = pc->maxHealth;
    }
    pc->health = (s8)v;
    if (((PlayerStatus*)in2->playerStatus)->health <= 0)
    {
        playerDie(obj);
    }
}

extern f32 lbl_803E7FB8;

extern f32 lbl_803E8034;
extern f32 lbl_803E803C;

extern f32 lbl_803E8054;

extern f32 lbl_803E8084;
extern f32 lbl_803E8088;

/* Number of directional sweep probes (parallel dirs[13]/dirMasks[13] tables). */
#define PLAYER_SWEEP_DIR_COUNT 13

/*
 * Probe for a climbable map surface (a HITQUERY_CLIMB_SURFACE collision hit) and,
 * if one is found near the player, seed the climb state at `dst` (PlayerState's
 * climb block: climbStepCount = surface height / step size, climbStepHeight,
 * climbStep) and return 1; return 0 when no ladder is in range. Called per
 * candidate direction from the player move handler.
 */
enum HitQueryMask
{
    HITQUERY_TEST_OBJECT_HITBOXES = 0x01,  /* also test reset-object hitboxes, not just map triangles */
    HITQUERY_REUSE_TRIANGLE_BUFFER = 0x10, /* reuse the loaded map-triangle buffer (skip block reload) */
    HITQUERY_SKIP_CULLED_OBJECTS = 0x80,   /* skip objects whose modelInstance flag 0x01000000 is set */
    /* Composite the player's ladder/climb probe issues: a climb-typed map
     * surface, map triangles only (no 0x01 -> no object hitboxes). */
    HITQUERY_CLIMB_SURFACE = 0x204,
};

void playerUpdatePathEffectCountdown(GameObject* obj, int inner);

int playerStopRidingObject(GameObject* obj);

void fn_80295918(GameObject* obj, int sel, f32 fval);

extern int lbl_803E7E70;
extern f32 lbl_803E80FC;
extern f32 lbl_803E8100;

extern f32 lbl_803E8104;
extern f32 lbl_803E8108;
extern f32 lbl_803E810C;
extern f32 lbl_803E8110;

extern int lbl_803E7E68;
extern int lbl_803E7E6C;


extern f32 lbl_803E8134;

int fn_80295A04(GameObject* obj, int sel);



void objSetPos(GameObject* obj, f32 f1, f32 f2, f32 f3);

int objIsCurModelNotZero(void* obj);

int isTrickyNear(GameObject* obj);

int fn_80295C0C(GameObject* obj);

int fn_80295C24(GameObject* obj);

int fn_80295C40(GameObject* obj);

int fn_80295C5C(GameObject* obj);

int fn_80295C88(GameObject* player);

int fn_80295CBC(GameObject* obj);

int playerIsDisguised(GameObject* obj);

int playerIsPathFollowing(GameObject* player);

void staffToggle(GameObject* obj, int a);

void playerSetDisguised(GameObject* obj, int mode);

int fn_8029605C(GameObject* obj, f32* p2, f32* p3);

void fn_802960E4(int a, f32 b, f32 c);

void fn_802960E8(GameObject* player, s16 effectId);

void fn_802960F4(GameObject* obj, f32** outFxOffsets);

f32 fn_8029610C(GameObject* obj);

int fn_80296118(GameObject* obj);

void fn_80296124(GameObject* obj, const Vec3f* position, const Vec3s* rotation, int unused);

void fn_802961A4(GameObject* obj, int* outMove, f32* outChargeLevel);

void objSetXRot(GameObject* obj, int v);

void fn_802961FC(GameObject* obj, u8 type);

f32 fn_80296214(GameObject* obj);

void fn_80296220(GameObject* obj, f32 v);

int Obj_IsParentSlackClear(GameObject* obj);

int fn_80296240(GameObject* obj);

int objFn_802962b4(GameObject* obj);

int fn_8029630C(GameObject* obj);

int objAnimFn_80296328(GameObject* obj);

u8 fn_80296414(GameObject* obj, GameObject* otherObj, u8* out);

int playerGetFlags3F0Bit5(GameObject* obj);

int EmissionController_IsLingering(GameObject* obj);

int fn_80296464(GameObject* player);

void playerSetHaveSpell(GameObject* obj, int spell, int set);

int playerHasSpell(GameObject* obj, int spell);

void objSetAnimStateFlags(GameObject* obj, int flag, int set);

int objGetAnimStateFlags(GameObject* obj, int flag);

int playerGetTimeScale(GameObject* obj, f32* out);

int playerSetHeldObject(GameObject* obj, GameObject* heldObj);

int fn_8029669C(GameObject* obj);

int fn_802966B4(GameObject* obj);

GameObject* objGetFirstChild(GameObject* obj);

int playerGetHeldObject(GameObject* obj, GameObject** outHeldObj);

f32 fn_802966F4(GameObject* obj);

int objFn_80296700(GameObject* obj);

void playerPutAwayStaff(GameObject* obj, int mode);

void playerPullOutStaff(GameObject* obj, int mode);

int playerGetMoney(GameObject* player);

void playerAddMoney(GameObject* obj, int amount);

void fn_8029697C(GameObject* obj, s16* out1, s16* out2);

int fn_802969F0(GameObject* obj);

int playerGetCurMagic(GameObject* player);

void playerAddRemoveMagic(GameObject* obj, int amount);

int playerGetMaxMagic(GameObject* obj);

void fn_80296A9C(GameObject* obj, int delta);

int playerGetMaxHealth(GameObject* obj);

int playerGetCurHealth(GameObject* obj);

void playerAddHealth(GameObject* obj, int amount);

void saveSetOverrideHealth(int v);

void playerCancelSpell(GameObject* obj, int p2);

int objGetAnimState80A(GameObject* obj);

void fn_80296BBC(GameObject* obj);

void cameraGetPrevPos2(GameObject* obj, f32* x, f32* y, f32* z);

void playerLock(GameObject* obj, int lock);

int playerStatusIsPositive(GameObject* obj);

int fn_80296C4C(GameObject* obj);

int playerIsDead(GameObject* player);

void playerSetIsDead(GameObject* obj, int flag);

void playerHeal(GameObject* obj);

void fn_80296D20(GameObject* obj, GameObject* parentObj);

void fn_80296EB4(GameObject* obj, int newParent);

void playerSetInCutscene(GameObject* obj);

void playerSetCutsceneCameraFlag(GameObject* obj);

void playerSetOverrideParentSlack(GameObject* obj);

u32 playerGetStateFlag310(GameObject* obj);

GameObject* playerGetFocusObject(GameObject* obj);

void fn_802972B4(GameObject* obj, u32* flags, f32* p5, f32* p6, f32* p7, u16* p8);

int fn_80297498(void);

int playerState41(GameObject* obj, int state, f32 fv);

int playerState40(int p1, int obj);

int playerState3F(int obj, int state);

int playerStateNop3E(void);

void fn_8029782C(GameObject* obj);

int playerState3D(int obj, int state, f32 fv);

int playerState3C(GameObject* obj, int state, f32 fv);

int playerState3B(GameObject* obj, int state, f32 fv);

int playerState3A(GameObject* obj, int state, f32 fv);

int playerState39(GameObject* obj, int state, f32 fv);

int playerState38(GameObject* obj, int state, f32 fv);

int playerState37(GameObject* obj, int state);

void fn_802985AC(GameObject* obj);

int playerStateSuperQuake(GameObject* obj, int state, f32 fv);

void fn_80298924(int obj);

int playerState35(GameObject* obj, int state);

int playerState34(GameObject* obj, int state);

int playerStateStaffLiftRock(int obj, int state, f32 fv);

void fn_802994A4(GameObject* obj);

int playerStateStaffBoost(GameObject* obj, int state, f32 fv);

int playerState31(GameObject* obj, int p2);

int playerState30(GameObject* obj, int state, f32 fv);

void fn_8029A420(GameObject* obj);

void fn_8029A4A8(GameObject* obj, int p2);

int playerStateFireLaser(int obj, int state, f32 fv);

int playerStateShootFireball(GameObject* obj, int state, f32 fv);

int playerStateTryCastSpell(GameObject* obj, int state, f32 fv);

int playerStateAimStaff(int obj, int state, f32 fv);

int playerStateStopAimStaff(int obj, int state, f32 fv);

int playerStateStartAimStaff(GameObject* obj, int state, f32 fv);

int playerState29(GameObject* obj, int state);

int playerState28(GameObject* obj, int state, f32 fv);

void fn_8029BC08(GameObject* obj);

int playerState27(GameObject* obj, int state, f32 fv);

int playerStateAttack(GameObject* obj, int state, f32 fv);

void fn_8029C8C8(GameObject* obj, int p2);

int playerState25(int obj, int state, f32 fv);

int playerState24(GameObject* obj, int state, f32 fv);

int playerState23(GameObject* obj, int state, f32 fv);

int playerState22(GameObject* obj, int state);

int playerState21(int obj, int state, f32 fv);

int playerState20(GameObject* obj, int state, f32 fv);

int playerState1F(GameObject* obj, int state, f32 fv);

int playerState1E(int obj, int state, f32 fv);

void fn_8029DAE0(GameObject* obj, int* p2);

int playerState1D(int obj, PlayerState* state, f32 fv);

int playerState1C(GameObject* obj, int state);

int playerState1B(GameObject* obj, int state, f32 fv);

int playerStateOnCloudRunner(GameObject* obj, int state);

int playerState19(GameObject* obj, int state);

void fn_8029F67C(GameObject* obj);

int playerStateOnBike(GameObject* obj, int state);

int playerState17(int p1, int state);

int playerStateMountBike(GameObject* obj, int state, f32 fv);

void fn_8029FFD0(GameObject* obj, int p2);

void objUpdateHitboxPos(int obj);

int playerStateClimbDownFromWall(GameObject* obj, int state);

int playerStateClimbUpFromWall(GameObject* obj, int state);

int playerStateClimbWall(GameObject* obj, int stateArg);

int playerStateClimbOntoWall(GameObject* obj, int state);

void playerPlayClimbingSound(GameObject* obj, int p2);

int playerState11(GameObject* obj, int state);

int playerStateSlideDownLadder(GameObject* obj, int state, f32 fv);

int playerStateOnLadder(int obj, int state);

int playerStateClimbOntoLadder(GameObject* obj, int state, f32 fv);

int playerState0D(GameObject* obj, int targetState);

int playerStateClimbLedge(int obj, int state, f32 fv);

int playerState0B(GameObject* obj, int state);

int playerStateGrabLedge(GameObject* obj, int state);

int playerState09(GameObject* obj, int state);

int playerState08(GameObject* obj, int state, f32 fv);

void fn_802A49A8(GameObject* obj);

int playerStateThrowing(GameObject* obj, int state);

void fn_802A4B4C(GameObject* obj);

int playerState06(GameObject* obj, int state);

int playerState05(GameObject* obj, int state);

int playerState04(int obj, int state, f32 fv);

int playerStateIceSpell(int obj, int state, f32 fv);

void fn_802A514C(GameObject* obj, int state);

int playerStateMoving(int obj, int state, f32 fv);

int playerStateIdle(int obj, int state, f32 fv);

int playerState00(int obj, int state);

s16 fn_802A71E0(int obj, int baseMoveId, int blendMoveId, int* blendAnchor, int* blendPlane,
                f32 samplePhase, f32 moveStepScale, int axis, int flags);

int playerCheckIfClimbingOntoWall(int obj, int state, int state2, void* out, f32 fv, u32 mask);

void fn_802A81B8(GameObject* obj, int state, f32* out);

int player_probeClimbable(GameObject* obj, int p4, void* src, int dst, int flag);

int fn_802A8680(int p1, int p2, void* src, f32* vec, int out, int flag);

int fn_802A87CC(GameObject* obj, char* cam, f32* out, f32* vec, f32 fa, f32 fb);

int fn_802A8EE4(int a, int b, void* c, int d, f32* e, f32 distance);

void fn_802A93F4(GameObject* obj, int p2, int p3);

void playerCastIceSpell(GameObject* unused);

int fn_802A97D0(GameObject* obj, int p2);

int playerCanCastPortalOpenSpell(GameObject* obj, int p2);

int playerCanCastQuakeSpell(GameObject* obj, int p2);

int playerCanCastBlasterSpell(GameObject* obj, int p2, int p3);

int playerIsBlasterSpellAvailable(GameObject* obj, int p2, int p3);

void fn_802A9D0C(int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8);

void fn_802AA014(GameObject* obj, int state, f32 aimInputZ, f32 zero);

void fn_802AA2B0(int obj, int state, f32 unused, f32 yoff);

void staffShootFireball(GameObject* obj, int state, f32 unused);

void objDoTeleportAnim(GameObject* obj);

void playerDie(GameObject* obj);

typedef struct PlayerSeqPlacement {
    ObjPlacement base;
    u8 pad18[8];
    s8 movementEnabled;
} PlayerSeqPlacement;

STATIC_ASSERT(offsetof(PlayerSeqPlacement, movementEnabled) == 0x20);

void fn_802AABE4(int obj);

void playerDrawTeleportAnim(GameObject* obj)
{
    int state = *(int*)&obj->extra;
    LightmapVertex* vp = gPlayerHudVtxBuf;
    LightmapVertex* p = vp;
    int i;
    f32 height;
    f32 v;
    struct
    {
        s16 rx, ry, rz, pad;
        f32 scale;
        f32 px, py, pz;
    } xf;
    f32 mtx[16];

    height = ((PlayerState*)state)->teleportAnimProgress;
    setTextColor(0, 0xff, 0xff, 0xff, 0x80);
    textureSetupFn_800799c0();
    textRenderSetupFn_800795e8();
    textRenderSetupFn_80079804();
    fn_80078740();
    GXSetColorUpdate(0);

    i = 0;
    for (; i < 8; i++)
    {
        v = lbl_803E7FA4 * (lbl_803E80C4 - height);
        if (i < 4)
        {
            p->y = 0x320;
        }
        else
        {
            p->y = v;
        }
        if (i < 4)
        {
            p->x = (lbl_803E7FA4 * lbl_802C2BF0[i * 3 + 0]);
            p->z = (lbl_803E7FA4 * lbl_802C2BF0[i * 3 + 2]);
        }
        else
        {
            p->x = (lbl_803E7FA4 * lbl_802C2BF0[i * 3 + 0]);
            p->z = (lbl_803E7FA4 * lbl_802C2BF0[i * 3 + 2]);
        }
        p->r = 0xff;
        p->g = 0;
        p->b = 0;
        p->a = 0x40;
        p++;
    }

    xf.px = obj->anim.localPosX - playerMapOffsetX;
    xf.py = obj->anim.localPosY;
    xf.pz = obj->anim.localPosZ - playerMapOffsetZ;
    xf.rx = ((PlayerState*)state)->targetYaw;
    xf.ry = 0;
    xf.rz = 0;
    xf.scale = lbl_803E7F6C;
    setMatrixFromObjectTransposed(&xf, mtx);
    PSMTXConcat(Camera_GetViewMatrix(), mtx, mtx);
    GXLoadPosMtxImm((const f32(*)[4])mtx, 0);
    drawFn_8005cf8c(vp, (u8*)lbl_802C2B30, 0xc);

    if (((PlayerState*)state)->teleportAnimProgress >= lbl_803E80E0)
    {
        int t = obj->anim.alpha - (framesThisStep << 2);
        if (t < 0)
        {
            t = 0;
        }
        obj->anim.alpha = t;
    }
    GXSetColorUpdate(1);
}

void fn_802AAF80(GameObject* obj, int inner, int a, int b, int c)
{
    int v;
    if (gPlayerPathObject != NULL && (((u32)((PlayerState*)inner)->flags3F4 >> 6) & 1) != 0)
    {
        (*gModgfxInterface)->renderEffects((void*)a, b, c, 1, gPlayerPathObject);
    }
    if (((PlayerState*)inner)->pendingBoneEffectId != 0)
    {
        (*gBoneParticleEffectInterface)
            ->spawnEffect((void*)obj, ((PlayerState*)inner)->pendingBoneEffectId, NULL, 0x64, NULL);
    }
    ((PlayerState*)inner)->pendingBoneEffectId = 0;
    if (((PlayerState*)inner)->teleportAnimActive == 1)
    {
        objDoTeleportAnim(obj);
    }
    if ((*gSkyInterface)->getVisibility(2) != 0)
    {
        playerUpdatePathEffectCountdown(obj, inner);
    }
    v = ((PlayerState*)inner)->flags360;
    if ((v & 0x60000u) != 0)
    {
        ((PartFxSpawnParams*)gPlayerPartFxParams)->posX = obj->anim.localPosX;
        ((PartFxSpawnParams*)gPlayerPartFxParams)->posY = obj->anim.localPosY;
        ((PartFxSpawnParams*)gPlayerPartFxParams)->posZ = obj->anim.localPosZ;
        if ((v & 0x40000u) != 0)
        {
            (*gPartfxInterface)->spawnObject((void*)obj, 0x427, gPlayerPartFxParams, 0x200001, -1, NULL);
            (*gPartfxInterface)->spawnObject((void*)obj, 0x427, gPlayerPartFxParams, 0x200001, -1, NULL);
            (*gPartfxInterface)->spawnObject((void*)obj, 0x427, gPlayerPartFxParams, 0x200001, -1, NULL);
        }
        if ((((PlayerState*)inner)->flags360 & 0x20000u) != 0)
        {
            (*gWaterfxInterface)
                ->spawnSplashBurst((void*)obj, obj->anim.localPosX,
                                   (obj->anim.localPosY + ((PlayerState*)inner)->waterDepth) -
                                       lbl_803E7F10,
                                   obj->anim.localPosZ, lbl_803E7FFC);
            (*gWaterfxInterface)->spawnRipple(
                obj->anim.localPosX,
                obj->anim.localPosY + ((PlayerState*)inner)->waterDepth,
                obj->anim.localPosZ, 0, lbl_803E80E4, 2);
            *(u32*)&((PlayerState*)inner)->flags360 &= ~PLAYER_FLAG_WATER_SPLASH_PENDING;
        }
    }
}

int fn_802AB1D0(GameObject* obj)
{
    GameObject* cur;
    u32* objs;
    GameObject* best;
    int count;
    int i;
    f32 dist;
    f32 bestDist;
    f32 scale;
    s16 yaw;
    void* held;

    if (obj->objectFlags & OBJECT_OBJFLAG_PARENT_SLACK)
    {
        return 0;
    }
    held = *(void**)((char*)*(int*)&obj->extra + 0x2d0);
    if (held != NULL)
    {
        return (int)held;
    }
    best = NULL;
    objs = ObjGroup_GetObjects(8, &count);
    i = 0;
    bestDist = lbl_803E7EA4;
    for (; i < count;)
    {
        cur = (GameObject*)objs[i++];
        if ((cur->anim.classId == 0x1c || cur->anim.classId == 0x2a) && cur->anim.alpha == 0xff)
        {
            f32 dx = cur->anim.worldPosX - obj->anim.worldPosX;
            f32 dy = cur->anim.worldPosY - obj->anim.worldPosY;
            f32 dz = cur->anim.worldPosZ - obj->anim.worldPosZ;
            dist = dx * dx + dy * dy + dz * dz;
            if (dist < lbl_803E80E8)
            {
                if (dist <= lbl_803E7EA4)
                {
                    scale = (f32)cur->anim.modelInstance->group8RegistrationCount;
                    if (scale <= lbl_803E7EA4)
                    {
                        scale = lbl_803E7EE0;
                    }
                    dist = sqrtf(dist) / scale;
                }
                yaw = Obj_GetYawDeltaToObject(obj, cur, 0);
                if (yaw < 0x5555 && yaw > -0x5555)
                {
                    if (dist < bestDist || lbl_803E7EA4 == bestDist)
                    {
                        bestDist = dist;
                        best = cur;
                    }
                }
            }
        }
    }
    return (int)best;
}

void playerCastSpell(int a, int b, int c)
{
    switch (c)
    {
    case GAMEBIT_STAFF_ABILITY_FIRE_BLASTER:
        gPlayerSelectedItem = GAMEBIT_STAFF_ABILITY_FIRE_BLASTER;
        break;
    case 0x958:
        gPlayerSelectedItem = 0x958;
        break;
    case GAMEBIT_STAFF_ABILITY_FREEZE_BLAST:
        gPlayerSelectedItem = GAMEBIT_STAFF_ABILITY_FREEZE_BLAST;
        break;
    case GAMEBIT_STAFF_ABILITY_STAFF_BOOSTER:
        gPlayerInteractTarget = ((PlayerState*)b)->cameraTargetObject;
        (*gPlayerInterface)->setState((void*)a, (void*)b, 0x32);
        *(int*)&((PlayerState*)b)->baddie.unk304 = (int)fn_802994A4;
        break;
    case 0x107:
    case 0xc55:
        (*gPlayerInterface)->setState((void*)a, (void*)b, 0x36);
        *(int*)&((PlayerState*)b)->baddie.unk304 = (int)fn_802985AC;
        break;
    case 0x40:
        ((PlayerState*)b)->stateTimer = lbl_803E7EDC;
        {
            int sub = *(int*)((char*)((GameObject*)a)->extra + 0x35c);
            int v = *(s16*)((char*)sub + 0x4) - 0xa;
            if (v < 0)
            {
                v = 0;
            }
            else if (v > *(s16*)((char*)sub + 0x6))
            {
                v = *(s16*)((char*)sub + 0x6);
            }
            *(s16*)((char*)sub + 0x4) = v;
        }
        playerSetDisguised((GameObject*)a, 1);
        Sfx_PlayFromObject(a, SFXTRIG_dn_boar1_c_209);
        break;
    case 0x5bd:
        c = -1;
        {
            int sub = *(int*)((char*)((GameObject*)a)->extra + 0x35c);
            int v = *(s16*)((char*)sub + 0x4) - 0x14;
            if (v < 0)
            {
                v = 0;
            }
            else if (v > *(s16*)((char*)sub + 0x6))
            {
                v = *(s16*)((char*)sub + 0x6);
            }
            *(s16*)((char*)sub + 0x4) = v;
        }
        {
            void* cam = (void*)(*gCameraInterface)->getTarget();
            if (cam != NULL)
            {
                s16 id = ((GameObject*)cam)->anim.seqId;
                if (id == 0x414 || id == 0x4a9)
                {
                    c = 0x5bd;
                    getAngle(((GameObject*)cam)->anim.hitVolumeTransforms->jointX - ((GameObject*)a)->anim.localPosX,
                             ((GameObject*)cam)->anim.hitVolumeTransforms->jointZ - ((GameObject*)a)->anim.localPosZ);
                }
            }
        }
        break;
    }
    ((PlayerState*)b)->animState = c;
}

void fn_802AB5A4(GameObject* obj, int p2, int flags)
{
    u8 f = (u8)flags;
    char* q = (char*)p2 + 4;
    if (f & 1)
    {
        curves_updateLocalPointTransforms((int)obj, (CurvesCollisionState*)q);
    }
    if (f & 2)
    {
        curves_preparePointCollisionFrame((int)obj, (CurvesCollisionState*)((char*)(int)p2 + 4));
        ((CurvesCollisionState*)q)->points[2][0] = obj->anim.worldPosX;
        ((CurvesCollisionState*)q)->points[2][1] = lbl_803E80EC + obj->anim.worldPosY;
        ((CurvesCollisionState*)q)->points[2][2] = obj->anim.worldPosZ;
    }
    if (f & 4)
    {
        ((ObjHitsPriorityState*)obj->anim.hitReactState)->localPosX = obj->anim.localPosX;
        ((ObjHitsPriorityState*)obj->anim.hitReactState)->localPosY = obj->anim.localPosY;
        ((ObjHitsPriorityState*)obj->anim.hitReactState)->localPosZ = obj->anim.localPosZ;
        ((ObjHitsPriorityState*)obj->anim.hitReactState)->worldPosX = obj->anim.worldPosX;
        ((ObjHitsPriorityState*)obj->anim.hitReactState)->worldPosY = obj->anim.worldPosY;
        ((ObjHitsPriorityState*)obj->anim.hitReactState)->worldPosZ = obj->anim.worldPosZ;
    }
}

void playerCalcWaterCurrent(f32* outX, f32* outZ, f32 p3, int player)
{
    int any;
    PlayerState* inner = ((GameObject*)player)->extra;
    f32 sumC;
    f32 sumS;
    f32 ratio;
    f32 angle;
    int* objs;
    int n;
    int i;

    sumC = sumS = lbl_803E7EA4;
    objs = (int*)ObjGroup_GetObjects(0x14, &n);
    any = 0;
    for (i = 0; i < n; i++)
    {
        int o = objs[i];
        if (*(u8*)((char*)*(int*)((char*)o + 0x4c) + 0x1a) & 2)
        {
            f32 dy;
            any = 1;
            dy = *(f32*)((char*)o + 0x10) - ((GameObject*)player)->anim.localPosY;
            if (dy <= 200.0f && dy >= -200.0f)
            {
                f32 dx = *(f32*)((char*)o + 0xc) - ((GameObject*)player)->anim.localPosX;
                f32 dz = *(f32*)((char*)o + 0x14) - ((GameObject*)player)->anim.localPosZ;
                f32 dist = sqrtf(dx * dx + dz * dz);
                f32 thresh = 1.5f * (f32)(u32) * (u8*)((char*)*(int*)((char*)o + 0x4c) + 0x19);
                if (dist < thresh)
                {
                    ratio = 0.0f;
                    if (thresh > 0.0f)
                    {
                        ratio = (thresh - dist) / thresh;
                    }
                    ratio = ratio * (10.0f * *(f32*)((char*)o + 0x8));
                    sumC = ratio * mathSinf(3.1415927f * (f32)(int)*(s16*)((char*)o + 0) / 32768.0f) + sumC;
                    sumS = ratio * mathCosf(3.1415927f * (f32)(int)*(s16*)((char*)o + 0) / 32768.0f) + sumS;
                }
            }
        }
    }
    objs = (int*)ObjGroup_GetObjects(0x50, &n);
    for (i = 0; i < n; i++)
    {
        int o = objs[i];
        f32 strength = (f32)(u32) * (u8*)((char*)*(int*)((char*)o + 0x4c) + 0x32) / 10.0f;
        f32 dy;
        any = 1;
        dy = *(f32*)((char*)o + 0x10) - ((GameObject*)player)->anim.localPosY;
        if (dy <= 200.0f && dy >= -200.0f)
        {
            f32 dx = *(f32*)((char*)o + 0xc) - ((GameObject*)player)->anim.localPosX;
            f32 dz = *(f32*)((char*)o + 0x14) - ((GameObject*)player)->anim.localPosZ;
            int a22 = (s16)(getAngle(dx, dz) + 0x84d0);
            f32 dist = sqrtf(dx * dx + dz * dz);
            f32 thresh = (f32)(int)(*(u8*)((char*)*(int*)((char*)o + 0x4c) + 0x29) << 3);
            if (dist < thresh)
            {
                ratio = lbl_803E7EA4;
                if (thresh > lbl_803E7EA4)
                {
                    ratio = (thresh - dist) / thresh;
                }
                ratio = ratio * strength;
                angle = 3.1415927f * (f32)(int)a22 / 32768.0f;
                sumC = ratio * mathSinf(angle) + sumC;
                sumS = ratio * mathCosf(angle) + sumS;
            }
        }
    }
    if (any)
    {
        f32 mag;
        f32 k;
        sumC = sumC / (f32)(int)any;
        sumS = sumS / (f32)(int)any;
        k = lbl_803E7F6C;
        inner->avoidVelX = inner->avoidVelX - k * sumC;
        inner->avoidVelZ = inner->avoidVelZ - k * sumS;
        {
            f32 k;
            inner->avoidVelX = inner->avoidVelX * (k = lbl_803E7F68);
            inner->avoidVelZ = inner->avoidVelZ * k;
        }
        mag = sqrtf(inner->avoidVelX * inner->avoidVelX + inner->avoidVelZ * inner->avoidVelZ);
        if (mag > lbl_803E7F1C)
        {
            f32 s = lbl_803E7F1C / mag;
            inner->avoidVelX = inner->avoidVelX * s;
            inner->avoidVelZ = inner->avoidVelZ * s;
        }
        *outX = inner->avoidVelX * timeDelta;
        *outZ = inner->avoidVelZ * timeDelta;
    }
    else
    {
        *outX = 0.0f;
        *outZ = 0.0f;
    }
}

int fn_802ABAE8(GameObject* obj, int state, int inner, f32 fv)
{
    int d = ((PlayerState*)inner)->targetYaw - (u16)((PlayerState*)inner)->prevTargetYaw;
    int near;
    int g;
    if (d > 0x8000)
        d -= 0xffff;
    if (d < -0x8000)
        d += 0xffff;
    if ((((u32)((PlayerState*)inner)->flags3F1 >> 5) & 1) || (((u32)((PlayerState*)inner)->flags3F0 >> 4) & 1))
    {
        d = 0;
    }
    {
        f32 f2 = lbl_803E7E98 * (((PlayerState*)state)->baddie.animSpeedC - lbl_803E7E9C) + lbl_803E7EE0;
        if (f2 < lbl_803E7EA4)
        {
            f2 = lbl_803E7EA4;
        }
        d = (int)((f32)(int)d * (lbl_803E7FC4 * f2));
        d = (d < -0xccc) ? -0xccc : ((d > 0xccc) ? 0xccc : d);
    }
    d -= (u16)((PlayerState*)inner)->headPitch;
    if (d > 0x8000)
        d = d - 0xffff;
    if (d < -0x8000)
        d = d + 0xffff;
    ((PlayerState*)inner)->headPitch =
        (f32)(int)((PlayerState*)inner)->headPitch + interpolate((f32)(int)d, lbl_803E7EB4, timeDelta);
    near = fn_802AB1D0(obj);
    if ((u32)near != 0 && (((u32)((PlayerState*)inner)->flags3F0 >> 7) & 1) == 0 &&
        (((u32)((PlayerState*)inner)->flags3F0 >> 6) & 1) == 0 &&
        (((u32)((PlayerState*)inner)->flags3F0 >> 4) & 1) == 0 &&
        (((u32)((PlayerState*)inner)->flags3F0 >> 5) & 1) == 0)
    {
        int gd = (u16)getAngle(-(*(f32*)((char*)near + 0xc) - obj->anim.localPosX),
                               -(*(f32*)((char*)near + 0x14) - obj->anim.localPosZ)) -
                 (u16)((PlayerState*)inner)->targetYaw;
        f32 t;
        f32 f5;
        if (gd > 0x8000)
            gd -= 0xffff;
        if (gd < -0x8000)
            gd += 0xffff;
        t = lbl_803E7EE0 - (((PlayerState*)state)->baddie.animSpeedC - lbl_803E7E9C) /
                               (((PlayerState*)inner)->maxSpeed - lbl_803E7E9C);
        f5 = lbl_803E80C4 * ((t < *(f32*)&lbl_803E7EA4) ? lbl_803E7EA4 : ((t > lbl_803E7EE0) ? lbl_803E7EE0 : t)) +
             lbl_803E80F4;
        g = (int)(((f32)(int)gd < lbl_803E80F8 * -f5)
                      ? lbl_803E80F8 * -f5
                      : (((f32)(int)gd > lbl_803E80F8 * f5) ? lbl_803E80F8 * f5 : (f32)(int)gd));
    }
    else
    {
        g = 0;
    }
    {
        int r0;
        int h;
        if (!((((u32)((PlayerState*)inner)->flags3F1 >> 5) & 1) || (((u32)((PlayerState*)inner)->flags3F0 >> 4) & 1)))
        {
            r0 = ((PlayerState*)inner)->targetYawRate;
        }
        else
        {
            r0 = 0;
        }
        if (r0 < -0x28)
        {
            r0 = -0x28;
        }
        else if (r0 > 0x28)
        {
            r0 = 0x28;
        }
        h = g + r0 * 0xb6;
        if (h < -0x3ffc)
        {
            h = -0x3ffc;
        }
        else if (h > 0x3ffc)
        {
            h = 0x3ffc;
        }
        h = h - (u16)((PlayerState*)inner)->bodyLeanAngle;
        if (h > 0x8000)
            h -= 0xffff;
        if (h < -0x8000)
            h += 0xffff;
        h = (int)((f32)(int)h * lbl_803E7EB4);
        if (h < -0x16c)
        {
            h = -0x16c;
        }
        else if (h > 0x16c)
        {
            h = 0x16c;
        }
        ((PlayerState*)inner)->bodyLeanAngle = (f32)(int)h * timeDelta + (f32)(int)*(s16*)((int)inner + 0x4D4);
        ((PlayerState*)inner)->bodyLeanHalf = ((PlayerState*)inner)->bodyLeanAngle / 2;
    }
    {
        int k = (int)(lbl_803E80F8 * (lbl_803E7ED8 * -fv));
        k -= (u16)((PlayerState*)inner)->headYaw;
        if (k > 0x8000)
            k -= 0xffff;
        if (k < -0x8000)
            k += 0xffff;
        ((PlayerState*)inner)->headYaw = *(s16*)((int)inner + 0x4D6) + k;
    }
}

void fn_802ABFBC(GameObject* obj, int state, PlayerState* inner)
{
    f32 x1, y1, z1;
    f32 pos[3];
    GameObject* sub;

    inner->headPitch *= powfBitEstimate(lbl_803E7FF4, timeDelta);
    sub = inner->cameraTargetObject;
    if (sub != NULL && sub->anim.modelInstance->unk58 != 0)
    {
        ObjPath_GetPointWorldPosition((GameObject*)obj, 5, &x1, &y1, &z1, 0);
        if (objModelGetVecFn_800395d8((GameObject*)sub, 0) != 0)
        {
            objPosFn_80039510((GameObject*)sub, 0, pos);
        }
        else
        {
            pos[0] = sub->anim.localPosX;
            pos[1] = sub->anim.localPosY;
            pos[2] = sub->anim.localPosZ;
        }

        {
            f32 dx = pos[0] - x1;
            f32 dy = pos[1] - y1;
            f32 dz = pos[2] - z1;

            int d = getAngle(-dy, sqrtf(dx * dx + dz * dz)) & 0xffff;
            d -= (u16)inner->headYaw;
            if (d > 0x8000)
                d = d - 0xffff;
            if (d < -0x8000)
                d = d + 0xffff;
            d *= lbl_803E7EB4;
            inner->headYaw += d * timeDelta;

            d = getAngle(-dx, -dz) & 0xffff;
            d -= (u16)inner->targetYaw;

            if (d > 0x8000)
                d = d - 0xffff;
            if (d < -0x8000)
                d = d + 0xffff;

            d = (d < -0x1c70) ? -0x1c70 : ((d > 0x1c70) ? 0x1c70 : d);
            d -= (u16)inner->bodyLeanAngle;

            if (d > 0x8000)
                d = d - 0xffff;
            if (d < -0x8000)
                d = d + 0xffff;

            d *= lbl_803E7EB4;
            inner->bodyLeanAngle += d * timeDelta;
            inner->bodyLeanHalf = inner->bodyLeanAngle / 2;
        }
    }
    else
    {
        inner->headYaw *= powfBitEstimate(lbl_803E7F1C, timeDelta);
    }
}

void fn_802AC32C(int p1, int p2, int p3)
{
    void* near;
    int angle1;
    int angle2;

    near = (void*)fn_802AB1D0((GameObject*)p1);
    if (near != NULL && ((ByteFlags*)((char*)p3 + 0x3f0))->b80 == 0 && ((ByteFlags*)((char*)p3 + 0x3f0))->b40 == 0)
    {
        f32 ratio;
        f32 clamped;
        f32 f5;

        if (--*(s16*)&((PlayerState*)p3)->lookAtTimer <= 0)
        {
            *(s16*)&((PlayerState*)p3)->lookAtTimer = (s16)randomGetRange(0x78, 0xf0);
            *(s16*)&((PlayerState*)p3)->lookAtRandOffset = (s16)randomGetRange(0, 0x28);
        }
        angle1 = getAngle(-(*(f32*)((char*)near + 0xc) - ((GameObject*)p1)->anim.localPosX),
                          -(*(f32*)((char*)near + 0x14) - ((GameObject*)p1)->anim.localPosZ)) &
                 0xffff;
        angle1 -= (u16)((PlayerState*)p3)->targetYaw;
        if (angle1 > 0x8000)
        {
            angle1 = angle1 - 0xFFFF;
        }
        if (angle1 < -0x8000)
        {
            angle1 = angle1 + 0xFFFF;
        }
        ratio = lbl_803E7EE0 -
                (((PlayerState*)p2)->baddie.animSpeedC - lbl_803E7E9C) / (((PlayerState*)p3)->maxSpeed - *(f32*)&lbl_803E7E9C);
        f5 = lbl_803E80C4;
        clamped = (ratio < lbl_803E7EA4) ? lbl_803E7EA4 : ((ratio > lbl_803E7EE0) ? lbl_803E7EE0 : ratio);
        f5 = f5 * clamped + lbl_803E80F4;
        angle1 = ((f32)angle1 < lbl_803E80F8 * -f5)
                     ? lbl_803E80F8 * -f5
                     : (((f32)angle1 > lbl_803E80F8 * f5) ? lbl_803E80F8 * f5 : (f32)angle1);
    }
    else
    {
        angle1 = 0;
        *(s16*)&((PlayerState*)p3)->lookAtTimer = angle1;
    }

    {
        int v480;
        if (((ByteFlags*)((char*)p3 + 0x3f1))->b20)
        {
            v480 = 0;
        }
        else
        {
            v480 = ((PlayerState*)p3)->targetYawRate;
        }
        v480 = (v480 < -0x28) ? -0x28 : ((v480 > 0x28) ? 0x28 : v480);
        angle1 += v480 * 0xb6;
    }
    angle1 = (angle1 < -0x3ffc) ? -0x3ffc : ((angle1 > 0x3ffc) ? 0x3ffc : angle1);
    angle1 -= (u16)((PlayerState*)p3)->bodyLeanAngle;
    if (angle1 > 0x8000)
    {
        angle1 = angle1 - 0xFFFF;
    }
    if (angle1 < -0x8000)
    {
        angle1 = angle1 + 0xFFFF;
    }
    angle1 *= lbl_803E7EB4;
    angle1 = (angle1 < -0x16c) ? -0x16c : ((angle1 > 0x16c) ? 0x16c : angle1);
    ((PlayerState*)p3)->bodyLeanAngle += angle1 * timeDelta;
    ((PlayerState*)p3)->bodyLeanHalf = (s16)(((PlayerState*)p3)->bodyLeanAngle / 2);

    angle2 = ((PlayerState*)p3)->targetYaw - (u16)((PlayerState*)p3)->prevTargetYaw;
    if (angle2 > 0x8000)
    {
        angle2 = angle2 - 0xFFFF;
    }
    if (angle2 < -0x8000)
    {
        angle2 = angle2 + 0xFFFF;
    }
    if (((ByteFlags*)((char*)p3 + 0x3f1))->b20)
    {
        angle2 = 0;
    }
    {
        f32 f2 = lbl_803E7E98 * (((PlayerState*)p2)->baddie.animSpeedC - lbl_803E7E9C) + lbl_803E7EE0;
        if (f2 < lbl_803E7EA4)
        {
            f2 = lbl_803E7EA4;
        }
        angle2 = (int)((f32)angle2 * (lbl_803E7FC4 * f2));
    }
    angle2 = (angle2 < -0xccc) ? -0xccc : ((angle2 > 0xccc) ? 0xccc : angle2);
    angle2 -= (u16)((PlayerState*)p3)->headPitch;
    if (angle2 > 0x8000)
    {
        angle2 = angle2 - 0xFFFF;
    }
    if (angle2 < -0x8000)
    {
        angle2 = angle2 + 0xFFFF;
    }
    ((PlayerState*)p3)->headPitch =
        (f32)((PlayerState*)p3)->headPitch + interpolate((f32)angle2, lbl_803E7EB4, timeDelta);
    ((PlayerState*)p3)->headYaw = (f32)((PlayerState*)p3)->headYaw * powfBitEstimate(lbl_803E7F1C, timeDelta);
}

int fn_802AC7DC(int obj, int state, int inner, f32 fv)
{
    int r;
    int ok;
    IntPair2 camp;
    MatrixTransform pos;
    u8 buf[52];
    f32 mtx[16];
    f32 dummy;

    camp = *(IntPair2*)&lbl_803E7E70;
    if (((PlayerState*)inner)->curAnimId != 0x48 && ((PlayerState*)inner)->curAnimId != 0x47 &&
        !((ByteFlags*)((char*)inner + 0x3f0))->b04 && !((ByteFlags*)((char*)inner + 0x3f0))->b08 &&
        ((PlayerState*)inner)->heldObj == NULL && !((ByteFlags*)((char*)inner + 0x3f0))->b02 &&
        ((PlayerState*)inner)->baddie.targetObj == NULL && !((ByteFlags*)((char*)inner + 0x3f6))->b40 &&
        ((PlayerState*)inner)->baddie.controlMode != 0x26)
    {
        ok = 1;
    }
    else
    {
        ok = 0;
    }
    if (ok != 0 && (((PlayerState*)inner)->buttonsHeld & PAD_TRIGGER_L) != 0 && getCurSeqNo() == 0)
    {
        if (!((ByteFlags*)((char*)inner + 0x3f1))->b20 && !((ByteFlags*)((char*)inner + 0x3f0))->b10)
        {
            f32 b;
            f32 a;
            a = ((PlayerState*)state)->baddie.animSpeedB;
            b = ((PlayerState*)state)->baddie.animSpeedA;
            pos.rotX = ((PlayerState*)inner)->yaw;
            pos.rotY = 0;
            pos.rotZ = 0;
            pos.scale = lbl_803E7EE0;
            pos.x = lbl_803E7EA4;
            pos.y = lbl_803E7EA4;
            pos.z = lbl_803E7EA4;
            setMatrixFromObjectPos(mtx, &pos);
            Matrix_TransformPoint(mtx, a, lbl_803E7EA4, -b, (f32*)((char*)inner + 0x4c8), &dummy,
                                  (f32*)((char*)inner + 0x4cc));
            ((ByteFlags*)((char*)inner + 0x3f0))->b80 = 0;
            ((ByteFlags*)((char*)inner + 0x3f0))->b40 = 0;
            ((ByteFlags*)((char*)inner + 0x3f0))->b10 = 0;
            ((ByteFlags*)((char*)inner + 0x3f1))->b08 = 1;
            {
                s16 v = ((PlayerState*)inner)->targetYaw;
                ((PlayerState*)inner)->yaw = v;
                ((GameObject*)obj)->anim.rotX = v;
            }
            ((ByteFlags*)((char*)inner + 0x3f1))->b20 = 1;
            {
                f32 z = lbl_803E7EA4;
                ((PlayerState*)inner)->aimInputZ = z;
                ((PlayerState*)inner)->aimInputX = z;
            }
        }
        if (!((ByteFlags*)((char*)inner + 0x3f1))->b10)
        {
            cameraSetInterpMode(2);
            (*gCameraInterface)->setMode(0x52, 1, 0, 8, &camp, 0x1e, 0xff);
            if (gPlayerFrameCounter - gPlayerLastSfxFrame > 2)
            {
                Sfx_PlayFromObject(obj, SFXTRIG_headcam_in);
            }
            gPlayerLastSfxFrame = gPlayerFrameCounter;
            ((ByteFlags*)((char*)inner + 0x3f1))->b10 = 1;
        }
    }
    else
    {
        if (((ByteFlags*)((char*)inner + 0x3f1))->b20)
        {
            s16 v = ((GameObject*)obj)->anim.rotX;
            ((PlayerState*)inner)->yaw = v;
            ((PlayerState*)inner)->targetYaw = v;
            ((PlayerState*)inner)->lastInputHeading = v;
            ((PlayerState*)inner)->baddie.animSpeedB = lbl_803E7EA4;
        }
        ((ByteFlags*)((char*)inner + 0x3f1))->b20 = 0;
        if (((ByteFlags*)((char*)inner + 0x3f1))->b10 && ((PlayerState*)inner)->curAnimId != 0x48 &&
            ((PlayerState*)inner)->curAnimId != 0x47 && getCurSeqNo() == 0)
        {
            (*gCameraInterface)->setMode(0x42, 0, 1, 0, NULL, 0x1e, 0xff);
            ((ByteFlags*)((char*)inner + 0x3f1))->b10 = 0;
        }
    }
    gPlayerFrameCounter = gPlayerFrameCounter + 1;
    if (!((ByteFlags*)((char*)inner + 0x3f0))->b20 && ((PlayerState*)inner)->waterDepth > lbl_803E7FA0 &&
        *(f32*)((char*)state + 0x1b0) < lbl_803E80FC)
    {
        fn_802AE83C(obj, inner, state);
        return 0;
    }
    {
        if (!((ByteFlags*)((char*)inner + 0x3f0))->b20 && !((ByteFlags*)((char*)inner + 0x3f0))->b08 &&
            !((ByteFlags*)((char*)inner + 0x3f0))->b04)
        {
            if (((ByteFlags*)((char*)inner + 0x3f1))->b01 || *(f32*)((char*)state + 0x1b0) < lbl_803E7F58)
            {
                ((PlayerState*)inner)->staffHoldFrames = 0;
            }
            else
            {
                ((PlayerState*)inner)->staffHoldFrames += 1;
            }
            ((PlayerState*)inner)->staffHoldFrames =
                (((PlayerState*)inner)->staffHoldFrames > 10) ? 10 : ((PlayerState*)inner)->staffHoldFrames;
            if (((PlayerState*)inner)->staffHoldFrames > 2)
            {
                ((ByteFlags*)((char*)inner + 0x3f0))->b80 = 0;
                ((ByteFlags*)((char*)inner + 0x3f0))->b10 = 0;
                ((ByteFlags*)((char*)inner + 0x3f0))->b08 = 0;
                staffFn_80170380(gPlayerStaffObject, 2);
                ((ByteFlags*)((char*)inner + 0x3f0))->b02 = 0;
                *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
                ObjHits_SyncObjectPositionIfDirty((GameObject*)obj);
                ((ByteFlags*)((char*)inner + 0x3f0))->b40 = 0;
                ((ByteFlags*)((char*)inner + 0x3f0))->b04 = 1;
                ((ByteFlags*)((char*)inner + 0x3f4))->b10 = 0;
                ((PlayerState*)inner)->isHoldingObject = 0;
                if (((PlayerState*)inner)->heldObj != NULL)
                {
                    s16 t = ((GameObject*)((PlayerState*)inner)->heldObj)->anim.seqId;
                    if (t == 0x3cf || t == 0x662)
                    {
                        objThrowFn_80182504((GameObject*)(((PlayerState*)inner)->heldObj));
                    }
                    else
                    {
                        objSaveFn_800ea774((GameObject*)((PlayerState*)inner)->heldObj);
                    }
                    *(s16*)((char*)((PlayerState*)inner)->heldObj + 0x6) =
                        *(s16*)((char*)((PlayerState*)inner)->heldObj + 0x6) & ~0x4000;
                    *(int*)((char*)((PlayerState*)inner)->heldObj + 0xf8) = 0;
                    ((PlayerState*)inner)->heldObj = 0;
                }
                *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
                return 3;
            }
        }
        if (!((ByteFlags*)((char*)inner + 0x3f0))->b20 && lbl_803E7EA4 != ((PlayerState*)inner)->verticalVel)
        {
            *(int*)&((PlayerState*)state)->baddie.unk308 = 0;
            return 0x42;
        }
        if (!((ByteFlags*)((char*)inner + 0x3f0))->b20 && !((ByteFlags*)((char*)inner + 0x3f0))->b08 &&
            !((ByteFlags*)((char*)inner + 0x3f0))->b04 && ((PlayerState*)inner)->baddie.targetObj == NULL &&
            !((ByteFlags*)((char*)inner + 0x3f6))->b40 && ((PlayerState*)inner)->baddie.controlMode != 0x26)
        {
            ok = 1;
        }
        else
        {
            ok = 0;
        }
        if (ok != 0 && ((PlayerState*)inner)->heldObj != NULL && ((PlayerState*)inner)->isHoldingObject == 0)
        {
            if ((*(int*)((char*)state + 0x310) & 0x4000) != 0)
            {
                *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A49A8;
                return 7;
            }
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A49A8;
            return 8;
        }
        if (!((ByteFlags*)((char*)inner + 0x3f0))->b20 && !((ByteFlags*)((char*)inner + 0x3f0))->b08 &&
            !((ByteFlags*)((char*)inner + 0x3f0))->b04 && !((ByteFlags*)((char*)inner + 0x3f0))->b02 &&
            ((PlayerState*)inner)->baddie.targetObj == NULL && !((ByteFlags*)((char*)inner + 0x3f6))->b40 &&
            ((PlayerState*)inner)->baddie.controlMode != 0x26)
        {
            ok = 1;
        }
        else
        {
            ok = 0;
        }
        if (ok != 0)
        {
            r = playerState08((GameObject*)obj, state, fv);
            if (r != 0)
            {
                return r;
            }
        }
        if (((PlayerState*)state)->baddie.targetObj != NULL)
        {
            s16 t = ((PlayerState*)state)->baddie.controlMode;
            if (t != 0x24 && t != 0x25 && t != 0x26 && !((ByteFlags*)((char*)inner + 0x3f6))->b20 &&
                *(u8*)&((PlayerState*)state)->baddie.hasTarget == 1)
            {
                *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029C8C8;
                return 0x25;
            }
        }
        {
            int btn = padGetTriggers(0);
            if ((btn & 0x20) != 0)
            {
                if (((ByteFlags*)((char*)inner + 0x3f4))->b40 && !((ByteFlags*)((char*)inner + 0x3f0))->b20 &&
                    !((ByteFlags*)((char*)inner + 0x3f0))->b08 && !((ByteFlags*)((char*)inner + 0x3f0))->b04 &&
                    ((PlayerState*)inner)->curAnimId != 0x44 && ((PlayerState*)inner)->heldObj == NULL &&
                    ((PlayerState*)inner)->baddie.targetObj == NULL && !((ByteFlags*)((char*)inner + 0x3f6))->b40 &&
                    ((PlayerState*)inner)->baddie.controlMode != 0x26 &&
                    (((GameObject*)obj)->objectFlags & OBJECT_OBJFLAG_PARENT_SLACK) == 0 &&
                    ((PlayerState*)inner)->idleDelayTimer == lbl_803E7EA4)
                {
                    ok = 1;
                }
                else
                {
                    ok = 0;
                }
                if (ok != 0 && !((ByteFlags*)((char*)inner + 0x3f0))->b02)
                {
                    staffFn_80170380(gPlayerStaffObject, 1);
                    ObjAnim_SetCurrentMove(obj, 0x4f, ((GameObject*)obj)->anim.currentMoveProgress, 0);
                    ObjAnim_SetCurrentEventStepFrames((ObjAnimComponent*)obj, 8);
                    if (gPlayerPathObject != NULL && ((ByteFlags*)((char*)inner + 0x3f4))->b40)
                    {
                        ((PlayerState*)inner)->staffActionRequest = 4;
                        ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
                    }
                    ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7EF8;
                    ((ByteFlags*)((char*)inner + 0x3f0))->b10 = 0;
                    ((ByteFlags*)((char*)inner + 0x3f0))->b40 = 0;
                    ((ByteFlags*)((char*)inner + 0x3f0))->b80 = 0;
                    ((ByteFlags*)((char*)inner + 0x3f0))->b08 = 0;
                    ((ByteFlags*)((char*)inner + 0x3f0))->b04 = 0;
                    ((PlayerState*)inner)->staffHoldFrames = 0;
                    ((ByteFlags*)((char*)inner + 0x3f0))->b02 = 1;
                    ((PlayerState*)inner)->isHoldingObject = 0;
                    if (((PlayerState*)inner)->heldObj != NULL)
                    {
                        s16 t = ((GameObject*)((PlayerState*)inner)->heldObj)->anim.seqId;
                        if (t == 0x3cf || t == 0x662)
                        {
                            objThrowFn_80182504((GameObject*)(((PlayerState*)inner)->heldObj));
                        }
                        else
                        {
                            objSaveFn_800ea774((GameObject*)((PlayerState*)inner)->heldObj);
                        }
                        *(s16*)((char*)((PlayerState*)inner)->heldObj + 0x6) =
                            *(s16*)((char*)((PlayerState*)inner)->heldObj + 0x6) & ~0x4000;
                        *(int*)((char*)((PlayerState*)inner)->heldObj + 0xf8) = 0;
                        ((PlayerState*)inner)->heldObj = 0;
                    }
                    ObjHits_MarkObjectPositionDirty((ObjAnimComponent*)obj);
                    *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
                    return 3;
                }
            }
        }
        if (((ByteFlags*)((char*)inner + 0x3f0))->b08 || ((ByteFlags*)((char*)inner + 0x3f0))->b04)
        {
            r = playerCheckIfClimbingOntoWall(obj, inner, state, buf, fv, 0x14);
            if (r == 0xc)
            {
                *(int*)&((PlayerState*)state)->baddie.unk308 = 0;
                return 10;
            }
            if (r == 9)
            {
                f32 mid;
                f32 lo;
                f32 hi = ((PlayerState*)inner)->spanTopY - lbl_803E7F10;
                mid = lbl_803E8100 + ((GameObject*)obj)->anim.localPosY;
                lo = lbl_803E7F30 + ((PlayerState*)inner)->spanBottomY;
                if (mid >= lo && mid <= hi)
                {
                    doRumble(lbl_803E7ED8);
                    *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029FFD0;
                    return 0x12;
                }
            }
        }
        if (((ByteFlags*)((char*)inner + 0x3f0))->b20)
        {
            r = playerCheckIfClimbingOntoWall(obj, inner, state, buf, lbl_803E7EE0, 0x100);
            if (r == 5)
            {
                gPlayerCurrentMoveId = -1;
                *(int*)&((PlayerState*)state)->baddie.unk308 = 0;
                return 0xc;
            }
            if (((PlayerState*)inner)->waterDepth < lbl_803E7FC0 && ((ByteFlags*)((char*)inner + 0x3f1))->b01)
            {
                ((ByteFlags*)((char*)inner + 0x3f0))->b20 = 0;
            }
        }
        return 0;
    }
}

void playerSetMovingAnims(int p1, int obj)
{
    char* t = (char*)lbl_80332EC0;
    *(int*)((char*)obj + 0x3fc) = *(int*)((char*)obj + 0x3f8);
    if (((ByteFlags*)((char*)obj + 0x3f0))->b20)
    {
        if (((ByteFlags*)((char*)obj + 0x3f1))->b20)
        {
            *(int*)((char*)obj + 0x3f8) = (int)(t + 0x310);
            *(int*)((char*)obj + 0x400) = (int)(t + 0xd8);
        }
        else
        {
            *(int*)((char*)obj + 0x3f8) = (int)(t + 0x210);
            *(int*)((char*)obj + 0x400) = (int)(t + 0xd8);
        }
    }
    else if (*(void**)((char*)obj + 0x7f8) != NULL)
    {
        *(int*)((char*)obj + 0x3f8) = (int)(t + 0x250);
        *(int*)((char*)obj + 0x400) = (int)(t + 0x390);
    }
    else if (((ByteFlags*)((char*)obj + 0x3f1))->b20)
    {
        if (*(u8*)((char*)obj + 0x8b3) != 0)
        {
            *(int*)((char*)obj + 0x3f8) = (int)(t + 0x290);
            *(int*)((char*)obj + 0x400) = (int)(t + 0x390);
        }
        else
        {
            *(int*)((char*)obj + 0x3f8) = (int)(t + 0x2d0);
            *(int*)((char*)obj + 0x400) = (int)(t + 0x390);
        }
    }
    else if (*(u8*)((char*)obj + 0x8b3) != 0)
    {
        *(int*)((char*)obj + 0x3f8) = (int)(t + 0x1d0);
        *(int*)((char*)obj + 0x400) = (int)(t + 0x390);
    }
    else
    {
        *(int*)((char*)obj + 0x3f8) = (int)(t + 0x190);
        *(int*)((char*)obj + 0x400) = (int)(t + 0x390);
    }
}

int fn_802AD2F4(GameObject* obj, int inner, int state)
{
    f32 hdiff;
    int sfx;
    f32 v[6];
    char* p35c;
    PlayerState* ps;
    obj->anim.velocityY = -((lbl_803E7EFC * timeDelta) - obj->anim.velocityY);
    p35c = ((char*)inner) + 0x35c;
    switch (obj->anim.currentMove)
    {
    case 0xa:

    case 0x54:

    case 0x90:
        ((PlayerState*)inner)->emissionState = 2;
        break;

    case 0x13:
    {
        f32 zz = lbl_803E7EA4;
        ((PlayerState*)state)->baddie.animSpeedB = zz;
        obj->anim.velocityY = zz;
    }
        if (obj->anim.currentMoveProgress >= (lbl_803E7F10 * ((PlayerState*)state)->baddie.moveSpeed))
        {
            ((ByteFlags*)(((char*)inner) + 0x3f2))->b08 = 0;
        }
        else if ((((PlayerState*)inner)->fallSeverity >= 2) && (((ByteFlags*)(((char*)inner) + 0x3f2))->b04 == 0))
        {
            s8 hv;
            Camera_EnableViewYOffset();
            CameraShake_SetAllMagnitudes(lbl_803E7ED8);
            ObjPath_GetPointWorldPosition((GameObject*)obj, 0xb, &v[3], &v[4], &v[5], 0);
            if (((PlayerState*)inner)->surfaceType == 0x1a)
            {
                hv = 0x14;
            }
            else
            {
                hv = 2;
            }
            ObjHits_RecordPositionHit(obj, NULL, (int)hv, 1, 0, v[3], v[4], v[5]);
            ((ByteFlags*)(((char*)inner) + 0x3f2))->b04 = 1;
        }
        if ((*((s8*)(&((PlayerState*)state)->baddie.moveDone))) != 0)
        {
            ((ByteFlags*)(((char*)inner) + 0x3f0))->b04 = 0;
            ((ByteFlags*)(((char*)inner) + 0x3f3))->b40 = 1;
            ((PlayerState*)inner)->staffHoldFrames = 0;
            return 1;
        }
        if (((PlayerState*)inner)->fallSeverity >= 2)
        {
            ((PlayerState*)inner)->emissionState = 4;
        }
        else
        {
            ((PlayerState*)inner)->emissionState = 3;
        }
        break;

    case 0xb:
    {
        f32 zz = lbl_803E7EA4;
        ((PlayerState*)state)->baddie.animSpeedB = zz;
        if ((*((s8*)(&((PlayerState*)state)->baddie.moveDone))) != 0)
        {
            if ((*(*((s8**)p35c))) > 0)
            {
                ObjAnim_SetCurrentMove((int)obj, 0xc, zz, 0);
                ((PlayerState*)state)->baddie.moveSpeed = lbl_803E8038;
            }
            else
            {
                ((ByteFlags*)(((char*)inner) + 0x3f0))->b04 = 0;
                ((PlayerState*)inner)->staffHoldFrames = 0;
                playerDie(obj);
            }
        }
        (*gPlayerInterface)->updateAnimRootMotion(obj, (void*)state, timeDelta, 2);
        ((PlayerState*)inner)->emissionState = 4;
        break;
    }

    case 0xc:
        if ((((*((int*)(&((PlayerState*)state)->baddie.eventFlags))) & 1) != 0) &&
            (((PlayerState*)inner)->characterId != 0))
        {
            Sfx_PlayFromObject((int)obj, SFXTRIG_fox_bigfallgrunt2);
            Sfx_PlayFromObject((int)obj, SFXTRIG_foot_ladder2);
        }
        if ((*((s8*)(&((PlayerState*)state)->baddie.moveDone))) != 0)
        {
            ((ByteFlags*)(((char*)inner) + 0x3f0))->b04 = 0;
            ((ByteFlags*)(((char*)inner) + 0x3f3))->b40 = 1;
            ((PlayerState*)inner)->staffHoldFrames = 0;
            return 1;
        }
        (*gPlayerInterface)->updateAnimRootMotion(obj, (void*)state, timeDelta, 2);
        ((PlayerState*)inner)->emissionState = 4;
        break;

    default:
        ObjAnim_SetCurrentMove((int)obj, 0x54, lbl_803E7EA4, 0);
        ObjAnim_SetCurrentEventStepFrames((ObjAnimComponent*)obj, 0x14);
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F6C;
        ((PlayerState*)inner)->emissionState = 2;
        ((PlayerState*)inner)->fallSeverity = 0;
        ((ByteFlags*)(((char*)inner) + 0x3f0))->b01 = 0;
        ((ByteFlags*)(((char*)inner) + 0x3f2))->b08 = 0;
        ((ByteFlags*)(((char*)inner) + 0x3f2))->b04 = 0;
        ((ByteFlags*)(((char*)inner) + 0x3f2))->b02 = 0;
        ((PlayerState*)inner)->prevWorldPosY = obj->anim.worldPosY;
        break;
    }

    ps = (PlayerState*)inner;
    hdiff = ((PlayerState*)inner)->prevWorldPosY - obj->anim.worldPosY;
    if ((((ByteFlags*)(((char*)inner) + 0x3f1))->b01 != 0) && (((ByteFlags*)(((char*)inner) + 0x3f0))->b01 == 0))
    {
        ((ByteFlags*)(((char*)inner) + 0x3f0))->b01 = 1;
        sfx = audioPickSoundEffect_8006ed24(ps->surfaceType, ps->footstepSoundId);
        if (hdiff > lbl_803E8104)
        {
            s8 hv;
            doRumble(lbl_803E7FA4);
            Camera_EnableViewYOffset();
            CameraShake_SetAllMagnitudes(lbl_803E7F58);
            ObjAnim_SetCurrentMove((int)obj, 0xb, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F34;
            Sfx_PlayFromObject((int)obj, SFXTRIG_foot_crawl2);
            Sfx_PlayFromObject((int)obj, SFXTRIG_watery_bubble);
            ObjPath_GetPointWorldPosition((GameObject*)obj, 0xb, &v[3], &v[4], &v[5], 0);
            if (ps->surfaceType == 0x1a)
            {
                hv = 0x14;
            }
            else
            {
                hv = 2;
            }
            ObjHits_RecordPositionHit(obj, NULL, (int)hv, 2, 0, v[3], v[4], v[5]);
            ((ByteFlags*)(((char*)inner) + 0x3f2))->b08 = 0;
            if (ps->waterDepth > lbl_803E7FC4)
            {
                Sfx_PlayFromObject((int)obj, SFXTRIG_foot_run_jingle3);
            }
        }
        else if (hdiff > lbl_803E8108)
        {
            doRumble(lbl_803E7ED8);
            ObjAnim_SetCurrentMove((int)obj, 0x13, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E800C;
            Sfx_PlayFromObject((int)obj, sfx);
            Sfx_StopFromObject((int)obj,
                               (u16)((ps->characterId == 0) ? (SFXTRIG_jump2) : (SFXTRIG_sa_climb02)));
            ((ByteFlags*)(((char*)inner) + 0x3f2))->b08 = 1;
            if (ps->waterDepth > lbl_803E7FC4)
            {
                Sfx_PlayFromObject((int)obj, SFXTRIG_foot_run_jingle3_429);
            }
        }
        else if (hdiff > lbl_803E810C)
        {
            doRumble(lbl_803E7ED8);
            ObjAnim_SetCurrentMove((int)obj, 0x13, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E800C;
            Sfx_PlayFromObject((int)obj, sfx);
            Sfx_PlayFromObject(
                (int)obj, (u16)((((PlayerState*)inner)->characterId == 0) ? (SFXTRIG_panting2) : (SFXTRIG_sa_jump03_var)));
            ((ByteFlags*)(((char*)inner) + 0x3f2))->b08 = 1;
            if (((PlayerState*)inner)->waterDepth > lbl_803E7FC4)
            {
                Sfx_PlayFromObject((int)obj, SFXTRIG_foot_run_jingle3_42a);
            }
        }
        else
        {
            doRumble(lbl_803E7F10);
            Sfx_PlayFromObject(0, sfx);
            ((ByteFlags*)(((char*)inner) + 0x3f0))->b04 = 0;
            ((PlayerState*)inner)->staffHoldFrames = 0;
            ((ByteFlags*)(((char*)inner) + 0x3f1))->b08 = 1;
            ((ByteFlags*)(((char*)inner) + 0x3f2))->b10 = 1;
            ((ByteFlags*)(((char*)inner) + 0x3f2))->b08 = 1;
            if (((PlayerState*)inner)->waterDepth > lbl_803E7FC4)
            {
                Sfx_PlayFromObject((int)obj, SFXTRIG_foot_run_jingle3_42b);
            }
        }
        if (hdiff > lbl_803E810C)
        {
            f32 z2 = lbl_803E7EA4;
            ((PlayerState*)state)->baddie.animSpeedC = z2;
            ((PlayerState*)state)->baddie.animSpeedA = z2;
        }
        ((PlayerState*)state)->baddie.animSpeedB = lbl_803E7EA4;
    }
    if (((ByteFlags*)(((char*)inner) + 0x3f0))->b01 == 0)
    {
        if ((*((f32*)(((char*)state) + 0x1b0))) < lbl_803E80C4)
        {
            ((ByteFlags*)(((char*)inner) + 0x3f2))->b08 = 1;
        }
        if ((hdiff > lbl_803E8104) && (ps->fallSeverity < 3))
        {
            ObjAnim_SetCurrentMove((int)obj, 0xa, lbl_803E7EA4, 0);
            ObjAnim_SetCurrentEventStepFrames((ObjAnimComponent*)obj, 0x19);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7EF8;
            ps->fallSeverity = 3;
            ((ByteFlags*)(((char*)inner) + 0x3f2))->b08 = 0;
        }
        else if ((hdiff > lbl_803E8108) && (ps->fallSeverity < 2))
        {
            if (Sfx_IsPlayingFromObject(
                    0, (u16)((((PlayerState*)inner)->characterId == 0) ? (SFXTRIG_jump2) : (SFXTRIG_sa_climb02))) == 0)
            {
                Sfx_PlayFromObject((int)obj, (u16)((ps->characterId == 0) ? (SFXTRIG_jump2) : (SFXTRIG_sa_climb02)));
            }
            ((PlayerState*)inner)->fallSeverity = 2;
        }
        else if ((hdiff > lbl_803E810C) && (((PlayerState*)inner)->fallSeverity < 1))
        {
            ObjAnim_SetCurrentMove((int)obj, 0x90, lbl_803E7EA4, 0);
            ObjAnim_SetCurrentEventStepFrames((ObjAnimComponent*)obj, 0x19);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7EFC;
            ((PlayerState*)inner)->fallSeverity = 1;
        }
    }
    if ((((ByteFlags*)(((char*)inner) + 0x3f2))->b08 != 0) &&
        ((((PlayerState*)inner)->buttonsJustPressed & 0x400) != 0))
    {
        ((ByteFlags*)(((char*)inner) + 0x3f2))->b02 = 1;
        ((PlayerState*)inner)->buttonsJustPressed = ps->buttonsJustPressed & (~0x400);
    }
    if (((((ByteFlags*)(((char*)inner) + 0x3f0))->b01 != 0) && (((ByteFlags*)(((char*)inner) + 0x3f2))->b02 != 0)) &&
        (ps->fallSeverity < 3))
    {
        fn_802AED2C(obj, inner, state);
        ((ByteFlags*)(((char*)inner) + 0x3f0))->b04 = 0;
        ps->staffHoldFrames = 0;
    }
    if ((ps->fallSeverity == 0) && (((ByteFlags*)(((char*)inner) + 0x3f4))->b10 == 0))
    {
        f32 b;
        f32 a;
        f32 c;
        ps->targetYawSmoothRate = (a = lbl_803E7FBC);
        ps->targetYawRateLimit = (b = lbl_803E7E98);
        ps->yawSmoothRate = a;
        ps->yawRateLimit = b;
        c = lbl_803E7F14;
        ps->targetAnimSpeed = c;
        ps->currentSpeed = ps->currentSpeed * c;
    }
    else
    {
        f32 a;
        f32 b;
        ps->targetYawSmoothRate = (a = lbl_803E7FBC);
        ps->targetYawRateLimit = (b = lbl_803E7EA4);
        ps->yawSmoothRate = a;
        ps->yawRateLimit = b;
        ps->targetAnimSpeed = b;
        ps->currentSpeed = ps->currentSpeed * b;
    }
    ps->currentSpeed = (ps->currentSpeed < lbl_803E8110)
                           ? (lbl_803E8110)
                           : ((ps->currentSpeed > ps->maxSpeed) ? (ps->maxSpeed) : (ps->currentSpeed));
    if (ps->curAnimId == 0x4b)
    {
        (*gCameraInterface)->setMode(0x42, 0, 1, 0, (void*)0, 0, 0xff);
        ps->curAnimId = 0x42;
    }
    return 0;
}

int fn_802ADC08(GameObject* obj, int inner, int p3)
{
    obj->anim.velocityY = obj->anim.velocityY - lbl_803DC67C * timeDelta;
    if (((PlayerState*)inner)->fallFrames > 5 && ((ByteFlags*)((char*)inner + 0x3f1))->b01)
    {
        u16 snd;
        doRumble(lbl_803E7F10);
        Sfx_PlayFromObject((int)obj, (u16)audioPickSoundEffect_8006ed24(((PlayerState*)inner)->surfaceType,
                                                                       ((PlayerState*)inner)->footstepSoundId));
        if (((PlayerState*)inner)->characterId == 0)
        {
            snd = 0x2cf;
        }
        else
        {
            snd = 0x25;
        }
        Sfx_PlayFromObject((int)obj, snd);
        ((ByteFlags*)((char*)inner + 0x3f0))->b08 = 0;
        ((ByteFlags*)((char*)inner + 0x3f1))->b08 = 1;
        ((ByteFlags*)((char*)inner + 0x3f2))->b10 = 1;
    }
    if (obj->anim.worldPosY <= ((PlayerState*)inner)->fallThresholdY ||
        ((*(s8*)((char*)p3 + 0x264) & 2) && (*(s8*)((char*)p3 + 0x264) & 0x20) == 0) || *(u8*)((char*)p3 + 0x262) != 0)
    {
        void* sub;
        ((ByteFlags*)((char*)inner + 0x3f0))->b80 = 0;
        ((ByteFlags*)((char*)inner + 0x3f0))->b10 = 0;
        ((ByteFlags*)((char*)inner + 0x3f0))->b08 = 0;
        staffFn_80170380(gPlayerStaffObject, 2);
        ((ByteFlags*)((char*)inner + 0x3f0))->b02 = 0;
        *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
        ObjHits_SyncObjectPositionIfDirty(obj);
        ((ByteFlags*)((char*)inner + 0x3f0))->b40 = 0;
        ((ByteFlags*)((char*)inner + 0x3f0))->b04 = 1;
        ((ByteFlags*)((char*)inner + 0x3f4))->b10 = 0;
        ((PlayerState*)inner)->isHoldingObject = 0;
        sub = ((PlayerState*)inner)->heldObj;
        if (sub != NULL)
        {
            s16 id = ((GameObject*)sub)->anim.seqId;
            if (id == 0x3cf || id == 0x662)
            {
                objThrowFn_80182504((GameObject*)sub);
            }
            else
            {
                objSaveFn_800ea774((GameObject*)sub);
            }
            *(s16*)((char*)((PlayerState*)inner)->heldObj + 0x6) &= ~0x4000;
            *(int*)((char*)((PlayerState*)inner)->heldObj + 0xf8) = 0;
            ((PlayerState*)inner)->heldObj = 0;
        }
    }
    ((PlayerState*)inner)->fallFrames += 1;
    {
        u32 v = ((PlayerState*)inner)->fallFrames;
        if (v > 0xa)
            v = 0xa;
        ((PlayerState*)inner)->fallFrames = v;
    }
    ((PlayerState*)inner)->emissionState = 1;
    {
        f32 f4, c4;
        ((PlayerState*)inner)->targetYawSmoothRate = (c4 = lbl_803E80C4);
        ((PlayerState*)inner)->targetYawRateLimit = (f4 = lbl_803E7FF4);
        ((PlayerState*)inner)->yawSmoothRate = c4;
        ((PlayerState*)inner)->yawRateLimit = f4;
    }
    ((PlayerState*)inner)->targetAnimSpeed = lbl_803DC684;
    {
        ((PlayerState*)inner)->currentSpeed =
            (((PlayerState*)inner)->currentSpeed < lbl_803E7EA4)
                ? lbl_803E7EA4
                : ((((PlayerState*)inner)->currentSpeed > ((PlayerState*)inner)->maxSpeed)
                       ? ((PlayerState*)inner)->maxSpeed
                       : ((PlayerState*)inner)->currentSpeed);
    }
    return 0;
}

void fn_802ADE80(GameObject* obj, int inner, int state)
{
    f32 t[3];
    f32 waterX;
    f32 waterZ;
    MatrixTransform v;
    struct
    {
        u8 pad[6];
        u16 mode;
        f32 scale;
        f32 x;
        f32 y;
        f32 z;
    } pfx;
    f32 mtx[16];
    f32 angle;
    f32 d;
    f32 accel;
    f32 vel;
    f32 cosv;
    f32 sinv;
    f32 a;
    int playEffect;
    u8 loopCount;
    int i;

    angle = ((PlayerState*)inner)->waterSurfaceY;
    angle = angle + mathSinf(gPlayerPi * (f32)(u32) * (u16*)((char*)inner + 0x89c) / lbl_803E7F98);
    *(s16*)&((PlayerState*)inner)->unk89C = lbl_803E8114 * timeDelta + (f32)(u32) * (u16*)((char*)inner + 0x89c);
    {
        d = angle - obj->anim.localPosY;
        if (d > 25.0f)
        {
            d = 25.0f;
        }
        accel = d / lbl_803E7FA0;
        accel = accel * lbl_803E8118;
        obj->anim.velocityY = accel * timeDelta + obj->anim.velocityY;
    }
    obj->anim.velocityY = obj->anim.velocityY - 0.1f * timeDelta;
    obj->anim.velocityY = obj->anim.velocityY * powfBitEstimate(lbl_803E7FD0, timeDelta);
    {
        vel = obj->anim.velocityY;
        obj->anim.velocityY = (vel < -4.0f) ? -4.0f : ((vel > 1.4f) ? 1.4f : vel);
    }
    playerCalcWaterCurrent(&waterX, &waterZ, lbl_803E7EE0, (int)obj);
    {
        cosv = mathSinf(gPlayerPi * (f32) ((PlayerState*)inner)->targetYaw / lbl_803E7F98);
        sinv = mathCosf(gPlayerPi * (f32) ((PlayerState*)inner)->targetYaw / lbl_803E7F98);
        a = -waterZ * sinv - waterX * cosv;
        ((PlayerState*)inner)->waterCurrentVelB +=
            timeDelta * (0.1f * ((waterX * sinv - waterZ * cosv) - ((PlayerState*)inner)->waterCurrentVelB));
        ((PlayerState*)inner)->waterCurrentVelA += timeDelta * (0.1f * (a - ((PlayerState*)inner)->waterCurrentVelA));
    }
    playEffect = 0;
    if (((PlayerState*)state)->baddie.controlMode == 1)
    {
        if ((*(int*)&((PlayerState*)state)->baddie.eventFlags & 0x200) != 0)
        {
            Sfx_PlayAtPositionFromObject((int)obj, obj->anim.localPosX,
                                         ((PlayerState*)inner)->waterSurfaceY, obj->anim.localPosZ, 0xe);
        }
        if (((PlayerState*)inner)->waterDepth < lbl_803E7FA0 &&
            (*(int*)&((PlayerState*)state)->baddie.eventFlags & 0x200) != 0)
        {
            t[0] = (f32)randomGetRange(-0x14, 0x14) / lbl_803E7ED8;
            t[2] = (f32)randomGetRange(-0x14, 0x14) / lbl_803E7ED8;
            playEffect = 1;
        }
    }
    else
    {
        if ((*(int*)&((PlayerState*)state)->baddie.eventFlags & 1) != 0)
        {
            Sfx_PlayAtPositionFromObject((int)obj, obj->anim.localPosX,
                                         ((PlayerState*)inner)->waterSurfaceY, obj->anim.localPosZ, 0xf);
        }
        if (((PlayerState*)inner)->waterDepth < lbl_803E7FA0 &&
            (*(int*)&((PlayerState*)state)->baddie.eventFlags & 0x200) != 0)
        {
            s8 c;
            t[0] = (f32)randomGetRange(-0x14, 0x14) / lbl_803E7ED8;
            c = ((PlayerState*)inner)->gaitLevel;
            if (c <= 8)
            {
                t[2] = lbl_803E8124;
            }
            else if (c <= 0xc)
            {
                t[2] = lbl_803E8124;
            }
            else
            {
                t[2] = lbl_803E8124;
            }
            playEffect = 1;
        }
    }
    if (playEffect != 0)
    {
        v.x = obj->anim.localPosX;
        v.y = lbl_803E7EA4;
        v.z = obj->anim.localPosZ;
        v.rotX = ((PlayerState*)inner)->targetYaw;
        v.rotY = 0;
        v.rotZ = 0;
        v.scale = lbl_803E7EE0;
        setMatrixFromObjectPos(mtx, &v);
        Matrix_TransformPoint(mtx, t[0], lbl_803E7EA4, t[2], &t[0], &t[1], &t[2]);
        (*gWaterfxInterface)->spawnRipple(
            t[0], ((PlayerState*)inner)->waterSurfaceY, t[2], 0, lbl_803E7EA4, 5);
        if (((PlayerState*)inner)->waterDepth > lbl_803E8128 && ((PlayerState*)state)->baddie.animSpeedC > lbl_803E7E9C)
        {
            u16 ang = ((PlayerState*)inner)->targetYaw -
                      getAngle(((PlayerState*)state)->baddie.animSpeedB, ((PlayerState*)state)->baddie.animSpeedA);
            (*gWaterfxInterface)->spawnSimpleRipple(
                t[0], ((PlayerState*)inner)->waterSurfaceY, t[2], ang, lbl_803E7EA4);
        }
    }
    ObjPath_GetPointWorldPosition((GameObject*)obj, 0x13, &v.x, &v.y, &v.z, 0);
    loopCount = (((PlayerState*)inner)->waterSurfaceY - v.y > lbl_803E7F10) ? 1 : 0;
    for (i = 0; i < loopCount; i++)
    {
        pfx.x = v.x + (f32)randomGetRange(-0x64, 0x64) / 20.0f;
        pfx.y = v.y + (f32)randomGetRange(-0x64, 0x64) / 50.0f;
        pfx.z = v.z + (f32)randomGetRange(-0x64, 0x64) / 20.0f;
        pfx.scale = ((PlayerState*)inner)->waterSurfaceY - pfx.y;
        if (pfx.scale > 0.0f)
        {
            (*gPartfxInterface)->spawnObject((void*)obj, 0x202, &pfx, 0x200001, -1, NULL);
        }
    }
}

int fn_802AE480(GameObject* obj, int inner, int state)
{
    f32 h;
    f32 lim;

    *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_HEADING_LOCK;
    ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F20;
    h = obj->anim.currentMoveProgress;
    if (h > lbl_803E7EFC && h < lbl_803E7F44 &&
        ((PlayerState*)state)->baddie.animSpeedC >
            *(f32*)((char*)((PlayerState*)inner)->moveParams + 0x1c) - lbl_803E7E9C &&
        ((PlayerState*)state)->baddie.inputMagnitude > lbl_803E7F2C && ((PlayerState*)inner)->yawRateSigned >= 0x96)
    {
        ((ByteFlags*)((char*)inner + 0x3f0))->b40 = 1;
        ((ByteFlags*)((char*)inner + 0x3f0))->b80 = 0;
        ((PlayerState*)inner)->animSoundId = ((PlayerState*)inner)->altAnimSoundId;
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E8070;
        ObjAnim_SetCurrentMove((int)obj, *(s16*)((char*)((PlayerState*)inner)->moveAnimTable + 0x3a), lbl_803E7EA4, 0);
        ObjAnim_SetCurrentEventStepFrames((ObjAnimComponent*)obj, 0x10);
        ((PlayerState*)inner)->unk858 = ((PlayerState*)inner)->yaw;
        ((PlayerState*)inner)->unk844 = (lbl_803E7F14 + (*(f32*)((char*)((PlayerState*)inner)->moveParams + 0x14) +
                                                         ((PlayerState*)state)->baddie.animSpeedC)) /
                                        lbl_803E7F30;
        ((PlayerState*)inner)->targetYaw = ((PlayerState*)inner)->yaw;
        ((PlayerState*)inner)->yaw += 0x8000;
        ((PlayerState*)state)->baddie.animSpeedC = -((PlayerState*)state)->baddie.animSpeedC;
        ((PlayerState*)state)->baddie.animSpeedA = -((PlayerState*)state)->baddie.animSpeedA;
    }
    if (((ByteFlags*)((char*)inner + 0x3f0))->b80)
    {
        if (((PlayerState*)state)->baddie.animSpeedC <=
                (lim = *(f32*)((char*)((PlayerState*)inner)->moveParams + 0x10)) &&
            ((PlayerState*)state)->baddie.animSpeedA <= lim)
        {
            ((PlayerState*)inner)->lastInputHeading = ((PlayerState*)inner)->yaw;
            ((ByteFlags*)((char*)inner + 0x3f0))->b40 = 0;
            ((ByteFlags*)((char*)inner + 0x3f0))->b80 = 0;
            return 1;
        }
        ((PlayerState*)inner)->currentSpeed = lbl_803E7EA4;
        ((PlayerState*)inner)->velSmoothRate = ((PlayerState*)inner)->velSmoothRateBase;
    }
    return 0;
}

void fn_802AE650(GameObject* obj, int state, int p3)
{
    f32 v;
    u32 b;
    f32 ee0;

    (*gPlayerInterface)->updateAnimRootMotion(obj, (void*)p3, timeDelta, 1);
    if (obj->anim.currentMoveProgress >=
        (ee0 = lbl_803E7EE0) - lbl_803E7F50 * ((PlayerState*)p3)->baddie.moveSpeed)
    {
        ((PlayerState*)p3)->baddie.animSpeedA =
            ((PlayerState*)state)->unk844 * ((lbl_803E7F14 + *(f32*)((char*)((PlayerState*)state)->moveParams + 0x14)) -
                                             ((PlayerState*)p3)->baddie.animSpeedA) +
            *(f32*)&((PlayerState*)p3)->baddie.animSpeedA;
        ((PlayerState*)p3)->baddie.animSpeedC = ((PlayerState*)p3)->baddie.animSpeedA;
        ((PlayerState*)state)->unk844 = lbl_803E7EFC * timeDelta + ((PlayerState*)state)->unk844;
        v = ((PlayerState*)state)->unk844;
        ((PlayerState*)state)->unk844 = (v < lbl_803E7EA4) ? lbl_803E7EA4 : ((v > ee0) ? ee0 : v);
    }
    if ((*(int*)&((PlayerState*)p3)->baddie.eventFlags & 0x200) != 0)
    {
        doRumble(lbl_803E7F10);
        Sfx_PlayFromObject((int)obj, SFXTRIG_rserv1_c);
        ((PlayerState*)state)->pendingFxFlags |= 4;
    }
    {
        f32 fa4 = lbl_803E7FA4;
        ((PlayerState*)state)->targetYawSmoothRate = fa4;
        ((PlayerState*)state)->yawSmoothRate = fa4;
    }
    b = (((PlayerState*)state)->flags3F1 >> 4) & 1;
    if (b != 0)
    {
        f32 ea4 = lbl_803E7EA4;
        ((PlayerState*)state)->targetYawRateLimit = ea4;
        ((PlayerState*)state)->yawRateLimit = ea4;
    }
    else
    {
        f32 ed4 = lbl_803E7ED4;
        ((PlayerState*)state)->targetYawRateLimit = ed4;
        ((PlayerState*)state)->yawRateLimit = ed4;
    }
    ((PlayerState*)state)->knockbackDrainRate = lbl_803E80E4;
    if (obj->anim.currentMoveProgress >= lbl_803E7EE0)
    {
        short tmp;
        ((ByteFlags*)((char*)state + 0x3f0))->b10 = 0;
        gPlayerSubState = 1;
        ((ByteFlags*)((char*)state + 0x3f1))->b02 = 1;
        ((ByteFlags*)((char*)state + 0x3f1))->b08 = 1;
        *(u8*)&((PlayerState*)state)->gaitLevel = 0xc;
        tmp = ((PlayerState*)state)->yaw;
        ((PlayerState*)state)->targetYaw = tmp;
        ((PlayerState*)state)->lastInputHeading = tmp;
        ObjAnim_SetCurrentMove((int)obj, gPlayerMoveTableA[(s8) * (u8*)((char*)state + 0x8cc)], lbl_803E7EA4, 0);
        ObjAnim_SetCurrentEventStepFrames((ObjAnimComponent*)obj, 1);
    }
}

void fn_802AE83C(int obj, int inner, int state)
{
    GameObject* sub;
    f32 z;

    ((ByteFlags*)((char*)inner + 0x3f1))->b40 = 0;
    ((ByteFlags*)((char*)inner + 0x3f0))->b40 = 0;
    ((ByteFlags*)((char*)inner + 0x3f0))->b80 = 0;
    ((ByteFlags*)((char*)inner + 0x3f0))->b08 = 0;
    ((ByteFlags*)((char*)inner + 0x3f0))->b04 = 0;
    ((PlayerState*)inner)->staffHoldFrames = 0;
    ((ByteFlags*)((char*)inner + 0x3f0))->b20 = 1;
    ((ByteFlags*)((char*)inner + 0x3f0))->b10 = 0;
    z = lbl_803E7EA4;
    ((PlayerState*)inner)->waterCurrentVelB = z;
    ((PlayerState*)inner)->waterCurrentVelA = z;
    Sfx_StopFromObject(obj,
                       (u16)(((PlayerState*)inner)->characterId == 0 ? SFXTRIG_jump2 : SFXTRIG_sa_climb02));

    if ((void*)gPlayerPathObject != NULL && ((ByteFlags*)((char*)inner + 0x3f4))->b40)
    {
        ((PlayerState*)inner)->staffActionRequest = 1;
        ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
    }
    ((PlayerState*)inner)->isHoldingObject = 0;
    sub = ((PlayerState*)inner)->heldObj;
    if (sub != NULL)
    {
        s16 id = sub->anim.seqId;
        if (id == 0x3cf || id == 0x662)
        {
            objThrowFn_80182504(sub);
        }
        else
        {
            objSaveFn_800ea774(sub);
        }
        *(s16*)((char*)((PlayerState*)inner)->heldObj + 6) &= ~0x4000;
        *(int*)((char*)((PlayerState*)inner)->heldObj + 0xf8) = 0;
        ((PlayerState*)inner)->heldObj = 0;
    }
    if (((GameObject*)obj)->anim.velocityY < lbl_803E812C)
    {
        Sfx_PlayFromObject(obj, SFXTRIG_mv_curtainopen16_212);
        (*gWaterfxInterface)
            ->spawnSplashBurst((void*)obj, ((GameObject*)obj)->anim.localPosX, ((GameObject*)obj)->anim.localPosY,
                               ((GameObject*)obj)->anim.localPosZ, lbl_803E7ED8);
    }
}

void fn_802AE9C8(GameObject* obj, int inner, int state)
{
    if (obj->anim.currentMoveProgress > lbl_803E7E98)
    {
        ObjAnim_SetCurrentMove((int)obj, 0x91, lbl_803E7EA4, 0);
    }
    else
    {
        ObjAnim_SetCurrentMove((int)obj, 0x12, lbl_803E7EA4, 0);
    }
    ObjAnim_SetCurrentEventStepFrames((ObjAnimComponent*)obj, 0xf);

    ((PlayerState*)inner)->maxSpeed = lbl_803E8068;
    ((PlayerState*)inner)->currentSpeed = lbl_803E7EA0 * (lbl_803E806C * ((PlayerState*)state)->baddie.inputMagnitude) +
                                          lbl_803E7EB4 * ((PlayerState*)state)->baddie.animSpeedC;
    ((PlayerState*)inner)->currentSpeed = (((PlayerState*)inner)->currentSpeed < lbl_803E7F18)
                                              ? lbl_803E7F18
                                              : ((((PlayerState*)inner)->currentSpeed > ((PlayerState*)inner)->maxSpeed)
                                                     ? ((PlayerState*)inner)->maxSpeed
                                                     : ((PlayerState*)inner)->currentSpeed);
    {
        f32 a = ((PlayerState*)inner)->currentSpeed;
        ((PlayerState*)state)->baddie.animSpeedA = a;
        ((PlayerState*)state)->baddie.animSpeedC = a;
    }

    obj->anim.velocityY = ((PlayerState*)state)->baddie.animSpeedA / lbl_803E8068;
    {
        f32 v = obj->anim.velocityY;
        f32 clamped;
        if (v < lbl_803E7EA4)
        {
            clamped = lbl_803E7EA4;
        }
        else if (v > lbl_803E7EE0)
        {
            clamped = lbl_803E7EE0;
        }
        else
        {
            clamped = v;
        }
        obj->anim.velocityY = clamped;
    }
    obj->anim.velocityY = obj->anim.velocityY * lbl_803DC680;
    obj->anim.velocityY =
        (obj->anim.velocityY < lbl_803E7E98)
            ? lbl_803E7E98
            : ((obj->anim.velocityY > lbl_803DC680) ? lbl_803DC680 : obj->anim.velocityY);
    ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7EE0 / (lbl_803E7ED4 * lbl_803DC680 / lbl_803DC67C);
    ((PlayerState*)inner)->groundRefY = obj->anim.worldPosY;
    ((PlayerState*)inner)->fallThresholdY = obj->anim.worldPosY - lbl_803E7ED8;

    ((ByteFlags*)((char*)inner + 0x3f0))->b08 = 1;
    ((ByteFlags*)((char*)inner + 0x3f0))->b04 = 0;
    ((PlayerState*)inner)->staffHoldFrames = 0;
    ((ByteFlags*)((char*)inner + 0x3f0))->b10 = 0;
    ((ByteFlags*)((char*)inner + 0x3f0))->b80 = 0;
    staffFn_80170380(gPlayerStaffObject, 2);
    ((ByteFlags*)((char*)inner + 0x3f0))->b02 = 0;
    *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
    ObjHits_SyncObjectPositionIfDirty(obj);
    if (((ByteFlags*)((char*)inner + 0x3f0))->b40)
    {
        ((PlayerState*)inner)->yaw += -0x8000;
    }
    ((ByteFlags*)((char*)inner + 0x3f0))->b40 = 0;
    ((ByteFlags*)((char*)inner + 0x3f1))->b01 = 0;
    ((PlayerState*)inner)->fallFrames = 0;
    if (((ByteFlags*)((char*)inner + 0x3f1))->b20)
    {
        int t = *(s16*)obj;
        ((PlayerState*)inner)->yaw = t;
        ((PlayerState*)inner)->targetYaw = t;
        ((PlayerState*)inner)->lastInputHeading = t;
        ((PlayerState*)inner)->baddie.animSpeedB = lbl_803E7EA4;
    }
    ((ByteFlags*)((char*)inner + 0x3f1))->b20 = 0;
    if (((ByteFlags*)((char*)inner + 0x3f1))->b10 && ((PlayerState*)inner)->curAnimId != 0x48 &&
        ((PlayerState*)inner)->curAnimId != 0x47 && getCurSeqNo() == 0)
    {
        (*gCameraInterface)->setMode(0x42, 0, 1, 0, NULL, 0x1e, 0xff);
        ((ByteFlags*)((char*)inner + 0x3f1))->b10 = 0;
    }
    {
        u16 sfxId;
        if (((PlayerState*)inner)->characterId == 0)
        {
            sfxId = 0x2d7;
        }
        else
        {
            sfxId = 0x2d6;
        }
        Sfx_PlayFromObject((int)obj, sfxId);
    }
    ((PlayerState*)inner)->isHoldingObject = 0;
    {
        void* sub = ((PlayerState*)inner)->heldObj;
        if (sub != NULL)
        {
            s16 id = ((GameObject*)sub)->anim.seqId;
            if (id == 0x3cf || id == 0x662)
            {
                objThrowFn_80182504((GameObject*)sub);
            }
            else
            {
                objSaveFn_800ea774((GameObject*)sub);
            }
            *(s16*)((char*)((PlayerState*)inner)->heldObj + 0x6) &= ~0x4000;
            *(int*)((char*)((PlayerState*)inner)->heldObj + 0xf8) = 0;
            ((PlayerState*)inner)->heldObj = 0;
        }
    }
}

void fn_802AED2C(GameObject* obj, int state, int p3)
{
    u16 sound;
    u32 b;

    if (((PlayerState*)state)->staffGrown != 0)
    {
        ObjAnim_SetCurrentMove((int)obj, 0x47f, lbl_803E7EA4, 0);
    }
    else
    {
        ObjAnim_SetCurrentMove((int)obj, 0x47b, lbl_803E7EA4, 0);
    }
    ((PlayerState*)p3)->baddie.moveSpeed = lbl_803E7F20;
    ((PlayerState*)state)->targetYaw = ((PlayerState*)state)->yaw;
    ((PlayerState*)state)->unk844 = lbl_803E7EA4;
    ((ByteFlags*)((char*)state + 0x3f0))->b10 = 1;
    ((ByteFlags*)((char*)state + 0x3f0))->b80 = 0;
    staffFn_80170380(gPlayerStaffObject, 2);
    ((ByteFlags*)((char*)state + 0x3f0))->b02 = 0;
    *(u32*)&((PlayerState*)state)->flags360 |= PLAYER_FLAG_TELEPORTED;
    ObjHits_SyncObjectPositionIfDirty(obj);
    ((ByteFlags*)((char*)state + 0x3f0))->b08 = 0;
    ((ByteFlags*)((char*)state + 0x3f0))->b04 = 0;
    ((PlayerState*)state)->staffHoldFrames = 0;
    ((ByteFlags*)((char*)state + 0x3f0))->b40 = 0;
    ((PlayerState*)state)->yawRateSigned = 0;
    ((PlayerState*)state)->targetYawRateSigned = 0;
    ((PlayerState*)state)->yawRate = 0;
    ((PlayerState*)state)->targetYawRate = 0;
    gPlayerSubState = 4;
    ((PlayerState*)state)->isHoldingObject = 0;
    if (((PlayerState*)state)->heldObj != NULL)
    {
        short id = ((GameObject*)((PlayerState*)state)->heldObj)->anim.seqId;
        if (id == 0x3cf || id == 0x662)
        {
            objThrowFn_80182504((GameObject*)(((PlayerState*)state)->heldObj));
        }
        else
        {
            objSaveFn_800ea774((GameObject*)((PlayerState*)state)->heldObj);
        }
        *(s16*)((char*)((PlayerState*)state)->heldObj + 6) &= ~0x4000;
        *(int*)((char*)((PlayerState*)state)->heldObj + 0xf8) = 0;
        ((PlayerState*)state)->heldObj = 0;
    }
    b = (((PlayerState*)state)->flags3F1 >> 5) & 1;
    if (b != 0)
    {
        short t = obj->anim.rotX;
        ((PlayerState*)state)->yaw = t;
        ((PlayerState*)state)->targetYaw = t;
        ((PlayerState*)state)->lastInputHeading = t;
        ((PlayerState*)state)->baddie.animSpeedB = lbl_803E7EA4;
    }
    ((ByteFlags*)((char*)state + 0x3f1))->b20 = 0;
    if (((PlayerState*)state)->waterDepth > lbl_803E7EE0)
    {
        if (((PlayerState*)state)->characterId == 0)
        {
            sound = 0x427;
        }
        else
        {
            sound = 0x427;
        }
        Sfx_PlayFromObject((int)obj, sound);
    }
    else
    {
        if (((PlayerState*)state)->characterId == 0)
        {
            sound = 0x3ce;
        }
        else
        {
            sound = 0x2e;
        }
        Sfx_PlayFromObject((int)obj, sound);
    }
}

static inline int staffCanContinueSpin(void* state)
{
    ByteFlags* bf = (ByteFlags*)((char*)state + 0x3f0);
    s16 t;

    if (bf->b10 || bf->b04 || bf->b08 || bf->b20 || ((PlayerState*)state)->baddie.controlMode == 0x36)
    {
        return 0;
    }

    t = ((PlayerState*)state)->baddie.controlMode;
    if ((u16)(t - 1) <= 1 || (u16)(t - 0x24) <= 1 || ((PlayerState*)state)->baddie.targetObj != NULL)
    {
        return 1;
    }
    return 0;
}

void staffAnimate(int obj, void* state, f32 dt)
{
    int prevChanged;
    int changed;
    int model;
    f32 f31;
    void* p;

    model = *(int*)((char*)Obj_GetActiveModel((GameObject*)obj) + 0x30);
    prevChanged = 0;

    if (*(s16*)&((PlayerState*)state)->staffAnimState != 3)
    {
        u8 b = ((PlayerState*)state)->staffActionRequest;
        if (b == 1)
        {
            staffDoGrowShrinkAnim((GameObject*)gPlayerPathObject, 0, ((ByteFlags*)((char*)state + 0x3f4))->b08, 0);
            ((PlayerState*)state)->staffGrown = 0;
            if (*(s16*)&((PlayerState*)state)->staffAnimState != 0 &&
                *(s16*)&((PlayerState*)state)->staffAnimState != 0xf)
            {
                *(s16*)&((PlayerState*)state)->staffAnimState = 3;
            }
        }
        else if (b == 4)
        {
            staffDoGrowShrinkAnim((GameObject*)gPlayerPathObject, 1, ((ByteFlags*)((char*)state + 0x3f4))->b08, 0);
            ((PlayerState*)state)->staffGrown = 1;
            if (*(s16*)&((PlayerState*)state)->staffAnimState != 0 &&
                *(s16*)&((PlayerState*)state)->staffAnimState != 0xf)
            {
                *(s16*)&((PlayerState*)state)->staffAnimState = 3;
            }
        }
    }

    f31 = -lbl_803E7F20;
    do
    {
        changed = 0;
        switch (*(s16*)&((PlayerState*)state)->staffAnimState)
        {
        case 2:
            if (prevChanged != 0)
            {
                Object_ObjAnimSetMove(obj, ((GameObject*)obj)->anim.currentMove,
                                      ((GameObject*)obj)->anim.currentMoveProgress, 0);
                p = *(void**)((char*)state + 0x4b8);
                if (p != NULL && (*(s16*)((char*)p + 0x44) == 0x1c || *(s16*)((char*)p + 0x44) == 0x2a))
                {
                    Object_ObjAnimSetMove(obj, 0x82, lbl_803E7EA4, 0);
                }
                else
                {
                    Object_ObjAnimSetMove(obj, 0x8d, lbl_803E7EA4, 0);
                }
                ObjAnim_SetCurrentEventStepFrames((ObjAnimComponent*)obj, 0xc);
            }
            if (((GameObject*)obj)->anim.activeMoveProgress >= lbl_803E8130)
            {
                ((PlayerState*)state)->staffGrown = 1;
            }
            if (((GameObject*)obj)->anim.activeMoveProgress >= lbl_803E7F1C)
            {
                staffDoGrowShrinkAnim((GameObject*)gPlayerPathObject, 1, 0, 0);
                *(s16*)&((PlayerState*)state)->staffAnimState = 3;
                changed = 1;
            }
            else
            {
                Object_ObjAnimAdvanceMove(obj, lbl_803E7F20, lbl_803E7EE0, NULL);
            }
            break;
        case 1:
            if (prevChanged != 0)
            {
                Object_ObjAnimSetMove(obj, ((GameObject*)obj)->anim.currentMove,
                                      ((GameObject*)obj)->anim.currentMoveProgress, 0);
                p = *(void**)((char*)state + 0x4b8);
                if (p != NULL && (*(s16*)((char*)p + 0x44) == 0x1c || *(s16*)((char*)p + 0x44) == 0x2a))
                {
                    Object_ObjAnimSetMove(obj, 0x82, lbl_803E7F68, 0);
                }
                else
                {
                    Object_ObjAnimSetMove(obj, 0x8d, lbl_803E7F68, 0);
                }
                ObjAnim_SetCurrentEventStepFrames((ObjAnimComponent*)obj, 0xc);
            }
            if (((GameObject*)obj)->anim.activeMoveProgress <= lbl_803E8130)
            {
                ((PlayerState*)state)->staffGrown = 0;
            }
            if (((GameObject*)obj)->anim.activeMoveProgress <= lbl_803E7EB4)
            {
                *(s16*)&((PlayerState*)state)->staffAnimState = 3;
                changed = 1;
            }
            else
            {
                Object_ObjAnimAdvanceMove(obj, f31, lbl_803E7EE0, NULL);
            }
            break;
        case 0xf:
            if (prevChanged != 0)
            {
                Object_ObjAnimSetMove(obj, ((GameObject*)obj)->anim.currentMove,
                                      ((GameObject*)obj)->anim.currentMoveProgress, 0);
                Object_ObjAnimSetMove(obj, lbl_8033366C[((PlayerState*)state)->moveVariantIndex], lbl_803E7EA4, 0);
                ObjAnim_SetCurrentEventStepFrames((ObjAnimComponent*)obj, 0xc);
            }
            if (((GameObject*)obj)->anim.activeMoveProgress >= lbl_803E7EE0 || !staffCanContinueSpin(state))
            {
                *(s16*)&((PlayerState*)state)->staffAnimState = 3;
                ((PlayerState*)state)->moveVariantIndex = 0xff;
                changed = 1;
            }
            else
            {
                Object_ObjAnimAdvanceMove(obj, lbl_8033369C[((PlayerState*)state)->moveVariantIndex], timeDelta, NULL);
            }
            break;
        case 3:
            if (((GameObject*)obj)->anim.activeMove != ((GameObject*)obj)->anim.currentMove)
            {
                Object_ObjAnimSetMove(obj, ((GameObject*)obj)->anim.currentMove,
                                      ((GameObject*)obj)->anim.currentMoveProgress, 0);
            }
            if (*(u16*)((char*)model + 0x58) == 0)
            {
                ((GameObject*)obj)->anim.activeMove = -1;
                *(s16*)&((PlayerState*)state)->staffAnimState = 0;
            }
            else
            {
                Object_ObjAnimAdvanceMove(obj, lbl_803E7EA4, timeDelta, NULL);
                Object_ObjAnimSetMoveProgress((ObjAnimComponent*)obj,
                                              ((GameObject*)obj)->anim.currentMoveProgress);
            }
            break;
        default:
            if (((PlayerState*)state)->staffGrown != 0)
            {
                if (((PlayerState*)state)->staffActionRequest == 0)
                {
                    staffDoGrowShrinkAnim((GameObject*)gPlayerPathObject, 0, 0, 0);
                    *(s16*)&((PlayerState*)state)->staffAnimState = 1;
                    changed = 1;
                }
            }
            else if (((PlayerState*)state)->staffActionRequest == 2)
            {
                *(s16*)&((PlayerState*)state)->staffAnimState = 2;
                changed = 1;
            }
            if (((PlayerState*)state)->moveVariantIndex == 5 || ((PlayerState*)state)->moveVariantIndex == 7)
            {
                *(s16*)&((PlayerState*)state)->staffAnimState = 0xf;
                changed = 1;
            }
            break;
        }
        prevChanged = changed;
    } while (changed != 0);
}

void playerProcessQueuedItemCommand(GameObject* obj, int state)
{
    u8 noMatch;
    s16 cmd;
    s16 item;

    if (((PlayerState*)state)->buttonsJustPressed & PAD_BUTTON_Y)
    {
        int yButtonItemResult;
        if (((PlayerState*)state)->buttonsJustPressed & PAD_BUTTON_Y)
        {
            yButtonItemResult = getYButtonItem(&item);
        }
        if (yButtonItemResult == 1)
        {
            buttonDisable(0, PAD_BUTTON_Y);
            ((PlayerState*)state)->buttonsJustPressed &= ~PAD_BUTTON_Y;
            ((PlayerState*)state)->queuedItemCommand = item;
        }
    }

    cmd = ((PlayerState*)state)->queuedItemCommand;
    if (cmd != -1 && cmd != ((PlayerState*)state)->animState && getCurSeqNo() == 0)
    {
        s16 sel = ((PlayerState*)state)->queuedItemCommand;
        noMatch = 0;
        switch (sel)
        {
        case GAMEBIT_STAFF_ABILITY_FIRE_BLASTER:
        case 0x958:
        case GAMEBIT_STAFF_ABILITY_FREEZE_BLAST:
            if (playerCanCastBlasterSpell(obj, state, sel) != 0)
            {
                ByteFlags* f1 = (ByteFlags*)((char*)state + 0x3f1);
                u8 c8;
                if (((PlayerState*)state)->baddie.targetObj != NULL)
                {
                    break;
                }
                c8 = ((PlayerState*)state)->curAnimId;
                if (c8 == 0x49)
                {
                    break;
                }
                if (c8 == 0x52 && !f1->b20 && !f1->b10 && ((PlayerState*)state)->baddie.controlMode != 0x1d)
                {
                    break;
                }
                if (f1->b20)
                {
                    s16 v = obj->anim.rotX;
                    ((PlayerState*)state)->yaw = v;
                    ((PlayerState*)state)->targetYaw = v;
                    ((PlayerState*)state)->lastInputHeading = v;
                    ((PlayerState*)state)->baddie.animSpeedB = lbl_803E7EA4;
                }
                f1->b20 = 0;
                if (f1->b10)
                {
                    u8 c = ((PlayerState*)state)->curAnimId;
                    if (c != 0x48 && c != 0x47 && getCurSeqNo() == 0)
                    {
                        (*gCameraInterface)->setMode(0x42, 0, 1, 0, NULL, 0x1e, 0xff);
                        f1->b10 = 0;
                    }
                }
                cameraSetInterpMode(2);
                (*gCameraInterface)->setMode(0x52, 1, 0, 0, NULL, 0x2d, 0xff);
                ((ByteFlags*)((char*)state + 0x3f6))->b40 = 1;
                (*gPlayerInterface)->setState(obj, (void*)state, 0x2a);
                *(int*)&((PlayerState*)state)->baddie.unk304 = (int)fn_8029A4A8;
                playerCastSpell((int)obj, state, ((PlayerState*)state)->queuedItemCommand);
            }
            else
            {
                noMatch = 1;
            }
            break;
        case 0x957:
            if (fn_802A97D0(obj, state) != 0)
            {
                playerCastSpell((int)obj, state, ((PlayerState*)state)->queuedItemCommand);
            }
            else
            {
                noMatch = 1;
            }
            break;
        case 0x107:
        case 0xc55:
            if (playerCanCastQuakeSpell(obj, state) != 0)
            {
                playerCastSpell((int)obj, state, ((PlayerState*)state)->queuedItemCommand);
            }
            else
            {
                noMatch = 1;
            }
            break;
        case 0x40:
        {
            PlayerState* inner = obj->extra;
            int ok;
            if (((PlayerState*)state)->baddie.targetObj != NULL || *(s16*)((char*)inner->playerStatus + 4) < 0xa ||
                ((ByteFlags*)((char*)inner + 0x3f3))->b08)
            {
                ok = 0;
            }
            else if (((PlayerState*)state)->baddie.controlMode == 1 || ((PlayerState*)state)->baddie.controlMode == 2)
            {
                ok = 1;
            }
            else
            {
                ok = 0;
            }
            if (ok && !((ByteFlags*)((char*)state + 0x3f3))->b08)
            {
                playerCastSpell((int)obj, state, sel);
            }
            else
            {
                noMatch = 1;
            }
            break;
        }
        case 0x5bd:
            if (playerCanCastPortalOpenSpell(obj, state) != 0)
            {
                playerCastSpell((int)obj, state, ((PlayerState*)state)->queuedItemCommand);
            }
            else
            {
                noMatch = 1;
            }
            break;
        default:
            playerCastSpell((int)obj, state, sel);
            break;
        }
        if (noMatch)
        {
            Sfx_PlayFromObject(0, SFXTRIG_id_10a);
        }
    }

    ((PlayerState*)state)->queuedItemCommand = -1;
}

void playerRunActiveSpells(GameObject* obj, int state)
{
    int inner;
    u8 result;
    void** p;
    int z[2];
    int v;
    if (playerIsBlasterSpellAvailable(obj, state, GAMEBIT_STAFF_ABILITY_FIRE_BLASTER) != 0)
    {
        mainSetBits(GAMEBIT_ITEM_Spell0965_Disabled, 0);
        mainSetBits(GAMEBIT_ITEM_FireBlaster_Disabled, 0);
    }
    else
    {
        mainSetBits(GAMEBIT_ITEM_Spell0965_Disabled, 1);
        mainSetBits(GAMEBIT_ITEM_FireBlaster_Disabled, 1);
    }
    if (playerIsBlasterSpellAvailable(obj, state, GAMEBIT_STAFF_ABILITY_FREEZE_BLAST) != 0)
    {
        mainSetBits(GAMEBIT_ITEM_Spell0961_Disabled, 0);
    }
    else
    {
        mainSetBits(GAMEBIT_ITEM_Spell0961_Disabled, 1);
    }
    inner = *(int*)&obj->extra;
    if (((PlayerState*)state)->baddie.targetObj != NULL || *(s16*)(((PlayerState*)inner)->playerStatus + 4) < 0xa ||
        ((ByteFlags*)((char*)inner + 0x3f3))->b08 != 0)
    {
        result = 0;
    }
    else if (((PlayerState*)state)->baddie.controlMode == 1 || ((PlayerState*)state)->baddie.controlMode == 2)
    {
        result = 1;
    }
    else
    {
        result = 0;
    }
    if (result != 0)
    {
        mainSetBits(GAMEBIT_ITEM_SharpClawDisguise_Disabled, 0);
    }
    else
    {
        mainSetBits(GAMEBIT_ITEM_SharpClawDisguise_Disabled, 1);
    }
    if (playerCanCastPortalOpenSpell(obj, state) != 0)
    {
        mainSetBits(GAMEBIT_ITEM_PortalSpell_Disabled, 0);
    }
    else
    {
        mainSetBits(GAMEBIT_ITEM_PortalSpell_Disabled, 1);
    }
    if (fn_802A97D0(obj, state) != 0)
    {
        mainSetBits(GAMEBIT_ITEM_StaffBooster_Disabled, 0);
    }
    else
    {
        mainSetBits(GAMEBIT_ITEM_StaffBooster_Disabled, 1);
    }
    if (playerCanCastQuakeSpell(obj, state) != 0)
    {
        mainSetBits(GAMEBIT_ITEM_SuperQuake_Disabled, 0);
    }
    else
    {
        mainSetBits(GAMEBIT_ITEM_SuperQuake_Disabled, 1);
    }
    switch (((PlayerState*)state)->animState)
    {
    case GAMEBIT_STAFF_ABILITY_FIRE_BLASTER:
        break;
    case GAMEBIT_STAFF_ABILITY_SHARPCLAW_DISGUISE:
        if ((getButtonsJustPressed(0) & 0x200) != 0 && ((ByteFlags*)((char*)state + 0x3f3))->b08 != 0 &&
            ((PlayerState*)state)->curAnimId != 0x44)
        {
            playerSetDisguised(obj, 0);
            ((PlayerState*)state)->animState = -1;
            ((PlayerState*)state)->queuedItemCommand = -1;
            buttonDisable(0, PAD_BUTTON_B);
        }
        ((PlayerState*)state)->stateTimer = ((PlayerState*)state)->stateTimer - timeDelta;
        if (((PlayerState*)state)->stateTimer <= lbl_803E7EA4)
        {
            if (*(s16*)((char*)*(int*)((char*)*(int*)&obj->extra + 0x35c) + 4) < 0)
            {
                v = 0;
            }
            else if (*(s16*)((char*)*(int*)((char*)*(int*)&obj->extra + 0x35c) + 4) >
                     *(s16*)((char*)*(int*)((char*)*(int*)&obj->extra + 0x35c) + 6))
            {
                v = *(s16*)((char*)*(int*)((char*)*(int*)&obj->extra + 0x35c) + 6);
            }
            else
            {
                v = *(s16*)((char*)*(int*)((char*)*(int*)&obj->extra + 0x35c) + 4);
            }
            *(s16*)((char*)*(int*)((char*)*(int*)&obj->extra + 0x35c) + 4) = v;
            ((PlayerState*)state)->stateTimer = lbl_803E7EDC;
        }
        break;
    case GAMEBIT_STAFF_ABILITY_FREEZE_BLAST:
        if (lbl_803DE42C != 0 && getCurSeqNo() != 0)
        {
            ((PlayerState*)state)->animState = -1;
            z[0] = 0;
            lbl_803DE42C = z[0];
            z[1] = z[0];
            p = gPlayerSpawnedObjects;
            for (; z[1] < 7; z[1]++)
            {
                if (p[z[1]] != NULL)
                {
                    Obj_FreeObject((GameObject*)p[z[1]]);
                    p[z[1]] = NULL;
                }
            }
            if (gPlayerResource != NULL)
            {
                Resource_Release(gPlayerResource);
                gPlayerResource = NULL;
            }
        }
        break;
    }
}

void fn_802AFB0C(int obj, int inner, int state)
{
    int orig;
    int work;
    int newAnim;
    int keepKnock;
    int knockKind;
    int canCounter;
    int anim;
    HitFxDesc desc;
    VecXYZ pos;
    u8 buf[12];
    StaffCollisionColorArgs col;
    int surfIdx;
    int damage;
    char* hitObj;

    col = *(StaffCollisionColorArgs*)lbl_802C2C68;
    knockKind = 0;
    if (*(f32*)(*(int*)&((GameObject*)obj)->extra + 0x838) > lbl_803E7ED8)
    {
        ((PlayerState*)inner)->knockbackTimer = lbl_803E7EA4;
    }
    if (gPlayerSfxTimerA > 0)
    {
        gPlayerSfxTimerA = gPlayerSfxTimerA - framesThisStep;
        if (gPlayerSfxTimerA < 0)
        {
            gPlayerSfxTimerA = 0;
        }
    }
    work = ObjHits_GetPriorityHitWithPosition((GameObject*)(obj), (int*)&hitObj, &surfIdx, (u32*)&damage, &pos.x,
                                              &pos.y, &pos.z);
    orig = work;
    if (**(s8**)&((PlayerState*)inner)->playerStatus <= 0)
    {
        **(s8**)&((PlayerState*)inner)->playerStatus = 1;
    }
    if (ObjHits_IsObjectEnabled((ObjAnimComponent*)obj) == 0 || objGetFlagsE5_2((u8*)obj) != 0 ||
        ((ByteFlags*)((char*)inner + 0x3f3))->b20 != 0 ||
        (((GameObject*)obj)->objectFlags & OBJECT_OBJFLAG_PARENT_SLACK))
    {
        return;
    }
    if (*(void**)((char*)inner + 0x7f0) != NULL && work != 0)
    {
        work = 0x15;
    }
    keepKnock = 1;
    if (work != 0)
    {
        if (surfIdx != -1)
        {
            pos.x = pos.x + playerMapOffsetX;
            pos.z = pos.z + playerMapOffsetZ;
        }
        if (*(s16*)((char*)state + 0x278) != 0)
        {
            work = 0x1b;
        }
        if (*(s8*)&((PlayerState*)state)->baddie.stateTag == 3 && *(s8*)((char*)state + 0x34f) <= work)
        {
            return;
        }
        *(s8*)((char*)state + 0x34f) = work;
        ((GameObject*)obj)->anim.activeMove = -1;
        newAnim = -1;
        {
            u32 fl = ((PlayerState*)inner)->flags3F0;
            if ((fl >> 4 & 1) != 0 || (fl >> 2 & 1) != 0 || (fl >> 3 & 1) != 0 || (fl >> 5 & 1) != 0 ||
                (anim = ((PlayerState*)state)->baddie.controlMode) == 0x36)
            {
                canCounter = 0;
            }
            else if ((u16)(anim - 1) <= 1 || (u16)(anim - 0x24) <= 1 || ((PlayerState*)state)->baddie.targetObj != NULL)
            {
                canCounter = 1;
            }
            else
            {
                canCounter = 0;
            }
        }
        switch (work)
        {
        case 0xb:
            if (canCounter && ((PlayerState*)state)->baddie.targetObj != NULL)
            {
                ((PlayerState*)inner)->moveVariantIndex = 2;
                newAnim = 0x23;
                ((PlayerState*)inner)->stateHandler = 0;
            }
            break;
        case 7:
        case 8:
        case 9:
            if (canCounter && ((PlayerState*)state)->baddie.targetObj != NULL)
            {
                ((PlayerState*)inner)->moveVariantIndex = 3;
                newAnim = 0x23;
                ((PlayerState*)inner)->stateHandler = 0;
            }
            break;
        case 0xc:
            if (canCounter && ((PlayerState*)state)->baddie.targetObj != NULL)
            {
                ((PlayerState*)inner)->moveVariantIndex = 1;
                newAnim = 0x23;
                ((PlayerState*)inner)->stateHandler = 0;
            }
            break;
        case 0xa:
            if (canCounter && ((PlayerState*)state)->baddie.targetObj != NULL)
            {
                ((PlayerState*)inner)->moveVariantIndex = 3;
                newAnim = 0x23;
                ((PlayerState*)inner)->stateHandler = 0;
            }
            break;
        case 4:
            if (canCounter)
            {
                newAnim = 0x1f;
                ((PlayerState*)inner)->stateHandler = 0;
            }
            break;
        case 1:
            damage = **(s8**)&((PlayerState*)inner)->playerStatus;
            break;
        case 0x15:
            switch (((PlayerState*)inner)->focusObject->anim.seqId)
            {
            case 0x714:
                Camera_EnableViewYOffset();
                CameraShake_SetAllMagnitudes(lbl_803E7EE0);
                break;
            }
            break;
        case 0x16:
            if (((ByteFlags*)((char*)inner + 0x3f0))->b02 == 0)
            {
                keepKnock = 0;
            }
            if (canCounter && ((PlayerState*)state)->baddie.targetObj == NULL)
            {
                ((PlayerState*)inner)->moveVariantIndex = 5;
            }
            break;
        case 0x19:
            Camera_EnableViewYOffset();
            CameraShake_SetAllMagnitudes(lbl_803E7EE0);
            break;
        case 0x1b:
            newAnim = *(s16*)((char*)state + 0x278);
            break;
        case 0x14:
        case 0x1a:
        case 0x1f:
            if (((PlayerState*)inner)->knockbackTimer <= lbl_803E7EA4)
            {
                knockKind = 1;
            }
            if (((ByteFlags*)((char*)inner + 0x3f0))->b02 == 0)
            {
                keepKnock = 0;
            }
            if (canCounter && ((PlayerState*)state)->baddie.targetObj == NULL)
            {
                ((PlayerState*)inner)->moveVariantIndex = 5;
            }
            break;
        case 0x1e:
            if (((ByteFlags*)((char*)inner + 0x3f3))->b08 == 0)
            {
                knockKind = 2;
                if (((ByteFlags*)((char*)inner + 0x3f0))->b02 == 0)
                {
                    keepKnock = 0;
                }
                if (canCounter && ((PlayerState*)state)->baddie.targetObj == NULL)
                {
                    ((PlayerState*)inner)->moveVariantIndex = 5;
                }
                break;
            }
            return;
        case 2:
        case 5:
        case 0x12:
        case 0x17:
        case 0x18:
            break;
        default:
            if (canCounter && ((PlayerState*)state)->baddie.targetObj != NULL)
            {
                ((PlayerState*)inner)->moveVariantIndex = 0;
                newAnim = 0x23;
                ((PlayerState*)inner)->stateHandler = 0;
            }
            break;
        }
        if ((*(u32*)&((PlayerState*)inner)->flags360 & 0x800) == 0 && knockKind != 0)
        {
            ((PlayerState*)inner)->knockbackTimer = lbl_803E7EDC;
            ((PlayerState*)inner)->knockbackHitTimer = lbl_803E8050;
            ((PlayerState*)inner)->knockbackDrainRate = lbl_803E7EE0;
            ((KnockBits*)((char*)inner + 0x7a8))->knock = (u8)knockKind;
        }
        if ((*(u32*)&((PlayerState*)inner)->flags360 & 0x800) != 0 && keepKnock != 0)
        {
            damage = 0;
            ((ByteFlags*)((char*)inner + 0x3f6))->b10 = 1;
            if (hitObj != NULL && ((GameObject*)hitObj)->anim.seqId != 0x2c5)
            {
                if (gPlayerSfxTimerA == 0)
                {
                    Sfx_PlayFromObject(
                        obj, (u16)(((PlayerState*)inner)->characterId == 0 ? SFXTRIG_pole1_c : SFXTRIG_wp_pole1_c));
                }
                gPlayerSfxTimerA = 6;
            }
            if (gPlayerStepSfxTimer == 0)
            {
                char* pt = *(char**)((char*)Player_GetActiveModel(obj) + 0x50);
                desc.x = playerMapOffsetX + *(f32*)(pt + surfIdx * 0x10 + 4);
                desc.y = *(f32*)(pt + surfIdx * 0x10 + 8);
                desc.z = playerMapOffsetZ + *(f32*)(pt + surfIdx * 0x10 + 0xc);
                (*gPartfxInterface)->spawnObject((void*)obj, 0x328, &desc, 0x200001, -1, NULL);
                desc.x -= ((GameObject*)obj)->anim.worldPosX;
                desc.y -= ((GameObject*)obj)->anim.worldPosY;
                desc.z -= ((GameObject*)obj)->anim.worldPosZ;
                if (gPlayerResource == NULL)
                {
                    gPlayerResource = Resource_Acquire(0x5a, 1);
                }
                col.red += randomGetRange(0, 0x9b);
                col.green += randomGetRange(0, 0x9b);
                desc.scale = lbl_803E7EE0;
                desc.rx = 0;
                desc.ry = 0;
                desc.rz = 0;
                (*gPlayerResource)->spawn((u8*)obj, 0, (PartFxSpawnParams*)&desc, 1, -1, &col);
                if (gPlayerResource != NULL)
                {
                    Resource_Release(gPlayerResource);
                }
                gPlayerResource = NULL;
                gPlayerStepSfxTimer = 10;
                return;
            }
            else
            {
                gPlayerStepSfxTimer = gPlayerStepSfxTimer - 1;
                return;
            }
        }
        if (damage != 0)
        {
            {
                int v;
                int hb = *(int*)&((GameObject*)obj)->extra;
                s8* hp = *(s8**)((char*)hb + 0x35c);
                v = *hp - damage;
                if (v < 0)
                {
                    v = 0;
                }
                else
                {
                    int hi = hp[1];
                    if (v > hi)
                    {
                        v = hi;
                    }
                }
                *hp = v;
                if (**(s8**)((char*)hb + 0x35c) <= 0)
                {
                    playerDie((GameObject*)obj);
                }
            }
            gPlayerStepSfxTimer = 0;
            if (hitObj != NULL)
            {
                switch (((GameObject*)hitObj)->anim.seqId)
                {
                case 0x11:
                case 0x33:
                case 0x13a:
                case 0x5b7:
                case 0x5b8:
                case 0x5b9:
                case 0x5e1:
                    Sfx_PlayFromObject((int)hitObj, SFXTRIG_snort);
                    break;
                case 0x5f9:
                case 0x5fa:
                case 0x5fe:
                    Sfx_PlayFromObject((int)hitObj, SFXTRIG_swd);
                    break;
                case 0x2c5:
                    Sfx_PlayFromObject((int)hitObj, SFXTRIG_wp_crtsmsh6);
                    break;
                case 0x709:
                    Sfx_PlayFromObject((int)hitObj, SFXTRIG_wp_fball2_c);
                    break;
                case 0x458:
                case 0x842:
                    Sfx_PlayFromObject((int)hitObj, SFXTRIG_baddie_mika_death);
                    break;
                }
            }
            switch (orig)
            {
            case 0x16:
                if (hitObj != NULL &&
                    (((GameObject*)hitObj)->anim.seqId == 0x613 || ((GameObject*)hitObj)->anim.seqId == 0x70f))
                {
                    Sfx_PlayFromObject(
                        obj, (u16)(((PlayerState*)inner)->characterId == 0 ? SFXTRIG_foxcom : SFXTRIG_sabrepush163));
                }
                else
                {
                    Sfx_PlayFromObject(obj, SFXTRIG_watery_bubble3);
                }
                break;
            case 0x14:
            case 0x1f:
                Sfx_PlayFromObject(
                    obj, (u16)(((PlayerState*)inner)->characterId == 0 ? SFXTRIG_foxcom : SFXTRIG_sabrepush163));
                Sfx_PlayFromObject(obj, SFXTRIG_en_cvdrip1c_393);
                if (Sfx_IsPlayingFromObject(obj, SFXTRIG_foot_metal_scuff) == 0)
                {
                    Sfx_PlayFromObject(obj, SFXTRIG_foot_metal_scuff);
                }
                if (**(s8**)&((PlayerState*)inner)->playerStatus > 0)
                {
                    objLightFn_8009a1dc((void*)obj, lbl_803E8024, buf, 6, 0);
                }
                break;
            case 0x1c:
                Sfx_PlayFromObject(obj, SFXTRIG_fox_var);
                if (**(s8**)&((PlayerState*)inner)->playerStatus > 0)
                {
                    objLightFn_8009a1dc((void*)obj, lbl_803E8024, buf, 8, 0);
                }
                break;
            default:
                Sfx_PlayFromObject(
                    obj, (u16)(((PlayerState*)inner)->characterId == 0 ? SFXTRIG_foxcom : SFXTRIG_sabrepush163));
                if (hitObj != NULL)
                {
                    switch (((GameObject*)hitObj)->anim.seqId)
                    {
                    case 0x33:
                        Sfx_PlayFromObject(obj, SFXTRIG_snort);
                        if (**(s8**)&((PlayerState*)inner)->playerStatus > 0)
                        {
                            objLightFn_8009a1dc((void*)obj, lbl_803E8024, buf, 5, 0);
                        }
                        break;
                    case 0x7c8:
                        if (**(s8**)&((PlayerState*)inner)->playerStatus > 0)
                        {
                            objLightFn_8009a1dc((void*)obj, lbl_803E8024, buf, 8, 0);
                        }
                        break;
                    default:
                        if (**(s8**)&((PlayerState*)inner)->playerStatus > 0)
                        {
                            objLightFn_8009a1dc((void*)obj, lbl_803E8024, buf, 5, 0);
                        }
                        break;
                    }
                }
                else
                {
                    if (**(s8**)&((PlayerState*)inner)->playerStatus > 0)
                    {
                        objLightFn_8009a1dc((void*)obj, lbl_803E8024, buf, 5, 0);
                    }
                }
                break;
            }
            if (**(s8**)&((PlayerState*)inner)->playerStatus > 0)
            {
                Obj_SetModelColorFadeRecursive((GameObject*)obj, 0xb4, 200, 0, 0, 1);
            }
            if (((PlayerState*)state)->baddie.controlMode == 0x1a)
            {
                fn_8009A8C8((GameObject*)obj, lbl_803E8134);
            }
            ((PlayerState*)inner)->idleHoldTimer = lbl_803E7EA4;
            ((PlayerState*)inner)->idleWaitTimer = randomGetRange(800, 0x44c);
            ((PlayerState*)inner)->isHoldingObject = 0;
            if (((PlayerState*)inner)->heldObj != NULL)
            {
                s16 t = ((GameObject*)((PlayerState*)inner)->heldObj)->anim.seqId;
                if (t == 0x3cf || t == 0x662)
                {
                    objThrowFn_80182504((GameObject*)(((PlayerState*)inner)->heldObj));
                }
                else
                {
                    objSaveFn_800ea774((GameObject*)((PlayerState*)inner)->heldObj);
                }
                *(s16*)((char*)((PlayerState*)inner)->heldObj + 6) =
                    *(s16*)((char*)((PlayerState*)inner)->heldObj + 6) & ~0x4000;
                *(int*)((char*)((PlayerState*)inner)->heldObj + 0xf8) = 0;
                ((PlayerState*)inner)->heldObj = 0;
            }
            if (newAnim != -1 && ((PlayerState*)state)->baddie.controlMode != newAnim &&
                **(s8**)&((PlayerState*)inner)->playerStatus > 0)
            {
                (*gPlayerInterface)->setState((void*)obj, (void*)state, newAnim);
                *(int*)&((PlayerState*)state)->baddie.unk304 = ((PlayerState*)inner)->stateHandler;
            }
        }
        else
        {
            gPlayerStepSfxTimer = 0;
        }
    }
    else
    {
        gPlayerStepSfxTimer = 0;
    }
}

void fn_802B066C(GameObject* obj, int state)
{
    f32 v;
    f32 posWork[6];
    f32 zero;

    if (((PlayerState*)state)->surfaceType == 0x1a)
    {
        return;
    }
    if (((ByteFlags*)((char*)state + 0x3f0))->b10 == 0)
    {
        v = sqrtf(obj->anim.velocityZ * obj->anim.velocityZ +
                  (obj->anim.velocityX * obj->anim.velocityX +
                   obj->anim.velocityY * obj->anim.velocityY));
        ((PlayerState*)state)->knockbackDrainRate = v;
        v = ((PlayerState*)state)->knockbackDrainRate;
        ((PlayerState*)state)->knockbackDrainRate =
            (v < lbl_803E7EE0) ? lbl_803E7EE0 : ((v > lbl_803E8138) ? lbl_803E8138 : v);
    }
    ((PlayerState*)state)->knockbackTimer =
        ((PlayerState*)state)->knockbackTimer - timeDelta * ((PlayerState*)state)->knockbackDrainRate;
    if (((PlayerState*)state)->knockbackTimer <= (zero = lbl_803E7EA4))
    {
        if (Sfx_IsPlayingFromObject((int)obj, SFXTRIG_foot_metal_scuff))
        {
            Sfx_StopFromObject((int)obj, SFXTRIG_foot_metal_scuff);
            Sfx_PlayFromObject((int)obj, SFXTRIG_foot_metal_land);
        }
        ((PlayerState*)state)->knockbackTimer = lbl_803E7EA4;
        return;
    }
    ((PlayerState*)state)->knockbackHitTimer = ((PlayerState*)state)->knockbackHitTimer - timeDelta;
    if (((PlayerState*)state)->knockbackHitTimer <= zero)
    {
        ObjPath_GetPointWorldPosition((GameObject*)obj, 0xb, &posWork[3], &posWork[4], &posWork[5], 0);
        ObjHits_RecordPositionHit(obj, NULL, 0x1f, 1, -1, posWork[3], posWork[4], posWork[5]);
        ((PlayerState*)state)->knockbackHitTimer = lbl_803E8050;
    }
}

void playerStaffInit(GameObject* obj, int state)
{
    GameObject* child;
    int b;

    if (gPlayerPathObject == NULL && Obj_IsLoadingLocked())
    {
        child = Obj_SetupObject(Obj_AllocObjectSetup(0x18, 0x69), 4, -1, -1, obj->anim.parent);
        gPlayerPathObject = child;
        ObjLink_AttachChild(obj, child, 2);
    }
    if (gPlayerPathObject != NULL)
    {
        *(int*)&((GameObject*)gPlayerPathObject)->anim.parent = *(int*)&obj->anim.parent;
    }

    ((PlayerState*)state)->chargeLevel -= lbl_803E7E98 * timeDelta;
    if (((PlayerState*)state)->chargeLevel < *(f32*)&lbl_803E7EA4)
    {
        ((PlayerState*)state)->chargeLevel = lbl_803E7EA4;
    }
    ((PlayerState*)state)->boulderChargeLevel -= lbl_803E7E98 * timeDelta;
    if (((PlayerState*)state)->boulderChargeLevel < *(f32*)&lbl_803E7EA4)
    {
        ((PlayerState*)state)->boulderChargeLevel = lbl_803E7EA4;
    }

    hudSetMagicCostPreview((u8)(int)((PlayerState*)state)->chargeLevel);

    if ((u32)obj != 0)
    {
        b = (((ObjAnimComponent*)obj)->bankIndex != 0);
    }
    else
    {
        b = 0;
    }
    if (b == 0 && mainGetBit(GAMEBIT_ITEM_Staff_Got))
    {
        staffToggle(obj, 0);
    }
}

void playerDoEyeAnims(GameObject* obj, int state)
{
    s16* vec9 = objModelGetVecFn_800395d8(obj, 9);
    s16* vec0 = objModelGetVecFn_800395d8(obj, 0);
    u8 doBlink = 0;
    PlayerState* inner = obj->extra;
    f32 f31v;
    f32 f30v;

    if ((s8) * (s8*)(((PlayerState*)state)->playerStatus) > 0)
    {
        characterDoEyeAnims(obj, (void*)(state + 0x364));
    }
    else
    {
        ObjTextureRuntimeSlot* t5 = objFindTexture(obj, 5, 0);
        ObjTextureRuntimeSlot* t4 = objFindTexture(obj, 4, 0);
        if (t5 != NULL)
        {
            t5->textureId = 0x200;
        }
        if (t4 != NULL)
        {
            t4->textureId = 0x200;
        }
    }
    if ((((PlayerState*)state)->flags360 & 0x2000000u) == 0)
    {
        ((PlayerState*)state)->headPitch =
            (f32)((PlayerState*)state)->headPitch * powfBitEstimate(lbl_803E7FF4, timeDelta);
        ((PlayerState*)state)->headYaw = (f32)((PlayerState*)state)->headYaw * powfBitEstimate(lbl_803E7F1C, timeDelta);
        ((PlayerState*)state)->bodyLeanAngle =
            (f32)((PlayerState*)state)->bodyLeanAngle * powfBitEstimate(lbl_803E7F1C, timeDelta);
        ((PlayerState*)state)->bodyLeanHalf =
            (f32)((PlayerState*)state)->bodyLeanHalf * powfBitEstimate(lbl_803E7F1C, timeDelta);
    }
    if (((ByteFlags*)((char*)state + 0x3f0))->b20)
    {
        f31v = inner->baddie.animSpeedC / *(f32*)((char*)(((PlayerState*)state)->moveParams) + 0x18);
        f31v = (f31v < lbl_803E7EA4) ? lbl_803E7EA4 : ((f31v > lbl_803E7EE0) ? lbl_803E7EE0 : f31v);
        f30v = lbl_803E7EE0 - f31v;
    }
    if (vec9 != NULL)
    {
        if (((ByteFlags*)((char*)state + 0x3f0))->b20)
        {
            f32 k = lbl_803E7E98;
            vec9[2] =
                k * ((f32)((PlayerState*)state)->headPitch * f30v + (f32)((PlayerState*)state)->bodyLeanHalf * f31v);
            vec9[1] =
                k * ((f32)((PlayerState*)state)->bodyLeanHalf * f30v + (f32)((PlayerState*)state)->headPitch * f31v);
        }
        else
        {
            vec9[2] = ((PlayerState*)state)->headPitch;
            vec9[1] = ((PlayerState*)state)->bodyLeanHalf;
        }
    }
    if (vec0 != NULL)
    {
        vec0[0] = -((PlayerState*)state)->headYaw;
        if (((ByteFlags*)((char*)state + 0x3f0))->b20)
        {
            int h4 = ((PlayerState*)state)->bodyLeanAngle / 2;
            int h0 = -(((PlayerState*)state)->headPitch / 2);
            f32 k = lbl_803E7E98;
            vec0[1] = k * ((f32)h4 * f30v + (f32)h0 * f31v);
            vec0[2] = k * ((f32)h0 * f30v + (f32)h4 * f31v);
        }
        else
        {
            vec0[1] = ((PlayerState*)state)->bodyLeanAngle / 2;
            vec0[2] = -(((PlayerState*)state)->headPitch / 2);
        }
    }
    if (!((ByteFlags*)((char*)state + 0x3f0))->b20)
    {
        obj->anim.rotZ = ((PlayerState*)state)->headPitch / 4;
    }
    else
    {
        obj->anim.rotZ = (f32)obj->anim.rotZ * powfBitEstimate(lbl_803E7FF4, timeDelta);
    }
    {
        int e;
        if (((PlayerState*)state)->baddie.controlMode == 1)
        {
            e = 1;
        }
        else
        {
            e = 0;
        }
        playerEyeAnimFn_80038988((int)obj, state + 0x364, e);
    }
    if ((obj->objectFlags & OBJECT_OBJFLAG_PARENT_SLACK) == 0)
    {
        if (((ByteFlags*)((char*)state + 0x3f1))->b20)
        {
            gPlayerSubState = 5;
        }
        else
        {
            if (fn_80295A04(obj, 2) == 0 && (s8) * (s8*)(((PlayerState*)state)->playerStatus) > 4 &&
                gPlayerSubState == 1 && randomGetRange(0, 0x12c) == 1)
            {
                gPlayerSubState = 2;
                doBlink = 1;
            }
            if (doBlink == 0 && gPlayerSubState == 2 && randomGetRange(0, 5) == 1)
            {
                gPlayerSubState = 1;
            }
        }
        {
            s16* vec1 = objModelGetVecFn_800395d8(obj, 1);
            if (vec1 != NULL)
            {
                vec1[0] = 0x1c2;
                vec1[1] = 0;
                vec1[2] = 0;
            }
        }
    }
}

void fn_802B0EA4(GameObject* obj, int inner, int state)
{
    int d;
    char* cam;
    f32 dx;
    f32 dz;
    f32 spd;
    f32 t;
    f32 u;
    int idx;
    f32 one;
    f32 v;

    if ((*(u32*)&((PlayerState*)inner)->flags360 & 0x800000) != 0)
    {
        s16 a = *(s16*)obj;
        ((PlayerState*)inner)->yaw = a;
        ((PlayerState*)inner)->targetYaw = a;
        ((PlayerState*)inner)->lastInputHeading = a;
        ((PlayerState*)state)->baddie.inputMagnitude = lbl_803E7EA4;
    }
    *(f32*)&((PlayerState*)state)->baddie.trackedObj = ((PlayerState*)state)->baddie.inputMagnitude;
    ((PlayerState*)inner)->prevYaw = ((PlayerState*)inner)->yaw;
    ((PlayerState*)inner)->prevTargetYaw = ((PlayerState*)inner)->targetYaw;
    ((PlayerState*)state)->baddie.inputMagnitude =
        sqrtf(((PlayerState*)state)->baddie.moveInputX * ((PlayerState*)state)->baddie.moveInputX +
              ((PlayerState*)state)->baddie.moveInputZ * ((PlayerState*)state)->baddie.moveInputZ);
    if (((PlayerState*)state)->baddie.inputMagnitude > lbl_803E7FA8)
    {
        ((PlayerState*)state)->baddie.inputMagnitude = *(f32*)&lbl_803E7FA8;
    }
    ((PlayerState*)state)->baddie.inputMagnitude = ((PlayerState*)state)->baddie.inputMagnitude / lbl_803E7FA8;
    ((PlayerState*)inner)->inputMagnitude =
        ((PlayerState*)state)->baddie.inputMagnitude - *(f32*)&((PlayerState*)state)->baddie.trackedObj;
    if (((PlayerState*)state)->baddie.inputMagnitude < lbl_803E7F6C)
    {
        ((PlayerState*)state)->baddie.inputMagnitude = lbl_803E7EA4;
        ((PlayerState*)inner)->inputHeading = ((PlayerState*)inner)->lastInputHeading;
    }
    else
    {
        ((PlayerState*)inner)->inputHeading =
            getAngle(((PlayerState*)state)->baddie.moveInputX, -((PlayerState*)state)->baddie.moveInputZ) & 0xffff;
        ((PlayerState*)inner)->inputHeading =
            ((PlayerState*)inner)->inputHeading - ((PlayerState*)state)->baddie.cameraYaw;
        if ((*(u32*)&((PlayerState*)inner)->flags360 & 0x1000000) == 0)
        {
            ((PlayerState*)inner)->lastInputHeading = ((PlayerState*)inner)->inputHeading;
        }
    }
    d = ((PlayerState*)inner)->inputHeading - (u16)((PlayerState*)inner)->yaw;
    if (d > 0x8000)
    {
        d = d - 0xffff;
    }
    if (d < -0x8000)
    {
        d = d + 0xffff;
    }
    ((PlayerState*)inner)->yawRate = (int)((f32)d / gPlayerDegToBinAngle);
    if (((PlayerState*)inner)->turnDeadzoneScale != lbl_803E7EA4)
    {
        f32 dead = ((PlayerState*)inner)->turnDeadzoneScale * ((PlayerState*)state)->baddie.animSpeedA;
        if ((f32)((PlayerState*)inner)->yawRate < dead && (f32)((PlayerState*)inner)->yawRate > -dead)
        {
            ((PlayerState*)inner)->yawRate = 0;
        }
    }
    if (d < 0)
    {
        ((PlayerState*)inner)->yawRateSigned = -((PlayerState*)inner)->yawRate;
    }
    else
    {
        ((PlayerState*)inner)->yawRateSigned = ((PlayerState*)inner)->yawRate;
    }
    if (((PlayerState*)state)->baddie.inputMagnitude < lbl_803E7F6C)
    {
        *(u8*)((char*)state + 0x34b) = 0;
    }
    else
    {
        d = d + 0xa000;
        if (d < 0)
        {
            d = d + 0xffff;
        }
        if (d > 0xffff)
        {
            d = d - 0xffff;
        }
        *(u8*)((char*)state + 0x34b) = (u8)(4 - d / 0x4000);
    }
    d = ((PlayerState*)inner)->inputHeading - (u16)((PlayerState*)inner)->targetYaw;
    if (d > 0x8000)
    {
        d = d - 0xffff;
    }
    if (d < -0x8000)
    {
        d = d + 0xffff;
    }
    ((PlayerState*)inner)->targetYawRate = (int)((f32)d / gPlayerDegToBinAngle);
    if (((PlayerState*)inner)->turnDeadzoneScale != lbl_803E7EA4)
    {
        f32 dead = ((PlayerState*)inner)->turnDeadzoneScale * ((PlayerState*)state)->baddie.animSpeedA;
        if ((f32)((PlayerState*)inner)->targetYawRate < dead && (f32)((PlayerState*)inner)->targetYawRate > -dead)
        {
            ((PlayerState*)inner)->targetYawRate = 0;
        }
    }
    if (d < 0)
    {
        ((PlayerState*)inner)->targetYawRateSigned = -((PlayerState*)inner)->targetYawRate;
    }
    else
    {
        ((PlayerState*)inner)->targetYawRateSigned = ((PlayerState*)inner)->targetYawRate;
    }
    d = ((PlayerState*)inner)->inputHeading - (u16)((PlayerState*)inner)->bodyLeanAngle;
    if (d > 0x8000)
    {
        d = d - 0xffff;
    }
    if (d < -0x8000)
    {
        d = d + 0xffff;
    }
    ((PlayerState*)inner)->bodyLeanRate = (int)((f32)d / gPlayerDegToBinAngle);
    if (d < 0)
    {
        ((PlayerState*)inner)->bodyLeanRateSigned = -((PlayerState*)inner)->bodyLeanRate;
    }
    else
    {
        ((PlayerState*)inner)->bodyLeanRateSigned = ((PlayerState*)inner)->bodyLeanRate;
    }
    *(int*)&((PlayerState*)inner)->cameraTargetObject = (*gCameraInterface)->getTarget();
    cam = *(char**)((char*)inner + 0x4b8);
    if (cam != NULL)
    {
        dx = ((GameObject*)cam)->anim.localPosX - obj->anim.localPosX;
        dz = ((GameObject*)cam)->anim.localPosZ - obj->anim.localPosZ;
        ((PlayerState*)inner)->targetObjectYaw = getAngle(-dx, -dz) & 0xffff;
        ((PlayerState*)inner)->targetObjectDist = sqrtf(dx * dx + dz * dz);
        ((PlayerState*)inner)->targetObjModelType =
            *(u8*)(*(int*)(*(int*)&((GameObject*)cam)->anim.modelInstance + 0x40) + 0x10) & 0xf;
    }
    d = ((PlayerState*)inner)->targetObjectYaw - (u16)((PlayerState*)inner)->targetYaw;
    if (d > 0x8000)
    {
        d = d - 0xffff;
    }
    if (d < -0x8000)
    {
        d = d + 0xffff;
    }
    ((PlayerState*)inner)->targetObjectBearing = (int)(f32)d;
    if (d < 0)
    {
        ((PlayerState*)inner)->targetObjectBearingAbs = -((PlayerState*)inner)->targetObjectBearing;
    }
    else
    {
        ((PlayerState*)inner)->targetObjectBearingAbs = ((PlayerState*)inner)->targetObjectBearing;
    }
    if (((ByteFlags*)((char*)inner + 0x3f1))->b20 != 0)
    {
        spd = sqrtf(((PlayerState*)state)->baddie.animSpeedA * ((PlayerState*)state)->baddie.animSpeedA +
                    ((PlayerState*)state)->baddie.animSpeedB * ((PlayerState*)state)->baddie.animSpeedB);
        t = ((t = lbl_803E7EA4), spd < t) ? t : ((spd > (t = ((PlayerState*)inner)->maxSpeed)) ? t : spd);
        if (lbl_803E7EE0 == ((PlayerState*)inner)->targetAnimSpeed)
        {
            ((PlayerState*)inner)->velSmoothRate = lbl_803E7F44;
        }
        else
        {
            u = t * ((PlayerState*)inner)->curveSpeedScale;
            idx = (int)u;
            ((PlayerState*)inner)->velSmoothRate =
                lbl_803E7EE0 /
                Curve_EvalCatmullRom((void*)(((PlayerState*)inner)->paramCurve0 + (idx + 1) * 4),
                                     u - (f32)idx, 0);
        }
    }
    else
    {
        spd = ((PlayerState*)state)->baddie.animSpeedA;
        t = (spd < (t = lbl_803E7EA4)) ? t : ((spd > (t = ((PlayerState*)inner)->maxSpeed)) ? t : spd);
        u = t * ((PlayerState*)inner)->curveSpeedScale;
        idx = (int)u;
        ((PlayerState*)inner)->velSmoothRate =
            lbl_803E7EE0 /
            Curve_EvalCatmullRom((void*)(((PlayerState*)inner)->paramCurve0 + (idx + 1) * 4), u - (f32)idx, 0);
    }
    u = t * ((PlayerState*)inner)->curveSpeedScale;
    idx = (int)u;
    ((PlayerState*)inner)->targetYawSmoothRate =
        Curve_EvalCatmullRom((void*)(((PlayerState*)inner)->paramCurve1 + (idx + 1) * 4), u - (f32)idx, 0);
    u = t * ((PlayerState*)inner)->curveSpeedScale;
    idx = (int)u;
    ((PlayerState*)inner)->targetYawRateLimit =
        Curve_EvalCatmullRom((void*)(((PlayerState*)inner)->paramCurve2 + (idx + 1) * 4), u - (f32)idx, 0);
    u = t * ((PlayerState*)inner)->curveSpeedScale;
    idx = (int)u;
    ((PlayerState*)inner)->yawSmoothRate =
        Curve_EvalCatmullRom((void*)(((PlayerState*)inner)->paramCurve3 + (idx + 1) * 4), u - (f32)idx, 0);
    u = t * ((PlayerState*)inner)->curveSpeedScale;
    idx = (int)u;
    ((PlayerState*)inner)->yawRateLimit =
        Curve_EvalCatmullRom((void*)(((PlayerState*)inner)->paramCurve4 + (idx + 1) * 4), u - (f32)idx, 0);
    if (((ByteFlags*)((char*)inner + 0x3f0))->b20 != 0)
    {
        f32 k;
        ((PlayerState*)inner)->targetYawSmoothRate = ((PlayerState*)inner)->targetYawSmoothRate * (k = lbl_803E80E4);
        ((PlayerState*)inner)->yawSmoothRate = ((PlayerState*)inner)->yawSmoothRate * k;
        ((PlayerState*)inner)->velSmoothRate = ((PlayerState*)inner)->velSmoothRate * lbl_803E7F44;
    }
    else
    {
        if (lbl_803E7EE0 != ((PlayerState*)inner)->yawSmoothScale)
        {
            f32 base = *(f32*)(((PlayerState*)inner)->moveParams + 0x10);
            f32 frac = (((PlayerState*)state)->baddie.animSpeedA - base) / (((PlayerState*)inner)->maxSpeed - base);
            f32 v430 = ((PlayerState*)inner)->yawSmoothRate;
            f32 diff = ((PlayerState*)inner)->yawSmoothScale - lbl_803E7EE0;
            ((PlayerState*)inner)->yawSmoothRate =
                v430 * (diff * ((frac < lbl_803E7EA4) ? lbl_803E7EA4 : ((frac > lbl_803E7EE0) ? lbl_803E7EE0 : frac)) +
                        *(f32*)&lbl_803E7EE0);
        }
    }
    if (*(void**)((char*)inner + 0x464) != NULL)
    {
        int n = ((PlayerState*)inner)->targetYawRateSigned;
        ((PlayerState*)inner)->leanCurveScale =
            Curve_EvalCatmullRom((void*)(((PlayerState*)inner)->leanCurve + (n / 5 + 1) * 4),
                                 (f32)(n % 5) / lbl_803E7F10, 0);
    }
    else
    {
        ((PlayerState*)inner)->leanCurveScale = lbl_803E7EE0;
    }
    one = lbl_803E7EE0;
    ((PlayerState*)inner)->leanCurveScale = one;
    if (((ByteFlags*)((char*)inner + 0x3f0))->b20 == 0 && ((PlayerState*)inner)->waterDepth > (v = lbl_803E7EA4))
    {
        ((PlayerState*)inner)->speedScale = (((PlayerState*)inner)->waterDepth - lbl_803E7FFC) / lbl_803E8098;
        if (!(((PlayerState*)inner)->speedScale < v))
        {
            v = (((PlayerState*)inner)->speedScale > one) ? one : ((PlayerState*)inner)->speedScale;
        }
        ((PlayerState*)inner)->speedScale = v;
        ((PlayerState*)inner)->speedScale = -(lbl_803E7E98 * ((PlayerState*)inner)->speedScale - lbl_803E7EE0);
    }
    else
    {
        if (((PlayerState*)state)->baddie.spawnRotY > 0)
        {
            ((PlayerState*)inner)->speedScale = (f32)((PlayerState*)state)->baddie.spawnRotY / lbl_803E7EE8;
            v = ((PlayerState*)inner)->speedScale;
            ((PlayerState*)inner)->speedScale =
                (v < lbl_803E7EA4) ? lbl_803E7EA4 : ((v > lbl_803E7EE0) ? lbl_803E7EE0 : v);
            ((PlayerState*)inner)->speedScale = -(lbl_803E7EAC * ((PlayerState*)inner)->speedScale - lbl_803E7EE0);
        }
        else
        {
            ((PlayerState*)inner)->speedScale = lbl_803E7EE0;
        }
    }
    if (((PlayerState*)inner)->heldObj != NULL)
    {
        ((PlayerState*)inner)->speedScale = ((PlayerState*)inner)->speedScale - lbl_803E7EFC;
    }
    v = ((PlayerState*)inner)->speedScale;
    t = (v < lbl_803E7E98) ? lbl_803E7E98 : ((v > lbl_803E7EE0) ? lbl_803E7EE0 : v);
    ((PlayerState*)inner)->speedScale = t;
    *(u32*)&((PlayerState*)inner)->flags360 &= ~0x1800000LL;
}

void fn_802B18BC(GameObject* obj, int state, f32 fv)
{
    f32 v;

    if ((((PlayerState*)state)->buttonsHeld & PAD_BUTTON_A) && playerCanCastQuakeSpell(obj, state))
    {
        ((ByteFlags*)((char*)state + 0x3f4))->b20 = 1;
        ((PlayerState*)state)->buttonHoldTimer += fv;
        v = ((PlayerState*)state)->buttonHoldTimer;
        ((PlayerState*)state)->buttonHoldTimer =
            (v < lbl_803E7EA4) ? lbl_803E7EA4 : ((v > lbl_803E813C) ? lbl_803E813C : v);
    }
    else
    {
        ((ByteFlags*)((char*)state + 0x3f4))->b20 = 0;
        ((PlayerState*)state)->buttonHoldTimer = lbl_803E7EA4;
    }

    ((PlayerState*)state)->rumbleCooldown -= fv;
    if (((PlayerState*)state)->rumbleCooldown < lbl_803E7EA4)
    {
        ((PlayerState*)state)->rumbleCooldown = *(f32*)&lbl_803E7EA4;
    }
    ((PlayerState*)state)->particleBurstCooldown -= fv;
    if (((PlayerState*)state)->particleBurstCooldown < lbl_803E7EA4)
    {
        ((PlayerState*)state)->particleBurstCooldown = *(f32*)&lbl_803E7EA4;
    }
    ((PlayerState*)state)->targetSuppressTimer -= fv;
    if (((PlayerState*)state)->targetSuppressTimer < lbl_803E7EA4)
    {
        ((PlayerState*)state)->targetSuppressTimer = *(f32*)&lbl_803E7EA4;
    }
    ((PlayerState*)state)->idleDelayTimer -= fv;
    if (((PlayerState*)state)->idleDelayTimer < lbl_803E7EA4)
    {
        ((PlayerState*)state)->idleDelayTimer = *(f32*)&lbl_803E7EA4;
    }
}

void playerDoControls(GameObject* obj, int state, f32 fv)
{
    u8 c;

    ((PlayerState*)state)->stickX = 0;
    ((PlayerState*)state)->stickY = 0;
    ((PlayerState*)state)->buttonsHeld = 0;
    ((PlayerState*)state)->buttonsJustPressed = 0;
    ((PlayerState*)state)->buttonsJustPressedIfNotBusy = 0;
    if ((((PlayerState*)state)->flags360 & 0x200000) == 0u && ((PlayerState*)state)->characterId != -1 &&
        (c = ((PlayerState*)state)->curAnimId) != 0x44 && c != 0x4e)
    {
        ((PlayerState*)state)->stickX = padGetStickX(0);
        ((PlayerState*)state)->stickY = padGetStickY(0);
        ((PlayerState*)state)->buttonsHeld = (u16)getButtonsHeld(0);
        ((PlayerState*)state)->buttonsJustPressed = (u16)getButtonsJustPressed(0);
        ((PlayerState*)state)->buttonsJustPressedIfNotBusy = (u16)getButtonsJustPressedIfNotBusy(0);
    }
    ((PlayerState*)state)->stickXf = (f32) * (int*)((char*)state + 0x6d0);
    ((PlayerState*)state)->stickYf = (f32) * (int*)((char*)state + 0x6d4);
    fn_802B18BC(obj, state, fv);
}

void fn_802B1B28(GameObject* obj, f32 fv)
{
    f32 x, y, z;
    f32 v;

    v = obj->anim.velocityX;
    obj->anim.velocityX = (v < lbl_803E801C) ? lbl_803E801C : ((v > lbl_803E7F10) ? lbl_803E7F10 : v);

    v = obj->anim.velocityY;
    obj->anim.velocityY = (v < lbl_803E811C) ? lbl_803E811C : ((v > lbl_803E80E4) ? lbl_803E80E4 : v);

    v = obj->anim.velocityZ;
    obj->anim.velocityZ = (v < lbl_803E801C) ? lbl_803E801C : ((v > lbl_803E7F10) ? lbl_803E7F10 : v);

    y = obj->anim.velocityY * fv;
    if (y > lbl_803E7ED8)
    {
        y = lbl_803E7ED8;
    }
    x = obj->anim.velocityX * fv;
    z = obj->anim.velocityZ * fv;
    objMove((GameObject*)obj, x, y, z);
}

void fn_802B1BF8(GameObject* a, int b, int state, f32 unusedTimeDelta)
{
    MatrixTransform v;
    f32 mtx[16];
    f32 oy;
    f32 f31v;
    f32 f30v;
    s8 flags = *(s8*)((char*)state + 0x34c);

    if ((flags & 2) == 0 && (flags & 1) == 0)
    {
        f31v = ((PlayerState*)state)->baddie.animSpeedA;
        f30v = ((PlayerState*)state)->baddie.animSpeedB;
        if (((ByteFlags*)((char*)b + 0x3f0))->b20)
        {
            f31v = f31v + ((PlayerState*)b)->waterCurrentVelA;
            f30v = f30v + ((PlayerState*)b)->waterCurrentVelB;
        }
        v.rotX = ((PlayerState*)b)->yaw;
        v.rotY = 0;
        v.rotZ = 0;
        v.scale = lbl_803E7EE0;
        v.x = lbl_803E7EA4;
        v.y = lbl_803E7EA4;
        v.z = lbl_803E7EA4;
        setMatrixFromObjectPos(mtx, &v);
        Matrix_TransformPoint(mtx, f30v, lbl_803E7EA4, -f31v, &a->anim.velocityX, &oy, &a->anim.velocityZ);
        a->anim.velocityX = a->anim.velocityX + ((PlayerState*)b)->pushVelX;
        a->anim.velocityZ = a->anim.velocityZ + ((PlayerState*)b)->pushVelZ;
    }
    else
    {
        int cosI = (int)mathSinf(gPlayerPi * (f32) * (s16*)((char*)b + 0x484) / lbl_803E7F98);
        int sinI = (int)mathCosf(gPlayerPi * (f32) * (s16*)((char*)b + 0x484) / lbl_803E7F98);
        ((PlayerState*)state)->baddie.animSpeedB =
            a->anim.velocityX * (f32)sinI - a->anim.velocityZ * (f32)cosI;
        ((PlayerState*)state)->baddie.animSpeedA =
            -a->anim.velocityZ * (f32)sinI - a->anim.velocityX * (f32)cosI;
    }

    if ((*(int*)((char*)state) & 0x200000) == 0)
    {
        a->anim.velocityY = a->anim.velocityY * powfBitEstimate(lbl_803E8140, timeDelta);
        a->anim.velocityY = a->anim.velocityY - ((PlayerState*)state)->baddie.gravity * timeDelta;
    }
}

void fn_802B1E5C(GameObject* obj, int state, int cfg, f32 dt)
{
    u32 b;
    void* found;
    int iv;
    f32 fv2;
    f32 clamp;
    f32 velMag;
    f32 damp;
    f32 r;
    f32 pos[3];
    f32 queryParams[4];
    TrackGroundHit** nearList;
    f32 pushX;
    f32 pushZ;

    found = 0;
    {
        f32 z = lbl_803E7EE0;
        ((PlayerState*)state)->targetAnimSpeed = z;
        ((PlayerState*)state)->yawSmoothScale = z;
    }
    ((PlayerState*)state)->velSmoothRateBase = lbl_803E8144;
    ((PlayerState*)state)->surfaceType = 0;
    b = ((PlayerState*)state)->flags3F0 >> 5 & 1;
    if (b == 0 || (b != 0 && lbl_803E80D0 != *(f32*)((char*)cfg + 0x1c0)))
    {
        ((PlayerState*)state)->waterSurfaceY = *(f32*)((char*)cfg + 0x1c0);
    }
    if (lbl_803E80D0 != ((PlayerState*)state)->waterSurfaceY)
    {
        ((PlayerState*)state)->waterDepth = ((PlayerState*)state)->waterSurfaceY - obj->anim.worldPosY;
    }
    else
    {
        ((PlayerState*)state)->waterDepth = lbl_803E7EA4;
    }
    ((ByteFlags*)((char*)state + 0x3f1))->b01 = 0;
    clamp = lbl_803E7EA4;
    pushX = lbl_803E7EA4;
    pushZ = lbl_803E7EA4;
    if ((*(s8*)((char*)cfg + 0x264) & 0x10) != 0)
    {
        ((ByteFlags*)((char*)state + 0x3f1))->b01 = 1;
        ((PlayerState*)state)->surfaceType = *(u8*)((char*)cfg + 0xbc);
        switch (((PlayerState*)state)->surfaceType)
        {
        case SURFACE_ICE:
            ((PlayerState*)state)->targetAnimSpeed = lbl_803E8148;
            ((PlayerState*)state)->yawSmoothScale = lbl_803E814C;
            ((PlayerState*)state)->velSmoothRateBase = lbl_803E8118;
            break;
        case SURFACE_SNOW:
            fv2 = lbl_803E7EE0;
            ((PlayerState*)state)->targetAnimSpeed = fv2;
            ((PlayerState*)state)->yawSmoothScale = fv2;
            ((PlayerState*)state)->velSmoothRateBase = lbl_803E7F6C;
            break;
        case 6:
            if ((*(s16*)&((PlayerState*)state)->hitIntervalTimer -= dt) <= 0)
            {
                *(s16*)&((PlayerState*)state)->hitIntervalTimer = 0x3c;
                ObjHits_RecordObjectHit(obj, NULL, 0x14, 2, 0);
            }
            break;
        case SURFACE_CONVEYOR:
            queryParams[0] = lbl_803E8150;
            found = (void*)ObjGroup_FindNearestObject(CFGUARDIAN_OBJGROUP, obj, queryParams);
            if (found != 0)
            {
                (*(void (*)(int, int, f32, f32*, f32*))(*(int*)(*(int*)(*(int*)((char*)found + 0x68)) + 0x20)))(
                    (int)found, (int)obj, lbl_803E7EE0, &pushX, &pushZ);
            }
            break;
        case SURFACE_LAVA:
            if ((*(s16*)&((PlayerState*)state)->hitIntervalTimer -= dt) <= 0)
            {
                *(s16*)&((PlayerState*)state)->hitIntervalTimer = 0x3c;
                ObjPath_GetPointWorldPosition((GameObject*)obj, 0xb, &pos[0], &pos[1], &pos[2], 0);
                ObjHits_RecordPositionHit(obj, NULL, 0x14, 2, -1, pos[0], pos[1], pos[2]);
            }
            break;
        case SURFACE_INSTANT_DEATH:
            ObjHits_RecordObjectHit(obj, NULL, 1, 0, 0);
            break;
        case 28:
            if (mainGetBit(0x21) == 0)
            {
                ((PlayerState*)state)->periodicHitTimer += dt;
                if (0x78 < ((PlayerState*)state)->periodicHitTimer)
                {
                    ((PlayerState*)state)->periodicHitTimer -= 0x78;
                    ObjPath_GetPointWorldPosition((GameObject*)obj, 0xb, &pos[0], &pos[1], &pos[2], 0);
                    ObjHits_RecordPositionHit(obj, NULL, 0x16, 2, -1, pos[0], pos[1], pos[2]);
                }
            }
            break;
        case 32:
            if (((PlayerState*)cfg)->baddie.animSpeedA > lbl_803E7E98)
            {
                fv2 = lbl_803E7F6C + ((PlayerState*)state)->sinkOffsetY;
                ((PlayerState*)state)->sinkOffsetY = (fv2 < clamp) ? fv2 : clamp;
            }
            else
            {
                ((PlayerState*)state)->sinkOffsetY = -(lbl_803E7E90 * dt - ((PlayerState*)state)->sinkOffsetY);
                if (lbl_803DE440 > clamp)
                {
                    lbl_803DE440 = lbl_803DE440 - dt;
                }
                else
                {
                    Sfx_PlayFromObject((int)obj, SFXTRIG_dn_boar1_c_208);
                    lbl_803DE440 = (f32)(int)randomGetRange(0x27, 0x3c);
                }
            }
            iv = hitDetectFn_80065e50(obj, obj->anim.localPosX, obj->anim.localPosY,
                                      obj->anim.localPosZ, &nearList, 0, 0x20);
            velMag = -((PlayerState*)state)->sinkOffsetY;
            if (1 < iv &&
                (velMag = velMag + (nearList[0]->height - nearList[iv - 1]->height), velMag > lbl_803E7FA0))
            {
                int inner;
                s8* p = *(s8**)&((PlayerState*)(inner = *(int*)&obj->extra))->playerStatus;
                iv = *p;
                iv = iv - 1;
                if (iv < 0)
                {
                    iv = 0;
                }
                else if (iv > p[1])
                {
                    iv = p[1];
                }
                *p = (s8)iv;
                if (**(s8**)&((PlayerState*)inner)->playerStatus <= 0)
                {
                    playerDie(obj);
                }
            }
            break;
        case 31:
            mainSetBits(0x643, 1);
            break;
        default:
            *(s16*)&((PlayerState*)state)->hitIntervalTimer = 0;
            {
                f32 zero;
                f32 sink = ((PlayerState*)state)->sinkOffsetY;
                if (sink < (zero = *(f32*)&lbl_803E7EA4))
                {
                    fv2 = lbl_803E7EFC * ((PlayerState*)cfg)->baddie.animSpeedA + sink;
                    ((PlayerState*)state)->sinkOffsetY = (fv2 < zero) ? fv2 : zero;
                    velMag = -((PlayerState*)state)->sinkOffsetY;
                }
            }
            break;
        }
        if (velMag != lbl_803E7EA4)
        {
            damp = lbl_803E7F14;
            r = -(lbl_803E7F6C * velMag - lbl_803E7EE0);
            damp = (damp > r) ? damp : r;
            obj->anim.velocityX = obj->anim.velocityX * powfBitEstimate(damp, dt);
            obj->anim.velocityZ = obj->anim.velocityZ * powfBitEstimate(damp, dt);
        }
    }
    r = interpolate(pushX - ((PlayerState*)state)->pushVelX, lbl_803E7FCC, timeDelta);
    ((PlayerState*)state)->pushVelX = ((PlayerState*)state)->pushVelX + r;
    r = interpolate(pushZ - ((PlayerState*)state)->pushVelZ, lbl_803E7FCC, timeDelta);
    ((PlayerState*)state)->pushVelZ = ((PlayerState*)state)->pushVelZ + r;
    if (found == 0)
    {
        ((PlayerState*)state)->pushVelX = ((PlayerState*)state)->pushVelX * powfBitEstimate(lbl_803E7FF4, timeDelta);
        ((PlayerState*)state)->pushVelZ = ((PlayerState*)state)->pushVelZ * powfBitEstimate(lbl_803E7FF4, timeDelta);
    }
    if (((PlayerState*)state)->pushVelX > lbl_803E7FEC && ((PlayerState*)state)->pushVelX < lbl_803E7EF8)
    {
        ((PlayerState*)state)->pushVelX = lbl_803E7EA4;
    }
    if (((PlayerState*)state)->pushVelZ > lbl_803E7FEC && ((PlayerState*)state)->pushVelZ < lbl_803E7EF8)
    {
        ((PlayerState*)state)->pushVelZ = lbl_803E7EA4;
    }
}

void playerItemGetAnimFn(int obj, int inner, int state)
{
    int p;
    int param = 0;
    int msg;

    while (ObjMsg_Pop((void*)obj, (u32*)&msg, (u32*)&p, (u32*)&param) != 0)
    {
        switch (msg)
        {
        case 0x80002:
            ((PlayerState*)inner)->queuedItemCommand = (s16)param;
            if (((PlayerState*)state)->baddie.targetObj != NULL &&
                (param == GAMEBIT_STAFF_ABILITY_FIRE_BLASTER || param == GAMEBIT_STAFF_ABILITY_FREEZE_BLAST))
            {
                ((PlayerState*)inner)->deferredItemCommand = (s16)param;
                ((PlayerState*)inner)->queuedItemCommand = -1;
            }
            break;
        case 0x60003:
        {
            f32 dz;
            f32 dx;
            f32 d;
            f32 zz;
            dx = *(f32*)(p + 0xc) - ((GameObject*)obj)->anim.localPosX;
            dz = ((PlayerState*)p)->baddie.posX - ((GameObject*)obj)->anim.localPosZ;
            zz = dz * dz;
            d = sqrtf(zz + dx * dx);
            if (d > lbl_803E7EE0)
            {
                dx = dx / d;
                dz = dz / d;
            }
            {
                f32 spd = lbl_803E7F9C;
                ((GameObject*)obj)->anim.velocityX = spd * dx;
                ((GameObject*)obj)->anim.velocityZ = spd * dz;
                ((GameObject*)obj)->anim.velocityY = spd;
            }
            (*gPlayerInterface)->setState((void*)obj, (void*)state, 0x21);
            *(int*)&((PlayerState*)state)->baddie.unk304 = 0;
            Player_ApplyStatusDamage((GameObject*)obj, param);
            ((PlayerState*)inner)->isHoldingObject = 0;
            if (((PlayerState*)inner)->heldObj != NULL)
            {
                s16 typ = ((GameObject*)((PlayerState*)inner)->heldObj)->anim.seqId;
                if (typ == 0x3cf || typ == 0x662)
                {
                    objThrowFn_80182504((GameObject*)(((PlayerState*)inner)->heldObj));
                }
                else
                {
                    objSaveFn_800ea774((GameObject*)((PlayerState*)inner)->heldObj);
                }
                *(s16*)((char*)((PlayerState*)inner)->heldObj + 0x6) =
                    *(s16*)((char*)((PlayerState*)inner)->heldObj + 0x6) & ~0x4000;
                *(int*)((char*)((PlayerState*)inner)->heldObj + 0xf8) = 0;
                ((PlayerState*)inner)->heldObj = 0;
            }
            break;
        }
        case 0x60004:
        {
            f32 dz;
            f32 dx = *(f32*)(p + 0xc) - ((GameObject*)obj)->anim.localPosX;
            f32 d;
            dz = ((PlayerState*)p)->baddie.posX - ((GameObject*)obj)->anim.localPosZ;
            d = sqrtf(dx * dx + dz * dz);
            if (d > lbl_803E7EE0)
            {
                dx = dx / d;
                dz = dz / d;
            }
            {
                f32 spd = lbl_803E7F9C;
                ((GameObject*)obj)->anim.velocityX = spd * -dx;
                ((GameObject*)obj)->anim.velocityZ = spd * -dz;
                ((GameObject*)obj)->anim.velocityY = spd;
            }
            (*gPlayerInterface)->setState((void*)obj, (void*)state, 0x21);
            *(int*)&((PlayerState*)state)->baddie.unk304 = 0;
            Player_ApplyStatusDamage((GameObject*)obj, param);
            ((PlayerState*)inner)->isHoldingObject = 0;
            if (((PlayerState*)inner)->heldObj != NULL)
            {
                s16 typ = ((GameObject*)((PlayerState*)inner)->heldObj)->anim.seqId;
                if (typ == 0x3cf || typ == 0x662)
                {
                    objThrowFn_80182504((GameObject*)(((PlayerState*)inner)->heldObj));
                }
                else
                {
                    objSaveFn_800ea774((GameObject*)((PlayerState*)inner)->heldObj);
                }
                *(s16*)((char*)((PlayerState*)inner)->heldObj + 0x6) =
                    *(s16*)((char*)((PlayerState*)inner)->heldObj + 0x6) & ~0x4000;
                *(int*)((char*)((PlayerState*)inner)->heldObj + 0xf8) = 0;
                ((PlayerState*)inner)->heldObj = 0;
            }
            Sfx_PlayFromObject(obj,
                               (u16)(((PlayerState*)inner)->characterId == 0 ? SFXTRIG_foxcom : SFXTRIG_sabrepush163));
            break;
        }
        case 0x60005:
        {
            f32 dz;
            f32 dx = *(f32*)(p + 0xc) - ((GameObject*)obj)->anim.localPosX;
            f32 d;
            dz = ((PlayerState*)p)->baddie.posX - ((GameObject*)obj)->anim.localPosZ;
            d = sqrtf(dx * dx + dz * dz);
            if (d > lbl_803E7EE0)
            {
                dx = dx / d;
                dz = dz / d;
            }
            {
                f32 spd = lbl_803E7F9C;
                ((GameObject*)obj)->anim.velocityX = spd * -dx;
                ((GameObject*)obj)->anim.velocityZ = spd * -dz;
                ((GameObject*)obj)->anim.velocityY = spd;
            }
            (*gPlayerInterface)->setState((void*)obj, (void*)state, 0x21);
            *(int*)&((PlayerState*)state)->baddie.unk304 = 0;
            ObjAnim_SetCurrentMove(obj, 0x450, lbl_803E7EA4, 0);
            Player_ApplyStatusDamage((GameObject*)obj, param);
            ((PlayerState*)inner)->isHoldingObject = 0;
            if (((PlayerState*)inner)->heldObj != NULL)
            {
                s16 typ = ((GameObject*)((PlayerState*)inner)->heldObj)->anim.seqId;
                if (typ == 0x3cf || typ == 0x662)
                {
                    objThrowFn_80182504((GameObject*)(((PlayerState*)inner)->heldObj));
                }
                else
                {
                    objSaveFn_800ea774((GameObject*)((PlayerState*)inner)->heldObj);
                }
                *(s16*)((char*)((PlayerState*)inner)->heldObj + 0x6) =
                    *(s16*)((char*)((PlayerState*)inner)->heldObj + 0x6) & ~0x4000;
                *(int*)((char*)((PlayerState*)inner)->heldObj + 0xf8) = 0;
                ((PlayerState*)inner)->heldObj = 0;
            }
            break;
        }
        case 0x7000a:
        {
            void* t;
            s16 bit;
            ((PlayerState*)inner)->triggerGameBitPtr = param;
            t = *(void**)(p + 0x64);
            if (t != NULL)
            {
                *(u32*)((char*)t + 0x30) &= ~0x4LL;
            }
            bit = *(s16*)((PlayerState*)inner)->triggerGameBitPtr;
            if (bit > 0)
            {
                if (mainGetBit(bit) != 0)
                {
                    ObjMsg_SendToObject((void*)p, 0x7000b, (void*)obj, 0);
                    break;
                }
                else
                {
                    f32 k;
                    f32 lim;
                    f32 r = *(f32*)(p + 8) / *(f32*)(*(int*)(p + 0x50) + 4);
                    lim = 30.0f;
                    k = 0.99f;
                    while (r * (((GameObject*)obj)->anim.hitboxScale * ((GameObject*)obj)->anim.rootMotionScale) > lim)
                    {
                        *(f32*)(p + 8) = *(f32*)(p + 8) * k;
                        r = *(f32*)(p + 8) / *(f32*)(*(int*)(p + 0x50) + 4);
                    }
                    mainSetBits(*(s16*)((PlayerState*)inner)->triggerGameBitPtr, 1);
                    (*gObjectTriggerInterface)->setObjects(*(s16*)(p + 0x46), 0, 0);
                    (*gObjectTriggerInterface)->runSequence(0, (void*)obj, -1);
                }
            }
            else
            {
                f32 k;
                f32 lim;
                f32 r = *(f32*)(p + 8) / *(f32*)(*(int*)(p + 0x50) + 4);
                lim = 30.0f;
                k = 0.99f;
                while (r * (((GameObject*)obj)->anim.hitboxScale * ((GameObject*)obj)->anim.rootMotionScale) > lim)
                {
                    *(f32*)(p + 8) = *(f32*)(p + 8) * k;
                    r = *(f32*)(p + 8) / *(f32*)(*(int*)(p + 0x50) + 4);
                }
                (*gObjectTriggerInterface)->setObjects(*(s16*)(p + 0x46), 0, 0);
                (*gObjectTriggerInterface)->runSequence(0, (void*)obj, -1);
            }
            ((PlayerState*)inner)->interactObject = p;
            ((PlayerState*)inner)->unk688 = *(s16*)(((PlayerState*)inner)->triggerGameBitPtr + 2);
            t = *(void**)(((PlayerState*)inner)->interactObject + 0x64);
            if (t != NULL)
            {
                *(int*)((char*)t + 0x30) = 0x1000;
            }
            if (gPlayerPathObject != 0 && ((ByteFlags*)((char*)inner + 0x3f4))->b40 != 0)
            {
                ((PlayerState*)inner)->staffActionRequest = 1;
                ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
            }
            break;
        }
        case 0x100008:
            ((PlayerState*)inner)->isHoldingObject = 1;
            if ((void*)((PlayerState*)inner)->heldObj == NULL)
            {
                int* mdl;
                ((PlayerState*)inner)->heldObj = (GameObject*)p;
                mdl = (int*)Obj_GetActiveModel(((PlayerState*)inner)->heldObj);
                if (mdl != NULL && (void*)*mdl != NULL && (*(u16*)(*mdl + 2) & 0x8000) == 0)
                {
                    *(u8*)((char*)((PlayerState*)inner)->heldObj + 0xf2) = ((GameObject*)obj)->lightColorSlot;
                }
                ((PlayerState*)inner)->unk7FC = (f32)(param >> 0x10) / lbl_803E7ED8;
                (*gPlayerInterface)->setState((void*)obj, (void*)state, 5);
                *(void (**)(GameObject*))&((PlayerState*)state)->baddie.unk304 = fn_802A4B4C;
                if (gPlayerPathObject != 0 && ((ByteFlags*)((char*)inner + 0x3f4))->b40 != 0)
                {
                    ((PlayerState*)inner)->staffActionRequest = 1;
                    ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
                }
            }
            break;
        case 0x100010:
            ((PlayerState*)inner)->isHoldingObject = 1;
            if ((void*)((PlayerState*)inner)->heldObj == NULL)
            {
                int* mdl;
                ((PlayerState*)inner)->heldObj = (GameObject*)p;
                mdl = (int*)Obj_GetActiveModel(((PlayerState*)inner)->heldObj);
                if (mdl != NULL && (void*)*mdl != NULL && (*(u16*)(*mdl + 2) & 0x8000) == 0)
                {
                    *(u8*)((char*)((PlayerState*)inner)->heldObj + 0xf2) = ((GameObject*)obj)->lightColorSlot;
                }
                ((PlayerState*)inner)->unk7FC = (f32)(param >> 0x10);
                (*gPlayerInterface)->setState((void*)obj, (void*)state, 5);
                *(void (**)(GameObject*))&((PlayerState*)state)->baddie.unk304 = fn_802A4B4C;
                if (gPlayerPathObject != 0 && ((ByteFlags*)((char*)inner + 0x3f4))->b40 != 0)
                {
                    ((PlayerState*)inner)->staffActionRequest = 1;
                    ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
                }
            }
            break;
        }
    }
}

int player_SeqFn(int obj, int obj2, ObjSeqState* seq, int endFlag)
{
    int tbl = (int)lbl_80332EC0;
    PlayerSeqPlacement* placement = (PlayerSeqPlacement*)((GameObject*)obj2)->anim.placementData;
    PlayerState* inner = ((GameObject*)obj)->extra;
    int result = 0;
    int va;
    int vb;
    f32 npos[3];
    f32 pz;
    f32 py;
    f32 px;
    int objCount;
    f32 nearArg;

    va = (int)objModelGetVecFn_800395d8((GameObject*)(obj), 0);
    vb = (int)objModelGetVecFn_800395d8((GameObject*)(obj), 9);
    seq->freeCallback = (ObjAnimSequenceFreeCallback)fn_802A93F4;
    if (gPlayerStaffObject != NULL)
    {
        staffFn_80170380(gPlayerStaffObject, 0);
    }
    playerStaffInit((GameObject*)obj, (int)inner);
    if (*(void**)&gPlayerEggObject == NULL && Obj_IsLoadingLocked() != 0)
    {
        ObjLink_AttachChild((GameObject*)obj,
                            (GameObject*)(gPlayerEggObject =
                                (int)Obj_SetupObject(Obj_AllocObjectSetup(0x18, 0x66a), 4, -1, -1,
                                                     ((GameObject*)obj)->anim.parent)),
                            3);
    }
    if (*(void**)&gPlayerEggObject != NULL)
    {
        *(int*)&((GameObject*)gPlayerEggObject)->anim.parent = *(int*)&((GameObject*)obj)->anim.parent;
        if (inner->characterId == 0)
        {
            *(s16*)(gPlayerEggObject + 6) |= 0x4000;
        }
    }
    if (gPlayerStaffObject == NULL && Obj_IsLoadingLocked() != 0)
    {
        gPlayerStaffObject =
            (GameObject*)Obj_SetupObject(Obj_AllocObjectSetup(0x24, 0x773), 5, -1, -1, ((GameObject*)obj)->anim.parent);
    }
    if (gPlayerStaffObject != NULL)
    {
        ObjPath_GetPointWorldPosition((GameObject*)obj, 4, &gPlayerStaffObject->anim.localPosX,
                                      &gPlayerStaffObject->anim.localPosY, &gPlayerStaffObject->anim.localPosZ, 0);
    }
    if ((((u32) * (u8*)((char*)inner + 0x3f3) >> 3 & 1) != 0 || inner->animState == 0x40) &&
        ((u32) * (u8*)((char*)inner + 0x3f4) >> 7 & 1) == 0)
    {
        playerSetDisguised((GameObject*)obj, 0);
        inner->animState = -1;
    }
    ObjHits_DisableObject((GameObject*)obj);
    *(u32*)&inner->flags360 &= ~PLAYER_FLAG_HITDETECT;
    if ((s8)seq->movementState != 0)
    {
        s8 c;
        *(u32*)&inner->flags360 &= ~PLAYER_FLAG_AIM_READY;
        {
            f32 fz = 0.0f;
            inner->knockbackTimer = fz;
            inner->knockbackHitTimer = fz;
        }
        if (((u32) * (u8*)((char*)inner + 0x3f2) >> 7 & 1) == 0)
        {
            if (gPlayerPathObject != NULL && ((u32) * (u8*)((char*)inner + 0x3f4) >> 6 & 1) != 0)
            {
                inner->staffActionRequest = 1;
                ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
            }
            inner->isHoldingObject = 0;
            {
                GameObject* p = inner->heldObj;
                if (p != NULL)
                {
                    s16 sp = p->anim.seqId;
                    if (sp == 0x3cf || sp == 0x662)
                    {
                        objThrowFn_80182504(p);
                    }
                    else
                    {
                        objSaveFn_800ea774(p);
                    }
                    *(s16*)((char*)inner->heldObj + 6) &= ~0x4000;
                    *(int*)((char*)inner->heldObj + 0xf8) = 0;
                    inner->heldObj = 0;
                }
            }
        }
        if (placement->movementEnabled == 0 || (c = (s8)seq->movementState) == 3 || c == 2)
        {
            seq->flags = seq->savedFlags;
            if ((s8)seq->movementState != 2)
            {
                seq->posOffsetScale = 1.0f;
                seq->posOffsetX = ((GameObject*)obj)->anim.localPosX - ((GameObject*)obj2)->anim.localPosX;
                seq->posOffsetY = ((GameObject*)obj)->anim.localPosY - ((GameObject*)obj2)->anim.localPosY;
                seq->posOffsetZ = ((GameObject*)obj)->anim.localPosZ - ((PlayerState*)obj2)->baddie.posX;
                seq->rotOffsetX = inner->targetYaw - (u16) * (s16*)obj2;
                if (seq->rotOffsetX > 0x8000)
                {
                    seq->rotOffsetX = seq->rotOffsetX - 0xffff;
                }
                if (seq->rotOffsetX < -0x8000)
                {
                    seq->rotOffsetX = seq->rotOffsetX + 0xffff;
                }
                seq->rotOffsetY = ((GameObject*)obj)->anim.rotY - (u16) * (s16*)((char*)obj2 + 2);
                if (seq->rotOffsetY > 0x8000)
                {
                    seq->rotOffsetY = seq->rotOffsetY - 0xffff;
                }
                if (seq->rotOffsetY < -0x8000)
                {
                    seq->rotOffsetY = seq->rotOffsetY + 0xffff;
                }
                seq->rotOffsetZ = (u16) * (s16*)((char*)obj2 + 4) - (u16)((GameObject*)obj)->anim.rotZ;
                if (seq->rotOffsetZ > 0x8000)
                {
                    seq->rotOffsetZ = seq->rotOffsetZ - 0xffff;
                }
                if (seq->rotOffsetZ < -0x8000)
                {
                    seq->rotOffsetZ = seq->rotOffsetZ + 0xffff;
                }
                seq->movementState = 2;
            }
            seq->posOffsetScale = -(seq->posOffsetDecay * timeDelta - seq->posOffsetScale);
            if (seq->posOffsetScale <= 0.0f)
            {
                seq->movementState = 0;
            }
            ((GameObject*)obj)->anim.activeMove = -1;
            inner->bodyLeanHalf = 0;
            inner->headPitch = 0;
            inner->bodyLeanAngle = 0;
            inner->headYaw = 0;
        }
        else if (c == 4)
        {
            f32 dz;
            f32 dy;
            f32 dx;
            int d;
            seq->flags &= ~0x4c;
            seq->savedFlags &= ~0x48;
            obj2 = (int)getFocusedNpc();
            if (objModelGetVecFn_800395d8((GameObject*)(obj2), 0) != 0)
            {
                objPosFn_80039510((GameObject*)(obj2), 0, npos);
            }
            else
            {
                ObjHitVolumeRuntimeTransform* pv = ((GameObject*)obj2)->anim.hitVolumeTransforms;
                if (pv == NULL)
                {
                    npos[0] = ((GameObject*)obj2)->anim.worldPosX;
                    npos[1] = ((GameObject*)obj2)->anim.worldPosY;
                    npos[2] = ((GameObject*)obj2)->anim.worldPosZ;
                }
                else
                {
                    npos[0] = pv->jointX;
                    npos[1] = pv->jointY;
                    npos[2] = pv->jointZ;
                }
            }
            ObjPath_GetPointWorldPosition((GameObject*)obj, 5, &px, &py, &pz, 0);
            dx = ((GameObject*)obj)->anim.worldPosX - npos[0];
            dy = (((PlayerState*)inner)->pathBearingEyeY + ((GameObject*)obj)->anim.worldPosY) - npos[1];
            dz = ((GameObject*)obj)->anim.worldPosZ - npos[2];
            {
                s16 ang = (s16)getAngle(dx, dz);
                lbl_803DE4B0 = ang;
                d = ang - (u16) ((PlayerState*)inner)->targetYaw;
            }
            if (d > 0x8000)
            {
                d -= 0xffff;
            }
            if (d < -0x8000)
            {
                d += 0xffff;
            }
            *(s16*)((char*)inner + 0x4d8) = -*(s16*)(va + 2);
            *(s16*)((char*)inner + 0x4dc) = -*(s16*)va;
            if (d >= 0)
            {
                if (d > 0x2aaa)
                {
                    ((PlayerState*)inner)->bodyLeanAimDelta = -0x2aaa;
                    ((PlayerState*)inner)->aimTurnYaw = d - 0x2aaa;
                }
                else
                {
                    ((PlayerState*)inner)->bodyLeanAimDelta = -d;
                    ((PlayerState*)inner)->aimTurnYaw = 0;
                }
            }
            else if (d < -0x2aaa)
            {
                ((PlayerState*)inner)->bodyLeanAimDelta = 0x2aaa;
                ((PlayerState*)inner)->aimTurnYaw = d + 0x2aaa;
            }
            else
            {
                ((PlayerState*)inner)->bodyLeanAimDelta = -d;
                ((PlayerState*)inner)->aimTurnYaw = 0;
            }
            ((PlayerState*)inner)->headYawAimDelta = (s16)getAngle(dy, sqrtf(dx * dx + dz * dz));
            {
                int v = ((PlayerState*)inner)->headYawAimDelta;
                if (v < -0x1000)
                {
                    v = -0x1000;
                }
                else if (v > 0x1000)
                {
                    v = 0x1000;
                }
                ((PlayerState*)inner)->headYawAimDelta = v;
            }
            seq->rotOffsetZ = 0;
            seq->posOffsetScale = 0.0f;
            seq->posOffsetDecay = 0.033333335f;
            seq->movementState = 5;
            {
                int mv;
                if (*(u32*)&((PlayerState*)inner)->heldObj != 0)
                {
                    mv = 8;
                }
                else
                {
                    mv = 0;
                }
                if (((GameObject*)obj)->anim.currentMove != mv)
                {
                    ObjAnim_SetCurrentMove(obj, mv, 0.0f, 0);
                    ObjAnim_SetCurrentEventStepFrames((ObjAnimComponent*)obj, 1);
                }
            }
            ObjAnim_AdvanceCurrentMove((int)obj, 0.005f, timeDelta, 0);
            result = 1;
        }
        else if (c == 5)
        {
            seq->flags &= ~0x4c;
            seq->savedFlags &= ~0x48;
            ObjHits_EnableObject((GameObject*)obj);
            if (seq->posOffsetScale >= 1.0f && (*gCameraInterface)->isZooming() == 0)
            {
                ((PlayerState*)inner)->bodyLeanHalf = 0;
                ((PlayerState*)inner)->headPitch = 0;
                if ((s8)endFlag == 0)
                {
                    seq->movementState = 0;
                }
                else
                {
                    seq->movementState = 6;
                }
                if (((PlayerState*)inner)->focusObject != NULL)
                {
                    (*gPlayerInterface)->setState((void*)obj, inner, 0x18);
                    *(void (**)(int))((char*)inner + 0x304) = (void (*)(int))fn_8029F67C;
                }
                else
                {
                    (*gPlayerInterface)->setState((void*)obj, inner, 1);
                    *(void (**)(int, int))((char*)inner + 0x304) = (void (*)(int, int))fn_802A514C;
                    ((PlayerState*)inner)->baddie.prevControlMode = 1;
                }
            }
            else
            {
                f32 prev = seq->posOffsetScale;
                f32 one;
                int dd;
                seq->posOffsetScale = seq->posOffsetDecay * timeDelta + prev;
                if (seq->posOffsetScale > 1.0f)
                {
                    seq->posOffsetScale = 1.0f;
                }
                prev = seq->posOffsetScale - prev;
                ((PlayerState*)inner)->targetYaw += (s16)(prev * (f32) * (s16*)((char*)inner + 0x4e0));
                *(s16*)obj = ((PlayerState*)inner)->yaw = ((PlayerState*)inner)->targetYaw;
                dd = *(s16*)((char*)inner + 0x4d8) - (u16) * (s16*)((char*)inner + 0x4da);
                if (dd > 0x8000)
                {
                    dd = dd - 0xffff;
                }
                if (dd < -0x8000)
                {
                    dd = dd + 0xffff;
                }
                *(s16*)(va + 2) = (s16)((f32)dd * seq->posOffsetScale + (f32) * (s16*)((char*)inner + 0x4d8));
                dd = *(s16*)((char*)inner + 0x4dc) - (u16) * (s16*)((char*)inner + 0x4de);
                if (dd > 0x8000)
                {
                    dd = dd - 0xffff;
                }
                if (dd < -0x8000)
                {
                    dd = dd + 0xffff;
                }
                *(s16*)va = (s16)((f32)dd * seq->posOffsetScale + (f32) * (s16*)((char*)inner + 0x4dc));
                *(s16*)(vb + 2) = (s16)((f32) * (s16*)((char*)inner + 0x4d2) * ((one = 1.0f) - seq->posOffsetScale));
                *(s16*)(vb + 4) = (s16)((f32) * (s16*)((char*)inner + 0x4d0) * (one - seq->posOffsetScale));
                ((GameObject*)obj)->anim.rotZ = *(s16*)(vb + 4) / 4;
                ((PlayerState*)inner)->bodyLeanAngle = *(s16*)(va + 2);
                ((PlayerState*)inner)->headYaw = -*(s16*)va;
            }
            ObjAnim_AdvanceCurrentMove((int)obj, 0.005f, timeDelta, 0);
            result = 1;
        }
        else if (c == 6)
        {
            seq->flags &= ~0x4c;
            seq->savedFlags &= ~0x48;
            ObjHits_EnableObject((GameObject*)obj);
            if ((s8)endFlag == 0)
            {
                seq->movementState = 0;
            }
            ObjAnim_AdvanceCurrentMove((int)obj, 0.005f, timeDelta, 0);
            result = 0;
        }
        else
        {
            f32 dx2;
            f32 dz2;
            f32 dist;
            f32 d2;
            if (c != 1)
            {
                seq->posOffsetX = ((GameObject*)obj)->anim.localPosX;
                seq->posOffsetY = ((GameObject*)obj)->anim.localPosY;
                seq->posOffsetZ = ((GameObject*)obj)->anim.localPosZ;
                lbl_803DE468 = 10000.0f;
                lbl_803DE46C = 0;
            }
            result = 1;
            seq->flags = 0;
            seq->movementState = 1;
            {
                f32 ax = seq->posOffsetX - ((GameObject*)obj)->anim.localPosX;
                f32 az = seq->posOffsetZ - ((GameObject*)obj)->anim.localPosZ;
                dist = sqrtf(ax * ax + az * az);
            }
            dx2 = ((GameObject*)obj2)->anim.localPosX - seq->posOffsetX;
            dz2 = ((PlayerState*)obj2)->baddie.posX - seq->posOffsetZ;
            d2 = sqrtf(dx2 * dx2 + dz2 * dz2);
            if (dist <= lbl_803DE468)
            {
                lbl_803DE46C += 1;
            }
            if (dist >= d2 || lbl_803DE46C > 5)
            {
                int dd3 = ((PlayerState*)inner)->targetYaw - (u16) * (s16*)obj2;
                if (dd3 > 0x8000)
                {
                    dd3 -= 0xffff;
                }
                if (dd3 < -0x8000)
                {
                    dd3 += 0xffff;
                }
                if (dd3 > 0x4000)
                {
                    dd3 = 0x4000;
                }
                if (dd3 < -0x4000)
                {
                    dd3 = -0x4000;
                }
                ((PlayerState*)inner)->targetYaw -= (dd3 * framesThisStep) >> 3;
                ((PlayerState*)inner)->yaw = ((PlayerState*)inner)->targetYaw;
                if (lbl_803DE46C > 6)
                {
                    dd3 = 0;
                }
                if (dd3 < 0x100 && dd3 > -0x100)
                {
                    seq->flags = seq->savedFlags;
                    seq->movementState = 0;
                    seq->prevFrame = seq->curFrame - 1;
                    ((GameObject*)obj)->anim.activeMove = -1;
                    result = 0;
                }
                else
                {
                    f32 fz3 = 0.0f;
                    ((PlayerState*)inner)->baddie.moveInputX = fz3;
                    ((PlayerState*)inner)->baddie.moveInputZ = fz3;
                    (*gPlayerInterface)->setOverride((u32)obj2);
                    *(int*)&((PlayerState*)inner)->baddie.unk31C = 0;
                    *(int*)&((PlayerState*)inner)->baddie.unk318 = 0;
                    ((GameObject*)obj)->userData1 = 0;
                    ((PlayerState*)inner)->baddie.cameraYaw = 0;
                    ((PlayerState*)inner)->baddie.physicsActive = 1;
                    *(u32*)((char*)inner + 4) = *(u32*)((char*)inner + 4) & ~0x100000;
                    ((PlayerState*)inner)->emissionState = 0;
                    fn_802B0EA4((GameObject*)(obj), (int)inner, (int)inner);
                    (*gPlayerInterface)->update((void*)obj, inner, timeDelta, timeDelta, gPlayerStateHandlers,
                        &gPlayerDefaultStateHandler);
                }
            }
            else
            {
                dx2 = dx2 / d2;
                dz2 = dz2 / d2;
                {
                    f32 k = 40.0f;
                    ((PlayerState*)inner)->baddie.moveInputX = k * -dx2;
                    ((PlayerState*)inner)->baddie.moveInputZ = k * dz2;
                }
                ((GameObject*)obj)->anim.localPosX = dist * dx2 + seq->posOffsetX;
                ((GameObject*)obj)->anim.localPosZ = dist * dz2 + seq->posOffsetZ;
                (*gPlayerInterface)->setOverride((u32)obj2);
                *(int*)&((PlayerState*)inner)->baddie.unk31C = 0;
                *(int*)&((PlayerState*)inner)->baddie.unk318 = 0;
                ((GameObject*)obj)->userData1 = 0;
                ((PlayerState*)inner)->baddie.cameraYaw = 0;
                ((PlayerState*)inner)->baddie.physicsActive = 1;
                *(u32*)((char*)inner + 4) = *(u32*)((char*)inner + 4) & ~0x100000;
                ((PlayerState*)inner)->emissionState = 0;
                fn_802B0EA4((GameObject*)(obj), (int)inner, (int)inner);
                (*gPlayerInterface)->update((void*)obj, inner, timeDelta, timeDelta, gPlayerStateHandlers,
                    &gPlayerDefaultStateHandler);
            }
            lbl_803DE468 = dist;
        }
        if ((s8)seq->movementState == 0)
        {
            (*gPlayerInterface)->setState((void*)obj, inner, 1);
            *(void (**)(int, int))((char*)inner + 0x304) = (void (*)(int, int))fn_802A514C;
            ((PlayerState*)inner)->baddie.prevControlMode = 1;
        }
    }
    else
    {
        seq->flags |= seq->savedFlags & ~0x400;
        *(u8*)((char*)inner + 0x34c) = 0;
        {
            f32 fz2 = 0.0f;
            ((PlayerState*)inner)->baddie.moveInputX = fz2;
            ((PlayerState*)inner)->baddie.moveInputZ = fz2;
        }
        ((PlayerState*)inner)->baddie.cameraYaw = 0;
        *(int*)&((PlayerState*)inner)->baddie.unk31C = 0;
        *(int*)&((PlayerState*)inner)->baddie.unk318 = 0;
        if (seq->flags & 1)
        {
            *(u32*)((char*)inner + 4) |= 0x100000;
            ((PlayerState*)inner)->baddie.physicsActive = 0;
        }
        for (vb = 0; vb < seq->eventCount; vb++)
        {
            switch (seq->eventIds[vb])
            {
            case 3:
            {
                f32 best;
                u8 found;
                void* objs = ObjGroup_GetObjects(10, &objCount);
                found = 0;
                best = 10000.0f;
                for (endFlag = 0, obj2 = (int)objs; endFlag < objCount; endFlag++)
                {
                    va = *(int*)obj2;
                    if ((u32)va != 0 && arrayIndexOf((int*)(tbl + 0x13c), 9, ((GameObject*)va)->anim.seqId) != -1)
                    {
                        f32 dsq = vec3f_distanceSquared((f32*)(va + 0x18), (f32*)(obj + 0x18));
                        if (dsq < best || found == 0)
                        {
                            best = dsq;
                            ((PlayerState*)inner)->focusObject = (GameObject*)va;
                            found = 1;
                        }
                    }
                    obj2 += 4;
                }
                if (found != 0)
                {
                    ((PlayerState*)inner)->unk6A4 = 1.0f;
                    ((PlayerState*)inner)->unk6A8 = ((PlayerState*)inner)->savedPosX;
                    ((PlayerState*)inner)->unk6AC = ((PlayerState*)inner)->savedPosY;
                    ((PlayerState*)inner)->unk6B0 = ((PlayerState*)inner)->savedPosZ;
                    va = (int)((PlayerState*)inner)->focusObject;
                    (*(void (*)(int, int)) * (int*)((char*)*(int*)(*(int*)(va + 0x68)) + 0x3c))(va, 2);
                    ((GameObject*)obj)->anim.flags |= 8;
                    ((GameObject*)obj)->anim.modelState->flags |= OBJ_MODEL_STATE_SHADOW_FADE_OUT;
                    ((GameObject*)obj)->anim.modelState->shadowAlphaStep = 0;
                    seq->flags &= ~4;
                    switch (((GameObject*)va)->anim.seqId)
                    {
                    case 0x72:
                    case 0x38c:
                        Music_Trigger(MUSICTRIG_drako_2, 1);
                        mainSetBits(0xc1f, 0);
                        ((PlayerState*)inner)->moveSequence = tbl + 0x3f0;
                        ((PlayerState*)inner)->moveSequenceFlags = 3;
                        ObjAnim_SetCurrentMove(obj, 0x17, 0.0f, 1);
                        break;
                    case 0x8c:
                        ((PlayerState*)inner)->moveSequence = tbl + 0x408;
                        ((PlayerState*)inner)->moveSequenceFlags = 4;
                        ObjAnim_SetCurrentMove(obj, 0x7b, 0.0f, 1);
                        if (getSbGalleon() != NULL)
                        {
                            (*gCameraInterface)->setFocus((void*)va, 0);
                            (*gObjectTriggerInterface)->setCamVars(0x4a, 1, 0, 0x78);
                        }
                        break;
                    case 0x416:
                        Music_Trigger(MUSICTRIG_WLC_Puzzle, 1);
                        ((PlayerState*)inner)->moveSequence = tbl + 0x438;
                        ((PlayerState*)inner)->moveSequenceFlags = 8;
                        ObjAnim_SetCurrentMove(obj, *(s16*)(tbl + 0x438), 0.0f, 1);
                        break;
                    case 0x419:
                        Music_Trigger(MUSICTRIG_starfox_rwing_1_e6, 1);
                        ((PlayerState*)inner)->moveSequence = tbl + 0x408;
                        ((PlayerState*)inner)->moveSequenceFlags = 4;
                        ObjAnim_SetCurrentMove(obj, 0x7b, 0.0f, 1);
                        break;
                    case 0x484:
                        Music_Trigger(MUSICTRIG_starfox_rwing_1_e6, 1);
                        ((PlayerState*)inner)->moveSequence = tbl + 0x420;
                        ((PlayerState*)inner)->moveSequenceFlags = 4;
                        ObjAnim_SetCurrentMove(obj, 0xf8, 0.0f, 1);
                        break;
                    default:
                        Music_Trigger(MUSICTRIG_inside_warlock, 1);
                    case 0x714:
                        ((PlayerState*)inner)->moveSequence = tbl + 0x420;
                        ((PlayerState*)inner)->moveSequenceFlags = 4;
                        ObjAnim_SetCurrentMove(obj, 0xf8, 0.0f, 1);
                    }
                    if (arrayIndexOf((int*)(tbl + 0x160), 4, ((GameObject*)va)->anim.seqId) != -1)
                    {
                        (*gPlayerInterface)->setState((void*)obj, inner, 0x1a);
                        *(void (**)(int))((char*)inner + 0x304) = (void (*)(int))fn_8029F67C;
                    }
                    else
                    {
                        (*gPlayerInterface)->setState((void*)obj, inner, 0x18);
                        *(void (**)(int))((char*)inner + 0x304) = (void (*)(int))fn_8029F67C;
                    }
                }
                break;
            }
            case 2:
                if (playerStopRidingObject((GameObject*)obj) != 0)
                {
                    seq->flags |= 4;
                }
                break;
            case 4:
                obj2 = (int)((PlayerState*)inner)->focusObject;
                (*gCameraInterface)->setFocus((void*)obj2, 0);
                (*gObjectTriggerInterface)->setCamVars(0x45, 0, 0, 0);
                ((PlayerState*)inner)->moveSequence = 0;
                if ((u32)obj2 != 0 && ((GameObject*)obj2)->anim.seqId == 0x22)
                {
                    (**(void (**)(int, int, int))((char*)(*gPlayerInterface) + 0x14))(obj, (int)inner, 0x16);
                    *(int*)&((PlayerState*)inner)->baddie.unk304 = 0;
                }
                else
                {
                    (**(void (**)(int, int, int))((char*)(*gPlayerInterface) + 0x14))(obj, (int)inner, 0x18);
                    *(void (**)(int))((char*)inner + 0x304) = (void (*)(int))fn_8029F67C;
                }
                break;
            case 0xb:
            {
                int gb = (int)((PlayerState*)inner)->focusObject;
                if ((u32)gb != 0 && ((GameObject*)gb)->anim.seqId == 0x416)
                {
                    (*gCameraInterface)->setFocus((void*)gb, 0);
                    (*gCameraInterface)->loadTriggeredCamAction(0, 0x69, 0);
                    (*gObjectTriggerInterface)->setCamVars(0x42, 4, 0, 0);
                }
                else if ((u32)gb != 0 && arrayIndexOf((int*)(tbl + 0x160), 4, ((GameObject*)gb)->anim.seqId) != -1)
                {
                    (*gObjectTriggerInterface)->setCamVars(0x53, 0, 0, 0);
                }
                else
                {
                    (*gCameraInterface)->loadTriggeredCamAction(0, 0x1d, 0);
                    (*gObjectTriggerInterface)->setCamVars(0x42, 4, 0, 0);
                }
                break;
            }
            case 6:
                (*gObjectTriggerInterface)->setCamVars(0x44, 0, 0, 0);
                (**(void (**)(int, int, int))((char*)(*gPlayerInterface) + 0x14))(obj, (int)inner, 0x17);
                *(int*)&((PlayerState*)inner)->baddie.unk304 = 0;
                break;
            case 7:
                seq->flags &= ~3;
                obj2 = *(int*)&((GameObject*)obj)->extra;
                (**(void (**)(int, int, int))((char*)(*gPlayerInterface) + 0x14))(obj, obj2, 0x3e);
                *(int*)&((PlayerState*)obj2)->baddie.unk304 = 0;
                *(u32*)(obj2 + 0x360) |= 1LL;
                ((GameObject*)obj)->anim.flags |= 8;
                break;
            case 8:
            {
                seq->flags = seq->savedFlags;
                obj2 = *(int*)&((GameObject*)obj)->extra;
                (**(void (**)(int, int, int))((char*)(*gPlayerInterface) + 0x14))(obj, obj2, 1);
                *(void (**)(int, int))(obj2 + 0x304) = (void (*)(int, int))fn_802A514C;
                *(u32*)((char*)obj2 + 0x360) &= ~0x1LL;
                ((GameObject*)obj)->anim.flags &= ~8;
                break;
            }
            case 0xa:
                if (gPlayerPathObject != NULL && ((u32) * (u8*)((char*)inner + 0x3f4) >> 6 & 1) != 0)
                {
                    ((PlayerState*)inner)->staffActionRequest = 2;
                    ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 0;
                }
                break;
            case 0x18:
                if (gPlayerPathObject != NULL && ((u32) * (u8*)((char*)inner + 0x3f4) >> 6 & 1) != 0)
                {
                    ((PlayerState*)inner)->staffActionRequest = 0;
                    ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 0;
                }
                break;
            case 0xd:
            {
                f32 spd;
                f32 dy2;
                f32 sp3;
                (*gObjectTriggerInterface)
                    ->setObjects(*(s16*)(*(int*)&((GameObject*)obj)->ownerObj + 0x46),
                                 *(int*)&((GameObject*)obj)->ownerObj, 0);
                {
                    int prt = *(int*)&((GameObject*)obj)->ownerObj;
                    obj2 = (int)((GameObject*)prt)->extra;
                    if (*(u32*)&((GameObject*)prt)->anim.hitReactState != 0)
                    {
                        spd = (f32) * (s16*)(*(int*)&((GameObject*)prt)->anim.hitReactState + 0x5a);
                    }
                    else
                    {
                        spd = ((GameObject*)prt)->anim.hitboxScale * ((GameObject*)prt)->anim.rootMotionScale;
                    }
                    dy2 = (((GameObject*)prt)->anim.hitVolumeTransforms->jointY - ((GameObject*)prt)->anim.localPosY) -
                          29.0f;
                }
                sp3 = spd * -mathCosf(gPlayerPi * (f32) ((PlayerState*)obj2)->targetYaw / 32768.0f);
                (*gObjectTriggerInterface)
                    ->setOverridePos(spd * -mathSinf(gPlayerPi * (f32) ((PlayerState*)obj2)->targetYaw / 32768.0f), dy2, sp3);
                (*gObjectTriggerInterface)->runSequence(((GameObject*)obj)->userData1, (void*)obj, -1);
                break;
            }
            case 0xf:
                objHitDetectFn_80062e84((GameObject*)obj, NULL, 1);
                break;
            case 0x10:
            {
                int t;
                nearArg = 400.0f;
                t = ObjGroup_FindNearestObject(6, (GameObject*)obj, &nearArg);
                if ((u32)t != 0)
                {
                    objHitDetectFn_80062e84((GameObject*)obj, (GameObject*)t, 1);
                }
                break;
            }
            case 0x17:
                va = *(int*)&((GameObject*)obj)->extra;
                if (*(u32*)(va + 0x7f8) != 0)
                {
                    *(u8*)(va + 0x800) = 0;
                    {
                        int p17 = *(int*)(va + 0x7f8);
                        if ((u32)p17 != 0)
                        {
                            s16 sp17 = ((GameObject*)p17)->anim.seqId;
                            if (sp17 == 0x3cf || sp17 == 0x662)
                            {
                                objThrowFn_80182504((GameObject*)(p17));
                            }
                            else
                            {
                                objSaveFn_800ea774((GameObject*)p17);
                            }
                            *(s16*)(*(int*)(va + 0x7f8) + 6) &= ~0x4000;
                            *(int*)(*(int*)(va + 0x7f8) + 0xf8) = 0;
                            *(int*)(va + 0x7f8) = 0;
                        }
                    }
                    *(u32*)((char*)va + 0x360) |= 0x800000LL;
                    (**(void (**)(int, int, int))((char*)(*gPlayerInterface) + 0x14))(obj, va, 1);
                    *(void (**)(int, int))(va + 0x304) = (void (*)(int, int))fn_802A514C;
                }
                break;
            case 0x14:
            {
                *(u32*)&((PlayerState*)inner)->flags360 |= 0x40000LL;
                break;
            }
            case 0x15:
            {
                *(u32*)&((PlayerState*)inner)->flags360 &= ~0x40000LL;
                break;
            }
            case 0x16:
            {
                *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_WATER_SPLASH_PENDING;
                break;
            }
            case 0x12:
            {
                *(u32*)&((PlayerState*)inner)->flags360 |= 0x8000LL;
                break;
            }
            case 0x13:
                loadUiDll(1);
                break;
            case 0x19:
                (*gMapEventInterface)->gotoRestartPoint();
                break;
            case 0x1c:
                staffToggle((GameObject*)(obj), 0);
                break;
            case 0x1d:
                (**(void (**)(int, int, int))((char*)(*gPlayerInterface) + 0x14))(obj, (int)inner, 0x1a);
                *(void (**)(int))((char*)inner + 0x304) = (void (*)(int))fn_8029F67C;
                break;
            case 0x1e:
                (**(void (**)(int, int, int))((char*)(*gPlayerInterface) + 0x14))(obj, (int)inner, 1);
                *(void (**)(int, int))((char*)inner + 0x304) = (void (*)(int, int))fn_802A514C;
                break;
            case 0x1f:
                __set_debug_bba((u8*)gPlayerModelChain);
                ObjModelChain_SetEnabled((ObjModelChain*)gPlayerModelChain, 1);
                break;
            case 0x20:
                ObjModelChain_SetEnabled((ObjModelChain*)gPlayerModelChain, 0);
                break;
            case 0x21:
                gPlayerSubState = 2;
                break;
            case 0x22:
                gPlayerSubState = 1;
                break;
            case 0x1a:
                if (*(u32*)&((PlayerState*)inner)->interactObject != 0)
                {
                    int p1a = *(int*)(((PlayerState*)inner)->interactObject + 0x50);
                    int snd = *(s16*)(p1a + 0x7a);
                    if (snd > -1)
                    {
                        (*gGameUIInterface)->showNpcDialogue(snd, 0x154, 300, 0);
                    }
                    else
                    {
                        (*gGameUIInterface)->showNpcDialogue(*(s16*)(p1a + 0x7c), 0x154, 300, 0);
                    }
                }
                break;
            case 1:
                if (*(u32*)&((PlayerState*)inner)->interactObject != 0)
                {
            ObjMsg_SendToObject((void*)((PlayerState*)inner)->interactObject, 0x7000b, (void*)obj, 0);
                    ((PlayerState*)inner)->interactObject = 0;
                }
                break;
            case 0x25:
                ((PlayerState*)inner)->pendingFxFlags ^= 1;
                break;
            case 0x26:
                ((PlayerState*)inner)->pendingFxFlags ^= 2;
                break;
            case 0x27:
                hudFn_8011f38c(1);
                break;
            case 0x28:
            {
                int h;
                int mapVal;
                switch (coordsToMapCell(((GameObject*)obj)->anim.localPosX, ((GameObject*)obj)->anim.localPosZ))
                {
                case 0x13:
                    mapVal = 0x10;
                    break;
                case 0xc:
                    mapVal = 0x14;
                    break;
                case 0xd:
                    mapVal = 0x18;
                    break;
                case 2:
                    mapVal = 0x1c;
                    break;
                }
                h = *(int*)&((GameObject*)obj)->extra;
                if ((s8) * (s8*)(*(int*)(h + 0x35c) + 1) <= mapVal - 4)
                {
                    int vv = mapVal;
                    if (mapVal < 0)
                    {
                        vv = 0;
                    }
                    else if (mapVal > 0x50)
                    {
                        vv = 0x50;
                    }
                    *(s8*)(*(int*)(h + 0x35c) + 1) = vv;
                    vv = mapVal;
                    h = *(int*)&((GameObject*)obj)->extra;
                    if (mapVal < 0)
                    {
                        vv = 0;
                    }
                    else
                    {
                        s8 cur2 = *(s8*)(*(int*)(h + 0x35c) + 1);
                        if (mapVal > cur2)
                        {
                            vv = cur2;
                        }
                    }
                    *(s8*)(*(int*)(h + 0x35c)) = vv;
                }
                break;
            }
            case 0x29:
                hudFn_8011f38c(0);
                break;
            case 0x2a:
                if ((*gMapEventInterface)->getMapAct(0xb) == 7)
                {
                    getEnvfxActImmediately((void*)obj, (void*)obj, 0x1fb, 0);
                    getEnvfxActImmediately((void*)obj, (void*)obj, 0x1ff, 0);
                    getEnvfxActImmediately((void*)obj, (void*)obj, 0x249, 0);
                    getEnvfxActImmediately((void*)obj, (void*)obj, 0x1fd, 0);
                }
                else
                {
                    getEnvfxActImmediately((void*)obj, (void*)obj, 0x217, 0);
                    getEnvfxActImmediately((void*)obj, (void*)obj, 0x216, 0);
                    getEnvfxActImmediately((void*)obj, (void*)obj, 0x22e, 0);
                    getEnvfxActImmediately((void*)obj, (void*)obj, 0x218, 0);
                    getEnvfxActImmediately((void*)obj, (void*)obj, 0x84, 0);
                    getEnvfxActImmediately((void*)obj, (void*)obj, 0x8a, 0);
                }
                skyFn_80088e54(0, 0.0f);
                break;
            case 0x2d:
                Rcp_SetSpiritVisionEnabled(1);
                break;
            case 0x2e:
                Rcp_SetSpiritVisionEnabled(0);
                break;
            case 0x2b:
            {
                register u32 m;
                m = ((GameObject*)obj)->anim.modelState->flags;
                m &= ~OBJ_MODEL_STATE_SHADOW_VISIBLE;
                ((GameObject*)obj)->anim.modelState->flags = m;
                break;
            }
            case 0x2c:
                ((GameObject*)obj)->anim.modelState->flags |= OBJ_MODEL_STATE_SHADOW_VISIBLE;
                break;
            case 0x31:
                viewFinderSetZoomTo50();
                break;
            case 0x32:
                viewFinderSetZoom(Camera_GetFovY());
                break;
            }
        }
        if (*(int*)(*(int*)&((GameObject*)obj)->extra + 0x360) & 1)
        {
            seq->flags &= ~3;
        }
    }
    if (lbl_803DE458 != 0)
    {
        seq->sequenceControlFlags |= OBJSEQ_CONTROL_SET_LATCH_A;
        lbl_803DE458 = 0;
    }
    {
        int g = (int)((PlayerState*)inner)->focusObject;
        if ((u32)g != 0 && (*(int (*)(int)) * (int*)((char*)*(int*)(*(int*)(g + 0x68)) + 0x38))(g) == 2)
        {
            seq->flags &= ~3;
        }
    }
    if (((u32) * (u8*)((char*)inner + 0x3f2) >> 6 & 1) != 0)
    {
        characterDoEyeAnims((GameObject*)obj, (char*)inner + 0x364);
    }
    if (gPlayerSubState == 2)
    {
        gPlayerSubState = 1;
    }
    if (((GameObject*)gPlayerPathObject)->anim.classId == 0x2d)
    {
        objSetAnimField48to0((GameObject*)gPlayerPathObject);
    }
    staffAnimate(obj, inner, timeDelta);
    if (gPlayerPathObject != NULL && ((u32) * (u8*)((char*)inner + 0x3f4) >> 6 & 1) != 0)
    {
        ((GameObject*)gPlayerPathObject)->objectFlags &= ~7;
        if (((PlayerState*)inner)->staffGrown == 0)
        {
            ((GameObject*)gPlayerPathObject)->objectFlags |= 2;
        }
    }
    *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
    objAudioFn_8006ef38((GameObject*)obj, &seq->animEvents, ((PlayerState*)inner)->animSoundId,
                        (void*)((char*)inner + 0x3c4), (void*)((char*)inner + 4),
                        ((PlayerState*)inner)->baddie.animSpeedA, 1.0f);
    return result;
}

void fn_802B4A9C(GameObject* obj, int inner, int inner2)
{
    int* target = (int*)(*gCameraInterface)->getOverrideTarget();
    u32 v = (((PlayerState*)inner)->flags3F4 >> 6) & 1;

    if (v != 0)
    {
        if ((*(u32*)&((PlayerState*)inner)->flags360 & 0x10) != 0)
        {
            if (gPlayerPathObject != NULL && v != 0)
            {
                ((PlayerState*)inner)->staffActionRequest = 2;
                ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 0;
            }
            ((PlayerState*)inner2)->baddie.hasTarget = 1;
            if (target != NULL)
            {
                *(int**)&((PlayerState*)inner2)->baddie.targetObj = target;
            }
            else
            {
                f32 dist = lbl_803E8150;
                *(int*)&((PlayerState*)inner2)->baddie.targetObj = ObjGroup_FindNearestObject(3, (GameObject*)obj, &dist);
            }
        }
        else
        {
            if (target != NULL)
            {
                if (*(int**)&((PlayerState*)inner2)->baddie.targetObj != target)
                {
                    ((PlayerState*)inner2)->baddie.hasTarget = 0;
                    if ((((GameObject*)target)->anim.hitVolumeBounds->flags & 0xf) == 1)
                    {
                        if (gPlayerPathObject != NULL)
                        {
                            u32 targetFlag = (((PlayerState*)inner)->flags3F4 >> 6) & 1;
                            if (targetFlag != 0)
                            {
                                ((PlayerState*)inner)->staffActionRequest = 2;
                                ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 0;
                            }
                        }
                        ((PlayerState*)inner2)->baddie.hasTarget = 1;
                    }
                }
                *(int**)&((PlayerState*)inner2)->baddie.targetObj = target;
            }
            else
            {
                *(int*)&((PlayerState*)inner2)->baddie.targetObj = 0;
                ((PlayerState*)inner2)->baddie.hasTarget = 0;
            }
        }
        if (*(int**)&((PlayerState*)inner2)->baddie.targetObj != NULL)
        {
            fn_8014C540((GameObject*)((PlayerState*)inner2)->baddie.targetObj, (int*)&((PlayerState*)inner)->flags884,
                        &((PlayerState*)inner)->animSpeedDecay, &((PlayerState*)inner)->animSpeedStart);
        }
        else
        {
            ((PlayerState*)inner)->deferredItemCommand = -1;
        }
    }
}

void playerAnimate(GameObject* obj, int state, f32 fv)
{
    u8 buf[0x40];

    ((PlayerState*)state)->baddie.gravity = lbl_803E7EB4;
    ((PlayerState*)state)->baddie.moveInputX = ((PlayerState*)state)->stickXf;
    ((PlayerState*)state)->baddie.moveInputZ = ((PlayerState*)state)->stickYf;
    *(int*)&((PlayerState*)state)->baddie.unk31C = ((PlayerState*)state)->buttonsJustPressed;
    *(int*)&((PlayerState*)state)->baddie.unk318 = ((PlayerState*)state)->buttonsHeld;
    Player_GetObjHitsState(obj)->hitVolumePriority = 0;
    Player_GetObjHitsState(obj)->hitVolumeId = 0;
    Player_GetObjHitsState(obj)->objectPairPriority = 0;
    Player_GetObjHitsState(obj)->objectPairHitVolume = 0;
    ((PlayerState*)state)->baddie.physicsActive = 1;
    *(u32*)((char*)state + 0x4) &= ~0x8100000;
    playerShadowClearPositionOverride(obj);
    ((PlayerState*)state)->emissionState = 0;
    *(u32*)&((PlayerState*)state)->flags360 &= ~PLAYER_FLAG_NO_POS_VELOCITY;
    *(int*)state |= 0x1000000;
    fn_802B0EA4(obj, state, state);
    if ((s8)playerCheckIfClimbingOntoWall((int)obj, state, state, buf, fv, 0x60) == 8)
    {
        *(int*)&((PlayerState*)state)->baddie.targetObj = 0;
        ((PlayerState*)state)->baddie.hasTarget = 0;
        (*gCameraInterface)->setTarget(0);
        if (gPlayerPathObject != 0 && ((ByteFlags*)((char*)state + 0x3f4))->b40)
        {
            ((PlayerState*)state)->staffActionRequest = 1;
            ((ByteFlags*)((char*)state + 0x3f4))->b08 = 1;
        }
        (*gPlayerInterface)->setState(obj, (void*)state, 0xa);
        *(int*)&((PlayerState*)state)->baddie.unk304 = 0;
    }
    (*gPlayerInterface)->update(obj, (void*)state, fv, fv, gPlayerStateHandlers, &gPlayerDefaultStateHandler);
    *(int*)state &= ~0x1000000;
}

void fn_802B4DE0(GameObject* obj, int p2)
{
    int off;
    int i;
    PlayerState* inner = obj->extra;

    if ((u32)gPlayerEggObject != 0)
    {
        Obj_FreeObject((GameObject*)gPlayerEggObject);
        ObjLink_DetachChild(obj, (GameObject*)gPlayerEggObject);
        gPlayerEggObject = 0;
    }
    if (gPlayerPathObject != NULL)
    {
        Obj_FreeObject((GameObject*)gPlayerPathObject);
        ObjLink_DetachChild(obj, (GameObject*)gPlayerPathObject);
        gPlayerPathObject = NULL;
    }
    if (gPlayerStaffObject != NULL)
    {
        gPlayerStaffObject = NULL;
    }
    for (i = 0, off = 0; i < inner->moveSlotCount; i++)
    {
        int e = *(int*)(inner->moveSlots + off + 0x64);
        if ((u32)e != 0)
            mm_free((void*)e);
        off += 0xb0;
    }
    ObjGroup_RemoveObject((int)obj, 0);
    ObjGroup_RemoveObject((int)obj, PLAYER_OBJGROUP);
    ObjModelChain_Free((ObjModelChain*)gPlayerModelChain);
}


extern PlayerModelChainEntry lbl_803DC660;
extern PlayerModelChainEntry* gPlayerModelChainConfig;

extern PlayerMoveSlot gPlayerMoveSlotData[28];

extern PlayerAnimSpeedTuning gPlayerAnimSpeedThresholds;

extern s16 gPlayerMoveTableA[96];

extern s16 gPlayerSpellGameBits[52];

extern s16 lbl_80333110[128];
extern s16 gPlayerMoveTableB[14];
extern u8 gPlayerSurfacePfxModeTable[36];
extern f32 gPlayerDefaultMoveParams[24];
extern PlayerMotionTuning lbl_803332B0;
extern s16 lbl_8033366C[24];
extern f32 lbl_8033369C[8];
extern s16 gPlayerMoveSlotTable[44];

extern int gPlayerStateHandlers[66];
extern f32 lbl_803DAF88[16];
extern LightmapVertex gPlayerHudVtxBuf[8];
extern f32 gPlayerPartFxParams[6];

