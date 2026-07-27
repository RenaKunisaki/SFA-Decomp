/*
 * WispBaddie object (DLL slot 225).
 *
 * Follows a ROM curve, chases nearby players, and owns the family tables
 * shared by the sequence-driven baddie objects.
 */
#include "dlls/objects/225_WispBaddie.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "game/objects/object.h"
#include "main/audio/sfx_play_api.h"
#include "main/audio/sfx_position_api.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/curve.h"
#include "main/dll/partfx_interface.h"
#include "main/dll/rom_curve_interface.h"
#include "main/frame_timing.h"
#include "main/mm.h"
#include "main/obj_group.h"
#include "main/objhits.h"
#include "main/vecmath.h"
#include "string.h"
#include "sys/objects.h"

#define WISPBADDIE_HIT_VOLUME_SLOT 10

#define WISPBADDIE_OBJECT_GROUP 3

#define WISPBADDIE_FLAG_PATH_NEEDS_LINK 0x01
#define WISPBADDIE_FLAG_CHASE_PLAYER    0x02
#define WISPBADDIE_FLAG_CHASE_LOCKOUT   0x04
#define WISPBADDIE_FLAG_CHASE_MASK      0x06

#define WISPBADDIE_PI              3.1415927f
#define WISPBADDIE_S16_ANGLE_SCALE 32768.0f

int gWispBaddieCurveInitData[2] = {2, 3};
int gWispBaddieLastSegmentEnd;

ObjectDescriptor gWispBaddieObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)WispBaddie_initialise,
    (ObjectDescriptorCallback)WispBaddie_release,
    0,
    (ObjectDescriptorCallback)WispBaddie_init,
    (ObjectDescriptorCallback)WispBaddie_update,
    (ObjectDescriptorCallback)WispBaddie_hitDetect,
    (ObjectDescriptorCallback)WispBaddie_render,
    (ObjectDescriptorCallback)WispBaddie_free,
    (ObjectDescriptorCallback)WispBaddie_getObjectTypeId,
    WispBaddie_getExtraSize,
};

void WispBaddie_updateMovement(GameObject* obj, WispBaddieState* state) {
    RomCurveWalker* curve;
    int pathEnded;
    f32 step;
    f32 wave;

    curve = state->curve;
    state->pathWavePhase += (s16)(512.0f * timeDelta);
    state->hoverWavePhase += (s16)(2048.0f * timeDelta);

    wave = 1.0f + mathSinf((WISPBADDIE_PI * (f32)state->pathWavePhase) / WISPBADDIE_S16_ANGLE_SCALE);
    pathEnded = Curve_AdvanceAlongPath(&curve->curve, state->hitRadius * wave);
    if (((pathEnded != 0) || (curve->atSegmentEnd != gWispBaddieLastSegmentEnd)) &&
        ((*gRomCurveInterface)->goNextPoint((void*)curve) != 0) &&
        ((*gRomCurveInterface)->initCurve((void*)state->curve, (void*)obj, 400.0f, gWispBaddieCurveInitData, -1) !=
         0)) {
        state->flags = (u8)(state->flags & ~WISPBADDIE_FLAG_PATH_NEEDS_LINK);
    }
    gWispBaddieLastSegmentEnd = curve->atSegmentEnd;

    if ((state->flags & WISPBADDIE_FLAG_CHASE_PLAYER) != 0) {
        obj->anim.velocityX = 0.006f * (state->player->anim.localPosX - obj->anim.localPosX) + obj->anim.velocityX;

        wave = mathSinf((WISPBADDIE_PI * (f32)state->hoverWavePhase) / WISPBADDIE_S16_ANGLE_SCALE);
        wave = ((30.0f + state->player->anim.localPosY) + 40.0f * wave) - obj->anim.localPosY;
        obj->anim.velocityY = 0.006f * wave + obj->anim.velocityY;
        obj->anim.velocityZ = 0.006f * (state->player->anim.localPosZ - obj->anim.localPosZ) + obj->anim.velocityZ;
    } else {
        obj->anim.velocityX = 0.006f * (curve->posX - obj->anim.localPosX) + obj->anim.velocityX;

        wave = mathSinf((WISPBADDIE_PI * (f32)state->hoverWavePhase) / WISPBADDIE_S16_ANGLE_SCALE);
        wave = (40.0f * wave + curve->posY) - obj->anim.localPosY;
        obj->anim.velocityY = 0.006f * wave + obj->anim.velocityY;
        obj->anim.velocityZ = 0.006f * (curve->posZ - obj->anim.localPosZ) + obj->anim.velocityZ;
    }

    obj->anim.velocityX = obj->anim.velocityX * (step = 0.9f);
    obj->anim.velocityY *= step;
    obj->anim.velocityZ *= step;

    if (obj->anim.velocityX > 2.1f) {
        obj->anim.velocityX = 2.1f;
    }
    if (obj->anim.velocityY > 2.1f) {
        obj->anim.velocityY = 2.1f;
    }
    if (obj->anim.velocityZ > 2.1f) {
        obj->anim.velocityZ = 2.1f;
    }
    if (obj->anim.velocityX < -2.1f) {
        obj->anim.velocityX = -2.1f;
    }
    if (obj->anim.velocityY < -2.1f) {
        obj->anim.velocityY = -2.1f;
    }
    if (obj->anim.velocityZ < -2.1f) {
        obj->anim.velocityZ = -2.1f;
    }

    (void)objMove(obj, obj->anim.velocityX * timeDelta, obj->anim.velocityY * timeDelta,
                  obj->anim.velocityZ * timeDelta);
}

