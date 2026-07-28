#ifndef MAIN_DLL_DLL_0050_CAMERAMODECRAWL_H_
#define MAIN_DLL_DLL_0050_CAMERAMODECRAWL_H_

#include "global.h"
#include "main/camera_object.h"

void CameraModeCrawl_copyToCurrent(void* param1, int param2);
void CameraModeCrawl_free(void);
void CameraModeCrawl_update(CameraObject* camera);
void CameraModeCrawl_init(void);
void CameraModeCrawl_release(void);
void CameraModeCrawl_initialise(void);

#endif /* MAIN_DLL_DLL_0050_CAMERAMODECRAWL_H_ */
