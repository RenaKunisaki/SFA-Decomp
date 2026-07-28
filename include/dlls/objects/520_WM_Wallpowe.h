#ifndef DLLS_OBJECTS_520_WM_WALLPOWE_H_
#define DLLS_OBJECTS_520_WM_WALLPOWE_H_

#include "global.h"

#define WM_WALLPOWE_NULL_DESCRIPTOR_WORD_COUNT 2

extern u32 gWM_WallpoweNullResourceDescriptor[WM_WALLPOWE_NULL_DESCRIPTOR_WORD_COUNT];

STATIC_ASSERT(sizeof(gWM_WallpoweNullResourceDescriptor) == 0x08);

#endif /* DLLS_OBJECTS_520_WM_WALLPOWE_H_ */
