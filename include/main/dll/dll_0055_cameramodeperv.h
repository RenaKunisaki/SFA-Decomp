#ifndef MAIN_DLL_DLL_0055_CAMERAMODEPERV_H_
#define MAIN_DLL_DLL_0055_CAMERAMODEPERV_H_

#include "global.h"
#include "main/camera_object.h"

void CameraModePerv_copyToCurrent(void);
void CameraModePerv_free(void);
void CameraModePerv_update(CameraObject* camera);
void CameraModePerv_init(CameraObject* camera);
void CameraModePerv_release(void);
void CameraModePerv_initialise(void);

#endif /* MAIN_DLL_DLL_0055_CAMERAMODEPERV_H_ */
