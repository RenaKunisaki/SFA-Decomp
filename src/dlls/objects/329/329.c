/*
 * DLL 0x149 - wind lifts used by the CFWindLift and CFTreasWind object
 * definitions. update ramps the lift alpha from its game bit,
 * runs the rise sequence with a squared ramp-in, and tracks up to 14
 * rider slots (riders get pulled by WindLift_updateRider's per-slot spring
 * model). The three fortress lifts (placement seqIds 0x58-0x5A in the
 * lookup tables) only run once GameBit 0x57 is set - the city's power
 * restored via the three power bases and the main crystal convergence
 * (see cfpowerbase/cfmaincrystal), which is what the freed old
 * CloudRunner sends you off to do.
 */

#include "dlls/objects/328_CFGuardian.h"

#include "game/objects/object.h"
#include "main/audio/music_api.h"
#include "main/obj_group.h"
#include "main/obj_message.h"
#include "sys/objects.h"
#include "dlls/object_descriptor.h"
#include "main/dll/player_motion.h"
#include "main/gamebits.h"
#include "main/audio/music_trigger_ids.h"
#include "main/vecmath.h"
#include "main/object_render.h"
#include "main/frame_timing.h"
#include "main/maketex_sequence_api.h"
#include "main/dll/CF/dll_0149_cfwindlift.h"
#include "main/dll/CF/dll_014A_cfpowerbase.h"
#include "main/dll/CF/dll_014B_cfmaincrystal.h"

#define CFWINDLIFT_OBJGROUP 0x49
#define CFWINDLIFT_OBJFLAG_PARENT_SLACK 0x1000

#define WINDLIFT_SLOTS 14 /* max tracked lift slots */
#define WINDLIFT_HEIGHT_BYTE_SCALE 4.0f
#define WINDLIFT_DEFAULT_HEIGHT    90.0f

/* WindLiftSlot.phaseFlags bits */
#define WLSLOT_RISING   0x1  /* rise force active this frame */
#define WLSLOT_PENDING  0x2  /* pull direction re-evaluation requested */
#define WLSLOT_PULLUP   0x4  /* pulling rider upward */
#define WLSLOT_PULLDOWN 0x8  /* pulling rider downward */
#define WLSLOT_HOLD     0x20 /* rider holding gamebit (grabbed) phase */
#define WLSLOT_RELEASE  0x40 /* rider released phase */
#define WLSLOT_LATCH    0x80 /* lift event latched/consumed */

typedef struct
{
    int duration;
    int seqId;
    int delay;
    int gamebit;
    int pad10;
    int timer;
    WindLiftSlot slots[WINDLIFT_SLOTS];
    int pad168;
    int pad16c;
    f32 liftHeight;
    u8 musicOn : 1;
    u8 active : 1;
    u8 _f2 : 6;
} WindLiftSub;

extern u8 gWindLiftSeqDurationTable[];
extern u8 gWindLiftSeqGamebitTable[];

static void WindLift_resetSlot(WindLiftSlot* slot)
{
    slot->phaseFlags = 0;
    slot->phaseFlags &= ~0xf1;
    slot->f4 = 0.1f;
    slot->riseSpeed = 0.0f;
    slot->speedDelta = 0.0f;
    slot->riderObj = 0;
    slot->oscCounter = 0;
}

/* WindLift_updateRider: per-rider wind lift physics - track the rider while
 * above the lift and in range, send the lift/drop messages on state
 * edges, and integrate the rise speed with ramp-up, oscillation damping
 * and player-mode handoff. The spring model pulls a rider toward the
 * lift column and lifts it with the wind; slot->phaseFlags carries the rider's
 * phase bits. */
