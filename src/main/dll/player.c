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

int lbl_80332EC0[5] = {0x1D, 0x1E, 0x1F, 0x20, 0x21};
void* gPlayerSpawnedObjects[7] = {NULL};

s16 lbl_80332EF0[30] = {
    0x000E, 0x000E, 0x000F, 0x0010, 0x0046, 0x0046, 0x0047, 0x0014, 0x0014, 0x0014,
    0x000D, 0x0022, 0x000D, 0x0022, 0x0052, 0x0052, 0x0053, 0x0053, 0x0055, 0x0055,
    0x0056, 0x0056, 0x0058, 0x0058, 0x0059, 0x0059, 0x0414, 0x0414, 0x0415, 0x0415,
};

s16 lbl_80332F2C[14] = {
    0x002D, 0x002E, 0x0038, 0x0039, 0x002F, 0x0030, 0x0031,
    0x0032, 0x0025, 0x0029, 0x0033, 0x0034, 0x003A, 0x003E,
};

s16 lbl_80332F48[24] = {
    0x006E, 0x00B9, 0x00BD, 0x00BB, 0x006F, 0x00BA, 0x00BC, 0x00B8,
    0x00BE, 0x0068, 0x006C, 0x006A, 0x00BF, 0x006B, 0x006D, 0x0069,
    0x0066, 0x0071, 0x0072, 0x0403, 0x0404, 0x0070, 0x00C0, 0x0000,
};

s16 lbl_80332F78[8] = {
    0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C,
};

s16 lbl_80332F88[28] = {
    0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F, 0x0040, 0x0041,
    0x3BA3, 0xD70A, 0x3E08, 0x7FCB, 0x3DDA, 0x6612, 0x3EB8, 0x4620,
    0x3EB1, 0x72EE, 0x3ED3, 0x92E1, 0x3ECC, 0xBFB1, 0x3F50, 0x2949,
    0x3F4C, 0xBFB1, 0x3F99, 0x8FC4,
};

const u8 lbl_802C2B30[12][16] = {
    {0x40, 0x02, 0x01}, {0x40, 0x03, 0x01, 0x02}, {0x40, 0x04, 0x05, 0x06},
    {0x40, 0x06, 0x05, 0x07}, {0x40, 0x06, 0x03, 0x02}, {0x40, 0x03, 0x06, 0x07},
    {0x40, 0x03, 0x07, 0x01}, {0x40, 0x07, 0x05, 0x01}, {0x40, 0x04, 0x00, 0x01},
    {0x40, 0x04, 0x01, 0x05}, {0x40, 0x00, 0x04, 0x02}, {0x40, 0x02, 0x04, 0x06},
};

const f32 lbl_802C2BF0[24] = {
    -14.5f, 20.0f, -14.5f, 14.5f, 20.0f, -14.5f, -14.5f, 20.0f, 14.5f, 14.5f, 20.0f, 14.5f,
    -14.5f, -7.0f, -14.5f, 14.5f, -7.0f, -14.5f, -14.5f, -7.0f, 14.5f, 14.5f, -7.0f, 14.5f,
};

const int lbl_802C2C50[6] = {0, 1, 2, 3, 4, 5};
const int lbl_802C2C68[4] = {6, 105, 105, 255};
const u8 lbl_802C2C78[16] = {0x0B, 0x04, 0x06, 0x0A, 0x0A, 0x03, 0x03, 0x02,
                                    0x0E, 0x10, 0x12, 0x13, 0x05, 0x00, 0x00, 0x00};
const u16 lbl_802C2C88[16] = {1, 2, 4, 8, 8, 16, 16, 64, 128, 256, 1, 32, 0xFFFF, 0, 0, 0};

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


void playerUpdateTail(int unused1, int* unused2, f32* vec, int unused3, int mode, f32 angle)
{
    f32 mtx1[12];
    f32 mtx2[12];

    switch (gPlayerSubState)
    {
    case 0:
        gPlayerModelChainOriginX = lbl_803E7E80;
        gPlayerModelChainOriginY = lbl_803E7E84;
        gPlayerModelChainOriginZ = lbl_803E7E88;
        break;
    case 1:
        gPlayerModelChainOriginX = lbl_803E7E80;
        gPlayerModelChainOriginY = lbl_803E7E84;
        gPlayerModelChainOriginZ = lbl_803E7E88;
        PSMTXRotRad(mtx1, 0x79, lbl_803E7E8C * mathCosfHighPrecision(lbl_803E7E90 * angle - lbl_803E7E94 * (f32)mode));
        PSMTXMultVecSR(mtx1, vec, vec);
        break;
    case 4:
        gPlayerModelChainOriginX = lbl_803E7E98;
        gPlayerModelChainOriginY = lbl_803E7E84;
        gPlayerModelChainOriginZ = lbl_803E7E88;
        PSMTXRotRad(mtx1, 0x79, lbl_803E7E8C * mathCosfHighPrecision(lbl_803E7E90 * angle - lbl_803E7E94 * (f32)mode));
        PSMTXMultVecSR(mtx1, vec, vec);
        break;
    case 5:
        gPlayerModelChainOriginX = lbl_803E7E9C;
        gPlayerModelChainOriginY = lbl_803E7E84;
        gPlayerModelChainOriginZ = lbl_803E7E88;
        PSMTXRotRad(mtx1, 0x79, lbl_803E7E8C * mathCosfHighPrecision(lbl_803E7E90 * angle - lbl_803E7E94 * (f32)mode));
        PSMTXMultVecSR(mtx1, vec, vec);
        break;
    case 2:
        gPlayerModelChainOriginX = lbl_803E7EA0;
        gPlayerModelChainOriginY = lbl_803E7EA4;
        gPlayerModelChainOriginZ = lbl_803E7EA8;
        PSMTXRotRad(mtx1, 0x79, lbl_803E7EAC * mathCosfHighPrecision(lbl_803E7E98 * angle));
        PSMTXRotRad(mtx2, 0x78, lbl_803E7EB0);
        PSMTXConcat(mtx2, mtx1, mtx1);
        PSMTXMultVecSR(mtx1, vec, vec);
        break;
    case 3:
        gPlayerModelChainOriginX = lbl_803E7E80;
        gPlayerModelChainOriginY = lbl_803E7E84;
        gPlayerModelChainOriginZ = lbl_803E7E88;
        PSMTXRotRad(mtx1, 0x79, lbl_803E7EB4 * mathCosfHighPrecision(lbl_803E7EB4 * angle - lbl_803E7EB8 * (f32)mode));
        if (mode == 1)
        {
            PSMTXRotRad(mtx2, 0x78, lbl_803E7EBC);
            PSMTXConcat(mtx2, mtx1, mtx1);
        }
        PSMTXMultVecSR(mtx1, vec, vec);
        break;
    }
}

void playerDoTailAnims(int obj, void* statep)
{
    int* state = (int*)statep;
    int v = *state;
    if ((void*)gPlayerModelChain != NULL)
    {
        ObjModelChain_SetOrigin((ObjModelChain*)gPlayerModelChain, gPlayerModelChainOriginX, gPlayerModelChainOriginY, gPlayerModelChainOriginZ);
        ObjModelChain_Update(state, v, (ObjModelChain*)gPlayerModelChain, playerUpdateTail);
    }
}
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
char sNotOnGroundFailureMessage[] = "FAIL ON NOT ON GROUND\n";

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

void playerUpdatePathEffectCountdown(GameObject* obj, int inner)
{
    f32 outvec[3];
    struct
    {
        u8 pad[0xc];
        f32 x;
        f32 y;
        f32 z;
    } buf;
    f32 mtx[12];
    u8 cnt = ((PlayerState*)inner)->stepDustCount;

    if (cnt != 0)
    {
        if (cnt & 1)
        {
            int t;
            memcpy(mtx, (void*)ObjPath_GetPointModelMtx(obj, 5), 0x30);
            mtx[3] = lbl_803E7EA4;
            mtx[7] = lbl_803E7EA4;
            mtx[11] = lbl_803E7EA4;
            buf.x = lbl_803E7EA4;
            buf.y = lbl_803E7EA4;
            t = ((PlayerState*)inner)->stepDustCount;
            buf.z = lbl_803E7EC8 * (f32)(int)randomGetRange(t + 4, t + 8);
            PSMTXMultVec(mtx, &buf.x, outvec);
            buf.x = lbl_803E7EA4;
            buf.y = lbl_803E7ECC;
            buf.z = lbl_803E7ED0;
            ObjPath_GetPointWorldPosition((GameObject*)obj, 0xa, &buf.x, &buf.y, &buf.z, 1);
            (*gPartfxInterface)->spawnObject((void*)obj, 0x7e5, &buf, 0x200001, -1, outvec);
        }
        ((PlayerState*)inner)->stepDustCount -= 1;
    }
}

int playerStopRidingObject(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    int sub;

    if ((void*)obj == NULL)
    {
        return 0;
    }
    (*gCameraInterface)->loadTriggeredCamAction(0, 1, 0);
    (*gObjectTriggerInterface)->setCamVars(0x42, 4, 0, 0);

    sub = (int)inner->focusObject;
    if ((void*)sub != NULL)
    {
        (*(void (**)(int, int))((char*)*((GameObject*)sub)->anim.dll + 0x3c))(sub, 0);
        (*gCameraInterface)->setFocus((void*)obj, 0);
        obj->anim.flags &= ~8;
        obj->anim.modelState->flags &= ~OBJ_MODEL_STATE_SHADOW_FADE_OUT;
        inner->focusObject = NULL;
        obj->anim.activeMove = -1;
        (*gPlayerInterface)->setState(obj, inner, 1);
        *(int*)&inner->baddie.unk304 = (int)fn_802A514C;
        Music_Trigger(MUSICTRIG_inside_warlock, 0);
        Music_Trigger(MUSICTRIG_drako_2, 0);
        Music_Trigger(MUSICTRIG_starfox_rwing_1_e6, 0);
        Music_Trigger(MUSICTRIG_WLC_Puzzle, 0);
        return 1;
    }
    return 0;
}

void fn_80295918(GameObject* obj, int sel, f32 fval)
{
    int state = *(int*)&obj->extra;
    int iv = (int)fval;
    switch (sel)
    {
    case 1:
    {
        if (((PlayerState*)state)->queuedBitCount < 4)
            *((u8*)((char*)state + 0x8b9) + ((PlayerState*)state)->queuedBitCount++) = (u8)iv;
        break;
    }
    case 6:
        (*gPlayerInterface)->setState(obj, (void*)state, 0x3f);
        break;
    case 5:
        (*gPlayerInterface)->setState(obj, (void*)state, 1);
        *(int*)&((PlayerState*)state)->baddie.unk304 = (int)fn_802A514C;
        break;
    case 10:
        *(u32*)&((PlayerState*)state)->flags360 |= 0x80000LL;
        break;
    case 11:
        *(u32*)&((PlayerState*)state)->flags360 &= ~0x80000LL;
        break;
    }
}

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

int fn_80295A04(GameObject* obj, int sel)
{
    int state = *(int*)&obj->extra;
    switch (sel)
    {
    case 1:
        if ((*(int*)((char*)state + 0x310) & 0x1000) != 0 ||
            (obj->objectFlags & OBJECT_OBJFLAG_PARENT_SLACK) != 0)
            return 0;
        return 1;
    case 2:
        switch (((PlayerState*)state)->baddie.controlMode)
        {
        case 1:
            return 0;
        case 2:
        {
            s16* list;
            s16 key;
            int i;
            i = 0;
            list = *(s16**)((char*)state + 0x3f8);
            key = obj->anim.currentMove;
            while (key != *list && i < 0x14)
            {
                list += 4;
                i += 4;
            }
            return i / 4;
        }
        default:
            return 5;
        }
    case 9:
        return *(s8*)&((PlayerState*)state)->baddie.stateTag == 3;
    case 10:
        return *(u32*)&((PlayerState*)state)->flags360 & 0x200;
    case 11:
        return *(u32*)&((PlayerState*)state)->flags360 & 0x100;
    case 13:
        return ((PlayerState*)state)->baddie.hasTarget == 1;
    case 14:
        return ((PlayerState*)state)->animState;
    case 18:
    {
        void* p = *(void**)((char*)state + 0x7f0);
        if (p != 0)
            return *(s16*)((char*)p + 0x46);
        return 0;
    }
    }
    return 0;
}



void objSetPos(GameObject* obj, f32 f1, f32 f2, f32 f3)
{
    int inner = *(int*)&obj->extra;
    obj->anim.previousWorldPosX = f1;
    obj->anim.previousLocalPosX = f1;
    obj->anim.worldPosX = f1;
    obj->anim.localPosX = f1;
    obj->anim.previousWorldPosY = f2;
    obj->anim.previousLocalPosY = f2;
    obj->anim.worldPosY = f2;
    obj->anim.localPosY = f2;
    obj->anim.previousWorldPosZ = f3;
    obj->anim.previousLocalPosZ = f3;
    obj->anim.worldPosZ = f3;
    obj->anim.localPosZ = f3;
    fn_802AB5A4(obj, inner, 7);
    (*gPlayerInterface)->setState(obj, (void*)inner, 1);
    *(int*)&((PlayerState*)inner)->baddie.unk304 = (int)fn_802A514C;
}

int objIsCurModelNotZero(void* obj)
{
    if (obj != NULL)
    {
        return ((ObjAnimComponent*)obj)->bankIndex != 0;
    }
    return 0;
}

int isTrickyNear(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    return inner->curAnimId != 0x44;
}

int fn_80295C0C(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    return ((inner->flags3F0 >> 1) & 1) == 0;
}

int fn_80295C24(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    return inner->targetSuppressTimer > lbl_803E7EA4;
}

int fn_80295C40(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    return inner->waterDepth > lbl_803E7ED4;
}

int fn_80295C5C(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    return inner->baddie.controlMode == 0x36 && ((ByteFlags*)((char*)inner + 0x3f3))->b10;
}

int fn_80295C88(GameObject* player)
{
    f32 dist = lbl_803E7EDC;
    return ObjGroup_FindNearestObject(LANTERNFIREFLY_OBJGROUP, player, &dist);
}

int fn_80295CBC(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    return inner->baddie.controlMode == 0x13;
}

int playerIsDisguised(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    return (inner->flags3F3 >> 3) & 1;
}

int playerIsPathFollowing(GameObject* player)
{
    PlayerState* inner = player->extra;
    return (inner->flags3F4 >> 6) & 1;
}

void staffToggle(GameObject* obj, int a)
{
    PlayerState* inner = obj->extra;

    if ((void*)gPlayerPathObject == NULL)
    {
        return;
    }
    if (((ByteFlags*)((char*)inner + 0x3f4))->b40 == a)
    {
        return;
    }
    if (a == 0)
    {
        if ((void*)gPlayerPathObject != NULL)
        {
            *(s16*)((char*)gPlayerPathObject + 6) |= 0x4000;
            if ((void*)gPlayerPathObject != NULL && ((ByteFlags*)((char*)inner + 0x3f4))->b40)
            {
                inner->staffActionRequest = 1;
                ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
            }
            mainSetBits(GAMEBIT_ITEM_SuperQuake_Disabled, 1);
            mainSetBits(GAMEBIT_ITEM_Spell0961_Disabled, 1);
            mainSetBits(GAMEBIT_ITEM_SharpClawDisguise_Disabled, 1);
            mainSetBits(GAMEBIT_ITEM_StaffBooster_Disabled, 1);
            mainSetBits(GAMEBIT_ITEM_Spell0965_Disabled, 1);
            mainSetBits(GAMEBIT_ITEM_FireBlaster_Disabled, 1);
            mainSetBits(GAMEBIT_ITEM_PortalSpell_Disabled, 1);
        }
    }
    else
    {
        if ((void*)gPlayerPathObject != NULL)
        {
            if ((void*)gPlayerPathObject != NULL && ((ByteFlags*)((char*)inner + 0x3f4))->b40)
            {
                inner->staffActionRequest = 4;
                ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
            }
            *(s16*)((char*)gPlayerPathObject + 6) &= ~0x4000;
            mainSetBits(GAMEBIT_ITEM_SuperQuake_Disabled, 0);
            mainSetBits(GAMEBIT_ITEM_Spell0961_Disabled, 0);
            mainSetBits(GAMEBIT_ITEM_SharpClawDisguise_Disabled, 0);
            mainSetBits(GAMEBIT_ITEM_StaffBooster_Disabled, 0);
            mainSetBits(GAMEBIT_ITEM_Spell0965_Disabled, 0);
            mainSetBits(GAMEBIT_ITEM_FireBlaster_Disabled, 0);
            mainSetBits(GAMEBIT_ITEM_PortalSpell_Disabled, 0);
        }
    }
    ((ByteFlags*)((char*)inner + 0x3f4))->b40 = a;
}

void playerSetDisguised(GameObject* obj, int mode)
{
    int inner = *(int*)&obj->extra;
    int oldModel;
    int newModel;
    void* tricky;

    objModelGetVecFn_800395d8(obj, 0);
    objModelGetVecFn_800395d8(obj, 9);
    if (mode != 0)
    {
        staffToggle(obj, 0);
        ((ByteFlags*)((char*)inner + 0x3f3))->b08 = 1;
        tricky = getTrickyObject();
        if (tricky != NULL)
        {
            trickyImpress((GameObject*)tricky);
        }
        mainSetBits(GAMEBIT_PlayerIsDisguised, 1);
        Sfx_PlayFromObject((int)obj, SFXTRIG_en_lrope_powerup);
        (*gBoneParticleEffectInterface)->spawnEffect((void*)obj, 0x801, NULL, 0x50, NULL);
        oldModel = (int)Obj_GetActiveModel(obj);
        Obj_SetActiveModelIndex(obj, 2);
        newModel = (int)Obj_GetActiveModel(obj);
        memcpy((void*)*(int*)((char*)newModel + 0x2c), (void*)*(int*)((char*)oldModel + 0x2c), 0x68);
        memcpy((void*)*(int*)((char*)newModel + 0x30), (void*)*(int*)((char*)oldModel + 0x30), 0x68);
        if (mode == 2)
        {
            ((ByteFlags*)((char*)inner + 0x3f4))->b80 = 1;
        }
    }
    else
    {
        staffToggle(obj, 1);
        ((ByteFlags*)((char*)inner + 0x3f3))->b08 = 0;
        ((ByteFlags*)((char*)inner + 0x3f4))->b80 = 0;
        (*gBoneParticleEffectInterface)->spawnEffect((void*)obj, 0x801, NULL, 0x50, NULL);
        oldModel = (int)Obj_GetActiveModel(obj);
        Obj_SetActiveModelIndex(obj, 1);
        newModel = (int)Obj_GetActiveModel(obj);
        memcpy((void*)*(int*)((char*)newModel + 0x2c), (void*)*(int*)((char*)oldModel + 0x2c), 0x68);
        memcpy((void*)*(int*)((char*)newModel + 0x30), (void*)*(int*)((char*)oldModel + 0x30), 0x68);
        mainSetBits(GAMEBIT_PlayerIsDisguised, 0);
        Sfx_PlayFromObject((int)obj, SFXTRIG_en_lrope_powerup);
    }
}

int fn_8029605C(GameObject* obj, f32* p2, f32* p3)
{
    void* inner = obj->extra;
    if (inner == NULL || getCurSeqNo() != 0)
    {
        return 0;
    }
    if ((((PlayerState*)inner)->flags360 & 0x400) != 0u)
    {
        *p2 = ((PlayerState*)inner)->aimScreenY;
        *p3 = ((PlayerState*)inner)->aimScreenX;
        return 1;
    }
    return 0;
}

void fn_802960E4(int a, f32 b, f32 c)
{
}

void fn_802960E8(GameObject* player, s16 effectId)
{
    PlayerState* inner = player->extra;
    inner->pendingBoneEffectId = effectId;
}

void fn_802960F4(GameObject* obj, f32** outFxOffsets)
{
    PlayerState* inner = obj->extra;
    if (outFxOffsets == NULL)
    {
        return;
    }
    *outFxOffsets = &inner->fxOffsetX;
}

f32 fn_8029610C(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    return inner->baddie.animSpeedA;
}

int fn_80296118(GameObject* obj)
{
    int inner = *(int*)&obj->extra;
    return *(int*)&((PlayerState*)inner)->baddie.targetObj;
}

void fn_80296124(GameObject* obj, const Vec3f* position, const Vec3s* rotation, int unused)
{
    PlayerState* inner = obj->extra;
    *(u32*)&((PlayerState*)inner)->flags360 &= ~0x4000LL;
    if (position != NULL)
    {
        obj->anim.localPosX = position->x;
        obj->anim.localPosY = position->y;
        obj->anim.localPosZ = position->z;
        *(u32*)&((PlayerState*)inner)->flags360 |= 0x4000LL;
    }
    if (rotation != NULL)
    {
        s16 t = rotation->x;
        obj->anim.rotX = t;
        inner->targetYaw = t;
        inner->yaw = t;
        inner->yaw = inner->targetYaw;
        obj->anim.rotY = rotation->y;
        obj->anim.rotZ = rotation->z;
        *(u32*)&((PlayerState*)inner)->flags360 |= 0x4000LL;
    }
}

void fn_802961A4(GameObject* obj, int* outMove, f32* outChargeLevel)
{
    PlayerState* inner = obj->extra;
    *outMove = obj->anim.currentMove;
    if (inner->baddie.controlMode == 0x26)
    {
        *outChargeLevel = inner->boulderChargeLevel;
    }
    else
    {
        *outChargeLevel = inner->chargeLevel;
    }
}

void objSetXRot(GameObject* obj, int v)
{
    PlayerState* inner = obj->extra;
    obj->anim.rotX = v;
    inner->targetYaw = v;
    inner->yaw = v;
    *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
}

void fn_802961FC(GameObject* obj, u8 type)
{
    u8 v = type;
    if (type > 2)
    {
        v = 0;
    }
    lbl_803DE459 = v;
}

f32 fn_80296214(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    return inner->verticalVel;
}

void fn_80296220(GameObject* obj, f32 v)
{
    PlayerState* inner = obj->extra;
    inner->verticalVel = v;
}

int Obj_IsParentSlackClear(GameObject* obj)
{
    return (obj->objectFlags & OBJECT_OBJFLAG_PARENT_SLACK) == 0;
}

int fn_80296240(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    ByteFlags* f = (ByteFlags*)((char*)inner + 0x3f0);
    s16 s;
    if (f->b04 || f->b08 || f->b20 || f->b10 || ((ByteFlags*)((char*)inner + 0x3f3))->b08)
    {
        return 0;
    }
    s = inner->baddie.controlMode;
    if (s == 1 || s == 2)
    {
        return 1;
    }
    return 0;
}

int objFn_802962b4(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    ByteFlags* f = (ByteFlags*)((char*)inner + 0x3f0);
    s16 s;
    if (f->b04 || f->b08 || f->b10)
    {
        return 0;
    }
    s = inner->baddie.controlMode;
    if (s == 1 || s == 2)
    {
        return 1;
    }
    return 0;
}

int fn_8029630C(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    return inner->baddie.controlMode != 0x26;
}

int objAnimFn_80296328(GameObject* obj)
{
    PlayerState* inner = ((GameObject*)obj)->extra;
    if (((((GameObject*)obj)->objectFlags & OBJECT_OBJFLAG_PARENT_SLACK) != 0 &&
         ((ByteFlags*)((char*)inner + 0x3f2))->b80 == 0) ||
        ((ByteFlags*)((char*)inner + 0x3f0))->b04 || ((ByteFlags*)((char*)inner + 0x3f0))->b08 ||
        ((ByteFlags*)((char*)inner + 0x3f0))->b20 || ((PlayerState*)inner)->heldObj != NULL ||
        ((ByteFlags*)((char*)inner + 0x3f0))->b02)
    {
        return 0;
    }
    if (inner->baddie.controlMode == 1 || inner->baddie.controlMode == 2 || inner->baddie.controlMode == 0x26 ||
        (inner->baddie.controlMode == 0x18 &&
         (mainGetBit(GAMEBIT_NW_SnowHorn03E3) || *(s16*)((char*)inner->focusObject + 0x46) == 0x416)) ||
        inner->baddie.targetObj != NULL)
    {
        return 1;
    }
    return 0;
}

u8 fn_80296414(GameObject* obj, GameObject* otherObj, u8* out)
{
    PlayerState* inner = obj->extra;
    *out = inner->surfaceDir;
    return inner->baddie.controlMode == 0x1c && *(u32*)&inner->contactObject == (u32)otherObj;
}

int playerGetFlags3F0Bit5(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    return (inner->flags3F0 >> 5) & 1;
}

int EmissionController_IsLingering(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    return inner->emissionState;
}

int fn_80296464(GameObject* player)
{
    PlayerState* inner = player->extra;
    return inner->flags360 & 1;
}

void playerSetHaveSpell(GameObject* obj, int spell, int set)
{
    PlayerState* inner = obj->extra;
    if ((u32)spell > 0xb)
    {
        return;
    }
    if (set != 0)
    {
        inner->staffUnlockedFlags |= (1 << spell);
    }
    else
    {
        inner->staffUnlockedFlags &= ~(1 << spell);
    }
    mainSetBits(gPlayerSpellGameBits[spell], set);
}

int playerHasSpell(GameObject* obj, int spell)
{
    PlayerState* inner = obj->extra;
    if ((u32)spell > 0xb)
    {
        return 0;
    }
    return inner->staffUnlockedFlags & (1 << spell);
}

void objSetAnimStateFlags(GameObject* obj, int flag, int set)
{
    PlayerState* inner = obj->extra;
    if (set != 0)
    {
        *(s8*)((char*)inner->playerStatus + 2) |= flag;
    }
    else
    {
        *(s8*)((char*)inner->playerStatus + 2) &= ~flag;
    }
}

