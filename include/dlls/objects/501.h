#ifndef DLLS_OBJECTS_501_H_
#define DLLS_OBJECTS_501_H_

#include "dlls/object_descriptor.h"
#include "game/objects/object_fwd.h"
#include "game/objects/object_setup.h"
#include "main/objseq.h"

/*
 * Only the placement prefix consumed by this DLL is modeled. The active-target
 * retail placement width is not available, so this does not claim a complete
 * record size.
 */
typedef struct Dll501PlacementView {
    ObjPlacement base;
    s16 segmentIndex;
    s16 gameBit;
    u8 unknown1C[0x08];
    u8 dampingDivisor;
} Dll501PlacementView;

/*
 * The object-trigger interface operates on the common ObjSeqState prefix.
 * This DLL's allocation contract proves eight additional opaque bytes.
 */
typedef struct Dll501State {
    ObjSeqState sequence;
    u8 unknown138[0x08];
} Dll501State;

STATIC_ASSERT(offsetof(Dll501PlacementView, base) == 0x00);
STATIC_ASSERT(offsetof(Dll501PlacementView, segmentIndex) == 0x18);
STATIC_ASSERT(offsetof(Dll501PlacementView, gameBit) == 0x1A);
STATIC_ASSERT(offsetof(Dll501PlacementView, unknown1C) == 0x1C);
STATIC_ASSERT(offsetof(Dll501PlacementView, dampingDivisor) == 0x24);

STATIC_ASSERT(offsetof(Dll501State, sequence) == 0x00);
STATIC_ASSERT(offsetof(Dll501State, unknown138) == 0x138);
STATIC_ASSERT(sizeof(Dll501State) == 0x140);

int dll501_getExtraSize(void);
int dll501_getObjectTypeId(void);
void dll501_free(GameObject* obj);
void dll501_render(GameObject* obj, int renderArg2, int renderArg3, int renderArg4, int renderArg5, s8 unusedVisible);
void dll501_hitDetect(void);
void dll501_update(GameObject* obj);
void dll501_init(GameObject* obj, int placement);
void dll501_release(void);
void dll501_initialise(void);

extern ObjectDescriptor gDll501ObjDescriptor;

#endif /* DLLS_OBJECTS_501_H_ */
