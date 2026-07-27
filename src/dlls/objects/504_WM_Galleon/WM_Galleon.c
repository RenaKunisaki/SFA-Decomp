/* WM_Galleon (DLL 0x01F8) - the World Map galleon. */
#include "main/dll/WC/dll_01F9_wmobjcreator.h"
#include "dlls/object_descriptor.h"
#include "main/render_lactions_api.h"
#include "game/objects/object.h"
#include "sys/objects.h"
#include "main/object_render.h"
#include "main/mapEventTypes.h"
#include "main/resource.h"
#include "main/objseq.h"
#include "main/gamebits.h"
#include "main/frame_timing.h"
#include "main/dll/dll_0011_screens.h"
#include "main/track_dolphin_api.h"
#include "main/dll/wmgalleonsetup_struct.h"
#include "main/dll/wmgalleonstate_struct.h"
#include "main/dll/player_api.h"

u32 lbl_803DC0F0 = 3;

STATIC_ASSERT(sizeof(WMGalleonState) == 0x10);
STATIC_ASSERT(offsetof(WMGalleonState, savedX) == 0x00);
STATIC_ASSERT(offsetof(WMGalleonState, savedY) == 0x04);
STATIC_ASSERT(offsetof(WMGalleonState, savedZ) == 0x08);
STATIC_ASSERT(offsetof(WMGalleonState, mapEventsLatched) == 0x0C);
STATIC_ASSERT(offsetof(WMGalleonState, savedYaw) == 0x0E);
STATIC_ASSERT(offsetof(WMGalleonSetup, yawByte) == 0x18);

#define WM_GALLEON_GAMEBIT_CUTSCENE_DONE    0x429
#define WM_GALLEON_GAMEBIT_CLEAR_DOOR       0xD1
#define WM_GALLEON_COMMAND_OPENED           1
#define WM_GALLEON_COMMAND_CLEAR_LACTIONS   2
#define WM_GALLEON_COMMAND_SCREEN_FADE      3
#define WM_GALLEON_COMMAND_ACTION_12        4
#define WM_GALLEON_COMMAND_ACTION_13        5
#define WM_GALLEON_COMMAND_CLEAR_MAP_EVENTS 6
#define WM_GALLEON_COMMAND_SHOW_MODEL       7
#define WM_GALLEON_COMMAND_HIDE_MODEL       8
#define WM_GALLEON_COMMAND_ACTION_11        9
#define WM_GALLEON_ACTION_OPENED            10
#define WM_GALLEON_ACTION_11                11
#define WM_GALLEON_ACTION_12                12
#define WM_GALLEON_ACTION_13                13


void* lbl_803DDC74;
extern u32* lbl_803DCA94;
s8 lbl_803DDC70;

int WM_Galleon_getExtraSize(void);
int WM_Galleon_getObjectTypeId(void);
void WM_Galleon_free(GameObject* obj, int leavingMap);
void WM_Galleon_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible);
void WM_Galleon_hitDetect(void);
void WM_Galleon_update(GameObject* obj);
void WM_Galleon_init(GameObject* obj, WMGalleonSetup* setup);
void WM_Galleon_release(void);
void WM_Galleon_initialise(void);

ObjectDescriptor gWM_GalleonObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)WM_Galleon_initialise,
    (ObjectDescriptorCallback)WM_Galleon_release,
    0,
    (ObjectDescriptorCallback)WM_Galleon_init,
    (ObjectDescriptorCallback)WM_Galleon_update,
    (ObjectDescriptorCallback)WM_Galleon_hitDetect,
    (ObjectDescriptorCallback)WM_Galleon_render,
    (ObjectDescriptorCallback)WM_Galleon_free,
    (ObjectDescriptorCallback)WM_Galleon_getObjectTypeId,
    (ObjectDescriptorExtraSizeCallback)WM_Galleon_getExtraSize,
};

