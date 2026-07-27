/*
 * Tumbleweed bush object (DLL slot 209).
 *
 * Manages a bush's detachable pieces and the shared motion helpers used by
 * spawned tumbleweed objects.
 */
#include "dlls/objects/209_TumbleWeedB.h"
#include "dlls/objects/210.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "game/objects/object.h"
#include "main/audio/sfx.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/frame_timing.h"
#include "main/object_render.h"
#include "main/obj_group.h"
#include "main/obj_list.h"
#include "main/objfx.h"
#include "main/objhits.h"
#include "main/sky_interface.h"
#include "main/track_dolphin_api.h"
#include "main/vecmath.h"
#include "main/vecmath_distance_api.h"
#include "sys/objects.h"
#include "sys/objects/lifecycle.h"
#include "string.h"

#define TUMBLEWEED_BUSH_SEQUENCE_A           0x28d
#define TUMBLEWEED_BUSH_SEQUENCE_B           0x3fd
#define TUMBLEWEED_BUSH_SEQUENCE_C           0x4b9
#define TUMBLEWEED_BUSH_SEQUENCE_D           0x4be
#define TUMBLEWEED_BUSH_SIBLING_A            0x39d
#define TUMBLEWEED_BUSH_SIBLING_B            0x3fb
#define TUMBLEWEED_BUSH_SIBLING_C            0x4ba
#define TUMBLEWEED_BUSH_SIBLING_D            0x4c1
#define TUMBLEWEED_BUSH_OBJECT_GROUP         0x31
#define TUMBLEWEED_BUSH_MAX_SIBLINGS         7
#define TUMBLEWEED_BUSH_PIECE_COUNT          3
#define TUMBLEWEED_BUSH_SIBLING_SETUP_SIZE   0x20
#define TUMBLEWEED_BUSH_SIBLING_SETUP_FLAGS  5
#define TUMBLEWEED_BUSH_ACTIVE_PIECE_PHASE   7
#define TUMBLEWEED_BUSH_QUERY_STATE_SLOT     8
#define TUMBLEWEED_BUSH_SET_ORIGIN_SLOT      9
#define TUMBLEWEED_BUSH_DETACH_SLOT          10
#define TUMBLEWEED_BUSH_HIT_EFFECT_ID        8
#define TUMBLEWEED_BUSH_HIT_COLOR_R          0xff
#define TUMBLEWEED_BUSH_HIT_COLOR_G          0xff
#define TUMBLEWEED_BUSH_HIT_COLOR_B          0x78
#define TUMBLEWEED_BUSH_ROTATION_CENTER      0x7f
#define TUMBLEWEED_BUSH_ROTATION_X_SHIFT     8
#define TUMBLEWEED_BUSH_ROTATION_YZ_SHIFT    7
#define TUMBLEWEED_BUSH_PIECE_SCALE          64.0f
#define TUMBLEWEED_BUSH_RENDER_SCALE         1.0f
#define TUMBLEWEED_BUSH_INIT_SCALE           0.0f
#define TUMBLEWEED_BUSH_HIT_RADIUS           15.0f
#define TUMBLEWEED_BUSH_HIT_Y_MIN            -5.0f
#define TUMBLEWEED_BUSH_HIT_Y_MAX            100.0f
#define TUMBLEWEED_BUSH_NEAREST_INITIAL_DIST 3.4028235e38f
#define TUMBLEWEED_PIECE_HORIZONTAL_DAMPING  10.0f
#define TUMBLEWEED_PIECE_GROUND_CLEARANCE    7.0f
#define TUMBLEWEED_PIECE_GRAVITY             -0.17f
#define TUMBLEWEED_PIECE_ROTATION_DAMPING    100

f32 gTumbleweedBushHitCooldown;