int objGetAnimStateFlags(GameObject* obj, int flag)
{
    PlayerState* inner = obj->extra;
    return *(s8*)((char*)inner->playerStatus + 2) & flag;
}

int playerGetTimeScale(GameObject* obj, f32* out)
{
    PlayerState* inner = obj->extra;
    *out = inner->timeScale;
    return inner->timeScaleMode;
}

int playerSetHeldObject(GameObject* obj, GameObject* heldObj)
{
    PlayerState* inner = obj->extra;
    GameObject* sub;

    if (heldObj != NULL)
    {
        inner->heldObj = heldObj;
        (*gPlayerInterface)->setState(obj, inner, 5);
        *(int*)&((PlayerState*)inner)->baddie.unk304 = (int)fn_802A4B4C;
    }
    else if (inner->heldObj != NULL)
    {
        inner->isHoldingObject = 0;
        sub = inner->heldObj;
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
            *(s16*)((char*)inner->heldObj + 6) &= ~0x4000;
            *(int*)((char*)inner->heldObj + 0xf8) = 0;
            inner->heldObj = NULL;
        }
        *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
        (*gPlayerInterface)->setState(obj, inner, 1);
        *(int*)&((PlayerState*)inner)->baddie.unk304 = (int)fn_802A514C;
    }
    return inner->heldObj != NULL;
}

int fn_8029669C(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    return inner->baddie.controlMode == 7;
}

int fn_802966B4(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    return inner->baddie.controlMode == 6;
}

GameObject* objGetFirstChild(GameObject* obj)
{
    return obj->childObjs[0];
}

int playerGetHeldObject(GameObject* obj, GameObject** outHeldObj)
{
    PlayerState* inner = obj->extra;
    *outHeldObj = inner->heldObj;
    return inner->heldObj != NULL;
}

f32 fn_802966F4(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    return inner->probeHitDist;
}

int objFn_80296700(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    if (inner->staffGrown != 0 && inner->staffActionRequest != 0)
    {
        return 1;
    }
    return 0;
}

void playerPutAwayStaff(GameObject* obj, int mode)
{
    PlayerState* inner = obj->extra;
    if (mode == 0)
    {
        if (gPlayerPathObject == NULL)
            return;
        if (((ByteFlags*)((char*)inner + 0x3f4))->b40 == 0)
            return;
        inner->staffActionRequest = 0;
        ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 0;
    }
    else if (mode == 1)
    {
        if (gPlayerPathObject == NULL)
            return;
        if (((ByteFlags*)((char*)inner + 0x3f4))->b40 == 0)
            return;
        inner->staffActionRequest = 1;
        ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
    }
    else
    {
        if (gPlayerPathObject == NULL)
            return;
        if (((ByteFlags*)((char*)inner + 0x3f4))->b40 == 0)
            return;
        inner->staffActionRequest = 1;
        ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 0;
    }
}

void playerPullOutStaff(GameObject* obj, int mode)
{
    PlayerState* inner = obj->extra;
    if (mode == 0)
    {
        if (gPlayerPathObject == NULL)
            return;
        if (((ByteFlags*)((char*)inner + 0x3f4))->b40 == 0)
            return;
        inner->staffActionRequest = 2;
        ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 0;
    }
    else if (mode == 1)
    {
        if (gPlayerPathObject == NULL)
            return;
        if (((ByteFlags*)((char*)inner + 0x3f4))->b40 == 0)
            return;
        inner->staffActionRequest = 4;
        ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
    }
    else
    {
        if (gPlayerPathObject == NULL)
            return;
        if (((ByteFlags*)((char*)inner + 0x3f4))->b40 == 0)
            return;
        inner->staffActionRequest = 4;
        ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 0;
    }
}

int playerGetMoney(GameObject* player)
{
    PlayerState* inner = player->extra;
    return *(u8*)((char*)inner->playerStatus + 8);
}

void playerAddMoney(GameObject* obj, int amount)
{
    PlayerState* inner = obj->extra;
    int cap;
    int total;
    if (mainGetBit(GAMEBIT_ITEM_200ScarabBag_Got))
    {
        cap = 0xc8;
    }
    else if (mainGetBit(GAMEBIT_ITEM_100ScarabBag_Got))
    {
        cap = 0x64;
    }
    else if (mainGetBit(GAMEBIT_ITEM_50ScarabBag_Got))
    {
        cap = 0x32;
    }
    else
    {
        cap = 0xa;
    }
    total = *(u8*)((char*)inner->playerStatus + 8);
    total += amount;
    if (amount > inner->maxMagicUsed)
    {
        inner->maxMagicUsed = (u8)amount;
    }
    if (total < 0)
    {
        total = 0;
    }
    else if (total > cap)
    {
        total = cap;
    }
    *(u8*)((char*)inner->playerStatus + 8) = (u8)total;
    mainSetBits(GAMEBIT_ITEM_GiveScarabs_Count, total);
}

void fn_8029697C(GameObject* obj, s16* out1, s16* out2)
{
    PlayerState* inner = obj->extra;
    *out1 = lbl_803E7EE4 * inner->aimInputX;
    if (*(void**)((char*)inner + 0x7f0) != NULL)
    {
        *out2 = lbl_803E7EE8 * inner->aimInputZ;
    }
    else
    {
        *out2 = lbl_803E7EEC * inner->aimInputZ;
    }
}

int fn_802969F0(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    if (((ByteFlags*)((char*)inner + 0x3f1))->b01)
    {
        return inner->surfaceType;
    }
    return -1;
}

int playerGetCurMagic(GameObject* player)
{
    PlayerState* inner = player->extra;
    return *(s16*)((char*)inner->playerStatus + 4);
}

void playerAddRemoveMagic(GameObject* obj, int amount)
{
    PlayerState* inner = obj->extra;
    int deref = inner->playerStatus;
    int m = *(s16*)((char*)deref + 4);
    m += amount;
    if (m < 0)
    {
        m = 0;
    }
    else if (m > *(s16*)((char*)deref + 6))
    {
        m = *(s16*)((char*)deref + 6);
    }
    *(s16*)((char*)deref + 4) = (s16)m;
    if (amount > 0)
    {
        Sfx_PlayFromObject(0, SFXTRIG_id_21c);
    }
}

int playerGetMaxMagic(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    return *(s16*)((char*)inner->playerStatus + 6);
}

void fn_80296A9C(GameObject* obj, int delta)
{
    PlayerState* inner = obj->extra;
    int deref = inner->playerStatus;
    int v = *(s16*)((char*)deref + 6) + delta;
    if (v < 0)
    {
        v = 0;
    }
    else if (v > 0x64)
    {
        v = 0x64;
    }
    *(s16*)((char*)deref + 6) = (s16)v;
}

int playerGetMaxHealth(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    return *(s8*)((char*)inner->playerStatus + 1);
}

int playerGetCurHealth(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    return *(s8*)((char*)inner->playerStatus);
}

void playerAddHealth(GameObject* obj, int amount)
{
    PlayerState* inner = obj->extra;
    int h = *(s8*)((char*)inner->playerStatus);
    h += amount;
    if (h < 0)
    {
        h = 0;
    }
    else if (h > *(s8*)((char*)inner->playerStatus + 1))
    {
        h = *(s8*)((char*)inner->playerStatus + 1);
    }
    *(s8*)((char*)inner->playerStatus) = (s8)h;
    if (*(s8*)((char*)inner->playerStatus) <= 0)
    {
        playerDie(obj);
    }
}

void saveSetOverrideHealth(int v)
{
    gPlayerPendingHealth = v;
}

void playerCancelSpell(GameObject* obj, int p2)
{
    playerCastSpell((int)obj, *(int*)&obj->extra, p2);
}

int objGetAnimState80A(GameObject* obj)
{
    void* inner = obj->extra;
    if (inner != NULL)
    {
        return ((PlayerState*)inner)->animState;
    }
    return 0;
}

void fn_80296BBC(GameObject* obj)
{
    int inner = *(int*)&obj->extra;
    *(u32*)&((PlayerState*)inner)->flags360 &= ~PLAYER_FLAG_HITDETECT;
}

void cameraGetPrevPos2(GameObject* obj, f32* x, f32* y, f32* z)
{
    int inner = *(int*)&obj->extra;
    *x = *(f32*)((char*)inner + 0x24);
    *y = *(f32*)((char*)inner + 0x28);
    *z = *(f32*)((char*)inner + 0x2c);
}

void playerLock(GameObject* obj, int lock)
{
    PlayerState* inner = obj->extra;
    if (lock != 0)
    {
        *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_LOCKED;
    }
    else
    {
        *(u32*)&((PlayerState*)inner)->flags360 &= ~PLAYER_FLAG_LOCKED;
    }
}

int playerStatusIsPositive(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    return *(s8*)((char*)inner->playerStatus) > 0;
}

int fn_80296C4C(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    return (inner->flags3F3 >> 1) & 1;
}

int playerIsDead(GameObject* player)
{
    PlayerState* inner = player->extra;
    return (inner->flags3F3 >> 2) & 1;
}

void playerSetIsDead(GameObject* obj, int flag)
{
    int inner = *(int*)&obj->extra;
    ((ByteFlags*)((char*)inner + 0x3f3))->b02 = flag;
}

void playerHeal(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    int deref = inner->playerStatus;
    int v = *(s8*)((char*)deref + 1);
    if (v < 0)
    {
        v = 0;
    }
    else
    {
        int hi = *(s8*)(deref + 1);
        if (v > hi)
        {
            v = hi;
        }
    }
    *(s8*)((char*)*(int*)((char*)inner + 0x35C)) = (s8)v;
    Obj_SetModelColorFadeRecursive(obj, 0x168, 0xc8, 0, 0, 1);
    ((ByteFlags*)((char*)inner + 0x3f3))->b04 = 1;
    inner->knockbackTimer = lbl_803E7EA4;
    inner->moveVariantIndex = 0xff;
}

void fn_80296D20(GameObject* obj, GameObject* parentObj)
{
    int state = (int)((GameObject*)obj)->extra;
    PlayerState* inner = ((GameObject*)obj)->extra;
    short type;

    if (((GameObject*)obj)->anim.parent == parentObj)
    {
        objHitDetectFn_80062e84((GameObject*)obj, NULL, 1);
        type = ((PlayerState*)state)->baddie.controlMode;
        if (type == 0xa || type == 0xc)
        {
            *(int*)((char*)state + 4) &= ~0x100000;
            fn_802AB5A4((GameObject*)obj, (int)inner, 5);
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
            if (((PlayerState*)inner)->heldObj != NULL)
            {
                short id = ((GameObject*)inner->heldObj)->anim.seqId;
                if (id == 0x3cf || id == 0x662)
                {
                    objThrowFn_80182504((GameObject*)(inner->heldObj));
                }
                else
                {
                    objSaveFn_800ea774((GameObject*)inner->heldObj);
                }
                *(s16*)((char*)inner->heldObj + 6) &= ~0x4000;
                *(int*)((char*)inner->heldObj + 0xf8) = 0;
                inner->heldObj = 0;
            }
            (*gPlayerInterface)->setState(obj, (void*)state, 2);
            *(int*)&((PlayerState*)state)->baddie.unk304 = (int)fn_802A514C;
        }
    }
}

void fn_80296EB4(GameObject* obj, int newParent)
{
    int oldParent = *(int*)&obj->anim.parent;
    int a0;
    int a1;
    int a2;
    int a3;
    int a4;
    int a5;
    PlayerState* inner = obj->extra;
    struct
    {
        f32 wp0[3];
        f32 wv[3];
        f32 wp2[3];
        f32 wp[3];
    } s;

    if ((void*)oldParent == (void*)newParent)
    {
        return;
    }
    if ((void*)oldParent != NULL)
    {
        Obj_TransformLocalPointToWorld(obj->anim.localPosX, obj->anim.localPosY, obj->anim.localPosZ,
                                       &s.wp[0], &s.wp[1], &s.wp[2], oldParent);
        Obj_TransformLocalPointToWorld(obj->anim.previousLocalPosX, obj->anim.previousLocalPosY,
                                       obj->anim.previousLocalPosZ, &s.wp2[0], &s.wp2[1], &s.wp2[2], oldParent);
        Obj_TransformLocalVectorToWorld(obj->anim.velocityX, lbl_803E7EA4, obj->anim.velocityZ,
                                        &s.wv[0], &s.wv[1], &s.wv[2], oldParent);
        a0 = Angle_AddWrappedS16(obj->anim.rotX, (s16*)oldParent);
        a1 = Angle_AddWrappedS16(inner->targetYaw, (s16*)oldParent);
        a2 = Angle_AddWrappedS16(inner->yaw, (s16*)oldParent);
        a3 = Angle_AddWrappedS16(inner->prevTargetYaw, (s16*)oldParent);
        a4 = Angle_AddWrappedS16(inner->prevYaw, (s16*)oldParent);
        a5 = Angle_AddWrappedS16(inner->lastInputHeading, (s16*)oldParent);
        Obj_TransformLocalPointToWorld(*(f32*)((char*)inner + 0x118), *(f32*)((char*)inner + 0x11c),
                                       *(f32*)((char*)inner + 0x120), &s.wp0[0], &s.wp0[1], &s.wp0[2], oldParent);
    }
    else
    {
        s.wp[0] = obj->anim.localPosX;
        s.wp[1] = obj->anim.localPosY;
        s.wp[2] = obj->anim.localPosZ;
        s.wp2[0] = obj->anim.previousLocalPosX;
        s.wp2[1] = obj->anim.previousLocalPosY;
        s.wp2[2] = obj->anim.previousLocalPosZ;
        s.wv[0] = obj->anim.velocityX;
        s.wv[2] = obj->anim.velocityZ;
        a0 = obj->anim.rotX;
        a1 = inner->targetYaw;
        a2 = inner->yaw;
        a3 = inner->prevTargetYaw;
        a4 = inner->prevYaw;
        a5 = inner->lastInputHeading;
        s.wp0[0] = *(f32*)((char*)inner + 0x118);
        s.wp0[1] = *(f32*)((char*)inner + 0x11c);
        s.wp0[2] = *(f32*)((char*)inner + 0x120);
    }
    if ((void*)newParent != NULL)
    {
        Obj_TransformWorldPointToLocal(s.wp[0], s.wp[1], s.wp[2], &obj->anim.localPosX,
                                       &obj->anim.localPosY, &obj->anim.localPosZ,
                                       newParent);
        Obj_TransformWorldPointToLocal(s.wp2[0], s.wp2[1], s.wp2[2], &obj->anim.previousLocalPosX,
                                       &obj->anim.previousLocalPosY,
                                       &obj->anim.previousLocalPosZ, newParent);
        Obj_TransformWorldVectorToLocal(s.wv[0], lbl_803E7EA4, s.wv[2], &obj->anim.velocityX, &s.wv[1],
                                        &obj->anim.velocityZ, newParent);
        obj->anim.rotX = Angle_SubWrappedS16(a0, (s16*)newParent);
        inner->targetYaw = Angle_SubWrappedS16(a1, (s16*)newParent);
        inner->yaw = Angle_SubWrappedS16(a2, (s16*)newParent);
        inner->prevTargetYaw = Angle_SubWrappedS16(a3, (s16*)newParent);
        inner->prevYaw = Angle_SubWrappedS16(a4, (s16*)newParent);
        inner->lastInputHeading = Angle_SubWrappedS16(a5, (s16*)newParent);
        Obj_TransformWorldPointToLocal(s.wp0[0], s.wp0[1], s.wp0[2], (f32*)((char*)inner + 0x118),
                                       (f32*)((char*)inner + 0x11c), (f32*)((char*)inner + 0x120), newParent);
    }
    else
    {
        obj->anim.localPosX = s.wp[0];
        obj->anim.localPosY = s.wp[1];
        obj->anim.localPosZ = s.wp[2];
        obj->anim.previousLocalPosX = s.wp2[0];
        obj->anim.previousLocalPosY = s.wp2[1];
        obj->anim.previousLocalPosZ = s.wp2[2];
        obj->anim.velocityX = s.wv[0];
        obj->anim.velocityZ = s.wv[2];
        obj->anim.rotX = a0;
        inner->targetYaw = a1;
        inner->yaw = a2;
        inner->prevTargetYaw = a3;
        inner->prevYaw = a4;
        inner->lastInputHeading = a5;
        *(f32*)((char*)inner + 0x118) = s.wp0[0];
        *(f32*)((char*)inner + 0x11c) = s.wp0[1];
        *(f32*)((char*)inner + 0x120) = s.wp0[2];
    }
    obj->anim.worldPosX = s.wp[0];
    obj->anim.worldPosY = s.wp[1];
    obj->anim.worldPosZ = s.wp[2];
    obj->anim.previousWorldPosX = s.wp2[0];
    obj->anim.previousWorldPosY = s.wp2[1];
    obj->anim.previousWorldPosZ = s.wp2[2];
    Player_GetObjHitsState(obj)->localPosX = obj->anim.localPosX;
    Player_GetObjHitsState(obj)->localPosY = obj->anim.localPosY;
    Player_GetObjHitsState(obj)->localPosZ = obj->anim.localPosZ;
    Player_GetObjHitsState(obj)->worldPosX = obj->anim.worldPosX;
    Player_GetObjHitsState(obj)->worldPosY = obj->anim.worldPosY;
    Player_GetObjHitsState(obj)->worldPosZ = obj->anim.worldPosZ;
    *(int*)&obj->anim.parent = newParent;
}

void playerSetInCutscene(GameObject* obj)
{
    int inner = *(int*)&obj->extra;
    ((ByteFlags*)((char*)inner + 0x3f2))->b20 = 1;
}

void playerSetCutsceneCameraFlag(GameObject* obj)
{
    int inner = *(int*)&obj->extra;
    ((ByteFlags*)((char*)inner + 0x3f2))->b40 = 1;
}

void playerSetOverrideParentSlack(GameObject* obj)
{
    int inner = *(int*)&obj->extra;
    ((ByteFlags*)((char*)inner + 0x3f2))->b80 = 1;
}

u32 playerGetStateFlag310(GameObject* obj)
{
    int inner = *(int*)&obj->extra;
    return *(int*)((char*)inner + 0x310);
}

GameObject* playerGetFocusObject(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    return inner->focusObject;
}

void fn_802972B4(GameObject* obj, u32* flags, f32* p5, f32* p6, f32* p7, u16* p8)
{
    PlayerState* inner = obj->extra;
    s8 idx;
    u8 mode;
    f32 zero;

    *flags = 0;
    zero = lbl_803E7EA4;
    *p5 = zero;
    *p6 = zero;
    *p7 = zero;
    if (inner->baddie.controlMode == 0x26)
    {
        *flags |= 1;
        idx = inner->hitWindowIndex;
        if (idx != -1)
        {
            *flags |= *(int*)((inner->moveSlots + 8) + (u32)inner->moveSlotIndex * 0xb0 + idx * 4);
            *p6 = *(f32*)((inner->moveSlots + 0x70) + (u32)inner->moveSlotIndex * 0xb0 + inner->hitWindowIndex * 4);
            *p7 = *(f32*)((inner->moveSlots + 0x7c) + (u32)inner->moveSlotIndex * 0xb0 + inner->hitWindowIndex * 4);
            *p5 = *(f32*)((inner->moveSlots + 0x94) + (u32)inner->moveSlotIndex * 0xb0 + inner->hitWindowIndex * 4);
        }
        if (*(u8*)((inner->moveSlots + 0x88) + (u32)inner->moveSlotIndex * 0xb0) & 2)
        {
            if (inner->hitCount < inner->hitCountMax)
            {
                *p7 = *p6 = lbl_803E7EA4;
            }
        }
        if ((*(u8*)((inner->moveSlots + 0x88) + (u32)inner->moveSlotIndex * 0xb0) & 1) &&
            inner->cutsceneTimer >= lbl_803E7EF0)
        {
            *flags |= 0x80;
        }
    }
    mode = inner->attackVariantMode;
    if (mode == 0)
    {
        *flags |= 0x100;
    }
    else if (mode == 1)
    {
        *flags |= 0x200;
    }
    else if (mode == 2)
    {
        *flags |= 0x400;
    }
    if (inner->baddie.controlMode == 0x2e || inner->baddie.controlMode == 0x2f)
    {
        *(u32*)flags &= 0x7dLL;
        *flags |= 2;
    }
    *p8 = 0x78;
}

int fn_80297498(void)
{
    return 0x0;
}

int playerState41(GameObject* obj, int state, f32 fv)
{
    PlayerState* inner = obj->extra;
    inner->probeHitDist = lbl_803E7ED8;
    *(u32*)&((PlayerState*)inner)->flags360 |= 0x2000000LL;
    *(int*)((char*)state + 0) |= 0x200000;
    if (lbl_803E7EA4 == inner->verticalVel)
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
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        ObjAnim_SetCurrentMove((int)obj, 0x12, lbl_803E7EA4, 1);
    }
    {
        f32 v = lbl_803E7EE0 + inner->verticalVel;
        f32 w;
        f32 clamped;
        ObjAnimComponent* o;
        w = v * lbl_803E7E98;
        o = (ObjAnimComponent*)obj;
        clamped = (w < lbl_803E7EA4) ? lbl_803E7EA4 : ((w > lbl_803E7EE0) ? lbl_803E7EE0 : w);
        ObjAnim_SetMoveProgress(o, lbl_803E7EE0 - clamped);
    }
    (*(void (*)(int, int, f32, f32, int))(*(int*)((char*)*gPlayerInterface + 0x44)))((int)obj, state, fv, lbl_803E7EE0,
                                                                              inner->inputHeading);
    ((PlayerState*)state)->baddie.velSmoothTime = lbl_803E7EF4;
    ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7EF8;
    obj->anim.velocityY = inner->verticalVel * fv;
    if (((PlayerState*)state)->baddie.inputMagnitude > lbl_803E7EFC)
    {
        f32 ryaw = (f32)inner->targetYawRate * fv;
        inner->targetYaw = (s16)((f32)(s16)inner->targetYaw + gPlayerDegToBinAngle * (ryaw * lbl_803E7F04));
        inner->yaw = inner->targetYaw;
    }
    fn_802ABAE8(obj, state, (int)inner, lbl_803E7EA4);
    return 0;
}

int playerState40(int p1, int obj)
{
    if (*(s8*)((char*)obj + 0x27a) != 0)
    {
        *(u8*)((char*)obj + 0x357) = 0;
    }
    *(u8*)((char*)obj + 0x357) += 1;
    if (*(s8*)((char*)obj + 0x346) != 0 && *(s8*)((char*)obj + 0x357) > 0x1e)
    {
        *(int*)((char*)obj + 0x308) = (int)fn_802A514C;
        return 2;
    }
    return 0;
}

int playerState3F(int obj, int state)
{
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        ObjAnim_SetCurrentMove(obj, 0xe, lbl_803E7EA4, 0);
        *(s8*)&((PlayerState*)state)->baddie.moveDone = 0;
    }
    ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F08;
    if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
    {
        *(int*)&((PlayerState*)state)->baddie.unk308 = 0;
        return 0x41;
    }
    return 0;
}

int playerStateNop3E(void)
{
    return 0x0;
}

void fn_8029782C(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
    ((ByteFlags*)((char*)inner + 0x3f6))->b20 = 0;
}

int playerState3D(int obj, int state, f32 fv)
{
    PlayerState* inner = ((GameObject*)obj)->extra;
    int r;
    f32 k;
    s16 hdr;

    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        ObjAnim_SetCurrentMove(obj, gPlayerMoveSlotTable[gPlayerMoveSlotData[7].moveTableIndex], lbl_803E7EA4, 0);
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F0C;
        k = lbl_803E7EA4;
        ((PlayerState*)state)->baddie.animSpeedC = k;
        ((PlayerState*)state)->baddie.animSpeedB = k;
        ((PlayerState*)state)->baddie.animSpeedA = k;
        ((GameObject*)obj)->anim.velocityX = k;
        ((GameObject*)obj)->anim.velocityY = k;
        ((GameObject*)obj)->anim.velocityZ = k;
    }
    r = playerState28((GameObject*)obj, state, fv);
    if (r != 0)
    {
        return r;
    }
    (*gPlayerInterface)->rotateTowardTarget((void*)obj, (void*)state, fv, 0x10);
    hdr = *(s16*)obj;
    inner->yaw = hdr;
    inner->targetYaw = hdr;
    (*gPlayerInterface)->updateAnimRootMotion((void*)obj, (void*)state, fv, 1);
    if (*(int*)&((PlayerState*)state)->baddie.eventFlags & 0x200)
    {
        doRumble(lbl_803E7F10);
        Sfx_PlayFromObject(obj, SFXTRIG_rserv1_c);
        inner->pendingFxFlags |= 4;
    }
    if ((((PlayerState*)state)->baddie.moveEventFlags & 1) == 0 &&
        ((GameObject*)obj)->anim.currentMoveProgress > lbl_803E7F14)
    {
        Sfx_PlayFromObject(obj, SFXTRIG_sp_sa_def01);
        ((PlayerState*)state)->baddie.moveEventFlags |= 1;
    }
    if ((((PlayerState*)state)->baddie.moveEventFlags & 2) == 0 &&
        ((GameObject*)obj)->anim.currentMoveProgress > lbl_803E7F18)
    {
        Sfx_PlayFromObject(obj, audioPickSoundEffect_8006ed24(inner->surfaceType, inner->footstepSoundId));
        ((PlayerState*)state)->baddie.moveEventFlags |= 2;
    }
    if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
    {
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029C8C8;
        return 0x25;
    }
    if (((GameObject*)obj)->anim.currentMoveProgress > lbl_803E7F1C)
    {
        if (((PlayerState*)state)->baddie.hasTarget != 1)
        {
            if (gPlayerPathObject != 0 && ((ByteFlags*)((char*)inner + 0x3f4))->b40)
            {
                inner->staffActionRequest = 0;
                ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 0;
            }
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
            return -1;
        }
        r = playerState30((GameObject*)obj, state, fv);
        if (r != 0)
        {
            return r;
        }
    }
    return 0;
}

