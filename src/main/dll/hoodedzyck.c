/*
 * hoodedzyck - the hooded-Zyck flyer baddie: line-probes the ground each frame
 * and toggles its move/visibility, chases the player with a per-frame yaw step,
 * and seeds its move/speed fields on init.
 *
 *   hoodedZyckUpdateWhileFrozen  freeze-event handler.
 *   hoodedZyck_tickPhaseTimer    counts the phase timer down, rearming it when it expires.
 *   hoodedZyck_getAngleDelta     signed yaw delta from the zyck to a target, wrapped to +/-32768.
 *   hoodedZyck_updateIdle        unengaged-slot update: per-frame ground probe + move/visibility toggle.
 *   hoodedZyck_updateB           chase/attack update.
 *   hoodedZyck_update            approach update.
 *   hoodedZyck_init              seeds BaddieState speed/move fields.
 */
#include "dolphin/mtx/mtx_legacy.h"
#include "main/frame_timing.h"
#include "game/objects/object_setup.h"
#include "game/objects/object.h"
#include "main/track_bbox_api.h"
#include "main/obj_group.h"
#include "sys/objects/lifecycle.h"
#include "sys/objects.h"
#include "main/obj_query.h"
#include "main/dll/baddie_state.h"
#include "main/dll/baddie_setmove.h"
#include "main/dll/dll_00C9_enemy.h"
#include "main/audio/sfx_ids.h"
#include "main/audio/sfx.h"
#include "main/objhits.h"
#include "main/dll/player_api.h"
#include "main/dll/objfsa.h"
#include "main/audio/sfx_trigger_ids.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/trig_float_helpers.h"
#include "main/dll/hoodedzyck.h"
#include "main/dll/baddie_frozen.h"

typedef struct DusterState
{
    u8 pad00[0x324];
    f32 phaseTimer; /* 0x324 */
    f32 decoyTimer; /* 0x328 */
    u8 pad32C[0x338 - 0x32C];
    u16 turnDelta; /* 0x338 hooded-zyck per-frame rotY step */
} DusterState;

typedef struct FCVars
{
    u8 pad000[0x324];
    f32 engineTimer; /* 0x324 */
    f32 emergeTimer; /* 0x328 */
    u8 pad32c[0x338 - 0x32c];
    u16 turnDelta; /* 0x338 */
} FCVars;

#define FIRECRAWLER_HIT_VOLUME_SLOT      9
#define LANTERNFIREFLY_OBJGROUP 0x30 /* DLL 0x10C lanternfirefly */

extern f32 lbl_803DBCE0;
extern f32 lbl_803DBCE4;
extern f32 lbl_803DBCE8;
extern f32 lbl_803DBCEC;

void hoodedZyckUpdateWhileFrozen(int obj, u8* state, int unused, int eventKind, int wpad0, int wpad1, Vec* wpad2,
                                 int wpad3)
{
    if (eventKind == 0x10)
    {
        ((BaddieState*)state)->reactionFlags = ((BaddieState*)state)->reactionFlags | 0x20;
    }
    else
    {
        ((BaddieState*)state)->reactionFlags = ((BaddieState*)state)->reactionFlags | 8;
        Sfx_PlayFromObject(obj, SFXTRIG_dn_boar1_c_244);
        ((BaddieState*)state)->hitCounter = 0;
    }
    return;
}

static void hoodedZyck_tickPhaseTimer(DusterState* st)
{
    st->phaseTimer = st->phaseTimer - timeDelta;
    if (st->phaseTimer <= 0.0f)
    {
        st->phaseTimer = (f32)(int)randomGetRange(0x3c, 0x78);
    }
}

static int hoodedZyck_getAngleDelta(GameObject* obj, GameObject* target)
{
    f32 d = (f32)(int)((u16)getAngle(obj->anim.localPosX - target->anim.localPosX,
                                     obj->anim.localPosZ - target->anim.localPosZ) -
                       (u16)obj->anim.rotX);
    if (d > 32768.0f)
    {
        d = -65535.0f + d;
    }
    if (d < -32768.0f)
    {
        d = 65535.0f + d;
    }
    return d;
}

