#ifndef MAIN_MAP_BLOCK_H_
#define MAIN_MAP_BLOCK_H_

#include "global.h"

typedef struct Texture Texture;

typedef union MapTextureRef
{
    s32 fileId;
    Texture* texture;
} MapTextureRef;

typedef struct MapBlockBoundsRec
{
    void* dlist;
    u16 dlistSize;
    s16 minX;
    s16 minY;
    s16 minZ;
    s16 maxX;
    s16 maxY;
    s16 maxZ;
    u8 flags;
    u8 pad13;
    u16 renderBitOffset;
    u8 pad16[2];
    u8 selector;
    u8 pad19[0x1C - 0x19];
} MapBlockBoundsRec;

STATIC_ASSERT(sizeof(MapBlockBoundsRec) == 0x1C);

typedef struct MapHitLine
{
    s16 x[2];
    s16 y[2];
    s16 z[2];
    u8 endpointData[2];
    u8 flags;
    s8 kind;
    s16 param;
    u8 pad12[2];
} MapHitLine;

STATIC_ASSERT(sizeof(MapHitLine) == 0x14);

/*
 * MapBlockData - the record returned by mapGetBlock(). Field widths
 * mirror the deref widths observed in mmp_barrel.c / mmp_moonrock.c /
 * track_dolphin.c; unobserved ranges are padded (positional unkNN
 * names, true size unverified - do not take sizeof).
 */
typedef struct MapShaderLayer
{
    union {
        s32 textureIndex;
        s32 texId;
        Texture* texture;
    };
    u8 typeBits;
    union {
        u8 overrideType;
        u8 overrideByte;
        u8 mapLayerId;
    };
    union {
        u8 scrollMtx;
        u8 mtxIndex;
    };
    u8 unk7;
} MapShaderLayer;

STATIC_ASSERT(offsetof(MapShaderLayer, mapLayerId) == 0x05);
STATIC_ASSERT(sizeof(MapShaderLayer) == 0x08);

typedef struct MapShader
{
    u8 pad0[0x24];
    MapShaderLayer layers[2];
    union {
        s32 auxTextureIndex;
        Texture* auxTexture;
    };
    u8 pad38[0x3C - 0x38];
    u32 flags;
    u8 pad40;
    u8 layerCount;
    u8 pad42[2];
} MapShader;

STATIC_ASSERT(sizeof(MapShader) == 0x44);
STATIC_ASSERT(offsetof(MapShader, layers) == 0x24);
STATIC_ASSERT(offsetof(MapShader, auxTexture) == 0x34);
STATIC_ASSERT(offsetof(MapShader, flags) == 0x3C);
STATIC_ASSERT(offsetof(MapShader, layerCount) == 0x41);

typedef struct MapPolygonGroup
{
    u8 pad00[0x10];
    u32 flags;
} MapPolygonGroup;

STATIC_ASSERT(offsetof(MapPolygonGroup, flags) == 0x10);
STATIC_ASSERT(sizeof(MapPolygonGroup) == 0x14);

typedef enum MapBlockFlag {
    MAP_BLOCK_FLAG_LOADED = 0x0008,
} MapBlockFlag;

typedef struct MapBlockData {
    void* unk0;
    u16 flags4; /* 0x04: MapBlockFlag plus an unidentified bit toggled per tick */
    u16 unk6;
    u32 size;
    f32 transform[3][4];
    u8 pad3C[0x4C - 0x3C];
    void* gcPolygons; /* 0x4C: MapTriIndex[] collision mesh (stride 8), count = nPolygons @0x98 */
    void* polygonGroups; /* 0x50: MapTriGroup[] (stride 0x14), count = polyGroupCount @0x9A */
    MapTextureRef* textures; /* 0x54: file IDs converted to texture pointers after loading */
    u8* vertices; /* 0x58: base of the packed VertexS16 array (stride 6) */
    void* vertexColors; /* 0x5C: RGBA4444 (stride 2) */
    void* vertexTexCoords; /* 0x60: vec2s (stride 4) */
    MapShader* shaders; /* 0x64: count = shaderCount @0xA2 */
    MapBlockBoundsRec* displayLists; /* 0x68: count = edgeCount @0xA1 */
    u8 pad6C[0x70 - 0x6C];
    MapHitLine* hits; /* 0x70: from HITS.bin; 0 in file, populated by MapBlock_initHits */
    void* auxData; /* 0x74: optional auxiliary allocation */
    void* renderInstrsMain; /* 0x78: normal geometry bitstream */
    void* renderInstrsTransp; /* 0x7C: transparent+glow bitstream */
    void* renderInstrsWater; /* 0x80: water+reflective bitstream */
    u16 nRenderInstrsMain; /* 0x84: stream size in bytes */
    u16 nRenderInstrsTransp; /* 0x86 */
    u16 nRenderInstrsWater; /* 0x88 */
    s16 minY; /* 0x8A: lower vertical bound */
    s16 maxY; /* 0x8C: upper vertical bound */
    s16 collisionYOffset; /* 0x8E: added to collision-group and vertex Y coordinates */
    u16 vertexCount; /* 0x90: entries in the vertices array (DCStoreRange size = count*6) */
    u8 pad92[0x98 - 0x92];
    u16 nPolygons; /* 0x98: entries in gcPolygons (cacheAllocAndCopy size = count<<3) */
    u16 polyGroupCount; /* 0x9A: render/poly groups (mapBlockGetPolygonGroup index bound) */
    u16 hitCount; /* 0x9C: entries in the HITS.bin segment table */
    u16 unk9E;
    u8 textureCount; /* 0xA0: entries in textures */
    u8 edgeCount; /* 0xA1: edges (mapBlockGetEdge index bound) */
    u8 shaderCount; /* 0xA2: entries in shaders */
    u8 padA3;
} MapBlockData;

STATIC_ASSERT(offsetof(MapBlockData, hitCount) == 0x9C);
STATIC_ASSERT(offsetof(MapBlockData, transform) == 0x0C);
STATIC_ASSERT(offsetof(MapBlockData, minY) == 0x8A);
STATIC_ASSERT(offsetof(MapBlockData, maxY) == 0x8C);
STATIC_ASSERT(offsetof(MapBlockData, textureCount) == 0xA0);
STATIC_ASSERT(offsetof(MapBlockData, shaderCount) == 0xA2);

extern MapBlockData** gMapBlocks;
extern s8* gMapBlockLayerTables[];

MapShader* mapBlockGetShader(MapBlockData* block, int index);

#endif
