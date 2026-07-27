#ifndef DLLS_OBJECTS_407_H_
#define DLLS_OBJECTS_407_H_

#include "dlls/object_descriptor.h"
#include "game/objects/object_fwd.h"
#include "game/objects/object_setup.h"
#include "main/dll/dll_0069_dll69func0.h"

#define DLL407_STATE_SIZE 0x10

/* Only the accessed placement prefix is recovered; the complete retail width is not established. */
typedef struct Dll407Placement {
    ObjPlacement base;
    s8 rotationParam;
    u8 mode;
    s16 scale;
    s16 stage;
    s16 gameBit;
} Dll407Placement;

typedef struct Dll407State {
    s32 gameBit;
    s16 sparkTimer;
    s16 activeTimer;
    s16 hitCooldown;
    u8 visibleToCamera;
    u8 mode;
    u8 active;
    u8 sparkArmed;
    u8 previousActive;
    u8 stage;
} Dll407State;

STATIC_ASSERT(offsetof(Dll407Placement, base) == 0x00);
STATIC_ASSERT(offsetof(Dll407Placement, rotationParam) == 0x18);
STATIC_ASSERT(offsetof(Dll407Placement, mode) == 0x19);
STATIC_ASSERT(offsetof(Dll407Placement, scale) == 0x1A);
STATIC_ASSERT(offsetof(Dll407Placement, stage) == 0x1C);
STATIC_ASSERT(offsetof(Dll407Placement, gameBit) == 0x1E);

STATIC_ASSERT(sizeof(Dll407State) == DLL407_STATE_SIZE);
STATIC_ASSERT(offsetof(Dll407State, gameBit) == 0x00);
STATIC_ASSERT(offsetof(Dll407State, sparkTimer) == 0x04);
STATIC_ASSERT(offsetof(Dll407State, activeTimer) == 0x06);
STATIC_ASSERT(offsetof(Dll407State, hitCooldown) == 0x08);
STATIC_ASSERT(offsetof(Dll407State, visibleToCamera) == 0x0A);
STATIC_ASSERT(offsetof(Dll407State, mode) == 0x0B);
STATIC_ASSERT(offsetof(Dll407State, active) == 0x0C);
STATIC_ASSERT(offsetof(Dll407State, sparkArmed) == 0x0D);
STATIC_ASSERT(offsetof(Dll407State, previousActive) == 0x0E);
STATIC_ASSERT(offsetof(Dll407State, stage) == 0x0F);

extern const Dll69EffectParams gDll407EffectParamTemplate;
extern s8 gDll407PuzzleProgress;
extern ObjectDescriptor gDll407ObjDescriptor;

int dll407_getExtraSize(void);
int dll407_getObjectTypeId(void);
void dll407_free(GameObject* obj);
void dll407_render(GameObject* obj, int renderArg2, int renderArg3, int renderArg4, int renderArg5, s8 visible);
void dll407_hitDetect(void);
void dll407_update(int objectAddress);
void dll407_init(GameObject* obj, const Dll407Placement* placement);
void dll407_release(void);
void dll407_initialise(void);

#endif /* DLLS_OBJECTS_407_H_ */
