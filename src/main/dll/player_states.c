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

int playerState1C(GameObject* obj, int state)
{
    PlayerState* inner = obj->extra;
    f32 k;
    f32 a, b;
    u8 s1, s2;

    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        *(s16*)((char*)state + 0x278) = 0x1c;
        inner->stateHandler = 0;
    }
    k = lbl_803E7EA4;
    ((PlayerState*)state)->baddie.animSpeedC = k;
    ((PlayerState*)state)->baddie.animSpeedB = k;
    ((PlayerState*)state)->baddie.animSpeedA = k;
    obj->anim.velocityX = k;
    obj->anim.velocityY = k;
    obj->anim.velocityZ = k;
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        s1 = 0;
        a = inner->surfaceNormalX;
        if (a < k)
        {
            s1 = 1;
            a = -a;
        }
        s2 = 0;
        b = inner->surfaceNormalZ;
        if (b < lbl_803E7EA4)
        {
            s2 = 1;
            b = -b;
        }
        if (a > b)
        {
            if (s1)
            {
                inner->surfaceDir = 0;
            }
            else
            {
                inner->surfaceDir = 1;
            }
        }
        else
        {
            if (s2)
            {
                inner->surfaceDir = 2;
            }
            else
            {
                inner->surfaceDir = 3;
            }
        }
        ObjAnim_SetCurrentMove((int)obj, 0x57, lbl_803E7EA4, 0);
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7FE8;
        Sfx_PlayFromObject((int)obj, (u16)(inner->characterId == 0 ? SFXTRIG_impact3 : SFXTRIG_literun116));
    }
    if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
    {
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
        return -1;
    }
    return 0;
}

int playerState1B(GameObject* obj, int state, f32 fv)
{
    PlayerState* inner = obj->extra;
    int curveId;
    int camArg = 0;
    f32 vec[3];
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        *(s16*)((char*)state + 0x278) = 0x1b;
        inner->stateHandler = (int)objUpdateHitboxPos;
        ObjHits_MarkObjectPositionDirty((ObjAnimComponent*)obj);
    }
    {
        int in2 = *(int*)&obj->extra;
        *(int*)((char*)in2 + 0x360) &= ~2LL;
        *(u32*)((char*)in2 + 0x360) |= 0x2000LL;
    }
    *(int*)((char*)state + 0x4) |= 0x100000;
    {
        f32 zero = lbl_803E7EA4;
        ((PlayerState*)state)->baddie.animSpeedA = zero;
        ((PlayerState*)state)->baddie.animSpeedB = zero;
        *(int*)((char*)state + 0) |= 0x200000;
        obj->anim.velocityX = zero;
        obj->anim.velocityZ = zero;
        ((PlayerState*)state)->baddie.physicsActive = 0;
        obj->anim.velocityY = zero;
    }
    switch (obj->anim.currentMove)
    {
    case 0x76:
    case 0x40d:
    {
        int active;
        int atDest;
        f32 amt = ((PlayerState*)state)->baddie.moveInputZ / lbl_803E7FA8;
        f32 clamped;
        f32 sp;
        f32 spd;
        if (amt < lbl_803E7EA4)
        {
            amt = -amt;
        }
        clamped = (amt < lbl_803E7EFC) ? lbl_803E7EFC : ((amt > lbl_803E7EE0) ? lbl_803E7EE0 : amt);
        sp = ((PlayerState*)state)->baddie.moveInputZ;
        if (sp > lbl_803E7EE0)
        {
            spd = lbl_803E7F44 * clamped;
            active = 1;
        }
        else if (sp < lbl_803E7ECC)
        {
            spd = lbl_803E7F44 * -clamped;
            active = 1;
        }
        else
        {
            spd = 0.0f;
            active = 0;
        }
        if (active != 0)
        {
            gPlayerSfxTimerC = gPlayerSfxTimerC - framesThisStep;
            if (gPlayerSfxTimerC <= 0)
            {
                gPlayerSfxTimerC = randomGetRange(0x1e, 0x2d);
                Sfx_PlayFromObject(0, SFXTRIG_foot_ladder3);
            }
        }
        ((PlayerState*)state)->baddie.animSpeedC =
            ((PlayerState*)state)->baddie.animSpeedC +
            interpolate(spd - ((PlayerState*)state)->baddie.animSpeedC, lbl_803E7EFC, timeDelta);
        inner->traveledDistance = ((PlayerState*)state)->baddie.animSpeedC * timeDelta + inner->traveledDistance;
        {
            f32 ph = ((PlayerState*)state)->baddie.animSpeedC;
            if (ph < lbl_803E7EF8 && ph > lbl_803E7FEC)
            {
                f32 zeroPh = lbl_803E7EA4;
                ((PlayerState*)state)->baddie.animSpeedC = zeroPh;
                if (obj->anim.currentMove != 0x76)
                {
                    ObjAnim_SetCurrentMove((int)obj, 0x76, zeroPh, 0);
                }
                ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F78;
            }
            else
            {
                if (obj->anim.currentMove != 0x40d)
                {
                    ObjAnim_SetCurrentMove((int)obj, 0x40d, lbl_803E7EA4, 0);
                }
                ObjAnim_SampleRootCurvePhase((ObjAnimComponent*)obj, ((PlayerState*)state)->baddie.animSpeedC,
                                             (f32*)((char*)state + 0x2a0));
            }
        }
        atDest = inner->traveledDistance > inner->travelTargetDistance || inner->traveledDistance < lbl_803E7EA4;
        if (atDest)
        {
            u8 anim;
            ObjAnim_SetCurrentMove((int)obj, 0x40f, lbl_803E7EA4, 0);
            anim = inner->curAnimId;
            if (anim != 0x48 && anim != 0x47)
            {
                camArg = inner->traveledDistance < lbl_803E7EA4 ? 0 : 1;
                (*(void (*)(int*))(*(int*)((char*)*gCameraInterface + 0x60)))(&camArg);
            }
        }
        else
        {
            inner->targetYaw = (s16)getAngle(-*(f32*)((char*)inner + 0x634), -inner->travelDirZ);
            inner->yaw = inner->targetYaw;
            obj->anim.rotY = 0;
        }
        break;
    }
    case 0x40f:
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F34;
        (*gPlayerInterface)->updateAnimRootMotion(obj, (void*)state, fv, 1);
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            u8 anim = inner->curAnimId;
            if (anim != 0x48 && anim != 0x47)
            {
                (*gCameraInterface)->setMode(0x42, 1, 1, 0, NULL, 0, 0xff);
            }
            *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
            return 2;
        }
        break;
    case 0x40e:
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F34;
        (*gPlayerInterface)->updateAnimRootMotion(obj, (void*)state, fv, 1);
        inner->targetYaw = (s16)getAngle(inner->hitNormalX, inner->hitNormalZ);
        inner->yaw = inner->targetYaw;
        sqrtf(inner->hitNormalX * inner->hitNormalX + inner->hitNormalZ * inner->hitNormalZ);
        obj->anim.rotY = 0;
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            ObjAnim_SetCurrentMove((int)obj, 0x40d, lbl_803E7EA4, 0);
        }
        break;
    default:
    {
        int found;
        curveId = 0x1f;
        found = (*gRomCurveInterface)->find(
            obj->anim.localPosX, obj->anim.localPosY, obj->anim.localPosZ,
            &curveId, 1, 0);
        if (found != -1)
        {
            int pt = (int)(*gRomCurveInterface)->getById(found);
            int pt2;
            *(f32*)((int)inner + 0x61c) = ((ObjHitVolumeRuntimeTransform*)pt)->jointZ;
            inner->curveStartY = ((ObjHitVolumeRuntimeTransform*)pt)->centerX;
            inner->curveStartZ = ((ObjHitVolumeRuntimeTransform*)pt)->centerY;
            obj->anim.localPosX = ((ObjHitVolumeRuntimeTransform*)pt)->jointZ;
            obj->anim.localPosY = ((ObjHitVolumeRuntimeTransform*)pt)->centerX;
            obj->anim.localPosZ = ((ObjHitVolumeRuntimeTransform*)pt)->centerY;
            inner->targetYaw = (s16)getAngle(inner->hitNormalX, inner->hitNormalZ);
            inner->yaw = inner->targetYaw;
            sqrtf(inner->hitNormalX * inner->hitNormalX + inner->hitNormalZ * inner->hitNormalZ);
            obj->anim.rotY = 0;
            found = (*gRomCurveInterface)->getRandomUnblockedLink((RomCurveDef*)pt, -1);
            if (found == -1)
            {
                found = (*gRomCurveInterface)->getRandomBlockedLink((RomCurveDef*)pt, -1);
            }
            pt2 = (int)(*gRomCurveInterface)->getById(found);
            *(f32*)((int)inner + 0x628) = *(f32*)((char*)pt2 + 0x8);
            inner->curveEndY = *(f32*)((char*)pt2 + 0xc);
            inner->curveEndZ = *(f32*)((char*)pt2 + 0x10);
            inner->traveledDistance = lbl_803E7EA4;
            PSVECSubtract((f32*)((char*)inner + 0x628), (f32*)((char*)inner + 0x61c), vec);
            inner->travelTargetDistance = PSVECMag(vec);
            PSVECNormalize(vec, (f32*)((char*)inner + 0x634));
        }
        ObjAnim_SetCurrentMove((int)obj, 0x40e, lbl_803E7EA4, 0);
        {
            u8 anim = inner->curAnimId;
            if (anim != 0x48 && anim != 0x47)
            {
                (*gCameraInterface)->setMode(0x50, 1, 0, 0, NULL, 0x28, 0xff);
            }
        }
        ((PlayerState*)state)->baddie.animSpeedC = lbl_803E7EA4;
        break;
    }
    }
    PSVECScale((f32*)((char*)inner + 0x634), vec, inner->traveledDistance);
    PSVECAdd((f32*)((char*)inner + 0x61c), vec, &obj->anim.localPosX);
    fn_802AB5A4(obj, (int)inner, 7);
    return 0;
}

int playerStateOnCloudRunner(GameObject* obj, int state)
{
    PlayerState* inner = obj->extra;
    void* sub;
    f32 v7b8, v7bc;
    f32 k;
    int res, halfW, halfH;

    *(u32*)&inner->flags360 &= ~PLAYER_FLAG_HITDETECT;
    ObjHits_EnableObject(obj);
    sub = *(void**)((char*)inner + 0x7f0);
    if (sub == NULL)
    {
        f32 z = lbl_803E7EA4;
        ((PlayerState*)state)->baddie.animSpeedC = z;
        ((PlayerState*)state)->baddie.animSpeedB = z;
        ((PlayerState*)state)->baddie.animSpeedA = z;
        obj->anim.velocityX = z;
        obj->anim.velocityY = z;
        obj->anim.velocityZ = z;
        ObjHits_EnableObject(obj);
    }
    else
    {
        if (*(s16*)((char*)sub + 0x46) != 0x714)
        {
            ObjHits_DisableObject(obj);
        }
    }
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        f32 z = lbl_803E7EA4;
        inner->aimInputX = z;
        inner->aimInputZ = z;
        (*gCameraInterface)->setMode(0x53, 1, sub != NULL ? 0x12 : -2, 0, NULL, 0, 0xff);
        ObjAnim_SetCurrentMove((int)obj, 0x43e, lbl_803E7EA4, 0);
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F34;
        inner->actionCooldown = lbl_803E7EA4;
        if (gPlayerPathObject != NULL)
        {
            if (((ByteFlags*)((char*)inner + 0x3f4))->b40 != 0)
            {
                inner->staffActionRequest = 4;
                ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
            }
        }
    }
    if (obj->anim.alpha > 1)
    {
        obj->anim.alpha = 1;
    }
    inner->actionCooldown = inner->actionCooldown - timeDelta;
    if (inner->actionCooldown < lbl_803E7EA4)
    {
        inner->actionCooldown = *(f32*)&lbl_803E7EA4;
    }
    if ((inner->buttonsJustPressed & PAD_BUTTON_A) != 0)
    {
        if (inner->actionCooldown <= lbl_803E7EA4)
        {
            buttonDisable(0, PAD_BUTTON_A);
            fn_802AA014(obj, state, inner->aimInputZ, lbl_803E7EA4);
            inner->actionCooldown = lbl_803E7F10;
        }
    }
    {
        f32 x = ((PlayerState*)state)->baddie.moveInputZ / lbl_803E7FA8;
        f32 c;
        void* hit;
        c = (x < lbl_803E7FF0) ? lbl_803E7FF0 : ((x > lbl_803E7FC4) ? lbl_803E7FC4 : x);
        hit = *(void**)((char*)inner + 0x7f0);
        if (hit != NULL && *(s16*)((char*)hit + 0x46) == 0x484)
        {
            c = c + lbl_803DC6E0;
        }
        if (hit == NULL)
        {
            c = c + lbl_803DC6E4;
        }
        inner->aimInputZ += interpolate(c - inner->aimInputZ, lbl_803DC6D4, timeDelta);
    }
    {
        f32 x = ((PlayerState*)state)->baddie.moveInputX / lbl_803E7FA8;
        f32 c;
        c = (x < lbl_803E7ECC) ? lbl_803E7ECC : ((x > lbl_803E7EE0) ? lbl_803E7EE0 : x);
        inner->aimInputX += interpolate(c - inner->aimInputX, lbl_803DC6D8, timeDelta);
    }
    {
        f32 d = inner->aimInputX;
        if (d > lbl_803E7EA4)
        {
            d = d - lbl_803E7EA0;
            if (d < lbl_803E7EA4)
            {
                d = lbl_803E7EA4;
            }
        }
        else
        {
            d = lbl_803E7EA0 + d;
            if (d > lbl_803E7EA4)
            {
                d = lbl_803E7EA4;
            }
        }
        {
            f32 p = lbl_803E7FB4 * d;
            inner->targetYaw = (s16)(p * lbl_803DC6DC + (f32)inner->targetYaw);
        }
        inner->yaw = inner->targetYaw;
    }
    if (inner->aimInputZ > lbl_803E7EA4)
    {
        Object_ObjAnimSetSecondaryBlendMove((ObjAnimComponent*)obj, 0x441, (int)(lbl_803E7FAC * inner->aimInputZ));
    }
    else
    {
        Object_ObjAnimSetSecondaryBlendMove((ObjAnimComponent*)obj, 0x440, (int)(lbl_803E7FAC * -inner->aimInputZ));
    }
    inner->headPitch = (f32)inner->headPitch * powfBitEstimate(lbl_803E7FF4, timeDelta);
    inner->headYaw = (f32)inner->headYaw * powfBitEstimate(lbl_803E7F1C, timeDelta);
    inner->bodyLeanHalf = lbl_803E7FB0 * inner->aimInputX;
    inner->bodyLeanAngle = (s16)(inner->bodyLeanHalf >> 1);
    *(u32*)&inner->flags360 &= ~PLAYER_FLAG_AIM_READY;
    v7bc = inner->aimInputZ;
    v7b8 = inner->aimInputX;
    res = getScreenResolution();
    halfW = res >> 17;
    halfH = (int)(u16)res >> 1;
    inner->aimScreenY = (k = lbl_803E7E98) * (v7b8 * (f32)halfH) + (f32)halfH;
    if (v7bc < lbl_803E7EA4)
    {
        inner->aimScreenX = k * (v7bc * (f32)halfW) + (f32)halfW;
    }
    else
    {
        inner->aimScreenX = lbl_803E7F44 * (v7bc * (f32)halfW) + (f32)halfW;
    }
    *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_AIM_READY;
    return 0;
}

int playerState19(GameObject* obj, int state)
{
    PlayerState* inner = obj->extra;
    int sub = (int)inner->focusObject;
    void* vec;
    int kind;
    ObjModel* joint;
    int n;
    f32 t;
    f32 pos1[3];
    f32 pos2[3];
    s16 ang[3];
    f32 localPt;
    f32 cam[3];

    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        *(s16*)((char*)state + 0x278) = 0x19;
        inner->stateHandler = 0;
    }
    {
        int inner2 = *(int*)&obj->extra;
        *(int*)((char*)inner2 + 0x360) &= ~0x2LL;
        *(int*)((char*)inner2 + 0x360) |= 0x2000;
    }
    *(int*)((char*)state + 0x4) |= 0x100000;
    {
        f32 z = lbl_803E7EA4;
        ((PlayerState*)state)->baddie.animSpeedA = z;
        ((PlayerState*)state)->baddie.animSpeedB = z;
        *(int*)((char*)state + 0x0) |= 0x200000;
        obj->anim.velocityX = z;
        obj->anim.velocityZ = z;
    }
    ((PlayerState*)state)->baddie.physicsActive = 0;
    ObjHits_DisableObject(obj);
    obj->anim.velocityY = lbl_803E7EA4;
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        (*(void (*)(int, void*, void*, void*))(*(int*)(*(int*)*(int*)((char*)sub + 0x68) + 0x28)))(
            sub, (char*)obj + 0xc, (char*)obj + 0x10, (char*)obj + 0x14);
        switch (*(s16*)((char*)sub + 0x46))
        {
        case 0x38c:
        case 0x72:
            (*gCameraInterface)->setMode(0x42, 0, 1, 0, NULL, 0x64, 0xff);
            break;
        default:
            (*gCameraInterface)->loadTriggeredCamAction(0, 1, 0);
            break;
        }
        kind = (*(int (*)(int))(*(int*)(*(int*)*(int*)((char*)sub + 0x68) + 0x30)))(sub);
        (*(void (*)(int, int))(*(int*)(*(int*)*(int*)((char*)sub + 0x68) + 0x3c)))(sub, 3);
        switch (kind)
        {
        case 1:
            n = 8;
            break;
        case 2:
        default:
            n = 9;
            break;
        }
        inner->targetYaw = *(s16*)((char*)sub + 0x0);
        inner->yaw = inner->targetYaw;
        obj->anim.rotY = 0;
        obj->anim.rotZ = 0;
        ObjAnim_SetCurrentMove((int)obj, ((s16*)inner->moveSequence)[n], lbl_803E7EA4, 1);
        joint = Player_GetActiveModel((int)obj);
        ObjModel_SampleJointTransform(joint, 0, 0, lbl_803E7EA4, obj->anim.rootMotionScale, pos1, ang);
        ObjModel_SampleJointTransform(joint, 0, 0, lbl_803E7EE0, obj->anim.rootMotionScale, pos2, ang);
        ang[0] = inner->targetYaw;
        ang[1] = 0;
        ang[2] = 0;
        vecRotateZXY(ang, pos2);
        pos2[0] = pos2[0] + obj->anim.localPosX;
        pos2[2] = pos2[2] + obj->anim.localPosZ;
        obj->anim.localPosY -= pos1[1];
        t = (*gPathControlInterface)
                ->sampleHeight((void*)obj, pos2[0], obj->anim.localPosY, pos2[2], lbl_803E7FA4);
        inner->warpStartX = pos2[0];
        inner->warpStartY = t;
        inner->warpStartZ = pos2[2];
        inner->warpDeltaY = obj->anim.localPosY - t;
        inner->warpKind = (u8)kind;
        obj->anim.flags &= ~0x8;
        obj->anim.activeMove = -1;
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7FE8;
    }
    t = lbl_803E7EE0 - obj->anim.currentMoveProgress;
    obj->anim.localPosY = inner->warpDeltaY * t + inner->warpStartY;
    vec = objModelGetVecFn_800395d8(obj, 5);
    if (vec != NULL)
    {
        *(s16*)vec = (f32) * (s16*)((char*)sub + 0x2) * t;
        *(s16*)((char*)vec + 0x4) = (f32) * (s16*)((char*)sub + 0x4) * t;
    }
    (*(void (*)(int, f32*, f32*, f32*))(*(int*)(*(int*)*(int*)((char*)sub + 0x68) + 0x34)))(sub, &cam[0], &cam[1],
                                                                                            &cam[2]);
    {
        f32 w = obj->anim.currentMoveProgress;
        f32 cx = w * (inner->warpStartX - cam[0]) + cam[0];
        f32 cy = w * (inner->warpStartY - cam[1]) + cam[1];
        f32 cz = w * (inner->warpStartZ - cam[2]) + cam[2];
        (*gCameraInterface)->overridePos(cx, cy, cz);
    }
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA == 0 &&
        *(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
    {
        if (vec != NULL)
        {
            *(s16*)vec = 0;
            *(s16*)((char*)vec + 0x4) = 0;
        }
        obj->anim.modelState->flags &= ~(long long)OBJ_MODEL_STATE_SHADOW_FADE_OUT;
        obj->anim.worldPosX = inner->savedPosX;
        obj->anim.worldPosZ = inner->savedPosZ;
        if (obj->anim.parent != NULL)
        {
            obj->anim.worldPosX += playerMapOffsetX;
            obj->anim.worldPosZ += playerMapOffsetZ;
        }
        Obj_TransformWorldPointToLocal(obj->anim.worldPosX, lbl_803E7EA4,
                                       obj->anim.worldPosZ, &obj->anim.localPosX,
                                       &localPt, &obj->anim.localPosZ,
                                       (int)obj->anim.parent);
        if (inner->warpKind == 1)
        {
            inner->targetYaw += 0x4000;
            inner->yaw = inner->targetYaw;
        }
        else
        {
            inner->targetYaw -= 0x4000;
            inner->yaw = inner->targetYaw;
        }
        ObjAnim_SetCurrentMove((int)obj, 0, lbl_803E7EA4, 1);
        ObjAnim_WriteStateWord((ObjAnimComponent*)obj, OBJANIM_STATE_INDEX_CURRENT,
                               OBJANIM_STATE_WORD_EVENT_COUNTDOWN, 0);
        (*(void (*)(int, int))(*(int*)(*(int*)*(int*)((char*)sub + 0x68) + 0x3c)))(sub, 0);
        fn_802AB5A4(obj, (int)inner, 7);
        ObjHits_EnableObject(obj);
        inner->focusObject = NULL;
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
        return 2;
    }
    return 0;
}

void fn_8029F67C(GameObject* obj)
{
    ObjModelState* modelState = obj->anim.modelState;
    s16* v;
    modelState->flags &= 0xFFFFEFFFLL;
    obj->anim.flags &= ~0x8;
    obj->anim.activeMove = -1;
    v = objModelGetVecFn_800395d8(obj, 9);
    if (v != NULL)
    {
        v[0] = 0;
        v[1] = 0;
        v[2] = 0;
    }
}

int playerStateOnBike(GameObject* obj, int state)
{
    PlayerState* inner = obj->extra;
    void* sub;
    f32 out;
    f32 a;
    int b;
    f32 c;
    int d;
    f32 ret;
    int blend;
    (*gCameraInterface)->func1C(2);
    ((PlayerState*)state)->baddie.physicsActive = 0;
    *(int*)((char*)state + 0x4) |= 0x100000;
    *(u32*)&((PlayerState*)inner)->flags360 &= ~PLAYER_FLAG_HITDETECT;
    ObjHits_DisableObject(obj);
    sub = *(void**)((char*)inner + 0x7f0);
    if (sub == NULL)
    {
        obj->anim.activeMove = -1;
        return 0;
    }
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        if (*(void**)((char*)inner + 0x6e8) == NULL)
        {
            inner->moveSequence = (int)lbl_803332B0.moveSequences[0];
        }
        ObjAnim_SetCurrentMove((int)obj, *(s16*)(inner->moveSequence + 0x2), lbl_803E7EA4, 0);
        ObjAnim_AdvanceCurrentMove((int)obj, lbl_803E7EA4, *(f32*)&lbl_803E7EA4, NULL);
    }
    if ((inner->moveSequenceFlags & 0x4) != 0)
    {
        ObjAnim_SetMoveProgress((ObjAnimComponent*)obj, *(f32*)((char*)sub + 0x98));
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7EA4;
    }
    else
    {
        ret = (*(f32 (*)(int, f32*))(*(int*)((char*)*(int*)*(int*)((char*)sub + 0x68) + 0x44)))((int)sub, &out);
        if (out <= lbl_803E7EE0)
        {
            ((PlayerState*)state)->baddie.moveSpeed = out;
        }
        else
        {
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F6C * ret + lbl_803E7EF8;
        }
    }
    if ((inner->moveSequenceFlags & 0x1) != 0)
    {
        (*(void (*)(int, f32*, int*))(*(int*)((char*)*(int*)*(int*)((char*)sub + 0x68) + 0x40)))((int)sub, &a, &b);
        blend = (int)(lbl_803E7FAC * a);
        if (blend < 0)
        {
            blend = -blend;
        }
        if (b != 0)
        {
            Object_ObjAnimSetSecondaryBlendMove((ObjAnimComponent*)obj, *(s16*)(inner->moveSequence + 0xa), blend);
        }
        else
        {
            Object_ObjAnimSetSecondaryBlendMove((ObjAnimComponent*)obj, *(s16*)(inner->moveSequence + 0x8), blend);
        }
    }
    else if ((inner->moveSequenceFlags & 0x8) != 0)
    {
        (*(void (*)(int, f32*, int*))(*(int*)((char*)*(int*)*(int*)((char*)sub + 0x68) + 0x40)))((int)sub, &c, &d);
        *(u32*)&((PlayerState*)inner)->flags360 |= 0x2000000LL;
        inner->headYaw = (s16)d;
        inner->bodyLeanAngle = (s16)c;
        inner->bodyLeanHalf = inner->bodyLeanAngle / 2;
        inner->headPitch = inner->bodyLeanAngle / 2;
    }
    if ((inner->moveSequenceFlags & 0x1) != 0)
    {
        ObjAnim_WriteStateWord((ObjAnimComponent*)obj, OBJANIM_STATE_INDEX_CURRENT, OBJANIM_STATE_WORD_PREV_EVENT_STATE,
                               0);
        ObjAnim_WriteStateWord((ObjAnimComponent*)obj, OBJANIM_STATE_INDEX_ACTIVE, OBJANIM_STATE_WORD_PREV_EVENT_STATE,
                               0);
    }
    if ((*(int (*)(int, int))(*(int*)((char*)*(int*)*(int*)((char*)sub + 0x68) + 0x2c)))((int)sub, (int)obj) != 0)
    {
        *(int*)&((PlayerState*)state)->baddie.unk308 = 0;
        return 0x1a;
    }
    return 0;
}

int playerState17(int p1, int state)
{
    if (mainGetBit(GAMEBIT_LV_EscapedFromPole))
    {
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
        return -1;
    }
    return 0;
}

int playerStateMountBike(GameObject* obj, int state, f32 fv)
{
    char* base = (char*)lbl_80332EC0;
    PlayerState* inner = obj->extra;
    int sub = (int)inner->focusObject;
    ObjModel* joint;
    f32 j0[3];
    f32 j1[3];
    f32 wpos[3];

    inner->flags360 &= ~2;
    inner->flags360 |= 0x2000;
    *(int*)((char*)state + 0x4) |= 0x100000;
    {
        f32 z = lbl_803E7EA4;
        ((PlayerState*)state)->baddie.animSpeedA = z;
        ((PlayerState*)state)->baddie.animSpeedB = z;
        *(int*)((char*)state + 0x0) |= 0x200000;
        obj->anim.velocityX = z;
        obj->anim.velocityZ = z;
    }
    *(s8*)&((PlayerState*)state)->baddie.physicsActive = 0;
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        *(s16*)((char*)state + 0x278) = 0x16;
        inner->stateHandler = 0;
    }
    ObjHits_DisableObject(obj);
    obj->anim.velocityY = lbl_803E7EA4;
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        int sel;
        f32 scratch[2];

        if (gPlayerPathObject != NULL && ((ByteFlags*)((char*)inner + 0x3f4))->b40 != 0)
        {
            inner->staffActionRequest = 1;
            ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
        }
        switch (*(s16*)((char*)sub + 0x46))
        {
        case 0x72:
            inner->moveSequence = (int)(base + 0x3f0);
            inner->moveSequenceFlags = 3;
            if (coordsToMapCell(obj->anim.localPosX, obj->anim.localPosZ) == 0x13)
            {
                mainSetBits(0xf0a, 1);
            }
            (*gCameraInterface)->setMode(0x45, 1, 0, 0, NULL, 0, 0xff);
            break;
        case 0x38c:
            inner->moveSequence = (int)(base + 0x3f0);
            inner->moveSequenceFlags = 3;
            (*gCameraInterface)->setFocus((void*)sub, 0);
            (*gCameraInterface)->setMode(0x45, 1, 0, 0, NULL, 0, 0xff);
            break;
        case 0x419:
            inner->moveSequence = (int)(base + 0x420);
            (*gCameraInterface)->setMode(0x53, 1, 0, 0, NULL, 0x2d, 0xff);
            break;
        case 0x416:
            inner->moveSequence = (int)(base + 0x438);
            inner->moveSequenceFlags = 8;
            (*gCameraInterface)->setFocus((void*)sub, 0);
            (*gCameraInterface)->loadTriggeredCamAction(0, 0x69, 0);
            break;
        case 0x8c:
            inner->moveSequence = (int)(base + 0x408);
            inner->moveSequenceFlags = 4;
            break;
        default:
            inner->moveSequence = (int)(base + 0x420);
            inner->moveSequenceFlags = 4;
            (*gCameraInterface)->loadTriggeredCamAction(0, 0x1d, 0);
            break;
        }
        {
            int t = (*(int (*)(int))(*(int*)(*(int*)(*(int*)((char*)sub + 0x68)) + 0x24)))(sub);
            (*(void (*)(int, int))(*(int*)(*(int*)(*(int*)((char*)sub + 0x68)) + 0x3c)))(sub, 1);
            switch (t)
            {
            case 1:
                sel = 6;
                break;
            case 2:
            default:
                sel = 7;
                break;
            }
        }
        inner->targetYaw = *(s16*)((char*)sub + 0x0);
        inner->yaw = inner->targetYaw;
        ObjAnim_SetCurrentMove((int)obj, ((s16*)inner->moveSequence)[sel], lbl_803E7EA4, 4);
        joint = Player_GetActiveModel((int)obj);
        ObjModel_SampleJointTransform(
            joint, 0, 0, lbl_803E7EA4, obj->anim.rootMotionScale, j0, (s16*)scratch);
        ObjModel_SampleJointTransform(
            joint, 0, 0, lbl_803E7EE0, obj->anim.rootMotionScale, j1, (s16*)scratch);
        (*(void (*)(int, void*, void*, void*))(*(int*)(*(int*)(*(int*)((char*)sub + 0x68)) + 0x28)))(
            sub, &wpos[0], &wpos[1], &wpos[2]);
        wpos[0] = wpos[0] - obj->anim.localPosX;
        wpos[1] = wpos[1] - obj->anim.localPosY;
        wpos[2] = wpos[2] - obj->anim.localPosZ;
        inner->warpStartX = obj->anim.localPosX;
        inner->warpStartY = obj->anim.localPosY;
        inner->warpStartZ = obj->anim.localPosZ;
        inner->warpDeltaX = wpos[0];
        inner->warpDeltaY = wpos[1] - j1[1];
        inner->warpDeltaZ = wpos[2];
        obj->anim.flags |= 8;
        obj->anim.modelState->flags |= OBJ_MODEL_STATE_SHADOW_FADE_OUT;
        obj->anim.modelState->shadowAlphaStep = 0;
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7FD8;
    }
    {
        obj->anim.localPosX =
            obj->anim.currentMoveProgress * inner->warpDeltaX + inner->warpStartX;
        obj->anim.localPosY =
            obj->anim.currentMoveProgress * inner->warpDeltaY + inner->warpStartY;
        obj->anim.localPosZ =
            obj->anim.currentMoveProgress * inner->warpDeltaZ + inner->warpStartZ;
        (*(void (*)(int, void*, void*, void*))(*(int*)(*(int*)(*(int*)((char*)sub + 0x68)) + 0x34)))(
            sub, &wpos[0], &wpos[1], &wpos[2]);
        (*gCameraInterface)
            ->overridePos(
                obj->anim.currentMoveProgress * (wpos[0] - inner->warpStartX) + inner->warpStartX,
                obj->anim.currentMoveProgress * (wpos[1] - inner->warpStartY) + inner->warpStartY,
                obj->anim.currentMoveProgress * (wpos[2] - inner->warpStartZ) + inner->warpStartZ);
    }
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA == 0 &&
        *(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
    {
        ObjAnim_SetCurrentMove((int)obj, *(s16*)inner->moveSequence, lbl_803E7EA4, 1);
        (*(void (*)(int, int))(*(int*)(*(int*)(*(int*)((char*)sub + 0x68)) + 0x3c)))(sub, 2);
        if (arrayIndexOf((int*)(base + 0x160), 4, *(s16*)((char*)sub + 0x46)) != -1)
        {
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029F67C;
            return 0x1b;
        }
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029F67C;
        return 0x19;
    }
    return 0;
}

void fn_8029FFD0(GameObject* obj, int p2)
{
    PlayerState* inner = obj->extra;
    s16 v = ((PlayerState*)p2)->baddie.controlMode;
    if (v != 0x15 && v != 0x14 && v != 0x12 && v != 0x13 && v != 0xe && v != 0xf && v != 0x10)
    {
        u8 curAnimId = inner->curAnimId;
        if (curAnimId != 0x48 && curAnimId != 0x47 && curAnimId != 0x42 && getCurSeqNo() == 0)
        {
            (*gCameraInterface)->setMode(0x42, 0, 1, 0, NULL, 0, 0xff);
            inner->curAnimId = 0x42;
        }
        *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
        ObjHits_SyncObjectPositionIfDirty(obj);
    }
    obj->anim.activeMove = -1;
}

