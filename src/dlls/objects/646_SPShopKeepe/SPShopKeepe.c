/*
 * SPShopKeepe (DLL 646) - the SnowHorn shopkeeper vendor character.
 *
 * The TU contains the shopkeeper's auxiliary state handlers, the T-Rex
 * lazerwall challenge handlers, the DR laser-turret callbacks, and the
 * shopkeeper object implementation.
 */
#include "main/track_dolphin_api.h"
#include "main/dll/DR/DRlaserturret.h"
#include "main/dll/trex_lazerwall.h"
#include "main/dll/dll_0004_dummy04.h"
#include "main/dll/rom_curve_interface.h"
#include "main/dll/boneparticleeffect_interface.h"
#include "main/dll/shopkeeperstate_struct.h"
#include "main/dll/pushcartstate97_types.h"
#include "main/frame_timing.h"
#include "main/gamebits.h"
#include "main/mapEvent.h"
#include "main/model_engine.h"
#include "main/objanim.h"
#include "main/objhits.h"
#include "main/objprint_character_api.h"
#include "main/obj_trigger.h"
#include "game/objects/object_setup.h"
#include "main/dll/dll_002E_moveLib.h"
#include "sys/objects/lifecycle.h"
#include "sys/objects.h"
#include "main/objseq.h"
#include "main/objtexture.h"
#include "main/pad.h"
#include "main/player_control_interface.h"
#include "main/rcp_dolphin.h"
#include "main/screen_transition.h"
#include "main/objtype.h"
#include "main/vecmath.h"
#include "main/dll/SP/dll_0286_spshopkeeper.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_float_helpers.h"
#include "main/object_render.h"
#include "dlls/object_descriptor.h"

typedef struct ShopKeeperShopInterface
{
    void* pad00[12];
    int (*getItemMinPrice)(GameObject* shop, int slot);
    void* pad34;
    int (*getItemPrice)(GameObject* shop, int slot);
    void* pad3C[2];
    int (*getItemIndex)(GameObject* shop);
} ShopKeeperShopInterface;

STATIC_ASSERT(offsetof(ShopKeeperShopInterface, getItemMinPrice) == 0x30);
STATIC_ASSERT(offsetof(ShopKeeperShopInterface, getItemPrice) == 0x38);
STATIC_ASSERT(offsetof(ShopKeeperShopInterface, getItemIndex) == 0x44);

#define SHOPKEEPER_SHOP_INTERFACE(shop) (*(ShopKeeperShopInterface**)((GameObject*)(shop))->anim.dll)

void ShopKeeper_spawnScarabs(GameObject* obj, int state, int count);
int ShopKeeper_getExtraSize(void);
int ShopKeeper_getObjectTypeId(void);
void ShopKeeper_free(GameObject* obj);
void ShopKeeper_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible);
void ShopKeeper_hitDetect(void);
void ShopKeeper_update(GameObject* obj);
void ShopKeeper_init(GameObject* obj);
void ShopKeeper_release(void);
void ShopKeeper_initialise(void);

ObjectDescriptor gShopKeeperObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)ShopKeeper_initialise,
    (ObjectDescriptorCallback)ShopKeeper_release,
    0,
    (ObjectDescriptorCallback)ShopKeeper_init,
    (ObjectDescriptorCallback)ShopKeeper_update,
    (ObjectDescriptorCallback)ShopKeeper_hitDetect,
    (ObjectDescriptorCallback)ShopKeeper_render,
    (ObjectDescriptorCallback)ShopKeeper_free,
    (ObjectDescriptorCallback)ShopKeeper_getObjectTypeId,
    (ObjectDescriptorExtraSizeCallback)ShopKeeper_getExtraSize,
};

const RomCurveSearchPair gShopKeeperCurveSearchKinds = {0xC, 0x1C};

void* gShopKeeperStateHandlers[8];

s16 gDrLaserTurretIdleAnimMoves[2] = {0x13, 0x11};
f32 gDrLaserTurretIdleAnimStepScales[2] = {0.01f, 0.0125f};

#define DLL801E66DC_OBJFLAG_RENDERED 0x800

int ShopKeeper_defaultStateHandler(void)
{
    return 0;
}

int ShopKeeper_state7Handler(void)
{
    return 0;
}

int ShopKeeper_popQueuedState(int objHandle, int animState)
{
    GameObject* obj = (GameObject*)objHandle;
    int state;
    f32 spawnParam;
    RingBufferQueue* stk;
    int nextState;

    state = (int)obj->extra;
    spawnParam = 1.0f;

    if (((BaddieState*)animState)->moveJustStartedA != 0)
    {
        if ((obj->objectFlags & DLL801E66DC_OBJFLAG_RENDERED) != 0)
        {
            (*gBoneParticleEffectInterface)->spawnEffect((void*)obj, 2031, &spawnParam, 80, NULL);
        }
    }

    ((ShopkeeperState*)state)->opacity = 0;
    ((BaddieState*)animState)->animSpeedA = 0.0f;
    if (((ShopkeeperState*)state)->opacity == 0)
    {
        stk = ((ShopkeeperState*)state)->msgStack;
        nextState = 0;
        if (Stack_IsEmpty(stk) == 0)
        {
            Stack_Pop(stk, &nextState);
        }
        return nextState + 1;
    }
    return 0;
}

