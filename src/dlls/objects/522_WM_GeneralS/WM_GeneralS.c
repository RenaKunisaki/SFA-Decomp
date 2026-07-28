/*
 * WM_GeneralS (DLL 0x020A) - General Scales at Krazoa Palace, the
 * cutscene actor driven entirely by sequence events (his appearance in
 * the final spirit ceremony).
 *
 * The SeqFn fades the model in/out through state->fadeAlpha, spawns
 * impact particles + sfx on the slam events, and attaches/detaches his
 * 'scalessword' child object on demand. He starts hidden (phase 1
 * skips render).
 */
#include "dlls/object_descriptor.h"
#include "game/objects/object.h"
#include "game/objects/object_setup.h"
#include "main/audio/sfx_play_legacy_api.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/dll/WM/dll_020A_wmgeneralscales.h"
#include "main/dll/partfx_interface.h"
#include "main/frame_timing.h"
#include "main/obj_link.h"
#include "main/object_render.h"
#include "sys/objects.h"
#include "sys/objects/lifecycle.h"

/* Sequence-event phase values. */
#define WMGENERALSCALES_PARTFX_SLAM 0x556 /* slam impact effect (anim events 2/3) */

#define WMGENERALSCALES_PHASE_IDLE   0
#define WMGENERALSCALES_PHASE_HIDDEN 1
#define WMGENERALSCALES_PHASE_SLAM0  2
#define WMGENERALSCALES_PHASE_SLAM1  3

/* romlist object type of the sword child (retail 'scalessword') */
#define WMGENERALSCALES_SWORD_OBJECT_TYPE 0x1B8

ObjectDescriptor gWM_GeneralScalesObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)WM_GeneralScales_initialise,
    (ObjectDescriptorCallback)WM_GeneralScales_release,
    0,
    (ObjectDescriptorCallback)WM_GeneralScales_init,
    (ObjectDescriptorCallback)WM_GeneralScales_update,
    (ObjectDescriptorCallback)WM_GeneralScales_hitDetect,
    (ObjectDescriptorCallback)WM_GeneralScales_render,
    (ObjectDescriptorCallback)WM_GeneralScales_free,
    (ObjectDescriptorCallback)WM_GeneralScales_getObjectTypeId,
    (ObjectDescriptorExtraSizeCallback)WM_GeneralScales_getExtraSize,
};

int WM_GeneralScales_SeqFn(int obj, int unused, ObjAnimUpdateState* animUpdate)
{
    WmGeneralScalesState* state;
    int i;
    u8 buf[16];

    state = ((GameObject*)obj)->extra;
    if (state->fadeAlpha != 0)
    {
        int a = state->fadeAlpha + framesThisStep;
        if (a < 0)
        {
            a = 0;
        }
        else if (a > 0xff)
        {
            a = 0xff;
        }
        state->fadeAlpha = a;
        Obj_SetModelRenderOpAlpha((void*)obj, (u8)a);
    }
    else
    {
        Obj_SetModelRenderOpAlpha((void*)obj, 0);
    }
    for (i = 0; i < animUpdate->eventCount; i++)
    {
        switch (animUpdate->eventIds[i])
        {
        case 1: /* hide */
            state->phase = WMGENERALSCALES_PHASE_HIDDEN;
            break;
        case 2: /* slam, tracked fx */
            state->phase = WMGENERALSCALES_PHASE_SLAM0;
            (*gPartfxInterface)->spawnObject((void*)obj, WMGENERALSCALES_PARTFX_SLAM, NULL, 2, -1, buf);
            Sfx_PlayFromObject(obj, SFXTRIG_id_7b);
            Sfx_PlayFromObject(obj, SFXTRIG_id_7c);
            state->unk00 = 0.0f;
            break;
        case 3: /* slam variant */
            state->phase = WMGENERALSCALES_PHASE_SLAM1;
            (*gPartfxInterface)->spawnObject((void*)obj, WMGENERALSCALES_PARTFX_SLAM, NULL, 2, -1, NULL);
            Sfx_PlayFromObject(obj, SFXTRIG_id_7b);
            Sfx_PlayFromObject(obj, SFXTRIG_id_7c);
            state->unk00 = 800.0f;
            break;
        case 4: /* back to idle */
            state->phase = WMGENERALSCALES_PHASE_IDLE;
            break;
        case 5: /* draw the sword: spawn + attach a scalessword child */
            if (((GameObject*)obj)->childObjs[0] == NULL && Obj_IsLoadingLocked() != 0)
            {
                ObjPlacement* setup = Obj_AllocObjectSetup(0x24, WMGENERALSCALES_SWORD_OBJECT_TYPE);
                setup->posX = ((GameObject*)obj)->anim.localPosX;
                setup->posY = ((GameObject*)obj)->anim.localPosY;
                setup->posZ = ((GameObject*)obj)->anim.localPosZ;
                setup->color[0] = 0x20;
                setup->color[1] = 4;
                setup->color[3] = 0xff;
                ObjLink_AttachChild((GameObject*)obj, Obj_SetupObject(setup, 5, -1, -1, 0), 0);
                ((GameObject*)((GameObject*)obj)->childObjs[0])->anim.rootMotionScale *= 1.1f;
            }
            break;
        case 6: /* sheathe: detach the sword child */
        {
            GameObject* child = ((GameObject*)obj)->childObjs[0];
            if (child != NULL)
            {
                ObjLink_DetachChild((GameObject*)obj, child);
            }
            break;
        }
        case 7: /* begin fade-in (model flag + alpha ramp from 1) */
        {
            ObjDef* def = ((GameObject*)obj)->anim.modelInstance;
            def->renderFlags |= 0x10;
            state->fadeAlpha = 1;
            break;
        }
        case 8: /* end fade: clear the flag, fully invisible */
        {
            ObjDef* def = ((GameObject*)obj)->anim.modelInstance;
            def->renderFlags &= ~0x10;
            Obj_SetModelRenderOpAlpha((void*)obj, 0);
            state->fadeAlpha = 0;
            break;
        }
        }
        animUpdate->eventIds[i] = 0;
    }
    return 0;
}

int WM_GeneralScales_getExtraSize(void)
{
    return sizeof(WmGeneralScalesState);
}
int WM_GeneralScales_getObjectTypeId(void)
{
    return 0x9;
}

void WM_GeneralScales_free(GameObject* obj)
{
    GameObject* child = obj->childObjs[0];
    if (child != NULL)
        ObjLink_DetachChild(obj, child);
}

void WM_GeneralScales_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible)
{
    WmGeneralScalesState* state = obj->extra;
    if (state->phase == WMGENERALSCALES_PHASE_HIDDEN)
        return;
    if (visible == 0)
        return;
    objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, 1.0f);
}

void WM_GeneralScales_hitDetect(void)
{
}

void WM_GeneralScales_update(void)
{
}

void WM_GeneralScales_init(GameObject* obj)
{
    WmGeneralScalesState* state = obj->extra;
    obj->animEventCallback = WM_GeneralScales_SeqFn;
    state->unk00 = 0.0f;
    state->phase = WMGENERALSCALES_PHASE_HIDDEN;
    obj->childObjs[0] = NULL;
}

void WM_GeneralScales_release(void)
{
}

void WM_GeneralScales_initialise(void)
{
}

