#include "main/dll/partfx_interface.h"
#include "sys/objects.h"
#include "sys/objects/lifecycle.h"
#include "game/objects/object_setup.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/audio/sfx.h"
#include "main/gamebits.h"
#include "main/game_ui_interface.h"
#include "main/model.h"
#include "main/objHitReact.h"
#include "main/objhits.h"
#include "main/objanim.h"
#include "main/objanim_update.h"
#include "main/objseq.h"
#include "main/resource.h"
#include "main/dll/path_control_interface.h"
#include "main/obj_group.h"
#include "main/obj_link.h"
#include "main/obj_path.h"
#include "main/frame_timing.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_trig_api.h"
#include "main/dll/DR/dll_0257_drearthwarrior.h"
#include "dlls/object_descriptor.h"
#include "main/dll/tricky_api.h"
#include "main/dll/dll_002E_moveLib.h"
#include "main/dll/dll_0282_barrelgener.h"
#include "main/camera.h"
#include "main/byte_flags.h"
#include "main/gamebit_ids.h"
#include "game/objects/object.h"
#include "main/object_render.h"
#include "main/objprint_anim_api.h"
#include "main/objprint_character_api.h"
#include "main/objprint_sound_api.h"
#include "main/objprint_api.h"
#include "main/pad.h"
#include "main/dll/baddie_state.h"
#include "main/dll/player_api.h"
#include "main/dll/player_motion_api.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/player_control_interface.h"
#include "main/maketex_timer_api.h"
#include "main/vecmath.h"
#include "dlls/objects/473_DIM2PrisonM.h"
#include "main/newshadows.h"
#include "main/newshadows_audio_api.h"

typedef struct DREarthWarriorPlacement
{
    ObjPlacement head; /* 0x00 */
    s8 spawnYaw; /* (s8)<<8 -> anim.rotX */
    u8 unk19;    /* -> EarthWarriorSub.unk990 */
    s16 airMeterMax;
} DREarthWarriorPlacement;

/* Combat sub-block of the EarthWarrior state (state+0xb58). */
typedef struct EarthWarriorSub
{
    u8 pad000[0x264];
    u8 unk264;
    u8 pad265[0xfb];
    int unk360;
    u8 pad364[0x8c];
    u8 flags3F0; /* ByteFlags: b40 leap, b80 airborne */
    u8 flags3F1;
    u8 flags3F2;
    u8 pad3F3[5];
    int moveTable; /* config row pointer */
    int prevMoveTable; /* 0x3FC: previously-selected move-table pointer; compared against moveTable to detect a move change */
    int configRow;       /* config row pointer */
    f32 animSpeedMax;    /* 0x404: upper clamp for BaddieState.animSpeedC (and symmetric +/-max fallback) */
    f32 targetAnimSpeed; /* 0x408: interpolate() target for BaddieState.animSpeedC, clamped to configRow[0xc] floor */
    u8 pad40C[4];
    f32 footstepCooldown; /* 0x410: per-tick countdown gating footstep rumble/sfx */
    u8 pad414[0xc];
    f32 yawStepRate; /* 0x420: base per-frame yaw-turn step (yawStepScale*yawStepRate*timeDelta caps appliedYaw turn) */
    u8 pad424[4];
    f32 yawSmoothDivisor; /* 0x428: divides smoothing const (K/yawSmoothDivisor) = interpolate() rate for appliedYaw yaw turn */
    f32 yawStepScale;            /* 0x42C: multiplier on yawStepRate for the appliedYaw yaw-turn per-frame cap */
    f32 currentYawSmoothDivisor; /* 0x430: divides smoothing const for the currentYaw turn (parallel to yawSmoothDivisor) */
    f32 currentYawStepRate;      /* 0x434: per-frame step cap (currentYawStepRate*timeDelta) for the currentYaw turn */
    f32 animSpeedSmoothing; /* 0x438: interpolate() rate/gain closing animSpeedC toward targetAnimSpeed (copied from unk830) */
    u8 pad43C[0x14];
    u8* unk450;
    u8* unk454;
    u8* unk458;
    u8* unk45C;
    u8* unk460;
    u8 pad464[0xc];
    f32 unk470;
    int unk474;
    s16 appliedYaw; /* 0x478: yaw written to anim.rotX each frame; turned toward target (accum += degToAngle*step) and latched from currentYaw/leapStartYaw at state entry */
    u8 pad47A[2];
    int yawTurnProgress; /* 0x47C: interpolate() source (cast f32) for the appliedYaw yaw turn; reset to 0 on state entry */
    int yawTurnDir; /* 0x480: sign source (<0 => negate) for the appliedYaw yaw-turn direction; reset to 0 on state entry */
    s16 currentYaw; /* current yaw */
    u8 pad486[2];
    int frameCounter;
    int turnDegrees; /* 0x48C: signed turn in degrees; magnitude*0xb6 (deg->binary angle) accumulated into currentYaw, sign gives direction */
    u8 pad490[4];
    int savedYaw;
    u8 pad498[0x3a];
    s16 aimHalfY;  /* 0x4D2: aimAccumY/2; written to secondary look-bone (vec9) */
    s16 aimAccumY; /* 0x4D4: integrated aim angle (accum += delta*timeDelta) toward clamped target from yawTurnDir; written to look-bone vec0[1] */
    s16 aimAccumX; /* 0x4D6: integrated aim angle (accum += delta) from spawnRotY phase; written negated to look-bone vec0[0] */
    u8 pad4D8[0x308];
    f32 unk7E0;
    u8 pad7E4[0x48];
    f32 animSpeedASmoothing;      /* 0x82C: interpolate() rate closing BaddieState.animSpeedA toward animSpeedC */
    f32 animSpeedSmoothingReload; /* 0x830: preset smoothing rate copied into animSpeedSmoothing on state entry */
    f32 unk834;
    u8 pad838[8];
    f32 animSpeedScale; /* 0x840: multiplier scaling the clamped input phase into targetAnimSpeed */
    f32 animSpeedRate; /* 0x844: per-frame anim-speed rate integrated into animSpeedC (animSpeedRate*timeDelta); captured from animSpeedA */
    u8 pad848[0x10];
    int leapStartYaw; /* 0x858: yaw latched (from currentYaw) at leap start; re-added to move progress to build leap yaw */
    u8 pad85C[0x4a];
    u8 soundId;       /* 0x8A6: active sound-effect id passed to objAudioFn_8006edcc (8 or 0xa) */
    u8 soundIdReload; /* 0x8A7: stored sound id copied into soundId on leap trigger */
    u8 pad8A8[8];
    u8 attackStage;
    u8 pad8B1[0x1b];
    s8 attackPhase; /* attack phase */
    u8 pad8CD[3];
    u8 unk8D0;
    u8 unk8D1;
    u8 unk8D2;
    u8 unk8D3;
    u8 unk8D4;
    u8 pad8D5[3];
    u16 flags8D8;
    u8 pad8DA[6];
    f32 posX;
    f32 posY;
    f32 posZ;
    f32 unk8EC;
    u8 pad8F0[0x90];
    int savedControlMode;
    u8 pad984[2];
    s16 unk986;
    u8 pad988[2];
    s16 health; /* 0x98a */
    u16 flags98C;
    u8 rideState; /* 0x98E: interaction mode set by setRiderMode(param); 2 = stunned/ridden (A-button icon, interaction disabled), 0 = normal */
    u8 pad98F;
    u8 unk990;
    u8 pad991;
    u8 unk992;
    u8 unk993;
    u8 flags994; /* ByteFlags: b01/b02/b08/b10/b80 */
    u8 unk995;
    u8 pad996[2];
    f32 airMeterTimer; /* 0x998: frame timer armed to 30 by s16toFloat at init */
    s8 interactSequenceId; /* 0x99C: fallback sequence id passed to runSequence when interacted-with but event not ready; -1 = none */
    u8 unk99D;
    u8 pad99E[2];
    ObjModelChain* modelChain; /* 0x9A0: tail chain allocated from gEarthWarriorTailChainDesc */
} EarthWarriorSub;

STATIC_ASSERT(sizeof(EarthWarriorSub) == 0x9a4);
STATIC_ASSERT(offsetof(EarthWarriorSub, moveTable) == 0x3f8);
STATIC_ASSERT(offsetof(EarthWarriorSub, configRow) == 0x400);
STATIC_ASSERT(offsetof(EarthWarriorSub, unk450) == 0x450);
STATIC_ASSERT(offsetof(EarthWarriorSub, appliedYaw) == 0x478);
STATIC_ASSERT(offsetof(EarthWarriorSub, currentYaw) == 0x484);
STATIC_ASSERT(offsetof(EarthWarriorSub, savedYaw) == 0x494);
STATIC_ASSERT(offsetof(EarthWarriorSub, unk7E0) == 0x7e0);
STATIC_ASSERT(offsetof(EarthWarriorSub, animSpeedASmoothing) == 0x82c);
STATIC_ASSERT(offsetof(EarthWarriorSub, unk8D0) == 0x8d0);
STATIC_ASSERT(offsetof(EarthWarriorSub, posX) == 0x8e0);
STATIC_ASSERT(offsetof(EarthWarriorSub, unk8EC) == 0x8ec);
STATIC_ASSERT(offsetof(EarthWarriorSub, unk986) == 0x986);
STATIC_ASSERT(offsetof(EarthWarriorSub, health) == 0x98a);
STATIC_ASSERT(offsetof(EarthWarriorSub, rideState) == 0x98e);
STATIC_ASSERT(offsetof(EarthWarriorSub, unk990) == 0x990);
STATIC_ASSERT(offsetof(EarthWarriorSub, flags994) == 0x994);
STATIC_ASSERT(offsetof(EarthWarriorSub, airMeterTimer) == 0x998);
STATIC_ASSERT(offsetof(EarthWarriorSub, interactSequenceId) == 0x99c);
STATIC_ASSERT(offsetof(EarthWarriorSub, modelChain) == 0x9a0);

