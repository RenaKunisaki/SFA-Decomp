/* DLL 0x01FE */
#include "dlls/object_descriptor.h"
#include "main/audio/sfx.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/dll/dll_01FE_pressureswitch.h"
#include "main/frame_timing.h"
#include "main/gamebits.h"
#include "main/mapEvent.h"
#include "main/object_render.h"
#include "main/vecmath.h"
#include "sys/objects.h"
#include "sys/objects/lifecycle.h"

typedef struct PressureswitchPlacement
{
    u8 pad0[0xC - 0x0];
    f32 restPosY; /* 0x0C: rest (up) Y position; pad sinks below this */
    u8 pad10[0x1A - 0x10];
    s16 unk1A;
    s16 triggerGameBit; /* 0x1C: game bit raised while the pad is pressed */
    s16 retriggerDelay; /* 0x1E: seconds before the pad can re-trigger (*60) */
    u8 pad20[0x4C - 0x20];
    u8 unk4C;
    u8 pad4D[0x2F8 - 0x4D];
    u8 unk2F8;
    u8 unk2F9;
    s8 unk2FA;
    u8 pad2FB[0x300 - 0x2FB];
} PressureswitchPlacement;

/* PressureSwitch_getExtraSize == 0x8. */
typedef struct PressureSwitchState
{
    s8 holdTimer; /* frames the switch stays pressed */
    s8 chimeLatch;
    s16 retriggerTimer;
    s16 mapGameBit; /* 0xf45/0xf46 per-map bit, -1 none */
    u8 flags;       /* PressureSwitchFlags overlay */
    u8 pad7;
} PressureSwitchState;

typedef struct PressureSwitchFlags
{
    u8 active : 1;        /* bit0: a trigger-type object (seqId 0x6d) is on the pad */
    u8 mapBitLatched : 1; /* bit1: map game bit latched on (not auto-cleared on release) */
    u8 otherFlags : 6;
} PressureSwitchFlags;

/* Re-derefs the +0x58 ObjAnimComponent.hitboxTransformState pointer per use. */
#define PSW_CONTACT_LIST(obj) ((ObjHitboxTransformState*)*(char**)((obj) + 0x58))

/* seqIds of objects this pad reacts to (compared against ent->anim.seqId). */
#define PSWITCH_TRIGGER_SEQ_ID 0x6d
#define PSWITCH_CHIME_SEQ_ID   0x146

int PressureSwitch_SeqFn(int obj, int unused, ObjAnimUpdateState* animUpdate)
{
    animUpdate->hitVolumePair = -1;
    animUpdate->sequenceEventActive = 0;
    return 0;
}

int PressureSwitch_getExtraSize(void)
{
    return 0x8;
}
int PressureSwitch_getObjectTypeId(void)
{
    return 0x0;
}

void PressureSwitch_free(void)
{
}

void PressureSwitch_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible)
{
    s32 isVisible = visible;
    if (isVisible != 0)
        objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, 1.0f);
}

void PressureSwitch_hitDetect(void)
{
}

