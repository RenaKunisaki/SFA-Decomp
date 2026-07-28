#ifndef DLLS_OBJECTS_293_CURVE_H_
#define DLLS_OBJECTS_293_CURVE_H_

#include "dlls/object_descriptor.h"
#include "game/objects/object_fwd.h"
#include "main/dll/dll_0015_curves.h"

/* Only the accessed placement prefix is recovered; the complete retail width is not established. */
typedef struct CurvePlacement {
    RomCurveDef curve; /* 0x00 */
    s8 yaw;            /* 0x2C: expanded to anim.rotX */
    s8 pitch;          /* 0x2D: expanded to anim.rotY */
    u8 pad2E[0xA];     /* 0x2E */
    s16 roll;          /* 0x38: used by curve types 0x08 and 0x1A */
} CurvePlacement;

STATIC_ASSERT(offsetof(CurvePlacement, curve) == 0x0);
STATIC_ASSERT(offsetof(CurvePlacement, yaw) == 0x2C);
STATIC_ASSERT(offsetof(CurvePlacement, pitch) == 0x2D);
STATIC_ASSERT(offsetof(CurvePlacement, pad2E) == 0x2E);
STATIC_ASSERT(offsetof(CurvePlacement, roll) == 0x38);

int curve_func0B(void);
void curve_func0A(void);
int curve_getExtraSize(void);
int curve_getObjectTypeId(void);
void curve_free(void);
void curve_render(GameObject* obj, int renderArg2, int renderArg3, int renderArg4, int renderArg5, s8 visible);
void curve_init(GameObject* obj, CurvePlacement* placement);

extern ObjectDescriptor12 gCurveObjDescriptor;

#endif /* DLLS_OBJECTS_293_CURVE_H_ */