int playerState3C(GameObject* obj, int state, f32 fv)
{
    PlayerState* inner = obj->extra;
    int r;
    f32 k;
    s16 hdr;

    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        ObjAnim_SetCurrentMove((int)obj, gPlayerMoveSlotTable[gPlayerMoveSlotData[6].moveTableIndex], lbl_803E7EA4, 0);
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F20;
        k = lbl_803E7EA4;
        ((PlayerState*)state)->baddie.animSpeedC = k;
        ((PlayerState*)state)->baddie.animSpeedB = k;
        ((PlayerState*)state)->baddie.animSpeedA = k;
        obj->anim.velocityX = k;
        obj->anim.velocityY = k;
        obj->anim.velocityZ = k;
    }
    r = playerState28(obj, state, fv);
    if (r != 0)
    {
        return r;
    }
    (*gPlayerInterface)->rotateTowardTarget(obj, (void*)state, fv, 0x10);
    hdr = *(s16*)obj;
    inner->yaw = hdr;
    inner->targetYaw = hdr;
    (*gPlayerInterface)->updateAnimRootMotion(obj, (void*)state, fv, 1);
    if ((((PlayerState*)state)->baddie.moveEventFlags & 1) == 0 &&
        obj->anim.currentMoveProgress > lbl_803E7F14)
    {
        Sfx_PlayFromObject((int)obj, SFXTRIG_sp_sa_def01);
        ((PlayerState*)state)->baddie.moveEventFlags |= 1;
    }
    if ((((PlayerState*)state)->baddie.moveEventFlags & 2) == 0 &&
        obj->anim.currentMoveProgress > lbl_803E7F18)
    {
        Sfx_PlayFromObject((int)obj, SFXTRIG_fox_fightbreath2);
        ((PlayerState*)state)->baddie.moveEventFlags |= 2;
    }
    if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
    {
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029C8C8;
        return 0x25;
    }
    if (obj->anim.currentMoveProgress > lbl_803E7F1C)
    {
        if (((PlayerState*)state)->baddie.hasTarget != 1)
        {
            if (gPlayerPathObject != 0 && ((ByteFlags*)((char*)inner + 0x3f4))->b40)
            {
                inner->staffActionRequest = 0;
                ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 0;
            }
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
            return -1;
        }
        r = playerState30(obj, state, fv);
        if (r != 0)
        {
            return r;
        }
    }
    return 0;
}

int playerState3B(GameObject* obj, int state, f32 fv)
{
    PlayerState* inner = obj->extra;
    int r;
    f32 k;
    s16 hdr;

    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        ObjAnim_SetCurrentMove((int)obj, gPlayerMoveSlotTable[gPlayerMoveSlotData[9].moveTableIndex], lbl_803E7EA4, 0);
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F24;
        ((PlayerState*)state)->baddie.moveEventFlags = 0;
        k = lbl_803E7EA4;
        ((PlayerState*)state)->baddie.animSpeedC = k;
        ((PlayerState*)state)->baddie.animSpeedB = k;
        ((PlayerState*)state)->baddie.animSpeedA = k;
        obj->anim.velocityX = k;
        obj->anim.velocityY = k;
        obj->anim.velocityZ = k;
    }
    r = playerState28(obj, state, fv);
    if (r != 0)
    {
        return r;
    }
    (*gPlayerInterface)->rotateTowardTarget(obj, (void*)state, fv, 1);
    hdr = *(s16*)obj;
    inner->yaw = hdr;
    inner->targetYaw = hdr;
    (*gPlayerInterface)->updateAnimRootMotion(obj, (void*)state, fv, 2);
    if (*(int*)&((PlayerState*)state)->baddie.eventFlags & 0x200)
    {
        doRumble(lbl_803E7F10);
        Sfx_PlayFromObject((int)obj, SFXTRIG_rserv1_c);
        inner->pendingFxFlags |= 4;
    }
    if ((((PlayerState*)state)->baddie.moveEventFlags & 1) == 0 &&
        obj->anim.currentMoveProgress > lbl_803E7F14)
    {
        Sfx_PlayFromObject((int)obj, SFXTRIG_fox_fightbreath2);
        ((PlayerState*)state)->baddie.moveEventFlags |= 1;
    }
    if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
    {
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029C8C8;
        return 0x25;
    }
    if (obj->anim.currentMoveProgress > lbl_803E7F1C)
    {
        if (((PlayerState*)state)->baddie.hasTarget != 1)
        {
            if (gPlayerPathObject != 0 && ((ByteFlags*)((char*)inner + 0x3f4))->b40)
            {
                inner->staffActionRequest = 0;
                ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 0;
            }
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
            return -1;
        }
        r = playerState30(obj, state, fv);
        if (r != 0)
        {
            return r;
        }
    }
    return 0;
}

int playerState3A(GameObject* obj, int state, f32 fv)
{
    PlayerState* inner = obj->extra;
    int r;
    f32 k;
    s16 hdr;

    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        ObjAnim_SetCurrentMove((int)obj, gPlayerMoveSlotTable[gPlayerMoveSlotData[8].moveTableIndex], lbl_803E7EA4, 0);
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F24;
        ((PlayerState*)state)->baddie.moveEventFlags = 0;
        k = lbl_803E7EA4;
        ((PlayerState*)state)->baddie.animSpeedC = k;
        ((PlayerState*)state)->baddie.animSpeedB = k;
        ((PlayerState*)state)->baddie.animSpeedA = k;
        obj->anim.velocityX = k;
        obj->anim.velocityY = k;
        obj->anim.velocityZ = k;
    }
    r = playerState28(obj, state, fv);
    if (r != 0)
    {
        return r;
    }
    (*gPlayerInterface)->rotateTowardTarget(obj, (void*)state, fv, 1);
    hdr = *(s16*)obj;
    inner->yaw = hdr;
    inner->targetYaw = hdr;
    (*gPlayerInterface)->updateAnimRootMotion(obj, (void*)state, fv, 2);
    if (*(int*)&((PlayerState*)state)->baddie.eventFlags & 0x200)
    {
        doRumble(lbl_803E7F10);
        Sfx_PlayFromObject((int)obj, SFXTRIG_rserv1_c);
        inner->pendingFxFlags |= 4;
    }
    if ((((PlayerState*)state)->baddie.moveEventFlags & 1) == 0 &&
        obj->anim.currentMoveProgress > lbl_803E7F14)
    {
        Sfx_PlayFromObject((int)obj, SFXTRIG_fox_fightbreath2);
        ((PlayerState*)state)->baddie.moveEventFlags |= 1;
    }
    if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
    {
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029C8C8;
        return 0x25;
    }
    if (obj->anim.currentMoveProgress > lbl_803E7F1C)
    {
        if (((PlayerState*)state)->baddie.hasTarget != 1)
        {
            if (gPlayerPathObject != 0 && ((ByteFlags*)((char*)inner + 0x3f4))->b40)
            {
                inner->staffActionRequest = 0;
                ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 0;
            }
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
            return -1;
        }
        r = playerState30(obj, state, fv);
        if (r != 0)
        {
            return r;
        }
    }
    return 0;
}

int playerState39(GameObject* obj, int state, f32 fv)
{
    PlayerState* inner = obj->extra;
    int r;
    f32 k;
    s16 hdr;

    *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_KNOCKBACK;
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7EF8;
        k = lbl_803E7EA4;
        ((PlayerState*)state)->baddie.animSpeedC = k;
        ((PlayerState*)state)->baddie.animSpeedB = k;
        ((PlayerState*)state)->baddie.animSpeedA = k;
        obj->anim.velocityX = k;
        obj->anim.velocityY = k;
        obj->anim.velocityZ = k;
    }
    r = playerState28(obj, state, fv);
    if (r != 0)
    {
        return r;
    }
    (*gPlayerInterface)->rotateTowardTarget(obj, (void*)state, fv, 1);
    hdr = *(s16*)obj;
    inner->yaw = hdr;
    inner->targetYaw = hdr;
    if ((padGetTriggers(0) & 0x20) == 0)
    {
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029C8C8;
        return 0x25;
    }
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        ((ByteFlags*)((char*)inner + 0x3f6))->b10 = 0;
    }
    if (((ByteFlags*)((char*)inner + 0x3f6))->b10)
    {
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7E8C;
        if (obj->anim.currentMove != 0x455)
        {
            doRumble(lbl_803E7ED8);
            ObjAnim_SetCurrentMove((int)obj, 0x455, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.animSpeedA = -inner->animSpeedStart;
        }
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            ((ByteFlags*)((char*)inner + 0x3f6))->b10 = 0;
        }
    }
    else
    {
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7EF8;
        if (obj->anim.currentMove != 0x458 &&
            ObjAnim_GetCurrentEventCountdown((ObjAnimComponent*)obj) == 0)
        {
            ObjAnim_SetCurrentMove((int)obj, 0x458, obj->anim.currentMoveProgress, 0);
            ObjAnim_SetCurrentEventStepFrames((ObjAnimComponent*)obj, 8);
        }
    }
    ((PlayerState*)state)->baddie.animSpeedA =
        ((PlayerState*)state)->baddie.animSpeedA * powfBitEstimate(inner->animSpeedDecay, timeDelta);
    return 0;
}

int playerState38(GameObject* obj, int state, f32 fv)
{
    PlayerState* inner = obj->extra;
    int r;

    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        f32 zero;
        ObjAnim_SetCurrentMove((int)obj, 0xfb, lbl_803E7EA4, 0);
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F28;
        zero = lbl_803E7EA4;
        ((PlayerState*)state)->baddie.animSpeedC = zero;
        ((PlayerState*)state)->baddie.animSpeedB = zero;
        ((PlayerState*)state)->baddie.animSpeedA = zero;
        obj->anim.velocityX = zero;
        obj->anim.velocityY = zero;
        obj->anim.velocityZ = zero;
    }

    r = playerState28(obj, state, fv);
    if (r != 0)
    {
        return r;
    }

    (*gPlayerInterface)->rotateTowardTarget(obj, (void*)state, fv, 1);
    inner->targetYaw = inner->yaw = *(s16*)((char*)obj);
    (*gPlayerInterface)->updateAnimRootMotion(obj, (void*)state, fv, 2);

    if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
    {
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029C8C8;
        return 0x25;
    }
    if (obj->anim.currentMoveProgress > lbl_803E7F2C)
    {
        if (((PlayerState*)state)->baddie.hasTarget != 1)
        {
            if ((void*)gPlayerPathObject != NULL && ((ByteFlags*)((char*)inner + 0x3f4))->b40)
            {
                inner->staffActionRequest = 0;
                ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 0;
            }
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
            return -1;
        }
        r = playerState30(obj, state, fv);
        if (r != 0)
        {
            return r;
        }
    }
    return 0;
}

int playerState37(GameObject* obj, int state)
{
    int inner = *(int*)&obj->extra;
    u8 v;
    ((ByteFlags*)((char*)inner + 0x3f6))->b20 = 1;
    v = *(u8*)((char*)state + 0x34b);
    if (v == 3)
    {
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029782C;
        return 0x3c;
    }
    if (v == 4)
    {
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029782C;
        return 0x3e;
    }
    if (v == 1)
    {
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029782C;
        return 0x3b;
    }
    *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029782C;
    return 0x39;
}

void fn_802985AC(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    ((ByteFlags*)((char*)inner + 0x3f4))->b20 = 0;
    inner->buttonHoldTimer = lbl_803E7EA4;
    ((ByteFlags*)((char*)inner + 0x3f3))->b10 = 0;
    inner->animState = -1;
    ObjHits_SyncObjectPositionIfDirty(obj);
}

int playerStateSuperQuake(GameObject* obj, int state, f32 fv)
{
    PlayerState* inner = obj->extra;
    f32 f;

    *(int*)state |= 0x200000;
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        ((ByteFlags*)((char*)inner + 0x3f3))->b10 = 0;
        if (inner->animState == 0xc55)
        {
            ((PlayerState*)inner)->chargeCapacity = 0x14;
        }
        else
        {
            ((PlayerState*)inner)->chargeCapacity = 0xa;
        }
        ObjHits_MarkObjectPositionDirty((ObjAnimComponent*)obj);
    }
    if (((ByteFlags*)((char*)inner + 0x3f0))->b20 == 0 && lbl_803E7EA4 != inner->verticalVel)
    {
        *(int*)&((PlayerState*)state)->baddie.unk308 = 0;
        return 0x42;
    }
    switch (obj->anim.currentMove)
    {
    case 0x84:
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            ObjAnim_SetCurrentMove((int)obj, 0x85, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7EFC;
        }
        break;
    case 0x85:
        inner->chargeLevel = inner->chargeLevel + lbl_803E7ED4 * fv / lbl_803E7EF0;
        inner->chargeLevel = lbl_803E7E98 * fv + inner->chargeLevel;
        if (inner->chargeLevel >= (f32)(u32) ((PlayerState*)inner)->chargeCapacity)
        {
            int amt;
            int r35c;
            int v;
            int hi;
            Sfx_PlayFromObject((int)obj, SFXTRIG_fox_roll2);
            amt = -((PlayerState*)inner)->chargeCapacity;
            r35c = *(int*)((char*)(*(int*)&obj->extra) + 0x35c);
            v = *(s16*)((char*)r35c + 4) + amt;
            if (v < 0)
            {
                v = 0;
            }
            else if (v > (hi = *(s16*)((char*)r35c + 6)))
            {
                v = hi;
            }
            *(s16*)((char*)r35c + 4) = v;
            if (amt > 0)
            {
                Sfx_PlayFromObject(0, SFXTRIG_id_21c);
            }
            ObjAnim_SetCurrentMove((int)obj, 0x86, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7EF8;
        }
        break;
    case 0x86:
        if (((ByteFlags*)((char*)inner + 0x3f3))->b10 == 0 &&
            obj->anim.currentMoveProgress > lbl_803E7EFC)
        {
            void* tricky = getTrickyObject();
            if (tricky != NULL)
            {
                trickyImpress((GameObject*)tricky);
            }
            Sfx_PlayFromObject((int)obj, SFXTRIG_staff_boulder_move1);
            superQuakeFn_8016d9fc(&obj->anim.localPosX);
            ((ByteFlags*)((char*)inner + 0x3f3))->b10 = 1;
            doRumble(lbl_803E7F30);
        }
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
            return 2;
        }
        break;
    default:
        Sfx_PlayFromObject((int)obj, SFXTRIG_staff_boulder_drops);
        f = lbl_803E7EA4;
        ((PlayerState*)state)->baddie.animSpeedC = f;
        ((PlayerState*)state)->baddie.animSpeedB = f;
        ((PlayerState*)state)->baddie.animSpeedA = f;
        obj->anim.velocityX = f;
        obj->anim.velocityY = f;
        obj->anim.velocityZ = f;
        ObjAnim_SetCurrentMove((int)obj, 0x84, f, 0);
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F34;
        inner->chargeLevel = lbl_803E7EA4;
        ((ByteFlags*)((char*)inner + 0x3f3))->b10 = 0;
        if (gPlayerPathObject != NULL && ((ByteFlags*)((char*)inner + 0x3f4))->b40)
        {
            inner->staffActionRequest = 4;
            ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
        }
        break;
    }
    return 0;
}

void fn_80298924(int obj)
{
    ObjHits_SyncObjectPositionIfDirty((GameObject*)obj);
}

int playerState35(GameObject* obj, int state)
{
    PlayerState* inner = obj->extra;
    f32 f;

    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        ObjHits_MarkObjectPositionDirty((ObjAnimComponent*)obj);
    }
    f = lbl_803E7EA4;
    ((PlayerState*)state)->baddie.animSpeedC = f;
    ((PlayerState*)state)->baddie.animSpeedB = f;
    ((PlayerState*)state)->baddie.animSpeedA = f;
    obj->anim.velocityX = f;
    obj->anim.velocityY = f;
    obj->anim.velocityZ = f;
    setAButtonIcon(0xe);
    setBButtonIcon(0xa);
    switch (obj->anim.currentMove)
    {
    case 0xe0:
        if (obj->anim.currentMoveProgress > lbl_803E7E98 &&
            (((PlayerState*)state)->baddie.moveEventFlags & 1) == 0)
        {
            ((PlayerState*)state)->baddie.moveEventFlags |= 1;
            Sfx_PlayFromObject((int)obj, SFXTRIG_recrate_hit);
        }
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            ObjAnim_SetCurrentMove((int)obj, 0xdf, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F40;
            ((PlayerState*)state)->baddie.moveEventFlags = 0;
        }
        break;
    case 0xde:
        if (obj->anim.currentMoveProgress > lbl_803E7E9C &&
            (((PlayerState*)state)->baddie.moveEventFlags & 1) == 0)
        {
            ((PlayerState*)state)->baddie.moveEventFlags |= 1;
            doRumble(lbl_803E7F10);
            Sfx_PlayFromObject((int)obj, SFXTRIG_staff_rapidfire);
            cfPrisonGuard_setGameBitMirror(gPlayerInteractTarget, 0);
        }
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            ObjAnim_SetCurrentMove((int)obj, 0xe4, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F40;
            Sfx_PlayFromObject((int)obj, SFXTRIG_staff_lever);
        }
        break;
    case 0xe1:
        if (obj->anim.currentMoveProgress > lbl_803E7E98 &&
            (((PlayerState*)state)->baddie.moveEventFlags & 1) == 0)
        {
            ((PlayerState*)state)->baddie.moveEventFlags |= 1;
            Sfx_PlayFromObject((int)obj, SFXTRIG_recrate_hit);
        }
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            ObjAnim_SetCurrentMove((int)obj, 0xde, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F40;
            ((PlayerState*)state)->baddie.moveEventFlags = 0;
        }
        break;
    case 0xdf:
        if (obj->anim.currentMoveProgress > lbl_803E7E9C &&
            (((PlayerState*)state)->baddie.moveEventFlags & 1) == 0)
        {
            ((PlayerState*)state)->baddie.moveEventFlags |= 1;
            doRumble(lbl_803E7F10);
            Sfx_PlayFromObject((int)obj, SFXTRIG_staff_rapidfire);
            cfPrisonGuard_setGameBitMirror(gPlayerInteractTarget, 1);
        }
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            ObjAnim_SetCurrentMove((int)obj, 0xe5, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F40;
            Sfx_PlayFromObject((int)obj, SFXTRIG_staff_lever);
        }
        break;
    case 0xe4:
    case 0xe5:
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
            return 2;
        }
        break;
    default:
        if (cfPrisonGuard_isGameBitMirrorSet(gPlayerInteractTarget) != 0)
        {
            ObjAnim_SetCurrentMove((int)obj, 0xe1, lbl_803E7EA4, 0);
        }
        else
        {
            ObjAnim_SetCurrentMove((int)obj, 0xe0, lbl_803E7EA4, 0);
        }
        staffactivated_calcInteractionTargetXZ(gPlayerInteractTarget, &((GameObject*)obj)->anim.localPosX,
                                               &((GameObject*)obj)->anim.localPosZ);
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F40;
        ((PlayerState*)state)->baddie.moveEventFlags = 0;
        inner->targetYaw = gPlayerInteractTarget->anim.rotX;
        inner->yaw = inner->targetYaw;
        if (gPlayerPathObject != NULL && ((ByteFlags*)((char*)inner + 0x3f4))->b40)
        {
            inner->staffActionRequest = 4;
            ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
        }
        break;
    }
    return 0;
}

int playerState34(GameObject* obj, int state)
{
    PlayerState* inner = obj->extra;
    f32 k;

    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        ObjHits_MarkObjectPositionDirty((ObjAnimComponent*)obj);
    }
    k = lbl_803E7EA4;
    ((PlayerState*)state)->baddie.animSpeedC = k;
    ((PlayerState*)state)->baddie.animSpeedB = k;
    ((PlayerState*)state)->baddie.animSpeedA = k;
    obj->anim.velocityX = k;
    obj->anim.velocityY = k;
    obj->anim.velocityZ = k;

    switch (obj->anim.currentMove)
    {
    case 0xdd:
        if (obj->anim.currentMoveProgress > lbl_803E7F44)
        {
            cfPrisonGuard_setLiftHeight(gPlayerInteractTarget, 0);
        }
        if (obj->anim.currentMoveProgress > lbl_803E7F48 &&
            (((PlayerState*)state)->baddie.moveEventFlags & 1) == 0)
        {
            Sfx_PlayFromObject((int)obj, SFXTRIG_recrate_smash);
            ((PlayerState*)state)->baddie.moveEventFlags |= 1;
        }
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
            return 2;
        }
        break;
    default:
        ObjAnim_SetCurrentMove((int)obj, 0xdd, k, 0);
        staffactivated_calcInteractionTargetXZ(gPlayerInteractTarget, &((GameObject*)obj)->anim.localPosX,
                                               &((GameObject*)obj)->anim.localPosZ);
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7EF8;
        ((PlayerState*)state)->baddie.moveEventFlags = 0;
        inner->targetYaw = gPlayerInteractTarget->anim.rotX;
        inner->yaw = inner->targetYaw;
        if ((void*)gPlayerPathObject != NULL && ((ByteFlags*)((char*)inner + 0x3f4))->b40)
        {
            inner->staffActionRequest = 4;
            ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
        }
        break;
    }
    return 0;
}

