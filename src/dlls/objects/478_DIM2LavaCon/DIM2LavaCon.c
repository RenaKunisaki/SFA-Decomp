#include "main/dll/dll_0011_screens.h"
#include "main/dll/savegame_load_api.h"
#include "main/audio/music_api.h"
#include "main/audio/music_trigger_ids.h"
#include "main/sky_api.h"
#include "main/render_envfx_api.h"
#include "main/gamebits.h"
#include "game/objects/object.h"
#include "sys/objects.h"
#include "dlls/objects/430_SH_LevelCon.h"
#include "main/object_render.h"
#include "main/rcp_dolphin_api.h"
#include "main/pi_dolphin_api.h"
#include "main/dll/player_objects.h"
#include "dlls/object_descriptor.h"

u8 gDim2LavaHeatAlphaTargets[8] = {0xFF, 0xCD, 0xB9, 0xAA, 0, 0, 0, 0};

#define DIM2LAVACONTROL_ENVFX_A 0x163
#define DIM2LAVACONTROL_ENVFX_B 0x166
#define DIM2LAVACONTROL_ENVFX_C 0x165
#define DIM2LAVACONTROL_ENVFX_D 0x164

typedef struct Dim2lavacontrolPlacement
{
    u8 pad0[0x14 - 0x0];
    s32 unk14;
    s8 unk18;
    u8 unk19;
    s16 countdownInit;
    s16 unk1C;
    s16 gameBit;
} Dim2lavacontrolPlacement;

typedef struct Dim2lavacontrolState
{
    s8 countdown;
    u8 countdownSave;
    s8 flags;
    u8 heatEffectAlpha;
    u8 phase;
    u8 pad5[0x8 - 0x5];
    SCGameBitLatchState latch;
    int musicTrack;
    u8 padC[0x24 - 0x10];
    f32 unk24;
} Dim2lavacontrolState;

typedef enum Dim2lavacontrolPhase
{
    DIM2LAVACONTROL_PHASE_WAIT = 0,      /* waits for its unlock game bit */
    DIM2LAVACONTROL_PHASE_TRIGGERED = 1, /* unlock bit set; control latched */
} Dim2lavacontrolPhase;

void dim2lavacontrol_setScale(GameObject *obj)
{
    Dim2lavacontrolState* sub = (obj)->extra;
    if (((s32)sub->flags & 1) == 0)
    {
        Dim2lavacontrolPlacement* p = *(Dim2lavacontrolPlacement**)&(obj)->anim.placementData;
        if ((s32)sub->countdown > 0)
        {
            sub->countdown -= 1;
            if (sub->countdown == 0)
            {
                sub->flags = (s8)(*(u8*)&sub->flags | 1);
                mainSetBits(p->gameBit, 1);
            }
        }
    }
}

int dim2lavacontrol_getExtraSize(void) { return 0x10; }

void dim2lavacontrol_free(void)
{
    setHeatEffectParams(0xC0, 1.0f);
    Music_Trigger(MUSICTRIG_PU3_Adventure_c4, 0);
    Rcp_DisableHeatEffect();
}

void dim2lavacontrol_render(GameObject *obj, int p2, int p3, int p4, int p5, s8 visible)
{
    s32 v = visible;
    if (v != 0) objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, 1.0f);
}

