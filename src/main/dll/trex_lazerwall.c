/*
 * trex_lazerwall - ThornTail/T-Rex "lazerwall" timed-challenge object.
 *
 * Three handlers, all installed as vtable slots by the spshopkeeper DLL
 * (dll_0286_spshopkeeper.c, lbl_803AD068[3..5]):
 *
 *   popQueuedState - advances the player along the rom-curve segment nearest
 *     the player, pushing a per-node kind onto the challenge's stack and
 *     popping the next queued state id.
 *   waitForStartBit - gate that returns 6 (a sequence/state id) once the
 *     challenge-start game bit is set.
 *   updateTimedChallenge - per-frame tick while the challenge runs. Queries
 *     the timer object (timerObj) for elapsed/limit times; when the game timer
 *     is disabled, the limit is reached, or the start tick fires, it stops the
 *     timer, clears the running bit, records win (limit reached) vs lose, pops
 *     up the HUD result, frees the object group and closes the title-menu HUD.
 *
 * Game bits owned/used here: 0x617 start, 0x624 win, 0x625 lose, 0x626 running.
 */
#include "main/mapEvent.h"
#include "sys/objects.h"
#include "game/objects/object.h"
#include "main/gamebits.h"
#include "main/objhits.h"
#include "main/dll/rom_curve_interface.h"
#include "main/dll/tricky_api.h"
#include "main/model_engine.h"
#include "main/dll/trex_lazerwall.h"
#include "main/dll/dll_0004_dummy04.h"

#define GAMEBIT_LAZERWALL_START   0x617
#define GAMEBIT_LAZERWALL_WIN     0x624
#define GAMEBIT_LAZERWALL_LOSE    0x625
#define GAMEBIT_LAZERWALL_RUNNING 0x626

/* challenge state-machine id returned to advance past the start gate */
#define WAITFORSTART_RESULT 6

/* node kinds pushed onto the challenge stack (rom-curve node tag 0xC == A) */
#define LAZERWALL_NODE_TAG_A  0xc
#define LAZERWALL_NODE_KIND_A 1
#define LAZERWALL_NODE_KIND_B 2

#define LAZERWALL_FLAG_ADVANCED 0x20 /* flags bit set after a curve advance */

extern f32 lbl_803E59DC;
extern const f32 lbl_803E59E0; /* curve-node Y bias */
extern u32 lbl_803E59D0;       /* head of the rom-curve search pair (first type id) */



int TREX_Lazerwall_popQueuedState(GameObject* obj, int animState)
{
    TREXLazerwallUpdateTimedChallengeState* state;
    GameObject* playerObj;
    RingBufferQueue* stackHandle;
    int node;
    u32 head[2];
    int pushKindA;
    int pushKindB;
    int popOut;

    *(RomCurveSearchPair*)head = *(RomCurveSearchPair*)&lbl_803E59D0;
    playerObj = Obj_GetPlayerObject();
    state = (obj)->extra;

    if (*(s8*)(animState + 0x27a) != 0)
    {
        if (Stack_IsEmpty(state->stack) != 0)
        {
            RomCurveFindFn findFn = (*gRomCurveInterface)->find;
            int found = findFn(playerObj->anim.localPosX, playerObj->anim.localPosY,
                               playerObj->anim.localPosZ, (int*)head, 2, -1);

            if (found != -1)
            {
                node = (int)(*gRomCurveInterface)->getById(found);
                (obj)->anim.localPosX = ((LazerwallCurveNode*)node)->x;
                (obj)->anim.localPosY = lbl_803E59E0 + ((LazerwallCurveNode*)node)->y;
                (obj)->anim.localPosZ = ((LazerwallCurveNode*)node)->z;
                *(s16*)(int)obj = (s16)((s32)((LazerwallCurveNode*)node)->rotZ << 8);
                state->nodeTargetY =
                    lbl_803E59E0 + ((LazerwallCurveNode*)node)->y;
                state->unk9CA = 0;
                state->curveNodeTag = ((LazerwallCurveNode*)node)->type;
            }

            if ((s8)((LazerwallCurveNode*)node)->type == LAZERWALL_NODE_TAG_A)
            {
                pushKindA = LAZERWALL_NODE_KIND_A;
                stackHandle = state->stack;
                if (Stack_IsFull(stackHandle) == 0)
                {
                    Stack_Push(stackHandle, &pushKindA);
                }
            }
            else
            {
                pushKindB = LAZERWALL_NODE_KIND_B;
                stackHandle = state->stack;
                if (Stack_IsFull(stackHandle) == 0)
                {
                    Stack_Push(stackHandle, &pushKindB);
                }
            }

            *(f32*)(animState + 0x280) = lbl_803E59DC;
            state->flags =
                (u8)(state->flags | LAZERWALL_FLAG_ADVANCED);
        }
    }

    state->popStateEnabled = 0xff;
    if (state->popStateEnabled == 0xff)
    {
        stackHandle = state->stack;
        popOut = 0;
        if (Stack_IsEmpty(stackHandle) == 0)
        {
            Stack_Pop(stackHandle, &popOut);
        }
        return popOut + 1;
    }
    return 0;
}

int TREX_Lazerwall_waitForStartBit(void)
{
    if (mainGetBit(GAMEBIT_LAZERWALL_START) != 0)
    {
        return WAITFORSTART_RESULT;
    }
    return 0;
}

int TREX_Lazerwall_updateTimedChallenge(GameObject* obj)
{
    TREXLazerwallUpdateTimedChallengeState* state;
    int elapsed;
    int now;
    int limit;

    state = (obj)->extra;
    *(u8*)&(obj)->anim.resetHitboxMode =
        (u8)(*(u8*)&(obj)->anim.resetHitboxMode | INTERACT_FLAG_DISABLED);
    state->popStateEnabled = 0;
    ObjHits_DisableObject(obj);

    (*(TimerQueryFn*)(*(int*)*(int*)(state->timerObj + 0x68) + 0x54))(
        state->timerObj, &elapsed, &now, &limit);

    now = now - elapsed;

    if (isGameTimerDisabled() != 0 || now >= limit || elapsed != 0)
    {
        gameTimerStop();
        hudFn_8011f6f0(0);
        mainSetBits(GAMEBIT_LAZERWALL_RUNNING, 0);

        if (now >= limit)
        {
            mainSetBits(GAMEBIT_LAZERWALL_WIN, 1);
        }
        else
        {
            mainSetBits(GAMEBIT_LAZERWALL_LOSE, 1);
        }

        hudFn_8011f38c(2);

        (*gMapEventInterface)->setObjGroupStatus((s32)(obj)->anim.mapEventSlot, 6, 0);

        gTitleMenuControlInterfaceCopy->vtable->func04(NULL, 0xf3, 0, 0, 0);
    }

    return 0;
}
