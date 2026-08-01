#include "sys/objects.h"
#include "main/mapEvent.h"
#include "main/dll/TrickyCurve.h"
#include "main/dll/sfxplayer.h"
#include "main/dll/dll_02B1_cmbsrc.h"
#include "main/gamebits.h"
#include "main/frame_timing.h"
#include "main/vecmath.h"

typedef struct CmbSrcColorIndexPair
{
    u32 a;
    u32 b;
} CmbSrcColorIndexPair;

#define SFXPLAYER_EFFECT_RING_COUNT       4
#define SFXPLAYER_EFFECT_HANDLES_PER_RING 2
#define SFXPLAYER_MODE_SEQUENCE           2
#define SFXPLAYER_RING_START_SFX          0x459
#define SFXPLAYER_TIMEOUT_RESET_SFX       0x1CE
#define SFXPLAYER_GAMEBIT_RING_ACTIVE     0xEDF
#define SFXPLAYER_RING_VISUAL_SETUP_SIZE  0x2C
#define SFXPLAYER_RING_VISUAL_OBJECT_ID   CMBSRC_SEQ_DEFAULT
#define SFXPLAYER_RING_HIT_SETUP_SIZE     4
#define SFXPLAYER_RING_HIT_OBJECT_ID      0x71C
#define SFXPLAYER_RING_SETUP_MODE         5
#define SFXPLAYER_EFFECT_RING_ROT_STEP    0x3FFF

#define SFXPLAYER_CONFIG_MAP_ID_OFFSET    0x18
#define SFXPLAYER_CONFIG_MODE_OFFSET      0x19
#define SFXPLAYER_CONFIG_EVENT_ID_OFFSET  0x1E
#define SFXPLAYER_CONFIG_FIELD20_OFFSET   0x20
#define SFXPLAYER_COMPLETE_RING_COUNT     4
#define SFXPLAYER_TIMER_ID                0x1D
#define SFXPLAYER_TIMER_SHORT_FRAMES      0x96
#define SFXPLAYER_TIMER_LONG_FRAMES       0xB4
#define SFXPLAYER_MODE_SINGLE             1
#define SFXPLAYER_GAMEBIT_SINGLE_COMPLETE 0x9F7
#define SFXPLAYER_SFX_COMPLETE            0x7E
#define SFXPLAYER_SFX_TIMEOUT_RESET       0x1CE
#define SFXPLAYER_SFX_RING_HIT            0x409
#define SFXPLAYER_HIT_TYPE_RING_TARGET    0x13

int gSfxplayerEffectHandles[8];

static const CmbSrcColorIndexPair sDFPRotatePColorIndices = {0x00040005, 0x0006000B};

#define SFXPLAYER_UPDATE_EFFECT_HANDLE_POS(handleExpr, obj, rot, angleStep)                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        if ((void*)(handleExpr) != NULL)                                                                               \
        {                                                                                                              \
            *(f32*)((handleExpr) + 0xc) = 0.0f;                                                                \
            *(f32*)((handleExpr) + 0x10) = 60.0f;                                                               \
            *(f32*)((handleExpr) + 0x14) = 93.0f;                                                               \
            (rot)[0] = (s16)(*(s16*)(obj) + (angleStep));                                                              \
            vecRotateZXY((rot), (f32*)((handleExpr) + 0xc));                                                           \
            *(f32*)((handleExpr) + 0xc) += *(f32*)((obj) + 0xc);                                                       \
            *(f32*)((handleExpr) + 0x10) += *(f32*)((obj) + 0x10);                                                     \
            *(f32*)((handleExpr) + 0x14) += *(f32*)((obj) + 0x14);                                                     \
        }                                                                                                              \
    } while (0)

