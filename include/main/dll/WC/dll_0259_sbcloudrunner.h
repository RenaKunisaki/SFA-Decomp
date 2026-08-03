#ifndef MAIN_DLL_WC_WCLEVCONTROL_H_
#define MAIN_DLL_WC_WCLEVCONTROL_H_

#include "game/objects/object.h"
#include "types.h"
#include "main/objseq.h"

typedef struct SBCloudRunnerState SBCloudRunnerState;

int SB_CloudRunner_SeqFn(GameObject* obj, int unused, ObjSeqState* animUpdate);
void SB_CloudRunner_UpdateSteer(GameObject* obj, SBCloudRunnerState* state);
void SB_CloudRunner_HandlePriorityHit(GameObject* obj, SBCloudRunnerState* state);

#endif /* MAIN_DLL_WC_WCLEVCONTROL_H_ */