void objUpdateHitboxPos(int obj)
{
    ObjHits_SyncObjectPositionIfDirty((GameObject*)obj);
}

int playerStateClimbDownFromWall(GameObject* obj, int state)
{
    PlayerState* inner = obj->extra;
    f32 fz;
    f32 obj98;
    f32 t1, t2, t3;
    f32 outY;
    playerPlayClimbingSound(obj, state);
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        u8 ic;
        ObjModel* model;
        s16 buf2[3];
        f32 buf1[3];
        ObjHits_MarkObjectPositionDirty((ObjAnimComponent*)obj);
        ic = inner->curAnimId;
        if (ic != 0x48 && ic != 0x47)
        {
            (*gCameraInterface)->setMode(0x42, 0, 1, 0, NULL, 0x3c, 0xff);
        }
        ObjAnim_SetCurrentMove((int)obj, lbl_80332F48[0x13], lbl_803E7EA4, 1);
        Object_ObjAnimSetSecondaryBlendMove((ObjAnimComponent*)obj, lbl_80332F48[0x14], 0);
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F34;
        model = Player_GetActiveModel((int)obj);
        ObjModel_SampleJointTransform(model, 0, 0, lbl_803E7EE0, obj->anim.rootMotionScale, buf1, buf2);
        inner->moveOffsetX = inner->groundNormalX * buf1[2];
        inner->moveOffsetZ = inner->groundNormalZ * buf1[2];
        obj->anim.localPosY = inner->spanBottomY;
        *(s16*)((char*)state + 0x278) = 0x15;
        inner->stateHandler = (int)fn_8029FFD0;
    }
    {
        int ex = *(int*)&obj->extra;
        *(u32*)((char*)ex + 0x360) &= ~2LL;
        *(u32*)((char*)ex + 0x360) |= 0x2000LL;
    }
    *(int*)((char*)state + 4) |= 0x100000;
    fz = lbl_803E7EA4;
    ((PlayerState*)state)->baddie.animSpeedA = fz;
    ((PlayerState*)state)->baddie.animSpeedB = fz;
    *(int*)((char*)state + 0) |= 0x200000;
    obj->anim.velocityX = fz;
    obj->anim.velocityZ = fz;
    *(int*)((char*)state + 4) |= 0x8000000;
    obj->anim.velocityY = fz;
    ObjAnim_WriteStateWord((ObjAnimComponent*)obj, OBJANIM_STATE_INDEX_CURRENT, OBJANIM_STATE_WORD_EVENT_STATE,
                           inner->animEventState);
    if ((*(int*)&((PlayerState*)state)->baddie.eventFlags & 0x200) != 0)
    {
        doRumble(lbl_803E7F10);
    }
    obj98 = obj->anim.currentMoveProgress;
    if (obj98 > lbl_803E7F68)
    {
        obj->anim.worldPosX = inner->savedPosX;
        obj->anim.worldPosZ = inner->savedPosZ;
        if (*(void**)&obj->anim.parent != NULL)
        {
            obj->anim.worldPosX = obj->anim.worldPosX + playerMapOffsetX;
            obj->anim.worldPosZ = obj->anim.worldPosZ + playerMapOffsetZ;
        }
        Obj_TransformWorldPointToLocal(obj->anim.worldPosX, lbl_803E7EA4,
                                       obj->anim.worldPosZ, &obj->anim.localPosX, &outY,
                                       &obj->anim.localPosZ, *(int*)&obj->anim.parent);
        fn_802AB5A4(obj, (int)inner, 5);
        ObjAnim_SetCurrentMove((int)obj, *(s16*)inner->moveAnimTable, lbl_803E7EA4, 1);
        *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
        return -1;
    }
    t1 = inner->moveOffsetX * obj98 + obj->anim.localPosX;
    t2 = obj->anim.localPosY - inner->moveOffsetY * (lbl_803E7EE0 - obj98);
    t3 = inner->moveOffsetZ * obj98 + obj->anim.localPosZ;
    (*gCameraInterface)->overridePos(t1, t2, t3);
    fn_802AB5A4(obj, (int)inner, 5);
    return 0;
}

int playerStateClimbUpFromWall(GameObject* obj, int state)
{
    PlayerState* inner = obj->extra;
    f32 fz;
    f32 obj98;
    f32 t1, t2, t3;
    f32 outY;
    playerPlayClimbingSound(obj, state);
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        u8 ic;
        ObjModel* model;
        s16 buf2[3];
        f32 buf1[3];
        ObjHits_MarkObjectPositionDirty((ObjAnimComponent*)obj);
        ic = inner->curAnimId;
        if (ic != 0x48 && ic != 0x47)
        {
            (*gCameraInterface)->setMode(0x42, 0, 1, 0, NULL, 0x3c, 0xff);
        }
        ObjAnim_SetCurrentMove((int)obj, lbl_80332F48[0x11], lbl_803E7EA4, 1);
        Object_ObjAnimSetSecondaryBlendMove((ObjAnimComponent*)obj, lbl_80332F48[0x12], 0);
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F84;
        model = Player_GetActiveModel((int)obj);
        ObjModel_SampleJointTransform(model, 0, 0, lbl_803E7EE0, obj->anim.rootMotionScale, buf1, buf2);
        inner->moveOffsetX = inner->groundNormalX * buf1[2];
        inner->moveOffsetZ = inner->groundNormalZ * buf1[2];
        obj->anim.localPosY = inner->spanTopY;
        *(s16*)((char*)state + 0x278) = 0x14;
        inner->stateHandler = (int)fn_8029FFD0;
    }
    {
        int ex = *(int*)&obj->extra;
        *(u32*)((char*)ex + 0x360) &= ~2LL;
        *(u32*)((char*)ex + 0x360) |= 0x2000LL;
    }
    *(int*)((char*)state + 4) |= 0x100000;
    fz = lbl_803E7EA4;
    ((PlayerState*)state)->baddie.animSpeedA = fz;
    ((PlayerState*)state)->baddie.animSpeedB = fz;
    *(int*)((char*)state + 0) |= 0x200000;
    obj->anim.velocityX = fz;
    obj->anim.velocityZ = fz;
    *(int*)((char*)state + 4) |= 0x8000000;
    obj->anim.velocityY = fz;
    ObjAnim_WriteStateWord((ObjAnimComponent*)obj, OBJANIM_STATE_INDEX_CURRENT, OBJANIM_STATE_WORD_EVENT_STATE,
                           inner->animEventState);
    obj98 = obj->anim.currentMoveProgress;
    if (obj98 > lbl_803E7F68)
    {
        obj->anim.worldPosX = inner->savedPosX;
        obj->anim.worldPosZ = inner->savedPosZ;
        if (*(void**)&obj->anim.parent != NULL)
        {
            obj->anim.worldPosX = obj->anim.worldPosX + playerMapOffsetX;
            obj->anim.worldPosZ = obj->anim.worldPosZ + playerMapOffsetZ;
        }
        Obj_TransformWorldPointToLocal(obj->anim.worldPosX, lbl_803E7EA4,
                                       obj->anim.worldPosZ, &obj->anim.localPosX, &outY,
                                       &obj->anim.localPosZ, *(int*)&obj->anim.parent);
        fn_802AB5A4(obj, (int)inner, 5);
        ObjAnim_SetCurrentMove((int)obj, *(s16*)inner->moveAnimTable, lbl_803E7EA4, 1);
        *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
        return -1;
    }
    t1 = inner->moveOffsetX * obj98 + obj->anim.localPosX;
    t2 = obj->anim.localPosY - inner->moveOffsetY * (lbl_803E7EE0 - obj98);
    t3 = inner->moveOffsetZ * obj98 + obj->anim.localPosZ;
    (*gCameraInterface)->overridePos(t1, t2, t3);
    fn_802AB5A4(obj, (int)inner, 5);
    return 0;
}

int playerStateClimbWall(GameObject* obj, int stateArg)
{
    int mask;
    int movingUp;
    int movingDown;
    int movingRight;
    int movingLeft;
    int dir;
    PlayerState* state = (PlayerState*)stateArg;
    ObjModel* model;
    PlayerState* inner = obj->extra;
    s16 i;
    f32 oldSpd;
    f32 dx;
    f32 dy;
    f32 ph;
    WallHit hit;
    f32 out1[3];
    f32 pnt[3];
    f32 dst[3];
    s16 tmp[3];

    if ((s8)state->baddie.moveJustStartedA != 0)
    {
        gPlayerCurrentMoveId = 0x10;
        ObjHits_MarkObjectPositionDirty((ObjAnimComponent*)obj);
    }
    {
        PlayerState* player = obj->extra;
        player->flags360 &= ~2LL;
        player->flags360 |= 0x2000;
    }
    state->baddie.flags4 |= 0x100000;
    {
        f32 z = lbl_803E7EA4;
        state->baddie.animSpeedA = z;
        state->baddie.animSpeedB = z;
        state->baddie.flags0 |= 0x200000;
        obj->anim.velocityX = z;
        obj->anim.velocityZ = z;
        state->baddie.flags4 |= 0x8000000;
        obj->anim.velocityY = z;
    }
    model = Player_GetActiveModel((int)obj);
    ph = state->baddie.moveSpeed;
    gPlayerPrevMoveId = gPlayerCurrentMoveId;
    switch ((s16)gPlayerCurrentMoveId)
    {
    case 0x10:
        if (obj->anim.currentMove == 0x66)
        {
            inner->moveAltToggle = 0;
            gPlayerCurrentMoveId = 0x16;
        }
        else
        {
            inner->moveAltToggle = 1;
            gPlayerCurrentMoveId = 0x15;
        }
        obj->anim.localPosY = inner->savedPosY;
        ph = 0.006f;
    case 0x15:
    case 0x16:
    {
        f32 z = 0.0f;
        inner->moveOffsetX = z;
        inner->moveOffsetY = z;
        inner->moveOffsetZ = z;
    }
        playerPlayClimbingSound(obj, stateArg);
        if (state->baddie.inputMagnitude <= 0.1f)
        {
            break;
        }
        oldSpd = obj->anim.currentMoveProgress;
        obj->anim.currentMoveProgress = 1.0f;
    default:
        if (1.0f == obj->anim.currentMoveProgress)
        {
            pnt[0] = -(30.0f * inner->groundNormalX - inner->savedPosX);
            pnt[1] = inner->savedPosY;
            pnt[2] = -(30.0f * inner->groundNormalZ - inner->savedPosZ);
            {
                int r = objBboxFn_800640cc(&inner->savedPosX, pnt, 0.0f, 3,
                                           (TrackBBoxHit*)&hit, obj, 1, 3, 0xff, 0);
                if (r != 0)
                {
                    obj->anim.localPosX = pnt[0];
                    obj->anim.localPosZ = pnt[2];
                    {
                        f32 ga = hit.ga;
                        inner->spanTopY = hit.gt * (hit.gb - ga) + ga;
                    }
                    {
                        f32 fz0 = hit.fz0;
                        inner->spanBottomY = hit.gt * (hit.fz1 - fz0) + fz0;
                    }
                    inner->groundNormalX = hit.nx;
                    inner->groundNormalY = hit.ny;
                    inner->groundNormalZ = hit.nz;
                    inner->groundNormalW = hit.nw;
                    inner->slopeTangentX = -hit.nz;
                    inner->slopeTangentY = 0.0f;
                    inner->slopeTangentZ = hit.nx;
                    inner->slopePlaneD = -(pnt[2] * inner->slopeTangentZ +
                                           (pnt[0] * inner->slopeTangentX + pnt[1] * inner->slopeTangentY));
                    inner->targetYaw = (s16)getAngle(inner->groundNormalX, inner->groundNormalZ);
                    inner->yaw = inner->targetYaw;
                    {
                        int hf = hit.flags;
                        if ((hf & 4) != 0)
                        {
                            dir = 0;
                        }
                        else if ((hf & 8) != 0)
                        {
                            dir = 1;
                        }
                        else if ((hf & 2) != 0)
                        {
                            dir = 2;
                        }
                        else
                        {
                            dir = 3;
                        }
                    }
                }
                else
                {
                    dir = 2;
                }
            }
            if (gPlayerCurrentMoveId != 0x15 && gPlayerCurrentMoveId != 0x16)
            {
                obj->anim.localPosY = inner->savedPosY;
            }
            if (state->baddie.inputMagnitude > 0.1f)
            {
                gPlayerCurrentMoveId =
                    ((getAngle(state->baddie.moveInputX, -state->baddie.moveInputZ) &
                      0xffff) +
                         0x1000 >>
                     13) &
                    7;
                gPlayerPrevMoveId = -1;
                if ((s16)gPlayerCurrentMoveId == 4 || (s16)gPlayerCurrentMoveId == 0)
                {
                    inner->moveAltToggle ^= 1;
                }
                movingUp = 0;
                movingDown = 0;
                movingRight = 0;
                movingLeft = 0;
                switch (gPlayerCurrentMoveId)
                {
                case 4:
                    movingUp = 1;
                    break;
                case 0:
                    movingDown = 1;
                    break;
                case 6:
                    movingRight = 1;
                    break;
                case 2:
                    movingLeft = 1;
                    break;
                case 3:
                    movingUp = 1;
                    movingLeft = 1;
                    break;
                case 5:
                    movingUp = 1;
                    movingRight = 1;
                    break;
                case 1:
                    movingDown = 1;
                    movingLeft = 1;
                    break;
                case 7:
                    movingDown = 1;
                    movingRight = 1;
                    break;
                }
                if (inner->moveAltToggle != 0)
                {
                    gPlayerCurrentMoveId += 8;
                }
                if (movingUp != 0)
                {
                    f32 fv = inner->spanTopY - inner->savedPosY;
                    f32 lo = lbl_803DAF88[12];
                    f32 hi;
                    if (lo < 0.0f)
                    {
                        lo = -lo;
                    }
                    hi = lbl_803DAF88[13];
                    if (hi < 0.0f)
                    {
                        hi = -hi;
                    }
                    if (fv < hi && (dir == 0 || dir == 3))
                    {
                        f32 frac = (fv - lo) / (hi - lo);
                        f32 m = (frac < 0.0f) ? 0.0f : ((frac > 1.0f) ? 1.0f : frac);
                        inner->animEventState = (s16)(16384.0f * m);
                        inner->moveOffsetY = m;
                        state->baddie.stateHandler = (int)fn_8029FFD0;
                        return 0x15;
                    }
                }
                else if (movingDown != 0)
                {
                    f32 fv = inner->savedPosY - inner->spanBottomY;
                    f32 lo = lbl_803DAF88[14];
                    f32 hi;
                    if (lo < 0.0f)
                    {
                        lo = -lo;
                    }
                    hi = lbl_803DAF88[15];
                    if (hi < 0.0f)
                    {
                        hi = -hi;
                    }
                    if (fv < hi && (dir == 1 || dir == 3))
                    {
                        f32 frac = (fv - lo) / (hi - lo);
                        f32 m = (frac < 0.0f) ? 0.0f : ((frac > 1.0f) ? 1.0f : frac);
                        inner->animEventState = (s16)(16384.0f * m);
                        inner->moveOffsetY = m;
                        state->baddie.stateHandler = (int)fn_8029FFD0;
                        return 0x16;
                    }
                }
                Object_ObjAnimSetMove((int)obj, lbl_80332F48[gPlayerCurrentMoveId], 0.0f, 1);
                ObjModel_SampleJointTransform(model, 1, 0, 1.0f, obj->anim.rootMotionScale, out1, tmp);
                obj->anim.activeMove = -1;
                inner->moveOffsetX = inner->slopeTangentX * -out1[0];
                inner->moveOffsetY = out1[1];
                inner->moveOffsetZ = inner->slopeTangentZ * -out1[0];
                if (movingUp == 0 && movingDown == 0)
                {
                    inner->moveOffsetY = 0.0f;
                }
                if (movingRight == 0 && movingLeft == 0)
                {
                    f32 z = 0.0f;
                    inner->moveOffsetX = z;
                    inner->moveOffsetZ = z;
                }
                mask = 0;
                if (out1[0] < 0.0f)
                {
                    dx = 7.0f * inner->slopeTangentX;
                    dy = 7.0f * inner->slopeTangentZ;
                }
                else
                {
                    dx = 7.0f * -inner->slopeTangentX;
                    dy = 7.0f * -inner->slopeTangentZ;
                }
                if (movingUp != 0 || movingDown != 0)
                {
                    pnt[1] = inner->savedPosY + out1[1];
                    if (out1[1] < 0.0f)
                    {
                        pnt[1] = pnt[1] - 11.0f;
                    }
                    else
                    {
                        pnt[1] += 11.0f;
                    }
                    for (i = 0, ph = 30.0f; i < 2; i++)
                    {
                        if (i != 0)
                        {
                            pnt[0] = inner->savedPosX + dx;
                            pnt[2] = inner->savedPosZ + dy;
                        }
                        else
                        {
                            pnt[0] = inner->savedPosX - dx;
                            pnt[2] = inner->savedPosZ - dy;
                        }
                        dst[0] = -(ph * inner->groundNormalX - pnt[0]);
                        dst[1] = pnt[1];
                        dst[2] = -(ph * inner->groundNormalZ - pnt[2]);
                        if (objBboxFn_800640cc(pnt, dst, 0.0f, 3, NULL, obj, 1, 3, 0xff, 0) != 0)
                        {
                            mask = mask | 1 << i;
                        }
                    }
                }
                else
                {
                    mask |= 3;
                }
                if (movingRight != 0 || movingLeft != 0)
                {
                    pnt[0] = dx + (inner->savedPosX + inner->moveOffsetX);
                    pnt[2] = dy + (inner->savedPosZ + inner->moveOffsetZ);
                    for (i = 0, dy = 30.0f; i < 2; i++)
                    {
                        if (i != 0)
                        {
                            pnt[1] = 11.0f + inner->savedPosY;
                        }
                        else
                        {
                            pnt[1] = inner->savedPosY - 11.0f;
                        }
                        dst[0] = -(dy * inner->groundNormalX - pnt[0]);
                        dst[1] = pnt[1];
                        dst[2] = -(dy * inner->groundNormalZ - pnt[2]);
                        if (objBboxFn_800640cc(pnt, dst, 0.0f, 3, NULL, obj, 1, 3, 0xff, 0) != 0)
                        {
                            mask = mask | 1 << (i + 2);
                        }
                    }
                }
                else
                {
                    mask |= 0xc;
                }
                ph = 0.02f;
                if (mask != 0xf)
                {
                    {
                        f32 z = 0.0f;
                        inner->moveOffsetX = z;
                        inner->moveOffsetY = z;
                        inner->moveOffsetZ = z;
                    }
                    {
                        int st2 = (s16)gPlayerCurrentMoveId;
                        if (st2 == 4 || st2 == 0 || ((st2 == 0xc) | (st2 == 8)) != 0)
                        {
                            inner->moveAltToggle ^= 1;
                        }
                    }
                    {
                        s16 ns;
                        if (inner->moveAltToggle != 0)
                        {
                            ns = 0x15;
                        }
                        else
                        {
                            ns = 0x16;
                        }
                        gPlayerCurrentMoveId = ns;
                    }
                    if (obj->anim.currentMove == lbl_80332F48[21] ||
                        obj->anim.currentMove == lbl_80332F48[22])
                    {
                        gPlayerPrevMoveId = *(s16*)&gPlayerCurrentMoveId;
                        obj->anim.currentMoveProgress = oldSpd;
                    }
                    ph = 0.006f;
                }
            }
            else
            {
                obj->anim.localPosY = inner->savedPosY;
                {
                    s16 ns;
                    if (inner->moveAltToggle != 0)
                    {
                        ns = 0x15;
                    }
                    else
                    {
                        ns = 0x16;
                    }
                    gPlayerCurrentMoveId = ns;
                }
                ph = 0.006f;
            }
        }
        if (gPlayerCurrentMoveId != 0x15 && gPlayerCurrentMoveId != 0x16)
        {
            f32 v = state->baddie.inputMagnitude;
            if (ph < 0.0f)
            {
                ph = -(0.003999997f * v + 0.034f);
            }
            else if (ph > 0.0f)
            {
                ph = 0.003999997f * v + 0.034f;
            }
        }
        playerPlayClimbingSound(obj, stateArg);
        break;
    }
    state->baddie.moveSpeed = ph;
    {
        s16 cur;
        if (gPlayerPrevMoveId != (cur = gPlayerCurrentMoveId))
        {
            ObjAnim_SetCurrentMove((int)obj, lbl_80332F48[cur], 0.0f, 1);
        }
    }
    {
        f32 sp = obj->anim.currentMoveProgress;
        (*gCameraInterface)
            ->overridePos(inner->moveOffsetX * sp + obj->anim.localPosX,
                          inner->moveOffsetY * sp + obj->anim.localPosY,
                          inner->moveOffsetZ * sp + obj->anim.localPosZ);
    }
    fn_802AB5A4(obj, (int)inner, 5);
    return 0;
}

int playerStateClimbOntoWall(GameObject* obj, int state)
{
    PlayerState* inner = obj->extra;
    PlayerState* in0 = obj->extra;
    int flag549;
    f32 fz;
    s16* tbl;
    int flags;
    ObjModel* model;
    u8 ic;
    f32 buf1[3];
    s16 buf2[3];
    f32 pos[2];
    *(u32*)&in0->flags360 &= ~PLAYER_FLAG_HITDETECT;
    *(u32*)&in0->flags360 |= PLAYER_FLAG_NO_POS_VELOCITY;
    *(int*)((char*)state + 0x4) |= 0x100000;
    fz = lbl_803E7EA4;
    ((PlayerState*)state)->baddie.animSpeedA = fz;
    ((PlayerState*)state)->baddie.animSpeedB = fz;
    *(int*)((char*)state + 0x0) |= 0x200000;
    obj->anim.velocityX = fz;
    obj->anim.velocityZ = fz;
    *(int*)((char*)state + 0x4) |= 0x8000000;
    obj->anim.velocityY = fz;
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        *(s16*)((char*)state + 0x278) = 0x12;
        inner->stateHandler = (int)fn_8029FFD0;
        if (gPlayerPathObject != NULL)
        {
            if (((ByteFlags*)((char*)inner + 0x3f4))->b40)
            {
                inner->staffActionRequest = 1;
                ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
            }
        }
        ObjHits_MarkObjectPositionDirty((ObjAnimComponent*)obj);
    }
    flag549 = inner->climbMoveVariant;
    if (flag549 != 0)
    {
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7EF8;
    }
    else
    {
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E8008;
    }
    playerPlayClimbingSound(obj, state);
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        f32 zero = lbl_803E7EA4;
        ((PlayerState*)state)->baddie.animSpeedA = zero;
        ((PlayerState*)state)->baddie.animSpeedB = zero;
        inner->targetYaw = (s16)getAngle(*(f32*)((int)inner + 0x56c), inner->groundNormalZ);
        inner->yaw = inner->targetYaw;
        obj->anim.localPosX = inner->climbStartPosX;
        obj->anim.localPosZ = inner->climbStartPosZ;
        if (flag549 != 0)
        {
            tbl = &lbl_803DC69C;
        }
        else
        {
            tbl = &lbl_803DC698;
        }
        flags = 0x25;
        if (flag549 != 0)
        {
            flags |= 0x40;
        }
        {
            inner->animEventState =
                fn_802A71E0((int)obj, tbl[0], tbl[1], (int*)((char*)inner + 0x598),
                            (int*)((char*)inner + 0x56c), lbl_803E7EA4, *(f32*)&lbl_803E7EA4, 2, (u8)flags);
        }
        model = Player_GetActiveModel((int)obj);
        ObjModel_SampleJointTransform(model, 0, 0, lbl_803E7EE0, obj->anim.rootMotionScale, buf1, buf2);
        fz = lbl_803E7EA4;
        inner->moveOffsetX = fz;
        inner->moveOffsetY = buf1[1];
        inner->moveOffsetZ = fz;
        pos[0] = inner->spanTopY;
        pos[1] = inner->spanBottomY;
        ic = inner->curAnimId;
        if (ic != 0x48 && ic != 0x47)
        {
            (*gCameraInterface)->setMode(0x4b, 1, 1, 8, pos, 0, 0);
        }
    }
    else
    {
        if (obj->anim.currentMoveProgress >= lbl_803E7EE0)
        {
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029FFD0;
            return 0x14;
        }
    }
    ObjAnim_WriteStateWord((ObjAnimComponent*)obj, OBJANIM_STATE_INDEX_CURRENT, OBJANIM_STATE_WORD_EVENT_STATE,
                           inner->animEventState);
    (*gCameraInterface)
        ->overridePos(obj->anim.localPosX,
                      inner->moveOffsetY * obj->anim.currentMoveProgress +
                          obj->anim.localPosY,
                      obj->anim.localPosZ);
    fn_802AB5A4(obj, (int)inner, 5);
    return 0;
}

void playerPlayClimbingSound(GameObject* obj, int p2)
{
    PlayerState* inner = obj->extra;
    int cell;
    int t;
    int sfx;

    if (*(int*)&((PlayerState*)p2)->baddie.eventFlags & 1)
    {
        cell = coordsToMapCell(obj->anim.localPosX, obj->anim.localPosZ);
        if (cell == 0x12)
        {
            Sfx_PlayFromObject((int)obj, SFXTRIG_mv_ropecreak22);
        }
        else
        {
            Sfx_PlayFromObject((int)obj, SFXTRIG_foot);
        }
    }
    if (gPlayerSfxTimerB > 0)
    {
        t = gPlayerSfxTimerB - framesThisStep;
        gPlayerSfxTimerB = t;
        if (t < 0)
            gPlayerSfxTimerB = 0;
    }
    if (*(int*)&((PlayerState*)p2)->baddie.eventFlags & 0x80)
    {
        if (gPlayerSfxTimerB == 0)
        {
            if (randomGetRange(1, 0x64) < 0x46)
            {
                if (inner->characterId == 0)
                {
                    sfx = 0x398;
                }
                else
                {
                    sfx = 0x25;
                }
                Sfx_PlayFromObject((int)obj, (u16)sfx);
                gPlayerSfxTimerB = 0x3c;
            }
        }
    }
}

int playerState11(GameObject* obj, int state)
{
    int inner = *(int*)&obj->extra;
    f32 k;
    f32 pos[2];

    *(u32*)&((PlayerState*)inner)->flags360 &= ~PLAYER_FLAG_HITDETECT;
    *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_NO_POS_VELOCITY;
    *(int*)((char*)state + 0x4) |= 0x100000;
    k = lbl_803E7EA4;
    ((PlayerState*)state)->baddie.animSpeedA = k;
    ((PlayerState*)state)->baddie.animSpeedB = k;
    *(int*)state |= 0x200000;
    obj->anim.velocityX = k;
    obj->anim.velocityZ = k;
    *(int*)((char*)state + 0x4) |= 0x8000000;
    obj->anim.velocityY = k;
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0 && gPlayerPathObject != 0 &&
        ((ByteFlags*)((char*)inner + 0x3f4))->b40)
    {
        ((PlayerState*)inner)->staffActionRequest = 1;
        ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
    }
    switch (obj->anim.currentMove)
    {
    case 0x41a:
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            fn_802AB5A4(obj, inner + 4, 5);
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029FFD0;
            return -0x13;
        }
        break;
    default:
    {
        pos[0] = ((PlayerState*)inner)->spanTopY;
        pos[1] = ((PlayerState*)inner)->spanBottomY;
        if (((PlayerState*)inner)->curAnimId != 0x48 && ((PlayerState*)inner)->curAnimId != 0x47)
        {
            (*gCameraInterface)->setMode(0x4b, 1, 1, 8, pos, 0, 0xff);
        }
        ObjAnim_SetCurrentMove((int)obj, 0x41a, lbl_803E7EA4, 1);
        ((PlayerState*)inner)->targetYaw =
            getAngle(((PlayerState*)inner)->groundNormalX, ((PlayerState*)inner)->groundNormalZ);
        ((PlayerState*)inner)->yaw = ((PlayerState*)inner)->targetYaw;
        obj->anim.localPosX = ((PlayerState*)inner)->climbStartPosX;
        obj->anim.localPosY = ((PlayerState*)inner)->savedPosY;
        obj->anim.localPosZ = ((PlayerState*)inner)->climbStartPosZ;
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E800C;
        break;
    }
    }
    fn_802AB5A4(obj, inner + 4, 5);
    return 0;
}

int playerStateSlideDownLadder(GameObject* obj, int state, f32 fv)
{
    PlayerState* inner = obj->extra;
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        ObjHits_MarkObjectPositionDirty((ObjAnimComponent*)obj);
        lbl_803DE498 = lbl_803E7EA4;
        ObjAnim_SetCurrentMove((int)obj, 0x35, lbl_803E7EA4, 1);
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F20;
        inner->moveStartPosY = obj->anim.localPosY;
        obj->anim.localPosY = inner->savedPosY;
        fn_802AB5A4(obj, (int)inner, 5);
    }
    if (inner->waterDepth > lbl_803E7FA0)
    {
        fn_802AB5A4(obj, (int)inner, 5);
        fn_802AE83C((int)obj, (int)inner, state);
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
        return 2;
    }
    *(int*)((char*)state + 0x4) |= 0x100000;
    *(int*)((char*)state + 0x4) |= 0x8000000;
    *(int*)((char*)state + 0) |= 0x200000;
    switch (obj->anim.currentMove)
    {
    case 0x35:
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            ObjAnim_SetCurrentMove((int)obj, 0x36, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F20;
        }
    case 0x36:
    {
        f32 f30 = lbl_803E7ED8 * -lbl_803DE498;
        f32 f3;
        if ((*(int*)&((PlayerState*)state)->baddie.eventFlags & 1) != 0)
        {
            Sfx_PlayFromObject((int)obj, SFXTRIG_mv_dive4_c);
        }
        f3 = obj->anim.localPosY - (lbl_803E8010 + inner->climbBaseY);
        if (f3 < lbl_803E7EA4)
        {
            f3 = lbl_803E7EA4;
        }
        if (f3 < f30)
        {
            f32 ed4 = lbl_803E7ED4;
            f32 base = ed4 * (lbl_803DE498 * lbl_803DE498 / (ed4 * f30));
            obj->anim.velocityY = -sqrtf(base * f3);
            if (obj->anim.velocityY >= lbl_803E7FEC)
            {
                u8 anim = inner->curAnimId;
                f32 v4ec;
                if (anim != 0x48 && anim != 0x47 && anim != 0x42)
                {
                    (*gCameraInterface)->setMode(0x42, 0, 1, 0, NULL, 0, 0xff);
                    inner->curAnimId = 0x42;
                }
                inner->moveStartPosY = obj->anim.localPosY;
                v4ec = inner->climbBaseY;
                obj->anim.worldPosY = v4ec;
                obj->anim.localPosY = v4ec;
                if (((ByteFlags*)((char*)inner + 0x547))->b80)
                {
                    ObjAnim_SetCurrentMove((int)obj, 0x37, lbl_803E7EA4, 1);
                    ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7FCC;
                    obj->anim.velocityY = lbl_803E7EA4;
                }
                else
                {
                    f32 zero = lbl_803E7EA4;
                    void* sub;
                    ((PlayerState*)state)->baddie.animSpeedC = zero;
                    ((PlayerState*)state)->baddie.animSpeedB = zero;
                    ((PlayerState*)state)->baddie.animSpeedA = zero;
                    obj->anim.velocityX = zero;
                    obj->anim.velocityY = zero;
                    obj->anim.velocityZ = zero;
                    fn_802AB5A4(obj, (int)inner, 5);
                    ((ByteFlags*)((char*)inner + 0x3f0))->b80 = 0;
                    ((ByteFlags*)((char*)inner + 0x3f0))->b10 = 0;
                    ((ByteFlags*)((char*)inner + 0x3f0))->b08 = 0;
                    staffFn_80170380(gPlayerStaffObject, 2);
                    ((ByteFlags*)((char*)inner + 0x3f0))->b02 = 0;
                    *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
                    ObjHits_SyncObjectPositionIfDirty(obj);
                    ((ByteFlags*)((char*)inner + 0x3f0))->b40 = 0;
                    ((ByteFlags*)((char*)inner + 0x3f0))->b04 = 1;
                    ((ByteFlags*)((char*)inner + 0x3f4))->b10 = 1;
                    inner->isHoldingObject = 0;
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
                        *(s16*)((char*)inner->heldObj + 0x6) &= ~0x4000;
                        *(int*)((char*)inner->heldObj + 0xf8) = 0;
                        inner->heldObj = 0;
                    }
                    *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
                    return 3;
                }
            }
        }
        else
        {
            if (obj->anim.velocityY > lbl_803E8014)
            {
                obj->anim.velocityY = obj->anim.velocityY - lbl_803E7F6C * fv;
            }
            if (obj->anim.velocityY < *(f32*)&lbl_803E8014)
            {
                obj->anim.velocityY = lbl_803E8014;
            }
            if (obj->anim.velocityY < lbl_803DE498)
            {
                lbl_803DE498 = obj->anim.velocityY;
            }
        }
    }
    break;
    case 0x37:
        if ((*(int*)&((PlayerState*)state)->baddie.eventFlags & 1) != 0)
        {
            int snd = audioPickSoundEffect_8006ed24(inner->surfaceType, inner->footstepSoundId);
            Sfx_PlayFromObject((int)obj, snd);
            doRumble(lbl_803E7F10);
            if (inner->waterDepth > lbl_803E7EA4)
            {
                (*gWaterfxInterface)
                    ->spawnSplashBurst((void*)obj, obj->anim.localPosX,
                                       obj->anim.localPosY, obj->anim.localPosZ,
                                       lbl_803E8018);
            }
        }
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            f32 local;
            obj->anim.worldPosX = inner->savedPosX;
            obj->anim.worldPosZ = inner->savedPosZ;
            if (obj->anim.parent != NULL)
            {
                obj->anim.worldPosX += playerMapOffsetX;
                obj->anim.worldPosZ += playerMapOffsetZ;
            }
            Obj_TransformWorldPointToLocal(obj->anim.worldPosX, lbl_803E7EA4,
                                           obj->anim.worldPosZ, &obj->anim.localPosX,
                                           &local, &obj->anim.localPosZ,
                                           *(int*)&obj->anim.parent);
            fn_802AB5A4(obj, (int)inner, 5);
            ObjAnim_SetCurrentMove((int)obj, *(s16*)(inner->moveAnimTable), lbl_803E7EA4, 1);
            *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
            return 2;
        }
        break;
    }
    {
        f32 w;
        f32 py;
        f32 cx = obj->anim.localPosX;
        f32 cy;
        f32 cz = obj->anim.localPosZ;
        f32 czOut = cz;
        switch (obj->anim.currentMove)
        {
        case 0x35:
            cy = obj->anim.currentMoveProgress *
                     (obj->anim.localPosY - inner->moveStartPosY) +
                 inner->moveStartPosY;
            break;
        case 0x37:
        {
            w = obj->anim.currentMoveProgress;
            cx = w * (inner->savedPosX - cx) + cx;
            py = obj->anim.localPosY;
            cy = (lbl_803E7EE0 - w) * (inner->moveStartPosY - py) + py;
            czOut = w * (inner->savedPosZ - cz) + cz;
        }
        break;
        default:
            cy = obj->anim.localPosY;
            break;
        }
        (*gCameraInterface)->overridePos(cx, cy, czOut);
    }
    fn_802AB5A4(obj, (int)inner, 5);
    return 0;
}