#define GAMEBIT_LAZERWALL_START   0x617
#define GAMEBIT_LAZERWALL_WIN     0x624
#define GAMEBIT_LAZERWALL_LOSE    0x625
#define GAMEBIT_LAZERWALL_RUNNING 0x626

#define WAITFORSTART_RESULT 6

#define LAZERWALL_NODE_TAG_A  0xc
#define LAZERWALL_NODE_KIND_A 1
#define LAZERWALL_NODE_KIND_B 2

#define LAZERWALL_FLAG_ADVANCED 0x20

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

    *(RomCurveSearchPair*)head = gShopKeeperCurveSearchKinds;
    playerObj = Obj_GetPlayerObject();
    state = (obj)->extra;

    if (((BaddieState*)animState)->moveJustStartedA != 0)
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
                (obj)->anim.localPosY = 6.0f + ((LazerwallCurveNode*)node)->y;
                (obj)->anim.localPosZ = ((LazerwallCurveNode*)node)->z;
                *(s16*)(int)obj = (s16)((s32)((LazerwallCurveNode*)node)->rotZ << 8);
                state->nodeTargetY = 6.0f + ((LazerwallCurveNode*)node)->y;
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

            ((BaddieState*)animState)->animSpeedA = 0.0f;
            state->flags = (u8)(state->flags | LAZERWALL_FLAG_ADVANCED);
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
    (obj)->anim.resetHitboxFlags =
        (u8)((obj)->anim.resetHitboxFlags | INTERACT_FLAG_DISABLED);
    state->popStateEnabled = 0;
    ObjHits_DisableObject(obj);

    (*(TimerQueryFn*)(*(int*)*(int*)(state->timerObj + 0x68) + 0x54))(
        state->timerObj, &elapsed, &now, &limit);

    now = now - elapsed;

    if (isGameTimerDisabled() != 0 || now >= limit || elapsed != 0)
    {
        gameTimerStop();
        setTrickyHudShowNearestInfo(0);
        mainSetBits(GAMEBIT_LAZERWALL_RUNNING, 0);

        if (now >= limit)
        {
            mainSetBits(GAMEBIT_LAZERWALL_WIN, 1);
        }
        else
        {
            mainSetBits(GAMEBIT_LAZERWALL_LOSE, 1);
        }

        setHudForceShowMask(2);

        (*gMapEventInterface)->setObjGroupStatus((s32)(obj)->anim.mapEventSlot, 6, 0);

        gTitleMenuControlInterfaceCopy->vtable->func04(NULL, 0xf3, 0, 0, 0);
    }

    return 0;
}

int DRlaserturret_updateIdle(GameObject* obj, DRLaserTurretAnimState* animState)
{
    void* playerObj;
    DRLaserTurretState* state;
    void* stack;
    int pushState;
    int sum;
    int rng;

    playerObj = Obj_GetPlayerObject();
    state = obj->extra;
    state->promptState = 0xff;
    animState->animStepScale = 0.007f;
    if (obj->anim.currentMove != 0)
    {
        ObjAnim_SetCurrentMove((int)obj, DR_LASERTURRET_ANIM_IDLE, 0.0f, 0);
    }
    ObjHits_EnableObject((GameObject*)obj);
    obj->anim.resetHitboxFlags &= ~INTERACT_FLAG_DISABLED;
    if (mainGetBit(DR_LASERTURRET_GAMEBIT_SHOP_OPEN) == 0)
    {
        pushState = DR_LASERTURRET_STATE_PUSH_IDLE;
        stack = state->stateStack;
        if (Stack_IsFull(stack) == 0)
        {
            Stack_Push(stack, &pushState);
        }
        return DR_LASERTURRET_STATE_CONTINUE;
    }
    ShopKeeper_turnTowardPlayer(obj, playerObj, 0);
    obj->anim.localPosY = state->bobAmplitude *
                 mathSinf((double)(3.1415927f * (float)(u32)state->bobPhase / 32768.0f)) +
             state->bobBaseY;
    sum = state->bobPhase + framesThisStep * 0x100;
    if (sum > 0xffff)
    {
        float rngf;
        rng = randomGetRange(0xf, 0x23);
        rngf = (float)rng;
        rngf = 0.1f * rngf;
        state->bobAmplitude = rngf;
    }
    state->bobPhase = sum;
    if ((obj->anim.resetHitboxFlags & INTERACT_FLAG_ACTIVATED) != 0)
    {
        if (playerGetMoney(playerObj) >= 1)
        {
            mainSetBits(DR_LASERTURRET_GAMEBIT_HAS_MONEY, 1);
            buttonDisable(0, DR_LASERTURRET_BUTTON_ACCEPT);
        }
        else
        {
            rng = randomGetRange(0, 2);
            (*gObjectTriggerInterface)->runSequence(rng, obj, -1);
            buttonDisable(0, DR_LASERTURRET_BUTTON_ACCEPT);
        }
    }
    return 0;
}