int WispBaddie_getExtraSize(void) {
    return sizeof(WispBaddieState);
}

int WispBaddie_getObjectTypeId(void) {
    return 0x9;
}

void WispBaddie_free(GameObject* obj) {
    WispBaddieState* state = obj->extra;
    ObjGroup_RemoveObject((int)obj, WISPBADDIE_OBJECT_GROUP);
    if (state->curve != NULL) {
        mm_free(state->curve);
        state->curve = NULL;
    }
}

void WispBaddie_render(GameObject* obj, int fwdArg2, int fwdArg3, int fwdArg4, int fwdArg5, s8 visible) {
    (void)obj;
    (void)fwdArg2;
    (void)fwdArg3;
    (void)fwdArg4;
    (void)fwdArg5;

    if (visible == 0)
        return;
}

void WispBaddie_hitDetect(GameObject* obj) {
    (void)obj;
}

void WispBaddie_update(GameObject* obj) {
    WispBaddieState* state;
    RomCurveWalker* curve;
    int hitPriority;
    f32 hitObjectBits;
    f32 hitPosX;
    f32 hitPosY;
    f32 hitPosZ;
    f32 hitSphereIndexBits;
    f32 hitVolumeBits;
    f32 delta[3];
    int particleMode;
    u8 flags;
    void* deltaAlias = (void*)delta;

    state = obj->extra;
    curve = state->curve;
    hitPriority = ObjHits_GetPriorityHitWithPosition(obj, (int*)&hitObjectBits, (int*)&hitSphereIndexBits,
                                                     (u32*)&hitVolumeBits, &hitPosX, &hitPosY, &hitPosZ);
    if (hitPriority != 0) {
        state->hitRadius = 0.01f;
        flags = state->flags;
        if ((flags & WISPBADDIE_FLAG_CHASE_PLAYER) != 0) {
            state->flags = (u8)(flags & ~WISPBADDIE_FLAG_CHASE_PLAYER);
            state->flags = (u8)(state->flags | WISPBADDIE_FLAG_CHASE_LOCKOUT);
        }
        Sfx_PlayAtPositionFromObject((int)obj, hitPosX, hitPosY, hitPosZ, SFXTRIG_robolaser16);
    }

    particleMode = 4;
    (*gPartfxInterface)->spawnObject((void*)obj, state->particleId, NULL, 1, -1, &particleMode);
    particleMode = 3;
    (*gPartfxInterface)->spawnObject((void*)obj, state->particleId, NULL, 2, -1, &particleMode);

    if (state->hitRadius < state->maxHitRadius) {
        state->hitRadius += 0.005f;
        ObjHits_DisableObject(obj);
    } else {
        state->hitRadius = state->maxHitRadius;
        particleMode = 2;
        (*gPartfxInterface)->spawnObject((void*)obj, state->particleId, NULL, 2, -1, &particleMode);
        particleMode = 0;
        (*gPartfxInterface)->spawnObject((void*)obj, state->particleId, NULL, 2, -1, &particleMode);
        ObjHits_SetHitVolumeSlot((ObjAnimComponent*)obj, WISPBADDIE_HIT_VOLUME_SLOT, 1, 0);
        ObjHits_EnableObject(obj);
    }

    particleMode = 1;
    (*gPartfxInterface)->spawnObject((void*)obj, state->particleId, NULL, 2, -1, &particleMode);
    state->player = Obj_GetPlayerObject();
    if (state->player != NULL) {
        delta[0] = state->player->anim.worldPosX - obj->anim.worldPosX;
        delta[1] = state->player->anim.worldPosY - obj->anim.worldPosY;
        delta[2] = state->player->anim.worldPosZ - obj->anim.worldPosZ;
        state->playerDistance = sqrtf(delta[2] * delta[2] + (delta[0] * delta[0] + delta[1] * delta[1]));
    }
    if (curve != NULL) {
        delta[0] = curve->posX - obj->anim.worldPosX;
        delta[1] = curve->posY - obj->anim.worldPosY;
        delta[2] = curve->posZ - obj->anim.worldPosZ;
        state->curveDistance = sqrtf(delta[2] * delta[2] + (delta[0] * delta[0] + delta[1] * delta[1]));
    }

    flags = state->flags;
    if ((flags & WISPBADDIE_FLAG_CHASE_PLAYER) != 0) {
        if (state->curveDistance > 250.0f) {
            state->flags = (u8)(flags & ~WISPBADDIE_FLAG_CHASE_PLAYER);
            state->flags = (u8)(state->flags | WISPBADDIE_FLAG_CHASE_LOCKOUT);
        }
        state->cryTimer -= timeDelta;
        if (state->cryTimer < 0.0f) {
            Sfx_PlayFromObject((u32)obj, SFXTRIG_fball2_c);
            state->cryTimer = (f32)randomGetRange(60, 120);
        }
        state->particleId = 0x338;
    }
    flags = state->flags;
    if ((flags & WISPBADDIE_FLAG_CHASE_LOCKOUT) != 0) {
        if (state->curveDistance < 60.0f) {
            state->flags = (u8)(flags & ~WISPBADDIE_FLAG_CHASE_LOCKOUT);
        }
        state->particleId = 0x337;
    }
    if ((state->flags & WISPBADDIE_FLAG_CHASE_MASK) == 0) {
        if ((state->hitRadius >= state->maxHitRadius) && (state->player != NULL) &&
            (state->playerDistance < state->triggerDistance)) {
            state->flags = (u8)(state->flags | WISPBADDIE_FLAG_CHASE_PLAYER);
        }
        state->particleId = 0x337;
    }
    WispBaddie_updateMovement(obj, state);
}

