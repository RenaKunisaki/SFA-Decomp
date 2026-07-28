#ifndef MAIN_DLL_DLL_0053_CAMERAMODECLOUDRUNNER_H_
#define MAIN_DLL_DLL_0053_CAMERAMODECLOUDRUNNER_H_

#include "main/vecmath.h"
#include "main/camera_object.h"

void CameraModeCloudRunner_copyToCurrent(void);
void CameraModeCloudRunner_free(void);
void CameraModeCloudRunner_update(CameraObject* camera);
void CameraModeCloudRunner_init(int* camera, int radius, f32* focus);
void CameraModeCloudRunner_release(void);
void CameraModeCloudRunner_initialise(void);

#endif /* MAIN_DLL_DLL_0053_CAMERAMODECLOUDRUNNER_H_ */
