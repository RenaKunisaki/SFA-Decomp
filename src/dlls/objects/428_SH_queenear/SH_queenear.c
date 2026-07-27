/*
 * SH_queenear (DLL 0x1AC) - the Queen EarthWalker in ThornTail
 * Hollow, the giant matriarch dinosaur the player tends.
 *
 * update() is driven by the area's map-event act: it picks the queen's
 * trigger-sequence event table for the current act, walks her toward the
 * player and runs idle/attention sequences. The feed and open-portal
 * sub-handlers cover the berry-feeding interaction (Y-button item 0x66d)
 * and the spell-portal opening. stateIndex selects the locomotion move
 * (gQueenEarthWalkerMoveTable/E24 tables); the flags byte tracks the per-frame mode.
 */
#include "sys/objects/lifecycle.h"
#include "main/gamebit_ids.h"
#include "game/objects/object.h"
#include "main/dll/player_api.h"
#include "sys/objects.h"
#include "main/objprint_anim_api.h"
#include "main/objprint_character_api.h"
#include "main/audio/sfx.h"
#include "main/dll/dll_0000_gameui_api.h"
#include "main/dll/SH/dll_01AC_shqueenearthwalker.h"
#include "main/mapEvent.h"
#include "main/obj_group.h"
#include "main/obj_trigger.h"
#include "main/objseq.h"
#include "main/gamebits.h"
#include "main/vecmath.h"
#include "main/frame_timing.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/audio/sfx_stop_channel_api.h"
#include "main/dll/tricky_api.h"
#include "main/objanim_update.h"
#include "dlls/object_descriptor.h"

u8 gQueenEarthWalkerEventTableAct1[4] = {1, 0, 0, 0};
u8 gQueenEarthWalkerEventTableAct2[4] = {1, 0x14, 0, 0};
u8 gQueenEarthWalkerEventTableFed[4] = {2, 0x0C, 0x0A, 0};
u8 gQueenEarthWalkerEventTableFeed[4] = {1, 0x0E, 0, 0};
u8 gQueenEarthWalkerEventTablePortalDefault[4] = {1, 0x0F, 0, 0};
u8 gQueenEarthWalkerEventTablePortalReady[4] = {1, 0x10, 0, 0};
u8 gQueenEarthWalkerEventTableSpell[4] = {1, 0x11, 0, 0};
u8 gQueenEarthWalkerEventTableBerry[4] = {1, 0x12, 0, 0};
u8 gQueenEarthWalkerEventTableDeparture[4] = {1, 0x13, 0, 0};
u8 gQueenEarthWalkerEventTableComplete[8] = {5, 7, 8, 9, 0x0A, 0x0B, 0, 0};

#define SHQUEENEARTHWALKER_OBJFLAG_HIDDEN 0x4000
/* object group scanned for the nearest target (player group) */
#define SHQUEENEARTHWALKER_TARGET_OBJGROUP 0xf

/* QueenEarthWalkerState::flags bits */
#define QEW_FLAG_STARTED   0x1  /* first update ran; per-act logic engaged */
#define QEW_FLAG_TARGETING 0x2  /* targeting the player */
#define QEW_FLAG_LATCHED   0x4  /* player position captured */
#define QEW_FLAG_EYE_ANIMS 0x8  /* run characterDoEyeAnims vs the bite */
#define QEW_FLAG_ACTIVE    0x10 /* feed sequence completed; suppress idle attacks */
#define QEW_FLAG_INIT_DONE 0x20 /* per-frame anim-event handshake (cleared each update) */

s16 gQueenEarthWalkerMoveTable[6] = {34, 34, 34, 5, 28, 0};
f32 gQueenEarthWalkerMoveSpeedTable[5] = {0.005f, 0.005f, 0.005f, 0.01f, 0.005f};

ObjectDescriptor gSH_queenearthwalkerObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    0,
    0,
    0,
    (ObjectDescriptorCallback)sh_queenearthwalker_init,
    (ObjectDescriptorCallback)sh_queenearthwalker_update,
    0,
    0,
    0,
    0,
    (ObjectDescriptorExtraSizeCallback)sh_queenearthwalker_getExtraSize,
};

