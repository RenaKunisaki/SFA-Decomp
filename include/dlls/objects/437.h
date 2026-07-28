#ifndef DLLS_OBJECTS_437_H_
#define DLLS_OBJECTS_437_H_

#include "dlls/object_descriptor.h"
#include "game/objects/object_fwd.h"
#include "game/objects/object_setup.h"
#include "main/dll/baddie_state.h"

#define DLL437_STATE_HANDLER_COUNT                   5
#define DLL437_SUBSTATE_HANDLER_COUNT                3
#define DLL437_SEQUENCE_ID_SC_LIGHTFOOT              0x27B
#define DLL437_SEQUENCE_ID_SC_BABY_LIGHTFOOT         0x27C
#define DLL437_COMPLETION_GAMEBIT_SC_TOTEM_BOND      0x64C

typedef int (*Dll437StateHandler)(GameObject* obj, int state, f32 timeDelta);
typedef int (*Dll437SubstateHandler)(int obj, int state);

typedef struct Dll437Placement {
    ObjPlacement base;
    u8 unknown18[2];
    union {
        s16 behaviorId;
        s16 completionGameBit;
    };
    s16 eventGameBit;
    u8 unknown1E[0x28 - 0x1E];
    s8 objectFlags;
    u8 unknown29[0x30 - 0x29];
    s16 activeGameBit;
} Dll437Placement;

typedef struct Dll437ControlState {
    const s16* moveIds;
    const f32* moveSpeeds;
    f32 completionTimer;
    f32 pulseTimer;
    f32 lifeTimer;
    f32 wanderTimer;
    u8 unknown18[0x24 - 0x18];
    u16 moveIndex;
    s16 weaponDefNoSentinel;
    s16 weaponDefNo;
    u16 movementSfxId;
    u8 completionCountdown;
    u8 unknown2D[0x30 - 0x2D];
} Dll437ControlState;

typedef struct Dll437ButtonTimingControlState {
    u8 unknown00[0x18];
    u16 phase;
    u16 previousPhase2;
    u16 previousPhase;
    u8 unknown1E[0x24 - 0x1E];
    u16 animationIndex;
    u8 unknown26[0x2D - 0x26];
    u8 difficulty;
} Dll437ButtonTimingControlState;

typedef struct Dll437State {
    GroundBaddieState groundBaddie;
    u8 unknown410[0x440 - 0x410];
} Dll437State;

STATIC_ASSERT(offsetof(Dll437Placement, base) == 0x00);
STATIC_ASSERT(offsetof(Dll437Placement, behaviorId) == 0x1A);
STATIC_ASSERT(offsetof(Dll437Placement, completionGameBit) == 0x1A);
STATIC_ASSERT(offsetof(Dll437Placement, eventGameBit) == 0x1C);
STATIC_ASSERT(offsetof(Dll437Placement, objectFlags) == 0x28);
STATIC_ASSERT(offsetof(Dll437Placement, activeGameBit) == 0x30);

STATIC_ASSERT(offsetof(Dll437ControlState, moveIds) == 0x00);
STATIC_ASSERT(offsetof(Dll437ControlState, moveSpeeds) == 0x04);
STATIC_ASSERT(offsetof(Dll437ControlState, completionTimer) == 0x08);
STATIC_ASSERT(offsetof(Dll437ControlState, pulseTimer) == 0x0C);
STATIC_ASSERT(offsetof(Dll437ControlState, lifeTimer) == 0x10);
STATIC_ASSERT(offsetof(Dll437ControlState, wanderTimer) == 0x14);
STATIC_ASSERT(offsetof(Dll437ControlState, moveIndex) == 0x24);
STATIC_ASSERT(offsetof(Dll437ControlState, weaponDefNoSentinel) == 0x26);
STATIC_ASSERT(offsetof(Dll437ControlState, weaponDefNo) == 0x28);
STATIC_ASSERT(offsetof(Dll437ControlState, movementSfxId) == 0x2A);
STATIC_ASSERT(offsetof(Dll437ControlState, completionCountdown) == 0x2C);

STATIC_ASSERT(offsetof(Dll437ButtonTimingControlState, phase) == 0x18);
STATIC_ASSERT(offsetof(Dll437ButtonTimingControlState, previousPhase2) == 0x1A);
STATIC_ASSERT(offsetof(Dll437ButtonTimingControlState, previousPhase) == 0x1C);
STATIC_ASSERT(offsetof(Dll437ButtonTimingControlState, animationIndex) == 0x24);
STATIC_ASSERT(offsetof(Dll437ButtonTimingControlState, difficulty) == 0x2D);

STATIC_ASSERT(sizeof(Dll437State) == 0x440);
STATIC_ASSERT(offsetof(Dll437State, groundBaddie) == 0x000);
STATIC_ASSERT(offsetof(Dll437State, unknown410) == 0x410);

extern Dll437StateHandler gDll437StateHandlers[DLL437_STATE_HANDLER_COUNT];
extern Dll437SubstateHandler gDll437SubstateHandlers[DLL437_SUBSTATE_HANDLER_COUNT];

extern s16 gDll437MoveIds0[2];
extern f32 gDll437MoveSpeeds0[2];
extern s16 gDll437MoveIds1[2];
extern f32 gDll437MoveSpeeds1[2];
extern s16 gDll437MoveIds2[2];
extern f32 gDll437MoveSpeeds2[2];
extern s16 gDll437MoveIds3[2];
extern f32 gDll437MoveSpeeds3[2];
extern s16 gDll437MoveIds4[2];
extern f32 gDll437MoveSpeeds4[2];


int dll437_getExtraSize(void);
int dll437_getObjectTypeId(void);
void dll437_free(GameObject* obj, int preserveChildren);
void dll437_render(GameObject* obj, int renderArg2, int renderArg3, int renderArg4, int renderArg5, s8 visible);
void dll437_hitDetect(void);
void dll437_update(GameObject* obj);
void dll437_init(GameObject* obj, const Dll437Placement* placement, int isReload);
void dll437_release(void);
void dll437_initialise(void);

extern ObjectDescriptor gDll437ObjDescriptor;

#endif /* DLLS_OBJECTS_437_H_ */
