/*
 * DLL 0x1F4 (slot 500) drives a set-dressing object that follows a looping
 * path and emits particle and sound effects. The retail source basename is
 * not yet recovered, so its symbols remain in the numbered DLL namespace.
 */
#include "dlls/objects/500.h"

#include "main/audio/sfx_trigger_ids.h"
#include "main/dll/expgfx_interface.h"
#include "main/dll/partfx_interface.h"
#include "main/frame_timing.h"
#include "main/object_render.h"
#include "main/vecmath.h"
#include "sys/objects.h"

#define DLL500_OBJECT_SFX_CHANNEL 0x40
#define DLL500_OBJECT_SFX_RANGE   100.0f

#define DLL500_STATIC_SEQUENCE_ID 0x3E4

#define DLL500_PARTICLE_SCALE 0.35f
#define DLL500_PARTICLE_ARG3  0xC0D

#define DLL500_BODY_PARTICLE_ID   0x7A8
#define DLL500_BODY_PARTICLE_MODE 6

#define DLL500_PATH_PARTICLE_ID   0x7C7
#define DLL500_PATH_PARTICLE_MODE 2

#define DLL500_MOVE_PROGRESS_RANDOM_MIN 0
#define DLL500_MOVE_PROGRESS_RANDOM_MAX 90
#define DLL500_MOVE_PROGRESS_DIVISOR    100.0f
#define DLL500_MOVE_SPEED               0.003f

#define DLL500_PATH_POINT_INDEX 0
#define DLL500_PATH_POINT_X     0.0f
#define DLL500_PATH_POINT_Y     -12.0f
#define DLL500_PATH_POINT_Z     0.0f

int dll500_getExtraSize(void) {
    return sizeof(Dll500State);
}

void dll500_free(GameObject* obj) {
    Sfx_StopObjectChannel((int)obj, DLL500_OBJECT_SFX_CHANNEL);
    (*gExpgfxInterface)->freeSource2((u32)obj);
}

void dll500_render(GameObject* obj, int renderArg2, int renderArg3, int renderArg4, int renderArg5, s8 visible) {
    if (visible != 0) {
        objRenderModelAndHitVolumes(obj, renderArg2, renderArg3, renderArg4, renderArg5, 1.0f);
    }
}

int dll500_processAnimEvents(GameObject* obj, int unused, ObjAnimUpdateState* animUpdate) {
    PartFxSpawnParams spawnParams;
    int frameIndex;

    (void)unused;
    if ((s32)randomGetRange(0, 1) != 0) {
        animUpdate->sequenceControlFlags = OBJSEQ_CONTROL_SET_LATCH_A;
    } else {
        animUpdate->sequenceControlFlags = OBJSEQ_CONTROL_CLEAR_LATCH_A;
    }
    animUpdate->sequenceEventActive = 0;
    animUpdate->flags = -1;
    animUpdate->flags &= ~0x20;

    if (Obj_GetPlayerObject() == NULL) {
        return 0;
    }
    if ((obj->objectFlags & OBJECT_OBJFLAG_RENDERED) != 0) {
        spawnParams.scale = DLL500_PARTICLE_SCALE;
        spawnParams.arg3 = DLL500_PARTICLE_ARG3;
        spawnParams.posX = spawnParams.posX - obj->anim.worldPosX;
        spawnParams.posY = spawnParams.posY - obj->anim.worldPosY;
        spawnParams.posZ = spawnParams.posZ - obj->anim.worldPosZ;
        for (frameIndex = 0; frameIndex < framesThisStep; frameIndex++) {
            (*gPartfxInterface)
                ->spawnObject(obj, DLL500_BODY_PARTICLE_ID, &spawnParams, DLL500_BODY_PARTICLE_MODE, -1, NULL);
        }
    }
    return 0;
}

