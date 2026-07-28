/*
 * WM_colrise (DLL 0x0201) - the rising column platform at Krazoa Palace.
 *
 * While its game bit allows and something stands on a column higher
 * than 3.0 above it (the rider registry the shared platform
 * helpers maintain), the column rises 0.25/tick toward
 * placement height + 120 and plays its rumble; otherwise it sinks
 * 0.125/tick back to placement height.
 */
#include "dlls/object_descriptor.h"
#include "main/audio/sfx.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/dll/WM/dll_0201_wmcolrise.h"
#include "main/frame_timing.h"
#include "main/gamebits.h"
#include "main/object_render.h"

#define WM_COLRISE_RENDER_SCALE          1.0f
#define WM_COLRISE_RIDER_HEIGHT          3.0f
#define WM_COLRISE_RAISED_OFFSET_LOW     20.0f
#define WM_COLRISE_RAISED_OFFSET_HIGH    100.0f
#define WM_COLRISE_SETTLE_SPEED          0.5f
#define WM_COLRISE_RISE_SPEED            0.25f
#define WM_COLRISE_SINK_SPEED            0.125f

int WM_colrise_SeqFn(GameObject* obj, int unused, ObjAnimUpdateState* animUpdate)
{
    animUpdate->hitVolumePair = -1;
    animUpdate->sequenceEventActive = 0;
    return 0;
}

int WM_colrise_getExtraSize(void)
{
    return sizeof(WMColriseState);
}
int WM_colrise_getObjectTypeId(void)
{
    return 0x0;
}

void WM_colrise_free(void)
{
}

void WM_colrise_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible)
{
    s32 v = visible;
    if (v != 0)
        objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, WM_COLRISE_RENDER_SCALE);
}

void WM_colrise_hitDetect(void)
{
}

void WM_colrise_update(GameObject* obj)
{
    WMColrisePlacement* placement;
    WMColriseState* state;
    s32 reached;
    f32 target;
    int i;

    placement = (WMColrisePlacement*)obj->anim.placementData;
    state = obj->extra;
    state->raiseTimer -= 1;
    if (state->raiseTimer < 0)
        state->raiseTimer = 0;
    /* rearm the 60-frame rise window while any rider sits more than
       3.0 above the column */
    if (obj->anim.hitboxTransformState->contactObjectCount > 0)
    {
        for (i = 0; i < obj->anim.hitboxTransformState->contactObjectCount; i++)
        {
            GameObject* rider = obj->anim.hitboxTransformState->contactObjects[i];
            if (rider->anim.localPosY - obj->anim.localPosY > WM_COLRISE_RIDER_HEIGHT)
            {
                state->raiseTimer = 0x3c;
            }
        }
    }
    reached = 0;
    if ((state->gameBit == -1 || (u32)mainGetBit(state->gameBit) != 0) && state->raiseTimer != 0)
    {
        target = WM_COLRISE_RAISED_OFFSET_LOW + (WM_COLRISE_RAISED_OFFSET_HIGH + placement->base.posY);
        if (obj->anim.localPosY > target)
        {
            obj->anim.localPosY = obj->anim.localPosY - WM_COLRISE_SETTLE_SPEED * timeDelta;
            if (obj->anim.localPosY > target)
            {
                obj->anim.localPosY = target;
            }
        }
        else
        {
            obj->anim.localPosY = WM_COLRISE_RISE_SPEED * timeDelta + obj->anim.localPosY;
            if (obj->anim.localPosY > target)
            {
                obj->anim.localPosY = target;
            }
            else
            {
                reached = 1;
            }
        }
    }
    else
    {
        obj->anim.localPosY = obj->anim.localPosY - WM_COLRISE_SINK_SPEED * timeDelta;
        if (obj->anim.localPosY < placement->base.posY)
        {
            obj->anim.localPosY = placement->base.posY;
        }
        else
        {
            reached = 1;
        }
    }
    if ((s8)reached != 0)
    {
        Sfx_PlayFromObject((int)obj, SFXTRIG_en_treedrum16_7d);
    }
    else
    {
        Sfx_StopObjectChannel((int)obj, 8);
    }
}

void WM_colrise_init(GameObject* obj, WMColrisePlacement* placement)
{
    WMColriseState* state = obj->extra;
    obj->animEventCallback = WM_colrise_SeqFn;
    obj->anim.rotX = (s16)((s32)placement->rotXByte << 8);
    state->gameBit = placement->gameBit;
}

void WM_colrise_release(void)
{
}

void WM_colrise_initialise(void)
{
}

ObjectDescriptor gWM_colriseObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)WM_colrise_initialise,
    (ObjectDescriptorCallback)WM_colrise_release,
    0,
    (ObjectDescriptorCallback)WM_colrise_init,
    (ObjectDescriptorCallback)WM_colrise_update,
    (ObjectDescriptorCallback)WM_colrise_hitDetect,
    (ObjectDescriptorCallback)WM_colrise_render,
    (ObjectDescriptorCallback)WM_colrise_free,
    (ObjectDescriptorCallback)WM_colrise_getObjectTypeId,
    (ObjectDescriptorExtraSizeCallback)WM_colrise_getExtraSize,
};
