/*
 * DLL 0x0200 - an arwing-attachment / flying NPC whose behaviour
 * is selected by the current map-event mode for its placement slot
 * (gMapEventInterface->getMapAct). Per mode it plays idle/move anims,
 * lets the player interact (A-button) to spend magic and grant game
 * bits, runs trigger sequences (dll_200_SeqFn / dll_200_unlockFireBlasterSpell), and in
 * mode 2 (dll_200_updateAct2Wander) steers a wandering attachment toward scripted
 * targets (gArwingAttachmentTargets) via getAngle/sqrtf. Object body is Dll200State
 * (0x28); render scales through objRenderModelAndHitVolumes and gates on
 * GameBit 0x2bd when the placement's map-act is 4.
 */
#include "dlls/object_descriptor.h"
#include "main/dll/dll200state_struct.h"
#include "main/debug.h"
#include "main/game_ui_interface.h"
#include "game/objects/object.h"
#include "main/mapEvent.h"
#include "main/objHitReact.h"
#include "main/objanim_update.h"
#include "main/objseq.h"
#include "main/gamebits.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/frame_timing.h"
#include "main/object_render.h"
#include "sys/objects.h"
#include "main/pad.h"
#include "main/vecmath.h"
#include "main/gamebit_ids.h"
#include "main/dll/dll_0200_dll200.h"
#include "main/dll/player_api.h"
#include "main/dll/player_status.h"
#include "game/objects/object_setup.h"

STATIC_ASSERT(sizeof(Dll200State) == 0x28);

#define PAD_BUTTON_A 0x100

/* Dll200State.mode high bit: set while an ObjHitReact reaction is playing,
 * which suspends the normal map-act scripted update for that tick. */
#define DLL200_MODE_HITREACTING 0x80

const ItemIdSet3 gArwingAttachmentItemSetWander = {0x166, 0x167, 0x256};
const ItemIdSet3 gArwingAttachmentItemSetIdle = {0x166, 0x167, 0x256};
ObjHitReactEntry gArwingAttachmentHitReactTable[] = {
    {731, -1, -1, {0xFF, 0xFF}, 0, {0, 0, 0}, 0.0f, {0, 0, 0, 0}},
    {731, -1, -1, {0xFF, 0xFF}, 0, {0, 0, 0}, 0.0f, {0, 0, 0, 0}},
    {731, -1, -1, {0xFF, 0xFF}, 0, {0, 0, 0}, 0.0f, {0, 0, 0, 0}},
    {731, -1, -1, {0xFF, 0xFF}, 0, {0, 0, 0}, 0.0f, {0, 0, 0, 0}},
    {731, -1, -1, {0xFF, 0xFF}, 0, {0, 0, 0}, 0.0f, {0, 0, 0, 0}},
    {731, -1, -1, {0xFF, 0xFF}, 0, {0, 0, 0}, 0.0f, {0, 0, 0, 0}},
    {731, -1, -1, {0xFF, 0xFF}, 0, {0, 0, 0}, 0.0f, {0, 0, 0, 0}},
    {731, -1, -1, {0xFF, 0xFF}, 0, {0, 0, 0}, 0.0f, {0, 0, 0, 0}},
    {731, -1, -1, {0xFF, 0xFF}, 0, {0, 0, 0}, 0.0f, {0, 0, 0, 0}},
    {731, -1, -1, {0xFF, 0xFF}, 0, {0, 0, 0}, 0.0f, {0, 0, 0, 0}},
    {731, -1, -1, {0xFF, 0xFF}, 0, {0, 0, 0}, 0.0f, {0, 0, 0, 0}},
};
ArwAttachTarget gArwingAttachmentTargets[] = {
    {0.0f, 0.0f, 0.0f, 0.0f, 0.02f},       {79.0f, 152.0f, 20.0f, 20.0f, 0.01f}, {138.0f, -6.0f, 20.0f, 20.0f, 0.02f},
    {-73.0f, -48.0f, 20.0f, 20.0f, 0.02f}, {-248.0f, -7.0f, 0.0f, 0.0f, 0.02f},  {0.0f, 0.0f, 0.0f, 0.0f, 0.02f},
};
ObjectDescriptor dll_200 = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)dll_200_initialise_nop,
    (ObjectDescriptorCallback)dll_200_release_nop,
    0,
    (ObjectDescriptorCallback)dll_200_init,
    (ObjectDescriptorCallback)dll_200_update,
    (ObjectDescriptorCallback)dll_200_hitDetect_nop,
    (ObjectDescriptorCallback)dll_200_render,
    (ObjectDescriptorCallback)dll_200_free_nop,
    (ObjectDescriptorCallback)dll_200_getObjectTypeId,
    (ObjectDescriptorExtraSizeCallback)dll_200_getExtraSize_ret_40,
};