int DRlaserturret_updateTracking(GameObject* obj, DRLaserTurretAnimState* animState)
{
    void* playerObj;
    DRLaserTurretState* state;
    void* stack;
    TrackGroundHit** arr;
    int pushState;
    int sum;
    int rng;
    float dist;
    float minDist;
    int count;
    int idx;
    f32 t;
    float rate;
    float target;
    float d;

    playerObj = Obj_GetPlayerObject();
    state = obj->extra;
    if (animState->stateEntered != 0)
    {
        rng = randomGetRange(0x1f4, 0x3e8);
        state->actionTimer = rng;
        state->flags = state->flags & ~DR_LASERTURRET_FLAG_ACTION_ACTIVE;
    }
    if ((state->flags & DR_LASERTURRET_FLAG_ACTION_ACTIVE) != 0)
    {
        if (animState->moveComplete != 0)
        {
            if (obj->anim.currentMove == DR_LASERTURRET_ANIM_TRACKING && animState->animStepScale > 0.0f)
            {
                ObjAnim_SetCurrentMove((int)obj, DR_LASERTURRET_ANIM_ALERT, 0.0f, 0);
            }
            else if (obj->anim.currentMove != 0)
            {
                ObjAnim_SetCurrentMove((int)obj, DR_LASERTURRET_ANIM_IDLE, 0.0f, 0);
            }
            animState->animStepScale = 0.007f;
            state->flags = state->flags & ~DR_LASERTURRET_FLAG_ACTION_ACTIVE;
            rng = randomGetRange(0x1f4, 0x3e8);
            state->actionTimer = rng;
        }
    }
    else
    {
        if (obj->anim.currentMove != DR_LASERTURRET_ANIM_ALERT && obj->anim.currentMove != 0)
        {
            ObjAnim_SetCurrentMove((int)obj, DR_LASERTURRET_ANIM_IDLE, 0.0f, 0);
            animState->animStepScale = 0.007f;
        }
    }
    state->actionTimer = state->actionTimer - timeDelta;
    if (state->actionTimer <= 0.0f && (state->flags & DR_LASERTURRET_FLAG_ACTION_ACTIVE) == 0)
    {
        Sfx_PlayFromObject((int)obj, DR_LASERTURRET_SFX_ACTION);
        if (obj->anim.currentMove == DR_LASERTURRET_ANIM_ALERT)
        {
            ObjAnim_SetCurrentMove((int)obj, DR_LASERTURRET_ANIM_TRACKING, 0.99f, 0);
            animState->animStepScale = -0.0125f;
        }
        else
        {
            rng = randomGetRange(0, 1);
            ObjAnim_SetCurrentMove((int)obj, gDrLaserTurretIdleAnimMoves[rng], 0.0f, 0);
            animState->animStepScale = gDrLaserTurretIdleAnimStepScales[rng];
        }
        state->flags = state->flags | DR_LASERTURRET_FLAG_ACTION_ACTIVE;
    }
    if (mainGetBit(DR_LASERTURRET_GAMEBIT_SHOP_OPEN) == 0)
    {
        pushState = DR_LASERTURRET_STATE_PUSH_TRACKING;
        stack = state->stateStack;
        if (Stack_IsFull(stack) == 0)
        {
            Stack_Push(stack, &pushState);
        }
        return DR_LASERTURRET_STATE_CONTINUE;
    }
    t = ShopKeeper_turnTowardPlayer(obj, playerObj, 0);
    rate = 0.02f;
    if (t > 80.0f)
    {
        target = -0.9f;
    }
    else
    {
        target = 0.0f;
    }
    d = rate * (target - animState->aimBlend);
    animState->aimBlend += d * timeDelta;
    if (animState->aimBlend > -0.002f)
    {
        animState->aimBlend = 0.0f;
    }
    animState->aimBlend = 0.0f;
    count = trackGetHeight(obj, obj->anim.localPosX, obj->anim.localPosY, obj->anim.localPosZ, &arr, 0, 0);
    minDist = 10000.0f;
    for (idx = 0; idx < count; idx++)
    {
        dist = arr[idx]->height - obj->anim.localPosY;
        if (dist < 0.0f)
        {
            dist = -dist;
        }
        if (dist < minDist)
        {
            state->bobBaseY = 6.0f + arr[idx]->height;
            minDist = dist;
        }
    }
    obj->anim.localPosY = state->bobAmplitude *
                 mathSinf((double)(3.1415927f * (float)(u32)state->bobPhase / 32768.0f)) +
             state->bobBaseY;
    sum = state->bobPhase + framesThisStep * 0x100;
    if (sum > 0xffff)
    {
        float rngf;
        rng = randomGetRange(0xf, 0x23);
        rngf = (float)rng;
        rngf = 0.1f * rngf;
        state->bobAmplitude = rngf;
    }
    state->bobPhase = sum;
    if (ObjTrigger_IsSet((int)obj) != 0)
    {
        rng = randomGetRange(0, 2);
        (*gObjectTriggerInterface)->runSequence(rng, obj, -1);
    }
    return 0;
}

