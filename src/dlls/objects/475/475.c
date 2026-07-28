#include "main/object_render.h"
#include "main/audio/sfx.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/gamebits.h"
#include "main/frame_timing.h"
#include "sys/objects.h"
#include "main/dll/dll_01DB_dll1db.h"

enum
{
    DIM2_CRUSHER_STATE_TOP = 1,
    DIM2_CRUSHER_STATE_BOTTOM = 2,
    DIM2_CRUSHER_STATE_RISING = 3,
    DIM2_CRUSHER_STATE_FALLING = 4
};

int dll_1DB_getExtraSize(void)
{
    return sizeof(Dim2CrusherState);
}
int dll_1DB_getObjectTypeId(void)
{
    return 0x0;
}

void dll_1DB_free(void)
{
}

void dll_1DB_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible)
{
    s32 v = visible;
    if (v != 0)
        objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, 1.0f);
}

void dll_1DB_hitDetect(void)
{
}

void dll_1DB_update(GameObject* obj)
{
    Dim2CrusherState* state;
    Dim2CrusherPlacement* placement;
    int found;
    GameObject* player;
    int i;
    int n;
    int contactListAddress;

    state = obj->extra;
    player = Obj_GetPlayerObject();
    placement = (Dim2CrusherPlacement*)obj->anim.placementData;
    found = 0;
    i = 0;
    contactListAddress = (int)obj->anim.hitboxTransformState;
    for (n = ((ObjHitboxTransformState*)contactListAddress)->contactObjectCount; n > 0; n--)
    {
        GameObject* entry =
            *(GameObject**)(contactListAddress + i + offsetof(ObjHitboxTransformState, contactObjects));
        if (entry == player)
        {
            found = 1;
            break;
        }
        i += sizeof(((ObjHitboxTransformState*)contactListAddress)->contactObjects[0]);
    }
    switch (state->motionState)
    {
    case DIM2_CRUSHER_STATE_TOP:
        Sfx_StopObjectChannel((int)obj, 8);
        if (found == 0)
        {
            state->contactLostFlag = 1;
        }
        else if (state->contactLostFlag != 0 && state->boardedFlag != 0)
        {
            Sfx_PlayFromObject((int)obj, SFXTRIG_mv_wickpickup16);
            state->motionState = DIM2_CRUSHER_STATE_FALLING;
            state->velocity = 0.0f;
        }
        if (mainGetBit(placement->triggerGameBit) != 0)
        {
            Sfx_PlayFromObject((int)obj, SFXTRIG_mv_wickpickup16);
            state->motionState = DIM2_CRUSHER_STATE_FALLING;
            state->velocity = 0.0f;
        }
        break;
    case DIM2_CRUSHER_STATE_BOTTOM:
        Sfx_StopObjectChannel((int)obj, 8);
        if (state->boardedFlag != 0)
        {
            if (found == 0)
            {
                Sfx_PlayFromObject((int)obj, SFXTRIG_mv_wickpickup16);
                state->motionState = DIM2_CRUSHER_STATE_RISING;
                state->velocity = 0.0f;
                state->boardedFlag = 0;
                mainSetBits(placement->boardedGameBit, 0);
            }
        }
        else
        {
            if (mainGetBit(placement->triggerGameBit) == 0)
            {
                Sfx_PlayFromObject((int)obj, SFXTRIG_mv_wickpickup16);
                state->motionState = DIM2_CRUSHER_STATE_RISING;
                state->velocity = 0.0f;
                state->boardedFlag = 0;
                mainSetBits(placement->boardedGameBit, 0);
            }
        }
        break;
    case DIM2_CRUSHER_STATE_RISING:
        state->velocity =
            state->velocity + (0.02f * timeDelta + 0.1f * (f32)(s32)(state->velocity < 0.0f));
        {
            f32 velocity = state->velocity;
            if (velocity > 1.5f)
            {
                state->velocity = 1.5f;
            }
        }
        obj->anim.localPosY = state->velocity * timeDelta + obj->anim.localPosY;
        if (obj->anim.localPosY > placement->base.posY)
        {
            Sfx_PlayFromObject((int)obj, SFXTRIG_en_lflsh2_b);
            obj->anim.localPosY = placement->base.posY;
            state->motionState = DIM2_CRUSHER_STATE_TOP;
            if (found != 0)
            {
                state->boardedFlag = 1;
                state->contactLostFlag = 0;
            }
        }
        break;
    case DIM2_CRUSHER_STATE_FALLING:
        state->velocity = -0.02f * timeDelta + state->velocity;
        {
            f32 velocity = state->velocity;
            if (velocity < -1.5f)
            {
                state->velocity = -1.5f;
            }
        }
        obj->anim.localPosY = state->velocity * timeDelta + obj->anim.localPosY;
        if (obj->anim.localPosY < placement->base.posY - 235.5f)
        {
            Sfx_PlayFromObject((int)obj, SFXTRIG_en_lflsh2_b);
            obj->anim.localPosY = placement->base.posY - 235.5f;
            state->motionState = DIM2_CRUSHER_STATE_BOTTOM;
            mainSetBits(placement->boardedGameBit, 1);
        }
        if (state->boardedFlag == 0)
        {
            if (mainGetBit(placement->triggerGameBit) == 0)
            {
                state->motionState = DIM2_CRUSHER_STATE_RISING;
                mainSetBits(placement->boardedGameBit, 0);
            }
        }
        break;
    }
}

void dll_1DB_init(GameObject* obj, Dim2CrusherPlacement* placement)
{
    Dim2CrusherState* state = obj->extra;
    s16 t = (s16)((s32)placement->rotX << 8);
    obj->anim.rotX = t;
    if (mainGetBit(placement->boardedGameBit) != 0)
    {
        state->motionState = DIM2_CRUSHER_STATE_BOTTOM;
    }
    else
    {
        state->motionState = DIM2_CRUSHER_STATE_TOP;
    }
    obj->objectFlags |= OBJECT_OBJFLAG_HITDETECT_DISABLED;
}

void dll_1DB_release(void)
{
}

void dll_1DB_initialise(void)
{
}

ObjectDescriptor dll_1DB = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)dll_1DB_initialise,
    (ObjectDescriptorCallback)dll_1DB_release,
    0,
    (ObjectDescriptorCallback)dll_1DB_init,
    (ObjectDescriptorCallback)dll_1DB_update,
    (ObjectDescriptorCallback)dll_1DB_hitDetect,
    (ObjectDescriptorCallback)dll_1DB_render,
    (ObjectDescriptorCallback)dll_1DB_free,
    (ObjectDescriptorCallback)dll_1DB_getObjectTypeId,
    (ObjectDescriptorExtraSizeCallback)dll_1DB_getExtraSize,
};
