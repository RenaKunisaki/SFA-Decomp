#ifndef MAIN_MAP_ROMLIST_PAGE_H_
#define MAIN_MAP_ROMLIST_PAGE_H_

#include "game/objects/object_setup.h"

typedef struct MapRomListPage
{
    u8 unk00[0x08];
    u16 objectDataSize;
    u8 unk0A[0x02];
    void* unk0C;
    u8* loadedObjectBits;
    void* unk14;
    u8 unk18;
    u8 mapLayer;
    u8 unk1A[0x06];
    ObjPlacement* objects;
    f32 worldX;
    f32 worldZ;
    void* unk2C;
    void* unk30;
    void* unk34;
} MapRomListPage;

STATIC_ASSERT(offsetof(MapRomListPage, objectDataSize) == 0x08);
STATIC_ASSERT(offsetof(MapRomListPage, loadedObjectBits) == 0x10);
STATIC_ASSERT(offsetof(MapRomListPage, objects) == 0x20);
STATIC_ASSERT(offsetof(MapRomListPage, worldX) == 0x24);
STATIC_ASSERT(offsetof(MapRomListPage, worldZ) == 0x28);
STATIC_ASSERT(sizeof(MapRomListPage) == 0x38);

typedef struct MapRomListIndex
{
    int groupOffset[32];
    int objectsSize;
    int curvesOffset;
    int groupsStart;
} MapRomListIndex;

STATIC_ASSERT(offsetof(MapRomListIndex, objectsSize) == 0x80);
STATIC_ASSERT(offsetof(MapRomListIndex, curvesOffset) == 0x84);
STATIC_ASSERT(offsetof(MapRomListIndex, groupsStart) == 0x88);
STATIC_ASSERT(sizeof(MapRomListIndex) == 0x8C);

#endif /* MAIN_MAP_ROMLIST_PAGE_H_ */
