/*
 * Procedural water-surface animator. All instances share the generated
 * height, color, and phase tables.
 */
#include "dlls/objects/310_WaveAnimato.h"

#include "dolphin/MSL_C/PPCEABI/bare/H/math_trig_api.h"
#include "game/objects/object.h"
#include "main/frame_timing.h"
#include "main/mm.h"
#include "main/obj_group.h"
#include "main/object_render.h"

typedef struct WaveAnimatorColor {
    u8 red;
    u8 green;
    u8 blue;
} WaveAnimatorColor;

STATIC_ASSERT(sizeof(WaveAnimatorColor) == 3);

#define WAVE_ANIMATOR_OBJECT_GROUP 27
#define WAVE_ANIMATOR_RENDER_SCALE 1.0f

u8 gWaveAnimatorPhaseUpdateLatch;
f32* gWaveAnimatorHeightTable;
s16* gWaveAnimatorPhaseTable;
WaveAnimatorColor* gWaveAnimatorColorTable;
u8 gWaveAnimatorInstanceCount;

void WaveAnimator_modelMtxFn(GameObject* obj, int arg0, int arg1, int arg2) {
    WaveAnimatorState* state = obj->extra;
    u32 newFlags;

    newFlags = (u32)state->flags | WAVE_ANIMATOR_STATE_MODEL_MTX_PENDING;
    state->flags = newFlags;
    state->modelMtxArg0 = arg0;
    state->modelMtxArg1 = arg1;
    state->modelMtxArg2 = arg2;
}

void WaveAnimator_func0B(GameObject* obj) {
    WaveAnimatorState* state = obj->extra;

    state->flags |= WAVE_ANIMATOR_STATE_FUNC_0B_LATCH;
}

void WaveAnimator_setScale(GameObject* obj, f32 scale) {
    WaveAnimatorState* state = obj->extra;

    state->flags |= WAVE_ANIMATOR_STATE_SCALE_PENDING;
    state->scaleB = scale;
}

void WaveAnimator_buildSharedTables(WaveAnimatorState* config) {
    int row;
    int heightIdx;
    int i;
    int j;
    int x;
    int stepX;
    int y;
    int phaseIdx;
    int stepY;
    f32 waveY;
    f32 initHeight;

    gWaveAnimatorHeightTable = mmAlloc(sizeof(f32) * config->period * config->period, 0xFFFFFF, 0);
    gWaveAnimatorColorTable = mmAlloc(sizeof(WaveAnimatorColor) * config->period * config->period, 0xFFFFFF, 0);

    x = config->originX;
    stepX = (s32)((65536.0f * config->spanX) / config->period);
    y = config->originY;
    stepY = (s32)((65536.0f * config->spanY) / config->period);

    initHeight = 0.0f;
    config->maxHeight = initHeight;
    config->minHeight = initHeight;

    i = 0;
    heightIdx = 0;
    for (; i < config->period; i++) {
        f32 xv;
        j = 0;
        row = heightIdx;
        xv = 3.1415927f * x;
        for (; j < config->period; j++) {
            f32 s1 = mathSinf((3.1415927f * y) / 32768.0f);
            f32 s2;
            waveY = config->ampY * s1;
            s2 = mathSinf(xv / 32768.0f);
            *(f32*)((u8*)gWaveAnimatorHeightTable + row) = config->ampX * s2 + waveY;
            if (*(f32*)((u8*)gWaveAnimatorHeightTable + row) < config->minHeight) {
                config->minHeight = *(f32*)((u8*)gWaveAnimatorHeightTable + row);
            }
            if (*(f32*)((u8*)gWaveAnimatorHeightTable + row) > config->maxHeight) {
                config->maxHeight = *(f32*)((u8*)gWaveAnimatorHeightTable + row);
            }
            y += stepY;
            row += 4;
            heightIdx += 4;
        }
        x += stepX;
    }

    {
        f32 colorSplitZero;
        f32 t;
        f32 negMin = -config->minHeight;
        heightIdx = 0;
        x = heightIdx;
        i = heightIdx;
        colorSplitZero = 0.0f;
        for (; heightIdx < config->period; heightIdx++) {
            int src[1];
            int byte[1];
            for (j = 0, src[0] = x, byte[0] = i; j < config->period; src[0] += 4, byte[0] += 3, x += 4, i += 3, j++) {
                f32 v = *(f32*)((u8*)gWaveAnimatorHeightTable + src[0]);
                if (v < colorSplitZero) {
                    t = (v - config->minHeight) / negMin;
                    *(u8*)((u8*)gWaveAnimatorColorTable + byte[0]) = 65.0f * t + 190.0f;
                    *(u8*)((u8*)gWaveAnimatorColorTable + byte[0] + 1) = 165.0f * t + 90.0f;
                    *(u8*)((u8*)gWaveAnimatorColorTable + byte[0] + 2) = 235.0f * t + 20.0f;
                } else {
                    *(u8*)((u8*)gWaveAnimatorColorTable + byte[0]) = 255;
                    *(u8*)((u8*)gWaveAnimatorColorTable + byte[0] + 1) = 255;
                    *(u8*)((u8*)gWaveAnimatorColorTable + byte[0] + 2) = 255;
                }
            }
        }
    }

    gWaveAnimatorPhaseTable = mmAlloc(2 * sizeof(s16) * config->gridN * config->gridN, 0xFFFFFF, 0);
    phaseIdx = 0;
    for (i = 0; i < config->gridN; i++) {
        for (j = 0; j < config->gridN; j++) {
            gWaveAnimatorPhaseTable[phaseIdx] = (s16)(i * 10);
            gWaveAnimatorPhaseTable[phaseIdx + 1] = (s16)(j * 10);
            phaseIdx += 2;
        }
    }
}