int WM_Galleon_SeqFn(GameObject* obj, int unused, ObjAnimUpdateState* animUpdate)
{
    int i;

    lbl_803DC0F0 = framesThisStep;
    animUpdate->hitVolumePair = -1;
    animUpdate->sequenceEventActive = 0;
    for (i = 0; i < animUpdate->eventCount; i++)
    {
        switch (animUpdate->eventIds[i])
        {
        case WM_GALLEON_COMMAND_OPENED:
            obj->userData1 = WM_GALLEON_ACTION_OPENED;
            break;
        case WM_GALLEON_COMMAND_ACTION_11:
            obj->userData1 = WM_GALLEON_ACTION_11;
            break;
        case WM_GALLEON_COMMAND_ACTION_12:
            obj->userData1 = WM_GALLEON_ACTION_12;
            break;
        case WM_GALLEON_COMMAND_ACTION_13:
            obj->userData1 = WM_GALLEON_ACTION_13;
            break;
        case WM_GALLEON_COMMAND_CLEAR_MAP_EVENTS:
            (*gMapEventInterface)->setObjGroupStatus(obj->anim.pad34, 1, 0);
            (*gMapEventInterface)->setObjGroupStatus(obj->anim.pad34, 2, 0);
            (*gMapEventInterface)->setObjGroupStatus(obj->anim.pad34, 4, 0);
            mainSetBits(WM_GALLEON_GAMEBIT_CLEAR_DOOR, 0);
            break;
        case WM_GALLEON_COMMAND_CLEAR_LACTIONS:
            getLActions((void*)obj, (void*)obj, 0x77, 0, 0, 0);
            getLActions((void*)obj, (void*)obj, 0x78, 0, 0, 0);
            getLActions((void*)obj, (void*)obj, 0x80, 0, 0, 0);
            break;
        case WM_GALLEON_COMMAND_SCREEN_FADE:
            (*(void (**)(int, int, int))((u8*)*lbl_803DCA94 + 0x14))(0, 0x1e, 0x50);
            break;
        case WM_GALLEON_COMMAND_SHOW_MODEL:
            lbl_803DDC70 = 1;
            break;
        case WM_GALLEON_COMMAND_HIDE_MODEL:
            lbl_803DDC70 = 0;
            break;
        }
    }

    if (mainGetBit(WM_GALLEON_GAMEBIT_CUTSCENE_DONE) != 0)
    {
        if ((u8)(*gMapEventInterface)->getObjGroupStatus(obj->anim.pad34, 2) != 0)
        {
            (*gMapEventInterface)->setObjGroupStatus(obj->anim.pad34, 1, 0);
            (*gMapEventInterface)->setObjGroupStatus(obj->anim.pad34, 2, 0);
        }
    }
    return 0;
}

int WM_Galleon_getExtraSize(void)
{
    return 0x10;
}
int WM_Galleon_getObjectTypeId(void)
{
    return 0x0;
}

void WM_Galleon_free(GameObject* obj, int leavingMap)
{
    if (obj->anim.seqId != 0x188)
    {
        WMGalleonState* state = obj->extra;
        if (state->mapEventsLatched != 0 && leavingMap == 0)
        {
            state->mapEventsLatched = 0;
        }
        if (lbl_803DDC74 != NULL)
        {
            Resource_Release(lbl_803DDC74);
            lbl_803DDC74 = NULL;
        }
    }
}

void WM_Galleon_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible)
{
    if (mainGetBit(GAMEBIT_WM_Galleon_despawn) != 0)
    {
        return;
    }
    if (visible == 0)
    {
        return;
    }
    if ((obj)->anim.seqId == 0x188 && ((GameObject*)(obj)->anim.parent)->userData1 >= 7)
    {
        return;
    }

    objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, 1.0f);

    if (lbl_803DDC70 != 0)
    {
        gScreensInterface->vtable->show(1);
    }
}

void WM_Galleon_hitDetect(void)
{
}

