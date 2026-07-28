#ifndef DLLS_OBJECTS_312_GROUNDANIMA_H_
#define DLLS_OBJECTS_312_GROUNDANIMA_H_

#include "dlls/object_descriptor.h"
#include "game/objects/object_fwd.h"
#include "game/objects/object_setup.h"

typedef enum GroundAnimatorStateFlag {
    GROUND_ANIMATOR_STATE_ON_MAP = 0x01,
    GROUND_ANIMATOR_STATE_COMPLETE = 0x02,
    GROUND_ANIMATOR_STATE_PRESSED = 0x04,
} GroundAnimatorStateFlag;

typedef struct GroundAnimatorPlacement {
    ObjPlacement base;  /* 0x00 */
    s16 sunkGameBit;    /* 0x18 */
    s16 enableGameBit;  /* 0x1A */
    u8 pad1C[2];        /* 0x1C */
    s16 modelVariant;   /* 0x1E */
    u8 maxSinkDepth;    /* 0x20: multiplied by 100 */
    u8 sfxIndex;        /* 0x21 */
    u8 disableAutoLink; /* 0x22 */
    u8 pad23[2];        /* 0x23 */
    u8 blockId;         /* 0x25 */
    u8 radius;          /* 0x26 */
    u8 yOffset;         /* 0x27 */
} GroundAnimatorPlacement;

/* GroundAnimator_getExtraSize proves the complete 0x30-byte allocation. */
typedef struct GroundAnimatorState {
    f32* falloffBuffer;       /* 0x00: per-vertex weights */
    s16* baseHeightBuffer;    /* 0x04: per-vertex base heights */
    GameObject* linkedObject; /* 0x08: nearest group-4 object */
    f32 sinkDepth;            /* 0x0C */
    f32 previousSinkDepth;    /* 0x10 */
    f32 radius;               /* 0x14 */
    f32 yOffset;              /* 0x18 */
    s16 blockEntries[6];      /* 0x1C: matching polygon-group indices */
    s16 vertexCount;          /* 0x28 */
    u8 entryCount;            /* 0x2A */
    u8 modelVariant;          /* 0x2B */
    u8 dirtyFrames;           /* 0x2C */
    u8 flags;                 /* 0x2D: GroundAnimatorStateFlag */
    u8 pad2E[2];              /* 0x2E */
} GroundAnimatorState;

STATIC_ASSERT(offsetof(GroundAnimatorPlacement, base) == 0x00);
STATIC_ASSERT(offsetof(GroundAnimatorPlacement, sunkGameBit) == 0x18);
STATIC_ASSERT(offsetof(GroundAnimatorPlacement, enableGameBit) == 0x1A);
STATIC_ASSERT(offsetof(GroundAnimatorPlacement, pad1C) == 0x1C);
STATIC_ASSERT(offsetof(GroundAnimatorPlacement, modelVariant) == 0x1E);
STATIC_ASSERT(offsetof(GroundAnimatorPlacement, maxSinkDepth) == 0x20);
STATIC_ASSERT(offsetof(GroundAnimatorPlacement, sfxIndex) == 0x21);
STATIC_ASSERT(offsetof(GroundAnimatorPlacement, disableAutoLink) == 0x22);
STATIC_ASSERT(offsetof(GroundAnimatorPlacement, pad23) == 0x23);
STATIC_ASSERT(offsetof(GroundAnimatorPlacement, blockId) == 0x25);
STATIC_ASSERT(offsetof(GroundAnimatorPlacement, radius) == 0x26);
STATIC_ASSERT(offsetof(GroundAnimatorPlacement, yOffset) == 0x27);
STATIC_ASSERT(sizeof(GroundAnimatorPlacement) == 0x28);

STATIC_ASSERT(offsetof(GroundAnimatorState, falloffBuffer) == 0x00);
STATIC_ASSERT(offsetof(GroundAnimatorState, baseHeightBuffer) == 0x04);
STATIC_ASSERT(offsetof(GroundAnimatorState, linkedObject) == 0x08);
STATIC_ASSERT(offsetof(GroundAnimatorState, sinkDepth) == 0x0C);
STATIC_ASSERT(offsetof(GroundAnimatorState, previousSinkDepth) == 0x10);
STATIC_ASSERT(offsetof(GroundAnimatorState, radius) == 0x14);
STATIC_ASSERT(offsetof(GroundAnimatorState, yOffset) == 0x18);
STATIC_ASSERT(offsetof(GroundAnimatorState, blockEntries) == 0x1C);
STATIC_ASSERT(offsetof(GroundAnimatorState, vertexCount) == 0x28);
STATIC_ASSERT(offsetof(GroundAnimatorState, entryCount) == 0x2A);
STATIC_ASSERT(offsetof(GroundAnimatorState, modelVariant) == 0x2B);
STATIC_ASSERT(offsetof(GroundAnimatorState, dirtyFrames) == 0x2C);
STATIC_ASSERT(offsetof(GroundAnimatorState, flags) == 0x2D);
STATIC_ASSERT(offsetof(GroundAnimatorState, pad2E) == 0x2E);
STATIC_ASSERT(sizeof(GroundAnimatorState) == 0x30);

u8 GroundAnimator_modelMtxFn(GameObject* obj);
u8 GroundAnimator_isFullySunk(GameObject* obj);
f32 GroundAnimator_applyPress(GameObject* obj, GameObject* target);
void GroundAnimator_gatherVertices(GameObject* obj, GroundAnimatorState* state, GroundAnimatorPlacement* placement);
int GroundAnimator_getExtraSize(void);
void GroundAnimator_free(GameObject* obj, int flags);
void GroundAnimator_render(GameObject* obj, int renderArg2, int renderArg3, int renderArg4, int renderArg5, s8 visible);
void GroundAnimator_update(GameObject* obj);
void GroundAnimator_init(GameObject* obj, GroundAnimatorPlacement* placement);

extern ObjectDescriptor14 gGroundAnimatorObjDescriptor;

#endif /* DLLS_OBJECTS_312_GROUNDANIMA_H_ */