int playerStateStaffLiftRock(int obj, int state, f32 fv)
{
    PlayerState* inner = ((GameObject*)obj)->extra;
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        ObjHits_MarkObjectPositionDirty((ObjAnimComponent*)obj);
    }
    setBButtonIcon(0xa);
    {
        f32 zero = lbl_803E7EA4;
        ((PlayerState*)state)->baddie.animSpeedC = zero;
        ((PlayerState*)state)->baddie.animSpeedB = zero;
        ((PlayerState*)state)->baddie.animSpeedA = zero;
        ((GameObject*)obj)->anim.velocityX = zero;
        ((GameObject*)obj)->anim.velocityY = zero;
        ((GameObject*)obj)->anim.velocityZ = zero;
    }
    switch (((GameObject*)obj)->anim.currentMove)
    {
    case 0xab:
        setAButtonIcon(2);
        if (lbl_803DE48C == 0)
        {
            if (((GameObject*)obj)->anim.currentMoveProgress > lbl_803E7E9C)
            {
                Sfx_PlayFromObject(obj, SFXTRIG_staff_rocket_boost);
                lbl_803DE48C = 1;
            }
        }
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            ObjAnim_SetCurrentMove(obj, 0xb1, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7EF8;
        }
        break;
    case 0xb1:
    {
        int flags;
        setAButtonIcon(2);
        cfPrisonGuard_setLiftHeight(gPlayerInteractTarget, 0);
        flags = inner->buttonsJustPressed;
        if ((flags & 0x100) != 0)
        {
            buttonDisable(0, PAD_BUTTON_A);
            gPlayerLiftRockPullAccum = lbl_803E7ED8;
            ObjAnim_SetCurrentMove(obj, 0xac, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7EA4;
        }
        else if ((flags & 0x200) != 0)
        {
            buttonDisable(0, PAD_BUTTON_B);
            Sfx_PlayFromObject(obj, SFXTRIG_staff_rocket_boost);
            ObjAnim_SetCurrentMove(obj, 0xd1, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F4C;
        }
        break;
    }
    case 0xd1:
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
            return 2;
        }
        break;
    case 0xac:
    {
        int count;
        f32 prog;
        setAButtonIcon(2);
        gPlayerLiftRockPullAccum = gPlayerLiftRockPullAccum - lbl_803E7EE0;
        if ((inner->buttonsJustPressedIfNotBusy & PAD_BUTTON_A) != 0 || getCurSeqNo() != 0)
        {
            buttonDisable(0, PAD_BUTTON_A);
            lbl_803DE460 = lbl_803DE460 - fv;
            if (lbl_803DE460 < lbl_803E7EA4)
            {
                Sfx_PlayFromObject(obj, (u16)(inner->characterId == 0 ? SFXTRIG_impact3 : SFXTRIG_literun116));
                lbl_803DE460 = (f32)(int)randomGetRange(0xa, 0x12);
            }
            switch (cfPrisonGuard_getPullRateMode(gPlayerInteractTarget))
            {
            case 2:
                gPlayerLiftRockPullAccum = gPlayerLiftRockPullAccum + lbl_803E7F50;
                break;
            default:
                gPlayerLiftRockPullAccum = gPlayerLiftRockPullAccum + lbl_803E7F54;
                break;
            case 0:
                gPlayerLiftRockPullAccum = gPlayerLiftRockPullAccum + lbl_803E7F58;
                break;
            }
        }
        if (gPlayerLiftRockPullAccum > lbl_803E7F5C)
        {
            gPlayerLiftRockPullAccum = lbl_803E7F5C;
        }
        else if (gPlayerLiftRockPullAccum < lbl_803E7F60)
        {
            gPlayerLiftRockPullAccum = lbl_803E7F60;
        }
        {
            f32 lh = (f32)(int)cfPrisonGuard_getLiftHeight(gPlayerInteractTarget);
            count = (int)(lh + gPlayerLiftRockPullAccum);
        }
        if (count <= 0)
        {
            gPlayerLiftRockPullAccum = lbl_803E7EA4;
            count = 0;
            ObjAnim_SetCurrentMove(obj, 0xb1, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7EF8;
        }
        else if (count > 0x800)
        {
            count = 0x800;
        }
        prog = (f32)count / lbl_803E7F64;
        if (prog >= lbl_803E7F68)
        {
            staffactivated_spawnMapEventDebris(gPlayerInteractTarget);
            Sfx_PlayFromObject(obj, (u16)(inner->characterId == 0 ? SFXTRIG_impact3 : SFXTRIG_literun116));
            ObjAnim_SetCurrentMove(obj, 0xd0, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F6C;
        }
        else
        {
            ObjAnim_SetMoveProgress((ObjAnimComponent*)obj,
                                    prog + (f32)(int)randomGetRange(-0x64, 0x64) / lbl_803E7F70);
        }
        cfPrisonGuard_setLiftHeight(gPlayerInteractTarget, count);
        break;
    }
    case 0xd0:
        cfPrisonGuard_setLiftHeight(gPlayerInteractTarget, 0x800);
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            Sfx_PlayFromObject(obj, SFXTRIG_menuups16k);
            ObjAnim_SetCurrentMove(obj, 0xb2, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7EF8;
        }
        break;
    case 0xb2:
        cfPrisonGuard_setLiftHeight(gPlayerInteractTarget, 0x800);
        if ((inner->buttonsJustPressed & PAD_BUTTON_B) != 0)
        {
            buttonDisable(0, PAD_BUTTON_B);
            Sfx_PlayFromObject(obj, SFXTRIG_staff_rocket_boost);
            ObjAnim_SetCurrentMove(obj, 0xad, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F4C;
        }
        break;
    case 0xad:
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
            return 2;
        }
        break;
    default:
        ObjAnim_SetCurrentMove(obj, 0xab, lbl_803E7EA4, 0);
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F40;
        staffactivated_calcInteractionTargetXZ(gPlayerInteractTarget,
                                               &((GameObject*)obj)->anim.localPosX,
                                               &((GameObject*)obj)->anim.localPosZ);
        inner->targetYaw = gPlayerInteractTarget->anim.rotX + 0x8000;
        inner->yaw = inner->targetYaw;
        if (gPlayerPathObject != NULL && ((ByteFlags*)((char*)inner + 0x3f4))->b40)
        {
            inner->staffActionRequest = 4;
            ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
        }
        gPlayerLiftRockPullAccum = lbl_803E7EA4;
        lbl_803DE48C = 0;
        lbl_803DE460 = lbl_803E7EA4;
        if (inner->curAnimId != 0x48 && inner->curAnimId != 0x47)
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
        break;
    }
    return 0;
}

void fn_802994A4(GameObject* obj)
{
    *(s16*)((char*)*(int*)&obj->extra + 0x80a) = -1;
    ObjHits_SyncObjectPositionIfDirty(obj);
}

int playerStateStaffBoost(GameObject* obj, int state, f32 fv)
{
    PlayerState* inner = obj->extra;
    u32 mask;
    s16 item;
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        ObjHits_MarkObjectPositionDirty((ObjAnimComponent*)obj);
    }
    if ((s16)getYButtonItem(&item) == 1 && item == 0x957)
    {
        mask = 0x900;
    }
    else
    {
        mask = 0x100;
    }
    *(int*)((char*)state + 0) |= 0x200000;
    switch (obj->anim.currentMove)
    {
    case 0x4:
        if (lbl_803DE48D == 0)
        {
            if (obj->anim.currentMoveProgress > lbl_803E7F74)
            {
                Sfx_PlayFromObject((int)obj, SFXTRIG_staff_quake_powerup);
                lbl_803DE48D = 1;
            }
        }
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            if ((inner->buttonsHeld & mask) != 0)
            {
                Sfx_PlayFromObject((int)obj, SFXTRIG_staff_quake_strike);
                ObjAnim_SetCurrentMove((int)obj, 0x87, lbl_803E7EA4, 0);
                ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7EF8;
            }
            else
            {
                ObjAnim_SetCurrentMove((int)obj, 0x43, lbl_803E7EA4, 0);
                ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F78;
            }
        }
        break;
    case 0x87:
        if ((inner->buttonsHeld & mask) != 0 &&
            inner->chargeLevel <=
                (f32) * (s16*)((char*)*(int*)((char*)*(int*)&obj->extra + 0x35c) + 0x4))
        {
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F20 * fv + ((PlayerState*)state)->baddie.moveSpeed;
            if (((PlayerState*)state)->baddie.moveSpeed > lbl_803E7F6C)
            {
                ((PlayerState*)state)->baddie.moveSpeed = *(f32*)&lbl_803E7F6C;
            }
            inner->chargeLevel = lbl_803E7F7C * fv + inner->chargeLevel;
            inner->chargeLevel = lbl_803E7E98 * fv + inner->chargeLevel;
            if (inner->chargeLevel >= lbl_803E7ED8)
            {
                int sub;
                int v;
                inner->chargeLevel = lbl_803E7EA4;
                sub = *(int*)((char*)*(int*)&obj->extra + 0x35c);
                v = *(s16*)((char*)sub + 0x4) - 0xa;
                if (v < 0)
                {
                    v = 0;
                }
                else if (v > *(s16*)((char*)sub + 0x6))
                {
                    v = *(s16*)((char*)sub + 0x6);
                }
                *(s16*)((char*)sub + 0x4) = v;
                Sfx_PlayFromObject((int)obj, SFXTRIG_staff_boulder_move2);
                ObjAnim_SetCurrentMove((int)obj, 0x88, lbl_803E7EA4, 0);
                ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F6C;
            }
        }
        else
        {
            ObjAnim_SetCurrentMove((int)obj, 0x43, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F78;
        }
        break;
    case 0x43:
        if ((inner->buttonsHeld & mask) != 0)
        {
            Sfx_PlayFromObject((int)obj, SFXTRIG_staff_quake_strike);
            ObjAnim_SetCurrentMove((int)obj, 0x87, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7EF8;
        }
        else if ((inner->buttonsJustPressed & PAD_BUTTON_B) != 0)
        {
            buttonDisable(0, PAD_BUTTON_B);
            ObjAnim_SetCurrentMove((int)obj, 0x44, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F80;
        }
        break;
    case 0x44:
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
            obj->anim.velocityY = lbl_803E7EA4;
            inner->animState = -1;
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
            return 2;
        }
        break;
    case 0x88:
        obj->anim.velocityY = lbl_803E7F6C * fv + obj->anim.velocityY;
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            void* t = getTrickyObject();
            if (t != NULL)
            {
                trickyImpress((GameObject*)t);
            }
            ObjAnim_SetCurrentMove((int)obj, 0x7f, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7EB4;
        }
        break;
    case 0x7f:
        obj->anim.velocityY = lbl_803E7EFC * fv + obj->anim.velocityY;
        if (obj->anim.velocityY > lbl_803E7F10)
        {
            obj->anim.velocityY = *(f32*)&lbl_803E7F10;
        }
        if (obj->anim.localPosY > gPlayerStaffBoostTargetY)
        {
            ObjAnim_SetCurrentMove((int)obj, 0x80, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F84;
        }
        break;
    case 0x80:
    {
        f32 p;
        obj->anim.velocityY = obj->anim.velocityY - lbl_803E7F88 * fv;
        p = powfBitEstimate(lbl_803E7F90, fv);
        obj->anim.velocityY = obj->anim.velocityY * p;
        (*gPlayerInterface)->updateAnimRootMotion(obj, (void*)state, fv, 1);
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
            obj->anim.velocityY = lbl_803E7EA4;
            inner->animState = -1;
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
            return 2;
        }
        break;
    }
    default:
    {
        f32 fromVec[3];
        f32 toVec[3];
        u8 hitBuf[0x58];
        f32 zero = lbl_803E7EA4;
        ((PlayerState*)state)->baddie.animSpeedC = zero;
        ((PlayerState*)state)->baddie.animSpeedB = zero;
        ((PlayerState*)state)->baddie.animSpeedA = zero;
        obj->anim.velocityX = zero;
        obj->anim.velocityY = zero;
        obj->anim.velocityZ = zero;
        ObjAnim_SetCurrentMove((int)obj, 0x4, zero, 0);
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F84;
        gPlayerStaffBoostStartY = obj->anim.localPosY;
        inner->targetYaw = gPlayerInteractTarget->anim.rotX;
        inner->yaw = inner->targetYaw;
        staffactivated_calcInteractionTargetXZ(gPlayerInteractTarget, &((GameObject*)obj)->anim.localPosX,
                                               &((GameObject*)obj)->anim.localPosZ);
        fn_802AB5A4(obj, (int)inner, 7);
        *(int*)((char*)state + 0x4) |= 0x8000000;
        fromVec[0] = obj->anim.localPosX;
        fromVec[1] = lbl_803E7ED8 + obj->anim.localPosY;
        fromVec[2] = obj->anim.localPosZ;
        toVec[0] = fromVec[0] - lbl_803E7F5C * mathSinf(gPlayerPi * (f32)(int)inner->targetYaw / lbl_803E7F98);
        toVec[1] = fromVec[1];
        toVec[2] = fromVec[2] - lbl_803E7F5C * mathCosf(gPlayerPi * (f32)(int)inner->targetYaw / lbl_803E7F98);
        if (objBboxFn_800640cc(fromVec, toVec, lbl_803E7EA4, 3, (TrackBBoxHit*)hitBuf, obj, 1, 1, 0xff, 0) != 0)
        {
            gPlayerStaffBoostTargetY = *(f32*)(hitBuf + 0x3c) - lbl_803E7F30;
        }
        else
        {
            gPlayerStaffBoostTargetY = lbl_803E7F5C + obj->anim.localPosY;
        }
        lbl_803DE48D = 0;
        if (gPlayerPathObject != NULL && ((ByteFlags*)((char*)inner + 0x3f4))->b40)
        {
            inner->staffActionRequest = 4;
            ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
        }
        inner->chargeLevel = lbl_803E7EA4;
        if (inner->curAnimId != 0x48 && inner->curAnimId != 0x47)
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
        break;
    }
    }
    return 0;
}

int playerState31(GameObject* obj, int p2)
{
    PlayerState* inner = obj->extra;
    u8 state30 = 0x1a;
    u8 state29 = 0x1a;
    void* near;
    f32 dist;
    f32 dir[3];
    f32 cosv;
    f32 sinv;
    f32 fz;
    dist = lbl_803E7F5C;
    near = (void*)ObjGroup_FindNearestObject(MAGICPLANT_OBJGROUP_B, obj, &dist);
    ((ByteFlags*)((char*)inner + 0x3f4))->b20 = 1;
    fz = lbl_803E7EA4;
    inner->buttonHoldTimer = fz;
    if (near != 0)
    {
        dir[0] = *(f32*)((char*)near + 0xc) - obj->anim.localPosX;
        dir[1] = *(f32*)((char*)near + 0x10) - obj->anim.localPosY;
        dir[2] = *(f32*)((char*)near + 0x14) - obj->anim.localPosZ;
        dir[1] = fz;
        Vec3_Normalize(dir);
        cosv = mathSinf(gPlayerPi * (f32)inner->targetYaw / lbl_803E7F98);
        sinv = mathCosf(gPlayerPi * (f32)inner->targetYaw / lbl_803E7F98);
        switch (*(u8*)(*(int*)((char*)near + 0x50) + 0x75))
        {
        case 3:
            if (dir[2] * cosv - dir[0] * sinv > lbl_803E7EA4)
            {
                state29 = 0x1a;
            }
            state30 = state29;
            break;
        case 2:
            state29 = 0x1a;
            break;
        case 1:
            state30 ^= state29;
            state29 ^= state30;
            state30 ^= state29;
            break;
        case 0:
        default:
            inner->altMoveToggle = (u8)(inner->altMoveToggle ^ 1);
            if (inner->altMoveToggle != 0)
            {
                state29 = 0x1a;
            }
            break;
        }
    }
    else
    {
        inner->altMoveToggle = (u8)(inner->altMoveToggle ^ 1);
        if (inner->altMoveToggle != 0)
        {
            state29 = 0x1a;
        }
    }
    if (*(u8*)((char*)p2 + 0x34b) == 2 && ((PlayerState*)p2)->baddie.inputMagnitude > lbl_803E7EAC)
    {
        ObjAnim_SetCurrentMove((int)obj, gPlayerMoveSlotTable[((s16*)((char*)inner->moveSlots + 2))[(u8)state30 * 88]],
                               lbl_803E7EA4, 0);
        inner->moveSlotIndex = state30;
        *(int*)&((PlayerState*)p2)->baddie.unk308 = (int)fn_8029BC08;
        return 0x27;
    }
    ObjAnim_SetCurrentMove((int)obj, gPlayerMoveSlotTable[((s16*)((char*)inner->moveSlots + 2))[(u8)state29 * 88]],
                           lbl_803E7EA4, 0);
    inner->moveSlotIndex = state29;
    *(int*)&((PlayerState*)p2)->baddie.unk308 = (int)fn_8029BC08;
    return 0x27;
}

int playerState30(GameObject* obj, int state, f32 fv)
{
    PlayerState* inner = obj->extra;
    struct
    {
        u8 pad[6];
        u16 mode;
        f32 scale;
        f32 x;
        f32 y;
        f32 z;
    } pfx;
    f32 timer;

    if (lbl_803DE42C != 0)
    {
        Sfx_KeepAliveLoopedObjectSound((int)obj, SFXTRIG_whit3_c);
        timer = inner->stateTimer - timeDelta;
        inner->stateTimer = timer;
        if (timer <= lbl_803E7EA4)
        {
            int sub = *(int*)((char*)*(int*)&obj->extra + 0x35c);
            int v = *(s16*)((char*)sub + 0x4) - 1;
            if (v < 0)
            {
                v = 0;
            }
            else if (v > *(s16*)((char*)sub + 0x6))
            {
                v = *(s16*)((char*)sub + 0x6);
            }
            *(s16*)((char*)sub + 0x4) = v;
            inner->stateTimer = lbl_803E7F58;
        }
        ObjPath_GetPointWorldPosition(gPlayerPathObject, 5, &pfx.x, &pfx.y, &pfx.z, 0);
        pfx.scale = lbl_803E7F9C;
        pfx.mode = 0;
        (*gPartfxInterface)->spawnObject((void*)gPlayerPathObject, 0x7f5, &pfx, 0x200001, -1, NULL);
        pfx.mode = 1;
        (*gPartfxInterface)->spawnObject((void*)gPlayerPathObject, 0x7f5, &pfx, 0x200001, -1, NULL);
        if ((inner->buttonsHeld & gPlayerHeldButtonMask) == 0 ||
            *(s16*)((char*)*(int*)((char*)*(int*)&obj->extra + 0x35c) + 0x4) == 0 || getCurSeqNo() != 0)
        {
            int z[2];
            void** p[1];
            z[0] = 0;
            lbl_803DE42C = z[0];
            z[1] = lbl_803DE42C;
            p[0] = gPlayerSpawnedObjects;
            do
            {
                if (*p[0] != NULL)
                {
                    Obj_FreeObject((GameObject*)*p[0]);
                    *p[0] = NULL;
                }
                p[0]++;
                z[1]++;
            } while (z[1] < 7);
            if (gPlayerResource != NULL)
            {
                Resource_Release(gPlayerResource);
                gPlayerResource = NULL;
            }
        }
    }
    if (inner->deferredItemCommand != -1 || (*(int*)&((PlayerState*)state)->baddie.unk31C & 0x800) != 0)
    {
        int r = playerStateTryCastSpell(obj, state, fv);
        if (r != 0)
        {
            return r;
        }
        inner->deferredItemCommand = -1;
    }
    if ((*(int*)&((PlayerState*)state)->baddie.unk31C & 0x400) != 0)
    {
        u8 sel = *(u8*)((char*)state + 0x34b);
        if (sel == 1)
        {
            inner->moveSlotIndex = 8;
            ObjAnim_SetCurrentMove(
                (int)obj, gPlayerMoveSlotTable[*(s16*)((inner->moveSlots + 2) + (u32)inner->moveSlotIndex * 0xb0)],
                lbl_803E7EA4, 0);
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029BC08;
            return 0x27;
        }
        if (sel == 3)
        {
            inner->moveSlotIndex = 9;
            ObjAnim_SetCurrentMove(
                (int)obj, gPlayerMoveSlotTable[*(s16*)((inner->moveSlots + 2) + (u32)inner->moveSlotIndex * 0xb0)],
                lbl_803E7EA4, 0);
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029BC08;
            return 0x27;
        }
        if (sel == 4)
        {
            inner->moveSlotIndex = 7;
            ObjAnim_SetCurrentMove(
                (int)obj, gPlayerMoveSlotTable[*(s16*)((inner->moveSlots + 2) + (u32)inner->moveSlotIndex * 0xb0)],
                lbl_803E7EA4, 0);
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029BC08;
            return 0x27;
        }
        if (sel == 2)
        {
            inner->moveSlotIndex = 6;
            ObjAnim_SetCurrentMove(
                (int)obj, gPlayerMoveSlotTable[*(s16*)((inner->moveSlots + 2) + (u32)inner->moveSlotIndex * 0xb0)],
                lbl_803E7EA4, 0);
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029BC08;
            return 0x27;
        }
        inner->moveSlotIndex = 5;
        ObjAnim_SetCurrentMove((int)obj,
                               gPlayerMoveSlotTable[*(s16*)((inner->moveSlots + 2) + (u32)inner->moveSlotIndex * 0xb0)],
                               lbl_803E7EA4, 0);
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029BC08;
        return 0x27;
    }
    if ((*(int*)&((PlayerState*)state)->baddie.unk31C & 0x100) != 0)
    {
        if (*(u8*)((char*)state + 0x34b) == 2 && ((PlayerState*)state)->baddie.inputMagnitude > lbl_803E7EAC)
        {
            inner->moveSlotIndex = 1;
            ObjAnim_SetCurrentMove(
                (int)obj, gPlayerMoveSlotTable[*(s16*)((inner->moveSlots + 2) + (u32)inner->moveSlotIndex * 0xb0)],
                lbl_803E7EA4, 0);
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029BC08;
            return 0x27;
        }
        if (*(u8*)((char*)state + 0x34b) == 3 && ((PlayerState*)state)->baddie.inputMagnitude > lbl_803E7EAC)
        {
            inner->moveSlotIndex = 4;
            ObjAnim_SetCurrentMove(
                (int)obj, gPlayerMoveSlotTable[*(s16*)((inner->moveSlots + 2) + (u32)inner->moveSlotIndex * 0xb0)],
                lbl_803E7EA4, 0);
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029BC08;
            return 0x27;
        }
        if (*(u8*)((char*)state + 0x34b) == 1 && ((PlayerState*)state)->baddie.inputMagnitude > lbl_803E7EAC)
        {
            inner->moveSlotIndex = 3;
            ObjAnim_SetCurrentMove(
                (int)obj, gPlayerMoveSlotTable[*(s16*)((inner->moveSlots + 2) + (u32)inner->moveSlotIndex * 0xb0)],
                lbl_803E7EA4, 0);
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029BC08;
            return 0x27;
        }
        if (*(u8*)((char*)state + 0x34b) == 4 && ((PlayerState*)state)->baddie.inputMagnitude > lbl_803E7EAC)
        {
            inner->moveSlotIndex = 2;
            ObjAnim_SetCurrentMove(
                (int)obj, gPlayerMoveSlotTable[*(s16*)((inner->moveSlots + 2) + (u32)inner->moveSlotIndex * 0xb0)],
                lbl_803E7EA4, 0);
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029BC08;
            return 0x27;
        }
        inner->moveSlotIndex = 0;
        ObjAnim_SetCurrentMove((int)obj,
                               gPlayerMoveSlotTable[*(s16*)((inner->moveSlots + 2) + (u32)inner->moveSlotIndex * 0xb0)],
                               lbl_803E7EA4, 0);
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029BC08;
        return 0x27;
    }
    return 0;
}

void fn_8029A420(GameObject* obj)
{
    PlayerState* inner = obj->extra;
    if (inner->curAnimId != 0x42 && getCurSeqNo() == 0)
    {
        (*gCameraInterface)->setMode(0x42, 0, 1, 0, NULL, 0x3c, 0xfe);
    }
    ((ByteFlags*)((char*)inner + 0x3f6))->b40 = 0;
    inner->animState = -1;
}

void fn_8029A4A8(GameObject* obj, int p2)
{
    int z[2];
    PlayerState* inner = obj->extra;
    int sel = ((PlayerState*)p2)->baddie.controlMode;

    if (sel == 0x2a)
        return;
    if (sel == 0x2e)
        return;
    if (sel == 0x2f)
        return;
    if (sel == 0x2c)
        return;

    *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
    inner->animState = -1;
    *(u32*)&((PlayerState*)inner)->flags360 &= ~0x2000400LL;

    if (((PlayerState*)p2)->baddie.controlMode != 0x2b)
    {
        if (inner->curAnimId != 0x42 && getCurSeqNo() == 0)
        {
            (*gCameraInterface)->setMode(0x42, 0, 1, 0, NULL, 0x3c, 0xfe);
        }
        ((ByteFlags*)((char*)inner + 0x3f6))->b40 = 0;
    }

    z[0] = 0;
    lbl_803DE42C = z[0];
    for (z[1] = z[0]; z[1] < 7; z[1]++)
    {
        if (gPlayerSpawnedObjects[z[1]] != NULL)
        {
            Obj_FreeObject((GameObject*)gPlayerSpawnedObjects[z[1]]);
            gPlayerSpawnedObjects[z[1]] = NULL;
        }
    }
    if (gPlayerResource != NULL)
    {
        Resource_Release(gPlayerResource);
        gPlayerResource = NULL;
    }
}

int playerStateFireLaser(int obj, int state, f32 fv)
{
    PlayerState* inner = ((GameObject*)obj)->extra;
    int r = fn_802AC7DC(obj, state, (int)inner, fv);
    if (r != 0)
    {
        return r;
    }
    setAButtonIcon(6);
    setBButtonIcon(0xa);
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        int p = *(int*)((char*)*(int*)&((GameObject*)obj)->extra + 0x35c);
        int val = *(s16*)((char*)p + 4);
        if (val < 0)
        {
            val = 0;
        }
        else
        {
            int hi = *(s16*)((char*)p + 6);
            if (val > hi)
            {
                val = hi;
            }
        }
        *(s16*)((char*)p + 4) = (s16)val;
        gPlayerFireLaserCountdown = lbl_803E7F30;
    }
    if (lbl_803E7F30 == gPlayerFireLaserCountdown || lbl_803E7FA0 == gPlayerFireLaserCountdown || lbl_803E7FA4 == gPlayerFireLaserCountdown)
    {
        fn_802AA2B0(obj, state, inner->aimInputZ, (f32)randomGetRange(-0xc8, 0xc8) / lbl_803E7F5C);
    }
    gPlayerFireLaserCountdown = gPlayerFireLaserCountdown - lbl_803E7EE0;
    if (gPlayerFireLaserCountdown < lbl_803E7EA4)
    {
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029A4A8;
        return 0x2d;
    }
    if (((PlayerState*)state)->baddie.targetObj == NULL)
    {
        if ((inner->buttonsJustPressed & PAD_BUTTON_B) != 0 || inner->curAnimId != 0x52)
        {
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029A420;
            return 0x2c;
        }
    }
    return 0;
}

int playerStateShootFireball(GameObject* obj, int state, f32 fv)
{
    PlayerState* inner = obj->extra;
    int r;
    f32 timer;
    struct
    {
        u8 pad[6];
        u16 mode;
        f32 scale;
        f32 x;
        f32 y;
        f32 z;
    } pfx2;
    struct
    {
        u8 pad[6];
        u16 mode;
        f32 scale;
        f32 x;
        f32 y;
        f32 z;
    } pfx;

    if (((PlayerState*)state)->baddie.targetObj == NULL)
    {
        f32 z = lbl_803E7EA4;
        ((PlayerState*)state)->baddie.animSpeedC = z;
        ((PlayerState*)state)->baddie.animSpeedB = z;
        ((PlayerState*)state)->baddie.animSpeedA = z;
        obj->anim.velocityX = z;
        obj->anim.velocityY = z;
        obj->anim.velocityZ = z;
    }
    r = fn_802AC7DC((int)obj, state, (int)inner, fv);
    if (r != 0)
    {
        return r;
    }
    setAButtonIcon(6);
    setBButtonIcon(0xa);
    if (lbl_803DE42C != 0)
    {
        Sfx_KeepAliveLoopedObjectSound((int)obj, SFXTRIG_whit3_c);
        timer = inner->stateTimer - timeDelta;
        inner->stateTimer = timer;
        if (timer <= lbl_803E7EA4)
        {
            int sub = *(int*)((char*)*(int*)&obj->extra + 0x35c);
            int v = *(s16*)((char*)sub + 0x4) - 1;
            if (v < 0)
            {
                v = 0;
            }
            else if (v > *(s16*)((char*)sub + 0x6))
            {
                v = *(s16*)((char*)sub + 0x6);
            }
            *(s16*)((char*)sub + 0x4) = v;
            inner->stateTimer = lbl_803E7F58;
        }
        ObjPath_GetPointWorldPosition(gPlayerPathObject, 5, &pfx.x, &pfx.y, &pfx.z, 0);
        pfx.scale = lbl_803E7F9C;
        pfx.mode = 0;
        (*gPartfxInterface)->spawnObject((void*)gPlayerPathObject, 0x7f5, &pfx, 0x200001, -1, NULL);
        pfx.mode = 1;
        (*gPartfxInterface)->spawnObject((void*)gPlayerPathObject, 0x7f5, &pfx, 0x200001, -1, NULL);
        if ((((PlayerState*)inner)->buttonsHeld & gPlayerHeldButtonMask) == 0 ||
            *(s16*)((char*)*(int*)((char*)*(int*)&obj->extra + 0x35c) + 0x4) == 0 || getCurSeqNo() != 0)
        {
            int z[2];
            void** p[1];
            z[1] = z[0] = lbl_803DE42C = 0;
            p[0] = gPlayerSpawnedObjects;
            do
            {
                if (*p[0] != NULL)
                {
                    Obj_FreeObject((GameObject*)*p[0]);
                    *p[0] = NULL;
                }
                p[0]++;
                z[1]++;
            } while (z[1] < 7);
            if (gPlayerResource != NULL)
            {
                Resource_Release(gPlayerResource);
                gPlayerResource = NULL;
            }
        }
    }
    switch (obj->anim.currentMove)
    {
    case 0x43f:
        if (((PlayerState*)state)->baddie.targetObj == NULL)
        {
            int res;
            int half;
            int low;
            f32 b;
            f32 a;
            f32 k;
            *(u32*)&((PlayerState*)inner)->flags360 &= ~PLAYER_FLAG_AIM_READY;
            a = inner->aimInputZ;
            b = inner->aimInputX;
            res = getScreenResolution();
            half = res >> 17;
            low = (res & 0xffff) >> 1;
            inner->aimScreenY = (k = lbl_803E7E98) * (b * (f32)(int)low) + (f32)(int)low;
            if (a < lbl_803E7EA4)
            {
                inner->aimScreenX = k * (a * (f32)(int)half) + (f32)(int)half;
            }
            else
            {
                inner->aimScreenX = lbl_803E7F44 * (a * (f32)(int)half) + (f32)(int)half;
            }
            *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_AIM_READY;
            if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
            {
                *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029A4A8;
                return 0x2d;
            }
        }
        break;
    default:
    {
        int i;
        int sub;
        int v;
        ObjPath_GetPointWorldPosition(gPlayerPathObject, 0, &pfx2.x, &pfx2.y, &pfx2.z, 0);
        for (i = 0; i < 0x28; i++)
        {
            (*gPartfxInterface)->spawnObject((void*)gPlayerPathObject, 0x3ed, &pfx2, 0x200001, -1, NULL);
        }
        sub = *(int*)((char*)*(int*)&obj->extra + 0x35c);
        v = *(s16*)((char*)sub + 0x4) - 2;
        if (v < 0)
        {
            v = 0;
        }
        else if (v > *(s16*)((char*)sub + 0x6))
        {
            v = *(s16*)((char*)sub + 0x6);
        }
        *(s16*)((char*)sub + 0x4) = v;
        staffShootFireball(obj, state, inner->aimInputZ);
        if (((PlayerState*)state)->baddie.targetObj == NULL)
        {
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029A4A8;
            return 0x2d;
        }
        else
        {
            lbl_803DE460 = lbl_803E7EA4;
            lbl_803DE464 = lbl_803E7EA4;
        }
    }
    break;
    }
    if (((PlayerState*)state)->baddie.targetObj == NULL)
    {
        if ((((PlayerState*)inner)->buttonsJustPressed & PAD_BUTTON_B) != 0 || inner->curAnimId != 0x52)
        {
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029A420;
            return 0x2c;
        }
    }
    return 0;
}

int playerStateTryCastSpell(GameObject* obj, int state, f32 fv)
{
    PlayerState* inner = obj->extra;
    f32 timer;
    struct
    {
        u8 pad[6];
        u16 mode;
        f32 scale;
        f32 x;
        f32 y;
        f32 z;
    } pfx;

    if (lbl_803DE42C != 0)
    {
        Sfx_KeepAliveLoopedObjectSound((int)obj, SFXTRIG_whit3_c);
        timer = inner->stateTimer - timeDelta;
        inner->stateTimer = timer;
        if (timer <= lbl_803E7EA4)
        {
            int sub = *(int*)((char*)*(int*)&obj->extra + 0x35c);
            int v = *(s16*)((char*)sub + 0x4) - 1;
            if (v < 0)
            {
                v = 0;
            }
            else if (v > *(s16*)((char*)sub + 0x6))
            {
                v = *(s16*)((char*)sub + 0x6);
            }
            *(s16*)((char*)sub + 0x4) = v;
            inner->stateTimer = lbl_803E7F58;
        }
        ObjPath_GetPointWorldPosition(gPlayerPathObject, 5, &pfx.x, &pfx.y, &pfx.z, 0);
        pfx.scale = lbl_803E7F9C;
        pfx.mode = 0;
        (*gPartfxInterface)->spawnObject((void*)gPlayerPathObject, 0x7f5, &pfx, 0x200001, -1, NULL);
        pfx.mode = 1;
        (*gPartfxInterface)->spawnObject((void*)gPlayerPathObject, 0x7f5, &pfx, 0x200001, -1, NULL);
        if ((inner->buttonsHeld & gPlayerHeldButtonMask) == 0 ||
            *(s16*)((char*)*(int*)((char*)*(int*)&obj->extra + 0x35c) + 0x4) == 0 || getCurSeqNo() != 0)
        {
            int z[2];
            void** p[1];
            inner->animState = -1;
            z[0] = lbl_803DE42C = z[1] = 0;
            p[0] = gPlayerSpawnedObjects;
            do
            {
                if (*p[0] != NULL)
                {
                    Obj_FreeObject((GameObject*)*p[0]);
                    *p[0] = NULL;
                }
                p[0]++;
                z[0]++;
            } while (z[0] < 7);
            if (gPlayerResource != NULL)
            {
                Resource_Release(gPlayerResource);
                gPlayerResource = NULL;
            }
        }
    }
    else if (inner->deferredItemCommand != -1 || (inner->buttonsJustPressed & PAD_BUTTON_Y) != 0)
    {
        int yitem;
        u16 b28;
        s16 item;
        if (inner->buttonsJustPressed & PAD_BUTTON_Y)
        {
            yitem = getYButtonItem(&item);
            b28 = 0x800;
        }
        else
        {
            yitem = 0;
            item = inner->deferredItemCommand;
            b28 = 0x100;
        }
        if (inner->deferredItemCommand != -1 ||
            (yitem == 1 && (item == GAMEBIT_STAFF_ABILITY_FIRE_BLASTER || item == GAMEBIT_STAFF_ABILITY_FREEZE_BLAST)))
        {
            buttonDisable(0, 0x900);
            ((PlayerState*)inner)->buttonsJustPressed = inner->buttonsJustPressed & ~0x900;
            gPlayerSelectedItem = item;
            if (item != inner->animState)
            {
                playerCastSpell((int)obj, (int)inner, item);
            }
            switch (gPlayerSelectedItem)
            {
            case GAMEBIT_STAFF_ABILITY_FIRE_BLASTER:
            {
                int sub = *(int*)((char*)*(int*)&obj->extra + 0x35c);
                if (*(s16*)((char*)sub + 0x4) >= 2)
                {
                    int r = playerStateShootFireball(obj, state, fv);
                    if (r != 0)
                    {
                        return r;
                    }
                }
                else
                {
                    Sfx_PlayFromObject(0, SFXTRIG_id_10a);
                }
                break;
            }
            case 0x958:
            {
                int sub = *(int*)((char*)*(int*)&obj->extra + 0x35c);
                if (*(s16*)((char*)sub + 0x4) >= 0)
                {
                    int r = playerStateFireLaser((int)obj, state, fv);
                    if (r != 0)
                    {
                        return r;
                    }
                }
                else
                {
                    Sfx_PlayFromObject(0, SFXTRIG_id_10a);
                }
                break;
            }
            case GAMEBIT_STAFF_ABILITY_FREEZE_BLAST:
            {
                int sub = *(int*)((char*)*(int*)&obj->extra + 0x35c);
                if (*(s16*)((char*)sub + 0x4) >= 1)
                {
                    int sub2;
                    int v;
                    playerCastIceSpell(obj);
                    gPlayerHeldButtonMask = b28;
                    lbl_803DE42C = 1;
                    lbl_803DE430 = lbl_803E7EA4;
                    inner->stateTimer = lbl_803E7F58;
                    sub2 = *(int*)((char*)*(int*)&obj->extra + 0x35c);
                    v = *(s16*)((char*)sub2 + 0x4) - 1;
                    if (v < 0)
                    {
                        v = 0;
                    }
                    else if (v > *(s16*)((char*)sub2 + 0x6))
                    {
                        v = *(s16*)((char*)sub2 + 0x6);
                    }
                    *(s16*)((char*)sub2 + 0x4) = v;
                }
                break;
            }
            }
        }
    }
    inner->animState = -1;
    return 0;
}

int playerStateAimStaff(int obj, int state, f32 fv)
{
    PlayerState* inner = ((GameObject*)obj)->extra;
    int r;
    f32 spin;
    PartFxSpawnParams pfx;

    r = fn_802AC7DC(obj, state, (int)inner, fv);
    if (r != 0)
    {
        return r;
    }
    {
        f32 z = lbl_803E7EA4;
        ((PlayerState*)state)->baddie.animSpeedC = z;
        ((PlayerState*)state)->baddie.animSpeedB = z;
        ((PlayerState*)state)->baddie.animSpeedA = z;
        ((GameObject*)obj)->anim.velocityX = z;
        ((GameObject*)obj)->anim.velocityY = z;
        ((GameObject*)obj)->anim.velocityZ = z;
    }
    *(u32*)&((PlayerState*)inner)->flags360 |= 0x2000000LL;
    setAButtonIcon(6);
    setBButtonIcon(0xa);
    switch (((GameObject*)obj)->anim.currentMove)
    {
    case 0x43e:
    {
        f32 t;
        f32 c;
        f32 a;
        t = ((PlayerState*)state)->baddie.moveInputZ / lbl_803E7FA8;
        c = (t < lbl_803E7ECC) ? lbl_803E7ECC : ((t > lbl_803E7EE0) ? lbl_803E7EE0 : t);
        inner->aimInputZ = inner->aimInputZ + interpolate(c - inner->aimInputZ, lbl_803E7EFC, timeDelta);
        t = ((PlayerState*)state)->baddie.moveInputX / lbl_803E7FA8;
        c = (t < lbl_803E7ECC) ? lbl_803E7ECC : ((t > lbl_803E7EE0) ? lbl_803E7EE0 : t);
        inner->aimInputX = inner->aimInputX + interpolate(c - inner->aimInputX, lbl_803E7EFC, timeDelta);
        if ((t = inner->aimInputX) > lbl_803E7EA4)
        {
            spin = t - lbl_803E7EA0;
            if (spin < lbl_803E7EA4)
            {
                spin = lbl_803E7EA4;
            }
        }
        else
        {
            spin = lbl_803E7EA0 + t;
            if (spin > lbl_803E7EA4)
            {
                spin = lbl_803E7EA4;
            }
        }
        a = inner->aimInputZ;
        if (a > *(f32*)&lbl_803E7EA4)
        {
            Object_ObjAnimSetSecondaryBlendMove((ObjAnimComponent*)obj, 0x441, (int)(lbl_803E7FAC * a));
        }
        else
        {
            Object_ObjAnimSetSecondaryBlendMove((ObjAnimComponent*)obj, 0x440, (int)(lbl_803E7FAC * -a));
        }
        inner->bodyLeanHalf = lbl_803E7FB0 * inner->aimInputX;
        objModelGetVecFn_800395d8((GameObject*)(obj), 9);
        *(u32*)&((PlayerState*)inner)->flags360 &= ~PLAYER_FLAG_AIM_READY;
        if (gPlayerSelectedItem == GAMEBIT_STAFF_ABILITY_FIRE_BLASTER)
        {
            f32 bv;
            f32 av;
            int res;
            int half;
            int low;
            f32 k;
            av = inner->aimInputZ;
            bv = inner->aimInputX;
            res = getScreenResolution();
            half = res >> 17;
            low = (res & 0xffff) >> 1;
            inner->aimScreenY = (k = lbl_803E7E98) * (bv * (f32)(int)low) + (f32)(int)low;
            if (av < lbl_803E7EA4)
            {
                inner->aimScreenX = k * (av * (f32)(int)half) + (f32)(int)half;
            }
            else
            {
                inner->aimScreenX = lbl_803E7F44 * (av * (f32)(int)half) + (f32)(int)half;
            }
            *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_AIM_READY;
        }
        if (lbl_803DE42C != 0)
        {
            f32 x;
            Sfx_KeepAliveLoopedObjectSound(obj, SFXTRIG_whit3_c);
            x = inner->stateTimer - timeDelta;
            inner->stateTimer = x;
            if (x <= lbl_803E7EA4)
            {
                int sub = *(int*)((char*)*(int*)&((GameObject*)obj)->extra + 0x35c);
                int v = *(s16*)((char*)sub + 0x4) - 1;
                if (v < 0)
                {
                    v = 0;
                }
                else if (v > *(s16*)((char*)sub + 0x6))
                {
                    v = *(s16*)((char*)sub + 0x6);
                }
                *(s16*)((char*)sub + 0x4) = v;
                inner->stateTimer = lbl_803E7F58;
            }
            ObjPath_GetPointWorldPosition(gPlayerPathObject, 5, &pfx.posX, &pfx.posY, &pfx.posZ, 0);
            pfx.scale = lbl_803E7F9C;
            pfx.arg3 = 0;
            (*gPartfxInterface)->spawnObject((void*)gPlayerPathObject, 0x7f5, &pfx, 0x200001, -1, NULL);
            pfx.arg3 = 1;
            (*gPartfxInterface)->spawnObject((void*)gPlayerPathObject, 0x7f5, &pfx, 0x200001, -1, NULL);
            if ((inner->buttonsHeld & gPlayerHeldButtonMask) == 0 ||
                *(s16*)((char*)*(int*)((char*)*(int*)&((GameObject*)obj)->extra + 0x35c) + 0x4) == 0 ||
                getCurSeqNo() != 0)
            {
                int z[2];
                void** p[1];
                z[1] = lbl_803DE42C = z[0] = 0;
                p[0] = gPlayerSpawnedObjects;
                do
                {
                    if (*p[0] != NULL)
                    {
                        Obj_FreeObject((GameObject*)*p[0]);
                        *p[0] = NULL;
                    }
                    p[0]++;
                    z[1]++;
                } while (z[1] < 7);
                if (gPlayerResource != NULL)
                {
                    Resource_Release(gPlayerResource);
                    gPlayerResource = NULL;
                }
            }
        }
        else if ((inner->buttonsJustPressed & 0x900) != 0)
        {
            int yitem;
            u16 b28;
            s16 item;
            if (inner->buttonsJustPressed & PAD_BUTTON_Y)
            {
            yitem = getYButtonItem(&item);
                b28 = 0x800;
            }
            else
            {
                yitem = 0;
                item = gPlayerSelectedItem;
                b28 = 0x100;
            }
            if ((inner->buttonsJustPressed & PAD_BUTTON_A) != 0 ||
                (yitem == 1 &&
                 (item == GAMEBIT_STAFF_ABILITY_FIRE_BLASTER || item == GAMEBIT_STAFF_ABILITY_FREEZE_BLAST)))
            {
                buttonDisable(0, 0x900);
                inner->buttonsJustPressed = inner->buttonsJustPressed & ~0x900;
                gPlayerSelectedItem = item;
                if (item != inner->animState)
                {
                    playerCastSpell(obj, (int)inner, item);
                }
                switch (gPlayerSelectedItem)
                {
                case GAMEBIT_STAFF_ABILITY_FIRE_BLASTER:
                {
                    int sub = *(int*)((char*)*(int*)&((GameObject*)obj)->extra + 0x35c);
                    if (*(s16*)((char*)sub + 0x4) >= 2)
                    {
                        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029A4A8;
                        return 0x2f;
                    }
                    Sfx_PlayFromObject(0, SFXTRIG_staff_swipes_long);
                    break;
                }
                case 0x958:
                {
                    int sub = *(int*)((char*)*(int*)&((GameObject*)obj)->extra + 0x35c);
                    if (*(s16*)((char*)sub + 0x4) >= 0)
                    {
                        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029A4A8;
                        return 0x30;
                    }
                    Sfx_PlayFromObject(0, SFXTRIG_staff_swipes_long);
                    break;
                }
                case GAMEBIT_STAFF_ABILITY_FREEZE_BLAST:
                {
                    int sub = *(int*)((char*)*(int*)&((GameObject*)obj)->extra + 0x35c);
                    if (*(s16*)((char*)sub + 0x4) >= 1)
                    {
                        int sub2;
                        int v;
                        playerCastIceSpell((GameObject*)obj);
                        gPlayerHeldButtonMask = b28;
                        lbl_803DE42C = 1;
                        lbl_803DE430 = lbl_803E7EA4;
                        inner->stateTimer = lbl_803E7F58;
                        sub2 = *(int*)((char*)*(int*)&((GameObject*)obj)->extra + 0x35c);
                        v = *(s16*)((char*)sub2 + 0x4) - 1;
                        if (v < 0)
                        {
                            v = 0;
                        }
                        else if (v > *(s16*)((char*)sub2 + 0x6))
                        {
                            v = *(s16*)((char*)sub2 + 0x6);
                        }
                        *(s16*)((char*)sub2 + 0x4) = v;
                        break;
                    }
                    Sfx_PlayFromObject(0, SFXTRIG_staff_swipes_long);
                    break;
                }
                }
            }
        }
        inner->targetYaw = lbl_803E7FB4 * spin + (f32)(int)inner->targetYaw;
        {
            s16 targetYaw = inner->targetYaw;
            inner->yaw = targetYaw;
            ((GameObject*)obj)->anim.rotX = targetYaw;
        }
        break;
    }
    default:
        ObjAnim_SetCurrentMove(obj, 0x43e, lbl_803E7EA4, 0);
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F34;
        lbl_803DE42C = 0;
        lbl_803DE430 = lbl_803E7EA4;
        break;
    }
    if ((inner->buttonsJustPressed & PAD_BUTTON_B) != 0 || inner->curAnimId != 0x52)
    {
        *(u32*)&((PlayerState*)inner)->flags360 &= ~0x2000000LL;
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029A420;
        return 0x2c;
    }
    return 0;
}

int playerStateStopAimStaff(int obj, int state, f32 fv)
{
    PlayerState* inner = ((GameObject*)obj)->extra;
    int r = fn_802AC7DC(obj, state, (int)inner, fv);
    if (r != 0)
    {
        return r;
    }
    if (((GameObject*)obj)->anim.currentMove != 0x449)
    {
        u8 c;
        ObjAnim_SetCurrentMove(obj, 0x449, lbl_803E7EA4, 0);
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F4C;
        Sfx_PlayFromObject(obj, SFXTRIG_staff_swipes_short);
        c = inner->curAnimId;
        if (c != 0x42 && c != 0x4c)
        {
            (*gCameraInterface)->setMode(0x42, 0, 1, 0, NULL, 0x3c, 0xfe);
        }
    }
    if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
    {
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
        return -1;
    }
    return 0;
}

int playerStateStartAimStaff(GameObject* obj, int state, f32 fv)
{
    PlayerState* inner = obj->extra;
    int r = fn_802AC7DC((int)obj, state, (int)inner, fv);
    u32 b;
    if (r != 0)
    {
        return r;
    }
    {
        f32 z = lbl_803E7EA4;
        ((PlayerState*)state)->baddie.animSpeedC = z;
        ((PlayerState*)state)->baddie.animSpeedB = z;
        ((PlayerState*)state)->baddie.animSpeedA = z;
        obj->anim.velocityX = z;
        obj->anim.velocityY = z;
        obj->anim.velocityZ = z;
    }
    setAButtonIcon(6);
    setBButtonIcon(0xa);
    switch (obj->anim.currentMove)
    {
    case 0x43d:
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029A4A8;
            return 0x2d;
        }
        break;
    case 0x448:
        if (obj->anim.currentMoveProgress > lbl_803E7E9C)
        {
            if (inner->staffGrown == 0)
            {
                Sfx_PlayFromObject((int)obj, SFXTRIG_wp_swddirt16);
                if (gPlayerPathObject != NULL)
                {
                    b = (((PlayerState*)inner)->flags3F4 >> 6) & 1;
                    if (b != 0)
                    {
                        inner->staffActionRequest = 2;
                        ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 0;
                    }
                }
            }
        }
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029A4A8;
            return 0x2d;
        }
        break;
    default:
    {
        f32 z;
        ObjAnim_SetCurrentMove((int)obj, 0x43d, lbl_803E7EA4, 0);
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F4C;
        if (gPlayerPathObject != NULL)
        {
            b = (((PlayerState*)inner)->flags3F4 >> 6) & 1;
            if (b != 0)
            {
                inner->staffActionRequest = 4;
                ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
            }
        }
        z = lbl_803E7EA4;
        lbl_803DE460 = z;
        lbl_803DE464 = z;
        inner->aimInputZ = z;
        inner->aimInputX = z;
        break;
    }
    }
    if ((inner->buttonsJustPressed & PAD_BUTTON_B) != 0 || inner->curAnimId != 0x52)
    {
        buttonDisable(0, PAD_BUTTON_B);
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029A420;
        return 0x2c;
    }
    return 0;
}

