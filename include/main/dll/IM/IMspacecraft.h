#ifndef MAIN_DLL_IM_IMSPACECRAFT_H_
#define MAIN_DLL_IM_IMSPACECRAFT_H_

#include "game/objects/object.h"
#include "ghidra_import.h"
#include "main/dll/curve_walker.h"

#define ROLLINGBARREL_EXTRA_SIZE  0x118

#define ROLLINGBARREL_GROUP_ID                0x2f
#define ROLLINGBARREL_SPECIAL_DESCRIPTOR_TYPE 0x72a

#define ROLLINGBARREL_STATE_ROLLING       0
#define ROLLINGBARREL_STATE_EXPLODED_WAIT 1
#define ROLLINGBARREL_STATE_RESPAWN_WAIT  2
#define ROLLINGBARREL_STATE_CLEANUP       3

typedef struct RollingBarrelState
{
    RomCurveWalker curve;
    f32 curveSpeed;
    f32 verticalSpeed;
    f32 timer;
    u8 state;
    u8 pitchRising;
    u8 hitVolumeSlot;
    u8 pad117;
} RollingBarrelState;

typedef struct RollingBarrelMapData
{
    s16 objectDefId;
    u8 pad02[0x08 - 0x02];
    f32 x;
    f32 y;
    f32 z;
    s32 respawnParam;
    u8 pad18[0x1A - 0x18];
    s16 verticalSpeed;
    s16 curveSpeed;
} RollingBarrelMapData;

STATIC_ASSERT(sizeof(RollingBarrelState) == ROLLINGBARREL_EXTRA_SIZE);
STATIC_ASSERT(offsetof(RollingBarrelState, curveSpeed) == 0x108);
STATIC_ASSERT(offsetof(RollingBarrelState, verticalSpeed) == 0x10C);
STATIC_ASSERT(offsetof(RollingBarrelState, timer) == 0x110);
STATIC_ASSERT(offsetof(RollingBarrelState, state) == 0x114);
STATIC_ASSERT(offsetof(RollingBarrelState, pitchRising) == 0x115);
STATIC_ASSERT(offsetof(RollingBarrelState, hitVolumeSlot) == 0x116);

STATIC_ASSERT(offsetof(RollingBarrelMapData, x) == 0x08);
STATIC_ASSERT(offsetof(RollingBarrelMapData, y) == 0x0C);
STATIC_ASSERT(offsetof(RollingBarrelMapData, z) == 0x10);
STATIC_ASSERT(offsetof(RollingBarrelMapData, respawnParam) == 0x14);
STATIC_ASSERT(offsetof(RollingBarrelMapData, verticalSpeed) == 0x1A);
STATIC_ASSERT(offsetof(RollingBarrelMapData, curveSpeed) == 0x1C);
STATIC_ASSERT(sizeof(RollingBarrelMapData) == 0x20);

void RollingBarrel_explode(GameObject* obj, int unusedExplosionVariant);
int RollingBarrel_getExtraSize(void);
int RollingBarrel_getObjectTypeId(void);
void RollingBarrel_free(GameObject* obj);
void RollingBarrel_render(GameObject* obj, int p1, int p2, int p3, int p4, s8 visible);
void RollingBarrel_hitDetect(void);
void RollingBarrel_update(GameObject* obj);
void RollingBarrel_init(GameObject* obj, RollingBarrelMapData* params);
void RollingBarrel_release(void);
void RollingBarrel_initialise(void);
#endif /* MAIN_DLL_IM_IMSPACECRAFT_H_ */
