#ifndef MAIN_DLL_DLL_00C4_TRICKY_H_
#define MAIN_DLL_DLL_00C4_TRICKY_H_

#include "game/objects/object.h"

#define TRICKY_ITEM_ID_COUNT 5

typedef struct TrickyItemIdList
{
    s32 ids[TRICKY_ITEM_ID_COUNT];
} TrickyItemIdList;

STATIC_ASSERT(sizeof(TrickyItemIdList) == 0x14);

extern const TrickyItemIdList gTrickyCmdQueryInit;
extern const TrickyItemIdList gTrickyFoodItemIds;
#include "ghidra_import.h"
#include "main/dll/tricky_state.h"
#include "main/objanim_update.h"
#include "dlls/object_descriptor.h"

extern ObjectDescriptor20WithPadding gTrickyObjDescriptor;

/* gTrickyObjDescriptor from slot02 onwards: the export table other objects reach through
   obj->anim.dll. */
typedef struct TrickyCompanionInterface
{
    void* pad00[8];
    int (*getAvailableCommands)(GameObject* tricky);
    int (*updateSideCommandPrompts)(GameObject* tricky);
    void (*sideCommandEnable)(GameObject* tricky, GameObject* target, int commandKind, int commandType);
    u8 (*getEnergy)(GameObject* tricky);
    u8 (*getEnergyMax)(GameObject* tricky);
    void (*commandPlayBall)(GameObject* tricky, int enabled, GameObject* target);
    int (*requestMoveToObject)(GameObject* tricky, GameObject* target);
    void (*requestRecall)(GameObject* tricky);
    u8 (*isPlayingBall)(GameObject* tricky);
    int (*isGuarding)(GameObject* tricky);
    int (*getCurrentCommandType)(GameObject* tricky, int* commandType);
} TrickyCompanionInterface;

STATIC_ASSERT(offsetof(TrickyCompanionInterface, getAvailableCommands) == 0x20);
STATIC_ASSERT(offsetof(TrickyCompanionInterface, updateSideCommandPrompts) == 0x24);
STATIC_ASSERT(offsetof(TrickyCompanionInterface, sideCommandEnable) == 0x28);
STATIC_ASSERT(offsetof(TrickyCompanionInterface, getEnergy) == 0x2C);
STATIC_ASSERT(offsetof(TrickyCompanionInterface, getEnergyMax) == 0x30);
STATIC_ASSERT(offsetof(TrickyCompanionInterface, commandPlayBall) == 0x34);
STATIC_ASSERT(offsetof(TrickyCompanionInterface, requestMoveToObject) == 0x38);
STATIC_ASSERT(offsetof(TrickyCompanionInterface, requestRecall) == 0x3C);
STATIC_ASSERT(offsetof(TrickyCompanionInterface, isPlayingBall) == 0x40);
STATIC_ASSERT(offsetof(TrickyCompanionInterface, isGuarding) == 0x44);
STATIC_ASSERT(offsetof(TrickyCompanionInterface, getCurrentCommandType) == 0x48);

#define TRICKY_INTERFACE(tricky) ((TrickyCompanionInterface*)*((GameObject*)(tricky))->anim.dll)

void trickyReportError(const char* fmt, ...);
void trickyDebugPrint(const char* fmt, ...);
u8* Tricky_findNearestGroup4BObject(u8* obj, TrickyState* state);
void FUN_80145230(u64 param_1, u64 param_2, double param_3, u64 param_4, u64 param_5, u64 param_6, u64 param_7,
                  u64 param_8, int param_9, int* param_10, int param_11, u32 param_12, u8 param_13, u32 param_14,
                  u32 param_15, u32 param_16);
void FUN_801455e8(u64 param_1, double param_2, double param_3, u64 param_4, u64 param_5, u64 param_6, u64 param_7,
                  u64 param_8, int param_9, int param_10);
