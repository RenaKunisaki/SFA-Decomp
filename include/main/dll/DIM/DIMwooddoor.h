#ifndef MAIN_DLL_DIM_DIMWOODDOOR_H_
#define MAIN_DLL_DIM_DIMWOODDOOR_H_

#include "ghidra_import.h"
#include "game/objects/object.h"
#include "main/dll/DIM/dimwooddoor_aim_api.h"

void DIMwooddoor_updateFallingDebris(GameObject* obj);
void DIMwooddoor_spawnShard(GameObject* obj, u8 variant);

#endif /* MAIN_DLL_DIM_DIMWOODDOOR_H_ */
