#ifndef MAIN_DLL_DLL_0044_CAMERAMODEVIEWFINDER_H_
#define MAIN_DLL_DLL_0044_CAMERAMODEVIEWFINDER_H_

#include "types.h"

struct CameraObject;

void firstPersonDoControls(struct CameraObject* obj);
int firstPersonEnter(struct CameraObject* cam, s16* p2);
void CameraModeViewfinder_copyToCurrent(s16* camObj);
void CameraModeViewfinder_free(int camObj);
void CameraModeViewfinder_update(struct CameraObject* obj);
void CameraModeViewfinder_init(struct CameraObject* obj, int mode, int* args);
void CameraModeViewfinder_release(void);
void CameraModeViewfinder_initialise(void);

#endif /* MAIN_DLL_DLL_0044_CAMERAMODEVIEWFINDER_H_ */
