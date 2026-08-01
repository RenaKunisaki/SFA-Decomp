#ifndef MAIN_DLL_DUSTER_H_
#define MAIN_DLL_DUSTER_H_

#include "main/dll/dll_00C9_enemy.h"
#include "game/objects/object.h"
#include "main/dll/duster_api.h"

void rachnopUpdateApproach(GameObject* obj, int state);
void rachnopUpdateAttack(GameObject* obj, int state);
void rachnopUpdateIdle(GameObject* obj, int state);
void spittingEbaUpdateIdle(GameObject* obj, int state);
void spittingEbaUpdateEngaged(GameObject* obj, int state);

void rachnopInit(u32 param_1,int param_2);
void spittingEbaSpawnPollen(GameObject* obj,int state);
void spittingEbaUpdateTimeOfDay(int param_9,int param_10);
void spittingEbaInit(u32 param_1,int param_2);
void wbInit(u32 param_1,int param_2);

enum
{
    DUSTER_WALL_PLANE_OFFSET = offsetof(EnemyState, wallPlane),
    DUSTER_WALL_NORMAL_X_OFFSET = offsetof(EnemyState, wallPlane.normal[0]),
    DUSTER_WALL_NORMAL_Y_OFFSET = offsetof(EnemyState, wallPlane.normal[1]),
    DUSTER_WALL_NORMAL_Z_OFFSET = offsetof(EnemyState, wallPlane.normal[2])
};

#endif /* MAIN_DLL_DUSTER_H_ */
