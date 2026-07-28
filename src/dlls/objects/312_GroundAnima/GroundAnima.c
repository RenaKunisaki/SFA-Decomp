/*
 * Deforms matching map-block geometry as a linked object presses and
 * releases the ground surface.
 */
#include "dlls/objects/312_GroundAnima.h"

#include "dolphin/MSL_C/PPCEABI/bare/H/math_float_helpers.h"
#include "dolphin/os/OSCache.h"
#include "main/frame_timing.h"
#include "main/dll/dll_00C4_tricky.h"
#include "main/gamebits.h"
#include "main/lightmap_api.h"
#include "main/object_render.h"
#include "main/shader_api.h"
#include "main/track_dolphin_api.h"
#include "sys/objects.h"
#include "sys/objects/lifecycle.h"

#define GROUND_ANIMATOR_OBJECT_GROUP        0x31
#define GROUND_ANIMATOR_TARGET_OBJECT_GROUP 0x04

#define GROUND_ANIMATOR_MOON_ROCK_SEQUENCE_ID 0x519

#define GROUND_ANIMATOR_SINK_DEPTH_SCALE 100.0f
#define GROUND_ANIMATOR_RENDER_SCALE     1.0f

u16 gGroundAnimatorSfxIds[4] = {0x109, 0x7E, 0, 0};

u8 GroundAnimator_getMagicCaveIndex(GameObject* obj) {
    GroundAnimatorState* state = obj->extra;
    return state->magicCaveId;
}

u8 GroundAnimator_isFullySunk(GameObject* obj) {
    GroundAnimatorState* state = obj->extra;
    f32 depth = state->sinkDepth;
    GroundAnimatorPlacement* placement = (GroundAnimatorPlacement*)obj->anim.placementData;
    u8 maxDepth = placement->maxSinkDepth;
    return depth > GROUND_ANIMATOR_SINK_DEPTH_SCALE * maxDepth;
}

f32 GroundAnimator_applyPress(GameObject* obj, GameObject* target) {
    GroundAnimatorPlacement* placement;
    GroundAnimatorState* state;
    f32 dy;
    f32 dx;
    f32 dz;
    f32 rangeSq;

    state = obj->extra;
    placement = (GroundAnimatorPlacement*)obj->anim.placementData;
    dy = target->anim.localPosY - obj->anim.localPosY;
    if (dy < (-20.0f) || dy > (20.0f)) {
        return (0.0f);
    }
    dx = target->anim.localPosX - obj->anim.localPosX;
    dz = target->anim.localPosZ - obj->anim.localPosZ;
    rangeSq = (10.0f) + state->radius;
    rangeSq = rangeSq * rangeSq;
    if (dx * dx + dz * dz > rangeSq) {
        return (-1.0f);
    }
    if (state->sinkDepth >= GROUND_ANIMATOR_SINK_DEPTH_SCALE * (f32)(u32)placement->maxSinkDepth) {
        if (state->linkedObject != NULL) {
            GameObject* linkedObject;
            state->sinkDepth = GROUND_ANIMATOR_SINK_DEPTH_SCALE * (f32)(u32)placement->maxSinkDepth;
            linkedObject = state->linkedObject;
            switch (linkedObject->anim.seqId) {
            case GROUND_ANIMATOR_MOON_ROCK_SEQUENCE_ID:
                mmpMoonRock_setFrozen(linkedObject, 0);
                break;
            default:
                (*(void (**)(void*, int))(*(int*)(*(int*)((char*)linkedObject + 0x68)) + 0x24))(linkedObject, 0);
                break;
            }
        }
    }
    state->sinkDepth = (5.0f) * timeDelta + state->sinkDepth;
    state->flags = state->flags | GROUND_ANIMATOR_STATE_PRESSED;
    return state->radius * (state->sinkDepth / (GROUND_ANIMATOR_SINK_DEPTH_SCALE * (f32)(u32)placement->maxSinkDepth));
}

