/*
 * DLL 0x1CF - a placement-driven static object. On init it reads its
 * placement: a gate game bit arms its degree-based rotY setup, rotX comes
 * from a byte angle, and the object starts hidden with updates and hit
 * detection disabled.
 */
#include "main/dll/dll_01CF_dll1cf.h"
#include "game/objects/object.h"
#include "main/gamebits.h"
#include "dlls/object_descriptor.h"
#include "main/object_render.h"

int dll_1CF_getExtraSize(void)
{
    return 0x0;
}

int dll_1CF_getObjectTypeId(void)
{
    return 0x0;
}

void dll_1CF_free(void)
{
}

void dll_1CF_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible)
{
    s32 visibleInt = visible;
    if (visibleInt != 0)
    {
        objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, 1.0f);
    }
}

void dll_1CF_hitDetect(void)
{
}

void dll_1CF_update(void)
{
}

void dll_1CF_init(GameObject* obj, Dll1CFPlacement* placement)
{
    if ((u32)mainGetBit(placement->gateGameBit) != 0u)
    {
        obj->anim.rotY = (s16)(((s32)placement->rotYDegrees << 13) / 45);
    }
    obj->anim.rotX = (s16)((s32)placement->rotXByte << 8);
    obj->objectFlags = (u16)(obj->objectFlags | (OBJECT_OBJFLAG_HITDETECT_DISABLED | OBJECT_OBJFLAG_HIDDEN |
                                                OBJECT_OBJFLAG_UPDATE_DISABLED));
}

void dll_1CF_release(void)
{
}

void dll_1CF_initialise(void)
{
}

ObjectDescriptor dll_1CF = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)dll_1CF_initialise,
    (ObjectDescriptorCallback)dll_1CF_release,
    0,
    (ObjectDescriptorCallback)dll_1CF_init,
    (ObjectDescriptorCallback)dll_1CF_update,
    (ObjectDescriptorCallback)dll_1CF_hitDetect,
    (ObjectDescriptorCallback)dll_1CF_render,
    (ObjectDescriptorCallback)dll_1CF_free,
    (ObjectDescriptorCallback)dll_1CF_getObjectTypeId,
    dll_1CF_getExtraSize,
};
