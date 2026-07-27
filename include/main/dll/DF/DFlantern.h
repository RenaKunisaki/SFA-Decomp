#ifndef MAIN_DLL_DF_DFLANTERN_H_
#define MAIN_DLL_DF_DFLANTERN_H_

#include "game/objects/object.h"
#include "ghidra_import.h"
#include "main/objanim_update.h"

void dfshshrine_updateHoverMotion(int obj);
int DFSH_Shrine_SeqFn(GameObject* obj, int unused, ObjAnimUpdateState* animUpdate);
int DFSH_Shrine_getExtraSize(void);
int DFSH_Shrine_getObjectTypeId(void);
void DFSH_Shrine_free(GameObject* obj);

#endif /* MAIN_DLL_DF_DFLANTERN_H_ */
