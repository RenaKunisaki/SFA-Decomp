#ifndef MAIN_DLL_PLAYER_MOTION_API_H_
#define MAIN_DLL_PLAYER_MOTION_API_H_

#include "global.h"
#include "game/objects/object.h"

void playerUpdateMotionState(GameObject* obj, int motionState, int baddieState);
void playerClampVelocityAndMove(GameObject* obj, f32 timeDelta);
void playerUpdateVelocityFromMotion(GameObject* obj, int motionState, int baddieState, f32 unusedTimeDelta);

#endif /* MAIN_DLL_PLAYER_MOTION_API_H_ */
