#ifndef DLLS_OBJECTS_409_H_
#define DLLS_OBJECTS_409_H_

#include "dlls/object_descriptor.h"
#include "game/objects/object_fwd.h"
#include "game/objects/object_setup.h"
#include "main/objanim_update.h"

#define DLL409_STATE_SIZE 0x14

/* Only the accessed placement prefix is recovered; the complete retail width is not established. */
typedef struct Dll409Placement {
    ObjPlacement base;
    u8 unknown18[2];
    s16 activationDistancePacked;
} Dll409Placement;

typedef struct Dll409State {
    s16 activationDistance;
    s16 actionTimer;
    s16 channel2Brightness;
    s16 channel2BrightnessDelta;
    s16 channel3Brightness;
    s16 channel3BrightnessDelta;
    s16 gfxHandle;
    u8 unlockCount;
    u8 phase;
    u8 sequenceEventState;
    u8 unknown11;
    u8 timerActionTriggered;
    u8 unknown13;
} Dll409State;

STATIC_ASSERT(offsetof(Dll409Placement, base) == 0x00);
STATIC_ASSERT(offsetof(Dll409Placement, unknown18) == 0x18);
STATIC_ASSERT(offsetof(Dll409Placement, activationDistancePacked) == 0x1A);

STATIC_ASSERT(sizeof(Dll409State) == DLL409_STATE_SIZE);
STATIC_ASSERT(offsetof(Dll409State, activationDistance) == 0x00);
STATIC_ASSERT(offsetof(Dll409State, actionTimer) == 0x02);
STATIC_ASSERT(offsetof(Dll409State, channel2Brightness) == 0x04);
STATIC_ASSERT(offsetof(Dll409State, channel2BrightnessDelta) == 0x06);
STATIC_ASSERT(offsetof(Dll409State, channel3Brightness) == 0x08);
STATIC_ASSERT(offsetof(Dll409State, channel3BrightnessDelta) == 0x0A);
STATIC_ASSERT(offsetof(Dll409State, gfxHandle) == 0x0C);
STATIC_ASSERT(offsetof(Dll409State, unlockCount) == 0x0E);
STATIC_ASSERT(offsetof(Dll409State, phase) == 0x0F);
STATIC_ASSERT(offsetof(Dll409State, sequenceEventState) == 0x10);
STATIC_ASSERT(offsetof(Dll409State, unknown11) == 0x11);
STATIC_ASSERT(offsetof(Dll409State, timerActionTriggered) == 0x12);
STATIC_ASSERT(offsetof(Dll409State, unknown13) == 0x13);

extern u32 gDll409Event9Result;
extern ObjectDescriptor gDll409ObjDescriptor;

int dll409_processAnimEvents(GameObject* obj, int unused, ObjAnimUpdateState* animUpdate);
int dll409_getExtraSize(void);
int dll409_getObjectTypeId(void);
void dll409_free(GameObject* obj);
void dll409_render(GameObject* obj, int renderArg2, int renderArg3, int renderArg4, int renderArg5, s8 visible);
void dll409_hitDetect(void);
void dll409_update(GameObject* obj);
void dll409_init(GameObject* obj, const Dll409Placement* placement);
void dll409_release(void);
void dll409_initialise(void);

#endif /* DLLS_OBJECTS_409_H_ */
