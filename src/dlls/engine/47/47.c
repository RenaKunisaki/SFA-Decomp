#include "sys/objects.h"
#include "main/dll/player_objects.h"
#include "main/pad.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/frame_timing.h"
#include "main/track_dolphin_api.h"
#include "dolphin/pad.h"

#define CARRYABLE_MSG_PLAYER_GRAB 0x100008

#define CARRYABLE_OBJGROUP 0x10

#define CARRY_STATE_RESTING 0
#define CARRY_STATE_GRABBED 1
#define CARRY_STATE_PUTDOWN 2

#define CARRYABLE_FLAG_JUST_GRABBED      0x01
#define CARRYABLE_FLAG_GRAVITY_DISABLED  0x02
#define CARRYABLE_FLAG_DROP_DISABLED     0x04
#define CARRYABLE_FLAG_SUPPRESS_POS_SAVE 0x08

typedef struct CarryableUpdateHeldState
{
    u8 pad0[0x2 - 0x0];
    s16 unk2;
    u8 pad4[0x5 - 0x4];
    s8 carryState;
    u8 isHeld;
    u8 flags;
    u8 surfaceType;
    u8 pad9[0x10 - 0x9];
} CarryableUpdateHeldState;


void Carryable_putDownAndSavePos(GameObject* obj)
{
    CarryableUpdateHeldState* state = obj->extra;
    state->carryState = CARRY_STATE_RESTING;
    state->isHeld = 0;
    if ((state->flags & CARRYABLE_FLAG_SUPPRESS_POS_SAVE) == 0)
    {
        obj->anim.localPosY += 10.0f;
        saveGame_saveObjectPos(obj);
        obj->anim.localPosY -= 10.0f;
    }
}

void Carryable_stopCarrying(GameObject* obj, void* state)
{
    GameObject* player = Obj_GetPlayerObject();
    GameObject* held;
    ((CarryableUpdateHeldState*)state)->carryState = CARRY_STATE_RESTING;
    Player_GetHeldObject(player, &held);
    if (held == obj)
    {
        playerSetHeldObject(player, NULL);
    }
}

void Carryable_setSuppressPositionSave(void* state, u8 enable)
{
    if (enable != 0)
    {
        ((CarryableUpdateHeldState*)state)->flags |= CARRYABLE_FLAG_SUPPRESS_POS_SAVE;
    }
    else
    {
        ((CarryableUpdateHeldState*)state)->flags &= ~CARRYABLE_FLAG_SUPPRESS_POS_SAVE;
    }
}

s32 Carryable_getDropDisabled(void* state)
{
    return (((CarryableUpdateHeldState*)state)->flags & CARRYABLE_FLAG_DROP_DISABLED) != 0;
}

void Carryable_setDropDisabled(void* state, u8 enable)
{
    if (enable != 0)
    {
        ((CarryableUpdateHeldState*)state)->flags |= CARRYABLE_FLAG_DROP_DISABLED;
    }
    else
    {
        ((CarryableUpdateHeldState*)state)->flags &= ~CARRYABLE_FLAG_DROP_DISABLED;
    }
}

void Carryable_setGravityEnabled(void* state, u8 clear)
{
    if (clear != 0)
    {
        ((CarryableUpdateHeldState*)state)->flags &= ~CARRYABLE_FLAG_GRAVITY_DISABLED;
    }
    else
    {
        ((CarryableUpdateHeldState*)state)->flags |= CARRYABLE_FLAG_GRAVITY_DISABLED;
    }
}

u8 Carryable_getSurfaceType(void* state)
{
    return ((CarryableUpdateHeldState*)state)->surfaceType;
}

s32 Carryable_wasJustGrabbed(void* state)
{
    return ((CarryableUpdateHeldState*)state)->flags & CARRYABLE_FLAG_JUST_GRABBED;
}

s32 Carryable_getCarryState(void* state)
{
    return ((CarryableUpdateHeldState*)state)->carryState;
}

void Carryable_free(GameObject* obj)
{
    objFreeObjectType((int)obj, CARRYABLE_OBJGROUP);
}

int Carryable_updateRenderState(GameObject* obj, int flag)
{
    ObjDef* p50 = (ObjDef*)(*(int**)&obj->anim.modelInstance);
    if (p50->shadowType == OBJ_SHADOW_TYPE_MODEL_GEOMETRIC)
    {
        if (obj->seqIndex == -1)
        {
            obj->anim.modelState->flags &= ~(long long)OBJ_MODEL_STATE_SHADOW_FADE_OUT;
        }
        else
        {
            obj->anim.modelState->flags |= OBJ_MODEL_STATE_SHADOW_FADE_OUT;
        }
    }
    if (obj->userData2 != 0)
    {
        if (flag != -1)
            return 0;
    }
    else
    {
        if (flag == 0)
            return 0;
    }
    return 1;
}

