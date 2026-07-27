#ifndef MAIN_DLL_DLL_0016_SCREENTRANSITION_H_
#define MAIN_DLL_DLL_0016_SCREENTRANSITION_H_

#include "game/objects/object.h"
#include "main/screen_transition.h"
#include "dolphin/gx/GXCull.h"

void screenTransition_drawWhiteWipe(int p1, int p2, int p3, u8 r, u8 g, u8 b);
void screenTransition_holdThenFadeIn(int duration, int type);

#endif
