#ifndef MAIN_LIGHTMAP_RENDER_QUEUE_API_H_
#define MAIN_LIGHTMAP_RENDER_QUEUE_API_H_

#include "global.h"
#include "main/map_block.h"

void lightmap_queueExternalRenderEntry(u32 slotPoolBase, u32 poolIndex, f32* position);
void fn_8005D3B4(MapBlockBoundsRec* bounds, MapBlockData* block, s32 selector);

#endif /* MAIN_LIGHTMAP_RENDER_QUEUE_API_H_ */