void WindLift_updateRider(GameObject* obj, GameObject* rider, WindLiftSlot* slot, f32 pull, int gb, int pm, u32 dur, f32 height)
{
    GameObject* player;
    f32 lim;
    f32 rise;
    f32 over;
    f32 speed;
    f32 thr;
    f32 dy;
    f32 dist;
    f32 factor;
    f32 scale;
    u8 flags;
    u8 fl;
    int fe;
    player = Obj_GetPlayerObject();
    dy = ((GameObject*)rider)->anim.localPosY - obj->anim.localPosY;
    if (dy < 0.0f)
    {
        return;
    }
    dist = Vec_xzDistance(&((GameObject*)rider)->anim.worldPosX, &obj->anim.worldPosX);
    if (dist > 10.0f + height && (slot->phaseFlags & 0xe0) == 0)
    {
        return;
    }
    flags = slot->phaseFlags;
    if ((flags & WLSLOT_LATCH) != 0 && gb != 0)
    {
        return;
    }
    if (dist < height)
    {
        if ((flags & 0xe0) == 0 || (flags & WLSLOT_LATCH) != 0)
        {
            if (gb != 0 && (!flags & WLSLOT_LATCH) != 0 && dy < 20.0f)
            {
                slot->phaseFlags |= WLSLOT_LATCH;
                return;
            }
            if ((flags & WLSLOT_PENDING) != 0)
            {
                if (dy / pull > 0.8f)
                {
                    slot->phaseFlags |= WLSLOT_PULLUP;
                    slot->phaseFlags &= ~WLSLOT_PULLDOWN;
                }
                else
                {
                    slot->phaseFlags |= WLSLOT_PULLDOWN;
                    slot->phaseFlags &= ~WLSLOT_PULLUP;
                }
                slot->phaseFlags &= ~WLSLOT_PENDING;
            }
            if (gb == 0)
            {
                slot->phaseFlags |= WLSLOT_RELEASE;
                slot->phaseFlags &= ~WLSLOT_HOLD;
                ObjMsg_SendToObject(rider, 0xf, obj, (((slot->phaseFlags & 0xe0) >> 4) << 8) | dur);
                slot->phaseFlags &= ~WLSLOT_LATCH;
            }
            else
            {
                if (dy > 30.0f)
                {
                    ObjMsg_SendToObject(rider, 0xf, obj, (((slot->phaseFlags & 0xe0) >> 4) << 8) | dur);
                }
                slot->phaseFlags |= WLSLOT_HOLD;
                slot->phaseFlags &= ~WLSLOT_RELEASE;
            }
        }
        scale = 0.324f;
        fl = slot->phaseFlags;
        fe = fl & 0xe;
        if (fe != 0 && (fl & WLSLOT_PULLDOWN) != 0 && gb == 0)
        {
            pull *= 0.6f;
        }
        pull *= 0.6f;
        if (pull <= 10.0f)
        {
            return;
        }
        if (dy < 3.0f)
        {
            dy = 3.0f;
        }
        if (gb == 0)
        {
            lim = pull - (pull / 50.0f) * (slot->riseSpeed * (slot->riseSpeed * slot->riseSpeed));
            if (dy > lim)
            {
                rise = 0.0f;
            }
            else
            {
                over = lim - dy;
                if (over > 20.0f)
                {
                    rise = 1.0f;
                }
                else
                {
                    rise = over / 20.0f;
                }
            }
            factor = rise;
            slot->phaseFlags |= WLSLOT_RISING;
            if (((slot->riseSpeed < -0.2f && slot->oscCounter % 2 != 0) ||
                 (slot->riseSpeed > 0.2f && slot->oscCounter % 2 == 0)) &&
                (slot->phaseFlags & WLSLOT_PULLDOWN) != 0)
            {
                if (slot->oscCounter++ > 2)
                {
                    slot->phaseFlags &= ~WLSLOT_PULLDOWN;
                    slot->phaseFlags |= WLSLOT_PULLUP;
                }
            }
        }
        else
        {
            speed = slot->riseSpeed;
            if (fe != 0)
            {
                thr = 0.1f;
            }
            else
            {
                thr = 0.5f;
            }
            if (speed > thr)
            {
                slot->oscCounter = 1;
            }
            scale *= 1.5f;
            if (slot->oscCounter == 0)
            {
                if ((slot->phaseFlags & 0xe) != 0)
                {
                    factor = 1.0f - dy / (1.55f * pull);
                }
                else
                {
                    factor = 1.0f - dy / (0.9f * pull);
                }
                if (factor < 0.0f)
                {
                    factor = 0.0f;
                }
                factor = factor * factor;
            }
            else
            {
                factor = 0.01f;
            }
        }
        slot->speedDelta = scale * factor - 0.18f;
        slot->riseSpeed = slot->riseSpeed + slot->speedDelta;
        if (slot->riseSpeed > 8.0f)
        {
            slot->riseSpeed = 8.0f;
        }
        if (0.0f == slot->riseSpeed)
        {
            slot->riseSpeed = -0.001f;
        }
        if (dy < 20.0f && gb != 0)
        {
            slot->riseSpeed = 0.0f;
            slot->oscCounter = 0;
            ObjMsg_SendToObject(rider, 0x10, obj, gb);
            slot->phaseFlags |= WLSLOT_LATCH;
            if (pm != 0)
            {
                player->anim.velocityY = 0.0f;
            }
        }
        if (pm != 0)
        {
            Player_SetLiftVelocityY((int)rider, slot->riseSpeed);
        }
        else
        {
            ((GameObject*)rider)->anim.localPosY = slot->riseSpeed * timeDelta + ((GameObject*)rider)->anim.localPosY;
            ((GameObject*)rider)->anim.velocityY = slot->riseSpeed * timeDelta;
        }
    }
    else
    {
        if (pm != 0)
        {
            Player_SetLiftVelocityY((int)rider, 0.0f);
        }
        if (pm == 0)
        {
            ObjMsg_SendToObject(rider, 0x10, obj, gb);
            slot->phaseFlags &= ~0xf1;
            slot->riseSpeed = 0.0f;
            slot->oscCounter = 0;
        }
    }
}

