#include "main/dll/partfx_interface.h"
#include "game/objects/object.h"
#include "game/objects/object_setup.h"
#include "main/objtexture.h"
#include "sys/objects.h"
#include "main/frame_timing.h"
#include "main/object_render.h"
#include "main/vecmath_distance_api.h"
#include "dlls/object_descriptor.h"

#define DLL1DF_OBJFLAG_HITDETECT_DISABLED 0x2000
#define DLL1DF_PARTFX 525

typedef struct Dll1DFPlaceData
{
    ObjPlacement base;
    u8 rotZByte; /* 0x18 */
    u8 rotYByte; /* 0x19 */
    u8 rotXByte; /* 0x1A */
    u8 scaleByte; /* 0x1B: nonzero scales root motion */
} Dll1DFPlaceData;

typedef struct Dll1DFState
{
    u8 pad0[0x4];
    u8 unk4;
    u8 unk5;
    u8 unk6;
    u8 pad7[0x10 - 0x7];
    f32 unk10; /* 0x10: primed to 0.01f at init */
    u8 pad14[0x24 - 0x14];
    f32 spawnTimer; /* 0x24: counts down by timeDelta while player is near */
} Dll1DFState;

STATIC_ASSERT(offsetof(Dll1DFPlaceData, rotZByte) == 0x18);
STATIC_ASSERT(offsetof(Dll1DFPlaceData, scaleByte) == 0x1B);
STATIC_ASSERT(offsetof(Dll1DFState, unk10) == 0x10);
STATIC_ASSERT(offsetof(Dll1DFState, spawnTimer) == 0x24);

int dll_1DF_getExtraSize(void) { return 0x28; }
int dll_1DF_getObjectTypeId(void) { return 0x0; }

void dll_1DF_free(void)
{
}

void dll_1DF_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible)
{
    if (visible != 0) objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, 1.0f);
}

void dll_1DF_hitDetect(void)
{
}

void dll_1DF_update(GameObject* obj)
{
    Dll1DFState* sub = obj->extra;
    ObjTextureRuntimeSlot* tex;
    GameObject* player;
    f32 dist;
    f32 t;

    tex = objFindTexture((GameObject*)(obj), 0, 0);
    if (tex != NULL)
    {
        if (obj->anim.seqId == 209)
        {
            f32 v = 0.0f;
            tex->colorR = v;
            tex->colorG = v;
            tex->colorB = v;
        }
        else
        {
            f32 v = 0.0f;
            tex->colorR = v;
            tex->colorG = v;
            tex->colorB = v;
        }
    }
    player = Obj_GetPlayerObject();
    dist = vec3f_distanceSquared(&player->anim.worldPosX, &obj->anim.worldPosX);
    if (dist < 90000.0f)
    {
        t = sub->spawnTimer - timeDelta;
        sub->spawnTimer = t;
        if (t < 0.0f)
        {
            (*gPartfxInterface)->spawnObject(obj, DLL1DF_PARTFX, NULL, 2, -1, NULL);
            sub->spawnTimer = 12.0f;
        }
    }
}

void dll_1DF_init(GameObject* obj, Dll1DFPlaceData* p)
{
    u32 scaleParam;
    void* objDef;
    void* modelState;
    obj->anim.rotZ = (s16)((u32)p->rotZByte << 8);
    obj->anim.rotY = (s16)((u32)p->rotYByte << 8);
    obj->anim.rotX = (s16)((u32)p->rotXByte << 8);
    scaleParam = p->scaleByte;
    if (scaleParam != 0)
    {
        objDef = *(void**)&obj->anim.modelInstance;
        obj->anim.rootMotionScale = ((ObjDef*)objDef)->rootMotionScaleBase * ((f32)scaleParam / 255.0f);
    }
    ((Dll1DFState*)obj->extra)->unk10 = 0.01f;
    modelState = *(void**)&obj->anim.modelState;
    if (modelState != NULL)
    {
        ((ObjModelState*)modelState)->flags |= 0x810;
    }
    obj->objectFlags |= DLL1DF_OBJFLAG_HITDETECT_DISABLED;
}

void dll_1DF_release(void)
{
}

void dll_1DF_initialise(void)
{
}

ObjectDescriptor gDll1DFObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)dll_1DF_initialise,
    (ObjectDescriptorCallback)dll_1DF_release,
    0,
    (ObjectDescriptorCallback)dll_1DF_init,
    (ObjectDescriptorCallback)dll_1DF_update,
    (ObjectDescriptorCallback)dll_1DF_hitDetect,
    (ObjectDescriptorCallback)dll_1DF_render,
    (ObjectDescriptorCallback)dll_1DF_free,
    (ObjectDescriptorCallback)dll_1DF_getObjectTypeId,
    (ObjectDescriptorExtraSizeCallback)dll_1DF_getExtraSize,
};