void WispBaddie_init(GameObject* obj, WispBaddiePlacement* placement, int skipAlloc) {
    WispBaddieState* state;
    f32 value;

    state = obj->extra;
    value = (f32)placement->maxHitRadiusParameter / 25.0f;
    state->maxHitRadius = value;
    state->hitRadius = value;
    state->triggerDistance = 4.0f * (f32)placement->triggerDistanceScale;
    state->particleId = 0x337;

    if (skipAlloc == 0) {
        state->curve = mmAlloc(sizeof(RomCurveWalker), 0x1A, 0);
        if (state->curve != NULL) {
            (void)memset(state->curve, 0, sizeof(RomCurveWalker));
        }
        if ((*gRomCurveInterface)
                ->initCurve((void*)state->curve, (void*)obj, state->triggerDistance, gWispBaddieCurveInitData, -1) ==
            0) {
            state->flags = (u8)(state->flags | WISPBADDIE_FLAG_PATH_NEEDS_LINK);
        }
        Sfx_PlayFromObject((u32)obj, SFXTRIG_id_23b);
    }
    obj->objectFlags = (u16)(obj->objectFlags | OBJECT_OBJFLAG_HITDETECT_DISABLED);
}

void WispBaddie_release(void) {
}

void WispBaddie_initialise(void) {
}

/* Baddie-family animation data shared with the sequence-driver TUs. */
typedef struct WispBaddieFamilyTable
{
    u8* tbl0;
    u8* tbl4;
    u8* tbl8;
    u8* tblC;
    u8* tbl10;
    u8* tbl14;
    u8* tbl18;
    u8* tbl1c;
    u8* tbl20;
    u8* tbl24;
} WispBaddieFamilyTable;

u8 lbl_8031DD30[288] = {
    0,   0,   0,  0,   0,   0,   0,  0,   0,   0,   0,  0,  0,  0,   0,  0, 0, 0,  0,   0,   0,  0, 0, 0, 0, 0,  0,
    0,   0,   0,  0,   0,   0,   0,  0,   0,   0,   0,  0,  0,  0,   0,  0, 0, 0,  0,   0,   0,  0, 0, 0, 0, 0,  0,
    0,   0,   0,  0,   0,   0,   0,  0,   0,   0,   0,  0,  0,  0,   0,  0, 0, 0,  0,   0,   0,  0, 0, 0, 0, 0,  0,
    0,   0,   0,  0,   0,   0,   0,  0,   0,   0,   0,  0,  0,  0,   0,  0, 0, 0,  0,   0,   0,  0, 0, 0, 0, 0,  0,
    0,   0,   0,  0,   0,   0,   0,  0,   0,   0,   0,  0,  0,  0,   0,  0, 0, 0,  0,   0,   0,  0, 0, 0, 0, 0,  0,
    0,   0,   0,  0,   0,   0,   0,  0,   0,   0,   0,  0,  0,  0,   0,  0, 0, 61, 163, 215, 10, 0, 0, 0, 0, 61, 163,
    215, 10,  61, 204, 204, 205, 61, 204, 204, 205, 0,  0,  0,  0,   0,  0, 0, 0,  0,   0,   0,  0, 0, 0, 0, 0,  61,
    163, 215, 10, 61,  163, 215, 10, 61,  35,  215, 10, 61, 35, 215, 10, 0, 0, 0,  0,   0,   0,  0, 0, 0, 0, 0,  0,
    0,   0,   0,  0,   0,   0,   0,  0,   0,   0,   0,  0,  0,  0,   0,  0, 0, 0,  0,   0,   0,  0, 0, 0, 0, 0,  0,
    0,   0,   0,  0,   0,   0,   0,  0,   0,   0,   0,  0,  0,  0,   0,  0, 0, 0,  0,   0,   0,  0, 0, 0, 0, 0,  0,
    0,   0,   0,  0,   0,   0,   0,  0,   0,   0,   0,  0,  0,  0,   0,  0, 0, 0};
u8 lbl_8031DE50[48] = {60, 35, 215, 10, 0, 0, 0, 0, 0,  0, 0, 0, 60, 35, 215, 10, 0, 0, 0, 0, 11, 0, 0, 0,
                       60, 35, 215, 10, 0, 0, 0, 0, 15, 0, 0, 0, 60, 35, 215, 10, 0, 0, 0, 0, 12, 0, 0, 0};
u8 lbl_8031DE80[324] = {
    0,  0,  0,  0,  0, 0, 0,  0,  13, 0,  0, 0, 64, 64,  0,  0,  0, 0, 0,  0,  0,  0,  0, 0, 64, 64,  0,  0,  0, 0,
    0,  0,  0,  0,  0, 0, 64, 64, 0,  0,  0, 0, 0,  0,   0,  0,  0, 0, 64, 64, 0,  0,  0, 0, 0,  0,   0,  0,  0, 0,
    64, 64, 0,  0,  0, 0, 0,  0,  0,  0,  0, 0, 64, 64,  0,  0,  0, 0, 0,  0,  0,  0,  0, 0, 64, 64,  0,  0,  0, 0,
    0,  0,  0,  0,  0, 0, 64, 64, 0,  0,  0, 0, 0,  0,   0,  0,  0, 0, 64, 64, 0,  0,  0, 0, 0,  0,   3,  0,  0, 0,
    64, 64, 0,  0,  0, 0, 0,  0,  3,  0,  0, 0, 64, 64,  0,  0,  0, 0, 0,  0,  6,  0,  0, 0, 64, 64,  0,  0,  0, 0,
    0,  0,  4,  0,  0, 0, 64, 64, 0,  0,  0, 0, 0,  0,   5,  0,  0, 0, 64, 64, 0,  0,  0, 0, 0,  0,   22, 25, 0, 0,
    64, 64, 0,  0,  0, 0, 0,  0,  6,  25, 0, 0, 63, 192, 0,  0,  0, 0, 0,  0,  24, 25, 0, 0, 63, 192, 0,  0,  0, 0,
    0,  0,  45, 25, 0, 0, 64, 64, 0,  0,  0, 0, 0,  0,   27, 26, 0, 0, 64, 64, 0,  0,  0, 0, 0,  0,   3,  25, 0, 0,
    64, 64, 0,  0,  0, 0, 0,  0,  7,  25, 0, 0, 64, 160, 0,  0,  0, 0, 0,  0,  26, 25, 0, 0, 64, 64,  0,  0,  0, 0,
    0,  0,  8,  25, 0, 0, 64, 0,  0,  0,  0, 0, 0,  0,   23, 25, 0, 0, 64, 64, 0,  0,  0, 0, 0,  0,   3,  25, 0, 0,
    64, 0,  0,  0,  0, 0, 0,  1,  11, 0,  0, 0, 64, 128, 0,  0,  0, 0, 0,  0,  30, 25, 0, 0};