void FUN_801457a4(u64 param_1, double param_2, double param_3, u64 param_4, u64 param_5, u64 param_6, u64 param_7,
                  u64 param_8, u32 param_9, u32 param_10, int param_11, u32 param_12, u32 param_13, u32 param_14,
                  u32 param_15, u32 param_16);
void tricky_attachToWalkGroup(GameObject* obj, int state);
void tricky_stateIdleWander(GameObject* obj, int state);
int Tricky_requestMoveToObject(int* obj, int targetObj);
void Tricky_commandPlayBall(int* obj, int commandEnabled, int targetObj);
void sideCommandEnable(GameObject* obj, GameObject* targetObj, int commandKind, int commandType);
int Tricky_updateSideCommandPrompts(int obj);
void Tricky_free(int obj, int shouldKeepFlameChildren);
void Tricky_init(GameObject* obj);
int tricky_SeqFn(int obj, int unused, ObjAnimUpdateState* animUpdate);
void Tricky_update(int obj);
void tricky_handleDefeat(GameObject* obj, int state);
void baddie_updateWhileFrozen(GameObject* obj, u8* state, u8 fromHit);
int baddie_spawnRewardDrops(GameObject* obj, int state, int spawnBits, u32 useAltMode, u32 mode);
u8 baddie_canSeeTarget(GameObject* obj, TrickyState* state, void* from, void* to);
void baddie_updateSightQuadrants(GameObject* obj, TrickyState* state, float radius);
void Tricky_render(GameObject* obj, int param_2, int param_3, int param_4, int param_5, char doRender);
void Tricky_hitDetect(GameObject* obj);
void FUN_80146f94(u64 param_1, double param_2, double param_3, u64 param_4, u64 param_5, u64 param_6, u64 param_7,
                  u64 param_8);
void baddieInstantiateWeapon(GameObject* obj, int state);
void FUN_80146fa4(u64 param_1, double param_2, double param_3, u64 param_4, u64 param_5, u64 param_6, u64 param_7,
                  u64 param_8, u32 param_9, int param_10, u32 param_11, u32 param_12, u32 param_13, u32 param_14,
                  u32 param_15, u32 param_16);
void FUN_80147218(u64 param_1, u64 param_2, double param_3, u64 param_4, u64 param_5, u64 param_6, u64 param_7,
                  u64 param_8, u32 param_9, int param_10, u32 param_11, u32 param_12, u32 param_13, u32 param_14,
                  u32 param_15, u32 param_16);
void FUN_8014721c(u64 param_1, u64 param_2, double param_3, u64 param_4, u64 param_5, u64 param_6, u64 param_7,
                  u64 param_8, u32 param_9, u32 param_10, u32 param_11);
void FUN_80147314(u64 param_1, double param_2, double param_3, u64 param_4, u64 param_5, u64 param_6, u64 param_7,
                  u64 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13, u32 param_14,
                  u32 param_15, u32 param_16);
void FUN_801476cc(u64 param_1, double param_2, double param_3, u64 param_4, u64 param_5, u64 param_6, u64 param_7,
                  u64 param_8, int param_9, int param_10);
void FUN_80147884(u64 param_1, u64 param_2, u64 param_3, u64 param_4, u64 param_5, u64 param_6, u64 param_7,
                  u64 param_8, u32 param_9, u32 param_10, float* param_11, float* param_12);
void FUN_80147a70(u64 param_1, u64 param_2, u64 param_3, u64 param_4, u64 param_5, u64 param_6, u64 param_7,
                  u64 param_8);
int Tricky_getExtraSize(void);
u8 Tricky_getEnergyMax(int* obj);
u8 Tricky_getEnergy(int* obj);
int Tricky_getCurrentCommandType(int* obj, int* out);
void Tricky_requestRecall(int* obj);
int Tricky_func13(int* obj);
int Tricky_isPlayingBall(int* obj);
int Tricky_getAvailableCommands(void);

#endif /* MAIN_DLL_DLL_00C4_TRICKY_H_ */
