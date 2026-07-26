/* DLL 0x19C */
#include "main/dll/dll_019C_dll19c.h"
#include "game/objects/object_setup.h"
#include "game/objects/object.h"
#include "sys/objects.h"
#include "main/audio/sfx_ids.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/resource.h"
#include "sys/objects/lifecycle.h"
#include "main/frame_timing.h"
#include "main/object_render.h"
#include "main/gamebits.h"
#include "main/audio/sfx.h"
#include "main/gamebit_ids.h"
#include "main/dll/foodbag.h"
#include "dlls/object_descriptor.h"

typedef struct Dll19CPlacement
{
    u8 pad0[0x4 - 0x0];
    u8 color[4];
    f32 posX;
    f32 posY;
    f32 posZ;
    u8 pad14[0x19 - 0x14];
    u8 unk19;
    u8 pad1A[0x1E - 0x1A];
    s8 rotXByte;
    s8 unk1F;
} Dll19CPlacement;

/* type id of the child object dll_19C_update spawns once its gate bit + spawn timer elapse */
#define DLL19C_CHILD_OBJ 0x248

int dll_19C_getExtraSize(void) { return 0x8; }
int dll_19C_getObjectTypeId(void) { return 0x0; }

void dll_19C_free(void)
{
}

void dll_19C_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible)
{
    s32 v = visible;
    if (v != 0) objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, 1.0f);
}

void dll_19C_hitDetect(void)
{
}

void dll_19C_update(GameObject* obj)
{

    Dll19CPlacement* def;
    Dll19CState* sub;
    Dll82Interface** res;
    ObjPlacement* setup;

    def = (Dll19CPlacement*)obj->anim.placementData;
    sub = obj->extra;
    if (obj->userData2 != 0)
    {
        if (mainGetBit(0x1d4) != 0)
        {
            obj->userData2 = 0;
        }
    }
    if (obj->userData2 == 0)
    {
        if (mainGetBit(GAMEBIT_WM_KrazTest1TorchesActive) != 0)
        {
            res = Resource_Acquire(0x82, 1);
            (*res)->spawn(obj, 0, NULL, 1, -1, NULL);
            (*res)->spawn(obj, 1, NULL, 1, -1, NULL);
            Sfx_PlayFromObject(0, SFXTRIG_hitpos_6);
            Resource_Release(res);
            sub->active = 1;
            obj->userData2 = 1;
        }
    }
    if (sub->active != 0)
    {
        sub->spawnTimer = (s16)(sub->spawnTimer - sub->active * framesThisStep);
    }
    if (sub->spawnTimer <= 0 && def->unk1F == 0 && Obj_IsLoadingLocked() != 0)
    {
        setup = Obj_AllocObjectSetup(0x18, DLL19C_CHILD_OBJ);
        setup->posX = def->posX;
        setup->posY = 50.0f + def->posY;
        setup->posZ = def->posZ;
        setup->objectId = DLL19C_CHILD_OBJ;
        setup->mapId = -1;
        setup->color[0] = def->color[0];
        setup->color[1] = def->color[1];
        setup->color[2] = def->color[2];
        setup->color[3] = def->color[3];
        Obj_SetupObject(setup, 5, obj->anim.mapEventSlot, -1, obj->anim.parent);
        sub->spawnTimer = 0x64;
        sub->active = 0;
    }
}

void dll_19C_init(GameObject *obj, u8* initData)
{
    register int self = (int)obj;
    register int state = *(int*)&((GameObject*)self)->extra;
    *(short*)self = (short)((int)((Dll19CPlacement*)initData)->rotXByte << 8);
    ((GameObject*)self)->userData2 = 0;
    ((Dll19CState*)state)->spawnTimer = 0x64;
    ((Dll19CState*)state)->active = 0;
    *(int*)state = 0;
    *(u8*)(self + 0x37) = 0xff;
    ((GameObject*)self)->anim.alpha = 0xff;
}

void dll_19C_release(void)
{
}

void dll_19C_initialise(void)
{
}

ObjectDescriptor dll_19C = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)dll_19C_initialise,
    (ObjectDescriptorCallback)dll_19C_release,
    0,
    (ObjectDescriptorCallback)dll_19C_init,
    (ObjectDescriptorCallback)dll_19C_update,
    (ObjectDescriptorCallback)dll_19C_hitDetect,
    (ObjectDescriptorCallback)dll_19C_render,
    (ObjectDescriptorCallback)dll_19C_free,
    (ObjectDescriptorCallback)dll_19C_getObjectTypeId,
    (ObjectDescriptorExtraSizeCallback)dll_19C_getExtraSize,
};
