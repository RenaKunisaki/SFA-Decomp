#ifndef MAIN_MAP_TEXTURE_STATE_H_
#define MAIN_MAP_TEXTURE_STATE_H_

#include "global.h"
#include "main/texture.h"

typedef struct MapTextureOverride
{
    union {
        Texture* texture;
        s32 textureId;
    };
    s32 frame;
    u32 flags;
    s16 refCount;
    u8 type;
    u8 pad;
} MapTextureOverride;

typedef struct MapTextureScroll
{
    f32 offsetX;
    f32 offsetY;
    s16 xStep;
    s16 yStep;
    u8 refCount;
    u8 pad[3];
} MapTextureScroll;

STATIC_ASSERT(sizeof(MapTextureOverride) == 0x10);
STATIC_ASSERT(sizeof(MapTextureScroll) == 0x10);

extern MapTextureOverride* lbl_803DCE6C;
extern MapTextureScroll* lbl_803DCE68;

#endif /* MAIN_MAP_TEXTURE_STATE_H_ */