/* DR_EarthWarrior_getExtraSize == 0x14fc; BaddieState head + family tail. */
typedef struct EarthWarriorState
{
    BaddieState baddie;
    u8 pad35C[0x38c - 0x35c];
    CharacterEyeAnimState eyeAnimState; /* 0x38c: head-aim / eye-blink record (characterDoEyeAnims) */
    u8 pad3B4[0x3bc - 0x3b4];
    ObjSoundState modelSoundState; /* 0x3bc: mouth/voice playback state (objSoundUpdateMouth) */
    MoveLibState moveLib; /* 0x3ec: dll_2E look-controller block */
    u8 padA10[0xb18 - 0xa10];
    Vec3f pathPoints[4]; /* 0xb18: path points 3..6 in world space, refreshed each render */
    u8 padB48[0xb54 - 0xb48];
    GameObject* helperObj;
    EarthWarriorSub sub; /* 0xb58 */
} EarthWarriorState;

STATIC_ASSERT(sizeof(EarthWarriorState) == 0x14fc);
STATIC_ASSERT(offsetof(EarthWarriorState, eyeAnimState) == 0x38c);
STATIC_ASSERT(offsetof(EarthWarriorState, modelSoundState) == 0x3bc);
STATIC_ASSERT(offsetof(EarthWarriorState, moveLib) == 0x3ec);
STATIC_ASSERT(offsetof(EarthWarriorState, pathPoints) == 0xb18);
STATIC_ASSERT(offsetof(EarthWarriorState, helperObj) == 0xb54);
STATIC_ASSERT(offsetof(EarthWarriorState, sub) == 0xb58);

typedef struct
{
    s16 v[5];
} EWPathRange;

typedef struct
{
    u32 m[4][4];
} EWColorTbl;

const EWPathRange gDREarthWarriorLookInitData1 = {{10, 10, 0, 0, 0}};
const EWPathRange gDREarthWarriorLookInitData2 = {{20, 20, 0, 0, 0}};

extern ObjModelChainDesc* gEarthWarriorTailChainDesc;

#define PAD_BUTTON_A 0x100

#define DREARTHWARRIOR_OBJGROUP           0xa
#define DREARTHWARRIOR_PARTFX             0x7e6
#define DREARTHWARRIOR_AIRMETER_BGTEXTURE 0x5cf /* HUD air-meter background texture id */

#define DREARTHWARRIOR_OBJFLAG_PARENT_SLACK 0x1000

#define DREARTHWARRIOR_CHILD_OBJ_HELPER   0x6f5
/* attacker seqId whose hits are ignored here (retail OBJECTS.bin). */
#define DREARTHWARRIOR_ATTACKER_SEQID_SWORD 0x23 /* "sword" (DLL 0xE2) */
#define DREARTHWARRIOR_EFFECT_RESOURCE_ID 0x5a /* shared effect resource -> gEarthWarriorResource */

f32 gEarthWarriorMatrix[16];
void* gDREarthWarriorStateHandlers[4];
void* gDREarthWarriorDefaultStateHandler;
void* gEarthWarriorResource;


extern u8 gDREarthWarriorInitData[];

typedef struct DREarthWarriorInitData
{
    u8 unk0[0xC];
    u8 unkC[0x30];
    u8 unk3C[0x10];
    u8 unk4C[0x18];
    u8 unk64[0x20];
    u8 unk84[0x54];
    u8 unkD8[0x40];
    u8 unk118[0xA4];
    u8 unk1BC[0xA4];
    u8 unk260[4];
} DREarthWarriorInitData;
extern u8 gDREarthWarriorRowIndices[];
const EWColorTbl gDREarthWarriorColors = {
    {{8, 255, 190, 120}, {8, 255, 255, 120}, {8, 180, 240, 255}, {8, 170, 255, 170}}
};
void DR_EarthWarrior_updateLookAtBones(GameObject* obj, int sub, int state);

static const u8 gDREarthWarriorPathSetupParam[4] = {1, 1, 1, 1};

static void DR_EarthWarrior_setupPathState(u8* pathState, DREarthWarriorInitData* base, EarthWarriorSub* s)
{
    (*gPathControlInterface)->setup(pathState, 4, base->unkC, base->unk3C, (void*)gDREarthWarriorPathSetupParam);
    s->aimAccumY = 0.0f;
    s->aimHalfY = (f32)s->yawTurnDir;
}

void DR_EarthWarrior_func23(GameObject* obj, int mode)
{
    EarthWarriorState* inner = (obj)->extra;
    switch (mode)
    {
    case 1:
        inner->sub.health += 4;
        objSoundStartTimed(obj, &inner->modelSoundState, 0x291, 0x1000, -1, 1);
        inner->sub.unk8EC = 4.32f;
        *(f32*)((char*)lbl_8033527C + 0x24) = inner->sub.unk8EC;
        break;
    default:
        break;
    }
}

int DR_EarthWarrior_updateLeap(GameObject* obj, int sub, int state)
{
    EarthWarriorSub* s = (EarthWarriorSub*)sub;
    *(u32*)&s->unk360 |= 0x1000000LL;
    ((BaddieState*)state)->moveSpeed = 0.035f;
    if ((obj)->anim.currentMoveProgress > 0.1f && (obj)->anim.currentMoveProgress < 0.25f &&
        ((BaddieState*)state)->animSpeedC >
            *(f32*)((char*)s->configRow + 0x1c) - 0.4f &&
        ((BaddieState*)state)->inputMagnitude > 0.8f && s->frameCounter >= 0x96)
    {
        ((ByteFlags*)&s->flags3F0)->b40 = 1;
        ((ByteFlags*)&s->flags3F0)->b80 = 0;
        s->soundId = s->soundIdReload;
        ((BaddieState*)state)->moveSpeed = 0.0165f;
        ObjAnim_SetCurrentMove((int)obj, *(s16*)((char*)s->moveTable + 0x3a), 0.0f, 0);
        ObjAnim_SetCurrentEventStepFrames((struct ObjAnimComponent*)obj, 0x10);
        s->leapStartYaw = s->currentYaw;
        s->animSpeedRate =
            (0.2f +
             (*(f32*)((char*)s->configRow + 0x14) + ((BaddieState*)state)->animSpeedC)) /
            60.0f;
        s->appliedYaw = s->currentYaw;
        s->currentYaw += 0x8000;
        ((BaddieState*)state)->animSpeedC = -((BaddieState*)state)->animSpeedC;
        ((BaddieState*)state)->animSpeedA = -((BaddieState*)state)->animSpeedA;
    }
    if (((ByteFlags*)&s->flags3F0)->b80 != 0)
    {
        f32 lim;
        if (((BaddieState*)state)->animSpeedC <= (lim = *(f32*)((char*)s->configRow + 0x10)) &&
            ((BaddieState*)state)->animSpeedA <= lim)
        {
            s->savedYaw = s->currentYaw;
            ((ByteFlags*)&s->flags3F0)->b40 = 0;
            ((ByteFlags*)&s->flags3F0)->b80 = 0;
            return 1;
        }
        s->targetAnimSpeed = 0.0f;
        s->animSpeedSmoothing = s->animSpeedSmoothingReload;
        s->flags8D8 |= 8;
    }
    return 0;
}

static void DR_EarthWarrior_slowTurn(GameObject* obj, EarthWarriorSub* q, BaddieState* baddie)
{
    baddie->moveSpeed = 0.02f;
    q->yawSmoothDivisor *= 2.0f;
    q->yawStepScale *= 0.5f;
    q->targetAnimSpeed *= 0.75f;
    q->appliedYaw = (s16)(32768.0f * (obj)->anim.currentMoveProgress);
}