void dll500_update(int obj) {
    PartFxSpawnParams spawnParams;
    f32 playerDistance;
    int frameIndex;

    playerDistance = Vec_distance((void*)((int)Obj_GetPlayerObject() + offsetof(GameObject, anim.worldPosX)),
                                  (void*)(obj + offsetof(GameObject, anim.worldPosX)));
    if (Sfx_IsPlayingFromObjectChannel(obj, DLL500_OBJECT_SFX_CHANNEL) == 0) {
        if (playerDistance < DLL500_OBJECT_SFX_RANGE) {
            Sfx_PlayFromObject((u32)obj, SFXTRIG_mushdizzylp12);
        }
    } else if (playerDistance >= DLL500_OBJECT_SFX_RANGE) {
        Sfx_StopObjectChannel(obj, DLL500_OBJECT_SFX_CHANNEL);
    }

    if (((GameObject*)obj)->anim.romDefNo != DLL500_STATIC_SEQUENCE_ID) {
        if (((GameObject*)obj)->userData2 == 0) {
            ((GameObject*)obj)->userData2 = 1;
            ObjAnim_SetMoveProgress((ObjAnimComponent*)obj, (f32)(s32)randomGetRange(DLL500_MOVE_PROGRESS_RANDOM_MIN,
                                                                                     DLL500_MOVE_PROGRESS_RANDOM_MAX) /
                                                                DLL500_MOVE_PROGRESS_DIVISOR);
        }
        ObjAnim_AdvanceCurrentMove(obj, DLL500_MOVE_SPEED, timeDelta, NULL);
    }

    if ((((GameObject*)obj)->objectFlags & OBJECT_OBJFLAG_RENDERED) != 0) {
        spawnParams.scale = DLL500_PARTICLE_SCALE;
        spawnParams.arg3 = DLL500_PARTICLE_ARG3;
        spawnParams.posX = DLL500_PATH_POINT_X;
        spawnParams.posY = DLL500_PATH_POINT_Y;
        spawnParams.posZ = DLL500_PATH_POINT_Z;
        ObjPath_GetPointWorldPosition((GameObject*)obj, DLL500_PATH_POINT_INDEX, &spawnParams.posX, &spawnParams.posY,
                                      &spawnParams.posZ, 1);
        if (((GameObject*)obj)->anim.parent != NULL) {
            spawnParams.posX = spawnParams.posX - ((GameObject*)obj)->anim.worldPosX;
            spawnParams.posY = spawnParams.posY - ((GameObject*)obj)->anim.worldPosY;
            spawnParams.posZ = spawnParams.posZ - ((GameObject*)obj)->anim.worldPosZ;
        } else {
            spawnParams.posX = spawnParams.posX - ((GameObject*)obj)->anim.localPosX;
            spawnParams.posY = spawnParams.posY - ((GameObject*)obj)->anim.localPosY;
            spawnParams.posZ = spawnParams.posZ - ((GameObject*)obj)->anim.localPosZ;
        }
        for (frameIndex = 0; frameIndex < framesThisStep; frameIndex++) {
            (*gPartfxInterface)
                ->spawnObject((void*)obj, DLL500_PATH_PARTICLE_ID, &spawnParams, DLL500_PATH_PARTICLE_MODE, -1, NULL);
        }
    }
}

void dll500_init(GameObject* obj, const Dll500PlacementView* placement) {
    Dll500State* state = obj->extra;

    if (obj->anim.romDefNo == DLL500_STATIC_SEQUENCE_ID) {
        obj->anim.rotX = (s16)((u32)placement->rotXStatic << 8);
    } else {
        obj->anim.rotX = (s16)((s32)placement->rotXSwing << 8);
    }
    obj->anim.rotY = 0;
    obj->anim.rotZ = 0;
    obj->userData2 = 0;
    state->active = 1;
    obj->animEventCallback = dll500_processAnimEvents;
}

ObjectDescriptor gDll500ObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    0,
    0,
    0,
    (ObjectDescriptorCallback)dll500_init,
    (ObjectDescriptorCallback)dll500_update,
    0,
    (ObjectDescriptorCallback)dll500_render,
    (ObjectDescriptorCallback)dll500_free,
    0,
    dll500_getExtraSize,
};
