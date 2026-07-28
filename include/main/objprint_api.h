#ifndef MAIN_OBJPRINT_API_H_
#define MAIN_OBJPRINT_API_H_

#include "global.h"
#include "game/objects/object.h"

typedef struct ModelFileHeader ModelFileHeader;
typedef struct ObjModel ObjModel;

int* objGetLookAtJointKeys(void);
void objPosFn_80039510(GameObject* obj, int key, f32* outPosition);
void characterClampJointVecs(GameObject* obj, int* keys, int count, int lo, int hi);
s16* objModelGetVecFn_800395d8(GameObject* obj, int target);
void characterHeadLookRelax(GameObject* obj, void* state);
void objSetColorFilter(s16 red, s16 green, s16 blue);
void objSetGlowColor(int red, int green, int blue, int alpha);
void objSetModelMatrixOverride(f32* matrix);
int objGetAlphaCompareThreshold(void);
void objSetAlphaCompareThreshold(u8 alpha);
void modelCalcVtxGroupMtxs(ModelFileHeader* def, ObjModel* model);
void staffMtxFn_8003b620(int staff, GameObject* obj, int model, int a, int b, int c);
void objModelClearVecFn_8003aa40(GameObject* obj);
int characterTrackJointList(GameObject* obj, int* keys, int count, u8* channels);
s16 objMathFn_8003a380(GameObject* obj, GameObject* target, f32* targetPos, u8* channels, s16* speeds,
                       f32 yOffset, int unused, int basePitch);
void objJointTracksSetAngles(u8* channelData, int count, s16 yaw, s16 pitch);
void characterDecayJointVecs(GameObject* obj, int* keys, int count);
void objFn_8003acfc(GameObject* obj, int* keys, int count, u8* channels);

#endif /* MAIN_OBJPRINT_API_H_ */