void hoodedZyck_updateIdle(GameObject* obj, int state)
{
    bool resetting;
    int groundHit;
    u8 noHit;
    int randBit;
    float toPos[3];
    float fromPos[3];
    float cosYaw;
    float sinYaw;
    float hitOut[22];

    hoodedZyck_tickPhaseTimer((DusterState*)state);
    if (0.0f != ((DusterState*)state)->decoyTimer)
    {
        ObjHits_DisableObject(obj);
        if ((obj)->anim.currentMove != 5)
        {
            fn_8014D08C(obj, state, 5, lbl_803DBCEC, 0, 0);
        }
        else if ((((BaddieState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
        {
            ObjHits_EnableObject(obj);
            ((DusterState*)state)->decoyTimer = 0.0f;
        }
        (obj)->anim.alpha = 0xff;
        resetting = true;
    }
    else
    {
        resetting = false;
    }
    if (!resetting)
    {
        (obj)->anim.rotX = (short)((obj)->anim.rotX + ((DusterState*)state)->turnDelta);
        fromPos[0] = (obj)->anim.localPosX;
        fromPos[1] = (obj)->anim.localPosY;
        fromPos[2] = (obj)->anim.localPosZ;
        fn_80292E20((u32)(u16)(obj)->anim.rotX, &sinYaw, &cosYaw);
        toPos[0] = (obj)->anim.localPosX - 10.0f * sinYaw;
        toPos[1] = 5.0f + (obj)->anim.localPosY;
        toPos[2] = (obj)->anim.localPosZ - 10.0f * cosYaw;
        groundHit = objBboxFn_800640cc(fromPos, toPos, 0.0f, 3, (TrackBBoxHit*)hitOut,
                                       obj,
                                       (u32) * (u8*)(state + 0x261),
                                       0xffffffff, 0xff, 0);
        noHit = !(groundHit & 0xff);
        if (!noHit || ((((BaddieState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0))
        {
            if (noHit && (obj)->anim.currentMove != 0)
            {
                ((DusterState*)state)->turnDelta = 0;
                fn_8014D08C(obj, state, 0, 1.0f, 0, 1);
            }
            else
            {
                float fz;
                fn_8014D08C(obj, state, 1, 0.75f, 0, 0);
                fz = 0.0f;
                (obj)->anim.velocityX = fz;
                (obj)->anim.velocityY = fz;
                (obj)->anim.velocityZ = fz;
                randBit = randomGetRange(0, 1);
                ((DusterState*)state)->turnDelta = (u16)((randBit - 1) * 0x12c);
            }
        }
        (obj)->anim.rotY = ((BaddieState*)state)->spawnRotY;
        (obj)->anim.rotZ = ((BaddieState*)state)->spawnRotZ;
    }
    return;
}

void hoodedZyck_updateB(GameObject* obj, u8* state)
{
    f32 scale;
    int moved;
    int turnRaw;
    u8 noHit;
    u16 mag;
    u8 bufA[88];
    u8 bufB[84];
    f32 tgtA[3];
    f32 posA[3];
    f32 tgtB[3];
    f32 posB[3];
    f32 range;
    f32 cosA;
    f32 sinA;
    f32 cosB;
    f32 sinB;

    {
        u8 n = *(u8*)(*(int*)&obj->anim.placementData + 0x2f);
        scale = n;
        if (0.0f == n)
        {
            scale = 10.0f;
        }
        scale = scale / 10.0f;
    }

    hoodedZyck_tickPhaseTimer((DusterState*)state);

    if (0.0f != ((FCVars*)state)->emergeTimer)
    {
        ObjHits_DisableObject(obj);
        if (obj->anim.currentMove != 5)
        {
            fn_8014D08C(obj, (int)state, 5, lbl_803DBCEC, 0, 0);
        }
        else if ((((BaddieState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
        {
            ObjHits_EnableObject(obj);
            ((FCVars*)state)->emergeTimer = 0.0f;
        }
        obj->anim.alpha = 0xff;
        moved = 1;
    }
    else
    {
        moved = 0;
    }

    if (moved == 0)
    {
        u32 ang;
        f32 diff;
        GameObject* other;

        *(s16*)obj = *(s16*)obj + ((FCVars*)state)->turnDelta;
        posA[0] = obj->anim.localPosX;
        posA[1] = obj->anim.localPosY;
        posA[2] = obj->anim.localPosZ;
        fn_80292E20((u16)obj->anim.rotX, &sinA, &cosA);
        tgtA[0] = -(10.0f * sinA - obj->anim.localPosX);
        tgtA[1] = 5.0f + obj->anim.localPosY;
        tgtA[2] = -(10.0f * cosA - obj->anim.localPosZ);
        /* 0x261 = BaddieState.contactSfxFlags; kept raw - typed member as a
         * call arg shifts arg emission bytes here. */
        noHit = !(u8)objBboxFn_800640cc(posA, tgtA, 0.0f, 3, (TrackBBoxHit*)bufA, obj,
                                        *(u8*)(state + 0x261), -1, 0xff, 0);
        ang =
            getAngle(
                obj->anim.localPosX - ((GameObject*)((BaddieState*)state)->trackedObj)->anim.localPosX,
                obj->anim.localPosZ - ((GameObject*)((BaddieState*)state)->trackedObj)->anim.localPosZ) &
            0xffff;
        diff = (f32)(int)(ang - ((int)*(s16*)obj & 0xffffu));
        if (diff > 32768.0f)
        {
            diff = -65535.0f + diff;
        }
        if (diff < -32768.0f)
        {
            diff = 65535.0f + diff;
        }
        turnRaw = diff;
        {
            s16 t = turnRaw;
            mag = (u16)(t >= 0 ? t : -t);
        }
        if (fn_80295C88(Obj_GetPlayerObject()) != 0)
        {
            range = 100.0f;
            other = (GameObject*)ObjGroup_FindNearestObject(LANTERNFIREFLY_OBJGROUP, obj, &range);
            if (other != NULL)
            {
                s16 yaw = Obj_GetYawDeltaToObject(obj, other, &range);
                int t;
                if (yaw < -300)
                {
                    yaw = -300;
                }
                else if (yaw > 300)
                {
                    yaw = 300;
                }
                t = yaw;
                ((FCVars*)state)->turnDelta = t;
                t = yaw >= 0 ? yaw : -yaw;
                if (t < 0x4000)
                {
                    *(s16*)obj = -*(s16*)obj;
                    posB[0] = obj->anim.localPosX;
                    posB[1] = obj->anim.localPosY;
                    posB[2] = obj->anim.localPosZ;
                    fn_80292E20((u16)obj->anim.rotX, &sinB, &cosB);
                    tgtB[0] = -(10.0f * sinB - obj->anim.localPosX);
                    tgtB[1] = 5.0f + obj->anim.localPosY;
                    tgtB[2] = -(10.0f * cosB - obj->anim.localPosZ);
                    if ((u8)objBboxFn_800640cc(posB, tgtB, 0.0f, 3, (TrackBBoxHit*)bufB, obj,
                                               *(u8*)(state + 0x261), -1, 0xff, 0) == 0)
                    {
                        if ((((BaddieState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
                        {
                            fn_8014D08C(obj, (int)state, 7, 1.0f / (2.0f * scale), 0, 1);
                        }
                        obj->anim.rotY = ((BaddieState*)state)->spawnRotY;
                        obj->anim.rotZ = ((BaddieState*)state)->spawnRotZ;
                    }
                    *(s16*)obj = -*(s16*)obj;
                }
                return;
            }
        }
        if (((BaddieState*)state)->trackedObj != NULL &&
            ((GameObject*)((BaddieState*)state)->trackedObj)->anim.hitboxScale > 56.0f)
        {
            ((BaddieState*)state)->speedScale = lbl_803DBCE8;
        }
        if ((((BaddieState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0 || noHit == 0 ||
            (mag < 3000 && noHit != 0 && obj->anim.currentMove != 0))
        {
            if (noHit != 0 && mag < 3000)
            {
                ((FCVars*)state)->turnDelta = 0;
                fn_8014D08C(obj, (int)state, 0, 1.0f / scale, 0, 1);
            }
            else
            {
                fn_8014D08C(obj, (int)state, 1, 0.75f / scale, 0, 0);
                {
                    f32 z = 0.0f;
                    obj->anim.velocityX = z;
                    obj->anim.velocityY = z;
                    obj->anim.velocityZ = z;
                }
                if (mag < 3000)
                {
                    ((FCVars*)state)->turnDelta = (randomGetRange(0, 1) - 1) * 300;
                }
                else if ((s16)turnRaw < 0)
                {
                    ((FCVars*)state)->turnDelta = 0xfed4;
                }
                else
                {
                    ((FCVars*)state)->turnDelta = 300;
                }
            }
        }
        obj->anim.rotY = ((BaddieState*)state)->spawnRotY;
        obj->anim.rotZ = ((BaddieState*)state)->spawnRotZ;
    }
}

void hoodedZyck_update(GameObject* obj, u8* state)
{
    int moved;
    int turnRaw;
    u16 mag;
    u32 grabbed;

    hoodedZyck_tickPhaseTimer((DusterState*)state);

    if (0.0f != ((FCVars*)state)->emergeTimer)
    {
        ObjHits_DisableObject(obj);
        if (obj->anim.currentMove != 5)
        {
            fn_8014D08C(obj, (int)state, 5, lbl_803DBCEC, 0, 0);
        }
        else if ((((BaddieState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
        {
            ObjHits_EnableObject(obj);
            ((FCVars*)state)->emergeTimer = 0.0f;
        }
        obj->anim.alpha = 0xff;
        moved = 1;
    }
    else
    {
        moved = 0;
    }

    if (moved == 0)
    {
        f32 z;
        *(s16*)obj = (f32)((FCVars*)state)->turnDelta * timeDelta + (f32)(int)*(s16*)obj;
        z = 0.0f;
        obj->anim.velocityX = z;
        obj->anim.velocityY = z;
        obj->anim.velocityZ = z;
        ObjHits_SetHitVolumeSlot((ObjAnimComponent*)obj, FIRECRAWLER_HIT_VOLUME_SLOT, 1, -1);
        turnRaw = hoodedZyck_getAngleDelta(obj, (GameObject*)((BaddieState*)state)->trackedObj);
        {
            int t = (s16)turnRaw;
            mag = (u16)(t >= 0 ? t : -t);
        }
        ObjHits_EnableObject(obj);
        grabbed = ((BaddieState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN;
        if (grabbed != 0 && obj->anim.currentMove == 6)
        {
            fn_8014D08C(obj, (int)state, 4, lbl_803DBCE0, 0, 1);
        }
        else
        {
            if (grabbed != 0 ||
                (mag < 1000 && obj->anim.currentMove != 2 && obj->anim.currentMove != 4 &&
                 obj->anim.currentMove != 6))
            {
                if (mag < 1000)
                {
                    if (((BaddieState*)state)->speedScale < 40.0f)
                    {
                        fn_8014D08C(obj, (int)state, 2, 0.75f, 0, 0);
                    }
                    else
                    {
                        fn_8014D08C(obj, (int)state, 6, lbl_803DBCE4, 0, 0);
                    }
                    ((FCVars*)state)->turnDelta = 0;
                }
                else
                {
                    fn_8014D08C(obj, (int)state, 1, 0.75f, 0, 0);
                    if ((s16)turnRaw < 0)
                    {
                        ((FCVars*)state)->turnDelta = 0xfed4;
                    }
                    else
                    {
                        ((FCVars*)state)->turnDelta = 300;
                    }
                }
            }
            obj->anim.rotY = ((BaddieState*)state)->spawnRotY;
            obj->anim.rotZ = ((BaddieState*)state)->spawnRotZ;
        }
    }
}

void hoodedZyck_init(GameObject* obj, BaddieState* st)
{
    f32 ratio;
    f32 base_v;
    u32 flags;
    u32 amt;
    amt = *((u8*)((int*)*(int*)&obj->anim.placementData) + 0x2f);
    ratio = amt;
    if (0.0f == amt)
    {
        ratio = 10.0f;
    }
    ratio = ratio / 10.0f;
    st->speedScale = 30.0f;
    *(u32*)&st->unk2E4 = 0x8b;
    flags = *(u32*)&st->unk2E4;
    *(u32*)&st->unk2E4 = flags | 0x20;
    st->unk308 = 0.02f * ratio;
    base_v = 1.0f;
    st->animDeltaScale = base_v;
    st->unk304 = 0.97f;
    *((u8*)st + 0x320) = 0;
    *(f32*)&st->eventFlags = 1.5f;
    *((u8*)st + 0x321) = 3;
    {
        f32 d2 = 2.0f;
        st->unk318 = d2;
        *((u8*)st + 0x322) = 5;
        st->unk31C = d2;
    }
    ((FCVars*)st)->turnDelta = 0;
    ((FCVars*)st)->engineTimer = 60.0f;
    ((FCVars*)st)->emergeTimer = base_v;
    obj->anim.alpha = 0;
    st->pathStep = 0.5f * ratio;
    st->reactionFlags = 0;
    ObjHits_EnableObject(obj);
}