int playerStateOnLadder(int obj, int state)
{
    ObjModel* jt;
    int inner;
    f32 t;
    f32 spd;
    f32 ph;
    f32 buf1[3];
    f32 buf2[3];
    s16 tmp[3];
    f32 outY;

    inner = *(int*)&((GameObject*)obj)->extra;
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        ObjHits_MarkObjectPositionDirty((ObjAnimComponent*)obj);
        if (gPlayerPathObject != 0 && ((u32) * (u8*)((char*)inner + 0x3f4) >> 6 & 1) != 0)
        {
            ((PlayerState*)inner)->staffActionRequest = 1;
            ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
        }
        if (((GameObject*)obj)->anim.currentMove == lbl_80332F2C[8] ||
            ((GameObject*)obj)->anim.currentMove == lbl_80332F2C[12])
        {
            gPlayerCurrentMoveId = 8;
        }
        else
        {
            gPlayerCurrentMoveId = 9;
        }
    }
    if (((PlayerState*)inner)->climbStep > 3)
    {
        setAButtonIcon(0x1a);
    }
    else
    {
        setAButtonIcon(0x1c);
    }
    {
        int base = *(int*)&((GameObject*)obj)->extra;
        *(u32*)((char*)base + 0x360) &= ~0x2LL;
        *(u32*)((char*)base + 0x360) |= 0x2000LL;
    }
    *(u32*)((char*)state + 4) |= 0x100000;
    {
        f32 z = lbl_803E7EA4;
        ((PlayerState*)state)->baddie.animSpeedA = z;
        ((PlayerState*)state)->baddie.animSpeedB = z;
        *(u32*)state |= 0x200000;
        ((GameObject*)obj)->anim.velocityX = z;
        ((GameObject*)obj)->anim.velocityZ = z;
        *(u32*)((char*)state + 4) |= 0x8000000;
        if (((PlayerState*)inner)->waterDepth > lbl_803E7FA0)
        {
            fn_802AB5A4((GameObject*)obj, inner, 5);
            fn_802AE83C(obj, inner, state);
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
            return 2;
        }
        ((GameObject*)obj)->anim.velocityY = z;
        {
            f32 mag = ((PlayerState*)state)->baddie.moveInputZ / lbl_803E7FA8;
            if (mag < z)
            {
                mag = -mag;
            }
            t = (mag < lbl_803E7EFC) ? lbl_803E7EFC : ((mag > lbl_803E7EE0) ? lbl_803E7EE0 : mag);
        }
    }
    jt = Player_GetActiveModel(obj);
    spd = lbl_803E7EA4;
    ph = ((PlayerState*)state)->baddie.moveSpeed;
    gPlayerPrevMoveId = gPlayerCurrentMoveId;
    if ((*(int*)&((PlayerState*)state)->baddie.eventFlags & 1) != 0)
    {
        switch (((PlayerState*)inner)->footstepSurface)
        {
        case 4:
            Sfx_PlayFromObject(obj, SFXTRIG_foot_33a);
            break;
        default:
            Sfx_PlayFromObject(obj, SFXTRIG_foot_var);
            break;
        }
    }
    switch ((s16)gPlayerCurrentMoveId)
    {
    case 8:
    case 9:
    case 12:
    case 13:
        ((GameObject*)obj)->anim.localPosY = ((PlayerState*)inner)->climbTargetY;
        ((GameObject*)obj)->anim.activeMove = -1;
        ((PlayerState*)inner)->climbingUp = 0;
        ((PlayerState*)inner)->climbStartY = ((PlayerState*)inner)->climbTargetY;
        ph = spd = lbl_803E7EA4;
        if ((gPlayerCurrentMoveId & 1) != 0)
        {
            gPlayerCurrentMoveId = 1;
        }
        else
        {
            gPlayerCurrentMoveId = 0;
        }
        break;
    case 6:
    case 7:
        if ((*(int*)&((PlayerState*)state)->baddie.eventFlags & 0x80) != 0)
        {
            Sfx_PlayFromObject(obj, SFXTRIG_foot);
            if (((PlayerState*)inner)->characterId == 0)
            {
                Sfx_PlayFromObject(obj, SFXTRIG_jump3);
            }
        }
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            ((GameObject*)obj)->anim.localPosY = ((PlayerState*)inner)->climbEndLocalY;
        }
        else
        {
            ObjModel_SampleJointTransform(jt, 0, 0, lbl_803E7EA4, ((GameObject*)obj)->anim.rootMotionScale, buf1, tmp);
            ObjModel_SampleJointTransform(jt, 0, 0, lbl_803E7EE0, ((GameObject*)obj)->anim.rootMotionScale, buf2, tmp);
            ((GameObject*)obj)->anim.localPosY = ((GameObject*)obj)->anim.currentMoveProgress *
                                                     ((lbl_803DE43C - (buf2[1] - buf1[1])) - (lbl_803DE438 + buf1[1])) +
                                                 lbl_803DE438;
        }
    case 10:
    case 11:
        if ((*(int*)&((PlayerState*)state)->baddie.eventFlags & 0x200) != 0)
        {
            doRumble(lbl_803E7F10);
            if (((PlayerState*)inner)->waterDepth > lbl_803E7EA4)
            {
                (*gWaterfxInterface)
                    ->spawnSplashBurst((void*)obj, ((GameObject*)obj)->anim.localPosX,
                                       ((GameObject*)obj)->anim.localPosY, ((GameObject*)obj)->anim.localPosZ,
                                       lbl_803E8018);
            }
        }
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            ((GameObject*)obj)->anim.worldPosX = ((PlayerState*)inner)->savedPosX;
            ((GameObject*)obj)->anim.worldPosZ = ((PlayerState*)inner)->savedPosZ;
            if (((GameObject*)obj)->anim.parent != NULL)
            {
                ((GameObject*)obj)->anim.worldPosX = ((GameObject*)obj)->anim.worldPosX + playerMapOffsetX;
                ((GameObject*)obj)->anim.worldPosZ = ((GameObject*)obj)->anim.worldPosZ + playerMapOffsetZ;
            }
            Obj_TransformWorldPointToLocal(((GameObject*)obj)->anim.worldPosX, lbl_803E7EA4,
                                           ((GameObject*)obj)->anim.worldPosZ, &((GameObject*)obj)->anim.localPosX,
                                           &outY, &((GameObject*)obj)->anim.localPosZ,
                                           *(int*)&((GameObject*)obj)->anim.parent);
            if (gPlayerCurrentMoveId == 6 || gPlayerCurrentMoveId == 7)
            {
                fn_802AB5A4((GameObject*)obj, inner, 7);
            }
            else
            {
                fn_802AB5A4((GameObject*)obj, inner, 5);
            }
            ObjAnim_SetCurrentMove(obj, **(s16**)((char*)inner + 0x3f8), lbl_803E7EA4, 1);
            *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
            return 2;
        }
        break;
    case 4:
    case 5:
        if (((PlayerState*)state)->baddie.moveInputZ > lbl_803E7F10)
        {
            ObjAnim_SetMoveProgress((ObjAnimComponent*)obj, lbl_803E7EA4);
        }
        else if (((PlayerState*)state)->baddie.moveInputZ < lbl_803E801C)
        {
            ObjAnim_SetMoveProgress((ObjAnimComponent*)obj, lbl_803E7EA4);
        }
        else
        {
            if ((*(int*)&((PlayerState*)state)->baddie.unk31C & 0x100) != 0 && ((PlayerState*)inner)->climbStep > 3)
            {
                *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029FFD0;
                return -0x10;
            }
            break;
        }
    default:
        if ((*(int*)&((PlayerState*)state)->baddie.eventFlags & 0x80) != 0)
        {
            Sfx_PlayFromObject(obj, SFXTRIG_foot_var);
        }
        if ((*(int*)&((PlayerState*)state)->baddie.unk31C & 0x100) != 0 && ((PlayerState*)inner)->climbStep > 3)
        {
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029FFD0;
            return -0x10;
        }
        if (lbl_803E7EE0 == ((GameObject*)obj)->anim.currentMoveProgress)
        {
            if (((PlayerState*)state)->baddie.moveInputZ < lbl_803E801C)
            {
                ((PlayerState*)inner)->climbingUp = 0;
                ph = -(lbl_803E7EF8 * t + lbl_803E7F20);
                if ((s16)gPlayerCurrentMoveId <= 1)
                {
                    gPlayerCurrentMoveId += 2;
                    spd = lbl_803E7F68;
                }
            }
            else
            {
                *(u8*)&((PlayerState*)inner)->climbStep += 1;
                ((PlayerState*)inner)->climbingUp = 1;
                ph = lbl_803E7EA4;
                if ((s16)gPlayerCurrentMoveId <= 1)
                {
                    gPlayerCurrentMoveId ^= 1;
                    spd = ph;
                }
                ((PlayerState*)inner)->climbStartY =
                    ((GameObject*)obj)->anim.localPosY + ((PlayerState*)inner)->moveStartPosY;
                ((PlayerState*)inner)->climbTargetY =
                    (f32) * (s8*)((char*)inner + 0x4e4) * ((PlayerState*)inner)->climbStepHeight +
                    ((PlayerState*)inner)->climbBaseY;
                ((GameObject*)obj)->anim.localPosY = ((PlayerState*)inner)->climbStartY;
            }
        }
        {
            f32 z2 = lbl_803E7EA4;
            if (z2 == ((GameObject*)obj)->anim.currentMoveProgress)
            {
                if (((PlayerState*)state)->baddie.moveInputZ > lbl_803E7F10)
                {
                    ((PlayerState*)inner)->climbingUp = 1;
                    if ((int)((PlayerState*)inner)->climbStep >= ((PlayerState*)inner)->climbStepCount - 3)
                    {
                        spd = z2;
                        ph = lbl_803E8020;
                        {
                            s16 ns;
                            if ((gPlayerCurrentMoveId & 1) != 0)
                            {
                                ns = 7;
                            }
                            else
                            {
                                ns = 6;
                            }
                            gPlayerCurrentMoveId = ns;
                        }
                        lbl_803DE438 = ((GameObject*)obj)->anim.localPosY;
                        lbl_803DE43C = ((PlayerState*)inner)->climbEndLocalY + lbl_803DAF88[0];
                        if (((PlayerState*)inner)->curAnimId != 0x48 && ((PlayerState*)inner)->curAnimId != 0x47)
                        {
                            (*gCameraInterface)->setMode(0x42, 0, 1, 0, NULL, 0x1e, 0xff);
                        }
                        break;
                    }
                    spd = z2;
                    ph = lbl_803E7F84 * t + lbl_803E7F20;
                    if ((s16)gPlayerCurrentMoveId > 1)
                    {
                        if ((gPlayerCurrentMoveId & 1) != 0)
                        {
                            gPlayerCurrentMoveId = 1;
                        }
                        else
                        {
                            gPlayerCurrentMoveId = 0;
                        }
                    }
                }
                else if (((PlayerState*)state)->baddie.moveInputZ < lbl_803E801C)
                {
                    *(u8*)&((PlayerState*)inner)->climbStep -= 1;
                    ((PlayerState*)inner)->climbingUp = 0;
                    if (((PlayerState*)inner)->climbStep < 1)
                    {
                        if (((PlayerState*)inner)->curAnimId != 0x48 && ((PlayerState*)inner)->curAnimId != 0x47 &&
                            ((PlayerState*)inner)->curAnimId != 0x42)
                        {
                            (*gCameraInterface)->setMode(0x42, 0, 1, 0, NULL, 0x1e, 0xff);
                            ((PlayerState*)inner)->curAnimId = 0x42;
                        }
                        if (((u32) * (u8*)((char*)inner + 0x547) >> 7 & 1) != 0)
                        {
                            spd = lbl_803E7EA4;
                            ph = lbl_803E7FE8;
                            {
                                s16 ns;
                                if ((gPlayerCurrentMoveId & 1) != 0)
                                {
                                    ns = 0xb;
                                }
                                else
                                {
                                    ns = 0xa;
                                }
                                gPlayerCurrentMoveId = ns;
                            }
                            ((GameObject*)obj)->anim.localPosY = ((PlayerState*)inner)->climbBaseY;
                            break;
                        }
                        else
                        {
                            {
                                f32 z3 = lbl_803E7EA4;
                                ((PlayerState*)state)->baddie.animSpeedC = z3;
                                ((PlayerState*)state)->baddie.animSpeedB = z3;
                                ((PlayerState*)state)->baddie.animSpeedA = z3;
                                ((GameObject*)obj)->anim.velocityX = z3;
                                ((GameObject*)obj)->anim.velocityY = z3;
                                ((GameObject*)obj)->anim.velocityZ = z3;
                            }
                            ((ByteFlags*)((char*)inner + 0x3f0))->b80 = 0;
                            ((ByteFlags*)((char*)inner + 0x3f0))->b10 = 0;
                            ((ByteFlags*)((char*)inner + 0x3f0))->b08 = 0;
                            staffFn_80170380(gPlayerStaffObject, 2);
                            ((ByteFlags*)((char*)inner + 0x3f0))->b02 = 0;
                            *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
                            ObjHits_SyncObjectPositionIfDirty((GameObject*)obj);
                            ((ByteFlags*)((char*)inner + 0x3f0))->b40 = 0;
                            ((ByteFlags*)((char*)inner + 0x3f0))->b04 = 1;
                            ((ByteFlags*)((char*)inner + 0x3f4))->b10 = 1;
                            ((PlayerState*)inner)->isHoldingObject = 0;
                            if (((PlayerState*)inner)->heldObj != NULL)
                            {
                                if (((GameObject*)((PlayerState*)inner)->heldObj)->anim.seqId == 0x3cf ||
                                    ((GameObject*)((PlayerState*)inner)->heldObj)->anim.seqId == 0x662)
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
                            fn_802AB5A4((GameObject*)obj, inner, 5);
                            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
                            return 3;
                        }
                    }
                    else
                    {
                        spd = lbl_803E7F68;
                        ph = -(lbl_803E7EF8 * t + lbl_803E7F20);
                        {
                            s16 ns;
                            if ((gPlayerCurrentMoveId & 1) != 0)
                            {
                                ns = 2;
                            }
                            else
                            {
                                ns = 3;
                            }
                            gPlayerCurrentMoveId = ns;
                        }
                        ((PlayerState*)inner)->climbTargetY =
                            (f32) * (s8*)((char*)inner + 0x4e4) * ((PlayerState*)inner)->climbStepHeight +
                            ((PlayerState*)inner)->climbBaseY;
                        {
                            f32 y2 = ((GameObject*)obj)->anim.localPosY - ((PlayerState*)inner)->moveStartPosY;
                            ((PlayerState*)inner)->climbStartY = y2;
                            ((GameObject*)obj)->anim.localPosY = y2;
                        }
                    }
                }
                else
                {
                    if (ObjAnim_GetCurrentEventCountdown((ObjAnimComponent*)obj) == 0)
                    {
                        spd = lbl_803E7EA4;
                        ph = lbl_803E7EF8;
                        if ((gPlayerCurrentMoveId & 1) != 0 && gPlayerCurrentMoveId != 5)
                        {
                            gPlayerCurrentMoveId = 5;
                        }
                        else if ((gPlayerCurrentMoveId & 1) == 0 && gPlayerCurrentMoveId != 4)
                        {
                            gPlayerCurrentMoveId = 4;
                        }
                        break;
                    }
                }
            }
        }
        if (ph < lbl_803E7EA4)
        {
            ph = -(lbl_803E7EF8 * t + lbl_803E7F20);
        }
        else if (ph > lbl_803E7EA4)
        {
            ph = lbl_803E7F84 * t + lbl_803E7F20;
        }
        if (*(s8*)&((PlayerState*)inner)->climbingUp != 0)
        {
            ((GameObject*)obj)->anim.localPosY =
                ((GameObject*)obj)->anim.currentMoveProgress *
                    (((PlayerState*)inner)->climbTargetY - ((PlayerState*)inner)->climbStartY) +
                ((PlayerState*)inner)->climbStartY;
        }
        else
        {
            ((GameObject*)obj)->anim.localPosY =
                (lbl_803E7EE0 - ((GameObject*)obj)->anim.currentMoveProgress) *
                    (((PlayerState*)inner)->climbTargetY - ((PlayerState*)inner)->climbStartY) +
                ((PlayerState*)inner)->climbStartY;
        }
        break;
    }
    ((PlayerState*)state)->baddie.moveSpeed = ph;
    if (gPlayerPrevMoveId != gPlayerCurrentMoveId)
    {
        ObjAnim_SetCurrentMove(obj, lbl_80332F2C[gPlayerCurrentMoveId], spd, 1);
        if ((s16)gPlayerCurrentMoveId <= 1 && ((PlayerState*)inner)->climbSampleDone == 0)
        {
            ObjModel_SampleJointTransform(jt, 0, 0, lbl_803E7EA4, ((GameObject*)obj)->anim.rootMotionScale, buf1, tmp);
            ObjModel_SampleJointTransform(jt, 0, 0, lbl_803E7EE0, ((GameObject*)obj)->anim.rootMotionScale, buf2, tmp);
            ((PlayerState*)inner)->moveStartPosY = buf2[1] - buf1[1];
            *(u8*)&((PlayerState*)inner)->climbSampleDone = 1;
        }
    }
    {
        f32 w;
        f32 py;
        f32 x = ((GameObject*)obj)->anim.localPosX;
        f32 y;
        f32 zz = ((GameObject*)obj)->anim.localPosZ;
        f32 zzOut = zz;
        switch ((s16)gPlayerCurrentMoveId)
        {
        case 0:
        case 1:
        case 2:
        case 3:
            y = ((GameObject*)obj)->anim.currentMoveProgress *
                    (((f32)(((PlayerState*)inner)->climbStep + 1) * ((PlayerState*)inner)->climbStepHeight +
                      ((PlayerState*)inner)->climbBaseY) -
                     ((GameObject*)obj)->anim.localPosY) +
                ((GameObject*)obj)->anim.localPosY;
            break;
        case 10:
        case 11:
            w = ((GameObject*)obj)->anim.currentMoveProgress;
            x = w * (((PlayerState*)inner)->savedPosX - x) + x;
            py = ((GameObject*)obj)->anim.localPosY;
            y = (lbl_803E7EE0 - w) * (((PlayerState*)inner)->climbTargetY - py) + py;
            zzOut = w * (((PlayerState*)inner)->savedPosZ - zz) + zz;
            break;
        case 6:
        case 7:
            w = ((GameObject*)obj)->anim.currentMoveProgress;
            x = w * (((PlayerState*)inner)->savedPosX - x) + x;
            y = w * (((PlayerState*)inner)->climbEndLocalY - ((GameObject*)obj)->anim.localPosY) +
                ((GameObject*)obj)->anim.localPosY;
            zzOut = w * (((PlayerState*)inner)->savedPosZ - zz) + zz;
            break;
        default:
            y = ((GameObject*)obj)->anim.localPosY;
            break;
        }
        (*gCameraInterface)->overridePos(x, y, zzOut);
    }
    fn_802AB5A4((GameObject*)obj, inner, 5);
    return 0;
}

int playerStateClimbOntoLadder(GameObject* obj, int state, f32 fv)
{
    int flag;
    PlayerState* innerV = obj->extra;
    PlayerState* inner = obj->extra;

    *(u32*)&((PlayerState*)innerV)->flags360 &= ~PLAYER_FLAG_HITDETECT;
    *(u32*)&((PlayerState*)innerV)->flags360 |= PLAYER_FLAG_NO_POS_VELOCITY;
    *(int*)((char*)state + 0x4) |= 0x100000;
    {
        f32 z = lbl_803E7EA4;
        ((PlayerState*)state)->baddie.animSpeedA = z;
        ((PlayerState*)state)->baddie.animSpeedB = z;
        *(int*)((char*)state + 0x0) |= 0x200000;
        obj->anim.velocityX = z;
        obj->anim.velocityZ = z;
        *(int*)((char*)state + 0x4) |= 0x8000000;
        obj->anim.velocityY = z;
    }
    flag = innerV->climbStep != 1;
    if (flag)
    {
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7EF8;
    }
    else
    {
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E8024;
    }
    if ((*(int*)&((PlayerState*)state)->baddie.eventFlags & 0x80) != 0)
    {
        int o = (int)obj;
        u16 sfxId = inner->characterId == 0 ? 0x398 : 0x1d;
        Sfx_PlayFromObject(o, sfxId);
    }
    if ((*(int*)&((PlayerState*)state)->baddie.eventFlags & 1) != 0)
    {
        switch (inner->footstepSurface)
        {
        case 4:
            Sfx_PlayFromObject((int)obj, SFXTRIG_foot_33a);
            break;
        default:
            Sfx_PlayFromObject((int)obj, SFXTRIG_foot_var);
            break;
        }
    }
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        s16* tbl;
        int sel;
        f32 jp[3];
        struct
        {
            f32 vx;
            f32 sp1c;
            f32 vy;
            f32 vz;
        } vb;
        ObjHits_MarkObjectPositionDirty((ObjAnimComponent*)obj);
        if (gPlayerPathObject != NULL && ((ByteFlags*)((char*)inner + 0x3f4))->b40 != 0)
        {
            inner->staffActionRequest = 1;
            ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
        }
        {
            f32 z = lbl_803E7EA4;
            ((PlayerState*)state)->baddie.animSpeedA = z;
            ((PlayerState*)state)->baddie.animSpeedB = z;
            *(s16*)((char*)state + 0x278) = 0xe;
            inner->stateHandler = (int)fn_8029FFD0;
            vb.sp1c = z;
        }
        if (flag)
        {
            vb.vx = -inner->moveDirX;
            vb.vy = -inner->moveDirY;
            vb.vz = -inner->moveDirZ;
        }
        else
        {
            vb.vx = inner->moveDirX;
            vb.vy = inner->moveDirY;
            vb.vz = inner->moveDirZ;
        }
        {
            int delta = (u16)getAngle(vb.vx, vb.vy) - inner->targetYaw;
            if (delta > 0x8000)
            {
                delta -= 0xffff;
            }
            if (delta < -0x8000)
            {
                delta += 0xffff;
            }
            inner->targetYaw += delta;
            inner->yaw = inner->targetYaw;
        }
        inner->savedLocalPosX = obj->anim.localPosX;
        inner->savedLocalPosZ = obj->anim.localPosZ;
        obj->anim.localPosX = inner->moveStartPosX;
        obj->anim.localPosZ = inner->moveStartPosZ;
        sel = inner->unk4FC >= *(f32*)&lbl_803E7EA4 ? 0 : 4;
        if (flag)
        {
            tbl = lbl_80332F88;
        }
        else
        {
            tbl = lbl_80332F78;
        }
        inner->eventCountdown =
            fn_802A71E0((int)obj, tbl[sel], tbl[sel + 2], (int*)inner->blendAnchor, (int*)&vb.vx,
                        lbl_803E7EA4, ((PlayerState*)state)->baddie.moveSpeed, 2, 9);
        {
            int f9 = 0x34;
            if (flag)
            {
                f9 |= 0x40;
            }
            fn_802A71E0((int)obj, tbl[sel], tbl[sel + 1], (int*)inner->blendAnchor, (int*)inner->pad51C,
                        lbl_803E7EA4, ((PlayerState*)state)->baddie.moveSpeed, 0, (u8)f9);
        }
        fn_802A71E0((int)obj, tbl[sel + 2], tbl[sel + 3], (int*)inner->blendAnchor, (int*)inner->pad51C,
                    lbl_803E7EA4, ((PlayerState*)state)->baddie.moveSpeed, 0, 0x1a);
        inner->climbTargetY = inner->climbStepHeight * (f32)(int)inner->climbStep + inner->climbBaseY;
        inner->climbStartY = obj->anim.localPosY;
        {
            ObjModel* joint = Player_GetActiveModel((int)obj);
            s16 scratch[3];
            f32 camBuf[2];
            ObjModel_SampleJointTransform(joint, 0, 0, lbl_803E7EE0, obj->anim.rootMotionScale, jp,
                                          scratch);
            lbl_803DE438 = obj->anim.localPosY + jp[1];
            lbl_803DE43C = inner->climbTargetY + lbl_803DAF88[1];
            camBuf[0] = inner->climbEndLocalY;
            camBuf[1] = inner->climbBaseY;
            if (inner->curAnimId != 0x48 && inner->curAnimId != 0x47)
            {
                (*gCameraInterface)->setMode(0x4b, 1, 1, 8, camBuf, 0, 0);
            }
        }
    }
    else
    {
        if (obj->anim.currentMoveProgress > lbl_803E7FF4)
        {
            Object_ObjAnimAdvanceMove((int)obj, ((PlayerState*)state)->baddie.moveSpeed, fv, NULL);
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029FFD0;
            return 0x10;
        }
    }
    {
        f32 mp = obj->anim.currentMoveProgress;
        if (mp >= lbl_803E7F18)
        {
            f32 g = lbl_803E8028 * (lbl_803E802C * mp - lbl_803E7F18);
            f32 c;
            c = (g < lbl_803E7EA4) ? lbl_803E7EA4 : ((g > lbl_803E7EE0) ? lbl_803E7EE0 : g);
            obj->anim.localPosY = c * (lbl_803DE43C - lbl_803DE438) + inner->climbStartY;
        }
    }
    ObjAnim_WriteStateWord((ObjAnimComponent*)obj, OBJANIM_STATE_INDEX_CURRENT,
                           OBJANIM_STATE_WORD_PREV_EVENT_STATE, 0);
    ObjAnim_WriteStateWord((ObjAnimComponent*)obj, OBJANIM_STATE_INDEX_ACTIVE,
                           OBJANIM_STATE_WORD_PREV_EVENT_STATE, 0);
    ObjAnim_WriteStateWord((ObjAnimComponent*)obj, OBJANIM_STATE_INDEX_ACTIVE,
                           OBJANIM_STATE_WORD_EVENT_COUNTDOWN, inner->eventCountdown);
    Object_ObjAnimAdvanceMove((int)obj, ((PlayerState*)state)->baddie.moveSpeed, fv, NULL);
    (*gCameraInterface)
        ->overridePos(obj->anim.localPosX,
                      obj->anim.currentMoveProgress *
                              (inner->climbTargetY - obj->anim.localPosY) +
                          obj->anim.localPosY,
                      obj->anim.localPosZ);
    fn_802AB5A4(obj, (int)inner, 5);
    return 0;
}

int playerState0D(GameObject* obj, int targetState)
{
    PlayerState* inner = obj->extra;
    f32 fz;
    *(u32*)&((PlayerState*)inner)->flags360 &= ~PLAYER_FLAG_HITDETECT;
    *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_NO_POS_VELOCITY;
    *(int*)((char*)targetState + 4) |= 0x100000;
    fz = lbl_803E7EA4;
    ((PlayerState*)targetState)->baddie.animSpeedA = fz;
    ((PlayerState*)targetState)->baddie.animSpeedB = fz;
    *(int*)((char*)targetState + 0) |= 0x200000;
    obj->anim.velocityX = fz;
    obj->anim.velocityZ = fz;
    return 0;
}

