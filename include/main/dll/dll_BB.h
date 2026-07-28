#ifndef MAIN_DLL_DLL_BB_H_
#define MAIN_DLL_DLL_BB_H_

#include "ghidra_import.h"
#include "main/dll/CAM/dll_0001_camcontrol.h"

void camcontrol_applyState(CamcontrolCameraState *camera);
void camcontrol_applyQueuedAction(void);
void Camera_applyTargetFlags(int targetFlagMode);
void Camera_setTargetFlag2(int enable);
void Camera_applyFrameFlags(int flags);
void Camera_setLetterbox(int yOffset,int applyNow);

#endif /* MAIN_DLL_DLL_BB_H_ */
