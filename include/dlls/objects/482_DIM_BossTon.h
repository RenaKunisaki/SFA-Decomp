#ifndef DLLS_OBJECTS_482_DIM_BOSSTON_H_
#define DLLS_OBJECTS_482_DIM_BOSSTON_H_

#include "dlls/object_descriptor.h"
#include "game/objects/object_fwd.h"
#include "game/objects/object_setup.h"
#include "main/dll/baddie_state.h"
#include "main/model_light.h"
#include "main/objanim_update.h"

#define DIMBOSSTONSIL_HIT_GAMEBIT 0x20C

/*
 * Only the fields consumed by this DLL are modeled. The active-target retail
 * placement files are unavailable, so this does not claim a complete record
 * width.
 */
typedef struct DIMbosstonsilPlacementView {
    ObjPlacement base;
    u8 unknown18[0x14];
    s16 eventId;
    s8 animObjectId;
} DIMbosstonsilPlacementView;

/*
 * initGroundBaddie establishes the shared GroundBaddieState contract. This
 * object's callbacks also give several family-specific roles to that same
 * allocation, so the complete 0x410-byte block exposes both views.
 */
typedef union DIMbosstonsilState {
    GroundBaddieState groundBaddie;
    struct {
        u8 unknown000[0x25F];
        u8 physicsActive;
        u8 unknown260[0x10];
        s16 hitReactionSubstate;
        u8 unknown272[0x02];
        s16 controlMode;
        u8 unknown276[0x04];
        s8 active;
        s8 stunReady;
        u8 unknown27C[0x24];
        f32 recoveryTimer;
        u8 unknown2A4[0x2C];
        GameObject* targetObj;
        u8 unknown2D4[0x72];
        s8 moveDone;
        u8 unknown347[0x02];
        s8 animFinished;
        u8 unknown34A[0x05];
        s8 lastHitPriority;
        u8 unknown350[0x04];
        s8 hitPoints;
        u8 unknown355[0x07];
        u8 moveScratch[0x84];
        void* savedPendingParentObj;
        u8 unknown3E4[0x10];
        s16 hitReactionGameBit;
        s16 eventGameBit;
        u8 unknown3F8[0x08];
        u16 stateFlags;
        u8 unknown402[0x03];
        u8 hitReactionMode;
        u8 unknown406[0x0A];
    };
} DIMbosstonsilState;

typedef int (*DIMbosstonsilHitReactionCallback)(GameObject* obj, DIMbosstonsilState* state);
typedef int (*DIMbosstonsilUpdateHitReactionCallback)(GameObject* obj, DIMbosstonsilState* state, int unused);

typedef struct DIMbosstonsilStateHandlerTable {
    DIMbosstonsilHitReactionCallback startIdle;
    DIMbosstonsilHitReactionCallback choose;
} DIMbosstonsilStateHandlerTable;

typedef struct DIMbosstonsilSubstateHandlerTable {
    DIMbosstonsilHitReactionCallback enable;
    DIMbosstonsilUpdateHitReactionCallback update;
} DIMbosstonsilSubstateHandlerTable;

STATIC_ASSERT(offsetof(DIMbosstonsilPlacementView, base) == 0x00);
STATIC_ASSERT(offsetof(DIMbosstonsilPlacementView, eventId) == 0x2C);
STATIC_ASSERT(offsetof(DIMbosstonsilPlacementView, animObjectId) == 0x2E);

STATIC_ASSERT(offsetof(DIMbosstonsilState, groundBaddie) == 0x000);
STATIC_ASSERT(offsetof(DIMbosstonsilState, physicsActive) == 0x25F);
STATIC_ASSERT(offsetof(DIMbosstonsilState, hitReactionSubstate) == 0x270);
STATIC_ASSERT(offsetof(DIMbosstonsilState, controlMode) == 0x274);
STATIC_ASSERT(offsetof(DIMbosstonsilState, active) == 0x27A);
STATIC_ASSERT(offsetof(DIMbosstonsilState, stunReady) == 0x27B);
STATIC_ASSERT(offsetof(DIMbosstonsilState, recoveryTimer) == 0x2A0);
STATIC_ASSERT(offsetof(DIMbosstonsilState, targetObj) == 0x2D0);
STATIC_ASSERT(offsetof(DIMbosstonsilState, moveDone) == 0x346);
STATIC_ASSERT(offsetof(DIMbosstonsilState, animFinished) == 0x349);
STATIC_ASSERT(offsetof(DIMbosstonsilState, lastHitPriority) == 0x34F);
STATIC_ASSERT(offsetof(DIMbosstonsilState, hitPoints) == 0x354);
STATIC_ASSERT(offsetof(DIMbosstonsilState, moveScratch) == 0x35C);
STATIC_ASSERT(offsetof(DIMbosstonsilState, savedPendingParentObj) == 0x3E0);
STATIC_ASSERT(offsetof(DIMbosstonsilState, hitReactionGameBit) == 0x3F4);
STATIC_ASSERT(offsetof(DIMbosstonsilState, eventGameBit) == 0x3F6);
STATIC_ASSERT(offsetof(DIMbosstonsilState, stateFlags) == 0x400);
STATIC_ASSERT(offsetof(DIMbosstonsilState, hitReactionMode) == 0x405);
STATIC_ASSERT(sizeof(DIMbosstonsilState) == 0x410);

STATIC_ASSERT(sizeof(DIMbosstonsilStateHandlerTable) == 0x08);
STATIC_ASSERT(sizeof(DIMbosstonsilSubstateHandlerTable) == 0x08);

int DIMbosstonsil_updateHitReaction(GameObject* obj, DIMbosstonsilState* state, int unused);
int DIMbosstonsil_enableHitReaction(GameObject* obj, DIMbosstonsilState* state);
int DIMbosstonsil_chooseHitReaction(GameObject* obj, DIMbosstonsilState* state);
int DIMbosstonsil_startIdleHitReaction(GameObject* obj, DIMbosstonsilState* state);
void DIMbosstonsil_checkHit(GameObject* obj, DIMbosstonsilState* state);
void dimBossTonsil_newState_hitFightMain(GameObject* obj, ObjAnimUpdateState* animUpdate, DIMbosstonsilState* state,
                                         DIMbosstonsilState* updateState);
int DIMbosstonsil_SeqFn(GameObject* obj, u32 unused, ObjAnimUpdateState* animUpdate);
void DIMbosstonsil_func0B(void);
int DIMbosstonsil_getControlMode(GameObject* obj);
int DIMbosstonsil_getExtraSize(void);
int DIMbosstonsil_getObjectTypeId(void);
void DIMbosstonsil_free(GameObject* obj);
void DIMbosstonsil_render(GameObject* obj, u32 renderArg2, u32 renderArg3, u32 renderArg4, u32 renderArg5,
                          char visible);
void DIMbosstonsil_hitDetect(GameObject* obj);
void DIMbosstonsil_update(GameObject* obj);
void DIMbosstonsil_init(GameObject* obj, u32 placementAddress, int isAltVariant);
void DIMbosstonsil_release(void);
void DIMbosstonsil_initialise(void);

extern DIMbosstonsilStateHandlerTable gDIMbosstonsilStateHandlers;
extern DIMbosstonsilSubstateHandlerTable gDIMbosstonsilSubstateHandlers;
extern ModelLightStruct* gDIMbosstonsilLight;
extern s8 gDIMbosstonsilRoutePhase;
extern ObjectDescriptor12 gDIM_BossTonsilObjDescriptor;

#endif /* DLLS_OBJECTS_482_DIM_BOSSTON_H_ */
