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

void DFropenode_setMinY(GameObject* obj, f32 value);
int DFropenode_isVisible(GameObject* obj);
void DFropenode_setVisible(GameObject* obj, int visible);
int DFropenode_getAngle(GameObject* obj);
void DFropenode_clearLinkedObj(GameObject* obj);
int DFropenode_findNearestRopePoint(GameObject* obj, f32 worldX, f32 worldY, f32 worldZ, f32* distanceOut,
                                    f32* phaseOut, u8* sideOut);
void DFropenode_applyForceAtPhase(f32 phase, f32 force, GameObject* obj);
void DFropenode_advancePhaseByDistance(GameObject* obj, f32* phase, f32 distance);
void DFropenode_getWorldPosAtPhase(f32 phase, GameObject* obj, f32* xOut, f32* yOut, f32* zOut);
void DFropenode_getPlaneEquation(GameObject* obj, f32* out);
int DFropenode_syncRopeToEndpoints(GameObject* obj);
int DFropenode_getExtraSize(void);
int DFropenode_getObjectTypeId(void);
void DFropenode_free(GameObject* obj);
void DFropenode_render(GameObject* obj, int p2, int p3);
void DFropenode_hitDetect(void);
void DFropenode_update(GameObject* obj);
void DFropenode_init(GameObject* obj, DFropenodePlacement* placement);
void DFropenode_release(void);
void DFropenode_initialise(void);

extern ObjectDescriptor20 gDFropenodeObjDescriptor;

#endif /* DLLS_OBJECTS_373_DFROPENODE_H_ */
