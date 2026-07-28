#ifndef MAIN_DLL_SB_DLL_01F0_SBKYTECAGE_H_
#define MAIN_DLL_SB_DLL_01F0_SBKYTECAGE_H_

#include "game/objects/object.h"
#include "dlls/object_descriptor.h"
#include "game/objects/object_setup.h"
#include "main/objanim_update.h"

typedef struct SBKyteCagePlacement
{
    ObjPlacement base;
    s8 rotXByte;
} SBKyteCagePlacement;

STATIC_ASSERT(offsetof(SBKyteCagePlacement, rotXByte) == 0x18);

int SB_KyteCage_SeqFn(GameObject* obj, int unused, ObjAnimUpdateState* animUpdate);
int SB_KyteCage_getExtraSize(void);
int SB_KyteCage_getObjectTypeId(void);
void SB_KyteCage_free(GameObject* obj);
void SB_KyteCage_render(void);
void SB_KyteCage_hitDetect(void);
void SB_KyteCage_update(GameObject* obj);
void SB_KyteCage_init(GameObject* obj, SBKyteCagePlacement* placement);
void SB_KyteCage_release(void);
void SB_KyteCage_initialise(void);

extern ObjectDescriptor gSB_KyteCageObjDescriptor;

#endif /* MAIN_DLL_SB_DLL_01F0_SBKYTECAGE_H_ */
