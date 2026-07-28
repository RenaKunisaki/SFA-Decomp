#ifndef DLLS_OBJECTS_371_LINK_LEVCON_H_
#define DLLS_OBJECTS_371_LINK_LEVCON_H_

#include "dlls/object_descriptor.h"
#include "game/objects/object_fwd.h"
#include "dlls/objects/430_SH_LevelCon.h"

typedef struct LINKLevelControlState {
    s8 previousPlayerAreaCell;
    u8 pad01[0x03];
    int unknown04;
    int musicTriggerId;
    SCGameBitLatchState musicLatch;
} LINKLevelControlState;

typedef struct LINKLevelControlEnvFxRampTables {
    s16 groupA[28];
    s16 groupB[28];
    s16 groupC[28];
    s16 groupD[28];
} LINKLevelControlEnvFxRampTables;

STATIC_ASSERT(offsetof(LINKLevelControlState, previousPlayerAreaCell) == 0x00);
STATIC_ASSERT(offsetof(LINKLevelControlState, pad01) == 0x01);
STATIC_ASSERT(offsetof(LINKLevelControlState, unknown04) == 0x04);
STATIC_ASSERT(offsetof(LINKLevelControlState, musicTriggerId) == 0x08);
STATIC_ASSERT(offsetof(LINKLevelControlState, musicLatch) == 0x0C);
STATIC_ASSERT(sizeof(LINKLevelControlState) == 0x10);

STATIC_ASSERT(offsetof(LINKLevelControlEnvFxRampTables, groupA) == 0x00);
STATIC_ASSERT(offsetof(LINKLevelControlEnvFxRampTables, groupB) == 0x38);
STATIC_ASSERT(offsetof(LINKLevelControlEnvFxRampTables, groupC) == 0x70);
STATIC_ASSERT(offsetof(LINKLevelControlEnvFxRampTables, groupD) == 0xA8);
STATIC_ASSERT(sizeof(LINKLevelControlEnvFxRampTables) == 0xE0);

void linkLevelControl_updateAreaMusic(GameObject* obj);
void linkLevelControl_applyEnterAreaEffects(GameObject* obj);
int linkLevelControl_getExtraSize(void);
void linkLevelControl_free(GameObject* obj);
void linkLevelControl_update(GameObject* obj);
void linkLevelControl_init(GameObject* obj);

extern LINKLevelControlEnvFxRampTables gLINKLevelControlEnvFxRampTables;
extern ObjectDescriptor gLINKLevelControlObjDescriptor;

#endif /* DLLS_OBJECTS_371_LINK_LEVCON_H_ */
