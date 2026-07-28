#ifndef DLLS_OBJECTS_370_LINKB_LEVCO_H_
#define DLLS_OBJECTS_370_LINKB_LEVCO_H_

#include "dlls/object_descriptor.h"
#include "game/objects/object_fwd.h"
#include "dlls/objects/430_SH_LevelCon.h"

typedef enum LINKBLevelControlStage {
    LINKB_LEVEL_CONTROL_STAGE_START = 0,
    LINKB_LEVEL_CONTROL_STAGE_1 = 1,
    LINKB_LEVEL_CONTROL_STAGE_2 = 2,
    LINKB_LEVEL_CONTROL_STAGE_3 = 3,
    LINKB_LEVEL_CONTROL_STAGE_4 = 4,
    LINKB_LEVEL_CONTROL_STAGE_5 = 5,
} LINKBLevelControlStage;

typedef struct LINKBLevelControlState {
    SCGameBitLatchState gameBitLatch;
    s8 trickyHitCount : 2;
    u8 stage : 3;
    u8 unusedStageBits : 3;
    u8 alternatePath : 1;
    u8 unusedFlags : 7;
    u8 pad06[0x02];
    f32 trickyTalkTimer;
    s16 musicTriggerId;
    u8 pad0E[0x02];
} LINKBLevelControlState;

typedef struct LINKBLevelControlEnvFxRampTables {
    s16 groupA[28];
    s16 groupB[28];
    s16 groupC[28];
    s16 groupD[28];
} LINKBLevelControlEnvFxRampTables;

STATIC_ASSERT(offsetof(LINKBLevelControlState, gameBitLatch) == 0x00);
STATIC_ASSERT(offsetof(LINKBLevelControlState, pad06) == 0x06);
STATIC_ASSERT(offsetof(LINKBLevelControlState, trickyTalkTimer) == 0x08);
STATIC_ASSERT(offsetof(LINKBLevelControlState, musicTriggerId) == 0x0C);
STATIC_ASSERT(offsetof(LINKBLevelControlState, pad0E) == 0x0E);
STATIC_ASSERT(sizeof(LINKBLevelControlState) == 0x10);

STATIC_ASSERT(offsetof(LINKBLevelControlEnvFxRampTables, groupA) == 0x00);
STATIC_ASSERT(offsetof(LINKBLevelControlEnvFxRampTables, groupB) == 0x38);
STATIC_ASSERT(offsetof(LINKBLevelControlEnvFxRampTables, groupC) == 0x70);
STATIC_ASSERT(offsetof(LINKBLevelControlEnvFxRampTables, groupD) == 0xA8);
STATIC_ASSERT(sizeof(LINKBLevelControlEnvFxRampTables) == 0xE0);

int linkbLevelControl_getExtraSize(void);
void linkbLevelControl_update(GameObject* obj);
void linkbLevelControl_init(GameObject* obj);

extern LINKBLevelControlEnvFxRampTables gLINKBLevelControlEnvFxRampTables;
extern ObjectDescriptor gLINKBLevelControlObjDescriptor;

#endif /* DLLS_OBJECTS_370_LINKB_LEVCO_H_ */
