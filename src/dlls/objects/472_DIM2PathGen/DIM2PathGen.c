/*
 * DIM2PathGen (DLL 0x1D8) - snowball path-generator for Snowhorn Wastes 2.
 * It loads the nearby RomCurve spline for action 10, then alternates between
 * two configured snowball types whenever the spawn timer expires. An inactive
 * snowball is reused from the object-group pool before a new one is allocated.
 */
#include "dlls/objects/472_DIM2PathGen.h"

#include "dlls/objects/471_DIM2SnowBal.h"
#include "game/objects/object.h"
#include "main/dll/rom_curve_interface.h"
#include "main/frame_timing.h"
#include "main/gamebits_api.h"
#include "main/obj_group.h"
#include "sys/objects.h"
#include "sys/objects/lifecycle.h"

#define DIM2_PATH_GENERATOR_FLAG_SPAWN_TOGGLE 0x01
#define DIM2_PATH_GENERATOR_FLAG_CURVE_BUILT  0x02
#define DIM2_PATH_GENERATOR_FLAG_USE_CURVE    0x04

#define DIM2_PATH_GENERATOR_CURVE_GROUP    21
#define DIM2_PATH_GENERATOR_CURVE_ACTION   10
#define DIM2_PATH_GENERATOR_SNOWBALL_GROUP 47

/* RomCurve definition record (subset) returned by gRomCurveInterface->getById. */
typedef struct Dim2PathRomCurveDef {
    u8 pad0[0x8 - 0x0];
    f32 originX;
    f32 originY;
    f32 originZ;
} Dim2PathRomCurveDef;

u8 DIM2PathGenerator_getCurveVals(GameObject* obj, int** outPathX, int** outPathY, int** outPathZ,
                                  int** outPathNodeData) {
    Dim2PathGeneratorState* state = obj->extra;

    *outPathX = (int*)state->pathX;
    *outPathY = (int*)state->pathY;
    *outPathZ = (int*)state->pathZ;
    if (outPathNodeData != NULL) {
        *outPathNodeData = (int*)state->pathNodeData;
    }

    return state->pointCount;
}

int DIM2PathGenerator_getExtraSize(void) {
    return sizeof(Dim2PathGeneratorState);
}

int DIM2PathGenerator_getObjectTypeId(void) {
    return 0;
}

void DIM2PathGenerator_free(GameObject* obj) {
}

void DIM2PathGenerator_render(GameObject* obj, int renderArg2, int renderArg3, int renderArg4, int renderArg5,
                              s8 visible) {
}

void DIM2PathGenerator_hitDetect(void) {
}

