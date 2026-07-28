/* Applies a game-bit-controlled XYZ offset to map-block polygons and edges. */
#include "dlls/objects/316_XYZAnimator.h"

#include "dolphin/os/OSCache.h"
#include "main/frame_timing.h"
#include "main/gamebits.h"
#include "main/map_block.h"
#include "main/mm.h"
#include "main/object_render.h"
#include "main/pi_dolphin_api.h"
#include "main/track_dolphin_api.h"

typedef struct XyzAnimatorPolygonGroup {
    u16 firstTriangle;
    u16 pad02[2];
    s16 posA;
    s16 posB;
} XyzAnimatorPolygonGroup;

typedef struct XyzAnimatorVertex {
    s16 x;
    s16 y;
    s16 z;
} XyzAnimatorVertex;

typedef struct XyzAnimatorEdge {
    u8 pad00[6];
    s16 v0x;
    s16 v0y;
    s16 v0z;
    s16 v1x;
    s16 v1y;
    s16 v1z;
} XyzAnimatorEdge;

STATIC_ASSERT(offsetof(XyzAnimatorPolygonGroup, firstTriangle) == 0x00);
STATIC_ASSERT(offsetof(XyzAnimatorPolygonGroup, posA) == 0x06);
STATIC_ASSERT(offsetof(XyzAnimatorPolygonGroup, posB) == 0x08);
STATIC_ASSERT(sizeof(XyzAnimatorVertex) == 0x06);
STATIC_ASSERT(offsetof(XyzAnimatorEdge, v0x) == 0x06);
STATIC_ASSERT(offsetof(XyzAnimatorEdge, v0y) == 0x08);
STATIC_ASSERT(offsetof(XyzAnimatorEdge, v0z) == 0x0A);
STATIC_ASSERT(offsetof(XyzAnimatorEdge, v1x) == 0x0C);
STATIC_ASSERT(offsetof(XyzAnimatorEdge, v1y) == 0x0E);
STATIC_ASSERT(offsetof(XyzAnimatorEdge, v1z) == 0x10);

f32 XyzAnimator_getCoordinate(GameObject* obj, u8 coordinate) {
    XyzAnimatorState* state;

    if (obj == NULL || (state = (XyzAnimatorState*)obj->extra, state == NULL)) {
        return 0.0f;
    }
    switch (coordinate) {
    case XYZ_ANIMATOR_COORD_WORLD_X:
        return obj->anim.localPosX + state->offsetX;
    case XYZ_ANIMATOR_COORD_OFFSET_X:
        return state->offsetX;
    case XYZ_ANIMATOR_COORD_WORLD_Y:
        return obj->anim.localPosY + state->offsetY;
    case XYZ_ANIMATOR_COORD_OFFSET_Y:
        return state->offsetY;
    case XYZ_ANIMATOR_COORD_WORLD_Z:
        return obj->anim.localPosZ + state->offsetZ;
    case XYZ_ANIMATOR_COORD_OFFSET_Z:
        return state->offsetZ;
    }
    return 0.0f;
}