void PressureSwitch_update(int obj)
{
    int byteOff[1];
    PressureswitchPlacement* placement;
    GameObject* self;
    PressureSwitchState* state;
    ObjHitboxTransformState* contacts;
    s8 playerFar;
    int i;
    GameObject* player;
    GameObject* tricky;
    s8 mapSlot;
    int bit;
    s8 moving;
    f32 curY;
    f32 pressedY;
    f32 heightThreshold;
    f32 posY;

    self = (GameObject*)obj;
    player = (GameObject*)Obj_GetPlayerObject();
    placement = (PressureswitchPlacement*)self->anim.placementData;
    state = self->extra;
    playerFar = 0;
    if (Vec_distance(&self->anim.worldPosX, &player->anim.worldPosX) > 100.0f)
    {
        playerFar = 1;
    }
    state->holdTimer -= 1;
    if (state->holdTimer < 0)
    {
        state->holdTimer = 0;
        state->chimeLatch = 0;
    }
    byteOff[0] = 0;
    ((PressureSwitchFlags*)&state->flags)->active = byteOff[0];
    if (PSW_CONTACT_LIST(obj) != NULL && PSW_CONTACT_LIST(obj)->contactObjectCount > 0)
    {
        state->retriggerTimer = (s16)(placement->retriggerDelay * 60);
        i = 0;
        heightThreshold = 7.0f;
        for (; i < (contacts = PSW_CONTACT_LIST(obj))->contactObjectCount; i++)
        {
            GameObject* ent = *(GameObject**)((char*)contacts + byteOff[0] + 256);
            if (ent->anim.seqId == PSWITCH_TRIGGER_SEQ_ID)
            {
                ((PressureSwitchFlags*)&state->flags)->active = 1;
            }
            if (ent->anim.localPosY - self->anim.localPosY > heightThreshold)
            {
                state->holdTimer = 5;
            }
            if (state->chimeLatch == 0 && ent != NULL && ent->anim.seqId == PSWITCH_CHIME_SEQ_ID)
            {
                if (playerFar == 0)
                {
                    Sfx_PlayFromObject(obj, SFXTRIG_mpick1_b);
                }
                state->chimeLatch = 1;
            }
            byteOff[0] += 4;
        }
    }
    else
    {
        mapSlot = self->anim.mapEventSlot;
        if (mapSlot == 11 && (*gMapEventInterface)->getMapAct(mapSlot) == 3 &&
            (tricky = (GameObject*)getTrickyObject()) != NULL &&
            Vec_distance(&self->anim.worldPosX, &tricky->anim.worldPosX) < 50.0f)
        {
            state->holdTimer = 5;
        }
    }
    mapSlot = self->anim.mapEventSlot;
    if (mapSlot == 11 && (*gMapEventInterface)->getMapAct(mapSlot) == 1 && playerFar == 0)
    {
        if (state->holdTimer != 0)
        {
            posY = placement->restPosY - self->anim.localPosY;
            if (posY > 2.5f && posY < 5.0f && mainGetBit(state->mapGameBit) == 0)
            {
                mainSetBits(GAMEBIT_WM_SwitchCamActive, 1);
            }
            else if (mainGetBit(GAMEBIT_WM_SwitchCamActive) != 0)
            {
                mainSetBits(GAMEBIT_WM_SwitchCamActive, 0);
            }
        }
        else if (mainGetBit(GAMEBIT_WM_SwitchCamActive) != 0)
        {
            mainSetBits(GAMEBIT_WM_SwitchCamActive, 0);
        }
    }
    moving = 0;
    if (state->holdTimer != 0)
    {
        pressedY = placement->restPosY - 5.0f;
        curY = self->anim.localPosY;
        if (curY < pressedY)
        {
            self->anim.localPosY = 0.25f * timeDelta + curY;
            if (self->anim.localPosY > pressedY)
            {
                self->anim.localPosY = pressedY;
            }
            mainSetBits(placement->triggerGameBit, 1);
            if (((PressureSwitchFlags*)&state->flags)->active)
            {
                mainSetBits(state->mapGameBit, 1);
            }
        }
        else
        {
            self->anim.localPosY = -(0.125f * timeDelta - curY);
            if (self->anim.localPosY < pressedY)
            {
                self->anim.localPosY = pressedY;
                mainSetBits(placement->triggerGameBit, 1);
                bit = state->mapGameBit;
                if (bit != -1)
                {
                    mainSetBits(bit, 1);
                    if (((PressureSwitchFlags*)&state->flags)->active)
                    {
                        ((PressureSwitchFlags*)&state->flags)->mapBitLatched = 1;
                    }
                }
            }
            else
            {
                moving = 1;
            }
        }
    }
    else
    {
        if (state->retriggerTimer == 0)
        {
            self->anim.localPosY = 0.125f * timeDelta + self->anim.localPosY;
            if (self->anim.localPosY > (posY = placement->restPosY))
            {
                self->anim.localPosY = posY;
            }
            else
            {
                moving = 1;
            }
            mainSetBits(placement->triggerGameBit, 0);
            bit = state->mapGameBit;
            if (bit != -1)
            {
                if (!((PressureSwitchFlags*)&state->flags)->mapBitLatched)
                {
                    mainSetBits(bit, 0);
                }
            }
        }
    }
    if (moving != 0)
    {
        Sfx_PlayFromObject(obj, SFXTRIG_en_treedrum16);
    }
    else
    {
        Sfx_StopObjectChannel(obj, 8);
    }
    if (state->retriggerTimer != 0)
    {
        state->retriggerTimer -= framesThisStep;
        if (state->retriggerTimer < 0)
        {
            state->retriggerTimer = 0;
        }
    }
}

void PressureSwitch_init(GameObject* obj, u8* init)
{
    PressureSwitchState* state;
    u32 mapId;

    state = obj->extra;
    obj->animEventCallback = PressureSwitch_SeqFn;
    obj->anim.rotX = (s16)((s8)init[0x18] << 8);
    state->retriggerTimer = (s16)(((PressureswitchPlacement*)init)->retriggerDelay * 0x3c);
    state->chimeLatch = 0;
    mapId = *(int*)(*(int*)&obj->anim.placementData + 0x14);
    if (mapId == 0x1f1a)
    {
        state->mapGameBit = 0xf45;
    }
    else if (mapId == 0x47293)
    {
        state->mapGameBit = 0xf46;
    }
    else
    {
        state->mapGameBit = -1;
    }
    if (state->mapGameBit != -1)
    {
        if (mainGetBit(state->mapGameBit) != 0)
        {
            ((PressureSwitchFlags*)&state->flags)->mapBitLatched = 1;
        }
    }
    if (mainGetBit(((PressureswitchPlacement*)init)->triggerGameBit) != 0)
    {
        obj->anim.localPosY = ((PressureswitchPlacement*)init)->restPosY - 25.0f;
        state->holdTimer = 0x1e;
    }
}

void PressureSwitch_release(void)
{
}

void PressureSwitch_initialise(void)
{
}

ObjectDescriptor gPressureSwitchObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)PressureSwitch_initialise,
    (ObjectDescriptorCallback)PressureSwitch_release,
    0,
    (ObjectDescriptorCallback)PressureSwitch_init,
    (ObjectDescriptorCallback)PressureSwitch_update,
    (ObjectDescriptorCallback)PressureSwitch_hitDetect,
    (ObjectDescriptorCallback)PressureSwitch_render,
    (ObjectDescriptorCallback)PressureSwitch_free,
    (ObjectDescriptorCallback)PressureSwitch_getObjectTypeId,
    PressureSwitch_getExtraSize,
};
