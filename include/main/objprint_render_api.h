#ifndef MAIN_OBJPRINT_RENDER_API_H_
#define MAIN_OBJPRINT_RENDER_API_H_

#include "types.h"

typedef struct GameObject GameObject;
typedef struct ModelLightStruct ModelLightStruct;
typedef struct ModelFileHeader ModelFileHeader;
typedef struct ObjModel ObjModel;

extern ModelLightStruct* lbl_803DCC64;

void objRender(int a, int b, int c, int d, GameObject* obj, int flag);
void objFuzzSetupGxState(void* obj);
void objRenderShadow(void* obj);
void objRenderShadowIfVisible(GameObject* obj, int a, int b, int c, int d, int e);
void objRenderAttachment(u8* obj, int* model);
void objRenderFuzz(int* obj);
void objRenderFn_800413d4(int* obj);
void objRenderFuzzShells(int* obj);
void objRenderInvalidateStateCache(void);
void objSetRenderingShadowPass(u8 enabled);
void objRenderFn_80041018(GameObject* obj);
void objSetOverrideColor(u8 red, u8 green, u8 blue);
void objRenderModel(GameObject* obj);
void objSetCurrentMatrix(u32 mtx);
void modelInitMtxs(ModelFileHeader* modelFile, ObjModel* model);
void modelBuildPosNrmMtxs(u8* modelFile, int* model, f32* matrix, f32* matrix2);
int objMatrixToRotation(f32* matrix, s16* outX, s16* outY, s16* outZ);
int modelRenderCb_8003c268(int obj, int* model, int renderOpIndex);
int shaderFuzzFn_8003cc1c(GameObject* obj, ObjModel* model, int renderOpIndex);

#endif /* MAIN_OBJPRINT_RENDER_API_H_ */
