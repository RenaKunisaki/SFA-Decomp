#ifndef MAIN_TRACK_DOLPHIN_H_
#define MAIN_TRACK_DOLPHIN_H_

#include "ghidra_import.h"
#include "game/objects/object.h"

typedef struct TrackBlockDescriptor
{
    void* object;
    s16 firstTriangle;
    u8 pad06[2];
    void* currentMatrix;
    void* currentCollisionMatrix;
    void* alternateMatrix;
    void* alternateCollisionMatrix;
} TrackBlockDescriptor;
struct TrackTriangle;

void trackDolphin_buildSweptBounds(u32 *boundsOut,float *startPoints,float *endPoints,
                                   float *radii,int pointCount);

/* extern-cleanup: defining-file public prototypes */
int collectShadowTrackTriangles(int* obj, int triBuf, void* planesOut, int vertsOut, int unusedTriangleCount,
                                f32 offX, f32 offZ, int unusedRenderMode, int kindSelector);
void objDrawShadowCasterMesh(Vec3f* vertices, ObjModelState* modelState, GameObject* obj, int triangleCount,
                             void* unusedDrawScratch, void* unusedBounds, f32 unusedYOffset);
void trackCollectGroundHits(struct TrackTriangle* triStart, struct TrackTriangle* triEnd, struct TrackBlockDescriptor* desc,
                 f32 qx, f32 qz, int allowDown);
#endif /* MAIN_TRACK_DOLPHIN_H_ */
