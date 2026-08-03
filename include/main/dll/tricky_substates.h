#ifndef MAIN_DLL_TRICKY_SUBSTATES_H_
#define MAIN_DLL_TRICKY_SUBSTATES_H_

#include "game/objects/object.h"
#include "types.h"

void trickyDigTunnel(u8* obj, u8* state);
void tricky_stateFindSecretDig(u8* obj, u8* state);
void tricky_stateFollowPlayer(u8* obj, u8* state);
int tricky_substateApproachThorntail(int obj, int state);
int tricky_substateFlameBreath(u8* obj, u8* state);
int tricky_substateBegForFood(GameObject* obj, int state);
int tricky_substateDigForFood(GameObject* obj, int state);
int tricky_substateIdlePick(u8* obj, u8* state);
u32 tricky_substateFidgetA(GameObject* obj, int* trickyState);
u32 tricky_substateFidgetB(GameObject* obj, int* trickyState);
u32 tricky_substateWaitMoveEnd(GameObject* obj, int* trickyState);
int tricky_substateHowlCall(GameObject* obj, int* trickyState);
int tricky_substateSleep(GameObject* obj, int* state);
u32 tricky_substateWaitQueuedMove(GameObject* obj, int* trickyState);
u32 tricky_substateReturnToHeel(GameObject* obj, int* trickyState);
int tricky_substateFollowIdle(GameObject* obj, int state);
u32 tricky_updateIdleBehavior(int obj, int* trickyState);
void tricky_pickAmbientActivity(u8* obj, u8* state);
void tricky_startRandomIdleMove(GameObject* obj, int trickyState);
int tricky_handleFeedOrTalk(GameObject* obj, int* state);

#endif /* MAIN_DLL_TRICKY_SUBSTATES_H_ */