s8 tumbleweedbush_spawnSibling(GameObject* obj) {
    TumbleweedBushState* state;
    TumbleweedBushPlacement* placement;
    int siblingSeqId;
    int objectIndex;
    int objectCount;
    f32 sunTime;
    int freePieceIndex;
    GameObject** objects;
    int siblingCount;
    TumbleweedBushPlacement* newPlacement;

    state = obj->extra;
    placement = (TumbleweedBushPlacement*)obj->anim.placementData;
    switch (obj->anim.seqId) {
    case TUMBLEWEED_BUSH_SEQUENCE_A:
        if ((*gSkyInterface)->getSunPosition(&sunTime) == 0) {
            return -1;
        }
        siblingSeqId = TUMBLEWEED_BUSH_SIBLING_A;
        break;
    case TUMBLEWEED_BUSH_SEQUENCE_B:
        siblingSeqId = TUMBLEWEED_BUSH_SIBLING_B;
        break;
    case TUMBLEWEED_BUSH_SEQUENCE_C:
        siblingSeqId = TUMBLEWEED_BUSH_SIBLING_C;
        break;
    case TUMBLEWEED_BUSH_SEQUENCE_D:
        siblingSeqId = TUMBLEWEED_BUSH_SIBLING_D;
        break;
    }

    objectIndex = 0;
    freePieceIndex = -1;
    while (objectIndex < (int)state->pieceCount && freePieceIndex == -1) {
        if (state->pieceObjects[objectIndex] == NULL) {
            freePieceIndex = objectIndex;
        }
        objectIndex++;
    }
    if (freePieceIndex == -1) {
        return -1;
    }

    objects = (GameObject**)ObjList_GetObjects(&objectIndex, &objectCount);
    siblingCount = 0;
    while (objectIndex < objectCount) {
        int currentIndex = *(int*)&objectIndex;

        objectIndex = currentIndex + 1;
        if (siblingSeqId == objects[currentIndex]->anim.seqId) {
            siblingCount++;
        }
    }
    if (siblingCount >= TUMBLEWEED_BUSH_MAX_SIBLINGS) {
        return -1;
    }
    if (Obj_IsLoadingLocked() == 0) {
        return -1;
    }

    newPlacement = (TumbleweedBushPlacement*)Obj_AllocObjectSetup(TUMBLEWEED_BUSH_SIBLING_SETUP_SIZE, siblingSeqId);
    ((ObjPlacement*)newPlacement)->posX = obj->anim.localPosX + state->pieceOffsets[freePieceIndex][0];
    ((ObjPlacement*)newPlacement)->posY = obj->anim.localPosY + state->pieceOffsets[freePieceIndex][1];
    ((ObjPlacement*)newPlacement)->posZ = obj->anim.localPosZ + state->pieceOffsets[freePieceIndex][2];
    newPlacement->base.color[0] = placement->base.color[0];
    newPlacement->base.color[1] = placement->base.color[1];
    newPlacement->base.color[2] = placement->base.color[2];
    newPlacement->base.color[3] = placement->base.color[3];
    newPlacement->scale = TUMBLEWEED_BUSH_PIECE_SCALE;

    if ((state->variant & 1) != 0) {
        switch (((ObjPlacement*)obj->anim.placementData)->mapId) {
        case 0x292c:
            if (state->spawnedCount == 6) {
                newPlacement->radiusByte = 1;
                objects = (GameObject**)ObjList_GetObjects(&objectIndex, &objectCount);
                while (objectIndex < objectCount) {
                    GameObject* markerObj = objects[objectIndex];

                    if (markerObj->anim.seqId == 0x27f) {
                        ((ObjPlacement*)newPlacement)->posX = markerObj->anim.localPosX;
                        ((ObjPlacement*)newPlacement)->posY = objects[objectIndex]->anim.localPosY;
                        ((ObjPlacement*)newPlacement)->posZ = objects[objectIndex]->anim.localPosZ;
                        objectIndex = objectCount;
                    }
                    objectIndex++;
                }
            }
            break;
        }
    }

    {
        GameObject* spawnedObj = Obj_SetupObject((ObjPlacement*)newPlacement, TUMBLEWEED_BUSH_SIBLING_SETUP_FLAGS,
                                                 obj->anim.mapEventSlot, -1, obj->anim.parent);

        state->pieceObjects[freePieceIndex] = spawnedObj;
        {
            GameObject* spawnedPiece = state->pieceObjects[freePieceIndex];

            ((void (*)(GameObject*, f64, f64))(*spawnedPiece->anim.dll)[TUMBLEWEED_BUSH_SET_ORIGIN_SLOT])(
                spawnedPiece, (f64)obj->anim.localPosX, (f64)obj->anim.localPosZ);
        }
    }
    state->spawnedCount++;
    return freePieceIndex;
}