void XyzAnimator_captureGeometry(XyzAnimatorPlacement* placement, XyzAnimatorState* state, int blockAddress) {
    int vertexDataOffset[1];
    int groupDataOffset[1];
    int triangleDataOffset[1];
    int blockIndex;
    int triangle;
    int triangleEnd;
    u16* mapEntry;
    int polygonGroupType;
    int edgeBufferOffset[1];
    int edgeIndex[1];
    XyzAnimatorVertex* vertex;
    MapBlockData* blockData = (MapBlockData*)blockAddress;

    vertexDataOffset[0] = 0;
    edgeBufferOffset[0] = 0;
    blockIndex = 0;
    groupDataOffset[0] = 0;
    triangleDataOffset[0] = groupDataOffset[0];
    for (; blockIndex < (int)(u32)blockData->polyGroupCount; blockIndex++) {
        mapEntry = mapBlockGetPolygonGroup((void*)blockAddress, blockIndex);
        polygonGroupType = mapBlockGetPolygonGroupType(mapEntry);
        if ((int)placement->blockLayer == polygonGroupType) {
            *(s16*)(state->posABuffer + groupDataOffset[0]) = ((XyzAnimatorPolygonGroup*)mapEntry)->posA;
            *(s16*)(state->posBBuffer + groupDataOffset[0]) = ((XyzAnimatorPolygonGroup*)mapEntry)->posB;
            groupDataOffset[0] += 2;
            triangleEnd = mapEntry[10];
            triangle = *mapEntry;
            vertexDataOffset[0] = triangleDataOffset[0];
            for (; triangle < triangleEnd; triangle++) {
                int vertex1Offset;
                int vertex2Offset;
                mapEntry = mapBlockGetPolygon((int*)blockAddress, triangle);
                vertex = (XyzAnimatorVertex*)(blockData->vertices + (u32)*mapEntry * 6);
                *(s16*)(state->geometryBuffer + vertexDataOffset[0]) = vertex->x;
                *(s16*)(state->geometryBuffer + vertexDataOffset[0] + 2) = vertex->y;
                *(s16*)(state->geometryBuffer + vertexDataOffset[0] + 4) = vertex->z;
                vertex1Offset = vertexDataOffset[0] + 6;
                vertex = (XyzAnimatorVertex*)(blockData->vertices + mapEntry[1] * 6);
                *(s16*)(state->geometryBuffer + vertex1Offset) = vertex->x;
                *(s16*)(state->geometryBuffer + vertex1Offset + 2) = vertex->y;
                *(s16*)(state->geometryBuffer + vertex1Offset + 4) = vertex->z;
                vertex2Offset = vertex1Offset + 6;
                vertex = (XyzAnimatorVertex*)(blockData->vertices + mapEntry[2] * 6);
                *(s16*)(state->geometryBuffer + vertex2Offset) = vertex->x;
                *(s16*)(state->geometryBuffer + vertex2Offset + 2) = vertex->y;
                *(s16*)(state->geometryBuffer + vertex2Offset + 4) = vertex->z;
                vertexDataOffset[0] += 0x12;
                triangleDataOffset[0] += 0x12;
            }
        }
    }
    edgeIndex[0] = 0;
    edgeBufferOffset[0] = edgeIndex[0];
    for (; edgeIndex[0] < (int)(u32)blockData->edgeCount; edgeIndex[0]++) {
        blockIndex = (int)mapBlockGetEdge((int*)blockAddress, edgeIndex[0]);
        *(s16*)(state->edgeV0xBuffer + edgeBufferOffset[0]) = ((XyzAnimatorEdge*)blockIndex)->v0x;
        *(s16*)(state->edgeV1xBuffer + edgeBufferOffset[0]) = ((XyzAnimatorEdge*)blockIndex)->v1x;
        *(s16*)(state->edgeV0yBuffer + edgeBufferOffset[0]) = ((XyzAnimatorEdge*)blockIndex)->v0y;
        *(s16*)(state->edgeV1yBuffer + edgeBufferOffset[0]) = ((XyzAnimatorEdge*)blockIndex)->v1y;
        *(s16*)(state->edgeV0zBuffer + edgeBufferOffset[0]) = ((XyzAnimatorEdge*)blockIndex)->v0z;
        *(s16*)(state->edgeV1zBuffer + edgeBufferOffset[0]) = ((XyzAnimatorEdge*)blockIndex)->v1z;
        edgeBufferOffset[0] += 2;
    }
}

int XyzAnimator_getExtraSize(void) {
    return sizeof(XyzAnimatorState);
}

