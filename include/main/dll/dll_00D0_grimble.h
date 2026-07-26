#ifndef MAIN_DLL_DLL_00D0_GRIMBLE_H_
#define MAIN_DLL_DLL_00D0_GRIMBLE_H_

#include "game/objects/object.h"
#include "dlls/object_descriptor.h"

int grimble_stateHandlerA00(GameObject* obj, char* state, f32 arg);
int grimble_stateHandlerA01(GameObject* obj, char* state, f32 arg);
int grimble_stateHandlerA02(GameObject* obj, char* state, f32 arg);

int grimble_getExtraSize(void);
int grimble_getObjectTypeId(void);
void grimble_free(GameObject* obj);
void grimble_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible);
void grimble_hitDetect(int obj);
void grimble_update(GameObject* obj);
void grimble_init(int obj, int p2, int p3);
void grimble_release(void);
void grimble_initialise(void);
void grimble_initialiseStateHandlerTables(void);

extern ObjectDescriptor gGrimbleObjDescriptor;

#endif /* MAIN_DLL_DLL_00D0_GRIMBLE_H_ */
