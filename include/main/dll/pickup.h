#ifndef MAIN_DLL_PICKUP_H_
#define MAIN_DLL_PICKUP_H_

#include "ghidra_import.h"
#include "game/objects/object_fwd.h"

void dll_9D_func03(GameObject *sourceObj, int variant, u8 *posSource, u32 flags);
void dll_9E_func03(GameObject *sourceObj, int variant, u8 *posSource, u32 flags);
void FUN_800fd9c4(short *param_1,int param_2,int param_3,u32 param_4,u32 param_5,
                 u32 *param_6);

#endif /* MAIN_DLL_PICKUP_H_ */