void GroundAnimator_gatherVertices(GameObject* obj, GroundAnimatorState* state, GroundAnimatorPlacement* placement) {
    void* polygonGroup;
    void* polygonIndexCursor;
    void* polygon;
    int vertexFalloffOffset;
    int vertexHeightOffset;
    int polygonFalloffOffset;
    int polygonHeightOffset;
    int offsets[2];
    int vertexIndex;
    MapBlockData* block;
    int mapCellX;
    int mapCellZ;
    int polygonGroupIndex;
    int polygonIndex;
    f32 vertexPosition[3];
    f32 maxFalloff;
    f32 blockLocalZ;
    f32 radiusSquared;
    f32 blockLocalX;
    block = mapGetBlock(
        objPosToMapBlockIdx((double)obj->anim.localPosX, (double)obj->anim.localPosY, (double)obj->anim.localPosZ));
    if (block == NULL || (((MapBlockData*)block)->flags4 & MAP_BLOCK_FLAG_LOADED) == 0) {
        return;
    }
    mapCellX = fastFloorf((obj->anim.localPosX - playerMapOffsetX) / (640.0f));
    mapCellZ = fastFloorf((obj->anim.localPosZ - playerMapOffsetZ) / (640.0f));
    blockLocalX = obj->anim.localPosX - ((640.0f) * mapCellX + playerMapOffsetX);
    blockLocalZ = obj->anim.localPosZ - ((640.0f) * mapCellZ + playerMapOffsetZ);
    offsets[0] = 0;
    state->entryCount = offsets[0];
    radiusSquared = state->radius * state->radius;
    for (polygonGroupIndex = 0, offsets[1] = offsets[0]; polygonGroupIndex < ((MapBlockData*)block)->polyGroupCount;
         polygonGroupIndex++) {
        polygonGroup = mapBlockGetPolygonGroup(block, polygonGroupIndex);
        if (placement->blockId == mapBlockGetPolygonGroupType(polygonGroup)) {
            polygonIndex = *(u16*)polygonGroup;
            polygonFalloffOffset = offsets[0];
            polygonHeightOffset = offsets[1];
            maxFalloff = (1.0f);
            for (; polygonIndex < *(u16*)((char*)polygonGroup + 0x14); polygonIndex++) {
                polygon = mapBlockGetPolygon((int*)block, polygonIndex);
                for (vertexIndex = 0, polygonIndexCursor = polygon, vertexFalloffOffset = polygonFalloffOffset,
                    vertexHeightOffset = polygonHeightOffset;
                     vertexIndex < 3; vertexIndex++) {
                    void* packedVertex = (char*)((MapBlockData*)block)->vertices + *(u16*)polygonIndexCursor * 6;
                    f32 dx;
                    f32 dz;
                    f32 normalizedDistance;
                    trackUnpackVector(packedVertex, vertexPosition);
                    dx = vertexPosition[0] - blockLocalX;
                    dz = vertexPosition[2] - blockLocalZ;
                    normalizedDistance = (dx * dx + dz * dz) / radiusSquared;
                    if (normalizedDistance > maxFalloff) {
                        normalizedDistance = maxFalloff;
                    }
                    normalizedDistance = normalizedDistance * normalizedDistance;
                    *(f32*)((char*)state->falloffBuffer + vertexFalloffOffset) = maxFalloff - normalizedDistance;
                    *(s16*)((char*)state->baseHeightBuffer + vertexHeightOffset) = vertexPosition[1];
                    vertexFalloffOffset += 4;
                    vertexHeightOffset += 2;
                    polygonFalloffOffset += 4;
                    polygonHeightOffset += 2;
                    offsets[0] += 4;
                    offsets[1] += 2;
                    polygonIndexCursor = (char*)polygonIndexCursor + 2;
                }
            }
            state->blockEntries[(state->entryCount)++] = polygonGroupIndex;
        }
    }
}

int GroundAnimator_getExtraSize(void) {
    return sizeof(GroundAnimatorState);
}

