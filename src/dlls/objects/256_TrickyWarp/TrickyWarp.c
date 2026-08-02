/*
 * DLL 0x100 - TrickyWarp.
 *
 * Tracks player/Tricky reachability through walk patch groups and '$'
 * rom-curve links, registering off-screen objects as warp candidates.
 */
#include "dlls/objects/256_TrickyWarp.h"
#include "main/frustum.h"
#include "main/dll/rom_curve_interface.h"
#include "main/gamebits.h"
#include "sys/objects.h"
#include "sys/objects/lifecycle.h"
#include "main/dll/dll_0014_api.h"
#include "main/dll/objfsa_query_api.h"
#include "main/objtype.h"

typedef struct TrickyWarpCurveEntry {
    u8 pad00[3];           /* 0x00 */
    u8 entryPatchGroup;    /* 0x03 */
    u8 linkPatchGroups[4]; /* 0x04 */
    u8 pad08[0xC];         /* 0x08 */
    u32 nodeId;            /* 0x14 */
    s8 action;             /* 0x18 */
    s8 type;               /* 0x19 */
} TrickyWarpCurveEntry;

typedef struct TrickyWarpCurveNode {
    u8 pad00[4];           /* 0x00 */
    u8 linkPatchGroups[4]; /* 0x04 */
    u8 pad08[0x28];        /* 0x08 */
    s16 requiredGameBit;   /* 0x30 */
    s16 forbiddenGameBit;  /* 0x32 */
} TrickyWarpCurveNode;

STATIC_ASSERT(offsetof(TrickyWarpCurveEntry, pad00) == 0x0);
STATIC_ASSERT(offsetof(TrickyWarpCurveEntry, entryPatchGroup) == 0x3);
STATIC_ASSERT(offsetof(TrickyWarpCurveEntry, linkPatchGroups) == 0x4);
STATIC_ASSERT(offsetof(TrickyWarpCurveEntry, pad08) == 0x8);
STATIC_ASSERT(offsetof(TrickyWarpCurveEntry, nodeId) == 0x14);
STATIC_ASSERT(offsetof(TrickyWarpCurveEntry, action) == 0x18);
STATIC_ASSERT(offsetof(TrickyWarpCurveEntry, type) == 0x19);

STATIC_ASSERT(offsetof(TrickyWarpCurveNode, pad00) == 0x0);
STATIC_ASSERT(offsetof(TrickyWarpCurveNode, linkPatchGroups) == 0x4);
STATIC_ASSERT(offsetof(TrickyWarpCurveNode, pad08) == 0x8);
STATIC_ASSERT(offsetof(TrickyWarpCurveNode, requiredGameBit) == 0x30);
STATIC_ASSERT(offsetof(TrickyWarpCurveNode, forbiddenGameBit) == 0x32);

#define TRICKYWARP_OBJ_GROUP          0x4B
#define TRICKYWARP_PATCH_GROUP_NONE   0
#define TRICKYWARP_CURVE_NODE_ID_NONE 0
#define TRICKYWARP_GAMEBIT_NONE       -1
#define TRICKYWARP_CURVE_LINK_COUNT   4
#define ROMCURVE_TYPE_TRICKYWARP      '$'
#define TRICKYWARP_VISIBILITY_RADIUS  19.0f

void TrickyWarp_free(GameObject* obj) {
    TrickyWarpState* state = obj->extra;
    if (state->active != 0) {
        objFreeObjectType((int)obj, TRICKYWARP_OBJ_GROUP);
    }
}

int TrickyWarp_getExtraSize(void) {
    return sizeof(TrickyWarpState);
}

void TrickyWarp_update(GameObject* obj) {
    TrickyWarpState* state;
    int isReachable;

    state = obj->extra;
    isReachable = TrickyWarp_isPlayerReachable(obj, state);
    if (isReachable != 0) {
        if (state->active == 0) {
            state->active = 1;
            objAddObjectType((int)obj, TRICKYWARP_OBJ_GROUP);
        }
    } else {
        if (state->active != 0) {
            state->active = 0;
            objFreeObjectType((int)obj, TRICKYWARP_OBJ_GROUP);
        }
    }
}

