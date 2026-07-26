#ifndef MAIN_DLL_DLL_80161130_H_
#define MAIN_DLL_DLL_80161130_H_

#include "main/dll/baddie_state.h"
#include "game/objects/object.h"

int grimble_stateHandlerB05(GameObject* obj, GroundBaddieState* state);
int grimble_stateHandlerB04(GameObject* obj, GroundBaddieState* state);
int grimble_stateHandlerB03(GameObject* obj, GroundBaddieState* state);
int grimble_stateHandlerB01(GameObject* obj, GroundBaddieState* state);
int grimble_stateHandlerB00(GameObject* obj, GroundBaddieState* state);
int grimble_stateHandlerA09(GameObject* obj, GroundBaddieState* state);
int grimble_stateHandlerA08(GameObject* obj, GroundBaddieState* state);
int grimble_stateHandlerA07(GameObject* obj, GroundBaddieState* state);
int grimble_stateHandlerA06(GameObject* obj, GroundBaddieState* state, f32 speed);
int grimble_stateHandlerA05(GameObject* obj, GroundBaddieState* state);
int grimble_stateHandlerA04(GameObject* obj, GroundBaddieState* state);
int grimble_stateHandlerA03(GameObject* obj, GroundBaddieState* state);
int scarab_updateProximityGate(GameObject* obj, GroundBaddieState* state);

#endif /* MAIN_DLL_DLL_80161130_H_ */