u8 lbl_8031DFC4[300] = {
    63, 128, 0,  0, 0, 0, 0,  11,  64, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  60, 0, 0, 0, 63, 128, 0,  0, 0, 0,
    0,  11,  61, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  61, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  62, 0, 0, 0,
    63, 128, 0,  0, 0, 0, 0,  11,  62, 0, 0, 0, 0,  0,   0,  0, 0, 0, 0,  0,   0,  0, 0, 0, 0,  0,   0,  0, 0, 0,
    0,  0,   0,  0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  64, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  63, 0, 0, 0,
    63, 128, 0,  0, 0, 0, 0,  11,  61, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  61, 0, 0, 0, 63, 128, 0,  0, 0, 0,
    0,  11,  62, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  62, 0, 0, 0, 0,  0,   0,  0, 0, 0, 0,  0,   0,  0, 0, 0,
    0,  0,   0,  0, 0, 0, 0,  0,   0,  0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  64, 0, 0, 0, 63, 128, 0,  0, 0, 0,
    0,  11,  60, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  61, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  61, 0, 0, 0,
    63, 128, 0,  0, 0, 0, 0,  11,  62, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  62, 0, 0, 0, 0,  0,   0,  0, 0, 0,
    0,  11,  0,  0, 0, 0, 0,  0,   0,  0, 0, 0, 0,  0,   0,  0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  64, 0, 0, 0};
u8 lbl_8031E0F0[300] = {62, 148, 122, 225, 0, 0, 0, 11, 69, 2, 2, 0, 62, 148, 122, 225, 0, 0, 0, 11, 65, 2, 2, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 66, 2, 2, 0, 62, 148, 122, 225, 0, 0, 0, 11, 66, 2, 2, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 67, 2, 2, 0, 62, 148, 122, 225, 0, 0, 0, 11, 67, 2, 2, 0,
                        0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0, 0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 69, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 68, 2, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 66, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 66, 2, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 67, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 67, 2, 0, 0,
                        0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0, 0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 69, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 65, 2, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 66, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 66, 2, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 67, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 67, 2, 0, 0,
                        0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0, 0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 69, 2, 0, 0};
u8 lbl_8031E21C[36] = {0, 0,  0,  0, 0, 0, 0, 11, 24, 1, 0, 0, 0, 0,  0,  0, 0, 0,
                       0, 12, 25, 1, 0, 0, 0, 0,  0,  0, 0, 0, 0, 10, 16, 1, 0, 0};
u8 lbl_8031E240[96] = {63, 128, 0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 63, 128, 0, 0, 0, 0, 0, 0, 20, 0, 0, 0,
                       63, 0,   0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 0,  0,   0, 0, 0, 0, 0, 0, 0,  0, 0, 0,
                       0,  0,   0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 63, 0,   0, 0, 0, 0, 0, 0, 21, 0, 0, 0,
                       63, 128, 0, 0, 0, 0, 0, 0, 21, 0, 0, 0, 63, 128, 0, 0, 0, 0, 0, 0, 21, 0, 0, 0};
u8 lbl_8031E2A0[300] = {63, 0,   0,   0,   0, 0, 0, 0,  40, 0, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 38, 0, 0, 0,
                        63, 76,  204, 205, 0, 0, 0, 1,  53, 0, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 47, 0, 0, 0,
                        63, 76,  204, 205, 0, 0, 0, 1,  54, 0, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 48, 0, 0, 0,
                        0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0, 0,  0,   0,   0,   0, 0, 0, 0, 0,  0, 0, 0,
                        63, 192, 0,   0,   0, 0, 0, 0,  57, 7, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 38, 9, 0, 0,
                        64, 0,   0,   0,   0, 0, 0, 1,  32, 0, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 49, 9, 0, 0,
                        63, 0,   0,   0,   0, 0, 0, 0,  57, 7, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 50, 9, 0, 0,
                        0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0, 0,  0,   0,   0,   0, 0, 0, 0, 0,  0, 0, 0,
                        63, 76,  204, 205, 0, 0, 0, 0,  39, 3, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 57, 7, 0, 0,
                        63, 153, 153, 154, 0, 0, 0, 0,  42, 1, 0, 0, 63, 153, 153, 154, 0, 0, 0, 0, 42, 1, 0, 0,
                        63, 153, 153, 154, 0, 0, 0, 0,  41, 2, 0, 0, 63, 153, 153, 154, 0, 0, 0, 0, 41, 2, 0, 0,
                        0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0, 0,  0,   0,   0,   0, 0, 0, 0, 0,  0, 0, 0,
                        63, 192, 0,   0,   0, 0, 0, 11, 28, 3, 0, 0};