char sArwingAttachmentDiffFormat[9] = "diff %d\n";

void dll_200_updateAct6Idle(GameObject* obj)
{
    Dll200State* state;
    ItemIdSet3 itemSet;

    state = obj->extra;
    Obj_GetPlayerObject();
    itemSet = gArwingAttachmentItemSetIdle;
    if ((*(u8*)&(obj)->anim.resetHitboxMode & INTERACT_FLAG_DISABLED) != 0)
    {
        *(u8*)&(obj)->anim.resetHitboxMode ^= INTERACT_FLAG_DISABLED;
    }
    if (mainGetBit(763) == 0)
    {
        if ((obj)->anim.currentMove != 7)
        {
            ObjAnim_SetCurrentMove((int)obj, 7, 0.0f, 0);
        }
        ObjAnim_AdvanceCurrentMove((int)obj, 0.005f, (f32)(u32)framesThisStep,
                                                                     NULL);
    }
    else
    {
        if ((obj)->anim.currentMove != 2)
        {
            ObjAnim_SetCurrentMove((int)obj, 2, 0.0f, 0);
        }
        ObjAnim_AdvanceCurrentMove((int)obj, 0.005f, (f32)(u32)framesThisStep,
                                                                     NULL);
    }
    if ((*(u8*)&(obj)->anim.resetHitboxMode & INTERACT_FLAG_ACTIVATED) != 0 && mainGetBit(763) == 0)
    {
        mainSetBits(763, 1);
        state->counter27 = 0;
        buttonDisable(0, PAD_BUTTON_A);
    }
    else if ((*(u8*)&(obj)->anim.resetHitboxMode & INTERACT_FLAG_ACTIVATED) != 0)
    {
        if ((*gGameUIInterface)->isOneOfItemsBeingUsed((s32*)&itemSet, 3) > -1)
        {
            mainSetBits(784, 1);
            state->counter27 += 1;
            buttonDisable(0, PAD_BUTTON_A);
        }
    }
}

