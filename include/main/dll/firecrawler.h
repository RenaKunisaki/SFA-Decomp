#ifndef MAIN_DLL_FIRECRAWLER_H_
#define MAIN_DLL_FIRECRAWLER_H_

#include "main/camera_interface.h"
#include "game/objects/object.h"
#include "main/model.h"
#include "main/modellight_api.h"
#include "main/gamebits.h"
#include "main/dll/baddie_state.h"
#include "main/dll/baddie_setmove.h"
#include "main/dll/curve_walker.h"
#include "main/dll/rom_curve_interface.h"
#include "main/audio/sfx_ids.h"
#include "main/audio/sfx.h"
#include "main/objhits.h"
#include "main/dll/modgfx.h"

void crawler_playReactionEffects(struct GameObject* obj, int* st);

extern u8 gCrawlerSeqTable[];
extern u8 gSnowwormSeqIndexReset[4];
extern u8 gSnowwormSeqIndexMax[4];

#endif