int playerState29(GameObject* obj, int state)
{
    PlayerState* inner = obj->extra;
    u32 b;
    if ((*(int*)&((PlayerState*)state)->baddie.unk31C & 0x100) != 0)
    {
        b = (((PlayerState*)inner)->flags3F4 >> 6) & 1;
        if (b != 0)
        {
            if (gPlayerPathObject != NULL && b != 0)
            {
                inner->staffActionRequest = 4;
                ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
            }
            *(int*)&((PlayerState*)state)->baddie.unk308 = 0;
            return 0x32;
        }
    }
    return 0;
}

int playerState28(GameObject* obj, int state, f32 fv)
{
    PlayerState* inner = obj->extra;
    int v;

    if (((PlayerState*)state)->baddie.hasTarget != 1 && ((PlayerState*)state)->baddie.controlMode != 0x26)
    {
        if (gPlayerPathObject != NULL && ((ByteFlags*)((char*)inner + 0x3f4))->b40)
        {
            inner->staffActionRequest = 0;
            ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 0;
        }
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
        return 2;
    }
    v = fn_802AC7DC((int)obj, state, (int)inner, fv);
    if (v != 0)
    {
        if (gPlayerPathObject != NULL && ((ByteFlags*)((char*)inner + 0x3f4))->b40)
        {
            inner->staffActionRequest = 1;
            ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
        }
        *(int*)&((PlayerState*)state)->baddie.targetObj = 0;
        ((PlayerState*)state)->baddie.hasTarget = 0;
        (*gCameraInterface)->setTarget(0);
        return v;
    }
    if (((PlayerState*)state)->baddie.controlMode == 0x26 || ((ByteFlags*)((char*)inner + 0x3f6))->b20)
    {
        return 0;
    }
    if (((PlayerState*)state)->baddie.controlMode != 0x39)
    {
        if ((padGetTriggers(0) & 0x20) != 0)
        {
            ((ByteFlags*)((char*)inner + 0x3f6))->b20 = 1;
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029782C;
            return 0x3a;
        }
    }
    if (((PlayerState*)state)->baddie.controlMode == 0x39)
    {
        return 0;
    }
    if ((*(int*)&((PlayerState*)state)->baddie.unk31C & 0x100) && gPlayerPathObject != NULL &&
        ((ByteFlags*)((char*)inner + 0x3f4))->b40)
    {
        inner->staffActionRequest = 4;
        ((ByteFlags*)((char*)inner + 0x3f4))->b08 = 1;
    }
    v = playerState30(obj, state, fv);
    if (v != 0)
        return v;
    return 0;
}

void fn_8029BC08(GameObject* obj)
{
    Player_GetObjHitsState(obj)->objectHitMask = 0;
    if (((GameObject*)gPlayerPathObject)->anim.classId == 0x2d)
    {
        objSetAnimField48to0((GameObject*)gPlayerPathObject);
    }
    gPlayerSubState = 1;
}

int playerState27(GameObject* obj, int state, f32 fv)
{
    PlayerState* inner = obj->extra;

    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        if (lbl_803DE459 == 0)
        {
            lbl_803DE459 = 1;
        }
        else if (lbl_803DE459 > 2)
        {
            lbl_803DE459 = 2;
        }
        ((PlayerState*)state)->baddie.moveSpeed = (&lbl_803DC690)[lbl_803DE459 - 1];
        ObjAnim_SetCurrentMove((int)obj, (&lbl_803DC688)[lbl_803DE459 - 1], lbl_803E7EA4, 0);
        lbl_803DE459 = 0;
    }
    if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
    {
        Player_GetObjHitsState(obj)->suppressOutgoingHits = 0;
        if (((PlayerState*)state)->baddie.targetObj != NULL)
        {
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029C8C8;
            return 0x25;
        }
        ((ByteFlags*)((char*)inner + 0x3f1))->b80 = 1;
        *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
        return 2;
    }
    (*gPlayerInterface)->updateAnimRootMotion(obj, (void*)state, fv, 1);
    return 0;
}