void TrickyCurve_updateEffectHandleRing(GameObject* obj)
{
    struct
    {
        s16 rotation[4];
        f32 baseVec[4];
    } buf;
    int* handles;
    SfxplayerState* state = (SfxplayerState*)obj->extra;
    s16 i;

    if (state->flags.bit10 != 0 && state->flags.bit20 == 0 && state->variantSfxTimer > 0x32)
    {
        Sfx_KeepAliveLoopedObjectSound((u32)obj, SFXPLAYER_RING_START_SFX);
        if ((*gMapEventInterface)->getMapAct(obj->anim.mapEventSlot) == SFXPLAYER_MODE_SEQUENCE)
        {
            obj->anim.rotX += (int)((1.0f + state->ringCount) * (30.0f * timeDelta));
        }
        else
        {
            obj->anim.rotX += (int)(30.0f * timeDelta);
        }
    }

    if (state->variantSfxTimer != 0 && state->flags.bit10 != 0)
    {
        state->variantSfxTimer -= (s16)timeDelta;
        if (state->variantSfxTimer <= 0)
        {
            state->variantSfxTimer = 200;
        }
    }

    buf.baseVec[1] = 0.0f;
    buf.baseVec[2] = 0.0f;
    buf.baseVec[3] = 0.0f;
    buf.baseVec[0] = 1.0f;
    buf.rotation[1] = buf.rotation[2] = 0;
    handles = gSfxplayerEffectHandles;

    for (i = 0; i < SFXPLAYER_EFFECT_RING_COUNT; i++)
    {
        SFXPLAYER_UPDATE_EFFECT_HANDLE_POS(handles[i * SFXPLAYER_EFFECT_HANDLES_PER_RING], (int)obj, buf.rotation,
                                           i * SFXPLAYER_EFFECT_RING_ROT_STEP);
        SFXPLAYER_UPDATE_EFFECT_HANDLE_POS(handles[i * SFXPLAYER_EFFECT_HANDLES_PER_RING + 1], (int)obj, buf.rotation,
                                           i * SFXPLAYER_EFFECT_RING_ROT_STEP);
    }
}

int sfxplayer_ensureEffectHandlePair(GameObject* obj, u8 ringIndex)
{
    u32 colorIndexWords[2];
    int* handles;
    int* pair;
    CmbSrcMapData* setup;
    int handleOffset;
    s16* colorIndices;

    *(CmbSrcColorIndexPair*)colorIndexWords = *(CmbSrcColorIndexPair*)&sDFPRotatePColorIndices;

    if (Obj_IsLoadingLocked() == 0)
    {
        return 0;
    }

    handleOffset = (ringIndex & 0xff) * 8;
    handles = gSfxplayerEffectHandles;
    if (*(void**)((int)handles + handleOffset) == NULL)
    {
        setup = (CmbSrcMapData*)Obj_AllocObjectSetup(SFXPLAYER_RING_VISUAL_SETUP_SIZE, SFXPLAYER_RING_VISUAL_OBJECT_ID);
        setup->base.color[2] = 0xff;
        setup->base.color[3] = 0xff;
        setup->base.color[0] = 2;
        setup->base.color[1] = 1;
        setup->base.posX = obj->anim.localPosX;
        setup->base.posY = obj->anim.localPosY;
        setup->base.posZ = obj->anim.localPosZ;
        setup->gameBit = -1;
        setup->rotX = 0;
        setup->rotZ = 0;
        setup->rotY = 0;
        if ((*gMapEventInterface)->getMapAct(obj->anim.mapEventSlot) == SFXPLAYER_MODE_SEQUENCE)
        {
            colorIndices = (s16*)colorIndexWords;
            setup->colorIndex = colorIndices[ringIndex & 0xff];
        }
        else
        {
            setup->colorIndex = (u8) * (s16*)((char*)colorIndexWords + 6);
        }
        setup->effectMode = 0;
        setup->pulseSubMode = 0;
        setup->colorDistance = 0x64;
        setup->effectDistance = 0;
        setup->pulseDistance = 0;
        setup->radius = 0.5f;
        setup->flags = 0xd2;
        setup->behaviorFlags = 0;
        *(int*)((int)handles + handleOffset) =
            (int)objSetupObject(&setup->base, SFXPLAYER_RING_SETUP_MODE, obj->anim.mapEventSlot, -1, obj->anim.parent);
    }

    {
        u8* pairBase = (u8*)gSfxplayerEffectHandles + 4;
        pair = (int*)(pairBase + ((ringIndex & 0xff) * 8));
    }
    if (*(void**)pair == NULL)
    {
        setup = (CmbSrcMapData*)Obj_AllocObjectSetup(SFXPLAYER_RING_HIT_SETUP_SIZE, SFXPLAYER_RING_HIT_OBJECT_ID);
        setup->base.color[2] = 0xff;
        setup->base.color[3] = 0xff;
        setup->base.color[0] = 2;
        setup->base.color[1] = 1;
        setup->base.posX = obj->anim.localPosX;
        setup->base.posY = obj->anim.localPosY;
        setup->base.posZ = obj->anim.localPosZ;
        *pair = (int)objSetupObject(&setup->base, SFXPLAYER_RING_SETUP_MODE, obj->anim.mapEventSlot, -1, obj->anim.parent);
    }

    return 1;
}