int DRlaserturret_startLinkedTarget(GameObject* obj)
{
    DRLaserTurretState* state;

    state = obj->extra;
    if (mainGetBit(DR_LASERTURRET_GAMEBIT_LINK_READY) == 0)
    {
        return 0;
    }
    if ((int)mainGetBit(DR_LASERTURRET_GAMEBIT_LINK_STARTED) == 0)
    {
        int* target;
        mainSetBits(DR_LASERTURRET_GAMEBIT_LINK_STARTED, 1);
        target = state->linkedTarget;
        (**(VtableFn***)((char*)target + 0x68))[0x24 / 4](target, 1, 2);
    }
    return DR_LASERTURRET_STATE_LINKED_TARGET;
}

int DRlaserturret_handlePromptChoice(GameObject* obj, void* param2, int dispatch)
{
    DRLaserTurretState* state;
    s8 stickHi;
    s8 stickLo;
    int btn;
    int cv;
    char nudge;
    ObjTextureRuntimeSlot* texture;

    state = obj->extra;
    if (dispatch == DR_LASERTURRET_PROMPT_COUNT)
    {
        padGetAnalogInput(0, &stickHi, &stickLo);
        if (stickLo < 0)
        {
            state->countValue--;
            Sfx_PlayFromObject(0, DR_LASERTURRET_SFX_PROMPT_TICK);
        }
        else if (stickLo > 0)
        {
            state->countValue++;
            Sfx_PlayFromObject(0, DR_LASERTURRET_SFX_PROMPT_TICK);
        }
        if (state->countValue > state->maxCount)
        {
            state->countValue = state->maxCount;
        }
        if (state->countValue > state->countScale << 1)
        {
            state->countValue = (s16)(state->countScale << 1);
        }
        else if (state->countValue < state->countScale >> 1)
        {
            state->countValue = (s16)(state->countScale >> 1);
        }
        cv = state->countValue;
        texture = objFindTexture((GameObject*)(obj), DR_LASERTURRET_ONES_TEXTURE_SLOT, 0);
        texture->textureId = (cv % 10) << DR_LASERTURRET_DIGIT_TEXTURE_SHIFT;
        texture = objFindTexture((GameObject*)(obj), DR_LASERTURRET_TENS_TEXTURE_SLOT, 0);
        texture->textureId = ((cv / 10) % 10) << DR_LASERTURRET_DIGIT_TEXTURE_SHIFT;
        cv = cv / 100;
        if (cv > DR_LASERTURRET_MAX_DIGIT)
            cv = DR_LASERTURRET_MAX_DIGIT;
        texture = objFindTexture((GameObject*)(obj), DR_LASERTURRET_HUNDREDS_TEXTURE_SLOT, 0);
        texture->textureId = cv << DR_LASERTURRET_DIGIT_TEXTURE_SHIFT;
    }
    else if (dispatch == DR_LASERTURRET_PROMPT_DIGIT_COUNT)
    {
        padGetAnalogInput(0, &stickHi, &stickLo);
        if (stickLo < 0)
        {
            state->digitCount--;
            Sfx_PlayFromObject(0, DR_LASERTURRET_SFX_PROMPT_TICK);
        }
        else if (stickLo > 0)
        {
            state->digitCount++;
            Sfx_PlayFromObject(0, DR_LASERTURRET_SFX_PROMPT_TICK);
        }
        if (state->digitCount > state->maxCount)
        {
            state->digitCount = state->maxCount;
        }
        if (state->digitCount > DR_LASERTURRET_MAX_DIGIT_COUNT)
        {
            state->digitCount = DR_LASERTURRET_MAX_DIGIT_COUNT;
        }
        else if (state->digitCount < DR_LASERTURRET_MIN_DIGIT_COUNT)
        {
            state->digitCount = DR_LASERTURRET_MIN_DIGIT_COUNT;
        }
        {
            cv = state->digitCount;
            texture = objFindTexture((GameObject*)(obj), DR_LASERTURRET_ONES_TEXTURE_SLOT, 0);
            texture->textureId = (cv % 10) << DR_LASERTURRET_DIGIT_TEXTURE_SHIFT;
            texture = objFindTexture((GameObject*)(obj), DR_LASERTURRET_TENS_TEXTURE_SLOT, 0);
            texture->textureId = ((cv / 10) % 10) << DR_LASERTURRET_DIGIT_TEXTURE_SHIFT;
            cv = cv / 100;
            if (cv > DR_LASERTURRET_MAX_DIGIT)
                cv = DR_LASERTURRET_MAX_DIGIT;
            texture = objFindTexture((GameObject*)(obj), DR_LASERTURRET_HUNDREDS_TEXTURE_SLOT, 0);
            texture->textureId = cv << DR_LASERTURRET_DIGIT_TEXTURE_SHIFT;
        }
        btn = getButtonsJustPressed(0);
        if ((btn & DR_LASERTURRET_BUTTON_CANCEL) != 0u)
        {
            state->flags = state->flags | DR_LASERTURRET_FLAG_CONFIRM_PROMPT;
            (*gScreenTransitionInterface)->start(0x1e, 1);
            return 1;
        }
    }
    btn = getButtonsJustPressed(0);
    if ((btn & DR_LASERTURRET_BUTTON_ACCEPT) == 0u)
    {
        return 0;
    }
    cv = state->countValue;
    if (cv < state->countTarget)
    {
        nudge = (state->nudgeCount < DR_LASERTURRET_MAX_NUDGE_COUNT) ? 0 : 2;
    }
    else
    {
        nudge = 1;
    }
    switch (dispatch)
    {
    case DR_LASERTURRET_PROMPT_COUNT:
        if ((s8)nudge == 0)
        {
            state->nudgeCount++;
        }
        return nudge == 0;
    case DR_LASERTURRET_PROMPT_NUDGE:
        if ((s8)nudge == 1)
        {
            int* target = state->linkedTarget;
            (**(VtableFn***)((char*)target + 0x68))[0x48 / 4](target, cv);
        }
        return nudge == 1;
    case DR_LASERTURRET_PROMPT_MAX_NUDGE:
        return nudge == 2;
    }
    return 0;
}