int playerStateAttack(GameObject* obj, int state, f32 fv)
{
    int r;
    u8 changed;
    int path;
    PlayerState* inner = obj->extra;
    f32 amt;

    r = playerState28(obj, state, fv);
    if (r != 0)
    {
        return r;
    }
    path = (int)gPlayerPathObject;
    *(s8*)&((PlayerState*)state)->baddie.stateTag = 1;
    gPlayerSubState = 5;
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA == 0)
    {
        if (lbl_803DE459 != 0)
        {
            doRumble(10.0f);
            *(int*)&((PlayerState*)state)->baddie.unk308 = 0;
            return 0x28;
        }
        changed = 0;
        if (((PlayerState*)state)->baddie.moveSpeed > 0.0f)
        {
            if ((*(int*)&((PlayerState*)state)->baddie.eventFlags & 0x200) != 0)
            {
                doRumble(5.0f);
                Sfx_PlayFromObject((int)obj, SFXTRIG_rserv1_c);
                inner->pendingFxFlags = inner->pendingFxFlags | 4;
            }
            if ((*(int*)&((PlayerState*)state)->baddie.eventFlags & 0x400) != 0)
            {
                doRumble(5.0f);
                Sfx_PlayFromObject((int)obj, SFXTRIG_rserv1_c);
                inner->pendingFxFlags = inner->pendingFxFlags | 4;
            }
            if ((((PlayerState*)state)->baddie.moveEventFlags & 1) == 0 &&
                obj->anim.currentMoveProgress >
                    *(f32*)((inner->moveSlots + 0x50) + (u32)inner->moveSlotIndex * 0xb0))
            {
                u16 sfx;
                if (inner->characterId == 0)
                {
                    sfx = 0x2de;
                }
                else
                {
                    sfx = 0x1c;
                }
                Sfx_PlayFromObject((int)obj, sfx);
                ((PlayerState*)state)->baddie.moveEventFlags = ((PlayerState*)state)->baddie.moveEventFlags | 1;
            }
            if ((((PlayerState*)state)->baddie.moveEventFlags & 2) == 0 &&
                obj->anim.currentMoveProgress >
                    *(f32*)((inner->moveSlots + 0x54) + (u32)inner->moveSlotIndex * 0xb0))
            {
                Sfx_PlayFromObject((int)obj, SFXTRIG_sswsh);
                ((PlayerState*)state)->baddie.moveEventFlags = ((PlayerState*)state)->baddie.moveEventFlags | 2;
            }
        }
        {
            int slot = inner->moveSlots + (u32)inner->moveSlotIndex * 0xb0;
            if (*(s8*)(slot + 0x15) >= 0)
            {
                if (obj->anim.currentMoveProgress > *(f32*)(slot + 0x28))
                {
                    *(u8*)((char*)state + 0x34a) = *(u8*)((char*)state + 0x34a) | 2;
                    if (*(u8*)((inner->moveSlots + 0x6c) + (u32)inner->moveSlotIndex * 0xb0) != 0u)
                    {
                        *(u8*)((char*)state + 0x34a) = *(u8*)((char*)state + 0x34a) | 4;
                        inner->moveChainIndex = 0;
                    }
                }
                if (obj->anim.currentMoveProgress >
                    *(f32*)((inner->moveSlots + 0x20) + (u32)inner->moveSlotIndex * 0xb0))
                {
                    *(u8*)((char*)state + 0x34a) = *(u8*)((char*)state + 0x34a) | 1;
                }
                if (obj->anim.currentMoveProgress >
                    *(f32*)((inner->moveSlots + 0x24) + (u32)inner->moveSlotIndex * 0xb0))
                {
                    *(u8*)((char*)state + 0x34a) = *(u8*)((char*)state + 0x34a) & ~1;
                }
                if ((*(int*)&((PlayerState*)state)->baddie.unk31C & 0x100) != 0 &&
                    (*(u8*)((char*)state + 0x34a) & 1) != 0)
                {
                    *(u8*)((char*)state + 0x34a) = *(u8*)((char*)state + 0x34a) | 4;
                    *(int*)&((PlayerState*)state)->baddie.unk31C =
                        *(int*)&((PlayerState*)state)->baddie.unk31C & ~0x100;
                    buttonDisable(0, PAD_BUTTON_A);
                    inner->moveChainIndex = *(u8*)((char*)state + 0x34b);
                }
                if ((*(u8*)((char*)state + 0x34a) & 4) != 0 && (*(u8*)((char*)state + 0x34a) & 2) != 0)
                {
                    f32 v = (f32)(u8)fn_8014C4D8((GameObject*)((PlayerState*)state)->baddie.targetObj);
                    int slot2 = inner->moveSlots + (u32)inner->moveSlotIndex * 0xb0;
                    if (v >= *(f32*)(slot2 + 0x8c))
                    {
                        inner->moveSlotIndex = *(u8*)((slot2 + 0x15) + (u32)inner->moveChainIndex);
                    }
                    else
                    {
                        inner->moveSlotIndex = *(u8*)(slot2 + 0x90);
                    }
                    changed = 1;
                }
            }
        }
    }
    else
    {
        lbl_803DE459 = 0;
        changed = 1;
        *(u32*)&inner->flags360 &= 0xFFFFFFFBF;
        Player_GetObjHitsState(obj)->suppressOutgoingHits = 0;
        {
            f32 z = 0.0f;
            inner->hitTimer = z;
            inner->hitCount = 0;
            inner->lastHitObject = 0;
            inner->activeHitWindow = -1;
            ((PlayerState*)state)->baddie.animSpeedC = z;
            ((PlayerState*)state)->baddie.animSpeedB = z;
            ((PlayerState*)state)->baddie.animSpeedA = z;
            obj->anim.velocityX = z;
            obj->anim.velocityY = z;
            obj->anim.velocityZ = z;
        }
    }
    if (((PlayerState*)state)->baddie.targetObj != NULL)
    {
        if (inner->moveSlotIndex >= 5 && inner->moveSlotIndex <= 9)
        {
            amt = (f32)inner->targetObjectBearing;
        }
        else
        {
            amt = (f32)inner->targetObjectBearing / 12.0f;
        }
        inner->targetYaw = (f32)(int)inner->targetYaw + amt;
        inner->yaw = inner->targetYaw;
    }
    else if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0 && inner->cameraTargetObject != NULL &&
             inner->targetObjModelType == 1)
    {
        if (inner->targetObjectBearingAbs < 0x4000)
        {
            amt = (f32)inner->targetObjectBearing;
        }
        inner->targetYaw = (f32)(int)inner->targetYaw + amt;
        inner->yaw = inner->targetYaw;
    }
    else if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        s16 inputHeading = inner->inputHeading;
        inner->targetYaw = inputHeading;
        inner->yaw = inputHeading;
    }
    if (changed != 0)
    {
        *(int*)&obj->anim.weaponDaTable = (inner->moveSlots + (u32)inner->moveSlotIndex * 0xb0) + 0x60;
        if (obj->anim.currentMove !=
            gPlayerMoveSlotTable[*(s16*)((inner->moveSlots + (u32)inner->moveSlotIndex * 0xb0) + 0x2)])
        {
            ObjAnim_SetCurrentMove(
                (int)obj, gPlayerMoveSlotTable[*(s16*)((inner->moveSlots + (u32)inner->moveSlotIndex * 0xb0) + 0x2)],
                *(f32*)((inner->moveSlots + (u32)inner->moveSlotIndex * 0xb0) + 0x68), 0);
            ObjAnim_SetCurrentEventStepFrames((ObjAnimComponent*)obj, 2);
        }
        *(u8*)((char*)state + 0x34a) = *(u8*)((char*)state + 0x34a) & ~0xef;
        ((PlayerState*)state)->baddie.moveSpeed = *(f32*)((inner->moveSlots + 0x1c) + (u32)inner->moveSlotIndex * 0xb0);
        inner->unk824 = ((PlayerState*)state)->baddie.moveSpeed;
        inner->cutsceneEnded = 0;
        ((PlayerState*)state)->baddie.animSpeedB = 0.0f;
        ((PlayerState*)state)->baddie.moveEventFlags = 0;
        if (((PlayerState*)state)->baddie.targetObj != NULL)
        {
            if (inner->moveSlotIndex >= 5 && inner->moveSlotIndex <= 9)
            {
                (*gPlayerInterface)->rotateTowardTarget(obj, (void*)state, fv, 1);
            }
            else
            {
                (*gPlayerInterface)->rotateTowardTarget(obj, (void*)state, fv, 2);
            }
            {
                s16 v = obj->anim.rotX;
                inner->yaw = v;
                inner->targetYaw = v;
            }
        }
        if (obj->anim.hitReactState != NULL)
        {
            Player_GetObjHitsState(obj)->suppressOutgoingHits = 0;
        }
        inner->activeHitWindow = -1;
        if (*(s16*)((char*)path + 0x44) == 0x2d)
        {
            objSetAnimField48to0((GameObject*)path);
            (*(void (*)(int, int)) * (int*)(*(int*)(*(int*)((char*)path + 0x68)) + 0x38))(
                path, *(u8*)((inner->moveSlots + 0x5c) + (u32)inner->moveSlotIndex * 0xb0));
            (*(void (*)(int, f32, f32)) * (int*)(*(int*)(*(int*)((char*)path + 0x68)) + 0x4c))(
                path, *(f32*)((inner->moveSlots + 0x48) + (u32)inner->moveSlotIndex * 0xb0),
                *(f32*)((inner->moveSlots + 0x4c) + (u32)inner->moveSlotIndex * 0xb0));
        }
        {
            f32 z = 0.0f;
            inner->boulderChargeLevel = z;
            inner->hitTimer = z;
            inner->hitCount = 0;
            inner->lastHitObject = 0;
        }
    }
    Player_GetObjHitsState(obj)->hitVolumePriority = 0xb;
    *(u8*)&Player_GetObjHitsState(obj)->hitVolumeId =
        *(u8*)((inner->moveSlots + 0x14) + (u32)inner->moveSlotIndex * 0xb0);
    {
        int slot = inner->moveSlots + (u32)inner->moveSlotIndex * 0xb0;
        f32 t = *(f32*)(slot + 0xa0);
        if (t >= 0.0f)
        {
            if (obj->anim.currentMoveProgress > t &&
                obj->anim.currentMoveProgress < *(f32*)(slot + 0xa4))
            {
                if (0.0f == inner->boulderChargeLevel)
                {
                    Sfx_PlayFromObject((int)obj, SFXTRIG_staff_boulder_drops);
                }
                inner->boulderChargeLevel = 2.0f * timeDelta + inner->boulderChargeLevel;
                if (inner->boulderChargeLevel > 60.0f)
                {
                    inner->boulderChargeLevel = 60.0f;
                }
            }
            else
            {
                inner->boulderChargeLevel = 0.0f;
            }
        }
    }
    if ((*(u8*)((inner->moveSlots + 0x88) + (u32)inner->moveSlotIndex * 0xb0) & 2) != 0 &&
        *(void**)&inner->lastHitObject != NULL)
    {
        if (inner->hitCount < inner->hitCountMax)
        {
            f32 t = inner->hitTimer - 1.0f;
            inner->hitTimer = t;
            if (t <= 0.0f)
            {
                ObjHits_RecordObjectHit((GameObject*)inner->lastHitObject, obj, 0xb, 1, 0);
                (*(u8*)&((PlayerState*)inner)->hitCount)++;
                inner->hitTimer = (f32)(u8)inner->hitInterval;
            }
        }
        else
        {
            inner->lastHitObject = 0;
        }
    }
    {
        int i;
        Player_GetObjHitsState(obj)->objectHitMask = 0;
        for (i = 0; i != 3; i++)
        {
            if (obj->anim.currentMoveProgress >=
                    *(f32*)((inner->moveSlots + ((u32)inner->moveSlotIndex * 0xb0 + i * 4)) + 0x30) &&
                obj->anim.currentMoveProgress <=
                    *(f32*)((inner->moveSlots + ((u32)inner->moveSlotIndex * 0xb0 + i * 4)) + 0x3c))
            {
                if ((s8)Player_GetObjHitsState(obj)->suppressOutgoingHits == 0)
                {
                    int bits;
                    switch (*(s8*)((char*)(inner->moveSlots + (u32)inner->moveSlotIndex * 0xb0 + 0x5d) + i))
                    {
                    case -1:
                        bits = 0;
                        break;
                    case 0:
                        bits = 0xc;
                        break;
                    case 1:
                        bits = 3;
                        break;
                    case 4:
                        bits = 0xf;
                        break;
                    case 2:
                        bits = 0x100000;
                        break;
                    case 3:
                        bits = 0x10000;
                        break;
                    default:
                        bits = 0;
                        break;
                    }
                    Player_GetObjHitsState(obj)->objectHitMask = bits;
                }
                if (i != inner->activeHitWindow)
                {
                    Player_GetObjHitsState(obj)->suppressOutgoingHits = 0;
                    inner->activeHitWindow = (s8)i;
                    inner->hitCount = 0;
                    inner->hitTimer = 0.0f;
                    inner->lastHitObject = 0;
                }
                break;
            }
        }
    }
    (*gPlayerInterface)->updateAnimRootMotion(obj, (void*)state, fv, 3);
    if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
    {
        Player_GetObjHitsState(obj)->suppressOutgoingHits = 0;
        if (((PlayerState*)state)->baddie.targetObj != NULL)
        {
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029C8C8;
            return 0x25;
        }
        ((struct {
             u8 hi : 1;
             u8 lo : 7;
         }*)&inner->flags3F1)
            ->hi = 1;
        *(u32*)&inner->flags360 |= PLAYER_FLAG_TELEPORTED;
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
        return 2;
    }
    if (obj->anim.currentMoveProgress >=
        *(f32*)((inner->moveSlots + 0x2c) + (u32)inner->moveSlotIndex * 0xb0))
    {
        if (((PlayerState*)state)->baddie.targetObj != NULL)
        {
            if ((*(int*)&((PlayerState*)state)->baddie.unk31C & 0x100) != 0)
            {
                Player_GetObjHitsState(obj)->suppressOutgoingHits = 0;
                inner->activeHitWindow = -1;
                (*gPlayerInterface)->rotateTowardTarget(obj, (void*)state, fv, 2);
                {
                    s16 v = obj->anim.rotX;
                    inner->yaw = v;
                    inner->targetYaw = v;
                }
                *(int*)&((PlayerState*)state)->baddie.unk308 = 0;
                return 0x31;
            }
        }
        else if ((*(int*)&((PlayerState*)state)->baddie.unk31C & 0x100) != 0 &&
                 ((PlayerState*)state)->baddie.inputMagnitude > 0.3f)
        {
            inner->targetYaw = inner->targetYaw + inner->targetYawRate * 0xb6;
            inner->yaw = inner->targetYaw;
            inner->targetYawRateSigned = 0;
            inner->targetYawRate = 0;
            inner->yawRateSigned = 0;
            inner->yawRate = 0;
            *(int*)&((PlayerState*)state)->baddie.unk308 = 0;
            return 0x32;
        }
    }
    return 0;
}

void fn_8029C8C8(GameObject* obj, int p2)
{
    PlayerState* inner = obj->extra;
    if (((PlayerState*)p2)->baddie.inputMagnitude < lbl_803E7F6C)
    {
        s16 h = obj->anim.rotX;
        inner->yaw = h;
        inner->targetYaw = h;
        inner->lastInputHeading = h;
        ((PlayerState*)p2)->baddie.inputMagnitude = lbl_803E7EA4;
    }
    else
    {
        int t = inner->inputHeading;
        inner->lastInputHeading = t;
        inner->yaw = (s16)t;
        inner->yawRate = 0;
        inner->yawRateSigned = 0;
    }
    gPlayerSubState = 1;
    if (((PlayerState*)p2)->baddie.controlMode != 0x24 && ((PlayerState*)p2)->baddie.controlMode != 0x25 &&
        lbl_803DE42C != 0)
    {
        int z[2];
        inner->animState = -1;
        z[0] = 0;
        lbl_803DE42C = z[0];
        for (z[1] = z[0]; z[1] < 7; z[1]++)
        {
            if (gPlayerSpawnedObjects[z[1]] != NULL)
            {
                Obj_FreeObject((GameObject*)gPlayerSpawnedObjects[z[1]]);
                gPlayerSpawnedObjects[z[1]] = NULL;
            }
        }
        if (gPlayerResource != NULL)
        {
            Resource_Release(gPlayerResource);
            gPlayerResource = NULL;
        }
    }
}

int playerState25(int obj, int state, f32 fv)
{
    PlayerState* inner = ((GameObject*)obj)->extra;
    f32 ratio, c, s, vx, t0, curveOut;
    f32 vy;
    int r;

    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        gPlayerSubState = 5;
    }
    r = playerState28((GameObject*)obj, state, fv);
    if (r != 0)
    {
        return r;
    }
    {
        f32 x = (((PlayerState*)state)->baddie.inputMagnitude - lbl_803E7F14) / lbl_803E7F2C;
        ratio = (x < lbl_803E7EA4) ? lbl_803E7EA4 : ((x > lbl_803E7EE0) ? lbl_803E7EE0 : x);
    }
    {
        f32 ang = gPlayerPi * (f32)(int)inner->inputHeading / lbl_803E7F98;
        vx = ratio * -mathSinf(ang);
        vx = inner->maxSpeed * vx;
    }
    {
        f32 ang = gPlayerPi * (f32)(int)inner->inputHeading / lbl_803E7F98;
        vy = inner->maxSpeed * (ratio * -mathCosf(ang));
    }
    {
        f32 a = interpolate(vx - inner->smoothVelX, lbl_803E7F44, timeDelta);
        f32 b = interpolate(vy - inner->smoothVelZ, lbl_803E7F44, timeDelta);
        inner->smoothVelX += a;
        inner->smoothVelZ += b;
    }
    ((PlayerState*)state)->baddie.animSpeedC =
        sqrtf(inner->smoothVelX * inner->smoothVelX + inner->smoothVelZ * inner->smoothVelZ);
    {
        f32 v = ((PlayerState*)state)->baddie.animSpeedC;
        f32 lo = *(f32*)inner->moveParams;
        ((PlayerState*)state)->baddie.animSpeedC =
            (((PlayerState*)state)->baddie.animSpeedC < lo)
                ? lo
                : ((v > inner->maxSpeed) ? inner->maxSpeed : ((PlayerState*)state)->baddie.animSpeedC);
    }
    {
        f32 ang = gPlayerPi * (f32)inner->targetYaw / lbl_803E7F98;
        c = mathSinf(ang);
    }
    {
        f32 ang = gPlayerPi * (f32)inner->targetYaw / lbl_803E7F98;
        s = mathCosf(ang);
    }
    {
        f32 cc = inner->smoothVelZ;
        f32 c8 = inner->smoothVelX;
        ((PlayerState*)state)->baddie.animSpeedA +=
            interpolate(-cc * s - c8 * c - ((PlayerState*)state)->baddie.animSpeedA, inner->targetAnimSpeed, timeDelta);
        ((PlayerState*)state)->baddie.animSpeedB +=
            interpolate(c8 * s - cc * c - ((PlayerState*)state)->baddie.animSpeedB, inner->targetAnimSpeed, timeDelta);
    }
    t0 = ((GameObject*)obj)->anim.currentMoveProgress;
    {
        u8 phase = *(u8*)&((PlayerState*)inner)->gaitLevel;
        int idx = (u8)((s8)phase >> 1);
        if (((PlayerState*)state)->baddie.animSpeedC < gPlayerAnimSpeedThresholds.gaitSpeedThresholds[idx])
        {
            if ((s8)phase == 4)
            {
                if (((PlayerState*)state)->baddie.inputMagnitude < lbl_803E7F14)
                {
                    *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029C8C8;
                    return 0x25;
                }
            }
            else
            {
                *(u8*)&((PlayerState*)inner)->gaitLevel -= 4;
            }
        }
        else
        {
            if (((PlayerState*)state)->baddie.animSpeedC >= gPlayerAnimSpeedThresholds.gaitSpeedThresholds[idx + 1] && (s8)phase < 8)
            {
                if ((s8)phase == 0)
                {
                    t0 = lbl_803E7EA4;
                }
                if (((PlayerState*)state)->baddie.animSpeedC < inner->maxSpeed)
                {
                    *(u8*)&((PlayerState*)inner)->gaitLevel += 4;
                }
            }
        }
    }
    {
        f32 ax;
        f32 az = ((PlayerState*)state)->baddie.animSpeedB;
        if (az < lbl_803E7EA4)
        {
            az = -az;
        }
        ax = ((PlayerState*)state)->baddie.animSpeedA;
        if (ax < *(f32*)&lbl_803E7EA4)
        {
            ax = -ax;
        }
        if (ObjAnim_SampleRootCurvePhase((ObjAnimComponent*)obj, ((PlayerState*)state)->baddie.animSpeedC,
                                         &curveOut) != 0)
        {
            ((PlayerState*)state)->baddie.moveSpeed = curveOut;
        }
        if (ax > az)
        {
            if (((PlayerState*)state)->baddie.animSpeedA < lbl_803E7EA4)
            {
                ((PlayerState*)state)->baddie.moveSpeed = -((PlayerState*)state)->baddie.moveSpeed;
            }
            if (((GameObject*)obj)->anim.currentMove != gPlayerMoveTableB[inner->gaitLevel])
            {
                if (ObjAnim_GetCurrentEventCountdown((ObjAnimComponent*)obj) == 0)
                {
                    ObjAnim_SetCurrentMove(obj, gPlayerMoveTableB[inner->gaitLevel], t0, 0);
                    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA == 0)
                    {
                        ObjAnim_SetCurrentEventStepFrames((ObjAnimComponent*)obj, 0xc);
                    }
                }
            }
        }
        else
        {
            if (((PlayerState*)state)->baddie.animSpeedB >= lbl_803E7EA4)
            {
                ((PlayerState*)state)->baddie.moveSpeed = -((PlayerState*)state)->baddie.moveSpeed;
            }
            if (((GameObject*)obj)->anim.currentMove != (gPlayerMoveTableB + 2)[inner->gaitLevel])
            {
                if (ObjAnim_GetCurrentEventCountdown((ObjAnimComponent*)obj) == 0)
                {
                    ObjAnim_SetCurrentMove(obj, (gPlayerMoveTableB + 2)[inner->gaitLevel], t0, 0);
                    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA == 0)
                    {
                        ObjAnim_SetCurrentEventStepFrames((ObjAnimComponent*)obj, 0xc);
                    }
                }
            }
        }
    }
    inner->targetYaw = (s16)(inner->targetYaw + (int)((f32)(int)inner->targetObjectBearing / lbl_803E7FC0));
    inner->yaw = inner->targetYaw;
    *(u32*)&((PlayerState*)inner)->flags360 |= 0x2000000LL;
    fn_802ABFBC((GameObject*)obj, state, inner);
    return 0;
}

int playerState24(GameObject* obj, int state, f32 fv)
{
    PlayerState* inner = obj->extra;
    f32 t, ang, vx, vy, dx, dy;
    f32 zero = lbl_803E7EA4;
    int r;

    ((PlayerState*)state)->baddie.animSpeedA = zero;
    ((PlayerState*)state)->baddie.animSpeedB = zero;
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        inner->maxSpeed = lbl_803E7FC4;
        *(u8*)&((PlayerState*)inner)->gaitLevel = 0;
        inner->smoothVelX = zero;
        inner->smoothVelZ = zero;
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F84;
        ((PlayerState*)state)->baddie.animSpeedC = zero;
        gPlayerSubState = 5;
    }

    r = playerState28(obj, state, fv);
    if (r != 0)
    {
        return r;
    }

    t = (((PlayerState*)state)->baddie.inputMagnitude - lbl_803E7F14) / lbl_803E7F2C;
    ang = (t < lbl_803E7EA4) ? lbl_803E7EA4 : ((t > lbl_803E7EE0) ? lbl_803E7EE0 : t);
    vx = inner->maxSpeed * (ang * -mathSinf(gPlayerPi * (f32)inner->inputHeading / lbl_803E7F98));
    vy = inner->maxSpeed * (ang * -mathCosf(gPlayerPi * (f32)inner->inputHeading / lbl_803E7F98));
    dx = interpolate(vx - inner->smoothVelX, lbl_803E7F44, timeDelta);
    dy = interpolate(vy - inner->smoothVelZ, lbl_803E7F44, timeDelta);
    inner->smoothVelX += dx;
    inner->smoothVelZ += dy;
    ((PlayerState*)state)->baddie.animSpeedC =
        sqrtf(inner->smoothVelX * inner->smoothVelX + inner->smoothVelZ * inner->smoothVelZ);
    ((PlayerState*)state)->baddie.animSpeedC =
        (((PlayerState*)state)->baddie.animSpeedC < lbl_803E7EA4)
            ? lbl_803E7EA4
            : ((((PlayerState*)state)->baddie.animSpeedC > inner->maxSpeed) ? inner->maxSpeed
                                                                            : ((PlayerState*)state)->baddie.animSpeedC);

    if (*(f32*)&((PlayerState*)state)->baddie.trackedObj >= lbl_803E7FC8 &&
        ((PlayerState*)state)->baddie.inputMagnitude >= lbl_803E7FC8 &&
        ((PlayerState*)state)->baddie.animSpeedC >= gPlayerAnimSpeedThresholds.gaitSpeedThresholds[1])
    {
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_8029C8C8;
        return 0x26;
    }

    if (obj->anim.currentMove != 0x8c)
    {
        ObjAnim_SetCurrentMove((int)obj, 0x8c, lbl_803E7EA4, 0);
        if (((PlayerState*)state)->baddie.prevControlMode == 0x39)
        {
            ObjAnim_SetCurrentEventStepFrames((ObjAnimComponent*)obj, 8);
        }
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F84;
    }

    inner->targetYaw += (int)((f32)inner->targetObjectBearing / lbl_803E7FC0);
    inner->yaw = inner->targetYaw;
    *(u32*)&((PlayerState*)inner)->flags360 |= 0x2000000LL;
    fn_802ABFBC(obj, state, inner);
    return 0;
}

