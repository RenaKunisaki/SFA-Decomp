#ifndef MAIN_DLL_SC_SCTOTEMBOND_H_
#define MAIN_DLL_SC_SCTOTEMBOND_H_

#include "ghidra_import.h"
#include "main/objanim_update.h"

typedef struct ScTotemBondState {
    s16 yaw;
    s16 pitch;
    s16 roll;
    u8 pad06[0x08 - 0x06];
    f32 x;
    f32 y;
    f32 z;
    f32 cameraDistance;
    f32 spawnTimer;
    f32 completionTimer;
    s32 active;
    s16 ringIndex;
    u8 eventFlags;
} ScTotemBondState;

typedef struct ScTotemBondObject {
    union {
        ObjAnimComponent anim;
        struct {
            s16 yaw;
            s16 pitch;
            s16 roll;
            u8 pad06[0x0C - 0x06];
            f32 x;
            f32 y;
            f32 z;
            u8 pad18[0x36 - 0x18];
            u8 mapAlpha;
            u8 pad37[0x46 - 0x37];
            s16 objectType;
            u8 pad48[0x4C - 0x48];
            u8 *definition;
            u8 pad50[0xB0 - 0x50];
        };
    };
    u16 objectFlags;
    u8 padB2[0xB8 - 0xB2];
    ScTotemBondState *state;
    u32 (*animEventCallback)(struct ScTotemBondObject *obj,u32 param2,ObjAnimUpdateState *animUpdate);
} ScTotemBondObject;

STATIC_ASSERT(offsetof(ScTotemBondObject, anim) == 0x00);
STATIC_ASSERT(offsetof(ScTotemBondObject, yaw) == offsetof(ObjAnimComponent, rotX));
STATIC_ASSERT(offsetof(ScTotemBondObject, x) == offsetof(ObjAnimComponent, localPosX));
STATIC_ASSERT(offsetof(ScTotemBondObject, mapAlpha) == offsetof(ObjAnimComponent, alpha));
STATIC_ASSERT(offsetof(ScTotemBondObject, objectType) == offsetof(ObjAnimComponent, seqId));
STATIC_ASSERT(offsetof(ScTotemBondObject, definition) == offsetof(ObjAnimComponent, placementData));
STATIC_ASSERT(offsetof(ScTotemBondObject, objectFlags) == 0xB0);
STATIC_ASSERT(offsetof(ScTotemBondObject, state) == 0xB8);
STATIC_ASSERT(offsetof(ScTotemBondObject, animEventCallback) == 0xBC);

void sc_totembond_spawnGameBitOrbs(ScTotemBondObject *obj,ScTotemBondState *state,f32 radius);
u32 sc_totembond_SeqFn(ScTotemBondObject *obj,u32 param_2,ObjAnimUpdateState *animUpdate);
void sc_totembond_update(ScTotemBondObject *obj);
void sc_totembond_init(ScTotemBondObject *obj,int params);
int sc_totembond_insertOrderedGameBit(u16* gameBitIds, u16 newValue);

#endif /* MAIN_DLL_SC_SCTOTEMBOND_H_ */
