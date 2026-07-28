#ifndef DLLS_OBJECTS_514_H_
#define DLLS_OBJECTS_514_H_

#include "global.h"

#define DLL202_NULL_DESCRIPTOR_WORD_COUNT 2

extern u32 gDll202NullResourceDescriptor[DLL202_NULL_DESCRIPTOR_WORD_COUNT];

STATIC_ASSERT(sizeof(gDll202NullResourceDescriptor) == 0x08);

#endif /* DLLS_OBJECTS_514_H_ */