void tumbleweedbush_removePieceReference(GameObject* obj, GameObject* piece) {
    TumbleweedBushState* state;
    int pieceIndex;

    state = obj->extra;
    pieceIndex = 0;
    while (pieceIndex < state->pieceCount) {
        if (state->pieceObjects[pieceIndex] == piece) {
            state->pieceObjects[pieceIndex] = NULL;
        }
        pieceIndex++;
    }
}

int TumbleWeedBush_getExtraSize(void) {
    return sizeof(TumbleweedBushState);
}

int TumbleWeedBush_getObjectTypeId(void) {
    return 0;
}

void TumbleWeedBush_free(GameObject* obj) {
    (void)obj;
}

void TumbleWeedBush_render(GameObject* obj, int fwdArg2, int fwdArg3, int fwdArg4, int fwdArg5, s8 visible) {
    s32 visibleInt = visible;

    if (visibleInt != 0) {
        objRenderModelAndHitVolumes(obj, fwdArg2, fwdArg3, fwdArg4, fwdArg5, TUMBLEWEED_BUSH_RENDER_SCALE);
    }
}

void TumbleWeedBush_hitDetect(GameObject* obj) {
    (void)obj;
}

void TumbleWeedBush_update(GameObject* obj) {
    TumbleweedBushState* state;
    GameObject* player;
    f32 hitPos[3];
    f32 sunTime;
    GameObject* hitObject;
    f32 deltaX, deltaZ, distance;
    GameObject** pieceSlot;
    int pieceIndex;

    state = obj->extra;
    player = Obj_GetPlayerObject();
    if (ObjHits_PollPriorityHitWithCooldown(obj, &gTumbleweedBushHitCooldown, (int*)&hitObject, hitPos) != 0) {
        if (hitObject->anim.seqId != TUMBLEWEED_BUSH_SIBLING_C) {
            objfx_spawnHitEmitterAtPos(hitPos, TUMBLEWEED_BUSH_HIT_EFFECT_ID, TUMBLEWEED_BUSH_HIT_COLOR_R,
                                       TUMBLEWEED_BUSH_HIT_COLOR_G, TUMBLEWEED_BUSH_HIT_COLOR_B);
            Sfx_PlayFromObject((u32)obj, SFXTRIG_wp_swdtest222_280);
            for (pieceIndex = 0; (u8)pieceIndex < state->pieceCount; pieceIndex++) {
                pieceSlot = &state->pieceObjects[(u8)pieceIndex];
                if (*pieceSlot != NULL) {
                    if (obj->anim.seqId == TUMBLEWEED_BUSH_SEQUENCE_A) {
                        if ((*gSkyInterface)->getSunPosition(&sunTime) == 0) {
                            continue;
                        }
                    }
                    ((void (*)(GameObject*))(*(*pieceSlot)->anim.dll)[TUMBLEWEED_BUSH_DETACH_SLOT])(*pieceSlot);
                }
            }
        }
    }
    deltaX = obj->anim.localPosX - player->anim.localPosX;
    deltaZ = obj->anim.localPosZ - player->anim.localPosZ;
    distance = sqrtf(deltaX * deltaX + deltaZ * deltaZ);
    if ((u16)(s32)distance < state->triggerRadius) {
        while (tumbleweedbush_spawnSibling(obj) != -1) {
        }
    }
    pieceIndex = 0;
    for (; (u8)pieceIndex < state->pieceCount; pieceIndex++) {
        pieceSlot = &state->pieceObjects[(u8)pieceIndex];
        if (*pieceSlot != NULL) {
            if (((int (*)(GameObject*))(*(*pieceSlot)->anim.dll)[TUMBLEWEED_BUSH_QUERY_STATE_SLOT])(*pieceSlot) > 1) {
                *pieceSlot = NULL;
            }
        }
    }
}

