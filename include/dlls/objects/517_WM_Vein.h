#ifndef DLLS_OBJECTS_517_WM_VEIN_H_
#define DLLS_OBJECTS_517_WM_VEIN_H_

#include "global.h"

#define WM_VEIN_NULL_DESCRIPTOR_WORD_COUNT 12

extern u32 gWMVeinNullResourceDescriptor[WM_VEIN_NULL_DESCRIPTOR_WORD_COUNT];

STATIC_ASSERT(sizeof(gWMVeinNullResourceDescriptor) == 0x30);

#endif /* DLLS_OBJECTS_517_WM_VEIN_H_ */