int Carryable_updateHeld(GameObject* obj, void* state)
{
    TrackGroundHit** list;
    GameObject* player;
    CarryableUpdateHeldState* held;
    held = obj->extra;
    held->surfaceType = 0;
    held->flags &= ~CARRYABLE_FLAG_JUST_GRABBED;
    player = Obj_GetPlayerObject();
    if (held->carryState == CARRY_STATE_RESTING)
    {
        struct
        {
            u8 a, b, c, d, e;
        }* t;
        int newCarryState = 0;
        t = (void*)*(u8**)((u8*)obj + 0x78);
        if ((t[obj->hitVolumeIndex].e & 0xf) == 6 && (buttonGetDisabled(0) & PAD_BUTTON_A) == 0 &&
            (obj->anim.resetHitboxFlags & INTERACT_FLAG_ACTIVATED) != 0 &&
            obj->userData2 == 0)
        {
            *(s16*)held = 0;
            buttonDisable(0, PAD_BUTTON_A);
            newCarryState = 1;
        }
        held->carryState = newCarryState;
        if (held->carryState != CARRY_STATE_RESTING)
        {
            held->flags |= CARRYABLE_FLAG_JUST_GRABBED;
            held->isHeld = 1;
        }
        if (obj->userData2 == 0)
        {
            GameObject* hit;
            int cnt, i, j;
            ObjHits_SyncObjectPositionIfDirty(obj);
            obj->anim.resetHitboxFlags &= ~INTERACT_FLAG_DISABLED;
            if ((held->flags & CARRYABLE_FLAG_GRAVITY_DISABLED) == 0)
            {
                obj->anim.velocityY = -(0.1f * timeDelta - obj->anim.velocityY);
                obj->anim.localPosY =
                    obj->anim.velocityY * timeDelta + obj->anim.localPosY;
            }
            cnt = trackGetHeight(obj, obj->anim.localPosX, obj->anim.localPosY,
                                       obj->anim.localPosZ, &list, 0, 1);
            hit = 0;
            i = 0;
            for (j = cnt; j > 0; j--)
            {
                if ((s8)list[i]->surfaceType != 0xe)
                {
                    if (obj->anim.localPosY < list[i]->height &&
                        obj->anim.localPosY > list[i]->height - 40.0f)
                    {
                        hit = list[i]->object;
                        obj->anim.localPosY = list[i]->height;
                        obj->anim.velocityY = 0.0f;
                        break;
                    }
                }
                i++;
            }
            i = 0;
            for (; cnt > 0; cnt--)
            {
                f32 d = obj->anim.localPosY - list[i]->height;
                if (d < 0.0f)
                {
                    d = -d;
                }
                if (d < 5.0f)
                {
                    s8 t2 = *(s8*)&list[i]->surfaceType;
                    if (t2 > held->surfaceType)
                    {
                        *(s8*)&held->surfaceType = t2;
                    }
                }
                i++;
            }
            if (hit != 0)
            {
                u8* owner = *(u8**)((u8*)hit + 0x58);
                u8 slot = (*(u8*)(owner + 0x10f))++;
                ((void**)(owner + 0x100))[(s8)slot] = obj;
            }
        }
    }
    else
    {
        ObjHits_MarkObjectPositionDirty(&obj->anim);
        obj->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
        if ((getButtonsJustPressed(0) & PAD_BUTTON_A) != 0)
        {
            if ((held->flags & CARRYABLE_FLAG_DROP_DISABLED) != 0 ||
                isTrickyNear(player) == 0)
            {
                Sfx_PlayFromObject(0, SFXTRIG_id_10a);
            }
            else
            {
                buttonDisable(0, PAD_BUTTON_A);
                held->isHeld = 0;
            }
        }
        if (obj->userData2 == 1)
        {
            held->carryState = CARRY_STATE_PUTDOWN;
        }
        if (held->carryState == CARRY_STATE_PUTDOWN && obj->userData2 == 0)
        {
            CarryableUpdateHeldState* h2 = obj->extra;
            h2->carryState = CARRY_STATE_RESTING;
            h2->isHeld = 0;
            if ((h2->flags & CARRYABLE_FLAG_SUPPRESS_POS_SAVE) == 0)
            {
                obj->anim.localPosY += 10.0f;
                saveGame_saveObjectPos(obj);
                obj->anim.localPosY -= 10.0f;
            }
        }
        if (*(s8*)&held->isHeld != 0)
        {
            ObjMsg_SendToObject(player, CARRYABLE_MSG_PLAYER_GRAB, obj,
                                (held->unk2 << 16) | (u16) * (s16*)held);
        }
    }
    return held->carryState;
}

void Carryable_init(GameObject* obj, void* state, int arg2)
{
    CarryableUpdateHeldState* s = (CarryableUpdateHeldState*)state;
    objAddObjectType((int)obj, CARRYABLE_OBJGROUP);
    s->unk2 = 0;
    s->carryState = CARRY_STATE_RESTING;
    s->pad4[0] = 0;
    s->isHeld = 0;
    (obj)->userData2 = 0;
}

void Carryable_release(void)
{
}

void Carryable_initialise(void)
{
}

void* Carryable_funcs[20] = {(void*)0x00000000,
                             (void*)0x00000000,
                             (void*)0x00000000,
                             (void*)0x000E0000,
                             Carryable_initialise,
                             Carryable_release,
                             (void*)0x00000000,
                             Carryable_init,
                             Carryable_updateHeld,
                             Carryable_updateRenderState,
                             Carryable_free,
                             Carryable_getCarryState,
                             Carryable_wasJustGrabbed,
                             Carryable_getSurfaceType,
                             Carryable_setGravityEnabled,
                             Carryable_setDropDisabled,
                             Carryable_getDropDisabled,
                             Carryable_setSuppressPositionSave,
                             Carryable_stopCarrying,
                             (void*)0x00000000};