void GroundAnimator_free(GameObject* obj, int flags) {
    void* polygonGroup;
    void* polygonIndexCursor;
    int vertexHeightOffset;
    int polygonHeightOffset;
    int heightOffset;
    int polygonGroupIndex;
    int polygonIndex;
    int vertexIndex;
    MapBlockData* block;
    GroundAnimatorState* state;
    GroundAnimatorPlacement* placement;
    void* polygon;
    s16* packedVertex;
    f32 vertexPosition[4];
    state = obj->extra;
    placement = (GroundAnimatorPlacement*)obj->anim.placementData;
    if (flags == 0) {
        block = mapGetBlock(
            objPosToMapBlockIdx((double)obj->anim.localPosX, (double)obj->anim.localPosY, (double)obj->anim.localPosZ));
        if (block != NULL) {
            for (polygonGroupIndex = 0, heightOffset = 0; polygonGroupIndex < ((MapBlockData*)block)->polyGroupCount;
                 polygonGroupIndex++) {
                polygonGroup = mapBlockGetPolygonGroup(block, polygonGroupIndex);
                if (placement->blockId == mapBlockGetPolygonGroupType(polygonGroup)) {
                    for (polygonIndex = *(u16*)polygonGroup, polygonHeightOffset = heightOffset;
                         polygonIndex < *(u16*)((char*)polygonGroup + 0x14); polygonIndex++) {
                        polygon = mapBlockGetPolygon((int*)block, polygonIndex);
                        for (vertexIndex = 0, polygonIndexCursor = polygon, vertexHeightOffset = polygonHeightOffset;
                             vertexIndex < 3; vertexIndex++) {
                            packedVertex =
                                (s16*)((char*)((MapBlockData*)block)->vertices + *(u16*)polygonIndexCursor * 6);
                            trackUnpackVector(packedVertex, vertexPosition);
                            if (state->baseHeightBuffer != NULL) {
                                vertexPosition[1] = (f32) * (s16*)((char*)state->baseHeightBuffer + vertexHeightOffset);
                                trackPackVector(packedVertex, vertexPosition);
                            }
                            vertexHeightOffset += 2;
                            polygonHeightOffset += 2;
                            heightOffset += 2;
                            polygonIndexCursor = (char*)polygonIndexCursor + 2;
                        }
                    }
                }
            }
        }
    }
    if (state->falloffBuffer != NULL) {
        mm_free(state->falloffBuffer);
    }
    ObjGroup_RemoveObject((int)obj, GROUND_ANIMATOR_OBJECT_GROUP);
}

void GroundAnimator_render(GameObject* obj, int renderArg2, int renderArg3, int renderArg4, int renderArg5,
                           s8 visible) {
    s32 visibility = visible;
    if (visibility != 0)
        objRenderModelAndHitVolumes(obj, renderArg2, renderArg3, renderArg4, renderArg5, GROUND_ANIMATOR_RENDER_SCALE);
}