int playerStateClimbLedge(int obj, int state, f32 fv)
{
    int inner = *(int*)&((GameObject*)obj)->extra;
    f32 diff = ((PlayerState*)inner)->leapTargetY - ((PlayerState*)inner)->characterHeightOffset;
    f32 blend;
    f32 z;
    f32 t;

    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        *(s16*)((char*)state + 0x278) = 0xc;
        ((PlayerState*)inner)->stateHandler = 0;
        ((GameObject*)obj)->anim.velocityY = lbl_803E7EA4;
    }
    z = lbl_803E7EA4;
    ((PlayerState*)inner)->probeHitDist = z;
    {
        int in2 = *(int*)&((GameObject*)obj)->extra;
        *(u32*)((char*)in2 + 0x360) &= ~2LL;
        *(u32*)((char*)in2 + 0x360) |= 0x2000LL;
    }
    *(u32*)((char*)state + 4) |= 0x100000;
    ((PlayerState*)state)->baddie.animSpeedA = z;
    ((PlayerState*)state)->baddie.animSpeedB = z;
    *(u32*)state |= 0x200000;
    ((GameObject*)obj)->anim.velocityX = z;
    ((GameObject*)obj)->anim.velocityZ = z;
    *(u32*)((char*)state + 4) |= 0x8000000;
    gPlayerPrevMoveId = gPlayerCurrentMoveId;
    switch (gPlayerCurrentMoveId)
    {
    case 0:
        t = (((GameObject*)obj)->anim.localPosY - ((PlayerState*)inner)->moveStartY) /
            (diff - ((PlayerState*)inner)->moveStartY);
        ((GameObject*)obj)->anim.localPosX =
            t * (((PlayerState*)inner)->moveEnd2X - ((PlayerState*)inner)->moveStartX) +
            ((PlayerState*)inner)->moveStartX;
        ((GameObject*)obj)->anim.localPosZ =
            t * (((PlayerState*)inner)->moveEnd2Z - ((PlayerState*)inner)->moveStartZ) +
            ((PlayerState*)inner)->moveStartZ;
        (*gPlayerInterface)->updateAnimRootMotion((void*)obj, (void*)state, fv, 0x14);
        ((GameObject*)obj)->anim.localPosY =
            *(f32*)((char*)state + 0x2b4) * timeDelta + ((GameObject*)obj)->anim.localPosY;
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            f32 d2;
            f32 v;
            gPlayerCurrentMoveId = 2;
            blend = lbl_803E7EF8;
            v = (5.0f + diff) - ((GameObject*)obj)->anim.localPosY;
            v = lbl_803E8030 * -v;
            if (v >= lbl_803E7EA4)
            {
                ((GameObject*)obj)->anim.velocityY = sqrtf(v);
            }
            else
            {
                ((GameObject*)obj)->anim.velocityY = lbl_803E7EA4;
            }
            Sfx_PlayFromObject(obj,
                               (u16)(((PlayerState*)inner)->characterId == 0 ? SFXTRIG_foxcom_var : SFXTRIG_sa_def));
        }
        break;
    case 2:
        if (((GameObject*)obj)->anim.localPosY >= diff)
        {
            gPlayerCurrentMoveId = 3;
            blend = lbl_803E800C;
            ((GameObject*)obj)->anim.velocityY = z;
            ((GameObject*)obj)->anim.localPosX = ((PlayerState*)inner)->moveEnd2X;
            ((GameObject*)obj)->anim.localPosY = diff;
            ((GameObject*)obj)->anim.localPosZ = ((PlayerState*)inner)->moveEnd2Z;
        }
        else
        {
            ((GameObject*)obj)->anim.velocityY = lbl_803E7E88 * fv + ((GameObject*)obj)->anim.velocityY;
            t = (((GameObject*)obj)->anim.localPosY - ((PlayerState*)inner)->moveStartY) /
                (diff - ((PlayerState*)inner)->moveStartY);
            ((GameObject*)obj)->anim.localPosX =
                t * (((PlayerState*)inner)->moveEnd2X - ((PlayerState*)inner)->moveStartX) +
                ((PlayerState*)inner)->moveStartX;
            ((GameObject*)obj)->anim.localPosZ =
                t * (((PlayerState*)inner)->moveEnd2Z - ((PlayerState*)inner)->moveStartZ) +
                ((PlayerState*)inner)->moveStartZ;
        }
        break;
    case 3:
        ((PlayerState*)inner)->moveStartX = ((GameObject*)obj)->anim.localPosX;
        ((PlayerState*)inner)->moveStartY = ((GameObject*)obj)->anim.localPosY;
        ((PlayerState*)inner)->moveStartZ = ((GameObject*)obj)->anim.localPosZ;
        if (((GameObject*)obj)->anim.currentMoveProgress > lbl_803E7F48)
        {
            if (((PlayerState*)state)->baddie.moveInputZ > lbl_803E7F10)
            {
                gPlayerCurrentMoveId = 5;
                blend = lbl_803E8024;
                Sfx_PlayFromObject(obj,
                                   (u16)(((PlayerState*)inner)->characterId == 0 ? SFXTRIG_jump3 : SFXTRIG_sabrepush));
                if (((PlayerState*)inner)->unk608 == 5)
                {
                    Sfx_PlayFromObject(obj, SFXTRIG_fox_swimstroke222);
                }
            }
            else if (((PlayerState*)state)->baddie.moveInputZ < lbl_803E801C)
            {
                ((PlayerState*)inner)->launchYaw = *(s16*)obj;
                gPlayerCurrentMoveId = 7;
                blend = lbl_803E8034;
                ((GameObject*)obj)->anim.velocityY = z;
            }
            else if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
            {
                gPlayerCurrentMoveId = 6;
                blend = lbl_803E8038;
            }
        }
        break;
    case 6:
        ((PlayerState*)inner)->moveStartX = ((GameObject*)obj)->anim.localPosX;
        ((PlayerState*)inner)->moveStartY = ((GameObject*)obj)->anim.localPosY;
        ((PlayerState*)inner)->moveStartZ = ((GameObject*)obj)->anim.localPosZ;
        if (((PlayerState*)state)->baddie.moveInputZ > lbl_803E7F10)
        {
            gPlayerCurrentMoveId = 5;
            blend = lbl_803E8024;
            Sfx_PlayFromObject(obj, (u16)(((PlayerState*)inner)->characterId == 0 ? SFXTRIG_jump3 : SFXTRIG_sabrepush));
            if (((PlayerState*)inner)->unk608 == 5)
            {
                Sfx_PlayFromObject(obj, SFXTRIG_fox_swimstroke222);
            }
        }
        else if (((PlayerState*)state)->baddie.moveInputZ < lbl_803E801C)
        {
            ((PlayerState*)inner)->launchYaw = *(s16*)obj;
            gPlayerCurrentMoveId = 7;
            blend = lbl_803E8034;
            ((GameObject*)obj)->anim.velocityY = z;
        }
        break;
    case 7:
    {
        f32 y2 =
            ((PlayerState*)inner)->launchDirZ * (lbl_803E7E98 + lbl_803DC6C0) + ((PlayerState*)inner)->launchAnchorZ;
        s16 ang;
        ((GameObject*)obj)->anim.localPosX = ((GameObject*)obj)->anim.currentMoveProgress *
                                                 ((((PlayerState*)inner)->launchDirX * (lbl_803E7E98 + lbl_803DC6C0) +
                                                   ((PlayerState*)inner)->launchAnchorX) -
                                                  ((PlayerState*)inner)->moveStartX) +
                                             ((PlayerState*)inner)->moveStartX;
        ((GameObject*)obj)->anim.localPosZ =
            ((GameObject*)obj)->anim.currentMoveProgress * (y2 - ((PlayerState*)inner)->moveStartZ) +
            ((PlayerState*)inner)->moveStartZ;
        ((GameObject*)obj)->anim.velocityY = -(lbl_803E7F6C * timeDelta - ((GameObject*)obj)->anim.velocityY);
        ang = -(lbl_803E7F98 * ((GameObject*)obj)->anim.currentMoveProgress - (f32)((PlayerState*)inner)->launchYaw);
        ((PlayerState*)inner)->yaw = ang;
        ((PlayerState*)inner)->targetYaw = ang;
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            ((PlayerState*)state)->baddie.animSpeedC = z;
            ((PlayerState*)state)->baddie.animSpeedA = z;
            ((PlayerState*)state)->baddie.animSpeedB = z;
            ((GameObject*)obj)->anim.velocityX = z;
            ((GameObject*)obj)->anim.velocityZ = z;
            *(u32*)((char*)state + 4) &= ~0x100000;
            fn_802AB5A4((GameObject*)obj, inner, 5);
            ((ByteFlags*)((char*)inner + 0x3f0))->b80 = 0;
            ((ByteFlags*)((char*)inner + 0x3f0))->b10 = 0;
            ((ByteFlags*)((char*)inner + 0x3f0))->b08 = 0;
            staffFn_80170380(gPlayerStaffObject, 2);
            ((ByteFlags*)((char*)inner + 0x3f0))->b02 = 0;
            *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
            ObjHits_SyncObjectPositionIfDirty((GameObject*)obj);
            ((ByteFlags*)((char*)inner + 0x3f0))->b40 = 0;
            ((ByteFlags*)((char*)inner + 0x3f0))->b04 = 1;
            ((ByteFlags*)((char*)inner + 0x3f4))->b10 = 1;
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
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
            return 3;
        }
        break;
    }
    case 5:
        t = ((GameObject*)obj)->anim.currentMoveProgress / lbl_803E7F68;
        z = (t < z) ? z : ((t > lbl_803E7EE0) ? lbl_803E7EE0 : t);
        ((GameObject*)obj)->anim.localPosX = z * (((PlayerState*)inner)->moveEndX - ((PlayerState*)inner)->moveStartX) +
                                             ((PlayerState*)inner)->moveStartX;
        ((GameObject*)obj)->anim.localPosY = z * (((PlayerState*)inner)->moveEndY - ((PlayerState*)inner)->moveStartY) +
                                             ((PlayerState*)inner)->moveStartY;
        ((GameObject*)obj)->anim.localPosZ = z * (((PlayerState*)inner)->moveEndZ - ((PlayerState*)inner)->moveStartZ) +
                                             ((PlayerState*)inner)->moveStartZ;
        if (((GameObject*)obj)->anim.currentMoveProgress > lbl_803E7F68)
        {
            *(u32*)((char*)state + 4) &= ~0x100000;
            fn_802AB5A4((GameObject*)obj, inner, 5);
            *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
            return 2;
        }
        break;
    default:
        gPlayerCurrentMoveId = 0;
        gPlayerPrevMoveId = 0;
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E803C;
        ObjAnim_SetCurrentMove(obj, lbl_80332EF0[gPlayerCurrentMoveId], lbl_803E7EA4, 0);
        ObjAnim_SetCurrentEventStepFrames((ObjAnimComponent*)obj, 10);
        {
            s16 ang = getAngle(((PlayerState*)inner)->launchDirX, ((PlayerState*)inner)->launchDirZ);
            ((PlayerState*)inner)->yaw = ang;
            ((PlayerState*)inner)->targetYaw = ang;
        }
        ((GameObject*)obj)->anim.velocityY = lbl_803E7EA4;
        Obj_TransformWorldPointToLocal(((GameObject*)obj)->anim.worldPosX, ((GameObject*)obj)->anim.worldPosY,
                                       ((GameObject*)obj)->anim.worldPosZ, (f32*)(obj + 0xc), (f32*)(obj + 0x10),
                                       (f32*)(obj + 0x14), *(int*)&((GameObject*)obj)->anim.parent);
        objHitDetectFn_80062e84((GameObject*)obj, ((PlayerState*)inner)->groundObject, 1);
        ((PlayerState*)inner)->moveStartX = ((GameObject*)obj)->anim.localPosX;
        ((PlayerState*)inner)->moveStartY = ((GameObject*)obj)->anim.localPosY;
        ((PlayerState*)inner)->moveStartZ = ((GameObject*)obj)->anim.localPosZ;
        {
            char* xf = *(char**)((char*)inner + 0x4c4);
            if (xf != NULL)
            {
                Obj_TransformWorldPointToLocal(((PlayerState*)inner)->launchAnchorX,
                                               ((PlayerState*)inner)->launchAnchorY,
                                               ((PlayerState*)inner)->launchAnchorZ, (f32*)(inner + 0x5d4),
                                               (f32*)(inner + 0x5d8), (f32*)(inner + 0x5dc), (int)xf);
                Obj_TransformWorldPointToLocal(((PlayerState*)inner)->moveEndX,
                                               ((PlayerState*)inner)->moveEndY,
                                               ((PlayerState*)inner)->moveEndZ, (f32*)(inner + 0x5ec),
                                               (f32*)(inner + 0x5f0), (f32*)(inner + 0x5f4),
                                               (int)((PlayerState*)inner)->groundObject);
                Obj_TransformWorldPointToLocal(((PlayerState*)inner)->moveEnd2X,
                                               ((PlayerState*)inner)->moveEnd2Y,
                                               ((PlayerState*)inner)->moveEnd2Z, (f32*)(inner + 0x5f8),
                                               (f32*)(inner + 0x5fc), (f32*)(inner + 0x600),
                                               (int)((PlayerState*)inner)->groundObject);
                ((PlayerState*)inner)->leapTargetY =
                    ((PlayerState*)inner)->leapTargetY - ((PlayerState*)inner)->groundObject->anim.localPosY;
                ((PlayerState*)inner)->leapBaseY =
                    ((PlayerState*)inner)->leapBaseY - ((PlayerState*)inner)->groundObject->anim.localPosY;
                ((PlayerState*)inner)->unk609 = 0;
            }
        }
        break;
    }
    if (gPlayerPrevMoveId != gPlayerCurrentMoveId)
    {
        ObjAnim_SetCurrentMove(obj, lbl_80332EF0[gPlayerCurrentMoveId], lbl_803E7EA4, 0);
        ((PlayerState*)state)->baddie.moveSpeed = blend;
    }
    fn_802AB5A4((GameObject*)obj, inner, 5);
    return 0;
}

int playerState0B(GameObject* obj, int state)
{
    PlayerState* inner = obj->extra;
    f32 fz;
    *(u32*)&((PlayerState*)inner)->flags360 &= ~PLAYER_FLAG_HITDETECT;
    *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_NO_POS_VELOCITY;
    *(int*)((char*)state + 4) |= 0x100000;
    fz = lbl_803E7EA4;
    ((PlayerState*)state)->baddie.animSpeedA = fz;
    ((PlayerState*)state)->baddie.animSpeedB = fz;
    *(int*)((char*)state + 0) |= 0x200000;
    obj->anim.velocityX = fz;
    obj->anim.velocityZ = fz;
    *(int*)((char*)state + 4) |= 0x8000000;
    obj->anim.velocityY = fz;
    *(int*)((char*)state + 0) |= 0x200000;
    switch (gPlayerCurrentMoveId)
    {
    case 0x12:
    case 0x1a:
        if (*(int*)&((PlayerState*)state)->baddie.eventFlags & 1)
        {
            Sfx_PlayFromObject((int)obj, (u16)(inner->characterId == 0 ? SFXTRIG_jump3 : SFXTRIG_sabrepush));
        }
        if ((((u32)inner->flags3F0 >> 5) & 1) || gPlayerCurrentMoveId == 0x1a)
        {
            if (*(int*)&((PlayerState*)state)->baddie.eventFlags & 0x80)
            {
                Sfx_PlayFromObject((int)obj, SFXTRIG_fox_swimstroke222);
            }
        }
    case 0xe:
    case 0x16:
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            *(int*)((char*)state + 4) &= ~0x100000;
            fn_802AB5A4(obj, (int)inner, 5);
            *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
            return 2;
        }
        break;
    default:
    {
        f32 lo;
        f32 hi;
        f32 t;
        f32 r;
        f32 v;
        if (inner->unk606 == 0x10)
        {
            gPlayerCurrentMoveId = 0x1a;
            lo = lbl_803E8040;
            hi = lbl_803E8044;
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F28;
        }
        else if ((v = inner->leapSpeed) >= lbl_803E8040)
        {
            gPlayerCurrentMoveId = 0xe;
            lo = lbl_803E8040;
            hi = lbl_803E7F30;
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F0C;
        }
        else if (v >= *(f32*)&lbl_803E8048)
        {
            gPlayerCurrentMoveId = 0x16;
            lo = lbl_803E8048;
            hi = lbl_803E8040;
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E804C;
        }
        else
        {
            gPlayerCurrentMoveId = 0x12;
            lo = lbl_803E8018;
            hi = lbl_803E8048;
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E804C;
        }
        t = (inner->leapSpeed - lo) / (hi - lo);
        t = t * lbl_803E7FAC;
        r = (t < lbl_803E7EA4) ? lbl_803E7EA4 : ((t > lbl_803E7FAC) ? lbl_803E7FAC : t);
        inner->secondaryBlendAmount = (s16)r;
        ObjAnim_SetCurrentMove((int)obj, lbl_80332EF0[gPlayerCurrentMoveId], lbl_803E7EA4, 0);
        ObjAnim_SetCurrentEventStepFrames((ObjAnimComponent*)obj, 0xa);
        inner->targetYaw = inner->yaw = (s16)getAngle(inner->launchDirX, inner->launchDirZ);
        Obj_TransformWorldPointToLocal(obj->anim.worldPosX, obj->anim.worldPosY,
                                       obj->anim.worldPosZ, (f32*)((char*)obj + 0xc),
                                       (f32*)((char*)obj + 0x10), (f32*)((char*)obj + 0x14),
                                       *(int*)&obj->anim.parent);
        objHitDetectFn_80062e84(obj, inner->groundObject, 1);
        inner->moveStartX = obj->anim.localPosX;
        inner->moveStartY = obj->anim.localPosY;
        inner->moveStartZ = obj->anim.localPosZ;
        if (*(void**)((char*)inner + 0x4c4) != NULL)
        {
            Obj_TransformWorldPointToLocal(*(f32*)((int)inner + 0x5d4), *(f32*)((int)inner + 0x5d8),
                                           *(f32*)((int)inner + 0x5dc), (f32*)((char*)inner + 0x5d4),
                                           (f32*)((char*)inner + 0x5d8), (f32*)((char*)inner + 0x5dc),
                                           (u32)inner->groundObject);
            Obj_TransformWorldPointToLocal(*(f32*)((int)inner + 0x5ec), *(f32*)((int)inner + 0x5f0),
                                           *(f32*)((int)inner + 0x5f4), (f32*)((char*)inner + 0x5ec),
                                           (f32*)((char*)inner + 0x5f0), (f32*)((char*)inner + 0x5f4),
                                           (u32)inner->groundObject);
            Obj_TransformWorldPointToLocal(*(f32*)((int)inner + 0x5f8), *(f32*)((int)inner + 0x5fc),
                                           *(f32*)((int)inner + 0x600), (f32*)((char*)inner + 0x5f8),
                                           (f32*)((char*)inner + 0x5fc), (f32*)((char*)inner + 0x600),
                                           (u32)inner->groundObject);
            inner->leapTargetY = inner->leapTargetY - inner->groundObject->anim.localPosY;
            inner->leapBaseY = inner->leapBaseY - inner->groundObject->anim.localPosY;
            inner->unk609 = 0;
        }
        break;
    }
    }
    obj->anim.localPosX =
        obj->anim.currentMoveProgress * (((PlayerState*)inner)->moveEndX - inner->moveStartX) +
        inner->moveStartX;
    obj->anim.localPosY =
        obj->anim.currentMoveProgress * (((PlayerState*)inner)->moveEndY - inner->moveStartY) +
        inner->moveStartY;
    obj->anim.localPosZ =
        obj->anim.currentMoveProgress * (((PlayerState*)inner)->moveEndZ - inner->moveStartZ) +
        inner->moveStartZ;
    Object_ObjAnimSetSecondaryBlendMove((ObjAnimComponent*)obj, lbl_80332EF0[gPlayerCurrentMoveId + 2],
                                        inner->secondaryBlendAmount);
    fn_802AB5A4(obj, (int)inner, 5);
    return 0;
}

int playerStateGrabLedge(GameObject* obj, int state)
{
    int inner = *(int*)&obj->extra;
    f32 fz;
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        void* sub;
        Sfx_PlayFromObject((int)obj,
                           (u16)(((PlayerState*)inner)->characterId == 0 ? SFXTRIG_foxcom_heel : SFXTRIG_sa_def01));
        *(s16*)((char*)state + 0x278) = 0xa;
        ((PlayerState*)inner)->stateHandler = 0;
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
    fz = lbl_803E7EA4;
    ((PlayerState*)inner)->probeHitDist = fz;
    {
        int e = *(int*)&obj->extra;
        *(u32*)((char*)e + 0x360) &= ~2LL;
        *(u32*)((char*)e + 0x360) |= 0x2000LL;
    }
    *(int*)((char*)state + 4) |= 0x100000;
    ((PlayerState*)state)->baddie.animSpeedA = fz;
    ((PlayerState*)state)->baddie.animSpeedB = fz;
    *(int*)((char*)state + 0) |= 0x200000;
    obj->anim.velocityX = fz;
    obj->anim.velocityZ = fz;
    *(int*)((char*)state + 4) |= 0x8000000;
    obj->anim.velocityY = fz;
    switch (obj->anim.currentMove)
    {
    case 0xd:
    case 0x22:
    {
        f32 c;
        f32 d = obj->anim.currentMoveProgress / lbl_803E7F44;
        c = (d < lbl_803E7EA4) ? lbl_803E7EA4 : ((d > lbl_803E7EE0) ? lbl_803E7EE0 : d);
        obj->anim.localPosX =
            c * (((PlayerState*)inner)->moveEnd2X - ((PlayerState*)inner)->moveStartX) +
            ((PlayerState*)inner)->moveStartX;
        obj->anim.localPosY =
            ((PlayerState*)inner)->moveStartY -
            obj->anim.currentMoveProgress *
                (((PlayerState*)inner)->moveStartY -
                 (((PlayerState*)inner)->leapTargetY - ((PlayerState*)inner)->characterHeightOffset));
        obj->anim.localPosZ =
            c * (((PlayerState*)inner)->moveEnd2Z - ((PlayerState*)inner)->moveStartZ) +
            ((PlayerState*)inner)->moveStartZ;
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            ObjAnim_SetCurrentMove((int)obj, lbl_80332EF0[6], lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E8038;
            gPlayerCurrentMoveId = 6;
            fn_802AB5A4(obj, inner + 4, 5);
            *(int*)&((PlayerState*)state)->baddie.unk308 = 0;
            return 0xd;
        }
        break;
    }
    default:
    {
        int m;
        int d = (u16)getAngle(((PlayerState*)inner)->launchDirX, ((PlayerState*)inner)->launchDirZ) -
                ((PlayerState*)inner)->targetYaw;
        if (d > 0x8000)
        {
            d -= 0xffff;
        }
        if (d < -0x8000)
        {
            d += 0xffff;
        }
        m = ((PlayerState*)inner)->unk607 == 1 ? 0xb : 0xa;
        ((PlayerState*)inner)->targetYaw += d;
        ((PlayerState*)inner)->yaw = ((PlayerState*)inner)->targetYaw;
        Obj_TransformWorldPointToLocal(obj->anim.worldPosX, obj->anim.worldPosY,
                                       obj->anim.worldPosZ, (f32*)((char*)obj + 0xc),
                                       (f32*)((char*)obj + 0x10), (f32*)((char*)obj + 0x14),
                                       *(int*)&obj->anim.parent);
        objHitDetectFn_80062e84(obj, ((PlayerState*)inner)->groundObject, 1);
        ((PlayerState*)inner)->moveStartX = obj->anim.localPosX;
        ((PlayerState*)inner)->moveStartY = obj->anim.localPosY;
        ((PlayerState*)inner)->moveStartZ = obj->anim.localPosZ;
        ObjAnim_SetCurrentMove((int)obj, lbl_80332EF0[m], lbl_803E7EA4, 4);
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F34;
        if (((PlayerState*)inner)->curAnimId != 0x48 && ((PlayerState*)inner)->curAnimId != 0x47)
        {
            struct
            {
                s16 a;
                u8 b;
                u8 c;
            } shk;
            shk.a = 0;
            shk.b = 0;
            shk.c = 1;
            (*gCameraInterface)->setMode(0x43, 1, 0, 4, &shk, 0, 0xff);
        }
        if (*(void**)((char*)inner + 0x4c4) != NULL)
        {
            Obj_TransformWorldPointToLocal(*(f32*)((int)inner + 0x5d4), *(f32*)((int)inner + 0x5d8),
                                           *(f32*)((int)inner + 0x5dc), (f32*)((char*)inner + 0x5d4),
                                           (f32*)((char*)inner + 0x5d8), (f32*)((char*)inner + 0x5dc),
                                           (u32)((PlayerState*)inner)->groundObject);
            Obj_TransformWorldPointToLocal(*(f32*)((int)inner + 0x5ec), *(f32*)((int)inner + 0x5f0),
                                           *(f32*)((int)inner + 0x5f4), (f32*)((char*)inner + 0x5ec),
                                           (f32*)((char*)inner + 0x5f0), (f32*)((char*)inner + 0x5f4),
                                           (u32)((PlayerState*)inner)->groundObject);
            Obj_TransformWorldPointToLocal(*(f32*)((int)inner + 0x5f8), *(f32*)((int)inner + 0x5fc),
                                           *(f32*)((int)inner + 0x600), (f32*)((char*)inner + 0x5f8),
                                           (f32*)((char*)inner + 0x5fc), (f32*)((char*)inner + 0x600),
                                           (u32)((PlayerState*)inner)->groundObject);
            ((PlayerState*)inner)->leapTargetY =
                ((PlayerState*)inner)->leapTargetY - ((PlayerState*)inner)->groundObject->anim.localPosY;
            ((PlayerState*)inner)->leapBaseY =
                ((PlayerState*)inner)->leapBaseY - ((PlayerState*)inner)->groundObject->anim.localPosY;
            ((PlayerState*)inner)->unk609 = 0;
        }
        break;
    }
    }
    ((PlayerState*)inner)->cameraFlags |= 4;
    fn_802AB5A4(obj, inner + 4, 5);
    return 0;
}

int playerState09(GameObject* obj, int state)
{
    int inner = *(int*)&obj->extra;
    f32 fz;
    int flagsBase;
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        *(s16*)((char*)state + 0x278) = 9;
        ((PlayerState*)inner)->stateHandler = 0;
    }
    flagsBase = *(int*)&obj->extra;
    *(u32*)((char*)flagsBase + 0x360) &= ~2LL;
    *(u32*)((char*)flagsBase + 0x360) |= 0x2000LL;
    *(int*)((char*)state + 4) |= 0x100000;
    fz = lbl_803E7EA4;
    ((PlayerState*)state)->baddie.animSpeedA = fz;
    ((PlayerState*)state)->baddie.animSpeedB = fz;
    *(int*)((char*)state + 0) |= 0x200000;
    obj->anim.velocityX = fz;
    obj->anim.velocityZ = fz;
    *(int*)((char*)state + 4) |= 0x8000000;
    obj->anim.velocityY = fz;
    switch (obj->anim.currentMove)
    {
    case 0x419:
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            ObjAnim_SetCurrentMove((int)obj, lbl_80332EF0[6], fz, 0);
            gPlayerCurrentMoveId = 6;
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E8038;
            fn_802AB5A4(obj, inner + 4, 5);
            *(int*)&((PlayerState*)state)->baddie.unk308 = 0;
            return 0xd;
        }
        break;
    default:
    {
        f32 k;
        ObjAnim_SetCurrentMove((int)obj, 0x419, fz, 1);
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7E90;
        ((PlayerState*)inner)->targetYaw =
            (s16)getAngle(((PlayerState*)inner)->launchDirX, ((PlayerState*)inner)->launchDirZ);
        ((PlayerState*)inner)->yaw = ((PlayerState*)inner)->targetYaw;
        k = lbl_803E7F10;
        obj->anim.worldPosX = k * ((PlayerState*)inner)->launchDirX + *(f32*)((int)inner + 0x5d4);
        obj->anim.worldPosY =
            ((PlayerState*)inner)->leapTargetY - ((PlayerState*)inner)->characterHeightOffset;
        obj->anim.worldPosZ = k * ((PlayerState*)inner)->launchDirZ + *(f32*)((int)inner + 0x5dc);
        Obj_TransformWorldPointToLocal(obj->anim.worldPosX, obj->anim.worldPosY,
                                       obj->anim.worldPosZ, &obj->anim.localPosX,
                                       &obj->anim.localPosY, &obj->anim.localPosZ,
                                       *(int*)&obj->anim.parent);
        objHitDetectFn_80062e84(obj, ((PlayerState*)inner)->groundObject, 1);
        if (*(void**)((char*)inner + 0x4c4) != NULL)
        {
            Obj_TransformWorldPointToLocal(*(f32*)((int)inner + 0x5d4), *(f32*)((int)inner + 0x5d8),
                                           *(f32*)((int)inner + 0x5dc), (f32*)((char*)inner + 0x5d4),
                                           (f32*)((char*)inner + 0x5d8), (f32*)((char*)inner + 0x5dc),
                                           (u32)((PlayerState*)inner)->groundObject);
            Obj_TransformWorldPointToLocal(*(f32*)((int)inner + 0x5ec), *(f32*)((int)inner + 0x5f0),
                                           *(f32*)((int)inner + 0x5f4), (f32*)((char*)inner + 0x5ec),
                                           (f32*)((char*)inner + 0x5f0), (f32*)((char*)inner + 0x5f4),
                                           (u32)((PlayerState*)inner)->groundObject);
            Obj_TransformWorldPointToLocal(*(f32*)((int)inner + 0x5f8), *(f32*)((int)inner + 0x5fc),
                                           *(f32*)((int)inner + 0x600), (f32*)((char*)inner + 0x5f8),
                                           (f32*)((char*)inner + 0x5fc), (f32*)((char*)inner + 0x600),
                                           (u32)((PlayerState*)inner)->groundObject);
            ((PlayerState*)inner)->leapTargetY =
                ((PlayerState*)inner)->leapTargetY - ((PlayerState*)inner)->groundObject->anim.localPosY;
            ((PlayerState*)inner)->leapBaseY =
                ((PlayerState*)inner)->leapBaseY - ((PlayerState*)inner)->groundObject->anim.localPosY;
            ((PlayerState*)inner)->unk609 = 0;
        }
        break;
    }
    }
    fn_802AB5A4(obj, inner + 4, 5);
    return 0;
}

int playerState08(GameObject* obj, int state, f32 fv)
{
    PlayerState* inner = obj->extra;
    int c;
    int i;
    int* list;
    u8 buf[64];
    f32 dist;
    int cnt41;
    int cnt20;
    int cnt30;

    dist = lbl_803E8050;
    if (inner->curAnimId != 0x44)
    {
        if (((PlayerState*)inner)->heldObj != NULL)
        {
            c = (s8)playerCheckIfClimbingOntoWall((int)obj, (int)inner, state, buf, fv, 0x22);
        }
        else
        {
            c = (s8)playerCheckIfClimbingOntoWall((int)obj, (int)inner, state, buf, fv, (u32)-0x141);
        }
        if ((s8)c == -1)
        {
            inner->climbProbeResult = -1;
            inner->climbProbeStableCount = 0;
        }
        else if ((s8)c == inner->climbProbeResult)
        {
            if (++inner->climbProbeStableCount > 200)
            {
                inner->climbProbeStableCount = 200;
            }
        }
        else
        {
            inner->climbProbeResult = c;
            inner->climbProbeStableCount = 0;
        }
        switch (inner->climbProbeResult)
        {
        case 0:
            if (((ByteFlags*)((char*)inner + 0x3f1))->b01)
            {
                *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029FFD0;
                return 0xf;
            }
            break;
        case 9:
            if (((ByteFlags*)((char*)inner + 0x3f1))->b01)
            {
                *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029FFD0;
                return 0x13;
            }
            break;
        case 4:
            gPlayerCurrentMoveId = -1;
            *(int*)&((PlayerState*)state)->baddie.unk308 = 0;
            return 0xd;
        case 5:
            if (((PlayerState*)inner)->heldObj == NULL)
            {
                gPlayerCurrentMoveId = -1;
                *(int*)&((PlayerState*)state)->baddie.unk308 = 0;
                return 0xc;
            }
            break;
        case 6:
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029DAE0;
            return -0x1d;
        case 0xd:
            *(int*)&((PlayerState*)state)->baddie.unk308 = 0;
            return 0x1d;
        case 7:
            fn_802AE9C8(obj, (int)inner, state);
            return 0;
        case 8:
            *(int*)&((PlayerState*)state)->baddie.unk308 = 0;
            return 0xb;
        case 0xb:
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)objUpdateHitboxPos;
            return 0x1c;
        case 10:
            *(int*)&((PlayerState*)state)->baddie.unk308 = 0;
            return 0x17;
        default:
            break;
        }
        if (((PlayerState*)inner)->heldObj == NULL && ((ByteFlags*)((char*)inner + 0x3f4))->b40)
        {
            list = (int*)ObjGroup_GetObjects(STAFFACTIVATED_OBJ_GROUP, &cnt41);
            for (i = 0; i < cnt41; i++)
            {
                int o = *list;
                gPlayerInteractTarget = (GameObject*)o;
                if ((*(u8*)((char*)o + 0xaf) & 4) != 0 && (*(u8*)((char*)o + 0xaf) & 0x10) == 0)
                {
                    switch ((u8)objGetByteParam1C(gPlayerInteractTarget))
                    {
                    case 2:
                        setAButtonIcon(2);
                        if ((*(int*)&((PlayerState*)state)->baddie.unk31C & 0x100) != 0)
                        {
                            buttonDisable(0, PAD_BUTTON_A);
                            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_80298924;
                            return 0x34;
                        }
                        break;
                    case 4:
                    case 5:
                        setAButtonIcon(0xe);
                        if ((*(int*)&((PlayerState*)state)->baddie.unk31C & 0x100) != 0)
                        {
                            buttonDisable(0, PAD_BUTTON_A);
                            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_80298924;
                            return 0x36;
                        }
                        break;
                    case 3:
                        setAButtonIcon(2);
                        if ((*(int*)&((PlayerState*)state)->baddie.unk31C & 0x100) != 0)
                        {
                            buttonDisable(0, PAD_BUTTON_A);
                            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_80298924;
                            return 0x35;
                        }
                        break;
                    case 0:
                        break;
                    }
                }
                list++;
            }
        }
    }
    ObjGroup_GetObjects(BABYCLOUDRUNNER_OBJGROUP, &cnt20);
    mainSetBits(GAMEBIT_ITEM_Flute_Disabled, !cnt20);
    if ((*gGameUIInterface)->isCurrentTriggerClear() != 0)
    {
        if ((*gGameUIInterface)->isEventReady(0x1ee) != 0)
        {
            char* found;
            s16* def = NULL;
            buttonDisable(0, PAD_BUTTON_A);
            found = (char*)ObjGroup_FindNearestObject(0xf, obj, &dist);
            if (found != NULL)
            {
                def = *(s16**)((char*)found + 0x4c);
            }
            if (def != NULL && *def == 0x860 && (*(u8*)((char*)found + 0xaf) & 4) != 0)
            {
                mainSetBits(GAMEBIT_ITEM_DinoHorn_3F1, 1);
                mainSetBits(GAMEBIT_ITEM_DinoHorn_3D8, 1);
                mainSetBits(GAMEBIT_ITEM_DinoHorn_651, 1);
            }
            return 0;
        }
        if ((*gGameUIInterface)->isEventReady(0x953) != 0 && gPlayerChildObject == NULL)
        {
            GameObject* player;
            void* att;
            buttonDisable(0, PAD_BUTTON_A);
            if (gPlayerPathObject != NULL && ((ByteFlags*)((char*)inner + 0x3f4))->b40)
            {
                inner->staffActionRequest = 1;
                ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
            }
            player = Obj_GetPlayerObject();
            if (Obj_IsLoadingLocked() == 0)
            {
                att = NULL;
            }
            else
            {
                ObjPlacement* setup = Obj_AllocObjectSetup(0x24, 0x62d);
                setup->objectId = 0x62d;
                setup->color[0] = 2;
                setup->color[2] = 0xff;
                setup->color[1] = 1;
                setup->color[3] = 0xff;
                setup->posX = player->anim.localPosX;
                setup->posY = player->anim.localPosY;
                setup->posZ = player->anim.localPosZ;
                att = Obj_SetupObject(setup, 4, player->anim.mapEventSlot, -1, player->anim.parent);
                gPlayerChildObject = att;
            }
            ObjLink_AttachChild((GameObject*)obj, (GameObject*)att, 1);
            (*gObjectTriggerInterface)->runSequence(0xd, (void*)obj, -1);
        }
    }
    if (inner->curAnimId != 0x44 && (*gGameUIInterface)->isCurrentTriggerClear() != 0 &&
        (*gGameUIInterface)->isEventReady(0x13e) != 0 &&
        (ObjGroup_GetObjects(LANTERNFIREFLY_OBJGROUP, &cnt30), cnt30 == 0))
    {
        gameBitDecrement(0x13d);
        if (Obj_IsLoadingLocked() != 0)
        {
            ObjPlacement* setup = Obj_AllocObjectSetup(0x24, 0x43b);
            setup->objectId = 0x43b;
            setup->size = 9;
            setup->color[0] = 2;
            setup->color[2] = 0xff;
            setup->color[1] = 1;
            setup->color[3] = 0xff;
            setup->posX = obj->anim.localPosX;
            setup->posY = lbl_803E7F58 + obj->anim.localPosY;
            setup->posZ = obj->anim.localPosZ;
            *(u8*)((char*)setup + 0x19) = 1;
            Obj_SetupObject(setup, 5, -1, -1, obj->anim.parent);
        }
        (*(void (*)(void))(*(int*)((char*)*gGameUIInterface + 0x10)))();
        return 0;
    }
    {
        if (*(u8*)&((PlayerState*)inner)->staffGrown != 0)
        {
            int r2;
            if ((*(int*)&((PlayerState*)state)->baddie.unk31C & 0x200) != 0 && gPlayerPathObject != NULL &&
                ((ByteFlags*)((char*)inner + 0x3f4))->b40)
            {
                inner->staffActionRequest = 0;
                ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 0;
            }
            {
                int in2 = *(int*)&obj->extra;
                u8 b;
                if ((*(int*)&((PlayerState*)state)->baddie.unk31C & 0x100) != 0 &&
                    (b = ((ByteFlags*)((char*)in2 + 0x3f4))->b40, b != 0))
                {
                    if (gPlayerPathObject != NULL && b != 0)
                    {
                        *(u8*)((char*)in2 + 0x8b4) = 4;
                        ((ByteFlags*)((char*)in2 + 0x3f4))->b08 = 1;
                    }
                    *(int*)&((PlayerState*)state)->baddie.unk308 = 0;
                    r2 = 0x32;
                }
                else
                {
                    r2 = 0;
                }
                if (r2 != 0)
                {
                    return r2;
                }
            }
        }
        else
        {
            if ((*(int*)&((PlayerState*)state)->baddie.unk31C & 0x100) != 0)
            {
                int ok2;
                if (((PlayerState*)inner)->heldObj != NULL || !((ByteFlags*)((char*)inner + 0x3f4))->b40 ||
                    ((ByteFlags*)((char*)inner + 0x3f0))->b20 || ((ByteFlags*)((char*)inner + 0x3f0))->b10)
                {
                    ok2 = 0;
                }
                else
                {
                    ok2 = 1;
                }
                if (ok2 != 0)
                {
                    if (((PlayerState*)inner)->staffActionRequest == 2 ||
                        (inner->cameraTargetObject != NULL && inner->targetObjectDist < lbl_803E8054 &&
                         inner->targetObjectBearingAbs < 0x4000 && ((PlayerState*)inner)->targetObjModelType == 1))
                    {
                        if (gPlayerPathObject != NULL && ((ByteFlags*)((char*)inner + 0x3f4))->b40)
                        {
                            inner->staffActionRequest = 4;
                            ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
                        }
                        *(int*)&((PlayerState*)state)->baddie.unk308 = 0;
                        return 0x32;
                    }
                    if (gPlayerPathObject != NULL && ((ByteFlags*)((char*)inner + 0x3f4))->b40)
                    {
                        inner->staffActionRequest = 2;
                        ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 0;
                    }
                }
            }
        }
        return 0;
    }
}