int TrickyWarp_isPlayerReachable(GameObject* obj, TrickyWarpState* state) {
    int curveCount;
    TrickyWarpCurveEntry** curveEntries;
    int curveIndex;
    int linkIndex;
    TrickyWarpCurveEntry* curveEntry;
    TrickyWarpCurveNode* curveNode;
    int nodeCount;
    GameObject* player;
    int playerPatchGroup;

    if (mainGetBit(GAMEBIT_TrickyWarpEnabled) == 0) {
        return 0;
    }
    if (getTrickyObject() == NULL) {
        return 0;
    }
    if (state->patchGroup == TRICKYWARP_PATCH_GROUP_NONE) {
        state->patchGroup = Objfsa_GetWalkGroupIndexAtPoint(&obj->anim.localPosX, 0);
        if (state->patchGroup != TRICKYWARP_PATCH_GROUP_NONE) {
            curveEntries = (TrickyWarpCurveEntry**)(*gRomCurveInterface)->getCurves(&curveCount);
            nodeCount = 0;
            for (curveIndex = 0; curveIndex < curveCount; curveIndex++) {
                curveEntry = curveEntries[curveIndex];
                if (curveEntry->type == ROMCURVE_TYPE_TRICKYWARP &&
                    curveEntry->entryPatchGroup == TRICKYWARP_PATCH_GROUP_NONE) {
                    for (linkIndex = 0; linkIndex < TRICKYWARP_CURVE_LINK_COUNT; linkIndex++) {
                        if (curveEntry->linkPatchGroups[linkIndex] == state->patchGroup) {
                            state->curveNodeIds[nodeCount] = curveEntry->nodeId;
                            nodeCount++;
                            break;
                        }
                    }
                }
            }
        } else {
            return 0;
        }
    }
    if (ViewFrustum_IsSphereVisible(&obj->anim.localPosX, TRICKYWARP_VISIBILITY_RADIUS) != 0) {
        return 0;
    }
    player = Obj_GetPlayerObject();
    playerPatchGroup = Objfsa_GetWalkGroupIndexAtPoint(&player->anim.localPosX, 0);
    if (playerPatchGroup != TRICKYWARP_PATCH_GROUP_NONE) {
        if (playerPatchGroup == state->patchGroup) {
            return 1;
        }
        for (curveIndex = 0; curveIndex < TRICKYWARP_CURVE_NODE_CAPACITY; curveIndex++) {
            if (state->curveNodeIds[curveIndex] == TRICKYWARP_CURVE_NODE_ID_NONE) {
                break;
            }
            curveNode = (TrickyWarpCurveNode*)(*gRomCurveInterface)->getById(state->curveNodeIds[curveIndex]);
            if (curveNode != NULL) {
                if (curveNode->requiredGameBit == TRICKYWARP_GAMEBIT_NONE ||
                    mainGetBit(curveNode->requiredGameBit) != 0) {
                    if (curveNode->forbiddenGameBit == TRICKYWARP_GAMEBIT_NONE ||
                        mainGetBit(curveNode->forbiddenGameBit) == 0) {
                        if (curveNode->linkPatchGroups[0] == playerPatchGroup) {
                            return 1;
                        }
                        if (curveNode->linkPatchGroups[1] == playerPatchGroup) {
                            return 1;
                        }
                        if (curveNode->linkPatchGroups[2] == playerPatchGroup) {
                            return 1;
                        }
                        if (curveNode->linkPatchGroups[3] == playerPatchGroup) {
                            return 1;
                        }
                    }
                }
            }
        }
    }
    return getPatchGroup(&player->anim.localPosX, state->patchGroup);
}

void TrickyWarp_init(GameObject* obj, TrickyWarpPlacement* placement) {
    u32 flags;
    flags = obj->objectFlags;
    flags |= OBJECT_OBJFLAG_HIDDEN;
    obj->objectFlags = flags;
    obj->anim.rotX = (s16)((u32)placement->rotXByte << 8);
}

ObjectDescriptor gTrickyWarpObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    0,
    0,
    0,
    (ObjectDescriptorCallback)TrickyWarp_init,
    (ObjectDescriptorCallback)TrickyWarp_update,
    0,
    0,
    (ObjectDescriptorCallback)TrickyWarp_free,
    0,
    TrickyWarp_getExtraSize,
};