u8 lbl_8031E3CC[208] = {
    0,  0,   0,  0,  0, 0, 0, 0,  0,  0, 0, 0,  0, 0, 0, 0,  63, 192, 0,  0,  0, 0, 0, 11, 56, 1, 5, 10, 0, 0, 0, 64,
    63, 192, 0,  0,  0, 0, 0, 11, 55, 2, 6, 11, 0, 0, 0, 64, 63, 192, 0,  0,  0, 0, 0, 11, 29, 0, 0, 0,  0, 0, 0, 0,
    63, 192, 0,  0,  0, 0, 0, 3,  46, 0, 0, 0,  0, 0, 0, 0,  63, 192, 0,  0,  0, 0, 0, 11, 51, 0, 0, 0,  0, 0, 0, 0,
    63, 192, 0,  0,  0, 0, 0, 11, 52, 0, 0, 0,  0, 0, 0, 0,  63, 192, 0,  0,  0, 0, 0, 11, 59, 7, 8, 12, 0, 0, 0, 64,
    63, 64,  0,  0,  0, 0, 0, 11, 58, 0, 0, 0,  0, 0, 0, 0,  63, 128, 0,  0,  0, 0, 0, 11, 36, 0, 0, 0,  0, 0, 0, 0,
    63, 51,  51, 51, 0, 0, 0, 11, 70, 0, 0, 0,  0, 0, 0, 0,  63, 51,  51, 51, 0, 0, 0, 11, 70, 0, 0, 0,  0, 0, 0, 0,
    63, 51,  51, 51, 0, 0, 0, 11, 71, 0, 0, 0,  0, 0, 0, 0};
u8 lbl_8031E49C[432] = {
    0,  0,   0,   0,   0, 0, 0, 0, 21, 0, 0, 0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0, 36, 0, 0,  0,   0, 0, 0, 0,
    63, 128, 0,   0,   0, 0, 0, 0, 24, 0, 5, 230, 1, 0, 0, 0, 63, 166, 102, 102, 0, 0, 0, 0, 25, 0, 9,  230, 1, 0, 0, 0,
    63, 128, 0,   0,   0, 0, 0, 0, 36, 0, 0, 0,   0, 0, 0, 0, 63, 166, 102, 102, 0, 0, 0, 0, 25, 0, 9,  230, 1, 0, 0, 0,
    64, 0,   0,   0,   0, 0, 0, 0, 7,  0, 0, 0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0, 36, 0, 0,  0,   0, 0, 0, 0,
    63, 128, 0,   0,   0, 0, 0, 0, 24, 0, 5, 230, 1, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0, 24, 0, 5,  230, 1, 0, 0, 0,
    63, 38,  102, 102, 0, 4, 0, 0, 17, 0, 0, 0,   0, 0, 0, 0, 63, 38,  102, 102, 0, 2, 0, 0, 18, 0, 0,  0,   0, 0, 0, 0,
    63, 38,  102, 102, 0, 2, 0, 0, 18, 0, 0, 0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0, 2,  0, 0,  0,   0, 0, 0, 0,
    63, 166, 102, 102, 0, 0, 0, 0, 25, 0, 9, 230, 1, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0, 24, 0, 5,  230, 1, 0, 0, 0,
    63, 38,  102, 102, 0, 8, 0, 0, 19, 0, 0, 0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 1, 0, 0, 16, 0, 33, 230, 2, 0, 0, 0,
    63, 38,  102, 102, 0, 8, 0, 0, 19, 0, 0, 0,   0, 0, 0, 0, 63, 166, 102, 102, 0, 0, 0, 0, 25, 0, 9,  230, 1, 0, 0, 0,
    63, 128, 0,   0,   0, 0, 0, 0, 24, 0, 5, 230, 1, 0, 0, 0, 63, 38,  102, 102, 0, 2, 0, 0, 18, 0, 0,  0,   0, 0, 0, 0,
    63, 38,  102, 102, 0, 4, 0, 0, 17, 0, 0, 0,   0, 0, 0, 0, 63, 140, 204, 205, 0, 1, 0, 0, 16, 0, 33, 230, 2, 0, 0, 0,
    63, 12,  204, 205, 0, 1, 0, 0, 12, 0, 0, 0,   0, 0, 0, 0, 63, 38,  102, 102, 0, 8, 0, 0, 19, 0, 0,  0,   0, 0, 0, 0,
    63, 38,  102, 102, 0, 2, 0, 0, 18, 0, 0, 0,   0, 0, 0, 0};
