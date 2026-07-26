#ifndef DLLS_OBJECTS_202_H_
#define DLLS_OBJECTS_202_H_

#include "dlls/object_descriptor.h"
#include "main/dll/baddie_state.h"

typedef struct GameObject GameObject;
typedef int (*IceBaddieStateHandler)(GameObject* obj, GroundBaddieState* state);

void iceBaddie_installStateHandlers(void);

int iceBaddie_updateOpenHitState(GameObject* obj, GroundBaddieState* state);
int iceBaddie_updateOpenState(GameObject* obj, GroundBaddieState* state);
int iceBaddie_updateHideResetState(GameObject* obj, GroundBaddieState* state);
int iceBaddie_updateImpactHitState(GameObject* obj, GroundBaddieState* state);
int iceBaddie_updateSpinState(GameObject* obj, GroundBaddieState* state);
int iceBaddie_stateHandlerA05(GameObject* obj, GroundBaddieState* state);
int iceBaddie_stateHandlerA06(GameObject* obj, GroundBaddieState* state);
int iceBaddie_updateHeightBlendState(GameObject* obj, GroundBaddieState* state);
int iceBaddie_updateControlMove5State(GameObject* obj, GroundBaddieState* state);
int iceBaddie_updateCommDownState(GameObject* obj, GroundBaddieState* state);
int iceBaddie_updateDropState(GameObject* obj, GroundBaddieState* state);
int iceBaddie_stateHandlerA0B(GameObject* obj, GroundBaddieState* state);
int iceBaddie_updateContactHitState(GameObject* obj, GroundBaddieState* state);
int iceBaddie_updateLandingState(GameObject* obj, GroundBaddieState* state);

int iceBaddie_checkTargetState(GameObject* obj, GroundBaddieState* state);
int iceBaddie_stateHandlerB01(GameObject* obj, GroundBaddieState* state);
int iceBaddie_stateHandlerB02(GameObject* obj, GroundBaddieState* state);
int iceBaddie_stateHandlerB03(GameObject* obj, GroundBaddieState* state);
int iceBaddie_stateHandlerB04(GameObject* obj, GroundBaddieState* state);
int iceBaddie_stateHandlerB05(GameObject* obj, GroundBaddieState* state);
int iceBaddie_stateHandlerB06(GameObject* obj, GroundBaddieState* state);
int iceBaddie_stateHandlerB07(GameObject* obj, GroundBaddieState* state);

extern IceBaddieStateHandler gIceBaddieStateHandlersA[];
extern IceBaddieStateHandler gIceBaddieStateHandlersB[];
extern ObjectDescriptor12 gIceBaddieObjDescriptor;

#endif /* DLLS_OBJECTS_202_H_ */
