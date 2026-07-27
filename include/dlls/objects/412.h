#ifndef DLLS_OBJECTS_412_H_
#define DLLS_OBJECTS_412_H_

#include "dlls/object_descriptor.h"
#include "game/objects/object_fwd.h"
#include "game/objects/object_setup.h"

/* Only the accessed placement prefix is recovered; the complete retail width is not established. */
typedef struct Dll412Placement {
    ObjPlacement base;
    u8 unknown18[0x1E - 0x18];
    s8 initialYaw;
    s8 disableChildSpawn;
} Dll412Placement;

typedef struct Dll412State {
    s32 unknown0;
    s16 spawnTimer;
    s16 spawnTimerRate;
} Dll412State;

STATIC_ASSERT(offsetof(Dll412Placement, base) == 0x00);
STATIC_ASSERT(offsetof(Dll412Placement, unknown18) == 0x18);
STATIC_ASSERT(offsetof(Dll412Placement, initialYaw) == 0x1E);
STATIC_ASSERT(offsetof(Dll412Placement, disableChildSpawn) == 0x1F);

STATIC_ASSERT(sizeof(Dll412State) == 0x08);
STATIC_ASSERT(offsetof(Dll412State, unknown0) == 0x00);
STATIC_ASSERT(offsetof(Dll412State, spawnTimer) == 0x04);
STATIC_ASSERT(offsetof(Dll412State, spawnTimerRate) == 0x06);

extern ObjectDescriptor gDll412ObjDescriptor;

int dll412_getExtraSize(void);
int dll412_getObjectTypeId(void);
void dll412_free(void);
void dll412_render(GameObject* obj, int renderArg2, int renderArg3, int renderArg4, int renderArg5, s8 visible);
void dll412_hitDetect(void);
void dll412_update(GameObject* obj);
void dll412_init(GameObject* obj, const Dll412Placement* placement);
void dll412_release(void);
void dll412_initialise(void);

#endif /* DLLS_OBJECTS_412_H_ */
