#ifndef DLLS_OBJECTS_515_H_
#define DLLS_OBJECTS_515_H_

#include "global.h"

#define DLL203_NULL_DESCRIPTOR_WORD_COUNT 2

extern u32 gDll203NullResourceDescriptor[DLL203_NULL_DESCRIPTOR_WORD_COUNT];

STATIC_ASSERT(sizeof(gDll203NullResourceDescriptor) == 0x08);

#endif /* DLLS_OBJECTS_515_H_ */
