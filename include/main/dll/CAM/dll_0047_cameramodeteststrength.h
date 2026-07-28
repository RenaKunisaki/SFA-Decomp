#ifndef MAIN_DLL_CAM_DLL_0047_CAMERAMODETESTSTRENGTH_H_
#define MAIN_DLL_CAM_DLL_0047_CAMERAMODETESTSTRENGTH_H_

#include "main/camera_object.h"
#include "types.h"

u32 camTestStrengthUpdateBlend(CameraObject* camera, u32 flagsIn);
void pathcam_advanceNodePair(int* nodeId, int* leadNodeId, f32 x, f32 y, f32 z, int tag);
int pathcam_walkToPathEnd(int curve, int* count, int tag);
void pathcam_buildWindowSamples(int* nodes, f32* outX, f32* outY, f32* outZ, f32* outPitch, f32* outYaw,
                                f32* outRoll, f32* outFov);
void pathcam_findTaggedNodeWindow(u8* node, int* out, int tag);
f32 pathcam_segmentParam(f32 x, f32 unused, f32 z, int* nodes);
void cameraModeTestStrengthFn_8010b238(f32 fovEnd, CameraObject* camera, f32* posEnd, s32 rotXEnd, s32 rotYEnd,
                                      s32 rotZEnd);
void CameraModeTestStrength_copyToCurrent(void);
void CameraModeTestStrength_free(void);
void CameraModeTestStrength_update(CameraObject* camera);
void CameraModeTestStrength_init(CameraObject* camera, int unused, int* settings);
void CameraModeTestStrength_release(void);
void CameraModeTestStrength_initialise(void);

#endif /* MAIN_DLL_CAM_DLL_0047_CAMERAMODETESTSTRENGTH_H_ */
