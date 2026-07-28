#ifndef MAIN_DLL_DLL_0051_CAMERAMODECANNON_H_
#define MAIN_DLL_DLL_0051_CAMERAMODECANNON_H_

#include "global.h"
#include "main/camera_object.h"

void CameraModeCannon_copyToCurrent(void);
void CameraModeCannon_free(void);
void CameraModeCannon_update(CameraObject* camera);
void CameraModeCannon_init(CameraObject* camera, int unused, int* p3);
void CameraModeCannon_release(void);
void CameraModeCannon_initialise(void);

#endif /* MAIN_DLL_DLL_0051_CAMERAMODECANNON_H_ */
