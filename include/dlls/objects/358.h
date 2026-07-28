#ifndef DLLS_OBJECTS_358_H_
#define DLLS_OBJECTS_358_H_

#include "dlls/object_descriptor.h"
#include "game/objects/object.h"
#include "game/objects/object_setup.h"
#include "main/vec_types.h"

typedef enum ExplodedPhase {
    EXPLODED_PHASE_IDLE = 0,
    EXPLODED_PHASE_ACTIVE = 1,
    EXPLODED_PHASE_EXPIRED = 2,
} ExplodedPhase;

typedef struct ExplodedPlacement {
    ObjPlacement base;
    u8 modelBankIndex;
    u8 pad19;
    Vec3s initialRotation;
    Vec3s initialVelocity;
    Vec3s acceleration;
    Vec3s spin;
    Vec3s spinVelocity;
    u16 lifetimeFrames;
    union {
        s16 floorOffset;
        u16 floorOffsetRaw;
    };
    u8 pad3C;
    s8 scaleByte;
    u8 pad3E[0x06];
} ExplodedPlacement;

typedef struct ExplodedState {
    Vec3f localCenter;
    Vec3f initialLocalCenter;
    Vec3f spin;
    Vec3f spinVelocity;
    Vec3f acceleration;
    u8 pad3C[0x18];
    f32 floorHeight;
    s32 elapsedFrames;
    s32 durationFrames;
    u8 pad60[0x06];
    u8 physicsFlags;
    u8 unknown67;
    u8 pad68;
    u8 phase; /* ExplodedPhase */
    u8 pad6A[0x02];
} ExplodedState;

typedef struct ExplodedObject {
    ObjAnimComponent anim;
    u8 padB0[0x08];
    ExplodedState* state;
} ExplodedObject;

STATIC_ASSERT(offsetof(ExplodedPlacement, base) == 0x00);
STATIC_ASSERT(offsetof(ExplodedPlacement, modelBankIndex) == 0x18);
STATIC_ASSERT(offsetof(ExplodedPlacement, initialRotation) == 0x1A);
STATIC_ASSERT(offsetof(ExplodedPlacement, initialVelocity) == 0x20);
STATIC_ASSERT(offsetof(ExplodedPlacement, acceleration) == 0x26);
STATIC_ASSERT(offsetof(ExplodedPlacement, spin) == 0x2C);
STATIC_ASSERT(offsetof(ExplodedPlacement, spinVelocity) == 0x32);
STATIC_ASSERT(offsetof(ExplodedPlacement, lifetimeFrames) == 0x38);
STATIC_ASSERT(offsetof(ExplodedPlacement, floorOffset) == 0x3A);
STATIC_ASSERT(offsetof(ExplodedPlacement, floorOffsetRaw) == 0x3A);
STATIC_ASSERT(offsetof(ExplodedPlacement, scaleByte) == 0x3D);
STATIC_ASSERT(offsetof(ExplodedPlacement, pad3E) == 0x3E);
STATIC_ASSERT(sizeof(ExplodedPlacement) == 0x44);

STATIC_ASSERT(offsetof(ExplodedState, localCenter) == 0x00);
STATIC_ASSERT(offsetof(ExplodedState, initialLocalCenter) == 0x0C);
STATIC_ASSERT(offsetof(ExplodedState, spin) == 0x18);
STATIC_ASSERT(offsetof(ExplodedState, spinVelocity) == 0x24);
STATIC_ASSERT(offsetof(ExplodedState, acceleration) == 0x30);
STATIC_ASSERT(offsetof(ExplodedState, pad3C) == 0x3C);
STATIC_ASSERT(offsetof(ExplodedState, floorHeight) == 0x54);
STATIC_ASSERT(offsetof(ExplodedState, elapsedFrames) == 0x58);
STATIC_ASSERT(offsetof(ExplodedState, durationFrames) == 0x5C);
STATIC_ASSERT(offsetof(ExplodedState, pad60) == 0x60);
STATIC_ASSERT(offsetof(ExplodedState, physicsFlags) == 0x66);
STATIC_ASSERT(offsetof(ExplodedState, unknown67) == 0x67);
STATIC_ASSERT(offsetof(ExplodedState, pad68) == 0x68);
STATIC_ASSERT(offsetof(ExplodedState, phase) == 0x69);
STATIC_ASSERT(offsetof(ExplodedState, pad6A) == 0x6A);
STATIC_ASSERT(sizeof(ExplodedState) == 0x6C);

STATIC_ASSERT(offsetof(ExplodedObject, anim) == 0x00);
STATIC_ASSERT(offsetof(ExplodedObject, padB0) == 0xB0);
STATIC_ASSERT(offsetof(ExplodedObject, state) == 0xB8);

void exploded_initDebrisState(ExplodedObject* obj, ExplodedPlacement* placement, int usePresetCenter,
                              ExplodedState* state);
void exploded_seedDebrisMotion(ExplodedObject* obj, ExplodedState* state, ExplodedPlacement* placement);
u8 exploded_getPhase(ExplodedObject* obj);
int exploded_getExtraSize(void);
u32 exploded_getObjectTypeId(ExplodedObject* obj);
void exploded_free(void);
void exploded_render(GameObject* obj, int renderArg2, int renderArg3, int renderArg4, int renderArg5, s8 visible);
void exploded_hitDetect(void);
int exploded_stepDebrisPhysics(ExplodedObject* obj, ExplodedState* state);
void exploded_update(ExplodedObject* obj);
void exploded_init(ExplodedObject* obj, ExplodedPlacement* placement, int usePresetCenter);
void exploded_release(void);
void exploded_initialise(void);

extern ObjectDescriptor16 gExplodedObjDescriptor;

#endif /* DLLS_OBJECTS_358_H_ */