void DRlaserturret_startTimedChallenge(GameObject* obj)
{
    DRLaserTurretState* state;

    state = obj->extra;
    if ((state->flags & DR_LASERTURRET_FLAG_START_SEQUENCE) != 0)
    {
        int* target;
        gameTimerInit(0x11, 0x1e);
        timerSetToCountUp();
        setTrickyHudShowNearestInfo(1);
        mainSetBits(DR_LASERTURRET_GAMEBIT_TIMER_STARTED, 1);
        target = state->linkedTarget;
        (**(VtableFn***)((char*)target + 0x68))[0x4c / 4](target, state->digitCount);
        gTitleMenuControlInterfaceCopy->vtable->func04(NULL, 0xf5, 0, 0, 0);
    }
    else
    {
        setHudForceShowMask(0);
    }
    state->flags = 0;
}

#define SPSHOPKEEPER_OBJFLAG_HITDETECT_DISABLED 0x2000

#define SPSHOPKEEPER_TARGET_OBJGROUP 9

STATIC_ASSERT(sizeof(ShopItemState) == 0xEC);

STATIC_ASSERT(sizeof(ShopkeeperState) == 0x9D8);
STATIC_ASSERT(offsetof(ShopkeeperState, msgStack) == 0x9B0);

/* Obj_AllocObjectSetup(36,...) buffer composed in ShopKeeper_spawnScarabs. Head is the
 * common ObjPlacement; ident slot (0x14) is repurposed as an int (vendorObj),
 * tail (0x18..0x1B) is file-local. */
typedef struct ShopkeeperSpawnSetup
{
    ObjPlacement base; /* 0x00..0x17 */
    s8 rotXByte;       /* 0x18: scarab spawn rotX (1/256 turns) */
    u8 kind;           /* 0x19: scarab variant (see SpscarabPlacement.kind) */
    s16 groundY;       /* 0x1A: scarab ground-height delta (see SpscarabState.groundY) */
    u8 pad1C[0x24 - 0x1C];
} ShopkeeperSpawnSetup;

STATIC_ASSERT(offsetof(ShopkeeperSpawnSetup, rotXByte) == 0x18);
STATIC_ASSERT(offsetof(ShopkeeperSpawnSetup, kind) == 0x19);
STATIC_ASSERT(offsetof(ShopkeeperSpawnSetup, groundY) == 0x1A);
STATIC_ASSERT(sizeof(ShopkeeperSpawnSetup) == 0x24);

/* object type id of the scarab coins the shopkeeper scatters (DLL 0x287) */
#define OBJTYPE_SPSCARAB 1151

/* ShopkeeperState.flags9D4 bits */
enum
{
    SHOPKEEPER_FLAG_PURCHASED = 0x02, /* purchase event fired */
    SHOPKEEPER_FLAG_FACING = 0x04,    /* turn to face the player */
    SHOPKEEPER_FLAG_LEAVING = 0x10,   /* leaving / screen transition */
    SHOPKEEPER_FLAG_TICK = 0x20       /* per-frame tick effect this frame */
};

void* gShopKeeperDefaultStateHandler;