u8 lbl_8031E64C[24] = {0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0};
u8 lbl_8031E664[432] = {0,  0,   0,   0,   0, 0, 0, 0, 21, 0,   0,   0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0,
                        36, 0,   0,   0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0, 24, 0,   5,   230, 1, 0, 0, 0,
                        63, 166, 102, 102, 0, 0, 0, 0, 25, 0,   9,   230, 1, 0, 0, 0, 63, 128, 0,   0,   0, 1, 0, 0,
                        16, 0,   33,  230, 2, 0, 0, 0, 63, 166, 102, 102, 0, 0, 0, 0, 25, 0,   9,   230, 1, 0, 0, 0,
                        63, 128, 0,   0,   0, 1, 0, 0, 16, 0,   33,  230, 2, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0,
                        36, 0,   0,   0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0, 24, 0,   5,   230, 1, 0, 0, 0,
                        63, 128, 0,   0,   0, 0, 0, 0, 24, 0,   5,   230, 1, 0, 0, 0, 63, 38,  102, 102, 0, 4, 0, 0,
                        17, 0,   0,   0,   0, 0, 0, 0, 63, 166, 102, 102, 0, 0, 0, 0, 25, 0,   9,   230, 1, 0, 0, 0,
                        63, 38,  102, 102, 0, 2, 0, 0, 18, 0,   0,   0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0,
                        2,  0,   0,   0,   0, 0, 0, 0, 63, 166, 102, 102, 0, 0, 0, 0, 25, 0,   9,   230, 1, 0, 0, 0,
                        63, 128, 0,   0,   0, 0, 0, 0, 24, 0,   5,   230, 1, 0, 0, 0, 63, 38,  102, 102, 0, 8, 0, 0,
                        19, 0,   0,   0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 1, 0, 0, 16, 0,   33,  230, 2, 0, 0, 0,
                        63, 38,  102, 102, 0, 8, 0, 0, 19, 0,   0,   0,   0, 0, 0, 0, 63, 166, 102, 102, 0, 0, 0, 0,
                        25, 0,   9,   230, 1, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0, 24, 0,   5,   230, 1, 0, 0, 0,
                        63, 38,  102, 102, 0, 2, 0, 0, 18, 0,   0,   0,   0, 0, 0, 0, 63, 38,  102, 102, 0, 4, 0, 0,
                        17, 0,   0,   0,   0, 0, 0, 0, 63, 140, 204, 205, 0, 1, 0, 0, 16, 0,   33,  230, 2, 0, 0, 0,
                        63, 12,  204, 205, 0, 1, 0, 0, 12, 0,   0,   0,   0, 0, 0, 0, 63, 38,  102, 102, 0, 8, 0, 0,
                        19, 0,   0,   0,   0, 0, 0, 0, 63, 38,  102, 102, 0, 2, 0, 0, 18, 0,   0,   0,   0, 0, 0, 0};
u8 lbl_8031E814[24] = {0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0};
u8 lbl_8031E82C[36] = {0, 0,  0,  0, 0, 0, 0, 11, 24, 2, 0, 0, 0, 0,  0,  0, 0, 0,
                       0, 10, 25, 2, 0, 0, 0, 0,  0,  0, 0, 0, 0, 24, 16, 4, 0, 0};
u8 lbl_8031E850[432] = {0,  0,   0,   0,   0, 0, 0, 0, 21, 0,   0,   0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0,
                        36, 0,   0,   0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0, 24, 0,   5,   230, 1, 0, 0, 0,
                        63, 102, 102, 102, 0, 0, 0, 0, 25, 0,   9,   230, 1, 0, 0, 0, 63, 128, 0,   0,   0, 1, 0, 0,
                        16, 0,   33,  230, 2, 0, 0, 0, 63, 102, 102, 102, 0, 0, 0, 0, 25, 0,   9,   230, 1, 0, 0, 0,
                        63, 128, 0,   0,   0, 1, 0, 0, 16, 0,   33,  230, 2, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0,
                        36, 0,   0,   0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0, 24, 0,   5,   230, 1, 0, 0, 0,
                        63, 128, 0,   0,   0, 0, 0, 0, 24, 0,   5,   230, 1, 0, 0, 0, 63, 38,  102, 102, 0, 4, 0, 0,
                        17, 0,   0,   0,   0, 0, 0, 0, 63, 102, 102, 102, 0, 0, 0, 0, 25, 0,   9,   230, 1, 0, 0, 0,
                        63, 38,  102, 102, 0, 2, 0, 0, 18, 0,   0,   0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0,
                        2,  0,   0,   0,   0, 0, 0, 0, 63, 102, 102, 102, 0, 0, 0, 0, 25, 0,   9,   230, 1, 0, 0, 0,
                        63, 128, 0,   0,   0, 0, 0, 0, 24, 0,   5,   230, 1, 0, 0, 0, 63, 38,  102, 102, 0, 8, 0, 0,
                        19, 0,   0,   0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 1, 0, 0, 16, 0,   33,  230, 2, 0, 0, 0,
                        63, 38,  102, 102, 0, 8, 0, 0, 19, 0,   0,   0,   0, 0, 0, 0, 63, 102, 102, 102, 0, 0, 0, 0,
                        25, 0,   9,   230, 1, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0, 24, 0,   5,   230, 1, 0, 0, 0,
                        63, 38,  102, 102, 0, 2, 0, 0, 18, 0,   0,   0,   0, 0, 0, 0, 63, 38,  102, 102, 0, 4, 0, 0,
                        17, 0,   0,   0,   0, 0, 0, 0, 63, 140, 204, 205, 0, 1, 0, 0, 16, 0,   33,  230, 2, 0, 0, 0,
                        63, 12,  204, 205, 0, 1, 0, 0, 12, 0,   0,   0,   0, 0, 0, 0, 63, 38,  102, 102, 0, 8, 0, 0,
                        19, 0,   0,   0,   0, 0, 0, 0, 63, 38,  102, 102, 0, 2, 0, 0, 18, 0,   0,   0,   0, 0, 0, 0};
