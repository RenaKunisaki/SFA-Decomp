#ifndef MAIN_SHADER_API_H_
#define MAIN_SHADER_API_H_

#include "global.h"
#include "main/map_romlist_page.h"
#include "main/map_texture_state.h"

struct GameObject;
struct MapBlockData;

#define ROM_LIST_PAGE_COUNT 120

extern f32 playerMapOffsetX;
extern f32 playerMapOffsetZ;
extern MapRomListPage* gLoadedRomListPages[ROM_LIST_PAGE_COUNT];

typedef MapRomListPage MapRomList;

typedef struct MapCellEntry
{
    s16 mapId;
    s16 adjacentMapId1;
    s16 adjacentMapId2;
    s16 blockId;
    s8 cellIndex;
    s8 romListIndex;
    s16 unkA;
} MapCellEntry;

STATIC_ASSERT(sizeof(MapCellEntry) == 0xC);

/* MAPINFO.bin per-record map type (curMapType / getCurMapType()). */
typedef enum MapType
{
    MAPTYPE_NORMAL        = 0, /* normal outdoor map */
    MAPTYPE_SUBMAP        = 1, /* normal submap (dungeon/indoor) */
    MAPTYPE_UNLOAD_UNUSED = 2, /* unused: unloads all objects immediately on load */
    MAPTYPE_SUBMAP_UNUSED = 3, /* unused: same as MAPTYPE_UNLOAD_UNUSED; only frontend2 has this */
    MAPTYPE_NO_HUD        = 4, /* hides PDA HUD; title screen + Arwing maps; no player object spawned */
} MapType;

MapCellEntry* mapGetCellEntry(int x, int z);
MapRomList* mapGetCurrentRomList(void);
void mapGetLoadedMapFlags(u8* outFlags);
s32 getCurMapType(void);
void mapTextureOverrideSetValue(int type, Texture* texture, int frame);
int objUpdateOpacity(struct GameObject* obj);
void mapUpdateCameraPosByTransformSpace(void);
void doPendingMapLoads(void);
void mapReloadWithFadeout(void);
void mapSetup(int mapType, f32 x, int* outMapId, int* outEvent, f32 y, f32 z);
void initMaps(void);
void unloadMap(void);
void beginLoadingMap(void);
void goToNextMapLayer(void);
void goToPrevMapLayer(void);
void buildPlayerRelativeFrustumPlanes(void);
s32 getCurMapLayer(void);
void mapUnloadRomListPage(int pageIndex);
void mapGetBlockGridRects(int gridX, int gridZ, int* rectA, int* rectB, int* rectC, int* rectD, int layer, int useVisGrid, int slot);
int mapTextureOverrideAcquire(Texture* texture, u32 flags, int type);
void mapTextureOverrideRelease(Texture* texture, int type);
s16* mapBlockFindTextureOverrideIndex(struct MapBlockData* block, int textureSlot);
int return0_8005669C(int unused);

#endif /* MAIN_SHADER_API_H_ */
