#ifndef MAIN_DLL_DLL_0159_BLASTED_H_
#define MAIN_DLL_DLL_0159_BLASTED_H_

#include "game/objects/object.h"
#include "dlls/object_descriptor.h"
#include "game/objects/object_setup.h"
#include "global.h"

typedef struct BlastedTargetSetup
{
    ObjPlacement base;
    s8 rotX;
    u8 pad19;
    s16 pieceCount;
    s16 triggerId;
    s16 completedGameBit;
    s16 progressGameBit;
} BlastedTargetSetup;

typedef struct BlastedTargetState
{
    u32 destroyedHitObjects[3];
    int triggerFired;
    u8 pieceCount;
    u8 damageStep;
    u8 reserved12[2];
} BlastedTargetState;

STATIC_ASSERT(offsetof(BlastedTargetSetup, rotX) == 0x18);
STATIC_ASSERT(offsetof(BlastedTargetSetup, pieceCount) == 0x1A);
STATIC_ASSERT(offsetof(BlastedTargetSetup, triggerId) == 0x1C);
STATIC_ASSERT(offsetof(BlastedTargetSetup, completedGameBit) == 0x1E);
STATIC_ASSERT(offsetof(BlastedTargetSetup, progressGameBit) == 0x20);
STATIC_ASSERT(sizeof(BlastedTargetSetup) == 0x24);
STATIC_ASSERT(offsetof(BlastedTargetState, triggerFired) == 0x0C);
STATIC_ASSERT(offsetof(BlastedTargetState, pieceCount) == 0x10);
STATIC_ASSERT(offsetof(BlastedTargetState, damageStep) == 0x11);
STATIC_ASSERT(sizeof(BlastedTargetState) == 0x14);

int blasted_activateMapLayer(GameObject* obj, int id);
int blasted_getExtraSize(void);
int blasted_getObjectTypeId(void);
void blasted_free(void);
void blasted_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible);
void blasted_hitDetect(void);
void blasted_update(GameObject* obj);
void blasted_init(GameObject* obj, BlastedTargetSetup* setup);
void blasted_release(void);
void blasted_initialise(void);

extern ObjectDescriptor gBlastedObjDescriptor;

#endif /* MAIN_DLL_DLL_0159_BLASTED_H_ */
