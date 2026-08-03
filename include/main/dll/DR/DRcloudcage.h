#ifndef MAIN_DLL_DR_DRCLOUDCAGE_H_
#define MAIN_DLL_DR_DRCLOUDCAGE_H_

#include "game/objects/object.h"
#include "types.h"

void drcloudcage_updateTrails(GameObject* obj, int state);
void drcloudcage_updateEngineFx(GameObject* obj, void* state, f32 distanceScale, int intensity, u8* unused,
                                u8 channelFlags);
f32 drcloudcage_getRouteIntensity(GameObject* obj, int state);

#endif /* MAIN_DLL_DR_DRCLOUDCAGE_H_ */