int WindLift_getExtraSize(void)
{
    return 0x178;
}

int WindLift_getObjectTypeId(void)
{
    return 0x0;
}

void WindLift_free(GameObject* obj)
{
    void* p = Obj_GetPlayerObject();
    if (p == NULL || Player_GetLiftVelocityY((int)p) == 0.0f)
    {
        Music_Trigger(MUSICTRIG_DIM_Cavern, 0);
    }
    ObjGroup_RemoveObject((int)obj, CFWINDLIFT_OBJGROUP);
}

void WindLift_render(int obj, int p2, int p3, int p4, int p5, s8 visible)
{
    s32 vis = visible;
    if (vis != 0)
        objRenderModelAndHitVolumes((GameObject*)obj, p2, p3, p4, p5, 1.0f);
}

void WindLift_hitDetect(void)
{
}

/* WindLift_update: fade the lift opacity with its gamebit, spin up
 * over the first second, then assign every nearby group-0x16 object
 * (and the player) to a rider slot and run the lift physics on each. */
void WindLift_update(GameObject* obj)
{
    u8* def;
    WindLiftSub* sub = obj->extra;
    int level;
    GameObject* player;
    f32 pull;
    int idx;
    int j;
    int found;
    int count;
    int** objs;
    int gb2;
    def = (u8*)obj->anim.placement;
    if (sub->active)
    {
        level = (int)(2.0f * timeDelta + (f32)(int)obj->anim.alpha);
        if (sub->gamebit != -1 && mainGetBit(sub->gamebit) == 0)
        {
            sub->active = 0;
        }
    }
    else
    {
        level = (int)-(2.0f * timeDelta - (f32)(int)obj->anim.alpha);
        if (sub->gamebit != -1 && mainGetBit(sub->gamebit) != 0)
        {
            sub->active = 1;
        }
    }
    obj->anim.alpha = (level < 0) ? 0 : ((level > 0xff) ? 0xff : level);
    /* the fortress lifts (table durations 1-4) stay dead until the
       city's power is restored (0x57, the crystal convergence) */
    if ((mainGetBit(GAMEBIT_CF_PowerOn) != 0 || sub->duration > 0xa) && sub->active)
    {
        int ticks = sub->timer;
        sub->timer = ticks + 1;
        if (ticks < 0x3c && mainGetBit(sub->seqId) == 0)
        {
            obj->anim.rotX -= ((framesThisStep * 100) * (sub->timer * sub->timer)) / 0x3c;
            Obj_SetActiveModelIndex(obj, 0);
            return;
        }
        Obj_SetActiveModelIndex(obj, 1);
        gb2 = mainGetBit(sub->delay);
        {
            int rotStep = framesThisStep * 0xb6;
            obj->anim.rotX -= rotStep * ((gb2 << 2) + 0xe);
        }
        pull = (f32)((WindliftPlacement*)def)->pullStrength;
        player = Obj_GetPlayerObject();
        if (mainGetBit(sub->seqId) != 0)
        {
            if (!sub->musicOn)
            {
                sub->musicOn = 1;
                Music_Trigger(MUSICTRIG_DIM_Cavern, 1);
            }
            if (player != NULL)
            {
                WindLift_updateRider(obj, player, &sub->slots[0], pull, gb2, 1, sub->duration, sub->liftHeight);
            }
        }
        else
        {
            if (sub->musicOn)
            {
                Music_Trigger(MUSICTRIG_DIM_Cavern, 0);
                sub->musicOn = 0;
            }
            if ((sub->slots[0].phaseFlags & 0xe0) != 0)
            {
                u8 flags;
                Player_SetLiftVelocityY((int)player, 0.0f);
                flags = sub->slots[0].phaseFlags;
                if ((flags & 0xe) != 0)
                {
                    sub->slots[0].phaseFlags = flags | WLSLOT_PENDING;
                }
                sub->slots[0].riseSpeed = 0.0f;
                sub->slots[0].oscCounter = 0;
                sub->slots[0].phaseFlags &= ~0xf1;
            }
        }
        objs = (int**)ObjGroup_GetObjects(CFGUARDIAN_OBJECT_GROUP, &count);
        count = count + 1;
        if (count > 0xe)
        {
            count = 0xe;
        }
        for (j = 1; j < WINDLIFT_SLOTS; j++)
        {
            sub->slots[j].linkIndex = -1;
        }
        for (idx = 1; idx < count; idx++)
        {
            found = -1;
            for (j = 1; j < WINDLIFT_SLOTS; j++)
            {
                if ((u32)sub->slots[j].riderObj == (u32)*objs)
                {
                    found = j;
                }
            }
            if (found == -1)
            {
                for (j = 1; j < WINDLIFT_SLOTS; j++)
                {
                    if ((u32)sub->slots[j].riderObj == 0)
                    {
                        found = j;
                        WindLift_resetSlot(&sub->slots[j]);
                        j = 2000;
                    }
                }
                if (found == -1)
                {
                    return;
                }
                sub->slots[found].riderObj = (int)*objs;
            }
            sub->slots[found].linkIndex = found;
            {
                int* rider = *objs;
                if ((((GameObject*)rider)->objectFlags & CFWINDLIFT_OBJFLAG_PARENT_SLACK) != 0)
                {
                    objs++;
                }
                else if (rider != NULL)
                {
                    WindLift_updateRider(obj, (GameObject*)*objs++, &sub->slots[found], pull, gb2, 0,
                                sub->duration, sub->liftHeight);
                }
            }
        }
        for (j = 1; j < WINDLIFT_SLOTS; j++)
        {
            if (sub->slots[j].linkIndex == -1)
            {
                sub->slots[j].riderObj = 0;
            }
        }
    }
}