void WM_Galleon_update(GameObject* obj)
{
    GameObject* player;
    WMGalleonState* state;
    int gameBitA4;

    if (mainGetBit(GAMEBIT_WM_Galleon_despawn) != 0)
    {
        return;
    }

    if (obj->anim.seqId == 0x188)
    {
        obj->anim.alpha = 0x80;
        return;
    }

    player = Obj_GetPlayerObject();
    state = obj->extra;

    if (mainGetBit(WM_GALLEON_GAMEBIT_CUTSCENE_DONE) != 0)
    {
        if ((u8)(*gMapEventInterface)->getObjGroupStatus(obj->anim.pad34, 2) != 0)
        {
            (*gMapEventInterface)->setObjGroupStatus(obj->anim.pad34, 1, 0);
            (*gMapEventInterface)->setObjGroupStatus(obj->anim.pad34, 2, 0);
        }
    }
    else if ((mainGetBit(GAMEBIT_WM_GalleonRelated00D0) == 0) &&
             ((u8)(*gMapEventInterface)->getObjGroupStatus(obj->anim.pad34, 2) == 0))
    {
        (*gMapEventInterface)->setObjGroupStatus(obj->anim.pad34, 1, 1);
        (*gMapEventInterface)->setObjGroupStatus(obj->anim.pad34, 2, 1);
    }

    if (mainGetBit(GAMEBIT_WM_GalleonRelated00D0) == 0)
    {
        if ((state->mapEventsLatched == 0) && (mainGetBit(WM_GALLEON_GAMEBIT_CUTSCENE_DONE) == 0))
        {
            (*gMapEventInterface)->setObjGroupStatus(obj->anim.pad34, 1, 1);
            (*gMapEventInterface)->setObjGroupStatus(obj->anim.pad34, 2, 1);
            state->mapEventsLatched = 1;
        }
    }
    else
    {
        if ((u8)(*gMapEventInterface)->getObjGroupStatus(obj->anim.pad34, 4) == 0)
        {
            (*gMapEventInterface)->setObjGroupStatus(obj->anim.pad34, 4, 1);
        }
        if (state->mapEventsLatched != 0)
        {
            state->mapEventsLatched = 0;
        }
    }

    gameBitA4 = mainGetBit(GAMEBIT_WM_GalleonRelated00A4);
    if (gameBitA4 != 0)
    {
        obj->userData1 = 10;
    }
    if (gameBitA4 == 0)
    {
        player->anim.localPosX = -121.0f;
        player->anim.localPosY = 116.0f;
        player->anim.localPosZ = 5.0f;
        objHitDetectFn_80062e84(player, obj, 0);
        fn_80296BBC(player);
        obj->userData2 = 1;
    }
    else if (obj->userData2 == 1)
    {
        obj->anim.localPosX = state->savedX;
        obj->anim.localPosY = state->savedY;
        obj->anim.localPosZ = state->savedZ;
        obj->anim.rotX = state->savedYaw;
        (*gObjectTriggerInterface)->runSequence(0, obj, -1);
        obj->userData2 = 2;
    }
}

void WM_Galleon_init(GameObject* obj, WMGalleonSetup* setup)
{
    WMGalleonState* state;
    int i;

    state = obj->extra;
    if (mainGetBit(GAMEBIT_WM_Galleon_despawn) != 0)
    {
        return;
    }
    if (obj->anim.seqId == 0x188)
    {
        return;
    }
    objSetSlot(obj, 0x5a);
    obj->animEventCallback = WM_Galleon_SeqFn;
    obj->anim.rotX = (s16)(setup->yawByte << 8);
    obj->userData1 = 9;
    state->savedX = obj->anim.localPosX;
    state->savedY = obj->anim.localPosY;
    state->savedZ = obj->anim.localPosZ;
    state->savedYaw = obj->anim.rotX;
    trackSetLinesEnabledByParam(0, obj, 0);
    for (i = 0; i < 5; i++)
    {
        (*gMapEventInterface)->setObjGroupStatus(obj->anim.pad34, i, 0);
    }
    mainSetBits(GAMEBIT_WM_GalleonRelated00A4, 1);
}

void WM_Galleon_release(void)
{
}

void WM_Galleon_initialise(void)
{
}