void dll_200_updateAct2Wander(GameObject* obj)
{
    Dll200State* state;
    u8 mode;
    s16 ang;
    s16 diff;
    f32 dx;
    f32 dy;
    f32 dist;
    f32 spd;
    ItemIdSet3 itemSet;
    ObjAnimEventList animEvents;

    state = obj->extra;
    Obj_GetPlayerObject();
    itemSet = gArwingAttachmentItemSetWander;
    obj->anim.localPosY = state->homeY;
    if (mainGetBit(GAMEBIT_WM_FoundKrystal) != 0)
    {
        *(u8*)&obj->anim.resetHitboxMode =
            (u8)(*(u8*)&obj->anim.resetHitboxMode & ~INTERACT_FLAG_DISABLED);
        if ((*(u8*)&obj->anim.resetHitboxMode & INTERACT_FLAG_ACTIVATED) != 0 &&
            (*gGameUIInterface)->isOneOfItemsBeingUsed((s32*)&itemSet, 3) > -1)
        {
            mainSetBits(0x4d1, 1);
            state->counter27 += 1;
            mainSetBits(0x310, 1);
            buttonDisable(0, PAD_BUTTON_A);
        }
    }
    else
    {
        *(u8*)&obj->anim.resetHitboxMode =
            (u8)(*(u8*)&obj->anim.resetHitboxMode | INTERACT_FLAG_DISABLED);
        if (state->modeTimer <= 0)
        {
            switch (randomGetRange(1, 4))
            {
            case 1:
                state->prevMode = state->mode;
                state->mode = 1;
                state->modeTimer = 400;
                break;
            case 2:
                state->prevMode = state->mode;
                state->mode = 2;
                state->modeTimer = 400;
                break;
            case 3:
                state->prevMode = state->mode;
                state->mode = 3;
                state->modeTimer = 400;
                break;
            case 4:
                state->prevMode = state->mode;
                state->mode = 4;
                state->modeTimer = 400;
                break;
            case 5:
                state->prevMode = state->mode;
                state->mode = 5;
                state->modeTimer = 400;
                break;
            }
        }
        else
        {
            mode = state->mode;
            if (mode == 12)
            {
                ang =
                    getAngle(gArwingAttachmentTargets[state->prevMode].x, gArwingAttachmentTargets[state->prevMode].y);
                diff = (s16)(ang - obj->anim.rotX);
                logPrintf(sArwingAttachmentDiffFormat, diff);
                if (diff < -1000 || diff > 1000)
                {
                    if (diff > 0)
                    {
                        obj->anim.rotX = (s16)(obj->anim.rotX + framesThisStep * 100);
                    }
                    else
                    {
                        obj->anim.rotX = (s16)(obj->anim.rotX - framesThisStep * 100);
                    }
                }
                else
                {
                    ObjAnim_SetCurrentMove((int)obj, gArwingAttachmentTargets[state->prevMode].moveId, 0.0f, 0);
                    state->animSpeed = gArwingAttachmentTargets[state->prevMode].speed;
                    state->mode = 13;
                }
            }
            else if (mode == 13)
            {
                if (ObjAnim_AdvanceCurrentMove((int)obj, state->animSpeed, timeDelta,
                                                                                 &animEvents) != 0)
                {
                    if ((f32)(int)obj->anim.currentMove ==
                        gArwingAttachmentTargets[state->prevMode].moveId)
                    {
                        ObjAnim_SetCurrentMove((int)obj, gArwingAttachmentTargets[state->prevMode].altMoveId, 0.0f,
                                               0);
                        state->animSpeed = gArwingAttachmentTargets[state->prevMode].speed;
                    }
                }
                state->modeTimer -= framesThisStep;
                if (state->modeTimer <= 0)
                {
                    state->modeTimer = 0;
                }
            }
            else
            {
                dx = gArwingAttachmentTargets[mode].x - (obj->anim.localPosX - state->homeX);
                dy = gArwingAttachmentTargets[mode].y - (obj->anim.localPosZ - state->homeZ);
                dist = sqrtf(dx * dx + dy * dy);
                ang = getAngle(dx, dy);
                diff = (s16)(ang - obj->anim.rotX);
                if (diff >= -1000 && diff <= 1000)
                {
                    if (obj->anim.currentMove != 59)
                    {
                        ObjAnim_SetCurrentMove((int)obj, 59, 0.0f, 0);
                        state->animSpeed = 0.04f;
                    }
                    spd = 0.25f;
                    obj->anim.velocityX = spd * (dx / dist);
                    obj->anim.velocityZ = spd * (dy / dist);
                    ObjAnim_SampleRootCurvePhase(&obj->anim, spd, &state->animSpeed);
                }
                else
                {
                    if (obj->anim.currentMove != 12)
                    {
                        ObjAnim_SetCurrentMove((int)obj, 12, 0.0f, 0);
                        state->animSpeed = 0.01f;
                    }
                    if (diff > 0)
                    {
                        obj->anim.rotX = (s16)(obj->anim.rotX + framesThisStep * 300);
                    }
                    else
                    {
                        obj->anim.rotX = (s16)(obj->anim.rotX - framesThisStep * 300);
                    }
                }
                if (dist < 4.0f)
                {
                    state->prevMode = state->mode;
                    state->mode = 12;
                    spd = 0.0f;
                    obj->anim.velocityX = spd;
                    obj->anim.velocityZ = spd;
                }
                obj->anim.localPosX =
                    obj->anim.velocityX * timeDelta + obj->anim.localPosX;
                obj->anim.localPosZ =
                    obj->anim.velocityZ * timeDelta + obj->anim.localPosZ;
                ObjAnim_AdvanceCurrentMove((int)obj, state->animSpeed, timeDelta,
                                                                             &animEvents);
            }
        }
    }
}

