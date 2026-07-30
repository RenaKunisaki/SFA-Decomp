#ifndef DLLS_OBJECTS_373_DFROPENODE_H_
#define DLLS_OBJECTS_373_DFROPENODE_H_

#include "dlls/object_descriptor.h"
#include "game/objects/object_fwd.h"
#include "game/objects/object_setup.h"

#define DFROPENODE_ROLE_ROPE_OWNER 0x01

typedef struct DFRope DFRope;

typedef struct DFropenodePlacement {
    ObjPlacement base;
    u8 roleFlags;
    u8 pad19[0x1B - 0x19];
    u8 variant;
    s16 fadeGameBit;
    u8 pad1E[0x20 - 0x1E];
} DFropenodePlacement;

typedef struct DFropenodeState {
    GameObject* linkedNode;
    f32 boundsMinX;
    f32 boundsMaxX;
    f32 boundsMinZ;
    f32 boundsMaxZ;
    f32 minimumY;
    s16 ropeYaw;
    u8 pad1A[2];
    f32 planeNormalX;
    f32 planeNormalY;
    f32 planeNormalZ;
    f32 planeDistance;
    DFRope* rope;
    u8 hidden : 1;
    u8 pad30 : 7;
    u8 pad31[3];
} DFropenodeState;

STATIC_ASSERT(offsetof(DFropenodePlacement, base) == 0x00);
STATIC_ASSERT(offsetof(DFropenodePlacement, roleFlags) == 0x18);
STATIC_ASSERT(offsetof(DFropenodePlacement, pad19) == 0x19);
STATIC_ASSERT(offsetof(DFropenodePlacement, variant) == 0x1B);
STATIC_ASSERT(offsetof(DFropenodePlacement, fadeGameBit) == 0x1C);
STATIC_ASSERT(offsetof(DFropenodePlacement, pad1E) == 0x1E);
STATIC_ASSERT(sizeof(DFropenodePlacement) == 0x20);

STATIC_ASSERT(offsetof(DFropenodeState, linkedNode) == 0x00);
STATIC_ASSERT(offsetof(DFropenodeState, boundsMinX) == 0x04);
STATIC_ASSERT(offsetof(DFropenodeState, boundsMaxX) == 0x08);
STATIC_ASSERT(offsetof(DFropenodeState, boundsMinZ) == 0x0C);
STATIC_ASSERT(offsetof(DFropenodeState, boundsMaxZ) == 0x10);
STATIC_ASSERT(offsetof(DFropenodeState, minimumY) == 0x14);
STATIC_ASSERT(offsetof(DFropenodeState, ropeYaw) == 0x18);
STATIC_ASSERT(offsetof(DFropenodeState, pad1A) == 0x1A);
STATIC_ASSERT(offsetof(DFropenodeState, planeNormalX) == 0x1C);
STATIC_ASSERT(offsetof(DFropenodeState, planeNormalY) == 0x20);
STATIC_ASSERT(offsetof(DFropenodeState, planeNormalZ) == 0x24);
STATIC_ASSERT(offsetof(DFropenodeState, planeDistance) == 0x28);
STATIC_ASSERT(offsetof(DFropenodeState, rope) == 0x2C);
STATIC_ASSERT(offsetof(DFropenodeState, pad31) == 0x31);
STATIC_ASSERT(sizeof(DFropenodeState) == 0x34);

void dfropenode_setMinY(GameObject* obj, f32 value);
int dfropenode_isVisible(GameObject* obj);
void dfropenode_setVisible(GameObject* obj, int visible);
int dfropenode_getAngle(GameObject* obj);
void dfropenode_clearLinkedObj(GameObject* obj);
int dfropenode_findNearestRopePoint(GameObject* obj, f32 worldX, f32 worldY, f32 worldZ, f32* distanceOut,
                                    f32* phaseOut, u8* sideOut);
void dfropenode_applyForceAtPhase(f32 phase, f32 force, GameObject* obj);
void dfropenode_advancePhaseByDistance(GameObject* obj, f32* phase, f32 distance);
void dfropenode_getWorldPosAtPhase(f32 phase, GameObject* obj, f32* xOut, f32* yOut, f32* zOut);
void dfropenode_getPlaneEquation(GameObject* obj, f32* out);
int dfropenode_syncRopeToEndpoints(GameObject* obj);
int dfropenode_getExtraSize(void);
int dfropenode_getObjectTypeId(void);
void dfropenode_free(GameObject* obj);
void dfropenode_render(GameObject* obj, int p2, int p3);
void dfropenode_hitDetect(void);
void dfropenode_update(GameObject* obj);
void dfropenode_init(GameObject* obj, DFropenodePlacement* placement);
void dfropenode_release(void);
void dfropenode_initialise(void);

extern ObjectDescriptor20 gDFropenodeObjDescriptor;

#endif /* DLLS_OBJECTS_373_DFROPENODE_H_ */
