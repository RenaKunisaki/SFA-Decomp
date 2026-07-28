#ifndef MAIN_DLL_DR_CLOUDRUNNER_STATE_H_
#define MAIN_DLL_DR_CLOUDRUNNER_STATE_H_

#include "global.h"
#include "main/dll/baddie_state.h"
#include "main/dll/curve_walker.h"
#include "main/objprint_sound_api.h"
#include "main/objprint_character_api.h"

typedef struct CloudRunnerState {
    BaddieState baddie;
    RomCurveWalker curveWalker; /* 0x35c: rom-curve follower; its posX/posY/posZ are copied into the object's anim.localPos */
    CharacterEyeAnimState eyeAnimState; /* 0x464: head-aim / eye-blink record (characterDoEyeAnims) */
    u8 pad48C[0x494 - 0x48c];
    ObjSoundState modelSoundState; /* 0x494: mouth/voice playback state (objSoundUpdateMouth) */
    u8 pad4C4[0xad5 - 0x4c4];
    u8 moveFlags;
    u8 padAD6[0xae8 - 0xad6];
    f32 spawnPosX; /* 0xae8: stored position fed to a spawned object's ObjPlacement.pos */
    f32 spawnPosY;
    f32 spawnPosZ;
    f32 lastPosX;
    f32 lastPosY;
    f32 lastPosZ;
    u8 padB00[4];
    int unkB04;
    u8 padB08[0xb50 - 0xb08];
    f32 pathFollowSpeed;
    u8 padB54[0xbae - 0xb54];
    s16 unkBAE;
    s16 airTimeRemaining;
    u8 flightState;
    u8 padBB3;
    u8 spawnVariant; /* 0xbb4: variant selector from spawn-setup byte +0x19; gates init (case 0 = early-out) */
    u8 padBB5;
    u8 flagsBB6; /* 0xbb6: bit flags (|=4, &=~8) */
    u8 unkBB7;
    u8 unkBB8;
    u8 padBB9;
    s16 headingAngle; /* 0xbba: yaw; loaded from/stored to anim.rotX, turned toward target by moveInputX */
    s16 pitchAngle;
    s16 rollAngle;
    u8 flagsBC0; /* ByteFlags */
    u8 flagsBC1; /* ByteFlags */
    u8 padBC2;
    s8 cooldownTimer;
    s8 sequenceIndex;
    u8 padBC5[3];
} CloudRunnerState;

#endif