void dll_200_updateAct1Interact(GameObject* obj)
{
    Dll200State* state;

    state = obj->extra;
    if ((obj)->anim.currentMove != 2)
    {
        ObjAnim_SetCurrentMove((int)obj, 2, 0.0f, 0);
    }
    ObjAnim_AdvanceCurrentMove((int)obj, 0.005f, (f32)(u32)framesThisStep,
                                                                 NULL);
    state->latch24 = 1;
    if (state->latch24 == 0)
    {
        if ((*(u8*)&(obj)->anim.resetHitboxMode & INTERACT_FLAG_ACTIVATED) != 0)
        {
            mainSetBits(GAMEBIT_WM_GalleonRelated00D0, 1);
            state->latch24 = 1;
            buttonDisable(0, PAD_BUTTON_A);
        }
    }
    else
    {
        *(u8*)&(obj)->anim.resetHitboxMode &= ~INTERACT_FLAG_DISABLED;
        if ((*(u8*)&(obj)->anim.resetHitboxMode & INTERACT_FLAG_ACTIVATED) != 0)
        {
            GameObject* player = Obj_GetPlayerObject();
            if (playerGetCurMagic(player) > 0)
            {
                state->mode25 = 2;
                (*gObjectTriggerInterface)->runSequence(2, (void*)obj, -1);
                buttonDisable(0, PAD_BUTTON_A);
            }
            else
            {
                if (mainGetBit(177) == 0 || mainGetBit(178) == 0 || mainGetBit(179) == 0)
                {
                    state->mode25 = 1;
                    (*gObjectTriggerInterface)->runSequence(1, (void*)obj, -1);
                    buttonDisable(0, PAD_BUTTON_A);
                }
            }
        }
    }
}

int dll_200_unlockFireBlasterSpell(GameObject* obj, int unused, ObjAnimUpdateState* animUpdate, int unused2)
{
    Dll200State* state;
    GameObject* player;
    int i;

    player = Obj_GetPlayerObject();
    state = obj->extra;
    *(u8*)&obj->anim.resetHitboxMode =
        (u8)(*(u8*)&obj->anim.resetHitboxMode | INTERACT_FLAG_DISABLED);

    for (i = 0; i < animUpdate->eventCount; i++)
    {
        u8 mode = state->mode25;
        if (mode == 1)
        {
            if (animUpdate->eventIds[i] == 4)
            {
                playerAddRemoveMagic(player, 5);
            }
        }
        else if (mode != 2)
        {
            u8 eventId = animUpdate->eventIds[i];
            if (eventId == 1)
            {
                mainSetBits(GAMEBIT_WM_GalleonRelated00D0, 1);
                state->latch24 = 1;
            }
            else if (eventId == 2)
            {
                playerSetHaveSpell(player, 0, 1);
                playerAddRemoveMagic(player, 5);
            }
        }
    }
    return 0;
}
int dll_200_SeqFn(GameObject* obj, int unused, ObjAnimUpdateState* animUpdate, int unused2)
{
    u8 mode;
    int i;
    Dll200State* state;

    mode = (*gMapEventInterface)->getMapAct((int)obj->anim.mapEventSlot);
    switch (mode)
    {
    case 0:
        break;
    case 1:
        dll_200_unlockFireBlasterSpell(obj, unused, animUpdate, unused2);
        break;
    case 2:
        break;
    case 4:
        *(u8*)&obj->anim.resetHitboxMode = (u8)(*(u8*)&obj->anim.resetHitboxMode | INTERACT_FLAG_DISABLED);
        break;
    case 6:
        state = obj->extra;
        *(u8*)&obj->anim.resetHitboxMode = (u8)(*(u8*)&obj->anim.resetHitboxMode | INTERACT_FLAG_DISABLED);
        for (i = 0; i < animUpdate->eventCount; i++)
        {
            switch (animUpdate->eventIds[i])
            {
            case 0:
                break;
            case 1:
                if (state->counter27 >= 2)
                {
                    mainSetBits(0x314, 1);
                }
                break;
            }
        }
        break;
    }
    return 0;
}

