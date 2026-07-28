#ifndef DLLS_OBJECTS_507_H_
#define DLLS_OBJECTS_507_H_

#include "dlls/object_descriptor.h"
#include "game/objects/object_fwd.h"
#include "game/objects/object_setup.h"
#include "main/objanim_update.h"

/*
 * Only the placement prefix consumed by this DLL is modeled. The active-target
 * retail placement width has not been established, so this does not claim a
 * complete record size.
 */
typedef struct Dll507PlacementView {
    ObjPlacement base;
    s8 rotationXHighByte;
    s8 baseMove;
    s16 triggerMode;
    s16 rotationY;
} Dll507PlacementView;

/* dll507_getExtraSize() establishes the complete allocation size. */
typedef struct Dll507State {
    u8 unknown00[0x04];
    s16 baseMove;
    s16 triggerMode;
    u8 unknown08;
    u8 hideModel;
    u8 unknown0A[0x02];
} Dll507State;

STATIC_ASSERT(offsetof(Dll507PlacementView, base) == 0x00);
STATIC_ASSERT(offsetof(Dll507PlacementView, rotationXHighByte) == 0x18);
STATIC_ASSERT(offsetof(Dll507PlacementView, baseMove) == 0x19);
STATIC_ASSERT(offsetof(Dll507PlacementView, triggerMode) == 0x1A);
STATIC_ASSERT(offsetof(Dll507PlacementView, rotationY) == 0x1C);

STATIC_ASSERT(offsetof(Dll507State, unknown00) == 0x00);
STATIC_ASSERT(offsetof(Dll507State, baseMove) == 0x04);
STATIC_ASSERT(offsetof(Dll507State, triggerMode) == 0x06);
STATIC_ASSERT(offsetof(Dll507State, unknown08) == 0x08);
STATIC_ASSERT(offsetof(Dll507State, hideModel) == 0x09);
STATIC_ASSERT(offsetof(Dll507State, unknown0A) == 0x0A);
STATIC_ASSERT(sizeof(Dll507State) == 0x0C);

int dll507_processAnimEvents(GameObject* obj, int unused, ObjAnimUpdateState* animUpdate);
int dll507_getExtraSize(void);
int dll507_getObjectTypeId(void);
void dll507_free(void);
void dll507_render(GameObject* obj, int renderArg2, int renderArg3, int renderArg4, int renderArg5, s8 visible);
void dll507_hitDetect(void);
void dll507_update(GameObject* obj);
void dll507_init(GameObject* obj, const Dll507PlacementView* placement);
void dll507_release(void);
void dll507_initialise(void);

extern ObjectDescriptor gDll507ObjDescriptor;

#endif /* DLLS_OBJECTS_507_H_ */
