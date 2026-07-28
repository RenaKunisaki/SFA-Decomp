#ifndef MAIN_OBJPRINT_ANIM_API_H_
#define MAIN_OBJPRINT_ANIM_API_H_

#include "global.h"
#include "game/objects/object.h"

void objSoundUpdateMouth(GameObject* obj, char* state);
void characterHeadLookCalm(GameObject* obj, s16* state, f32 value);
void characterHeadLookAlert(int obj, s16* curve, s16* state, f32 val);

#endif /* MAIN_OBJPRINT_ANIM_API_H_ */