int dll_200_getExtraSize_ret_40(void)
{
    return sizeof(Dll200State);
}

int dll_200_getObjectTypeId(void)
{
    return 0x1;
}

void dll_200_free_nop(void)
{
}

/* returns immediately if not visible; when the placement's map-act is 4,
 * gate render on GameBit 0x2bd, otherwise render directly via
 * objRenderModelAndHitVolumes. */
void dll_200_render(GameObject* obj, int p1, int p2, int p3, int p4, s8 visible)
{
    int areaId;
    if (visible == 0)
        return;
    areaId = (*gMapEventInterface)->getMapAct((int)obj->anim.mapEventSlot);
    if ((u8)areaId == 4)
    {
        if (mainGetBit(0x2bd) == 0u)
            return;
        objRenderModelAndHitVolumes(obj, p1, p2, p3, p4, 1.0f);
        return;
    }
    objRenderModelAndHitVolumes(obj, p1, p2, p3, p4, 1.0f);
}

void dll_200_hitDetect_nop(void)
{
}
void dll_200_update(int obj)
{
    u8 ev;
    u8 ret;
    Dll200State* state;
    GameObject* o = (GameObject*)obj;

    state = o->extra;
    ret = ObjHitReact_Update(obj, gArwingAttachmentHitReactTable, 11,
                             (u8)((state->mode & DLL200_MODE_HITREACTING) ? 1 : 0), &state->hitReactVec);
    if (ret != 0)
    {
        state->mode = (u8)(state->mode | DLL200_MODE_HITREACTING);
    }
    else
    {
        state->mode = (u8)(state->mode & ~DLL200_MODE_HITREACTING);
        ev = (*gMapEventInterface)->getMapAct((int)o->anim.mapEventSlot);
        switch (ev)
        {
        case 1:
            dll_200_updateAct1Interact((GameObject*)(obj));
            break;
        case 2:
            dll_200_updateAct2Wander((GameObject*)obj);
            break;
        case 4:
            *(u8*)&o->anim.resetHitboxMode =
                (u8)(*(u8*)&o->anim.resetHitboxMode | INTERACT_FLAG_DISABLED);
            if (o->anim.currentMove != 2)
            {
                ObjAnim_SetCurrentMove(obj, 2, 0.0f, 0);
            }
            ObjAnim_AdvanceCurrentMove((int)obj, 0.005f, (f32)(u32)framesThisStep,
                                                                         NULL);
            break;
        case 6:
            dll_200_updateAct6Idle((GameObject*)(obj));
            break;
        case 0:
        case 3:
        case 5:
            return;
        }
    }
}

void dll_200_init(GameObject* obj, Dll200Placement* def)
{
    Dll200State* state;
    obj->userData1 = 0;
    obj->anim.rotX = (s16)((s32)def->rotXByte << 8);
    obj->animEventCallback = dll_200_SeqFn;
    state = obj->extra;
    state->defNoLow = (u8)def->head.objectId;
    state->unk1C = 0;
    state->unk18 = 0;
    state->homeX = def->head.posX;
    state->homeY = def->head.posY;
    state->homeZ = def->head.posZ;
    state->latch24 = mainGetBit(GAMEBIT_WM_GalleonRelated00D0);
    state->counter27 = 0;
    state->mode = 1;
    state->prevMode = 0xc;
    state->modeTimer = 0x12c;
    state->animSpeed = 0.0f;
    state->unk14 = 1.0f;
}

void dll_200_release_nop(void)
{
}

void dll_200_initialise_nop(void)
{
}


