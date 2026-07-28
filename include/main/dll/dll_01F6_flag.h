#ifndef MAIN_DLL_DLL_01F6_FLAG_H_
#define MAIN_DLL_DLL_01F6_FLAG_H_

#include "game/objects/object.h"
#include "dlls/object_descriptor.h"
#include "game/objects/object_setup.h"

typedef struct FlagPlacement
{
    ObjPlacement base;
    s8 rotXByte;
} FlagPlacement;

STATIC_ASSERT(offsetof(FlagPlacement, rotXByte) == 0x18);

int Flag_getExtraSize(void);
int Flag_getObjectTypeId(void);
void Flag_free(void);
void Flag_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible);
void Flag_hitDetect(void);
void Flag_update(GameObject* obj);
void Flag_init(GameObject* obj, FlagPlacement* placement);
void Flag_release(void);
void Flag_initialise(void);

extern ObjectDescriptor gFlagObjDescriptor;

#endif /* MAIN_DLL_DLL_01F6_FLAG_H_ */
