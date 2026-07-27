#ifndef MAIN_DLL_CREATOR1C4_H_
#define MAIN_DLL_CREATOR1C4_H_

#include "ghidra_import.h"
#include "game/objects/object.h"
#include "main/objanim_update.h"

int gpsh_objcreator_getExtraSize(void);
int gpsh_objcreator_getObjectTypeId(void);
void gpsh_objcreator_free(void);
void gpsh_objcreator_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible);
void gpsh_objcreator_hitDetect(void);
void gpsh_objcreator_update(GameObject* obj);
void gpsh_objcreator_init(GameObject* obj, int* def);
void gpsh_objcreator_release(void);
void gpsh_objcreator_initialise(void);

#endif /* MAIN_DLL_CREATOR1C4_H_ */