void fn_802A49A8(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    inner->moveParams = (int)gPlayerDefaultMoveParams;
    inner->moveAnimTable = (int)gPlayerMoveTableA;
}

int playerStateThrowing(GameObject* obj, int state)
{
    PlayerState* inner = obj->extra;
    f32 k;

    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        if (((PlayerState*)inner)->heldObj != NULL)
        {
            ObjHits_MarkObjectPositionDirty((ObjAnimComponent*)inner->heldObj);
        }
        ObjAnim_SetCurrentMove((int)obj, 0x443, lbl_803E7EAC, 0);
        *(s16*)((char*)state + 0x278) = 1;
        inner->stateHandler = (int)fn_802A514C;
    }
    k = lbl_803E7EA4;
    ((PlayerState*)state)->baddie.animSpeedC = k;
    ((PlayerState*)state)->baddie.animSpeedB = k;
    ((PlayerState*)state)->baddie.animSpeedA = k;
    obj->anim.velocityX = k;
    obj->anim.velocityY = k;
    obj->anim.velocityZ = k;
    ((PlayerState*)state)->baddie.moveSpeed = lbl_803E8058;

    if (*(int*)&((PlayerState*)state)->baddie.eventFlags & 1)
    {
        Sfx_PlayFromObject((int)obj, (u16)(inner->characterId == 0 ? SFXTRIG_foxcom_decoy : SFXTRIG_sa_jump02));
    }

    if (((PlayerState*)inner)->heldObj == NULL && *(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
    {
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
        return 2;
    }
    if (((PlayerState*)inner)->heldObj != NULL && obj->anim.currentMoveProgress > lbl_803E7E9C)
    {
        inner->isHoldingObject = 0;
        if (((PlayerState*)inner)->heldObj != NULL)
        {
            GameObject* s2 = inner->heldObj;
            s16 id = s2->anim.seqId;
            if (id == 0x3cf || id == 0x662)
            {
                objThrowFn_80182504(s2);
            }
            else
            {
                objSaveFn_800ea774(s2);
            }
            *(s16*)((char*)inner->heldObj + 6) &= ~0x4000;
            *(int*)((char*)inner->heldObj + 0xf8) = 0;
            inner->heldObj = 0;
        }
    }
    return 0;
}

void fn_802A4B4C(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    void* p = ((PlayerState*)inner)->heldObj;
    if (p != NULL)
    {
        ((GameObject*)p)->userData2 = 1;
    }
    *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
}

int playerState06(GameObject* obj, int state)
{
    PlayerState* inner = obj->extra;
    GameObject* sub;

    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        ObjAnim_SetCurrentMove((int)obj, 0x447, lbl_803E7EA4, 0);
        *(s16*)((char*)state + 0x278) = 1;
        inner->stateHandler = (int)fn_802A514C;
    }
    if ((*(int*)&((PlayerState*)state)->baddie.eventFlags & 1) && (sub = inner->heldObj) != NULL)
    {
        switch (sub->anim.seqId)
        {
        case 0x6d:
        case 0x754:
            Sfx_PlayFromObject((int)obj, SFXTRIG_barrel_putdown_31f);
            break;
        case 0x1f4:
        case 0x1f5:
        case 0x1f6:
        case 0x1f7:
        case 0x1f8:
        case 0x1f9:
        case 0x519:
            Sfx_PlayFromObject((int)obj, SFXTRIG_weetinkoneshot);
            break;
        default:
            Sfx_PlayFromObject((int)obj, SFXTRIG_vineclimb116);
            break;
        }
    }
    ((PlayerState*)state)->baddie.animSpeedA = lbl_803E7EA4;
    ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F40;

    sub = inner->heldObj;
    if (sub == NULL && *(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
    {
        *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
        return 2;
    }
    if (sub != NULL && obj->anim.currentMoveProgress > lbl_803E7F48)
    {
        inner->isHoldingObject = 0;
        if (((PlayerState*)inner)->heldObj != NULL)
        {
            GameObject* s2 = inner->heldObj;
            s16 id = s2->anim.seqId;
            if (id == 0x3cf || id == 0x662)
            {
                objThrowFn_80182504(s2);
            }
            else
            {
                objSaveFn_800ea774(s2);
            }
            *(s16*)((char*)inner->heldObj + 6) &= ~0x4000;
            *(int*)((char*)inner->heldObj + 0xf8) = 0;
            inner->heldObj = 0;
        }
    }
    return 0;
}

int playerState05(GameObject* obj, int state)
{
    PlayerState* inner = obj->extra;
    ((PlayerState*)state)->baddie.animSpeedB = lbl_803E7EA4;
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        if (gPlayerPathObject != NULL)
        {
            if (((ByteFlags*)((char*)inner + 0x3f4))->b40)
            {
                inner->staffActionRequest = 1;
                ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
            }
        }
        *(s16*)((char*)state + 0x278) = 1;
        inner->stateHandler = (int)fn_802A514C;
    }
    switch (obj->anim.currentMove)
    {
    case 5:
    {
        void* sub;
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F40;
        ((PlayerState*)state)->baddie.animSpeedA = lbl_803E7EA4;
        sub = ((PlayerState*)inner)->heldObj;
        if (sub != NULL)
        {
            f32 amt;
            if (obj->anim.currentMoveProgress > lbl_803E7E98)
            {
                ((GameObject*)sub)->userData2 = 1;
            }
            amt = interpolate((f32)inner->targetObjectBearing, lbl_803E805C, timeDelta);
            inner->targetYaw = (f32)inner->targetYaw + amt;
            inner->yaw = inner->targetYaw;
        }
        if (obj->anim.currentMoveProgress > lbl_803E7F2C)
        {
            inner->moveAnimTable = (int)lbl_80333110;
            ObjAnim_SetCurrentMove((int)obj, *(s16*)inner->moveAnimTable, lbl_803E7EA4, 0);
            *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
            return 2;
        }
        break;
    }
    default:
    {
        void* sub = ((PlayerState*)inner)->heldObj;
        if (sub != NULL && ((GameObject*)sub)->anim.seqId == 0x112)
        {
            inner->moveAnimTable = (int)lbl_80333110;
            *(int*)((char*)inner->heldObj + 0xf8) = 1;
            ObjAnim_SetCurrentMove((int)obj, *(s16*)inner->moveAnimTable, lbl_803E7EA4, 0);
            *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
            return 2;
        }
        else
        {
            ObjAnim_SetCurrentMove((int)obj, 5, lbl_803E7EA4, 0);
        }
        break;
    }
    }
    if (*(int*)&((PlayerState*)state)->baddie.eventFlags & 1)
    {
        u16 snd;
        if (inner->characterId == 0)
        {
            snd = 0x320;
        }
        else
        {
            snd = 0x3c1;
        }
        Sfx_PlayFromObject((int)obj, snd);
    }
    return 0;
}

int playerState04(int obj, int state, f32 fv)
{
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        ObjAnim_SetCurrentMove(obj, 0x92, lbl_803E7EA4, 0);
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E8060;
    }
    (*gPlayerInterface)->updateAnimRootMotion((void*)obj, (void*)state, fv, 3);
    if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
    {
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
        return 2;
    }
    return 0;
}

int playerStateIceSpell(int obj, int state, f32 fv)
{
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        ObjAnim_SetCurrentMove(obj, 0x8e, lbl_803E7EA4, 0);
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E8060;
    }
    (*gPlayerInterface)->updateAnimRootMotion((void*)obj, (void*)state, fv, 3);
    if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
    {
        void** p;
        int z[2];
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
        showDeathMenu();
    }
    return 0;
}

void fn_802A514C(GameObject* obj, int state)
{
    PlayerState* inner = obj->extra;
    ((ByteFlags*)((char*)inner + 0x3f1))->b80 = 0;
    {
        s16 mode = ((PlayerState*)state)->baddie.controlMode;
        if (mode != 2 && mode != 1 && mode != 5 && mode != 7 && mode != 6)
        {
            void* sub;
            inner->isHoldingObject = 0;
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
                *(s16*)((char*)inner->heldObj + 0x6) &= ~0x4000;
                *(int*)((char*)inner->heldObj + 0xf8) = 0;
                inner->heldObj = 0;
            }
        }
    }
    {
        s16 mode = ((PlayerState*)state)->baddie.controlMode;
        if (mode != 2 && mode != 1)
        {
            ((ByteFlags*)((char*)inner + 0x3f0))->b10 = 0;
            ((ByteFlags*)((char*)inner + 0x3f0))->b80 = 0;
            ((ByteFlags*)((char*)inner + 0x3f0))->b40 = 0;
            ((ByteFlags*)((char*)inner + 0x3f0))->b08 = 0;
            ((ByteFlags*)((char*)inner + 0x3f0))->b04 = 0;
            inner->staffHoldFrames = 0;
            ((ByteFlags*)((char*)inner + 0x3f0))->b20 = 0;
            if (((ByteFlags*)((char*)inner + 0x3f1))->b20)
            {
                s16 t = obj->anim.rotX;
                inner->yaw = t;
                inner->targetYaw = t;
                inner->lastInputHeading = t;
                inner->baddie.animSpeedB = lbl_803E7EA4;
            }
            ((ByteFlags*)((char*)inner + 0x3f1))->b20 = 0;
            if (((ByteFlags*)((char*)inner + 0x3f1))->b10)
            {
                u8 anim = inner->curAnimId;
                if (anim != 0x48 && anim != 0x47 && getCurSeqNo() == 0)
                {
                    (*gCameraInterface)->setMode(0x42, 0, 1, 0, NULL, 0x1e, 0xff);
                    ((ByteFlags*)((char*)inner + 0x3f1))->b10 = 0;
                }
            }
            *(u32*)&((PlayerState*)inner)->flags360 &= ~0x2000000LL;
        }
    }
    if (((PlayerState*)state)->baddie.controlMode != 2)
    {
        staffFn_80170380(gPlayerStaffObject, 2);
        ((ByteFlags*)((char*)inner + 0x3f0))->b02 = 0;
        *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
        ObjHits_SyncObjectPositionIfDirty(obj);
    }
    gPlayerSubState = 1;
}

int playerStateMoving(int obj, int state, f32 fv)
{
    int inner;
    int dir;
    f32 t;
    f32 spd;
    f32 ya;

    inner = *(int*)&((GameObject*)obj)->extra;
    ((ByteFlags*)((char*)inner + 0x3f1))->b02 = 0;
    ((ByteFlags*)((char*)inner + 0x3f1))->b04 = 0;
    ((ByteFlags*)((char*)inner + 0x3f1))->b08 = 0;
    ((ByteFlags*)((char*)inner + 0x3f2))->b10 = 0;
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        *(u32*)&((PlayerState*)inner)->flags360 |= 0x2000000LL;
        ((ByteFlags*)((char*)inner + 0x3f0))->b80 = 0;
        ((ByteFlags*)((char*)inner + 0x3f0))->b40 = 0;
        ((ByteFlags*)((char*)inner + 0x3f3))->b40 = 0;
        *(u8*)&((PlayerState*)inner)->gaitLevel = 0;
        ((PlayerState*)inner)->unk81E = 0;
        ((ByteFlags*)((char*)inner + 0x3f2))->b10 = 1;
    }
    {
        int r = fn_802AC7DC(obj, state, inner, fv);
        if (r != 0)
        {
            return r;
        }
    }
    playerSetMovingAnims(obj, inner);
    {
        u32 fl = *(u8*)((char*)inner + 0x3f0);
        if ((fl >> 5 & 1) != 0)
        {
            *(u32*)state |= 0x200000;
            *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_NO_POS_VELOCITY;
            *(u32*)&((PlayerState*)inner)->flags360 |= 0x2000000LL;
            *(s16*)((char*)state + 0x278) = 2;
            ((PlayerState*)inner)->stateHandler = (int)fn_802A514C;
            if (((u32) * (u8*)((char*)inner + 0x3f1) >> 5 & 1) != 0)
            {
                ((PlayerState*)inner)->maxSpeed = lbl_803E7F2C;
            }
            else
            {
                ((PlayerState*)inner)->maxSpeed = lbl_803E8064;
            }
        }
        else if (((u32) * (u8*)((char*)inner + 0x3f1) >> 5 & 1) != 0)
        {
            *(u32*)&((PlayerState*)inner)->flags360 |= 0x2000000LL;
            *(u32*)state |= 0x800000;
            *(s16*)((char*)state + 0x278) = 0;
            ((PlayerState*)inner)->maxSpeed = lbl_803E7ED4;
        }
        else if ((fl >> 3 & 1) != 0 || (fl >> 2 & 1) != 0)
        {
            *(u32*)state |= 0x200000;
            *(u32*)&((PlayerState*)inner)->flags360 |= 0x2000000LL;
            ((PlayerState*)inner)->maxSpeed = lbl_803E8068;
        }
        else
        {
            *(u32*)&((PlayerState*)inner)->flags360 |= 0x2000000LL;
            *(u32*)state |= 0x800000;
            *(s16*)((char*)state + 0x278) = 0;
            ((PlayerState*)inner)->maxSpeed = lbl_803E806C;
        }
    }
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        if (((u32) * (u8*)((char*)inner + 0x3f1) >> 5 & 1) == 0 && ((u32) * (u8*)((char*)inner + 0x3f0) >> 2 & 1) == 0)
        {
            ((PlayerState*)inner)->yaw = ((PlayerState*)inner)->yaw + ((PlayerState*)inner)->yawRate * 0xb6;
        }
        ((PlayerState*)inner)->yawRateSigned = 0;
        ((PlayerState*)inner)->yawRate = 0;
    }
    {
        t = ((((PlayerState*)state)->baddie.inputMagnitude - lbl_803E7F14) / lbl_803E7F2C < lbl_803E7EA4)
                ? lbl_803E7EA4
                : (((((PlayerState*)state)->baddie.inputMagnitude - lbl_803E7F14) / lbl_803E7F2C > lbl_803E7EE0)
                       ? lbl_803E7EE0
                       : (((PlayerState*)state)->baddie.inputMagnitude - lbl_803E7F14) / lbl_803E7F2C);
    }
    ((PlayerState*)inner)->currentSpeed =
        (((PlayerState*)inner)->maxSpeed - lbl_803E7F6C) * (t * ((PlayerState*)inner)->speedScale);
    {
        u32 fl = *(u8*)((char*)inner + 0x3f0);
        if ((fl >> 6 & 1) != 0)
        {
            *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_HEADING_LOCK;
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E8070;
            {
                s16 cd = (s16)(lbl_803E7F98 * ((GameObject*)obj)->anim.currentMoveProgress +
                               (f32) * (int*)((char*)inner + 0x858));
                ((PlayerState*)inner)->targetYaw = cd;
                ((PlayerState*)inner)->lastInputHeading = cd;
            }
            if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
            {
                ((ByteFlags*)((char*)inner + 0x3f0))->b40 = 0;
                {
                    int a = ((PlayerState*)inner)->yaw;
                    ((PlayerState*)inner)->targetYaw = a;
                    ((PlayerState*)inner)->lastInputHeading = a;
                }
                *(u8*)&((PlayerState*)inner)->gaitLevel = 0xc;
                ((ByteFlags*)((char*)inner + 0x3f1))->b04 = 1;
                ((ByteFlags*)((char*)inner + 0x3f1))->b08 = 1;
            }
            ((PlayerState*)state)->baddie.animSpeedC =
                ((PlayerState*)inner)->unk844 * timeDelta + ((PlayerState*)state)->baddie.animSpeedC;
            ((PlayerState*)inner)->currentSpeed = lbl_803E7EA4;
            if (((GameObject*)obj)->anim.currentMoveProgress > lbl_803E7EFC &&
                ((GameObject*)obj)->anim.currentMoveProgress < lbl_803E8074)
            {
                ((PlayerState*)inner)->pendingFxFlags |= 8;
            }
        }
        else if ((fl >> 4 & 1) != 0)
        {
            fn_802AE650((GameObject*)obj, inner, state);
        }
        else if ((fl >> 7 & 1) != 0)
        {
            int r = fn_802AE480((GameObject*)obj, inner, state);
            if (r != 0)
            {
                *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
                return 2;
            }
        }
        else if ((fl >> 1 & 1) != 0)
        {
            int leave;
            *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_KNOCKBACK;
            {
                f32 z = 0.0f;
                ((PlayerState*)state)->baddie.animSpeedC = z;
                ((PlayerState*)state)->baddie.animSpeedC = z;
                ((PlayerState*)state)->baddie.animSpeedB = z;
                ((PlayerState*)state)->baddie.animSpeedA = z;
                ((GameObject*)obj)->anim.velocityX = z;
                ((GameObject*)obj)->anim.velocityY = z;
                ((GameObject*)obj)->anim.velocityZ = z;
                {
                    f32 w = 20.0f;
                    ((PlayerState*)inner)->targetYawSmoothRate = w;
                    ((PlayerState*)inner)->targetYawRateLimit = z;
                    ((PlayerState*)inner)->yawSmoothRate = w;
                    ((PlayerState*)inner)->yawRateLimit = z;
                    ((PlayerState*)inner)->currentSpeed = z;
                }
            }
            {
                u32 fl2;
                int stay;
                if ((padGetTriggers(0) & 0x20) != 0 &&
                    ((u32) * (u8*)((char*)inner + 0x3f4) >> 6 & 1) != 0 &&
                    ((fl2 = *(u8*)((char*)inner + 0x3f0)) >> 5 & 1) == 0 && (fl2 >> 3 & 1) == 0 &&
                    (fl2 >> 2 & 1) == 0 && ((PlayerState*)inner)->curAnimId != 0x44 &&
                    ((PlayerState*)inner)->heldObj == NULL && ((PlayerState*)inner)->baddie.targetObj == NULL &&
                    ((u32) * (u8*)((char*)inner + 0x3f6) >> 6 & 1) == 0 &&
                    ((PlayerState*)inner)->baddie.controlMode != 0x26 &&
                    (((GameObject*)obj)->objectFlags & OBJECT_OBJFLAG_PARENT_SLACK) == 0 &&
                    ((PlayerState*)inner)->idleDelayTimer == lbl_803E7EA4)
                {
                    stay = 1;
                }
                else
                {
                    stay = 0;
                }
                if (!stay)
                {
                    if (gPlayerPathObject != 0 && ((u32) * (u8*)((char*)inner + 0x3f4) >> 6 & 1) != 0)
                    {
                        ((PlayerState*)inner)->staffActionRequest = 1;
                        ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
                    }
                    staffFn_80170380(gPlayerStaffObject, 2);
                    ((ByteFlags*)((char*)inner + 0x3f0))->b02 = 0;
                    *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
                    ObjHits_SyncObjectPositionIfDirty((GameObject*)obj);
                    leave = 1;
                }
                else
                {
                    leave = 0;
                }
            }
            if (leave)
            {
                *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
                return 2;
            }
        }
        else if ((fl >> 5 & 1) != 0)
        {
            fn_802ADE80((GameObject*)obj, inner, state);
        }
        else if ((fl >> 3 & 1) != 0)
        {
            fn_802ADC08((GameObject*)obj, inner, state);
        }
        else if ((fl >> 2 & 1) != 0)
        {
            int r = fn_802AD2F4((GameObject*)obj, inner, state);
            if (r != 0)
            {
                *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
                return 2;
            }
        }
    }
    {
        int calm;
        {
            u32 fl = *(u8*)((char*)inner + 0x3f0);
            if ((fl >> 5 & 1) == 0 && (fl >> 6 & 1) == 0 && (fl >> 4 & 1) == 0 && (fl >> 2 & 1) == 0 &&
                (fl >> 3 & 1) == 0 && (fl >> 1 & 1) == 0 && ((PlayerState*)inner)->heldObj == NULL &&
                ((PlayerState*)inner)->curAnimId != 0x44)
            {
                calm = 1;
            }
            else
            {
                calm = 0;
            }
        }
        if (calm && (((PlayerState*)inner)->buttonsJustPressed & PAD_BUTTON_X) != 0)
        {
            fn_802AED2C((GameObject*)obj, inner, state);
        }
    }
    {
        int ok;
        {
            u32 fl = *(u8*)((char*)inner + 0x3f0);
            if ((fl >> 5 & 1) == 0 && (fl >> 6 & 1) == 0 && (fl >> 7 & 1) == 0 && (fl >> 4 & 1) == 0 &&
                (fl >> 2 & 1) == 0 && (fl >> 3 & 1) == 0 && ((u32) * (u8*)((char*)inner + 0x3f1) >> 5 & 1) == 0)
            {
                ok = 1;
            }
            else
            {
                ok = 0;
            }
        }
        if (ok && ((PlayerState*)state)->baddie.animSpeedC > lbl_803E7EAC + *(f32*)(((PlayerState*)inner)->moveParams + 0x14) &&
            (((PlayerState*)inner)->inputMagnitude < lbl_803E8030 || ((PlayerState*)inner)->yawRateSigned >= 0x96))
        {
            ((PlayerState*)inner)->pendingFxFlags |= 8;
            ((ByteFlags*)((char*)inner + 0x3f0))->b80 = 1;
            ((PlayerState*)inner)->animSoundId = ((PlayerState*)inner)->altAnimSoundId;
            *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_HEADING_LOCK;
            ((PlayerState*)inner)->unk844 = ((PlayerState*)state)->baddie.animSpeedA;
            ObjAnim_SetCurrentMove(obj, *(s16*)(((PlayerState*)inner)->moveAnimTable + 0x3c), lbl_803E7EA4, 0);
        }
    }
    {
        u32 fl = *(u8*)((char*)inner + 0x3f0);
        if ((fl >> 7 & 1) == 0 && (fl >> 6 & 1) == 0 && ((u32) * (u8*)((char*)inner + 0x3f1) >> 5 & 1) == 0)
        {
            if (((PlayerState*)inner)->yawRateSigned < 0x96)
            {
                f32 d = interpolate((f32) * (int*)((char*)inner + 0x47c),
                                    lbl_803E7EE0 / ((PlayerState*)inner)->targetYawSmoothRate, timeDelta);
                {
                    f32 m =
                        timeDelta * (((PlayerState*)inner)->targetYawRateLimit * ((PlayerState*)inner)->leanCurveScale);
                    d = (d > m) ? m : d;
                }
                if (((PlayerState*)inner)->targetYawRate < 0)
                {
                    d = -d;
                }
                ((PlayerState*)inner)->targetYaw =
                    (s16)(gPlayerDegToBinAngle * d + (f32) ((PlayerState*)inner)->targetYaw);
            }
            if (((PlayerState*)inner)->yawRateSigned < 0x96)
            {
                f32 d = interpolate((f32) * (int*)((char*)inner + 0x488), lbl_803E7EE0 / ((PlayerState*)inner)->yawSmoothRate,
                                    timeDelta);
                {
                    f32 m = ((PlayerState*)inner)->yawRateLimit * timeDelta;
                    d = (d > m) ? m : d;
                }
                if (((PlayerState*)inner)->yawRate < 0)
                {
                    d = -d;
                }
                ((PlayerState*)inner)->yaw = (s16)(gPlayerDegToBinAngle * d + (f32) * (s16*)((char*)inner + 0x484));
            }
            else
            {
                u32 fl3 = *(u8*)((char*)inner + 0x3f0);
                if ((fl3 >> 3 & 1) == 0 && (fl3 >> 2 & 1) == 0 && (fl3 >> 4 & 1) == 0 &&
                    ((PlayerState*)state)->baddie.animSpeedC <= *(f32*)(((PlayerState*)inner)->moveParams + 4) &&
                    ((PlayerState*)state)->baddie.animSpeedA <= *(f32*)(((PlayerState*)inner)->moveParams + 0xc))
                {
                    ((PlayerState*)inner)->yaw = ((PlayerState*)inner)->yaw + ((PlayerState*)inner)->yawRate * 0xb6;
                }
            }
        }
    }
    {
        u32 fl;
        u32 fl1 = ((PlayerState*)inner)->flags3F1;
        if ((fl1 >> 5 & 1) != 0)
        {
            spd = ((PlayerState*)inner)->maxSpeed *
                  (t * -mathSinf((gPlayerPi * (f32) * (int*)((char*)inner + 0x474)) / lbl_803E7F98));
            ya = ((PlayerState*)inner)->maxSpeed *
                 (t * -mathCosf((gPlayerPi * (f32) * (int*)((char*)inner + 0x474)) / lbl_803E7F98));
            t = interpolate(spd - ((PlayerState*)inner)->smoothVelX, ((PlayerState*)inner)->velSmoothRate, timeDelta);
            {
                f32 dy = interpolate(ya - ((PlayerState*)inner)->smoothVelZ, ((PlayerState*)inner)->velSmoothRate,
                                     timeDelta);
                ((PlayerState*)inner)->smoothVelX = ((PlayerState*)inner)->smoothVelX + t;
                ((PlayerState*)inner)->smoothVelZ = ((PlayerState*)inner)->smoothVelZ + dy;
            }
            ((PlayerState*)state)->baddie.animSpeedC =
                sqrtf(((PlayerState*)inner)->smoothVelX * ((PlayerState*)inner)->smoothVelX +
                      ((PlayerState*)inner)->smoothVelZ * ((PlayerState*)inner)->smoothVelZ);
            {
                ((PlayerState*)state)->baddie.animSpeedC =
                    (((PlayerState*)state)->baddie.animSpeedC < **(f32**)((char*)inner + 0x400))
                        ? **(f32**)((char*)inner + 0x400)
                        : ((((PlayerState*)state)->baddie.animSpeedC > ((PlayerState*)inner)->maxSpeed)
                               ? ((PlayerState*)inner)->maxSpeed
                               : ((PlayerState*)state)->baddie.animSpeedC);
            }
            t = mathSinf((gPlayerPi * (f32) ((PlayerState*)inner)->targetYaw) / lbl_803E7F98);
            {
                f32 sn = mathCosf((gPlayerPi * (f32) ((PlayerState*)inner)->targetYaw) / lbl_803E7F98);
                f32 nx = -((PlayerState*)inner)->smoothVelZ * sn - ((PlayerState*)inner)->smoothVelX * t;
                ya = ((PlayerState*)inner)->smoothVelX * sn - ((PlayerState*)inner)->smoothVelZ * t;
                ((PlayerState*)state)->baddie.animSpeedA =
                    ((PlayerState*)state)->baddie.animSpeedA +
                    interpolate(nx - ((PlayerState*)state)->baddie.animSpeedA, ((PlayerState*)inner)->targetAnimSpeed,
                                timeDelta);
                ((PlayerState*)state)->baddie.animSpeedB =
                    ((PlayerState*)state)->baddie.animSpeedB +
                    interpolate(ya - ((PlayerState*)state)->baddie.animSpeedB, ((PlayerState*)inner)->targetAnimSpeed,
                                timeDelta);
            }
            spd = ((PlayerState*)state)->baddie.animSpeedB;
            spd = (spd < lbl_803E7EA4) ? -spd : spd;
            t = ((PlayerState*)state)->baddie.animSpeedA;
            t = (t < *(f32*)&lbl_803E7EA4) ? -t : t;
            {
                int r = ObjAnim_SampleRootCurvePhase((ObjAnimComponent*)obj,
                                                     ((PlayerState*)state)->baddie.animSpeedC,
                                                     (f32*)(state + 0x2a0));
                if (r == 0)
                {
                    ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F78;
                }
            }
            if (((u32) * (u8*)((char*)inner + 0x3f0) >> 5 & 1) != 0)
            {
                ((PlayerState*)state)->baddie.moveSpeed = ((PlayerState*)state)->baddie.moveSpeed * lbl_803E7E98;
            }
            if (t > spd)
            {
                if (((PlayerState*)state)->baddie.animSpeedA < lbl_803E7EA4)
                {
                    dir = 1;
                }
                else
                {
                    dir = 0;
                }
            }
            else if (((PlayerState*)state)->baddie.animSpeedB >= lbl_803E7EA4)
            {
                dir = 3;
            }
            else
            {
                dir = 2;
            }
        }
        else
        {
            fl = *(u8*)((char*)inner + 0x3f0);
            if ((fl >> 6 & 1) == 0 && (fl1 >> 2 & 1) == 0 && (fl >> 4 & 1) == 0 && (fl1 >> 1 & 1) == 0 &&
                (fl >> 3 & 1) == 0 && (fl >> 2 & 1) == 0 && (fl >> 1 & 1) == 0)
            {
                f32 d = interpolate(((PlayerState*)inner)->currentSpeed - ((PlayerState*)state)->baddie.animSpeedC,
                                    ((PlayerState*)inner)->velSmoothRate, timeDelta);
                d = (d < lbl_803E7EA8 * timeDelta) ? lbl_803E7EA8 * timeDelta : ((d > lbl_803E7EFC * timeDelta) ? lbl_803E7EFC * timeDelta : d);
                if (((PlayerState*)inner)->yawRateSigned >= 0x96 && d > lbl_803E7EA4)
                {
                    d = lbl_803E7ED4 * -d;
                }
                ((PlayerState*)state)->baddie.animSpeedC = ((PlayerState*)state)->baddie.animSpeedC + d;
                {
                    ((PlayerState*)state)->baddie.animSpeedC =
                        (((PlayerState*)state)->baddie.animSpeedC < **(f32**)((char*)inner + 0x400))
                            ? **(f32**)((char*)inner + 0x400)
                            : ((((PlayerState*)state)->baddie.animSpeedC > ((PlayerState*)inner)->maxSpeed)
                                   ? ((PlayerState*)inner)->maxSpeed
                                   : ((PlayerState*)state)->baddie.animSpeedC);
                }
                ((PlayerState*)state)->baddie.animSpeedB = lbl_803E7EA4;
            }
            else if (((ByteFlags*)((char*)inner + 0x3f0))->b08 != 0 ||
                     ((ByteFlags*)((char*)inner + 0x3f0))->b04 != 0)
            {
                t = ((PlayerState*)inner)->currentSpeed *
                    -mathSinf((gPlayerPi * (gPlayerDegToBinAngle * (f32) * (int*)((char*)inner + 0x48c))) / lbl_803E7F98);
                ya = ((PlayerState*)inner)->currentSpeed *
                     mathCosf((gPlayerPi * (gPlayerDegToBinAngle * (f32) * (int*)((char*)inner + 0x48c))) / lbl_803E7F98);
                if (((u32) * (u8*)((char*)inner + 0x3f0) >> 2 & 1) != 0)
                {
                    ((PlayerState*)state)->baddie.animSpeedC =
                        ((PlayerState*)state)->baddie.animSpeedC * powfBitEstimate(lbl_803E7F90, timeDelta);
                }
                else
                {
                    ((PlayerState*)state)->baddie.animSpeedC =
                        -(lbl_803E7F20 * timeDelta - ((PlayerState*)state)->baddie.animSpeedC);
                }
                {
                    f32 v2 = lbl_803E7E8C * ya;
                    f32 m = (v2 < lbl_803E8078) ? lbl_803E8078 : ((v2 > lbl_803E807C) ? lbl_803E807C : v2);
                    ((PlayerState*)state)->baddie.animSpeedC = m * timeDelta + ((PlayerState*)state)->baddie.animSpeedC;
                }
                {
                    f32 v = ((PlayerState*)state)->baddie.animSpeedC;
                    ((PlayerState*)state)->baddie.animSpeedC =
                        (v < lbl_803E8080)
                            ? lbl_803E8080
                            : ((v > lbl_803E7EFC + ((PlayerState*)inner)->maxSpeed) ? lbl_803E7EFC + ((PlayerState*)inner)->maxSpeed
                                                                            : v);
                }
                t = t * lbl_803E7F74;
                ((PlayerState*)state)->baddie.animSpeedB =
                    ((PlayerState*)state)->baddie.animSpeedB +
                    interpolate(t - ((PlayerState*)state)->baddie.animSpeedB, lbl_803E807C, timeDelta);
            }
            else
            {
                f32 lim;
                f32 v;
                v = ((PlayerState*)state)->baddie.animSpeedC;
                lim = ((PlayerState*)inner)->maxSpeed;
                ((PlayerState*)state)->baddie.animSpeedC = (v < -lim) ? -lim : ((v > lim) ? lim : v);
            }
            {
                if (((u32) * (u8*)((char*)inner + 0x3f0) >> 4 & 1) == 0 &&
                    ((u32) * (u8*)((char*)inner + 0x3f1) >> 1 & 1) == 0 &&
                    ((u32) * (u8*)((char*)inner + 0x3f0) >> 1 & 1) == 0)
                {
                    ((PlayerState*)state)->baddie.animSpeedA =
                        ((PlayerState*)state)->baddie.animSpeedA +
                        interpolate(((PlayerState*)state)->baddie.animSpeedC - ((PlayerState*)state)->baddie.animSpeedA,
                                    ((PlayerState*)inner)->targetAnimSpeed, timeDelta);
                }
            }
            dir = 0;
        }
    }
    {
        u32 fl = *(u8*)((char*)inner + 0x3f0);
        if ((fl >> 7 & 1) == 0 && (fl >> 6 & 1) == 0 && (fl >> 4 & 1) == 0 && (fl >> 2 & 1) == 0 &&
            (fl >> 3 & 1) == 0 && (fl >> 1 & 1) == 0)
        {
            int step;
            int locked;
            locked = 0;
            if (((u32) * (u8*)((char*)inner + 0x3f1) >> 3 & 1) != 0)
            {
                locked = 1;
                spd = lbl_803E7EA4;
            }
            else
            {
                spd = ((GameObject*)obj)->anim.currentMoveProgress;
            }
            step = ((PlayerState*)inner)->gaitLevel / 4 * 2;
            ((PlayerState*)inner)->gaitStepLevel = (step >> 1) + 1;
            if (((PlayerState*)inner)->gaitStepLevel > 4)
            {
                ((PlayerState*)inner)->gaitStepLevel = 4;
            }
            {
                u8 c;
                if (((PlayerState*)inner)->gaitStepLevel > 3)
                {
                    c = ((PlayerState*)inner)->runAnimSoundId;
                }
                else
                {
                    c = ((PlayerState*)inner)->walkAnimSoundId;
                }
                ((PlayerState*)inner)->animSoundId = c;
            }
            {
                f32 v = ((PlayerState*)state)->baddie.animSpeedC;
                f32* tb = (f32*)((PlayerState*)inner)->moveParams;
                if (v < tb[step])
                {
                    if (((PlayerState*)inner)->gaitLevel == 4)
                    {
                        if (((PlayerState*)state)->baddie.animSpeedA < tb[4] &&
                            ((PlayerState*)state)->baddie.inputMagnitude < lbl_803E7F14)
                        {
                            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
                            return 2;
                        }
                    }
                    else
                    {
                        *(u8*)&((PlayerState*)inner)->gaitLevel -= 4;
                    }
                }
                else if (v >= tb[step + 1])
                {
                    int cc = ((PlayerState*)inner)->gaitLevel;
                    if (cc < 0x14)
                    {
                        if (cc == 0)
                        {
                            spd = lbl_803E7EA4;
                        }
                        if (v < ((PlayerState*)inner)->maxSpeed)
                        {
                            *(u8*)&((PlayerState*)inner)->gaitLevel += 4;
                        }
                    }
                }
            }
            if (locked != 0 || *(void**)((char*)inner + 0x3fc) != *(void**)((char*)inner + 0x3f8) ||
                ((GameObject*)obj)->anim.currentMove !=
                    *(s16*)(((PlayerState*)inner)->moveAnimTable + (((PlayerState*)inner)->gaitLevel + dir) * 2))
            {
                if (ObjAnim_GetCurrentEventCountdown((ObjAnimComponent*)obj) == 0 ||
                    ((u32) * (u8*)((char*)inner + 0x3f2) >> 4 & 1) != 0)
                {
                    ObjAnim_SetCurrentMove(
                        obj,
                        *(s16*)(((PlayerState*)inner)->moveAnimTable + (((PlayerState*)inner)->gaitLevel + dir) * 2),
                        spd, 0);
                    if (((u32) * (u8*)((char*)inner + 0x3f1) >> 5 & 1) != 0 &&
                        *(s8*)&((PlayerState*)state)->baddie.moveJustStartedA == 0)
                    {
                        ObjAnim_SetCurrentEventStepFrames((ObjAnimComponent*)obj, 0xc);
                    }
                }
            }
        }
    }
    {
        f32 v = (f32)((PlayerState*)state)->baddie.spawnRotY / lbl_803E7EE8;
        t = (v < (t = lbl_803E7ECC)) ? t : ((v > (t = lbl_803E7EE0)) ? t : v);
    }
    {
        f32 ad = t;
        int pos;
        if (t > lbl_803E7EA4)
        {
            pos = 1;
        }
        else
        {
            pos = 0;
        }
        if (t < *(f32*)&lbl_803E7EA4)
        {
            ad = -t;
        }
        if (((u32) * (u8*)((char*)inner + 0x3f1) >> 5 & 1) == 0)
        {
            u32 fl = *(u8*)((char*)inner + 0x3f0);
            if ((fl >> 7 & 1) == 0 && (fl >> 6 & 1) == 0 && (fl >> 4 & 1) == 0 && (fl >> 2 & 1) == 0 &&
                (fl >> 3 & 1) == 0 && (fl >> 1 & 1) == 0)
            {
                if ((fl >> 5 & 1) == 0)
                {
                    Object_ObjAnimSetSecondaryBlendMove((ObjAnimComponent*)obj,
                                                        *(s16*)(((PlayerState*)inner)->moveAnimTable + 2 +
                                                                (((PlayerState*)inner)->gaitLevel + pos) * 2),
                                                        (int)(lbl_803E7FAC * ad));
                }
                {
                    int r = ObjAnim_SampleRootCurvePhase((ObjAnimComponent*)obj,
                                                         ((PlayerState*)state)->baddie.animSpeedC,
                                                         (f32*)(state + 0x2a0));
                    if (r == 0)
                    {
                        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F78;
                    }
                }
            }
        }
    }
    fn_802ABAE8((GameObject*)obj, state, inner, t);
    return 0;
}

