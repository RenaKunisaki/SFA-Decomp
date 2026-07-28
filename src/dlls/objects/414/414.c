/*
 * DLL 0x19E (generated slot 414).
 *
 * This unit controls a companion-related sparkle and egg-interaction effect.
 * Retail OBJECTS.bin and the root romlists do not associate its descriptor
 * with a named object, so the unit retains a numbered namespace.
 */
#include "dlls/objects/414.h"

#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/audio/sfx_ids.h"
#include "main/audio/sfx_play_api.h"
#include "main/camera.h"
#include "main/dll/expgfx_interface.h"
#include "main/dll/modgfx_interface.h"
#include "main/dll/partfx_interface.h"
#include "main/frame_timing.h"
#include "main/gamebits_api.h"
#include "main/resource.h"
#include "main/vecmath.h"
#include "main/voxmaps.h"

#define DLL414_LOS_MIN_DISTANCE       50.0f
#define DLL414_LOS_OBJECT_OFFSET      32.0f
#define DLL414_LOS_CAMERA_OFFSET      (-20.0f)
#define DLL414_PACKED_SCALE_DIVISOR   8192.0f
#define DLL414_IDLE_PARTICLE_ID       0x1F7
#define DLL414_ACTIVATION_PARTICLE_ID 0x1A3
#define DLL414_EFFECT_RESOURCE_ID     0x69

#define DLL414_IDLE_PARTICLE_MODE        0x12
#define DLL414_IDLE_DELAY_RANDOM_MIN     (-10)
#define DLL414_IDLE_DELAY_RANDOM_MAX     10
#define DLL414_IDLE_DELAY_BASE           60
#define DLL414_EFFECT_SCALE              (-2.0f)
#define DLL414_EFFECT_SPAWN_FLAGS        0x10004
#define DLL414_ACTIVATION_PARTICLE_COUNT 100
#define DLL414_ACTIVE_RESET_TIME         1000
#define DLL414_SEQUENCE_RESET_TIME       300
#define DLL414_SETTLE_TRIGGER_WINDOW     20
#define DLL414_SEQUENCE_SETTLE_BASE      0x398
#define DLL414_SEQUENCE_SETTLE_STEP      0x28
#define DLL414_EFFECT_PARAM1_BASE        0x19D
#define DLL414_EFFECT_PARAM2_BASE        0x19E
#define DLL414_ROTATION_INDEX_MASK       0x3F
#define DLL414_ROTATION_INDEX_SHIFT      10
#define DLL414_DEFAULT_SCALE             0.1f
#define DLL414_EFFECT_SFX_CHANNEL        0x40

typedef enum Dll414SequenceStage {
    DLL414_SEQUENCE_STAGE_NONE = 0,
    DLL414_SEQUENCE_STAGE_FIRST = 1,
    DLL414_SEQUENCE_STAGE_SECOND = 2,
    DLL414_SEQUENCE_STAGE_COMPLETE = 3,
} Dll414SequenceStage;

typedef struct Dll414RenderScratch {
    f32 cameraDelta[3];
    PartFxSpawnParams particleParams;
} Dll414RenderScratch;

typedef struct Dll414EffectSpawnBuffer {
    u8 args[16];
    f32 scale;
} Dll414EffectSpawnBuffer;

STATIC_ASSERT(sizeof(Dll414RenderScratch) == 0x24);
STATIC_ASSERT(offsetof(Dll414RenderScratch, particleParams) == 0x0C);
STATIC_ASSERT(sizeof(Dll414EffectSpawnBuffer) == 0x14);
STATIC_ASSERT(offsetof(Dll414EffectSpawnBuffer, scale) == 0x10);

s8 gDll414SequenceStage;

const Dll69EffectParams gDll414EffectParamsTemplate = {0x3E7, 0x8C, 0x8D, 0x28};

int dll414_getExtraSize(void) {
    return sizeof(Dll414State);
}

int dll414_getObjectTypeId(void) {
    return 1;
}

void dll414_free(GameObject* obj) {
    (*gModgfxInterface)->detachSource(obj);
    (*gExpgfxInterface)->freeSource2((u32)obj);
}