int ShopKeeper_SeqFn(GameObject* obj, int unused, ObjSeqState* seq, s8 advance)
{
    int state;
    int digit;
    int slot;
    int i;
    int state2;
    void* player;
    int hundreds;
    ObjTextureRuntimeSlot* tex;
    UiDllVTable** uiDll;
    f32 range;
    f32 speed;

    state = *(int*)&(obj)->extra;
    /* second copy of the extra pointer; the (int)(long) round-trip is
     * load-bearing - it splits the value web so state2 gets its own
     * register home (md5-verified: removing it changes codegen) */
    state2 = (int)(long)*(int*)&(obj)->extra;
    player = Obj_GetPlayerObject();
    range = 1.0f;
    ((ShopkeeperState*)state)->flags9D4 &= ~SHOPKEEPER_FLAG_TICK;
    if (((ShopkeeperState*)state)->flags9D4 & SHOPKEEPER_FLAG_LEAVING)
    {
        if ((*gScreenTransitionInterface)->isFinished() != 0)
        {
            (*gScreenTransitionInterface)->step(0x1E, 1);
            (*gObjectTriggerInterface)->endSequence(*(s8*)&seq->slot);
        }
        return 0;
    }
    if (dll_2E_updateSequenceTurn(obj, seq, (MoveLibState*)(state + 0x35C), 0, 0) != 0)
    {
        return 1;
    }
    seq->freeCallback = (ObjAnimSequenceFreeCallback)DRlaserturret_startTimedChallenge;
    seq->flags &= ~0x20;
    speed = 0.0f;
    ((ShopkeeperState*)state2)->baddie.animSpeedA = speed;
    ((ShopkeeperState*)state)->flags9D4 |= SHOPKEEPER_FLAG_FACING;
    if (advance != 0)
    {
        ObjAnim_AdvanceCurrentMove((int)obj, speed, timeDelta, NULL);
    }
    if ((obj)->seqIndex == -1)
    {
        if (seq->movementState != 0)
        {
            slot = SHOPKEEPER_SHOP_INTERFACE(((ShopkeeperState*)state)->vendorObj)
                       ->getItemIndex((GameObject*)((ShopkeeperState*)state)->vendorObj);
            if (slot != -1)
            {
                ((ShopkeeperState*)state)->price =
                    (s16)SHOPKEEPER_SHOP_INTERFACE(((ShopkeeperState*)state)->vendorObj)
                        ->getItemPrice((GameObject*)((ShopkeeperState*)state)->vendorObj, slot);
                ((ShopkeeperState*)state)->minPrice =
                    (s16)SHOPKEEPER_SHOP_INTERFACE(((ShopkeeperState*)state)->vendorObj)
                        ->getItemMinPrice((GameObject*)((ShopkeeperState*)state)->vendorObj, slot);
                ((ShopkeeperState*)state)->priceShown = ((ShopkeeperState*)state)->price;
                ((ShopkeeperState*)state)->unk9D2 = 0;
                digit = ((ShopkeeperState*)state)->price;
                tex = objFindTexture(obj, 8, 0);
                tex->textureId = (digit % 10) * 0x100;
                tex = objFindTexture(obj, 7, 0);
                tex->textureId = ((digit / 10) % 10) * 0x100;
                hundreds = digit / 100;
                if (hundreds > 9)
                {
                    hundreds = 9;
                }
                tex = objFindTexture(obj, 6, 0);
                tex->textureId = hundreds << 8;
            }
            seq->movementState = 0;
            seq->conditionCallback = (ObjAnimSequenceConditionCallback)DRlaserturret_handlePromptChoice;
        }
        if (SHOPKEEPER_SHOP_INTERFACE(((ShopkeeperState*)state)->vendorObj)
                ->getItemIndex((GameObject*)((ShopkeeperState*)state)->vendorObj) != -1)
        {
            setAButtonIcon(0x12);
            setBButtonIcon(0xA);
        }
    }
    for (i = 0; i < seq->eventCount; i++)
    {
        switch (seq->eventIds[i])
        {
        case 1:
            ShopKeeper_spawnScarabs(obj, state, ((ShopkeeperState*)state)->amount);
            ((ShopkeeperState*)state)->flags9D4 |= SHOPKEEPER_FLAG_PURCHASED;
            break;
        case 2:
            (*gPlayerInterface)->setState((void*)obj, (void*)state2, 3);
            (*gBoneParticleEffectInterface)->spawnEffect((void*)obj, 0x7EF, &range, 0x50, NULL);
            ((ShopkeeperState*)state)->opacity = 0;
            break;
        case 3:
            (*gPlayerInterface)->setState((void*)obj, (void*)state2, 2);
            ((ShopkeeperState*)state)->flags9D4 |= SHOPKEEPER_FLAG_TICK;
            ((ShopkeeperState*)state)->opacity = 0xFF;
            break;
        case 4:
            if (((GameObject*)player)->anim.romDefNo == 0)
            {
                warpToMap(0xF, 0);
            }
            else
            {
                warpToMap(0xE, 0);
            }
            break;
        case 5:
            if (getCurUiDll() == 0x10)
            {
                uiDll = getCurUiDllInterface();
                (*uiDll)->setState(0);
            }
            break;
        case 6:
            if (getCurUiDll() == 0x10)
            {
                uiDll = getCurUiDllInterface();
                (*uiDll)->setState(2);
            }
            break;
        case 7:
            if (getCurUiDll() == 0x10)
            {
                uiDll = getCurUiDllInterface();
                (*uiDll)->setState(4);
            }
            break;
        case 9:
            playerAddMoney(player, ((ShopkeeperState*)state)->amount);
            break;
        case 10:
            playerAddMoney(player, -(int)((ShopkeeperState*)state)->amount);
            break;
        case 0xB:
            (*gBoneParticleEffectInterface)->spawnEffect((void*)obj, 0x7EF, &range, 0x50, NULL);
            break;
        case 0xC:
            ((ShopkeeperState*)state)->amount = 1;
            digit = ((ShopkeeperState*)state)->amount;
            tex = objFindTexture(obj, 8, 0);
            tex->textureId = (digit % 10) * 0x100;
            tex = objFindTexture(obj, 7, 0);
            tex->textureId = ((digit / 10) % 10) * 0x100;
            digit = digit / 100;
            if (digit > 9)
            {
                digit = 9;
            }
            tex = objFindTexture(obj, 6, 0);
            tex->textureId = digit << 8;
            break;
        }
    }
    (obj)->anim.alpha = ((ShopkeeperState*)state)->opacity;
    return 0;
}

