#ifndef MAIN_DLL_BOMBPLANT_PLACEMENT_H_
#define MAIN_DLL_BOMBPLANT_PLACEMENT_H_

#include "global.h"
#include "game/objects/object_setup.h"

typedef struct BombplantPlacement
{
    ObjPlacement base;
    s16 growTimer;
    s16 timerBase; /* 0x1a: base value for grow/regrow timer (+ random spread) */
    s16 gameBit; /* 0x1c: gated mainGetBit */
    s8 spawnYawByte; /* 0x1e: spore yaw param (<<8 -> spore spawnYaw) */
    s8 objectTypeParam; /* 0x1f: signed byte, <<8 -> object rotX seed */
} BombplantPlacement;

#endif
