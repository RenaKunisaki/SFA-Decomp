#ifndef MAIN_DLL_CRROCKFALLPLACEMENT_STRUCT_H_
#define MAIN_DLL_CRROCKFALLPLACEMENT_STRUCT_H_

#include "types.h"
#include "game/objects/object_setup.h"

typedef struct CrrockfallPlacement
{
    ObjPlacement base;
    u8 pad18[0x1A - 0x18];
    u8 triggerRange;
    u8 scaleByte;
    s16 gameBitId;
    s16 fallDelay;
} CrrockfallPlacement;

#endif
