#ifndef DLLS_OBJECTS_430_SH_LEVEL_CON_H_
#define DLLS_OBJECTS_430_SH_LEVEL_CON_H_

#include "dlls/object_descriptor.h"
#include "game/objects/object_fwd.h"

typedef struct SCGameBitLatchState {
    int activeMask;
} SCGameBitLatchState;

typedef struct ShLevelControlState {
    union {
        u32 flags;
        s32 storyFlags;
    };
    union {
        u8 waitCounter;
        u8 earlySceneDelay;
    };
    u8 mapAct;
    union {
        u8 bloopEventState;
        u8 thornTailState;
    };
    u8 unknown07;
    union {
        struct {
            f32 airMeterTimer;
            f32 hudTextTimer;
            s16 dayNightMusicLatch;
            s16 musicLatch;
        };
        struct {
            u8 unknown08[0x0A];
            s16 mapOverride;
        };
    };
} ShLevelControlState;

STATIC_ASSERT(sizeof(SCGameBitLatchState) == 0x04);
STATIC_ASSERT(offsetof(SCGameBitLatchState, activeMask) == 0x00);

STATIC_ASSERT(sizeof(ShLevelControlState) == 0x14);
STATIC_ASSERT(offsetof(ShLevelControlState, flags) == 0x00);
STATIC_ASSERT(offsetof(ShLevelControlState, storyFlags) == 0x00);
STATIC_ASSERT(offsetof(ShLevelControlState, waitCounter) == 0x04);
STATIC_ASSERT(offsetof(ShLevelControlState, mapAct) == 0x05);
STATIC_ASSERT(offsetof(ShLevelControlState, bloopEventState) == 0x06);
STATIC_ASSERT(offsetof(ShLevelControlState, unknown07) == 0x07);
STATIC_ASSERT(offsetof(ShLevelControlState, airMeterTimer) == 0x08);
STATIC_ASSERT(offsetof(ShLevelControlState, hudTextTimer) == 0x0C);
STATIC_ASSERT(offsetof(ShLevelControlState, dayNightMusicLatch) == 0x10);
STATIC_ASSERT(offsetof(ShLevelControlState, musicLatch) == 0x12);
STATIC_ASSERT(offsetof(ShLevelControlState, earlySceneDelay) == 0x04);
STATIC_ASSERT(offsetof(ShLevelControlState, thornTailState) == 0x06);
STATIC_ASSERT(offsetof(ShLevelControlState, unknown08) == 0x08);
STATIC_ASSERT(offsetof(ShLevelControlState, mapOverride) == 0x12);

typedef struct SkyEnvFxRampTables {
    s16 groupA[28];
    s16 groupB[28];
    s16 groupC[28];
    s16 groupD[28];
} SkyEnvFxRampTables;

STATIC_ASSERT(offsetof(SkyEnvFxRampTables, groupA) == 0x00);
STATIC_ASSERT(offsetof(SkyEnvFxRampTables, groupB) == 0x38);
STATIC_ASSERT(offsetof(SkyEnvFxRampTables, groupC) == 0x70);
STATIC_ASSERT(offsetof(SkyEnvFxRampTables, groupD) == 0xA8);
STATIC_ASSERT(sizeof(SkyEnvFxRampTables) == 0xE0);

struct SCTotemLogPuzzleUpdateState;

int SH_LevelControl_getExtraSize(void);
void SH_LevelControl_free(void);
int SH_LevelControl_sequenceCallback(void* obj, void* unused, struct SCTotemLogPuzzleUpdateState* updateState);
void SH_LevelControl_updateTotemPuzzleMapState(void* obj, void* state);
void SCGameBitLatch_Update(SCGameBitLatchState* state, int mask, s16 clearIfSetBit, s16 clearIfClearBit, s16 latchBit,
                           int musicId);
void SCGameBitLatch_UpdateInverted(SCGameBitLatchState* state, int mask, s16 clearIfSetBit, s16 clearIfClearBit,
                                   s16 latchBit, int musicId);
void SH_LevelControl_update(GameObject* obj);
void SH_LevelControl_init(GameObject* obj);

extern ObjectDescriptor gSH_LevelControlObjDescriptor;

#endif /* DLLS_OBJECTS_430_SH_LEVEL_CON_H_ */