void dll414_render(GameObject* obj, int renderArg2, int renderArg3, int renderArg4, int renderArg5, s8 visible) {
    Dll414State* state;
    CameraViewSlot* camera;
    f32 distance;
    f32 inverseDistance;
    f32 objectOffsetZ, objectOffsetY, objectOffsetX;
    f32 cameraOffsetZ, cameraOffsetY, cameraOffsetX;
    f32 normalZ, normalY, normalX;
    Dll414RenderScratch scratch;
    f32 objectTraceStart[3];
    f32 cameraTraceEnd[3];
    f32 objectGridStorage[2];
    f32 cameraGridStorage[2];
    int traceResultStorage[2];

    (void)renderArg2;
    (void)renderArg3;
    (void)renderArg4;
    (void)renderArg5;

    state = obj->extra;
    if (visible == 0) {
        state->delayTimer = 0;
        state->lineOfSightVisible = 0;
    } else if (state->active != 0) {
        state->lineOfSightVisible = 1;
        camera = Camera_GetCurrentViewSlot();
        scratch.cameraDelta[0] = camera->x - obj->anim.localPosX;
        scratch.cameraDelta[1] = camera->y - obj->anim.localPosY;
        scratch.cameraDelta[2] = camera->z - obj->anim.localPosZ;
        distance =
            sqrtf(scratch.cameraDelta[2] * scratch.cameraDelta[2] +
                  (scratch.cameraDelta[0] * scratch.cameraDelta[0] + scratch.cameraDelta[1] * scratch.cameraDelta[1]));
        if (distance > DLL414_LOS_MIN_DISTANCE) {
            inverseDistance = 1.0f / distance;
            normalX = scratch.cameraDelta[0] * inverseDistance;
            scratch.cameraDelta[0] = normalX;
            normalY = scratch.cameraDelta[1] * inverseDistance;
            scratch.cameraDelta[1] = normalY;
            normalZ = scratch.cameraDelta[2] * inverseDistance;
            scratch.cameraDelta[2] = normalZ;
            objectOffsetX = DLL414_LOS_OBJECT_OFFSET * normalX;
            objectTraceStart[0] = objectOffsetX;
            objectOffsetY = DLL414_LOS_OBJECT_OFFSET * normalY;
            objectTraceStart[1] = objectOffsetY;
            objectOffsetZ = DLL414_LOS_OBJECT_OFFSET * normalZ;
            objectTraceStart[2] = objectOffsetZ;
            objectTraceStart[0] = objectOffsetX + obj->anim.localPosX;
            objectTraceStart[1] = objectOffsetY + obj->anim.localPosY;
            objectTraceStart[2] = objectOffsetZ + obj->anim.localPosZ;
            cameraOffsetX = DLL414_LOS_CAMERA_OFFSET * normalX;
            cameraTraceEnd[0] = cameraOffsetX;
            cameraOffsetY = DLL414_LOS_CAMERA_OFFSET * normalY;
            cameraTraceEnd[1] = cameraOffsetY;
            cameraOffsetZ = DLL414_LOS_CAMERA_OFFSET * normalZ;
            cameraTraceEnd[2] = cameraOffsetZ;
            cameraTraceEnd[0] = cameraOffsetX + camera->x;
            cameraTraceEnd[1] = cameraOffsetY + camera->y;
            cameraTraceEnd[2] = cameraOffsetZ + camera->z;
            voxmaps_worldToGrid(objectTraceStart, (s16*)objectGridStorage);
            voxmaps_worldToGrid(cameraTraceEnd, (s16*)cameraGridStorage);
            if (voxmaps_traceLine((VoxPos*)objectGridStorage, (VoxPos*)cameraGridStorage, (VoxPos*)traceResultStorage,
                                  NULL, 0) == 0) {
                state->lineOfSightVisible = 0;
                (*gExpgfxInterface)->freeSource((int)obj);
            }
        }
        if (state->delayTimer > 0) {
            state->delayTimer -= framesThisStep;
        } else {
            if (state->lineOfSightVisible != 0) {
                scratch.particleParams.posX = 0.0f;
                scratch.particleParams.posY = 5.0f;
                scratch.particleParams.posZ = 0.0f;
                (*gPartfxInterface)
                    ->spawnObject(obj, DLL414_IDLE_PARTICLE_ID, &scratch.particleParams, DLL414_IDLE_PARTICLE_MODE, -1,
                                  NULL);
            }
            state->delayTimer = (s16)(randomGetRange(DLL414_IDLE_DELAY_RANDOM_MIN, DLL414_IDLE_DELAY_RANDOM_MAX) +
                                      DLL414_IDLE_DELAY_BASE);
        }
    }
}

