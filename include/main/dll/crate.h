#ifndef MAIN_DLL_CRATE_H_
#define MAIN_DLL_CRATE_H_

#include "ghidra_import.h"
#include "main/dll/sfxplayer.h"
#include "main/objanim_update.h"

u32 sfxplayer_updateState(int obj, u32 unused, ObjAnimUpdateState *animUpdate);

#endif /* MAIN_DLL_CRATE_H_ */