void GroundAnimator_update(GameObject* obj) {
    int offsets[2];
    int vertexHeightOffset;
    u8 previousOnMapFlag;
    int entryIndex;
    void* tricky;
    u8 interactionEnabled;
    int polygonIndex;
    int vertexIndex;
    GroundAnimatorState* state;
    GroundAnimatorPlacement* placement;
    void* polygonGroup;
    void* linkedObject;
    int polygonHeightOffset;
    int polygonFalloffOffset;
    int vertexFalloffOffset;
    f32 searchDistance;
    void* polygonIndexCursor;
    void* polygon;
    MapBlockData* block;
    s8 blockIndex;
    f32 vertexPosition[3];
    Obj_GetPlayerObject();
    state = obj->extra;
    placement = (GroundAnimatorPlacement*)obj->anim.placementData;
    if (placement->blockId == 0) {
        return;
    }
    blockIndex =
        objPosToMapBlockIdx((double)obj->anim.localPosX, (double)obj->anim.localPosY, (double)obj->anim.localPosZ);
    previousOnMapFlag = state->flags & GROUND_ANIMATOR_STATE_ON_MAP;
    if (blockIndex > -1) {
        state->flags = state->flags | GROUND_ANIMATOR_STATE_ON_MAP;
    } else {
        state->flags = state->flags & ~GROUND_ANIMATOR_STATE_ON_MAP;
    }
    if ((state->flags & GROUND_ANIMATOR_STATE_ON_MAP) != previousOnMapFlag) {
        state->dirtyFrames = 2;
    }
    if ((state->flags & GROUND_ANIMATOR_STATE_ON_MAP) == 0) {
        return;
    }
    if ((state->flags & GROUND_ANIMATOR_STATE_ON_MAP) != 0 && state->falloffBuffer == NULL) {
        int bufferAddress;
        block = mapGetBlock(blockIndex);
        state->vertexCount = (s16)(mapBlockCountTrianglesByType(block, placement->blockId) * 3);
        if (state->vertexCount > 0) {
            bufferAddress = (int)mmAlloc(state->vertexCount * 6, 5, 0);
            state->falloffBuffer = (f32*)bufferAddress;
            state->baseHeightBuffer = (s16*)(bufferAddress + state->vertexCount * 4);
            GroundAnimator_gatherVertices(obj, state, placement);
        }
    }
    if (state->vertexCount == 0) {
        return;
    }
    if (placement->disableAutoLink == 0) {
        if (state->linkedObject == NULL) {
            searchDistance = (100.0f);
            state->linkedObject =
                (GameObject*)ObjGroup_FindNearestObject(GROUND_ANIMATOR_TARGET_OBJECT_GROUP, obj, &searchDistance);
            linkedObject = state->linkedObject;
            if (linkedObject != NULL) {
                switch (state->linkedObject->anim.seqId) {
                case GROUND_ANIMATOR_MOON_ROCK_SEQUENCE_ID:
                    if ((state->flags & GROUND_ANIMATOR_STATE_COMPLETE) == 0) {
                        mmpMoonRock_setFrozen((GameObject*)(linkedObject), 1);
                    }
                    mmpMoonRock_setPosition((GameObject*)(linkedObject), obj->anim.localPosX,
                                             obj->anim.localPosY - state->yOffset, obj->anim.localPosZ);
                    break;
                default:
                    if ((state->flags & GROUND_ANIMATOR_STATE_COMPLETE) == 0) {
                        (*(void (**)(void*, int))(*(int*)(*(int*)((char*)linkedObject + 0x68)) + 0x24))(linkedObject,
                                                                                                        1);
                    }
                    (*(void (**)(void*, f32, f32, f32))(*(int*)(*(int*)((char*)linkedObject + 0x68)) + 0x38))(
                        linkedObject, obj->anim.localPosX, obj->anim.localPosY - state->yOffset, obj->anim.localPosZ);
                    break;
                }
            }
        } else if ((state->linkedObject->objectFlags & OBJECT_OBJFLAG_FREED) != 0) {
            state->linkedObject = 0;
        }
    }
    block = mapGetBlock(blockIndex);
    if (block == NULL || (((MapBlockData*)block)->flags4 & MAP_BLOCK_FLAG_LOADED) == 0) {
        return;
    }
    if (state->sinkDepth > (0.0f)) {
        if ((state->flags & GROUND_ANIMATOR_STATE_PRESSED) != 0) {
            state->flags = state->flags & ~GROUND_ANIMATOR_STATE_PRESSED;
        } else if (state->sinkDepth < GROUND_ANIMATOR_SINK_DEPTH_SCALE * (f32)(u32)placement->maxSinkDepth) {
            state->sinkDepth = state->sinkDepth - timeDelta;
            if (state->sinkDepth < (0.0f)) {
                state->sinkDepth = (0.0f);
            }
        }
        if (state->sinkDepth != state->previousSinkDepth) {
            state->dirtyFrames = 2;
            state->previousSinkDepth = state->sinkDepth;
        }
        if (state->dirtyFrames != 0) {
            f32 maxSinkDepth;
            state->dirtyFrames -= 1;
            if (state->previousSinkDepth >
                (maxSinkDepth = GROUND_ANIMATOR_SINK_DEPTH_SCALE * (f32)(u32)placement->maxSinkDepth)) {
                state->previousSinkDepth = maxSinkDepth;
                state->sinkDepth = maxSinkDepth;
                if (state->linkedObject != NULL && state->linkedObject->extra != NULL) {
                    switch (state->linkedObject->anim.seqId) {
                    case GROUND_ANIMATOR_MOON_ROCK_SEQUENCE_ID:
                        mmpMoonRock_setFrozen(state->linkedObject, 0);
                        break;
                    default:
                        (*(void (**)(void*, int))(*(int*)(*(int*)((char*)state->linkedObject + 0x68)) + 0x24))(
                            state->linkedObject, 0);
                        break;
                    }
                }
                mainSetBits(placement->sunkGameBit, 1);
                state->flags = state->flags | GROUND_ANIMATOR_STATE_COMPLETE;
                Sfx_PlayFromObject((u32)obj, gGroundAnimatorSfxIds[placement->sfxIndex]);
            }
            offsets[0] = 0;
            offsets[1] = offsets[0];
            entryIndex = 0;
            for (; entryIndex < state->entryCount; entryIndex++) {
                polygonGroup = mapBlockGetPolygonGroup(block, state->blockEntries[entryIndex]);
                polygonIndex = *(u16*)polygonGroup;
                polygonFalloffOffset = offsets[0];
                polygonHeightOffset = offsets[1];
                for (; polygonIndex < *(u16*)((char*)polygonGroup + 0x14); polygonIndex++) {
                    polygon = mapBlockGetPolygon((int*)block, polygonIndex);
                    for (vertexIndex = 0, vertexFalloffOffset = polygonFalloffOffset, polygonIndexCursor = polygon,
                        vertexHeightOffset = polygonHeightOffset;
                         vertexIndex < 3; vertexIndex++) {
                        if (*(f32*)((char*)state->falloffBuffer + vertexFalloffOffset) > (0.0f)) {
                            void* packedVertex =
                                (char*)((MapBlockData*)block)->vertices + *(u16*)polygonIndexCursor * 6;
                            f32 sinkOffset;
                            trackUnpackVector(packedVertex, vertexPosition);
                            sinkOffset = (state->previousSinkDepth / GROUND_ANIMATOR_SINK_DEPTH_SCALE) *
                                         *(f32*)((char*)state->falloffBuffer + vertexFalloffOffset);
                            vertexPosition[1] =
                                (f32) * (s16*)((char*)state->baseHeightBuffer + vertexHeightOffset) - sinkOffset;
                            trackPackVector(packedVertex, vertexPosition);
                        }
                        vertexFalloffOffset += 4;
                        vertexHeightOffset += 2;
                        polygonFalloffOffset += 4;
                        polygonHeightOffset += 2;
                        offsets[0] += 4;
                        offsets[1] += 2;
                        polygonIndexCursor = (char*)polygonIndexCursor + 2;
                    }
                }
            }
            DCStoreRangeNoSync((void*)((MapBlockData*)block)->vertices, ((MapBlockData*)block)->vertexCount * 6);
        }
    }
    if (placement->enableGameBit == -1 || mainGetBit(placement->enableGameBit) != 0) {
        interactionEnabled = 1;
    } else {
        interactionEnabled = 0;
    }
    if ((state->flags & GROUND_ANIMATOR_STATE_COMPLETE) == 0 && interactionEnabled != 0) {
        tricky = getTrickyObject();
        if (tricky != NULL && mainGetBit(GAMEBIT_Tricky_Usable) != 0) {
            obj->anim.resetHitboxFlags = obj->anim.resetHitboxFlags & ~INTERACT_FLAG_PROMPT_SUPPRESSED;
        } else {
            obj->anim.resetHitboxFlags = obj->anim.resetHitboxFlags | INTERACT_FLAG_PROMPT_SUPPRESSED;
        }
        obj->anim.resetHitboxFlags = obj->anim.resetHitboxFlags & ~INTERACT_FLAG_DISABLED;
        if (tricky != NULL && (obj->anim.resetHitboxFlags & INTERACT_FLAG_IN_RANGE) != 0) {
            TRICKY_INTERFACE(tricky)->sideCommandEnable((GameObject*)tricky, obj, 1, 1);
        }
    } else {
        obj->anim.resetHitboxFlags = obj->anim.resetHitboxFlags | INTERACT_FLAG_DISABLED;
    }
    objRenderFn_80041018(obj);
}

