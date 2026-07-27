#ifndef DLLS_OBJECTS_414_H_
#define DLLS_OBJECTS_414_H_

#include "dlls/object_descriptor.h"
#include "game/objects/object_fwd.h"
#include "game/objects/object_setup.h"
#include "main/dll/dll_0069_dll69func0.h"

typedef enum Dll414Mode {
    DLL414_MODE_SPARKLE = 0,
    DLL414_MODE_EGG_INTERACTION = 1,
} Dll414Mode;

/*
 * Partial setup view. The init callback proves the fields through 0x1E, but
 * retail has no OBJECTS.bin definition or romlist placement establishing the
 * record's complete width.
 */
typedef struct Dll414Placement {
    ObjPlacement base;
    s8 rotationIndex;
    u8 mode;
    s16 scalePacked;
    s16 sequenceIndex;
    s16 gameBitId;
} Dll414Placement;

typedef struct Dll414State {
    s32 gameBitId;
    s16 delayTimer;
    s16 resetTimer;
    s16 settleTimer;
    u8 lineOfSightVisible;
    u8 mode;
    u8 active;
    u8 needsOpenSfx;
    u8 previousActive;
    u8 sequenceIndex;
} Dll414State;

STATIC_ASSERT(offsetof(Dll414Placement, base) == 0x00);
STATIC_ASSERT(offsetof(Dll414Placement, rotationIndex) == 0x18);
STATIC_ASSERT(offsetof(Dll414Placement, mode) == 0x19);
STATIC_ASSERT(offsetof(Dll414Placement, scalePacked) == 0x1A);
STATIC_ASSERT(offsetof(Dll414Placement, sequenceIndex) == 0x1C);
STATIC_ASSERT(offsetof(Dll414Placement, gameBitId) == 0x1E);

STATIC_ASSERT(sizeof(Dll414State) == 0x10);
STATIC_ASSERT(offsetof(Dll414State, gameBitId) == 0x00);
STATIC_ASSERT(offsetof(Dll414State, delayTimer) == 0x04);
STATIC_ASSERT(offsetof(Dll414State, resetTimer) == 0x06);
STATIC_ASSERT(offsetof(Dll414State, settleTimer) == 0x08);
STATIC_ASSERT(offsetof(Dll414State, lineOfSightVisible) == 0x0A);
STATIC_ASSERT(offsetof(Dll414State, mode) == 0x0B);
STATIC_ASSERT(offsetof(Dll414State, active) == 0x0C);
STATIC_ASSERT(offsetof(Dll414State, needsOpenSfx) == 0x0D);
STATIC_ASSERT(offsetof(Dll414State, previousActive) == 0x0E);
STATIC_ASSERT(offsetof(Dll414State, sequenceIndex) == 0x0F);

extern ObjectDescriptor gDll414ObjDescriptor;
extern const Dll69EffectParams gDll414EffectParamsTemplate;
extern s8 gDll414SequenceStage;

int dll414_getExtraSize(void);
int dll414_getObjectTypeId(void);
void dll414_free(GameObject* obj);
void dll414_render(GameObject* obj, int renderArg2, int renderArg3, int renderArg4, int renderArg5, s8 visible);
void dll414_hitDetect(void);
void dll414_update(GameObject* obj);
void dll414_init(GameObject* obj, const Dll414Placement* placement);
void dll414_release(void);
void dll414_initialise(void);

#endif /* DLLS_OBJECTS_414_H_ */