void DR_EarthWarrior_updateLookAtBones(GameObject* obj, int sub, int state)
{
    EarthWarriorSub* s = (EarthWarriorSub*)sub;
    int angle;
    int delta;
    s16* vec0;
    s16* vec9;
    f32 aimScale;
    angle = s->yawTurnDir << 1;
    if (angle < -0x41)
    {
        delta = -0x41;
    }
    else if (angle > 0x41)
    {
        delta = 0x41;
    }
    else
    {
        delta = angle;
    }
    delta = delta * 0xb6;
    delta -= (u16)s->aimAccumY;
    if (delta > 0x8000)
    {
        delta = delta - 0xffff;
    }
    if (delta < -0x8000)
    {
        delta = delta + 0xffff;
    }
    aimScale = 0.15f;
    delta = (int)((f32)delta * aimScale);
    delta = (delta < -0x16c) ? -0x16c : ((delta > 0x16c) ? 0x16c : delta);
    s->aimAccumY = delta * timeDelta + (f32)(s32)*(s16*)&s->aimAccumY;
    s->aimHalfY = s->aimAccumY / 2;
    {
        f32 step;
        f32 scale;
        f32 ph;

        ph = (f32)(s32)((BaddieState*)state)->spawnRotY / 8192.0f;
        scale = 182.0f;
        step = 10.0f;
        delta = (int)(scale * (step * -((ph < -1.0f) ? -1.0f : ((ph > 1.0f) ? 1.0f : ph))));
        delta -= (u16)s->aimAccumX;
    }
    if (delta > 0x8000)
    {
        delta = delta - 0xffff;
    }
    if (delta < -0x8000)
    {
        delta = delta + 0xffff;
    }
    s->aimAccumX += delta;
    vec0 = objModelGetVecFn_800395d8(obj, 0);
    vec9 = objModelGetVecFn_800395d8(obj, 9);
    objModelGetVecFn_800395d8(obj, 4);
    objModelGetVecFn_800395d8(obj, 5);
    if (vec0 != NULL)
    {
        int sv;
        vec0[0] = -s->aimAccumX;
        vec0[1] = s->aimAccumY / 2;
        sv = vec0[1];
        sv = (sv < -4000) ? -4000 : ((sv > 4000) ? 4000 : sv);
        vec0[1] = sv;
        vec0[2] = 0;
    }
    if (vec9 != NULL)
    {
        int sv;
        int t;
        vec9[1] = s->aimHalfY;
        sv = vec9[1];
        sv = (sv < -3000) ? -3000 : ((sv > 3000) ? 3000 : sv);
        vec9[1] = sv;
        t = s->aimHalfY;
        if (t < 0)
        {
            t = -t;
        }
        vec9[0] = (s16)(t >> 1);
    }
}
int DR_EarthWarrior_updateLeap(GameObject* obj, int sub, int state);
int DR_EarthWarrior_defaultStateHandler(void)
{
    return 0x0;
}

int DR_EarthWarrior_stateHandler03(GameObject* obj, int baddie)
{
    EarthWarriorState* inner = (obj)->extra;
    f32 fz;
    (obj)->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
    fz = 0.0f;
    ((BaddieState*)baddie)->animSpeedC = fz;
    ((BaddieState*)baddie)->animSpeedB = fz;
    ((BaddieState*)baddie)->animSpeedA = fz;
    (obj)->anim.velocityX = fz;
    (obj)->anim.velocityY = fz;
    (obj)->anim.velocityZ = fz;
    if (((BaddieState*)baddie)->moveJustStartedA != 0)
    {
        if (((ByteFlags*)&inner->sub.flags994)->b80)
        {
            ObjAnim_SetCurrentMove((int)obj, 7, fz, 0);
        }
        else
        {
            ObjAnim_SetCurrentMove((int)obj, 8, fz, 0);
        }
        ((BaddieState*)baddie)->moveSpeed = 0.02f;
    }
    if (((BaddieState*)baddie)->moveDone != 0)
    {
        if (inner->sub.rideState == 2)
        {
            inner->sub.health -= 1;
            if (inner->sub.health <= 0)
            {
                inner->sub.unk8EC = lbl_803DC76C;
                CameraShake_Enable();
                CameraShake_SetOffset(1.0f);
                playerAddHealth(Obj_GetPlayerObject(), -1);
                inner->sub.health = 0;
            }
            return inner->sub.savedControlMode + 1;
        }
    }
    return 0;
}

