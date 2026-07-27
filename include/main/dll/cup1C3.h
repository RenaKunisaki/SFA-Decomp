#ifndef MAIN_DLL_CUP1C3_H_
#define MAIN_DLL_CUP1C3_H_

#include "ghidra_import.h"
#include "game/objects/object.h"
#include "main/objanim_update.h"

int dll_197_getExtraSize(void);
int dll_197_getObjectTypeId(void);
void dll_197_free(int obj);
void dll_197_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible);
void dll_197_hitDetect(void);
void dll_197_update(int obj);

#endif /* MAIN_DLL_CUP1C3_H_ */
