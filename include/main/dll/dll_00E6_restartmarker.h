#ifndef MAIN_DLL_DLL_00E6_RESTARTMARKER_H_
#define MAIN_DLL_DLL_00E6_RESTARTMARKER_H_

#include "game/objects/object.h"
#include "dlls/object_descriptor.h"
#include "game/objects/object_setup.h"

typedef struct RestartMarkerPlacement
{
    ObjPlacement base;
    u8 rotXByte;
} RestartMarkerPlacement;

STATIC_ASSERT(offsetof(RestartMarkerPlacement, rotXByte) == 0x18);
STATIC_ASSERT(sizeof(RestartMarkerPlacement) == 0x1c);

extern ObjectDescriptor gReStartMarkerObjDescriptor;

void restartmarker_init(GameObject* obj, RestartMarkerPlacement* placement);

#endif /* MAIN_DLL_DLL_00E6_RESTARTMARKER_H_ */
