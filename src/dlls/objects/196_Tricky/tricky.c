/*
 * Tricky companion DLL.
 *
 * Blend-channel weight animation (Tricky_updateBlendChannelWeight), the
 * impress fade (tricky_updateModelVariantFade / trickyImpress), queued-path particle emission
 * (Tricky_emitQueuedPathParticles), baddie target search
 * (trickyFindNearestUsableBaddie) and queued-command target selection
 * (trickySelectQueuedCommandTarget), plus small state accessors.
 */

#include "main/dll/partfx_interface.h"
#include "main/vecmath.h"
#include "main/obj_group.h"
#include "main/obj_link.h"
#include "main/dll/ppcwgpipe_struct.h"
#include "main/dll/baddie_control_interface.h"
#include "main/dll/tricky_state.h"
#include "game/objects/object.h"
#include "sys/objects.h"
#include "main/model.h"
#include "sys/objects/lifecycle.h"
#include "main/mapEventTypes.h"
#include "main/objseq.h"
#include "main/frame_timing.h"
#include "main/gamebit_ids.h"
#include "main/gamebits_api.h"
#include "main/dll/dll_80136a40.h"
#include "main/dll/dll_00C9_enemy.h"
#include "main/dll/dll_0019_dll19func0.h"
#include "main/track_dolphin_api.h"
#include "main/audio/sfx_channel_query_api.h"
#include "main/audio/sfx_play_api.h"
#include "main/dll/objfsa_romcurve.h"
#include "main/lightmap_api.h"
#include "main/pi_dolphin_api.h"
#include "main/dll/path_control_interface.h"
#include "main/dll/rom_curve_interface.h"
#include "main/obj_list.h"
#include "main/audio/sfx_ids.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/objhits.h"
#include "main/objHitReact.h"
#include "main/objfx.h"
#include "main/dll/objfsa.h"
#include "main/gamebits.h"
#include "main/dll/skeetla.h"
#include "main/objprint_sound_api.h"
#include "main/dll/dll_00C4_tricky.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/dll/baddie/trickyfollow.h"
#include "main/dll/objfsa_query_api.h"
#include "main/dll/modgfx.h"
#include "main/dll/dll_0014_api.h"
#include "main/dll/skeetla_anim_api.h"
#include "main/dll/Hcurves_api.h"
#include "main/dll/baddie/MMP_critterspit.h"
#include "main/dll/mmp_cratercritter.h"
#include "main/vecmath_distance_api.h"
#include "main/audio/sfx.h"
#include "dlls/objects/243_flameblast.h"
#include "main/dll/tumbleweedbush.h"
#include "main/audio/sfx_looped_object_api.h"
#include "main/dll/player_target.h"
#include "game/objects/object_setup.h"
#include "main/trig.h"
#include "main/frustum.h"
#include "dlls/objects/245_SidekickBal.h"
#include "dlls/objects/417_NW_mammoth.h"
#include "main/dll/tricky_substates.h"
#include "dlls/objects/209_TumbleWeedB.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_float_helpers.h"
#include "main/dll/skeetla_route_api.h"
#include "main/dll/tricky_rollroute.h"
#include "main/game_ui_interface.h"
#include "main/sky_interface.h"
#include "main/dll/dll_0000_gameui_api.h"
#include "main/pad.h"
#include "main/dll/tricky_api.h"
#include "main/objprint_api.h"
#include "main/dll/objfx_api.h"
#include "main/dll/dll_005A_staffcollisionfunc03.h"
#include "main/audio/sfx_limited_object_api.h"
#include "main/audio/sfx_stop_channel_api.h"
#include "main/object_render.h"
#include "main/shader_api.h"
#include "main/objanim.h"
#include "main/objprint_anim_api.h"
#include "main/objprint_character_api.h"
#include "main/dll_000A_expgfx.h"
#include "main/track_bbox_api.h"
#include "main/obj_path.h"
#include "main/model_light.h"
#include "dolphin/mtx.h"
#include "main/dll/cmenu_item_table.h"
#include "main/dll/boneparticleeffect_interface.h"
#include "main/dll/baddie_state.h"
#include "main/dll/player_api.h"
#include "main/dll/player_state_api.h"
#include "main/gameloop_gamebit_api.h"
#include "main/dll/WC/WCbeacon.h"
#include "main/voxmaps.h"
#include "main/dll/DR/dll_026B_drchimmey.h"
#include "main/dll/DIM/dll_01C4_dimicewall.h"
#include "main/dll/DIM/dimlogfire.h"
#include "main/dll/DIM/dll_01D1_dimtruthhornice.h"
#include "main/dll/SH/dll_01B3_shbeacon.h"
#include "main/main_internal.h"
#include "main/dll/baddie_frozen.h"
#include "dlls/objects/316_XYZAnimator.h"
#include "main/dll/dll_0014_unk.h"
#include "main/pi_dolphin_texture_api.h"
#include "main/pi_dolphin_path_api.h"
#include "main/newshadows_audio_api.h"
#include "string.h"

typedef struct
{
    u16 a;
    u16 b;
} TrickySfxPair;

extern f32 lbl_803E23E8;
extern f32 lbl_803E2418;
extern f32 lbl_803E23DC;
extern f32 lbl_803E23E0;
extern f32 lbl_803E23E4;
extern f32 lbl_803E23EC;
extern f32 lbl_803E23F0;
extern f32 lbl_803E23F4;
extern f32 lbl_803E23F8;
extern f32 lbl_803E2408;
extern f32 lbl_803E240C;
extern f32 lbl_803E2410;
extern f32 lbl_803E2414;
extern f32 lbl_803E2424;
extern f32 lbl_803E2428;
extern f32 lbl_803E242C;
extern f32 lbl_803E2430;
extern f32 lbl_803E2434;
extern f32 lbl_803E2438;
extern f32 lbl_803E244C;
extern f32 lbl_803E2448;
extern f32 lbl_803E2450;
extern f32 lbl_803E2420;
extern f32 lbl_803E243C;
extern f32 lbl_803E2440;
extern f32 lbl_803E2454;
extern f32 lbl_803E2458;
extern f32 lbl_803E2468;
extern f32 lbl_803E246C;
extern f32 lbl_803E2470;
extern f32 lbl_803E2474;
extern f32 lbl_803E247C;
extern f32 lbl_803E2478;
extern f32 lbl_803E2480;
extern f32 lbl_803E2484;
extern char lbl_8031D2E8[];
extern u32 gSkeetlaFootstepSfxIds01;
extern f32 lbl_803E241C;
extern f32 lbl_803E2488;
extern f32 gTrickyFollowMaxSpeed;
extern f32 gTrickyFollowAnim17Speed;
extern f32 gTrickyFollowAnim18Speed;
extern f32 gTrickyFollowVerticalDeltaDivisorA;
extern f32 lbl_803E249C;
extern f32 gTrickyFollowVerticalDeltaDivisorB;
extern f32 lbl_803E24A4;
extern f32 lbl_803E24A8;
extern f32 lbl_803E24AC;
extern f32 gTrickyFollowArcCoefficient;
extern f32 lbl_803E24B4;
extern f32 lbl_803E24B8;
extern f32 lbl_803E24BC;
extern f32 lbl_803E24C0;
extern f32 lbl_803E2444;
extern f32 lbl_803E24C4;
extern char sInWaterMessage[];
extern char lbl_8031D478[];
extern f32 lbl_803E24C8;
extern f32 lbl_803E24CC;
extern f32 lbl_803E24D0;
extern const char sTrickyShouldNeverStopCirclingError[];
extern f32 lbl_803E24D4;
extern f32 lbl_803E24D8;
extern f32 lbl_803E24DC;
extern f32 lbl_803E24E0;
extern f32 lbl_803E24E4;
extern f32 lbl_803E24E8;
extern f32 lbl_803E24EC;
extern f32 lbl_803E24F0;
extern f32 lbl_803E24F4;
extern f32 lbl_803E24F8;
extern f32 lbl_803E24FC;
extern f32 lbl_803E2500;
extern f32 lbl_803E2504;
extern f32 lbl_803E2508;
extern f32 lbl_803E250C;
extern f32 lbl_803E2518;
extern f32 lbl_803E251C;
extern f32 lbl_803E2524;
extern f32 lbl_803E2510;
extern u32 gTrickySubstateSfxIdPairB;
extern u32 gTrickySubstateSfxIdPairA;
extern f32 lbl_803E2514;
extern f32 lbl_803E2520;
extern f64 lbl_803E2528;
extern f32 lbl_803E2530;
extern f32 lbl_803E2534;
extern char gTrickyPathPointCollision[];
extern u32 lbl_803E23C8;
extern char sSidekickCommandDebugTextBlock[];
extern f32 lbl_803DBC40[2];
extern f32 lbl_803DBC48;
extern u16 lbl_803E23C0;
extern f32 lbl_803E253C;
extern f32 lbl_803E2540;
extern u32 lbl_803E2558;
extern u32 lbl_803E2560;
extern u32 lbl_803E2564;
extern u16 lbl_803E2568;
extern f32 lbl_803E2574;
extern f32 lbl_803E2570;
extern f32 lbl_803E2578;
extern f32 lbl_803E257C;
extern f32 lbl_803E256C;
extern f32 lbl_803E2598;
extern f32 lbl_803E25A0;
extern f32 lbl_803E25A8;
extern f32 lbl_803E25AC;
extern f32 lbl_803E25B4;
extern f32 lbl_803E25B8;
extern f32 lbl_803E25BC;
extern f32 lbl_803E25C0;
extern f32 lbl_803E25C4;
extern f32 lbl_803E25C8;
extern f32 lbl_803E25A4;
extern f32 lbl_803E2588;
extern f32 lbl_803E258C;
extern f32 lbl_803E2590;
extern f32 lbl_803E2594;
extern f32 lbl_803E259C;
extern TrickySfxPair lbl_803E23C4;
extern f32 lbl_803E2538;
extern f32 lbl_803E2544;
extern f32 lbl_803E2548;
extern f32 lbl_803E254C;
extern f32 lbl_803E2550;

/* The one partfx effect emitted along Tricky's queued impress path. */
#define TRICKY_PATH_PARTFX 0x533

#define TRICKY_BADDIE_TARGET_OBJGROUP 49 /* baddie object group scanned by trickyFindNearestUsableBaddie */
/* creatures excluded from Tricky's baddie targeting (retail OBJECTS.bin names). */
#define TRICKY_SEQID_WHIRLPOOL    2129 /* "Whirlpool" (DLL 0xC9) */
#define TRICKY_SEQID_VAMBAT       1022 /* "Vambat" (DLL 0xC9) */
#define TRICKY_SEQID_WB           1239 /* "WB" (DLL 0xC9) */
#define TRICKY_SEQID_SC_BABYLIGHT 636  /* "SC_babyligh" (DLL 0x1B5) */
#define TRICKY_SEQID_PINPON       593  /* "PinPon" (DLL 0xC9) */

#define TUMBLEWEED_BLEND_FLAGS_OFFSET    0x82e
#define TUMBLEWEED_BLEND_WEIGHT_OFFSET   0x830
#define TUMBLEWEED_BLEND_VELOCITY_OFFSET 0x834

typedef struct TrickyImpressState
{
    u8 pad0[0x14 - 0x0];
    f32 animSpeed;
    u8 pad18[0x24 - 0x18];
    GameObject* stayPoint;
    u8 pad28[0x54 - 0x28];
    u32 flags54;
    u8 pad58[0x408 - 0x58];
    f32 renderPosX;
    f32 renderPosY;
    f32 renderPosZ;
    s16 unk414;
    u8 pad416[0x7A8 - 0x416];
    s32 childObj0; /* 0x7A8: attached child object handle (slot 0) */
    u8 pad7AC[0x7B0 - 0x7AC];
    s32 childObj1; /* 0x7B0: attached child object handle (slot 1) */
    u8 pad7B4[0x7B8 - 0x7B4];
    s32 childObj2;   /* 0x7B8: attached child object handle (slot 2) */
    u8 childSlotMap; /* 0x7BC: packed 2-bit slot index per impress child (childObj0/1/2 via >>6/>>4/>>2 & 3) */
    u8 pad7BD[0x808 - 0x7BD];
    f32 unk808;
    u8 pad80C[0x810 - 0x80C];
} TrickyImpressState;

typedef struct
{
    u8 pending : 1;
    u8 active : 1;
    u8 rest : 6;
} TumbleweedBlendFlags;

struct Bits58
{
    u8 _pad[0x58];
    u8 b7 : 1;
    u8 b6 : 1;
    u8 lo : 6;
};

typedef struct
{
    u8 s0 : 2;
    u8 s1 : 2;
    u8 s2 : 2;
    u8 s3 : 2;
} AnimSlots;


/* Bit setter at bit 6 (0x40) of obj->_b8->_58. */
void trickySetSoundSuppressed(GameObject* obj, int v)
{
    ((struct Bits58*)((GameObject*)obj)->extra)->b6 = v;
}

int trickyTryPlaySound(GameObject* obj, u16 sfxId, int vol)
{
    u8* b = ((GameObject*)obj)->extra;
    s16 v;
    if ((u32)((b[0x58] >> 6) & 1) != 0u)
        return 0;
    v = ((GameObject*)obj)->anim.currentMove;
    switch (v)
    {
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
    case 46:
    case 47:
        return 0;
    }
    if (Sfx_IsPlayingFromObjectChannel((int)obj, 16) != 0)
        return 0;
    objAudioFn_800393f8(obj, &((TrickyState*)b)->soundState, sfxId, vol, -1, 0);
    return 1;
}

void objAnimFreeChildren(int a, int b, GameObject** c)
{
    char buf[4];
    void *v0, *v1, *v2;

    if (*c == NULL)
    {
        return;
    }
    ObjLink_DetachChild((GameObject*)a, *c);
    Obj_FreeObject(*c);
    *c = NULL;
    buf[0] = -1;
    buf[1] = -1;
    buf[2] = -1;
    v0 = (void*)((TrickyImpressState*)b)->childObj0;
    if (v0 != NULL)
    {
        buf[((TrickyImpressState*)b)->childSlotMap >> 6 & 3] = 1;
    }
    v1 = (void*)((TrickyImpressState*)b)->childObj1;
    if (v1 != NULL)
    {
        buf[((TrickyImpressState*)b)->childSlotMap >> 4 & 3] = 1;
    }
    v2 = (void*)((TrickyImpressState*)b)->childObj2;
    if (v2 != NULL)
    {
        buf[((TrickyImpressState*)b)->childSlotMap >> 2 & 3] = 1;
    }
    if (buf[0] == -1)
    {
        if (v0 != NULL)
        {
            ObjLink_DetachChild((GameObject*)a, v0);
            ObjLink_AttachChild((GameObject*)a, (GameObject*)((TrickyImpressState*)b)->childObj0, 0);
            ((AnimSlots*)(b + 0x7bc))->s0 = 0;
        }
        else if (v1 != NULL)
        {
            ObjLink_DetachChild((GameObject*)a, v1);
            ObjLink_AttachChild((GameObject*)a, (GameObject*)((TrickyImpressState*)b)->childObj1, 0);
            ((AnimSlots*)(b + 0x7bc))->s1 = 0;
        }
        else if (v2 != NULL)
        {
            ObjLink_DetachChild((GameObject*)a, v2);
            ObjLink_AttachChild((GameObject*)a, (GameObject*)((TrickyImpressState*)b)->childObj2, 0);
            ((AnimSlots*)(b + 0x7bc))->s2 = 0;
        }
    }
}

/* Weighted blend-channel animator. On state[0x82e] bit 0x80,
 * primes channel 1 (weight 0, target weight ratio at +0x830) and latches
 * the active flag. While bit 0x40 is set, ramps state[0x830] toward
 * data[0] / data[1] with acceleration lbl_803E23E4 and damping
 * lbl_803E23F0, clamps to [0, lbl_803E23E8], and pushes the result to the
 * model's blend channel 1 as `lbl_803E23F8 * weight - lbl_803E23E8`. */
void Tricky_updateBlendChannelWeight(int obj, u8* state)
{
    ObjModel* model;
    f32 target;
    f32 max;
    Obj_GetActiveModel((GameObject*)obj);
    if ((u32)((state[TUMBLEWEED_BLEND_FLAGS_OFFSET] >> 7) & 1) != 0)
    {
        model = Obj_GetActiveModel((GameObject*)obj);
        ObjModel_SetBlendChannelTargets(model, 1, -1, 0x1a, lbl_803E23DC, 0x21);
        *(f32*)(state + TUMBLEWEED_BLEND_WEIGHT_OFFSET) = lbl_803E23E0;
        ObjModel_SetBlendChannelWeight(model, 0, lbl_803E23DC);
        ((TumbleweedBlendFlags*)(state + TUMBLEWEED_BLEND_FLAGS_OFFSET))->pending = 0;
        ((TumbleweedBlendFlags*)(state + TUMBLEWEED_BLEND_FLAGS_OFFSET))->active = 1;
    }
    if ((u32)((state[TUMBLEWEED_BLEND_FLAGS_OFFSET] >> 6) & 1) != 0)
    {
        u8* data = *(u8**)(state + 0);
        target = (f32)(u32)data[0] / (f32)(u32)data[1];
        if (target > *(f32*)(state + TUMBLEWEED_BLEND_WEIGHT_OFFSET))
        {
            *(f32*)(state + TUMBLEWEED_BLEND_VELOCITY_OFFSET) =
                lbl_803E23E4 * timeDelta + *(f32*)(state + TUMBLEWEED_BLEND_VELOCITY_OFFSET);
            *(f32*)(state + TUMBLEWEED_BLEND_WEIGHT_OFFSET) =
                *(f32*)(state + TUMBLEWEED_BLEND_VELOCITY_OFFSET) * timeDelta +
                *(f32*)(state + TUMBLEWEED_BLEND_WEIGHT_OFFSET);
            if (*(f32*)(state + TUMBLEWEED_BLEND_WEIGHT_OFFSET) > (max = lbl_803E23E8))
            {
                *(f32*)(state + TUMBLEWEED_BLEND_VELOCITY_OFFSET) = lbl_803E23DC;
                *(f32*)(state + TUMBLEWEED_BLEND_WEIGHT_OFFSET) = max;
            }
            else if (*(f32*)(state + TUMBLEWEED_BLEND_WEIGHT_OFFSET) > target)
            {
                if (*(f32*)(state + TUMBLEWEED_BLEND_VELOCITY_OFFSET) < lbl_803E23EC)
                {
                    *(f32*)(state + TUMBLEWEED_BLEND_VELOCITY_OFFSET) = lbl_803E23DC;
                    *(f32*)(state + TUMBLEWEED_BLEND_WEIGHT_OFFSET) = target;
                }
                else
                {
                    *(f32*)(state + TUMBLEWEED_BLEND_VELOCITY_OFFSET) =
                        *(f32*)(state + TUMBLEWEED_BLEND_VELOCITY_OFFSET) * lbl_803E23F0;
                }
            }
        }
        else if (target < *(f32*)(state + TUMBLEWEED_BLEND_WEIGHT_OFFSET))
        {
            *(f32*)(state + TUMBLEWEED_BLEND_VELOCITY_OFFSET) =
                *(f32*)(state + TUMBLEWEED_BLEND_VELOCITY_OFFSET) - lbl_803E23E4 * timeDelta;
            *(f32*)(state + TUMBLEWEED_BLEND_WEIGHT_OFFSET) =
                *(f32*)(state + TUMBLEWEED_BLEND_VELOCITY_OFFSET) * timeDelta +
                *(f32*)(state + TUMBLEWEED_BLEND_WEIGHT_OFFSET);
            if (*(f32*)(state + TUMBLEWEED_BLEND_WEIGHT_OFFSET) < *(f32*)&lbl_803E23DC)
            {
                *(f32*)(state + TUMBLEWEED_BLEND_WEIGHT_OFFSET) = *(f32*)(state + TUMBLEWEED_BLEND_VELOCITY_OFFSET) =
                    lbl_803E23DC;
            }
            if (*(f32*)(state + TUMBLEWEED_BLEND_WEIGHT_OFFSET) < target)
            {
                if (*(f32*)(state + TUMBLEWEED_BLEND_VELOCITY_OFFSET) > lbl_803E23F4)
                {
                    *(f32*)(state + TUMBLEWEED_BLEND_VELOCITY_OFFSET) = lbl_803E23DC;
                    *(f32*)(state + TUMBLEWEED_BLEND_WEIGHT_OFFSET) = target;
                }
                else
                {
                    *(f32*)(state + TUMBLEWEED_BLEND_VELOCITY_OFFSET) =
                        *(f32*)(state + TUMBLEWEED_BLEND_VELOCITY_OFFSET) * lbl_803E23F0;
                }
            }
        }
        ObjModel_SetBlendChannelWeight(Obj_GetActiveModel((GameObject*)obj), 1,
                                       lbl_803E23F8 * *(f32*)(state + TUMBLEWEED_BLEND_WEIGHT_OFFSET) - lbl_803E23E8);
    }
}

void tricky_updateModelVariantFade(int obj, int state)
{
    u8 ratio = (u8)((s32) * (u8*)(*(int*)(state + 0) + 2) / 10);

    if (((TrickyState*)state)->modelVariant != ratio)
    {
        f32 t;
        if (mainGetBit(1005) == 0)
        {
            mainSetBits(1005, 1);
            (*gObjectTriggerInterface)->runSequence(5, (void*)obj, -1);
            ((TrickyState*)state)->stateFlags |= 0x4000;
            *(f32*)(state + 0x828) = *(f32*)(state + 0x828) + lbl_803E2408;
        }
        *(f32*)(state + 0x828) = *(f32*)(state + 0x828) - timeDelta;
        t = *(f32*)(state + 0x828);
        if (!(t > lbl_803E2408))
        {
            if (t > lbl_803E23DC)
            {
                f32 alpha;
                if (t > lbl_803E23E0)
                {
                    alpha = lbl_803E23E8 - (t - lbl_803E23E0) / lbl_803E23E0;
                }
                else
                {
                    Obj_GetActiveModel((GameObject*)obj)->textureRefs->swapSelector = ratio;
                    alpha = *(f32*)(state + 0x828) / lbl_803E23E0;
                }
                Obj_SetModelColorOverrideRecursive((GameObject*)obj, 255, 255, 255, lbl_803E240C * alpha, 1);
            }
            else
            {
                ((TrickyState*)state)->modelVariant = ratio;
                Obj_SetModelColorOverrideRecursive((GameObject*)obj, 0, 0, 0, 0, 0);
            }
        }
    }
}

/* Set bit 0x80000000 of obj->_b8->_54
 * and store lbl_803E2408 into obj->_b8->_808. */
void trickyImpress(GameObject* obj)
{
    TrickyImpressState* b = ((GameObject*)obj)->extra;
    b->flags54 |= 0x80000000;
    b->unk808 = lbl_803E2408;
}
/* GameBit-gated bit toggle on obj->_b8->_54: requires mainGetBit(GAMEBIT_Tricky_Usable); sets bit 0x10000 then
 * checks bit 0x10. Returns 1 only when the post-OR check passes. */
int trickyFn_80138f14(GameObject* obj)
{
    TrickyImpressState* b = obj->extra;
    if ((u32)mainGetBit(GAMEBIT_Tricky_Usable) != 0u)
    {
        b->flags54 |= 0x10000LL;
        if ((b->flags54 & 0x10) != 0u)
        {
            return 1;
        }
    }
    return 0;
}

PPCWGPipe GXWGFifo : (0xCC008000);

f32 trickyGetAnimSpeed(GameObject* obj)
{
    return ((TrickyImpressState*)obj->extra)->animSpeed;
}

GameObject* trickyGetStayPoint(GameObject* obj)
{
    return ((TrickyImpressState*)obj->extra)->stayPoint;
}
int trickyGetAimPitchOffset(GameObject* obj)
{
    return ((TrickyImpressState*)obj->extra)->unk414;
}
void* trickyGetQueuedPathParticlePos(GameObject* obj)
{
    return &((TrickyImpressState*)obj->extra)->renderPosX;
}

GameObject* trickyFindNearestUsableBaddie(GameObject* origin, f32 maxRadius, int allowSpecialTypes)
{
    int* objs;
    int* tmpList;
    GameObject* closest;
    int i;
    f32 bestDistSq;
    int count;

    bestDistSq = maxRadius;
    closest = 0;
    tmpList = (int*)ObjGroup_GetObjects(3, &count);
    bestDistSq = bestDistSq * bestDistSq;
    i = 0;
    objs = tmpList;

    for (; i < count; objs++, i++)
    {
        int* data;
        f32 obj_extra;
        int v1, v2;
        s32 g1, g2;

        if (dll_19_func1B((GameObject*)(*objs)) != 0)
        {
            obj_extra = (*gBaddieControlInterface)->getHealthFraction((GameObject*)*objs);
        }
        else
        {
            obj_extra = enemy_getHealthFraction((GameObject*)*objs);
        }

        data = (int*)((GameObject*)*objs)->anim.placementData;
        g1 = *(s16*)((char*)data + 0x18);
        if (g1 == -1)
        {
            v1 = 0;
        }
        else
        {
            v1 = mainGetBit(g1);
        }
        g2 = *(s16*)((char*)data + 0x1a);
        if (g2 == -1)
        {
            v2 = 1;
        }
        else
        {
            v2 = mainGetBit(g2);
        }

        if (ObjGroup_ContainsObject(*objs, TRICKY_BADDIE_TARGET_OBJGROUP) == 0 && obj_extra > lbl_803E23DC && v1 == 0 &&
            v2 != 0)
        {
            if (((GameObject*)*objs)->anim.seqId != TRICKY_SEQID_WHIRLPOOL)
            {
                if ((*gMapEventInterface)->shouldNotSaveTime(*(int*)((char*)data + 0x14)) != 0)
                {
                    if (allowSpecialTypes == 0)
                    {
                        s16 m = ((GameObject*)*objs)->anim.seqId;
                        if (m == TRICKY_SEQID_VAMBAT || m == TRICKY_SEQID_WB || m == TRICKY_SEQID_SC_BABYLIGHT ||
                            m == TRICKY_SEQID_PINPON)
                            continue;
                    }
                    {
                        f32 dist = vec3f_distanceSquared(&origin->anim.worldPosX, &((GameObject*)*objs)->anim.worldPosX);
                        if (dist < bestDistSq)
                        {
                            bestDistSq = dist;
                            closest = (GameObject*)*objs;
                        }
                    }
                }
            }
        }
    }
    return closest;
}

void Tricky_emitQueuedPathParticles(u8* a, u8* b)
{
    struct
    {
        s16 hx, hy, hz;
        f32 fk;
        f32 dx, dy, dz;
    } stk;
    u8 i = 0x14;
    u32 flags = ((TrickyImpressState*)b)->flags54;
    if ((flags & 0x1800) == 0)
        return;
    stk.dx = ((TrickyImpressState*)b)->renderPosX - ((GameObject*)a)->anim.worldPosX;
    stk.dy = ((TrickyImpressState*)b)->renderPosY - ((GameObject*)a)->anim.worldPosY;
    stk.dz = ((TrickyImpressState*)b)->renderPosZ - ((GameObject*)a)->anim.worldPosZ;
    stk.fk = lbl_803E23E8;
    stk.hx = ((GameObject*)a)->anim.rotX;
    stk.hy = ((GameObject*)a)->anim.rotY;
    stk.hz = ((GameObject*)a)->anim.rotZ;
    if ((flags & 0x800) == 0)
    {
        while (i-- != 0)
        {
            (*gPartfxInterface)->spawnObject(a, TRICKY_PATH_PARTFX, &stk, 2, -1, NULL);
        }
        ((TrickyImpressState*)b)->flags54 = ((TrickyImpressState*)b)->flags54 & ~0x1000LL;
    }
}
int trickySelectQueuedCommandTarget(TrickyState* state, int commandType)
{
    f32 bestPriorityDist;
    f32 bestFallbackDist;
    int ref;
    int i;
    GameObject* bestPriorityTarget;
    GameObject* bestFallbackTarget;

    bestPriorityDist = lbl_803E2418;
    bestPriorityTarget = NULL;
    bestFallbackDist = bestPriorityDist;
    bestFallbackTarget = NULL;

    for (i = 0, ref = (int)state; i < state->commandCount; ref += 8, i++)
    {
        if (*(s8*)(ref + 0x74d) == commandType)
        {
            f32 dist = getXZDistance(&state->playerObj->anim.worldPosX,
                                     &((GameObject*)*(int*)(ref + 0x748))->anim.worldPosX);

            if (*(s8*)(ref + 0x74c) == 1)
            {
                if (dist < bestPriorityDist)
                {
                    bestPriorityDist = dist;
                    bestPriorityTarget = (GameObject*)*(int*)(ref + 0x748);
                }
            }
            else if (dist < bestFallbackDist)
            {
                bestFallbackDist = dist;
                bestFallbackTarget = (GameObject*)*(int*)(ref + 0x748);
            }
        }
    }

    if (bestPriorityTarget != NULL)
    {
        state->followObj = bestPriorityTarget;
    }
    else
    {
        if (bestFallbackTarget == NULL)
        {
            return 0;
        }
        state->followObj = bestFallbackTarget;
    }

    {
        f32* targetPos = &state->followObj->anim.worldPosX;
        if (state->targetPosPtr != targetPos)
        {
            state->targetPosPtr = targetPos;
            {
                u32 m;
                u32 f2 = state->stateFlags;
                m = ~TRICKY_STATE_FLAG_PATH_PATCHES_VALID;
                state->stateFlags = f2 & m;
            }
            state->linkedWalkGroup = 0;
        }
    }

    state->substate = 0;
    return 1;
}

/*
 * Tricky per-frame collision, ground
 * snapping and path-control update.
 *
 * trickyUpdateCollisionAndPathState snaps Tricky to the ground, applies water
 * buoyancy, processes priority hits (lighting fx, hit sparks, out-of-water
 * bark), then drives the path-control interface and copies the resulting
 * yaw/roll back onto the object. trickyAdvanceRouteTargetAhead walks the
 * RomCurve route target forward and trickyTurnTowardYaw eases the object's
 * facing toward a requested yaw.
 */


/* group owned by another DLL, queried here */
#define SIDEREPEL_OBJGROUP      0x40 /* DLL 0xEB siderepel */
#define SKEETLA_TARGET_OBJGROUP 5

/* Per-node fan-out limit: status[]/bestDistances[]/outRoutes[] hold at most
 * this many linked route candidates (status[8] / f32 bestDistances[8]). */
#define TRICKY_ROUTE_CANDIDATE_COUNT 8

#define SKEETLA_LINKED_SOURCE_ID_OBJ_A 0x1ca
#define SKEETLA_LINKED_SOURCE_ID_OBJ_B 0x160
#define SKEETLA_PARTICLE_SPARK_A       0xca
#define SKEETLA_PARTICLE_SPARK_B       0xcb

/* attacker seqId that triggers the staff-impact sfx (retail OBJECTS.bin). */
#define SKEETLA_ATTACKER_SEQID_STAFF 0x69 /* "staff" (DLL 0xE2) */
#define SKEETLA_PARTICLE_SPAWN_FLAGS   0x200001
#define SKEETLA_PARTICLE_RANDOM_RATE   4


void trickyUpdateCollisionAndPathState(u8* obj)
{
    TrickyState* state;
    f32 hitOffsetY;
    void* lastContactObj;
    f32 nearestDistance;
    f32 hitPos[3];
    f32 lightArgs[3];
    f32* hitPosPtr;
    u8 doGroundSnap;
    int doHeightSnap;
    int hitKind;

    state = (TrickyState*)((GameObject*)obj)->extra;
    doGroundSnap = 0;
    nearestDistance = lbl_803E2424;

    if ((objPosToMapBlockIdx(((GameObject*)obj)->anim.worldPosX, ((GameObject*)obj)->anim.worldPosY,
                             ((GameObject*)obj)->anim.worldPosZ) == -1) &&
        ((state->stateFlags & 0x80000) == 0))
    {
        state->heightUpdateActive = 0;
        ((GameObject*)obj)->anim.localPosX = ((GameObject*)obj)->anim.previousLocalPosX;
        ((GameObject*)obj)->anim.localPosY = ((GameObject*)obj)->anim.previousLocalPosY;
        ((GameObject*)obj)->anim.localPosZ = ((GameObject*)obj)->anim.previousLocalPosZ;
    }

    state->stateFlags &= ~0x80000LL;

    if (state->groundSnapCounter != 0)
    {
        state->groundSnapCounter -= 1;
        doGroundSnap = 1;
    }
    else if ((state->stateFlags & 0x2000) != 0)
    {
        doGroundSnap = 1;
    }

    if (doGroundSnap != 0)
    {
        hitDetectFn_800658a4((GameObject*)obj, ((GameObject*)obj)->anim.worldPosX, ((GameObject*)obj)->anim.worldPosY,
                             ((GameObject*)obj)->anim.worldPosZ, &hitOffsetY, 0);
        ((GameObject*)obj)->anim.localPosY -= hitOffsetY;
        state->heightUpdateActive = 0;
    }

    if (((s8)state->heightUpdateActive != 0) && (((state->statusFlags >> 5) & 1) == 0u))
    {
        if (lbl_803E23DC == state->waterLevel)
        {
            doHeightSnap = 0;
        }
        else if (lbl_803E2410 == state->eventTime)
        {
            doHeightSnap = 1;
        }
        else if (state->currentTime - state->eventTime > lbl_803E2414)
        {
            doHeightSnap = 1;
        }
        else
        {
            doHeightSnap = 0;
        }

        if (doHeightSnap != 0)
        {
            ((GameObject*)obj)->anim.velocityY = *(f32*)&lbl_803E23DC;
            ((GameObject*)obj)->anim.localPosY = state->currentTime - lbl_803E23EC;
        }
        else
        {
            ((GameObject*)obj)->anim.velocityY += lbl_803E2428 * timeDelta;
            ((GameObject*)obj)->anim.localPosY += ((GameObject*)obj)->anim.velocityY * timeDelta;
        }
    }
    else
    {
        ((GameObject*)obj)->anim.velocityY = lbl_803E23DC;
    }

    lastContactObj = (void*)((GameObject*)obj)->anim.hitReactState->activeHit;
    if ((((GameObject*)obj)->anim.hitReactState->flags & OBJHITS_PRIORITY_STATE_PAIR_RESPONSE_APPLIED) == 0 ||
        (((GameObject*)lastContactObj)->anim.seqId == 0x1f))
    {
        lastContactObj = NULL;
    }

    if ((state->stateFlags & 8) != 0)
    {
        state->contactTimer += timeDelta;
        if (state->contactTimer >= lbl_803E242C)
        {
            if (vec3f_distanceSquared((f32*)(obj + 0x18), &Obj_GetPlayerObject()->anim.worldPosX) > lbl_803E2430)
            {
                state->contactTimer -= lbl_803E242C;
                ((GameObject*)obj)->anim.modelInstance->runtimeSourceHitMask = 0x7f;
                state->stateFlags &= ~8LL;
            }
        }
    }
    else if ((state->lastContactObj != NULL) && (lastContactObj == state->lastContactObj))
    {
        state->contactTimer += timeDelta;
        if (state->contactTimer >= *(f32*)&lbl_803E23E0)
        {
            state->contactTimer -= lbl_803E23E0;
            state->stateFlags |= 8;
            ((GameObject*)obj)->anim.modelInstance->runtimeSourceHitMask = 0x7e;
        }
    }
    else
    {
        state->contactTimer = lbl_803E23DC;
    }

    state->lastContactObj = lastContactObj;
    hitKind = ObjHits_PollPriorityHitWithCooldown((GameObject*)obj, &state->hitCooldown, (int*)&lastContactObj,
                                                  (hitPosPtr = hitPos));
    state->light = hitKind;

    switch (state->light)
    {
    case 1:
    case 2:
    case 4:
    case 5:
    case 0xe:
    case 0xf:
    case 0x11:
    case 0x13:
        objLightFn_8009a1dc(obj, lbl_803E2434, lightArgs, 1, 0);
        break;
    case 7:
    case 8:
    case 9:
    case 0xa:
    case 0xb:
    case 0xc:
        objfx_spawnHitEmitterAtPos(hitPosPtr, 8, 0xff, 0x20, 0x20);
        objLightFn_8009a1dc(obj, lbl_803E2434, lightArgs, 4, 0);
        if (((GameObject*)lastContactObj)->anim.seqId == SKEETLA_ATTACKER_SEQID_STAFF)
        {
        Sfx_PlayFromObject((u32)obj, SFXTRIG_stftest_var);
        }
        break;
    case 0x1f:
        state->particleTimer = lbl_803E2438;
        break;
    }

    if ((s8)state->heightUpdateActive == 0)
    {
        (*gPathControlInterface)->attachObject(obj, &state->pathControlFlags);
    }

    if ((coordsToMapCell(((GameObject*)obj)->anim.localPosX, ((GameObject*)obj)->anim.localPosZ) == 0xe) ||
        ((u32)ObjGroup_FindNearestObject(SKEETLA_TARGET_OBJGROUP, (GameObject*)obj, &nearestDistance) != 0u))
    {
        state->pathControlFlags &= ~4;
    }
    else
    {
        state->pathControlFlags |= 4;
    }

    (*gPathControlInterface)->update(obj, &state->pathControlFlags, timeDelta);
    (*gPathControlInterface)->apply(obj, &state->pathControlFlags);
    (*gPathControlInterface)->advance(obj, &state->pathControlFlags, timeDelta);

    ((GameObject*)obj)->anim.rotY = state->pathRotY;
    ((GameObject*)obj)->anim.rotZ = state->pathRotZ;
}

int trickyAdvanceRouteTargetAhead(int obj, RomCurveWalker* route, f32 speed)
{
    f32 limit;
    f32 maxSq, dist, step;
    int iter;
    int result;
    f32 maxDist;

    result = 0;
    maxDist = lbl_803E244C * (speed * timeDelta);
    maxSq = maxDist * maxDist;
    dist = getXZDistance(&route->posX, (f32*)(obj + 0x18));
    if (route->reverse != 0)
    {
        step = lbl_803E2448;
    }
    else
    {
        step = lbl_803E23F8;
    }
    iter = 0;
    limit = lbl_803E2424;
    for (; iter < 5; iter++)
    {
        if (dist > limit && maxSq < dist)
        {
            return result;
        }
        result = 1;
        RomCurve_stepClamped(route, step);
        dist = getXZDistance(&route->posX, (f32*)(obj + 0x18));
    }
    return 1;
}

int trickyTurnTowardYaw(u8* obj, s16 targetYaw)
{
    u8* state;
    int currentYaw;
    int delta;
    int step;

    state = ((GameObject*)obj)->extra;
    ((TrickyState*)state)->targetYaw = targetYaw;

    delta = (u16)(s16)targetYaw;
    currentYaw = ((GameObject*)obj)->anim.rotX;
    delta = currentYaw - delta;
    if (delta > 0x8000)
    {
        delta -= 0xffff;
    }
    if (delta < -0x8000)
    {
        delta += 0xffff;
    }

    if ((((TrickyState*)state)->stateFlags & 0x100000) != 0)
    {
        ((TrickyState*)state)->stateFlags |= 0x200000LL;
    }
    else
    {
        ((TrickyState*)state)->stateFlags &= ~0x200000LL;
    }
    ((TrickyState*)state)->stateFlags &= 0xef2fffff;

    if (delta > 0x10)
    {
        ((TrickyState*)state)->stateFlags |= 0x900000LL;
    }
    else if (delta < -0x10)
    {
        ((TrickyState*)state)->stateFlags |= 0x500000LL;
    }
    else
    {
        ((GameObject*)obj)->anim.rotX = targetYaw;
        return 0;
    }

    if (delta > 0x200)
    {
        step = (s32)(lbl_803E2450 * timeDelta);
        ((GameObject*)obj)->anim.rotX = currentYaw - step;
        ((TrickyState*)state)->stateFlags |= 0x10000000LL;
    }
    else if (delta < -0x200)
    {
        step = (s32)(lbl_803E2450 * timeDelta);
        ((GameObject*)obj)->anim.rotX = currentYaw + step;
        ((TrickyState*)state)->stateFlags |= 0x10000000LL;
    }
    else
    {
        ((GameObject*)obj)->anim.rotX = targetYaw;
    }

    return delta;
}

/*
 * Tricky steering, animation selection and RomCurve route walking.
 *
 * trickyMove steers toward a target point with object-avoidance
 * (trickyApplyObjectAvoidanceToStep) and picks a walk/run/turn anim plus
 * footstep sfx by speed. The RomCurve helpers (trickySelectRouteEntry and
 * friends) choose and walk the spline route Tricky follows, gated by game
 * bits on each curve. skeetla_spawnLinkedSparks emits the contact-spark
 * particles for the object Tricky is linked to.
 */

f32 lbl_803DBC40[2] = {0.05f, 8.5f};
f32 lbl_803DBC48 = 8.0f;
char sSkeetlaVelDebugFmt[] = "Vel %f\n";

/* group owned by another DLL, queried here */

/* Per-node fan-out limit: status[]/bestDistances[]/outRoutes[] hold at most
 * this many linked route candidates (status[8] / f32 bestDistances[8]). */


/* attacker seqId that triggers the staff-impact sfx (retail OBJECTS.bin). */


static inline int skeetla_isInWater(u8* state)
{
    if (lbl_803E23DC == ((TrickyState*)state)->waterLevel)
    {
        return 0;
    }
    if (lbl_803E2410 == ((TrickyState*)state)->eventTime)
    {
        return 1;
    }
    if ((((TrickyState*)state)->currentTime - ((TrickyState*)state)->eventTime) > lbl_803E2414)
    {
        return 1;
    }
    return 0;
}

static inline f32 skeetla_pathSpeedDelta(u8* obj)
{
    TrickyState* state = (TrickyState*)((GameObject*)obj)->extra;
    f32* currentPathPoint;
    f32 dx;
    f32 dz;
    f32 previousSpeed;
    f32 currentSpeed;

    currentPathPoint = state->targetPosPtr;
    if (state->targetPosPtr == state->previousPathPoint)
    {
        dx = state->previousPathX - ((GameObject*)obj)->anim.worldPosX;
        dz = state->previousPathZ - ((GameObject*)obj)->anim.worldPosZ;
        previousSpeed = oneOverTimeDelta * sqrtf((dx * dx) + (dz * dz));

        dx = currentPathPoint[0] - ((GameObject*)obj)->anim.worldPosX;
        dz = currentPathPoint[2] - ((GameObject*)obj)->anim.worldPosZ;
        currentSpeed = oneOverTimeDelta * sqrtf((dx * dx) + (dz * dz));
        return currentSpeed - previousSpeed;
    }
    return lbl_803E23DC;
}

static inline void skeetla_updateFacingFromMoveVector(u8* obj, s16* turnDeltaOut)
{
    u8* state;
    int yaw;

    state = ((GameObject*)obj)->extra;
    if (((((TrickyState*)state)->dirX * ((TrickyState*)state)->dirX) +
         (((TrickyState*)state)->dirZ * ((TrickyState*)state)->dirZ)) > lbl_803E23EC)
    {
        yaw = (s16)getAngle(-((TrickyState*)state)->dirX, -((TrickyState*)state)->dirZ);
        *turnDeltaOut = trickyTurnTowardYaw(obj, yaw);
        ((TrickyState*)state)->dirX = -mathSinf((lbl_803E2454 * (f32)(int)*(s16*)obj) / lbl_803E2458);
        ((TrickyState*)state)->dirZ = -mathCosf((lbl_803E2454 * (f32)(int)*(s16*)obj) / lbl_803E2458);
    }
}

static inline void skeetla_playFootstepSfx(u8* obj, u16 sfxId)
{
    u8* state = ((GameObject*)obj)->extra;
    if (((((TrickyState*)((GameObject*)obj)->extra)->statusFlags >> 6) & 1) == 0u &&
        ((((GameObject*)obj)->anim.currentMove >= 0x30) || (((GameObject*)obj)->anim.currentMove < 0x29)) &&
        (Sfx_IsPlayingFromObjectChannel((int)obj, 0x10) == 0))
    {
        objAudioFn_800393f8((GameObject*)obj, &((TrickyState*)state)->soundState, sfxId, 0x500, -1, 0);
    }
}

int trickyMove(GameObject* obj, f32* targetPos)
{
    f32 prospectivePos[3];
    f32 adjustedPos[3];
    u16 sfxIds[3];
    u16 sfxId;
    char* debugStrings;
    TrickyState* state;
    f32 moveSpeed;
    f32 length;
    s16 previousYaw;
    int td;
    s16 turnDelta;
    int animId;
    u32 f;

    debugStrings = lbl_8031D2E8;
    state = obj->extra;
    moveSpeed = state->speed;
    trickyDebugPrint(sSkeetlaVelDebugFmt, moveSpeed);

    state->dirX = targetPos[0] - obj->anim.worldPosX;
    state->dirZ = targetPos[2] - obj->anim.worldPosZ;
    length = sqrtf((state->dirX * state->dirX) +
                   (state->dirZ * state->dirZ));
    if (lbl_803E23DC != length)
    {
        state->dirX /= length;
        state->dirZ /= length;
    }

    if (moveSpeed < lbl_803E2420)
    {
        f32 stepX;
        f32 stepZ;
        stepX = lbl_803E2420 * state->dirX;
        prospectivePos[0] = stepX * timeDelta + obj->anim.worldPosX;
        prospectivePos[1] = obj->anim.worldPosY;
        stepZ = lbl_803E2420 * state->dirZ;
        prospectivePos[2] = stepZ * timeDelta + obj->anim.worldPosZ;
    }
    else
    {
        prospectivePos[0] = timeDelta * (state->dirX * moveSpeed) + obj->anim.worldPosX;
        prospectivePos[1] = obj->anim.worldPosY;
        prospectivePos[2] = timeDelta * (state->dirZ * moveSpeed) + obj->anim.worldPosZ;
    }

    adjustedPos[0] = prospectivePos[0];
    adjustedPos[1] = prospectivePos[1];
    adjustedPos[2] = prospectivePos[2];
    trickyApplyObjectAvoidanceToStep(&obj->anim.worldPosX, adjustedPos, targetPos);
    if (vec3f_distanceSquared(prospectivePos, adjustedPos) > lbl_803E2468)
    {
        state->dirX = adjustedPos[0] - obj->anim.worldPosX;
        state->dirZ = adjustedPos[2] - obj->anim.worldPosZ;
        length = sqrtf((state->dirX * state->dirX) +
                       (state->dirZ * state->dirZ));
        if (lbl_803E23DC != length)
        {
            state->dirX /= length;
            state->dirZ /= length;
        }
    }

    if (moveSpeed >= lbl_803E2420)
    {
        skeetla_updateFacingFromMoveVector((u8*)obj, &turnDelta);
        if (skeetla_isInWater((u8*)state) != 0)
        {
            objAnimFn_8013a3f0((int)obj, 7, lbl_803E2468, 0x2000000);
            state->cooldownC = lbl_803E2440;
            state->particleTimer = lbl_803E23DC;
            trickyDebugPrint(debugStrings + 0x184);
        }
        else
        {
            if (state->stateIndex == 1)
            {
                if ((skeetla_pathSpeedDelta((u8*)obj) >= lbl_803E23DC ? skeetla_pathSpeedDelta((u8*)obj)
                                                                      : -skeetla_pathSpeedDelta((u8*)obj)) > lbl_803E23DC)
                {
                    state->sfxIntervalTimer -= timeDelta;
                    if (state->sfxIntervalTimer <= lbl_803E23DC)
                    {
                        state->sfxIntervalTimer = (f32)(int)randomGetRange(600, 1200);
                        if (Sfx_IsPlayingFromObjectChannel((int)obj, 0x10) == 0)
                        {
                            if (moveSpeed > lbl_803E23E8)
                            {
                                sfxId = randomGetRange(0x34d, 0x34e);
                                skeetla_playFootstepSfx((u8*)obj, sfxId);
                            }
                            else
                            {
                                *(u32*)sfxIds = gSkeetlaFootstepSfxIds01;
                                sfxIds[2] = gSkeetlaFootstepSfxId2;
                                if (mainGetBit(GAMEBIT_ITEM_TrickyBall_Bought) != 0)
                                {
                                    randomGetRange(0, 2);
                                }
                                else
                                {
                                    randomGetRange(0, 1);
                                }
                                sfxId = sfxIds[randomGetRange(0, 2)];
                                skeetla_playFootstepSfx((u8*)obj, sfxId);
                            }
                        }
                    }
                }
            }

            if (moveSpeed > lbl_803E246C)
            {
                state->voiceCooldown = lbl_803E2440;
                objAnimFn_8013a3f0((int)obj, 0x30, lbl_803E2468, 0x3000000);
            }
            else if (moveSpeed > lbl_803E23E8)
            {
                objAnimFn_8013a3f0((int)obj, 5, lbl_803E2468, 0x3000000);
            }
            else if (moveSpeed > lbl_803E2470)
            {
                objAnimFn_8013a3f0((int)obj, 4, lbl_803E2468, 0x3000000);
            }
            else if (moveSpeed > lbl_803E2474)
            {
                objAnimFn_8013a3f0((int)obj, 2, lbl_803E2468, 0x3000000);
            }
            else
            {
                objAnimFn_8013a3f0((int)obj, 1, lbl_803E2468, 0x3000000);
            }
            trickyDebugPrint(debugStrings + 0x1a0);
        }
    }
    else
    {
        previousYaw = obj->anim.rotX;
        turnDelta = 0;
        skeetla_updateFacingFromMoveVector((u8*)obj, &turnDelta);
        td = turnDelta;

        if ((state->stateFlags & 0x100000) != 0)
        {
            if (skeetla_isInWater((u8*)state) != 0)
            {
                trickyDebugPrint(debugStrings + 0x1bc);
                objAnimFn_8013a3f0((int)obj, 8, lbl_803E243C, 0);
                state->cooldownC = lbl_803E2440;
                state->particleTimer = lbl_803E23DC;
            }
            else
            {
                u32 flags;
                trickyDebugPrint(debugStrings + 0x1d0);
                flags = state->stateFlags;
                if ((flags & 0x400000) != 0)
                {
                    td = td >= 0 ? td : -td;
                    if (td > 0x3555)
                    {
                        animId = 0x27;
                    }
                    else
                    {
                        td = td >= 0 ? td : -td;
                        if (td > 0x2000)
                        {
                            animId = 0xb;
                        }
                        else
                        {
                            animId = 9;
                        }
                    }
                }
                else if ((flags & 0x800000) != 0)
                {
                    td = td >= 0 ? td : -td;
                    if (td > 0x3555)
                    {
                        animId = 0x28;
                    }
                    else
                    {
                        td = td >= 0 ? td : -td;
                        if (td > 0x2000)
                        {
                            animId = 0xc;
                        }
                        else
                        {
                            animId = 10;
                        }
                    }
                }
                obj->anim.rotX = previousYaw;
                objAnimFn_8013a3f0((int)obj, animId, lbl_803E2478, 0x1000100);
            }
        }

        state->speed = lbl_803E2420;
        f = state->stateFlags;
        if (((f & 0x100000) == 0) && ((f & 0x200000) == 0))
        {
            return 0;
        }
    }
    return 1;
}

int objAnimFn_8013a3f0(int obj, int newState, f32 speed, u32 flags)
{
    int t = *(int*)&((GameObject*)obj)->extra;
    f32 fz;
    if (((TrickyState*)t)->moveId == newState)
    {
        if (((GameObject*)obj)->anim.currentMove == newState)
        {
            ((TrickyState*)t)->moveProgress = speed;
            ((TrickyState*)t)->stateFlags = ((TrickyState*)t)->stateFlags | flags;
        }
        return 1;
    }
    if ((flags & 0x4000000) != 0)
    {
        ((TrickyState*)t)->animTransitionTimer = lbl_803E247C;
    }
    ((TrickyState*)t)->moveId = newState;
    ((TrickyState*)t)->moveProgressTarget = speed;
    ((TrickyState*)t)->pendingStateFlags = flags;
    if ((flags & 0x20) == 0)
    {
        ((TrickyState*)t)->stateFlags = ((TrickyState*)t)->stateFlags & ~(u64)0x20;
    }
    if ((flags & 0x40) == 0)
    {
        ((TrickyState*)t)->stateFlags = ((TrickyState*)t)->stateFlags & ~(u64)0x40;
    }
    if ((flags & 0x80) == 0)
    {
        ((TrickyState*)t)->stateFlags = ((TrickyState*)t)->stateFlags & ~(u64)0x80;
    }
    if ((flags & 0x100) == 0)
    {
        ((TrickyState*)t)->stateFlags = ((TrickyState*)t)->stateFlags & ~(u64)0x100;
    }
    fz = lbl_803E23E8;
    ((TrickyState*)t)->sidestepDelta = fz;
    ((TrickyState*)t)->backstepDelta = fz;
    ((TrickyState*)t)->verticalDelta = fz;
    ((TrickyState*)t)->rotStepScale = fz;
    if (((TrickyState*)t)->animTransitionTimer >= lbl_803E247C)
    {
        return 1;
    }
    return 0;
}

static inline void* skeetla_validateRouteEntry(void* entry)
{
    if (entry == NULL)
    {
        return NULL;
    }
    if (((((ObjfsaRomCurveDef*)entry)->requiredBit == -1) ||
         (mainGetBit(((ObjfsaRomCurveDef*)entry)->requiredBit) != 0)) &&
        ((((ObjfsaRomCurveDef*)entry)->forbiddenBit == -1) ||
         (mainGetBit(((ObjfsaRomCurveDef*)entry)->forbiddenBit) == 0)))
    {
        return entry;
    }

    return NULL;
}


void* trickyFindNearestLinkedRouteEntry(u8* context, u8* routeDef, int linkSelector, int routeFlagValue)
{
    void* candidates[4];
    void* entry;
    f32 bestDistance;
    f32 distance;
    u16 mask;
    u16 i;
    u16 count;
    u16 bestIndex;
    int curveId;
    s16 requiredBit;
    s16 forbiddenBit;

    i = 0;
    count = 0;
    mask = 1;
    while (i < 4)
    {
        curveId = ((ObjfsaRomCurveDef*)routeDef)->linkIds[i];
        if ((curveId > -1) && (((((ObjfsaRomCurveDef*)routeDef)->blockedLinkMask & mask) ^ routeFlagValue) == 0))
        {
            candidates[count] = (*gRomCurveInterface)->getById(curveId);
            if (candidates[count] != NULL)
            {
                entry = candidates[count];
                if ((linkSelector == 0) || (((ObjfsaRomCurveDef*)routeDef)->linkSelectors[count] == linkSelector))
                {
                    requiredBit = ((ObjfsaRomCurveDef*)entry)->requiredBit;
                    if ((requiredBit == -1) || (mainGetBit(requiredBit) != 0))
                    {
                        forbiddenBit = ((ObjfsaRomCurveDef*)entry)->forbiddenBit;
                        if ((forbiddenBit == -1) || (mainGetBit(forbiddenBit) == 0))
                        {
                            if ((((ObjfsaRomCurveDef*)routeDef)->unk1A != 9) || (((ObjfsaRomCurveDef*)entry)->unk1A != 8))
                            {
                                count++;
                            }
                        }
                    }
                }
            }
        }
        i++;
        mask <<= 1;
        routeFlagValue <<= 1;
    }

    if (count != 0)
    {
        bestDistance = getXZDistance(&((TrickyState*)context)->playerObj->anim.worldPosX,
                                     (f32*)((u8*)candidates[0] + 8));
        bestIndex = 0;
        for (i = 1; i < count; i++)
        {
            distance = getXZDistance(&((TrickyState*)context)->playerObj->anim.worldPosX,
                                     (f32*)((u8*)candidates[i] + 8));
            if (distance < bestDistance)
            {
                bestDistance = distance;
                bestIndex = i;
            }
        }

        return candidates[bestIndex];
    }
    return NULL;
}

void* trickyFindPathRouteEntry(u8* state, u32 route, int pathId)
{
    if (pathId == 0)
    {
        return NULL;
    }

    if ((((TrickyState*)state)->cachedPathId == pathId) && (*(u32*)&((TrickyState*)state)->cachedRouteEntry == route))
    {
        ((TrickyState*)state)->cachedRouteEntry = pathSearchGetNextPoint((PathSearch*)(state + 0x6b8));
        if (((TrickyState*)state)->cachedRouteEntry == NULL)
        {
            return NULL;
        }

        ((TrickyState*)state)->cachedRouteEntry = skeetla_validateRouteEntry(((TrickyState*)state)->cachedRouteEntry);
        if (((TrickyState*)state)->cachedRouteEntry != NULL)
        {
            return ((TrickyState*)state)->cachedRouteEntry;
        }
    }

    pathSearchBegin((PathSearch*)(state + 0x6b8), (PathPoint*)route,
                ((TrickyState*)state)->targetPosPtr, pathId,
                ((TrickyState*)state)->route.reverse);
    if (pathSearchStep((PathSearch*)(state + 0x6b8), 0x1f4) != 1)
    {
        return NULL;
    }

    pathSearchBuildPath((PathSearch*)(state + 0x6b8));
    ((TrickyState*)state)->cachedRouteEntry = pathSearchGetNextPoint((PathSearch*)(state + 0x6b8));
    ((TrickyState*)state)->cachedPathId = pathId;
    return ((TrickyState*)state)->cachedRouteEntry;
}

int trickyFindReachableRouteIndex(u8* state, void** routes, u8* routeFlags, int pathId)
{
    s8 status[TRICKY_ROUTE_CANDIDATE_COUNT];
    s8 i;
    s8 j;
    s8 failedCount;
    void** wp;
    u8* sp;
    s8* stp;

    for (i = 0, wp = routes, sp = state; i < TRICKY_ROUTE_CANDIDATE_COUNT; i++)
    {
        if (*wp != 0)
        {
            pathSearchBegin((PathSearch*)(sp + 0x538), (PathPoint*)*wp,
                        ((TrickyState*)state)->targetPosPtr, pathId, routeFlags[i]);
        }
        wp++;
        sp += 0x30;
    }

    for (i = 0; i < 100; i++)
    {
        failedCount = 0;
        for (j = 0, wp = routes, sp = state, stp = status; j < TRICKY_ROUTE_CANDIDATE_COUNT; j++)
        {
            if (*wp != 0)
            {
                *stp = pathSearchStep((PathSearch*)(sp + 0x538), 1);
            }
            else
            {
                *stp = -1;
            }

            switch (*stp)
            {
            case 1:
                return j;
            case -1:
                *wp = 0;
                failedCount++;
                break;
            }
            wp++;
            sp += 0x30;
            stp++;
        }

        switch (failedCount)
        {
        case 7:
            for (i = 0, wp = routes; i < TRICKY_ROUTE_CANDIDATE_COUNT; i++)
            {
                if (*wp != 0)
                {
                    status[(int)i] = pathSearchStep((PathSearch*)(state + ((int)i * 0x30 + 0x538)), 0x1f4);
                    if (status[(int)i] == 1)
                    {
                        return i;
                    }
                    return -1;
                }
                wp++;
            }
        case 8:
            return -1;
        }
    }

    return -1;
}

void* trickySelectRouteEntry(u8* state, u8* routeDef, u8 routeFlagValue)
{
    void* entry;

    entry = NULL;

    if ((*(u8**)&((TrickyState*)state)->cachedRouteDef == routeDef) &&
        (((TrickyState*)state)->cachedWalkGroup == ((TrickyState*)state)->walkGroup) &&
        (((TrickyState*)state)->cachedRouteFlags == (routeFlagValue & 0xffu)))
    {
        entry = skeetla_validateRouteEntry(((TrickyState*)state)->validatedRouteEntry);
    }

    if (entry == NULL)
    {
        entry =
            trickyFindNearestLinkedRouteEntry(state, routeDef, ((TrickyState*)state)->walkGroup, routeFlagValue & 0xff);
        if (entry == NULL)
        {
            entry = trickyFindPathRouteEntry(state, (u32)routeDef, ((TrickyState*)state)->walkGroup);
        }

        if (entry == NULL)
        {
            if (((TrickyState*)state)->savedWalkGroup != 0)
            {
                entry = trickyFindNearestLinkedRouteEntry(state, routeDef, ((TrickyState*)state)->savedWalkGroup,
                                                          routeFlagValue & 0xff);
                if (entry == NULL)
                {
                    entry = trickyFindPathRouteEntry(state, (u32)routeDef, ((TrickyState*)state)->savedWalkGroup);
                }
                if (entry != NULL)
                {
                    ((TrickyState*)state)->walkGroup = ((TrickyState*)state)->savedWalkGroup;
                }
            }

            if (entry == NULL)
            {
                entry = trickyFindNearestLinkedRouteEntry(state, routeDef, 0, routeFlagValue & 0xff);
                ((TrickyState*)state)->walkGroup = 0;
            }
        }
    }

    *(u8**)&((TrickyState*)state)->cachedRouteDef = routeDef;
    ((TrickyState*)state)->validatedRouteEntry = entry;
    ((TrickyState*)state)->cachedWalkGroup = ((TrickyState*)state)->walkGroup;
    ((TrickyState*)state)->cachedRouteFlags = routeFlagValue;
    return entry;
}

void trickyRankLinkedRouteCandidates(GameObject* obj, u8* outRouteFlags, s16 linkSelector, void** outRoutes)
{
    f32 bestDistances[TRICKY_ROUTE_CANDIDATE_COUNT];
    int i;
    void** curves;
    void* curve;
    u8 j;
    void* linkedCurve;
    u8 routeFlags;
    f32 cz;
    f32* p;
    f32 score;
    f32 init;
    int count;
    u8 k;
    int linkCurveId;
    TrickyState* state;
    f32* bd;
    void** rp;
    void** cp;

    state = obj->extra;
    curves = (void**)(*gRomCurveInterface)->getCurves(&count);

    init = lbl_803E2418;
    bd = bestDistances;
    rp = outRoutes;
    for (i = 0; i < TRICKY_ROUTE_CANDIDATE_COUNT; i++)
    {
        *bd++ = init;
        *rp++ = NULL;
    }

    if (linkSelector == 0)
    {
        return;
    }

    for (i = 0, cp = curves; i < count; i++)
    {
        curve = *cp++;
        if ((((ObjfsaRomCurveDef*)curve)->type != 0x24) || (*(u8*)((u8*)curve + 3) != 0))
        {
            continue;
        }
        if (((((ObjfsaRomCurveDef*)curve)->requiredBit != -1) &&
             (mainGetBit(((ObjfsaRomCurveDef*)curve)->requiredBit) == 0)) ||
            ((((ObjfsaRomCurveDef*)curve)->forbiddenBit != -1) &&
             (mainGetBit(((ObjfsaRomCurveDef*)curve)->forbiddenBit) != 0)))
        {
            continue;
        }

        cz = ((ObjfsaRomCurveDef*)curve)->z;
        p = state->targetPosPtr;
        {
            f32 sq0 = (p[2] - cz) * (p[2] - cz);
            f32 sq1 = (p[0] - ((ObjfsaRomCurveDef*)curve)->x) * (p[0] - ((ObjfsaRomCurveDef*)curve)->x);
            f32 sq2 = (obj->anim.worldPosX - ((ObjfsaRomCurveDef*)curve)->x) *
                      (obj->anim.worldPosX - ((ObjfsaRomCurveDef*)curve)->x);
            f32 sq3 = (obj->anim.worldPosZ - cz) * (obj->anim.worldPosZ - cz);
            score = sq0 + (sq1 + (sq2 + sq3));
        }
        if (score < bestDistances[7])
        {
            for (j = 0; j < 4; j++)
            {
                linkCurveId = ((ObjfsaRomCurveDef*)curve)->linkIds[j];
                if ((linkCurveId > -1) && (((ObjfsaRomCurveDef*)curve)->linkSelectors[j] == linkSelector))
                {
                    if (((ObjfsaRomCurveDef*)curve)->unk1A == 8)
                    {
                        linkedCurve = (*gRomCurveInterface)->getById(linkCurveId);
                        if ((linkedCurve != NULL) && (((ObjfsaRomCurveDef*)linkedCurve)->unk1A == 9))
                        {
                            continue;
                        }
                    }

                    routeFlags = (u8)(((ObjfsaRomCurveDef*)curve)->blockedLinkMask >> (u8)j);
                    break;
                }
            }

            if (j == 4)
            {
                continue;
            }

            for (j = 0; j < TRICKY_ROUTE_CANDIDATE_COUNT; j++)
            {
                if (score < bestDistances[j])
                {
                    for (k = 7; k > j; k--)
                    {
                        outRouteFlags[k] = outRouteFlags[k - 1];
                        outRoutes[k] = outRoutes[k - 1];
                        bestDistances[k] = bestDistances[k - 1];
                    }

                    outRouteFlags[j] = (routeFlags & 1) ^ 1;
                    outRoutes[j] = curve;
                    bestDistances[j] = score;
                    break;
                }
            }
        }
    }
}

void skeetla_spawnLinkedSparks(u8* obj)
{
    u8* state;
    GameObject* linkedObj;
    SkeetlaParticleSpawnArgs args;

    state = ((GameObject*)obj)->extra;
    linkedObj = ((TrickyState*)state)->followObj;

    args.x = ((TrickyState*)state)->sparkPos0X;
    args.y = ((TrickyState*)state)->sparkPos0Y;
    args.z = ((TrickyState*)state)->sparkPos0Z;
    args.objectId = ((GameObject*)obj)->anim.rotX;
    if (linkedObj->anim.seqId == SKEETLA_LINKED_SOURCE_ID_OBJ_A)
    {
        args.sourceId =
            (u8)(*(u32(**)(u8*))(*(int*)(*(int*)&linkedObj->anim.dll) + 0x28))((u8*)linkedObj);
    }
    else if (linkedObj->anim.seqId == SKEETLA_LINKED_SOURCE_ID_OBJ_B)
    {
        args.sourceId =
            (u8)(*(u32(**)(u8*))(*(int*)(*(int*)&linkedObj->anim.dll) + 0x28))((u8*)linkedObj);
    }
    else
    {
        args.sourceId = 0;
    }

    if ((int)randomGetRange(0, SKEETLA_PARTICLE_RANDOM_RATE) == 0)
    {
        (*gPartfxInterface)->spawnObject(obj, SKEETLA_PARTICLE_SPARK_A, &args, SKEETLA_PARTICLE_SPAWN_FLAGS, -1, NULL);
    }
    if ((int)randomGetRange(0, SKEETLA_PARTICLE_RANDOM_RATE) == 0)
    {
        (*gPartfxInterface)->spawnObject(obj, SKEETLA_PARTICLE_SPARK_B, &args, SKEETLA_PARTICLE_SPAWN_FLAGS, -1, NULL);
    }

    args.x = ((TrickyState*)state)->sparkPos1X;
    args.y = ((TrickyState*)state)->sparkPos1Y;
    args.z = ((TrickyState*)state)->sparkPos1Z;
    args.objectId = ((GameObject*)obj)->anim.rotX;

    if ((int)randomGetRange(0, SKEETLA_PARTICLE_RANDOM_RATE) == 0)
    {
        (*gPartfxInterface)->spawnObject(obj, SKEETLA_PARTICLE_SPARK_A, &args, SKEETLA_PARTICLE_SPAWN_FLAGS, -1, NULL);
    }
    if ((int)randomGetRange(0, SKEETLA_PARTICLE_RANDOM_RATE) == 0)
    {
        (*gPartfxInterface)->spawnObject(obj, SKEETLA_PARTICLE_SPARK_B, &args, SKEETLA_PARTICLE_SPAWN_FLAGS, -1, NULL);
    }
}

void trickyAdjustStepAroundPoint(f32* start, f32* end, f32* guardPoint, f32* center, f32 minDistance, f32 moveDistance)
{
    f32 projection[3];
    f32 dx;
    f32 centerToEnd;
    f32 minDistanceSq;
    f32 limitDistanceSq;
    f32 guardDistance;
    f32 startGuardDistance;
    f32 slope;
    f32 intercept;
    f32 perpSlope;
    f32 dz;
    f32 centerToStart;
    f32 length;
    int useBlendedDistance;

    useBlendedDistance = 0;
    centerToStart = getXZDistance(center, start);
    centerToEnd = getXZDistance(center, end);
    minDistanceSq = minDistance * minDistance;
    limitDistanceSq = moveDistance * moveDistance;

    if (centerToEnd > centerToStart)
    {
        return;
    }

    guardDistance = getXZDistance(guardPoint, center);
    if (guardDistance < minDistanceSq)
    {
        return;
    }

    startGuardDistance = getXZDistance(start, guardPoint);
    if (getXZDistance(start, center) > startGuardDistance)
    {
        return;
    }

    if (centerToStart < limitDistanceSq)
    {
        limitDistanceSq = centerToStart;
        useBlendedDistance = 1;
    }

    if (!(centerToEnd < limitDistanceSq))
    {
        return;
    }

    slope = (end[2] - start[2]) / (end[0] - start[0]);
    dz = start[0] - end[0];
    intercept = start[2] - (slope * start[0]);
    perpSlope = dz / (end[2] - start[2]);
    projection[0] = ((center[2] - (perpSlope * center[0])) - intercept) / (slope - perpSlope);
    projection[2] = (slope * projection[0]) + intercept;

    if (!(getXZDistance(center, projection) < minDistanceSq))
    {
        return;
    }

    dx = end[0] - center[0];
    dz = end[2] - center[2];
    length = sqrtf((dx * dx) + (dz * dz));
    if (lbl_803E23DC != length)
    {
        dx /= length;
        dz /= length;
    }

    if (useBlendedDistance != 0)
    {
        moveDistance = sqrtf(limitDistanceSq);
        {
            f32 blend = moveDistance - sqrtf(centerToEnd);
            moveDistance = moveDistance - (blend * lbl_803E2480);
        }
    }

    end[0] = center[0] + (dx * moveDistance);
    end[2] = center[2] + (dz * moveDistance);
}


/* group owned by another DLL, queried here */

void trickyApplyObjectAvoidanceToStep(f32* start, f32* end, f32* guardPoint)
{
    int count;
    int startIndex;
    int objectCount;
    int i;
    void** objects;
    u8* obj;
    u8* def;
    ObjHitsPriorityState* hitState;
    u16 minRadius;
    void** op;
    f32 scale;

    objects = (void**)ObjGroup_GetObjects(SIDEREPEL_OBJGROUP, &count);
    for (i = 0, op = objects, scale = lbl_803E2484; i < count; i++)
    {
        obj = *op;
        def = *(u8**)&((GameObject*)obj)->anim.placementData;
        trickyAdjustStepAroundPoint(start, end, guardPoint, &((GameObject*)obj)->anim.worldPosX,
                                    scale * (f32)(u32) * (u16*)(def + 0x18),
                                    scale * (f32)(u32) * (u16*)(def + 0x1a));
        op++;
    }

    objects = ObjList_GetObjects(&startIndex, &objectCount);
    for (i = startIndex; i < objectCount; i++)
    {
        obj = objects[i];
        def = *(u8**)&((GameObject*)obj)->anim.modelInstance;
        minRadius = *(u16*)(def + 0x84);
        if (minRadius != 0)
        {
            hitState = (ObjHitsPriorityState*)((GameObject*)obj)->anim.hitReactState;
            if ((hitState != NULL) && ((*(s16*)&hitState->flags & 1) != 0))
            {
                trickyAdjustStepAroundPoint(start, end, guardPoint, &((GameObject*)obj)->anim.worldPosX,
                                            (f32)(u32)minRadius * lbl_803E2484,
                                            (f32)(u32) * (u16*)(def + 0x86) * lbl_803E2484);
            }
        }
    }
}

/*
 * Tricky sidekick follow/path-walk movement. trickyFn_8013b368 is
 * the per-frame movement step that resolves the target's walk/patch group and
 * drives motion through a substate machine and RomCurveWalker route;
 * trickyUpdateApproachSpeed ramps the follow speed toward a target point. The
 * lbl_803E2xxx externs are this DLL's .sdata2 float constants.
 */


int trickyFn_8013b368(GameObject* obj, f32 vel, TrickyState* state)
{
    int tp;
    f32* target;
    char* strs = lbl_8031D2E8;
    u8 moved;
    int wg;
    int targetWg;
    u8* prevNode;
    u16 pp;
    int trickyPatch;
    s16 link;
    u32 prod;
    int dir;
    int i;
    ObjfsaRomCurveDef* node;
    u8 slot;
    f32* patchTarget;
    int absDiff;
    u16 ulink;
    s16 yawA;
    s16 yawB;
    s16 diff;
    char type;
    u8 step;
    u8 mask;
    char found;
    f32 velBefore;
    f32 dist;
    f32 len;
    f32 v;
    f32 k;
    f32 sqz;
    f32 sqx;
    u8 pair[2];
    u8 routeFlags[8];
    struct
    {
        s16 angle; /* -anim.rotX */
        s16 _pad0;
        s16 _pad1;
    } rot;
    f32 delta[3];
    ObjfsaWalkGroupPatchInfo wgi;
    void* routePtrs[9];

    moved = 1;
    if ((state->followPhase < 5) && (isInWalkGroupOrPatch(&obj->anim.worldPosX) == 0))
    {
        (*gPathControlInterface)->attachObject(obj, &state->pathControlFlags);
        obj->anim.localPosX = state->homePosX;
        obj->anim.localPosY = state->homePosY;
        obj->anim.localPosZ = state->homePosZ;
        obj->anim.worldPosX = state->homePosX;
        obj->anim.worldPosY = state->homePosY;
        obj->anim.worldPosZ = state->homePosZ;
        ObjHits_SyncObjectPosition(obj);
    }
    target = state->targetPosPtr;
    wg = Objfsa_GetWalkGroupIndexAtPoint(&obj->anim.worldPosX, 0);
    if ((wg != 0) && (state->activeWalkGroup != wg))
    {
        state->activeWalkGroup = wg;
        {
            u32 m;
            u32 f2 = state->stateFlags;
            m = ~0x400;
            state->stateFlags = f2 & m;
        }
        state->patch[0] = 0;
        state->patch[1] = 0;
        state->patch[2] = 0;
        state->patch[3] = 0;
    }
    targetWg = Objfsa_GetWalkGroupIndexAtPoint(target, &wgi);
    if (((wg != 0) && (targetWg == 0)) && ((ulink = getPatchGroup(target, wg)) != 0))
    {
        walkPath_writeU16LE(ulink, pair);
        if (pair[0] == wg)
        {
            targetWg = pair[1];
        }
        else
        {
            targetWg = pair[0];
        }
    }
    if ((targetWg != 0) && (targetWg != state->walkGroup))
    {
        state->walkGroup = targetWg;
    }
    state->savedWalkGroup = state->walkGroup;
    trickyDebugPrint(strs + 0x1e8, state->activeWalkGroup, wg, targetWg,
                     state->walkGroup);
    if (state->activeWalkGroup == 0)
    {
        trickyReportError(strs + 0x214, obj->anim.worldPosX, obj->anim.worldPosY,
                          obj->anim.worldPosZ);
    }
    velBefore = state->speed;
    trickyUpdateApproachSpeed(obj, vel, state, target, 0);
    trickyDebugPrint(strs + 0x268, velBefore, state->speed);
    if (targetWg == state->activeWalkGroup)
    {
        int wref;
        int sref;
        int tref;

        state->stateFlags = state->stateFlags | 0x400;
        i = 0;
        mask = 1;
        wref = (int)&wgi;
        sref = (int)state;
        tref = (int)state;
        for (; i < 4; wref += 2, sref += 2, tref += 12, i++, mask = mask << 1)
        {
            if (wgi.patchMask & mask)
            {
                *(s16*)(sref + 152) = *(u16*)(wref + 2);
                *(f32*)(tref + 160) = ((TrickyPoint3*)target)->x;
                *(f32*)(tref + 164) = ((TrickyPoint3*)target)->y;
                *(f32*)(tref + 168) = ((TrickyPoint3*)target)->z;
            }
        }
    }
    if ((targetWg != 0) && (targetWg == state->activeWalkGroup))
    {
        state->linkedWalkGroup = 0;
    }
    else
    {
        prod = targetWg * state->activeWalkGroup & 0xffff;
        if (prod != 0)
        {
            u16* ids = wgi.patchGroupIds;

            for (i = 0, link = prod; i < 4; ids++, i++)
            {
                if ((prod == *ids) && (((1 << i) & wgi.patchMask) != 0))
                {
                    state->linkedWalkGroup = link;
                    state->linkedPatchPos.x = ((TrickyPoint3*)target)->x;
                    state->linkedPatchPos.y = ((TrickyPoint3*)target)->y;
                    state->linkedPatchPos.z = ((TrickyPoint3*)target)->z;
                }
            }
        }
    }
    if (isInWalkGroupOrPatch(target) != 0)
    {
        trickyDebugPrint(strs + 0x284);
    }
    else
    {
        trickyDebugPrint(strs + 0x2b0);
    }
    trickyDebugPrint(strs + 0x2e4, getPatchGroup(target, state->activeWalkGroup));
    if ((state->stateFlags & 0x400) != 0)
    {
        int pref;
        int qref;

        i = 0;
        pref = (int)state;
        qref = (int)state;
        for (; i < 4; pref += 2, qref += 12, i++)
        {
            if (*(s16*)(pref + 152) != 0)
            {
                trickyDebugPrint(strs + 0x308, i, *(f32*)(qref + 160), *(f32*)(qref + 164),
                                 *(f32*)(qref + 168));
            }
        }
    }
    if (state->linkedWalkGroup != 0)
    {
        trickyDebugPrint(strs + 0x328, state->linkedPatchPos.x, state->linkedPatchPos.y,
                         state->linkedPatchPos.z);
    }
    tp = getPatchGroup(target, state->activeWalkGroup) & 0xffff;
    trickyPatch = getPatchGroup(&obj->anim.worldPosX, state->activeWalkGroup) & 0xffff;
    if ((targetWg != 0) && (wg == targetWg))
    {
        state->followPhase = 1;
    }
    else
    {
        ulink = walkGroupFn_800db3e4(&obj->anim.worldPosX, target, state->activeWalkGroup);
        if (ulink != 0)
        {
            state->followPhase = 1;
            if (ulink != state->activeWalkGroup)
            {
                state->activeWalkGroup = ulink;
                {
                    u32 m;
                    u32 f2 = state->stateFlags;
                    m = ~0x400;
                    state->stateFlags = f2 & m;
                }
                state->patch[0] = 0;
                state->patch[1] = 0;
                state->patch[2] = 0;
                state->patch[3] = 0;
            }
        }
        else if (state->followPhase < 5)
        {
            if ((u32)tp != 0)
            {
                if (targetWg == 0)
                {
                    if (wg != 0)
                    {
                        int pref = (int)state;
                        
                        for (i = 0; i < 4; pref += 2, i++)
                        {
                            if (*(s16*)(pref + 152) == tp)
                            {
                                slot = i;
                                state->followPhase = 2;
                                break;
                            }
                        }
                        if (i == 4)
                        {
                            if (tp & !(0xff - state->cachedWalkGroup))
                            {
                                state->walkGroup = (int)(tp & 0xff00) >> 8;
                            }
                            else
                            {
                                state->walkGroup = tp & 0xff;
                            }
                            state->followPhase = 5;
                        }
                    }
                    else
                    {
                        if ((u32)trickyPatch != 0)
                        {
                            int pref = (int)state;
                            
                            for (i = 0; i < 4; pref += 2, i++)
                            {
                                if (*(s16*)(pref + 152) == trickyPatch)
                                {
                                    trickyPatch = i & 0xffff;
                                    state->followPhase = 2;
                                    break;
                                }
                            }
                            if (i == 4)
                            {
                                Objfsa_GetNearestPatchExit(target, &state->patchExitPos.x, trickyPatch);
                                state->followPhase = 4;
                            }
                        }
                        else
                        {
                            trickyReportError(strs + 0x344);
                            state->followPhase = 0;
                        }
                    }
                }
                else
                {
                    if (wg != 0)
                    {
                        int pref = (int)state;
                        
                        for (i = 0; i < 4; pref += 2, i++)
                        {
                            if (*(s16*)(pref + 152) == tp)
                            {
                                slot = i;
                                state->followPhase = 2;
                                break;
                            }
                        }
                        if (i == 4)
                        {
                            state->followPhase = 5;
                        }
                    }
                    else
                    {
                        if (wg == 0 &&
                            (u32)(tp = getPatchGroup(&obj->anim.worldPosX, state->activeWalkGroup) & 0xffff) !=
                                0)
                        {
                            if (state->linkedWalkGroup == tp)
                            {
                                state->followPhase = 3;
                            }
                            else
                            {
                                Objfsa_GetNearestPatchExit(target, &state->patchExitPos.x, (u16)tp);
                                state->followPhase = 4;
                            }
                        }
                        else
                        {
                            pp = tp;
                            i = isPointWithinPatchGroup(&obj->anim.worldPosX, state->activeWalkGroup, pp);
                            trickyReportError(strs + 0x374, pp, targetWg, wg, state->activeWalkGroup, i);
                            state->followPhase = 0;
                        }
                    }
                }
            }
            else
            {
                if (targetWg == 0)
                {
                    if (wg != 0)
                    {
                        u16 pid = Objfsa_GetPatchGroupIdAtPoint(target);
                        if (pid == 0)
                        {
                            state->followPhase = 0;
                        }
                        else
                        {
                            state->walkGroup = pid & 0xff;
                            state->followPhase = 5;
                        }
                    }
                    else
                    {
                        state->followPhase = 0;
                    }
                }
                else
                {
                    if (wg != 0)
                    {
                        if (isPointWithinPatchGroup(&obj->anim.worldPosX, state->activeWalkGroup,
                                                    (targetWg = targetWg * wg & 0xffff)) != 0)
                        {
                            if (state->linkedWalkGroup == targetWg)
                            {
                                state->followPhase = 3;
                            }
                            else
                            {
                                state->followPhase = 5;
                            }
                        }
                        else
                        {
                            int pref = (int)state;
                            
                            for (i = 0; i < 4; pref += 2, i++)
                            {
                                if (*(s16*)(pref + 152) == targetWg)
                                {
                                    slot = i;
                                    state->followPhase = 2;
                                    break;
                                }
                            }
                            if ((i == 4) || (targetWg != state->linkedWalkGroup))
                            {
                                state->followPhase = 5;
                            }
                        }
                    }
                    else
                    {
                        u16 p = getPatchGroup(&obj->anim.worldPosX, state->activeWalkGroup);
                        if (p != 0)
                        {
                            if (targetWg == state->activeWalkGroup)
                            {
                                int pref = (int)state;
                                
                                for (i = 0; i < 4; pref += 2, i++)
                                {
                                    if (*(s16*)(pref + 152) == p)
                                    {
                                        slot = i;
                                        state->followPhase = 2;
                                        break;
                                    }
                                }
                                if (i == 4)
                                {
                                    Objfsa_GetNearestPatchExit(target, &state->patchExitPos.x, (u16)p);
                                    state->followPhase = 4;
                                }
                            }
                            else if (state->linkedWalkGroup == p)
                            {
                                state->followPhase = 3;
                            }
                            else
                            {
                                Objfsa_GetNearestPatchExit(target, &state->patchExitPos.x, (u16)p);
                                state->followPhase = 4;
                            }
                        }
                        else
                        {
                            trickyReportError(strs + 0x3ec);
                            state->followPhase = 0;
                        }
                    }
                }
            }
        }
    }
    if (state->followPhase < 5)
    {
        state->stateFlags &= ~0x2000LL;
    }
    trickyDebugPrint(strs + 0x404, state->followPhase);
    switch (state->followPhase)
    {
    case 0:
        trickyDebugPrint(strs + 0x41c);
        v = lbl_803E241C * timeDelta + velBefore;
        state->speed = (v < 0.0f) ? 0.0f : v;
        if (0.0f == state->speed)
        {
            moved = 0;
        }
        else
        {
            moved = trickyMove(obj, target);
        }
        break;
    case 1:
        trickyDebugPrint(strs + 0x428);
        moved = trickyMove(obj, target);
        break;
    case 2:
        trickyDebugPrint(strs + 0x434);
        state->speed = velBefore;
        trickyUpdateApproachSpeed(obj, 0.0f, state, patchTarget = &state->patchTargets[slot].x, 1);
        moved = trickyMove(obj, patchTarget);
        break;
    case 4:
        trickyDebugPrint(strs + 0x448);
        state->speed = velBefore;
        trickyUpdateApproachSpeed(obj, lbl_803E2488, state, &state->patchExitPos.x, 1);
        moved = trickyMove(obj, &state->patchExitPos.x);
        break;
    case 3:
        trickyDebugPrint(strs + 0x45c);
        state->speed = velBefore;
        trickyUpdateApproachSpeed(obj, lbl_803E2488, state, &state->linkedPatchPos.x, 1);
        moved = trickyMove(obj, &state->linkedPatchPos.x);
        break;
    case 6:
        trickyDebugPrint(strs + 0x46c, 10,
                         (int)getXZDistance(&state->routeSeedNode->x, &obj->anim.worldPosX));
        dist = getXZDistance(&state->routeSeedNode->x, &obj->anim.worldPosX);
        if (lbl_803E23E0 > dist)
        {
            state->route.reverse = state->routeSeedDir;
            prevNode = (u8*)state->routeSeedNode;
            node = trickySelectRouteEntry((u8*)state, prevNode, state->routeSeedDir);
            if (node == 0)
            {
                state->followPhase = 0;
            }
            else
            {
                    u8* nextNode = trickySelectRouteEntry((u8*)state, (u8*)node, state->routeSeedDir);
                    if (nextNode == 0)
                {
                    state->followPhase = 0;
                }
                else
                {
                        RomCurve_setupHermiteSegment(&state->route, prevNode, node, nextNode);
                    RomCurve_stepClamped(&state->route, lbl_803E2484);
                    yawA = getAngle(state->prevLocalPosX - obj->anim.localPosX,
                                    state->prevLocalPosZ - obj->anim.localPosZ);
                    yawB = getAngle(state->prevLocalPosX - state->route.posX,
                                    state->prevLocalPosZ - state->route.posZ);
                    diff = yawA - (u16)yawB;
                    if (0x8000 < diff)
                    {
                        diff = diff - 0xffff;
                    }
                    if (diff < -0x8000)
                    {
                        diff = diff + 0xffff;
                    }
                    if (diff > 0x4000)
                    {
                        diff -= 0x8000;
                    }
                    else if (diff < -0x4000)
                    {
                        diff += 0x8000;
                    }
                    absDiff = (diff >= 0) ? diff : -diff;
                    if (0x1000 < absDiff)
                    {
                        state->speed = velBefore;
                        trickyUpdateApproachSpeed(obj, lbl_803E246C, state, &state->route.posX, 1);
                    }
                    trickyAdvanceRouteTargetAhead((int)obj, &state->route, state->speed);
                    moved = trickyMove(obj, &state->route.posX);
                    switch (*(s8*)(prevNode + 0x1a))
                    {
                    case 1:
                        node = state->route.nodeA0;
                        state->dirX = node->x - obj->anim.worldPosX;
                        state->dirZ = node->z - obj->anim.worldPosZ;
                        sqx = state->dirX * state->dirX;
                        sqz = state->dirZ * state->dirZ;
                        len = sqrtf(sqx + sqz);
                        if (0.0f != len)
                        {
                            state->dirX = state->dirX / len;
                            state->dirZ = state->dirZ / len;
                        }
                        state->speed = gTrickyFollowMaxSpeed;
                        objAnimFn_8013a3f0((int)obj, 0x15, lbl_803E2468, 0x4000000);
                        state->followPhase = 9;
                        state->voiceCooldown = lbl_803E2440;
                        break;
                    case 5:
                        node = state->route.nodeA0;
                        state->dirX = node->x - obj->anim.worldPosX;
                        state->dirZ = node->z - obj->anim.worldPosZ;
                        sqx = state->dirX * state->dirX;
                        sqz = state->dirZ * state->dirZ;
                        len = sqrtf(sqx + sqz);
                        if (0.0f != len)
                        {
                            state->dirX = state->dirX / len;
                            state->dirZ = state->dirZ / len;
                        }
                        if ((int)randomGetRange(0, 1) != 0)
                        {
                            objAnimFn_8013a3f0((int)obj, 0x17, gTrickyFollowAnim17Speed, 0x40000c0);
                        }
                        else
                        {
                            objAnimFn_8013a3f0((int)obj, 0x18, gTrickyFollowAnim18Speed, 0x40000c0);
                        }
                        state->verticalDelta =
                            (((ObjfsaRomCurveDef*)state->route.nodeA0)->y - obj->anim.worldPosY) /
                            gTrickyFollowVerticalDeltaDivisorA;
                        state->followPhase = 0xc;
                        if (state->route.reverse != 0)
                        {
                            while (state->route.atSegmentEnd != 0)
                            {
                                RomCurve_stepClamped(&state->route, lbl_803E2448);
                            }
                        }
                        else
                        {
                            while (state->route.atSegmentEnd == 0)
                            {
                                RomCurve_stepClamped(&state->route, lbl_803E23F8);
                            }
                        }
                        state->voiceCooldown = lbl_803E2440;
                        break;
                    case 6:
                        node = state->route.nodeA0;
                        state->dirX = node->x - obj->anim.worldPosX;
                        state->dirZ = node->z - obj->anim.worldPosZ;
                        sqx = state->dirX * state->dirX;
                        sqz = state->dirZ * state->dirZ;
                        len = sqrtf(sqx + sqz);
                        if (0.0f != len)
                        {
                            state->dirX = state->dirX / len;
                            state->dirZ = state->dirZ / len;
                        }
                        objAnimFn_8013a3f0((int)obj, 0x19, lbl_803E249C, 0x40000c0);
                        state->verticalDelta =
                            (obj->anim.worldPosY - ((ObjfsaRomCurveDef*)state->route.nodeA0)->y) /
                            gTrickyFollowVerticalDeltaDivisorB;
                        state->followPhase = 0xe;
                        if (state->route.reverse != 0)
                        {
                            while (state->route.atSegmentEnd != 0)
                            {
                                RomCurve_stepClamped(&state->route, lbl_803E2448);
                            }
                        }
                        else
                        {
                            while (state->route.atSegmentEnd == 0)
                            {
                                RomCurve_stepClamped(&state->route, lbl_803E23F8);
                            }
                        }
                        state->voiceCooldown = lbl_803E2440;
                        break;
                    case 2:
                    case 7:
                        state->stateFlags = state->stateFlags | 0x2000;
                    default:
                        state->followPhase = 7;
                    }
                }
            }
        }
        else
        {
            node = state->routeSeedNode;
            if (node == NULL)
            {
                node = NULL;
            }
            else if (((node->requiredBit != -1) && (mainGetBit(node->requiredBit) == 0)) ||
                     ((node->forbiddenBit != -1) && (mainGetBit(node->forbiddenBit) != 0)))
            {
                node = NULL;
            }
            if ((node != 0) || (wg == 0))
            {
                state->speed = velBefore;
                trickyUpdateApproachSpeed(obj, lbl_803E246C, state,
                                          &state->routeSeedNode->x, 1);
                moved = trickyMove(obj, &state->routeSeedNode->x);
            }
            else
            {
                state->followPhase = 0;
            }
        }
        break;
    case 5:
        trickyDebugPrint(strs + 0x480);
        trickyRankLinkedRouteCandidates(obj, routeFlags, (s16)wg, routePtrs);
        i = trickyFindReachableRouteIndex((u8*)state, routePtrs, routeFlags, state->walkGroup);
        if (i == -1)
        {
            state->speed = velBefore;
            return 2;
        }
        state->routeSeedDir = routeFlags[i];
        state->routeSeedNode = routePtrs[i];
        state->speed = velBefore;
        trickyUpdateApproachSpeed(obj, lbl_803E2488, state, &state->routeSeedNode->x, 1);
        moved = trickyMove(obj, &state->routeSeedNode->x);
        state->followPhase = 6;
        break;
    case 7:
        trickyDebugPrint(strs + 0x490);
        if ((state->savedWalkGroup != 0) && (wg == state->savedWalkGroup))
        {
            v = lbl_803E241C * timeDelta + velBefore;
            state->speed = (v < 0.0f) ? 0.0f : v;
        }
        node = state->route.nodeA0;
        if ((((ObjfsaRomCurveDef*)state->route.node9C)->unk1A != 9) && (node->unk1A != 9))
        {
            f32* tpos = state->targetPosPtr;
            delta[0] = tpos[0] - obj->anim.worldPosX;
            delta[1] = tpos[1] - obj->anim.worldPosY;
            delta[2] = tpos[2] - obj->anim.worldPosZ;
            rot.angle = -obj->anim.rotX;
            rot._pad0 = 0;
            rot._pad1 = 0;
            vecRotateZXY(&rot.angle, delta);
            if ((delta[2] > 0.0f) && (0.0f != state->speed))
            {
                for (step = 0; step < 4; step++)
                {
                    u8 grp = node->linkSelectors[step];
                    if (grp == state->walkGroup)
                    {
                        break;
                    }
                }
                if (step == 4)
                {
                    pathSearchBegin(&state->pathSearches[0], (PathPoint*)state->route.nodeA4,
                                state->targetPosPtr, state->walkGroup,
                                state->route.reverse);
                    pathSearchBegin(&state->pathSearches[1], (PathPoint*)state->route.node9C,
                                state->targetPosPtr, state->walkGroup,
                                state->route.reverse ^ 1);
                    found = 0;
                    for (i = 0; (u8)(i = i + 1) < 100 && (found != 1);)
                    {
                        found = pathSearchStep(&state->pathSearches[0], 1);
                        if (found != 1)
                        {
                            found = pathSearchStep(&state->pathSearches[1], 1);
                            switch (found)
                            {
                            case 0:
                                break;
                            case 1:
                                prod = (state->route.reverse ^ 1) & 0xff;
                                if (prod == 0)
                                {
                                    RomCurve_stepClamped(&state->route, lbl_803E23F8);
                                }
                                else
                                {
                                    RomCurve_stepClamped(&state->route, lbl_803E2448);
                                }
                                state->route.reverse = prod;
                                RomCurve_swapEndpointNodes(&state->route);
                                break;
                            case -1:
                                found = 1;
                                break;
                            }
                        }
                    }
                }
            }
        }
        dir = state->route.reverse;
        if (((dir == 0) && (state->route.atSegmentEnd != 0)) || ((dir != 0 && (state->route.atSegmentEnd == 0))))
        {
            node = trickySelectRouteEntry((u8*)state, state->route.nodeA4, dir & 0xff);
            if (node != 0)
            {
                curveFn_800da23c(&state->route, node);
                type = ((ObjfsaRomCurveDef*)state->route.node9C)->unk1A;
                switch (type)
                {
                case 2:
                case 7:
                    prod = state->stateFlags;
                    if ((prod & 0x2000) != 0)
                    {
                        state->stateFlags = prod & ~0x2000LL;
                    }
                    else
                    {
                        state->stateFlags = prod | 0x2000;
                    }
                    break;
                }
            }
            else
            {
                state->followPhase = 0;
                break;
            }
        }
        else
        {
            node = trickySelectRouteEntry((u8*)state, state->route.nodeA0, dir & 0xff);
            if (node == 0)
            {
                state->followPhase = 0;
                break;
            }
            if (node != state->route.nodeA4)
            {
                RomCurve_setSegmentEndNode(&state->route, node);
            }
        }
        if ((state->savedWalkGroup == 0) || (wg != state->savedWalkGroup))
        {
            yawA = getAngle(state->prevLocalPosX - obj->anim.localPosX,
                            state->prevLocalPosZ - obj->anim.localPosZ);
            yawB = getAngle(state->prevLocalPosX - state->route.posX,
                            state->prevLocalPosZ - state->route.posZ);
            diff = yawA - (u16)yawB;
            if (0x8000 < diff)
            {
                diff = diff - 0xffff;
            }
            if (diff < -0x8000)
            {
                diff = diff + 0xffff;
            }
            if (diff > 0x4000)
            {
                diff -= 0x8000;
            }
            else if (diff < -0x4000)
            {
                diff += 0x8000;
            }
            absDiff = (diff >= 0) ? diff : -diff;
            if (0x1000 < absDiff)
            {
                state->speed = velBefore;
                trickyUpdateApproachSpeed(obj, lbl_803E246C, state, &state->route.posX, 1);
            }
        }
        trickyAdvanceRouteTargetAhead((int)obj, &state->route, state->speed);
        moved = trickyMove(obj, &state->route.posX);
        type = ((ObjfsaRomCurveDef*)state->route.nodeA0)->unk1A;
        switch (type)
        {
        case 1:
            state->followPhase = 8;
            break;
        case 5:
            state->followPhase = 0xb;
            break;
        case 6:
            state->followPhase = 0xd;
            break;
        }
        break;
    case 8:
        trickyDebugPrint(strs + 0x49c);
        v = lbl_803E2420 * timeDelta + velBefore;
        state->speed = (v > gTrickyFollowMaxSpeed) ? gTrickyFollowMaxSpeed : v;
        if ((state->savedWalkGroup != 0) && (wg == state->savedWalkGroup))
        {
            v = lbl_803E241C * timeDelta + velBefore;
            state->speed = (v < 0.0f) ? 0.0f : v;
        }
        yawA = getAngle(state->prevLocalPosX - obj->anim.localPosX,
                        state->prevLocalPosZ - obj->anim.localPosZ);
        yawB = getAngle(state->prevLocalPosX - state->route.posX,
                        state->prevLocalPosZ - state->route.posZ);
        diff = yawA - (u16)yawB;
        if (0x8000 < diff)
        {
            diff = diff - 0xffff;
        }
        if (diff < -0x8000)
        {
            diff = diff + 0xffff;
        }
        if (diff > 0x4000)
        {
            diff -= 0x8000;
        }
        else if (diff < -0x4000)
        {
            diff += 0x8000;
        }
        absDiff = (diff >= 0) ? diff : -diff;
        if (0x1000 < absDiff)
        {
            state->speed = velBefore;
            trickyUpdateApproachSpeed(obj, lbl_803E246C, state, &state->route.posX, 1);
        }
        trickyAdvanceRouteTargetAhead((int)obj, &state->route, state->speed);
        trickyMove(obj, &state->route.posX);
        dir = state->route.reverse;
        if (((dir == 0) && (state->route.atSegmentEnd != 0)) || ((dir != 0 && (state->route.atSegmentEnd == 0))))
        {
            u8* nextRouteNode = trickySelectRouteEntry((u8*)state, state->route.nodeA4, dir & 0xff);
            if (nextRouteNode == 0)
            {
                state->followPhase = 0;
            }
            else
            {
                curveFn_800da23c(&state->route, nextRouteNode);
                node = state->route.nodeA0;
                state->dirX = node->x - obj->anim.worldPosX;
                state->dirZ = node->z - obj->anim.worldPosZ;
                sqx = state->dirX * state->dirX;
                sqz = state->dirZ * state->dirZ;
                len = sqrtf(sqx + sqz);
                if (0.0f != len)
                {
                    state->dirX = state->dirX / len;
                    state->dirZ = state->dirZ / len;
                }
                state->speed = gTrickyFollowMaxSpeed;
                objAnimFn_8013a3f0((int)obj, 0x15, lbl_803E2468, 0x4000000);
                state->followPhase = 9;
                state->voiceCooldown = lbl_803E2440;
            }
        }
        break;
    case 9:
        trickyDebugPrint(strs + 0x4ac);
        if ((u8)(state->stateFlags & 0x10000000))
        {
            v = lbl_803E23F4 * timeDelta + velBefore;
            if (v < 0.0f)
            {
                v = 0.0f;
            }
        }
        else if (velBefore > (v = lbl_803E24A4))
        {
            k = lbl_803E241C * timeDelta + velBefore;
            v = (k < v) ? v : k;
        }
        else
        {
            k = lbl_803E2420 * timeDelta + velBefore;
            v = (k > v) ? v : k;
        }
        state->speed = v;
        {
            f32 dz;
            f32 dx;
            dx = ((TrickyState*)obj->extra)->dirX;
            sqx = dx;
            sqx = sqx * sqx;
            dz = ((TrickyState*)obj->extra)->dirZ;
            sqz = dz;
            sqz = sqz * sqz;
            if (sqx + sqz > lbl_803E23EC)
            {
                trickyTurnTowardYaw((u8*)obj, (s16)getAngle(-dx, -dz));
            }
        }
        if (obj->anim.currentMoveProgress < lbl_803E24A8)
        {
            ObjAnim_SampleRootCurvePhase(&obj->anim, state->speed, &state->moveProgress);
            obj->anim.localPosX =
                timeDelta * (state->dirX * state->speed) +
                obj->anim.localPosX;
            obj->anim.localPosZ =
                timeDelta * (state->dirZ * state->speed) +
                obj->anim.localPosZ;
        }
        else
        {
            ObjAnim_SampleRootCurvePhase(&obj->anim, state->speed * lbl_803E24AC, &state->moveProgress);
            obj->anim.localPosX =
                timeDelta * (state->dirX * (state->speed * (k = lbl_803E24AC))) +
                obj->anim.localPosX;
            obj->anim.localPosZ =
                timeDelta * (state->dirZ * (state->speed * k)) +
                obj->anim.localPosZ;
        }
        if ((state->stateFlags & TRICKY_STATE_FLAG_MOVE_ADVANCING) != 0)
        {
            f32 dx;
            f32 dz;
            f32 arcCoefficient;
            TrickyJumpArc* arc = &state->jumpArc;
            node = state->route.nodeA0;
            dx = node->x - obj->anim.worldPosX;
            sqx = dx * dx;
            dx = node->z - obj->anim.worldPosZ;
            dx = dx * dx;
            len = sqrtf(sqx + dx);
            arc->duration = len / lbl_803E24A4;
            arc->time = (v = 0.0f);
            arc->baseX = obj->anim.worldPosX;
            arc->baseY = obj->anim.worldPosY;
            arc->baseZ = obj->anim.worldPosZ;
            arc->landX = node->x;
            arc->landZ = node->z;
            k = arc->duration;
            arcCoefficient = gTrickyFollowArcCoefficient * k;
            arc->riseCoeff = -(arcCoefficient * k -
                               (node->y - obj->anim.worldPosY)) /
                              k;
            objAnimFn_8013a3f0((int)obj, 0x16, v, 0x4000000);
            state->arcMoveProgress = arc->time / arc->duration;
            state->speed = lbl_803E24A4;
            state->followPhase = 10;
            if (state->route.reverse != 0)
            {
                while (state->route.atSegmentEnd != 0)
                {
                    RomCurve_stepClamped(&state->route, lbl_803E2448);
                }
            }
            else
            {
                while (state->route.atSegmentEnd == 0)
                {
                    RomCurve_stepClamped(&state->route, lbl_803E23F8);
                }
            }
        }
        break;
    case 10:
    {
        TrickyJumpArc* arc = &state->jumpArc;
        trickyDebugPrint(strs + 0x4b8);
        arc->time = arc->time + timeDelta;
        if (arc->time >= arc->duration)
        {
            obj->anim.localPosY = ((ObjfsaRomCurveDef*)state->route.nodeA0)->y;
            state->arcMoveProgress = lbl_803E23E8;
            state->followPhase = 7;
        }
        else
        {
            f32 baseX = arc->baseX;
            f32 baseZ;
            obj->anim.localPosX = (arc->landX - baseX) * (arc->time / arc->duration) + baseX;
            k = arc->time;
            {
                f32 ck;
                ck = gTrickyFollowArcCoefficient * k;
                obj->anim.localPosY = ck * k + (arc->riseCoeff * k + arc->baseY);
            }
            baseZ = arc->baseZ;
            obj->anim.localPosZ = (arc->landZ - baseZ) * (arc->time / arc->duration) + baseZ;
            v = arc->duration;
            if (v <= lbl_803E24B4)
            {
                state->arcMoveProgress = arc->time / v;
            }
            else
            {
                k = arc->time;
                if (k <= lbl_803E24B8)
                {
                    state->arcMoveProgress = k / lbl_803E24B4;
                }
                else if (k >= v - lbl_803E24B8)
                {
                    f32 adj;
                    adj = lbl_803E24B4 - v;
                    state->arcMoveProgress = (adj + k) / lbl_803E24B4;
                }
                else
                {
                    k = (k - lbl_803E24B8) / (v - lbl_803E24BC);
                    state->arcMoveProgress = k * lbl_803E24A8 + lbl_803E24AC;
                }
            }
            objHitDetectFn_80062e84(obj, NULL, 0);
            state->heightUpdateActive = 0;
        }
        break;
    }
    case 0xb:
        trickyDebugPrint(strs + 0x4c4);
        v = lbl_803E2420 * timeDelta + velBefore;
        state->speed = (v > gTrickyFollowMaxSpeed) ? gTrickyFollowMaxSpeed : v;
        if ((state->savedWalkGroup != 0) && (wg == state->savedWalkGroup))
        {
            v = lbl_803E241C * timeDelta + velBefore;
            state->speed = (v < 0.0f) ? 0.0f : v;
        }
        yawA = getAngle(state->prevLocalPosX - obj->anim.localPosX,
                        state->prevLocalPosZ - obj->anim.localPosZ);
        yawB = getAngle(state->prevLocalPosX - state->route.posX,
                        state->prevLocalPosZ - state->route.posZ);
        diff = yawA - (u16)yawB;
        if (0x8000 < diff)
        {
            diff = diff - 0xffff;
        }
        if (diff < -0x8000)
        {
            diff = diff + 0xffff;
        }
        if (diff > 0x4000)
        {
            diff -= 0x8000;
        }
        else if (diff < -0x4000)
        {
            diff += 0x8000;
        }
        absDiff = (diff >= 0) ? diff : -diff;
        if (0x1000 < absDiff)
        {
            state->speed = velBefore;
            trickyUpdateApproachSpeed(obj, lbl_803E246C, state, &state->route.posX, 1);
        }
        trickyAdvanceRouteTargetAhead((int)obj, &state->route, state->speed);
        trickyMove(obj, &state->route.posX);
        dir = state->route.reverse;
        if (((dir == 0) && (state->route.atSegmentEnd != 0)) || ((dir != 0 && (state->route.atSegmentEnd == 0))))
        {
            u8* nextRouteNode = trickySelectRouteEntry((u8*)state, state->route.nodeA4, dir & 0xff);
            if (nextRouteNode == 0)
            {
                state->followPhase = 0;
            }
            else
            {
                curveFn_800da23c(&state->route, nextRouteNode);
                node = state->route.nodeA0;
                state->dirX = node->x - obj->anim.worldPosX;
                state->dirZ = node->z - obj->anim.worldPosZ;
                sqx = state->dirX * state->dirX;
                sqz = state->dirZ * state->dirZ;
                len = sqrtf(sqx + sqz);
                if (0.0f != len)
                {
                    state->dirX = state->dirX / len;
                    state->dirZ = state->dirZ / len;
                }
                if ((int)randomGetRange(0, 1) != 0)
                {
                    objAnimFn_8013a3f0((int)obj, 0x17, gTrickyFollowAnim17Speed, 0x40000c0);
                }
                else
                {
                    objAnimFn_8013a3f0((int)obj, 0x18, gTrickyFollowAnim18Speed, 0x40000c0);
                }
                state->verticalDelta =
                    (((ObjfsaRomCurveDef*)state->route.nodeA0)->y - obj->anim.worldPosY) /
                    gTrickyFollowVerticalDeltaDivisorA;
                state->followPhase = 0xc;
                if (state->route.reverse != 0)
                {
                    while (state->route.atSegmentEnd != 0)
                    {
                        RomCurve_stepClamped(&state->route, lbl_803E2448);
                    }
                }
                else
                {
                    while (state->route.atSegmentEnd == 0)
                    {
                        RomCurve_stepClamped(&state->route, lbl_803E23F8);
                    }
                }
                state->voiceCooldown = lbl_803E2440;
            }
        }
        break;
    case 0xc:
    case 0xe:
        trickyDebugPrint(strs + 0x4d4);
        state->heightUpdateActive = 0;
        trickyAdvanceRouteTargetAhead((int)obj, &state->route, state->speed);
        {
            f32 dz;
            f32 dx;
            dx = ((TrickyState*)obj->extra)->dirX;
            sqz = dx;
            sqz = sqz * sqz;
            dz = ((TrickyState*)obj->extra)->dirZ;
            sqx = dz;
            sqx = sqx * sqx;
            if (sqz + sqx > lbl_803E23EC)
            {
                trickyTurnTowardYaw((u8*)obj, (s16)getAngle(-dx, -dz));
            }
        }
        if ((state->stateFlags & TRICKY_STATE_FLAG_MOVE_ADVANCING) != 0)
        {
            state->speed = lbl_803E24C0;
            trickyMove(obj, &state->route.posX);
            state->followPhase = 7;
        }
        break;
    case 0xd:
        trickyDebugPrint(strs + 0x4e8);
        v = lbl_803E2420 * timeDelta + velBefore;
        state->speed = (v > gTrickyFollowMaxSpeed) ? gTrickyFollowMaxSpeed : v;
        if ((state->savedWalkGroup != 0) && (wg == state->savedWalkGroup))
        {
            v = lbl_803E241C * timeDelta + velBefore;
            state->speed = (v < 0.0f) ? 0.0f : v;
        }
        yawA = getAngle(state->prevLocalPosX - obj->anim.localPosX,
                        state->prevLocalPosZ - obj->anim.localPosZ);
        yawB = getAngle(state->prevLocalPosX - state->route.posX,
                        state->prevLocalPosZ - state->route.posZ);
        diff = yawA - (u16)yawB;
        if (0x8000 < diff)
        {
            diff = diff - 0xffff;
        }
        if (diff < -0x8000)
        {
            diff = diff + 0xffff;
        }
        if (diff > 0x4000)
        {
            diff -= 0x8000;
        }
        else if (diff < -0x4000)
        {
            diff += 0x8000;
        }
        absDiff = (diff >= 0) ? diff : -diff;
        if (0x1000 < absDiff)
        {
            state->speed = velBefore;
            trickyUpdateApproachSpeed(obj, lbl_803E246C, state, &state->route.posX, 1);
        }
        trickyAdvanceRouteTargetAhead((int)obj, &state->route, state->speed);
        trickyMove(obj, &state->route.posX);
        dir = state->route.reverse;
        if (((dir == 0) && (state->route.atSegmentEnd != 0)) || ((dir != 0 && (state->route.atSegmentEnd == 0))))
        {
            u8* nextRouteNode = trickySelectRouteEntry((u8*)state, state->route.nodeA4, dir & 0xff);
            if (nextRouteNode == 0)
            {
                state->followPhase = 0;
            }
            else
            {
                curveFn_800da23c(&state->route, nextRouteNode);
                node = state->route.nodeA0;
                state->dirX = node->x - obj->anim.worldPosX;
                state->dirZ = node->z - obj->anim.worldPosZ;
                sqx = state->dirX * state->dirX;
                sqz = state->dirZ * state->dirZ;
                len = sqrtf(sqx + sqz);
                if (0.0f != len)
                {
                    state->dirX = state->dirX / len;
                    state->dirZ = state->dirZ / len;
                }
                objAnimFn_8013a3f0((int)obj, 0x19, lbl_803E249C, 0x40000c0);
                state->verticalDelta =
                    (obj->anim.worldPosY - ((ObjfsaRomCurveDef*)state->route.nodeA0)->y) /
                    gTrickyFollowVerticalDeltaDivisorB;
                state->followPhase = 0xe;
                if (state->route.reverse != 0)
                {
                    while (state->route.atSegmentEnd != 0)
                    {
                        RomCurve_stepClamped(&state->route, lbl_803E2448);
                    }
                }
                else
                {
                    while (state->route.atSegmentEnd == 0)
                    {
                        RomCurve_stepClamped(&state->route, lbl_803E23F8);
                    }
                }
                state->voiceCooldown = lbl_803E2440;
            }
        }
        break;
    default:
        trickyDebugPrint(strs + 0x4f8);
    }
    if (state->followPhase < 5)
    {
        if (isInWalkGroupOrPatch(&obj->anim.worldPosX) != 0)
        {
            state->homePosX = obj->anim.worldPosX;
            state->homePosY = obj->anim.worldPosY;
            state->homePosZ = obj->anim.worldPosZ;
        }
        else
        {
            (*gPathControlInterface)->attachObject(obj, &state->pathControlFlags);
            obj->anim.localPosX = state->homePosX;
            obj->anim.localPosY = state->homePosY;
            obj->anim.localPosZ = state->homePosZ;
            obj->anim.worldPosX = state->homePosX;
            obj->anim.worldPosY = state->homePosY;
            obj->anim.worldPosZ = state->homePosZ;
            ObjHits_SyncObjectPosition(obj);
        }
    }
    step = state->followPhase;
    if (((((step == 0) || (step == 2)) || (step == 4)) || (step == 3)) &&
        (0.0f == state->speed))
    {
        return 2;
    }
    if (moved != 0)
    {
        return 1;
    }
    return 0;
}

void trickyUpdateApproachSpeed(GameObject* obj, f32 baseRadius, TrickyState* state, f32* targetPos, u8 flag)
{
    struct
    {
        s16 angle; /* -anim.rotX */
        s16 _pad0;
        s16 _pad1;
    } params;
    f32 delta[3];
    f32 dec;
    f32 td;
    f32 v;
    f32 sum;
    f32 distSq;
    f32 thresh;
    f32 dist;
    f32 dx;
    f32 dz;
    f32 vel;
    f32 candidate;
    f32* otherTarget;
    TrickyState* ctx;
    f32 minSpeed;

    sum = lbl_803E2420;
    v = state->speed;
    td = timeDelta;
    dec = lbl_803E241C * td;
    minSpeed = 0.0f;
    while (v > minSpeed)
    {
        sum = v * td + sum;
        v = v + dec;
    }
    thresh = baseRadius + sum;
    distSq = thresh;
    distSq = distSq * thresh;
    dist = getXZDistance(targetPos, &obj->anim.worldPosX);
    if (dist < distSq)
    {
        candidate = state->speed;
        candidate = lbl_803E241C * timeDelta + candidate;
        state->speed = (candidate < 0.0f) ? 0.0f : candidate;
        return;
    }
    if (flag != 0)
    {
        delta[0] = targetPos[0] - obj->anim.worldPosX;
        delta[1] = targetPos[1] - obj->anim.worldPosY;
        delta[2] = targetPos[2] - obj->anim.worldPosZ;
        params.angle = -obj->anim.rotX;
        params._pad0 = 0;
        params._pad1 = 0;
        vecRotateZXY(&params.angle, delta);
        if (delta[2] > 0.0f)
        {
            candidate = state->speed;
            candidate = lbl_803E241C * timeDelta + candidate;
            state->speed = (candidate < 0.0f) ? 0.0f : candidate;
            return;
        }
    }
    if ((state->stateFlags & 0x10000000) != 0)
    {
        state->speed = lbl_803E23F4 * timeDelta + state->speed;
        if (state->speed < 0.0f)
        {
            state->speed = 0.0f;
        }
        return;
    }
    {
        f32 deltaSpeed = lbl_803E2488 + thresh;
        f32 deltaSpeedSq = deltaSpeed * deltaSpeed;
        ctx = obj->extra;
        otherTarget = ctx->targetPosPtr;
        if (otherTarget == ctx->previousPathPoint)
        {
            dx = ctx->previousPathX - obj->anim.worldPosX;
            dz = ctx->previousPathZ - obj->anim.worldPosZ;
            vel = sqrtf(dx * dx + dz * dz) * oneOverTimeDelta;
            dx = *(f32*)((u8*)otherTarget + 0) - obj->anim.worldPosX;
            dz = *(f32*)((u8*)otherTarget + 8) - obj->anim.worldPosZ;
            {
                f32 distOther = sqrtf(dx * dx + dz * dz) * oneOverTimeDelta;
                candidate = distOther - vel;
            }
        }
        else
        {
            candidate = 0.0f;
        }
        if (dist < deltaSpeedSq)
        {
            if (candidate > 0.0f)
            {
                f32 curSpeed = state->speed;
                if (candidate < curSpeed)
                {
                    f32 step = lbl_803E241C * timeDelta + curSpeed;
                    state->speed = (step < candidate) ? candidate : step;
                    return;
                }
                else
                {
                    f32 step;
                    if (candidate > gTrickyFollowMaxSpeed)
                    {
                        step = lbl_803E2420 * timeDelta + state->speed;
                        state->speed = (step > gTrickyFollowMaxSpeed) ? gTrickyFollowMaxSpeed : step;
                        return;
                    }
                    step = lbl_803E2420 * timeDelta + state->speed;
                    state->speed = (step > candidate) ? candidate : step;
                    return;
                }
            }
        }
    }
    if ((state->stateFlags & 0x00100000) != 0)
    {
        state->speed = lbl_803E243C * timeDelta + state->speed;
        if (state->speed > gTrickyFollowMaxSpeed)
        {
            state->speed = gTrickyFollowMaxSpeed;
        }
        return;
    }
    {
        f32 step = state->speed;
        step = lbl_803E2420 * timeDelta + step;
        state->speed = (step > gTrickyFollowMaxSpeed) ? gTrickyFollowMaxSpeed : step;
    }
}


#define TRICKYWARP_OBJ_GROUP 0x4b /* DLL 0x100 trickywarp */

void trickyFn_8013d8f0(u8* self, u8* state)
{
    u8* nearest;
    f32 rejectDist;
    f32 minDist;
    f32 dist;
    f32 z;
    u8** objs;
    u8** objsList;
    int count;
    int i;
    int inWater;
    u8* best;

    nearest = NULL;
    best = NULL;
    minDist = lbl_803E2418;

    if (trickyFoodFn_8013db3c(self, state) == 0)
    {
        ((TrickyState*)state)->stateIndex = 1;
        ((TrickyState*)state)->substate = 0;
        z = lbl_803E23DC;
        ((TrickyState*)state)->cooldownA = z;
        ((TrickyState*)state)->cooldownB.f = z;
        ((TrickyState*)state)->stateFlags = ((TrickyState*)state)->stateFlags & (u64)~0x10u;
        ((TrickyState*)state)->stateFlags = ((TrickyState*)state)->stateFlags & (u64)~0x10000u;
        ((TrickyState*)state)->stateFlags = ((TrickyState*)state)->stateFlags & (u64)~0x20000u;
        ((TrickyState*)state)->stateFlags = ((TrickyState*)state)->stateFlags & (u64)~0x40000u;
        ((TrickyState*)state)->commandPhase = -1;
        return;
    }

    objsList = (u8**)ObjGroup_GetObjects(TRICKYWARP_OBJ_GROUP, &count);
    i = 0;
    objs = objsList;
    rejectDist = lbl_803E24C4;
    for (; i < count; i++)
    {
        dist = getXZDistance((f32*)((u8*)((TrickyState*)state)->playerObj + 0x18), (f32*)(*objs + 0x18));
        if (dist > rejectDist)
        {
            dist = getXZDistance((f32*)(self + 0x18), (f32*)(*objs + 0x18));
            if (dist < minDist)
            {
                best = *objs;
                minDist = dist;
            }
        }
        objs++;
    }

    nearest = best;
    if (nearest != NULL)
    {
        ((TrickyState*)state)->followObj = (GameObject*)nearest;
        if ((u8*)((TrickyState*)state)->targetPosPtr != nearest + 0x18)
        {
            ((TrickyState*)state)->targetPosPtr = (f32*)(nearest + 0x18);
            {
                u32 m;
                u32 f2 = ((TrickyState*)state)->stateFlags;
                m = ~0x400;
                ((TrickyState*)state)->stateFlags = f2 & m;
            }
            ((TrickyState*)state)->linkedWalkGroup = 0;
        }
        if (trickyFn_8013b368((GameObject*)self, lbl_803E247C, (TrickyState*)state) == 1)
            return;
    }

    if (lbl_803E23DC == ((TrickyState*)state)->waterLevel)
    {
        inWater = 0;
    }
    else if (lbl_803E2410 == ((TrickyState*)state)->eventTime)
    {
        inWater = 1;
    }
    else if (((TrickyState*)state)->currentTime - ((TrickyState*)state)->eventTime > lbl_803E2414)
    {
        inWater = 1;
    }
    else
    {
        inWater = 0;
    }

    if (inWater != 0)
    {
        objAnimFn_8013a3f0((int)self, 8, lbl_803E243C, 0);
        ((TrickyState*)state)->cooldownC = lbl_803E2440;
        ((TrickyState*)state)->particleTimer = lbl_803E23DC;
        trickyDebugPrint(sInWaterMessage);
    }
    else
    {
        objAnimFn_8013a3f0((int)self, 0, lbl_803E2444, 0);
        trickyDebugPrint(lbl_8031D478);
    }
}

/*
 * Tricky's "is this critter worth eating?" decision.
 *
 * trickyFoodFn_8013db3c is queried with Tricky (arg1) and a candidate
 * critter (arg2). It returns:
 *   0 - not interested,
 *   1 - interested (critter is valid prey here),
 *   2 - interested AND within eating range.
 *
 * A critter is rejected outright while another object of group 0x53 is
 * nearby. Otherwise critters of type != 3 are accepted depending on the
 * level object's map cell: cell 0x38 gates acceptance behind a set of
 * game bits, any other cell flags the critter's per-instance cooldown
 * (the 4-bit mode field packed at byte 0x58) and accepts it. The final
 * range test promotes a "1" result to "2" when the critter sits within
 * lbl_803E24C4 squared units of Tricky.
 */

/* per-critter packed flags at byte 0x58; bits 27..30 hold a countdown mode */
struct CritterFlags
{
    u32 pad_high : 3;
    u32 mode : 4;
    u32 pad_low : 1;
};

#define MMPCRITTERSPIT_OBJFLAG_PARENT_SLACK 0x1000
#define PRESSURESWITCHFB_REMOVE_GROUP_ID    0x53 /* DLL 0xFB pressureswitchfb (self-registers) */


int trickyFoodFn_8013db3c(u8* tricky, u8* critter)
{
    int result = 0;
    f32 dist = lbl_803E242C;
    struct CritterFlags* flags = (struct CritterFlags*)&critter[0x58];

    if (flags->mode != 0)
    {
        flags->mode--;
        result = 1;
    }

    if ((u8*)ObjGroup_FindNearestObject(PRESSURESWITCHFB_REMOVE_GROUP_ID, (GameObject*)tricky, &dist) != NULL)
    {
        return 0;
    }

    if ((s8)critter[0xD] != 3)
    {
        u8* levelObj = (u8*)*(u32*)(critter + 4);

        if ((((GameObject*)levelObj)->objectFlags & MMPCRITTERSPIT_OBJFLAG_PARENT_SLACK) != 0)
        {
            if (coordsToMapCell(((GameObject*)tricky)->anim.localPosX, ((GameObject*)tricky)->anim.localPosZ) == 0x38)
            {
                if ((mainGetBit(0x385) == 0) && (mainGetBit(0x384) != 0))
                {
                    if ((mainGetBit(GAMEBIT_ITEM_TrickyFood_Count) != 0) ||
                        (mainGetBit(GAMEBIT_ITEM_TrickyFood_GrabInProgress) != 0))
                    {
                        result = 1;
                    }
                }
            }
            else
            {
                flags->mode = 0x1F;
                result = 1;
            }
        }
    }

    if (result == 1)
    {
        u8* levelObj = (u8*)*(u32*)(critter + 4);

        if (vec3f_distanceSquared(&((GameObject*)levelObj)->anim.worldPosX, &((GameObject*)tricky)->anim.worldPosX) <
            lbl_803E24C4)
        {
            return 2;
        }
    }
    return result;
}

/*
 * Tricky "growl/dig" action handler.
 *
 * trickyGrowl drives a four-step substate machine for the Tricky sidekick:
 *   0  growl windup  - barks (sfx 0x299), kicks off anim move 0x33
 *   1  face target   - turns toward the followed object (extra+0x28), with a
 *                      random chance to bark again, until anim flag + timer hit
 *   2  dig start     - if loading isn't locked, spawns seven child objects
 *                      (Obj_AllocObjectSetup/Obj_SetupObject into scratch700..),
 *                      plays/loops the dig sfx (0x3db/0x3dc) and runs anim 0x34
 *   3  dig end       - on move progress >= threshold, resets child anim speed,
 *                      stops the dig loop, barks (sfx 0x29d) and clears the
 *                      action's state flags, returning to substate 0
 *
 * Barks are gated on bit 6 of TrickyGrowlState.unk58, the current anim move
 * being outside [0x29,0x30), and no sfx already playing on channel 0x10.
 */

typedef struct TrickyGrowlState
{
    u8 pad0[0x8 - 0x0];
    f32 unk8; /* 0x08: target Z (paired with deref base for X) */
    u8 padC[0x58 - 0xC];
    u8 unk58; /* 0x58: bit 6 suppresses barks */
    u8 pad59[0x60 - 0x59];
} TrickyGrowlState;

#define CHILD_OBJECT_COUNT 7
#define TRICKY_CHILD_OBJ_FLAMEBLAST 0x4f0 /* "flameblast" (DLL 0xF3) */

enum
{
    TRICKYGROWL_WINDUP = 0,
    TRICKYGROWL_FACE_TARGET = 1,
    TRICKYGROWL_DIG_START = 2,
    TRICKYGROWL_DIG_END = 3
};

void trickyGrowl(void* obj, void* trickyState)
{
    void* state;
    int i;
    int j;
    void* digState;
    void** slot;
    void* setup;
    void** slot2;
    char* strBase = lbl_8031D2E8;

    switch (((TrickyState*)trickyState)->substate)
    {
    case TRICKYGROWL_WINDUP:
        trickyDebugPrint(strBase + 0x558);
        if (trickyFn_8013b368(obj, lbl_803E24C8, trickyState) == 0)
        {
            state = ((GameObject*)obj)->extra;
            if ((((u32)((TrickyGrowlState*)state)->unk58 >> 6) & 1) == 0u)
            {
                s16 move = ((GameObject*)obj)->anim.currentMove;
                if (move >= 0x30 || move < 0x29)
                {
                    if (Sfx_IsPlayingFromObjectChannel((u32)obj, 0x10) == 0)
                    {
                        objAudioFn_800393f8(obj, &((TrickyState*)state)->soundState, 0x299, 0x100, -1, 0);
                    }
                }
            }
            ((TrickyState*)trickyState)->substate = TRICKYGROWL_FACE_TARGET;
            objAnimFn_8013a3f0((int)obj, 0x33, lbl_803E2444, 0x4000000);
            *(int*)((char*)trickyState + 0x728) = 0;
        }
        break;
    case TRICKYGROWL_FACE_TARGET:
        trickyDebugPrint(strBase + 0x568);
        if (*((TrickyState*)trickyState)->progressPtr != 0 && *(int*)((char*)trickyState + 0x728) != 0)
        {
            ((TrickyState*)trickyState)->substate = TRICKYGROWL_DIG_START;
        }
        else
        {
            f32* target = ((TrickyState*)((GameObject*)obj)->extra)->targetPosPtr;
            trickyTurnTowardYaw(obj,
                                getAngle(-(*(f32*)target - ((GameObject*)obj)->anim.worldPosX),
                                         -(((TrickyGrowlState*)target)->unk8 - ((GameObject*)obj)->anim.worldPosZ)));
            if (randomGetRange(0, 10) == 0)
            {
                state = ((GameObject*)obj)->extra;
                if (((((TrickyGrowlState*)state)->unk58 >> 6) & 1) == 0u)
                {
                    s16 move = ((GameObject*)obj)->anim.currentMove;
                    if (move >= 0x30 || move < 0x29)
                    {
                        if (Sfx_IsPlayingFromObjectChannel((u32)obj, 0x10) == 0)
                        {
                            objAudioFn_800393f8(obj, &((TrickyState*)state)->soundState, 0x299, 0x100, -1, 0);
                        }
                    }
                }
            }
        }
        break;
    case TRICKYGROWL_DIG_START:
        trickyDebugPrint(strBase + 0x57c);
        if (trickyFn_8013b368(obj, lbl_803E24CC, trickyState) == 0)
        {
            if ((u8)Obj_IsLoadingLocked() != 0)
            {
                ((TrickyState*)trickyState)->stateFlags =
                    ((TrickyState*)trickyState)->stateFlags | TRICKY_STATE_FLAG_CHILDREN_ACTIVE;
                for (i = 0, slot = trickyState; i < CHILD_OBJECT_COUNT; slot++, i++)
                {
                    setup = (void*)Obj_AllocObjectSetup(0x24, TRICKY_CHILD_OBJ_FLAMEBLAST);
                    *(u8*)((char*)setup + 0x4) = 2;
                    *(u8*)((char*)setup + 0x5) = 1;
                    *(s16*)((char*)setup + 0x1a) = i;
                    slot[0x700 / 4] = (void*)Obj_SetupObject((ObjPlacement*)setup, 5,
                                                             ((GameObject*)obj)->anim.mapEventSlot, -1,
                                                             ((GameObject*)obj)->anim.parent);
                }
                Sfx_PlayFromObject((u32)obj, SFXTRIG_en_cvdrip1c_3db);
                Sfx_AddLoopedObjectSound((u32)obj, SFXTRIG_trpopn_c);
            }
            (*((TrickyState*)trickyState)->progressPtr)--;
            objAnimFn_8013a3f0((int)obj, 0x34, lbl_803E2444, 0x4000000);
            ((TrickyState*)trickyState)->stateFlags = ((TrickyState*)trickyState)->stateFlags | 0x10;
            ((TrickyState*)trickyState)->substate = TRICKYGROWL_DIG_END;
            *(int*)((char*)trickyState + 0x728) = 0;
        }
        break;
    case TRICKYGROWL_DIG_END:
        trickyDebugPrint(strBase + 0x590);
        if (((GameObject*)obj)->anim.currentMoveProgress >= lbl_803E24D0)
        {
            ((TrickyState*)trickyState)->stateFlags &= ~(u64)TRICKY_STATE_FLAG_CHILDREN_ACTIVE;
            ((TrickyState*)trickyState)->stateFlags =
                ((TrickyState*)trickyState)->stateFlags | TRICKY_STATE_FLAG_CHILDREN_CLEANUP;
            for (j = 0, slot2 = trickyState; j < CHILD_OBJECT_COUNT; slot2++, j++)
            {
                objSetAnimSpeedTo1((GameObject*)slot2[0x700 / 4]);
            }
            Sfx_RemoveLoopedObjectSound((u32)obj, SFXTRIG_trpopn_c);
            digState = ((GameObject*)obj)->extra;
            if (((((TrickyGrowlState*)digState)->unk58 >> 6) & 1) == 0u)
            {
                s16 move = ((GameObject*)obj)->anim.currentMove;
                if (move >= 0x30 || move < 0x29)
                {
                    if (Sfx_IsPlayingFromObjectChannel((u32)(int)obj, 0x10) == 0)
                    {
                        objAudioFn_800393f8(obj, &((TrickyState*)digState)->soundState, 0x29d, 0, -1, 0);
                    }
                }
            }
            ((TrickyState*)trickyState)->stateIndex = 1;
            ((TrickyState*)trickyState)->substate = TRICKYGROWL_WINDUP;
            {
                f32 resetValue = lbl_803E23DC;
                ((TrickyState*)trickyState)->cooldownA = resetValue;
                ((TrickyState*)trickyState)->cooldownB.f = resetValue;
            }
            ((TrickyState*)trickyState)->stateFlags &= ~0x10LL;
            ((TrickyState*)trickyState)->stateFlags &= ~0x10000LL;
            ((TrickyState*)trickyState)->stateFlags &= ~0x20000LL;
            ((TrickyState*)trickyState)->stateFlags &= ~0x40000LL;
            {
                s8 mm = -1;
                ((TrickyState*)trickyState)->commandPhase = mm;
            }
        }
        else
        {
            f32* target = ((TrickyState*)((GameObject*)obj)->extra)->targetPosPtr;
            trickyTurnTowardYaw(obj,
                                getAngle(-(*(f32*)target - ((GameObject*)obj)->anim.worldPosX),
                                         -(((TrickyGrowlState*)target)->unk8 - ((GameObject*)obj)->anim.worldPosZ)));
        }
        break;
    }
}

/*
 * Tricky companion "circle the enemy" combat behaviour (part of the
 * tricky AI module; operates on TrickyState, the per-object scratch at
 * GameObject.extra).
 *
 * trickyFindCirclingTarget       - picks the object Tricky should circle:
 *                                   the current follow target if it is the
 *                                   special seqId 0x6a3 actor, else the
 *                                   player's lock-on target, validated
 *                                   against ObjGroup 3 by a triangle-
 *                                   inequality distance test.
 * trickyUpdateCirclingTargetPosition
 *                                - orbits Tricky around followObj: picks a
 *                                   random spin direction once, advances the
 *                                   orbit angle while it stays near the seed
 *                                   heading, and writes the desired
 *                                   x/y/z onto the state; trickyFn_8013b368
 *                                   then steers toward it.
 * trickyUpdateCircling                    - the circling state machine, dispatched on
 *                                   substate t->substate (0 acquire, 1 approach,
 *                                   2/3/4 the special charge/spawn/finish
 *                                   path, 5 orbit-and-pick-best). It spawns
 *                                   helper objects (ids 0x17b, 0x4f0),
 *                                   plays/loops bark and effect sounds, and
 *                                   drives the shared TRICKY_* state macros.
 */

/* group owned by another DLL, queried here */
#define ANIMOBJD2_OBJFLAG_FREED 0x40
/* Objects spawned by the trickyUpdateCircling state machine (retail OBJECTS.bin names
   "TrickyFood" and "flameblast"). */
#define ANIMOBJD2_TRICKY_FOOD_OBJ_ID 0x17b
#define ANIMOBJD2_FLAMEBLAST_OBJ_ID  0x4f0
/* seqId of the special actor Tricky circles when it is the current follow target (docblock: "the special seqId 0x6a3 actor") */
#define ANIMOBJD2_CIRCLE_TARGET_SEQID 0x6a3

/* trickyUpdateCircling circling substate machine (TrickyState.substate; this object's
 * own values, not a globally shared TrickyState enum). */
enum AnimObjD2Substate
{
    ANIMOBJD2_SUBSTATE_ACQUIRE = 0,  /* find/lock onto a target        */
    ANIMOBJD2_SUBSTATE_APPROACH = 1, /* close on the seed heading      */
    ANIMOBJD2_SUBSTATE_CHARGE = 2,   /* retarget + start charge anim   */
    ANIMOBJD2_SUBSTATE_SPAWN = 3,    /* spawn the 7 drip helper objects*/
    ANIMOBJD2_SUBSTATE_FINISH = 4,   /* speed up helpers, bark, reset  */
    ANIMOBJD2_SUBSTATE_ORBIT = 5     /* orbit and pick the best target */
};

/* Spawn-setup buffer seeded in the substate-3 drip burst (defNo 0x4f0).
 * Reuses ObjPlacement's color head and adds the class-specific index at
 * 0x1a; store widths per target asm (stb color, sth index). */
typedef struct AnimObjD2DripSetup
{
    ObjPlacement head; /* 0x00: color[0..1] written */
    u8 pad18[0x1a - 0x18];
    s16 index; /* 0x1a */
} AnimObjD2DripSetup;

void* trickyFindCirclingTarget(GameObject* obj, void* state);

typedef struct
{
    u8 a : 1;
    u8 b : 1;
    u8 c : 6;
} TrickyCfgBits;

#define TRICKY_STATE_FLAGS_OFFSET      0x54
#define TRICKY_STATE_FLAG_4            0x4
#define TRICKY_STATE_FLAG_800          0x800
#define TRICKY_STATE_FLAG_1000         0x1000
#define TRICKY_STATE_FLAG_8000000      0x8000000
#define TRICKY_STATE_TARGET_DIRTY_FLAG 0x00000400LL
#define TRICKY_STATE_RESET_FLAG_10     0x00000010LL
#define TRICKY_STATE_RESET_FLAG_10000  0x00010000LL
#define TRICKY_STATE_RESET_FLAG_20000  0x00020000LL
#define TRICKY_STATE_RESET_FLAG_40000  0x00040000LL

#define TRICKY_RETARGET(st, X)                                                                                         \
    {                                                                                                                  \
        u32 px = (u32) & ((GameObject*)(X))->anim.worldPosX;                                                           \
        if (*(u32*)((st) + 0x28) != px)                                                                                \
        {                                                                                                              \
            *(u32*)((st) + 0x28) = px;                                                                                 \
            {                                                                                                          \
                u32 m;                                                                                                 \
                u32 f2 = *(u32*)((st) + TRICKY_STATE_FLAGS_OFFSET);                                                    \
                m = ~TRICKY_STATE_TARGET_DIRTY_FLAG;                                                                   \
                *(u32*)((st) + TRICKY_STATE_FLAGS_OFFSET) = f2 & m;                                                    \
            }                                                                                                          \
            *(s16*)((st) + 0xd2) = 0;                                                                                  \
        }                                                                                                              \
    }

#define TRICKY_RESET_TAIL(st)                                                                                          \
    {                                                                                                                  \
        f32 z = lbl_803E23DC;                                                                                          \
        *(f32*)((st) + 0x71c) = z;                                                                                     \
        *(f32*)((st) + 0x720) = z;                                                                                     \
        *(u32*)((st) + TRICKY_STATE_FLAGS_OFFSET) &= 0xFFFFFFEFLL;                                                     \
        *(u32*)((st) + TRICKY_STATE_FLAGS_OFFSET) &= 0xFFFEFFFFLL;                                                     \
        *(u32*)((st) + TRICKY_STATE_FLAGS_OFFSET) &= 0xFFFDFFFFLL;                                                     \
        *(u32*)((st) + TRICKY_STATE_FLAGS_OFFSET) &= 0xFFFBFFFFLL;                                                     \
        *(s8*)((st) + 0xd) = 0xFF;                                                                                     \
    }
#define TRICKY_RESET(st)                                                                                               \
    *(u8*)((st) + 8) = 1;                                                                                              \
    *(u8*)((st) + 0xa) = 0;                                                                                            \
    TRICKY_RESET_TAIL(st)

#define TRICKY_BARK(obj, snd, p4)                                                                                      \
    {                                                                                                                  \
        cfg = *(u8**)&((GameObject*)(obj))->extra;                                                                     \
        if (!((TrickyCfgBits*)(cfg + 0x58))->b)                                                                        \
        {                                                                                                              \
            s16 a0 = ((GameObject*)(obj))->anim.currentMove;                                                           \
            if (a0 >= 0x30 || a0 < 0x29)                                                                               \
            {                                                                                                          \
                if (Sfx_IsPlayingFromObjectChannel((int)(obj), 0x10) == 0)                                             \
                {                                                                                                      \
                    objAudioFn_800393f8((GameObject*)(obj), &((TrickyState*)cfg)->soundState, snd, p4, -1, 0);         \
                }                                                                                                      \
            }                                                                                                          \
        }                                                                                                              \
    }

void trickyUpdateCircling(GameObject* gobj, TrickyState* t)
{
    char* str = lbl_8031D2E8;
    u8 ok;
    int go;
    int* best = NULL;
    f32 detourRatio;
    f32 bestd = lbl_803E23DC;
    int count;
    u8* cfg;

    switch (t->substate)
    {
    case ANIMOBJD2_SUBSTATE_ACQUIRE:
    {
        trickyDebugPrint(str + 0x5a0);
        ok = trickyFn_8013b368(gobj, lbl_803E24D4, t);
        if ((t->followObj = trickyFindNearestUsableBaddie(t->playerObj, lbl_803E24D8, 0)) != NULL)
        {
            TRICKY_RETARGET((u8*)t, t->followObj);
            go = 1;
        }
        else
        {
            t->stateIndex = 1;
            t->substate = go = 0;
            TRICKY_RESET_TAIL((u8*)t)
        }
        if (go != 0)
        {
            if (*(int*)&t->stateFlags728 == 0)
            {
                {
                    void* ct = trickyFindCirclingTarget((GameObject*)(gobj), t);
                    t->cooldownB.ptr = ct;
                    if (ct != NULL)
                    {
                        t->followObj = t->cooldownB.obj;
                        *(int*)&t->unk724 = 0;
                        t->substate = ANIMOBJD2_SUBSTATE_ORBIT;
                        break;
                    }
                }
            }
            if (ok == 2)
            {
                TRICKY_RESET((u8*)t);
                break;
            }
            if (getXZDistance(&gobj->anim.worldPosX, &t->followObj->anim.worldPosX) <
                lbl_803E24DC)
            {
                int b;
                f32 z;
                t->substate = ANIMOBJD2_SUBSTATE_APPROACH;
                b = 1;
                z = lbl_803E23DC;
                t->cooldownA = z;
                if (z == t->waterLevel)
                {
                    b = 0;
                }
                else if (lbl_803E2410 == t->eventTime)
                {
                    b = 1;
                }
                else if (t->currentTime - t->eventTime > lbl_803E2414)
                {
                    b = 1;
                }
                else
                {
                    b = 0;
                }
                if (b != 0)
                {
                    objAnimFn_8013a3f0((int)gobj, 8, lbl_803E243C, 0);
                    t->cooldownC = lbl_803E2440;
                    t->particleTimer = lbl_803E23DC;
                    trickyDebugPrint(str + 0x184);
                }
                else
                {
                    objAnimFn_8013a3f0((int)gobj, 0, lbl_803E2444, 0);
                    trickyDebugPrint(str + 0x190);
                }
            }
        }
        break;
    }
    case ANIMOBJD2_SUBSTATE_APPROACH:
    {
        trickyDebugPrint(str + 0x5b4, *t->progressPtr, *(int*)&t->stateFlags728);
        ok = trickyFn_8013b368(gobj, lbl_803E24D4, t);
        if ((t->followObj = trickyFindNearestUsableBaddie(t->playerObj, lbl_803E24D8, 0)) != NULL)
        {
            TRICKY_RETARGET((u8*)t, t->followObj);
            go = 1;
        }
        else
        {
            t->stateIndex = 1;
            t->substate = go = 0;
            TRICKY_RESET_TAIL((u8*)t)
        }
        if (go != 0)
        {
            if (*(int*)&t->stateFlags728 == 0)
            {
                {
                    void* ct = trickyFindCirclingTarget((GameObject*)(gobj), t);
                    t->cooldownB.ptr = ct;
                    if (ct != NULL)
                    {
                        t->followObj = t->cooldownB.obj;
                        *(int*)&t->unk724 = 0;
                        t->substate = ANIMOBJD2_SUBSTATE_ORBIT;
                        break;
                    }
                }
            }
            if (ok == 2)
            {
                TRICKY_RESET((u8*)t);
                break;
            }
            if (ok == 0)
            {
                objAnimFn_8013a3f0((int)gobj, 0x33, lbl_803E243C, 0);
            }
            if (*(int*)&t->stateFlags728 != 0)
            {
                if (*t->progressPtr < 2)
                {
                    *(int*)&t->stateFlags728 = 0;
                    if (Obj_IsLoadingLocked() != 0)
                    {
                        t->stateFlags |= TRICKY_STATE_FLAG_4;
                        TRICKY_RESET((u8*)t);
                        if (t->child == NULL)
                        {
                            int o = (int)Obj_AllocObjectSetup(0x20, ANIMOBJD2_TRICKY_FOOD_OBJ_ID);
                            s8 slots[4];
                            int free_;
                            slots[0] = -1;
                            slots[1] = -1;
                            slots[2] = -1;
                            if (t->childA != NULL)
                            {
                                slots[((TrickyPackedSlots*)((char*)t + 0x7bc))->promptASlot] = 1;
                            }
                            if (t->childB != NULL)
                            {
                                slots[((TrickyPackedSlots*)((char*)t + 0x7bc))->promptBSlot] = 1;
                            }
                            if (t->child != NULL)
                            {
                                slots[((TrickyPackedSlots*)((char*)t + 0x7bc))->zzzSlot] = 1;
                            }
                            if (slots[0] == -1)
                            {
                                free_ = 0;
                            }
                            else if (slots[1] == -1)
                            {
                                free_ = 1;
                            }
                            else if (slots[2] == -1)
                            {
                                free_ = 2;
                            }
                            else if (slots[3] == -1)
                            {
                                free_ = 3;
                            }
                            else
                            {
                                free_ = -1;
                            }
                            ((TrickyPackedSlots*)((char*)t + 0x7bc))->zzzSlot = free_;
                            *(int*)&t->child =
                                (int)Obj_SetupObject((ObjPlacement*)o, 4, -1, -1, (void*)*(int*)&gobj->anim.parent);
                            ObjLink_AttachChild(gobj, t->child, ((TrickyPackedSlots*)((char*)t + 0x7bc))->zzzSlot);
                            {
                                f32 z3 = lbl_803E23DC;
                                t->childPhaseTimer0 = z3;
                                t->childPhaseTimer1 = z3;
                                t->childPhaseTimer2 = z3;
                            }
                        }
                    }
                }
                else
                {
                    t->substate = ANIMOBJD2_SUBSTATE_CHARGE;
                    break;
                }
            }
            if (getXZDistance(&gobj->anim.worldPosX, &t->followObj->anim.worldPosX) >
                lbl_803E24E0)
            {
                t->substate = ANIMOBJD2_SUBSTATE_ACQUIRE;
                break;
            }
            t->cooldownA -= timeDelta;
            if (t->cooldownA < lbl_803E23DC)
            {
                f32 rv;
                rv = (s32)randomGetRange(0xc8, 0x258);
                t->cooldownA = rv * lbl_803E24A8;
                TRICKY_BARK((int*)gobj, 0x29b, 0x1000);
            }
        }
        break;
    }
    case ANIMOBJD2_SUBSTATE_CHARGE:
    {
        trickyDebugPrint(str + 0x5cc);
        ok = trickyFn_8013b368(gobj, lbl_803E24E4, t);
        if ((t->followObj = trickyFindNearestUsableBaddie(t->playerObj, lbl_803E24D8, 0)) != NULL)
        {
            TRICKY_RETARGET((u8*)t, t->followObj);
            go = 1;
        }
        else
        {
            t->stateIndex = 1;
            t->substate = go = 0;
            TRICKY_RESET_TAIL((u8*)t)
        }
        if (go != 0 && ok != 1)
        {
            objAnimFn_8013a3f0((int)gobj, 0x34, lbl_803E2444, 0x4000000);
            t->stateFlags |= TRICKY_STATE_RESET_FLAG_10;
            t->substate = ANIMOBJD2_SUBSTATE_SPAWN;
            *(int*)&t->stateFlags728 = 0;
        }
        break;
    }
    case ANIMOBJD2_SUBSTATE_SPAWN:
        if (gobj->anim.currentMove != 0x34)
        {
            break;
        }
        if (gobj->anim.currentMoveProgress > lbl_803E24E8)
        {
            if (Obj_IsLoadingLocked() != 0)
            {
                t->stateFlags |= TRICKY_STATE_FLAG_800;
                {
                    int i = 0;
                    u8* p = (u8*)t;
                    for (; i < 7; i++)
                    {
                        int o = (int)Obj_AllocObjectSetup(0x24, ANIMOBJD2_FLAMEBLAST_OBJ_ID);
                        ((AnimObjD2DripSetup*)o)->head.color[0] = 2;
                        ((AnimObjD2DripSetup*)o)->head.color[1] = 1;
                        ((AnimObjD2DripSetup*)o)->index = i;
                        *(int*)(p + 0x700) = (int)Obj_SetupObject((ObjPlacement*)o, 5, gobj->anim.mapEventSlot, -1,
                                                                  (void*)*(int*)&gobj->anim.parent);
                        p += 4;
                    }
                }
                Sfx_PlayFromObject((u32)gobj, SFXTRIG_en_cvdrip1c_3db);
                Sfx_AddLoopedObjectSound((u32)gobj, SFXTRIG_trpopn_c);
            }
            *t->progressPtr -= 2;
            t->substate = ANIMOBJD2_SUBSTATE_FINISH;
        }
        break;
    case ANIMOBJD2_SUBSTATE_FINISH:
    {
        u32 fl;
        trickyDebugPrint(str + 0x5e4);
        fl = t->stateFlags;
        if (fl & TRICKY_STATE_FLAG_8000000)
        {
            t->stateFlags = fl & ~(u64)TRICKY_STATE_FLAG_800;
            t->stateFlags |= TRICKY_STATE_FLAG_1000;
            {
                u8* p;
                int i = 0;
                p = (u8*)t;
                for (; i < 7; i++)
                {
                    objSetAnimSpeedTo1((GameObject*)*(int*)(p + 0x700));
                    p += 4;
                }
            }
            Sfx_RemoveLoopedObjectSound((u32)gobj, SFXTRIG_trpopn_c);
            TRICKY_BARK((int*)gobj, 0x29d, 0);
            {
                u32 m;
                u32 f2 = t->stateFlags;
                m = ~TRICKY_STATE_RESET_FLAG_10;
                t->stateFlags = f2 & m;
            }
            t->substate = ANIMOBJD2_SUBSTATE_ACQUIRE;
        }
        break;
    }
    case ANIMOBJD2_SUBSTATE_ORBIT:
    {
        void** p;
        GameObject* tgt;
        GameObject* found = trickyFindNearestUsableBaddie(t->playerObj, lbl_803E24D8, 0);
        if (found != NULL && found->anim.seqId == ANIMOBJD2_CIRCLE_TARGET_SEQID)
        {
            tgt = found;
        }
        else
        {
            tgt = (GameObject*)Player_GetTargetObject((int)t->playerObj);
        }
        if ((u32)tgt != t->cooldownB.u || *(int*)&t->stateFlags728 != 0)
        {
            TRICKY_RETARGET((u8*)t, t->followObj);
            t->substate = ANIMOBJD2_SUBSTATE_ACQUIRE;
        }
        else
        {
            void** list = (void**)ObjGroup_GetObjects(TRICKYWARP_OBJ_GROUP, &count);
            int i = 0;
            p = list;
            detourRatio = lbl_803E23F8;
            for (; i < count; i++)
            {
                f32 d1 = Vec_xzDistance(&((GameObject*)p[0])->anim.worldPosX, &tgt->anim.worldPosX);
                f32 d2 = Vec_xzDistance(&((GameObject*)p[0])->anim.worldPosX,
                                        &t->playerObj->anim.worldPosX);
                f32 d3 = Vec_xzDistance(&tgt->anim.worldPosX, &t->playerObj->anim.worldPosX);
                if (d1 + d2 > detourRatio * d3)
                {
                    f32 d4 = Vec_xzDistance(&((GameObject*)p[0])->anim.worldPosX, &gobj->anim.worldPosX);
                    if (d2 - d4 > bestd)
                    {
                        bestd = d2 - d4;
                        best = p[0];
                    }
                }
                p++;
            }
            {
                int* c = *(int**)&t->unk724;
                if (c != NULL && (((GameObject*)c)->objectFlags & ANIMOBJD2_OBJFLAG_FREED))
                {
                    *(int*)&t->unk724 = 0;
                    TRICKY_RETARGET((u8*)t, t->playerObj);
                }
            }
            if (best != NULL)
            {
                /* unk724 NULL-checks kept raw: typing as ->unk724 shifts
                       saved-register coloring and regresses (the int reads/
                       stores below are byte-neutral as fields). */
                if (*(void**)((u8*)t + 0x724) == NULL)
                {
                    TRICKY_BARK((int*)gobj, 0x35b, 0x500);
                }
                if (*(void**)((u8*)t + 0x724) == NULL || *(int**)&t->unk724 != best)
                {
                    *(int**)&t->unk724 = best;
                    TRICKY_RETARGET((u8*)t, *(int*)&t->unk724);
                }
            }
        }
        {
            u8 r;
            if (*(void**)((u8*)t + 0x724) != NULL)
            {
                r = trickyFn_8013b368(gobj, lbl_803E2488, t);
            }
            else
            {
                r = trickyFn_8013b368(gobj, lbl_803E2418, t);
            }
            if (r != 1)
            {
                int b;
                if (lbl_803E23DC == t->waterLevel)
                {
                    b = 0;
                }
                else if (lbl_803E2410 == t->eventTime)
                {
                    b = 1;
                }
                else if (t->currentTime - t->eventTime > lbl_803E2414)
                {
                    b = 1;
                }
                else
                {
                    b = 0;
                }
                if (b != 0)
                {
                    objAnimFn_8013a3f0((int)gobj, 8, lbl_803E243C, 0);
                    t->cooldownC = lbl_803E2440;
                    t->particleTimer = lbl_803E23DC;
                    trickyDebugPrint(str + 0x184);
                }
                else
                {
                    objAnimFn_8013a3f0((int)gobj, 0, lbl_803E2444, 0);
                    trickyDebugPrint(str + 0x190);
                }
            }
        }
        break;
    }
    }
}

void* trickyFindCirclingTarget(GameObject* obj, void* state)
{
    GameObject* target;
    void** list;
    int count;
    int i;
    f32 d1, d2, d3;

    target = (GameObject*)((TrickyState*)state)->followObj;
    if (target->anim.seqId == ANIMOBJD2_CIRCLE_TARGET_SEQID)
    {
        return target;
    }

    target = (GameObject*)fn_80296118(((TrickyState*)state)->playerObj);
    if (target != NULL)
    {
        list = (void**)ObjGroup_GetObjects(3, &count);
        for (i = 0; i < count; i++)
        {
            if ((GameObject*)*list == target)
            {
                d1 = Vec_xzDistance(&obj->anim.worldPosX, &target->anim.worldPosX);
                d2 = Vec_xzDistance(&obj->anim.worldPosX,
                                    &((TrickyState*)state)->playerObj->anim.worldPosX);
                d3 = Vec_xzDistance(&target->anim.worldPosX,
                                    &((TrickyState*)state)->playerObj->anim.worldPosX);
                if ((d1 + d2) < lbl_803E23F8 * d3)
                {
                    return target;
                }
                break;
            }
            list++;
        }
    }
    return NULL;
}

void trickyUpdateCirclingTargetPosition(void* objPtr, void* state)
{
    GameObject* obj = (GameObject*)objPtr;
    GameObject* target = ((TrickyState*)state)->followObj;
    f32 dx = target->anim.worldPosX - obj->anim.worldPosX;
    f32 dz = target->anim.worldPosZ - obj->anim.worldPosZ;
    int angle = atan2_8002178c(dx, dz);
    s32 delta;
    s32 absDelta;

    if (((TrickyState*)state)->substate == ANIMOBJD2_SUBSTATE_ACQUIRE)
    {
        ((TrickyState*)state)->scratch700.i = randomGetRange(0, 1);
        if (((TrickyState*)state)->scratch700.i == 0)
        {
            ((TrickyState*)state)->scratch700.i = -1;
        }
        ((TrickyState*)state)->scratch704.i = angle;
        ((TrickyState*)state)->substate = ANIMOBJD2_SUBSTATE_APPROACH;
    }

    delta = angle - (s32)(u16)((TrickyState*)state)->scratch704.u;
    if (delta > 0x8000)
        delta -= 0xFFFF;
    if (delta < -0x8000)
        delta += 0xFFFF;

    if (delta >= 0)
    {
        absDelta = delta;
    }
    else
    {
        absDelta = -delta;
    }
    if (absDelta < 0x2000)
    {
        ((TrickyState*)state)->scratch704.i =
            ((TrickyState*)state)->scratch704.i + (((TrickyState*)state)->scratch700.i << 11);
    }

    ((TrickyState*)state)->scratch708.f = ((TrickyState*)state)->followObj->anim.worldPosX -
                                          lbl_803E24D4 * fsin16Precise((u16)((TrickyState*)state)->scratch704.i);
    ((TrickyState*)state)->scratch70C.f = ((TrickyState*)state)->followObj->anim.worldPosY;
    ((TrickyState*)state)->scratch710.f = ((TrickyState*)state)->followObj->anim.worldPosZ -
                                          lbl_803E24D4 * fcos16Precise((u16)((TrickyState*)state)->scratch704.i);

    if (trickyFn_8013b368(objPtr, lbl_803E2488, state) == 0)
    {
        trickyReportError(sTrickyShouldNeverStopCirclingError);
    }
}

const char sTrickyShouldNeverStopCirclingError[] = "error tricky should never stop when circling\n";

/*
 * Tricky companion-AI substate handlers (TrickyState::substate machines).
 *
 * Each entry point is one behavior tick dispatched off TrickyState->substate:
 *   tricky_fetchBall - fetch/carry-ball behavior (grab a thrown ball via
 *                      sidekickBall_* entry points, swim or walk to it, return it).
 *   tricky_idleAndEat - idle/eat ambient state (random bark cues, eating anim).
 *   tricky_trackTumbleweed - track a TumbleweedBush target and steer Tricky toward it,
 *                 gated by game bit 0x48b.
 *   tricky_moveToFollowTarget - simple swim-or-walk move toward the follow target.
 *
 * Common to all: water is detected by comparing waterLevel / eventTime /
 * currentTime to pick a swim anim vs a ground anim. tricky_fetchBall and tricky_idleAndEat play a
 * localized bark sfx unless one is already on object channel 16. Debug strings
 * are emitted via
 * trickyDebugPrint. tricky_state.h owns the TrickyState layout; the lbl_803E*
 * floats are pooled .sdata2 tuning constants shared throughout this DLL.
 *
 * tricky_fetchBall's case numbering/fallthrough (0 into 1, 4 into 5 via the label
 * inside the if) is ground truth from the retail jump table at 0x8031D910 --
 * do not renumber or "un-nest" case 5.
 */

typedef struct
{
    u8 hi : 4;
    u8 pad : 4;
} TrickyNibblePair;

#define TRICKY_CLEAR_RESET_FLAGS(st)                                                                                   \
    {                                                                                                                  \
        *(u32*)&((TrickyState*)(st))->stateFlags &= ~(u64)TRICKY_STATE_RESET_FLAG_10;                                  \
        *(u32*)&((TrickyState*)(st))->stateFlags &= ~(u64)TRICKY_STATE_RESET_FLAG_10000;                               \
        *(u32*)&((TrickyState*)(st))->stateFlags &= ~(u64)TRICKY_STATE_RESET_FLAG_20000;                               \
        *(u32*)&((TrickyState*)(st))->stateFlags &= ~(u64)TRICKY_STATE_RESET_FLAG_40000;                               \
        {                                                                                                              \
            s8 mm;                                                                                                     \
            mm = -1;                                                                                                   \
            *(s8*)&((TrickyState*)(st))->commandPhase = mm;                                                            \
        }                                                                                                              \
    }

void tricky_fetchBall(GameObject* obj, register int state)
{
    int status;
    int extra;
    int useSwimAnim;
    s16 move;
    f32 bob;
    f32 resetTimer;
    f32* targetPos;

    switch (((TrickyState*)state)->substate)
    {
    case 0:
        ((TrickyState*)state)->scratch700.ptr = ((TrickyState*)state)->followObj;
        ((TrickyState*)state)->scratch704.f = lbl_803E24EC;
        ((TrickyState*)state)->substate = 1;
        ((TrickyState*)state)->sfxIntervalTimer = (f32)(s32)randomGetRange(150, 300);
        /* fall through */
    case 1:
        if (sidekickBall_isHeldOrMoving((GameObject*)((TrickyState*)state)->scratch700.i) != 0)
        {
            status = trickyFn_8013b368(obj, lbl_803E24F0, (TrickyState*)state);
            if (status == 0)
            {
                if (lbl_803E23DC == ((TrickyState*)state)->waterLevel)
                {
                    useSwimAnim = 0;
                }
                else if (lbl_803E2410 == ((TrickyState*)state)->eventTime)
                {
                    useSwimAnim = 1;
                }
                else if (((TrickyState*)state)->currentTime - ((TrickyState*)state)->eventTime > lbl_803E2414)
                {
                    useSwimAnim = 1;
                }
                else
                {
                    useSwimAnim = 0;
                }
                if (useSwimAnim != 0)
                {
                    objAnimFn_8013a3f0((int)obj, 28, lbl_803E24F4, 0x4000000);
                }
                else
                {
                    objAnimFn_8013a3f0((int)obj, 17, lbl_803E24F4, 0x4000000);
                }
                ((TrickyState*)state)->stateFlags |= TRICKY_STATE_RESET_FLAG_10;
                ((TrickyState*)state)->substate = 3;
                sidekickBall_setIdle((GameObject*)(((TrickyState*)state)->scratch700.i), obj);
            }
            else if (status == 2)
            {
                extra = *(int*)&(obj)->extra;
                if ((((u32) * (u8*)(extra + 0x58) >> 6) & 1) == 0)
                {
                    move = (obj)->anim.currentMove;
                    if (move >= 48 || move < 41)
                    {
                        if (Sfx_IsPlayingFromObjectChannel((int)obj, 16) == 0)
                        {
                            objAudioFn_800393f8(obj, &((TrickyState*)extra)->soundState, 861, 1280, -1, 0);
                        }
                    }
                }
                ((TrickyState*)state)->stateIndex = 1;
                ((TrickyState*)state)->substate = 0;
                resetTimer = lbl_803E23DC;
                ((TrickyState*)state)->cooldownA = resetTimer;
                ((TrickyState*)state)->cooldownB.f = resetTimer;
                TRICKY_CLEAR_RESET_FLAGS(state);
            }
        }
        else
        {
            status = trickyFn_8013b368(obj, lbl_803E2408, (TrickyState*)state);
            if (status == 0)
            {
                if (((TrickyState*)state)->scratch704.f > lbl_803E23DC)
                {
                    if (lbl_803E23DC == ((TrickyState*)state)->waterLevel)
                    {
                        useSwimAnim = 0;
                    }
                    else if (lbl_803E2410 == ((TrickyState*)state)->eventTime)
                    {
                        useSwimAnim = 1;
                    }
                    else if (((TrickyState*)state)->currentTime - ((TrickyState*)state)->eventTime > lbl_803E2414)
                    {
                        useSwimAnim = 1;
                    }
                    else
                    {
                        useSwimAnim = 0;
                    }
                    if (useSwimAnim != 0)
                    {
                        objAnimFn_8013a3f0((int)obj, 8, lbl_803E243C, 0);
                        ((TrickyState*)state)->cooldownC = lbl_803E2440;
                        ((TrickyState*)state)->particleTimer = lbl_803E23DC;
                        trickyDebugPrint(sInWaterMessage);
                    }
                    else
                    {
                        objAnimFn_8013a3f0((int)obj, 0, lbl_803E2444, 0);
                        trickyDebugPrint(lbl_8031D478);
                    }
                    ((TrickyState*)state)->scratch704.f -= timeDelta;
                    if (((TrickyState*)state)->scratch704.f <= lbl_803E23DC)
                    {
                        if (lbl_803E23DC == ((TrickyState*)state)->waterLevel)
                        {
                            useSwimAnim = 0;
                        }
                        else if (lbl_803E2410 == ((TrickyState*)state)->eventTime)
                        {
                            useSwimAnim = 1;
                        }
                        else if (((TrickyState*)state)->currentTime - ((TrickyState*)state)->eventTime > lbl_803E2414)
                        {
                            useSwimAnim = 1;
                        }
                        else
                        {
                            useSwimAnim = 0;
                        }
                        if (useSwimAnim != 0)
                        {
                            ((TrickyState*)state)->scratch704.f = lbl_803E24EC;
                        }
                        else
                        {
                            ((TrickyState*)state)->scratch708.f = lbl_803E24F8;
                        }
                    }
                }
                else
                {
                    objAnimFn_8013a3f0((int)obj, 16, lbl_803E243C, 0x4000000);
                    ((TrickyState*)state)->scratch708.f -= timeDelta;
                    if (((TrickyState*)state)->scratch708.f <= lbl_803E23DC)
                    {
                        ((TrickyState*)state)->scratch704.f = lbl_803E24EC;
                    }
                }
            }
            else if (status == 1)
            {
                ((TrickyState*)state)->sfxIntervalTimer -= timeDelta;
                if (((TrickyState*)state)->sfxIntervalTimer <= lbl_803E23DC)
                {
                    ((TrickyState*)state)->sfxIntervalTimer = (f32)(s32)randomGetRange(150, 300);
                    extra = *(int*)&(obj)->extra;
                    if ((((u32) * (u8*)(extra + 0x58) >> 6) & 1) != 0)
                    {
                        break;
                    }
                    move = (obj)->anim.currentMove;
                    if (move < 48)
                    {
                        if (move >= 41)
                        {
                            break;
                        }
                    }
                    if (Sfx_IsPlayingFromObjectChannel((int)obj, 16) == 0)
                    {
                        objAudioFn_800393f8(obj, &((TrickyState*)extra)->soundState, 865, 1280, -1, 0);
                    }
                }
            }
            else
            {
                if (lbl_803E23DC == ((TrickyState*)state)->waterLevel)
                {
                    useSwimAnim = 0;
                }
                else if (lbl_803E2410 == ((TrickyState*)state)->eventTime)
                {
                    useSwimAnim = 1;
                }
                else if (((TrickyState*)state)->currentTime - ((TrickyState*)state)->eventTime > lbl_803E2414)
                {
                    useSwimAnim = 1;
                }
                else
                {
                    useSwimAnim = 0;
                }
                if (useSwimAnim != 0)
                {
                    objAnimFn_8013a3f0((int)obj, 8, lbl_803E243C, 0);
                    ((TrickyState*)state)->cooldownC = lbl_803E2440;
                    ((TrickyState*)state)->particleTimer = lbl_803E23DC;
                    trickyDebugPrint(sInWaterMessage);
                }
                else
                {
                    objAnimFn_8013a3f0((int)obj, 0, lbl_803E2444, 0);
                    trickyDebugPrint(lbl_8031D478);
                }
            }
        }
        break;
    case 6:
        if ((obj)->anim.currentMoveProgress >= lbl_803E24FC)
        {
            status = ((TrickyState*)state)->scratch700.i;
            *(float*)(status + 0x10) += lbl_803E2488;
            bob = -mathCosf(lbl_803E2454 * (f32)(s32) * (short*)obj / lbl_803E2458);
            sidekickBall_launch((GameObject*)((TrickyState*)state)->scratch700.i, obj,
                                -mathSinf(lbl_803E2454 * (f32)(s32) * (short*)obj / lbl_803E2458),
                                lbl_803E23E8, bob);
            ((TrickyState*)state)->substate = 2;
        }
        break;
    case 2:
        if ((((TrickyState*)state)->stateFlags & TRICKY_STATE_FLAG_MOVE_ADVANCING) != 0)
        {
            *(float*)(state + 0x828) = lbl_803E2408;
            if (((TrickyState*)state)->progressPtr[2] >= 0xef)
            {
                ((TrickyState*)state)->progressPtr[2] = 0;
            }
            else
            {
                ((TrickyState*)state)->progressPtr[2]++;
            }
            {
                u32 m;
                u32 f2 = ((TrickyState*)state)->stateFlags;
                m = ~TRICKY_STATE_RESET_FLAG_10;
                ((TrickyState*)state)->stateFlags = f2 & m;
            }
            ((TrickyState*)state)->substate = 7;
            targetPos = &((TrickyState*)state)->followObj->anim.worldPosX;
            if (((TrickyState*)state)->targetPosPtr != targetPos)
            {
                ((TrickyState*)state)->targetPosPtr = targetPos;
                {
                    u32 m;
                    u32 f2 = ((TrickyState*)state)->stateFlags;
                    m = ~TRICKY_STATE_TARGET_DIRTY_FLAG;
                    ((TrickyState*)state)->stateFlags = f2 & m;
                }
                ((TrickyState*)state)->linkedWalkGroup = 0;
            }
        }
        break;
    case 7:
        status = trickyFn_8013b368(obj, lbl_803E2408, (TrickyState*)state);
        if (status != 1)
        {
            if (lbl_803E23DC == ((TrickyState*)state)->waterLevel)
            {
                useSwimAnim = 0;
            }
            else if (lbl_803E2410 == ((TrickyState*)state)->eventTime)
            {
                useSwimAnim = 1;
            }
            else if (((TrickyState*)state)->currentTime - ((TrickyState*)state)->eventTime > lbl_803E2414)
            {
                useSwimAnim = 1;
            }
            else
            {
                useSwimAnim = 0;
            }
            if (useSwimAnim != 0)
            {
                objAnimFn_8013a3f0((int)obj, 8, lbl_803E243C, 0);
                ((TrickyState*)state)->cooldownC = lbl_803E2440;
                ((TrickyState*)state)->particleTimer = lbl_803E23DC;
                trickyDebugPrint(sInWaterMessage);
            }
            else
            {
                objAnimFn_8013a3f0((int)obj, 0, lbl_803E2444, 0);
                trickyDebugPrint(lbl_8031D478);
            }
            return;
        }
        if (sidekickBall_isIdle(((TrickyState*)state)->followObj) != 0)
        {
            ((TrickyState*)state)->scratch704.f = lbl_803E24EC;
            ((TrickyState*)state)->substate = 1;
        }
        break;
    case 3:
        if ((obj)->anim.currentMoveProgress >= lbl_803E24A8)
        {
            ((TrickyState*)state)->substate = 4;
        }
        break;
    case 4:
        if ((obj)->anim.currentMoveProgress >= lbl_803E24D0)
        {
            targetPos = &((TrickyState*)state)->playerObj->anim.worldPosX;
            if (((TrickyState*)state)->targetPosPtr != targetPos)
            {
                ((TrickyState*)state)->targetPosPtr = targetPos;
                {
                    u32 m;
                    u32 f2 = ((TrickyState*)state)->stateFlags;
                    m = ~TRICKY_STATE_TARGET_DIRTY_FLAG;
                    ((TrickyState*)state)->stateFlags = f2 & m;
                }
                ((TrickyState*)state)->linkedWalkGroup = 0;
            }
            ((TrickyState*)state)->substate = 5;
        case 5:
            if (trickyFn_8013b368(obj, lbl_803E24C8, (TrickyState*)state) == 0)
            {
                if (lbl_803E23DC == ((TrickyState*)state)->waterLevel)
                {
                    useSwimAnim = 0;
                }
                else if (lbl_803E2410 == ((TrickyState*)state)->eventTime)
                {
                    useSwimAnim = 1;
                }
                else if (((TrickyState*)state)->currentTime - ((TrickyState*)state)->eventTime > lbl_803E2414)
                {
                    useSwimAnim = 1;
                }
                else
                {
                    useSwimAnim = 0;
                }
                if (useSwimAnim != 0)
                {
                    objAnimFn_8013a3f0((int)obj, 29, lbl_803E24F4, 0x4000000);
                }
                else
                {
                    objAnimFn_8013a3f0((int)obj, 19, lbl_803E24F4, 0x4000000);
                }
                ((TrickyState*)state)->substate = 6;
            }
        }
        break;
    }
    if (((((TrickyState*)state)->stateFlags & 0x10000) != 0) &&
        ViewFrustum_IsSphereVisible(&(obj)->anim.localPosX, lbl_803E2500) == 0)
    {
        Obj_FreeObject(((TrickyState*)state)->followObj);
    }
    else
    {
        sidekickBall_keepAlive((GameObject*)((TrickyState*)state)->scratch700.i);
    }
}

void tricky_idleAndEat(GameObject* obj, int state)
{
    int extra;
    int inWater;
    s16 move;

    if (tricky_handleFeedOrTalk(obj, (int*)state) == 0)
    {
        if (trickyFn_8013b368(obj, lbl_803E2488, (TrickyState*)state) == 0)
        {
            ((TrickyState*)state)->idleSfxTimer -= timeDelta;
            if (((TrickyState*)state)->idleSfxTimer <= lbl_803E23DC)
            {
                ((TrickyState*)state)->idleSfxTimer = (f32)(s32)randomGetRange(500, 750);
                extra = *(int*)&obj->extra;
                if ((((u32) * (u8*)(extra + 0x58) >> 6) & 1) == 0)
                {
                    move = obj->anim.currentMove;
                    if (move >= 48 || move < 41)
                    {
                        if (Sfx_IsPlayingFromObjectChannel((int)obj, 16) == 0)
                        {
                            objAudioFn_800393f8(obj, &((TrickyState*)extra)->soundState, 864, 1280, -1, 0);
                        }
                    }
                }
            }
            if (lbl_803E23DC == ((TrickyState*)state)->waterLevel)
            {
                inWater = 0;
            }
            else if (lbl_803E2410 == ((TrickyState*)state)->eventTime)
            {
                inWater = 1;
            }
            else if (((TrickyState*)state)->currentTime - ((TrickyState*)state)->eventTime > lbl_803E2414)
            {
                inWater = 1;
            }
            else
            {
                inWater = 0;
            }
            if (inWater != 0)
            {
                objAnimFn_8013a3f0((int)obj, 8, lbl_803E243C, 0);
                ((TrickyState*)state)->cooldownC = lbl_803E2440;
                ((TrickyState*)state)->particleTimer = lbl_803E23DC;
                trickyDebugPrint(sInWaterMessage);
            }
            else
            {
                switch (obj->anim.currentMove)
                {
                case 13:
                    if ((((TrickyState*)state)->stateFlags & TRICKY_STATE_FLAG_MOVE_ADVANCING) != 0)
                    {
                        objAnimFn_8013a3f0((int)obj, 49, lbl_803E243C, 0);
                    }
                    break;
                case 49:
                    break;
                default:
                    objAnimFn_8013a3f0((int)obj, 13, lbl_803E2444, 0);
                    break;
                }
                trickyDebugPrint(lbl_8031D478);
            }
        }
    }
}

void tricky_trackTumbleweed(GameObject* obj, register int state)
{
    int inWater;
    float dx;
    float dz;
    float distance;
    f32 resetTimer;
    float* targetPos;
    GameObject* trackedObj;
    u32 currentBit;
    u8 bitIndex;
    u8 newBit;

    switch (((TrickyState*)state)->substate)
    {
    case 0:
        newBit = mainGetBit(GAMEBIT_NW_MammothTumbleweedCount);
        ((TrickyNibblePair*)&((TrickyState*)state)->scratch700)->hi = newBit;
        ((TrickyState*)state)->scratch710.i = 0;
        ((TrickyState*)state)->substate = 1;
    case 1:
        currentBit = mainGetBit(GAMEBIT_NW_MammothTumbleweedCount);
        bitIndex = ((TrickyNibblePair*)&((TrickyState*)state)->scratch700)->hi;
        if (bitIndex != currentBit)
        {
            ((TrickyNibblePair*)&((TrickyState*)state)->scratch700)->hi++;
            **(u8**)state -= 2;
        }
        targetPos = NW_mammoth_getSpawnPosition(((TrickyState*)state)->followObj);
        trackedObj = tumbleweedbush_findNearestActive(targetPos);
        if (trackedObj != 0 && **(u8**)state != 0)
        {
            if (trackedObj != ((TrickyState*)state)->scratch710.obj &&
                (u8*)((TrickyState*)state)->targetPosPtr != (u8*)(state + 0x704))
            {
                ((TrickyState*)state)->targetPosPtr = (f32*)(state + 0x704);
                {
                    u32 m;
                    u32 f2 = ((TrickyState*)state)->stateFlags;
                    m = ~TRICKY_STATE_TARGET_DIRTY_FLAG;
                    ((TrickyState*)state)->stateFlags = f2 & m;
                }
                ((TrickyState*)state)->linkedWalkGroup = 0;
            }
            dx = *targetPos - obj->anim.worldPosX;
            dz = targetPos[2] - obj->anim.worldPosZ;
            distance = sqrtf(dx * dx + dz * dz);
            if (lbl_803E23DC != distance)
            {
                dx = dx / distance;
                dz = dz / distance;
            }
            distance = lbl_803E24D4;
            ((TrickyState*)state)->scratch704.f = -(distance * dx - trackedObj->anim.worldPosX);
            ((TrickyState*)state)->scratch708.f = trackedObj->anim.worldPosY;
            ((TrickyState*)state)->scratch70C.f = -(distance * dz - trackedObj->anim.worldPosZ);
            if (trickyFn_8013b368(obj, lbl_803E2488, (TrickyState*)state) == 0)
            {
                if (lbl_803E23DC == ((TrickyState*)state)->waterLevel)
                {
                    inWater = 0;
                }
                else if (lbl_803E2410 == ((TrickyState*)state)->eventTime)
                {
                    inWater = 1;
                }
                else if (((TrickyState*)state)->currentTime - ((TrickyState*)state)->eventTime > lbl_803E2414)
                {
                    inWater = 1;
                }
                else
                {
                    inWater = 0;
                }
                if (inWater != 0)
                {
                    objAnimFn_8013a3f0((int)obj, 8, lbl_803E243C, 0);
                    ((TrickyState*)state)->cooldownC = lbl_803E2440;
                    ((TrickyState*)state)->particleTimer = lbl_803E23DC;
                    trickyDebugPrint(sInWaterMessage);
                }
                else
                {
                    objAnimFn_8013a3f0((int)obj, 0, lbl_803E2444, 0);
                    trickyDebugPrint(lbl_8031D478);
                }
            }
        }
        else
        {
            ((TrickyState*)state)->stateIndex = 1;
            ((TrickyState*)state)->substate = 0;
            resetTimer = lbl_803E23DC;
            ((TrickyState*)state)->cooldownA = resetTimer;
            ((TrickyState*)state)->cooldownB.f = resetTimer;
            TRICKY_CLEAR_RESET_FLAGS(state);
        }
        break;
    }
}

void tricky_moveToFollowTarget(int obj, int state)
{
    int inWater;
    int result;

    result = trickyFn_8013b368((GameObject*)obj, lbl_803E247C, (TrickyState*)state);
    if (result == 0)
    {
        if (lbl_803E23DC == ((TrickyState*)state)->waterLevel)
        {
            inWater = 0;
        }
        else if (lbl_803E2410 == ((TrickyState*)state)->eventTime)
        {
            inWater = 1;
        }
        else if (((TrickyState*)state)->currentTime - ((TrickyState*)state)->eventTime > lbl_803E2414)
        {
            inWater = 1;
        }
        else
        {
            inWater = 0;
        }
        if (inWater != 0)
        {
            objAnimFn_8013a3f0(obj, 8, lbl_803E243C, 0);
            ((TrickyState*)state)->cooldownC = lbl_803E2440;
            ((TrickyState*)state)->particleTimer = lbl_803E23DC;
            trickyDebugPrint(sInWaterMessage);
        }
        else
        {
            objAnimFn_8013a3f0(obj, 0, lbl_803E2444, 0);
            trickyDebugPrint(lbl_8031D478);
        }
    }
}

/* Tricky flame/guard AI. Spawns Tricky's flameblast (def 0x4F0) for the
 * fire-breath/guard behaviour. */

#define TRICKY_STATE_HELPERS_ACTIVE_FLAG   0x00000800
#define TRICKY_STATE_HELPERS_FINISHED_FLAG 0x00001000
#define TRICKY_GUARD_HELPER_COUNT          7
#define TRICKY_GUARD_APPROACH_GROUP        3
#define TRICKY_GUARD_HELPER_SETUP_SIZE     0x24
#define TRICKY_GUARD_HELPER_DEF_ID         0x04F0

typedef struct TrickyRuntime
{
    u8* helperSpawnCount;
    u8 pad04[0x08 - 0x04];
    u8 growlLatState;
    u8 guardReachedTarget; /* set to 1 once the guard's walk group matches the target point */
    u8 guardState;
    u8 pad0B[0x0D - 0x0B];
    s8 unk0D;
    u8 pad0E[0x24 - 0x0E];
    ObjAnimComponent* homeObj;
    f32* targetPosition;
    u8 pad2C[TRICKY_STATE_FLAGS_OFFSET - 0x2C];
    u32 flags;
    u8 pad58[0xD2 - 0x58];
    u16 targetTurnTimer;
    u8 padD4[0x700 - 0xD4];
    void* guardHelpers[TRICKY_GUARD_HELPER_COUNT];
    f32 guardPoint[3];
    f32 guardTimer;
    ObjAnimComponent* guardTarget;
    s32 guardWalkGroup;
    u8 guardCanSpawnHelpers;
} TrickyRuntime;

STATIC_ASSERT(offsetof(TrickyRuntime, flags) == TRICKY_STATE_FLAGS_OFFSET);
STATIC_ASSERT(offsetof(TrickyRuntime, helperSpawnCount) == 0x00);
STATIC_ASSERT(offsetof(TrickyRuntime, growlLatState) == 0x08);
STATIC_ASSERT(offsetof(TrickyRuntime, guardState) == 0x0A);
STATIC_ASSERT(offsetof(TrickyRuntime, unk0D) == 0x0D);
STATIC_ASSERT(offsetof(TrickyRuntime, homeObj) == 0x24);
STATIC_ASSERT(offsetof(TrickyRuntime, targetPosition) == 0x28);
STATIC_ASSERT(offsetof(TrickyRuntime, targetTurnTimer) == 0xD2);
STATIC_ASSERT(offsetof(TrickyRuntime, guardHelpers) == 0x700);
STATIC_ASSERT(offsetof(TrickyRuntime, guardPoint) == 0x71C);
STATIC_ASSERT(offsetof(TrickyRuntime, guardTimer) == 0x728);
STATIC_ASSERT(offsetof(TrickyRuntime, guardTarget) == 0x72C);
STATIC_ASSERT(offsetof(TrickyRuntime, guardWalkGroup) == 0x730);
STATIC_ASSERT(offsetof(TrickyRuntime, guardCanSpawnHelpers) == 0x734);

#define TRICKY_RUNTIME(st) ((TrickyRuntime*)(st))

#define TRICKY_CLEAR_FLAG(st, flag)                                                                                    \
    {                                                                                                                  \
        u32 m;                                                                                                         \
        u32 f2 = TRICKY_RUNTIME(st)->flags;                                                                            \
        m = ~(flag);                                                                                                   \
        TRICKY_RUNTIME(st)->flags = f2 & m;                                                                            \
    }

#define TRICKY_CLEAR_TARGET_DIRTY(st) TRICKY_CLEAR_FLAG(st, TRICKY_STATE_TARGET_DIRTY_FLAG)

#define TRICKY_MARK_HELPERS_FINISHED(st)                                                                               \
    {                                                                                                                  \
        TRICKY_CLEAR_FLAG(st, TRICKY_STATE_HELPERS_ACTIVE_FLAG);                                                       \
        TRICKY_RUNTIME(st)->flags |= TRICKY_STATE_HELPERS_FINISHED_FLAG;                                               \
    }

#define TRICKY_RUNTIME_CLEAR_RESET_FLAGS(st)                                                                           \
    {                                                                                                                  \
        TRICKY_CLEAR_FLAG(st, TRICKY_STATE_RESET_FLAG_10);                                                             \
        TRICKY_CLEAR_FLAG(st, TRICKY_STATE_RESET_FLAG_10000);                                                          \
        TRICKY_CLEAR_FLAG(st, TRICKY_STATE_RESET_FLAG_20000);                                                          \
        TRICKY_CLEAR_FLAG(st, TRICKY_STATE_RESET_FLAG_40000);                                                          \
        TRICKY_RUNTIME(st)->unk0D = -1;                                                                                \
    }


int trickyGuardFindBaddieTarget(TrickyRuntime* state);

static inline int trickyGuardIsBaddieTargetValid(TrickyRuntime* trickyState)
{
    u32 target = (u32)trickyState->guardTarget;
    int count;
    int* list;
    int i;

    list = (int*)ObjGroup_GetObjects(TRICKY_GUARD_APPROACH_GROUP, &count);
    for (i = 0; (s16)i < count; i++)
    {
        if ((u32)*list == target)
        {
            return 1;
        }
        list++;
    }
    return 0;
}

void trickyGuard(ObjAnimComponent* obj, TrickyRuntime* trickyState)
{
    char* strBase = lbl_8031D2E8;
    int i;
    int temp;
    int temp2;
    int temp3;
    void** slot;
    void** slot2;
    int i2;
    void* setup;
    int found;
    int newTarget;

    switch (trickyState->guardState)
    {
    case 0:
        trickyDebugPrint(strBase + 0x648);
        trickyState->guardWalkGroup = Objfsa_GetWalkGroupIndexAtPoint(trickyState->targetPosition, 0x0);
        trickyState->guardPoint[0] =
            (f32)(trickyState->homeObj->worldPosX -
                  lbl_803E247C * mathSinf((lbl_803E2454 * trickyState->homeObj->rotX) / lbl_803E2458));
        trickyState->guardPoint[1] = trickyState->homeObj->worldPosY;
        trickyState->guardPoint[2] =
            (f32)(trickyState->homeObj->worldPosZ -
                  lbl_803E247C * mathCosf((lbl_803E2454 * trickyState->homeObj->rotX) / lbl_803E2458));
        trickyState->guardCanSpawnHelpers = 0;
        trickyState->guardState = 1;
        break;
    case 1:
        trickyDebugPrint(strBase + 0x654);
        trickyFn_8013b368((GameObject*)obj, lbl_803E2488, (TrickyState*)trickyState);
        if (trickyState->guardWalkGroup == Objfsa_GetWalkGroupIndexAtPoint(&obj->worldPosX, 0x0))
        {
            trickyState->guardState = 2;
        }
        break;
    case 2:
        trickyDebugPrint(strBase + 0x664);
        if (trickyFn_8013b368((GameObject*)obj, lbl_803E2488, (TrickyState*)trickyState) == 0)
        {
            if ((u32)trickyState->targetPosition != (u32)trickyState->guardPoint)
            {
                trickyState->targetPosition = trickyState->guardPoint;
                TRICKY_CLEAR_TARGET_DIRTY(trickyState);
                trickyState->targetTurnTimer = 0;
            }
            trickyState->guardState = 3;
        }
        else
        {
            trickyGuardFindBaddieTarget(trickyState);
            break;
        }
    case 3:
        trickyDebugPrint(strBase + 0x674);
        if (trickyFn_8013b368((GameObject*)obj, lbl_803E2488, (TrickyState*)trickyState) == 0)
        {
            if (lbl_803E23DC == ((TrickyState*)trickyState)->waterLevel)
            {
                found = 0;
            }
            else if (lbl_803E2410 == ((TrickyState*)trickyState)->eventTime)
            {
                found = 1;
            }
            else if ((((TrickyState*)trickyState)->currentTime - ((TrickyState*)trickyState)->eventTime) > lbl_803E2414)
            {
                found = 1;
            }
            else
            {
                found = 0;
            }
            if (found != 0)
            {
                objAnimFn_8013a3f0((int)obj, 0x8, lbl_803E243C, 0);
                ((TrickyState*)trickyState)->cooldownC = lbl_803E2440;
                ((TrickyState*)trickyState)->particleTimer = lbl_803E23DC;
                trickyDebugPrint(strBase + 0x184);
            }
            else
            {
                objAnimFn_8013a3f0((int)obj, 0, lbl_803E2444, 0);
                trickyDebugPrint(strBase + 0x190);
            }
        }
        trickyGuardFindBaddieTarget(trickyState);
        break;
    case 4:
        trickyDebugPrint(strBase + 0x684);
        if (trickyFn_8013b368((GameObject*)obj, lbl_803E247C, (TrickyState*)trickyState) == 0)
        {
            trickyState->flags = trickyState->flags | TRICKY_STATE_RESET_FLAG_10;
            if (*trickyState->helperSpawnCount != 0 && trickyState->guardCanSpawnHelpers != 0)
            {
                if ((u8)Obj_IsLoadingLocked() != 0)
                {
                    trickyState->flags = trickyState->flags | TRICKY_STATE_HELPERS_ACTIVE_FLAG;
                    for (i = 0, slot = (void**)trickyState; i < TRICKY_GUARD_HELPER_COUNT; i++)
                    {
                        setup = (void*)Obj_AllocObjectSetup(TRICKY_GUARD_HELPER_SETUP_SIZE,
                                                           TRICKY_GUARD_HELPER_DEF_ID);
                        *(u8*)((char*)setup + 0x4) = 2;
                        *(u8*)((char*)setup + 0x5) = 1;
                        *(s16*)((char*)setup + 0x1a) = i;
                        slot[0x700 / 4] =
                            (void*)Obj_SetupObject((ObjPlacement*)setup, 5, obj->mapEventSlot, -1, obj->parent);
                        slot++;
                    }
                    Sfx_PlayFromObject((int)obj, SFXTRIG_en_cvdrip1c_3db);
                    Sfx_AddLoopedObjectSound((int)obj, SFXTRIG_trpopn_c);
                }
                (*trickyState->helperSpawnCount)--;
                objAnimFn_8013a3f0((int)obj, 0x34, lbl_803E2444, 0x4000000);
                trickyState->guardState = 5;
            }
            else
            {
                objAnimFn_8013a3f0((int)obj, 0x32, lbl_803E23EC, 0x4000000);
                trickyState->guardState = 6;
            }
        }
        else
        {
            if (trickyState->guardWalkGroup == Objfsa_GetWalkGroupIndexAtPoint(trickyState->targetPosition, 0x0))
            {
                break;
            }
            newTarget = (int)&trickyState->homeObj->worldPosX;
            if ((u32)trickyState->targetPosition != newTarget)
            {
                trickyState->targetPosition = (f32*)newTarget;
                TRICKY_CLEAR_TARGET_DIRTY(trickyState);
                trickyState->targetTurnTimer = 0;
            }
            trickyState->guardState = 2;
            break;
        }
    case 5:
        trickyDebugPrint(strBase + 0x694);
        if ((double)obj->currentMoveProgress >= (double)lbl_803E24D0)
        {
            TRICKY_MARK_HELPERS_FINISHED(trickyState);
            for (i2 = 0, slot2 = (void**)trickyState; i2 < TRICKY_GUARD_HELPER_COUNT; i2++)
            {
                objSetAnimSpeedTo1((GameObject*)slot2[0x700 / 4]);
                slot2++;
            }
            Sfx_RemoveLoopedObjectSound((int)obj, SFXTRIG_trpopn_c);
            temp = (int)((GameObject*)obj)->extra;
            if ((((u32)((TrickyState*)temp)->statusFlags >> 6) & 1) == 0)
            {
                s16 a0 = obj->currentMove;
                if (a0 >= 0x30 || a0 < 0x29)
                {
                    if (Sfx_IsPlayingFromObjectChannel((u32)obj, 0x10) == 0)
                    {
                        objAudioFn_800393f8((GameObject*)obj, &((TrickyState*)temp)->soundState, 0x29d, 0, -1, 0);
                    }
                }
            }
            trickyState->flags &= ~(u64)TRICKY_STATE_RESET_FLAG_10;
            if (trickyGuardFindBaddieTarget(trickyState) == 0)
            {
                newTarget = (int)&trickyState->homeObj->worldPosX;
                if ((u32)trickyState->targetPosition != newTarget)
                {
                    trickyState->targetPosition = (f32*)newTarget;
                    TRICKY_CLEAR_TARGET_DIRTY(trickyState);
                    trickyState->targetTurnTimer = 0;
                }
                trickyState->guardState = 2;
            }
        }
        else if (trickyGuardIsBaddieTargetValid(trickyState) != 0)
        {
            f32* targ = ((TrickyRuntime*)((GameObject*)obj)->extra)->targetPosition;
            trickyTurnTowardYaw((u8*)obj,
                                getAngle(-(targ[0] - obj->worldPosX), -(targ[2] - obj->worldPosZ)));
        }
        break;
    case 6:
        trickyDebugPrint(strBase + 0x6a4);
        if ((double)obj->currentMoveProgress >= (double)lbl_803E24D0)
        {
            objAnimFn_8013a3f0((int)obj, 0x33, lbl_803E2444, 0x4000000);
            trickyState->guardTimer = lbl_803E23DC;
            temp2 = (int)((GameObject*)obj)->extra;
            if ((((u32)((TrickyState*)temp2)->statusFlags >> 6) & 1) == 0)
            {
                s16 a0 = obj->currentMove;
                if (a0 >= 0x30 || a0 < 0x29)
                {
                    if (Sfx_IsPlayingFromObjectChannel((u32)obj, 0x10) == 0)
                    {
                        objAudioFn_800393f8((GameObject*)obj, &((TrickyState*)temp2)->soundState, 0x299, 0x100, -1, 0);
                    }
                }
            }
            trickyState->guardState = 7;
        }
        else if (trickyGuardIsBaddieTargetValid(trickyState) != 0)
        {
            f32* targ = ((TrickyRuntime*)((GameObject*)obj)->extra)->targetPosition;
            trickyTurnTowardYaw((u8*)obj,
                                getAngle(-(targ[0] - obj->worldPosX), -(targ[2] - obj->worldPosZ)));
        }
        break;
    case 7:
        trickyDebugPrint(strBase + 0x6b8);
        if (randomGetRange(0, 10) == 0)
        {
            temp3 = (int)((GameObject*)obj)->extra;
            if ((((u32)((TrickyState*)temp3)->statusFlags >> 6) & 1) == 0)
            {
                s16 a0 = obj->currentMove;
                if (a0 >= 0x30 || a0 < 0x29)
                {
                    if (Sfx_IsPlayingFromObjectChannel((int)obj, 0x10) == 0)
                    {
                        objAudioFn_800393f8((GameObject*)obj, &((TrickyState*)temp3)->soundState, 0x299, 0x100, -1, 0);
                    }
                }
            }
        }
        trickyState->guardTimer = trickyState->guardTimer + timeDelta;
        if (((double)trickyState->guardTimer >= (double)lbl_803E24D8 &&
             (double)getXZDistance(trickyState->targetPosition, &obj->worldPosX) >= (double)lbl_803E24C4) ||
            trickyGuardIsBaddieTargetValid(trickyState) == 0)
        {
            objAnimFn_8013a3f0((int)obj, 0x32, lbl_803E23F4, 0x4000000);
            trickyState->guardState = 8;
        }
        else
        {
            f32* targ = ((TrickyRuntime*)((GameObject*)obj)->extra)->targetPosition;
            trickyTurnTowardYaw((u8*)obj,
                                getAngle(-(targ[0] - obj->worldPosX), -(targ[2] - obj->worldPosZ)));
        }
        break;
    case 8:
        trickyDebugPrint(strBase + 0x6c8);
        if ((double)obj->currentMoveProgress <= (double)lbl_803E2420)
        {
            trickyState->flags &= ~(u64)TRICKY_STATE_RESET_FLAG_10;
            if (trickyGuardFindBaddieTarget(trickyState) == 0)
            {
                newTarget = (int)&trickyState->homeObj->worldPosX;
                if ((u32)trickyState->targetPosition != newTarget)
                {
                    trickyState->targetPosition = (f32*)newTarget;
                    TRICKY_CLEAR_TARGET_DIRTY(trickyState);
                    trickyState->targetTurnTimer = 0;
                }
                trickyState->guardState = 2;
            }
        }
        break;
    }
}

int trickyGuardFindBaddieTarget(TrickyRuntime* trickyState)
{
    int count;
    f32 d;
    f32 bestDist;
    int* list;
    int i;
    int* groupObjects;
    u32 best = 0;

    groupObjects = (int*)ObjGroup_GetObjects(TRICKY_GUARD_APPROACH_GROUP, &count);
    i = 0;
    list = groupObjects;
    for (; (s16)i < count; i++)
    {
        d = getXZDistance((float*)(*list + 0x18), trickyState->guardPoint);
        if (best == 0)
        {
            if (trickyState->guardWalkGroup == Objfsa_GetWalkGroupIndexAtPoint((float*)(*list + 0x18), 0x0))
            {
                bestDist = d;
                best = *list;
            }
        }
        else if (d < bestDist)
        {
            if (trickyState->guardWalkGroup == Objfsa_GetWalkGroupIndexAtPoint((float*)(*list + 0x18), 0x0))
            {
                bestDist = d;
                best = *list;
            }
        }
        list++;
    }
    if (best != 0)
    {
        trickyState->guardTarget = (ObjAnimComponent*)best;
        if ((u32)trickyState->targetPosition != (best + 0x18))
        {
            trickyState->targetPosition = (f32*)(best + 0x18);
            TRICKY_CLEAR_TARGET_DIRTY(trickyState);
            trickyState->targetTurnTimer = 0;
        }
        trickyState->guardState = 4;
        return 1;
    }
    return 0;
}

void trickyFlame(GameObject* obj, int trickyState)
{
    char* strBase = lbl_8031D2E8;
    void** slot;
    int i;
    void** slot2;
    int i2;
    void* setup;
    int dieFlag;
    int newTarget;
    f32 fz;

    switch (((TrickyRuntime*)trickyState)->guardState)
    {
    case 0:
        trickyDebugPrint(strBase + 0x700);
        *(int*)&((TrickyRuntime*)trickyState)->guardPoint[0] =
            (int)Objfsa_FindNearestCurveType24(&((TrickyRuntime*)trickyState)->homeObj->worldPosX, -1, 4);
        if (*(u8*)(*(int*)&((TrickyRuntime*)trickyState)->guardPoint[0] + 0x3) != 0)
        {
            newTarget = *(int*)&((TrickyRuntime*)trickyState)->guardPoint[0] + 0x8;
            if (*(u32*)&((TrickyRuntime*)trickyState)->targetPosition != newTarget)
            {
                *(int*)&((TrickyRuntime*)trickyState)->targetPosition = newTarget;
                TRICKY_CLEAR_TARGET_DIRTY(trickyState);
                ((TrickyRuntime*)trickyState)->targetTurnTimer = 0;
            }
            ((TrickyRuntime*)trickyState)->guardState = 1;
        }
        else
        {
            *(int*)&((TrickyRuntime*)trickyState)->guardPoint[1] =
                (int)(*gRomCurveInterface)
                    ->getById(*(int*)(*(int*)&((TrickyRuntime*)trickyState)->guardPoint[0] + 0x1c));
            newTarget = *(int*)&((TrickyRuntime*)trickyState)->guardPoint[1] + 0x8;
            if (*(u32*)&((TrickyRuntime*)trickyState)->targetPosition != newTarget)
            {
                *(int*)&((TrickyRuntime*)trickyState)->targetPosition = newTarget;
                TRICKY_CLEAR_TARGET_DIRTY(trickyState);
                ((TrickyRuntime*)trickyState)->targetTurnTimer = 0;
            }
            ((TrickyRuntime*)trickyState)->guardState = 3;
        }
        trickyFn_8013b368((GameObject*)obj, lbl_803E2488, (TrickyState*)trickyState);
        break;
    case 3:
        trickyDebugPrint(strBase + 0x70c);
        trickyFn_8013b368((GameObject*)obj, lbl_803E2488, (TrickyState*)trickyState);
        if ((u8) * (u8*)(*(int*)&((TrickyRuntime*)trickyState)->guardPoint[1] + 0x3) ==
            Objfsa_GetWalkGroupIndexAtPoint((float*)&(obj)->anim.worldPosX, 0x0))
        {
            ((TrickyRuntime*)trickyState)->guardReachedTarget = 1;
            ((TrickyRuntime*)trickyState)->guardState = 4;
        }
        break;
    case 4:
        trickyDebugPrint(strBase + 0x720);
        newTarget = *(int*)&((TrickyRuntime*)trickyState)->guardPoint[0] + 0x8;
        trickyUpdateApproachSpeed((GameObject*)obj, lbl_803E2488, (TrickyState*)trickyState, (f32*)newTarget, 1);
        trickyMove(obj, (f32*)newTarget);
        if (Objfsa_GetWalkGroupIndexAtPoint((float*)&(obj)->anim.worldPosX, 0x0) == 0)
        {
            ((TrickyRuntime*)trickyState)->flags |= TRICKY_STATE_RESET_FLAG_10;
            ((TrickyRuntime*)trickyState)->guardState = 5;
        }
        break;
    case 5:
        trickyDebugPrint(strBase + 0x734);
        newTarget = *(int*)&((TrickyRuntime*)trickyState)->guardPoint[0] + 0x8;
        trickyUpdateApproachSpeed((GameObject*)obj, lbl_803E2488, (TrickyState*)trickyState, (f32*)newTarget, 1);
        if (trickyMove(obj, (f32*)newTarget) != 0)
        {
            break;
        }
        objAnimFn_8013a3f0((int)obj, 0x1a, lbl_803E23E4, 0x4000000);
        ((TrickyRuntime*)trickyState)->guardState = 7;
        (*((TrickyRuntime*)trickyState)->helperSpawnCount) -= 4;
        /* fall through */
    case 7:
        trickyDebugPrint(strBase + 0x744);
        {
            s16 srcAng = (s16)((s8) * (u8*)(*(int*)&((TrickyRuntime*)trickyState)->guardPoint[0] + 0x2c) << 8);
            s16 delta = (s16)(srcAng - (u16) * (s16*)obj);
            int absDelta;
            if (delta > 0x8000)
            {
                delta = (s16)(delta - 0xFFFF);
            }
            if (delta < -0x8000)
            {
                delta = (s16)(delta + 0xFFFF);
            }
            absDelta = delta;
            absDelta = (absDelta >= 0) ? absDelta : -absDelta;
            if (absDelta >= 0x4000)
            {
                srcAng = (s16)(srcAng + 0x8000);
            }
            trickyTurnTowardYaw((u8*)obj, srcAng);
        }
        do
        {
            if ((double)(obj)->anim.currentMoveProgress > (double)lbl_803E24AC)
            {
                if ((((TrickyRuntime*)trickyState)->flags & TRICKY_STATE_HELPERS_ACTIVE_FLAG) == 0)
                {
                    if ((u8)Obj_IsLoadingLocked() != 0)
                    {
                        ((TrickyRuntime*)trickyState)->flags |= TRICKY_STATE_HELPERS_ACTIVE_FLAG;
                        for (i = 0, slot = (void**)trickyState; i < TRICKY_GUARD_HELPER_COUNT; i++)
                        {
                            setup = (void*)Obj_AllocObjectSetup(TRICKY_GUARD_HELPER_SETUP_SIZE,
                                                               TRICKY_GUARD_HELPER_DEF_ID);
                            *(u8*)((char*)setup + 0x4) = 2;
                            *(u8*)((char*)setup + 0x5) = 1;
                            *(s16*)((char*)setup + 0x1a) = i;
                            slot[0x700 / 4] = (void*)Obj_SetupObject((ObjPlacement*)setup, 5,
                                                                    (obj)->anim.mapEventSlot, -1,
                                                                    (obj)->anim.parent);
                            slot++;
                        }
                        Sfx_PlayFromObject((int)obj, SFXTRIG_en_cvdrip1c_3db);
                        Sfx_AddLoopedObjectSound((int)obj, SFXTRIG_trpopn_c);
                    }
                }
                else
                {
                    int (*cb)(int, int) = *(int (**)(int, int))(trickyState + 0x724);
                    if (cb != NULL && cb((int)((TrickyRuntime*)trickyState)->homeObj, 1) == 0)
                    {
                    }
                    else if ((double)(obj)->anim.currentMoveProgress > (double)lbl_803E2504)
                    {
                        TRICKY_MARK_HELPERS_FINISHED(trickyState);
                        for (i = 0, slot = (void**)trickyState; i < TRICKY_GUARD_HELPER_COUNT; i++)
                        {
                            objSetAnimSpeedTo1((GameObject*)slot[0x700 / 4]);
                            slot++;
                        }
                        Sfx_RemoveLoopedObjectSound((int)obj, SFXTRIG_trpopn_c);
                        i = (int)(obj)->extra;
                        if ((((u32)((TrickyState*)i)->statusFlags >> 6) & 1) == 0)
                        {
                            s16 a0 = (obj)->anim.currentMove;
                            if (a0 >= 0x30 || a0 < 0x29)
                            {
                                if (Sfx_IsPlayingFromObjectChannel((u32)obj, 0x10) == 0)
                                {
                                    objAudioFn_800393f8(obj, &((TrickyState*)i)->soundState, 0x29d, 0, -1, 0);
                                }
                            }
                        }
                        dieFlag = 0;
                        break;
                    }
                }
            }
            dieFlag = 1;
        } while (0);
        if (dieFlag == 0)
        {
            ((TrickyRuntime*)trickyState)->guardState = 8;
            ((TrickyRuntime*)trickyState)->guardTimer = lbl_803E24F8;
        }
        break;
    case 1:
        trickyDebugPrint(strBase + 0x750);
        {
            int r = trickyFn_8013b368((GameObject*)obj, lbl_803E2488, (TrickyState*)trickyState);
            if (r == 0)
            {
                ((TrickyRuntime*)trickyState)->flags |= TRICKY_STATE_RESET_FLAG_10;
                ((TrickyRuntime*)trickyState)->guardState = 2;
            }
            else if (r == 2)
            {
                ((TrickyRuntime*)trickyState)->growlLatState = 1;
                ((TrickyRuntime*)trickyState)->guardState = 0;
                fz = lbl_803E23DC;
                ((TrickyRuntime*)trickyState)->guardPoint[0] = fz;
                ((TrickyRuntime*)trickyState)->guardPoint[1] = fz;
                TRICKY_RUNTIME_CLEAR_RESET_FLAGS(trickyState);
            }
        }
        break;
    case 2:
        trickyDebugPrint(strBase + 0x764);
        newTarget = (int)((TrickyRuntime*)trickyState)->homeObj + 0x18;
        trickyUpdateApproachSpeed((GameObject*)obj, lbl_803E2418, (TrickyState*)trickyState, (f32*)newTarget, 1);
        if (trickyMove(obj, (f32*)newTarget) == 0)
        {
            objAnimFn_8013a3f0((int)obj, 0x1a, lbl_803E23E4, 0x4000000);
            ((TrickyRuntime*)trickyState)->guardState = 6;
            (*((TrickyRuntime*)trickyState)->helperSpawnCount) -= 4;
        }
        break;
    case 6:
        trickyDebugPrint(strBase + 0x778);
        do
        {
            if ((double)(obj)->anim.currentMoveProgress > (double)lbl_803E24AC)
            {
                if ((((TrickyRuntime*)trickyState)->flags & TRICKY_STATE_HELPERS_ACTIVE_FLAG) == 0)
                {
                    if ((u8)Obj_IsLoadingLocked() != 0)
                    {
                        ((TrickyRuntime*)trickyState)->flags |= TRICKY_STATE_HELPERS_ACTIVE_FLAG;
                        for (i = 0, slot = (void**)trickyState; i < TRICKY_GUARD_HELPER_COUNT; i++)
                        {
                            setup = (void*)Obj_AllocObjectSetup(TRICKY_GUARD_HELPER_SETUP_SIZE,
                                                               TRICKY_GUARD_HELPER_DEF_ID);
                            *(u8*)((char*)setup + 0x4) = 2;
                            *(u8*)((char*)setup + 0x5) = 1;
                            *(s16*)((char*)setup + 0x1a) = i;
                            slot[0x700 / 4] = (void*)Obj_SetupObject((ObjPlacement*)setup, 5,
                                                                    (obj)->anim.mapEventSlot, -1,
                                                                    (obj)->anim.parent);
                            slot++;
                        }
                        Sfx_PlayFromObject((int)obj, SFXTRIG_en_cvdrip1c_3db);
                        Sfx_AddLoopedObjectSound((int)obj, SFXTRIG_trpopn_c);
                    }
                }
                else
                {
                    int (*cb)(int, int) = *(int (**)(int, int))(trickyState + 0x724);
                    if (cb != NULL && cb((int)((TrickyRuntime*)trickyState)->homeObj, 1) == 0)
                    {
                    }
                    else if ((double)(obj)->anim.currentMoveProgress > (double)lbl_803E2504)
                    {
                        TRICKY_MARK_HELPERS_FINISHED(trickyState);
                        for (i2 = 0, slot2 = (void**)trickyState; i2 < TRICKY_GUARD_HELPER_COUNT; i2++)
                        {
                            objSetAnimSpeedTo1((GameObject*)slot2[0x700 / 4]);
                            slot2++;
                        }
                        Sfx_RemoveLoopedObjectSound((int)obj, SFXTRIG_trpopn_c);
                        slot = (void**)(obj)->extra;
                        if ((((u32)((TrickyState*)slot)->statusFlags >> 6) & 1) == 0)
                        {
                            s16 a0 = (obj)->anim.currentMove;
                            if (a0 >= 0x30 || a0 < 0x29)
                            {
                                if (Sfx_IsPlayingFromObjectChannel((u32)obj, 0x10) == 0)
                                {
                                    objAudioFn_800393f8(obj, &((TrickyState*)slot)->soundState, 0x29d, 0, -1, 0);
                                }
                            }
                        }
                        dieFlag = 0;
                        break;
                    }
                }
            }
            dieFlag = 1;
        } while (0);
        if (dieFlag == 0)
        {
            ((TrickyRuntime*)trickyState)->growlLatState = 1;
            ((TrickyRuntime*)trickyState)->guardState = 0;
            fz = lbl_803E23DC;
            ((TrickyRuntime*)trickyState)->guardPoint[0] = fz;
            ((TrickyRuntime*)trickyState)->guardPoint[1] = fz;
            TRICKY_RUNTIME_CLEAR_RESET_FLAGS(trickyState);
        }
        break;
    case 8:
        trickyDebugPrint(strBase + 0x784);
        ((TrickyRuntime*)trickyState)->guardTimer = ((TrickyRuntime*)trickyState)->guardTimer - timeDelta;
        if (((TrickyRuntime*)trickyState)->guardTimer <= lbl_803E23DC)
        {
            newTarget = *(int*)&((TrickyRuntime*)trickyState)->guardPoint[1] + 0x8;
            trickyUpdateApproachSpeed((GameObject*)obj, lbl_803E2488, (TrickyState*)trickyState, (f32*)newTarget, 1);
            trickyMove(obj, (f32*)newTarget);
            if (Objfsa_GetWalkGroupIndexAtPoint((float*)&(obj)->anim.worldPosX, 0x0) != 0)
            {
                ((TrickyRuntime*)trickyState)->growlLatState = 1;
                ((TrickyRuntime*)trickyState)->guardState = 0;
                fz = lbl_803E23DC;
                ((TrickyRuntime*)trickyState)->guardPoint[0] = fz;
                ((TrickyRuntime*)trickyState)->guardPoint[1] = fz;
                TRICKY_RUNTIME_CLEAR_RESET_FLAGS(trickyState);
            }
        }
        break;
    }
}
void fn_8014128C(void)
{
}

/*
 * Tricky rom-curve route walker. tricky_updateBallRoll is the per-frame
 * update that rolls the object along its rom-curve route.
 *
 * Before init (init-done byte 0x0a == 0): the ball homes onto its curve
 * (CANNONBALL_CURVE). Once the owner and the ball share a walk group it
 * picks the route direction by comparing owner-to-endpoint distances,
 * binds the route walker to the chosen segment, steps it, seeds the sfx
 * timer and marks init done.
 *
 * After init: at each segment end it gathers the valid branch nodes
 * (gated by the node-set's per-branch mask byte), picks the nearest to
 * the current owner, retargets the walker, then accelerates/decays the
 * roll speed toward CANNONBALL_SFX_TIMER limits, advances and moves the
 * ball. Off the walk grid it sets CANNONBALL_HIDE_FLAG. The sfx timer
 * periodically plays the rolling sound (0x29b) on object channel 0x10
 * when the current move is outside the 0x29..0x2f window.
 */

/* The "ball" is the Tricky cannonball's TrickyState extra block: substate is
 * the init-done byte, speed the roll speed, stateFlags the flag word, route the
 * embedded RomCurveWalker, followObj/playerObj the owner links, scratch700 the
 * curve link and scratch708 the rolling-sfx countdown. */
#define CANNONBALL_HIDE_FLAG        0x10
#define CANNONBALL_SPEED_DECAY_FLAG 0x10000000

/* lbl_803E2*: this DLL's f32 route/speed constants. */

void tricky_updateBallRoll(int obj, int ball)
{
    TrickyState* ts = (TrickyState*)ball;
    int toNode;
    u8 nodeCount;
    int node;
    ObjfsaRomCurveDef* nodeSet;
    s32* link;
    u32 mask;
    int bit;
    int i;
    int curve;
    int fromNode;
    s32 nodeIds[4];
    void* curveArg;
    int nextNode;
    int candidateNode;
    int targetNode;
    int walkGroup;
    int sfxState;
    float speed;
    double distance;
    double bestDistance;

    nodeCount = 0;
    if (ts->substate != 0)
    {
        if (ts->route.reverse == 0)
        {
            if (ts->route.atSegmentEnd != 0)
            {
                nodeSet = (ObjfsaRomCurveDef*)ts->route.nodeA4;
                mask = 1;
                link = nodeSet->linkIds;
                for (bit = 0; bit < 4; bit++)
                {
                    node = *link++;
                    if (node > -1 && ((nodeSet->blockedLinkMask & mask) == 0))
                    {
                        nodeIds[nodeCount++] = node;
                    }
                    mask <<= 1;
                }
            }
        }
        else if (ts->route.atSegmentEnd == 0)
        {
            int node2;
            ObjfsaRomCurveDef* nodeSet2;
            s32* link2;
            u32 mask2;
            nodeSet2 = (ObjfsaRomCurveDef*)ts->route.nodeA4;
            mask2 = 1;
            link2 = nodeSet2->linkIds;
            for (bit = 0; bit < 4; bit++)
            {
                node2 = *link2++;
                if (node2 > -1 && ((nodeSet2->blockedLinkMask & mask2) != 0))
                {
                    nodeIds[nodeCount++] = node2;
                }
                mask2 <<= 1;
            }
        }

        if (nodeCount != 0)
        {
            targetNode = (int)(*gRomCurveInterface)->getById(nodeIds[0]);
            bestDistance = getXZDistance((float*)((int)ts->followObj + 0x18), (float*)(targetNode + 8));

            for (i = 1, link = &nodeIds[1]; i < nodeCount; i++)
            {
                candidateNode = (int)(*gRomCurveInterface)->getById(*link);
                distance = getXZDistance((float*)((int)ts->followObj + 0x18), (float*)(candidateNode + 8));
                if (distance < bestDistance)
                {
                    targetNode = candidateNode;
                    bestDistance = distance;
                }
                link++;
            }

            curveFn_800da23c(&ts->route, (void*)targetNode);
        }

        speed = ts->speed;
        if ((u8)(ts->stateFlags & CANNONBALL_SPEED_DECAY_FLAG) != 0)
        {
            speed += lbl_803E23F4 * timeDelta;
            if (speed < lbl_803E23DC)
            {
                speed = lbl_803E23DC;
            }
        }
        else if (speed > lbl_803E2508)
        {
            speed += lbl_803E241C * timeDelta;
            if (speed < lbl_803E2508)
            {
                speed = lbl_803E2508;
            }
        }
        else
        {
            speed += lbl_803E2420 * timeDelta;
            if (speed > lbl_803E2508)
            {
                speed = lbl_803E2508;
            }
        }

        ts->speed = speed;
        trickyAdvanceRouteTargetAhead(obj, &ts->route, ts->speed);
        trickyMove((GameObject*)obj, &ts->route.posX);

        if (Objfsa_GetWalkGroupIndexAtPoint((float*)&((GameObject*)obj)->anim.worldPosX, NULL) != 0)
        {
            ts->stateFlags &= ~(u64)CANNONBALL_HIDE_FLAG;
        }
        else
        {
            ts->stateFlags |= CANNONBALL_HIDE_FLAG;
        }

        ts->scratch708.f -= timeDelta;
        if (ts->scratch708.f < lbl_803E23DC)
        {
            ts->scratch708.f = (f32)(int)randomGetRange(200, 600);

            sfxState = *(int*)&((GameObject*)obj)->extra;
            if (((u32)(*(u8*)(sfxState + 0x58) >> 6 & 1) == 0) &&
                ((((GameObject*)obj)->anim.currentMove >= 0x30 || ((GameObject*)obj)->anim.currentMove < 0x29) &&
                 !Sfx_IsPlayingFromObjectChannel(obj, 0x10)))
            {
                objAudioFn_800393f8((GameObject*)obj, &((TrickyState*)sfxState)->soundState, 0x29b, 0x1000, -1, 0);
            }
        }
    }
    else
    {
        trickyFn_8013b368((GameObject*)obj, lbl_803E2488, (TrickyState*)ball);
        if (Objfsa_GetWalkGroupIndexAtPoint((float*)&((GameObject*)obj)->anim.worldPosX, NULL) ==
            (walkGroup = Objfsa_GetWalkGroupIndexAtPoint((float*)((int)ts->scratch700.ptr + 8), NULL)))
        {
            curve = (int)ts->scratch700.ptr;

            nextNode = (*gRomCurveInterface)->getRandomUnblockedLink((RomCurveDef*)curve, 0);
            fromNode = (int)(*gRomCurveInterface)->getById(nextNode);

            nextNode = (*gRomCurveInterface)->getRandomBlockedLink((RomCurveDef*)curve, 0);
            toNode = (int)(*gRomCurveInterface)->getById(nextNode);

            bestDistance = getXZDistance((float*)((u8*)ts->playerObj + 0x18), (float*)(fromNode + 8));
            distance = getXZDistance((float*)((u8*)ts->playerObj + 0x18), (float*)(toNode + 8));

            curveArg = (void*)curve;
            if (bestDistance > distance)
            {
                nextNode = (*gRomCurveInterface)->getRandomUnblockedLink((RomCurveDef*)fromNode, 0);
                targetNode = (int)(*gRomCurveInterface)->getById(nextNode);
                ts->route.reverse = 0;
            }
            else
            {
                fromNode = toNode;
                nextNode = (*gRomCurveInterface)->getRandomBlockedLink((RomCurveDef*)toNode, 0);
                targetNode = (int)(*gRomCurveInterface)->getById(nextNode);
                ts->route.reverse = 1;
            }

            RomCurve_setupHermiteSegment(&ts->route, curveArg, (void*)fromNode, (void*)targetNode);
            if (ts->route.reverse != 0)
            {
                RomCurve_stepClamped(&ts->route, lbl_803E250C);
            }
            else
            {
                RomCurve_stepClamped(&ts->route, lbl_803E23E0);
            }

            ts->scratch708.f = lbl_803E23DC;
            ts->substate = 1;
        }
    }
}

void fn_8014187C(void)
{
}

/*
 * Tricky companion behaviour states (part of the tricky DLL, 0x00C4).
 *
 * Each function here is one entry of Tricky's per-frame substate machine,
 * dispatched off state[0xa] (the substate index) either directly or through
 * the function-pointer table walked in tricky_stateFollowPlayer
 * (((TrickyFnRow*)(base + state[0xa]*4))->fn). They drive Tricky along ROM
 * curve paths (rom_curve_interface), follow/feed the player, run the dig
 * and flame-breath sequences, pick random idle moves and emit the matching
 * object sounds (audio/sfx). tricky_handleFeedOrTalk handles the shared
 * feeding/Y-button-item interaction and is called as a guard at the top of
 * most states. Water-vs-land animation selection (the repeated
 * waterLevel/unk2B0/unk2B4 ladder) chooses swim vs walk anims throughout.
 */

/* GameCube controller button mask */
#define PAD_BUTTON_A 0x100

#define THORNTAIL_OBJGROUP 0x4d /* DLL 0x1AD shthorntail */

/* child objects spawned by this TU (retail OBJECTS.bin names) */
#define TRICKY_CHILD_OBJ_FOOD       0x17b /* "TrickyFood" */

/*
 * A ROM/FSA walk-curve node as Tricky's tunnel/follow states see it (via the
 * rom_curve_interface getById() / Objfsa_*Curve* lookups). Byte-compatible with
 * the shared RomCurveDef, but with the walk-group id at +3 that those states
 * key off (which RomCurveDef leaves in its pad).
 */
typedef struct TrickyCurveNode
{
    u8 pad00[3];
    u8 walkGroup; /* 0x03 walk-group index compared to Objfsa_GetWalkGroupIndexAtPoint */
    u8 pad04[4];
    f32 x;  /* 0x08 */
    f32 y;  /* 0x0c */
    f32 z;  /* 0x10 */
    u32 id; /* 0x14 */
    u8 pad18[3];
    u8 linkDirMask;
    s32 links[4]; /* 0x1c linked curve ids */
} TrickyCurveNode;

STATIC_ASSERT(offsetof(TrickyCurveNode, walkGroup) == 0x03);
STATIC_ASSERT(offsetof(TrickyCurveNode, x) == 0x08);
STATIC_ASSERT(offsetof(TrickyCurveNode, z) == 0x10);
STATIC_ASSERT(offsetof(TrickyCurveNode, id) == 0x14);
STATIC_ASSERT(offsetof(TrickyCurveNode, links) == 0x1c);

void tricky_handlePlayerContact(u8* obj, u8* state);
const TrickyItemIdList gTrickyCmdQueryInit = {{0, 1, 3, 4, 5}};
const TrickyItemIdList gTrickyFoodItemIds = {{0, 1, 3, 4, 5}};


static inline void trickyPlayWhineSfx(u32 id, u8* obj)
{
    u8* ptr = ((GameObject*)obj)->extra;
    if (((u32)((TrickyState*)ptr)->statusFlags >> 6 & 1) == 0 &&
        (((GameObject*)obj)->anim.currentMove >= 0x30 || ((GameObject*)obj)->anim.currentMove < 0x29) &&
        Sfx_IsPlayingFromObjectChannel((int)obj, 0x10) == 0)
    {
        objAudioFn_800393f8((GameObject*)obj, &((TrickyState*)ptr)->soundState, id, 0x500, -1, 0);
    }
}

static inline void trickyAdvanceNode(u8* state)
{
    int idx;
    int off;
    int k;
    int linkId;
    idx = 0;
    off = 0;
    for (k = 4; k != 0; k--)
    {
        linkId = *(int*)((u8*)((TrickyState*)state)->scratch704.ptr + off + 0x1c);
        if (linkId > -1 && linkId != ((TrickyCurveNode*)((TrickyState*)state)->scratch700.ptr)->id)
        {
            ((TrickyState*)state)->scratch700.ptr = ((TrickyState*)state)->scratch704.ptr;
            ((TrickyState*)state)->scratch704.ptr =
                (u8*)(*gRomCurveInterface)->getById(((int*)((char*)((TrickyState*)state)->scratch704.ptr + 0x1c))[idx]);
            break;
        }
        off += 4;
        idx++;
    }
}

void trickyDigTunnel(u8* obj, u8* state)
{
    u32 sfxTable;
    u8* base;
    u8* pc;
    u8* pos;
    u8* ptr;
    int gidx;
    int k;
    int off;
    int idx;
    int v;
    int inWater;
    u16 id;
    f32 vz, vx, spd, z, vxx;

    base = (u8*)lbl_8031D2E8;
    sfxTable = gTrickySubstateSfxIdPairB;
    switch (state[0xa])
    {
    case 0:
        pc = Objfsa_FindNearestCurveType24(((TrickyState*)state)->targetPosPtr, -1, 2);
        ((TrickyState*)state)->scratch708.ptr = (u8*)(*gRomCurveInterface)->getById(((TrickyCurveNode*)pc)->links[0]);
        ((TrickyState*)state)->scratch700.ptr = pc;
        ((TrickyState*)state)->scratch704.ptr = (u8*)(*gRomCurveInterface)->getById(((TrickyCurveNode*)pc)->links[1]);
        if (((TrickyCurveNode*)((TrickyState*)state)->scratch704.ptr)->walkGroup != 0)
        {
            *(u32*)&((TrickyState*)state)->scratch704.ptr =
                ((TrickyState*)state)->scratch704.u ^ ((TrickyState*)state)->scratch708.u;
            ((TrickyState*)state)->scratch708.u =
                ((TrickyState*)state)->scratch708.u ^ ((TrickyState*)state)->scratch704.u;
            *(u32*)&((TrickyState*)state)->scratch704.ptr =
                ((TrickyState*)state)->scratch704.u ^ ((TrickyState*)state)->scratch708.u;
        }
        ptr = (u8*)&((TrickyCurveNode*)((TrickyState*)state)->scratch708.ptr)->x;
        if (((TrickyState*)state)->targetPosPtr != (f32*)ptr)
        {
            ((TrickyState*)state)->targetPosPtr = (f32*)ptr;
            {
                u32 m;
                u32 f2 = ((TrickyState*)state)->stateFlags;
                m = ~0x400;
                ((TrickyState*)state)->stateFlags = f2 & m;
            }
            ((TrickyState*)state)->linkedWalkGroup = 0;
        }
        state[0xa] = 1;
    case 1:
        trickyDebugPrint((char*)(base + 0x7b8));
        trickyFn_8013b368((GameObject*)obj, lbl_803E2488, (TrickyState*)state);
        gidx = Objfsa_GetWalkGroupIndexAtPoint((f32*)(obj + 0x18), NULL);
        if (((TrickyCurveNode*)((TrickyState*)state)->scratch708.ptr)->walkGroup == gidx)
        {
            state[0x9] = 1;
            state[0xa] = 2;
        }
        break;
    case 2:
        trickyDebugPrint((char*)(base + 0x7cc));
        pos = (u8*)&((TrickyCurveNode*)((TrickyState*)state)->scratch700.ptr)->x;
        trickyUpdateApproachSpeed((GameObject*)obj, lbl_803E2488, (TrickyState*)state, (f32*)pos, 1);
        if (trickyMove((GameObject*)obj, (f32*)pos) == 0)
        {
            ((TrickyState*)state)->stateFlags |= 0x2010;
            state[0xa] = 3;
        }
        else
        {
            if (Objfsa_GetWalkGroupIndexAtPoint((f32*)(obj + 0x18), NULL) == 0)
            {
                ((TrickyState*)state)->stateFlags |= 0x2010;
            }
        }
        break;
    case 3:
        objAnimFn_8013a3f0((int)obj, 0xe, lbl_803E2510, 0x4000000);
        ((TrickyState*)state)->dirX =
            ((TrickyCurveNode*)((TrickyState*)state)->scratch704.ptr)->x - ((TrickyCurveNode*)((TrickyState*)state)->scratch700.ptr)->x;
        ((TrickyState*)state)->dirZ =
            ((TrickyCurveNode*)((TrickyState*)state)->scratch704.ptr)->z - ((TrickyCurveNode*)((TrickyState*)state)->scratch700.ptr)->z;
        Sfx_AddLoopedObjectSound((u32)obj, SFXTRIG_trwhin1);
        ((TrickyState*)state)->scratch70C.f = (f32)(int)randomGetRange(0x14, 0xb4);
        state[0xa] = 4;
    case 4:
        trickyDebugPrint((char*)(base + 0x7e4));
        ((TrickyState*)state)->scratch70C.f -= timeDelta;
        if (((TrickyState*)state)->scratch70C.f <= lbl_803E23DC)
        {
            ((TrickyState*)state)->scratch70C.f = (f32)(int)randomGetRange(0x14, 0xb4);
            ((TrickyState*)state)->scratch70C.f *= lbl_803E2424;
            ptr = ((GameObject*)obj)->extra;
            if (((u32)((TrickyState*)ptr)->statusFlags >> 6 & 1) == 0 &&
                (((GameObject*)obj)->anim.currentMove >= 0x30 || ((GameObject*)obj)->anim.currentMove < 0x29) &&
                Sfx_IsPlayingFromObjectChannel((int)obj, 0x10) == 0)
            {
                objAudioFn_800393f8((GameObject*)obj, &((TrickyState*)ptr)->soundState, 0x360, 0x500, -1, 0);
            }
        }
        spd = ((f32(**)(u8*, u8*))(**(u8***)((u8*)((TrickyState*)state)->followObj + 0x68)))[8](
            (u8*)((TrickyState*)state)->followObj, obj);
        ((GameObject*)obj)->anim.localPosX =
            ((TrickyState*)state)->dirX * spd + ((TrickyCurveNode*)((TrickyState*)state)->scratch700.ptr)->x;
        ((GameObject*)obj)->anim.localPosZ =
            ((TrickyState*)state)->dirZ * spd + ((TrickyCurveNode*)((TrickyState*)state)->scratch700.ptr)->z;
        vx = *(f32*)(*(u8**)&((GameObject*)obj)->extra + 0x2c);
        vxx = vx * vx;
        vz = *(f32*)(*(u8**)&((GameObject*)obj)->extra + 0x30);
        spd = vz * vz;
        if (vxx + spd > lbl_803E23EC)
        {
            trickyTurnTowardYaw(obj, getAngle(-vx, -vz));
        }
        if (((u8(**)(u8*))(**(u8***)((u8*)((TrickyState*)state)->followObj + 0x68)))[9](
                (u8*)((TrickyState*)state)->followObj) !=
            0)
        {
            trickyAdvanceNode(state);
            **(u8**)state -= 4;
            Sfx_RemoveLoopedObjectSound((u32)obj, SFXTRIG_trwhin1);
            state[0xa] = 5;
            id = *(u16*)((char*)&sfxTable + randomGetRange(0, 1) * 2);
            ptr = ((GameObject*)obj)->extra;
            if (((u32)((TrickyState*)ptr)->statusFlags >> 6 & 1) == 0 &&
                (((GameObject*)obj)->anim.currentMove >= 0x30 || ((GameObject*)obj)->anim.currentMove < 0x29) &&
                Sfx_IsPlayingFromObjectChannel((int)obj, 0x10) == 0)
            {
                objAudioFn_800393f8((GameObject*)obj, &((TrickyState*)ptr)->soundState, id, 0x500, -1, 0);
            }
        }
        break;
    case 5:
        trickyDebugPrint((char*)(base + 0x7f8),
                         Vec_xzDistance(&((GameObject*)obj)->anim.worldPosX,
                                        &((TrickyCurveNode*)((TrickyState*)state)->scratch704.ptr)->x));
        pos = (u8*)&((TrickyCurveNode*)((TrickyState*)state)->scratch704.ptr)->x;
        trickyUpdateApproachSpeed((GameObject*)obj, lbl_803E2488, (TrickyState*)state, (f32*)pos, 1);
        if (trickyMove((GameObject*)obj, (f32*)pos) == 0)
        {
            trickyAdvanceNode(state);
            state[0xa] = 6;
        }
        break;
    case 6:
        trickyDebugPrint((char*)(base + 0x810));
        pos = (u8*)&((TrickyCurveNode*)((TrickyState*)state)->scratch704.ptr)->x;
        trickyUpdateApproachSpeed((GameObject*)obj, lbl_803E2488, (TrickyState*)state, (f32*)pos, 1);
        if (trickyMove((GameObject*)obj, (f32*)pos) == 0)
        {
            if (lbl_803E23DC == ((TrickyState*)state)->waterLevel)
            {
                inWater = 0;
            }
            else if (lbl_803E2410 == ((TrickyState*)state)->eventTime)
            {
                inWater = 1;
            }
            else if (((TrickyState*)state)->currentTime - ((TrickyState*)state)->eventTime > lbl_803E2414)
            {
                inWater = 1;
            }
            else
            {
                inWater = 0;
            }
            if (inWater != 0)
            {
                objAnimFn_8013a3f0((int)obj, 8, lbl_803E243C, 0);
                ((TrickyState*)state)->cooldownC = lbl_803E2440;
                ((TrickyState*)state)->particleTimer = lbl_803E23DC;
                trickyDebugPrint((char*)(base + 0x184));
            }
            else
            {
                objAnimFn_8013a3f0((int)obj, 0, lbl_803E2444, 0);
                trickyDebugPrint((char*)(base + 0x190));
            }
            ((TrickyState*)state)->stateFlags &= ~0x2010;
            state[0xa] = 7;
        }
        break;
    case 7:
        trickyDebugPrint((char*)(base + 0x824));
        gidx = Objfsa_GetWalkGroupIndexAtPoint(&((TrickyState*)state)->playerObj->anim.worldPosX, NULL);
        if (Objfsa_GetWalkGroupIndexAtPoint((f32*)(obj + 0x18), NULL) == gidx)
        {
            state[0x8] = 1;
            state[0xa] = 0;
            z = lbl_803E23DC;
            ((TrickyState*)state)->cooldownA = z;
            ((TrickyState*)state)->cooldownB.f = z;
            ((TrickyState*)state)->stateFlags &= ~0x10LL;
            ((TrickyState*)state)->stateFlags &= ~0x10000LL;
            ((TrickyState*)state)->stateFlags &= ~0x20000LL;
            ((TrickyState*)state)->stateFlags &= ~0x40000LL;
            {
                s8 mm;
                mm = -1;
                ((TrickyState*)state)->commandPhase = mm;
            }
        }
        break;
    }
}

void tricky_stateFindSecretDig(u8* obj, u8* state)
{
    u32 sfxTable;
    u8* ptr;
    u8* pc;
    int ret;
    f32 spd;
    f32 dist;
    f32 z;

    sfxTable = gTrickySubstateSfxIdPairA;
    pc = (u8*)((TrickyState*)state)->followObj;
    switch (state[0xa])
    {
    case 0:
        ((TrickyState*)state)->scratch70C.ptr =
            Objfsa_FindNearestEnabledCurveType24(&((TrickyState*)state)->followObj->anim.worldPosX, -1, 2);
        if (((TrickyState*)state)->scratch70C.ptr != NULL &&
            getXZDistance(&((TrickyState*)state)->followObj->anim.worldPosX,
                          &((TrickyCurveNode*)((TrickyState*)state)->scratch70C.ptr)->x) > lbl_803E2514)
        {
            ((TrickyState*)state)->scratch70C.ptr = NULL;
        }
        state[0xa] = 1;
    case 1:
        ret = trickyFn_8013b368((GameObject*)obj, lbl_803E2488, (TrickyState*)state);
        if (ret == 0)
        {
            if (((TrickyState*)state)->scratch70C.ptr != NULL)
            {
                state[0xa] = 2;
                ptr = (u8*)&((TrickyCurveNode*)((TrickyState*)state)->scratch70C.ptr)->x;
                if (((TrickyState*)state)->targetPosPtr != (f32*)ptr)
                {
                    ((TrickyState*)state)->targetPosPtr = (f32*)ptr;
                    {
                        u32 m;
                        u32 f2 = ((TrickyState*)state)->stateFlags;
                        m = ~0x400;
                        ((TrickyState*)state)->stateFlags = f2 & m;
                    }
                    ((TrickyState*)state)->linkedWalkGroup = 0;
                }
            }
            else
            {
                ((TrickyState*)state)->stateFlags |= 0x10;
                state[0xa] = 3;
                ((TrickyState*)state)->scratch700.f = lbl_803E23DC;
                ((TrickyState*)state)->scratch710.f = (f32)(int)randomGetRange(0x28, 0x50);
                Sfx_AddLoopedObjectSound((u32)obj, SFXTRIG_trwhin1);
                objAnimFn_8013a3f0((int)obj, 0xe, lbl_803E2510, 0x4000000);
            }
        }
        else if (ret == 2)
        {
            state[0x8] = 1;
            state[0xa] = 0;
            z = lbl_803E23DC;
            ((TrickyState*)state)->cooldownA = z;
            ((TrickyState*)state)->cooldownB.f = z;
            ((TrickyState*)state)->stateFlags &= ~0x10LL;
            ((TrickyState*)state)->stateFlags &= ~0x10000LL;
            ((TrickyState*)state)->stateFlags &= ~0x20000LL;
            ((TrickyState*)state)->stateFlags &= ~0x40000LL;
            {
                s8 mm;
                mm = -1;
                ((TrickyState*)state)->commandPhase = mm;
            }
        }
        break;
    case 2:
        if (trickyFn_8013b368((GameObject*)obj, lbl_803E2418, (TrickyState*)state) == 0)
        {
            ((TrickyState*)state)->stateFlags |= 0x10;
            state[0xa] = 3;
            ((TrickyState*)state)->scratch700.f = lbl_803E23DC;
            Sfx_AddLoopedObjectSound((u32)obj, SFXTRIG_trwhin1);
            objAnimFn_8013a3f0((int)(u32)obj, 0xe, lbl_803E2510, 0x4000000);
        }
        break;
    case 3:
        ((TrickyState*)state)->scratch700.f += timeDelta;
        ((TrickyState*)state)->scratch710.f -= timeDelta;
        if (((TrickyState*)state)->scratch700.f >= lbl_803E24F8)
        {
            state[0xa] = 4;
            ((TrickyState*)state)->scratch704.f = ((GameObject*)obj)->anim.worldPosX;
            ((TrickyState*)state)->scratch708.f = ((GameObject*)obj)->anim.worldPosZ;
            ptr = ((TrickyState*)state)->scratch70C.ptr;
            if (ptr != NULL)
            {
                pc = (u8*)((TrickyState*)state)->followObj;
                ((TrickyState*)state)->dirX = ((TrickyCurveNode*)ptr)->x - ((GameObject*)pc)->anim.worldPosX;
                ((TrickyState*)state)->dirZ = ((TrickyCurveNode*)ptr)->z - ((GameObject*)pc)->anim.worldPosZ;
                dist = sqrtf(((TrickyState*)state)->dirX * ((TrickyState*)state)->dirX +
                          ((TrickyState*)state)->dirZ * ((TrickyState*)state)->dirZ);
                if (lbl_803E23DC != dist)
                {
                    ((TrickyState*)state)->dirX = ((TrickyState*)state)->dirX / dist;
                    ((TrickyState*)state)->dirZ = ((TrickyState*)state)->dirZ / dist;
                }
            }
        }
        break;
    case 4:
        ((TrickyState*)state)->scratch710.f -= timeDelta;
        if (((TrickyState*)state)->scratch710.f <= lbl_803E23DC)
        {
            ((TrickyState*)state)->scratch710.f = (f32)(int)randomGetRange(0x28, 0x50);
            ((TrickyState*)state)->scratch710.f *= lbl_803E2424;
            trickyPlayWhineSfx(0x360, obj);
        }
        spd = ((f32(**)(u8*, u8*))(**(u8***)(pc + 0x68)))[8](pc, obj);
        ((GameObject*)obj)->anim.localPosX = ((TrickyState*)state)->scratch704.f - ((TrickyState*)state)->dirX * spd;
        ((GameObject*)obj)->anim.localPosZ = ((TrickyState*)state)->scratch708.f - ((TrickyState*)state)->dirZ * spd;
        if (((u8(**)(u8*))(**(u8***)(pc + 0x68)))[9](pc) != 0)
        {
            Sfx_RemoveLoopedObjectSound((u32)obj, SFXTRIG_trwhin1);
            **(u8**)state -= 4;
            state[0x8] = 1;
            state[0xa] = 0;
            z = lbl_803E23DC;
            ((TrickyState*)state)->cooldownA = z;
            ((TrickyState*)state)->cooldownB.f = z;
            ((TrickyState*)state)->stateFlags &= ~0x10LL;
            ((TrickyState*)state)->stateFlags &= ~0x10000LL;
            ((TrickyState*)state)->stateFlags &= ~0x20000LL;
            ((TrickyState*)state)->stateFlags &= ~0x40000LL;
            {
                s8 mm;
                mm = -1;
                ((TrickyState*)state)->commandPhase = mm;
            }
            trickyPlayWhineSfx(*(u16*)((char*)&sfxTable + randomGetRange(0, 1) * 2), obj);
        }
        break;
    }
}

typedef struct TrickyFnRow
{
    u8 pad[0x6c];
    int (*fn)(u8*, u8*);
} TrickyFnRow;

typedef struct
{
    u8 bf7 : 1;
    u8 bf6 : 1;
    u8 bf5 : 1;
    u8 rest : 5;
} FlagByte728;

void tricky_stateFollowPlayer(u8* obj, u8* state)
{
    u8* base;
    u8* found;
    u8* other;
    GameObject* target;
    u8* ptr;
    int inWater;
    f32 z;

    base = (u8*)lbl_8031D2E8;
    found = NULL;
    if ((((TrickyState*)state)->stateFlags & 0x10) == 0)
    {
        if (state[0x7d0] != 0)
        {
            switch ((int)state[0x7d0])
            {
            case 1:
            {
                target = ((TrickyState*)state)->pendingFollowObj;
                other = ((GameObject*)obj)->extra;
                if ((((GameObject*)obj)->objectFlags & OBJECT_OBJFLAG_PARENT_SLACK) == 0)
                {
                    if ((((TrickyState*)other)->stateFlags & 0x10) == 0)
                    {
                        ((TrickyState*)other)->followObj = target;
                        if (((TrickyState*)other)->targetPosPtr != &target->anim.worldPosX)
                        {
                            ((TrickyState*)other)->targetPosPtr = &target->anim.worldPosX;
                            {
                                u32 m;
                                u32 f2 = ((TrickyState*)other)->stateFlags;
                                m = ~0x400;
                                ((TrickyState*)other)->stateFlags = f2 & m;
                            }
                            ((TrickyState*)other)->linkedWalkGroup = 0;
                        }
                        other[0xa] = 0;
                        other[0x8] = 10;
                    }
                    else
                    {
                        other[0x7d0] = 1;
                        ((TrickyState*)other)->pendingFollowObj = target;
                        ((TrickyState*)other)->stateFlags |= 0x10000LL;
                    }
                }
                if (tricky_handleFeedOrTalk((GameObject*)obj, (int*)state) == 0 &&
                    trickyFn_8013b368((GameObject*)obj, lbl_803E2488, (TrickyState*)state) == 0)
                {
                    ((TrickyState*)state)->idleSfxTimer -= timeDelta;
                    if (((TrickyState*)state)->idleSfxTimer <= lbl_803E23DC)
                    {
                        ((TrickyState*)state)->idleSfxTimer = (f32)(int)randomGetRange(500, 0x2ee);
                        ptr = ((GameObject*)obj)->extra;
                        if (((u32)((TrickyState*)ptr)->statusFlags >> 6 & 1) == 0 &&
                            (((GameObject*)obj)->anim.currentMove >= 0x30 ||
                             ((GameObject*)obj)->anim.currentMove < 0x29) &&
                            Sfx_IsPlayingFromObjectChannel((int)obj, 0x10) == 0)
                        {
                            objAudioFn_800393f8((GameObject*)obj, &((TrickyState*)ptr)->soundState, 0x360, 0x500, -1,
                                               0);
                        }
                    }
                    if (lbl_803E23DC == ((TrickyState*)state)->waterLevel)
                    {
                        inWater = 0;
                    }
                    else if (lbl_803E2410 == ((TrickyState*)state)->eventTime)
                    {
                        inWater = 1;
                    }
                    else if (((TrickyState*)state)->currentTime - ((TrickyState*)state)->eventTime > lbl_803E2414)
                    {
                        inWater = 1;
                    }
                    else
                    {
                        inWater = 0;
                    }
                    if (inWater != 0)
                    {
                        objAnimFn_8013a3f0((int)obj, 8, lbl_803E243C, 0);
                        ((TrickyState*)state)->cooldownC = lbl_803E2440;
                        ((TrickyState*)state)->particleTimer = lbl_803E23DC;
                        trickyDebugPrint((char*)(base + 0x184));
                    }
                    else
                    {
                        switch (((GameObject*)obj)->anim.currentMove)
                        {
                        case 0xd:
                            if (((TrickyState*)state)->stateFlags & TRICKY_STATE_FLAG_MOVE_ADVANCING)
                            {
                                objAnimFn_8013a3f0((int)obj, 0x31, lbl_803E243C, 0);
                            }
                            break;
                        default:
                            objAnimFn_8013a3f0((int)obj, 0xd, lbl_803E2444, 0);
                        case 0x31:
                            break;
                        }
                        trickyDebugPrint((char*)(base + 0x190));
                    }
                }
            }
            break;
            default:
                break;
            }
            state[0x7d0] = 0;
            return;
        }
        found = Tricky_findNearestGroup4BObject(obj, (TrickyState*)state);
    }
    if (found != NULL)
    {
        state[0x374] = 2;
        (*gPathControlInterface)->attachObject(obj, &((TrickyState*)state)->pathControlFlags);
        state[8] = 1;
        state[0xa] = 0;
        z = lbl_803E23DC;
        ((TrickyState*)state)->cooldownA = z;
        ((TrickyState*)state)->cooldownB.f = z;
        ((TrickyState*)state)->stateFlags &= ~0x10LL;
        ((TrickyState*)state)->stateFlags &= ~0x10000LL;
        ((TrickyState*)state)->stateFlags &= ~0x20000LL;
        ((TrickyState*)state)->stateFlags &= ~0x40000LL;
        {
            s8 mm;
            mm = -1;
            ((TrickyState*)state)->commandPhase = mm;
        }
        ((GameObject*)obj)->anim.localPosX = ((GameObject*)found)->anim.localPosX;
        ((GameObject*)obj)->anim.localPosY = ((GameObject*)found)->anim.localPosY;
        ((GameObject*)obj)->anim.localPosZ = ((GameObject*)found)->anim.localPosZ;
        ((GameObject*)obj)->anim.worldPosX = ((GameObject*)found)->anim.worldPosX;
        ((GameObject*)obj)->anim.worldPosY = ((GameObject*)found)->anim.worldPosY;
        ((GameObject*)obj)->anim.worldPosZ = ((GameObject*)found)->anim.worldPosZ;
        ObjHits_SyncObjectPosition((GameObject*)obj);
        ((GameObject*)obj)->anim.rotX = ((GameObject*)found)->anim.rotX;
        state[9] = 0;
        z = lbl_803E23DC;
        ((TrickyState*)state)->prevSpeed = z;
        ((TrickyState*)state)->speed = z;
        ((TrickyState*)state)->homePosX = ((GameObject*)found)->anim.worldPosX;
        ((TrickyState*)state)->homePosY = ((GameObject*)found)->anim.worldPosY;
        ((TrickyState*)state)->homePosZ = ((GameObject*)found)->anim.worldPosZ;
        ((TrickyState*)state)->stateFlags |= 0x80000LL;
        ((TrickyState*)state)->stateFlags &= ~0x2000LL;
    }
    else
    {
        ((TrickyState*)state)->cooldownA -= timeDelta;
        if (((TrickyState*)state)->cooldownA < *(f32*)&lbl_803E23DC)
        {
            ((TrickyState*)state)->cooldownA = lbl_803E23DC;
        }
        tricky_handlePlayerContact(obj, state);
        {
            if (((int (**)(u8*, u8*))(base + 0x6c))[state[0xa]](obj, state) == 0)
            {
                if (lbl_803E23DC == ((TrickyState*)state)->waterLevel)
                {
                    inWater = 0;
                }
                else if (lbl_803E2410 == ((TrickyState*)state)->eventTime)
                {
                    inWater = 1;
                }
                else if (((TrickyState*)state)->currentTime - ((TrickyState*)state)->eventTime > lbl_803E2414)
                {
                    inWater = 1;
                }
                else
                {
                    inWater = 0;
                }
                if (inWater != 0)
                {
                    objAnimFn_8013a3f0((int)obj, 8, lbl_803E243C, 0);
                    ((TrickyState*)state)->cooldownC = lbl_803E2440;
                    ((TrickyState*)state)->particleTimer = lbl_803E23DC;
                }
                else
                {
                    objAnimFn_8013a3f0((int)obj, 0x25, lbl_803E2518, 0);
                }
            }
        }
    }
}

int tricky_substateApproachThorntail(int obj, int state)
{
    int tex;
    short move;
    u16 sfxId;
    float pos[3];

    objPosFn_80039510(((TrickyState*)state)->followObj, 0, pos);
    if (getXZDistance(pos, (float*)(state + 0x72c)) > lbl_803E2424)
    {
        ((TrickyState*)state)->wanderTargetX = pos[0];
        ((TrickyState*)state)->wanderTargetY = pos[1];
        ((TrickyState*)state)->wanderTargetZ = pos[2];
    }
    if ((((u32)((TrickyState*)state)->stateFlags728 >> 5) & 1) != 0)
    {
        if (Sfx_IsPlayingFromObjectChannel(obj, 16) != 0)
        {
            return 0;
        }
        tricky_startRandomIdleMove((GameObject*)(obj), state);
    }
    else if ((u8)trickyFn_8013b368((GameObject*)obj, lbl_803E24C8, (TrickyState*)state) != 1)
    {
        ((FlagByte728*)&((TrickyState*)state)->stateFlags728)->bf5 = 1;
        sfxId = randomGetRange(862, 863);
        tex = *(int*)&((GameObject*)obj)->extra;
        if ((((u32)((TrickyState*)tex)->statusFlags >> 6) & 1) == 0)
        {
            move = ((GameObject*)obj)->anim.currentMove;
            if (move >= 48 || move < 41)
            {
                if (Sfx_IsPlayingFromObjectChannel(obj, 16) == 0)
                {
                    objAudioFn_800393f8((GameObject*)obj, &((TrickyState*)tex)->soundState, sfxId, 1280, -1, 0);
                }
            }
        }
        return 0;
    }
    return 1;
}

int tricky_substateFlameBreath(u8* obj, u8* state)
{
    int i;
    int j;
    u8* ptr;
    u8* p;
    u8* q;
    u8* e;

    switch (((GameObject*)obj)->anim.currentMove)
    {
    case 0x1a:
        if (((GameObject*)obj)->anim.currentMoveProgress > lbl_803E24AC &&
            (((TrickyState*)state)->stateFlags & TRICKY_STATE_FLAG_CHILDREN_ACTIVE) == 0)
        {
            if (Obj_IsLoadingLocked() != 0)
            {
                ((TrickyState*)state)->stateFlags |= TRICKY_STATE_FLAG_CHILDREN_ACTIVE;
                for (i = 0, p = state; i < 7; p += 4, i++)
                {
                    e = (u8*)Obj_AllocObjectSetup(0x24, TRICKY_CHILD_OBJ_FLAMEBLAST);
                    e[4] = 2;
                    e[5] = 1;
                    *(s16*)(e + 0x1a) = i;
                    *(u8**)(p + 0x700) = (u8*)Obj_SetupObject((ObjPlacement*)e, 5,
                                                              ((GameObject*)obj)->anim.mapEventSlot, -1,
                                                              ((GameObject*)obj)->anim.parent);
                }
                Sfx_PlayFromObject((int)obj, SFXTRIG_en_cvdrip1c_3db);
                Sfx_AddLoopedObjectSound((u32)obj, SFXTRIG_trpopn_c);
            }
        }
        else
        {
            if (((TrickyState*)state)->stateFlags & TRICKY_STATE_FLAG_MOVE_ADVANCING)
            {
                ((TrickyState*)state)->stateFlags &= ~(u64)TRICKY_STATE_FLAG_CHILDREN_ACTIVE;
                ((TrickyState*)state)->stateFlags |= TRICKY_STATE_FLAG_CHILDREN_CLEANUP;
                for (j = 0, q = state; j < 7; q += 4, j++)
                {
                    objSetAnimSpeedTo1((GameObject*)*(u8**)(q + 0x700));
                }
                Sfx_RemoveLoopedObjectSound((u32)obj, SFXTRIG_trpopn_c);
                ptr = ((GameObject*)obj)->extra;
                if (((u32)((TrickyState*)ptr)->statusFlags >> 6 & 1) == 0 &&
                    (((GameObject*)obj)->anim.currentMove >= 0x30 || ((GameObject*)obj)->anim.currentMove < 0x29) &&
                    Sfx_IsPlayingFromObjectChannel((int)obj, 0x10) == 0)
                {
                    objAudioFn_800393f8((GameObject*)obj, &((TrickyState*)ptr)->soundState, 0x29d, 0, -1, 0);
                }
                state[0xa] = 10;
            }
        }
        break;
    default:
        objAnimFn_8013a3f0((int)obj, 0x1a, lbl_803E23E4, 0);
    }
    return 1;
}

int tricky_substateBegForFood(GameObject* obj, int state)
{
    int tex;
    int result;
    short move;
    TrickyItemIdList buf;

    buf = gTrickyFoodItemIds;
    if (tricky_handleFeedOrTalk(obj, (int*)state) != 0)
    {
        ((TrickyState*)state)->cooldownB.f = lbl_803E23DC;
        {
            u32 m;
            u32 f2 = ((TrickyState*)state)->stateFlags;
            m = ~0x10;
            ((TrickyState*)state)->stateFlags = f2 & m;
        }
        ((TrickyState*)state)->substate = 0;
        return 1;
    }
    result = (*gGameUIInterface)->isOneOfItemsBeingUsed(buf.ids, TRICKY_ITEM_ID_COUNT);
    switch (result)
    {
    case 0:
    case 1:
    case 3:
    case 4:
    case 5:
        tex = *(int*)&(obj)->extra;
        if (((((TrickyState*)tex)->statusFlags >> 6) & 1) == 0u)
        {
            move = (obj)->anim.currentMove;
            if (move >= 48 || move < 41)
            {
                if (Sfx_IsPlayingFromObjectChannel((int)obj, 16) == 0)
                {
                    objAudioFn_800393f8(obj, &((TrickyState*)tex)->soundState, 861, 1280, -1, 0);
                }
            }
        }
        break;
    }
    if (lbl_803E23DC == ((TrickyState*)state)->cooldownB.f)
    {
        {
            u32 m;
            u32 f2 = ((TrickyState*)state)->stateFlags;
            m = ~0x10;
            ((TrickyState*)state)->stateFlags = f2 & m;
        }
        ((TrickyState*)state)->substate = 0;
    }
    if ((u8)trickyFn_8013b368((GameObject*)obj, lbl_803E2408, (TrickyState*)state) == 1)
    {
        return 1;
    }
    return 0;
}

int tricky_substateDigForFood(GameObject* obj, int state)
{
    short move;
    int b;
    PartFxSpawnParams spawnBuf;

    if (tricky_handleFeedOrTalk(obj, (int*)state) != 0)
    {
        return 1;
    }
    *(u8*)&(obj)->anim.resetHitboxMode = *(u8*)&(obj)->anim.resetHitboxMode | INTERACT_FLAG_PROMPT_SUPPRESSED;
    move = (obj)->anim.currentMove;
    switch (move)
    {
    case 44:
    case 45:
        if ((((TrickyState*)state)->stateFlags & TRICKY_STATE_FLAG_MOVE_ADVANCING) != 0)
        {
            objAnimFn_8013a3f0((int)obj, 46, lbl_803E249C, 0);
        }
        break;
    case 46:
    {
        if (((((TrickyState*)state)->stateFlags & TRICKY_STATE_FLAG_MOVE_ADVANCING) != 0) &&
            (((((TrickyState*)state)->stateFlags & 0x10000) != 0 || randomGetRange(0, 2) == 0) ||
             ((TrickyState*)state)->cooldownB.f > lbl_803E23DC))
        {
            objAnimFn_8013a3f0((int)obj, 47, lbl_803E23EC, 0);
        }
        spawnBuf.posX = (obj)->anim.worldPosX;
        spawnBuf.posY = (obj)->anim.worldPosY;
        spawnBuf.posZ = (obj)->anim.worldPosZ;
        spawnBuf.scale = lbl_803E23F0;
        (*gPartfxInterface)->spawnObject((void*)obj, 2022, &spawnBuf, 0x200001, -1, NULL);
        break;
    }
    case 47:
        if ((((TrickyState*)state)->stateFlags & TRICKY_STATE_FLAG_MOVE_ADVANCING) != 0)
        {
            if (lbl_803E23DC == ((TrickyState*)state)->waterLevel)
            {
                b = 0;
            }
            else if (lbl_803E2410 == ((TrickyState*)state)->eventTime)
            {
                b = 1;
            }
            else if (((TrickyState*)state)->currentTime - ((TrickyState*)state)->eventTime > lbl_803E2414)
            {
                b = 1;
            }
            else
            {
                b = 0;
            }
            if (b != 0)
            {
                objAnimFn_8013a3f0((int)obj, 8, lbl_803E243C, 0);
                ((TrickyState*)state)->cooldownC = lbl_803E2440;
                ((TrickyState*)state)->particleTimer = lbl_803E23DC;
                trickyDebugPrint(sInWaterMessage);
            }
            else
            {
                objAnimFn_8013a3f0((int)obj, 0, lbl_803E2444, 0);
                trickyDebugPrint(lbl_8031D478);
            }
            {
                u32 m;
                u32 f2 = ((TrickyState*)state)->stateFlags;
                m = ~0x10;
                ((TrickyState*)state)->stateFlags = f2 & m;
            }
            ((TrickyState*)state)->substate = 0;
        }
        break;
    }
    return 1;
}

int tricky_substateIdlePick(u8* obj, u8* state)
{
    u8* ptr;

    if (tricky_handleFeedOrTalk((GameObject*)obj, (int*)state) != 0)
    {
        return 1;
    }
    if ((u8)trickyFn_8013b368((GameObject*)obj, lbl_803E2418, (TrickyState*)state) != 1)
    {
        if (((TrickyState*)state)->childB != NULL)
        {
            ptr = ((GameObject*)obj)->extra;
            if (((u32)((TrickyState*)ptr)->statusFlags >> 6 & 1) == 0 &&
                (((GameObject*)obj)->anim.currentMove >= 0x30 || ((GameObject*)obj)->anim.currentMove < 0x29) &&
                Sfx_IsPlayingFromObjectChannel((int)obj, 0x10) == 0)
            {
                objAudioFn_800393f8((GameObject*)obj, &((TrickyState*)ptr)->soundState, 0x357, 0, -1, 0);
            }
            objAnimFn_8013a3f0((int)obj, 0x26, lbl_803E251C, 0);
            state[0xa] = 5;
        }
        else
        {
            switch (randomGetRange(0, 6))
            {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
                tricky_startRandomIdleMove((GameObject*)obj, (int)state);
                break;
            default:
                tricky_pickAmbientActivity(obj, state);
                break;
            }
        }
    }
    return 1;
}

u32 tricky_substateFidgetA(GameObject* obj, int* trickyState)
{
    short move;
    int foodResult;

    foodResult = tricky_handleFeedOrTalk(obj, trickyState);
    if (foodResult != 0)
    {
        return 1;
    }
    move = (obj)->anim.currentMove;
    switch (move)
    {
    case 0x23:
        if ((((TrickyState*)trickyState)->stateFlags & TRICKY_STATE_FLAG_MOVE_ADVANCING) != 0)
        {
            objAnimFn_8013a3f0((int)obj, 0x24, lbl_803E2478, 0);
        }
        break;
    case 0x24:
        if (((((TrickyState*)trickyState)->stateFlags & TRICKY_STATE_FLAG_MOVE_ADVANCING) != 0) &&
            ((int)randomGetRange(0, 3) == 0))
        {
            ((TrickyState*)trickyState)->substate = 0;
        }
        break;
    }
    return 1;
}

u32 tricky_substateFidgetB(GameObject* obj, int* trickyState)
{
    short move;
    int foodResult;

    foodResult = tricky_handleFeedOrTalk(obj, trickyState);
    if (foodResult != 0)
    {
        return 1;
    }
    move = (obj)->anim.currentMove;
    switch (move)
    {
    case 0x21:
        if ((((TrickyState*)trickyState)->stateFlags & TRICKY_STATE_FLAG_MOVE_ADVANCING) != 0)
        {
            objAnimFn_8013a3f0((int)obj, 0x22, lbl_803E2478, 0);
        }
        break;
    case 0x22:
        if (((((TrickyState*)trickyState)->stateFlags & TRICKY_STATE_FLAG_MOVE_ADVANCING) != 0) &&
            ((int)randomGetRange(0, 3) == 0))
        {
            ((TrickyState*)trickyState)->substate = 0;
        }
        break;
    }
    return 1;
}

u32 tricky_substateWaitMoveEnd(GameObject* obj, int* trickyState)
{
    int ref;
    int val;
    int idx;

    if (tricky_handleFeedOrTalk(obj, trickyState) != 0)
    {
        return 1;
    }
    for (val = 0; val < *(char*)((int)trickyState + 0x827); val++)
    {
        idx = val + 0x81f;
        if (*(char*)((int)trickyState + idx) != '\0')
            continue;
        ref = *(int*)&(obj)->extra;
        if (((u32)(((TrickyState*)ref)->statusFlags >> 6 & 1)) != 0U)
            continue;
        if ((int)(obj)->anim.currentMove >= 0x30 || (int)(obj)->anim.currentMove < 0x29)
        {
            if (((int (*)(GameObject*, int))Sfx_IsPlayingFromObjectChannel)(obj, 0x10) == 0)
            {
                objAudioFn_800393f8(obj, &((TrickyState*)ref)->soundState, 0x357, 0, 0xffffffff, 0);
            }
        }
    }
    if (tricky_handleFeedOrTalk(obj, trickyState) != 0)
    {
        return 1;
    }
    if ((((TrickyState*)trickyState)->stateFlags & TRICKY_STATE_FLAG_MOVE_ADVANCING) != 0)
    {
        if (((TrickyState*)trickyState)->moveId == (int)(obj)->anim.currentMove)
        {
            ((TrickyState*)trickyState)->substate = 0;
        }
    }
    return 1;
}

int tricky_substateHowlCall(GameObject* obj, int* trickyState)
{
    char bval;
    short move;
    float fval;
    int b[1];
    int val;
    PartFxSpawnParams fxBuf;
    int ia;
    float fa;
    int ib;

    if (tricky_handleFeedOrTalk(obj, trickyState) != 0)
    {
        return 1;
    }
    *(u8*)&(obj)->anim.resetHitboxMode = *(u8*)&(obj)->anim.resetHitboxMode | INTERACT_FLAG_PROMPT_SUPPRESSED;
    move = (obj)->anim.currentMove;
    switch (move)
    {
    case 0x29:
        if ((((TrickyState*)trickyState)->stateFlags & TRICKY_STATE_FLAG_MOVE_ADVANCING) != 0)
        {
            objAnimFn_8013a3f0((int)obj, 0x2a, lbl_803E2520, 0);
        }
        break;
    case 0x2a:
        ((TrickyState*)trickyState)->moveHoldTimer = ((TrickyState*)trickyState)->moveHoldTimer - timeDelta;
        if (((TrickyState*)trickyState)->moveHoldTimer <= lbl_803E23DC)
        {
            if (((((TrickyState*)trickyState)->stateFlags & 0x10000) != 0) || (((TrickyState*)trickyState)->cooldownB.f > lbl_803E23DC))
            {
                objAnimFn_8013a3f0((int)obj, 0x2b, lbl_803E23EC, 0);
            }
            else
            {
                val = (*gSkyInterface)->getSunPosition(0);
                if (val == 0)
                {
                    objAnimFn_8013a3f0((int)obj, 0x2c, lbl_803E251C, 0);
                    ((TrickyState*)trickyState)->substate = 9;
                }
            }
        }
        for (val = 0; val < *(char*)((int)trickyState + 0x827); val++)
        {
            b[0] = val + 0x81f;
            bval = *((char*)trickyState + b[0]);
            if (bval == '\0')
            {
                objAudioFn_800393f8(obj, (ObjSoundState*)(trickyState + 0xea), 0x390, 0x500, -1, 0);
            }
            else if (bval == '\a')
            {
                objAudioFn_800393f8(obj, (ObjSoundState*)(trickyState + 0xea), 0x391, 0x100, -1, 0);
            }
        }
        fval = ((TrickyState*)trickyState)->sparkleFxTimer - timeDelta;
        ((TrickyState*)trickyState)->sparkleFxTimer = fval;
        if (fval <= lbl_803E23DC)
        {
            if (((obj)->objectFlags & OBJECT_OBJFLAG_RENDERED) != 0)
            {
                fxBuf.posX = ((TrickyState*)trickyState)->renderPosX;
                fxBuf.posY = lbl_803E23F8 + ((TrickyState*)trickyState)->renderPosY;
                fxBuf.posZ = ((TrickyState*)trickyState)->renderPosZ;
                (*gPartfxInterface)->spawnObject((void*)obj, 0x7f0, &fxBuf, 0x200001, -1, NULL);
            }
            ((TrickyState*)trickyState)->sparkleFxTimer = lbl_803E24C8;
        }
        break;
    case 0x2b:
        if ((((TrickyState*)trickyState)->stateFlags & TRICKY_STATE_FLAG_MOVE_ADVANCING) != 0)
        {
            if (lbl_803E23DC == ((TrickyState*)trickyState)->waterLevel)
            {
                b[0] = 0;
            }
            else if (lbl_803E2410 == ((TrickyState*)trickyState)->eventTime)
            {
                b[0] = 1;
            }
            else if (((TrickyState*)trickyState)->currentTime - ((TrickyState*)trickyState)->eventTime > lbl_803E2414)
            {
                b[0] = 1;
            }
            else
            {
                b[0] = 0;
            }
            if (b[0] != 0)
            {
                objAnimFn_8013a3f0((int)obj, 8, lbl_803E243C, 0);
                ((TrickyState*)trickyState)->cooldownC = lbl_803E2440;
                ((TrickyState*)trickyState)->particleTimer = lbl_803E23DC;
                trickyDebugPrint(sInWaterMessage);
            }
            else
            {
                objAnimFn_8013a3f0((int)obj, 0, lbl_803E2444, 0);
                trickyDebugPrint(lbl_8031D478);
            }
            {
                u32 m;
                u32 f2 = ((TrickyState*)trickyState)->stateFlags;
                m = ~0x10;
                ((TrickyState*)trickyState)->stateFlags = f2 & m;
            }
            ((TrickyState*)trickyState)->substate = 0;
        }
        break;
    }
    return 1;
}

int tricky_substateSleep(GameObject* obj, int* state)
{
    s8 slots[4];
    u8* ptr;
    u8* e;
    int idx;
    f32 z;

    if (tricky_handleFeedOrTalk(obj, state) != 0)
    {
        ((u8*)state)[0xa] = 0;
        return 1;
    }
    if (cMenuGetSelectedItem() == 0xc1)
    {
        ((u8*)state)[0xa] = 0;
        return 1;
    }
    ((TrickyState*)state)->sfxRepeatTimer -= timeDelta;
    if (((TrickyState*)state)->sfxRepeatTimer < lbl_803E23DC)
    {
        ptr = (obj)->extra;
        if (((u32)((TrickyState*)ptr)->statusFlags >> 6 & 1) == 0 &&
            ((obj)->anim.currentMove >= 0x30 || (obj)->anim.currentMove < 0x29) &&
            Sfx_IsPlayingFromObjectChannel((int)obj, 0x10) == 0)
        {
            objAudioFn_800393f8(obj, &((TrickyState*)ptr)->soundState, 0x29a, 0x100, -1, 0);
        }
        ((TrickyState*)state)->sfxRepeatTimer = lbl_803E2440;
    }
    if (((TrickyState*)state)->child == NULL && Obj_IsLoadingLocked() != 0)
    {
        e = (u8*)Obj_AllocObjectSetup(0x20, TRICKY_CHILD_OBJ_FOOD);
        slots[0] = -1;
        slots[1] = -1;
        slots[2] = -1;
        if (((TrickyState*)state)->childA != NULL)
        {
            slots[((TrickyPackedSlots*)((u8*)state + 0x7bc))->promptASlot] = 1;
        }
        if (((TrickyState*)state)->childB != NULL)
        {
            slots[((TrickyPackedSlots*)((u8*)state + 0x7bc))->promptBSlot] = 1;
        }
        if (((TrickyState*)state)->child != NULL)
        {
            slots[((TrickyPackedSlots*)((u8*)state + 0x7bc))->zzzSlot] = 1;
        }
        if (slots[0] == -1)
        {
            idx = 0;
        }
        else if (slots[1] == -1)
        {
            idx = 1;
        }
        else if (slots[2] == -1)
        {
            idx = 2;
        }
        else if (slots[3] == -1)
        {
            idx = 3;
        }
        else
        {
            idx = -1;
        }
        ((TrickyPackedSlots*)((u8*)state + 0x7bc))->zzzSlot = idx;
        ((TrickyState*)state)->child = Obj_SetupObject((ObjPlacement*)e, 4, -1, -1, (obj)->anim.parent);
        ObjLink_AttachChild(obj, ((TrickyState*)state)->child,
                            ((TrickyPackedSlots*)((u8*)state + 0x7bc))->zzzSlot);
        z = lbl_803E23DC;
        ((TrickyState*)state)->childPhaseTimer0 = z;
        ((TrickyState*)state)->childPhaseTimer1 = z;
        ((TrickyState*)state)->childPhaseTimer2 = z;
    }
    if ((*gSkyInterface)->getSunPosition(0) != 0 && ((TrickyState*)state)->cooldownA <= lbl_803E23DC &&
        mainGetBit(GAMEBIT_ITEM_TrickyCall_Got) != 0)
    {
        objAnimFn_8013a3f0((int)obj, 0x29, lbl_803E2444, 0);
        ptr = (obj)->extra;
        if (((u32)((TrickyState*)ptr)->statusFlags >> 6 & 1) == 0 &&
            ((obj)->anim.currentMove >= 0x30 || (obj)->anim.currentMove < 0x29) &&
            Sfx_IsPlayingFromObjectChannel((int)obj, 0x10) == 0)
        {
            objAudioFn_800393f8(obj, &((TrickyState*)ptr)->soundState, 0x354, 0x1000, -1, 0);
        }
        ((TrickyState*)state)->stateFlags |= 0x10;
        ((u8*)state)[0xa] = 4;
        ((TrickyState*)state)->moveHoldTimer = (f32)(int)randomGetRange(0x78, 0xf0);
    }
    return 1;
}

u32 tricky_substateWaitQueuedMove(GameObject* obj, int* trickyState)
{
    int val;

    val = tricky_handleFeedOrTalk(obj, trickyState);
    if (val != 0)
    {
        return 1;
    }
    if ((((TrickyState*)trickyState)->stateFlags & TRICKY_STATE_FLAG_MOVE_ADVANCING) != 0)
    {
        if (((TrickyState*)trickyState)->moveId == (int)(obj)->anim.currentMove)
        {
            ((TrickyState*)trickyState)->substate = 0;
        }
    }
    return 1;
}

u32 tricky_substateReturnToHeel(GameObject* obj, int* trickyState)
{
    int val;

    val = tricky_handleFeedOrTalk(obj, trickyState);
    if (val != 0)
    {
        return 1;
    }
    val = trickyFn_8013b368((GameObject*)obj, lbl_803E2408, (TrickyState*)trickyState);
    if (val == 1)
    {
        if (lbl_803E23DC == ((TrickyState*)trickyState)->cooldownA)
        {
            ((TrickyState*)trickyState)->substate = 0;
        }
        return 1;
    }
    ((TrickyState*)trickyState)->substate = 0;
    return 0;
}

int tricky_substateFollowIdle(GameObject* obj, int state)
{
    int tex;
    short move;
    u8 result;
    int followBase;
    int inWater;
    float threshold;

    *(int*)&((TrickyState*)state)->followObj = (int)((TrickyState*)state)->playerObj;
    followBase = *(u32*)&((TrickyState*)state)->followObj + 0x18;
    if (*(u32*)&((TrickyState*)state)->targetPosPtr != followBase)
    {
        *(int*)&((TrickyState*)state)->targetPosPtr = followBase;
        {
            u32 m;
            u32 f2 = ((TrickyState*)state)->stateFlags;
            m = ~0x400;
            ((TrickyState*)state)->stateFlags = f2 & m;
        }
        ((TrickyState*)state)->linkedWalkGroup = 0;
    }
    if (lbl_803E23DC == ((TrickyState*)state)->cooldownA)
    {
        {
            s8 mm;
            mm = -1;
            ((TrickyState*)state)->commandPhase = mm;
        }
        threshold = lbl_803E24C8;
    }
    else
    {
        if ((((TrickyState*)state)->stateFlags & 0x20000) != 0)
        {
            ((TrickyState*)state)->commandPhase = 0;
            ((TrickyState*)state)->stateFlags = ((TrickyState*)state)->stateFlags & ~0x20000LL;
        }
        threshold = lbl_803E2408;
    }
    result = trickyFn_8013b368((GameObject*)obj, threshold, (TrickyState*)state);
    if (result != 1)
    {
        if (result == 2)
        {
            if ((((TrickyState*)state)->stateFlags & 2) != 0)
            {
                tex = *(int*)&(obj)->extra;
                if (((((TrickyState*)tex)->statusFlags >> 6) & 1) == 0u)
                {
                    move = (obj)->anim.currentMove;
                    if (move >= 48 || move < 41)
                    {
                        if (Sfx_IsPlayingFromObjectChannel((int)obj, 16) == 0)
                        {
                            objAudioFn_800393f8(obj, &((TrickyState*)tex)->soundState, 861, 1280, -1, 0);
                        }
                    }
                }
            }
        }
        if (lbl_803E23DC == ((TrickyState*)state)->waterLevel)
        {
            inWater = 0;
        }
        else if (lbl_803E2410 == ((TrickyState*)state)->eventTime)
        {
            inWater = 1;
        }
        else if (((TrickyState*)state)->currentTime - ((TrickyState*)state)->eventTime > lbl_803E2414)
        {
            inWater = 1;
        }
        else
        {
            inWater = 0;
        }
        if (inWater != 0)
        {
            return 0;
        }
        return tricky_updateIdleBehavior((int)obj, (int*)state);
    }
    ((FlagByte728*)&((TrickyState*)state)->stateFlags728)->bf7 = 1;
    return 1;
}

u32 tricky_updateIdleBehavior(int obj, int* trickyState)
{
    int done;
    int extra;
    u32 bitVal;

    done = tricky_handleFeedOrTalk((GameObject*)(obj), trickyState);
    if (done != 0)
    {
        return 1;
    }
    if (((TrickyState*)trickyState)->cooldownC > lbl_803E23DC)
    {
        objAnimFn_8013a3f0(obj, 0x1b, lbl_803E23EC, 0);
        ((TrickyState*)trickyState)->substate = 2;
        ((TrickyState*)trickyState)->cooldownC = lbl_803E23DC;
        return 1;
    }
    if ((((TrickyState*)trickyState)->stateFlags728 >> 7 & 1) != 0U)
    {
        *(f32*)&((TrickyState*)trickyState)->unk724 = lbl_803E2524;
        ((FlagByte728*)&((TrickyState*)trickyState)->stateFlags728)->bf7 = 0;
        ((FlagByte728*)&((TrickyState*)trickyState)->stateFlags728)->bf6 = 1;
    }
    if ((((TrickyState*)trickyState)->stateFlags728 >> 6 & 1) != 0U)
    {
        *(f32*)&((TrickyState*)trickyState)->unk724 = *(f32*)&((TrickyState*)trickyState)->unk724 - timeDelta;
        if (*(f32*)&((TrickyState*)trickyState)->unk724 <= lbl_803E23DC)
        {
            ((TrickyState*)trickyState)->cooldownA = lbl_803E2438;
            bitVal = randomGetRange(200, 500);
            *(f32*)&((TrickyState*)trickyState)->unk724 = (f32)(s32)(bitVal);
            ((FlagByte728*)&((TrickyState*)trickyState)->stateFlags728)->bf6 = 0;
            ((TrickyState*)trickyState)->substate = 1;
        }
        return 0;
    }
    if (Sfx_IsPlayingFromObjectChannel(obj, 0x10))
    {
        return 1;
    }
    done = (*gSkyInterface)->getSunPosition(0);
    if (done == 0)
    {
        ((TrickyState*)trickyState)->stateFlags = ((TrickyState*)trickyState)->stateFlags & ~0x20000000LL;
    }
    done = (*gSkyInterface)->getSunPosition(0);
    if ((done != 0) && ((((TrickyState*)trickyState)->stateFlags & 0x20000000U) == 0))
    {
        ((TrickyState*)trickyState)->stateFlags = ((TrickyState*)trickyState)->stateFlags | 0x20000000LL;
        done = *(int*)&((GameObject*)obj)->extra;
        if (((((TrickyState*)done)->statusFlags >> 6 & 1) == 0U) &&
            ((((GameObject*)obj)->anim.currentMove >= 0x30 || (((GameObject*)obj)->anim.currentMove < 0x29)) &&
             !Sfx_IsPlayingFromObjectChannel(obj, 0x10)))
        {
            objAudioFn_800393f8((GameObject*)obj, &((TrickyState*)done)->soundState, 0x353, 0x500, 0xffffffff, 0);
        }
        return 0;
    }
    if (*(u8*)*trickyState <= 3)
    {
        objAnimFn_8013a3f0(obj, 0x14, lbl_803E2444, 0);
        ((TrickyState*)trickyState)->substate = 3;
        ((TrickyState*)trickyState)->sfxRepeatTimer = lbl_803E2440;
        return 1;
    }
    *(f32*)&((TrickyState*)trickyState)->unk724 = *(f32*)&((TrickyState*)trickyState)->unk724 - timeDelta;
    if (*(f32*)&((TrickyState*)trickyState)->unk724 <= lbl_803E23DC)
    {
        bitVal = randomGetRange(200, 500);
        *(f32*)&((TrickyState*)trickyState)->unk724 = (f32)(s32)(bitVal);
        if (*(u8*)*trickyState <= 7)
        {
            objAnimFn_8013a3f0(obj, 0x14, lbl_803E2444, 0);
            ((TrickyState*)trickyState)->substate = 3;
            ((TrickyState*)trickyState)->sfxRepeatTimer = lbl_803E2440;
            return 1;
        }
        if (((TrickyState*)trickyState)->cooldownA > lbl_803E23DC)
        {
            tricky_startRandomIdleMove((GameObject*)(obj), (int)trickyState);
        }
        else
        {
            if ((u32)((TrickyState*)trickyState)->childB != 0)
            {
                extra = *(int*)&((GameObject*)obj)->extra;
                if ((((((TrickyState*)extra)->statusFlags >> 6 & 1) == 0U) &&
                     (((GameObject*)obj)->anim.currentMove >= 0x30 || (((GameObject*)obj)->anim.currentMove < 0x29)) &&
                     !Sfx_IsPlayingFromObjectChannel(obj, 0x10)))
                {
                    objAudioFn_800393f8((GameObject*)obj, &((TrickyState*)extra)->soundState, 0x357, 0, 0xffffffff, 0);
                }
                objAnimFn_8013a3f0(obj, 0x26, lbl_803E251C, 0);
                ((TrickyState*)trickyState)->substate = 5;
            }
            else
            {
                bitVal = randomGetRange(0, 6);
                switch ((int)bitVal)
                {
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                    tricky_startRandomIdleMove((GameObject*)(obj), (int)trickyState);
                    break;
                default:
                    tricky_pickAmbientActivity((u8*)obj, (u8*)trickyState);
                    break;
                }
            }
        }
        return 1;
    }
    return 0;
}

void tricky_pickAmbientActivity(u8* obj, u8* state)
{
    f32 arr[2];
    u8* ptr;
    u8 lo;
    u8 hi;
    u8* found;
    int sv;
    f32 ang;

    lo = 1;
    hi = 3;
    arr[0] = lbl_803E2524;
    found = (u8*)ObjGroup_FindNearestObject(THORNTAIL_OBJGROUP, (GameObject*)obj, arr);
    if (found != NULL && (((GameObject*)found)->objectFlags & OBJECT_OBJFLAG_RENDERED) != 0)
    {
        lo = 0;
    }
    if ((*gSkyInterface)->getSunPosition(0) == 0 || mainGetBit(GAMEBIT_ITEM_TrickyCall_Got) == 0)
    {
        hi = 2;
    }
    switch (randomGetRange(lo, hi))
    {
    case 0:
        ((TrickyState*)state)->followObj = (GameObject*)found;
        objPosFn_80039510((GameObject*)found, 0, (float*)(state + 0x72c));
        if ((u8*)((TrickyState*)state)->targetPosPtr != state + 0x72c)
        {
            ((TrickyState*)state)->targetPosPtr = (f32*)(state + 0x72c);
            {
                u32 m;
                u32 f2 = ((TrickyState*)state)->stateFlags;
                m = ~0x400;
                ((TrickyState*)state)->stateFlags = f2 & m;
            }
            ((TrickyState*)state)->linkedWalkGroup = 0;
        }
        ((FlagByte728*)&((TrickyState*)state)->stateFlags728)->bf5 = 0;
        state[0xa] = 0xc;
        break;
    case 1:
        sv = randomGetRange(0x20, 0xff);
        sv = (s16)((((GameObject*)obj)->anim.rotX + sv) * 0x100);
        ang = lbl_803E2454 * (f32)sv / lbl_803E2458;
        ((TrickyState*)state)->wanderTargetX =
            (f32)(lbl_803E2528 * -mathSinf(ang) + ((GameObject*)obj)->anim.localPosX);
        ((TrickyState*)state)->wanderTargetY = ((GameObject*)obj)->anim.localPosY;
        ((TrickyState*)state)->wanderTargetZ =
            (f32)(lbl_803E2484 * -mathCosf(ang) + ((GameObject*)obj)->anim.localPosZ);
        if ((u8*)((TrickyState*)state)->targetPosPtr != state + 0x72c)
        {
            ((TrickyState*)state)->targetPosPtr = (f32*)(state + 0x72c);
            {
                u32 m;
                u32 f2 = ((TrickyState*)state)->stateFlags;
                m = ~0x400;
                ((TrickyState*)state)->stateFlags = f2 & m;
            }
            ((TrickyState*)state)->linkedWalkGroup = 0;
        }
        state[0xa] = 8;
        break;
    case 2:
        objAnimFn_8013a3f0((int)obj, 0x2d, lbl_803E2530, 0);
        ((TrickyState*)state)->stateFlags |= 0x10;
        state[0xa] = 9;
        break;
    case 3:
        objAnimFn_8013a3f0((int)obj, 0x29, lbl_803E2444, 0);
        ptr = ((GameObject*)obj)->extra;
        if (((u32)((TrickyState*)ptr)->statusFlags >> 6 & 1) == 0 &&
            (((GameObject*)obj)->anim.currentMove >= 0x30 || ((GameObject*)obj)->anim.currentMove < 0x29) &&
            Sfx_IsPlayingFromObjectChannel((int)obj, 0x10) == 0)
        {
            objAudioFn_800393f8((GameObject*)obj, &((TrickyState*)ptr)->soundState, 0x354, 0x1000, -1, 0);
        }
        ((TrickyState*)state)->stateFlags |= 0x10;
        state[0xa] = 4;
        ((TrickyState*)state)->moveHoldTimer = (f32)(int)randomGetRange(0x78, 0xf0);
        break;
    }
}

void tricky_startRandomIdleMove(GameObject* obj, int trickyState)
{
    int val;
    int state;

    val = randomGetRange(0, 4);
    switch (val)
    {
    case 0:
        objAnimFn_8013a3f0((int)obj, 0, lbl_803E2444, 0);
        ((TrickyState*)trickyState)->substate = 2;
        break;
    case 1:
        state = *(int*)&(obj)->extra;
        if (((u32)(((TrickyState*)state)->statusFlags >> 6 & 1)) == 0U)
        {
            if ((obj)->anim.currentMove >= 0x30 || (obj)->anim.currentMove < 0x29)
            {
                if (Sfx_IsPlayingFromObjectChannel((int)obj, 0x10) == 0)
                {
                    objAudioFn_800393f8(obj, &((TrickyState*)state)->soundState, 0x357, 0, 0xffffffff, 0);
                }
            }
        }
        objAnimFn_8013a3f0((int)obj, 0x26, lbl_803E251C, 0);
        ((TrickyState*)trickyState)->substate = 5;
        break;
    case 2:
        objAnimFn_8013a3f0((int)obj, 0x21, lbl_803E2478, 0);
        ((TrickyState*)trickyState)->substate = 6;
        break;
    case 3:
        objAnimFn_8013a3f0((int)obj, 0x23, lbl_803E2478, 0);
        ((TrickyState*)trickyState)->substate = 7;
        break;
    case 4:
        objAnimFn_8013a3f0((int)obj, 0x25, lbl_803E2518, 0);
        ((TrickyState*)trickyState)->substate = 2;
        break;
    }
}

int tricky_handleFeedOrTalk(GameObject* obj, int* state)
{
    u8* b;
    u8 gu;
    int g;
    u8 flag;
    u8 a;
    u8 c;
    u8 d;
    u8 n;
    u8 cnt;
    int inWater;
    s16 item[4];

    flag = 0;
    *(u8*)&obj->anim.resetHitboxMode &= ~INTERACT_FLAG_PROMPT_SUPPRESSED;
    n = mainGetBit(GAMEBIT_ITEM_TrickyFood_Count);
    if (n != 0)
    {
        getYButtonItem(item);
        if (item[0] == 0xc1)
        {
            flag = 1;
        }
        if (cMenuGetSelectedItem() == 0xc1)
        {
            flag = 1;
        }
    }
    if (flag != 0)
    {
        if (*(u8*)&obj->anim.resetHitboxMode & INTERACT_FLAG_ACTIVATED)
        {
            if ((*gGameUIInterface)->isEventReady(0xc1) != 0)
            {
                a = **(u8**)state;
                c = *(*(u8**)state + 1);
                if (a == c)
                {
                    b = obj->extra;
                    ((TrickyState*)b)->stateFlags |= 0x4000;
                    ((TrickyState*)b)->stateFlags |= 1;
                    if (lbl_803E23DC == ((TrickyState*)b)->waterLevel)
                    {
                        inWater = 0;
                    }
                    else if (lbl_803E2410 == ((TrickyState*)b)->eventTime)
                    {
                        inWater = 1;
                    }
                    else if (((TrickyState*)b)->currentTime - ((TrickyState*)b)->eventTime > lbl_803E2414)
                    {
                        inWater = 1;
                    }
                    else
                    {
                        inWater = 0;
                    }
                    if (inWater != 0)
                    {
                        objAnimFn_8013a3f0((int)obj, 8, lbl_803E243C, 0);
                        ((TrickyState*)b)->cooldownC = lbl_803E2440;
                        ((TrickyState*)b)->particleTimer = lbl_803E23DC;
                        trickyDebugPrint(sInWaterMessage);
                    }
                    else
                    {
                        objAnimFn_8013a3f0((int)obj, 0, lbl_803E2444, 0);
                        trickyDebugPrint(lbl_8031D478);
                    }
                    (*gObjectTriggerInterface)->runSequence(3, (void*)obj, -1);
                    ((TrickyByteFlags*)&((TrickyState*)b)->flags82E)->bit5 = 1;
                }
                else
                {
                    d = c - a;
                    cnt = (u32)d >> 2;
                    if (d % 4)
                    {
                        cnt += 1;
                    }
                    if (cnt > n)
                    {
                        ((TrickyState*)state)->progressValue = a + (n << 2);
                        mainSetBits(GAMEBIT_ITEM_TrickyFood_Count, 0);
                    }
                    else
                    {
                        ((TrickyState*)state)->progressValue = a + (cnt << 2);
                        mainSetBits(GAMEBIT_ITEM_TrickyFood_Count, n - cnt);
                    }
                    if (((TrickyState*)state)->progressValue > *(*(u8**)state + 1))
                    {
                        ((TrickyState*)state)->progressValue = *(*(u8**)state + 1);
                    }
                    b = obj->extra;
                    ((TrickyState*)b)->stateFlags |= 0x4000;
                    if (lbl_803E23DC == ((TrickyState*)b)->waterLevel)
                    {
                        inWater = 0;
                    }
                    else if (lbl_803E2410 == ((TrickyState*)b)->eventTime)
                    {
                        inWater = 1;
                    }
                    else if (((TrickyState*)b)->currentTime - ((TrickyState*)b)->eventTime > lbl_803E2414)
                    {
                        inWater = 1;
                    }
                    else
                    {
                        inWater = 0;
                    }
                    if (inWater != 0)
                    {
                        objAnimFn_8013a3f0((int)obj, 8, lbl_803E243C, 0);
                        ((TrickyState*)b)->cooldownC = lbl_803E2440;
                        ((TrickyState*)b)->particleTimer = lbl_803E23DC;
                        trickyDebugPrint(sInWaterMessage);
                    }
                    else
                    {
                        objAnimFn_8013a3f0((int)obj, 0, lbl_803E2444, 0);
                        trickyDebugPrint(lbl_8031D478);
                    }
                    (*gObjectTriggerInterface)->runSequence(2, (void*)obj, -1);
                    ((TrickyByteFlags*)&((TrickyState*)b)->flags82E)->bit5 = 1;
                    ((TrickyState*)state)->stateFlags |= 0x40000000LL;
                }
                buttonDisable(0, PAD_BUTTON_A);
                return 1;
            }
        }
        else
        {
            *(u8*)&obj->anim.resetHitboxMode &= ~INTERACT_FLAG_DISABLED;
            Obj_SetActiveHitVolumeBounds(obj, 0, 0, 0, 0, 4);
        }
    }
    else
    {
        gu = mainGetBit(GAMEBIT_TrickyTalk);
        if (gu != 0xff && cMenuGetSelectedItem() == -1)
        {
            if (*(u8*)&obj->anim.resetHitboxMode & INTERACT_FLAG_ACTIVATED)
            {
                mainSetBits(GAMEBIT_TrickyTalk, 0xff);
                b = obj->extra;
                g = gu;
                ((TrickyState*)b)->stateFlags |= 0x4000;
                if (g != 2)
                {
                    ((TrickyState*)b)->stateFlags |= 1;
                }
                if (lbl_803E23DC == ((TrickyState*)b)->waterLevel)
                {
                    inWater = 0;
                }
                else if (lbl_803E2410 == ((TrickyState*)b)->eventTime)
                {
                    inWater = 1;
                }
                else if (((TrickyState*)b)->currentTime - ((TrickyState*)b)->eventTime > lbl_803E2414)
                {
                    inWater = 1;
                }
                else
                {
                    inWater = 0;
                }
                if (inWater != 0)
                {
                    objAnimFn_8013a3f0((int)obj, 8, lbl_803E243C, 0);
                    ((TrickyState*)b)->cooldownC = lbl_803E2440;
                    ((TrickyState*)b)->particleTimer = lbl_803E23DC;
                    trickyDebugPrint(sInWaterMessage);
                }
                else
                {
                    objAnimFn_8013a3f0((int)obj, 0, lbl_803E2444, 0);
                    trickyDebugPrint(lbl_8031D478);
                }
                (*gObjectTriggerInterface)->runSequence(g, (void*)obj, -1);
                ((TrickyByteFlags*)&((TrickyState*)b)->flags82E)->bit5 = 1;
                buttonDisable(0, PAD_BUTTON_A);
                return 1;
            }
            *(u8*)&obj->anim.resetHitboxMode &= ~INTERACT_FLAG_DISABLED;
            Obj_SetActiveHitVolumeBounds(obj, 0, 0, 0, 0, 2);
        }
    }
    return 0;
}

void tricky_handlePlayerContact(u8* obj, u8* state)
{
    int hit[1];
    u8* ptr;
    f32 fv;
    int inWater;

    ((TrickyState*)state)->cooldownB.f -= timeDelta;
    if (((TrickyState*)state)->cooldownB.f < *(f32*)&lbl_803E23DC)
    {
        ((TrickyState*)state)->cooldownB.f = lbl_803E23DC;
    }
    if (ObjHits_GetPriorityHit((GameObject*)obj, hit, 0, 0) != 0 && *(u8**)(hit[0] + 0xc4) != NULL &&
        *(s16*)(*(u8**)(hit[0] + 0xc4) + 0x44) == 1)
    {
        fv = ((TrickyState*)state)->cooldownB.f;
        if (fv <= lbl_803E23DC)
        {
            ((TrickyState*)state)->cooldownB.f = fv + lbl_803E24EC;
            ptr = ((GameObject*)obj)->extra;
            if (((u32)((TrickyState*)ptr)->statusFlags >> 6 & 1) == 0 &&
                (((GameObject*)obj)->anim.currentMove >= 0x30 || ((GameObject*)obj)->anim.currentMove < 0x29) &&
                Sfx_IsPlayingFromObjectChannel((int)obj, 0x10) == 0)
            {
                objAudioFn_800393f8((GameObject*)obj, &((TrickyState*)ptr)->soundState, 0x34f, 0x500, -1, 0);
            }
        }
        else
        {
            ((TrickyState*)state)->cooldownB.f = fv + lbl_803E2440;
            if (state[0xa] != 0xb)
            {
                if (((TrickyState*)state)->stateFlags & 0x10)
                {
                    if (((TrickyState*)state)->cooldownB.f > lbl_803E2534)
                    {
                        ((TrickyState*)state)->cooldownB.f *= lbl_803E24A8;
                        if (mainGetBit(GAMEBIT_ITEM_TrickyFlame_Got) != 0)
                        {
                            if (lbl_803E23DC == ((TrickyState*)state)->waterLevel)
                            {
                                inWater = 0;
                            }
                            else if (lbl_803E2410 == ((TrickyState*)state)->eventTime)
                            {
                                inWater = 1;
                            }
                            else if (((TrickyState*)state)->currentTime - ((TrickyState*)state)->eventTime >
                                     lbl_803E2414)
                            {
                                inWater = 1;
                            }
                            else
                            {
                                inWater = 0;
                            }
                            if (inWater == 0)
                            {
                                state[0xa] = 0xb;
                                return;
                            }
                        }
                        ptr = ((GameObject*)obj)->extra;
                        if (((u32)((TrickyState*)ptr)->statusFlags >> 6 & 1) == 0 &&
                            (((GameObject*)obj)->anim.currentMove >= 0x30 ||
                             ((GameObject*)obj)->anim.currentMove < 0x29) &&
                            Sfx_IsPlayingFromObjectChannel((int)obj, 0x10) == 0)
                        {
                            objAudioFn_800393f8((GameObject*)obj, &((TrickyState*)ptr)->soundState, 0x350, 0x500, -1,
                                               0);
                        }
                    }
                    else
                    {
                        ptr = ((GameObject*)obj)->extra;
                        if (((u32)((TrickyState*)ptr)->statusFlags >> 6 & 1) == 0 &&
                            (((GameObject*)obj)->anim.currentMove >= 0x30 ||
                             ((GameObject*)obj)->anim.currentMove < 0x29) &&
                            Sfx_IsPlayingFromObjectChannel((int)obj, 0x10) == 0)
                        {
                            objAudioFn_800393f8((GameObject*)obj, &((TrickyState*)ptr)->soundState, 0x350, 0x500, -1,
                                               0);
                        }
                    }
                }
                else
                {
                    ptr = ((GameObject*)obj)->extra;
                    if (((u32)((TrickyState*)ptr)->statusFlags >> 6 & 1) == 0 &&
                        (((GameObject*)obj)->anim.currentMove >= 0x30 || ((GameObject*)obj)->anim.currentMove < 0x29) &&
                        Sfx_IsPlayingFromObjectChannel((int)obj, 0x10) == 0)
                    {
                        objAudioFn_800393f8((GameObject*)obj, &((TrickyState*)ptr)->soundState, 0x350, 0x500, -1, 0);
                    }
                    state[0xa] = 10;
                    ((TrickyState*)state)->stateFlags |= 0x10;
                }
            }
        }
    }
}


typedef struct TrickyDestroyState
{
    u8 pad0[0x700 - 0x0];
    s32 childObj; /* 0x700: child flame object handle (per-slot, walked by Tricky_free) */
    u8 pad704[0x708 - 0x704];
} TrickyDestroyState;

typedef struct TrickyInitFlags
{
    u8 initBit7 : 1;
    u8 bit6 : 1;
    u8 bit5 : 1;
    u8 bit4 : 1;
    u8 bit3 : 1;
    u8 bit2 : 1;
    u8 bit1 : 1;
    u8 bit0 : 1;
} TrickyInitFlags;

typedef struct TrickyStatusFlags58
{
    u8 bit7 : 1;
    u8 bit6 : 1;
    u8 heightTracking : 1;
    u8 bit4 : 1;
    u8 bit3 : 1;
    u8 bit2 : 1;
    u8 bit1 : 1;
    u8 bit0 : 1;
} TrickyStatusFlags58;

typedef void (*TrickyHandlerFn)(int obj, int state);


/* group owned by another DLL, queried here */

/* child/reward objects spawned by this DLL (retail OBJECTS.bin names) */
#define TRICKY_CHILD_OBJ_BADGE_A       0x244 /* "TrickyBadge" */
#define TRICKY_CHILD_OBJ_BADGE_B       0x254 /* "TrickyBadge" */
#define TRICKY_CHILD_OBJ_QUEST         0x17c /* "TrickyQuest..." */
#define TRICKY_CHILD_OBJ_EXCLAMATION   0x175 /* "TrickyExcla..." */
#define TRICKY_CHILD_OBJ_SIDEKICK_BALL 0x112 /* "SidekickBal..." (DLL 0xF5 sidekickball) */
#define TRICKY_OBJ_BLUE_MUSHROOM       0x6a  /* "BlueMushroo..." (DLL 0x1A7) */

/* stateFlags movement-enable bits: each gates applying its matching per-frame
 * position delta (backstepDelta / verticalDelta / sidestepDelta) or the
 * rotate-toward-target interpolation in the per-frame update. */
#define TRICKY_STATE_FLAG_SIDESTEP      0x20  /* apply sidestepDelta lateral offset */
#define TRICKY_STATE_FLAG_BACKSTEP      0x40  /* apply backstepDelta offset */
#define TRICKY_STATE_FLAG_VERTICAL_MOVE 0x80  /* apply verticalDelta to localPosY */
#define TRICKY_STATE_FLAG_ROTATE        0x100 /* interpolate rotation toward targetYaw target */
/* stateFlags flame-particle child bookkeeping: 0x800 marks the 7 flame children
 * as spawned; on teardown it is cleared and 0x1000 is set. */
#define TRICKY_STATE_FLAG_FLAME_CHILDREN_ACTIVE  0x800  /* 7 flame child objects are spawned */
#define TRICKY_STATE_FLAG_FLAME_CHILDREN_CLEANUP 0x1000 /* flame children torn down this cycle */
/* GameObject.objectFlags bit (distinct field from stateFlags above). */
#define TRICKY_OBJFLAG_PARENT_SLACK            0x1000
#define TRICKY_HEIGHT_TRACK_FIREPIPE_OBJECT_ID 0x46406
#define TRICKY_OBJGROUP                        1
#define TRICKY_BBOX_HIT_SCRATCH_SIZE           84
int lbl_803DDA4C;
u32 gTrickyHelperObject;

u16 gSkeetlaFootstepSfxId2 = 0x355;
f32 lbl_803E23DC = 0.0f;
f32 lbl_803E23E0 = 10.0f;
f32 lbl_803E23E4 = 0.004f;
f32 lbl_803E23E8 = 1.0f;
f32 lbl_803E23EC = 0.01f;
f32 lbl_803E23F0 = 0.7f;
f32 lbl_803E23F4 = -0.01f;
f32 lbl_803E23F8 = 2.0f;
f32 lbl_803E2408 = 20.0f;
f32 lbl_803E240C = 196.0f;
f32 lbl_803E2410 = -100000.0f;
f32 lbl_803E2414 = 8.0f;
f32 lbl_803E2418 = 3.4028235e38f;
f32 lbl_803E241C = -0.15f;
f32 lbl_803E2420 = 0.05f;
f32 lbl_803E2424 = 100.0f;
f32 lbl_803E2428 = -0.17f;
f32 lbl_803E242C = 40.0f;
f32 lbl_803E2430 = 400.0f;
f32 lbl_803E2434 = 0.014f;
f32 lbl_803E2438 = 300.0f;
f32 lbl_803E243C = 0.02f;
f32 lbl_803E2440 = 600.0f;
f32 lbl_803E2444 = 0.005f;
f32 lbl_803E2448 = -2.0f;
f32 lbl_803E244C = 1.5f;
f32 lbl_803E2450 = 512.0f;
f32 lbl_803E2454 = 3.1415927f;
f32 lbl_803E2458 = 32768.0f;

u8* Tricky_findNearestGroup4BObject(u8* obj, TrickyState* state)
{
    int* objs;
    int count[1];
    u8* result;
    f32 d;
    f32 bestD;
    int i;

    result = 0;
    objs = (int*)ObjGroup_GetObjects(TRICKYWARP_OBJ_GROUP, count);
    d = getXZDistance(&state->playerObj->anim.worldPosX, &((GameObject*)obj)->anim.worldPosX);
    if ((d >= lbl_803E2538) || (state->cooldownA > lbl_803E23DC))
    {
        if (ViewFrustum_IsSphereVisible(&((GameObject*)obj)->anim.localPosX, lbl_803E2500) == 0)
        {
            bestD = lbl_803E2418;
            for (i = 0; i < count[0]; i++)
            {
                f32 cd = getXZDistance(&state->playerObj->anim.worldPosX, &((GameObject*)*objs)->anim.worldPosX);
                if (cd < d && cd < bestD)
                {
                    bestD = cd;
                    result = (u8*)*objs;
                }
                objs++;
            }
        }
    }
    return result;
}

void tricky_stateIdleWander(GameObject* obj, int state)
{
    int sfxState;
    int isInWater;
    u32 sfxDisabled;
    u32 transitionFlag;

    if (tricky_handleFeedOrTalk(obj, (int*)state) == 0)
    {
        ((TrickyState*)state)->wanderTargetX =
            (obj)->anim.worldPosX - mathSinf((lbl_803E2454 * (f32) * (s16*)obj) / lbl_803E2458);
        ((TrickyState*)state)->wanderTargetY = (obj)->anim.worldPosY;
        ((TrickyState*)state)->wanderTargetZ =
            (obj)->anim.worldPosZ - mathCosf((lbl_803E2454 * (f32) * (s16*)obj) / lbl_803E2458);

        if (trickyFn_8013b368(obj, lbl_803E247C, (TrickyState*)state) != 1)
        {
            ((TrickyState*)state)->idleSfxTimer -= timeDelta;
            if (((TrickyState*)state)->idleSfxTimer <= lbl_803E23DC)
            {
                ((TrickyState*)state)->idleSfxTimer = (f32)(int)randomGetRange(0x1f4, 0x2ee);
                sfxState = *(int*)&(obj)->extra;
                sfxDisabled = ((TrickyByteFlags*)&((TrickyState*)sfxState)->statusFlags)->bit6;
                if ((sfxDisabled == 0) && (((obj)->anim.currentMove >= 0x30) || ((obj)->anim.currentMove < 0x29)) &&
                    (Sfx_IsPlayingFromObjectChannel((int)obj, 0x10) == 0))
                {
                    objAudioFn_800393f8(obj, &((TrickyState*)sfxState)->soundState, 0x360, 0x500, -1, 0);
                }
            }

            if (lbl_803E23DC == ((TrickyState*)state)->waterLevel)
            {
                isInWater = 0;
            }
            else if (lbl_803E2410 == ((TrickyState*)state)->eventTime)
            {
                isInWater = 1;
            }
            else if ((((TrickyState*)state)->currentTime - ((TrickyState*)state)->eventTime) > lbl_803E2414)
            {
                isInWater = 1;
            }
            else
            {
                isInWater = 0;
            }

            if (isInWater)
            {
                objAnimFn_8013a3f0((int)obj, 8, lbl_803E243C, 0);
                ((TrickyState*)state)->cooldownC = lbl_803E2440;
                ((TrickyState*)state)->particleTimer = lbl_803E23DC;
                trickyDebugPrint(sInWaterMessage);
            }
            else
            {
                switch ((obj)->anim.currentMove)
                {
                case 0x31:
                    break;
                case 0xd:
                    transitionFlag = ((TrickyState*)state)->stateFlags & 0x08000000;
                    if (transitionFlag != 0)
                    {
                        objAnimFn_8013a3f0((int)obj, 0x31, lbl_803E243C, 0);
                    }
                    break;
                default:
                    objAnimFn_8013a3f0((int)obj, 0xd, lbl_803E2444, 0);
                    break;
                }
                trickyDebugPrint(lbl_8031D478);
            }
        }
    }
}

void tricky_attachToWalkGroup(GameObject* obj, int state)
{
    u8 pathBytes[16];
    u32 pathByte = (u8)Objfsa_GetWalkGroupIndexAtPoint(&obj->anim.worldPosX, NULL);

    pathBytes[0] = pathByte;
    if (pathByte == 0)
    {
        int pathId = Objfsa_GetPatchGroupIdAtPoint(&obj->anim.worldPosX);
        if (pathId != 0)
        {
            walkPath_writeU16LE(pathId & 0xffff, pathBytes);
        }
    }
    if (pathBytes[0] != 0)
    {
        f32 resetTimer;

        ((TrickyState*)state)->walkGroup = pathBytes[0];
        ((TrickyState*)state)->stateIndex = 1;
        ((TrickyState*)state)->substate = 0;
        resetTimer = lbl_803E23DC;
        ((TrickyState*)state)->cooldownA = resetTimer;
        ((TrickyState*)state)->cooldownB.f = resetTimer;
        ((TrickyState*)state)->stateFlags = ((TrickyState*)state)->stateFlags & (u64)~0x10u;
        ((TrickyState*)state)->stateFlags = ((TrickyState*)state)->stateFlags & (u64)~0x10000u;
        ((TrickyState*)state)->stateFlags = ((TrickyState*)state)->stateFlags & (u64)~0x20000u;
        ((TrickyState*)state)->stateFlags = ((TrickyState*)state)->stateFlags & (u64)~0x40000u;
        *(s8*)&((TrickyState*)state)->commandPhase = -1;
    }
    if (gTrickyHelperObject == 0)
    {
        int setup = (int)Obj_AllocObjectSetup(0x18, 0x25);
        gTrickyHelperObject = (int)Obj_SetupObject((ObjPlacement*)setup, 4, -1, -1, obj->anim.parent);
    }
    ((TrickyByteFlags*)&((TrickyState*)state)->statusFlags)->bit7 = 1;
}

static inline int trickyGetState(int obj)
{
    return *(int*)&((GameObject*)obj)->extra;
}

int tricky_SeqFn(int obj, int unused, ObjAnimUpdateState* animUpdate)
{
    int i;
    int state;
    int slot;
    int j;
    int k;
    u8* p;
    int setup;
    u8 blockFlags[120];

    state = trickyGetState(obj);
    if ((((TrickyState*)state)->stateFlags & 0x200) == 0)
    {
        ObjHits_DisableObject((GameObject*)obj);
        Sfx_StopObjectChannel(obj, 0x7f);
        if ((((TrickyState*)state)->stateFlags & TRICKY_STATE_FLAG_FLAME_CHILDREN_ACTIVE) != 0)
        {
            ((TrickyState*)state)->stateFlags =
                ((TrickyState*)state)->stateFlags & ~(u64)TRICKY_STATE_FLAG_FLAME_CHILDREN_ACTIVE;
            ((TrickyState*)state)->stateFlags =
                ((TrickyState*)state)->stateFlags | TRICKY_STATE_FLAG_FLAME_CHILDREN_CLEANUP;
            for (k = 0, slot = state; k < 7; slot = slot + 4, k = k + 1)
            {
                objSetAnimSpeedTo1((GameObject*)*(int*)(slot + 0x700));
            }
            Sfx_RemoveLoopedObjectSound(obj, SFXTRIG_trpopn_c);
            slot = *(int*)&((GameObject*)obj)->extra;
            if ((((TrickyByteFlags*)(slot + 0x58))->bit6 == 0) &&
                (((((GameObject*)obj)->anim.currentMove >= 0x30 || (((GameObject*)obj)->anim.currentMove < 0x29)) &&
                  (Sfx_IsPlayingFromObjectChannel(obj, 0x10) == 0))))
            {
                objAudioFn_800393f8((GameObject*)obj, &((TrickyState*)slot)->soundState, 0x29d, 0, 0xffffffff, 0);
            }
        }
        Sfx_RemoveLoopedObjectSound(obj, SFXTRIG_trwhin1);
        ((TrickyState*)state)->stateFlags = ((TrickyState*)state)->stateFlags | 0x200;
        if ((animUpdate->hitVolumePair & 3) == 0)
        {
            ((TrickyState*)state)->stateFlags = ((TrickyState*)state)->stateFlags | 0x4000;
        }
        if (((TrickyByteFlags*)&((TrickyState*)state)->flags82E)->bit5 == 0)
        {
            ObjModel_ClearBlendChannels(Obj_GetActiveModel((GameObject*)obj));
            ((TrickyByteFlags*)&((TrickyState*)state)->flags82E)->bit6 = 0;
        }
    }
    for (i = 0; i < animUpdate->eventCount; i++)
    {
        switch (animUpdate->eventIds[i])
        {
        case 1:
            if ((((TrickyState*)state)->stateFlags & TRICKY_STATE_FLAG_FLAME_CHILDREN_ACTIVE) != 0)
            {
                ((TrickyState*)state)->stateFlags &= ~(u64)TRICKY_STATE_FLAG_FLAME_CHILDREN_ACTIVE;
                ((TrickyState*)state)->stateFlags |= TRICKY_STATE_FLAG_FLAME_CHILDREN_CLEANUP;
                for (j = 0, slot = state; j < 7; slot = slot + 4, j = j + 1)
                {
                    objSetAnimSpeedTo1((GameObject*)*(int*)(slot + 0x700));
                }
                Sfx_RemoveLoopedObjectSound(obj, SFXTRIG_trpopn_c);
                slot = *(int*)&((GameObject*)obj)->extra;
                if ((((TrickyByteFlags*)(slot + 0x58))->bit6 == 0) &&
                    (((((GameObject*)obj)->anim.currentMove >= 0x30 || (((GameObject*)obj)->anim.currentMove < 0x29)) &&
                      (Sfx_IsPlayingFromObjectChannel(obj, 0x10) == 0))))
                {
                    objAudioFn_800393f8((GameObject*)obj, &((TrickyState*)slot)->soundState, 0x29d, 0, 0xffffffff, 0);
                }
            }
            else if (Obj_IsLoadingLocked())
            {
                ((TrickyState*)state)->stateFlags =
                    ((TrickyState*)state)->stateFlags | TRICKY_STATE_FLAG_FLAME_CHILDREN_ACTIVE;
                for (k = 0, p = (u8*)state; k < 7; p += 4, k = k + 1)
                {
                    setup = (int)Obj_AllocObjectSetup(0x24, TRICKY_CHILD_OBJ_FLAMEBLAST);
                    *(u8*)(setup + 4) = 2;
                    *(u8*)(setup + 5) = 1;
                    *(s16*)(setup + 0x1a) = k;
                    *(int*)(p + 0x700) = (int)Obj_SetupObject((ObjPlacement*)setup, 5, ((GameObject*)obj)->anim.mapEventSlot, -1,
                                                              ((GameObject*)obj)->anim.parent);
                }
                Sfx_PlayFromObject(obj, SFXTRIG_en_cvdrip1c_3db);
                Sfx_AddLoopedObjectSound(obj, SFXTRIG_trpopn_c);
            }
            break;
        case 2:
            mainSetBits(GAMEBIT_Tricky_LoadBadge, 1);
            if ((mainGetBit(GAMEBIT_Tricky_LoadBadge) != 0 && *(void**)&((TrickyState*)state)->spawnedChild == NULL) &&
                Obj_IsLoadingLocked())
            {
                mapBlockFn_80059c2c(blockFlags);
                if (blockFlags[0xd] != 0)
                {
                    setup = (int)Obj_AllocObjectSetup(0x20, TRICKY_CHILD_OBJ_BADGE_A);
                }
                else
                {
                    setup = (int)Obj_AllocObjectSetup(0x20, TRICKY_CHILD_OBJ_BADGE_B);
                }
                *(int*)&((TrickyState*)state)->spawnedChild =
                    (int)Obj_SetupObject((ObjPlacement*)setup, 4, -1, -1, ((GameObject*)obj)->anim.parent);
                ObjLink_AttachChild((GameObject*)obj, ((TrickyState*)state)->spawnedChild, 3);
            }
            break;
        case 3:
            **(u8**)&((TrickyState*)state)->progressPtr = ((TrickyState*)state)->progressValue;
            break;
        case 0x2b:
            ((GameObject*)obj)->anim.modelState->flags &= ~(u64)OBJ_MODEL_STATE_SHADOW_VISIBLE;
            break;
        case 0x2c:
            ((GameObject*)obj)->anim.modelState->flags |= OBJ_MODEL_STATE_SHADOW_VISIBLE;
            break;
        }
    }
    objAnimFreeChildren(obj, state, (GameObject**)(state + 0x7a8)); /* raw: arrow form shifts bytes */
    objAnimFreeChildren(obj, state, (GameObject**)(state + 0x7b0)); /* raw: arrow form shifts bytes */
    objAnimFreeChildren(obj, state, (GameObject**)&((TrickyState*)state)->child);
    tricky_updateModelVariantFade(obj, state);
    Tricky_updateBlendChannelWeight(obj, (u8*)state);
    objAudioFn_8006ef38((GameObject*)obj, &animUpdate->animEvents, 1, (void*)(state + 0x7d8),
                        (void*)(state + 0xf8), lbl_803E23E8,
                        *(f32*)&lbl_803E23E8);
    if ((((TrickyState*)state)->stateFlags & 1) != 0)
    {
        animUpdate->hitVolumePair &= ~0x40;
        characterDoEyeAnims((GameObject*)obj, (void*)(state + 0x378));
        return (*gObjectTriggerInterface)->func20((GameObject*)obj, animUpdate, 1, 0xf, 0x1e, 0, 0);
    }
    return 0;
}

void Tricky_requestRecall(int* obj)
{
    register TrickyState* state = ((GameObject*)obj)->extra;
    if (mainGetBit(GAMEBIT_Tricky_Usable))
    {
        state->stateFlags |= 0x10000LL;
    }
}

int Tricky_func13(int* obj)
{
    u8 mode = ((TrickyState*)((GameObject*)obj)->extra)->stateIndex;
    if (mode == 8 || mode == 0xe)
        return 1;
    return 0;
}

int Tricky_isPlayingBall(int* obj)
{
    u8 mode;
    int result;
    mode = ((TrickyState*)((GameObject*)obj)->extra)->stateIndex;
    switch (mode)
    {
    case 5:
        result = 1;
        break;
    default:
        result = 0;
        break;
    }
    return result;
}

int Tricky_requestMoveToObject(int* obj, int targetObj)
{
    GameObject* tricky = (GameObject*)obj;
    TrickyState* state = tricky->extra;
    u8* target = (u8*)targetObj;
    s32 objBlocked = tricky->objectFlags & TRICKY_OBJFLAG_PARENT_SLACK;

    if (objBlocked != 0)
    {
        return 0;
    }
    if ((state->stateFlags & 0x10) == 0)
    {
        state->followObj = (GameObject*)target;
        if ((u8*)state->targetPosPtr != target + 0x18)
        {
            state->targetPosPtr = (f32*)(target + 0x18);
            {
                u32 m;
                u32 f2 = state->stateFlags;
                m = ~TRICKY_STATE_TARGET_DIRTY_FLAG;
                state->stateFlags = f2 & m;
            }
            state->linkedWalkGroup = 0;
        }
        state->substate = 0;
        state->stateIndex = 10;
    }
    else
    {
        state->pendingFollowRequest = 1;
        state->pendingFollowObj = (GameObject*)target;
        state->stateFlags |= 0x10000LL;
    }
    return 1;
}

void Tricky_commandPlayBall(int* obj, int commandEnabled, int targetObj)
{
    register TrickyState* state = ((GameObject*)obj)->extra;

    if (commandEnabled != 0)
    {
        if (state->stateIndex == 5)
        {
            if (state->substate != 0)
            {
                state->followObj = (GameObject*)targetObj;
            }
        }
        else
        {
            u32 busy = state->stateFlags & 0x10;
            void* nextTarget;
            if (busy != 0)
            {
                return;
            }
            state->scratch700.i = (int)Objfsa_FindNearestEnabledCurveType24((void*)(targetObj + 0x18), -1, 3);
            state->scratch710.f = (f32)(int)randomGetRange(0x168, 0x28);
            state->stateIndex = 5;
            state->followObj = (GameObject*)targetObj;
            nextTarget = (void*)(state->scratch700.i + 8);
            if ((void*)state->targetPosPtr != nextTarget)
            {
                state->targetPosPtr = (f32*)nextTarget;
                {
                    u32 m;
                    u32 f2 = state->stateFlags;
                    m = ~TRICKY_STATE_TARGET_DIRTY_FLAG;
                    state->stateFlags = f2 & m;
                }
                state->linkedWalkGroup = 0;
            }
            state->substate = 0;
        }
    }
    else
    {
        *(u32*)&state->stateFlags |= 0x10000LL;
    }
}

u8 Tricky_getEnergyMax(int* obj)
{
    return ((TrickyState*)((GameObject*)obj)->extra)->progressPtr[1];
}
u8 Tricky_getEnergy(int* obj)
{
    return ((TrickyState*)((GameObject*)obj)->extra)->progressPtr[0];
}

void sideCommandEnable(GameObject* obj, GameObject* targetObj, int commandKind, int commandType)
{
    int remaining;
    u8* commandEntry;
    u32 count;
    int commandIndex;
    TrickyState* state;

    state = obj->extra;
    if (state->commandCount == ARRAY_COUNT(state->commands))
    {
        trickyReportError(sSidekickCommandDebugTextBlock);
        return;
    }
    state->commandRequestBits = (u8)(state->commandRequestBits | (1 << commandType));
    commandIndex = 0;
    commandEntry = (u8*)state;
    count = state->commandCount;
    for (remaining = count; remaining > 0; remaining--)
    {
        if (*(GameObject**)(commandEntry + offsetof(TrickyState, commands)) == targetObj)
        {
            state->commands[commandIndex].ttl = 3;
            return;
        }
        commandEntry += sizeof(TrickyCommand);
        commandIndex++;
    }
    state->commands[count].targetObj = targetObj;
    state->commands[state->commandCount].kind = commandKind;
    state->commands[state->commandCount].type = commandType;
    state->commands[state->commandCount].ttl = 3;
    state->commandCount++;
}

int Tricky_getCurrentCommandType(int* obj, int* out)
{
    *out = ((TrickyState*)((GameObject*)obj)->extra)->commandPhase;
    return 1;
}

int Tricky_updateSideCommandPrompts(int obj)
{
    int state;
    u32 commandMask;
    char cmdByte;
    u16 promptId;
    u8 cond;
    u8 promptA;
    u8 promptB;
    u8 promptC;
    u32 bitVal;
    int ref;
    int refB;
    int refC;
    u16* setup;
    u32 spawnedObj;
    u8 i;
    char flagsB[4];
    char flagsA[4];
    u32 promptTable[4];

    state = *(int*)&((GameObject*)obj)->extra;
    cond = false;
    promptA = false;
    promptB = false;
    promptC = false;
    promptTable[0] = lbl_803E23C8;
    bitVal = mainGetBit(GAMEBIT_Tricky_Usable);
    if (bitVal != 0)
    {
        if ((((TrickyState*)state)->stateFlags & 0x10) != 0)
        {
            ((TrickyState*)state)->commandRequestBits = 0;
        }
        commandMask = ((TrickyState*)state)->commandRequestBits | 9;
        if (((((TrickyState*)state)->stateIndex == 8) || (((TrickyState*)state)->stateIndex == 0xd)) ||
            ((((TrickyState*)state)->stateIndex == 0xe && (((TrickyState*)state)->substate == 1))))
        {
            commandMask |= 0x10;
            promptA = true;
        }
        else
        {
            if (trickyFindNearestUsableBaddie(((TrickyState*)state)->playerObj, lbl_803E2524, 1) !=
                NULL)
            {
                promptA = true;
                promptC = true;
            }
        }
        if (((TrickyState*)state)->commandRequestBits != 0)
        {
            for (i = 0; i < ((TrickyState*)state)->commandCount; i++)
            {
                ref = state + i * 8;
                cmdByte = *(char*)(ref + 0x74c);
                if (cmdByte == '\0')
                {
                    if (((GameObject*)*(int*)(ref + 0x748))->anim.seqId == TRICKY_OBJ_BLUE_MUSHROOM)
                    {
                        promptB = true;
                    }
                    promptA = true;
                }
                else if (cmdByte == '\x01')
                {
                    cond = true;
                }
            }
        }
        if (((((TrickyState*)state)->stateFlags & 0x10) == 0) &&
            (bitVal = mainGetBit(GAMEBIT_ITEM_TrickyBall_Usable), bitVal != 0))
        {
            ref = (int)Obj_GetPlayerObject();
            ref = fn_80296240((GameObject*)(ref));
            if ((ref != 0) && (bitVal = mainGetBit(GAMEBIT_NoBallsAllowed), bitVal == 0))
            {
                if (playerGetFlags3F0Bit5(((TrickyState*)state)->playerObj) == 0)
                {
                    commandMask |= 0x20;
                }
            }
        }
        if (mainGetBit(GAMEBIT_ITEM_TrickyCall_Got) == 0)
        {
            commandMask &= ~1;
        }
        if (mainGetBit(0x9e) == 0)
        {
            commandMask &= ~4;
        }
        if (mainGetBit(GAMEBIT_ITEM_TrickyFlame_Got) == 0)
        {
            commandMask &= ~0x10;
        }
        ((TrickyState*)state)->commandRequestBits = 0;
        if ((cond) && ((((TrickyState*)state)->stateFlags & 0x200) == 0))
        {
            *(float*)(state + 0x7b4) = lbl_803E24F8;
            if ((((TrickyState*)state)->childB == NULL) && (Obj_IsLoadingLocked() != 0))
            {
                bitVal = randomGetRange(0, 1);
                promptId = *(u16*)((int)promptTable + bitVal * 2);
                ref = *(int*)&((GameObject*)obj)->extra;
                if ((((TrickyByteFlags*)&((TrickyState*)ref)->statusFlags)->bit6 == 0) && (((((GameObject*)obj)->anim.currentMove >= 0x30 ||
                                                                (((GameObject*)obj)->anim.currentMove < 0x29)) &&
                                                               !Sfx_IsPlayingFromObjectChannel(obj, 0x10))))
                {
                    objAudioFn_800393f8((GameObject*)obj, &((TrickyState*)ref)->soundState, promptId, 0x500,
                                       0xffffffff, 0);
                }
                setup = (u16*)Obj_AllocObjectSetup(0x20, TRICKY_CHILD_OBJ_QUEST);
                flagsB[0] = -1;
                flagsB[1] = -1;
                flagsB[2] = -1;
                if (((TrickyState*)state)->childA != NULL)
                {
                    flagsB[*(u8*)(state + 0x7bc) >> 6 & 3] = '\x01';
                }
                if (((TrickyState*)state)->childB != NULL)
                {
                    flagsB[*(u8*)(state + 0x7bc) >> 4 & 3] = '\x01';
                }
                if (((TrickyState*)state)->child != NULL)
                {
                    flagsB[*(u8*)(state + 0x7bc) >> 2 & 3] = '\x01';
                }
                if (flagsB[0] == -1)
                {
                    bitVal = 0;
                }
                else if (flagsB[1] == -1)
                {
                    bitVal = 1;
                }
                else if (flagsB[2] == -1)
                {
                    bitVal = 2;
                }
                else if (flagsB[3] == -1)
                {
                    bitVal = 3;
                }
                else
                {
                    bitVal = 0xffffffff;
                }
                ((TrickyPackedSlots*)(state + 0x7bc))->promptBSlot = bitVal;
                spawnedObj = (int)Obj_SetupObject((ObjPlacement*)setup, 4, -1, 0xffffffff, ((GameObject*)obj)->anim.parent);
                *(u32*)(state + 0x7b0) = spawnedObj; /* raw: arrow form shifts bytes */
                ObjLink_AttachChild((GameObject*)obj, ((TrickyState*)state)->childB, *(u8*)(state + 0x7bc) >> 4 & 3);
            }
        }
        else if (((TrickyState*)state)->childB != NULL)
        {
            *(float*)(state + 0x7b4) = *(float*)(state + 0x7b4) - timeDelta;
            if (*(float*)(state + 0x7b4) <= lbl_803E23DC)
            {
                objAnimFreeChildren(obj, state, (GameObject**)(state + 0x7b0)); /* raw: arrow form shifts bytes */
            }
        }
        if ((promptA) && ((((TrickyState*)state)->stateFlags & 0x200) == 0))
        {
            *(float*)(state + 0x7ac) = lbl_803E24F8;
            if ((((TrickyState*)state)->childA == NULL) && (Obj_IsLoadingLocked() != 0))
            {
                if (randomGetRange(0, 3) == 0)
                {
                    if (promptB)
                    {
                        refB = *(int*)&((GameObject*)obj)->extra;
                        if ((((TrickyByteFlags*)&((TrickyState*)refB)->statusFlags)->bit6 == 0) &&
                            (((((GameObject*)obj)->anim.currentMove >= 0x30 ||
                               (((GameObject*)obj)->anim.currentMove < 0x29)) &&
                              !Sfx_IsPlayingFromObjectChannel(obj, 0x10))))
                        {
                            objAudioFn_800393f8((GameObject*)obj, &((TrickyState*)refB)->soundState, 0x359,
                                               0x500, 0xffffffff, 0);
                        }
                    }
                    else if ((((promptC) &&
                               (refC = *(int*)&((GameObject*)obj)->extra, ((TrickyByteFlags*)&((TrickyState*)refC)->statusFlags)->bit6 == 0)) &&
                              ((((GameObject*)obj)->anim.currentMove >= 0x30 ||
                                (((GameObject*)obj)->anim.currentMove < 0x29)))) &&
                             !Sfx_IsPlayingFromObjectChannel(obj, 0x10))
                    {
                        objAudioFn_800393f8((GameObject*)obj, &((TrickyState*)refC)->soundState, 0x358, 0x500,
                                           0xffffffff, 0);
                    }
                }
                setup = (u16*)Obj_AllocObjectSetup(0x20, TRICKY_CHILD_OBJ_EXCLAMATION);
                flagsA[0] = -1;
                flagsA[1] = -1;
                flagsA[2] = -1;
                if (((TrickyState*)state)->childA != NULL)
                {
                    flagsA[*(u8*)(state + 0x7bc) >> 6 & 3] = '\x01';
                }
                if (((TrickyState*)state)->childB != NULL)
                {
                    flagsA[*(u8*)(state + 0x7bc) >> 4 & 3] = '\x01';
                }
                if (((TrickyState*)state)->child != NULL)
                {
                    flagsA[*(u8*)(state + 0x7bc) >> 2 & 3] = '\x01';
                }
                if (flagsA[0] == -1)
                {
                    bitVal = 0;
                }
                else if (flagsA[1] == -1)
                {
                    bitVal = 1;
                }
                else if (flagsA[2] == -1)
                {
                    bitVal = 2;
                }
                else if (flagsA[3] == -1)
                {
                    bitVal = 3;
                }
                else
                {
                    bitVal = 0xffffffff;
                }
                ((TrickyPackedSlots*)(state + 0x7bc))->promptASlot = bitVal;
                spawnedObj = (int)Obj_SetupObject((ObjPlacement*)setup, 4, -1, 0xffffffff, ((GameObject*)obj)->anim.parent);
                *(u32*)(state + 0x7a8) = spawnedObj; /* raw: arrow form shifts bytes */
                ObjLink_AttachChild((GameObject*)obj, ((TrickyState*)state)->childA, *(u8*)(state + 0x7bc) >> 6 & 3);
            }
        }
        else if (((TrickyState*)state)->childA != NULL)
        {
            *(float*)(state + 0x7ac) = *(float*)(state + 0x7ac) - timeDelta;
            if (*(float*)(state + 0x7ac) <= lbl_803E23DC)
            {
                objAnimFreeChildren(obj, state, (GameObject**)(state + 0x7a8)); /* raw: arrow form shifts bytes */
            }
        }
        return commandMask;
    }
    return -1;
}

int Tricky_getAvailableCommands(void)
{
    int r = 0;
    if (mainGetBit(GAMEBIT_Tricky_Usable) != 0)
    {
        r = TRICKY_ABILITY_FIND_SECRET | TRICKY_ABILITY_STAY;
        if (mainGetBit(GAMEBIT_ITEM_TrickyCall_Got) != 0)
            r |= TRICKY_ABILITY_CALL;
        if (mainGetBit(GAMEBIT_ITEM_TrickyBall_Bought) != 0)
            r |= TRICKY_ABILITY_THROW_BALL;
        if (mainGetBit(GAMEBIT_ITEM_TrickyFlame_Got) != 0)
            r |= TRICKY_ABILITY_FLAME;
    }
    return r;
}

int Tricky_getExtraSize(void)
{
    return 0x83c;
}

void Tricky_free(int obj, int shouldKeepFlameChildren)
{
    int i;
    int childSlot;
    int state;
    u32 objId = obj;

    state = *(int*)&((GameObject*)obj)->extra;
    freeAndNull((void**)&((TrickyState*)state)->pathSearches[0].nodes);
    freeAndNull((void**)&((TrickyState*)state)->pathSearches[1].nodes);
    freeAndNull((void**)&((TrickyState*)state)->pathSearches[2].nodes);
    freeAndNull((void**)&((TrickyState*)state)->pathSearches[3].nodes);
    freeAndNull((void**)&((TrickyState*)state)->pathSearches[4].nodes);
    freeAndNull((void**)&((TrickyState*)state)->pathSearches[5].nodes);
    freeAndNull((void**)&((TrickyState*)state)->pathSearches[6].nodes);
    freeAndNull((void**)&((TrickyState*)state)->pathSearches[7].nodes);
    freeAndNull((void**)&((TrickyState*)state)->pathSearches[8].nodes);
    ObjGroup_RemoveObject(obj, TRICKY_OBJGROUP);
    (*gExpgfxInterface)->freeSource(objId);
    if ((shouldKeepFlameChildren == 0) &&
        ((((TrickyState*)state)->stateFlags & TRICKY_STATE_FLAG_FLAME_CHILDREN_ACTIVE) != 0))
    {
        ((TrickyState*)state)->stateFlags =
            ((TrickyState*)state)->stateFlags & ~(u64)TRICKY_STATE_FLAG_FLAME_CHILDREN_ACTIVE;
        ((TrickyState*)state)->stateFlags =
            ((TrickyState*)state)->stateFlags | TRICKY_STATE_FLAG_FLAME_CHILDREN_CLEANUP;
        i = 0;
        childSlot = state;
        do
        {
            objSetAnimSpeedTo1((GameObject*)((TrickyDestroyState*)childSlot)->childObj);
            childSlot = childSlot + 4;
            i = i + 1;
        } while (i < 7);
        Sfx_RemoveLoopedObjectSound(objId, SFXTRIG_trpopn_c);
        childSlot = *(int*)&((GameObject*)obj)->extra;
        if ((((TrickyByteFlags*)&((TrickyState*)childSlot)->statusFlags)->bit6 == 0) &&
            (((((GameObject*)obj)->anim.currentMove >= 0x30 || (((GameObject*)obj)->anim.currentMove < 0x29)) &&
              (Sfx_IsPlayingFromObjectChannel(obj, 0x10) == 0))))
        {
            objAudioFn_800393f8((GameObject*)obj, &((TrickyState*)childSlot)->soundState, 0x29d, 0, 0xffffffff, 0);
        }
    }
    doNothing_onTrickyFree();
    objAnimFreeChildren(obj, state, (GameObject**)(state + 0x7a8)); /* raw: arrow form shifts bytes */
    objAnimFreeChildren(obj, state, (GameObject**)(state + 0x7b0)); /* raw: arrow form shifts bytes */
    objAnimFreeChildren(obj, state, (GameObject**)&((TrickyState*)state)->child);
    if (*(void**)&((TrickyState*)state)->spawnedChild != NULL)
    {
        ObjLink_DetachChild((GameObject*)obj, ((TrickyState*)state)->spawnedChild);
        Obj_FreeObject((GameObject*)((TrickyState*)state)->spawnedChild);
    }
    if (((((TrickyState*)state)->statusFlags >> 7 & 1) != 0u) && (gTrickyHelperObject != 0))
    {
        Obj_FreeObject((GameObject*)gTrickyHelperObject);
        gTrickyHelperObject = 0;
    }
    return;
}

void Tricky_render(GameObject* obj, int p2, int p3, int p4, int p5, char doRender)
{
    u8 mode;
    int i;
    int pathState;
    int pathPoint;
    s16* pathInfo;
    int state;

    if (doRender != '\0')
    {
        state = *(int*)&(obj)->extra;
        objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, lbl_803E23E8);
        pathState = *(int*)&(obj)->extra;
        i = 0;
        pathPoint = pathState;
        do
        {
            ObjPath_GetPointWorldPosition(obj, i + 4, (float*)(pathPoint + 0x3d8), (float*)(pathPoint + 0x3dc),
                                          (float*)(pathPoint + 0x3e0), 0);
            pathPoint = pathPoint + 0xc;
            i = i + 1;
        } while (i < 4);
        ObjPath_GetPointWorldPosition(obj, 8, (float*)(pathState + 0x408), (float*)(pathState + 0x40c),
                                      (float*)(pathState + 0x410), 0);
        pathInfo = objModelGetVecFn_800395d8(obj, 0);
        *(s16*)(pathState + 0x414) = pathInfo[1];
        if ((((TrickyState*)state)->stateFlags & 0x10) != 0)
        {
            switch (((TrickyState*)state)->stateIndex)
            {
            case 2:
                skeetla_spawnLinkedSparks((u8*)obj);
                break;
            case 3:
                if (((TrickyState*)state)->substate == 4)
                {
                    skeetla_spawnLinkedSparks((u8*)obj);
                }
                break;
            }
            if ((((((TrickyState*)state)->stateFlags & 0x200) == 0) && (((TrickyState*)state)->stateIndex == 0xb)) &&
                (((TrickyState*)state)->substate >= 3))
            {
                if (((TrickyState*)state)->substate != 3)
                {
                    ((TrickyState*)state)->scratch700.obj->anim.localPosX = ((TrickyState*)state)->renderPosX;
                    ((TrickyState*)state)->scratch700.obj->anim.localPosY = ((TrickyState*)state)->renderPosY;
                    ((TrickyState*)state)->scratch700.obj->anim.localPosZ = ((TrickyState*)state)->renderPosZ;
                }
                objRenderModelAndHitVolumes(((TrickyState*)state)->scratch700.obj, p2, p3, p4, p5, lbl_803E23E8);
            }
        }
        Tricky_emitQueuedPathParticles((u8*)obj, (u8*)state);
        ObjPath_GetPointWorldPositionArray(obj, 4, 4, (float*)((TrickyState*)state)->footPoints);
        ((TrickyState*)state)->particleTimer = ((TrickyState*)state)->particleTimer - timeDelta;
        if (((TrickyState*)state)->particleTimer > lbl_803E23DC)
        {
            objParticleFn_80099d84((GameObject*)obj, lbl_803E253C, 6, lbl_803E23E8, 0);
        }
    }
    return;
}

void Tricky_hitDetect(GameObject* obj)
{
    f32 dy;
    f32 y;
    GameObject** objects;
    int i;
    void* firepipeObj;
    int state;
    f32 height;
    int count[2];

    state = *(int*)&obj->extra;
    y = obj->anim.localPosY;
    dy = (y - obj->anim.previousLocalPosY >= lbl_803E23DC) ? y - obj->anim.previousLocalPosY
                                                           : -(y - obj->anim.previousLocalPosY);
    if (lbl_803E23E8 == dy)
    {
        if (y == obj->anim.worldPosY)
        {
            ((TrickyStatusFlags58*)&((TrickyState*)state)->statusFlags)->heightTracking = 1;
            *(s32*)&((TrickyState*)state)->heightTrackObjId = -1;
            ((TrickyState*)state)->trackedHeight = lbl_803E23DC;
        }
    }
    else
    {
        firepipeObj = ObjList_FindObjectById(TRICKY_HEIGHT_TRACK_FIREPIPE_OBJECT_ID);
        if ((firepipeObj != 0) && (getXZDistance(&obj->anim.worldPosX, (f32*)((int)firepipeObj + 0x18)) < lbl_803E2540))
        {
            ((TrickyStatusFlags58*)&((TrickyState*)state)->statusFlags)->heightTracking = 1;
            ((TrickyState*)state)->heightTrackObjId = TRICKY_HEIGHT_TRACK_FIREPIPE_OBJECT_ID;
            ((TrickyState*)state)->trackedHeight = lbl_803E23DC;
        }
    }
    if ((((TrickyState*)state)->statusFlags >> 5 & 1) != 0u)
    {
        {
            GameObject** t = (GameObject**)ObjGroup_GetObjects(XYZ_ANIMATOR_OBJECT_GROUP, count);
            i = 0;
            objects = t;
        }
        for (; i < count[0]; i++)
        {
            height = XyzAnimator_getCoordinate(*objects, XYZ_ANIMATOR_COORD_WORLD_Y);
            if (*(s32*)&((TrickyState*)state)->heightTrackObjId == -1)
            {
                dy = (height - obj->anim.localPosY >= lbl_803E23DC) ? height - obj->anim.localPosY
                                                                    : -(height - obj->anim.localPosY);
                if (dy < lbl_803E24B8)
                {
                    ((TrickyState*)state)->heightTrackObjId = (*objects)->anim.placement->mapId;
                }
            }
            if (((TrickyState*)state)->heightTrackObjId == (u32)(*objects)->anim.placement->mapId)
            {
                if ((((TrickyState*)state)->trackedHeight != lbl_803E23DC) &&
                    (((TrickyState*)state)->trackedHeight == height))
                {
                    ((TrickyStatusFlags58*)&((TrickyState*)state)->statusFlags)->heightTracking = 0;
                }
                else
                {
                    obj->anim.localPosY = height;
                    ((TrickyState*)state)->trackedHeight = height;
                }
                break;
            }
            objects = objects + 1;
        }
        if (i == count[0])
        {
            ((TrickyStatusFlags58*)&((TrickyState*)state)->statusFlags)->heightTracking = 0;
        }
    }
    return;
}

/* Tricky sidekick command state machine and per-frame update. */
#define TRICKY_RESET_COMMAND(state)                                                                                    \
    *(u8*)((state) + 8) = 1;                                                                                           \
    *(u8*)((state) + 0xa) = 0;                                                                                         \
    z = lbl_803E23DC;                                                                                                  \
    *(f32*)((state) + 0x71c) = z;                                                                                      \
    *(f32*)((state) + 0x720) = z;                                                                                      \
    *(u32*)((state) + 0x54) = *(u32*)((state) + 0x54) & (u64)~0x10U;                                                  \
    *(u32*)((state) + 0x54) = *(u32*)((state) + 0x54) & (u64)~0x10000U;                                               \
    *(u32*)((state) + 0x54) = *(u32*)((state) + 0x54) & (u64)~0x20000U;                                               \
    *(u32*)((state) + 0x54) = *(u32*)((state) + 0x54) & (u64)~0x40000U;                                               \
    ((TrickyState*)(state))->commandPhase = -1

#define TRICKY_VOICE(obj, sfx, vol)                                                                                    \
    {                                                                                                                  \
        st = ((GameObject*)obj)->extra;                                                                                \
        if (((TrickyByteFlags*)&st->statusFlags)->bit6 == 0)                                                           \
        {                                                                                                              \
            if (((GameObject*)obj)->anim.currentMove >= 0x30 || ((GameObject*)obj)->anim.currentMove < 0x29)           \
            {                                                                                                          \
                if (Sfx_IsPlayingFromObjectChannel((obj), 0x10) == 0)                                                  \
                {                                                                                                      \
                    objAudioFn_800393f8((GameObject*)(obj), &st->soundState, (sfx), (vol), 0xffffffff, 0);             \
                }                                                                                                      \
            }                                                                                                          \
        }                                                                                                              \
    }

#define TRICKY_SPAWN_FOOD_BUBBLE(obj, state)                                                                                \
    if (*(void**)((state) + 0x7b8) == NULL)                                                                            \
    {                                                                                                                  \
        int setup_;                                                                                                    \
        s8 used_[4];                                                                                                   \
        int slot_;                                                                                                     \
        setup_ = (int)Obj_AllocObjectSetup(0x20, TRICKY_CHILD_OBJ_FOOD);                                                               \
        used_[0] = -1;                                                                                                 \
        used_[1] = -1;                                                                                                 \
        used_[2] = -1;                                                                                                 \
        if (*(void**)((state) + 0x7a8) != NULL)                                                                        \
        {                                                                                                              \
            used_[((TrickyPackedSlots*)((state) + 0x7bc))->promptASlot] = 1;                                                    \
        }                                                                                                              \
        if (*(void**)((state) + 0x7b0) != NULL)                                                                        \
        {                                                                                                              \
            used_[((TrickyPackedSlots*)((state) + 0x7bc))->promptBSlot] = 1;                                                    \
        }                                                                                                              \
        if (*(void**)((state) + 0x7b8) != NULL)                                                                        \
        {                                                                                                              \
            used_[((TrickyPackedSlots*)((state) + 0x7bc))->zzzSlot] = 1;                                                    \
        }                                                                                                              \
        if (used_[0] == -1)                                                                                            \
        {                                                                                                              \
            slot_ = 0;                                                                                                 \
        }                                                                                                              \
        else if (used_[1] == -1)                                                                                       \
        {                                                                                                              \
            slot_ = 1;                                                                                                 \
        }                                                                                                              \
        else if (used_[2] == -1)                                                                                       \
        {                                                                                                              \
            slot_ = 2;                                                                                                 \
        }                                                                                                              \
        else if (used_[3] == -1)                                                                                       \
        {                                                                                                              \
            slot_ = 3;                                                                                                 \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            slot_ = -1;                                                                                                \
        }                                                                                                              \
        ((TrickyPackedSlots*)((state) + 0x7bc))->zzzSlot = slot_;                                                           \
        *(int*)((state) + 0x7b8) = (int)Obj_SetupObject((ObjPlacement*)setup_, 4, -1, -1, *(void**)((obj) + 0x30));    \
        ObjLink_AttachChild((GameObject*)(obj), *(GameObject**)((state) + 0x7b8),                                    \
                            ((TrickyPackedSlots*)((state) + 0x7bc))->zzzSlot);                                             \
        z = lbl_803E23DC;                                                                                              \
        *(f32*)((state) + 0x7c0) = z;                                                                                  \
        *(f32*)((state) + 0x7c4) = z;                                                                                  \
        *(f32*)((state) + 0x7c8) = z;                                                                                  \
    }

void Tricky_update(int obj)
{
    char* base;
    int state;
    TrickyState* trickyState;
    int found;
    int sfxId;
    TrickyState* st;
    struct
    {
        int index;
    } childLoop;
    int i;
    int ref;
    int setup;
    int count;
    u32 flags;
    GameObject* step;
    int played;
    int talking;
    f32* target;
    f32 z;
    u8 blockFlags[120];
    TrickyItemIdList cmdQuery;
    TrickySfxPair pair;

    base = lbl_8031D2E8;
    state = *(int*)&((GameObject*)obj)->extra;
    trickyState = (TrickyState*)state;
    found = 0;
    cmdQuery = gTrickyCmdQueryInit;
    pair = lbl_803E23C4;
    walkgroupFindExitPointFn_800dc398();
    if (mainGetBit(GAMEBIT_Tricky_LoadBadge) != 0 && *(void**)&trickyState->spawnedChild == NULL &&
        Obj_IsLoadingLocked())
    {
        mapBlockFn_80059c2c(blockFlags);
        if (blockFlags[0xd] != 0)
        {
            setup = (int)Obj_AllocObjectSetup(0x20, TRICKY_CHILD_OBJ_BADGE_A);
        }
        else
        {
            setup = (int)Obj_AllocObjectSetup(0x20, TRICKY_CHILD_OBJ_BADGE_B);
        }
        *(int*)&trickyState->spawnedChild =
            (int)Obj_SetupObject((ObjPlacement*)setup, 4, -1, -1, ((GameObject*)obj)->anim.parent);
        ObjLink_AttachChild((GameObject*)obj, trickyState->spawnedChild, 3);
    }
    if ((trickyState->stateFlags & 0x40000000) != 0)
    {
        u8* voiceCursor = *(u8**)state;

        if (*voiceCursor == *(voiceCursor + 1))
        {
            TRICKY_VOICE(obj, 0x364, 0x500);
        }
        else
        {
            TRICKY_VOICE(obj, 0x363, 0x500);
        }
        trickyState->stateFlags &= ~0x40000000LL;
    }
    {
        int flagsByte = trickyState->flags358;
        trickyDebugPrint(base + 0x894, flagsByte & 1, flagsByte & 2, flagsByte & 4, flagsByte & 8,
                         flagsByte & 0x10, flagsByte & 0x20, flagsByte & 0x40, flagsByte & 0x80);
    }
    {
        u8* debugCursor = *(u8**)state;

        trickyDebugPrint(base + 0x8b4, *debugCursor, *(debugCursor + 1));
    }
    if ((trickyState->stateFlags & 0x200) != 0)
    {
        ObjHits_EnableObject((GameObject*)obj);
        if ((trickyState->stateFlags & 0x4000) == 0)
        {
            TRICKY_RESET_COMMAND(state);
            trickyState->followPhase = 0;
            trickyState->prevSpeed = z;
            trickyState->speed = z;
            trickyState->homePosX = ((GameObject*)obj)->anim.worldPosX;
            trickyState->homePosY = ((GameObject*)obj)->anim.worldPosY;
            trickyState->homePosZ = ((GameObject*)obj)->anim.worldPosZ;
            (*gPathControlInterface)->attachObject((void*)obj, &trickyState->pathControlFlags);
            if (((GameObject*)obj)->anim.currentMove == 8 || ((GameObject*)obj)->anim.currentMove == 7)
            {
                trickyState->waterLevel = lbl_803E2414;
                trickyState->eventTime = lbl_803E2544;
            }
            else
            {
                trickyState->waterLevel = lbl_803E23DC;
            }
        }
        *(s32*)&trickyState->stateFlags &= ~0x4201;
        if (((TrickyByteFlags*)&trickyState->flags82E)->bit5 != 0)
        {
            ((TrickyByteFlags*)&trickyState->flags82E)->bit5 = 0;
        }
        else
        {
            ((TrickyByteFlags*)&trickyState->flags82E)->bit7 = 1;
        }
    }
    if (trickyState->followObj != NULL &&
        (trickyState->followObj->objectFlags & OBJECT_OBJFLAG_FREED) != 0)
    {
        if ((trickyState->stateFlags & 0x10) != 0)
        {
            trickyState->stateFlags &= ~0x10LL;
            trickyState->groundSnapCounter = 2;
            (*gPathControlInterface)->attachObject((void*)obj, &trickyState->pathControlFlags);
            ((GameObject*)obj)->anim.localPosX = trickyState->homePosX;
            ((GameObject*)obj)->anim.localPosY = trickyState->homePosY;
            ((GameObject*)obj)->anim.localPosZ = trickyState->homePosZ;
            ((GameObject*)obj)->anim.worldPosX = trickyState->homePosX;
            ((GameObject*)obj)->anim.worldPosY = trickyState->homePosY;
            ((GameObject*)obj)->anim.worldPosZ = trickyState->homePosZ;
            ObjHits_SyncObjectPosition((GameObject*)obj);
            childLoop.index = 0;
            trickyState->followPhase = childLoop.index;
            z = lbl_803E23DC;
            trickyState->prevSpeed = z;
            trickyState->speed = z;
            trickyState->stateFlags |= 0x80000LL;
            trickyState->stateFlags &= ~(u64)0x2000;
            if ((trickyState->stateFlags & TRICKY_STATE_FLAG_FLAME_CHILDREN_ACTIVE) != 0)
            {
                u8* childCursor;

                trickyState->stateFlags =
                    trickyState->stateFlags & ~(u64)TRICKY_STATE_FLAG_FLAME_CHILDREN_ACTIVE;
                trickyState->stateFlags =
                    trickyState->stateFlags | TRICKY_STATE_FLAG_FLAME_CHILDREN_CLEANUP;
                childCursor = (u8*)state;
                for (; childLoop.index < 7; childCursor += 4, childLoop.index++)
                {
                    objSetAnimSpeedTo1((GameObject*)*(int*)(childCursor + 0x700));
                }
                Sfx_RemoveLoopedObjectSound(obj, SFXTRIG_trpopn_c);
                TRICKY_VOICE(obj, 0x29d, 0);
            }
            Sfx_RemoveLoopedObjectSound(obj, SFXTRIG_trwhin1);
        }
        TRICKY_RESET_COMMAND(state);
        trickyState->followObj = NULL;
    }
    {
        int cmd;

        if ((trickyState->stateFlags & 0x10) != 0 && (*gGameUIInterface)->isEventReady(0xc1) != 0)
        {
            cmd = 0;
        }
        else
        {
            cmd = (*gGameUIInterface)->isOneOfItemsBeingUsed(cmdQuery.ids, TRICKY_ITEM_ID_COUNT);
        }
        ref = state;
        count = trickyState->commandCount;
        for (i = 0; i < count; i++, ref += 8)
        {
            if (*(s8*)(ref + 0x74d) == cmd)
            {
                found = 1;
                break;
            }
        }
        if ((trickyState->stateFlags & 0x10) == 0 && trickyFoodFn_8013db3c((u8*)obj, (u8*)state) == 2)
        {
            trickyState->stateIndex = 0x11;
        }
        else if (trickyState->stateIndex == 8 && cmd == 4)
        {
            *(u8*)&trickyState->wanderTargetZ = *(u8*)&trickyState->wanderTargetZ ^ 1;
        }
        else if (trickyState->stateIndex == 0xd && cmd == 4 && found == 0)
        {
            *(int*)&trickyState->stateFlags728 = 1;
        }
        else if (trickyState->stateIndex == 0xe && cmd == 4)
        {
            *(int*)&trickyState->stateFlags728 = 1;
        }
        else if (cmd == 0)
        {
            trickyState->stateFlags |= 0x30002LL;
        }
        else
        {
            flags = trickyState->stateFlags;
            if ((flags & 0x10) == 0)
            {
                switch (cmd)
                {
                case 1:
                    trickyState->commandPhase = 1;
                    trickySelectQueuedCommandTarget(trickyState, 1);
                    TRICKY_VOICE(obj, 0x13c, 0);
                    switch (trickyState->followObj->anim.seqId)
                    {
                    case 0x1ca:
                        if (**(u8**)state < 4)
                        {
                            if (Obj_IsLoadingLocked())
                            {
                                trickyState->stateFlags |= 4;
                                TRICKY_RESET_COMMAND(state);
                                TRICKY_SPAWN_FOOD_BUBBLE(obj, state);
                            }
                        }
                        else
                        {
                            trickyState->stateIndex = 2;
                        }
                        break;
                    case 0x160:
                        if (**(u8**)state < 4)
                        {
                            if (Obj_IsLoadingLocked())
                            {
                                trickyState->stateFlags |= 4;
                                TRICKY_RESET_COMMAND(state);
                                TRICKY_SPAWN_FOOD_BUBBLE(obj, state);
                            }
                        }
                        else
                        {
                            trickyState->stateIndex = 3;
                        }
                        break;
                    case 0x6a:
                    case 0x193:
                    case 0x3fb:
                    case 0x658:
                        trickyState->stateIndex = 9;
                        break;
                    case 0x195:
                        if (**(u8**)state < 2)
                        {
                            if (Obj_IsLoadingLocked())
                            {
                                trickyState->stateFlags |= 4;
                                TRICKY_RESET_COMMAND(state);
                                TRICKY_SPAWN_FOOD_BUBBLE(obj, state);
                            }
                        }
                        else
                        {
                            trickyState->stateIndex = 0x10;
                        }
                        break;
                    case 0x352:
                        if (**(u8**)state < 4)
                        {
                            if (Obj_IsLoadingLocked())
                            {
                                trickyState->stateFlags |= 4;
                                TRICKY_RESET_COMMAND(state);
                                TRICKY_SPAWN_FOOD_BUBBLE(obj, state);
                            }
                        }
                        else
                        {
                            trickyState->stateIndex = 2;
                        }
                        break;
                    case 0x358:
                        trickyState->stateIndex = 0xe;
                        break;
                    default:
                        TRICKY_RESET_COMMAND(state);
                        trickyReportError(base + 0x8c4);
                        break;
                    }
                    break;
                case 3:
                    played = 0;
                    if (trickyState->commandPhase == 3)
                    {
                        ref = state;
                        count = trickyState->commandCount;
                        for (i = 0; i < count; i++, ref += 8)
                        {
                            if (*(s8*)(ref + 0x74d) == 3)
                            {
                                played = 1;
                            }
                        }
                    }
                    else
                    {
                        played = 1;
                    }
                    if (played != 0)
                    {
                        trickyState->commandPhase = 3;
                        if (trickySelectQueuedCommandTarget(trickyState, 3) != 0)
                        {
                            switch (trickyState->followObj->anim.seqId)
                            {
                            case 0x36:
                            case 0x104:
                            case 0x131:
                            case 0x19f:
                            case 0x26c:
                            case 0x475:
                            case 0x546:
                            case 0x7c3:
                                trickyState->stateIndex = 0xa;
                                trickyState->idleSfxTimer = (f32)(int)randomGetRange(0x1f4, 0x2ee);
                                break;
                            case 0x6f0:
                                trickyState->stateIndex = 0xe;
                                break;
                            default:
                                trickyState->stateIndex = 8;
                                break;
                            }
                        }
                        else
                        {
                            trickyState->stateFlags |= 0x40000LL;
                        }
                    }
                    break;
                case 4:
                    if (**(u8**)state < 4)
                    {
                        if (Obj_IsLoadingLocked())
                        {
                            trickyState->stateFlags |= 4;
                            TRICKY_RESET_COMMAND(state);
                            TRICKY_SPAWN_FOOD_BUBBLE(obj, state);
                        }
                    }
                    else
                    {
                        trickyState->commandPhase = 4;
                        trickySelectQueuedCommandTarget(trickyState, 4);
                        trickyState->stateIndex = 7;
                        switch (trickyState->followObj->anim.seqId)
                        {
                        case 0x1c9:
                            *(void**)&trickyState->unk724 = dimicewall_countdownCallback;
                            break;
                        case 0x718:
                            *(void**)&trickyState->unk724 = dimtruthhornice_countdownCallback;
                            break;
                        case 0x551:
                            *(void**)&trickyState->unk724 = vfpflamepoint_countdownCallback;
                            break;
                        case 0x191:
                            *(void**)&trickyState->unk724 = dimlogfire_countdownCallback;
                            break;
                        case 0x470:
                            *(void**)&trickyState->unk724 = drchimmey_countdownCallback;
                            break;
                        case 0x102:
                        case 0x194:
                        case 0x542:
                        case 0x54c:
                        case 0x6f9:
                            *(void**)&trickyState->unk724 = 0;
                            break;
                        case 0x3c:
                            *(void**)&trickyState->unk724 = shbeacon_resetFadeTimerCallback;
                            break;
                        case 0x50f:
                            *(void**)&trickyState->unk724 = wcbeacon_aButtonCallback;
                            break;
                        default:
                            TRICKY_RESET_COMMAND(state);
                            trickyReportError(base + 0x8c4);
                            break;
                        }
                    }
                    break;
                case 5:
                    if (Obj_IsLoadingLocked())
                    {
                        trickyState->commandPhase = 5;
                        setup = (int)Obj_AllocObjectSetup(0x18, TRICKY_CHILD_OBJ_SIDEKICK_BALL);
                        *(u8*)(setup + 7) = 0xff;
                        *(u8*)(setup + 4) = 2;
                        ((ObjPlacement*)setup)->posX = ((GameObject*)obj)->anim.worldPosX;
                        ((ObjPlacement*)setup)->posY = ((GameObject*)obj)->anim.worldPosY;
                        ((ObjPlacement*)setup)->posZ = ((GameObject*)obj)->anim.worldPosZ;
                        trickyState->followObj =
                            Obj_SetupObject((ObjPlacement*)setup, 5, -1, -1, ((GameObject*)obj)->anim.parent);
                        target = &trickyState->followObj->anim.worldPosX;
                        if (trickyState->targetPosPtr != target)
                        {
                            trickyState->targetPosPtr = target;
                            {
                                u32 m;
                                u32 f2 = trickyState->stateFlags;
                                m = ~TRICKY_STATE_TARGET_DIRTY_FLAG;
                                trickyState->stateFlags = f2 & m;
                            }
                            trickyState->linkedWalkGroup = 0;
                        }
                        trickyState->substate = 0;
                        trickyState->stateIndex = 0xb;
                    }
                    break;
                default:
                    if (trickyState->stateIndex == 1 && trickyState->commandPhase != 0 &&
                        (flags & 0x20000) == 0)
                    {
                        step = trickyFindNearestUsableBaddie(trickyState->playerObj, lbl_803E24D8, 0);
                        if (step != NULL)
                        {
                            trickyState->followObj = step;
                            if (trickyState->targetPosPtr != &step->anim.worldPosX)
                            {
                                trickyState->targetPosPtr = &step->anim.worldPosX;
                                {
                                    u32 m;
                                    u32 f2 = trickyState->stateFlags;
                                    m = ~TRICKY_STATE_TARGET_DIRTY_FLAG;
                                    trickyState->stateFlags = f2 & m;
                                }
                                trickyState->linkedWalkGroup = 0;
                            }
                            trickyState->stateIndex = 0xd;
                            trickyState->substate = 0;
                            *(int*)&trickyState->stateFlags728 = 0;
                        }
                    }
                    break;
                }
            }
            else if (cmd == 3)
            {
                trickyState->stateFlags = flags | 0x40000LL;
            }
        }
    }
    flags = trickyState->stateFlags;
    if ((flags & 0x10) == 0)
    {
        if ((flags & 0x10000) != 0)
        {
            if ((flags & 0x20000) != 0)
            {
                TRICKY_RESET_COMMAND(state);
                *(u8*)&trickyState->commandPhase = 0;
            }
            else
            {
                TRICKY_RESET_COMMAND(state);
            }
            trickyState->cooldownA = lbl_803E2548;
        }
        else if ((flags & 0x40000) != 0)
        {
            trickyState->followObj = (GameObject*)obj;
            trickyState->stateIndex = 0xf;
            trickyState->idleSfxTimer = (f32)(int)randomGetRange(0x1f4, 0x2ee);
            {
                u32 m;
                u32 f2 = trickyState->stateFlags;
                m = ~TRICKY_STATE_RESET_FLAG_40000;
                trickyState->stateFlags = f2 & m;
            }
            trickyState->commandPhase = 3;
            if (trickyState->targetPosPtr != &trickyState->wanderTargetX)
            {
                trickyState->targetPosPtr = &trickyState->wanderTargetX;
                {
                    u32 m;
                    u32 f2 = trickyState->stateFlags;
                    m = ~TRICKY_STATE_TARGET_DIRTY_FLAG;
                    trickyState->stateFlags = f2 & m;
                }
                trickyState->linkedWalkGroup = 0;
            }
        }
    }
    *(u8*)&((GameObject*)obj)->anim.resetHitboxMode =
        *(u8*)&((GameObject*)obj)->anim.resetHitboxMode | INTERACT_FLAG_DISABLED;
    trickyState->heightUpdateActive = 1;
    ((TrickyHandlerFn*)(base + 0x24))[trickyState->stateIndex](obj, state);
    trickyState->stateFlags &= ~(u64)0x2;
    trickyState->animTransitionTimer += timeDelta;
    if (trickyState->animTransitionTimer > lbl_803E247C)
    {
        if (((GameObject*)obj)->anim.currentMove != trickyState->moveId)
        {
            if ((trickyState->pendingStateFlags & 0x1000000) != 0 &&
                (trickyState->stateFlags & 0x1000000) != 0)
            {
                ObjAnim_SetCurrentMove(obj, trickyState->moveId, ((GameObject*)obj)->anim.currentMoveProgress,
                                       0);
            }
            else
            {
                ObjAnim_SetCurrentMove(obj, trickyState->moveId, lbl_803E23DC, 0);
            }
            trickyState->stateFlags &= ~0x060001e0LL;
            trickyState->stateFlags |= trickyState->pendingStateFlags;
            trickyState->animTransitionTimer = lbl_803E23DC;
            trickyState->moveProgress = trickyState->moveProgressTarget;
        }
    }
    if ((trickyState->stateFlags & 0x2000000) != 0)
    {
        ((GameObject*)obj)->anim.localPosX += timeDelta * (trickyState->dirX * trickyState->speed);
        ((GameObject*)obj)->anim.localPosZ += timeDelta * (trickyState->dirZ * trickyState->speed);
        ObjAnim_SampleRootCurvePhase((ObjAnimComponent*)obj, trickyState->speed, (float*)(state + 0x34));
    }
    if (trickyState->moveProgress == lbl_803E23DC)
    {
        ObjAnim_SetMoveProgress((ObjAnimComponent*)obj, trickyState->arcMoveProgress);
    }
    if (ObjAnim_AdvanceCurrentMove((int)obj, trickyState->moveProgress, timeDelta,
                                                                    (void*)(state + 0x80c)) != 0)
    {
        trickyState->stateFlags |= 0x8000000LL;
    }
    else
    {
        trickyState->stateFlags &= ~0x8000000LL;
    }
    if ((trickyState->stateFlags & TRICKY_STATE_FLAG_ROTATE) != 0)
    {
        int rotationDiff;
        int rotationStep;

        rotationDiff = trickyState->targetYaw - (u16)((GameObject*)obj)->anim.rotX;
        if (rotationDiff > 0x8000)
        {
            rotationDiff -= 0xffff;
        }
        if (rotationDiff < -0x8000)
        {
            rotationDiff += 0xffff;
        }
        rotationStep = (int)((f32)trickyState->rotRate * trickyState->rotStepScale);
        if ((rotationDiff >= 0 ? rotationDiff : -rotationDiff) >= 4)
        {
            if ((rotationStep > 0 && rotationDiff > 0) || (rotationStep < 0 && rotationDiff < 0))
            {
                if ((rotationStep >= 0 ? rotationStep : -rotationStep) >
                    (rotationDiff >= 0 ? rotationDiff : -rotationDiff))
                {
                    ((GameObject*)obj)->anim.rotX += rotationDiff;
                }
                else
                {
                    ((GameObject*)obj)->anim.rotX += rotationStep;
                }
            }
            else
            {
                ((GameObject*)obj)->anim.rotX += rotationStep;
            }
        }
        else
        {
            ((GameObject*)obj)->anim.rotX += rotationDiff;
        }
    }
    if ((trickyState->stateFlags & TRICKY_STATE_FLAG_BACKSTEP) != 0)
    {
        ((GameObject*)obj)->anim.localPosX += trickyState->backstepDelta *
                                              (trickyState->dirX * -trickyState->backstepScale);
        ((GameObject*)obj)->anim.localPosZ += trickyState->backstepDelta *
                                              (trickyState->dirZ * -trickyState->backstepScale);
    }
    if ((trickyState->stateFlags & TRICKY_STATE_FLAG_VERTICAL_MOVE) != 0)
    {
        ((GameObject*)obj)->anim.localPosY +=
            trickyState->verticalScale * trickyState->verticalDelta;
    }
    if ((trickyState->stateFlags & TRICKY_STATE_FLAG_SIDESTEP) != 0)
    {
        ((GameObject*)obj)->anim.localPosX +=
            trickyState->sidestepDelta * (trickyState->dirZ * trickyState->sidestepScale);
        ((GameObject*)obj)->anim.localPosZ += trickyState->sidestepDelta *
                                              (trickyState->dirX * -trickyState->sidestepScale);
    }
    if (trickyState->followObj != NULL)
    {
        trickyState->followPosValid = 1;
        trickyState->followPosX = trickyState->followObj->anim.worldPosX;
        trickyState->followPosY = trickyState->followObj->anim.worldPosY;
        trickyState->followPosZ = trickyState->followObj->anim.worldPosZ;
    }
    else
    {
        trickyState->followPosValid = 0;
    }
    if (((GameObject*)obj)->anim.currentMove == 0x2a)
    {
        characterHeadLookRelax((GameObject*)(obj), (void*)(state + 0x378));
        characterCloseEyes((GameObject*)(obj), (void*)(state + 0x378));
    }
    else
    {
        characterUpdateHeadLook((GameObject*)obj, (CharacterEyeAnimState*)(state + 0x378), lbl_803E23DC);
        characterDoEyeAnims((GameObject*)obj, (void*)(state + 0x378));
    }
    objAnimFn_80038f38((GameObject*)obj, (char*)state + 0x3a8);
    {
        f32* pathCursor;
        TrickyState* pathState;

        pathState = ((GameObject*)obj)->extra;
        pathCursor = pathState->targetPosPtr;
        pathState->previousPathPoint = pathCursor;
        if (pathState->previousPathPoint != NULL)
        {
            pathState->previousPathX = pathCursor[0];
            pathState->previousPathY = pathCursor[1];
            pathState->previousPathZ = pathCursor[2];
        }
    }
    trickyState->prevSpeed = trickyState->speed;
    i = trickyState->commandCount - 1;
    {
        int cur = state + i * 8;

        for (; i >= 0; cur -= 8, i--)
        {
            *(s8*)(cur + 0x74e) -= 1;
            if (*(s8*)(cur + 0x74e) == 0)
            {
                memmove((void*)(cur + 0x748), (void*)(state + (i + 1) * 8 + 0x748),
                        (trickyState->commandCount - i - 1) * 8);
                trickyState->commandCount -= 1;
            }
        }
    }
    if (getXZDistance(&((GameObject*)obj)->anim.worldPosX,
                      &trickyState->playerObj->anim.worldPosX) >= lbl_803E2538 &&
        mainGetBit(GAMEBIT_Tricky_Usable) != 0)
    {
        trickyState->stateFlags |= 0x10000LL;
    }
    trickyState->cooldownC -= timeDelta;
    if (trickyState->cooldownC < *(f32*)&lbl_803E23DC)
    {
        trickyState->cooldownC = lbl_803E23DC;
    }
    if ((trickyState->stateFlags & 4) != 0)
    {
        st = ((GameObject*)obj)->extra;
        if (((TrickyByteFlags*)((u8*)st + 0x58))->bit6 != 0)
        {
            played = 0;
        }
        else
        {
            switch (((GameObject*)obj)->anim.currentMove)
            {
            case 0x29:
            case 0x2a:
            case 0x2b:
            case 0x2c:
            case 0x2d:
            case 0x2e:
            case 0x2f:
                played = 0;
                break;
            default:
                if (Sfx_IsPlayingFromObjectChannel(obj, 0x10) != 0)
                {
                    played = 0;
                }
                else
                {
                    objAudioFn_800393f8((GameObject*)obj, &st->soundState, 0x298, 0x500, 0xffffffff, 0);
                    played = 1;
                }
                break;
            }
        }
        if (played != 0)
        {
            trickyState->stateFlags &= ~(u64)0x4;
        }
    }
    trickyState->voiceCooldown -= timeDelta;
    if (trickyState->voiceCooldown < *(f32*)&lbl_803E23DC)
    {
        trickyState->voiceCooldown = lbl_803E23DC;
    }
    if (trickyState->voiceCooldown > lbl_803E23DC)
    {
        TRICKY_VOICE(obj, 0x29c, 0x100);
    }
    trickyUpdateCollisionAndPathState((u8*)obj);
    if ((trickyState->stateFlags & 0x80000000) != 0)
    {
        trickyState->impressTimer -= timeDelta;
        if (trickyState->impressTimer <= lbl_803E23DC)
        {
            trickyState->stateFlags &= 0x7FFFFFFF;
            sfxId = ((u16*)&pair)[randomGetRange(0, 1)];
            TRICKY_VOICE(obj, sfxId, 0x500);
        }
    }
    tricky_updateModelVariantFade(obj, state);
    Tricky_updateBlendChannelWeight(obj, (u8*)state);
    if (trickyState->speed > lbl_803E254C)
    {
        objAudioFn_8006ef38((GameObject*)obj, (ObjAnimEventList*)(state + 0x80c), 1, (void*)(state + 0x7d8),
                            (void*)(state + 0xf8), trickyState->speed, lbl_803E23E8);
    }
    if (lbl_803E23DC == trickyState->waterLevel)
    {
        talking = 0;
    }
    else if (lbl_803E2410 == trickyState->eventTime)
    {
        talking = 1;
    }
    else if (trickyState->currentTime - trickyState->eventTime > lbl_803E2414)
    {
        talking = 1;
    }
    else
    {
        talking = 0;
    }
    if (talking != 0)
    {
        u8* soundCursor;
        int sfx2;

        soundCursor = (u8*)state + 0x80c;
        sfx2 = 0;
        for (i = 0, count = *(s8*)(soundCursor + 0x1b); i < count; i++)
        {
            switch (*(s8*)(soundCursor + i + 0x13))
            {
            case 0:
            case 1:
            case 2:
                sfx2 = 0x433;
                break;
            }
        }
        if (sfx2 != 0)
        {
            Sfx_PlayFromObject(obj, (u16)sfx2);
        }
    }
    trickyState->prevLocalPosX = ((GameObject*)obj)->anim.previousLocalPosX;
    trickyState->prevLocalPosY = ((GameObject*)obj)->anim.previousLocalPosY;
    trickyState->prevLocalPosZ = ((GameObject*)obj)->anim.previousLocalPosZ;
    if (*(void**)&trickyState->child != NULL)
    {
        trickyState->childPhaseTimer0 += timeDelta;
        trickyState->childPhaseTimer1 += timeDelta;
        trickyState->childPhaseTimer2 += timeDelta;
        if (trickyState->childPhaseTimer2 > *(f32*)&lbl_803E24C8)
        {
            trickyState->childPhaseTimer2 -= lbl_803E24C8;
        }
        if (trickyState->childPhaseTimer2 >= lbl_803E2408)
        {
            *(s16*)(*(int*)&trickyState->child + 6) =
                *(s16*)(*(int*)&trickyState->child + 6) | 0x4000;
        }
        else
        {
            *(s16*)(*(int*)&trickyState->child + 6) =
                *(s16*)(*(int*)&trickyState->child + 6) & ~0x4000;
        }
        if (trickyState->childPhaseTimer1 > lbl_803E24D8)
        {
            if (trickyState->childPhaseTimer1 > lbl_803E2440)
            {
                trickyState->childPhaseTimer1 -= lbl_803E2440;
            }
            *(s16*)(*(int*)&trickyState->child + 6) =
                *(s16*)(*(int*)&trickyState->child + 6) | 0x4000;
        }
        if (trickyState->childPhaseTimer0 > lbl_803E2550)
        {
            if (mainGetBit(GAMEBIT_ITEM_TrickyFood_Count) != 0)
            {
                TRICKY_VOICE(obj, 0x392, 0x500);
            }
            else
            {
                TRICKY_VOICE(obj, 0x298, 0x500);
            }
            trickyState->childPhaseTimer0 -= lbl_803E2550;
        }
        ObjAnim_AdvanceCurrentMove(*(int*)&trickyState->child, lbl_803E23EC, timeDelta, 0);
    }
    if (*(void**)&trickyState->childB != NULL)
    {
        ObjAnim_AdvanceCurrentMove(*(int*)&trickyState->childB, lbl_803E23EC, timeDelta, 0);
    }
    if (*(void**)&trickyState->childA != NULL)
    {
        ObjAnim_AdvanceCurrentMove(*(int*)&trickyState->childA, lbl_803E23EC, timeDelta, 0);
    }
}


void Tricky_init(GameObject* obj)
{
    int state;
    int model;
    int pathState;
    u32 modelVariant;
    u16 startPath[4];

    state = *(int*)&(obj)->extra;
    startPath[0] = lbl_803E23C0;
    mainSetBits(GAMEBIT_TrickyTalk, 0xff);
    if (mainGetBit(GAMEBIT_ITEM_TrickyBall_Bought) != 0)
    {
        mainSetBits(GAMEBIT_ITEM_TrickyBall_Usable, 1);
    }
    (obj)->animEventCallback = tricky_SeqFn;
    ObjGroup_AddObject((int)obj, TRICKY_OBJGROUP);
    trickyVoxAllocFn_8004b5d4(&((TrickyState*)state)->pathSearches[0]);
    trickyVoxAllocFn_8004b5d4(&((TrickyState*)state)->pathSearches[1]);
    trickyVoxAllocFn_8004b5d4(&((TrickyState*)state)->pathSearches[2]);
    trickyVoxAllocFn_8004b5d4(&((TrickyState*)state)->pathSearches[3]);
    trickyVoxAllocFn_8004b5d4(&((TrickyState*)state)->pathSearches[4]);
    trickyVoxAllocFn_8004b5d4(&((TrickyState*)state)->pathSearches[5]);
    trickyVoxAllocFn_8004b5d4(&((TrickyState*)state)->pathSearches[6]);
    trickyVoxAllocFn_8004b5d4(&((TrickyState*)state)->pathSearches[7]);
    trickyVoxAllocFn_8004b5d4(&((TrickyState*)state)->pathSearches[8]);
    ((TrickyState*)state)->progressPtr = (*gMapEventInterface)->getTrickyEnergy();
    ((TrickyState*)state)->playerObj = Obj_GetPlayerObject();
    ((TrickyState*)state)->stateIndex = 0;
    ((TrickyState*)state)->commandRequestBits = 0;
    ((TrickyState*)state)->previousPathPoint = NULL;
    ((TrickyState*)state)->activeWalkGroup = 0;
    ((TrickyState*)state)->homePosX = (obj)->anim.worldPosX;
    ((TrickyState*)state)->homePosY = (obj)->anim.worldPosY;
    ((TrickyState*)state)->homePosZ = (obj)->anim.worldPosZ;
    modelVariant = *(u8*)(((TrickyState*)state)->progressPtr + 2) / 10;
    ((TrickyState*)state)->modelVariant = modelVariant;
    model = (int)Obj_GetActiveModel(obj);
    *(u8*)(*(int*)(model + 0x34) + 8) = ((TrickyState*)state)->modelVariant;
    pathState = (int)&((TrickyState*)state)->pathControlFlags;
    (*gPathControlInterface)->init((void*)pathState, 1, 0xa7, 1);
    (*gPathControlInterface)->setLocalPointCollision((void*)pathState, 1, gTrickyPathPointCollision, &lbl_803DBC48, 2);
    (*gPathControlInterface)->setup((void*)pathState, 2, lbl_8031D2E8, lbl_803DBC40, startPath);
    (*gPathControlInterface)->attachObject((void*)obj, (void*)pathState);
    doNothing_onTrickyInit();
    walkgroupFindExitPointFn_800dc398();
    ((TrickyState*)state)->groundSnapCounter = 2;
    ((TrickyInitFlags*)&((TrickyState*)state)->flags82E)->initBit7 = 1;
    ((TrickyState*)state)->commandPhase = -1;
}

void trickyReportError(const char* fmt, ...)
{
}

void trickyDebugPrint(const char* fmt, ...)
{
}

/* pooled sidekick-command debug format strings (embedded NULs), raw bytes. */
char sSidekickCommandDebugTextBlock[] = {
    0x73, 0x69, 0x64, 0x65, 0x43, 0x6F, 0x6D, 0x6D, 0x61, 0x6E, 0x64, 0x45, 0x6E, 0x61, 0x62, 0x6C, 0x65, 0x20,
    0x77, 0x61, 0x72, 0x6E, 0x69, 0x6E, 0x67, 0x3A, 0x20, 0x6E, 0x65, 0x65, 0x64, 0x20, 0x74, 0x6F, 0x20, 0x69,
    0x6E, 0x63, 0x72, 0x65, 0x61, 0x73, 0x65, 0x20, 0x4D, 0x41, 0x58, 0x5F, 0x43, 0x4F, 0x4D, 0x4D, 0x5F, 0x50,
    0x52, 0x45, 0x53, 0x45, 0x4E, 0x54, 0x0A, 0x00, 0x00, 0x00, 0x68, 0x69, 0x74, 0x73, 0x3A, 0x20, 0x25, 0x64,
    0x20, 0x25, 0x64, 0x20, 0x25, 0x64, 0x20, 0x25, 0x64, 0x20, 0x25, 0x64, 0x20, 0x25, 0x64, 0x20, 0x25, 0x64,
    0x20, 0x25, 0x64, 0x00, 0x00, 0x00, 0x0A, 0x45, 0x6E, 0x65, 0x72, 0x67, 0x79, 0x3A, 0x20, 0x25, 0x64, 0x2F,
    0x25, 0x64, 0x0A, 0x00, 0x66, 0x69, 0x6E, 0x64, 0x20, 0x63, 0x6F, 0x6D, 0x6D, 0x61, 0x6E, 0x64, 0x20, 0x75,
    0x73, 0x65, 0x64, 0x20, 0x6F, 0x6E, 0x20, 0x74, 0x68, 0x65, 0x20, 0x77, 0x72, 0x6F, 0x6E, 0x67, 0x20, 0x6F,
    0x62, 0x6A, 0x65, 0x63, 0x74, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
