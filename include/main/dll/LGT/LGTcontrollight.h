#ifndef MAIN_DLL_LGT_LGTCONTROLLIGHT_H_
#define MAIN_DLL_LGT_LGTCONTROLLIGHT_H_

#include "main/dll/LGT/lgtcontrollightrec_struct.h"
#include "game/objects/object.h"


int firefly_animEventCallback(GameObject* obj);
void firefly_initFlightRec(GameObject* obj, LgtFireFlyRec* record);
void firefly_pickWanderTarget(GameObject* obj, LgtFireFlyRec* record);

#endif /* MAIN_DLL_LGT_LGTCONTROLLIGHT_H_ */