void dim2lavacontrol_update(GameObject* obj)
{
    int alphaDelta;
    GameObject* heldObj;
    Dim2lavacontrolState* state;
    if ((obj)->userData1 != 0)
    {
        if ((obj)->userData1 == 2)
        {
            getEnvfxActImmediately(0, 0, DIM2LAVACONTROL_ENVFX_A, 0);
            getEnvfxActImmediately(0, 0, DIM2LAVACONTROL_ENVFX_B, 0);
            getEnvfxActImmediately(0, 0, DIM2LAVACONTROL_ENVFX_C, 0);
            getEnvfxActImmediately(0, 0, DIM2LAVACONTROL_ENVFX_D, 0);
        }
        else
        {
            getEnvfxAct(0, 0, DIM2LAVACONTROL_ENVFX_A, 0);
            getEnvfxAct(0, 0, DIM2LAVACONTROL_ENVFX_B, 0);
            getEnvfxAct(0, 0, DIM2LAVACONTROL_ENVFX_C, 0);
            getEnvfxAct(0, 0, DIM2LAVACONTROL_ENVFX_D, 0);
        }
        (obj)->userData1 = 0;
    }
    state = (obj)->extra;
    switch (state->phase)
    {
    case DIM2LAVACONTROL_PHASE_WAIT:
        if (mainGetBit(0xacd) != 0)
        {
            mainSetBits(0xcc3, 1);
            state->phase = DIM2LAVACONTROL_PHASE_TRIGGERED;
        }
        break;
    case DIM2LAVACONTROL_PHASE_TRIGGERED:
        break;
    }
    alphaDelta = state->heatEffectAlpha - gDim2LavaHeatAlphaTargets[state->countdown];
    if (alphaDelta != 0)
    {
        if (alphaDelta > 0)
        {
            state->heatEffectAlpha -= 1;
        }
        else
        {
            state->heatEffectAlpha += 1;
        }
        setHeatEffectParams(state->heatEffectAlpha, 1.0f);
    }
    if (Player_GetHeldObject(Obj_GetPlayerObject(), &heldObj) != 0)
    {
        if ((state->latch.activeMask & 2) && state->musicTrack != 0xe0)
        {
            Music_Trigger(state->musicTrack, 0);
            state->musicTrack = 0xe0;
            Music_Trigger(MUSICTRIG_WLC_Puzzle_e0, 1);
        }
    }
    else
    {
        if ((state->latch.activeMask & 2) && state->musicTrack != 0xd7)
        {
            Music_Trigger(state->musicTrack, 0);
            state->musicTrack = 0xd7;
            Music_Trigger(MUSICTRIG_WLC_Chambers, 1);
        }
    }
    SCGameBitLatch_Update(&state->latch, 1, -1, -1, 0xd99, 0xde);
    SCGameBitLatch_Update(&state->latch, 2, -1, -1, 0xda5, state->musicTrack);
    SCGameBitLatch_Update(&state->latch, 8, -1, -1, 0xf04, 0x96);
    SCGameBitLatch_UpdateInverted(&state->latch, 0x10, -1, -1, 0xf04, 0x2c);
    SCGameBitLatch_Update(&state->latch, 4, -1, -1, 0xcbb, 0xc4);
}

void dim2lavacontrol_init(GameObject *obj, Dim2lavacontrolPlacement* placement)
{
    Dim2lavacontrolState* state;
    u8 i;
    int gameBitState;
    if (getSaveGameLoadStatus() != 0)
    {
        (obj)->userData1 = 2;
    }
    else
    {
        (obj)->userData1 = 1;
    }
    for (i = 1; i <= 0x2d; i++)
    {
        gameBitFn_800ea2e0(i);
    }
    state = (obj)->extra;
    state->countdown = (s8)placement->countdownInit;
    state->countdownSave = *(u8*)&state->countdown;
    if (mainGetBit(placement->gameBit) != 0)
    {
        gameBitState = 1;
    }
    else
    {
        gameBitState = 0;
    }
    state->flags = (s8)(*(u8*)&state->flags | gameBitState);
    state->musicTrack = 0xd7;
    state->phase = DIM2LAVACONTROL_PHASE_WAIT;
    if ((state->flags & 1) != 0)
    {
        *(u8*)&state->countdown = 0;
        state->heatEffectAlpha = gDim2LavaHeatAlphaTargets[0];
        setHeatEffectParams(gDim2LavaHeatAlphaTargets[0], 1.0f);
    }
    else
    {
        *(u8*)&state->countdown = 3;
        state->heatEffectAlpha = gDim2LavaHeatAlphaTargets[3];
        setHeatEffectParams(gDim2LavaHeatAlphaTargets[3], 1.0f);
    }
    Music_Trigger(MUSICTRIG_WLC_Corridors, 1);
    envFxActFn_800887f8(0);
}

ObjectDescriptor12 gDIM2LavaControlObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_11_SLOTS,
    0,
    0,
    0,
    (ObjectDescriptorCallback)dim2lavacontrol_init,
    (ObjectDescriptorCallback)dim2lavacontrol_update,
    0,
    (ObjectDescriptorCallback)dim2lavacontrol_render,
    (ObjectDescriptorCallback)dim2lavacontrol_free,
    0,
    (ObjectDescriptorExtraSizeCallback)dim2lavacontrol_getExtraSize,
    (ObjectDescriptorCallback)dim2lavacontrol_setScale,
    0,
};
