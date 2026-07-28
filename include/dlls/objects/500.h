#ifndef DLLS_OBJECTS_500_H_
#define DLLS_OBJECTS_500_H_

#include "dlls/object_descriptor.h"
#include "game/objects/object_fwd.h"
#include "game/objects/object_setup.h"
#include "main/objanim_update.h"

/* dll500_getExtraSize() allocates this complete 0x01-byte state. */
typedef struct Dll500State {
    u8 active;
} Dll500State;

/*
 * Only the placement prefix consumed by this DLL is modeled. The active-target
 * retail placement width is not yet evidenced.
 */
typedef struct Dll500PlacementView {
    ObjPlacement base;
    s8 rotXSwing;
    u8 unknown19;
    u8 rotXStatic;
} Dll500PlacementView;

STATIC_ASSERT(offsetof(Dll500State, active) == 0x00);
STATIC_ASSERT(sizeof(Dll500State) == 0x01);

STATIC_ASSERT(offsetof(Dll500PlacementView, base) == 0x00);
STATIC_ASSERT(offsetof(Dll500PlacementView, rotXSwing) == 0x18);
STATIC_ASSERT(offsetof(Dll500PlacementView, unknown19) == 0x19);
STATIC_ASSERT(offsetof(Dll500PlacementView, rotXStatic) == 0x1A);

int dll500_getExtraSize(void);
void dll500_free(GameObject* obj);
void dll500_render(GameObject* obj, int renderArg2, int renderArg3, int renderArg4, int renderArg5, s8 visible);
int dll500_processAnimEvents(GameObject* obj, int unused, ObjAnimUpdateState* animUpdate);
void dll500_update(int obj);
void dll500_init(GameObject* obj, const Dll500PlacementView* placement);

extern ObjectDescriptor gDll500ObjDescriptor;

#endif /* DLLS_OBJECTS_500_H_ */
