#ifndef DLLS_OBJECTS_437_H_
#define DLLS_OBJECTS_437_H_

#include "dlls/object_descriptor.h"
#include "game/objects/object_fwd.h"
#include "game/objects/object_setup.h"
#include "main/dll/baddie_state.h"

#define DLL1B5_STATE_HANDLER_COUNT                   5
#define DLL1B5_SUBSTATE_HANDLER_COUNT                3
#define DLL1B5_SEQUENCE_ID_SC_LIGHTFOOT              0x27B
#define DLL1B5_SEQUENCE_ID_SC_BABY_LIGHTFOOT         0x27C
#define DLL1B5_COMPLETION_GAMEBIT_SC_TOTEM_BOND      0x64C

typedef int (*Dll1B5StateHandler)(GameObject* obj, int state, f32 timeDelta);
typedef int (*Dll1B5SubstateHandler)(int obj, int state);

typedef struct Dll1B5Placement {
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
} Dll1B5Placement;

typedef struct Dll1B5ControlState {
    const s16* moveIds;
    const f32* moveSpeeds;
    f32 completionTimer;
    f32 pulseTimer;
    f32 lifeTimer;
    f32 wanderTimer;
    u8 unknown18[0x1E - 0x18];
    u16 targetSector;
    u16 targetYawDelta;
    u16 targetDistance;
    u16 moveIndex;
    s16 weaponDefNoSentinel;
    s16 weaponDefNo;
    u16 movementSfxId;
    u8 completionCountdown;
    u8 unknown2D;
    u8 challengeCompletePending;
    u8 unknown2F;
} Dll1B5ControlState;

typedef struct Dll1B5ButtonTimingControlState {
    u8 unknown00[0x18];
    u16 phase;
    u16 previousPhase2;
    u16 previousPhase;
    u8 unknown1E[0x24 - 0x1E];
    u16 animationIndex;
    u8 unknown26[0x2D - 0x26];
    u8 difficulty;
} Dll1B5ButtonTimingControlState;

typedef struct Dll1B5State {
    GroundBaddieState groundBaddie;
    u8 unknown410[0x440 - 0x410];
} Dll1B5State;

STATIC_ASSERT(offsetof(Dll1B5Placement, base) == 0x00);
STATIC_ASSERT(offsetof(Dll1B5Placement, behaviorId) == 0x1A);
STATIC_ASSERT(offsetof(Dll1B5Placement, completionGameBit) == 0x1A);
STATIC_ASSERT(offsetof(Dll1B5Placement, eventGameBit) == 0x1C);
STATIC_ASSERT(offsetof(Dll1B5Placement, objectFlags) == 0x28);
STATIC_ASSERT(offsetof(Dll1B5Placement, activeGameBit) == 0x30);

STATIC_ASSERT(offsetof(Dll1B5ControlState, moveIds) == 0x00);
STATIC_ASSERT(offsetof(Dll1B5ControlState, moveSpeeds) == 0x04);
STATIC_ASSERT(offsetof(Dll1B5ControlState, completionTimer) == 0x08);
STATIC_ASSERT(offsetof(Dll1B5ControlState, pulseTimer) == 0x0C);
STATIC_ASSERT(offsetof(Dll1B5ControlState, lifeTimer) == 0x10);
STATIC_ASSERT(offsetof(Dll1B5ControlState, wanderTimer) == 0x14);
STATIC_ASSERT(offsetof(Dll1B5ControlState, targetSector) == 0x1E);
STATIC_ASSERT(offsetof(Dll1B5ControlState, targetYawDelta) == 0x20);
STATIC_ASSERT(offsetof(Dll1B5ControlState, targetDistance) == 0x22);
STATIC_ASSERT(offsetof(Dll1B5ControlState, moveIndex) == 0x24);
STATIC_ASSERT(offsetof(Dll1B5ControlState, weaponDefNoSentinel) == 0x26);
STATIC_ASSERT(offsetof(Dll1B5ControlState, weaponDefNo) == 0x28);
STATIC_ASSERT(offsetof(Dll1B5ControlState, movementSfxId) == 0x2A);
STATIC_ASSERT(offsetof(Dll1B5ControlState, challengeCompletePending) == 0x2E);
STATIC_ASSERT(sizeof(Dll1B5ControlState) == 0x30);
STATIC_ASSERT(offsetof(Dll1B5ControlState, completionCountdown) == 0x2C);

STATIC_ASSERT(offsetof(Dll1B5ButtonTimingControlState, phase) == 0x18);
STATIC_ASSERT(offsetof(Dll1B5ButtonTimingControlState, previousPhase2) == 0x1A);
STATIC_ASSERT(offsetof(Dll1B5ButtonTimingControlState, previousPhase) == 0x1C);
STATIC_ASSERT(offsetof(Dll1B5ButtonTimingControlState, animationIndex) == 0x24);
STATIC_ASSERT(offsetof(Dll1B5ButtonTimingControlState, difficulty) == 0x2D);

STATIC_ASSERT(sizeof(Dll1B5State) == 0x440);
STATIC_ASSERT(offsetof(Dll1B5State, groundBaddie) == 0x000);
STATIC_ASSERT(offsetof(Dll1B5State, unknown410) == 0x410);

extern Dll1B5StateHandler gDll1B5StateHandlers[DLL1B5_STATE_HANDLER_COUNT];
extern Dll1B5SubstateHandler gDll1B5SubstateHandlers[DLL1B5_SUBSTATE_HANDLER_COUNT];

extern s16 gDll1B5MoveIds0[2];
extern f32 gDll1B5MoveSpeeds0[2];
extern s16 gDll1B5MoveIds1[2];
extern f32 gDll1B5MoveSpeeds1[2];
extern s16 gDll1B5MoveIds2[2];
extern f32 gDll1B5MoveSpeeds2[2];
extern s16 gDll1B5MoveIds3[2];
extern f32 gDll1B5MoveSpeeds3[2];
extern s16 gDll1B5MoveIds4[2];
extern f32 gDll1B5MoveSpeeds4[2];


int dll437_getExtraSize(void);
int dll437_getObjectTypeId(void);
void dll437_free(GameObject* obj, int preserveChildren);
void dll437_render(GameObject* obj, int renderArg2, int renderArg3, int renderArg4, int renderArg5, s8 visible);
void dll437_hitDetect(void);
void dll437_update(GameObject* obj);
void dll437_init(GameObject* obj, const Dll1B5Placement* placement, int isReload);
void dll437_release(void);
void dll437_initialise(void);

extern ObjectDescriptor gDll1B5ObjDescriptor;

#endif /* DLLS_OBJECTS_437_H_ */
