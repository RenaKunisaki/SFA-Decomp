#ifndef MAIN_TRACK_DOLPHIN_API_H_
#define MAIN_TRACK_DOLPHIN_API_H_

#include "types.h"
#include "game/objects/object.h"
#include "main/model_render_instrs_api.h"
#include "main/track_dolphin_map_api.h"

typedef struct TrackGroundHit
{
    f32 height;
    f32 normalX;
    f32 normalY;
    f32 normalZ;
    GameObject* object;
    u8 surfaceType;
    u8 pad15[3];
} TrackGroundHit;

STATIC_ASSERT(sizeof(TrackGroundHit) == 0x18);

typedef struct TrackQueryBounds
{
    s32 minX;
    s32 minY;
    s32 minZ;
    s32 maxX;
    s32 maxY;
    s32 maxZ;
} TrackQueryBounds;

STATIC_ASSERT(sizeof(TrackQueryBounds) == 0x18);

struct Shader;
struct MapBlockData;
typedef struct ObjModel ObjModel;

int objShadowRender(GameObject* obj, int renderMode, int unused, int frameCount);
int trackIntersectRebuildPending(void);
int trackGetNearestGroundOffsetAndNormal(GameObject* obj, f32 x, f32 y, f32 z, f32* outGroundOffset,
                                         f32* outNormal, int queryMask);
int trackGetNearestGroundOffset(GameObject* obj, f32 x, f32 y, f32 z, f32* outGroundOffset, int queryMask);
int hitDetectFn_80065e50(GameObject* obj, f32 x, f32 y, f32 z, TrackGroundHit*** hitsOut, int mode, int queryMask);
int hitDetectFn_80067958(GameObject* contactSource, f32* startPoints, f32* endPoints, int pointCount, void* results,
                        int flags);
void hitDetect_calcSweptSphereBounds(TrackQueryBounds* boundsOut, f32* startPoints, f32* endPoints, f32* radii,
                                     int pointCount);
void hitDetectFn_800691c0(GameObject* obj, TrackQueryBounds* bounds, u32 mask, int flags);
void trackSetLinesEnabledByParam(int matchValue, GameObject* obj, int flag);
void doNothing_80062A50(GameObject* obj, f32 x, f32 y, f32 z);
void objHitDetectFn_80062e84(GameObject* obj, GameObject* newParent, int mode);
void playerShadowClearPositionOverride(GameObject* obj);
int shadowInit(GameObject* obj, u32 arena, int flags);
void objShadowInvalidate(GameObject* obj);
void shadowVolumesSetDirty(s32 dirty);
void getSunFlareScissorRect(int* outX, int* outY, int* outWidth, int* outHeight);
void trackGetGridOrigin(int** outOrigin);
void trackGetTriangleBuffer(int* outCount, int* outTable);
void mapInitFn_80069990(void);
void trackIntersect(void);
void mapBlockRender_setVtxDcrs(u8 doSetup, struct MapBlockData* block, struct Shader* shader,
                               ModelRenderInstrsState* state);
void initTextures(void);
void mapClearBlockEdgeFlags(void);
void* mapBlockGetPolygon(int* obj, int idx);
void* mapBlockGetEdge(int* obj, int idx);
void gxErrorFn_80060b40(void);
struct MapBlockData* MapBlock_loadFromFile(int blockId);
void setMapBlockFlag(void);
void trackTickDynamicSlotCooldowns(void);
void setupToRenderMapBlock(struct MapBlockData* block, void* posMtx);
void renderMapBlock(struct MapBlockData* block, u8 type);
void shadowBeginFrame(void);
void shadowVolumeBeginFrame(void);
void trackInvalidateDynamicSlotsForObject(GameObject* target);
void objDrawGroundShadow(GameObject* obj, ObjModel* model);
int findSurfaceInYRange(GameObject* obj, f32 x, f32 lo, f32 z, f32 hi, f32* outSurfaceY,
                        GameObject** outSurfaceObj);
void renderGlows(void);
void MapBlock_init(struct MapBlockData* block);
void MapBlock_initHits(struct MapBlockData* block, int index);
int mapBlockCountTrianglesByType(struct MapBlockData* block, int type);
void buildShadowVolumeBox(f32* direction, f32* out, f32 lowerScale);
int trackGetHeightAboveGround(GameObject* obj, f32 x, f32 y, f32 z, f32* outDepth, int queryMask);
extern int lbl_803DCF34;
extern f32* lbl_803DCF38;

#endif /* MAIN_TRACK_DOLPHIN_API_H_ */