int DR_EarthWarrior_stateHandler02(GameObject* obj, int state)
{
    EarthWarriorState* inner = (obj)->extra;
    EarthWarriorSub* q = &inner->sub;
#define hitState ((ObjHitsPriorityState*)(obj)->anim.hitReactState)
    ((ByteFlags*)&q->flags3F1)->b04 = 0;
    ((ByteFlags*)&q->flags3F1)->b08 = 0;
    ((ByteFlags*)&q->flags3F2)->b10 = 0;
    if (((EarthWarriorState*)state)->baddie.moveJustStartedA != 0)
    {
        ((ByteFlags*)&q->flags3F0)->b80 = 0;
        ((ByteFlags*)&q->flags3F0)->b40 = 0;
        *(u8*)&q->attackPhase = 0;
        ((ByteFlags*)&q->flags3F2)->b10 = 1;
    }
    if (!((ByteFlags*)&q->flags3F0)->b80 && !((ByteFlags*)&q->flags3F0)->b40 &&
        !((ByteFlags*)&inner->sub.flags994)->b01 && (*(int*)&((EarthWarriorState*)state)->baddie.unk31C & 0x100))
    {
        buttonDisable(0, PAD_BUTTON_A);
        ((ByteFlags*)&inner->sub.flags994)->b01 = 1;
        hitState->suppressOutgoingHits = 0;
        ObjAnim_SetCurrentMove((int)obj, 0x14, 0.0f, 0);
        ((EarthWarriorState*)state)->baddie.moveDone = 0;
        Sfx_PlayFromObject((int)obj, SFXTRIG_earthhuff);
    }
    ((EarthWarriorState*)state)->baddie.flags0 |= 0x800000;
    ((EarthWarriorState*)state)->baddie.stateId = 0;
    q->animSpeedMax = 4.32f;
    if (((EarthWarriorState*)state)->baddie.moveJustStartedA != 0)
    {
        q->currentYaw += q->turnDegrees * 0xb6;
        q->frameCounter = 0;
        q->turnDegrees = 0;
    }
    {
        f32 a;
        f32 ph = (((BaddieState*)state)->inputMagnitude - 0.2f) / 0.8f;
        f32 t;
        a = q->animSpeedMax - 0.05f;
        t = (ph < 0.0f) ? 0.0f : ((ph > 1.0f) ? 1.0f : ph);
        q->targetAnimSpeed = a * (t * q->animSpeedScale);
    }
    if (((ByteFlags*)&q->flags3F0)->b40)
    {
        *(u32*)&q->unk360 |= 0x1000000LL;
        ((EarthWarriorState*)state)->baddie.moveSpeed = 0.0165f;
        {
            s16 yaw = (32768.0f * (obj)->anim.currentMoveProgress + (f32)q->leapStartYaw);
            *(s16*)&q->appliedYaw = yaw;
            q->savedYaw = yaw;
        }
        if (((EarthWarriorState*)state)->baddie.moveDone != 0)
        {
            s16 sw;
            ((ByteFlags*)&q->flags3F0)->b40 = 0;
            sw = q->currentYaw;
            q->appliedYaw = sw;
            q->savedYaw = sw;
            *(u8*)&q->attackPhase = 0xc;
            ((ByteFlags*)&q->flags3F1)->b04 = 1;
            ((ByteFlags*)&q->flags3F1)->b08 = 1;
        }
        ((EarthWarriorState*)state)->baddie.animSpeedC =
            q->animSpeedRate * timeDelta + ((EarthWarriorState*)state)->baddie.animSpeedC;
        q->targetAnimSpeed = 0.0f;
        if ((obj)->anim.currentMoveProgress > 0.1f && (obj)->anim.currentMoveProgress < 0.5f)
        {
            q->flags8D8 |= 8;
        }
    }
    else if (((ByteFlags*)&q->flags3F0)->b80)
    {
        if (DR_EarthWarrior_updateLeap(obj, (int)q, state) != 0)
        {
            return 2;
        }
    }
    else if (((ByteFlags*)&inner->sub.flags994)->b01)
    {
        ((EarthWarriorState*)state)->baddie.moveSpeed = 0.02f;
        if (((EarthWarriorState*)state)->baddie.moveDone != 0)
        {
            ((ByteFlags*)&inner->sub.flags994)->b01 = 0;
            ((ByteFlags*)&q->flags3F1)->b08 = 1;
            hitState->suppressOutgoingHits = 0;
        }
        {
            f32 m2;
            f32 m1;
            q->yawSmoothDivisor *= (m1 = 2.0f);
            q->yawStepScale *= (m2 = 0.5f);
            q->currentYawSmoothDivisor *= m1;
            q->currentYawStepRate *= m2;
        }
        q->targetAnimSpeed *= 0.75f;
        if (q->targetAnimSpeed < *(f32*)(q->configRow + 0xc))
        {
            q->targetAnimSpeed = *(f32*)(q->configRow + 0xc);
        }
        hitState->hitVolumePriority = 0x15;
        hitState->hitVolumeId = 2;
    }
    if (!((ByteFlags*)&inner->sub.flags994)->b01 && !((ByteFlags*)&q->flags3F0)->b40 &&
        !((ByteFlags*)&q->flags3F0)->b80 &&
        ((EarthWarriorState*)state)->baddie.animSpeedC >
            0.3f + *(f32*)(q->configRow + 0x14) &&
        (q->unk470 < -0.3f || q->frameCounter >= 0x96))
    {
        ((ByteFlags*)&q->flags3F0)->b80 = 1;
        *(u32*)&q->unk360 |= 0x1000000LL;
        q->animSpeedRate = ((EarthWarriorState*)state)->baddie.animSpeedA;
        ObjAnim_SetCurrentMove((int)obj, *(s16*)(q->moveTable + 0x3c), 0.0f, 0);
        ((EarthWarriorState*)state)->baddie.moveSpeed = 0.035f;
    }
    if (!((ByteFlags*)&q->flags3F0)->b80 && !((ByteFlags*)&q->flags3F0)->b40)
    {
        if (q->frameCounter < 0x96)
        {
            f32 v = interpolate((f32)q->yawTurnProgress,
                                1.0f / q->yawSmoothDivisor, timeDelta);
            f32 cap = timeDelta * (q->yawStepScale * q->yawStepRate);
            if (v > cap)
            {
                v = cap;
            }
            if (q->yawTurnDir < 0)
            {
                v = -v;
            }
            *(s16*)&q->appliedYaw =
                (182.044f * v + (f32)(s32)q->appliedYaw);
        }
        if (q->frameCounter < 0x96)
        {
            f32 v = interpolate((f32)q->frameCounter,
                                1.0f / q->currentYawSmoothDivisor, timeDelta);
            f32 cap = q->currentYawStepRate * timeDelta;
            if (v > cap)
            {
                v = cap;
            }
            if (q->turnDegrees < 0)
            {
                v = -v;
            }
            *(s16*)&q->currentYaw =
                (182.044f * v + (f32)(s32)q->currentYaw);
        }
        else if (((EarthWarriorState*)state)->baddie.animSpeedC <= *(f32*)(q->configRow + 0x4) &&
                 ((EarthWarriorState*)state)->baddie.animSpeedA <= *(f32*)(q->configRow + 0xc))
        {
            q->currentYaw += q->turnDegrees * 0xb6;
        }
    }
    if (!((ByteFlags*)&q->flags3F0)->b40 && !((ByteFlags*)&q->flags3F1)->b04)
    {
        f32 r = interpolate(q->targetAnimSpeed - ((EarthWarriorState*)state)->baddie.animSpeedC,
                            q->animSpeedSmoothing, timeDelta);
        r = (r < -0.1f * timeDelta) ? -0.1f * timeDelta
                                           : ((r > 0.1f * timeDelta) ? 0.1f * timeDelta : r);
        if (q->frameCounter >= 0x96 && r > 0.0f)
        {
            r = 2.0f * -r;
        }
        ((EarthWarriorState*)state)->baddie.animSpeedC += r;
        ((EarthWarriorState*)state)->baddie.animSpeedC =
            (((EarthWarriorState*)state)->baddie.animSpeedC < *(f32*)q->configRow)
                ? *(f32*)q->configRow
                : ((((EarthWarriorState*)state)->baddie.animSpeedC > q->animSpeedMax)
                       ? q->animSpeedMax
                       : ((EarthWarriorState*)state)->baddie.animSpeedC);
        ((EarthWarriorState*)state)->baddie.animSpeedB = 0.0f;
    }
    else
    {
        ((EarthWarriorState*)state)->baddie.animSpeedC =
            (((EarthWarriorState*)state)->baddie.animSpeedC < -q->animSpeedMax)
                ? -q->animSpeedMax
                : ((((EarthWarriorState*)state)->baddie.animSpeedC > q->animSpeedMax)
                       ? q->animSpeedMax
                       : ((EarthWarriorState*)state)->baddie.animSpeedC);
    }
    ((EarthWarriorState*)state)->baddie.animSpeedA +=
        interpolate(((EarthWarriorState*)state)->baddie.animSpeedC - ((EarthWarriorState*)state)->baddie.animSpeedA,
                    q->animSpeedASmoothing, timeDelta);
    if (!((ByteFlags*)&q->flags3F0)->b80 && !((ByteFlags*)&q->flags3F0)->b40 &&
        !((ByteFlags*)&inner->sub.flags994)->b01)
    {
        f32 blend;
        int i2;
        int skip = 0;
        if (((ByteFlags*)&q->flags3F1)->b08)
        {
            skip = 1;
            blend = 0.0f;
        }
        else
        {
            blend = (obj)->anim.currentMoveProgress;
        }
        i2 = (q->attackPhase / 4) << 1;
        q->attackStage = (i2 >> 1) + 1;
        if (q->attackStage > 4)
        {
            q->attackStage = 4;
        }
        q->soundId = (q->attackStage > 3) ? 0xa : 8;
        {
            f32 v294 = ((EarthWarriorState*)state)->baddie.animSpeedC;
            if (v294 < ((f32*)q->configRow)[i2])
            {
                if (q->attackPhase == 4)
                {
                    if (((EarthWarriorState*)state)->baddie.animSpeedA < *(f32*)(q->configRow + 0x10) &&
                        ((BaddieState*)state)->inputMagnitude < 0.2f)
                    {
                        return 2;
                    }
                }
                else
                {
                    q->attackPhase -= 4;
                }
            }
            else if (v294 >= *(f32*)((char*)(q->configRow + 4) + i2 * 4))
            {
                if (q->attackPhase < 0x14)
                {
                    if (q->attackPhase == 0)
                    {
                        blend = 0.85f;
                    }
                    if (v294 < q->animSpeedMax)
                    {
                        *(u8*)&q->attackPhase += 4;
                    }
                }
            }
        }
        if ((skip != 0 || (void*)q->prevMoveTable != (void*)q->moveTable ||
             (obj)->anim.currentMove !=
                 *(s16*)(q->moveTable + q->attackPhase * 2)) &&
            (ObjAnim_GetCurrentEventCountdown((ObjAnimComponent*)obj) == 0 ||
             ((ByteFlags*)&q->flags3F2)->b10 != 0))
        {
            if ((obj)->anim.currentMove == 0x14)
            {
                blend = 0.85f;
            }
            ObjAnim_SetCurrentMove(
                (int)obj, *(s16*)(q->moveTable + q->attackPhase * 2), blend, 0);
        }
    }
    if (!((ByteFlags*)&q->flags3F0)->b80 && !((ByteFlags*)&q->flags3F0)->b40 &&
        !((ByteFlags*)&inner->sub.flags994)->b01)
    {
        if (ObjAnim_SampleRootCurvePhase((ObjAnimComponent*)obj,
                                         ((EarthWarriorState*)state)->baddie.animSpeedC,
                                         (f32*)(state + 0x2a0)) == 0)
        {
            ((EarthWarriorState*)state)->baddie.moveSpeed = 0.005f;
        }
    }
    DR_EarthWarrior_updateLookAtBones(obj, (int)q, state);
    return 0;
}
#undef hitState