f32 ShopKeeper_turnTowardPlayer(GameObject* obj, GameObject* player, int snap)
{
    f32 dist;
    f32 dx;
    f32 dz;
    int angleDelta;

    dx = player->anim.localPosX - obj->anim.localPosX;
    dz = player->anim.localPosZ - obj->anim.localPosZ;
    dist = sqrtf(dx * dx + dz * dz);
    if (dist)
    {
        dx /= dist;
        dz /= dist;
    }
    if (dist > 10.0f)
    {
        angleDelta = getAngle(dx, dz) & 0xffff;
        if (snap != 0)
        {
            obj->anim.rotX = angleDelta;
        }
        else
        {
            angleDelta = angleDelta - (u16)obj->anim.rotX;
            if (angleDelta > 0x8000)
            {
                angleDelta -= 0xFFFF;
            }
            if (angleDelta < -0x8000)
            {
                angleDelta += 0xFFFF;
            }
            if (angleDelta > 0x2000)
            {
                angleDelta -= 0x2000;
            }
            else if (angleDelta < -0x2000)
            {
                angleDelta += 0x2000;
            }
            else
            {
                angleDelta = 0;
            }
            obj->anim.rotX = (s16)((f32)(angleDelta >> 3) * timeDelta + (f32) * (s16*)obj);
        }
    }
    return dist;
}

void ShopKeeper_spawnScarabs(GameObject* obj, int state, int count)
{
    int i;
    f32 groundHeight;
    int setup;

    if (Obj_IsLoadingLocked() == 0)
        return;

    (*gMapEventInterface)->setObjGroupStatus((s32)(obj)->anim.mapEventSlot, 6, 1);

    trackGetNearestGroundOffset(obj, (obj)->anim.localPosX, (obj)->anim.localPosY, (obj)->anim.localPosZ, &groundHeight,
                         0);

    for (i = 0; i < count; i++)
    {
        setup = (int)Obj_AllocObjectSetup(0x24, OBJTYPE_SPSCARAB);
        ((ShopkeeperSpawnSetup*)setup)->base.posX = (obj)->anim.localPosX;
        ((ShopkeeperSpawnSetup*)setup)->base.posY = (obj)->anim.localPosY;
        ((ShopkeeperSpawnSetup*)setup)->base.posZ = (obj)->anim.localPosZ;
        ((ShopkeeperSpawnSetup*)setup)->rotXByte = randomGetRange(-128, 127);
        ((ShopkeeperSpawnSetup*)setup)->groundY = (obj)->anim.localPosY - groundHeight;
        ((ShopkeeperSpawnSetup*)setup)->base.color[1] = 1;
        ((ShopkeeperSpawnSetup*)setup)->base.color[3] = 255;
        ((ShopkeeperSpawnSetup*)setup)->base.color[0] = 16;
        ((ShopkeeperSpawnSetup*)setup)->base.color[2] = 6;
        ((ShopkeeperSpawnSetup*)setup)->base.ident = (int)((ShopkeeperState*)state)->vendorObj;
        objSetupObject((ObjPlacement*)setup, 5, (obj)->anim.mapEventSlot, -1, (obj)->anim.parent);
    }

    for (i = 0; i < count; i++)
    {
        setup = (int)Obj_AllocObjectSetup(0x24, OBJTYPE_SPSCARAB);
        ((ShopkeeperSpawnSetup*)setup)->base.posX = (obj)->anim.localPosX;
        ((ShopkeeperSpawnSetup*)setup)->base.posY = (obj)->anim.localPosY;
        ((ShopkeeperSpawnSetup*)setup)->base.posZ = (obj)->anim.localPosZ;
        ((ShopkeeperSpawnSetup*)setup)->rotXByte = randomGetRange(-128, 127);
        ((ShopkeeperSpawnSetup*)setup)->groundY = (obj)->anim.localPosY - groundHeight;
        ((ShopkeeperSpawnSetup*)setup)->base.color[1] = 1;
        ((ShopkeeperSpawnSetup*)setup)->base.color[3] = 255;
        ((ShopkeeperSpawnSetup*)setup)->base.color[0] = 16;
        ((ShopkeeperSpawnSetup*)setup)->base.color[2] = 6;
        ((ShopkeeperSpawnSetup*)setup)->kind = 1;
        ((ShopkeeperSpawnSetup*)setup)->base.ident = (int)((ShopkeeperState*)state)->vendorObj;
        objSetupObject((ObjPlacement*)setup, 5, (obj)->anim.mapEventSlot, -1, (obj)->anim.parent);
    }
}