int TrickyCurve_activateEffectHandleRing(GameObject* obj, int unused, ObjSeqState* animUpdate)
{
    SfxplayerState* state = (SfxplayerState*)obj->extra;
    int i;

    state->flags.bit80 = 1;
    gameTimerStop();
    for (i = 0; i < animUpdate->eventCount; i++)
    {
        switch ((int)animUpdate->eventIds[i])
        {
        case 1:
            state->flags.bit10 = 1;
            state->ringCount = 0;
            mainSetBits(state->activationEventId, 0);
            mainSetBits(SFXPLAYER_GAMEBIT_RING_ACTIVE, 1);
            for (i = 0; i < SFXPLAYER_EFFECT_RING_COUNT; i++)
            {
                sfxplayer_ensureEffectHandlePair(obj, i);
            }
            state->flags.bit40 = 1;
            break;
        }
    }

    TrickyCurve_updateEffectHandleRing(obj);
    return 0;
}

int sfxplayer_getExtraSize(void)
{
    return 0xa;
}
int sfxplayer_getObjectTypeId(void)
{
    return 0x0;
}

void sfxplayer_free(u32 obj, int flag)
{
    u32* handles;
    s16 i;

    if (flag == 0)
    {
        handles = (u32*)gSfxplayerEffectHandles;
        for (i = 0; i < SFXPLAYER_EFFECT_RING_COUNT; i++)
        {
            if (handles[i * SFXPLAYER_EFFECT_HANDLES_PER_RING] != 0)
            {
                Obj_FreeObject((GameObject*)handles[i * SFXPLAYER_EFFECT_HANDLES_PER_RING]);
            }
            handles[i * SFXPLAYER_EFFECT_HANDLES_PER_RING] = 0;
            if (handles[i * SFXPLAYER_EFFECT_HANDLES_PER_RING + 1] != 0)
            {
                Obj_FreeObject((GameObject*)handles[i * SFXPLAYER_EFFECT_HANDLES_PER_RING + 1]);
            }
            handles[i * SFXPLAYER_EFFECT_HANDLES_PER_RING + 1] = 0;
            Sfx_PlayFromObject(obj, SFXPLAYER_TIMEOUT_RESET_SFX);
        }
    }
    gameTimerStop();
}

#undef SFXPLAYER_UPDATE_EFFECT_HANDLE_POS

void sfxplayer_render(void)
{
}

void sfxplayer_hitDetect(void)
{
}

void sfxplayer_update(GameObject* obj)
{
    u32* handles;
    s16 i;
    s16 hitType;
    u8 mode;
    SfxplayerState* state;
    SfxplayerStateFlags* flags;
    u32 hitObj;

    state = (SfxplayerState*)obj->extra;
    flags = &state->flags;
    if ((flags->bit20 == 0) && (mainGetBit(state->eventId) == 0))
    {
        if (state->ringCount == SFXPLAYER_COMPLETE_RING_COUNT)
        {
            Sfx_PlayFromObject(0, SFXPLAYER_SFX_COMPLETE);
            flags->bit20 = 1;
            flags->bit10 = 0;
            flags->bit40 = 0;
            mainSetBits(state->eventId, 1);
            mainSetBits(SFXPLAYER_GAMEBIT_RING_ACTIVE, 0);
            mode = (*gMapEventInterface)->getMapAct(obj->anim.mapEventSlot);
            if (mode == SFXPLAYER_MODE_SINGLE)
            {
                mainSetBits(SFXPLAYER_GAMEBIT_SINGLE_COMPLETE, 1);
            }
            gameTimerStop();
        }
        else
        {
            if (flags->bit80 != 0)
            {
                flags->bit80 = 0;
                if (flags->bit10 != 0)
                {
                    mode = (*gMapEventInterface)->getMapAct(obj->anim.mapEventSlot);
                    if (mode == SFXPLAYER_MODE_SINGLE)
                    {
                        gameTimerInit(SFXPLAYER_TIMER_ID, SFXPLAYER_TIMER_SHORT_FRAMES);
                    }
                    else
                    {
                        gameTimerInit(SFXPLAYER_TIMER_ID, SFXPLAYER_TIMER_LONG_FRAMES);
                    }
                    timerSetToCountUp();
                }
            }
            if (isGameTimerDisabled() != 0)
            {
                handles = (u32*)gSfxplayerEffectHandles;
                for (i = 0; i < SFXPLAYER_EFFECT_RING_COUNT; i++)
                {
                    if (handles[i * SFXPLAYER_EFFECT_HANDLES_PER_RING] != 0)
                    {
                        Obj_FreeObject((GameObject*)handles[i * SFXPLAYER_EFFECT_HANDLES_PER_RING]);
                    }
                    handles[i * SFXPLAYER_EFFECT_HANDLES_PER_RING] = 0;
                    if (handles[i * SFXPLAYER_EFFECT_HANDLES_PER_RING + 1] != 0)
                    {
                        Obj_FreeObject((GameObject*)handles[i * SFXPLAYER_EFFECT_HANDLES_PER_RING + 1]);
                    }
                    handles[i * SFXPLAYER_EFFECT_HANDLES_PER_RING + 1] = 0;
                    Sfx_PlayFromObject((int)obj, SFXPLAYER_SFX_TIMEOUT_RESET);
                }
                state->ringCount = 0;
                flags->bit40 = 0;
                flags->bit10 = 0;
                mainSetBits(SFXPLAYER_GAMEBIT_RING_ACTIVE, 0);
            }
            TrickyCurve_updateEffectHandleRing(obj);
            handles = (u32*)gSfxplayerEffectHandles;
            for (i = 0; i < SFXPLAYER_EFFECT_RING_COUNT; i++)
            {
                if (handles[i * SFXPLAYER_EFFECT_HANDLES_PER_RING] != 0)
                {
                    hitObj = 0;
                    hitType = ObjHits_GetPriorityHit((GameObject*)(handles[i * SFXPLAYER_EFFECT_HANDLES_PER_RING + 1]),
                                                     (int*)&hitObj, 0x0, 0x0);
                    if (hitType == SFXPLAYER_HIT_TYPE_RING_TARGET)
                    {
                        mode = (*gMapEventInterface)->getMapAct(obj->anim.mapEventSlot);
                        if ((mode == SFXPLAYER_MODE_SINGLE) || (((GameObject*)hitObj)->userData1 == i))
                        {
                            if (handles[i * SFXPLAYER_EFFECT_HANDLES_PER_RING] != 0)
                            {
                                Obj_FreeObject((GameObject*)handles[i * SFXPLAYER_EFFECT_HANDLES_PER_RING]);
                            }
                            handles[i * SFXPLAYER_EFFECT_HANDLES_PER_RING] = 0;
                            if (handles[i * SFXPLAYER_EFFECT_HANDLES_PER_RING + 1] != 0)
                            {
                                Obj_FreeObject((GameObject*)handles[i * SFXPLAYER_EFFECT_HANDLES_PER_RING + 1]);
                            }
                            handles[i * SFXPLAYER_EFFECT_HANDLES_PER_RING + 1] = 0;
                            Sfx_PlayFromObject(0, SFXPLAYER_SFX_RING_HIT);
                            state->ringCount++;
                        }
                    }
                }
            }
        }
    }
    return;
}