int WaveAnimator_getExtraSize(void) {
    return sizeof(WaveAnimatorState);
}

int WaveAnimator_getObjectTypeId(void) {
    return 0;
}

void WaveAnimator_free(GameObject* obj) {
    if (--gWaveAnimatorInstanceCount == 0) {
        if (gWaveAnimatorHeightTable != NULL) {
            mm_free(gWaveAnimatorHeightTable);
        }
        if (gWaveAnimatorPhaseTable != NULL) {
            mm_free(gWaveAnimatorPhaseTable);
        }
        if (gWaveAnimatorColorTable != NULL) {
            mm_free(gWaveAnimatorColorTable);
        }
    }
    ObjGroup_RemoveObject((int)obj, WAVE_ANIMATOR_OBJECT_GROUP);
}

void WaveAnimator_render(GameObject* obj, int renderArg2, int renderArg3, int renderArg4, int renderArg5, s8 visible) {
    s32 visibility = visible;
    if (visibility != 0) {
        objRenderModelAndHitVolumes(obj, renderArg2, renderArg3, renderArg4, renderArg5, WAVE_ANIMATOR_RENDER_SCALE);
    }
}

void WaveAnimator_hitDetect(GameObject* obj) {
    int i;
    int j;
    int phaseIdx;
    WaveAnimatorState* state;

    if (gWaveAnimatorPhaseUpdateLatch != 0) {
        return;
    }
    state = obj->extra;
    phaseIdx = 0;
    for (i = 0; i < state->gridN; i++) {
        for (j = 0; j < state->gridN; j++) {
            gWaveAnimatorPhaseTable[phaseIdx] += framesThisStep >> 1;
            while (gWaveAnimatorPhaseTable[phaseIdx] >= state->period) {
                gWaveAnimatorPhaseTable[phaseIdx] -= state->period;
            }
            gWaveAnimatorPhaseTable[phaseIdx + 1] += framesThisStep >> 1;
            while (gWaveAnimatorPhaseTable[phaseIdx + 1] >= state->period) {
                gWaveAnimatorPhaseTable[phaseIdx + 1] -= state->period;
            }
            phaseIdx += 2;
        }
    }
    gWaveAnimatorPhaseUpdateLatch = 1;
}

void WaveAnimator_update(void) {
}

void WaveAnimator_init(GameObject* obj, WaveAnimatorPlacement* placement) {
    WaveAnimatorState* state = obj->extra;
    f32 scale;

    state->sinkDepthScale = placement->sinkDepthScale;
    state->originX = placement->originX;
    state->originY = placement->originY;
    state->spanX = placement->spanX;
    state->spanY = placement->spanY;
    state->ampX = placement->ampX;
    state->ampY = placement->ampY;
    state->period = placement->period;
    state->gridN = placement->gridN;
    scale = (1.0f);
    state->scaleA = scale;
    state->scaleB = scale;
    if (gWaveAnimatorInstanceCount == 0) {
        WaveAnimator_buildSharedTables(state);
    }
    ObjGroup_AddObject((int)obj, WAVE_ANIMATOR_OBJECT_GROUP);
    gWaveAnimatorInstanceCount++;
}

void WaveAnimator_release(void) {
}

void WaveAnimator_initialise(void) {
}

ObjectDescriptor14 gWaveAnimatorObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_13_SLOTS,
    (ObjectDescriptorCallback)WaveAnimator_initialise,
    (ObjectDescriptorCallback)WaveAnimator_release,
    0,
    (ObjectDescriptorCallback)WaveAnimator_init,
    (ObjectDescriptorCallback)WaveAnimator_update,
    (ObjectDescriptorCallback)WaveAnimator_hitDetect,
    (ObjectDescriptorCallback)WaveAnimator_render,
    (ObjectDescriptorCallback)WaveAnimator_free,
    (ObjectDescriptorCallback)WaveAnimator_getObjectTypeId,
    (ObjectDescriptorCallback)WaveAnimator_getExtraSize,
    (ObjectDescriptorCallback)WaveAnimator_setScale,
    (ObjectDescriptorCallback)WaveAnimator_func0B,
    (ObjectDescriptorCallback)WaveAnimator_modelMtxFn,
    0,
};