int DR_EarthWarrior_stateHandler01(GameObject* obj, int baddie)
{
    EarthWarriorState* inner = (obj)->extra;
    EarthWarriorSub* q = &inner->sub;
    int moveId;
    if (((BaddieState*)baddie)->moveJustStartedA != 0)
    {
        ((BaddieState*)baddie)->animSpeedC = 0.0f;
    }
    ((BaddieState*)baddie)->animSpeedA -=
        interpolate(((BaddieState*)baddie)->animSpeedA, q->animSpeedASmoothing, timeDelta);
    if (((BaddieState*)baddie)->animSpeedA <= *(f32*)((char*)lbl_8033527C + 0x8))
    {
        ((BaddieState*)baddie)->animSpeedA = 0.0f;
    }
    {
        f32 z = 0.0f;
        ((BaddieState*)baddie)->animSpeedB = z;
        (obj)->anim.velocityX = z;
        (obj)->anim.velocityZ = z;
    }
    if (!((ByteFlags*)&q->flags3F0)->b80 && !((ByteFlags*)&q->flags3F0)->b40 &&
        !((ByteFlags*)&inner->sub.flags994)->b01 && (*(int*)&((BaddieState*)baddie)->unk31C & 0x100))
    {
        buttonDisable(0, PAD_BUTTON_A);
        ((ByteFlags*)&inner->sub.flags994)->b01 = 1;
        ((ObjHitsPriorityState*)(obj)->anim.hitReactState)->suppressOutgoingHits = 0;
        ObjAnim_SetCurrentMove((int)obj, 0x14, 0.0f, 0);
        ((BaddieState*)baddie)->moveDone = 0;
        return 3;
    }
    if (*(f32*)&((EarthWarriorState*)baddie)->baddie.trackedObj >= 0.22f &&
        ((BaddieState*)baddie)->inputMagnitude >= 0.22f &&
        ((BaddieState*)baddie)->animSpeedC >= *(f32*)(q->configRow + 0x4))
    {
        return 3;
    }
    moveId = *(s16*)q->moveTable;
    ((BaddieState*)baddie)->stateId = 0;
    q->animSpeedMax = 4.32f;
    {
        f32 a;
        f32 ph = (((BaddieState*)baddie)->inputMagnitude - 0.2f) / 0.8f;
        f32 t;
        a = q->animSpeedMax - 0.05f;
        t = (ph < 0.0f) ? 0.0f : ((ph > 1.0f) ? 1.0f : ph);
        q->targetAnimSpeed = a * (t * q->animSpeedScale);
    }
    ((BaddieState*)baddie)->animSpeedC +=
        interpolate(q->targetAnimSpeed - ((BaddieState*)baddie)->animSpeedC, q->animSpeedSmoothing, timeDelta);
    if (((BaddieState*)baddie)->moveJustStartedA != 0)
    {
        q->yawTurnProgress = 0;
        q->yawTurnDir = 0;
        q->frameCounter = 0;
        q->turnDegrees = 0;
        q->soundId = 8;
        q->attackStage = 0;
        ((BaddieState*)baddie)->velSmoothTime = 8.0f;
        ((BaddieState*)baddie)->moveSpeed = 0.005f;
    }
    if ((obj)->anim.currentMove == *(s16*)(q->moveTable + 0x30) ||
        (obj)->anim.currentMove == *(s16*)(q->moveTable + 0x32))
    {
        if (((BaddieState*)baddie)->moveDone != 0 &&
            ObjAnim_GetCurrentEventCountdown((ObjAnimComponent*)obj) == 0 &&
            !((ByteFlags*)&inner->sub.flags994)->b01)
        {
            ObjAnim_SetCurrentMove((int)obj, moveId, 0.0f, 0);
            ((BaddieState*)baddie)->moveSpeed = 0.005f;
        }
    }
    else if (!((ByteFlags*)&inner->sub.flags994)->b01)
    {
        ObjAnim_SetCurrentMove((int)obj, moveId, 0.0f, 0);
        ((BaddieState*)baddie)->moveSpeed = 0.005f;
    }
    {
        f32 v = interpolate((f32)q->yawTurnProgress, 1.0f / q->yawSmoothDivisor, timeDelta);
        f32 cap = timeDelta * (q->yawStepScale * q->yawStepRate);
        v = (v < cap) ? v : cap;
        if (q->yawTurnDir < 0)
        {
            v = -v;
        }
        *(s16*)&q->appliedYaw = (182.044f * v + (f32)(s32)q->appliedYaw);
    }
    {
        f32 v = interpolate((f32)q->frameCounter, 1.0f / q->currentYawSmoothDivisor, timeDelta);
        f32 cap = q->currentYawStepRate * timeDelta;
        v = (v < cap) ? v : cap;
        if (q->turnDegrees < 0)
        {
            v = -v;
        }
        *(s16*)&q->currentYaw = (182.044f * v + (f32)(s32)q->currentYaw);
    }
    DR_EarthWarrior_updateLookAtBones(obj, (int)q, baddie);
    return 0;
}

int DR_EarthWarrior_stateHandler00(GameObject* obj)
{
    EarthWarriorState* inner = obj->extra;
    inner->sub.flags98C |= 0x20;
    return 2;
}

int DR_EarthWarrior_SeqFn(GameObject* obj, int unused, ObjAnimUpdateState* animUpdate)
{
    EarthWarriorState* inner = (obj)->extra;
    int i;
    f32 fz;
    (obj)->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
    if (dll_2E_updateSequenceTurn(obj, (ObjSeqState*)animUpdate, &inner->moveLib, 0, 0) != 0)
    {
        return 1;
    }
    for (i = 0; i < animUpdate->eventCount; i++)
    {
        int eventId = animUpdate->eventIds[i];
        switch (eventId)
        {
        case 0xa:
            break;
        case 0xe:
        case 0xf:
            inner->moveLib.modeBits |= 1;
            ((ObjHitsPriorityState*)(obj)->anim.hitReactState)->shapeFlags &= ~0x20;
            break;
        case 0x10:
            inner->moveLib.modeBits &= ~1;
            ((ObjHitsPriorityState*)(obj)->anim.hitReactState)->shapeFlags |= 0x20;
            break;
        }
    }
    *(u32*)&inner->sub.unk360 |= 0x800000LL;
    (*gPathControlInterface)->attachObject((void*)obj, (char*)&inner->baddie + 4);
    fz = 0.0f;
    inner->baddie.animSpeedC = fz;
    inner->baddie.animSpeedB = fz;
    inner->baddie.animSpeedA = fz;
    (obj)->anim.velocityX = fz;
    (obj)->anim.velocityY = fz;
    (obj)->anim.velocityZ = fz;
    return 0;
}

void DR_EarthWarrior_handleRiderScale(GameObject* obj, f32 scale)
{
    MatrixTransform v;
    f32 lp0, lp1, lp2;
    int mtx = ObjPath_GetPointModelMtx(obj, 2);
    ObjPath_GetPointLocalPosition(obj, 2, &lp0, &lp1, &lp2);
    v.x = lp0;
    v.y = lp1;
    v.z = lp2;
    v.rotX = 0;
    v.rotY = 0;
    v.rotZ = 0;
    v.scale = scale / (obj)->anim.modelInstance->rootMotionScaleBase;
    setMatrixFromObjectPos(gEarthWarriorMatrix, &v);
    mtx44_mult(gEarthWarriorMatrix, (void*)mtx, gEarthWarriorMatrix);
    objSetModelMatrixOverride(gEarthWarriorMatrix);
}

void DR_EarthWarrior_resetToRomListPosition(void)
{
}

int DR_EarthWarrior_getRacePosition(void)
{
    return 0x0;
}

f32 DR_EarthWarrior_func19(GameObject* obj, f32* out)
{
    EarthWarriorState* inner = obj->extra;
    f32 animSpeed;
    animSpeed = 0.001f * inner->baddie.animSpeedC + 0.005f;
    *out = -((animSpeed < 0.005f) ? 0.005f : ((animSpeed > 0.01f) ? 0.01f : animSpeed));
    return 0.0f;
}

void DR_EarthWarrior_getPlayerAnim(GameObject* obj, f32* a, int* b)
{
    EarthWarriorState* inner = obj->extra;
    *a = (f32)(s32)inner->sub.aimAccumY;
    *b = inner->sub.aimAccumX;
}

void DR_EarthWarrior_setMountState(GameObject* obj, int param)
{
    EarthWarriorState* inner = obj->extra;
    inner->sub.rideState = param;
    if (param == 0)
    {
        mainSetBits(0x7bc, 0);
        mainSetBits(0x7d4, 1);
        inner->moveLib.modeBits &= ~1;
        ((ByteFlags*)&inner->sub.flags994)->b02 = 0;
        (*gGameUIInterface)->airMeterShutdown();
    }
    else
    {
        EarthWarriorState* inner2 = obj->extra;
        DREarthWarriorPlacement* placement = (DREarthWarriorPlacement*)obj->anim.placementData;
        ((ByteFlags*)&inner2->sub.flags994)->b02 = 1;
        (*gGameUIInterface)
            ->initAirMeter(placement->airMeterMax, DREARTHWARRIOR_AIRMETER_BGTEXTURE);
        (*gGameUIInterface)->runAirMeter(inner2->sub.health);
        mainSetBits(0x7bc, 1);
        mainSetBits(0x7d4, 0);
    }
}

int DR_EarthWarrior_getMountState(void)
{
    return 0x0;
}

void DR_EarthWarrior_getCameraPosition(GameObject* obj, f32* x, f32* y, f32* z)
{
    *x = obj->anim.localPosX;
    *y = obj->anim.localPosY;
    *z = obj->anim.localPosZ;
}

int DR_EarthWarrior_getDismountSide(GameObject* obj)
{
    EarthWarriorState* inner = obj->extra;
    if (inner->sub.unk992 != 0)
    {
        return 2;
    }
    return 1;
}

int DR_EarthWarrior_canDismount(void)
{
    return 0x0;
}

void DR_EarthWarrior_getRiderPosition(GameObject* obj, f32* x, f32* y, f32* z)
{
    EarthWarriorState* inner = obj->extra;
    *x = inner->sub.posX;
    *y = inner->sub.posY;
    *z = inner->sub.posZ;
}

int DR_EarthWarrior_getMountSide(GameObject* obj)
{
    EarthWarriorState* inner = obj->extra;
    if (inner->sub.unk993 != 0)
    {
        return 1;
    }
    return 2;
}

int DR_EarthWarrior_canMount(void)
{
    return 0x0;
}

int DR_EarthWarrior_getExtraSize(void)
{
    return 0x14fc;
}

int DR_EarthWarrior_getObjectTypeId(void)
{
    return 0x43;
}

void DR_EarthWarrior_free(GameObject* obj)
{
    EarthWarriorState* inner = (obj)->extra;
    if (inner->sub.modelChain != NULL)
    {
        ObjModelChain_Free(inner->sub.modelChain);
    }
    ObjGroup_RemoveObject((int)obj, DREARTHWARRIOR_OBJGROUP);
    if (((ByteFlags*)&inner->sub.flags994)->b02)
    {
        (*gGameUIInterface)->airMeterShutdown();
    }
    if (inner->helperObj != NULL)
    {
        ObjLink_DetachChild(obj, inner->helperObj);
        Obj_FreeObject(inner->helperObj);
    }
}