int playerStateIdle(int obj, int state, f32 fv)
{
    char* tbl;
    int inner;
    int move;
    f32 t;
    f32 v;
    int calm;

    tbl = (char*)lbl_80332EC0;
    inner = *(int*)&((GameObject*)obj)->extra;
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        if (((PlayerState*)state)->baddie.prevControlMode != 0x24 &&
            ((PlayerState*)state)->baddie.prevControlMode != 0x25)
        {
            ((PlayerState*)state)->baddie.animSpeedC = lbl_803E7EA4;
        }
        else if (((ByteFlags*)((char*)inner + 0x3f1))->b20 == 0)
        {
            int a = ((PlayerState*)inner)->inputHeading;
            ((PlayerState*)inner)->lastInputHeading = a;
            ((PlayerState*)inner)->yaw = a;
            ((PlayerState*)inner)->yawRate = 0;
            ((PlayerState*)inner)->yawRateSigned = 0;
        }
        else
        {
            f32 z = lbl_803E7EA4;
            ((PlayerState*)inner)->smoothVelX = z;
            ((PlayerState*)inner)->smoothVelZ = z;
        }
        ((PlayerState*)inner)->idleHoldTimer = lbl_803E7EA4;
        ((PlayerState*)inner)->idleWaitTimer = randomGetRange(800, 0x44c);
    }
    ((PlayerState*)state)->baddie.animSpeedA =
        ((PlayerState*)state)->baddie.animSpeedA -
        interpolate(((PlayerState*)state)->baddie.animSpeedA, ((PlayerState*)inner)->targetAnimSpeed, timeDelta);
    if (((PlayerState*)state)->baddie.animSpeedA <= *(f32*)(tbl + 0x398))
    {
        ((PlayerState*)state)->baddie.animSpeedA = lbl_803E7EA4;
    }
    {
        f32 z = lbl_803E7EA4;
        ((PlayerState*)state)->baddie.animSpeedB = z;
        ((GameObject*)obj)->anim.velocityX = z;
        ((GameObject*)obj)->anim.velocityZ = z;
    }
    {
        int r = fn_802AC7DC(obj, state, inner, fv);
        if (r != 0)
        {
            return r;
        }
    }
    if (*(f32*)&((PlayerState*)state)->baddie.trackedObj >= lbl_803E7FC8 &&
        ((PlayerState*)state)->baddie.inputMagnitude >= lbl_803E7FC8 &&
        ((PlayerState*)state)->baddie.animSpeedC >= *(f32*)(((PlayerState*)inner)->moveParams + 4))
    {
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
        return 3;
    }
    playerSetMovingAnims(obj, inner);
    if (*(s16**)((char*)inner + 0x3f8) == (s16*)(tbl + 0x190))
    {
        if (((PlayerState*)inner)->idleHoldTimer >= lbl_803E7FBC && **(s8**)&((PlayerState*)inner)->playerStatus <= 4)
        {
            move = 0x5d;
            fv = lbl_803E7F78;
            if (RandomTimer_UpdateRangeTrigger((void*)(inner + 0x3ec), lbl_803E7ED4, lbl_803E7F10) != 0)
            {
                Sfx_PlayFromObject(obj, SFXTRIG_fox_452);
            }
        }
        else
        {
            move = **(s16**)((char*)inner + 0x3f8);
            fv = lbl_803E7F78;
            if (((PlayerState*)inner)->idleWaitTimer <= 0)
            {
                if (((PlayerState*)inner)->curAnimId != 0x44)
                {
                    u32 i = ((PlayerState*)inner)->stopMoveIndex;
                    move = gPlayerStopMoves[i];
                    if (((PlayerState*)inner)->characterId == 0)
                    {
                        fv = ((f32*)(tbl + 0x170))[i];
                    }
                    else
                    {
                        fv = ((f32*)(tbl + 0x180))[i];
                    }
                    ((PlayerState*)inner)->stopMoveIndex += 1;
                    ((PlayerState*)inner)->stopMoveIndex = (u8)(((PlayerState*)inner)->stopMoveIndex % 3);
                }
                ((PlayerState*)inner)->idleWaitTimer = randomGetRange(800, 0x44c);
            }
        }
        if (((GameObject*)obj)->anim.currentMove == **(s16**)((char*)inner + 0x3f8))
        {
            ((PlayerState*)inner)->idleHoldTimer = ((PlayerState*)inner)->idleHoldTimer + timeDelta;
            v = ((PlayerState*)inner)->idleHoldTimer;
            ((PlayerState*)inner)->idleHoldTimer =
                (v < lbl_803E7EA4) ? lbl_803E7EA4 : ((v > lbl_803E7FBC) ? lbl_803E7FBC : v);
            *(u16*)&((PlayerState*)inner)->idleWaitTimer = (f32) * (s16*)((char*)inner + 0x812) - timeDelta;
            {
                int cd = ((PlayerState*)inner)->idleWaitTimer;
                if (cd < 0)
                {
                    cd = 0;
                }
                else if (cd > 0x44c)
                {
                    cd = 0x44c;
                }
                ((PlayerState*)inner)->idleWaitTimer = (s16)cd;
            }
        }
        else
        {
            if (((GameObject*)obj)->anim.currentMove != 0x5d)
            {
                ((PlayerState*)inner)->idleHoldTimer = lbl_803E7EA4;
            }
            ((PlayerState*)inner)->idleWaitTimer = randomGetRange(800, 0x44c);
        }
    }
    else
    {
        move = **(s16**)((char*)inner + 0x3f8);
        fv = lbl_803E7F78;
    }
    if (((ByteFlags*)((char*)inner + 0x3f0))->b20 != 0)
    {
        *(u32*)state |= 0x200000;
        *(u32*)&((PlayerState*)inner)->flags360 &= ~0x2000000LL;
        *(s16*)((char*)state + 0x278) = 1;
        ((PlayerState*)inner)->stateHandler = (int)fn_802A514C;
        if (((ByteFlags*)((char*)inner + 0x3f1))->b20 != 0)
        {
            ((PlayerState*)inner)->maxSpeed = lbl_803E7F2C;
        }
        else
        {
            ((PlayerState*)inner)->maxSpeed = lbl_803E8064;
        }
    }
    else
    {
        if (((ByteFlags*)((char*)inner + 0x3f1))->b20 != 0)
        {
            *(u32*)&((PlayerState*)inner)->flags360 |= 0x2000000LL;
            *(s16*)((char*)state + 0x278) = 0;
            ((PlayerState*)inner)->maxSpeed = lbl_803E7ED4;
        }
        else
        {
            *(u32*)&((PlayerState*)inner)->flags360 |= 0x2000000LL;
            *(s16*)((char*)state + 0x278) = 0;
            ((PlayerState*)inner)->maxSpeed = lbl_803E806C;
        }
    }
    {
        f32 frac = (((PlayerState*)state)->baddie.inputMagnitude - lbl_803E7F14) / lbl_803E7F2C;
        t = (frac < lbl_803E7EA4) ? lbl_803E7EA4 : ((frac > lbl_803E7EE0) ? lbl_803E7EE0 : frac);
    }
    ((PlayerState*)inner)->currentSpeed =
        (((PlayerState*)inner)->maxSpeed - lbl_803E7F6C) * (t * ((PlayerState*)inner)->speedScale);
    if (((ByteFlags*)((char*)inner + 0x3f0))->b20 != 0)
    {
        fn_802ADE80((GameObject*)obj, inner, state);
    }
    {
        u32 fl = ((PlayerState*)inner)->flags3F0;
        if ((fl >> 5 & 1) == 0 && (fl >> 6 & 1) == 0 && (fl >> 4 & 1) == 0 && (fl >> 2 & 1) == 0 &&
            (fl >> 3 & 1) == 0 && (fl >> 1 & 1) == 0 && ((PlayerState*)inner)->heldObj == NULL &&
            ((PlayerState*)inner)->curAnimId != 0x44)
        {
            calm = 1;
        }
        else
        {
            calm = 0;
        }
    }
    if (calm && (((PlayerState*)inner)->buttonsJustPressed & PAD_BUTTON_X) != 0)
    {
        fn_802AED2C((GameObject*)obj, inner, state);
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
        return 3;
    }
    if (((ByteFlags*)((char*)inner + 0x3f1))->b20 == 0)
    {
        ((PlayerState*)state)->baddie.animSpeedC =
            ((PlayerState*)state)->baddie.animSpeedC +
            interpolate(((PlayerState*)inner)->currentSpeed - ((PlayerState*)state)->baddie.animSpeedC,
                        ((PlayerState*)inner)->velSmoothRate, timeDelta);
    }
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        ((PlayerState*)inner)->targetYawRateSigned = 0;
        ((PlayerState*)inner)->targetYawRate = 0;
        ((PlayerState*)inner)->yawRateSigned = 0;
        ((PlayerState*)inner)->yawRate = 0;
        ((PlayerState*)inner)->animSoundId = ((PlayerState*)inner)->walkAnimSoundId;
        ((PlayerState*)inner)->gaitStepLevel = 0;
        ((PlayerState*)state)->baddie.velSmoothTime = lbl_803E8018;
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E8084;
        if (((ByteFlags*)((char*)inner + 0x3f0))->b20 == 0 && ((ByteFlags*)((char*)inner + 0x3f1))->b20 == 0)
        {
            if (((PlayerState*)state)->baddie.prevControlMode == 2)
            {
                int mA;
                int mB;
                if (((GameObject*)obj)->anim.currentMove !=
                        (mA = *(s16*)(((PlayerState*)inner)->moveAnimTable + 0x30)) &&
                    (mB = *(s16*)(((PlayerState*)inner)->moveAnimTable + 0x32),
                     ((GameObject*)obj)->anim.currentMove != mB) &&
                    ((ByteFlags*)((char*)inner + 0x3f3))->b40 == 0)
                {
                    if (((GameObject*)obj)->anim.currentMoveProgress <= lbl_803E7E98)
                    {
                        ObjAnim_SetCurrentMove(obj, mA, lbl_803E7EA4, 0);
                    }
                    else
                    {
                        ObjAnim_SetCurrentMove(obj, mB, lbl_803E7EA4, 0);
                    }
                }
                ((PlayerState*)state)->baddie.moveSpeed = lbl_803E8088;
            }
            else if (((GameObject*)obj)->anim.currentMove != move)
            {
                ObjAnim_SetCurrentMove(obj, move, lbl_803E7EA4, 0);
                ((PlayerState*)state)->baddie.moveSpeed = fv;
            }
        }
        else if (((GameObject*)obj)->anim.currentMove != move)
        {
            ObjAnim_SetCurrentMove(obj, move, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = fv;
        }
    }
    if (((GameObject*)obj)->anim.currentMove == *(s16*)(((PlayerState*)inner)->moveAnimTable + 0x30) ||
        ((GameObject*)obj)->anim.currentMove == *(s16*)(((PlayerState*)inner)->moveAnimTable + 0x32))
    {
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0 &&
            ObjAnim_GetCurrentEventCountdown((ObjAnimComponent*)obj) == 0)
        {
            ObjAnim_SetCurrentMove(obj, move, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = fv;
        }
    }
    else if (((ByteFlags*)((char*)inner + 0x3f0))->b20 == 0 && ((ByteFlags*)((char*)inner + 0x3f1))->b20 == 0 &&
             ((PlayerState*)inner)->targetYawRateSigned > 5)
    {
        if (((GameObject*)obj)->anim.currentMove != *(s16*)(((PlayerState*)inner)->moveAnimTable + 0x3e) &&
            ObjAnim_GetCurrentEventCountdown((ObjAnimComponent*)obj) == 0)
        {
            ObjAnim_SetCurrentMove(obj, *(s16*)(((PlayerState*)inner)->moveAnimTable + 0x3e), lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7E90;
        }
    }
    else if (((GameObject*)obj)->anim.currentMove != move &&
             ObjAnim_GetCurrentEventCountdown((ObjAnimComponent*)obj) == 0)
    {
        s16 cur = ((GameObject*)obj)->anim.currentMove;
        if (cur == gPlayerStopMoves[0] || cur == gPlayerStopMoves[1] || cur == gPlayerStopMoves[2] ||
            cur == gPlayerStopMoves[3])
        {
            if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
            {
                ObjAnim_SetCurrentMove(obj, move, lbl_803E7EA4, 0);
                ((PlayerState*)state)->baddie.moveSpeed = fv;
            }
        }
        else
        {
            ObjAnim_SetCurrentMove(obj, move, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = fv;
            if (move == 0x5d)
            {
                ObjAnim_SetCurrentEventStepFrames((ObjAnimComponent*)obj, 0x1e);
            }
        }
    }
    if (((ByteFlags*)((char*)inner + 0x3f1))->b20 == 0)
    {
        f32 step;
        f32 lim;
        step = interpolate((f32) * (int*)((char*)inner + 0x47c),
                           lbl_803E7EE0 / ((PlayerState*)inner)->targetYawSmoothRate, timeDelta);
        lim = timeDelta * (((PlayerState*)inner)->targetYawRateLimit * ((PlayerState*)inner)->leanCurveScale);
        step = (step < lim) ? step : lim;
        if (((PlayerState*)inner)->targetYawRate < 0)
        {
            step = -step;
        }
        *(u16*)&((PlayerState*)inner)->targetYaw = gPlayerDegToBinAngle * step + (f32) ((PlayerState*)inner)->targetYaw;
        step = interpolate((f32) * (int*)((char*)inner + 0x488), lbl_803E7EE0 / ((PlayerState*)inner)->yawSmoothRate,
                           timeDelta);
        lim = ((PlayerState*)inner)->yawRateLimit * timeDelta;
        step = (step < lim) ? step : lim;
        if (((PlayerState*)inner)->yawRate < 0)
        {
            step = -step;
        }
        *(u16*)&((PlayerState*)inner)->yaw = gPlayerDegToBinAngle * step + (f32) * (s16*)((char*)inner + 0x484);
    }
    else
    {
        f32 vx;
        f32 vz;
        f32 c;
        c = mathSinf((gPlayerPi * (f32) * (int*)((char*)inner + 0x474)) / lbl_803E7F98);
        vx = t * -c;
        vx = ((PlayerState*)inner)->maxSpeed * vx;
        c = mathCosf((gPlayerPi * (f32) * (int*)((char*)inner + 0x474)) / lbl_803E7F98);
        vz = t * -c;
        vz = ((PlayerState*)inner)->maxSpeed * vz;
        vx = interpolate(vx - ((PlayerState*)inner)->smoothVelX, ((PlayerState*)inner)->velSmoothRate, timeDelta);
        vz = interpolate(vz - ((PlayerState*)inner)->smoothVelZ, ((PlayerState*)inner)->velSmoothRate, timeDelta);
        ((PlayerState*)inner)->smoothVelX = ((PlayerState*)inner)->smoothVelX + vx;
        ((PlayerState*)inner)->smoothVelZ = ((PlayerState*)inner)->smoothVelZ + vz;
        ((PlayerState*)state)->baddie.animSpeedC =
            sqrtf(((PlayerState*)inner)->smoothVelX * ((PlayerState*)inner)->smoothVelX +
                  ((PlayerState*)inner)->smoothVelZ * ((PlayerState*)inner)->smoothVelZ);
        ((PlayerState*)state)->baddie.animSpeedC =
            (((PlayerState*)state)->baddie.animSpeedC < lbl_803E7EA4)
                ? lbl_803E7EA4
                : ((((PlayerState*)state)->baddie.animSpeedC > ((PlayerState*)inner)->maxSpeed)
                       ? ((PlayerState*)inner)->maxSpeed
                       : ((PlayerState*)state)->baddie.animSpeedC);
    }
    if (((ByteFlags*)((char*)inner + 0x3f0))->b20 == 0)
    {
        fn_802AC32C(obj, state, inner);
    }
    return 0;
}

int playerState00(int obj, int state)
{
    if (mainGetBit(GAMEBIT_CF_DoStandUpAnim))
    {
        mainSetBits(GAMEBIT_CF_DoStandUpAnim, 0);
        (*gObjectTriggerInterface)->runSequence(0x10, (void*)obj, -1);
    }
    *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
    return 2;
}

s16 fn_802A71E0(int obj, int baseMoveId, int blendMoveId, int* blendAnchor, int* blendPlane,
                f32 samplePhase, f32 moveStepScale, int axis, int flags)
{
    ObjModel* model;
    int controlFlags;
    u8 moveFlags;
    int useSecondary;
    int axisOffset;
    f32* sampledPosition;
    int blendWeight;
    f32 baseDistance, blendDistance, blendFactor;
    f32 jointPosition[3];
    s16 jointRotation[3];
    model = Player_GetActiveModel(obj);
    moveFlags = 0;
    controlFlags = (u8)flags;
    if (controlFlags & 0x2)
    {
        moveFlags |= 0x2;
    }
    if (controlFlags & 0x40)
    {
        moveFlags |= 0x4;
    }
    if (controlFlags & 0x10)
    {
        moveFlags |= 0x8;
    }
    if (controlFlags & 0x20)
    {
        moveFlags |= 0x1;
    }
    useSecondary = controlFlags & 0x4;
    if (useSecondary != 0)
    {
        ((int (*)(int, int, u8, f32))ObjAnim_SetCurrentMove)(obj, baseMoveId, moveFlags, lbl_803E7EA4);
        ObjAnim_AdvanceCurrentMove((int)obj, moveStepScale, lbl_803E7EA4, NULL);
        ObjModel_SampleJointTransform(model, 0, 0, samplePhase, ((GameObject*)obj)->anim.rootMotionScale,
                                      jointPosition, jointRotation);
    }
    else
    {
        ((int (*)(int, int, u8, f32))Object_ObjAnimSetMove)(obj, baseMoveId, moveFlags, lbl_803E7EA4);
        Object_ObjAnimAdvanceMove(obj, moveStepScale, lbl_803E7EA4, NULL);
        ObjModel_SampleJointTransform(model, 1, 0, samplePhase, ((GameObject*)obj)->anim.rootMotionScale,
                                      jointPosition, jointRotation);
    }
    axisOffset = (u8)axis << 2;
    sampledPosition = jointPosition;
    baseDistance = *(f32*)((char*)sampledPosition + axisOffset);
    if (baseDistance < lbl_803E7EA4)
    {
        baseDistance = -baseDistance;
    }
    if (useSecondary != 0)
    {
        Object_ObjAnimSetSecondaryBlendMove((ObjAnimComponent*)obj, blendMoveId, 0);
        ObjModel_SampleJointTransform(model, 0, 2, samplePhase, ((GameObject*)obj)->anim.rootMotionScale,
                                      jointPosition, jointRotation);
    }
    else
    {
        Object_ObjAnimSetPrimaryBlendMove((ObjAnimComponent*)obj, blendMoveId, 0);
        ObjModel_SampleJointTransform(model, 1, 2, samplePhase, ((GameObject*)obj)->anim.rootMotionScale,
                                      jointPosition, jointRotation);
    }
    blendDistance = *(f32*)((char*)sampledPosition + axisOffset);
    if (blendDistance < 0.0f)
    {
        blendDistance = -blendDistance;
    }
    blendFactor = *(f32*)((char*)blendPlane + 0xc) +
        (*(f32*)((char*)blendAnchor + 0x0) * *(f32*)((char*)blendPlane + 0x0) +
         *(f32*)((char*)blendAnchor + 0x8) * *(f32*)((char*)blendPlane + 0x8));
    if (blendFactor < 0.0f)
    {
        blendFactor = -blendFactor;
    }
    blendFactor = (blendFactor - baseDistance) / (blendDistance - baseDistance);
    if (controlFlags & 0x1)
    {
        if (blendFactor < 0.0f)
        {
            blendFactor = 0.0f;
        }
    }
    else
    {
        if (blendFactor < 0.0f)
        {
            blendFactor = -blendFactor;
        }
    }
    if (blendFactor > lbl_803E7EE0)
    {
        blendFactor = lbl_803E7EE0;
    }
    blendWeight = (int)(lbl_803E7FAC * blendFactor);
    if (useSecondary != 0)
    {
        Object_ObjAnimSetSecondaryBlendMove((ObjAnimComponent*)obj, blendMoveId, (s16)blendWeight);
    }
    else
    {
        Object_ObjAnimSetPrimaryBlendMove((ObjAnimComponent*)obj, blendMoveId, (s16)blendWeight);
    }
    return blendWeight;
}

int playerCheckIfClimbingOntoWall(int obj, int state, int state2, void* out, f32 fv, u32 mask)
{
    f32* dir;
    int objCount;
    f32 nearDist;
    f32 rot[3];
    f32 vec[3];
    f32 start[3];
    f32 end[3];
    f32 sc1[3];
    f32* sc1p = sc1;
    f32 sc0[3];
    f32* sc0p = sc0;
    s8 dirs[13] = {0xb, 4, 6, 0xa, 0xa, 3, 3, 2, 0xe, 0x10, 0x12, 0x13, 5};
    u16 dirMasks[13] = {1, 2, 4, 8, 8, 0x10, 0x10, 0x40, 0x80, 0x100, 1, 0x20, 0xffff};
    struct
    {
        u8 pad[2];
        u16 mode;
        u8 pad2[4];
        f32 scale;
        f32 x;
        f32 y;
        f32 z;
    } pfx;
    TrackBBoxHit buf;
    u8 useAlt;
    f32 hd;
    f32 dp;
    int i;
    s8 ok;
    f32 ang;
    f32 lo;
    int k;
    s8 flagB;
    s8 flagA;
    u8 hit;
    int ai;

    ai = (u16)getAngle(((PlayerState*)state2)->baddie.moveInputX, -((PlayerState*)state2)->baddie.moveInputZ) -
         ((PlayerState*)state2)->baddie.cameraYaw;
    ang = (gPlayerPi * (f32)ai) / lbl_803E7F98;
    rot[0] = -mathSinf(ang);
    rot[1] = lbl_803E7EA4;
    rot[2] = -mathCosf(ang);
    fn_802A81B8((GameObject*)(obj), state, vec);
    sc1p[0] = lbl_803E808C * rot[0];
    sc1p[1] = lbl_803E808C * rot[1];
    sc1p[2] = lbl_803E808C * rot[2];
    sc0p[0] = lbl_803E808C * vec[0];
    sc0p[1] = lbl_803E808C * vec[1];
    sc0p[2] = lbl_803E808C * vec[2];
    ((PlayerState*)state)->flags360 &= ~PLAYER_FLAG_LEDGE_DETECTED;
    for (i = 0; i < PLAYER_SWEEP_DIR_COUNT; i++)
    {
        if ((mask & dirMasks[i]) == 0)
        {
            continue;
        }
        ok = 0;
        useAlt = 0;
        flagB = 1;
        flagA = 0;
        switch (i)
        {
        case 1:
        case 7:
        case 12:
        {
            u8 b;
            s16 v = ((PlayerState*)state2)->baddie.controlMode;
            if (v == 0xc)
            {
                continue;
            }
            if ((u16)(v - 9) <= 2)
            {
                continue;
            }
            b = ((PlayerState*)state)->flags3F0;
            if ((u32)b >> 3 & 1)
            {
                continue;
            }
            if ((u32)b >> 2 & 1)
            {
                continue;
            }
            flagB = 0;
            flagA = 1;
            ok = 1;
            break;
        }
        case 0:
        case 10:
            if (((u32)((PlayerState*)state)->flags3F1 & 1) == 0)
            {
                logPrintf(sNotOnGroundFailureMessage);
                continue;
            }
            ok = 1;
            break;
        case 3:
        case 5:
        {
            u8 b = ((PlayerState*)state)->flags3F0;
            if ((u32)b >> 3 & 1 || (u32)b >> 2 & 1)
            {
                ok = 1;
            }
            useAlt = 1;
            break;
        }
        case 2:
        {
            u8 b2;
            if (((u32)((PlayerState*)state)->flags3F1 & 1) == 0)
            {
                u8 b = ((PlayerState*)state)->flags3F0;
                if (((u32)b >> 3 & 1) == 0 && ((u32)b >> 2 & 1) == 0)
                {
                    continue;
                }
            }
            b2 = ((PlayerState*)state)->flags3F0;
            if ((u32)b2 >> 3 & 1 || (u32)b2 >> 2 & 1)
            {
                ok = 1;
            }
            break;
        }
        case 4:
        case 6:
        {
            u8 b2;
            if (((u32)((PlayerState*)state)->flags3F1 & 1) == 0)
            {
                u8 b = ((PlayerState*)state)->flags3F0;
                if (((u32)b >> 3 & 1) == 0 && ((u32)b >> 2 & 1) == 0)
                {
                    continue;
                }
            }
            b2 = ((PlayerState*)state)->flags3F0;
            if ((u32)b2 >> 3 & 1 || (u32)b2 >> 2 & 1)
            {
                ok = 1;
            }
            break;
        }
        case 11:
            flagB = 0;
            ok = 1;
            break;
        }
        if (ok == 0)
        {
            if (((PlayerState*)state2)->baddie.inputMagnitude < lbl_803E7EFC)
            {
                continue;
            }
        }
        if (useAlt == 0)
        {
            if (ok == 0)
            {
                end[0] = ((GameObject*)obj)->anim.localPosX + sc1p[0];
                end[1] = ((GameObject*)obj)->anim.localPosY + sc1p[1];
                end[2] = ((GameObject*)obj)->anim.localPosZ + sc1p[2];
                dir = rot;
            }
            else
            {
                end[0] = ((GameObject*)obj)->anim.localPosX + sc0p[0];
                end[1] = ((GameObject*)obj)->anim.localPosY + sc0p[1];
                end[2] = ((GameObject*)obj)->anim.localPosZ + sc0p[2];
                dir = vec;
            }
            start[0] = ((GameObject*)obj)->anim.localPosX;
            start[1] = ((GameObject*)obj)->anim.localPosY;
            start[2] = ((GameObject*)obj)->anim.localPosZ;
        }
        else
        {
            if (ok == 0)
            {
                start[0] = ((GameObject*)obj)->anim.localPosX + sc1p[0];
                start[1] = ((GameObject*)obj)->anim.localPosY + sc1p[1];
                start[2] = ((GameObject*)obj)->anim.localPosZ + sc1p[2];
                dir = rot;
            }
            else
            {
                start[0] = ((GameObject*)obj)->anim.localPosX + sc0p[0];
                start[1] = ((GameObject*)obj)->anim.localPosY + sc0p[1];
                start[2] = ((GameObject*)obj)->anim.localPosZ + sc0p[2];
                dir = vec;
            }
            end[0] = ((GameObject*)obj)->anim.localPosX;
            end[1] = ((GameObject*)obj)->anim.localPosY;
            end[2] = ((GameObject*)obj)->anim.localPosZ;
        }
        hit = objBboxFn_800640cc(start, end, lbl_803E7EA4, 3, &buf, (GameObject*)obj, 1, dirs[i],
                                0xff, 10);
        if (flagA != 0 && hit != 0)
        {
            ((PlayerState*)state)->probeHitDist = buf.distance;
        }
        if (flagB != 0 && hit != 0)
        {
            dp = buf.normalX * dir[0] + buf.normalY * dir[1] + buf.normalZ * dir[2];
            switch (i)
            {
            case 3:
            case 5:
                if (((GameObject*)obj)->anim.localPosY < lbl_803E7F10 + buf.lineStartY &&
                    ((GameObject*)obj)->anim.localPosY < lbl_803E7F10 + buf.lineEndY)
                {
                    hit = 0;
                }
                break;
            case 2:
            case 4:
            case 6:
                if (((u32)((PlayerState*)state)->flags3F1 & 1) != 0)
                {
                    if (dp > lbl_803E8090 || (((GameObject*)obj)->anim.localPosY > buf.upperY0 - lbl_803E7ED8 &&
                                              ((GameObject*)obj)->anim.localPosY > buf.upperY1 - lbl_803E7ED8))
                    {
                        hit = 0;
                    }
                }
                else
                {
                    if (dp > lbl_803E8094)
                    {
                        hit = 0;
                    }
                }
                break;
            case 0:
            case 10:
                break;
            default:
                if (dp > lbl_803E8090)
                {
                    hit = 0;
                }
            }
        }
        if (flagB != 0 && hit != 0)
        {
            if (useAlt == 0)
            {
                start[0] = ((GameObject*)obj)->anim.localPosX;
                start[1] = ((GameObject*)obj)->anim.localPosY;
                start[2] = ((GameObject*)obj)->anim.localPosZ;
                end[0] = -(lbl_803E808C * buf.normalX - ((GameObject*)obj)->anim.localPosX);
                end[1] = ((GameObject*)obj)->anim.localPosY;
                end[2] = -(lbl_803E808C * buf.normalZ - ((GameObject*)obj)->anim.localPosZ);
            }
            else
            {
                start[0] = lbl_803E808C * buf.normalX + ((GameObject*)obj)->anim.localPosX;
                start[1] = ((GameObject*)obj)->anim.localPosY;
                start[2] = lbl_803E808C * buf.normalZ + ((GameObject*)obj)->anim.localPosZ;
                end[0] = ((GameObject*)obj)->anim.localPosX;
                end[1] = ((GameObject*)obj)->anim.localPosY;
                end[2] = ((GameObject*)obj)->anim.localPosZ;
            }
            hit = objBboxFn_800640cc(start, end, lbl_803E7EA4, 3, &buf, (GameObject*)obj, 1,
                                    dirs[i], 0xff, 10);
        }
        if (hit == 0)
        {
            continue;
        }
        hd = buf.distance;
        if (useAlt != 0)
        {
            hd = lbl_803E808C - hd;
        }
        switch (i)
        {
        case 0:
        {
            GameObject* target = buf.object;
            if (target == NULL)
            {
                continue;
            }
            if ((*(int (*)(int)) * (int*)((char*)target->anim.dll[0] + 0x2c))((int)target) != 0 &&
                ((PlayerState*)state2)->baddie.inputMagnitude > lbl_803E7EFC && hd <= lbl_803E7ED4 + lbl_803DC6C0)
            {
                switch (fn_802A8EE4(obj, state, &buf, state + 0x5a8, end, hd))
                {
                case 2:
                    return 4;
                case 3:
                    return 5;
                }
            }
            if (!(hd < lbl_803E7FA4))
            {
                continue;
            }
            if (target->anim.resetHitboxFlags & 8)
            {
                continue;
            }
            ((PlayerState*)state)->flags360 |= (u32)PLAYER_FLAG_LEDGE_DETECTED;
            if ((*(int*)&((PlayerState*)state2)->baddie.unk31C & 0x100) == 0)
            {
                continue;
            }
            ((PlayerState*)state)->surfaceNormalX = buf.normalX;
            ((PlayerState*)state)->surfaceNormalY = buf.normalY;
            ((PlayerState*)state)->surfaceNormalZ = buf.normalZ;
            ((PlayerState*)state)->surfaceNormalW = buf.sourceNormalW;
            *(u8*)&((PlayerState*)state)->stickEdgeLatch = 0;
            if (buf.object != NULL)
            {
                Obj_TransformWorldPointToLocal(end[0], end[1], end[2], &((PlayerState*)state)->contactPointX,
                                               &((PlayerState*)state)->contactPointY,
                                               &((PlayerState*)state)->contactPointZ, (u32)buf.object);
                ((PlayerState*)state)->contactObject = (int)buf.object;
            }
            else
            {
                ((PlayerState*)state)->contactPointX = end[0];
                ((PlayerState*)state)->contactPointY = end[1];
                ((PlayerState*)state)->contactPointZ = end[2];
                ((PlayerState*)state)->contactObject = 0;
            }
            return 6;
        }
        case 10:
            if (!(hd < lbl_803E8098))
            {
                continue;
            }
            if ((*(int*)&((PlayerState*)state2)->baddie.unk31C & 0x100) == 0)
            {
                continue;
            }
            ((PlayerState*)state)->surfaceNormalX = buf.normalX;
            ((PlayerState*)state)->surfaceNormalY = buf.normalY;
            ((PlayerState*)state)->surfaceNormalZ = buf.normalZ;
            ((PlayerState*)state)->surfaceNormalW = buf.sourceNormalW;
            *(u8*)&((PlayerState*)state)->stickEdgeLatch = 0;
            if (buf.object != NULL)
            {
                Obj_TransformWorldPointToLocal(end[0], end[1], end[2], &((PlayerState*)state)->contactPointX,
                                               &((PlayerState*)state)->contactPointY,
                                               &((PlayerState*)state)->contactPointZ, (u32)buf.object);
                ((PlayerState*)state)->contactObject = (int)buf.object;
            }
            else
            {
                ((PlayerState*)state)->contactPointX = end[0];
                ((PlayerState*)state)->contactPointY = end[1];
                ((PlayerState*)state)->contactPointZ = end[2];
                ((PlayerState*)state)->contactObject = 0;
            }
            return 0xd;
        case 3:
        case 4:
            if (!(hd <= lbl_803E7F58))
            {
                continue;
            }
            if (player_probeClimbable((GameObject*)obj, state, &buf, state + 0x4e4, i == 3) == 0)
            {
                continue;
            }
            return 0;
        case 5:
        case 6:
            if (!(hd <= lbl_803E7EE0 + lbl_803DC6C0))
            {
                continue;
            }
            if (fn_802A8680(obj, state, &buf, end, state + 0x548, i == 5) == 0)
            {
                continue;
            }
            return 9;
        case 1:
        case 7:
        case 12:
            if (!(hd < lbl_803E7F58))
            {
                continue;
            }
            switch (fn_802A87CC((GameObject*)obj, (char*)&buf, (f32*)(state + 0x5a8), end, hd, fv))
            {
            case 4:
                return 8;
            case 5:
                return 7;
            }
            break;
        case 2:
        case 9:
            if (!(hd <= lbl_803E7EE0 + lbl_803DC6C0))
            {
                continue;
            }
            switch (fn_802A8EE4(obj, state, &buf, state + 0x5a8, end, hd))
            {
            case 2:
                return 4;
            case 3:
                return 5;
            case 6:
                return 0xc;
            }
            break;
        case 8:
        {
            s8 ok2;
            int t8;
            if (!(hd <= lbl_803E7EE0 + lbl_803DC6C0))
            {
                continue;
            }
            nearDist = lbl_803E808C;
            t8 = ObjGroup_FindNearestObject(0x23, (GameObject*)obj, &nearDist);
            ok2 = 1;
            if ((u32)t8 != 0)
            {
                if ((*(u8 (*)(int)) * (int*)((char*)*(int*)*(int*)(t8 + 0x68) + 0x24))(t8) == 0)
                {
                    ok2 = 0;
                }
            }
            if (ok2 == 0)
            {
                continue;
            }
            ((PlayerState*)state)->hitNormalX = buf.normalX;
            ((PlayerState*)state)->hitNormalY = buf.normalY;
            ((PlayerState*)state)->hitNormalZ = buf.normalZ;
            ((PlayerState*)state)->hitNormalW = buf.normalW;
            return 0xb;
        }
        case 11:
            if (!(hd < lbl_803E809C))
            {
                continue;
            }
            if (buf.kind == 0xd)
            {
                if (!(((PlayerState*)state2)->baddie.animSpeedA > lbl_803E80A0))
                {
                    continue;
                }
                if (((PlayerState*)state)->particleBurstCooldown <= lbl_803E7EA4)
                {
                    for (k = 0; k < 0x4b; k++)
                    {
                        lo = buf.lineStartX;
                        pfx.x = lo + (buf.lineEndX - lo) * (f32)randomGetRange(0, 100) / lbl_803E7F5C;
                        lo = buf.lineStartY;
                        pfx.y = lo + (buf.upperY0 - lo) * (f32)randomGetRange(0, 100) / lbl_803E7F5C;
                        lo = buf.lineStartZ;
                        pfx.z = lo + (buf.lineEndZ - lo) * (f32)randomGetRange(0, 100) / lbl_803E7F5C;
                        pfx.scale = lbl_803E7EE0;
                        pfx.mode = 0x3c;
                        (*gPartfxInterface)->spawnObject((void*)obj, 0x804, &pfx, 0x200001, -1, NULL);
                    }
                    ((PlayerState*)state)->particleBurstCooldown = lbl_803E7F30;
                }
            }
            else
            {
                ObjPath_GetPointWorldPosition((GameObject*)obj, 0xb, &pfx.x, &pfx.y, &pfx.z, 0);
                ObjHits_RecordPositionHit((GameObject*)obj, NULL, 8, 1, -1, pfx.x, pfx.y, pfx.z);
            }
            break;
        }
    }
    if ((*(int*)&((PlayerState*)state2)->baddie.unk31C & 0x100) != 0 && (mask & 0x200) != 0)
    {
        int* objs = (int*)ObjGroup_GetObjects(10, &objCount);
        int k2;
        for (k2 = 0; k2 < objCount; k2++)
        {
            int cur = *objs;
            if ((*(int (*)(int, int)) * (int*)((char*)*(int*)*(int*)(cur + 0x68) + 0x20))(cur, obj) != 0)
            {
                ((PlayerState*)state)->focusObject = (GameObject*)cur;
                return 0xa;
            }
            objs++;
        }
    }
    return -1;
}

void fn_802A81B8(GameObject* obj, int state, f32* out)
{
    f32 mag;
    u32 flag = (((PlayerState*)state)->flags3F1 >> 5) & 1;

    if (flag != 0 || ((PlayerState*)state)->baddie.targetObj != NULL)
    {
        out[0] = obj->anim.velocityX;
        out[1] = lbl_803E7EA4;
        out[2] = obj->anim.velocityZ;
        mag = PSVECMag(out);
        if (mag > lbl_803E7EA4)
        {
            mag = lbl_803E7EE0 / mag;
            PSVECScale(out, out, mag);
        }
        else
        {
            out[0] = -mathSinf(gPlayerPi * (f32)((PlayerState*)state)->targetYaw / lbl_803E7F98);
            out[1] = lbl_803E7EA4;
            out[2] = -mathCosf(gPlayerPi * (f32)((PlayerState*)state)->targetYaw / lbl_803E7F98);
        }
    }
    else
    {
        out[0] = -mathSinf(gPlayerPi * (f32)((PlayerState*)state)->targetYaw / lbl_803E7F98);
        out[1] = lbl_803E7EA4;
        out[2] = -mathCosf(gPlayerPi * (f32)((PlayerState*)state)->targetYaw / lbl_803E7F98);
    }
}

int player_probeClimbable(GameObject* obj, int p4, void* src, int dst, int flag)
{
    TrackGroundHit** hits;
    f32 pos[3];
    f32 y;
    f32 minDist;
    int best;
    int i;
    int count;
    TrackGroundHit* chosen;
    f32 zero;

    *(u8*)((char*)dst + 3) = 0;
    ((ByteFlags*)((char*)dst + 0x63))->b80 = 1;
    if ((*(s8*)((char*)src + 0x52) & 0x08) == 0)
    {
        ((ByteFlags*)((char*)dst + 0x63))->b80 = 0;
    }

    {
        f32 s4 = *(f32*)((char*)src + 0x4);
        f32 t = 0.5f;
        *(f32*)((char*)dst + 0x48) = s4 + t * (*(f32*)((char*)src + 0x8) - s4);
        *(f32*)((char*)dst + 0x4c) = *(f32*)((char*)src + 0xc);
        *(f32*)((char*)dst + 0x50) =
            *(f32*)((char*)src + 0x14) + t * (*(f32*)((char*)src + 0x18) - *(f32*)((char*)src + 0x14));
    }

    if (flag != 0)
    {
        *(f32*)((char*)dst + 0x28) = -*(f32*)((char*)src + 0x1c);
        *(f32*)((char*)dst + 0x2c) = -*(f32*)((char*)src + 0x20);
        *(f32*)((char*)dst + 0x30) = -*(f32*)((char*)src + 0x24);
        *(f32*)((char*)dst + 0x34) = -*(f32*)((char*)src + 0x28);
    }
    else
    {
        *(f32*)((char*)dst + 0x28) = *(f32*)((char*)src + 0x1c);
        *(f32*)((char*)dst + 0x2c) = *(f32*)((char*)src + 0x20);
        *(f32*)((char*)dst + 0x30) = *(f32*)((char*)src + 0x24);
        *(f32*)((char*)dst + 0x34) = *(f32*)((char*)src + 0x28);
    }

    *(f32*)((char*)dst + 0x38) = -*(f32*)((char*)src + 0x24);
    *(f32*)((char*)dst + 0x3c) = zero = 0.0f;
    *(f32*)((char*)dst + 0x40) = *(f32*)((int)src + 0x1c);
    *(f32*)((char*)dst + 0x44) = -(*(f32*)((char*)dst + 0x48) * *(f32*)((char*)dst + 0x38) +
                                   *(f32*)((char*)dst + 0x4c) * *(f32*)((char*)dst + 0x3c) +
                                   *(f32*)((char*)dst + 0x50) * *(f32*)((char*)dst + 0x40));

    *(f32*)((char*)dst + 0x54) = *(f32*)((char*)p4 + 0x768);
    *(f32*)((char*)dst + 0x58) = zero;
    *(f32*)((char*)dst + 0x5c) = *(f32*)((char*)p4 + 0x770);
    *(f32*)((char*)dst + 0x18) = *(f32*)((char*)dst + 0x54) * *(f32*)((char*)dst + 0x38) +
                                 *(f32*)((char*)dst + 0x58) * *(f32*)((char*)dst + 0x3c) +
                                 *(f32*)((char*)dst + 0x5c) * *(f32*)((char*)dst + 0x40) + *(f32*)((char*)dst + 0x44);

    *(s8*)((char*)dst + 0x62) = (s8)(int)*(s8*)((char*)src + 0x53);

    if (*(f32*)((char*)dst + 0x18) > -9.0f && *(f32*)((char*)dst + 0x18) < 9.0f)
    {
        *(f32*)((char*)dst + 0x8) = *(f32*)((char*)src + 0xc);
        PSVECScale((f32*)((char*)src + 0x1c), pos, -(&lbl_803DC6B8)[1]);
        PSVECAdd((f32*)((int)dst + 0x48), pos, pos);
        y = *(f32*)((char*)src + 0x3c);
        pos[1] = y;
        count = hitDetectFn_80065e50(obj, pos[0], y, pos[2], &hits, 0, HITQUERY_CLIMB_SURFACE);

        minDist = 10000.0f;
        best = -1;
        for (i = 0; i < count; i++)
        {
            TrackGroundHit* entry = hits[i];
            if (entry->normalY > 0.707f)
            {
                f32 d = pos[1] - entry->height;
                if (d < 0.0f)
                {
                    d = -d;
                }
                if (d < minDist)
                {
                    minDist = d;
                    best = i;
                }
            }
        }

        chosen = hits[best];
        *(f32*)((char*)dst + 0x4) = chosen->height;
        *(s8*)((char*)dst + 0x1) = (s8)(s32)((2.2f + (*(f32*)((char*)src + 0x3c) - *(f32*)((char*)dst + 0x8))) / 8.8f);
        *(f32*)((char*)dst + 0xc) =
            (*(f32*)((char*)src + 0x3c) - *(f32*)((char*)dst + 0x8)) / (f32) * (s8*)((char*)dst + 0x1);

        if (obj->anim.localPosY > *(f32*)((char*)dst + 0x4) - 10.0f)
        {
            *(s8*)((char*)dst + 0x0) = *(u8*)((char*)dst + 0x1) - 3;
        }
        else
        {
            *(s8*)((char*)dst + 0x0) = 1;
        }
        return 1;
    }
    return 0;
}

int fn_802A8680(int p1, int p2, void* src, f32* vec, int out, int flag)
{
    f32 p48;
    f32 m44;
    f32 d1;
    f32 m4c;
    f32 nx;
    f32 ny;
    f32 d2;
    f32 c38;
    *(f32*)((char*)out + 0x44) = *(f32*)((char*)vec + 0x0);
    *(f32*)((char*)out + 0x48) = *(f32*)((char*)src + 0xc);
    *(f32*)((char*)out + 0x4c) = *(f32*)((char*)vec + 0x8);
    *(f32*)((char*)out + 0x50) = ((PlayerState*)p2)->savedPosX;
    *(f32*)((char*)out + 0x54) = lbl_803E7EA4;
    *(f32*)((char*)out + 0x58) = ((PlayerState*)p2)->savedPosZ;
    if (flag != 0)
    {
        *(u8*)((char*)out + 0x1) = 1;
    }
    else
    {
        *(u8*)((char*)out + 0x1) = 0;
    }
    *(f32*)((char*)out + 0x24) = *(f32*)((char*)src + 0x1c);
    *(f32*)((char*)out + 0x28) = *(f32*)((char*)src + 0x20);
    *(f32*)((char*)out + 0x2c) = *(f32*)((char*)src + 0x24);
    *(f32*)((char*)out + 0x30) = *(f32*)((char*)src + 0x28);
    *(f32*)((char*)out + 0x34) = -*(f32*)((char*)src + 0x24);
    c38 = lbl_803E7EA4;
    *(f32*)((char*)out + 0x38) = c38;
    *(f32*)((char*)out + 0x3c) = *(f32*)((char*)src + 0x1c);
    *(f32*)((char*)out + 0x40) = -(*(f32*)((char*)out + 0x44) * *(f32*)((char*)out + 0x34) +
                                   *(f32*)((char*)out + 0x48) * *(f32*)((char*)out + 0x38) +
                                   *(f32*)((char*)out + 0x4c) * *(f32*)((char*)out + 0x3c));
    nx = -*(f32*)((char*)out + 0x2c);
    ny = *(f32*)((char*)out + 0x24);
    d1 = -(nx * *(f32*)((char*)src + 0x4) + ny * *(f32*)((char*)src + 0x14)) +
         (ny * (m4c = *(f32*)((char*)out + 0x4c)) +
          (nx * (m44 = *(f32*)((char*)out + 0x44)) + (p48 = c38 * *(f32*)((char*)out + 0x48))));
    nx = -nx;
    ny = -ny;
    d2 = -(nx * *(f32*)((char*)src + 0x8) + ny * *(f32*)((char*)src + 0x18)) + (ny * m4c + (nx * m44 + p48));
    if (d1 > lbl_803E80BC && d2 > lbl_803E80BC)
    {
        *(f32*)((char*)out + 0x8) = *(f32*)((char*)src + 0xc);
        *(f32*)((char*)out + 0x4) = *(f32*)((char*)src + 0x3c);
        *(s8*)((char*)out + 0x2) = (int)*(s8*)((char*)src + 0x53);
        return 1;
    }
    return 0;
}

int fn_802A87CC(GameObject* obj, char* cam, f32* out, f32* vec, f32 fa, f32 fb)
{
    f32* dp;
    char* cp;
    f32* px2;
    f32* py2;
    f32* pz2;
    int inner;
    f32* b6b8;
    s8 mode;
    int wallHit;
    int tris;
    int verts;
    void* parent;
    f32* pl;

    f32 x2;
    f32 x1;
    f32 z2;
    f32 z1;
    f32 y2;
    f32 y1;
    TrackGroundHit** list;
    f32 planes[8];
    struct
    {
        f32 x;
        f32 y;
        f32 z;
    } probe;
    f32 dists[2];

    mode = 0;
    inner = *(int*)&obj->extra;
    if (fa <= ((PlayerState*)inner)->baddie.animSpeedA * fb || fa <= 3.5f)
    {
        s8 st = *(s8*)((char*)cam + 0x50);
        if (st == 2 || st == 0x11)
        {
            mode = 4;
        }
        else if (((PlayerState*)inner)->baddie.animSpeedA >= 1.2530199f)
        {
            mode = 5;
        }
        else if (st != 4)
        {
            mode = 4;
        }
    }
    out[7] = ((GameObject*)cam)->anim.worldPosY;
    out[8] = ((GameObject*)cam)->anim.worldPosZ;
    out[9] = ((GameObject*)cam)->anim.velocityX;
    out[7] = -out[7];
    out[8] = -out[8];
    out[9] = -out[9];
    out[10] = -((GameObject*)cam)->anim.velocityY;
    out[0xb] = vec[0];
    out[0xc] = vec[1];
    out[0xd] = vec[2];
    parent = *(void**)cam;
    if (mode == 4)
    {
        f32 thresh;
        int i;
        int j;
        int j8;
        wallHit = 0;
        if (parent != NULL)
        {
            tris = *(int*)((char*)(int)((ObjAnimComponent*)parent)->modelInstance + 0x34);
            verts = *(int*)((char*)(int)((ObjAnimComponent*)parent)->modelInstance + 0x3c);
        }
        else
        {
            tris = lbl_803DCF34;
            verts = (int)lbl_803DCF38;
        }
        planes[0] = out[9];
        planes[1] = 0.0f;
        planes[2] = -out[7];
        planes[3] = -(planes[0] * *(f32*)((char*)cam + 0x4) + planes[2] * ((GameObject*)cam)->anim.localPosZ);
        planes[4] = -planes[0];
        planes[5] = 0.0f;
        planes[6] = -planes[2];
        planes[7] =
            -(planes[4] * ((GameObject*)cam)->anim.rootMotionScale + planes[6] * ((GameObject*)cam)->anim.worldPosX);
        i = 0;
        pl = planes;
        dp = dists;
        cp = cam;
        b6b8 = &lbl_803DC6B8;
        px2 = &x2;
        py2 = &y2;
        pz2 = &z2;
        thresh = 0.5f;
        do
        {
            f32 dot = PSVECDotProduct(pl, vec);
            *dp = pl[3] + dot;
            if (*dp < thresh + b6b8[1])
            {
                int tri;
                if (*(s16*)(cp + 0x4c) > -1)
                {
                    tri = tris + *(s16*)(cp + 0x4c) * 0x10;
                }
                else
                {
                    tri = 0;
                }
                if ((void*)tri != NULL && ((*(s8*)(tri + 3) & 0x3f) == 5 || (*(s8*)(tri + 3) & 0x3f) == 2))
                {
                    j = *(s16*)(tri + 4) * 0xc;
                    x1 = *(f32*)(verts + j);
                    y1 = 0.0f;
                    j8 = j + 8;
                    z1 = *(f32*)(verts + j8);
                    j = *(s16*)(tri + 6) * 0xc;
                    x2 = *(f32*)(verts + j);
                    y2 = 0.0f;
                    j8 = j + 8;
                    z2 = *(f32*)(verts + j8);
                    if (parent != NULL)
                    {
                        Obj_TransformLocalPointToWorld(x1, y1, z1, &x1, &y1, &z1, (int)parent);
                        Obj_TransformLocalPointToWorld(x2, y2, z2, px2, py2, pz2, (int)parent);
                    }
                    {
                        f32 dz = z2 - z1;
                        f32 dx = x1 - x2;
                        f32 inv = 1.0f / sqrtf(dz * dz + dx * dx);
                        dz = dz * inv;
                        dx = dx * inv;
                        if (dz * out[7] + dx * out[9] < 0.5f)
                        {
                            wallHit = 1;
                        }
                    }
                }
                else
                {
                    wallHit = 1;
                }
            }
            pl += 4;
            dp++;
            cp += 2;
            i++;
        } while (i < 2);
        if (dists[0] < dists[1])
        {
            *(u8*)((char*)out + 0x5f) = 0;
        }
        else
        {
            *(u8*)((char*)out + 0x5f) = 1;
        }
        if (wallHit != 0)
        {
            out[0xb] = out[0xb] + ((0.5f + b6b8[1]) - dists[*(u8*)((char*)out + 0x5f)]) *
                                      planes[(u32) * (u8*)((char*)out + 0x5f) * 4];
            out[0xd] = out[0xd] + ((0.5f + b6b8[1]) - dists[*(u8*)((char*)out + 0x5f)]) *
                                      planes[(u32) * (u8*)((char*)out + 0x5f) * 4 + 2];
        }
        out[0x11] = -(out[7] * (0.5f + lbl_803DC6C0) - out[0xb]);
        out[0x13] = -(out[9] * (0.5f + lbl_803DC6C0) - out[0xd]);
        {
            f32 f = 5.0f;
            out[0x14] = f * out[7] + out[0xb];
            out[0x16] = f * out[9] + out[0xd];
        }
        out[1] = ((GameObject*)cam)->anim.localPosX +
                 *(f32*)((char*)cam + 0x48) * (((GameObject*)cam)->anim.localPosY - ((GameObject*)cam)->anim.localPosX);
        probe.x = out[0x14];
        probe.y = out[1];
        probe.z = out[0x16];
        Obj_TransformLocalPointToWorld(probe.x, probe.y, probe.z, &probe.x, &probe.y, &probe.z,
                                       *(int*)&obj->anim.parent);
        {
            int cnt = hitDetectFn_80065e50(obj, probe.x, probe.y, probe.z, &list, 0, 0x201);
            if (cnt != 0)
            {
                TrackGroundHit** pp;
                f32 best = 10000.0f;
                f32 best2 = best;
                int bi = -1;
                int i2 = 0;
                pp = list;
                for (; cnt > 0; cnt--)
                {
                    f32 dy = probe.y - (*pp)->height;
                    if (dy >= 0.0f && (best < 0.0f || dy < best))
                    {
                        best = dy;
                        bi = i2;
                    }
                    if ((*pp)->normalY > 0.707f && dy >= 0.0f && (best2 < 0.0f || dy < best2))
                    {
                        best2 = dy;
                    }
                    pp++;
                    i2++;
                }
                if (best < 40.0f && bi != -1 && list[bi]->normalY <= 0.707f && list[bi]->normalY > 0.175f)
                {
                    return 0;
                }
                if (best2 < 40.0f)
                {
                    return 0;
                }
            }
        }
        probe.x = out[0x11];
        probe.y = out[1];
        probe.z = out[0x13];
        Obj_TransformLocalPointToWorld(probe.x, probe.y, probe.z, &probe.x, &probe.y, &probe.z,
                                       *(int*)&obj->anim.parent);
        if (hitDetectFn_800658a4(obj, probe.x, probe.y, probe.z, out + 0x12, 0x205) == 0)
        {
            out[0x12] = out[1] - out[0x12];
        }
        else
        {
            out[0x12] = out[1];
        }
        out[2] = ((GameObject*)cam)->anim.localPosX;
        out[0] = out[1] - out[2];
        *(u8*)((char*)out + 0x5e) = *(u8*)((char*)cam + 0x50);
        *(u8*)((char*)out + 0x60) = *(u8*)((char*)cam + 0x53);
        if (obj->anim.parent != NULL)
        {
            Obj_TransformLocalPointToWorld(out[0xb], out[0xc], out[0xd], out + 0xb, out + 0xc, out + 0xd,
                                           *(int*)&obj->anim.parent);
            Obj_TransformLocalPointToWorld(out[0x11], out[0x12], out[0x13], out + 0x11, out + 0x12,
                                           out + 0x13, *(int*)&obj->anim.parent);
            Obj_TransformLocalPointToWorld(out[0x14], out[0x15], out[0x16], out + 0x14, out + 0x15,
                                           out + 0x16, *(int*)&obj->anim.parent);
            ((PlayerState*)inner)->leapTargetY =
                ((PlayerState*)inner)->leapTargetY + *(f32*)(*(int*)&obj->anim.parent + 0x10);
            ((PlayerState*)inner)->leapBaseY =
                ((PlayerState*)inner)->leapBaseY + *(f32*)(*(int*)&obj->anim.parent + 0x10);
        }
        *(u8*)((char*)out + 0x61) = 1;
        if (parent != NULL && (((ObjAnimComponent*)parent)->modelInstance->flags & 0x8000) == 0)
        {
            *(void**)((char*)inner + 0x4c4) = parent;
        }
        else
        {
            ((PlayerState*)inner)->groundObject = NULL;
        }
    }
    else
    {
        ((PlayerState*)inner)->groundObject = NULL;
    }
    return mode;
}

int fn_802A8EE4(int a, int b, void* c, int d, f32* e, f32 distance)
{
    char* cp;
    f32* b6b8;
    f32* pbx;
    f32* pby;
    f32* pbz;
    int tbl1, tbl2;
    EmitPlane* pl;
    void* hit;
    int i;
    int j;
    int k;
    f32 bx, ax, bz, az, by, ay;
    f32 threshold;
    EmitPlane planes[2];

    ((PlayerState*)b)->groundObject = NULL;
    *(f32*)((char*)d + 0x1c) = *(f32*)((char*)c + 0x1c);
    *(f32*)((char*)d + 0x20) = *(f32*)((char*)c + 0x20);
    *(f32*)((char*)d + 0x24) = *(f32*)((char*)c + 0x24);
    *(f32*)((char*)d + 0x28) = *(f32*)((char*)c + 0x28);
    *(u8*)((char*)d + 0x60) = *(u8*)((char*)c + 0x53);
    hit = *(void**)((char*)c + 0x0);
    if (hit != NULL)
    {
        tbl1 = *(int*)((char*)(int)((ObjAnimComponent*)hit)->modelInstance + 0x34);
        tbl2 = *(int*)((char*)(int)((ObjAnimComponent*)hit)->modelInstance + 0x3c);
    }
    else
    {
        tbl1 = lbl_803DCF34;
        tbl2 = (int)lbl_803DCF38;
    }
    planes[0].nx = -*(f32*)((char*)d + 0x24);
    planes[0].ny = lbl_803E7EA4;
    planes[0].nz = *(f32*)((char*)d + 0x1c);
    planes[0].d = -(planes[0].nx * *(f32*)((char*)c + 0x4) + planes[0].nz * *(f32*)((char*)c + 0x14));
    planes[1].nx = -planes[0].nx;
    planes[1].ny = lbl_803E7EA4;
    planes[1].nz = -planes[0].nz;
    planes[1].d = -(planes[1].nx * *(f32*)((char*)c + 0x8) + planes[1].nz * *(f32*)((char*)c + 0x18));
    i = 0;
    pl = planes;
    cp = (char*)c;
    b6b8 = &lbl_803DC6B8;
    pbx = &bx;
    pby = &by;
    pbz = &bz;
    threshold = lbl_803E7E98;
    do
    {
        f32 dot = PSVECDotProduct((f32*)pl, (f32*)e);
        if (pl->d + dot < threshold + b6b8[1])
        {
            void* face;
            if (*(s16*)(cp + 0x4c) > -1)
            {
                face = (void*)(tbl1 + *(s16*)(cp + 0x4c) * 0x10);
            }
            else
            {
                face = NULL;
            }
            if (face != NULL &&
                (((s8) * (s8*)((char*)face + 0x3) & 0x3f) == 6 || ((s8) * (s8*)((char*)face + 0x3) & 0x3f) == 0x10))
            {
                j = *(s16*)((char*)face + 0x4) * 0xc;
                ax = *(f32*)(tbl2 + j);
                ay = lbl_803E7EA4;
                k = j + 8;
                az = *(f32*)(tbl2 + k);
                j = *(s16*)((char*)face + 0x6) * 0xc;
                bx = *(f32*)(tbl2 + j);
                by = lbl_803E7EA4;
                k = j + 8;
                bz = *(f32*)(tbl2 + k);
                if (hit != NULL)
                {
                    Obj_TransformLocalPointToWorld(ax, ay, az, &ax, &ay, &az, (int)hit);
                    Obj_TransformLocalPointToWorld(bx, by, bz, pbx, pby, pbz, (int)hit);
                }
                {
                    f32 dz = bz - az;
                    f32 dx = ax - bx;
                    f32 scale = lbl_803E7EE0 / sqrtf(dz * dz + dx * dx);
                    dz = dz * scale;
                    dx = dx * scale;
                    if (dz * *(f32*)((char*)d + 0x1c) + dx * *(f32*)((char*)d + 0x24) < lbl_803E7E98)
                    {
                        return 0;
                    }
                }
            }
            else
            {
                return 0;
            }
        }
        pl++;
        cp += 2;
        i++;
    } while (i < 2);
    *(f32*)((char*)d + 0x2c) = *(f32*)((char*)e + 0x0);
    *(f32*)((char*)d + 0x30) = *(f32*)((char*)e + 0x4);
    *(f32*)((char*)d + 0x34) = *(f32*)((char*)e + 0x8);
    {
        f32 e2;
        f32 e3;
        *(f32*)((char*)d + 0x44) =
            -(*(f32*)((char*)d + 0x1c) * ((e2 = lbl_803E7E98) + (e3 = lbl_803DC6C0)) - *(f32*)((char*)d + 0x2c));
        *(f32*)((char*)d + 0x4c) = -(*(f32*)((char*)d + 0x24) * (e2 + lbl_803DC6C0) - *(f32*)((char*)d + 0x34));
    }
    {
        f32 f = lbl_803E7F10;
        *(f32*)((char*)d + 0x50) = f * *(f32*)((char*)d + 0x1c) + *(f32*)((char*)d + 0x2c);
        *(f32*)((char*)d + 0x58) = f * *(f32*)((char*)d + 0x24) + *(f32*)((char*)d + 0x34);
    }
    *(f32*)((char*)d + 0x38) = ((PlayerState*)b)->savedPosX;
    *(f32*)((char*)d + 0x3c) = lbl_803E7EA4;
    *(f32*)((char*)d + 0x40) = ((PlayerState*)b)->savedPosZ;
    *(f32*)((char*)d + 0x4) =
        *(f32*)((char*)c + 0x48) * (*(f32*)((char*)c + 0x40) - *(f32*)((char*)c + 0x3c)) + *(f32*)((char*)c + 0x3c);
    *(u8*)((char*)d + 0x5e) = *(u8*)((char*)c + 0x50);
    *(u8*)((char*)d + 0x61) = 1;
    if (hitDetectFn_800658a4((GameObject*)a, *(f32*)((char*)d + 0x44), *(f32*)((char*)d + 0x4),
                             *(f32*)((char*)d + 0x4c), (f32*)((char*)d + 0x48), 0x205) == 0)
    {
        *(f32*)((int)d + 0x48) = *(f32*)((char*)d + 0x4) - *(f32*)((int)d + 0x48);
    }
    else
    {
        return 0;
    }
    if ((s8) * (s8*)((char*)c + 0x50) != 0x10)
    {
        *(f32*)((char*)d + 0x8) = ((GameObject*)a)->anim.previousLocalPosY;
        *(f32*)((char*)d + 0x0) = *(f32*)((char*)d + 0x4) - *(f32*)((char*)d + 0x8);
        if ((((PlayerState*)b)->flags3F1 & 1) != 0u)
        {
            if (hit != NULL && (((ObjAnimComponent*)hit)->modelInstance->flags & 0x8000) == 0)
            {
                ((PlayerState*)b)->groundObject = (GameObject*)hit;
            }
            if (*(f32*)((char*)d + 0x0) <= lbl_803E80C8)
            {
                if (*(f32*)((char*)d + 0x0) > lbl_803E80C4)
                {
                    return 2;
                }
            }
            if (*(f32*)((char*)d + 0x0) <= lbl_803E80C4 && *(f32*)((char*)d + 0x0) >= lbl_803E8018)
            {
                return 3;
            }
        }
        else
        {
            f32 q;
            q = *(f32*)((char*)c + 0x48) * (*(f32*)((char*)c + 0x10) - *(f32*)((char*)c + 0xc)) +
                *(f32*)((char*)c + 0xc);
            q = *(f32*)((char*)d + 0x4) - q;
            if (*(f32*)((char*)d + 0x0) >= lbl_803E7ED8 && *(f32*)((char*)d + 0x0) <= lbl_803E7FBC && q >= lbl_803E80C4)
            {
                if (hit != NULL && (((ObjAnimComponent*)hit)->modelInstance->flags & 0x8000) == 0)
                {
                    ((PlayerState*)b)->groundObject = (GameObject*)hit;
                }
                return 6;
            }
        }
    }
    else
    {
        *(f32*)((char*)d + 0x8) = ((GameObject*)a)->anim.localPosY;
        *(f32*)((char*)d + 0x0) = *(f32*)((char*)d + 0x4) - *(f32*)((char*)d + 0x8);
        if (*(f32*)((char*)d + 0x0) >= lbl_803E8044)
        {
            return 0;
        }
        if (hit != NULL && (((ObjAnimComponent*)hit)->modelInstance->flags & 0x8000) == 0)
        {
            ((PlayerState*)b)->groundObject = (GameObject*)hit;
        }
        return 3;
    }
    return 0;
}

void fn_802A93F4(GameObject* obj, int p2, int p3)
{
    PlayerState* inner = obj->extra;
    f32 dist;
    void* found;
    s16* vec;
    ObjTextureRuntimeSlot* tex;
    dist = lbl_803E80CC;
    obj->anim.rootMotionScale = lbl_803E7EE0;
    viewFinderSetZoom(Camera_GetFovY());
    obj->objectFlags &= ~OBJECT_OBJFLAG_PARENT_SLACK;
    obj->anim.alpha = 0xff;
    ((ByteFlags*)((char*)inner + 0x3f2))->b80 = 0;
    if (((ByteFlags*)((char*)inner + 0x3f2))->b40)
    {
        inner->targetSuppressTimer = lbl_803E7FBC;
    }
    ((ByteFlags*)((char*)inner + 0x3f2))->b40 = 0;
    ((ByteFlags*)((char*)inner + 0x3f2))->b20 = 0;
    ((ByteFlags*)((char*)inner + 0x3f4))->b80 = 0;
    ObjHits_EnableObject(obj);
    obj->anim.velocityY = lbl_803E7EA4;
    if ((*(s16*)((char*)p3 + 0x6e) & 1) != 0)
    {
        fn_802AB5A4(obj, (int)inner, 7);
    }
    ObjModelChain_SetEnabled((ObjModelChain*)gPlayerModelChain, 1);
    inner->timeScaleMode = 2;
    if (gPlayerChildObject != NULL)
    {
        found = (void*)ObjGroup_FindNearestObject(BABYCLOUDRUNNER_OBJGROUP, obj, &dist);
        if (found != NULL)
        {
            (*(void (*)(void*))(*(int*)((char*)*(int*)*(int*)((char*)found + 0x68) + 0x24)))(found);
        }
        ObjLink_DetachChild(obj, (GameObject*)gPlayerChildObject);
        Obj_FreeObject((GameObject*)gPlayerChildObject);
        gPlayerChildObject = NULL;
    }
    *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
    inner->interactObject = 0;
    ((ByteFlags*)((char*)inner + 0x3f0))->b10 = 0;
    ((ByteFlags*)((char*)inner + 0x3f0))->b08 = 0;
    ((ByteFlags*)((char*)inner + 0x3f0))->b04 = 0;
    inner->staffHoldFrames = 0;
    ((ByteFlags*)((char*)inner + 0x3f0))->b80 = 0;
    ((ByteFlags*)((char*)inner + 0x3f0))->b40 = 0;
    ((ByteFlags*)((char*)inner + 0x3f0))->b20 = 0;
    inner->animState = -1;
    ((ByteFlags*)((char*)inner + 0x3f6))->b40 = 0;
    staffFn_80170380(gPlayerStaffObject, 2);
    ((ByteFlags*)((char*)inner + 0x3f0))->b02 = 0;
    *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
    ObjHits_SyncObjectPositionIfDirty(obj);
    inner->waterDepth = lbl_803E7EA4;
    inner->waterSurfaceY = lbl_803E80D0;
    inner->idleDelayTimer = lbl_803E7FA4;
    inner->baddie.physicsActive = 1;
    *(int*)((char*)inner + 0x4) &= ~0x100000;
    *(int*)((char*)inner + 0x4) |= 0x8000000;
    if (*(s8*)(*(int*)((char*)*(int*)&obj->extra + 0x35c)) <= 0)
    {
        (*gPlayerInterface)->setState(obj, inner, 3);
        *(int*)&((PlayerState*)inner)->baddie.unk304 = 0;
    }
    vec = (s16*)objModelGetVecFn_800395d8(obj, 1);
    if (vec != NULL)
    {
        vec[0] = 0;
        vec[1] = 0;
        vec[2] = 0;
    }
    ObjModel_ClearBlendChannels(Obj_GetActiveModel(obj));
    tex = objFindTexture(obj, 1, 0);
    tex->offsetS = 0;
    tex->offsetT = 0;
    tex = objFindTexture(obj, 0, 0);
    tex->offsetS = 0;
    tex->offsetT = 0;
}

void playerCastIceSpell(GameObject* unused)
{
    ObjPlacement* setup;
    s8 i;

    if (!Obj_IsLoadingLocked())
        return;
    for (i = 0; i < 7; i++)
    {
        if (gPlayerSpawnedObjects[i] == NULL)
        {
            setup = Obj_AllocObjectSetup(0x24, 0x4ec);
            ObjPath_GetPointWorldPosition(gPlayerPathObject, 0, &setup->posX, &setup->posY, &setup->posZ, 0);
            setup->color[0] = 2;
            setup->color[1] = 1;
            setup->color[2] = 0xff;
            setup->color[3] = 0xff;
            *(s16*)((char*)setup + 0x1a) = (s16)(i * 3);
            *(s16*)((char*)setup + 0x1c) = 0;
            gPlayerSpawnedObjects[i] = Obj_SetupObject(setup, 5, -1, -1, NULL);
        }
    }
}

int fn_802A97D0(GameObject* obj, int p2)
{
    PlayerState* inner = obj->extra;
    void* slot;
    u8 af;
    u8 c;
    s16 sel = ((PlayerState*)p2)->baddie.controlMode;

    if (!((sel != 1 && sel != 2 && sel != 0x26) || !mainGetBit(GAMEBIT_STAFF_ABILITY_STAFF_BOOSTER) ||
          (slot = inner->cameraTargetObject) == NULL || *(s16*)((char*)slot + 0x46) != 0x64f ||
          ((af = *(u8*)((char*)slot + 0xaf)) & 4) == 0 || (af & 0x18) != 0 ||
          ((PlayerState*)p2)->baddie.targetObj != NULL || (c = inner->curAnimId) == 0x48 || c == 0x47 || c == 0x44 ||
          ((PlayerState*)inner)->heldObj != NULL || ((ByteFlags*)((char*)inner + 0x3f0))->b20 ||
          ((ByteFlags*)((char*)inner + 0x3f0))->b04 || ((ByteFlags*)((char*)inner + 0x3f0))->b08 ||
          ((ByteFlags*)((char*)inner + 0x3f4))->b40 == 0 ||
          *(s16*)((char*)*(int*)((char*)*(int*)&obj->extra + 0x35c) + 4) < 0xa))
    {
        return 1;
    }
    return 0;
}

int playerCanCastPortalOpenSpell(GameObject* obj, int p2)
{
    PlayerState* inner = obj->extra;
    s16 sel = ((PlayerState*)p2)->baddie.controlMode;

    if (sel == 1 || sel == 2)
    {
        void* slot = inner->cameraTargetObject;
        u8 af;
        u8 c;
        if (slot == NULL || *(s16*)((char*)slot + 0x46) != 0x414 || ((af = *(u8*)((char*)slot + 0xaf)) & 4) == 0 ||
            (af & 0x18) != 0)
        {
            return 0;
        }
        if (((PlayerState*)p2)->baddie.targetObj != NULL || (c = inner->curAnimId) == 0x48 || c == 0x47 || c == 0x44 ||
            ((PlayerState*)inner)->heldObj != NULL || ((ByteFlags*)((char*)inner + 0x3f0))->b20 ||
            ((ByteFlags*)((char*)inner + 0x3f0))->b04 || ((ByteFlags*)((char*)inner + 0x3f0))->b08 ||
            ((ByteFlags*)((char*)inner + 0x3f4))->b40 == 0 || *(s16*)((char*)inner->playerStatus + 4) < 0x14 ||
            !mainGetBit(GAMEBIT_STAFF_ABILITY_OPEN_PORTAL))
        {
            return 0;
        }
        return 1;
    }
    return 0;
}

int playerCanCastQuakeSpell(GameObject* obj, int p2)
{
    PlayerState* inner = obj->extra;
    int threshold;
    if (mainGetBit(GAMEBIT_STAFF_ABILITY_SUPER_QUAKE))
    {
        threshold = 0x14;
    }
    else
    {
        threshold = 0xa;
    }
    if (mainGetBit(GAMEBIT_STAFF_ABILITY_GROUND_QUAKE) == 0 ||
        *(s16*)((char*)*(int*)((char*)*(int*)&obj->extra + 0x35c) + 4) < threshold || inner->curAnimId == 0x44 ||
        ((PlayerState*)inner)->heldObj != NULL || ((ByteFlags*)((char*)inner + 0x3f0))->b20 ||
        ((ByteFlags*)((char*)inner + 0x3f0))->b04 || ((ByteFlags*)((char*)inner + 0x3f0))->b08 ||
        ((ByteFlags*)((char*)inner + 0x3f4))->b40 == 0)
    {
        return 0;
    }
    {
        s16 v;
        if ((v = ((PlayerState*)p2)->baddie.controlMode) == 1 || v == 2 || v == 0x25 || v == 0x24)
        {
            return 1;
        }
    }
    return 0;
}

int playerCanCastBlasterSpell(GameObject* obj, int p2, int p3)
{
    PlayerState* inner = obj->extra;
    u8 c;
    int v;
    if ((c = inner->curAnimId) == 0x48 || c == 0x47 || c == 0x44 || ((PlayerState*)inner)->heldObj != NULL ||
        ((ByteFlags*)((char*)inner + 0x3f0))->b20 || ((ByteFlags*)((char*)inner + 0x3f0))->b04 ||
        ((ByteFlags*)((char*)inner + 0x3f0))->b08 || ((ByteFlags*)((char*)inner + 0x3f4))->b40 == 0)
    {
        return 0;
    }
    if (p3 == GAMEBIT_STAFF_ABILITY_FIRE_BLASTER)
    {
        if (*(s16*)((char*)inner->playerStatus + 4) < 2)
            return 0;
    }
    else
    {
        if (*(s16*)((char*)inner->playerStatus + 4) < 1)
            return 0;
    }
    if ((v = ((PlayerState*)p2)->baddie.controlMode) == 1 || v == 2 || v == 0x2a || v == 0x2c || (u16)(v - 0x2e) <= 1 ||
        v == 0x2d)
    {
        return 1;
    }
    return 0;
}

int playerIsBlasterSpellAvailable(GameObject* obj, int p2, int p3)
{
    PlayerState* inner = obj->extra;
    u8 c;
    int v;
    if ((c = inner->curAnimId) == 0x48 || c == 0x47 || c == 0x44 || ((PlayerState*)inner)->heldObj != NULL ||
        ((ByteFlags*)((char*)inner + 0x3f0))->b20 || ((ByteFlags*)((char*)inner + 0x3f0))->b04 ||
        ((ByteFlags*)((char*)inner + 0x3f0))->b08 || ((ByteFlags*)((char*)inner + 0x3f4))->b40 == 0)
    {
        return 0;
    }
    if (p3 == GAMEBIT_STAFF_ABILITY_FIRE_BLASTER)
    {
        if (*(s16*)((char*)inner->playerStatus + 4) < 2)
            return 0;
    }
    else
    {
        if (*(s16*)((char*)inner->playerStatus + 4) < 1)
            return 0;
    }
    if ((v = ((PlayerState*)p2)->baddie.controlMode) == 1 || v == 2 || (u16)(v - 0x24) <= 1 || (u16)(v - 0x2a) <= 2 ||
        (u16)(v - 0x2e) <= 1 || v == 0x2d)
    {
        return 1;
    }
    return 0;
}

void fn_802A9D0C(int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8)
{
    void* vec;
    s16 v;
    f32 a, b, c;
    int d, e, flag;
    s16 angle;
    int clamped;
    int inner;
    if (p8 != 0)
    {
        vec = (void*)objModelGetVecFn_800395d8((GameObject*)(p1), 0);
        if (vec != NULL)
        {
            v = *(s16*)((char*)vec + 0x2);
            if (v > 0)
            {
                *(s16*)((char*)vec + 0x2) -= (s16)(lbl_803E8050 * timeDelta);
                if (*(s16*)((char*)vec + 0x2) < 0)
                {
                    *(s16*)((char*)vec + 0x2) = 0;
                }
            }
            else
            {
                *(s16*)((char*)vec + 0x2) += (s16)(lbl_803E8050 * timeDelta);
                if (*(s16*)((char*)vec + 0x2) > 0)
                {
                    *(s16*)((char*)vec + 0x2) = 0;
                }
            }
        }
        (*(void (*)(int, int, int, int, int, int))(*(int*)((char*)*(int*)*(int*)((char*)p3 + 0x68) + 0x10)))(
            p3, p4, p5, p6, p7, -1);
        ((GameObject*)p1)->anim.previousWorldPosX = ((GameObject*)p1)->anim.worldPosX;
        ((GameObject*)p1)->anim.previousWorldPosY = ((GameObject*)p1)->anim.worldPosY;
        ((GameObject*)p1)->anim.previousWorldPosZ = ((GameObject*)p1)->anim.worldPosZ;
        ((GameObject*)p1)->anim.previousLocalPosX = ((GameObject*)p1)->anim.localPosX;
        ((GameObject*)p1)->anim.previousLocalPosY = ((GameObject*)p1)->anim.localPosY;
        ((GameObject*)p1)->anim.previousLocalPosZ = ((GameObject*)p1)->anim.localPosZ;
    }
    (*(void (*)(int, f32*, f32*, f32*))(*(int*)((char*)*(int*)*(int*)((char*)p3 + 0x68) + 0x28)))(p3, &a, &b, &c);
    ((GameObject*)p1)->anim.localPosX = a;
    ((GameObject*)p1)->anim.localPosY = b;
    ((GameObject*)p1)->anim.localPosZ = c;
    inner = *(int*)&((GameObject*)p1)->extra;
    if (((PlayerState*)inner)->baddie.controlMode != 0x18 &&
        (((GameObject*)p1)->objectFlags & OBJECT_OBJFLAG_PARENT_SLACK) == 0)
    {
        flag = 1;
        (*(void (*)(int, int, int*))(*(int*)((char*)*(int*)*(int*)((char*)p3 + 0x68) + 0x54)))(p3, 2, &d);
        angle = (s16)(((PlayerState*)p2)->targetYaw - (u16)d);
        if (angle > 0x8000)
        {
            angle = angle - 0xFFFF;
        }
        if (angle < -0x8000)
        {
            angle = angle + 0xFFFF;
        }
        (*(void (*)(int, int, int*))(*(int*)((char*)*(int*)*(int*)((char*)p3 + 0x68) + 0x54)))(p3, 3, &e);
        clamped = (angle < (s16)-e) ? (s16)-e : ((angle > (s16)e) ? (s16)e : angle);
        ((PlayerState*)p2)->targetYaw = (s16)d + clamped;
        (*(void (*)(int, int, int*))(*(int*)((char*)*(int*)*(int*)((char*)p3 + 0x68) + 0x54)))(p3, 4, &flag);
        if (flag != 0)
        {
            ((GameObject*)p1)->anim.rotY = ((GameObject*)p3)->anim.rotY;
            ((GameObject*)p1)->anim.rotZ = ((GameObject*)p3)->anim.rotZ;
        }
    }
    else
    {
        ((GameObject*)p1)->anim.rotY = ((GameObject*)p3)->anim.rotY;
        ((GameObject*)p1)->anim.rotZ = ((GameObject*)p3)->anim.rotZ;
        ((PlayerState*)p2)->targetYaw = ((GameObject*)p3)->anim.rotX;
    }
    v = ((PlayerState*)p2)->targetYaw;
    ((PlayerState*)p2)->yaw = v;
    ((GameObject*)p1)->anim.rotX = v;
    ((GameObject*)p1)->anim.worldPosX = ((GameObject*)p1)->anim.localPosX;
    ((GameObject*)p1)->anim.worldPosY = ((GameObject*)p1)->anim.localPosY;
    ((GameObject*)p1)->anim.worldPosZ = ((GameObject*)p1)->anim.localPosZ;
    ((GameObject*)p1)->anim.velocityX = *(f32*)((char*)p3 + 0x24);
    ((GameObject*)p1)->anim.velocityY = *(f32*)((char*)p3 + 0x28);
    ((GameObject*)p1)->anim.velocityZ = *(f32*)((char*)p3 + 0x2c);
    fn_802AB5A4((GameObject*)p1, p2, 7);
}

void fn_802AA014(GameObject* obj, int state, f32 aimInputZ, f32 zero)
{
    void* o;
    int slot;
    ObjPlacement* setup;
    f32 v[3];
    f32 fov, ycomp, cot, aspect, xcomp, len;
    f32 scale;
    f32 mix;
    f32 t;
    int res, h2, hw;
    PlayerState* inner;

    inner = obj->extra;
    slot = (int)Camera_GetCurrentViewSlot();
    if (Obj_IsLoadingLocked())
    {
        setup = Obj_AllocObjectSetup(0x24, 0x14b);
        setup->color[0] = 2;
        setup->color[1] = 1;
        setup->color[2] = 0xff;
        setup->color[3] = 0xff;
        setup->posX = *(f32*)((char*)slot + 0xc);
        setup->posY = *(f32*)((char*)slot + 0x10);
        setup->posZ = *(f32*)((char*)slot + 0x14);
        Sfx_PlayFromObject((int)obj, SFXTRIG_staff_rocket_hitdirt);
        o = Obj_SetupObject(setup, 5, -1, -1, NULL);
        if (o != NULL)
        {
            *(s16*)((char*)o + 6) |= 0x2000;
            res = getScreenResolution();
            hw = res >> 17;
            *(s16*)((char*)o + 0) = *(s16*)((char*)slot + 0);
            t = Camera_GetFovY();
            t *= lbl_803E80D4;
            fov = (gPlayerPi * t) / lbl_803E7F98;
            cot = mathSinf(fov);
            cot = lbl_803E7F5C * (cot / mathCosf(fov));
            aspect = Camera_GetAspectRatio();
            h2 = (u16)res >> 1;
            t = (inner->aimScreenY - (f32)h2) / (f32)h2;
            t *= aspect;
            ycomp = cot * -t;
            xcomp = cot * ((inner->aimScreenX - (f32)hw) / (f32)hw);
            len = sqrtf(lbl_803E80AC + (ycomp * ycomp + xcomp * xcomp));
            v[0] = ycomp / len;
            v[1] = xcomp / len;
            v[2] = lbl_803E7F5C / len;
            Matrix_TransformVector(Camera_GetWorldMatrix(), v, v);
            *(f32*)((char*)o + 0x24) = v[0] * (scale = lbl_803E80D8);
            *(f32*)((char*)o + 0x28) = v[1] * scale;
            *(f32*)((char*)o + 0x2c) = v[2] * scale;
            mix = lbl_803E7ED4;
            *(f32*)((char*)o + 0xc) = *(f32*)((char*)o + 0x18) =
                mix * *(f32*)((char*)o + 0x24) + *(f32*)((char*)slot + 0xc);
            *(f32*)((char*)o + 0x10) = *(f32*)((char*)o + 0x1c) =
                mix * *(f32*)((char*)o + 0x28) + *(f32*)((char*)slot + 0x10);
            *(f32*)((char*)o + 0x14) = *(f32*)((char*)o + 0x20) =
                mix * *(f32*)((char*)o + 0x2c) + *(f32*)((char*)slot + 0x14);
            *(s16*)((char*)o + 2) = *(s16*)((char*)slot + 2) / 2;
            *(s16*)((char*)o + 0) = -*(s16*)((char*)slot + 0);
            *(int*)((char*)o + 0xf4) = 0x64;
        }
    }
}

void fn_802AA2B0(int obj, int state, f32 unused, f32 yoff)
{
    int slot;
    ObjPlacement* setup;
    f32 x1, y1, z1, x0, y0, z0;
    f32 dx, dy, dz, len;

    slot = (int)Camera_GetCurrentViewSlot();
    if (Obj_IsLoadingLocked() != 0)
    {
        Sfx_PlayFromObject(0, SFXTRIG_staff_rocket_hitdirt);
        setup = Obj_AllocObjectSetup(0x24, 0x655);
        setup->color[0] = 2;
        setup->color[1] = 1;
        setup->color[2] = 0xff;
        setup->color[3] = 0xff;
        ObjPath_GetPointWorldPosition((GameObject*)gPlayerPathObject, 0, &x0, &y0, &z0, 0);
        setup->posX = x0 + yoff;
        setup->posY = y0 + yoff;
        setup->posZ = z0 + yoff;
        setup = (ObjPlacement*)Obj_SetupObject(setup, 5, -1, -1, NULL);
        if (setup != NULL)
        {
            ObjPath_GetPointWorldPosition((GameObject*)gPlayerPathObject, 0, &x0, &y0, &z0, 0);
            ObjPath_GetPointWorldPosition((GameObject*)gPlayerPathObject, 1, &x1, &y1, &z1, 0);
            dx = x0 - x1;
            dy = y0 - y1;
            dz = z0 - z1;
            len = sqrtf(dx * dx + dy * dy + dz * dz);
            dx = dx / len;
            dy = dy / len;
            dz = dz / len;
            *(s16*)setup = (s16)getAngle(dx, dz);
            setup->unk02 = (s16)(-getAngle(dy, sqrtf(dx * dx + dz * dz)));
            setup->posX = setup->posX * lbl_803E7EF0;
            arwprojectile_placeForward((GameObject*)setup, lbl_803E7ED8);
            arwprojectile_setLifetime((GameObject*)setup, 0x32);
            if (slot == 1)
            {
                arwprojectile_createLinkedEffect((GameObject*)setup, 1);
            }
        }
    }
}

void staffShootFireball(GameObject* obj, int state, f32 unused)
{
    int spawned = 0;
    PlayerState* inner = obj->extra;
    GameObject* fb;
    int slot;
    ObjPlacement* setup;
    f32 vec[3];
    MatrixTransform v;
    f32 mtx[16];

    slot = (int)Camera_GetCurrentViewSlot();
    if (Obj_IsLoadingLocked())
    {
        Sfx_PlayFromObject((int)obj, SFXTRIG_wp_hitpos_6_20a);
        setup = Obj_AllocObjectSetup(0x24, 0x14b);
        *(u8*)((char*)setup + 0x4) = 2;
        *(u8*)((char*)setup + 0x5) = 1;
        *(u8*)((char*)setup + 0x6) = 0xff;
        *(u8*)((char*)setup + 0x7) = 0xff;
        if (((PlayerState*)state)->baddie.targetObj != NULL)
        {
            ObjPath_GetPointWorldPosition(gPlayerPathObject, 0, (f32*)((char*)setup + 0x8), (f32*)((char*)setup + 0xc),
                                          (f32*)((char*)setup + 0x10), 0);
        }
        else
        {
            ((ObjPlacement*)setup)->posX = *(f32*)((char*)slot + 0xc);
            ((ObjPlacement*)setup)->posY = *(f32*)((char*)slot + 0x10);
            ((ObjPlacement*)setup)->posZ = *(f32*)((char*)slot + 0x14);
        }
        *(s8*)((char*)setup + 0x19) = (s8)(*(int (*)(void*))(
            *(int*)((char*)*(int*)(*(int*)((char*)gPlayerPathObject + 0x68)) + 0x44)))(gPlayerPathObject);
        if (((PlayerState*)state)->baddie.targetObj == NULL)
        {
            *(s16*)((char*)setup + 0x1a) = 1;
        }
        fb = Obj_SetupObject(setup, 5, -1, -1, NULL);
        if (fb == NULL)
        {
            return;
        }
        fb->anim.flags = fb->anim.flags | OBJANIM_FLAG_OWNS_PLACEMENT_DATA;
        if (((PlayerState*)state)->baddie.targetObj != NULL)
        {
            ObjHitVolumeRuntimeTransform* pt;
            GameObject* target;
            GameObject* ppo;
            f32 dx;
            f32 dz;
            f32 dy;
            target = *(GameObject**)&((PlayerState*)state)->baddie.targetObj;
            spawned = (int)target;
            pt = &target->anim.hitVolumeTransforms[target->hitVolumeIndex];
            dx = pt->jointX - (ppo = (GameObject*)gPlayerPathObject)->anim.localPosX;
            dy = pt->jointY - ppo->anim.localPosY;
            dz = pt->jointZ - ppo->anim.localPosZ;
            v.x = 0.0f;
            v.y = 0.0f;
            v.z = 0.0f;
            v.scale = 1.0f;
            v.rotX = inner->targetYaw;
            v.rotY = (s16)getAngle(dy, sqrtf(dx * dx + dz * dz));
            v.rotZ = 0;
            if (obj->anim.parent != NULL)
            {
                v.rotX = v.rotX + *(s16*)(*(int*)&obj->anim.parent);
            }
            setMatrixFromObjectPos(mtx, &v);
            Matrix_TransformPoint(mtx, 0.0f, 0.0f, -10.0f, &fb->anim.velocityX, &fb->anim.velocityY,
                                  &fb->anim.velocityZ);
            fb->anim.worldPosX = fb->anim.localPosX;
            fb->anim.worldPosY = fb->anim.localPosY;
            fb->anim.worldPosZ = fb->anim.localPosZ;
            fb->anim.rotX = inner->targetYaw;
            fb->anim.rotY = *(s16*)((char*)slot + 0x2) / 2;
        }
        else
        {
            int res = getScreenResolution();
            int half = res >> 17;
            f32 fov;
            f32 cot;
            f32 fx;
            f32 mag;
            fb->anim.rotX = *(s16*)((char*)slot + 0x0);
            fov = Camera_GetFovY();
            fov *= 91.022f;
            fov = gPlayerPi * fov / 32768.0f;
            {
                f32 sn = mathSinf(fov);
                cot = 100.0f * (sn / mathCosf(fov));
            }
            fx = cot * -((inner->aimScreenY - (f32)(int)((res & 0xffff) >> 1)) / (f32)(int)((res & 0xffff) >> 1) *
                         Camera_GetAspectRatio());
            cot = cot * ((inner->aimScreenX - (f32)half) / (f32)half);
            mag = sqrtf(10000.0f + (fx * fx + cot * cot));
            vec[0] = fx / mag;
            vec[1] = cot / mag;
            vec[2] = 100.0f / mag;
            Matrix_TransformVector(Camera_GetWorldMatrix(), vec, vec);
            fb->anim.velocityX = -10.0f * vec[0];
            fb->anim.velocityY = -10.0f * vec[1];
            fb->anim.velocityZ = -10.0f * vec[2];
            fb->anim.localPosX = fb->anim.worldPosX = 2.0f * fb->anim.velocityX + *(f32*)((char*)slot + 0xc);
            fb->anim.localPosY = fb->anim.worldPosY = 2.0f * fb->anim.velocityY + *(f32*)((char*)slot + 0x10);
            fb->anim.localPosZ = fb->anim.worldPosZ = 2.0f * fb->anim.velocityZ + *(f32*)((char*)slot + 0x14);
            fb->anim.rotY = *(s16*)((char*)slot + 0x2) / 2;
            fb->anim.rotX = -*(s16*)((char*)slot + 0x0);
        }
        *(int*)((char*)fb + 0xf4) = 0x5f;
        *(int*)((char*)fb + 0xf8) = spawned;
    }
}

void objDoTeleportAnim(GameObject* obj);

void playerDie(GameObject* obj);

typedef struct PlayerSeqPlacement {
    ObjPlacement base;
    u8 pad18[8];
    s8 movementEnabled;
} PlayerSeqPlacement;

STATIC_ASSERT(offsetof(PlayerSeqPlacement, movementEnabled) == 0x20);

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

