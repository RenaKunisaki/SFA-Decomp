#ifndef MAIN_DLL_DLL_0049_CAMERAMODECOMBAT_H_
#define MAIN_DLL_DLL_0049_CAMERAMODECOMBAT_H_

#include "main/camera_object.h"
#include "game/objects/object.h"
#include "types.h"

void CameraModeCombat_copyToCurrent(void);
void camCombatIntroEvalPos(CameraObject* camera, float* outX, float* outY, float* outZ, f32* targetY);
void CameraModeCombat_free(CameraObject* camera);
void CameraModeCombat_update(CameraObject* cam);
void CameraModeCombat_init(CameraObject* camera, u32 unused, GameObject** targetPtr);
void CameraModeCombat_release(void);
void CameraModeCombat_initialise(void);

#endif /* MAIN_DLL_DLL_0049_CAMERAMODECOMBAT_H_ */