void DR_EarthWarrior_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 vis)
{
    EarthWarriorState* inner = (obj)->extra;
    if (vis == -1)
    {
        objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, 1.0f);
        ObjPath_GetPointWorldPosition(obj, 0xb, &inner->sub.posX, &inner->sub.posY, &inner->sub.posZ, 0);
        ObjPath_GetPointWorldPositionArray(obj, 3, 4, (f32*)inner->pathPoints);
    }
    else if (vis != 0)
    {
        objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, 1.0f);
        ObjPath_GetPointWorldPosition(obj, 0xb, &inner->sub.posX, &inner->sub.posY, &inner->sub.posZ, 0);
        ObjPath_GetPointWorldPositionArray(obj, 3, 4, (f32*)inner->pathPoints);
        dll_2E_setTargetFromPathPoint(obj, &inner->moveLib, 0);
    }
}

void DR_EarthWarrior_hitDetect(GameObject* obj)
{
    f32 hz;
    f32 hy;
    f32 hx;
    void* hitObj;
    struct
    {
        s16 angles[4];
        f32 mat[4];
    } v;
    EWColorTbl rows;
    EarthWarriorState* inner = obj->extra;
    ObjHitsPriorityState* hitState = (ObjHitsPriorityState*)obj->anim.hitReactState;
    rows = gDREarthWarriorColors;
    if (!(obj->objectFlags & DREARTHWARRIOR_OBJFLAG_PARENT_SLACK))
    {
        if (hitState->contactFlags != 0)
        {
            int i = hitState->contactHitVolume;
            i = (i < 0) ? 0 : ((i > 0x23) ? 0x23 : i);
            v.mat[0] = 1.0f;
            v.angles[2] = 0;
            v.angles[1] = 0;
            v.angles[0] = 0;
            v.mat[1] = hitState->contactPosX;
            v.mat[2] = hitState->contactPosY;
            v.mat[3] = hitState->contactPosZ;
            (*(void (*)(int, int, void*, int, int, void*))(*(int*)(*(int*)gEarthWarriorResource + 0x4)))(
                0, 1, &v, 0x401, -1, rows.m[gDREarthWarriorRowIndices[i]]);
            ((ObjHitsPriorityState*)obj->anim.hitReactState)->suppressOutgoingHits = 1;
            doRumble(10.0f);
        }
        if (hitState->lastHitObject != 0)
        {
            doRumble(10.0f);
        }
        obj->anim.rotX = inner->sub.appliedYaw;
        if (inner->baddie.controlMode != 3)
        {
            int hit = ObjHits_GetPriorityHitWithPosition(obj, (int*)&hitObj, 0, 0, &hx, &hy, &hz);
            if (hit != 0)
            {
                if (objGetFlagsE5_2((u8*)obj) != 0 && inner->sub.rideState == 2)
                {
                    return;
                }
                Obj_SpawnHitLightAndFade(obj, (const Vec3f*)&hx, 5.0f);
                if (hit == 0x1a || hitObj == Obj_GetPlayerObject() || ((GameObject*)hitObj)->anim.seqId == DREARTHWARRIOR_ATTACKER_SEQID_SWORD)
                {
                    return;
                }
                {
                    objSoundStartTimed(obj, &inner->modelSoundState, 0x28e, 0x1000, -1, 1);
                    {
                        s16 d = obj->anim.rotX - (u16)((GameObject*)hitObj)->anim.rotX;
                        if (d > 0x8000)
                        {
                            d = (s16)(d - 0xffff);
                        }
                        if (d < -0x8000)
                        {
                            d += 0xffff;
                        }
                        if (d > 0x4000 || d < -0x4000)
                        {
                            ((ByteFlags*)&inner->sub.flags994)->b80 = 0;
                        }
                        else
                        {
                            ((ByteFlags*)&inner->sub.flags994)->b80 = 1;
                        }
                    }
                    inner->sub.savedControlMode = inner->baddie.controlMode;
                    (*gPlayerInterface)->setState(obj, inner, 3);
                }
            }
        }
        if (*(int*)inner & 0x800000)
        {
            if ((*(u8*)((char*)inner + 0x262) != 0 || (*(s8*)((char*)inner + 0x264) & 0xf0)) &&
                inner->sub.footstepCooldown <= 0.0f && inner->baddie.animSpeedA > 3.408f)
            {
                doRumble((f32)(int)randomGetRange(2, 5));
                inner->sub.footstepCooldown = 30.0f;
                Sfx_PlayFromObject((int)obj, SFXTRIG_foot_run_jingle4);
            }
            if (*(u8*)((char*)inner + 0x262) != 0 ||
                (((ObjHitsPriorityState*)obj->anim.hitReactState)->flags & 8))
            {
                f32 spd;
                f32 vcos;
                f32 vsin;
                spd = sqrtf(obj->anim.velocityX * obj->anim.velocityX +
                            obj->anim.velocityZ * obj->anim.velocityZ);
                obj->anim.velocityX = oneOverTimeDelta * (obj->anim.worldPosX -
                                                                         obj->anim.previousWorldPosX);
                obj->anim.velocityZ = oneOverTimeDelta * (obj->anim.worldPosZ -
                                                                         obj->anim.previousWorldPosZ);
                vcos = mathSinf((3.1415927f * (f32)(s32)inner->sub.currentYaw) / 32768.0f);
                vsin = mathCosf((3.1415927f * (f32)(s32)inner->sub.currentYaw) / 32768.0f);
                inner->baddie.animSpeedA =
                    -obj->anim.velocityZ * vsin - obj->anim.velocityX * vcos;
                inner->baddie.animSpeedA *= 2.0f;
                inner->baddie.animSpeedA =
                    (inner->baddie.animSpeedA < 1.2960001f)
                        ? 1.2960001f
                        : ((inner->baddie.animSpeedA > inner->sub.animSpeedMax) ? inner->sub.animSpeedMax
                                                                                : inner->baddie.animSpeedA);
                inner->baddie.animSpeedA = (inner->baddie.animSpeedA < 0.0f)
                                               ? 0.0f
                                               : ((inner->baddie.animSpeedA > spd) ? spd : inner->baddie.animSpeedA);
                if (!((ByteFlags*)&inner->sub.flags3F0)->b40)
                {
                    inner->baddie.animSpeedC = inner->baddie.animSpeedA;
                }
            }
            inner->baddie.flags0 &= ~0x800000;
        }
        inner->sub.footstepCooldown -= timeDelta;
        if (inner->sub.footstepCooldown < 0.0f)
        {
            inner->sub.footstepCooldown = 0.0f;
        }
        if ((void*)inner != NULL)
        {
            ObjModelChain_AdvancePhase(inner->sub.modelChain);
        }
    }
}

void DR_EarthWarrior_runController(GameObject* obj, int t, int p3)
{
    int inner = *(int*)&obj->extra;
    int sub;
    int slot;
    Obj_GetPlayerObject();
    sub = inner + 0xb58;
    slot = (int)Camera_GetCurrent();
    ((EarthWarriorState*)inner)->baddie.hitPoints = 0;
    ((EarthWarriorState*)inner)->baddie.flags0 &= ~0x8000;
    if (((EarthWarriorState*)inner)->sub.rideState == 2)
    {
        ((EarthWarriorState*)inner)->baddie.moveInputX = (f32)padGetStickX(0);
        ((EarthWarriorState*)inner)->baddie.moveInputZ = (f32)padGetStickY(0);
        *(int*)&((EarthWarriorState*)inner)->baddie.unk31C = getButtonsJustPressed(0);
        *(int*)&((EarthWarriorState*)inner)->baddie.unk318 = getButtonsHeld(0);
        ((EarthWarriorState*)inner)->baddie.cameraYaw = *(s16*)slot;
    }
    else
    {
        f32 v = 0.0f;
        ((EarthWarriorState*)inner)->baddie.moveInputX = v;
        ((EarthWarriorState*)inner)->baddie.moveInputZ = v;
        *(int*)&((EarthWarriorState*)inner)->baddie.unk31C = 0;
        *(int*)&((EarthWarriorState*)inner)->baddie.unk318 = 0;
        ((EarthWarriorState*)inner)->baddie.cameraYaw = 0;
    }
    ((EarthWarriorState*)inner)->baddie.flags0 |= 0x1000000;
    fn_802B0EA4(obj, sub, inner);
    (*gPlayerInterface)->update(obj, (void*)inner, timeDelta, timeDelta, gDREarthWarriorStateHandlers,
                                &gDREarthWarriorDefaultStateHandler);
    obj->anim.rotY =
        (s16)(obj->anim.rotY + (((EarthWarriorState*)inner)->baddie.spawnRotY >> 2));
    obj->anim.rotZ =
        (s16)(obj->anim.rotZ + (((EarthWarriorState*)inner)->baddie.spawnRotZ >> 2));
    if (((ByteFlags*)&((EarthWarriorState*)inner)->sub.flags994)->b02)
    {
        (*gGameUIInterface)->runAirMeter(((EarthWarriorState*)inner)->sub.health);
    }
    fn_802B1BF8(obj, sub, inner, timeDelta);
    fn_802B1B28(obj, timeDelta);
    (*gPathControlInterface)->update((void*)obj, (void*)(inner + 4), timeDelta);
    (*gPathControlInterface)->apply((void*)obj, (void*)(inner + 4));
    (*gPathControlInterface)->advance((void*)obj, (void*)(inner + 4), timeDelta);
    obj->anim.rotX = ((EarthWarriorSub*)sub)->appliedYaw;
}

