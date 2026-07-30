#ifndef MAIN_TEX_DOLPHIN_H_
#define MAIN_TEX_DOLPHIN_H_

#include "ghidra_import.h"
#include "main/frustum.h"
#include "main/model_render_instrs_api.h"

struct MapBlockData;
struct MapBlockBoundsRec;
struct Shader;

u8 mapBlockBounds_HasCornerPastDepthThreshold(struct MapBlockBoundsRec* bounds, float* xform);
u32
frustumTestAabbWithPlaneOffsets(f32 minX, f32 maxX, f32 minY, f32 maxY, f32 minZ,
            f32 maxZ, f32 *planeOffsets);
u8
mapBlockBounds_ComputeAndTestPlanes(struct MapBlockBoundsRec* bounds,struct MapBlockData *block,FrustumPlane *planes,int planeCount,
            f32 *minX,f32 *minY,f32 *minZ,f32 *maxX,f32 *maxY,f32 *maxZ);
void FUN_8005e884(u32 param_1,u32 param_2,int param_3,int param_4,int *param_5,
                 float *param_6);
void mapBlockRender_drawDimmedAabbLights(struct MapBlockBoundsRec* bounds, struct MapBlockData* block,
                                        float* viewMtx);
void mapBlockRender_drawLightmapIndirectPasses(struct MapBlockData* blockData, struct Shader* shader,
                                               ModelRenderInstrsState* state, float (*viewMtx)[4]);
struct Shader* mapBlockRender_setLightmapShader(struct MapBlockData* blockData,
                                                   ModelRenderInstrsState* state);
#ifdef TEX_SETSHADER_U8
struct Shader* mapBlockRender_setShader(u8 doSetup, struct MapBlockData* blockData,
                                           ModelRenderInstrsState* state);
#else
struct Shader* mapBlockRender_setShader(int doSetup, struct MapBlockData* blockData,
                                           ModelRenderInstrsState* state);
#endif
void mapBlockRender_callList(u8 passSelect, u32 visArg, struct MapBlockData* block, struct Shader* shader,
                             ModelRenderInstrsState* state, float* mtx);

#endif /* MAIN_TEX_DOLPHIN_H_ */
