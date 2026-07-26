/*
 * dll_00CF cannonclaw - a trigger-once cannon-arm awakener: plays move 0x208
 * until the Tricky object's gate game bit fires, then disables its own hits
 * and stops animating (userData1 latch).
 *
 * The object descriptor and all CannonClaw callbacks live in this TU.
 */
#include "game/objects/object.h"
#include "sys/objects/lifecycle.h"
#include "sys/objects.h"
#include "main/dll/dll_00CF_cannonclaw.h"
#include "main/gamebits.h"
#include "main/objhits.h"
#include "main/frame_timing.h"
#include "main/object_render.h"

#define CANNONCLAW_OBJID_TRICKY 0x1723
#define CANNONCLAW_MOVE_ARM     0x208

int cannonclaw_getExtraSize(void)
{
    return 0x0;
}

int cannonclaw_getObjectTypeId(void)
{
    return 0x0;
}

void cannonclaw_free(void)
{
}

void cannonclaw_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible)
{
    if (visible != 0)
    {
        switch (obj->userData1)
        {
        case 0:
            objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, 1.0f);
            break;
        default:
            break;
        }
    }
}

void cannonclaw_hitDetect(void)
{
}

void cannonclaw_update(GameObject* obj)
{
    GameObject* trickyObj;
    getTrickyObject();
    trickyObj = ObjList_FindObjectById(CANNONCLAW_OBJID_TRICKY);
    if (obj->userData1 != 0)
        return;
    if (obj->anim.currentMove != CANNONCLAW_MOVE_ARM)
    {
        ObjAnim_SetCurrentMove((int)obj, CANNONCLAW_MOVE_ARM, 0.0f, 0);
    }
    ObjAnim_AdvanceCurrentMove((int)obj, 0.005f, timeDelta, NULL);
    if (trickyObj == NULL)
        return;
    if (mainGetBit(trickyObj->anim.placementData[13]) == 0)
        return;
    obj->userData1 = 1;
    *(u8*)&obj->anim.resetHitboxMode = (u8)(*(u8*)&obj->anim.resetHitboxMode | INTERACT_FLAG_DISABLED);
    ObjHits_DisableObject(obj);
}

void cannonclaw_init(GameObject* obj, CannonClawPlacement* placement)
{
    s8 rotXByte = placement->rotXByte;
    s16 rotX = rotXByte << 8;
    obj->anim.rotX = rotX;
}

void cannonclaw_release(void)
{
}

void cannonclaw_initialise(void)
{
}

ObjectDescriptor gCannonClawObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)cannonclaw_initialise,
    (ObjectDescriptorCallback)cannonclaw_release,
    0,
    (ObjectDescriptorCallback)cannonclaw_init,
    (ObjectDescriptorCallback)cannonclaw_update,
    (ObjectDescriptorCallback)cannonclaw_hitDetect,
    (ObjectDescriptorCallback)cannonclaw_render,
    (ObjectDescriptorCallback)cannonclaw_free,
    (ObjectDescriptorCallback)cannonclaw_getObjectTypeId,
    cannonclaw_getExtraSize,
};
