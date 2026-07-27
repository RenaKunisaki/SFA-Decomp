#ifndef DLLS_OBJECTS_410_H_
#define DLLS_OBJECTS_410_H_

#include "dlls/object_descriptor.h"
#include "game/objects/object_fwd.h"
#include "game/objects/object_setup.h"

typedef struct Dll410Placement {
    ObjPlacement base;
    u8 unknown18[0x1E - 0x18];
    s8 initialYaw;
    s8 triggerGameBitOffset;
} Dll410Placement;

typedef struct Dll410State {
    s16 spawnTimer;
    s16 spawnTimerRate;
} Dll410State;

STATIC_ASSERT(offsetof(Dll410Placement, base) == 0x00);
STATIC_ASSERT(offsetof(Dll410Placement, unknown18) == 0x18);
STATIC_ASSERT(offsetof(Dll410Placement, initialYaw) == 0x1E);
STATIC_ASSERT(offsetof(Dll410Placement, triggerGameBitOffset) == 0x1F);

STATIC_ASSERT(sizeof(Dll410State) == 0x04);
STATIC_ASSERT(offsetof(Dll410State, spawnTimer) == 0x00);
STATIC_ASSERT(offsetof(Dll410State, spawnTimerRate) == 0x02);

extern ObjectDescriptor gDll410ObjDescriptor;

int dll410_getExtraSize(void);
int dll410_getObjectTypeId(void);
void dll410_free(void);
void dll410_render(GameObject* obj, int renderArg2, int renderArg3, int renderArg4, int renderArg5, s8 visible);
void dll410_hitDetect(void);
void dll410_update(GameObject* obj);
void dll410_init(GameObject* obj, const Dll410Placement* placement);
void dll410_release(void);
void dll410_initialise(void);

#endif /* DLLS_OBJECTS_410_H_ */
