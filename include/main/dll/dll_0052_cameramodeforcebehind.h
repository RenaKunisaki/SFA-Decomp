#ifndef MAIN_DLL_DLL_0052_CAMERAMODEFORCEBEHIND_H_
#define MAIN_DLL_DLL_0052_CAMERAMODEFORCEBEHIND_H_

#include "global.h"
#include "main/camera_object.h"

void CameraModeForceBehind_copyToCurrent(void);
void CameraModeForceBehind_free(void);
void CameraModeForceBehind_update(CameraObject* camera);
void CameraModeForceBehind_init(CameraObject* camera, int unused, f32* params);
void CameraModeForceBehind_release(void);
void CameraModeForceBehind_initialise(void);

#endif /* MAIN_DLL_DLL_0052_CAMERAMODEFORCEBEHIND_H_ */
