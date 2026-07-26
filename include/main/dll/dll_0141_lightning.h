#ifndef MAIN_DLL_DLL_0141_LIGHTNING_H_
#define MAIN_DLL_DLL_0141_LIGHTNING_H_

#include "ghidra_import.h"
#include "game/objects/object.h"
#include "dlls/object_descriptor.h"

#define MMP_LIGHTNING_OBJGROUP 0x48

typedef struct LightningPlacement LightningPlacement;

int lightning_getExtraSize(void);
void lightning_free(GameObject* obj, int mode);
void lightning_render(GameObject* obj);
void lightning_update(GameObject* obj);
void lightning_init(GameObject* obj, LightningPlacement* placement);

extern ObjectDescriptor gLightningObjDescriptor;

#endif /* MAIN_DLL_DLL_0141_LIGHTNING_H_ */