/*
 * Processes animation events that drive the Queen's attack and feeding
 * behaviour. Event IDs 0/1 enable and disable the eye-animation branch;
 * events 2/3 enter and leave targeting, with event 3 also arming two
 * hit-volume pair bits.
 *
 * While targeting, the Queen latches the player's position once and then
 * either runs the bite or eye tracking according to QEW_FLAG_EYE_ANIMS.
 * QEW_FLAG_INIT_DONE is a one-shot guard that stops looping SFX channel 0x7f.
 */
int sh_queenearthwalker_processAnimEvents(GameObject* obj, void* unused, ObjAnimUpdateState* animUpdate)
{
    QueenEarthWalkerState* state = (obj)->extra;
    int i;
    u8 flags;

    if ((state->flags & QEW_FLAG_INIT_DONE) == 0)
    {
        Sfx_StopObjectChannel((int)obj, 0x7f);
        state->flags &= ~QEW_FLAG_ACTIVE;
        state->flags |= QEW_FLAG_INIT_DONE;
    }

    for (i = 0; i < animUpdate->eventCount; i++)
    {
        switch (animUpdate->eventIds[i])
        {
        case 0:
            state->flags |= QEW_FLAG_EYE_ANIMS;
            break;
        case 1:
            state->flags &= ~QEW_FLAG_EYE_ANIMS;
            break;
        case 2:
            state->flags |= QEW_FLAG_TARGETING;
            break;
        case 3:
            state->flags &= ~QEW_FLAG_TARGETING;
            animUpdate->hitVolumePair |= 0x8;
            animUpdate->hitVolumePair |= 0x40;
            break;
        }
    }

    flags = state->flags;
    if ((flags & QEW_FLAG_TARGETING) != 0)
    {
        if ((flags & QEW_FLAG_LATCHED) == 0)
        {
            GameObject* player;

            animUpdate->hitVolumePair &= ~0x8;
            player = Obj_GetPlayerObject();
            state->eyeAnimEnabled = 1;
            state->targetX = player->anim.localPosX;
            state->targetY = player->anim.localPosY;
            state->targetZ = player->anim.localPosZ;
            characterHeadLookCalm(obj, (s16*)((u8*)state + 0x8), 0.0f);
        }
        animUpdate->hitVolumePair &= ~0x40;
        if ((state->flags & QEW_FLAG_EYE_ANIMS) != 0)
        {
            characterCloseEyes(obj, (u8*)state + 0x8);
        }
        else
        {
            characterDoEyeAnims(obj, (u8*)state + 0x8);
        }
    }
    return 0;
}

void openPortalFn_801d4364(GameObject* obj, void* state)
{
    GameObject* player;

    player = Obj_GetPlayerObject();
    (obj)->anim.resetHitboxFlags &= ~INTERACT_FLAG_DISABLED;
    if (mainGetBit(0xc48) != 0)
    {
        ((QueenEarthWalkerState*)state)->eventTable = gQueenEarthWalkerEventTableComplete;
    }
    else if (mainGetBit(GAMEBIT_SH_Related023C) != 0)
    {
        ((QueenEarthWalkerState*)state)->eventTable = gQueenEarthWalkerEventTablePortalReady;
    }
    else if (mainGetBit(GAMEBIT_STAFF_ABILITY_OPEN_PORTAL) != 0)
    {
        (obj)->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
        if (playerHasSpell(player, 3) != 0 &&
            getXZDistance(&player->anim.worldPosX, &(obj)->anim.worldPosX) <
                1e+04f)
        {
            mainSetBits(0x23b, 1);
        }
    }
    else if (mainGetBit(GAMEBIT_SH_RescuedEggs) != 0)
    {
        ((QueenEarthWalkerState*)state)->eventTable = gQueenEarthWalkerEventTableComplete;
    }
    else
    {
        ((QueenEarthWalkerState*)state)->eventTable = gQueenEarthWalkerEventTablePortalDefault;
    }

    player = Obj_GetPlayerObject();
    ((u8*)state)[8] = 1;
    ((QueenEarthWalkerState*)state)->targetX = player->anim.localPosX;
    ((QueenEarthWalkerState*)state)->targetY = player->anim.localPosY;
    ((QueenEarthWalkerState*)state)->targetZ = player->anim.localPosZ;
    characterHeadLookCalm(obj, (s16*)((int)state + 0x8), 0.0f);
}