void sfxplayer_init(GameObject* obj, int config)
{
    SfxplayerState* state;

    state = (SfxplayerState*)obj->extra;
    obj->anim.rotX = (s16)((s8) * (u8*)(config + SFXPLAYER_CONFIG_MAP_ID_OFFSET) << 8);
    obj->animEventCallback = (void*)TrickyCurve_activateEffectHandleRing;
    state->config19 = *(u8*)(config + SFXPLAYER_CONFIG_MODE_OFFSET);
    state->eventId = *(s16*)(config + SFXPLAYER_CONFIG_EVENT_ID_OFFSET);
    state->config20 = *(s16*)(config + SFXPLAYER_CONFIG_FIELD20_OFFSET);
    state->unk4 = 1;
    gSfxplayerEffectHandles[0] = 0;
    gSfxplayerEffectHandles[1] = 0;
    gSfxplayerEffectHandles[2] = 0;
    gSfxplayerEffectHandles[3] = 0;
    gSfxplayerEffectHandles[4] = 0;
    gSfxplayerEffectHandles[5] = 0;
    gSfxplayerEffectHandles[6] = 0;
    gSfxplayerEffectHandles[7] = 0;
    gameTimerStop();
    if (mainGetBit(state->eventId) != 0)
    {
        state->flags.bit20 = 1;
    }
    obj->objectFlags = obj->objectFlags | (OBJECT_OBJFLAG_HITDETECT_DISABLED | OBJECT_OBJFLAG_HIDDEN);
}

void sfxplayer_release(void)
{
}

void sfxplayer_initialise(void)
{
}

ObjectDescriptor gSfxplayerObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)sfxplayer_initialise,
    (ObjectDescriptorCallback)sfxplayer_release,
    0,
    (ObjectDescriptorCallback)sfxplayer_init,
    (ObjectDescriptorCallback)sfxplayer_update,
    (ObjectDescriptorCallback)sfxplayer_hitDetect,
    (ObjectDescriptorCallback)sfxplayer_render,
    (ObjectDescriptorCallback)sfxplayer_free,
    (ObjectDescriptorCallback)sfxplayer_getObjectTypeId,
    sfxplayer_getExtraSize,
};
