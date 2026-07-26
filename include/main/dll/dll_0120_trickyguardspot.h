#ifndef MAIN_DLL_DLL_0120_TRICKYGUARDSPOT_H_
#define MAIN_DLL_DLL_0120_TRICKYGUARDSPOT_H_

#include "game/objects/object_setup.h"
#include "main/objanim_internal.h"
#include "dlls/object_descriptor.h"
#include "game/objects/object.h"

#define TRICKY_GUARD_SPOT_DLL_ID             0x0120
#define TRICKY_GUARD_SPOT_CLASS_ID           0x0030
#define TRICKY_GUARD_SPOT_DEF_ID             0x04C6
#define TRICKY_GUARD_SPOT_OBJECT_DEF_BYTES   0xC0
#define TRICKY_GUARD_SPOT_PLACEMENT_BYTES    0x24
#define TRICKY_GUARD_SPOT_EXTRA_STATE_BYTES  0x08
#define TRICKY_GUARD_SPOT_GROUP              0x1E
#define TRICKY_GUARD_SPOT_ACTION             1
#define TRICKY_GUARD_SPOT_ACTION_PARAM       3
typedef GameObject TrickyGuardSpotObject;

/* Tricky object vtable subset used by guard-action objects. */
typedef struct TrickyGuardSpotInterfaceVTable
{
    void* pad00[10];
    void (*sideCommandEnable)(GameObject* tricky, TrickyGuardSpotObject* obj, int commandKind, int commandType);
    void* pad2C[4];
    void (*requestRecall)(GameObject* tricky);
    void* pad40;
    int (*isBusy)(GameObject* tricky);
} TrickyGuardSpotInterfaceVTable;

STATIC_ASSERT(offsetof(TrickyGuardSpotInterfaceVTable, sideCommandEnable) == 0x28);
STATIC_ASSERT(offsetof(TrickyGuardSpotInterfaceVTable, requestRecall) == 0x3C);
STATIC_ASSERT(offsetof(TrickyGuardSpotInterfaceVTable, isBusy) == 0x44);

typedef struct TrickyGuardSpotPlacement
{
    ObjPlacement base;
    s8 initialYaw;
    u8 resetSeconds;
    s16 triggerRadius;
    u8 pad1C[2];
    s16 rangeGameBit;
    u8 pad20[4];
} TrickyGuardSpotPlacement;

typedef struct TrickyGuardSpotStateFlags
{
    u8 trickyInRange : 1;
    u8 flags : 7;
} TrickyGuardSpotStateFlags;

typedef struct TrickyGuardSpotState
{
    u32 resetTimer;
    TrickyGuardSpotStateFlags flags;
    u8 pad05[3];
} TrickyGuardSpotState;

STATIC_ASSERT(sizeof(TrickyGuardSpotPlacement) == TRICKY_GUARD_SPOT_PLACEMENT_BYTES);
STATIC_ASSERT(offsetof(TrickyGuardSpotPlacement, initialYaw) == 0x18);
STATIC_ASSERT(offsetof(TrickyGuardSpotPlacement, triggerRadius) == 0x1A);
STATIC_ASSERT(offsetof(TrickyGuardSpotPlacement, rangeGameBit) == 0x1E);
STATIC_ASSERT(sizeof(TrickyGuardSpotState) == TRICKY_GUARD_SPOT_EXTRA_STATE_BYTES);

extern ObjectDescriptor gTrickyGuardSpotObjDescriptor;

int TrickyGuardSpot_getExtraSize(void);
void TrickyGuardSpot_free(TrickyGuardSpotObject* obj);
void TrickyGuardSpot_render(void);
void TrickyGuardSpot_update(TrickyGuardSpotObject* obj);
void TrickyGuardSpot_init(TrickyGuardSpotObject* obj, TrickyGuardSpotPlacement* def);

#endif /* MAIN_DLL_DLL_0120_TRICKYGUARDSPOT_H_ */
