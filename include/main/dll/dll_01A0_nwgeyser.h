#ifndef MAIN_DLL_DLL_01A0_NWGEYSER_H_
#define MAIN_DLL_DLL_01A0_NWGEYSER_H_

#include "game/objects/object.h"
#include "ghidra_import.h"
#include "main/objanim_update.h"

void nw_geyser_free(GameObject* obj);
void nw_geyser_init(GameObject* obj);
void nw_geyser_update(GameObject* obj);
int NW_geyser_SeqFn(GameObject* obj, int unused, ObjAnimUpdateState* animUpdate);

#endif /* MAIN_DLL_DLL_01A0_NWGEYSER_H_ */