void queenFeedFn_801d44a4(GameObject* obj, void* state)
{
    s16 triggerId;
    s32 total;
    void* tricky;
    GameObject* player;

    switch (((QueenEarthWalkerState*)state)->stateIndex)
    {
    case 0:
        if (mainGetBit(GAMEBIT_SH_ReturnedToQueen) != 0)
        {
            (*gObjectTriggerInterface)->runSequence(1, obj, -1);
            ((QueenEarthWalkerState*)state)->stateIndex = 1;
        }
        break;
    case 1:
        (obj)->anim.resetHitboxFlags &= ~INTERACT_FLAG_DISABLED;
        if (cMenuGetSelectedItem() == -1)
        {
            if (getYButtonItem(&triggerId) == 0 || triggerId != 0x66d)
            {
                tricky = getTrickyObject();
                if (tricky != NULL && getXZDistance((f32*)((u8*)tricky + 0x18), &(obj)->anim.worldPosX) <
                                          2.25e+04f)
                {
                    Obj_SetActiveHitVolumeBounds(obj, 0, 0, 0, 0, 2);
                }
                else
                {
                    (obj)->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
                }
                break;
            }
        }
        Obj_SetActiveHitVolumeBounds(obj, 0, 0, 0, 0, 4);
        if (ObjTrigger_IsSetById((int)obj, 0x66d) != 0)
        {
            ((QueenEarthWalkerState*)state)->flags |= QEW_FLAG_ACTIVE;
            total = mainGetBit(GAMEBIT_ITEM_WhiteShroom_Count);
            total += mainGetBit(GAMEBIT_ITEM_WhiteGrubTub_Used);
            mainSetBits(GAMEBIT_ITEM_WhiteShroom_Count, 0);
            mainSetBits(GAMEBIT_ITEM_WhiteGrubTub_Used, total);
            if (total != 6)
            {
                ((QueenEarthWalkerState*)state)->flags |= QEW_FLAG_TARGETING;
                if (randomGetRange(0, 1) != 0)
                {
                    (*gObjectTriggerInterface)->runSequence(3, obj, -1);
                }
                else
                {
                    (*gObjectTriggerInterface)->runSequence(4, obj, -1);
                }
            }
            else
            {
                (*gObjectTriggerInterface)->runSequence(5, obj, -1);
                ((QueenEarthWalkerState*)state)->stateIndex = 2;
            }
        }
        break;
    case 2:
        (*gObjectTriggerInterface)->runSequence(6, obj, -1);
        mainSetBits(0x9e, 1);
        ((QueenEarthWalkerState*)state)->stateIndex = 3;
        break;
    case 3:
        Obj_SetActiveHitVolumeBounds(obj, 0, 0, 0, 0, 2);
        ((QueenEarthWalkerState*)state)->flags &= ~QEW_FLAG_LATCHED;
        ((QueenEarthWalkerState*)state)->flags &= ~QEW_FLAG_EYE_ANIMS;
        ((QueenEarthWalkerState*)state)->eventTable = gQueenEarthWalkerEventTableFed;
        player = Obj_GetPlayerObject();
        ((u8*)state)[0x8] = 1;
        ((QueenEarthWalkerState*)state)->targetX = player->anim.localPosX;
        ((QueenEarthWalkerState*)state)->targetY = player->anim.localPosY;
        ((QueenEarthWalkerState*)state)->targetZ = player->anim.localPosZ;
        characterHeadLookCalm(obj, (s16*)((int)state + 0x8), 0.0f);
        break;
    default:
        break;
    }
}

int sh_queenearthwalker_getExtraSize(void)
{
    return 0x40;
}