u8 lbl_8031EA00[24] = {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0};
u8 lbl_8031EA18[468] = {0,  0,   0,   0,   0, 0, 0, 0,  21, 0,   0,   0,   0, 0, 0, 0, 63, 128, 0,   0,   0,  0, 0, 0,
                        36, 0,   0,   0,   0, 0, 0, 0,  63, 128, 0,   0,   0, 0, 0, 0, 24, 0,   5,   230, 1,  0, 0, 0,
                        63, 166, 102, 102, 0, 0, 0, 0,  25, 0,   9,   230, 1, 0, 0, 0, 63, 128, 0,   0,   0,  1, 0, 0,
                        16, 0,   33,  230, 2, 0, 0, 0,  63, 166, 102, 102, 0, 0, 0, 0, 25, 0,   9,   230, 1,  0, 0, 0,
                        63, 128, 0,   0,   0, 1, 0, 0,  16, 0,   33,  230, 2, 0, 0, 0, 63, 38,  102, 102, 0,  8, 0, 0,
                        19, 0,   0,   0,   0, 0, 0, 0,  63, 128, 0,   0,   0, 0, 0, 0, 24, 0,   5,   230, 1,  0, 0, 0,
                        63, 128, 0,   0,   0, 0, 0, 0,  24, 0,   5,   230, 1, 0, 0, 0, 63, 38,  102, 102, 0,  4, 0, 0,
                        17, 0,   0,   0,   0, 0, 0, 0,  63, 166, 102, 102, 0, 0, 0, 0, 25, 0,   9,   230, 1,  0, 0, 0,
                        63, 38,  102, 102, 0, 2, 0, 0,  18, 0,   0,   0,   0, 0, 0, 0, 63, 128, 0,   0,   0,  0, 0, 0,
                        2,  0,   0,   0,   0, 0, 0, 0,  63, 166, 102, 102, 0, 0, 0, 0, 25, 0,   9,   230, 1,  0, 0, 0,
                        63, 128, 0,   0,   0, 1, 0, 0,  16, 0,   33,  230, 2, 0, 0, 0, 63, 128, 0,   0,   0,  0, 0, 0,
                        24, 0,   5,   230, 1, 0, 0, 0,  63, 128, 0,   0,   0, 1, 0, 0, 16, 0,   33,  230, 2,  0, 0, 0,
                        63, 38,  102, 102, 0, 8, 0, 0,  19, 0,   0,   0,   0, 0, 0, 0, 63, 166, 102, 102, 0,  0, 0, 0,
                        25, 0,   9,   230, 1, 0, 0, 0,  63, 128, 0,   0,   0, 0, 0, 0, 24, 0,   5,   230, 1,  0, 0, 0,
                        63, 38,  102, 102, 0, 2, 0, 0,  18, 0,   0,   0,   0, 0, 0, 0, 63, 38,  102, 102, 0,  4, 0, 0,
                        17, 0,   0,   0,   0, 0, 0, 0,  63, 140, 204, 205, 0, 1, 0, 0, 16, 0,   33,  230, 2,  0, 0, 0,
                        63, 12,  204, 205, 0, 1, 0, 0,  12, 0,   0,   0,   0, 0, 0, 0, 63, 38,  102, 102, 0,  8, 0, 0,
                        19, 0,   0,   0,   0, 0, 0, 0,  63, 38,  102, 102, 0, 2, 0, 0, 18, 0,   0,   0,   0,  0, 0, 0,
                        0,  0,   0,   0,   0, 0, 0, 11, 24, 1,   0,   0,   0, 0, 0, 0, 0,  0,   0,   12,  25, 1, 0, 0,
                        0,  0,   0,   0,   0, 0, 0, 10, 16, 2,   0,   0};
u8 lbl_8031EBEC[24] = {0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0};
u8 lbl_8031EC04[48] = {60, 35, 215, 10, 0, 0, 0, 0, 0, 0, 0, 0, 60, 35, 215, 10, 0, 0, 0, 0, 1, 0, 0, 0,
                       60, 35, 215, 10, 0, 0, 0, 0, 2, 0, 0, 0, 60, 35, 215, 10, 0, 0, 0, 0, 1, 0, 0, 0};
u8 lbl_8031EC34[24] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
u8 lbl_8031EC4C[300] = {
    63, 128, 0,  0, 0, 0, 0,  11,  20, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  16, 0, 0, 0, 63, 128, 0,  0, 0, 0,
    0,  11,  18, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  18, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  19, 0, 0, 0,
    63, 128, 0,  0, 0, 0, 0,  11,  19, 0, 0, 0, 0,  0,   0,  0, 0, 0, 0,  0,   0,  0, 0, 0, 0,  0,   0,  0, 0, 0,
    0,  0,   0,  0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  20, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  17, 0, 0, 0,
    63, 128, 0,  0, 0, 0, 0,  11,  18, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  18, 0, 0, 0, 63, 128, 0,  0, 0, 0,
    0,  11,  19, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  19, 0, 0, 0, 0,  0,   0,  0, 0, 0, 0,  0,   0,  0, 0, 0,
    0,  0,   0,  0, 0, 0, 0,  0,   0,  0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  20, 0, 0, 0, 63, 128, 0,  0, 0, 0,
    0,  11,  16, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  18, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  18, 0, 0, 0,
    63, 128, 0,  0, 0, 0, 0,  11,  19, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  19, 0, 0, 0, 0,  0,   0,  0, 0, 0,
    0,  11,  0,  0, 0, 0, 0,  0,   0,  0, 0, 0, 0,  0,   0,  0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  20, 0, 0, 0};
u8 lbl_8031ED78[300] = {62, 148, 122, 225, 0, 0, 0, 11, 20, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 16, 2, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 18, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 18, 2, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 19, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 19, 2, 0, 0,
                        0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0, 0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 20, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 17, 2, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 18, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 18, 2, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 19, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 19, 2, 0, 0,
                        0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0, 0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 20, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 16, 2, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 18, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 18, 2, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 19, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 19, 2, 0, 0,
                        0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0, 0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 20, 2, 0, 0};
