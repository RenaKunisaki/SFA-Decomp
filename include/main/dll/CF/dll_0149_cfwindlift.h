#ifndef MAIN_DLL_CF_DLL_0149_CFWINDLIFT_H_
#define MAIN_DLL_CF_DLL_0149_CFWINDLIFT_H_

#include "global.h"
#include "game/objects/object_setup.h"

typedef struct WindliftPlacement
{
    ObjPlacement base;
    s16 unk18;
    s16 pullStrength; /* 0x1A: wind pull strength passed to WindLift_updateRider */
    u8 pad1C[0x22 - 0x1C];
    s16 unk22;
    u8 pad24[0x28 - 0x24];
} WindliftPlacement;

typedef struct WindliftObjectDef
{
    ObjPlacement base;
    s8 unk18;
    s8 heightByte;    /* 0x19: lift height in 4-unit increments (0 = default) */
    s16 pullStrength; /* 0x1A */
    s16 delay;
    s16 seqId;
    u8 pad20[0x22 - 0x20];
    s16 unk22;
    u8 pad24[0x28 - 0x24];
} WindliftObjectDef;

typedef struct
{
    int riderObj;
    f32 f4;
    f32 speedDelta;
    f32 riseSpeed;
    u8 phaseFlags;
    u8 oscCounter;
    u8 pad12[2];
    int linkIndex;
} WindLiftSlot;

#endif /* MAIN_DLL_CF_DLL_0149_CFWINDLIFT_H_ */
