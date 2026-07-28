/* DLL 0x0220 */
#include "dlls/object_descriptor.h"
#include "dolphin/mtx/vec.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/camera.h"
#include "main/dll/expgfx_interface.h"
#include "main/frame_timing.h"
#include "main/gamebits.h"
#include "main/objanim_internal.h"
#include "main/object_render_legacy.h"
#include "main/objfx.h"
#include "game/objects/object_setup.h"

#define VFPDOORSWITCH_OBJFLAG_HITDETECT_DISABLED 0x2000
#define VFP_DOORSWITCH_LIFTIND_OBJ 0x3e7

static const f32 lbl_803E6118 = 0.025f;
static const f32 lbl_803E611C = 1.0f;
static const f32 lbl_803E6120 = 100.0f;
static const f32 lbl_803E6124 = 80.0f;

typedef struct VfpDoorSwitchState
{
    s16 gameBitId;
    u8 activated : 1;
    u8 exploded : 1;
    u8 _state2_lo : 6;
} VfpDoorSwitchState;

typedef struct VfpDoorSwitchPlacement
{
    ObjPlacement base;
    s8 rotXByte;
    s8 rotZByte;
    u8 pad1A[2];
    s16 rotY;
    s16 gameBitId;
} VfpDoorSwitchPlacement;

void vfpdoorswitch_updateExplodingVariant(GameObject* obj)
{
    VfpDoorSwitchState* state = obj->extra;
    CameraViewSlot* camView = Camera_GetCurrentViewSlot();

    if (state->activated == 0)
    {
        if (mainGetBit(state->gameBitId) != 0)
        {
            Sfx_PlayFromObject(0, SFXTRIG_menuups16k);
            Sfx_PlayFromObject((int)obj, SFXTRIG_dn_boar1_c_10d);
            Sfx_PlayFromObject((int)obj, SFXTRIG_gate_stops);
            state->activated = 1;
        }
    }
    if (state->activated != 0)
    {
        ObjAnim_AdvanceCurrentMove((int)obj, lbl_803E6118, timeDelta, NULL);
        if (state->exploded == 0)
        {
            if (obj->anim.currentMoveProgress >= lbl_803E611C)
            {
                Vec vec;
                PSVECSubtract(&camView->position, &obj->anim.localPos, &vec);
                PSVECNormalize(&vec, &vec);
                PSVECScale(&vec, &vec, lbl_803E6120);
                PSVECAdd(&obj->anim.localPos, &vec, &obj->anim.localPos);
                obj->anim.worldPosX = obj->anim.localPosX;
                obj->anim.worldPosY = obj->anim.localPosY;
                obj->anim.worldPosZ = obj->anim.localPosZ;
                spawnExplosionLegacy((int)obj, lbl_803E6124, 1, 1, 0, 0, 0, 0, 0);
                state->exploded = 1;
                obj->anim.flags |= OBJANIM_FLAG_HIDDEN;
            }
        }
    }
}

int VFP_DoorSwitch_getExtraSize(void)
{
    return 0x4;
}

int VFP_DoorSwitch_getObjectTypeId(void)
{
    return 0x0;
}

void VFP_DoorSwitch_free(int obj)
{
    (*gExpgfxInterface)->freeSource2(obj);
}

void VFP_DoorSwitch_render(int p1, int p2, int p3, int p4, int p5, s8 visible)
{
    objRenderModelAndHitVolumes(p1, p2, p3, p4, p5, lbl_803E611C);
}

void VFP_DoorSwitch_hitDetect(void)
{
}

void VFP_DoorSwitch_update(GameObject* obj)
{
    VfpDoorSwitchState* state;
    if ((obj)->anim.seqId != VFP_DOORSWITCH_LIFTIND_OBJ)
    {
        vfpdoorswitch_updateExplodingVariant(obj);
        return;
    }
    state = (obj)->extra;
    if (state->activated != 0)
        return;
    if (mainGetBit(state->gameBitId) == 0)
        return;
    Sfx_PlayFromObject(0, SFXTRIG_menuups16k);
    Sfx_PlayFromObject((int)obj, SFXTRIG_dn_boar1_c_10d);
    Sfx_PlayFromObject((int)obj, SFXTRIG_gate_stops);
    Obj_SetActiveModelIndex(obj, 1);
    state->activated = 1;
}

void VFP_DoorSwitch_init(GameObject* obj, int data)
{
    VfpDoorSwitchPlacement* def = (VfpDoorSwitchPlacement*)data;
    VfpDoorSwitchState* state = obj->extra;
    obj->anim.rotX = (((s32)def->rotXByte) << 8);
    obj->anim.rotZ = (((s32)def->rotZByte) << 8);
    obj->anim.rotY = def->rotY;
    state->gameBitId = def->gameBitId;
    if (mainGetBit(state->gameBitId) != 0)
    {
        ((ObjAnimSetProgressObjectFirstFn)ObjAnim_SetMoveProgress)((int)obj, lbl_803E611C);
        state->activated = 1;
        state->exploded = 1;
        obj->anim.flags |= OBJANIM_FLAG_HIDDEN;
    }
    if (obj->anim.seqId == VFP_DOORSWITCH_LIFTIND_OBJ && state->activated != 0)
    {
        *&obj->anim.bankIndex = 1;
    }
    obj->objectFlags |= VFPDOORSWITCH_OBJFLAG_HITDETECT_DISABLED;
}

void VFP_DoorSwitch_release(void)
{
}

void VFP_DoorSwitch_initialise(void)
{
}

ObjectDescriptor gVFP_DoorSwitchObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)VFP_DoorSwitch_initialise,
    (ObjectDescriptorCallback)VFP_DoorSwitch_release,
    0,
    (ObjectDescriptorCallback)VFP_DoorSwitch_init,
    (ObjectDescriptorCallback)VFP_DoorSwitch_update,
    (ObjectDescriptorCallback)VFP_DoorSwitch_hitDetect,
    (ObjectDescriptorCallback)VFP_DoorSwitch_render,
    (ObjectDescriptorCallback)VFP_DoorSwitch_free,
    (ObjectDescriptorCallback)VFP_DoorSwitch_getObjectTypeId,
    (ObjectDescriptorExtraSizeCallback)VFP_DoorSwitch_getExtraSize,
};
