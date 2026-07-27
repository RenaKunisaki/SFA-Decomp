#ifndef MAIN_DLL_GROUNDANIMATOR_H_
#define MAIN_DLL_GROUNDANIMATOR_H_

#include "game/objects/object.h"
#include "ghidra_import.h"
#include "dlls/object_descriptor.h"

extern ObjectDescriptor13 gAppleOnTreeObjDescriptor;
void AppleOnTree_setPosition(GameObject* obj, float* pos);
void FUN_8017de58(u64 param_1, double param_2, double param_3, u64 param_4, u64 param_5, u64 param_6, u64 param_7,
                  u64 param_8, u32 param_9);

#endif /* MAIN_DLL_GROUNDANIMATOR_H_ */
