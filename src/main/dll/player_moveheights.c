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

void fn_802AABE4(int obj)
{
    s16* moveId;
    f32* outputHeight;
    ObjModel* model;
    GameObject* object;
    PlayerState* player;
    s16* moveTable;
    s16 moveIndex;
    s16 jointRotation[3];
    f32 jointPosition[3];

    object = (GameObject*)obj;
    model = object->anim.modelBanks[object->anim.bankIndex];
    player = object->extra;
    moveTable = player->moveAnimIds;

    ObjAnim_SetCurrentMove(obj, moveTable[0], lbl_803E7EA4, 0);
    ObjModel_SampleJointTransform(model, 0, 0, lbl_803E7EA4, object->anim.rootMotionScale, jointPosition, jointRotation);
    lbl_803DAF88[0] = jointPosition[1];

    ObjAnim_SetCurrentMove(obj, lbl_80332F2C[0], lbl_803E7EA4, 0);
    ObjModel_SampleJointTransform(model, 0, 0, lbl_803E7EA4, object->anim.rootMotionScale, jointPosition, jointRotation);
    lbl_803DAF88[1] = jointPosition[1];

    moveIndex = 12;
    moveId = &lbl_80332F48[17];
    outputHeight = &lbl_803DAF88[moveIndex];
    for (; moveIndex <= 15; moveIndex++)
    {
        ObjAnim_SetCurrentMove(obj, moveId[0], lbl_803E7EA4, 0);
        ObjModel_SampleJointTransform(model, 0, 0, lbl_803E7EA4, object->anim.rootMotionScale, jointPosition, jointRotation);
        outputHeight[0] = jointPosition[1];
        moveId++;
        outputHeight++;
    }
    ObjAnim_WriteStateWord(&object->anim, OBJANIM_STATE_INDEX_CURRENT, OBJANIM_STATE_WORD_EVENT_COUNTDOWN, 0);
}

void playerDrawTeleportAnim(GameObject* obj);

void fn_802AAF80(GameObject* obj, int inner, int a, int b, int c);

int fn_802AB1D0(GameObject* obj);

void playerCastSpell(int a, int b, int c);

void fn_802AB5A4(GameObject* obj, int p2, int flags);

void playerCalcWaterCurrent(f32* outX, f32* outZ, f32 p3, int player);

int fn_802ABAE8(GameObject* obj, int state, int inner, f32 fv);

void fn_802ABFBC(GameObject* obj, int state, PlayerState* inner);

void fn_802AC32C(int p1, int p2, int p3);

int fn_802AC7DC(int obj, int state, int inner, f32 fv);

void playerSetMovingAnims(int p1, int obj);

int fn_802AD2F4(GameObject* obj, int inner, int state);

int fn_802ADC08(GameObject* obj, int inner, int p3);

void fn_802ADE80(GameObject* obj, int inner, int state);

int fn_802AE480(GameObject* obj, int inner, int state);

void fn_802AE650(GameObject* obj, int state, int p3);

void fn_802AE83C(int obj, int inner, int state);

void fn_802AE9C8(GameObject* obj, int inner, int state);

void fn_802AED2C(GameObject* obj, int state, int p3);

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

void staffAnimate(int obj, void* state, f32 dt);

void playerProcessQueuedItemCommand(GameObject* obj, int state);

void playerRunActiveSpells(GameObject* obj, int state);

void fn_802AFB0C(int obj, int inner, int state);

void fn_802B066C(GameObject* obj, int state);

void playerStaffInit(GameObject* obj, int state);

void playerDoEyeAnims(GameObject* obj, int state);

void fn_802B0EA4(GameObject* obj, int inner, int state);

void fn_802B18BC(GameObject* obj, int state, f32 fv);

void playerDoControls(GameObject* obj, int state, f32 fv);

void fn_802B1B28(GameObject* obj, f32 fv);

void fn_802B1BF8(GameObject* a, int b, int state, f32 unusedTimeDelta);

void fn_802B1E5C(GameObject* obj, int state, int cfg, f32 dt);

void playerItemGetAnimFn(int obj, int inner, int state);

int player_SeqFn(int obj, int obj2, ObjSeqState* seq, int endFlag);

void fn_802B4A9C(GameObject* obj, int inner, int inner2);

void playerAnimate(GameObject* obj, int state, f32 fv);

void fn_802B4DE0(GameObject* obj, int p2);


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

