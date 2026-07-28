#ifndef H_MAIN_DLL_DLL_80198A00_H
#define H_MAIN_DLL_DLL_80198A00_H

#include "global.h"

int triggerPointInBox(GameObject* obj, f32* point);
void triggerEvalPlaneCrossing(GameObject* obj, GameObject* seqObj);
void triggerEvalCurveLoop(GameObject* obj, GameObject* seqObj);

#endif /* H_MAIN_DLL_DLL_80198A00_H */
