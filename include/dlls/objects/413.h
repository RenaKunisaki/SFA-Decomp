#ifndef DLLS_OBJECTS_413_H_
#define DLLS_OBJECTS_413_H_

#include "dlls/object_descriptor.h"
#include "game/objects/object_fwd.h"
#include "game/objects/object_setup.h"

typedef struct Dll413Placement {
    ObjPlacement base;
    u8 unknown18;
    s8 variant;
} Dll413Placement;

typedef struct Dll413State {
    u8 unknown00[0x08];
    f32 positionX;
    f32 positionY;
    f32 positionZ;
    u8 unknown14[0x2C - 0x14];
    s16 angularVelocityZ;
    s16 angularVelocityX;
    s16 effectTimer;
    s16 despawnTimer;
    u16 spinPhase;
    u8 flags;
    u8 unknown37;
} Dll413State;

STATIC_ASSERT(offsetof(Dll413Placement, base) == 0x00);
STATIC_ASSERT(offsetof(Dll413Placement, unknown18) == 0x18);
STATIC_ASSERT(offsetof(Dll413Placement, variant) == 0x19);

STATIC_ASSERT(sizeof(Dll413State) == 0x38);
STATIC_ASSERT(offsetof(Dll413State, unknown00) == 0x00);
STATIC_ASSERT(offsetof(Dll413State, positionX) == 0x08);
STATIC_ASSERT(offsetof(Dll413State, positionY) == 0x0C);
STATIC_ASSERT(offsetof(Dll413State, positionZ) == 0x10);
STATIC_ASSERT(offsetof(Dll413State, unknown14) == 0x14);
STATIC_ASSERT(offsetof(Dll413State, angularVelocityZ) == 0x2C);
STATIC_ASSERT(offsetof(Dll413State, angularVelocityX) == 0x2E);
STATIC_ASSERT(offsetof(Dll413State, effectTimer) == 0x30);
STATIC_ASSERT(offsetof(Dll413State, despawnTimer) == 0x32);
STATIC_ASSERT(offsetof(Dll413State, spinPhase) == 0x34);
STATIC_ASSERT(offsetof(Dll413State, flags) == 0x36);
STATIC_ASSERT(offsetof(Dll413State, unknown37) == 0x37);

extern ObjectDescriptor gDll413ObjDescriptor;

int dll413_getExtraSize(void);
int dll413_getObjectTypeId(void);
void dll413_free(GameObject* obj);
void dll413_render(void);
void dll413_hitDetect(GameObject* obj);
void dll413_update(GameObject* obj);
void dll413_init(GameObject* obj);
void dll413_release(void);
void dll413_initialise(void);

#endif /* DLLS_OBJECTS_413_H_ */