void GroundAnimator_init(GameObject* obj, GroundAnimatorPlacement* placement) {
    GroundAnimatorState* state = obj->extra;
    state->magicCaveId = (u8)placement->magicCaveId;
    state->yOffset = placement->yOffset;
    state->previousSinkDepth = (-1.0f);
    state->radius = placement->radius;
    if (placement->blockId != 0) {
        if (mainGetBit(placement->sunkGameBit) != 0) {
            state->sinkDepth = GROUND_ANIMATOR_SINK_DEPTH_SCALE * placement->maxSinkDepth;
            state->flags |= GROUND_ANIMATOR_STATE_COMPLETE;
        }
        ObjGroup_AddObject((int)obj, GROUND_ANIMATOR_OBJECT_GROUP);
        if (placement->sfxIndex > 1) {
            placement->sfxIndex = 0;
        }
    }
}

ObjectDescriptor14 gGroundAnimatorObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_13_SLOTS,
    0,
    0,
    0,
    (ObjectDescriptorCallback)GroundAnimator_init,
    (ObjectDescriptorCallback)GroundAnimator_update,
    0,
    (ObjectDescriptorCallback)GroundAnimator_render,
    (ObjectDescriptorCallback)GroundAnimator_free,
    0,
    (ObjectDescriptorCallback)GroundAnimator_getExtraSize,
    (ObjectDescriptorCallback)GroundAnimator_applyPress,
    (ObjectDescriptorCallback)GroundAnimator_isFullySunk,
    (ObjectDescriptorCallback)GroundAnimator_getMagicCaveIndex,
    0,
};
