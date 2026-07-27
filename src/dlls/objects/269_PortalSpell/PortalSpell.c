/*
 * PortalSpell (DLL 0x10D, descriptor gPortalSpellDoorObjDescriptor).
 */
#include "main/dll/dll_010D_portalspell.h"
#include "main/object_render.h"
#include "main/dll/player_api.h"
#include "sys/objects/lifecycle.h"
#include "main/dll/dll_80136a40.h"
#include "sys/objects.h"
#include "main/dll/portalspelldoorstate_struct.h"
#include "main/objseq.h"
#include "main/gamebits.h"
#include "main/frame_timing.h"
#include "game/objects/object_setup.h"

typedef struct PortalspelldoorPlacement
{
    ObjPlacement base;
    u8 pad18[0x1E - 0x18];
    s16 openedGameBit;
} PortalspelldoorPlacement;

STATIC_ASSERT(sizeof(PortalSpellDoorState) == 0x10);

#define PORTALSPELLDOOR_OBJFLAG_UPDATE_DISABLED    0x8000
#define PORTALSPELLDOOR_OBJFLAG_HIDDEN             0x4000
#define PORTALSPELLDOOR_OBJFLAG_HITDETECT_DISABLED 0x2000

extern f32 lbl_803E3A8C;
extern f32 lbl_803E3A90;
extern f32 lbl_803E3A88;

int PortalSpellDoor_getExtraSize(void)
{
    return 0x10;
}
int PortalSpellDoor_getObjectTypeId(void)
{
    return 0x0;
}

void PortalSpellDoor_free(void)
{
}

void PortalSpellDoor_render(int obj, int p2, int p3, int p4, int p5, s8 visible)
{
    s32 v = visible;
    if (v != 0)
        objRenderModelAndHitVolumes((GameObject*)obj, p2, p3, p4, p5, lbl_803E3A88);
}

void PortalSpellDoor_hitDetect(void)
{
}

void PortalSpellDoor_update(GameObject* obj)
{
    typedef struct
    {
        u8 open : 1;
    } PortalFlags;
    PortalSpellDoorState* state;
    int player;
    int p4c;
    int timer;

    player = (int)Obj_GetPlayerObject();
    state = obj->extra;
    p4c = *(int*)&obj->anim.placementData;
    if (playerHasSpell((GameObject*)(player), 3) != 0)
    {
        *(u8*)&obj->anim.resetHitboxMode &= ~INTERACT_FLAG_PROMPT_SUPPRESSED;
    }
    else
    {
        *(u8*)&obj->anim.resetHitboxMode |= INTERACT_FLAG_PROMPT_SUPPRESSED;
    }
    if (((PortalFlags*)&state->flags0C)->open)
    {
        obj->anim.flags |= OBJANIM_FLAG_HIDDEN;
        if (objGetAnimState80A((GameObject*)(player)) == 0x5bd)
        {
            playerCancelSpell((GameObject*)player, -1);
        }
        mainSetBits(((PortalspelldoorPlacement*)p4c)->openedGameBit, 1);
    }
    else
    {
        if (objGetAnimState80A((GameObject*)(player)) == 0x5bd && state->openTimer == -1)
        {
            state->openTimer = 0;
        }
    }
    if (state->openTimer != -1)
    {
        timer = state->openTimer - framesThisStep;
        state->openTimer = timer;
        if (timer < 0)
        {
            int tricky;
            *(u8*)&obj->anim.resetHitboxMode |= INTERACT_FLAG_DISABLED;
            (*gObjectTriggerInterface)->runSequence(0, (void*)obj, -1);
            tricky = (int)getTrickyObject();
            if ((void*)tricky != NULL)
            {
                trickyImpress((GameObject*)tricky);
            }
            ((PortalFlags*)&state->flags0C)->open = 1;
            state->openTimer = -1;
        }
    }
}

void PortalSpellDoor_init(GameObject* obj, u8* data)
{
    PortalSpellDoorState* state = obj->extra;
    obj->anim.rotX = (s16)((s32)(s8)data[0x18] << 8);
    obj->anim.rotY = (s16)((s32) * (s16*)(data + 0x1c) << 8);
    obj->anim.rootMotionScale = lbl_803E3A8C;
    {
        f32 _ab = obj->anim.hitboxScale * obj->anim.rootMotionScale;
        state->openAmount = _ab * lbl_803E3A90;
    }
    if (mainGetBit(*(s16*)(data + 0x1e)) != 0)
    {
        obj->anim.flags = (s16)(obj->anim.flags | OBJANIM_FLAG_HIDDEN);
        obj->objectFlags =
            (u16)(obj->objectFlags |
                  (PORTALSPELLDOOR_OBJFLAG_UPDATE_DISABLED | PORTALSPELLDOOR_OBJFLAG_HIDDEN |
                   PORTALSPELLDOOR_OBJFLAG_HITDETECT_DISABLED));
    }
    state->openTimer = -1;
}

void PortalSpellDoor_release(void)
{
}

void PortalSpellDoor_initialise(void)
{
}

ObjectDescriptor gPortalSpellDoorObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)PortalSpellDoor_initialise,
    (ObjectDescriptorCallback)PortalSpellDoor_release,
    0,
    (ObjectDescriptorCallback)PortalSpellDoor_init,
    (ObjectDescriptorCallback)PortalSpellDoor_update,
    (ObjectDescriptorCallback)PortalSpellDoor_hitDetect,
    (ObjectDescriptorCallback)PortalSpellDoor_render,
    (ObjectDescriptorCallback)PortalSpellDoor_free,
    (ObjectDescriptorCallback)PortalSpellDoor_getObjectTypeId,
    PortalSpellDoor_getExtraSize,
};