u8 lbl_8031EEA4[36] = {0, 0,  0, 0, 0, 0, 0, 11, 0, 1, 0, 0, 0, 0,  0, 0, 0, 0,
                       0, 12, 0, 1, 0, 0, 0, 0,  0, 0, 0, 0, 0, 10, 0, 1, 0, 0};
u8 lbl_8031EEC8[96] = {63, 128, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 63, 128, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0,
                       63, 0,   0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0,  0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                       0,  0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 63, 0,   0, 0, 0, 0, 0, 0, 7, 0, 0, 0,
                       63, 128, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 63, 128, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0};
u8 lbl_8031EF28[300] = {63, 0,   0,   0,   0, 0, 0, 0, 15, 0, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 12, 0, 0, 0,
                        63, 76,  204, 205, 0, 0, 0, 0, 14, 0, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 14, 0, 0, 0,
                        63, 76,  204, 205, 0, 0, 0, 0, 13, 0, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 13, 0, 0, 0,
                        0,  0,   0,   0,   0, 0, 0, 0, 0,  0, 0, 0, 0,  0,   0,   0,   0, 0, 0, 0, 0,  0, 0, 0,
                        63, 192, 0,   0,   0, 0, 0, 0, 15, 0, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 12, 0, 0, 0,
                        64, 0,   0,   0,   0, 0, 0, 0, 14, 0, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 14, 0, 0, 0,
                        63, 0,   0,   0,   0, 0, 0, 0, 13, 0, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 13, 0, 0, 0,
                        0,  0,   0,   0,   0, 0, 0, 0, 0,  0, 0, 0, 0,  0,   0,   0,   0, 0, 0, 0, 0,  0, 0, 0,
                        63, 76,  204, 205, 0, 0, 0, 0, 15, 0, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 12, 0, 0, 0,
                        63, 153, 153, 154, 0, 0, 0, 0, 14, 0, 0, 0, 63, 153, 153, 154, 0, 0, 0, 0, 14, 0, 0, 0,
                        63, 153, 153, 154, 0, 0, 0, 0, 13, 0, 0, 0, 63, 153, 153, 154, 0, 0, 0, 0, 13, 0, 0, 0,
                        0,  0,   0,   0,   0, 0, 0, 0, 0,  0, 0, 0, 0,  0,   0,   0,   0, 0, 0, 0, 0,  0, 0, 0,
                        63, 192, 0,   0,   0, 0, 0, 0, 15, 0, 0, 0};
u8 lbl_8031F054[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
u8 lbl_8031F064[240] = {
    0,  0,   0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0, 64, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0,  0,   0, 0, 0, 0,
    63, 128, 0, 0, 0, 8, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 63, 128, 0, 0, 0, 1, 0, 0, 1, 0, 0,  0,   0, 0, 0, 0,
    64, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 63, 128, 0, 0, 0, 4, 0, 0, 3, 0, 0,  0,   0, 0, 0, 0,
    63, 128, 0, 0, 0, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 63, 128, 0, 0, 0, 8, 0, 0, 5, 0, 0,  0,   0, 0, 0, 0,
    64, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 63, 128, 0, 0, 0, 2, 0, 0, 6, 0, 0,  0,   0, 0, 0, 0,
    63, 128, 0, 0, 0, 4, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 63, 128, 0, 0, 0, 1, 0, 0, 1, 0, 33, 230, 2, 0, 0, 0,
    63, 128, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 63, 128, 0, 0, 0, 8, 0, 0, 5, 0, 0,  0,   0, 0, 0, 0,
    63, 128, 0, 0, 0, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0};
u8 lbl_8031F154[24] = {0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0};
WispBaddieFamilyTable lbl_8031F16C[6] = {
    {lbl_8031DE50, lbl_8031DE80, lbl_8031DFC4, lbl_8031E49C, lbl_8031E2A0, lbl_8031E21C, lbl_8031E240, lbl_8031E3CC,
     lbl_8031E64C, lbl_8031E0F0},
    {lbl_8031DE50, lbl_8031DE80, lbl_8031DFC4, lbl_8031E49C, lbl_8031E2A0, lbl_8031E21C, lbl_8031E240, lbl_8031E3CC,
     lbl_8031E64C, lbl_8031E0F0},
    {lbl_8031DE50, lbl_8031DE80, lbl_8031DFC4, lbl_8031E664, lbl_8031E2A0, lbl_8031E21C, lbl_8031E240, lbl_8031E3CC,
     lbl_8031E814, lbl_8031E0F0},
    {lbl_8031DE50, lbl_8031DE80, lbl_8031DFC4, lbl_8031E850, lbl_8031E2A0, lbl_8031E82C, lbl_8031E240, lbl_8031E3CC,
     lbl_8031EA00, lbl_8031E0F0},
    {lbl_8031DE50, lbl_8031DE80, lbl_8031DFC4, lbl_8031EA18, lbl_8031E2A0, lbl_8031E21C, lbl_8031E240, lbl_8031E3CC,
     lbl_8031EBEC, lbl_8031E0F0},
    {lbl_8031EC04, lbl_8031EC34, lbl_8031EC4C, lbl_8031F064, lbl_8031EF28, lbl_8031EEA4, lbl_8031EEC8, lbl_8031F054,
     lbl_8031F154, lbl_8031ED78},
};
u8 lbl_8031F25C[12] = {15, 60, 10, 50, 7, 20, 5, 20, 3, 15, 3, 15};
f32 lbl_8031F268[6] = {0.5f, 0.5f, 0.7f, 0.6f, 1.5f, 1.5f};
u32 gGroundBaddieModelChainIds[4] = {6, 7, 8, 9};