void DR_EarthWarrior_update(GameObject* obj)
{
    EarthWarriorState* inner = (obj)->extra;
    int j;
    int i;
#define hitState ((ObjHitsPriorityState*)(obj)->anim.hitReactState)
    Obj_GetPlayerObject();
    hitState->hitVolumePriority = 0;
    hitState->hitVolumeId = 0;
    if (inner->helperObj == NULL && Obj_IsLoadingLocked() != 0)
    {
        ObjPlacement* setup = Obj_AllocObjectSetup(0x18, DREARTHWARRIOR_CHILD_OBJ_HELPER);
        GameObject* newObj = Obj_SetupObject(setup, 4, (obj)->anim.mapEventSlot, -1, (obj)->anim.parent);
        ObjLink_AttachChild(obj, newObj, 2);
        inner->helperObj = newObj;
    }
    inner->sub.unk986 = 5;
    (obj)->anim.resetHitboxFlags &= ~INTERACT_FLAG_DISABLED;
    if (inner->sub.rideState == 2)
    {
        setAButtonIcon(0x13);
        (obj)->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
        hitState->lateralResponseWeight = 0xf4;
        hitState->axialResponseWeight = 0xf4;
        DR_EarthWarrior_runController(obj, timeDelta, -1);
    }
    else
    {
        f32 z;
        hitState->lateralResponseWeight = 0;
        hitState->axialResponseWeight = 0;
        z = 0.0f;
        inner->baddie.animSpeedC = z;
        inner->baddie.animSpeedB = z;
        inner->baddie.animSpeedA = z;
        (obj)->anim.velocityX = z;
        (obj)->anim.velocityY = z;
        (obj)->anim.velocityZ = z;
        DR_EarthWarrior_runController(obj, framesThisStep, -1);
    }
    characterDoEyeAnims(obj, &inner->eyeAnimState);
    objSoundUpdateMouth(obj, &inner->modelSoundState);
    dll_2E_updateLookAt(obj, &inner->moveLib);
    if ((obj)->anim.resetHitboxFlags & INTERACT_FLAG_ACTIVATED)
    {
        ((ByteFlags*)&inner->sub.flags994)->b10 = 1;
        if ((*gGameUIInterface)->isItemBeingUsed(0xc1) != 0)
        {
            (*gObjectTriggerInterface)->runSequence(1, (void*)obj, -1);
            buttonDisable(0, PAD_BUTTON_A);
            inner->sub.health += 4;
            mainSetBits(GAMEBIT_ITEM_TrickyFood_Count, mainGetBit(GAMEBIT_ITEM_TrickyFood_Count) - 1);
        }
        else if (inner->sub.interactSequenceId != -1)
        {
            if ((*gGameUIInterface)->isAnyItemBeingUsed() == 0)
            {
                if (((ByteFlags*)&inner->sub.flags994)->b08 == 0)
                {
                    (*gObjectTriggerInterface)->runSequence(inner->sub.interactSequenceId, (void*)obj, -1);
                    buttonDisable(0, PAD_BUTTON_A);
                }
                else
                {
                    ((ByteFlags*)&inner->sub.flags994)->b10 = 1;
                }
            }
        }
    }
    *(s8*)((char*)inner + 0x264) |= 0x10;
    {
        f32 saved = (obj)->anim.velocityY;
        (obj)->anim.velocityY = 0.0f;
        inner->baddie.eventFlags &= ~7;
        objAudioFn_8006edcc(obj, inner->baddie.eventFlags, inner->sub.soundId, inner->pathPoints,
                            (void*)((char*)inner + 0x4), inner->baddie.animSpeedA,
                            (inner->sub.soundId == 8) ? 2.5f : 2.75f);
        (obj)->anim.velocityY = saved;
    }
    if (inner->sub.flags8D8 & 8)
    {
        f32 vecA[3];
        struct
        {
            s16 angles[4];
            f32 mat[4];
        } w;
        vecA[0] = 0.05f * (obj)->anim.velocityX;
        vecA[1] = 0.0f;
        vecA[2] = 0.05f * (obj)->anim.velocityZ;
        for (i = 0; i < 4; i++)
        {
            w.mat[1] = 8.0f * (obj)->anim.velocityX + inner->pathPoints[i].x;
            w.mat[2] = inner->pathPoints[i].y;
            w.mat[3] = 8.0f * (obj)->anim.velocityZ + inner->pathPoints[i].z;
            w.mat[0] = 1.0f;
            w.angles[0] = 2;
            for (j = 2; j != 0; j--)
            {
                (*gPartfxInterface)->spawnObject((void*)obj, DREARTHWARRIOR_PARTFX, &w, 0x200001, -1, vecA);
            }
        }
        inner->sub.flags8D8 &= ~8;
    }
#undef hitState
}
void DR_EarthWarrior_init(GameObject* obj, DREarthWarriorPlacement* def)
{
    DREarthWarriorInitData* base = (DREarthWarriorInitData*)gDREarthWarriorInitData;
    EarthWarriorState* inner = (obj)->extra;
    u32 stk = *(const u32*)gDREarthWarriorPathSetupParam;
    EWPathRange r2 = gDREarthWarriorLookInitData1;
    EWPathRange r1 = gDREarthWarriorLookInitData2;
    u8* pathState;
    (obj)->anim.rotX = (s16)(def->spawnYaw << 8);
    (obj)->animEventCallback = DR_EarthWarrior_SeqFn;
    ObjGroup_AddObject((int)obj, DREARTHWARRIOR_OBJGROUP);
    inner->sub.unk990 = def->unk19;
    inner->sub.unk986 = 5;
    inner->sub.interactSequenceId = -1;
    (*gPlayerInterface)->init(obj, inner, 4, 1);
    inner->baddie.flags0 |= 0x4000;
    inner->baddie.gravity = 0.17f;
    pathState = (u8*)&inner->baddie + 4;
    (*gPathControlInterface)->init(pathState, 0, 0x48683, 1);
    (*gPathControlInterface)->setup(pathState, 4, base->unkC, base->unk3C, &stk);
    (*gPathControlInterface)->setLocalPointCollision(pathState, 1, base->unk4C, base->unk64, 8);
    pathState[0x264] = 0x28;
    (*gPathControlInterface)->attachObject((void*)obj, pathState);
    ObjHits_EnableObject(obj);
    ((ObjHitsPriorityState*)(obj)->anim.hitReactState)->trackContactMask = 9;
    dll_2E_initState(obj, &inner->moveLib, -0x2000, 0x31c7, 2);
    dll_2E_setMoveTables(&inner->moveLib, &r1, &r2, 2);
    dll_2E_setLookAtMaxDistance(&inner->moveLib, 150.0f);
    inner->moveLib.modeBits |= 2;
    inner->sub.unk8EC = 4.32f;
    inner->sub.health = def->airMeterMax;
    inner->sub.moveTable = (int)base->unkD8;
    inner->sub.configRow = (int)base->unk84;
    {
        f32 v = 1.0f;
        inner->sub.unk834 = v;
        inner->sub.animSpeedASmoothing = v;
    }
    inner->sub.animSpeedSmoothingReload = 0.06f;
    inner->sub.unk450 = base->unk118;
    inner->sub.unk8D0 = 0x29;
    inner->sub.unk454 = base->unk1BC;
    inner->sub.unk8D1 = 0x29;
    inner->sub.unk458 = base->unk260;
    inner->sub.unk8D2 = 0x2e;
    inner->sub.unk45C = base->unk1BC;
    inner->sub.unk8D3 = 0x29;
    inner->sub.unk460 = base->unk260;
    inner->sub.unk8D4 = 0x2e;
    inner->sub.unk7E0 = 5.555f;
    {
        s16 h = (obj)->anim.rotX;
        inner->sub.savedYaw = h;
        inner->sub.unk474 = h;
        inner->sub.currentYaw = h;
        inner->sub.appliedYaw = h;
    }
    ((ByteFlags*)&inner->sub.flags994)->b08 = 0;
    inner->sub.interactSequenceId = 2;
    storeZeroToFloatParam(&inner->sub.airMeterTimer);
    s16toFloat(&inner->sub.airMeterTimer, 0x1e);
    ((ByteFlags*)&inner->sub.flags994)->b02 = 0;
    inner->sub.unk99D = 1;
    inner->helperObj = NULL;
    if (mainGetBit(0x9ec) != 0)
    {
        inner->sub.unk995 = 1;
    }
    inner->sub.modelChain = ObjModelChain_Alloc(&gEarthWarriorTailChainDesc, 1);
    ObjModelChain_SetOrigin(inner->sub.modelChain, 0.15f, 0.75f, -0.05f);
    *(int*)((char*)obj + 0x108) = (int)dim2prisonmammoth_updateModelChain;
    ObjModelChain_SetEnabled(inner->sub.modelChain, 1);
}