void TumbleWeedBush_init(GameObject* obj, TumbleweedBushPlacement* placement, int flags) {
    static const f32 sTumbleweedBushInitScale[1] = { TUMBLEWEED_BUSH_INIT_SCALE };
    TumbleweedBushState* state;
    f32 scale;
    int offsetTableIndex;
    int pieceIndex;

    state = obj->extra;
    state->scale = sTumbleweedBushInitScale[0];
    state->triggerRadius = (u16)(placement->radiusByte * 2);
    state->variant = placement->variant;
    obj->anim.rotZ =
        (s16)((placement->rotZByte - TUMBLEWEED_BUSH_ROTATION_CENTER) << TUMBLEWEED_BUSH_ROTATION_YZ_SHIFT);
    obj->anim.rotY =
        (s16)((placement->rotYByte - TUMBLEWEED_BUSH_ROTATION_CENTER) << TUMBLEWEED_BUSH_ROTATION_YZ_SHIFT);
    obj->anim.rotX = (s16)(placement->rotXByte << TUMBLEWEED_BUSH_ROTATION_X_SHIFT);
    obj->anim.rootMotionScale = placement->scale;
    scale = obj->anim.rootMotionScale;
    ObjHitbox_SetCapsuleBounds((ObjAnimComponent*)obj, (s32)(TUMBLEWEED_BUSH_HIT_RADIUS * scale),
                               (s32)(TUMBLEWEED_BUSH_HIT_Y_MIN * scale), (s32)(TUMBLEWEED_BUSH_HIT_Y_MAX * scale));
    switch (obj->anim.seqId) {
    case TUMBLEWEED_BUSH_SEQUENCE_A:
    case TUMBLEWEED_BUSH_SEQUENCE_C:
    case TUMBLEWEED_BUSH_SEQUENCE_D:
        state->pieceCount = TUMBLEWEED_BUSH_PIECE_COUNT;
        offsetTableIndex = 0;
        break;
    case TUMBLEWEED_BUSH_SEQUENCE_B:
        state->pieceCount = TUMBLEWEED_BUSH_PIECE_COUNT;
        offsetTableIndex = 1;
        break;
    }
    if (flags == 0) {
        pieceIndex = 0;
        for (; pieceIndex < state->pieceCount; pieceIndex++) {
            state->pieceObjects[pieceIndex] = NULL;
            memcpy(state->pieceOffsets[pieceIndex], gTumbleweedBushPieceOffsetTable[offsetTableIndex][pieceIndex],
                   sizeof(state->pieceOffsets[pieceIndex]));
            state->pieceOffsets[pieceIndex][0] *= obj->anim.rootMotionScale;
            state->pieceOffsets[pieceIndex][1] *= obj->anim.rootMotionScale;
            state->pieceOffsets[pieceIndex][2] *= obj->anim.rootMotionScale;
            vecRotateZXY((s16*)obj, state->pieceOffsets[pieceIndex]);
        }
    }
}

void TumbleWeedBush_release(void) {
}

void TumbleWeedBush_initialise(void) {
}

GameObject* tumbleweedbush_findNearestActive(f32* position) {
    int objectCount;
    GameObject** objects;
    f32 nearestDistance;
    int objectIndex;
    GameObject* nearestObj;

    nearestDistance = TUMBLEWEED_BUSH_NEAREST_INITIAL_DIST;
    nearestObj = NULL;
    {
        GameObject** objectList = (GameObject**)ObjGroup_GetObjects(TUMBLEWEED_BUSH_OBJECT_GROUP, &objectCount);

        objectIndex = 0;
        objects = objectList;
    }
    while (objectIndex < objectCount) {
        if ((*objects)->anim.seqId == TUMBLEWEED_BUSH_SIBLING_B) {
            if (((TumbleweedState*)(*objects)->extra)->phase > TUMBLEWEED_PHASE_ARMED) {
                f32 distance = vec3f_distanceSquared(&(*objects)->anim.worldPosX, position);

                if (distance < nearestDistance) {
                    nearestDistance = distance;
                    nearestObj = *objects;
                }
            }
        }
        objects++;
        objectIndex++;
    }
    return nearestObj;
}

