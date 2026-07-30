#ifndef MAIN_DLL_SP_DLL_0286_SPSHOPKEEPER_H_
#define MAIN_DLL_SP_DLL_0286_SPSHOPKEEPER_H_

#include "game/objects/object.h"
#include "main/objseq.h"

int ShopKeeper_SeqFn(GameObject* obj, int p2, ObjSeqState* seq, s8 advance);
f32 ShopKeeper_turnTowardPlayer(GameObject* obj, GameObject* player, int snap);

#endif