void DIM2PathGenerator_update(GameObject* obj) {
    const Dim2PathGeneratorPlacementView* placement;
    Dim2PathGeneratorState* state = obj->extra;
    int toggle;
    int** objects;
    int objectIndex;
    int curveGroup;
    int count;

    placement = (const Dim2PathGeneratorPlacementView*)obj->anim.placementData;
    if (mainGetBit(placement->activeGameBit) == 0) {
        return;
    }

    if ((state->flags & DIM2_PATH_GENERATOR_FLAG_USE_CURVE) != 0) {
        if ((state->flags & DIM2_PATH_GENERATOR_FLAG_CURVE_BUILT) == 0) {
            int found;

            curveGroup = DIM2_PATH_GENERATOR_CURVE_GROUP;
            found = (*gRomCurveInterface)
                        ->find(obj->anim.localPosX, obj->anim.localPosY, obj->anim.localPosZ, &curveGroup, 1,
                               DIM2_PATH_GENERATOR_CURVE_ACTION);
            if (found != -1) {
                Dim2PathRomCurveDef* curve = (Dim2PathRomCurveDef*)(*gRomCurveInterface)->getById(found);

                (*gRomCurveInterface)->countRandomPoints((RomCurveDef*)curve);
                state->pointCount = (*gRomCurveInterface)
                                        ->buildRandomPoints((RomCurvePlacementDef*)curve, state->pathX, state->pathY,
                                                            state->pathZ, state->pathNodeData);
                state->flags |= DIM2_PATH_GENERATOR_FLAG_CURVE_BUILT;
                state->originX = curve->originX;
                state->originY = curve->originY;
                state->originZ = curve->originZ;
            }
        }
    } else {
        state->originX = obj->anim.localPosX;
        state->originY = obj->anim.localPosY;
        state->originZ = obj->anim.localPosZ;
    }

    if ((state->spawnTimer -= framesThisStep) > 0) {
        return;
    }

    toggle = state->flags & DIM2_PATH_GENERATOR_FLAG_SPAWN_TOGGLE;
    state->spawnTimer = state->spawnPeriod;
    state->flags &= ~DIM2_PATH_GENERATOR_FLAG_SPAWN_TOGGLE;
    objects = (int**)ObjGroup_GetObjects(DIM2_PATH_GENERATOR_SNOWBALL_GROUP, &count);
    for (objectIndex = 0; objectIndex < count; objectIndex++) {
        if (state->spawnTypes[toggle] == ((GameObject*)objects[objectIndex])->anim.seqId) {
            int* childPlacementData = *(int**)((char*)objects[objectIndex] + 0x4c);
            int poolIndex;

            ((Dim2SnowBallPlacement*)childPlacementData)->base.posX = state->originX;
            ((Dim2SnowBallPlacement*)childPlacementData)->base.posY = state->originY;
            ((Dim2SnowBallPlacement*)childPlacementData)->base.posZ = state->originZ;
            ((Dim2SnowBallPlacement*)childPlacementData)->base.mapId = placement->base.mapId;
            (*(void (**)(int*, int*, int))(**(int**)((char*)objects[objectIndex] + 0x68) + 4))(objects[objectIndex],
                                                                                               childPlacementData, 1);
            ObjGroup_RemoveObject((int)objects[objectIndex], DIM2_PATH_GENERATOR_SNOWBALL_GROUP);
            ObjGroup_GetObjects(DIM2_PATH_GENERATOR_SNOWBALL_GROUP, &count);
            for (poolIndex = 0; poolIndex < count; poolIndex++) {
            }
            state->flags |= (toggle ^ 1) & DIM2_PATH_GENERATOR_FLAG_SPAWN_TOGGLE;
            return;
        }
    }

    if (Obj_IsLoadingLocked()) {
        Dim2SnowBallPlacement* np =
            (Dim2SnowBallPlacement*)Obj_AllocObjectSetup(sizeof(Dim2SnowBallPlacement), state->spawnTypes[toggle]);

        np->base.posX = state->originX;
        np->base.posY = state->originY;
        np->base.posZ = state->originZ;
        np->base.color[0] = placement->base.color[0];
        np->base.color[2] = placement->base.color[2];
        np->base.color[1] = placement->base.color[1];
        np->base.color[3] = placement->base.color[3];
        np->base.color[3] = 255;
        np->base.mapActFlagsLo = placement->base.mapActFlagsLo;
        np->rotationXByte = (s8)placement->childRotationXByte;
        np->unknown1A = placement->childUnknown1A;
        np->unknown1C = placement->childUnknown1C;
        np->base.mapId = placement->base.mapId;
        Obj_SetupObject((ObjPlacement*)np, 5, obj->anim.mapEventSlot, -1, NULL);
        state->flags |= (toggle ^ 1) & DIM2_PATH_GENERATOR_FLAG_SPAWN_TOGGLE;
    }
}

void DIM2PathGenerator_init(GameObject* obj, int* placementData) {
    Dim2PathGeneratorState* state;

    obj->anim.rotX = (s16)((u32)((Dim2PathGeneratorPlacementView*)placementData)->childRotationXByte << 8);
    state = obj->extra;
    state->spawnPeriod = ((Dim2PathGeneratorPlacementView*)placementData)->spawnPeriod;
    state->spawnTimer = (s16)((Dim2PathGeneratorPlacementView*)placementData)->initialSpawnDelay;
    state->spawnTypes[0] = (s16)((Dim2PathGeneratorPlacementView*)placementData)->primarySpawnType;
    {
        s16 secondarySpawnType = ((Dim2PathGeneratorPlacementView*)placementData)->secondarySpawnType;

        if (secondarySpawnType == -1) {
            state->spawnTypes[1] = (s16)((Dim2PathGeneratorPlacementView*)placementData)->primarySpawnType;
        } else {
            state->spawnTypes[1] = secondarySpawnType;
        }
    }

    state->flags = (u8)(state->flags | DIM2_PATH_GENERATOR_FLAG_USE_CURVE);
    obj->objectFlags = (u16)(obj->objectFlags | OBJECT_OBJFLAG_HITDETECT_DISABLED);
}

void DIM2PathGenerator_release(void) {
}

void DIM2PathGenerator_initialise(void) {
}

ObjectDescriptor11WithPadding gDIM2PathGeneratorObjDescriptor = {
    {
        0,
        0,
        0,
        OBJECT_DESCRIPTOR_FLAGS_11_SLOTS,
        (ObjectDescriptorCallback)DIM2PathGenerator_initialise,
        (ObjectDescriptorCallback)DIM2PathGenerator_release,
        0,
        (ObjectDescriptorCallback)DIM2PathGenerator_init,
        (ObjectDescriptorCallback)DIM2PathGenerator_update,
        (ObjectDescriptorCallback)DIM2PathGenerator_hitDetect,
        (ObjectDescriptorCallback)DIM2PathGenerator_render,
        (ObjectDescriptorCallback)DIM2PathGenerator_free,
        (ObjectDescriptorCallback)DIM2PathGenerator_getObjectTypeId,
        (ObjectDescriptorExtraSizeCallback)DIM2PathGenerator_getExtraSize,
        (ObjectDescriptorCallback)DIM2PathGenerator_getCurveVals,
    },
    0,
};