void tumbleweedbush_activatePiece(GameObject* obj) {
    TumbleweedState* state = obj->extra;

    state->phase = TUMBLEWEED_BUSH_ACTIVE_PIECE_PHASE;
}

const f32 gTumbleweedPieceHorizontalDamping[1] = { 10.0f };
const f32 gTumbleweedPieceGroundClearance[1] = { 7.0f };
const f32 gTumbleweedPieceGravity[1] = { -0.17f };
const f32 gTumbleweedPieceRestVelocity[1] = { 0.0f };

void tumbleweedbush_updateDetachedPiece(GameObject* piece, TumbleweedState* state) {
    f32 groundDistance;

    piece->anim.velocityX /= gTumbleweedPieceHorizontalDamping[0];
    if (fn_80065684(piece, piece->anim.localPosX, piece->anim.localPosY, piece->anim.localPosZ, &groundDistance, 0) !=
        0) {
        if (groundDistance > gTumbleweedPieceGroundClearance[0]) {
            piece->anim.velocityY += gTumbleweedPieceGravity[0] * timeDelta;
        } else {
            piece->anim.localPosY -= groundDistance - gTumbleweedPieceGroundClearance[0];
            piece->anim.velocityY = gTumbleweedPieceRestVelocity[0];
        }
    }
    piece->anim.velocityZ /= gTumbleweedPieceHorizontalDamping[0];

    state->rotVelocityZ /= TUMBLEWEED_PIECE_ROTATION_DAMPING;
    state->rotVelocityY /= TUMBLEWEED_PIECE_ROTATION_DAMPING;
    state->rotVelocityX /= TUMBLEWEED_PIECE_ROTATION_DAMPING;

    piece->anim.localPosX += piece->anim.velocityX * timeDelta;
    piece->anim.localPosY += piece->anim.velocityY * timeDelta;
    piece->anim.localPosZ += piece->anim.velocityZ * timeDelta;

    piece->anim.rotZ += (f32)(int)state->rotVelocityZ * timeDelta;
    piece->anim.rotY += (f32)(int)state->rotVelocityY * timeDelta;
    piece->anim.rotX += (f32)(int)state->rotVelocityX * timeDelta;
}

f32 gTumbleweedBushPieceOffsetTable[2][4][3] = {
    {
        {-22.0f, 56.0f, 0.0f},
        {0.0f, 95.0f, 54.0f},
        {18.0f, 90.0f, -12.0f},
        {0.0f, 0.0f, 0.0f},
    },
    {
        {-22.0f, 56.0f, 0.0f},
        {0.0f, 80.0f, 54.0f},
        {18.0f, 90.0f, -12.0f},
        {-60.0f, 88.0f, 0.0f},
    },
};

ObjectDescriptor11WithPadding gTumbleWeedBushObjDescriptor = {
    {
        0,
        0,
        0,
        OBJECT_DESCRIPTOR_FLAGS_11_SLOTS,
        (ObjectDescriptorCallback)TumbleWeedBush_initialise,
        (ObjectDescriptorCallback)TumbleWeedBush_release,
        0,
        (ObjectDescriptorCallback)TumbleWeedBush_init,
        (ObjectDescriptorCallback)TumbleWeedBush_update,
        (ObjectDescriptorCallback)TumbleWeedBush_hitDetect,
        (ObjectDescriptorCallback)TumbleWeedBush_render,
        (ObjectDescriptorCallback)TumbleWeedBush_free,
        (ObjectDescriptorCallback)TumbleWeedBush_getObjectTypeId,
        TumbleWeedBush_getExtraSize,
        (ObjectDescriptorCallback)tumbleweedbush_removePieceReference,
    },
    0,
};