int playerState23(GameObject* obj, int state, f32 fv)
{
    MoveTable* mt = (MoveTable*)lbl_80332EC0;
    PlayerState* inner = obj->extra;
    u32 flags;
    int idx;

    ((PlayerState*)state)->baddie.stateTag = 3;
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        if (((PlayerState*)state)->baddie.targetObj != NULL && (inner->flags884 & 1))
        {
            doRumble(lbl_803E7ED8);
            flags = inner->flags884;
            if (flags & 2)
            {
                idx = 3;
            }
            else if (flags & 4)
            {
                idx = 1;
            }
            else if (flags & 8)
            {
                idx = 2;
            }
            else
            {
                idx = 3;
            }
            ObjAnim_SetCurrentMove((int)obj, mt->moves[idx], mt->blend[idx], 0);
            ((PlayerState*)state)->baddie.moveSpeed = mt->angles[idx];
            ((PlayerState*)state)->baddie.animSpeedA = -inner->animSpeedStart;
        }
        else
        {
            ObjAnim_SetCurrentMove((int)obj, mt->moves[inner->moveVariantIndex], lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = mt->angles[inner->moveVariantIndex];
        }
    }
    if (((PlayerState*)state)->baddie.targetObj != NULL)
    {
        inner->targetYaw = inner->targetYaw + (int)((f32)inner->targetObjectBearing / lbl_803E7FC0);
        inner->yaw = inner->targetYaw;
    }
    ((PlayerState*)state)->baddie.animSpeedA =
        ((PlayerState*)state)->baddie.animSpeedA * powfBitEstimate(inner->animSpeedDecay, fv);
    (*gPlayerInterface)->updateAnimRootMotion(obj, (void*)state, fv, 2);
    if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
    {
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
        return 2;
    }
    return 0;
}

int playerState22(GameObject* obj, int state)
{
    PlayerState* inner = obj->extra;
    ((PlayerState*)state)->baddie.stateTag = 3;
    if (*(s8*)((char*)inner->playerStatus) > 0)
    {
        ObjAnim_SetCurrentMove((int)obj, 0xc8, lbl_803E7EA4, 0);
        *(int*)&((PlayerState*)state)->baddie.unk308 = 0;
        return -0x21;
    }
    return 0;
}

int playerState21(int obj, int state, f32 fv)
{
    PlayerState* inner = ((GameObject*)obj)->extra;
    u16 sfxId;
    int d;

    switch (((GameObject*)obj)->anim.currentMove)
    {
    case 0x450:
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7FCC;
        if (((GameObject*)obj)->anim.velocityY < lbl_803E7EE0 && ((ByteFlags*)((char*)inner + 0x3f1))->b01)
        {
            if (inner->characterId == 0)
            {
                sfxId = 0x2d2;
            }
            else
            {
                sfxId = 0x214;
            }
            Sfx_PlayFromObject(obj, sfxId);
            ObjAnim_SetCurrentMove(obj, 0xc6, lbl_803E7EA4, 0);
        }
        if (((GameObject*)obj)->anim.velocityX * ((GameObject*)obj)->anim.velocityX +
                ((GameObject*)obj)->anim.velocityZ * ((GameObject*)obj)->anim.velocityZ >
            lbl_803E7EE0)
        {
            d = getAngle(((GameObject*)obj)->anim.velocityX, ((GameObject*)obj)->anim.velocityZ) & 0xffff;
            d -= (u16) ((PlayerState*)inner)->targetYaw;
            if (d > 0x8000)
            {
                d -= 0xffff;
            }
            if (d < -0x8000)
            {
                d += 0xffff;
            }
            ((PlayerState*)inner)->targetYaw += (d * (int)fv >> 3);
            inner->yaw = ((PlayerState*)inner)->targetYaw;
        }
        break;
    case 0xc4:
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F6C;
        if (((GameObject*)obj)->anim.velocityY < lbl_803E7EE0 && ((ByteFlags*)((char*)inner + 0x3f1))->b01)
        {
            if (inner->characterId == 0)
            {
                sfxId = 0x2d2;
            }
            else
            {
                sfxId = 0x214;
            }
            Sfx_PlayFromObject(obj, sfxId);
            ObjAnim_SetCurrentMove(obj, 0xc6, lbl_803E7EA4, 0);
        }
        if (((GameObject*)obj)->anim.velocityX * ((GameObject*)obj)->anim.velocityX +
                ((GameObject*)obj)->anim.velocityZ * ((GameObject*)obj)->anim.velocityZ >
            lbl_803E7EE0)
        {
            d = getAngle(((GameObject*)obj)->anim.velocityX, ((GameObject*)obj)->anim.velocityZ) & 0xffff;
            d -= (u16) ((PlayerState*)inner)->targetYaw;
            if (d > 0x8000)
            {
                d -= 0xffff;
            }
            if (d < -0x8000)
            {
                d += 0xffff;
            }
            ((PlayerState*)inner)->targetYaw += (d * (int)fv >> 3);
            inner->yaw = ((PlayerState*)inner)->targetYaw;
        }
        break;
    case 0xc6:
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F6C;
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            ObjAnim_SetCurrentMove(obj, 0xc8, lbl_803E7EA4, 0);
        }
        ((GameObject*)obj)->anim.velocityZ = ((GameObject*)obj)->anim.velocityX = lbl_803E7EA4;
        break;
    case 0xc8:
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7EF8;
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            *(u32*)&((PlayerState*)inner)->flags360 |= PLAYER_FLAG_TELEPORTED;
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
            return -1;
        }
        break;
    default:
        ObjAnim_SetCurrentMove(obj, 0xc4, lbl_803E7EA4, 0);
        break;
    }
    *(s8*)((char*)state + 0x34c) |= 2;
    ((GameObject*)obj)->anim.velocityX = ((GameObject*)obj)->anim.velocityX * powfBitEstimate(lbl_803E7FD0, fv);
    ((GameObject*)obj)->anim.velocityZ = ((GameObject*)obj)->anim.velocityZ * powfBitEstimate(lbl_803E7FD0, fv);
    return 0;
}

int playerState20(GameObject* obj, int state, f32 fv)
{
    ((PlayerState*)state)->baddie.stateTag = 3;
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        ObjAnim_SetCurrentMove((int)obj, 0x44c, lbl_803E7EA4, 0);
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7FD4;
    }
    switch (obj->anim.currentMove)
    {
    case 0x44c:
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            ObjAnim_SetCurrentMove((int)obj, 0x44d, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7FCC;
        }
        break;
    case 0x44d:
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
            return 2;
        }
        break;
    }
    (*gPlayerInterface)->updateAnimRootMotion(obj, (void*)state, fv, 1);
    return 0;
}

int playerState1F(GameObject* obj, int state, f32 fv)
{
    PlayerState* inner = obj->extra;
    int hit;

    ((PlayerState*)state)->baddie.stateTag = 3;
    if (*(s8*)&((PlayerState*)state)->baddie.moveJustStartedA != 0)
    {
        if (ObjHits_GetPriorityHit(obj, &hit, 0, 0))
        {
            inner->targetYaw = (s16)getAngle(-*(f32*)((char*)hit + 0x24), -*(f32*)((char*)hit + 0x2c));
            inner->yaw = inner->targetYaw;
        }
        ObjAnim_SetCurrentMove((int)obj, 0x407, lbl_803E7EA4, 0);
        ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7F34;
    }
    switch (obj->anim.currentMove)
    {
    case 0x407:
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            ObjAnim_SetCurrentMove((int)obj, 0x408, lbl_803E7EA4, 0);
            ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7FCC;
        }
        break;
    case 0x408:
        if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
        {
            *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
            return 2;
        }
        break;
    }
    (*gPlayerInterface)->updateAnimRootMotion(obj, (void*)state, fv, 1);
    return 0;
}

int playerState1E(int obj, int state, f32 fv)
{
    ((PlayerState*)state)->baddie.stateTag = 3;
    ((PlayerState*)state)->baddie.moveSpeed = lbl_803E7FD8;
    ((PlayerState*)state)->baddie.animSpeedA = lbl_803E7EA4;
    (*gPlayerInterface)->updateAnimRootMotion((void*)obj, (void*)state, fv, 2);
    if (*(s8*)&((PlayerState*)state)->baddie.moveDone != 0)
    {
        *(int*)&((PlayerState*)state)->baddie.unk308 = (int)fn_802A514C;
        return 2;
    }
    return 0;
}

void fn_8029DAE0(GameObject* obj, int* p2)
{
    PlayerState* inner = obj->extra;
    u8 c;
    *p2 &= ~0x4000;
    c = inner->curAnimId;
    if (c != 0x48 && c != 0x47 && getCurSeqNo() == 0)
    {
        (*gCameraInterface)->setMode(0x42, 0, 1, 0, NULL, 0x3c, 0xfe);
    }
    ObjHits_SyncObjectPositionIfDirty(obj);
}

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


PlayerModelChainEntry lbl_803DC660 = {lbl_80332EC0, 5};
PlayerModelChainEntry* gPlayerModelChainConfig = &lbl_803DC660;