int ShopKeeper_getExtraSize(void)
{
    return 0x9d8;
}

int ShopKeeper_getObjectTypeId(void)
{
    return 0x0;
}

void ShopKeeper_free(GameObject* obj)
{
    Stack_Free(((ShopkeeperState*)obj->extra)->msgStack);
    return;
}

void ShopKeeper_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible)
{
    int state = *(int*)&(obj)->extra;
    f32 fxParams[4];
    fxParams[0] = 1.0f;
    if (((ShopkeeperState*)state)->baddie.controlMode != 7 && visible != 0)
    {
        objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, 1.0f);
        dll_2E_setTargetFromPathPoint(obj, (MoveLibState*)(state + 0x35c), 0);
    }
    if ((((ShopkeeperState*)state)->flags9D4 & SHOPKEEPER_FLAG_TICK) != 0)
    {
        (*gBoneParticleEffectInterface)->spawnEffect((void*)obj, 0x7ef, fxParams, 0x50, NULL);
    }
}

void ShopKeeper_hitDetect(void)
{
}

void ShopKeeper_update(GameObject* obj)
{
    void* player;
    int state;
    f32 dist;
    player = Obj_GetPlayerObject();
    state = *(int*)&(obj)->extra;
    dist = 10000.0f;
    ((ShopkeeperState*)state)->flags9D4 &= ~SHOPKEEPER_FLAG_TICK;
    if (((ShopkeeperState*)state)->textTimer > 0.0f)
    {
        gameTextShow(0x433);
        ((ShopkeeperState*)state)->textTimer = ((ShopkeeperState*)state)->textTimer - timeDelta;
        if (((ShopkeeperState*)state)->textTimer < 0.0f)
        {
            ((ShopkeeperState*)state)->textTimer = 0.0f;
        }
    }
    if ((((ShopkeeperState*)state)->flags9D4 & SHOPKEEPER_FLAG_FACING) != 0)
    {
        ShopKeeper_turnTowardPlayer(obj, player, 1);
    }
    (obj)->anim.rootMotionScale = (obj)->anim.modelInstance->rootMotionScaleBase;
    if (((ShopkeeperState*)state)->vendorObj == NULL)
    {
        ((ShopkeeperState*)state)->vendorObj =
            objGetNearestTypeTo(SPSHOPKEEPER_TARGET_OBJGROUP, obj, &dist);
    }
    ((ShopkeeperState*)state)->playerMoney = playerGetMoney(player);
    (*gPlayerInterface)->update((void*)obj, (void*)state, timeDelta, timeDelta, gShopKeeperStateHandlers, &gShopKeeperDefaultStateHandler);
    dll_2E_updateLookAt(obj, &((ShopkeeperState*)state)->moveLib);
    characterDoEyeAnims(obj, &((ShopkeeperState*)state)->eyeAnimState);
    (obj)->anim.alpha = ((ShopkeeperState*)state)->opacity;
}

void ShopKeeper_init(GameObject* obj)
{
    int state = *(int*)&(obj)->extra;
    (obj)->objectFlags |= SPSHOPKEEPER_OBJFLAG_HITDETECT_DISABLED;
    (obj)->animEventCallback = ShopKeeper_SeqFn;
    (obj)->anim.modelState->flags |= 0x810;
    ((ShopkeeperState*)state)->unk9B8 = 0.1f * (f32)(s32)randomGetRange(0xF, 0x23);
    ((ShopkeeperState*)state)->msgStack = Queue_Alloc(4, 4);
    ((ShopkeeperState*)state)->opacity = 0xFF;
    ((ShopkeeperState*)state)->textTimer = 300.0f;
    dll_2E_initState(obj, (MoveLibState*)(state + 0x35C), -0x1C71, 0x3555, 2);
    ((ShopkeeperState*)state)->moveLib.modeBits |= 0x12;
}

void ShopKeeper_release(void)
{
}

void ShopKeeper_initialise(void)
{
    gShopKeeperStateHandlers[0] = DRlaserturret_startLinkedTarget;
    gShopKeeperStateHandlers[1] = DRlaserturret_updateTracking;
    gShopKeeperStateHandlers[2] = DRlaserturret_updateIdle;
    gShopKeeperStateHandlers[3] = TREX_Lazerwall_updateTimedChallenge;
    gShopKeeperStateHandlers[4] = TREX_Lazerwall_waitForStartBit;
    gShopKeeperStateHandlers[5] = TREX_Lazerwall_popQueuedState;
    gShopKeeperStateHandlers[6] = ShopKeeper_popQueuedState;
    gShopKeeperStateHandlers[7] = ShopKeeper_state7Handler;
    gShopKeeperDefaultStateHandler = ShopKeeper_defaultStateHandler;
}

