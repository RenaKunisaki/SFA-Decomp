#ifndef MAIN_DLL_WMGALLEONSTATE_STRUCT_H_
#define MAIN_DLL_WMGALLEONSTATE_STRUCT_H_

#include "types.h"

typedef struct WMGalleonState
{
    f32 savedX;
    f32 savedY;
    f32 savedZ;
    u8 mapEventsLatched; /* 0x0C: cleared on a non-map-change free */
    u8 pad0D;
    s16 savedYaw;
} WMGalleonState;

#endif