void dll414_hitDetect(void) {
}

void dll414_update(GameObject* obj) {
    Dll414State* state;
    Dll69Interface** effectResource;
    Dll414EffectSpawnBuffer effectSpawn;
    Dll69EffectParams effectParams;
    int particleIndex;

    state = obj->extra;
    effectParams = gDll414EffectParamsTemplate;

    ((void (*)(void*, int))Sfx_PlayFromObject)(obj, SFXmn_eggylaugh216);
    objUpdateOpacity(obj);
    if (state->settleTimer > 0) {
        state->settleTimer -= framesThisStep;
    }

    if (state->mode == DLL414_MODE_EGG_INTERACTION) {
        effectSpawn.scale = DLL414_EFFECT_SCALE;
        state->previousActive = state->active;
        if ((ObjHits_GetPriorityHit(obj, NULL, NULL, NULL) != 0) ||
            ((state->settleTimer != 0) && (state->settleTimer <= DLL414_SETTLE_TRIGGER_WINDOW))) {
            state->active = (u8)(1 - state->active);
            if (state->active != 0) {
                state->resetTimer = DLL414_ACTIVE_RESET_TIME;
            }
            if (state->settleTimer != 0) {
                state->settleTimer = 0;
                gDll414SequenceStage = DLL414_SEQUENCE_STAGE_COMPLETE;
                state->resetTimer = DLL414_SEQUENCE_RESET_TIME;
                if (state->sequenceIndex == 2) {
                    mainSetBits(0x1d1, 1);
                }
            }
        }

        if ((state->active != 0) && (state->resetTimer != 0)) {
            state->resetTimer -= framesThisStep;
            if (state->resetTimer <= 0) {
                state->resetTimer = 0;
                state->active = 0;
            }
        }

        if ((state->active != 0) && (state->delayTimer <= 0) && (state->needsOpenSfx != 0)) {
            state->needsOpenSfx = 0;
            ((void (*)(void*, int))Sfx_PlayFromObject)(obj, SFXmn_sml_trex_snap1);
        }

        if (state->active != state->previousActive) {
            if (state->active != 0) {
                effectResource = Resource_Acquire(DLL414_EFFECT_RESOURCE_ID, 1);
                effectParams.param1 = state->sequenceIndex * 2 + DLL414_EFFECT_PARAM1_BASE;
                effectParams.param2 = state->sequenceIndex * 2 + DLL414_EFFECT_PARAM2_BASE;
                (*effectResource)->spawn(obj, 1, effectSpawn.args, DLL414_EFFECT_SPAWN_FLAGS, -1, &effectParams);
                Resource_Release(effectResource);

                particleIndex = 0;
                do {
                    (*gPartfxInterface)->spawnObject(obj, DLL414_ACTIVATION_PARTICLE_ID, NULL, 0, -1, NULL);
                    particleIndex++;
                } while (particleIndex < DLL414_ACTIVATION_PARTICLE_COUNT);

                if ((state->gameBitId != -1) && (mainGetBit(state->gameBitId) == 0)) {
                    mainSetBits(state->gameBitId, 1);
                }
                if ((gDll414SequenceStage == DLL414_SEQUENCE_STAGE_NONE) && (state->sequenceIndex == 0) &&
                    (mainGetBit(state->gameBitId) != 0)) {
                    gDll414SequenceStage = DLL414_SEQUENCE_STAGE_FIRST;
                }
                if ((gDll414SequenceStage == DLL414_SEQUENCE_STAGE_FIRST) && (state->sequenceIndex == 1) &&
                    (mainGetBit(state->gameBitId) != 0)) {
                    gDll414SequenceStage = DLL414_SEQUENCE_STAGE_SECOND;
                }
                if ((gDll414SequenceStage == DLL414_SEQUENCE_STAGE_SECOND) && (state->sequenceIndex == 2) &&
                    (mainGetBit(state->gameBitId) != 0)) {
                    mainSetBits(0x1d1, 1);
                    gDll414SequenceStage = DLL414_SEQUENCE_STAGE_COMPLETE;
                }
                state->needsOpenSfx = 1;
                state->delayTimer = 1;
            } else {
                Sfx_StopObjectChannel((int)obj, DLL414_EFFECT_SFX_CHANNEL);
                (*gModgfxInterface)->detachSource(obj);
                (*gExpgfxInterface)->freeSource((u32)obj);
                if ((state->gameBitId != -1) && (mainGetBit(state->gameBitId) != 0)) {
                    mainSetBits(state->gameBitId, 0);
                }
                if ((gDll414SequenceStage == DLL414_SEQUENCE_STAGE_FIRST) && (state->sequenceIndex == 0)) {
                    gDll414SequenceStage = DLL414_SEQUENCE_STAGE_NONE;
                }
                if ((gDll414SequenceStage == DLL414_SEQUENCE_STAGE_SECOND) && (state->sequenceIndex == 1)) {
                    gDll414SequenceStage = DLL414_SEQUENCE_STAGE_NONE;
                }
                if ((gDll414SequenceStage == DLL414_SEQUENCE_STAGE_COMPLETE) && (state->sequenceIndex == 2) &&
                    (mainGetBit(0x1d5) == 0)) {
                    mainSetBits(0x1d1, 0);
                    gDll414SequenceStage = DLL414_SEQUENCE_STAGE_NONE;
                }
            }
        }
    }
}