PlayerMoveSlot gPlayerMoveSlotData[28] = {
    {
        0, 0, 0, 0, 210,
        {20, 20, 20},
        1, {10, 10, 10, 10, 10}, {0, 0},
        0.025f, 0.0f, 0.5f, 0.5f, 0.9f,
        {0.32f, -1.0f, -1.0f},
        {0.42f, -1.0f, -1.0f},
        0.0f, 1.0f, 0.2f, 0.35f, 0.0f,
        0, {0, 0, 0},
        {0, NULL},
        0.0f, 0, {0, 0, 0},
        {1.0f, 0.0f, 0.0f},
        {0.9f, 0.0f, 0.0f},
        0, {0, 0, 0}, 0.0f, 10, {0, 0, 0},
        {0.1f, 0.1f, 0.0f},
        -1.0f, 0.0f,
        {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        1, 0, 16, 0, 210,
        {4, 4, 4},
        1, {18, 18, 18, 18, 18}, {0, 0},
        0.025f, 0.0f, 0.5f, 0.5f, 0.9f,
        {0.32f, -1.0f, -1.0f},
        {0.42f, -1.0f, -1.0f},
        0.0f, 1.0f, 0.2f, 0.35f, 0.0f,
        0, {1, 1, 1},
        {0, NULL},
        0.0f, 0, {0, 0, 0},
        {0.8f, 0.0f, 0.0f},
        {0.9f, 0.0f, 0.0f},
        0, {0, 0, 0}, 0.0f, 0, {0, 0, 0},
        {0.1f, 0.1f, 0.0f},
        -1.0f, 0.0f,
        {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        2, 0, 27, 0, 210,
        {0, 0, 0},
        1, {21, 21, 21, 21, 21}, {0, 0},
        0.025f, 0.0f, 0.6f, 0.6f, 0.9f,
        {0.42f, -1.0f, -1.0f},
        {0.52f, -1.0f, -1.0f},
        0.0f, 1.0f, 0.2f, 0.35f, 0.0f,
        0, {1, 0, 0},
        {0, NULL},
        0.0f, 0, {0, 0, 0},
        {1.0f, 0.0f, 0.0f},
        {0.9f, 0.0f, 0.0f},
        0, {0, 0, 0}, 0.0f, 0, {0, 0, 0},
        {0.1f, 0.0f, 0.0f},
        -1.0f, 0.0f,
        {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        3, 0, 33, 0, 210,
        {16, 4, 4},
        1, {14, 14, 14, 14, 14}, {0, 0},
        0.025f, 0.0f, 0.6f, 0.6f, 0.8f,
        {0.35f, -1.0f, -1.0f},
        {0.45f, -1.0f, -1.0f},
        0.0f, 1.0f, 0.2f, 0.35f, 0.0f,
        0, {0, 0, 0},
        {0, NULL},
        0.0f, 0, {0, 0, 0},
        {0.0f, 0.0f, 0.0f},
        {0.9f, 0.0f, 0.0f},
        0, {0, 0, 0}, 0.0f, 0, {0, 0, 0},
        {0.1f, 0.0f, 0.0f},
        -1.0f, 0.0f,
        {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        4, 0, 31, 0, 210,
        {8, 4, 4},
        1, {14, 14, 14, 14, 14}, {0, 0},
        0.025f, 0.0f, 0.6f, 0.6f, 0.8f,
        {0.35f, -1.0f, -1.0f},
        {0.45f, -1.0f, -1.0f},
        0.0f, 1.0f, 0.2f, 0.35f, 0.0f,
        0, {1, 0, 0},
        {0, NULL},
        0.0f, 0, {0, 0, 0},
        {0.0f, 0.0f, 0.0f},
        {0.9f, 0.0f, 0.0f},
        0, {0, 0, 0}, 0.0f, 0, {0, 0, 0},
        {0.1f, 0.0f, 0.0f},
        -1.0f, 0.0f,
        {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        5, 0, 17, 0, 210,
        {4, 4, 4},
        1, {-1, -1, -1, -1, -1}, {0, 0},
        0.025f, 0.0f, 0.5f, 0.5f, 0.5f,
        {-1.0f, -1.0f, -1.0f},
        {-1.0f, -1.0f, -1.0f},
        0.0f, 0.0f, 0.2f, 0.35f, 0.0f,
        0, {0, 0, 0},
        {0, NULL},
        0.0f, 0, {0, 0, 0},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
        0, {0, 0, 0}, 0.0f, 0, {0, 0, 0},
        {0.0f, 0.0f, 0.0f},
        -1.0f, 0.0f,
        {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        6, 0, 19, 0, 210,
        {4, 4, 4},
        1, {16, 13, 16, 13, 16}, {0, 0},
        0.03f, 0.0f, 1.0f, 0.5f, 0.5f,
        {-1.0f, -1.0f, -1.0f},
        {-1.0f, -1.0f, -1.0f},
        0.0f, 0.0f, 0.2f, 0.35f, 0.0f,
        0, {0, 0, 0},
        {0, NULL},
        0.0f, 0, {0, 0, 0},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
        0, {0, 0, 0}, 0.0f, 0, {0, 0, 0},
        {0.0f, 0.0f, 0.0f},
        -1.0f, 0.0f,
        {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        7, 0, 20, 0, 210,
        {4, 4, 4},
        1, {-1, -1, -1, -1, -1}, {0, 0},
        0.025f, 0.0f, 1.0f, 0.5f, 0.5f,
        {-1.0f, -1.0f, -1.0f},
        {-1.0f, -1.0f, -1.0f},
        0.0f, 0.0f, 0.2f, 0.35f, 0.0f,
        0, {0, 0, 0},
        {0, NULL},
        0.0f, 0, {0, 0, 0},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
        0, {0, 0, 0}, 0.0f, 0, {0, 0, 0},
        {0.0f, 0.0f, 0.0f},
        -1.0f, 0.0f,
        {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        8, 0, 17, 0, 210,
        {4, 4, 4},
        1, {13, 13, 13, 13, 13}, {0, 0},
        0.025f, 0.0f, 0.75f, 0.75f, 0.8f,
        {-1.0f, -1.0f, -1.0f},
        {-1.0f, -1.0f, -1.0f},
        0.0f, 0.0f, 0.2f, 0.35f, 0.0f,
        0, {0, 0, 0},
        {0, NULL},
        0.0f, 0, {0, 0, 0},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
        0, {0, 0, 0}, 0.0f, 13, {0, 0, 0},
        {0.1f, 0.0f, 0.0f},
        -1.0f, 0.0f,
        {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        9, 0, 18, 0, 210,
        {4, 4, 4},
        1, {13, 13, 13, 13, 13}, {0, 0},
        0.025f, 0.0f, 0.75f, 0.75f, 0.8f,
        {-1.0f, -1.0f, -1.0f},
        {-1.0f, -1.0f, -1.0f},
        0.0f, 0.0f, 0.2f, 0.35f, 0.0f,
        0, {0, 0, 0},
        {0, NULL},
        0.0f, 0, {0, 0, 0},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
        0, {0, 0, 0}, 0.0f, 13, {0, 0, 0},
        {0.1f, 0.0f, 0.0f},
        -1.0f, 0.0f,
        {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        10, 0, 22, 0, 210,
        {12, 0, 0},
        1, {12, 12, 14, 12, 19}, {0, 0},
        0.025f, 0.0f, 0.5f, 0.5f, 0.9f,
        {0.32f, -1.0f, -1.0f},
        {0.42f, -1.0f, -1.0f},
        0.0f, 1.0f, 0.2f, 0.35f, 0.0f,
        0, {1, 1, 1},
        {0, NULL},
        0.0f, 0, {0, 0, 0},
        {1.0f, 0.0f, 0.0f},
        {0.9f, 0.0f, 0.0f},
        0, {0, 0, 0}, 1.1f, 11, {0, 0, 0},
        {0.1f, 0.0f, 0.0f},
        -1.0f, 0.0f,
        {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        11, 0, 21, 0, 210,
        {20, 0, 0},
        1, {10, 10, 10, 10, 10}, {0, 0},
        0.025f, 0.0f, 0.5f, 0.5f, 0.9f,
        {0.32f, -1.0f, -1.0f},
        {0.42f, -1.0f, -1.0f},
        0.0f, 1.0f, 0.2f, 0.35f, 0.0f,
        0, {0, 0, 0},
        {0, NULL},
        0.0f, 0, {0, 0, 0},
        {1.0f, 0.0f, 0.0f},
        {0.9f, 0.0f, 0.0f},
        0, {0, 0, 0}, 0.0f, 10, {0, 0, 0},
        {0.1f, 0.0f, 0.0f},
        -1.0f, 0.0f,
        {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        12, 0, 11, 0, 210,
        {12, 12, 0},
        1, {15, 15, 15, 15, 15}, {0, 0},
        0.015f, 0.0f, 0.5f, 0.5f, 0.9f,
        {0.2f, 0.36f, -1.0f},
        {0.3f, 0.44f, -1.0f},
        0.0f, 0.45f, 0.2f, 0.35f, 0.0f,
        0, {2, 3, 0},
        {0, NULL},
        0.0f, 0, {0, 0, 0},
        {1.0f, 3.0f, 0.0f},
        {0.9f, 0.9f, 0.0f},
        1, {0, 0, 0}, 0.0f, 0, {0, 0, 0},
        {0.1f, 0.2f, 0.0f},
        -1.0f, 0.0f,
        {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        13, 0, 7, 0, 210,
        {0, 0, 0},
        1, {-1, -1, -1, -1, -1}, {0, 0},
        0.04f, 0.0f, 1.0f, 0.5f, 0.9f,
        {0.32f, -1.0f, -1.0f},
        {0.52f, -1.0f, -1.0f},
        0.0f, 0.87f, 0.2f, 0.35f, 0.0f,
        0, {2, 0, 0},
        {0, NULL},
        0.0f, 0, {0, 0, 0},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
        0, {0, 0, 0}, 0.0f, 0, {0, 0, 0},
        {0.3f, 0.0f, 0.0f},
        -1.0f, 0.0f,
        {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        14, 0, 9, 0, 210,
        {20, 12, 0},
        1, {27, 27, 27, 27, 27}, {0, 0},
        0.02f, 0.0f, 1.0f, 0.8f, 0.8f,
        {0.2f, 0.63f, -1.0f},
        {0.27f, 0.7f, -1.0f},
        0.0f, 1.0f, 0.2f, 0.35f, 0.0f,
        0, {1, 1, 1},
        {0, NULL},
        0.0f, 0, {0, 0, 0},
        {2.5f, 2.0f, 0.0f},
        {0.9f, 0.9f, 0.0f},
        0, {0, 0, 0}, 0.0f, 0, {0, 0, 0},
        {0.2f, 0.2f, 0.0f},
        -1.0f, 0.0f,
        {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        15, 0, 15, 0, 210,
        {20, 0, 0},
        1, {-1, -1, -1, -1, -1}, {0, 0},
        0.015f, 0.0f, 1.0f, 0.5f, 0.5f,
        {0.4f, -1.0f, -1.0f},
        {0.5f, -1.0f, -1.0f},
        0.0f, 0.65f, 0.2f, 0.35f, 0.0f,
        0, {3, 0, 0},
        {0, NULL},
        0.0f, 0, {0, 0, 0},
        {10.0f, 0.0f, 0.0f},
        {0.84f, 0.0f, 0.0f},
        1, {0, 0, 0}, 0.0f, 0, {0, 0, 0},
        {0.3f, 0.0f, 0.0f},
        -1.0f, 0.0f,
        {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        16, 0, 28, 0, 210,
        {4, 4, 0},
        1, {23, 23, 23, 23, 23}, {0, 0},
        0.017f, 0.0f, 0.6f, 0.6f, 1.0f,
        {0.38f, -1.0f, -1.0f},
        {0.44f, -1.0f, -1.0f},
        0.0f, 1.0f, 0.35f, 0.42f, 0.0f,
        0, {1, 1, 0},
        {0, NULL},
        0.0f, 0, {0, 0, 0},
        {2.0f, 0.0f, 0.0f},
        {0.9f, 0.0f, 0.0f},
        1, {0, 0, 0}, 0.0f, 0, {0, 0, 0},
        {0.2f, 0.0f, 0.0f},
        -1.0f, 0.0f,
        {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        17, 0, 30, 0, 210,
        {20, 0, 0},
        1, {-1, -1, -1, -1, -1}, {0, 0},
        0.008f, 0.0f, 1.0f, 0.5f, 0.5f,
        {0.63f, -1.0f, -1.0f},
        {0.7f, -1.0f, -1.0f},
        0.0f, 1.0f, 0.2f, 0.35f, 0.0f,
        0, {1, 0, 0},
        {0, NULL},
        0.0f, 0, {0, 0, 0},
        {10.0f, 0.0f, 0.0f},
        {0.84f, 0.0f, 0.0f},
        1, {0, 0, 0}, 0.0f, -1, {0, 0, 0},
        {0.3f, 0.0f, 0.0f},
        0.15f, 0.5f,
        {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        18, 0, 34, 0, 210,
        {0, 0, 0},
        1, {12, 12, 14, 12, 19}, {0, 0},
        0.022f, 0.0f, 0.5f, 0.5f, 0.9f,
        {0.32f, -1.0f, -1.0f},
        {0.4f, -1.0f, -1.0f},
        0.0f, 1.0f, 0.2f, 0.35f, 0.0f,
        0, {1, 0, 0},
        {0, NULL},
        0.0f, 0, {0, 0, 0},
        {0.8f, 0.0f, 0.0f},
        {0.9f, 0.0f, 0.0f},
        0, {0, 0, 0}, 1.1f, 20, {0, 0, 0},
        {0.1f, 0.0f, 0.0f},
        -1.0f, 0.0f,
        {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        19, 0, 12, 0, 210,
        {0, 4, 8},
        1, {17, 17, 17, 17, 17}, {0, 0},
        0.01f, 0.5f, 0.9f, 0.9f, 0.9f,
        {0.15f, 0.42f, 0.68f},
        {0.25f, 0.5f, 0.75f},
        0.0f, 1.0f, 0.2f, 0.35f, 0.0f,
        0, {4, 4, 4},
        {0, NULL},
        0.0f, 0, {0, 0, 0},
        {1.0f, 1.0f, 0.0f},
        {0.9f, 0.9f, 0.9f},
        2, {0, 0, 0}, 0.0f, 0, {0, 0, 0},
        {0.1f, 0.1f, 0.2f},
        -1.0f, 0.0f,
        {5, 5, 12, 3, 3, 2, 0, 0},
    },
    {
        20, 0, 16, 0, 210,
        {4, 4, 4},
        1, {18, 18, 18, 18, 18}, {0, 0},
        0.025f, 0.0f, 0.5f, 0.5f, 0.9f,
        {0.32f, -1.0f, -1.0f},
        {0.42f, -1.0f, -1.0f},
        0.0f, 1.0f, 0.2f, 0.35f, 0.0f,
        0, {1, 1, 1},
        {0, NULL},
        0.0f, 0, {0, 0, 0},
        {0.8f, 0.0f, 0.0f},
        {0.9f, 0.0f, 0.0f},
        0, {0, 0, 0}, 0.0f, 18, {0, 0, 0},
        {0.1f, 0.1f, 0.0f},
        -1.0f, 0.0f,
        {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        21, 0, 16, 0, 210,
        {4, 4, 4},
        1, {12, 12, 14, 12, 19}, {0, 0},
        0.025f, 0.0f, 0.5f, 0.5f, 0.9f,
        {0.32f, -1.0f, -1.0f},
        {0.42f, -1.0f, -1.0f},
        0.0f, 1.0f, 0.2f, 0.35f, 0.0f,
        0, {1, 1, 1},
        {0, NULL},
        0.0f, 0, {0, 0, 0},
        {0.8f, 0.0f, 0.0f},
        {0.9f, 0.0f, 0.0f},
        0, {0, 0, 0}, 1.1f, 22, {0, 0, 0},
        {0.1f, 0.1f, 0.0f},
        -1.0f, 0.0f,
        {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        22, 0, 34, 0, 210,
        {0, 0, 0},
        1, {21, 21, 21, 21, 21}, {0, 0},
        0.025f, 0.0f, 0.6f, 0.6f, 0.9f,
        {0.42f, -1.0f, -1.0f},
        {0.52f, -1.0f, -1.0f},
        0.0f, 1.0f, 0.2f, 0.35f, 0.0f,
        0, {1, 0, 0},
        {0, NULL},
        0.0f, 0, {0, 0, 0},
        {0.8f, 0.0f, 0.0f},
        {0.9f, 0.0f, 0.0f},
        0, {0, 0, 0}, 1.1f, 21, {0, 0, 0},
        {0.1f, 0.0f, 0.0f},
        -1.0f, 0.0f,
        {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        23, 0, 13, 0, 210,
        {16, 4, 4},
        1, {-1, -1, -1, -1, -1}, {0, 0},
        0.01f, 0.0f, 0.8f, 0.8f, 0.8f,
        {0.52f, -1.0f, -1.0f},
        {0.62f, -1.0f, -1.0f},
        0.0f, 1.0f, 0.2f, 0.35f, 0.0f,
        0, {1, 1, 1},
        {0, NULL},
        0.0f, 1, {0, 0, 0},
        {0.8f, 0.0f, 0.0f},
        {0.9f, 0.0f, 0.0f},
        0, {0, 0, 0}, 0.0f, 0, {0, 0, 0},
        {0.2f, 0.1f, 0.0f},
        -1.0f, 0.0f,
        {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        24, 0, 40, 0, 210,
        {16, 4, 4},
        1, {-1, -1, -1, -1, -1}, {0, 0},
        0.01f, 0.0f, 0.5f, 0.5f, 0.9f,
        {0.32f, -1.0f, -1.0f},
        {0.42f, -1.0f, -1.0f},
        0.0f, 1.0f, 0.2f, 0.35f, 0.0f,
        0, {3, 1, 1},
        {0, NULL},
        0.0f, 0, {0, 0, 0},
        {0.8f, 0.0f, 0.0f},
        {0.9f, 0.0f, 0.0f},
        0, {0, 0, 0}, 0.0f, 0, {0, 0, 0},
        {0.2f, 0.1f, 0.0f},
        -1.0f, 0.0f,
        {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        25, 0, 0, 0, 210,
        {20, 20, 20},
        1, {-1, -1, -1, -1, -1}, {0, 0},
        0.025f, 0.0f, 0.5f, 0.5f, 0.9f,
        {0.32f, -1.0f, -1.0f},
        {0.42f, -1.0f, -1.0f},
        0.0f, 1.0f, 0.2f, 0.35f, 0.0f,
        0, {0, 0, 0},
        {0, NULL},
        0.0f, 0, {0, 0, 0},
        {1.0f, 0.0f, 0.0f},
        {0.9f, 0.0f, 0.0f},
        0, {0, 0, 0}, 0.0f, -1, {0, 0, 0},
        {0.1f, 0.1f, 0.0f},
        -1.0f, 0.0f,
        {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        26, 0, 42, 0, 210,
        {4, 4, 4},
        1, {-1, -1, -1, -1, -1}, {0, 0},
        0.025f, 0.0f, 0.5f, 0.5f, 0.9f,
        {0.35f, -1.0f, -1.0f},
        {0.45f, -1.0f, -1.0f},
        0.0f, 1.0f, 0.2f, 0.35f, 0.0f,
        0, {1, 1, 1},
        {0, NULL},
        0.0f, 0, {0, 0, 0},
        {0.8f, 0.0f, 0.0f},
        {0.9f, 0.0f, 0.0f},
        0, {0, 0, 0}, 0.0f, -1, {0, 0, 0},
        {0.1f, 0.1f, 0.0f},
        -1.0f, 0.0f,
        {0, 0, 0, 0, 0, 0, 0, 0},
    },
    {
        27, 0, 41, 0, 210,
        {4, 4, 4},
        1, {-1, -1, -1, -1, -1}, {0, 0},
        0.01f, 0.0f, 0.8f, 0.8f, 0.8f,
        {0.52f, -1.0f, -1.0f},
        {0.62f, -1.0f, -1.0f},
        0.0f, 1.0f, 0.2f, 0.35f, 0.0f,
        0, {1, 1, 1},
        {0, NULL},
        0.0f, 1, {0, 0, 0},
        {0.8f, 0.0f, 0.0f},
        {0.9f, 0.0f, 0.0f},
        0, {0, 0, 0}, 0.0f, 0, {0, 0, 0},
        {0.3f, 0.1f, 0.0f},
        -1.0f, 0.0f,
        {0, 0, 0, 0, 0, 0, 0, 0},
    },
};

PlayerAnimSpeedTuning gPlayerAnimSpeedThresholds = {
    {0.005f, 0.1f, 0.08f, 0.55f, 0.53f, 2.3993998f},
    {{0.0f, 0.0f, 0.0f}, {0.0f, 17.0f, 0.0f}},
    {0.0f, 5.0f, 0.0f},
    {114, 908, 399, 1046, 1049, 140, 1156, 1048, 1812},
    {1156, 1049, 1048, 1812},
    {0.002f, 0.003f, 0.0015f, 0.008f},
    {0.0022f, 0.002f, 0.0015f, 0.008f},
};

s16 gPlayerMoveTableA[96] = {
    0,    0,    0,    0,    22,   32,   31,   22,
    22,   32,   31,   22,   2,    122,  121,  2,
    3,    36,   35,   3,    3,    3,    3,    3,
    30,   69,   0,    0,    0,    231,  230,  232,
    0,    1072, 1072, 1072, 1073, 1075, 1074, 1073,
    1073, 1075, 1074, 1073, 1053, 1055, 1054, 1053,
    1082, 1084, 1083, 1082, 1082, 1084, 1083, 1082,
    30,   69,   0,    0,    0,    231,  242,  232,
    1043, 1043, 1043, 1043, 76,   76,   76,   76,
    76,   76,   76,   76,   115,  115,  115,  115,
    116,  116,  116,  116,  116,  116,  116,  116,
    1043, 1043, 1043, 1043, 1043, 1043, 1043, 1043,
};

s16 gPlayerSpellGameBits[52] = {
    45,     64,    471,    1469,  1486,   1532,  1911,   2391,  2392,  263,   3157,  0,      0,
    77,     0,     101,    0,     90,     0,     78,     0,     1024,  0,     1033,  0,      75,
    0,      74,    0,      1025,  0,      99,    0,      73,    0,     72,    15523, -10486, 15523,
    -10486, 15564, -13107, 15564, -13107, 15428, -25690, 15333, 24642, 15379, 29884, 15379,  29884,
};

s16 lbl_80333110[128] = {8,    8,    8,    8,    7,    7,    7,    7,    7,    7,    7,    7,    1051, 1051, 1051, 1051,
                         1051, 1051, 1051, 1051, 1051, 1051, 1051, 1051, 1093, 1093, 1457, 1090, 1092, 235,  234,  8,
                         140,  140,  140,  140,  147,  148,  149,  150,  147,  148,  149,  150,  147,  148,  149,  150,
                         147,  148,  149,  150,  147,  148,  149,  150,  1093, 1093, 1457, 1090, 1092, 235,  234,  8,
                         91,   91,   91,   91,   214,  215,  216,  217,  214,  215,  216,  217,  214,  215,  216,  217,
                         214,  215,  216,  217,  214,  215,  216,  217,  1093, 1093, 1457, 1090, 1092, 235,  234,  8,
                         1043, 1043, 1043, 1043, 218,  219,  131,  220,  218,  219,  131,  220,  218,  219,  131,  220,
                         218,  219,  131,  220,  218,  219,  131,  220,  1093, 1093, 1457, 1090, 1092, 235,  234,  8};
s16 gPlayerMoveTableB[14] = {140, 140, 140, 140, 147, 148, 149, 150, 147, 148, 149, 150, 140, 0};
u8 gPlayerSurfacePfxModeTable[36] = {0, 1, 2, 3, 0, 0, 0, 0, 0, 3, 0, 0, 0, 7, 5, 0, 0, 0,
                                     0, 0, 0, 3, 5, 0, 4, 6, 0, 7, 0, 0, 0, 0, 8, 0, 9, 0};
f32 gPlayerDefaultMoveParams[24] = {
    0.005f, 0.13329999f, 0.106639996f, 0.71982f, 0.69315994f, 1.2530199f,
    1.22636f, 1.8928598f, 1.8661999f, 2.3993998f, 2.3793998f, 2.3993998f,
    0.005f, 0.012f, 0.01f, 0.26f, 0.23f, 0.35f,
    0.3f, 0.52f, 0.5f, 0.7f, 0.68f, 0.7f,
};
PlayerMotionTuning lbl_803332B0 = {
    {
        {23, 201, 24, 25, 26, 193, 195, 194, 205, 206, -1, -1},
        {123, 123, 123, 123, 123, 123, 123, 123, 123, 123, -1, -1},
        {248, 248, 248, 248, 248, 248, 246, 247, 249, 250, -1, -1},
        {252, 252, 252, 252, 252, 252, 252, 252, 252, 252, -1, -1},
    },
    {
        12.0f, 12.0f, 12.0f, 12.0f, 12.0f, 12.0f, 12.0f, 12.0f,
        12.0f, 12.0f, 12.0f, 12.0f, 12.0f, 12.0f, 12.0f, 12.0f,
        12.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 16.0f, 16.0f,
        16.0f, 24.0f, 24.0f, 32.0f, 32.0f, 32.0f, 32.0f, 32.0f,
        32.0f, 32.0f, 32.0f, 32.0f, 32.0f, 32.0f, 32.0f, 32.0f,
        32.0f,
    },
    {
        8.0f, 8.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f,
        5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f,
        5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f,
        5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f,
        5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f,
        5.0f,
    },
    {
        14.0f, 14.0f, 14.0f, 14.0f, 14.0f, 14.0f, 14.0f, 14.0f,
        14.0f, 14.0f, 14.0f, 14.0f, 14.0f, 13.0f, 12.0f, 11.0f,
        10.0f, 9.6f, 8.0f, 7.2f, 6.8f, 6.8f, 6.8f, 6.8f,
        6.8f, 6.8f, 6.8f, 6.8f, 6.8f, 6.8f, 6.8f, 6.8f,
        6.8f, 6.8f, 6.8f, 6.8f, 6.8f, 6.8f, 6.8f, 6.8f,
        6.8f, 6.8f, 6.8f, 6.8f, 6.8f, 6.8f,
    },
    {
        8.0f, 8.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f,
        5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f,
        5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f,
        5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f,
        5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f,
        5.0f,
    },
    {
        14.0f, 14.0f, 14.0f, 14.0f, 14.0f, 14.0f, 14.0f, 14.0f,
        14.0f, 14.0f, 14.0f, 14.0f, 14.0f, 13.0f, 12.0f, 11.0f,
        10.0f, 9.6f, 8.0f, 7.2f, 6.8f, 6.8f, 6.8f, 6.8f,
        6.8f, 6.8f, 6.8f, 6.8f, 6.8f, 6.8f, 6.8f, 6.8f,
        6.8f, 6.8f, 6.8f, 6.8f, 6.8f, 6.8f, 6.8f, 6.8f,
        6.8f, 6.8f, 6.8f, 6.8f, 6.8f, 6.8f,
    },
};
s16 lbl_8033366C[24] = {168,   167,    166,   165,    92,    1071,   92,    21,     15692, -13107, 15692, -13107,
                        15692, -13107, 15692, -13107, 15692, -13107, 15820, -13107, 15692, -13107, 0,     0};
f32 lbl_8033369C[8] = {0.01f, 0.02f, 0.02f, 0.015f, 0.015f, 0.01f, 0.02f, 0.005f};
s16 gPlayerMoveSlotTable[44] = {1113, 1114, 0,    0,    0,    0,   0,    1120, 0,    1122, 0,    1124, 1125, 1126, 0,
                                1128, 1129, 151,  152,  153,  154, 1130, 1131, 1109, 0,    0,    1112, 1132, 1133, 1134,
                                1135, 1136, 1137, 1138, 1139, 0,   0,    0,    0,    0,    1145, 1152, 1129, 0};

int gPlayerStateHandlers[66];
f32 lbl_803DAF88[16];
LightmapVertex gPlayerHudVtxBuf[8];
f32 gPlayerPartFxParams[6];

void* jumptable_80334ABC[12] = {
    (void*)((u8*)fn_80295918 + 0xD8),
    (void*)((u8*)fn_80295918 + 0x40),
    (void*)((u8*)fn_80295918 + 0xD8),
    (void*)((u8*)fn_80295918 + 0xD8),
    (void*)((u8*)fn_80295918 + 0xD8),
    (void*)((u8*)fn_80295918 + 0x84),
    (void*)((u8*)fn_80295918 + 0x64),
    (void*)((u8*)fn_80295918 + 0xD8),
    (void*)((u8*)fn_80295918 + 0xD8),
    (void*)((u8*)fn_80295918 + 0xD8),
    (void*)((u8*)fn_80295918 + 0xB0),
    (void*)((u8*)fn_80295918 + 0xC4),
};
void* jumptable_80334AEC[19] = {
    (void*)((u8*)fn_80295A04 + 0x120),
    (void*)((u8*)fn_80295A04 + 0x24),
    (void*)((u8*)fn_80295A04 + 0x54),
    (void*)((u8*)fn_80295A04 + 0x120),
    (void*)((u8*)fn_80295A04 + 0x120),
    (void*)((u8*)fn_80295A04 + 0x120),
    (void*)((u8*)fn_80295A04 + 0x120),
    (void*)((u8*)fn_80295A04 + 0x120),
    (void*)((u8*)fn_80295A04 + 0x120),
    (void*)((u8*)fn_80295A04 + 0xB8),
    (void*)((u8*)fn_80295A04 + 0xD0),
    (void*)((u8*)fn_80295A04 + 0xDC),
    (void*)((u8*)fn_80295A04 + 0x120),
    (void*)((u8*)fn_80295A04 + 0xE8),
    (void*)((u8*)fn_80295A04 + 0xFC),
    (void*)((u8*)fn_80295A04 + 0x120),
    (void*)((u8*)fn_80295A04 + 0x120),
    (void*)((u8*)fn_80295A04 + 0x120),
    (void*)((u8*)fn_80295A04 + 0x104),
};
void* jumptable_80334B38[25] = {
    (void*)((u8*)playerState1D + 0x220),
    (void*)((u8*)playerState1D + 0x220),
    (void*)((u8*)playerState1D + 0x25C),
    (void*)((u8*)playerState1D + 0x25C),
    (void*)((u8*)playerState1D + 0x25C),
    (void*)((u8*)playerState1D + 0x25C),
    (void*)((u8*)playerState1D + 0x25C),
    (void*)((u8*)playerState1D + 0x25C),
    (void*)((u8*)playerState1D + 0x25C),
    (void*)((u8*)playerState1D + 0x25C),
    (void*)((u8*)playerState1D + 0x25C),
    (void*)((u8*)playerState1D + 0x25C),
    (void*)((u8*)playerState1D + 0x25C),
    (void*)((u8*)playerState1D + 0x220),
    (void*)((u8*)playerState1D + 0x25C),
    (void*)((u8*)playerState1D + 0x25C),
    (void*)((u8*)playerState1D + 0x25C),
    (void*)((u8*)playerState1D + 0x25C),
    (void*)((u8*)playerState1D + 0x1EC),
    (void*)((u8*)playerState1D + 0x25C),
    (void*)((u8*)playerState1D + 0x25C),
    (void*)((u8*)playerState1D + 0x25C),
    (void*)((u8*)playerState1D + 0x25C),
    (void*)((u8*)playerState1D + 0x25C),
    (void*)((u8*)playerState1D + 0x220),
};
void* jumptable_80334B9C[8] = {
    (void*)((u8*)playerStateClimbWall + 0x3AC),
    (void*)((u8*)playerStateClimbWall + 0x3DC),
    (void*)((u8*)playerStateClimbWall + 0x3BC),
    (void*)((u8*)playerStateClimbWall + 0x3C4),
    (void*)((u8*)playerStateClimbWall + 0x3A4),
    (void*)((u8*)playerStateClimbWall + 0x3D0),
    (void*)((u8*)playerStateClimbWall + 0x3B4),
    (void*)((u8*)playerStateClimbWall + 0x3E8),
};
void* jumptable_80334BBC[8] = {
    (void*)((u8*)playerStateClimbLedge + 0xEC),
    (void*)((u8*)playerStateClimbLedge + 0x654),
    (void*)((u8*)playerStateClimbLedge + 0x1DC),
    (void*)((u8*)playerStateClimbLedge + 0x264),
    (void*)((u8*)playerStateClimbLedge + 0x654),
    (void*)((u8*)playerStateClimbLedge + 0x59C),
    (void*)((u8*)playerStateClimbLedge + 0x330),
    (void*)((u8*)playerStateClimbLedge + 0x3CC),
};
void* jumptable_80334BDC[13] = {
    (void*)((u8*)playerState0B + 0x11C),
    (void*)((u8*)playerState0B + 0x16C),
    (void*)((u8*)playerState0B + 0x16C),
    (void*)((u8*)playerState0B + 0x16C),
    (void*)((u8*)playerState0B + 0xB0),
    (void*)((u8*)playerState0B + 0x16C),
    (void*)((u8*)playerState0B + 0x16C),
    (void*)((u8*)playerState0B + 0x16C),
    (void*)((u8*)playerState0B + 0x11C),
    (void*)((u8*)playerState0B + 0x16C),
    (void*)((u8*)playerState0B + 0x16C),
    (void*)((u8*)playerState0B + 0x16C),
    (void*)((u8*)playerState0B + 0xB0),
};
void* jumptable_80334C10[14] = {
    (void*)((u8*)playerState08 + 0xFC),
    (void*)((u8*)playerState08 + 0x1F0),
    (void*)((u8*)playerState08 + 0x1F0),
    (void*)((u8*)playerState08 + 0x1F0),
    (void*)((u8*)playerState08 + 0x144),
    (void*)((u8*)playerState08 + 0x15C),
    (void*)((u8*)playerState08 + 0x180),
    (void*)((u8*)playerState08 + 0x1A4),
    (void*)((u8*)playerState08 + 0x1BC),
    (void*)((u8*)playerState08 + 0x120),
    (void*)((u8*)playerState08 + 0x1E0),
    (void*)((u8*)playerState08 + 0x1CC),
    (void*)((u8*)playerState08 + 0x1F0),
    (void*)((u8*)playerState08 + 0x194),
};
void* jumptable_80334C60[13] = {
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x6D4),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x94C),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x99C),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x8C0),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x8C0),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x900),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x900),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x94C),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0xA0C),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x99C),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x824),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0xAA0),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x94C),
};
void* jumptable_80334C94[11] = {
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x5C4),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x5B4),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x554),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x524),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x554),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x524),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x554),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x5B4),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x5B4),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x5B4),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x5C4),
};
void* jumptable_80334CC0[13] = {
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x234),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x1EC),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x288),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x260),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x2D8),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x260),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x2D8),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x1EC),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x330),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x330),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x234),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x328),
    (void*)((u8*)playerCheckIfClimbingOntoWall + 0x1EC),
};
void* jumptable_80334CF4[12] = {
    (void*)((u8*)fn_802AFB0C + 0x854),
    (void*)((u8*)fn_802AFB0C + 0x910),
    (void*)((u8*)fn_802AFB0C + 0x7FC),
    (void*)((u8*)fn_802AFB0C + 0x910),
    (void*)((u8*)fn_802AFB0C + 0x910),
    (void*)((u8*)fn_802AFB0C + 0x910),
    (void*)((u8*)fn_802AFB0C + 0x910),
    (void*)((u8*)fn_802AFB0C + 0x910),
    (void*)((u8*)fn_802AFB0C + 0x8D4),
    (void*)((u8*)fn_802AFB0C + 0x910),
    (void*)((u8*)fn_802AFB0C + 0x910),
    (void*)((u8*)fn_802AFB0C + 0x854),
};
void* jumptable_80334D24[32] = {
    (void*)((u8*)fn_802AFB0C + 0x430),
    (void*)((u8*)fn_802AFB0C + 0x314),
    (void*)((u8*)fn_802AFB0C + 0x454),
    (void*)((u8*)fn_802AFB0C + 0x430),
    (void*)((u8*)fn_802AFB0C + 0x2FC),
    (void*)((u8*)fn_802AFB0C + 0x454),
    (void*)((u8*)fn_802AFB0C + 0x430),
    (void*)((u8*)fn_802AFB0C + 0x278),
    (void*)((u8*)fn_802AFB0C + 0x278),
    (void*)((u8*)fn_802AFB0C + 0x278),
    (void*)((u8*)fn_802AFB0C + 0x2D0),
    (void*)((u8*)fn_802AFB0C + 0x24C),
    (void*)((u8*)fn_802AFB0C + 0x2A4),
    (void*)((u8*)fn_802AFB0C + 0x430),
    (void*)((u8*)fn_802AFB0C + 0x430),
    (void*)((u8*)fn_802AFB0C + 0x430),
    (void*)((u8*)fn_802AFB0C + 0x430),
    (void*)((u8*)fn_802AFB0C + 0x430),
    (void*)((u8*)fn_802AFB0C + 0x454),
    (void*)((u8*)fn_802AFB0C + 0x430),
    (void*)((u8*)fn_802AFB0C + 0x398),
    (void*)((u8*)fn_802AFB0C + 0x328),
    (void*)((u8*)fn_802AFB0C + 0x34C),
    (void*)((u8*)fn_802AFB0C + 0x454),
    (void*)((u8*)fn_802AFB0C + 0x454),
    (void*)((u8*)fn_802AFB0C + 0x380),
    (void*)((u8*)fn_802AFB0C + 0x398),
    (void*)((u8*)fn_802AFB0C + 0x390),
    (void*)((u8*)fn_802AFB0C + 0x430),
    (void*)((u8*)fn_802AFB0C + 0x430),
    (void*)((u8*)fn_802AFB0C + 0x3E4),
    (void*)((u8*)fn_802AFB0C + 0x398),
};
void* jumptable_80334DA4[30] = {
    (void*)((u8*)fn_802B1E5C + 0x13C),
    (void*)((u8*)fn_802B1E5C + 0x4A8),
    (void*)((u8*)fn_802B1E5C + 0x4A8),
    (void*)((u8*)fn_802B1E5C + 0x154),
    (void*)((u8*)fn_802B1E5C + 0x4A8),
    (void*)((u8*)fn_802B1E5C + 0x290),
    (void*)((u8*)fn_802B1E5C + 0x4A8),
    (void*)((u8*)fn_802B1E5C + 0x4A8),
    (void*)((u8*)fn_802B1E5C + 0x4A8),
    (void*)((u8*)fn_802B1E5C + 0x4A8),
    (void*)((u8*)fn_802B1E5C + 0x120),
    (void*)((u8*)fn_802B1E5C + 0x4A8),
    (void*)((u8*)fn_802B1E5C + 0x4A8),
    (void*)((u8*)fn_802B1E5C + 0x4A8),
    (void*)((u8*)fn_802B1E5C + 0x4A8),
    (void*)((u8*)fn_802B1E5C + 0x4A8),
    (void*)((u8*)fn_802B1E5C + 0x4A8),
    (void*)((u8*)fn_802B1E5C + 0x4A8),
    (void*)((u8*)fn_802B1E5C + 0x4A8),
    (void*)((u8*)fn_802B1E5C + 0x4A8),
    (void*)((u8*)fn_802B1E5C + 0x4A8),
    (void*)((u8*)fn_802B1E5C + 0x4A8),
    (void*)((u8*)fn_802B1E5C + 0x4A8),
    (void*)((u8*)fn_802B1E5C + 0x204),
    (void*)((u8*)fn_802B1E5C + 0x4A8),
    (void*)((u8*)fn_802B1E5C + 0x2AC),
    (void*)((u8*)fn_802B1E5C + 0x1B8),
    (void*)((u8*)fn_802B1E5C + 0x4A8),
    (void*)((u8*)fn_802B1E5C + 0x498),
    (void*)((u8*)fn_802B1E5C + 0x344),
};
void* jumptable_80334E1C[51] = {
    (void*)((u8*)player_SeqFn + 0x1B64),
    (void*)((u8*)player_SeqFn + 0x18C4),
    (void*)((u8*)player_SeqFn + 0x11AC),
    (void*)((u8*)player_SeqFn + 0xE50),
    (void*)((u8*)player_SeqFn + 0x11D0),
    (void*)((u8*)player_SeqFn + 0x1B64),
    (void*)((u8*)player_SeqFn + 0x1398),
    (void*)((u8*)player_SeqFn + 0x13E8),
    (void*)((u8*)player_SeqFn + 0x1444),
    (void*)((u8*)player_SeqFn + 0x1B64),
    (void*)((u8*)player_SeqFn + 0x14A0),
    (void*)((u8*)player_SeqFn + 0x128C),
    (void*)((u8*)player_SeqFn + 0x1B64),
    (void*)((u8*)player_SeqFn + 0x150C),
    (void*)((u8*)player_SeqFn + 0x1B64),
    (void*)((u8*)player_SeqFn + 0x1640),
    (void*)((u8*)player_SeqFn + 0x1654),
    (void*)((u8*)player_SeqFn + 0x1B64),
    (void*)((u8*)player_SeqFn + 0x1770),
    (void*)((u8*)player_SeqFn + 0x1788),
    (void*)((u8*)player_SeqFn + 0x1730),
    (void*)((u8*)player_SeqFn + 0x1744),
    (void*)((u8*)player_SeqFn + 0x175C),
    (void*)((u8*)player_SeqFn + 0x1688),
    (void*)((u8*)player_SeqFn + 0x14D8),
    (void*)((u8*)player_SeqFn + 0x1794),
    (void*)((u8*)player_SeqFn + 0x185C),
    (void*)((u8*)player_SeqFn + 0x1B64),
    (void*)((u8*)player_SeqFn + 0x17AC),
    (void*)((u8*)player_SeqFn + 0x17BC),
    (void*)((u8*)player_SeqFn + 0x17EC),
    (void*)((u8*)player_SeqFn + 0x181C),
    (void*)((u8*)player_SeqFn + 0x1834),
    (void*)((u8*)player_SeqFn + 0x1844),
    (void*)((u8*)player_SeqFn + 0x1850),
    (void*)((u8*)player_SeqFn + 0x1B64),
    (void*)((u8*)player_SeqFn + 0x1B64),
    (void*)((u8*)player_SeqFn + 0x18F0),
    (void*)((u8*)player_SeqFn + 0x1904),
    (void*)((u8*)player_SeqFn + 0x1918),
    (void*)((u8*)player_SeqFn + 0x1924),
    (void*)((u8*)player_SeqFn + 0x1A04),
    (void*)((u8*)player_SeqFn + 0x1A10),
    (void*)((u8*)player_SeqFn + 0x1B28),
    (void*)((u8*)player_SeqFn + 0x1B40),
    (void*)((u8*)player_SeqFn + 0x1B10),
    (void*)((u8*)player_SeqFn + 0x1B1C),
    (void*)((u8*)player_SeqFn + 0x1B64),
    (void*)((u8*)player_SeqFn + 0x1B64),
    (void*)((u8*)player_SeqFn + 0x1B54),
    (void*)((u8*)player_SeqFn + 0x1B5C),
};
