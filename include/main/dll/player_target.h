#ifndef MAIN_DLL_PLAYER_TARGET_H_
#define MAIN_DLL_PLAYER_TARGET_H_

#include "game/objects/object.h"
int fn_80296118(GameObject* playerObj);

static inline int Player_GetTargetObject(int playerObj)
{
    return fn_80296118((GameObject*)(playerObj));
}

#endif