void dll414_init(GameObject* obj, const Dll414Placement* placement) {
    Dll414State* state;
    Dll69Interface** effectResource;
    Dll414EffectSpawnBuffer effectSpawn;

    state = obj->extra;
    obj->anim.rotX = (s16)(((s32)placement->rotationIndex & DLL414_ROTATION_INDEX_MASK) << DLL414_ROTATION_INDEX_SHIFT);
    if (placement->scalePacked > 0) {
        obj->anim.rootMotionScale = placement->scalePacked / DLL414_PACKED_SCALE_DIVISOR;
    } else {
        obj->anim.rootMotionScale = DLL414_DEFAULT_SCALE;
    }

    state->mode = placement->mode;
    state->active = 0;
    state->sequenceIndex = 0;
    state->gameBitId = placement->gameBitId;
    effectSpawn.scale = DLL414_EFFECT_SCALE;

    switch (state->mode) {
    case DLL414_MODE_SPARKLE:
        state->active = 1;
        effectResource = Resource_Acquire(DLL414_EFFECT_RESOURCE_ID, 1);
        if (placement->sequenceIndex == 0) {
            (*effectResource)->spawn(obj, 0, effectSpawn.args, DLL414_EFFECT_SPAWN_FLAGS, -1, NULL);
        }
        break;
    case DLL414_MODE_EGG_INTERACTION:
        state->sequenceIndex = placement->sequenceIndex;
        state->needsOpenSfx = 0;
        state->settleTimer = state->sequenceIndex * DLL414_SEQUENCE_SETTLE_STEP + DLL414_SEQUENCE_SETTLE_BASE;
        state->previousActive = 0;
        break;
    }
    state->delayTimer = 0;
}

void dll414_release(void) {
}

void dll414_initialise(void) {
}

ObjectDescriptor gDll414ObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)dll414_initialise,
    (ObjectDescriptorCallback)dll414_release,
    NULL,
    (ObjectDescriptorCallback)dll414_init,
    (ObjectDescriptorCallback)dll414_update,
    (ObjectDescriptorCallback)dll414_hitDetect,
    (ObjectDescriptorCallback)dll414_render,
    (ObjectDescriptorCallback)dll414_free,
    (ObjectDescriptorCallback)dll414_getObjectTypeId,
    dll414_getExtraSize,
};