void sh_queenearthwalker_update(GameObject* obj)
{
    void* state;
    GameObject* player;
    void* target;
    u8 action;
    s8 mapSlot;
    u8 stateFlags;
    u8 eventIndex;
    int currentMove;
    s16 targetMove;

    state = (obj)->extra;
    ((QueenEarthWalkerState*)state)->flags &= ~QEW_FLAG_INIT_DONE;
    mapSlot = (obj)->anim.mapEventSlot;
    action = (*gMapEventInterface)->getMapAct(mapSlot);

    if ((((QueenEarthWalkerState*)state)->flags & QEW_FLAG_STARTED) != 0)
    {
        switch (action)
        {
        case 2:
            queenFeedFn_801d44a4(obj, state);
            break;
        case 3:
        case 4:
            if (mainGetBit(GAMEBIT_ITEM_MoonPassKey_Got) != 0)
            {
                ((QueenEarthWalkerState*)state)->eventTable = gQueenEarthWalkerEventTableComplete;
            }
            else
            {
                ((QueenEarthWalkerState*)state)->eventTable = gQueenEarthWalkerEventTableFeed;
            }
            player = Obj_GetPlayerObject();
            ((QueenEarthWalkerState*)state)->eyeAnimEnabled = 1;
            ((QueenEarthWalkerState*)state)->targetX = player->anim.localPosX;
            ((QueenEarthWalkerState*)state)->targetY = player->anim.localPosY;
            ((QueenEarthWalkerState*)state)->targetZ = player->anim.localPosZ;
            characterHeadLookCalm(obj, (s16*)((u8*)state + 0x8), 0.0f);
            break;
        case 5:
            openPortalFn_801d4364(obj, state);
            break;
        case 6:
            if (mainGetBit(GAMEBIT_ITEM_BigScarabBag_Got) != 0)
            {
                ((QueenEarthWalkerState*)state)->eventTable = gQueenEarthWalkerEventTableComplete;
            }
            else
            {
                ((QueenEarthWalkerState*)state)->eventTable = gQueenEarthWalkerEventTableSpell;
            }
            player = Obj_GetPlayerObject();
            ((QueenEarthWalkerState*)state)->eyeAnimEnabled = 1;
            ((QueenEarthWalkerState*)state)->targetX = player->anim.localPosX;
            ((QueenEarthWalkerState*)state)->targetY = player->anim.localPosY;
            ((QueenEarthWalkerState*)state)->targetZ = player->anim.localPosZ;
            characterHeadLookCalm(obj, (s16*)((u8*)state + 0x8), 0.0f);
            break;
        case 7:
            if (mainGetBit(0x199) != 0)
            {
                ((QueenEarthWalkerState*)state)->eventTable = gQueenEarthWalkerEventTableComplete;
            }
            else
            {
                ((QueenEarthWalkerState*)state)->eventTable = gQueenEarthWalkerEventTableBerry;
            }
            player = Obj_GetPlayerObject();
            ((QueenEarthWalkerState*)state)->eyeAnimEnabled = 1;
            ((QueenEarthWalkerState*)state)->targetX = player->anim.localPosX;
            ((QueenEarthWalkerState*)state)->targetY = player->anim.localPosY;
            ((QueenEarthWalkerState*)state)->targetZ = player->anim.localPosZ;
            characterHeadLookCalm(obj, (s16*)((u8*)state + 0x8), 0.0f);
            break;
        case 8:
            player = Obj_GetPlayerObject();
            ((QueenEarthWalkerState*)state)->eyeAnimEnabled = 1;
            ((QueenEarthWalkerState*)state)->targetX = player->anim.localPosX;
            ((QueenEarthWalkerState*)state)->targetY = player->anim.localPosY;
            ((QueenEarthWalkerState*)state)->targetZ = player->anim.localPosZ;
            characterHeadLookCalm(obj, (s16*)((u8*)state + 0x8), 0.0f);
            break;
        case 0:
        case 1:
        default:
            break;
        }
    }
    else
    {
        switch (action)
        {
        case 1:
            target = (void*)ObjGroup_FindNearestObject(SHQUEENEARTHWALKER_TARGET_OBJGROUP, obj, NULL);
            (*gObjectTriggerInterface)->preempt((int)target, 0x1324);
            (*gObjectTriggerInterface)->runSequence(1, target, 0x10);
            ((QueenEarthWalkerState*)state)->flags |= (QEW_FLAG_LATCHED | QEW_FLAG_EYE_ANIMS);
            ((QueenEarthWalkerState*)state)->eventTable = gQueenEarthWalkerEventTableAct1;
            break;
        case 2:
            if (mainGetBit(GAMEBIT_ITEM_WhiteGrubTub_Used) == 6)
            {
                (*gObjectTriggerInterface)->preempt((int)obj, 0x18f6);
                (*gObjectTriggerInterface)->runSequence(6, obj, 1);
                ((QueenEarthWalkerState*)state)->stateIndex = 3;
            }
            else
            {
                if (mainGetBit(GAMEBIT_SH_ReturnedToQueen) != 0)
                {
                    ((QueenEarthWalkerState*)state)->stateIndex = 1;
                }
                ((QueenEarthWalkerState*)state)->flags |= (QEW_FLAG_LATCHED | QEW_FLAG_EYE_ANIMS);
                ((QueenEarthWalkerState*)state)->eventTable = gQueenEarthWalkerEventTableAct2;
            }
            break;
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            (*gObjectTriggerInterface)->preempt((int)obj, 0x18f6);
            (*gObjectTriggerInterface)->runSequence(6, obj, 1);
            ((QueenEarthWalkerState*)state)->stateIndex = 3;
            break;
        case 8:
            target = (void*)ObjGroup_FindNearestObject(SHQUEENEARTHWALKER_TARGET_OBJGROUP, obj, NULL);
            (*gObjectTriggerInterface)->preempt((int)target, 0x6a4);
            (*gObjectTriggerInterface)->runSequence(7, target, 8);
            ((QueenEarthWalkerState*)state)->stateIndex = 4;
            ((QueenEarthWalkerState*)state)->eventTable = gQueenEarthWalkerEventTableDeparture;
            break;
        default:
            break;
        }
        ((QueenEarthWalkerState*)state)->flags |= QEW_FLAG_STARTED;
        return;
    }

    if ((((QueenEarthWalkerState*)state)->flags & QEW_FLAG_EYE_ANIMS) != 0)
    {
        characterCloseEyes(obj, (u8*)state + 0x8);
    }
    else
    {
        characterDoEyeAnims(obj, (u8*)state + 0x8);
    }

    currentMove = (obj)->anim.currentMove;
    targetMove = gQueenEarthWalkerMoveTable[((QueenEarthWalkerState*)state)->stateIndex];
    if (currentMove != targetMove)
    {
        ObjAnim_SetCurrentMove((int)obj, targetMove, 0.0f, 0);
    }
    ObjAnim_AdvanceCurrentMove(
        (int)obj, gQueenEarthWalkerMoveSpeedTable[((QueenEarthWalkerState*)state)->stateIndex], timeDelta, NULL);

    stateFlags = ((QueenEarthWalkerState*)state)->flags;
    if ((stateFlags & QEW_FLAG_ACTIVE) == 0)
    {
        ((QueenEarthWalkerState*)state)->flags &= ~QEW_FLAG_TARGETING;
        if (ObjTrigger_IsSet((int)obj) != 0 && obj->anim.hitVolumeBounds->flags != 4)
        {
            eventIndex = randomGetRange(1, *((QueenEarthWalkerState*)state)->eventTable);
            ((QueenEarthWalkerState*)state)->flags |= QEW_FLAG_TARGETING;
            (*gObjectTriggerInterface)
                ->runSequence(((u8*)((QueenEarthWalkerState*)state)->eventTable)[eventIndex], obj, -1);
        }
    }

    if (RandomTimer_UpdateRangeTrigger(&((QueenEarthWalkerState*)state)->attackTimer, 2.0f,
                                       5.0f) != 0)
    {
        Sfx_PlayFromObject((u32)obj, SFXTRIG_thorntail);
    }
}

void sh_queenearthwalker_init(GameObject* obj, QueenEarthWalkerMapData* mapData)
{
    obj->anim.rotX = (s16)(mapData->yawByte << 8);
    obj->animEventCallback = sh_queenearthwalker_processAnimEvents;
    obj->objectFlags |= SHQUEENEARTHWALKER_OBJFLAG_HIDDEN;
}