void XyzAnimator_free(GameObject* obj, int flags) {
    int blockAddress;
    XyzAnimatorState* state;
    XyzAnimatorPlacement* placement;
    f32 zeroOffset;

    state = (XyzAnimatorState*)(obj)->extra;
    placement = (XyzAnimatorPlacement*)obj->anim.placementData;
    zeroOffset = 0.0f;
    state->offsetX = zeroOffset;
    state->offsetY = zeroOffset;
    state->offsetZ = zeroOffset;
    if (flags == 0) {
        blockAddress = objPosToMapBlockIdx((double)(obj)->anim.localPosX, (double)(obj)->anim.localPosY,
                                           (double)(obj)->anim.localPosZ);
        blockAddress = (int)mapGetBlock(blockAddress);
        if (((void*)blockAddress != NULL) && (state->vertexCount != 0)) {
            XyzAnimator_applyToMapBlock(placement, state, blockAddress);
        }
    }
    if ((void*)state->geometryBuffer != NULL) {
        mm_free((void*)state->geometryBuffer);
    }
    ObjGroup_RemoveObject((int)obj, XYZ_ANIMATOR_OBJECT_GROUP);
}

void XyzAnimator_render(GameObject* obj, int renderArg2, int renderArg3, int renderArg4, int renderArg5, s8 visible) {
    s32 isVisible = visible;

    if (isVisible != 0) {
        objRenderModelAndHitVolumes(obj, renderArg2, renderArg3, renderArg4, renderArg5, 1.0f);
    }
}

void XyzAnimator_applyToMapBlock(XyzAnimatorPlacement* placement, XyzAnimatorState* state, int blockAddress) {
    XyzAnimatorVertex* vertex;
    MapBlockData* blockData = (MapBlockData*)blockAddress;
    int vertexOffset[1];
    int vertexIndex;
    int polygonGroupIndex;
    int polygonGroupType;
    int dataOffset;
    u16* mapEntry;
    f32 scale;
    int triangle;
    int triangleEnd;
    int edgeIndex;
    int groupDataOffset[1];
    void* shaderLayer;

    polygonGroupIndex = 0;
    groupDataOffset[0] = 0;
    vertexOffset[0] = groupDataOffset[0];
    for (; polygonGroupIndex < (int)(u32)blockData->polyGroupCount; polygonGroupIndex++) {
        mapEntry = mapBlockGetPolygonGroup((void*)blockAddress, polygonGroupIndex);
        polygonGroupType = mapBlockGetPolygonGroupType(mapEntry);
        if ((int)placement->blockLayer == polygonGroupType) {
            ((XyzAnimatorPolygonGroup*)mapEntry)->posA =
                (s16)(state->offsetY + (f32) * (s16*)(state->posABuffer + groupDataOffset[0]));
            ((XyzAnimatorPolygonGroup*)mapEntry)->posB =
                (s16)(state->offsetY + (f32) * (s16*)(state->posBBuffer + groupDataOffset[0]));
            groupDataOffset[0] += 2;
            triangleEnd = mapEntry[10];
            triangle = *mapEntry;
            vertexIndex = vertexOffset[0];
            scale = 8.0f;
            for (; triangle < triangleEnd; triangle++) {
                mapEntry = mapBlockGetPolygon((int*)blockAddress, triangle);
                dataOffset = vertexIndex;
                for (edgeIndex = 3; edgeIndex != 0; edgeIndex--) {
                    vertex = (XyzAnimatorVertex*)(blockData->vertices + (u32)*mapEntry * 6);
                    vertex->x = (s16)(scale * state->offsetX + (f32) * (s16*)(state->geometryBuffer + dataOffset));
                    vertex->y = (s16)(scale * state->offsetY + (f32) * (s16*)(state->geometryBuffer + dataOffset + 2));
                    vertex->z = (s16)(scale * state->offsetZ + (f32) * (s16*)(state->geometryBuffer + dataOffset + 4));
                    dataOffset += 6;
                    vertexIndex += 6;
                    vertexOffset[0] += 6;
                    mapEntry++;
                }
            }
        }
    }
    DCStoreRange((void*)blockData->vertices, (u32)blockData->vertexCount * 6);
    edgeIndex = 0;
    dataOffset = edgeIndex;
    for (; edgeIndex < (int)(u32)blockData->edgeCount; edgeIndex++) {
        vertexOffset[0] = (int)mapBlockGetEdge((int*)blockAddress, edgeIndex);
        shaderLayer = mapBlockGetShader((MapBlockData*)blockAddress, *(u8*)(vertexOffset[0] + 0x13));
        shaderLayer = Shader_getLayer(shaderLayer, 0);
        if ((int)*(u8*)((int)shaderLayer + 5) == placement->blockLayer) {
            scale = 8.0f;
            ((XyzAnimatorEdge*)vertexOffset[0])->v0x =
                (s16)(scale * state->offsetX + (f32) * (s16*)(state->edgeV0xBuffer + dataOffset));
            ((XyzAnimatorEdge*)vertexOffset[0])->v1x =
                (s16)(scale * state->offsetX + (f32) * (s16*)(state->edgeV1xBuffer + dataOffset));
            ((XyzAnimatorEdge*)vertexOffset[0])->v0y =
                (s16)(scale * state->offsetY + (f32) * (s16*)(state->edgeV0yBuffer + dataOffset));
            ((XyzAnimatorEdge*)vertexOffset[0])->v1y =
                (s16)(scale * state->offsetY + (f32) * (s16*)(state->edgeV1yBuffer + dataOffset));
            ((XyzAnimatorEdge*)vertexOffset[0])->v0z =
                (s16)(scale * state->offsetZ + (f32) * (s16*)(state->edgeV0zBuffer + dataOffset));
            ((XyzAnimatorEdge*)vertexOffset[0])->v1z =
                (s16)(scale * state->offsetZ + (f32) * (s16*)(state->edgeV1zBuffer + dataOffset));
        }
        dataOffset += 2;
    }
    *(int*)blockAddress = return0_80060B90((void*)blockAddress);
}

