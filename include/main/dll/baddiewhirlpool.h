#ifndef MAIN_DLL_BADDIEWHIRLPOOL_H_
#define MAIN_DLL_BADDIEWHIRLPOOL_H_

#include "main/dll/baddie_state.h"

typedef struct GameObject GameObject;

void iceBaddie_enterWhirlpoolGroup(GameObject* obj, GroundBaddieState* state);
void iceBaddie_leaveWhirlpoolGroup(GameObject* obj, GroundBaddieState* state);
void baddie_initWhirlpoolState(int* obj, GroundBaddieState* state);

#endif /* MAIN_DLL_BADDIEWHIRLPOOL_H_ */