/* WindLift_init: look up the lift's sequence timings, scale its rise
 * height from the def byte, arm it from the gamebits and clear all 14
 * rider slots. */
void WindLift_init(GameObject* obj, u8* def)
{
    int i;
    WindLiftSub* sub = obj->extra;
    sub->seqId = ((WindliftObjectDef*)def)->seqId;
    sub->duration = seqPairTableLookup(gWindLiftSeqDurationTable, 4, sub->seqId);
    sub->gamebit = seqPairTableLookup(gWindLiftSeqGamebitTable, 3, sub->seqId);
    if (sub->gamebit == 0)
    {
        sub->gamebit = -1;
    }
    if (sub->duration == 0)
    {
        sub->duration = 100;
    }
    sub->delay = ((WindliftObjectDef*)def)->delay;
    sub->timer = 0;
    if (((WindliftObjectDef*)def)->heightByte != 0)
    {
        sub->liftHeight = WINDLIFT_HEIGHT_BYTE_SCALE * (f32)((WindliftObjectDef*)def)->heightByte;
    }
    else
    {
        sub->liftHeight = WINDLIFT_DEFAULT_HEIGHT;
    }
    obj->anim.rootMotionScale =
        (*(f32*)(*(char**)&obj->anim.modelInstance + 4) * sub->liftHeight) / WINDLIFT_DEFAULT_HEIGHT;
    /* skip the rise-in ramp after the convergence cutscene (0x57)
       or for long lifts */
    if (mainGetBit(GAMEBIT_CF_PowerOn) != 0 || sub->duration >= 0xa)
    {
        sub->timer = 0x3c;
    }
    sub->active = 1;
    if (sub->gamebit != -1)
    {
        if (mainGetBit(sub->gamebit) != 0)
        {
            sub->timer = 0x3c;
        }
        else
        {
            sub->active = 0;
            obj->anim.alpha = 0;
        }
    }
    {
        WindLiftSub* p = sub;
        for (i = 0; i < WINDLIFT_SLOTS; i++)
        {
            WindLift_resetSlot(&p->slots[i]);
        }
    }
    ObjGroup_AddObject((int)obj, CFWINDLIFT_OBJGROUP);
}

void WindLift_release(void)
{
}

void WindLift_initialise(void)
{
}

u8 gWindLiftSeqDurationTable[] = {
    0x00, 0x00, 0x00, 0x58, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x59, 0x00, 0x00, 0x00, 0x02,
    0x00, 0x00, 0x00, 0x5A, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x0A, 0xD7, 0x00, 0x00, 0x00, 0x04,
};
u8 gWindLiftSeqGamebitTable[] = {
    0x00, 0x00, 0x0A, 0x94, 0x00, 0x00, 0x00, 0x95, 0x00, 0x00, 0x0A, 0x98,
    0x00, 0x00, 0x00, 0x95, 0x00, 0x00, 0x0A, 0x99, 0x00, 0x00, 0x00, 0x95,
};

ObjectDescriptor gWindLiftObjDescriptor = {
    0, 0, 0, OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)WindLift_initialise, (ObjectDescriptorCallback)WindLift_release, 0,
    (ObjectDescriptorCallback)WindLift_init, (ObjectDescriptorCallback)WindLift_update,
    (ObjectDescriptorCallback)WindLift_hitDetect, (ObjectDescriptorCallback)WindLift_render,
    (ObjectDescriptorCallback)WindLift_free, (ObjectDescriptorCallback)WindLift_getObjectTypeId,
    WindLift_getExtraSize,
};