void DR_EarthWarrior_release(void)
{
    if (gEarthWarriorResource != NULL)
    {
        Resource_Release(gEarthWarriorResource);
        gEarthWarriorResource = NULL;
    }
}

void DR_EarthWarrior_initialise(void)
{
    ((void**)gDREarthWarriorStateHandlers)[0] = DR_EarthWarrior_stateHandler00;
    ((void**)gDREarthWarriorStateHandlers)[1] = DR_EarthWarrior_stateHandler01;
    ((void**)gDREarthWarriorStateHandlers)[2] = DR_EarthWarrior_stateHandler02;
    ((void**)gDREarthWarriorStateHandlers)[3] = DR_EarthWarrior_stateHandler03;
    gDREarthWarriorDefaultStateHandler = DR_EarthWarrior_defaultStateHandler;
    if (gEarthWarriorResource == NULL)
    {
        gEarthWarriorResource = Resource_Acquire(DREARTHWARRIOR_EFFECT_RESOURCE_ID, 1);
    }
}

u8 gDREarthWarriorInitData[132] = {
    0x02, 0x8F, 0x08, 0x00, 0x01, 0x00, 0x02, 0x90, 0x10, 0x00, 0x03, 0x00, 0xC1, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xC1, 0x40, 0x00, 0x00, 0x41, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC1, 0x40, 0x00, 0x00, 0x41, 0x10,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x41, 0x40, 0x00, 0x00, 0xC1, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x41,
    0x40, 0x00, 0x00, 0x3D, 0xCC, 0xCC, 0xCD, 0x3D, 0xCC, 0xCC, 0xCD, 0x3D, 0xCC, 0xCC, 0xCD, 0x3D, 0xCC, 0xCC, 0xCD,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xC2, 0x0C, 0x00, 0x00, 0x42, 0x0C, 0x00, 0x00, 0x40, 0xA0, 0x00, 0x00, 0x40, 0xA0, 0x00, 0x00, 0x40, 0xA0,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x0A,
};

int lbl_8033527C[12] = {
    0x3BA3D70A, 0x3E75C290, 0x3E449BA6, 0x3FA5E355, 0x3F9FBE77, 0x4010624E,
    0x400D4FE0, 0x405A1CAC, 0x40570A3E, 0x408A3D71, 0x4089999A, 0x408A3D71,
};

u8 gDREarthWarriorRowIndices[960] = {
    0,  0,   0,   0,   0,  0,   0,   0,   0,  0,   0,   0,   0,  0,   0,   0,   0,  0,   0,   0,   0,  0,   0,   0,
    1,  2,   0,   0,   0,  0,   0,   0,   0,  0,   0,   0,   0,  2,   0,   2,   0,  2,   0,   2,   0,  22,  0,   22,
    0,  22,  0,   22,  0,  22,  0,   22,  0,  22,  0,   22,  0,  22,  0,   22,  0,  22,  0,   22,  0,  4,   0,   4,
    0,  4,   0,   4,   0,  4,   0,   4,   0,  4,   0,   4,   0,  2,   0,   2,   0,  2,   0,   2,   0,  2,   0,   28,
    0,  27,  0,   2,   65, 64,  0,   0,   65, 64,  0,   0,   65, 64,  0,   0,   65, 64,  0,   0,   65, 64,  0,   0,
    65, 64,  0,   0,   65, 64,  0,   0,   65, 64,  0,   0,   65, 64,  0,   0,   65, 64,  0,   0,   65, 64,  0,   0,
    65, 64,  0,   0,   65, 64,  0,   0,   65, 64,  0,   0,   65, 64,  0,   0,   65, 64,  0,   0,   65, 64,  0,   0,
    65, 64,  0,   0,   65, 80,  0,   0,   65, 128, 0,   0,   66, 0,   0,   0,   66, 0,   0,   0,   66, 0,   0,   0,
    66, 0,   0,   0,   66, 0,   0,   0,   66, 0,   0,   0,   66, 0,   0,   0,   66, 0,   0,   0,   66, 0,   0,   0,
    66, 0,   0,   0,   66, 0,   0,   0,   66, 0,   0,   0,   66, 0,   0,   0,   66, 0,   0,   0,   66, 0,   0,   0,
    66, 0,   0,   0,   66, 0,   0,   0,   66, 0,   0,   0,   66, 0,   0,   0,   66, 0,   0,   0,   66, 0,   0,   0,
    65, 128, 0,   0,   65, 128, 0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,
    65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,
    65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,
    65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,
    65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,
    65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,
    65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   65, 32,  0,   0,   64, 224, 0,   0,
    64, 224, 0,   0,   64, 224, 0,   0,   64, 224, 0,   0,   64, 224, 0,   0,   64, 224, 0,   0,   64, 224, 0,   0,
    64, 224, 0,   0,   64, 224, 0,   0,   64, 224, 0,   0,   64, 224, 0,   0,   64, 224, 0,   0,   64, 224, 0,   0,
    64, 208, 0,   0,   64, 192, 0,   0,   64, 176, 0,   0,   64, 160, 0,   0,   64, 153, 153, 154, 64, 128, 0,   0,
    64, 102, 102, 102, 64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154,
    64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154,
    64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154,
    64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154,
    64, 89,  153, 154, 64, 89,  153, 154, 64, 89,  153, 154, 65, 0,   0,   0,   65, 0,   0,   0,   64, 160, 0,   0,
    64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,
    64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,
    64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,
    64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,
    64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,
    64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,   64, 160, 0,   0,
    64, 160, 0,   0,   64, 160, 0,   0,   65, 96,  0,   0,   65, 96,  0,   0,   65, 96,  0,   0,   65, 96,  0,   0,
    65, 96,  0,   0,   65, 96,  0,   0,   65, 96,  0,   0,   65, 96,  0,   0,   65, 96,  0,   0,   65, 96,  0,   0,
    65, 96,  0,   0,   65, 96,  0,   0,   65, 96,  0,   0,   65, 80,  0,   0,   65, 64,  0,   0,   65, 48,  0,   0,
    65, 32,  0,   0,   65, 25,  153, 154, 65, 0,   0,   0,   64, 230, 102, 102, 64, 217, 153, 154, 64, 217, 153, 154,
    64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154,
    64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154,
    64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154,
    64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154, 64, 217, 153, 154,
};

s32 lbl_8033566C[4] = {0x17, 0x18, 0x19, 0x1A};

ObjModelChainDesc lbl_803DC760 = {lbl_8033566C, 4};
ObjModelChainDesc* gEarthWarriorTailChainDesc = &lbl_803DC760;

ObjectDescriptor24WithPadding gDR_EarthWarriorObjDescriptor = {
    {
        0,
        0,
        0,
        OBJECT_DESCRIPTOR_FLAGS_24_SLOTS,
        (ObjectDescriptorCallback)DR_EarthWarrior_initialise,
        (ObjectDescriptorCallback)DR_EarthWarrior_release,
        0,
        (ObjectDescriptorCallback)DR_EarthWarrior_init,
        (ObjectDescriptorCallback)DR_EarthWarrior_update,
        (ObjectDescriptorCallback)DR_EarthWarrior_hitDetect,
        (ObjectDescriptorCallback)DR_EarthWarrior_render,
        (ObjectDescriptorCallback)DR_EarthWarrior_free,
        (ObjectDescriptorCallback)DR_EarthWarrior_getObjectTypeId,
        (ObjectDescriptorExtraSizeCallback)DR_EarthWarrior_getExtraSize,
        (ObjectDescriptorCallback)DR_EarthWarrior_canMount,
        (ObjectDescriptorCallback)DR_EarthWarrior_getMountSide,
        (ObjectDescriptorCallback)DR_EarthWarrior_getRiderPosition,
        (ObjectDescriptorCallback)DR_EarthWarrior_canDismount,
        (ObjectDescriptorCallback)DR_EarthWarrior_getDismountSide,
        (ObjectDescriptorCallback)DR_EarthWarrior_getCameraPosition,
        (ObjectDescriptorCallback)DR_EarthWarrior_getMountState,
        (ObjectDescriptorCallback)DR_EarthWarrior_setMountState,
        (ObjectDescriptorCallback)DR_EarthWarrior_getPlayerAnim,
        (ObjectDescriptorCallback)DR_EarthWarrior_func19,
        (ObjectDescriptorCallback)DR_EarthWarrior_getRacePosition,
        (ObjectDescriptorCallback)DR_EarthWarrior_resetToRomListPosition,
        (ObjectDescriptorCallback)DR_EarthWarrior_handleRiderScale,
        (ObjectDescriptorCallback)DR_EarthWarrior_func23,
    },
    0,
};
