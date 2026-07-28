#ifndef MAIN_DLL_FIREPIPE_H_
#define MAIN_DLL_FIREPIPE_H_

#include "ghidra_import.h"
#include "game/objects/object.h"
#include "dlls/object_descriptor.h"
#include "game/objects/object_setup.h"
#include "main/objanim_internal.h"
#include "main/modellight_api.h"
#include "main/dll/firepipe_effect_api.h"

typedef struct FirePipeExtra {
    GameObject* effectObjs[8];
    u8 effectCount;
    u8 pad21[0x24 - 0x21];
    f32 cycleTimer;
    f32 emitTimer;
    ModelLightStruct* glowLight;
    int activeSpawn;
    int effectType;
    f32 effectScale;
    s16 clearVolumeA;
    s16 clearVolumeB;
    u8 effectMode;
    u8 flags;
    u8 pad42[0x44 - 0x42];
} FirePipeExtra;

typedef struct FirePipeMapData {
    ObjPlacement base;
    s8 rotX;
    u8 rotY;
    s16 cycleTime;
    s16 scale;
    s16 gameBit;
    s16 startOffset;
    u8 flags;
    u8 pad23;
} FirePipeMapData;

STATIC_ASSERT(offsetof(FirePipeMapData, rotX) == 0x18);
STATIC_ASSERT(offsetof(FirePipeMapData, rotY) == 0x19);
STATIC_ASSERT(offsetof(FirePipeMapData, cycleTime) == 0x1A);
STATIC_ASSERT(offsetof(FirePipeMapData, scale) == 0x1C);
STATIC_ASSERT(offsetof(FirePipeMapData, gameBit) == 0x1E);
STATIC_ASSERT(offsetof(FirePipeMapData, startOffset) == 0x20);
STATIC_ASSERT(offsetof(FirePipeMapData, flags) == 0x22);
STATIC_ASSERT(sizeof(FirePipeMapData) == 0x24);

int firepipe_spawnEffectObject(FirePipeExtra *extra, GameObject *obj, ObjPlacement *spawnDef);
int firepipe_clearLinkedUpdateFlag(GameObject *obj);
int firepipe_setLinkedUpdateFlag(GameObject *obj);
void firepipe_updateState(GameObject *obj);
int firepipe_getExtraSize(void);
u32 firepipe_stateCallback(GameObject *obj);
int firepipe_getObjectTypeId(void);
void firepipe_free(GameObject *obj);
void firepipe_render(GameObject *obj, int param_2, int param_3, int param_4, int param_5, char param_6);
void firepipe_update(GameObject *obj);
void firepipe_init(GameObject *obj, FirePipeMapData *mapData);

extern ObjectDescriptor gFirePipeObjDescriptor;

#endif /* MAIN_DLL_FIREPIPE_H_ */