void XyzAnimator_update(GameObject* obj) {
    XyzAnimatorPlacement* placement = (XyzAnimatorPlacement*)obj->anim.placementData;
    XyzAnimatorState* state = (XyzAnimatorState*)obj->extra;
    int blockAddress;
    u8* polygonGroup;
    int polygonGroupIndex;
    int completedAxes;
    u8* bufferAddress;
    int streamSize;
    int value;

    blockAddress = (int)mapGetBlock(objPosToMapBlockIdx(obj->anim.localPosX, obj->anim.localPosY, obj->anim.localPosZ));
    if ((u32)blockAddress == 0) {
        state->passCount = 0;
        return;
    }
    if ((((MapBlockData*)blockAddress)->flags4 & MAP_BLOCK_FLAG_LOADED) == 0) {
        return;
    }
    if (state->vertexCount == 0) {
        for (polygonGroupIndex = 0; polygonGroupIndex < ((MapBlockData*)blockAddress)->polyGroupCount;
             polygonGroupIndex++) {
            polygonGroup = mapBlockGetPolygonGroup((void*)blockAddress, polygonGroupIndex);
            value = mapBlockGetPolygonGroupType(polygonGroup);
            if (placement->blockLayer == value) {
                state->polygonGroupCount++;
                state->vertexCount += (*(u16*)(polygonGroup + 0x14) - *(u16*)(polygonGroup + 0));
            }
        }
        if (state->vertexCount == 0) {
            return;
        }
        state->vertexCount *= 3;
        if (placement->triggerGameBit == -1) {
            state->triggerBitValue = 1;
        } else {
            state->triggerBitValue = mainGetBit(placement->triggerGameBit);
        }
        state->edgeCount = ((MapBlockData*)blockAddress)->edgeCount;
        state->offsetX = (f32)placement->startX;
        state->offsetY = (f32)placement->startY;
        state->offsetZ = (f32)placement->startZ;
        if (placement->completionGameBit != -1 && mainGetBit(placement->completionGameBit) != 0) {
            state->offsetX = (f32)placement->targetX;
            state->offsetY = (f32)placement->targetY;
            state->offsetZ = (f32)placement->targetZ;
            state->triggerBitValue = 1;
        }
        value = state->vertexCount * 6 + state->polygonGroupCount * 0xc;
        value = value + state->edgeCount * 0xc;
        bufferAddress = mmAlloc(value, 5, 0);
        state->geometryBuffer = bufferAddress;
        streamSize = state->polygonGroupCount * 2;
        bufferAddress = bufferAddress + state->vertexCount * 6;
        state->polygonBuffer0 = bufferAddress;
        bufferAddress = bufferAddress + streamSize;
        state->polygonBuffer1 = bufferAddress;
        bufferAddress = bufferAddress + streamSize;
        state->posABuffer = bufferAddress;
        bufferAddress = bufferAddress + streamSize;
        state->posBBuffer = bufferAddress;
        bufferAddress = bufferAddress + streamSize;
        state->polygonBuffer4 = bufferAddress;
        bufferAddress = bufferAddress + streamSize;
        state->polygonBuffer5 = bufferAddress;
        bufferAddress = bufferAddress + streamSize;
        streamSize = state->edgeCount * 2;
        state->edgeV0xBuffer = bufferAddress;
        bufferAddress = bufferAddress + streamSize;
        state->edgeV1xBuffer = bufferAddress;
        bufferAddress = bufferAddress + streamSize;
        state->edgeV0yBuffer = bufferAddress;
        bufferAddress = bufferAddress + streamSize;
        state->edgeV1yBuffer = bufferAddress;
        bufferAddress = bufferAddress + streamSize;
        state->edgeV0zBuffer = bufferAddress;
        bufferAddress = bufferAddress + streamSize;
        state->edgeV1zBuffer = bufferAddress;
        XyzAnimator_captureGeometry(placement, state, blockAddress);
        if (placement->mode != XYZ_ANIMATOR_MODE_DEFERRED_ONESHOT) {
            XyzAnimator_applyToMapBlock(placement, state, blockAddress);
            ((MapBlockData*)blockAddress)->flags4 = ((MapBlockData*)blockAddress)->flags4 ^ 1;
            XyzAnimator_applyToMapBlock(placement, state, blockAddress);
            ((MapBlockData*)blockAddress)->flags4 = ((MapBlockData*)blockAddress)->flags4 ^ 1;
        }
    }
    if (placement->mode == XYZ_ANIMATOR_MODE_GATED) {
        value = mainGetBit(placement->triggerGameBit);
        if (state->triggerBitValue != value) {
            state->triggerBitValue = value;
            if (value == 0) {
                if (placement->completionGameBit > -1) {
                    mainSetBits(placement->completionGameBit, 0);
                }
            }
            if (state->passCount > 2) {
                state->passCount = 0;
            }
        }
        if (state->passCount > 2) {
            return;
        }
        if (state->loopSfxId != 0) {
            Sfx_KeepAliveLoopedObjectSound((u32)obj, state->loopSfxId);
        }
    } else {
        if (state->passCount > 2) {
            return;
        }
        if (state->triggerBitValue == 0) {
            state->triggerBitValue = mainGetBit(placement->triggerGameBit);
            if (state->triggerBitValue == 0) {
                return;
            }
        }
    }
    switch (placement->mode) {
    case XYZ_ANIMATOR_MODE_ONESHOT:
    case XYZ_ANIMATOR_MODE_DEFERRED_ONESHOT:
        completedAxes = 0;
        if (placement->startX > placement->targetX) {
            state->offsetX = -(0.1f * ((f32)(int)placement->speedX * timeDelta) - state->offsetX);
            if (state->offsetX <= (f32)placement->targetX) {
                state->offsetX = (f32)placement->targetX;
                completedAxes = 1;
            }
        } else {
            state->offsetX = 0.1f * ((f32)(int)placement->speedX * timeDelta) + state->offsetX;
            if (state->offsetX >= (f32)placement->targetX) {
                state->offsetX = (f32)placement->targetX;
                completedAxes = 1;
            }
        }
        if (placement->startY > placement->targetY) {
            state->offsetY = -(0.1f * ((f32)(int)placement->speedY * timeDelta) - state->offsetY);
            if (state->offsetY <= (f32)placement->targetY) {
                state->offsetY = (f32)placement->targetY;
                completedAxes += 1;
            }
        } else {
            state->offsetY = 0.1f * ((f32)(int)placement->speedY * timeDelta) + state->offsetY;
            if (state->offsetY >= (f32)placement->targetY) {
                state->offsetY = (f32)placement->targetY;
                completedAxes += 1;
            }
        }
        if (placement->startZ > placement->targetZ) {
            state->offsetZ = -(0.1f * ((f32)(int)placement->speedZ * timeDelta) - state->offsetZ);
            if (state->offsetZ <= (f32)placement->targetZ) {
                state->offsetZ = (f32)placement->targetZ;
                completedAxes += 1;
            }
        } else {
            state->offsetZ = 0.1f * ((f32)(int)placement->speedZ * timeDelta) + state->offsetZ;
            if (state->offsetZ >= (f32)placement->targetZ) {
                state->offsetZ = (f32)placement->targetZ;
                completedAxes += 1;
            }
        }
        if (completedAxes == 3) {
            if (placement->completionGameBit != -1) {
                mainSetBits(placement->completionGameBit, 1);
            }
            state->passCount += 1;
        }
        break;
    case XYZ_ANIMATOR_MODE_LOOP:
        if (placement->startX > placement->targetX) {
            state->offsetX = -(0.1f * ((f32)(int)placement->speedX * timeDelta) - state->offsetX);
            if (state->offsetX < (f32)placement->targetX) {
                state->offsetX = (f32)(placement->startX - (int)((f32)placement->targetX - state->offsetX));
            }
        } else {
            state->offsetX = 0.1f * ((f32)(int)placement->speedX * timeDelta) + state->offsetX;
            if (state->offsetX > (f32)placement->startX) {
                state->offsetX = (f32)(placement->targetX + (int)(state->offsetX - (f32)placement->targetX));
            }
        }
        if (placement->startY > placement->targetY) {
            state->offsetY = -(0.1f * ((f32)(int)placement->speedY * timeDelta) - state->offsetY);
            if (state->offsetY < (f32)placement->targetY) {
                state->offsetY =
                    -(0.1f * (f32)(int)((f32)placement->targetY - state->offsetY) - (f32)placement->startY);
            }
        } else {
            state->offsetY = 0.1f * ((f32)(int)placement->speedY * timeDelta) + state->offsetY;
            if (state->offsetY > (f32)placement->startY) {
                state->offsetY = (f32)(placement->targetY + (int)(state->offsetY - (f32)placement->targetY));
            }
        }
        if (placement->startZ > placement->targetZ) {
            state->offsetZ = -(0.1f * ((f32)(int)placement->speedZ * timeDelta) - state->offsetZ);
            if (state->offsetZ < (f32)placement->targetZ) {
                state->offsetZ = (f32)(placement->startZ - (int)((f32)placement->targetZ - state->offsetZ));
            }
        } else {
            state->offsetZ = 0.1f * ((f32)(int)placement->speedZ * timeDelta) + state->offsetZ;
            if (state->offsetZ > (f32)placement->startZ) {
                state->offsetZ = (f32)(placement->targetZ + (int)(state->offsetZ - (f32)placement->targetZ));
            }
        }
        break;
    case XYZ_ANIMATOR_MODE_GATED:
        completedAxes = 0;
        if (state->triggerBitValue != 0) {
            if (placement->startX > placement->targetX) {
                state->offsetX = -(0.1f * ((f32)(int)placement->speedX * timeDelta) - state->offsetX);
                if (state->offsetX <= (f32)placement->targetX) {
                    state->offsetX = (f32)placement->targetX;
                    completedAxes = 1;
                }
            } else {
                state->offsetX = 0.1f * ((f32)(int)placement->speedX * timeDelta) + state->offsetX;
                if (state->offsetX >= (f32)placement->targetX) {
                    state->offsetX = (f32)placement->targetX;
                    completedAxes = 1;
                }
            }
            if (placement->startY > placement->targetY) {
                state->offsetY = -(0.1f * ((f32)(int)placement->speedY * timeDelta) - state->offsetY);
                if (state->offsetY <= (f32)placement->targetY) {
                    state->offsetY = (f32)placement->targetY;
                    completedAxes += 1;
                }
            } else {
                state->offsetY = 0.1f * ((f32)(int)placement->speedY * timeDelta) + state->offsetY;
                if (state->offsetY >= (f32)placement->targetY) {
                    state->offsetY = (f32)placement->targetY;
                    completedAxes += 1;
                }
            }
            if (placement->startZ > placement->targetZ) {
                state->offsetZ = -(0.1f * ((f32)(int)placement->speedZ * timeDelta) - state->offsetZ);
                if (state->offsetZ <= (f32)placement->targetZ) {
                    state->offsetZ = (f32)placement->targetZ;
                    completedAxes += 1;
                }
            } else {
                state->offsetZ = 0.1f * ((f32)(int)placement->speedZ * timeDelta) + state->offsetZ;
                if (state->offsetZ >= (f32)placement->targetZ) {
                    state->offsetZ = (f32)placement->targetZ;
                    completedAxes += 1;
                }
            }
            if (completedAxes == 3) {
                if (placement->completionGameBit != -1) {
                    mainSetBits(placement->completionGameBit, 1);
                }
                state->passCount += 1;
            }
        } else {
            if (placement->startX > placement->targetX) {
                state->offsetX = 0.1f * ((f32)(int)placement->speedX * timeDelta) + state->offsetX;
                if (state->offsetX >= (f32)placement->startX) {
                    state->offsetX = (f32)placement->startX;
                    completedAxes = 1;
                }
            } else {
                state->offsetX = -(0.1f * ((f32)(int)placement->speedX * timeDelta) - state->offsetX);
                if (state->offsetX <= (f32)placement->startX) {
                    state->offsetX = (f32)placement->startX;
                    completedAxes = 1;
                }
            }
            if (placement->startY > placement->targetY) {
                state->offsetY = 0.1f * ((f32)(int)placement->speedY * timeDelta) + state->offsetY;
                if (state->offsetY >= (f32)placement->startY) {
                    state->offsetY = (f32)placement->startY;
                    completedAxes += 1;
                }
            } else {
                state->offsetY = -(0.1f * ((f32)(int)placement->speedY * timeDelta) - state->offsetY);
                if (state->offsetY <= (f32)placement->startY) {
                    state->offsetY = (f32)placement->startY;
                    completedAxes += 1;
                }
            }
            if (placement->startZ > placement->targetZ) {
                state->offsetZ = 0.1f * ((f32)(int)placement->speedZ * timeDelta) + state->offsetZ;
                if (state->offsetZ >= (f32)placement->startZ) {
                    state->offsetZ = (f32)placement->startZ;
                    completedAxes += 1;
                }
            } else {
                state->offsetZ = -(0.1f * ((f32)(int)placement->speedZ * timeDelta) - state->offsetZ);
                if (state->offsetZ <= (f32)placement->startZ) {
                    state->offsetZ = (f32)placement->startZ;
                    completedAxes += 1;
                }
            }
            if (completedAxes == 3) {
                state->passCount += 1;
            }
        }
        break;
    }
    XyzAnimator_applyToMapBlock(placement, state, blockAddress);
    return;
}

void XyzAnimator_init(GameObject* obj) {
    XyzAnimatorState* state = (XyzAnimatorState*)obj->extra;
    int mapId;

    ObjGroup_AddObject((int)obj, XYZ_ANIMATOR_OBJECT_GROUP);
    mapId = *(int*)(*(int*)&obj->anim.placementData + 0x14);
    switch (mapId) {
    case 0x46406:
    case 0x4BAB1:
        state->loopSfxId = 0x7D;
        break;
    case 0x49275:
    case 0x49CB7:
    case 0x4C797:
        state->loopSfxId = 0x4B7;
        break;
    }
}

ObjectDescriptor gXYZAnimatorObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    0,
    0,
    0,
    (ObjectDescriptorCallback)XyzAnimator_init,
    (ObjectDescriptorCallback)XyzAnimator_update,
    0,
    (ObjectDescriptorCallback)XyzAnimator_render,
    (ObjectDescriptorCallback)XyzAnimator_free,
    0,
    XyzAnimator_getExtraSize,
};
