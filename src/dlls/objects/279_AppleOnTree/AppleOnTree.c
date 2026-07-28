#include "dlls/objects/279_AppleOnTree.h"

#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/dll/partfx_interface.h"
#include "main/dll/waterfx_interface.h"
#include "main/dll_000A_expgfx.h"
#include "main/frame_timing.h"
#include "main/gamebits.h"
#include "main/object_render.h"
#include "main/objfx.h"
#include "main/objhits.h"
#include "main/objseq.h"
#include "main/objtexture.h"
#include "main/sky_interface.h"
#include "main/track_dolphin_api.h"
#include "main/vecmath.h"
#include "main/vecmath_distance_api.h"
#include "sys/objects.h"

#define APPLE_ON_TREE_FLAG_INACTIVE           0x02
#define APPLE_ON_TREE_FLAG_PICKUP_PENDING     0x04
#define APPLE_ON_TREE_FLAG_LANDING_SFX_PLAYED 0x08

#define APPLE_ON_TREE_PARTICLE_BURST       0x55A
#define APPLE_ON_TREE_PARTICLE_BURST_COUNT 8

#define APPLE_ON_TREE_MESSAGE_IN_RANGE 0x7000A
#define APPLE_ON_TREE_MESSAGE_PICKUP   0x7000B

#define APPLE_ON_TREE_GAME_BIT_NONE       -1
#define APPLE_ON_TREE_HANGING_MODEL_INDEX 0
#define APPLE_ON_TREE_FALLEN_MODEL_INDEX  1
#define APPLE_ON_TREE_HEALTH_RESTORE      2
#define APPLE_ON_TREE_PICKUP_TRIGGER_ID   0x444
#define APPLE_ON_TREE_MESSAGE_QUEUE_SIZE  2

typedef enum AppleOnTreeAnimState {
    APPLE_ON_TREE_STATE_GROWING,
    APPLE_ON_TREE_STATE_RIPE,
    APPLE_ON_TREE_STATE_FALLING,
    APPLE_ON_TREE_STATE_LANDED,
    APPLE_ON_TREE_STATE_KNOCKED,
    APPLE_ON_TREE_STATE_BURST,
    APPLE_ON_TREE_STATE_FADEOUT,
} AppleOnTreeAnimState;

typedef enum AppleOnTreeKnockReason {
    APPLE_ON_TREE_KNOCK_NATURALLY,
    APPLE_ON_TREE_KNOCK_WHILE_FALLING,
    APPLE_ON_TREE_KNOCK_WHILE_LANDED,
} AppleOnTreeKnockReason;

extern f32 lbl_803E37C8;
extern f32 lbl_803E37CC;
extern f32 lbl_803E37D0;
extern f32 lbl_803E37D4;
extern f32 lbl_803E37D8;
extern f32 lbl_803E37DC;
extern f32 lbl_803E37E0;
extern f32 lbl_803E37E4;
extern f32 lbl_803E37E8;
extern f32 gAppleOnTreePickupXZRange;
extern f32 gAppleOnTreePickupRange;
extern f32 lbl_803E37F4;
extern f32 lbl_803E37F8;
extern f32 lbl_803E37FC;
extern f32 lbl_803E3800;
extern f32 lbl_803E3804;
extern f32 lbl_803E3808;
extern f32 lbl_803E380C;
extern f32 lbl_803E3810;
extern f32 lbl_803E3814;
extern f32 lbl_803E3818;
extern const f32 lbl_803E3828;
extern f32 lbl_803E382C;
extern f32 lbl_803E3830;
extern f32 lbl_803E3834;
extern f32 lbl_803E3838;

static inline void appleontree_markFallen(GameObject* obj) {
    AppleOnTreeState* state = obj->extra;

    if ((obj->anim.flags & OBJANIM_FLAG_OWNS_PLACEMENT_DATA) != 0) {
        Obj_FreeObject(obj);
    } else {
        if (obj->anim.hitReactState != NULL) {
            ObjHits_DisableObject(obj);
        }
        state->flags = (u8)(state->flags | APPLE_ON_TREE_FLAG_INACTIVE);
    }
}

ObjectDescriptor13 gAppleOnTreeObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_13_SLOTS,
    0,
    0,
    0,
    (ObjectDescriptorCallback)AppleOnTree_init,
    (ObjectDescriptorCallback)AppleOnTree_update,
    0,
    (ObjectDescriptorCallback)AppleOnTree_render,
    (ObjectDescriptorCallback)AppleOnTree_free,
    0,
    AppleOnTree_getExtraSize,
    (ObjectDescriptorCallback)AppleOnTree_func0A,
    (ObjectDescriptorCallback)AppleOnTree_setPosition,
    (ObjectDescriptorCallback)AppleOnTree_modelMtxFn,
};

void AppleOnTree_setPosition(GameObject* obj, f32* position) {
    AppleOnTreeState* state = obj->extra;

    if (state->animState == APPLE_ON_TREE_STATE_KNOCKED) {
        return;
    }
    if (state->animState == APPLE_ON_TREE_STATE_BURST) {
        return;
    }
    if (state->animState == APPLE_ON_TREE_STATE_FADEOUT) {
        return;
    }
    obj->anim.localPosX = position[0];
    obj->anim.localPosY = position[1];
    obj->anim.localPosZ = position[2];
}

void appleontree_knockLoose(GameObject* obj, int message) {
    AppleOnTreeState* state = obj->extra;
    int healthRestore;

    switch (message) {
    case APPLE_ON_TREE_KNOCK_NATURALLY:
        healthRestore = APPLE_ON_TREE_HEALTH_RESTORE;
        break;
    case APPLE_ON_TREE_KNOCK_WHILE_FALLING:
        healthRestore = APPLE_ON_TREE_HEALTH_RESTORE;
        break;
    case APPLE_ON_TREE_KNOCK_WHILE_LANDED:
        healthRestore = APPLE_ON_TREE_HEALTH_RESTORE;
        break;
    default:
        healthRestore = 0;
        break;
    }
    state->healthRestore = healthRestore;
    state->animState = APPLE_ON_TREE_STATE_KNOCKED;
    state->elapsedTime = timeDelta;
    state->flightTime = timeDelta;
    state->rotX = randomGetRange(-0x8000, 0x7fff);
    state->rotY = randomGetRange(-0x8000, 0x7fff);
    state->rotZ = 0x2000;

    if (trackGetHeightAboveGround(obj, obj->anim.localPosX, obj->anim.localPosY, obj->anim.localPosZ, &state->dropHeight, 0) == 0) {
        appleontree_markFallen(obj);
    } else {
        f32 m = state->gravity;
        f32 gravityTerm = lbl_803E37D8 * m;
        f32 q = sqrtf(-(gravityTerm * state->dropHeight - lbl_803E37D4));
        f32 r;

        gravityTerm = lbl_803E37DC * m;
        if (gravityTerm >= lbl_803E37D4) {
            r = gravityTerm;
        } else {
            r = -gravityTerm;
        }
        if (r <= lbl_803E37E0) {
            r = lbl_803E37C8;
        } else {
            f32 r2;
            r = (lbl_803E37E4 - q) / gravityTerm;
            r2 = (lbl_803E37E4 + q) / gravityTerm;
            r = (r > *(f32*)&lbl_803E37D4) ? r : r2;
        }
        state->totalFlightTime = r;

        if (state->waterAcceleration < lbl_803E37D4) {
            state->dropHeight = -(lbl_803E37D8 * state->fallScale - state->dropHeight);
        } else {
            state->dropHeight = lbl_803E37E8 * (lbl_803E37D8 * state->fallScale) + state->dropHeight;
        }

        if (state->dropHeight <= lbl_803E37D4) {
            state = obj->extra;
            if ((obj->anim.flags & OBJANIM_FLAG_OWNS_PLACEMENT_DATA) != 0) {
                Obj_FreeObject(obj);
            } else {
                if (obj->anim.hitReactState != NULL) {
                    ObjHits_DisableObject(obj);
                }
                state->flags = (u8)(state->flags | APPLE_ON_TREE_FLAG_INACTIVE);
            }
        } else {
            state->positionY = obj->anim.localPosY;
            state->splashPositionY = obj->anim.localPosY - state->dropHeight;
            if (obj->anim.hitReactState != NULL) {
                ObjHits_DisableObject(obj);
            }
            Sfx_PlayFromObject((int)obj, SFXTRIG_en_tranch_6);
        }
    }
}

void appleontree_handleCollectableHit(GameObject* obj) {
    AppleOnTreeState* state = obj->extra;
    GameObject* player = Obj_GetPlayerObject();

    if (!(Vec_xzDistance(&player->anim.worldPosX, &obj->anim.worldPosX) < gAppleOnTreePickupXZRange))
        return;
    if (!(Vec_distance(&player->anim.worldPosX, &obj->anim.worldPosX) < gAppleOnTreePickupRange))
        return;

    if (mainGetBit(GAMEBIT_SawApple) == 0) {
        (*gObjectTriggerInterface)->setObjects(APPLE_ON_TREE_PICKUP_TRIGGER_ID, 0, 0);
        state->triggerGameBit = APPLE_ON_TREE_GAME_BIT_NONE;
        state->pickupMessageValue = 0;
        state->pickupMessageArgument = lbl_803E37C8;
        ObjMsg_SendToObject(player, APPLE_ON_TREE_MESSAGE_IN_RANGE, obj, (int)&state->triggerGameBit);
        mainSetBits(GAMEBIT_SawApple, 1);
        state->flags = (u8)(state->flags | APPLE_ON_TREE_FLAG_PICKUP_PENDING);
    } else {
        playerAddHealth(player, state->healthRestore);
        itemPickupDoParticleFx(obj, lbl_803E37C8, 0xff, 0x28);
        Sfx_PlayFromObject((int)obj, SFXTRIG_cam90_c);
        appleontree_markFallen(obj);
    }
}

u8 AppleOnTree_modelMtxFn(GameObject* obj) {
    return ((AppleOnTreeState*)obj->extra)->animState;
}

void AppleOnTree_func0A(void) {
}

int AppleOnTree_getExtraSize(void) {
    return APPLE_ON_TREE_STATE_SIZE;
}

void AppleOnTree_free(GameObject* obj) {
    (*gExpgfxInterface)->freeSource((u32)obj);
}

void AppleOnTree_render(GameObject* obj, int arg1, int arg2, int arg3, int arg4, s8 renderState) {
    AppleOnTreeState* state = obj->extra;
    if ((state->flags & APPLE_ON_TREE_FLAG_INACTIVE) == 0) {
        objRenderModelAndHitVolumes(obj, arg1, arg2, arg3, arg4, lbl_803E37C8);
    }
}

int appleontree_bounceGroundStep(GameObject* obj, AppleOnTreeState* state, f32 positionY) {
    f32 zero = lbl_803E37D4;
    f32 m = state->gravity;

    if (zero != m) {
        if (state->dropHeight - (state->positionY - positionY) < zero) {
            f32 b = state->bounceVelocity;
            if (zero == b) {
                f32 g = lbl_803E37D8 * m;
                f32 q = sqrtf(b * b - g * state->dropHeight);
                f32 t = lbl_803E37DC * m;
                f32 r;

                if (t >= lbl_803E37D4) {
                    r = t;
                } else {
                    r = -t;
                }
                if (r <= lbl_803E37E0) {
                    r = lbl_803E37C8;
                } else {
                    f32 r2;
                    f32 nb;
                    nb = -b;
                    r = (nb - q) / t;
                    r2 = (nb + q) / t;
                    r = (r > *(f32*)&lbl_803E37D4) ? r : r2;
                }
                state->flightTime = state->flightTime - r;
                state->positionY = state->positionY - state->dropHeight;
                state->dropHeight = lbl_803E37D4;
                (obj)->anim.localPosY = state->positionY;
                (obj)->anim.rotX = state->rotX;
                (obj)->anim.rotY = state->rotY;
                (obj)->anim.rotZ = state->rotZ;
                state->bounceVelocity = -state->waterAcceleration;
                if ((state->flags & APPLE_ON_TREE_FLAG_LANDING_SFX_PLAYED) == 0) {
                    Sfx_PlayFromObject((int)obj, SFXTRIG_pk_fruit_lands);
                    state->flags = (u8)(state->flags | APPLE_ON_TREE_FLAG_LANDING_SFX_PLAYED);
                }
                return 1;
            } else if (b < lbl_803E37F4) {
                (obj)->anim.localPosY = state->positionY;
                state->gravity = zero;
                state->bounceVelocity = zero;
                return 1;
            } else {
                f32 g;
                f32 q;
                f32 t;
                f32 r;
                m = m + state->extraAcceleration;
                g = lbl_803E37D8 * m;
                q = sqrtf(b * b - g * state->dropHeight);
                t = lbl_803E37DC * m;

                if (t >= lbl_803E37D4) {
                    r = t;
                } else {
                    r = -t;
                }
                if (r <= lbl_803E37E0) {
                    r = lbl_803E37C8;
                } else {
                    f32 r2;
                    f32 nb;
                    nb = -b;
                    r = (nb - q) / t;
                    r2 = (nb + q) / t;
                    r = (r > *(f32*)&lbl_803E37D4) ? r : r2;
                }
                state->flightTime = state->flightTime - r;
                (obj)->anim.localPosY = state->positionY;
                state->bounceVelocity = state->bounceVelocity * lbl_803E37F8;
                return 0;
            }
        } else {
            (obj)->anim.localPosY = positionY;
            return 1;
        }
    }
    return 1;
}

int appleontree_bounceWaterStep(GameObject* obj, AppleOnTreeState* state, f32 positionY) {
    if (lbl_803E37D4 == state->extraAcceleration) {
        if (state->dropHeight - (state->positionY - positionY) <= lbl_803E37D4) {
            f32 b;
            f32 m = state->gravity;
            f32 g;
            f32 q;
            f32 t;
            f32 a;
            f32 r;
            f32 rad;
            b = state->bounceVelocity;
            g = lbl_803E37D8 * m;
            q = sqrtf(b * b - g * state->dropHeight);
            t = lbl_803E37DC * m;

            if (t >= lbl_803E37D4) {
                a = t;
            } else {
                a = -t;
            }
            if (a <= lbl_803E37E0) {
                r = lbl_803E37C8;
            } else {
                f32 r2;
                f32 nb;
                nb = -b;
                r = (nb - q) / t;
                r2 = (nb + q) / t;
                r = (r > *(f32*)&lbl_803E37D4) ? r : r2;
            }
            state->flightTime = state->flightTime - r;
            state->positionY = state->positionY - state->dropHeight;
            rad = lbl_803E37D4;
            state->dropHeight = rad;
            obj->anim.localPosY = state->positionY;
            obj->anim.rotX = state->rotX;
            obj->anim.rotY = state->rotY;
            obj->anim.rotZ = state->rotZ;
            {
                f32 g2 = lbl_803E37DC * state->gravity;
                state->bounceVelocity = g2 * r + state->bounceVelocity;
            }
            state->extraAcceleration = state->waterAcceleration;
            (*gWaterfxInterface)
                ->spawnSplashBurst((void*)obj, obj->anim.localPosX, state->splashPositionY, obj->anim.localPosZ, rad);
            return 0;
        } else {
            obj->anim.localPosY = positionY;
            return 1;
        }
    } else if (positionY - state->positionY >= lbl_803E37D4) {
        f32 b;
        f32 m = state->gravity + state->extraAcceleration;
        f32 g;
        f32 q;
        f32 t;
        f32 r;
        b = state->bounceVelocity;
        g = lbl_803E37D8 * m;
        q = sqrtf(b * b - g * state->dropHeight);
        t = lbl_803E37DC * m;

        if (t >= lbl_803E37D4) {
            r = t;
        } else {
            r = -t;
        }
        if (r <= lbl_803E37E0) {
            r = lbl_803E37C8;
        } else {
            f32 r2;
            f32 nb;
            nb = -b;
            r = (nb - q) / t;
            r2 = (nb + q) / t;
            r = (r > *(f32*)&lbl_803E37D4) ? r : r2;
        }
        state->flightTime = state->flightTime - r;
        obj->anim.localPosY = state->positionY;
        state->extraAcceleration = lbl_803E37FC;
        state->bounceVelocity = lbl_803E3800;
        return 0;
    } else {
        obj->anim.localPosY = positionY;
        return 1;
    }
}

void AppleOnTree_update(GameObject* obj) {
    f32 fa;
    int val;
    u32 gameBitValue;
    ObjTextureRuntimeSlot* texture;
    int state;
    int placement;
    int particleIndex;
    f32 fc;
    f32 fb;
    f32 fd;
    f32 frac;
    f32 sunTime;
    int message;

    state = *(int*)&obj->extra;
    placement = *(int*)&obj->anim.placementData;
    message = 0;
    if ((((AppleOnTreeState*)state)->flags & APPLE_ON_TREE_FLAG_PICKUP_PENDING) != 0) {
        while (val = ObjMsg_Pop(obj, (u32*)&message, 0x0, 0x0), val != 0) {
            switch (message) {
            case APPLE_ON_TREE_MESSAGE_PICKUP: {
                playerAddHealth(Obj_GetPlayerObject(), (int)((AppleOnTreeState*)state)->healthRestore);
                itemPickupDoParticleFx(obj, lbl_803E37C8, 0xff, 0x28);
                Sfx_PlayFromObject((int)obj, SFXTRIG_cam90_c);
                val = *(int*)&obj->extra;
                if (obj->anim.flags & OBJANIM_FLAG_OWNS_PLACEMENT_DATA) {
                    Obj_FreeObject(obj);
                } else {
                    if (obj->anim.hitReactState != NULL) {
                        ObjHits_DisableObject(obj);
                    }
                    ((AppleOnTreeState*)val)->flags = ((AppleOnTreeState*)val)->flags | APPLE_ON_TREE_FLAG_INACTIVE;
                }
                ((AppleOnTreeState*)state)->flags =
                    ((AppleOnTreeState*)state)->flags & ~APPLE_ON_TREE_FLAG_PICKUP_PENDING;
            }
            }
        }
        if ((((AppleOnTreeState*)state)->flags & APPLE_ON_TREE_FLAG_PICKUP_PENDING) != 0)
            return;
    }
    if ((((AppleOnTreeState*)state)->flags & APPLE_ON_TREE_FLAG_INACTIVE) == 0) {
        ((AppleOnTreeState*)state)->elapsedTime = ((AppleOnTreeState*)state)->elapsedTime + timeDelta;
        fa = ((AppleOnTreeState*)state)->flightTime;
        ((AppleOnTreeState*)state)->flightTime = fa + timeDelta;
        fb = ((AppleOnTreeState*)state)->elapsedTime;
        frac = fb / ((AppleOnTreeState*)state)->phaseDuration;
        switch (((AppleOnTreeState*)state)->animState) {
        case APPLE_ON_TREE_STATE_GROWING:
            val = ObjHits_GetPriorityHit(obj, 0x0, 0x0, 0x0);
            if ((val != 0) || ((((AppleOnTreePlacement*)placement)->despawnGameBit != APPLE_ON_TREE_GAME_BIT_NONE &&
                                (gameBitValue = mainGetBit((int)((AppleOnTreePlacement*)placement)->despawnGameBit),
                                 gameBitValue != 0)))) {
                int burstIndex;
                state = *(int*)&obj->extra;
                burstIndex = 0;
                do {
                    (*gPartfxInterface)->spawnObject(obj, APPLE_ON_TREE_PARTICLE_BURST, NULL, 2, -1, NULL);
                    burstIndex = burstIndex + 1;
                } while (burstIndex < APPLE_ON_TREE_PARTICLE_BURST_COUNT);
                if (obj->anim.hitReactState != NULL) {
                    ObjHits_DisableObject(obj);
                }
                ((AppleOnTreeState*)state)->flags = ((AppleOnTreeState*)state)->flags | APPLE_ON_TREE_FLAG_INACTIVE;
                ((AppleOnTreeState*)state)->elapsedTime = timeDelta;
                ((AppleOnTreeState*)state)->animState = APPLE_ON_TREE_STATE_BURST;
            } else {
                if (frac > ((AppleOnTreeState*)state)->growthEnd) {
                    obj->anim.rootMotionScale = obj->anim.modelInstance->rootMotionScaleBase;
                    ((AppleOnTreeState*)state)->animState = APPLE_ON_TREE_STATE_RIPE;
                } else {
                    fb = ((AppleOnTreeState*)obj->extra)->elapsedTime / ((AppleOnTreeState*)obj->extra)->phaseDuration;
                    fb = fb * (lbl_803E37C8 / ((AppleOnTreeState*)obj->extra)->growthEnd);
                    obj->anim.rootMotionScale = obj->anim.modelInstance->rootMotionScaleBase * fb;
                }
            }
            break;
        case APPLE_ON_TREE_STATE_RIPE:
            val = ObjHits_GetPriorityHit(obj, 0x0, 0x0, 0x0);
            if ((val != 0) || ((((AppleOnTreePlacement*)placement)->despawnGameBit != APPLE_ON_TREE_GAME_BIT_NONE &&
                                (gameBitValue = mainGetBit((int)((AppleOnTreePlacement*)placement)->despawnGameBit),
                                 gameBitValue != 0)))) {
                state = *(int*)&obj->extra;
                particleIndex = 0;
                do {
                    (*gPartfxInterface)->spawnObject(obj, APPLE_ON_TREE_PARTICLE_BURST, NULL, 2, -1, NULL);
                    particleIndex = particleIndex + 1;
                } while (particleIndex < APPLE_ON_TREE_PARTICLE_BURST_COUNT);
                if (obj->anim.hitReactState != NULL) {
                    ObjHits_DisableObject(obj);
                }
                ((AppleOnTreeState*)state)->flags = ((AppleOnTreeState*)state)->flags | APPLE_ON_TREE_FLAG_INACTIVE;
                ((AppleOnTreeState*)state)->elapsedTime = timeDelta;
                ((AppleOnTreeState*)state)->animState = APPLE_ON_TREE_STATE_BURST;
            } else {
                if (frac > ((AppleOnTreeState*)state)->ripeEnd) {
                    particleIndex = 0;
                    do {
                        (*gPartfxInterface)->spawnObject(obj, APPLE_ON_TREE_PARTICLE_BURST, NULL, 2, -1, NULL);
                        particleIndex = particleIndex + 1;
                    } while (particleIndex < APPLE_ON_TREE_PARTICLE_BURST_COUNT);
                    ((AppleOnTreeState*)state)->animState = APPLE_ON_TREE_STATE_FALLING;
                } else {
                    if ((*gSkyInterface)->getSunPosition(&sunTime) != 0) {
                        ObjAnim_AdvanceCurrentMove((int)obj, lbl_803E3804, timeDelta, 0);
                    } else {
                        ObjAnim_AdvanceCurrentMove((int)obj, lbl_803E3808, timeDelta, 0);
                    }
                }
            }
            break;
        case APPLE_ON_TREE_STATE_FALLING:
            if (frac > ((AppleOnTreeState*)state)->fallEnd) {
                val = *(int*)&obj->extra;
                texture = objFindTexture(obj, 0, 0);
                texture->textureId = 0;
                ((AppleOnTreeState*)val)->fallScale = lbl_803E37C8;
                obj->anim.rootMotionScale = obj->anim.modelInstance->rootMotionScaleBase;
                Obj_SetActiveModelIndex(obj, APPLE_ON_TREE_FALLEN_MODEL_INDEX);
                ((AppleOnTreeState*)state)->animState = APPLE_ON_TREE_STATE_LANDED;
            } else {
                f32 fallProgress;
                val = *(int*)&obj->extra;
                fallProgress = -(((AppleOnTreeState*)val)->phaseDuration * ((AppleOnTreeState*)val)->ripeEnd -
                                 ((AppleOnTreeState*)val)->elapsedTime) /
                               (((AppleOnTreeState*)val)->phaseDuration *
                                (((AppleOnTreeState*)val)->fallEnd - ((AppleOnTreeState*)val)->ripeEnd));
                fa = ((AppleOnTreeState*)val)->elapsedTime;
                fc = fa * fa;
                fc = fc * fc;
                state = 0x100 - (int)((fc * fc) / ((AppleOnTreeState*)val)->fallBlendDivisor);
                texture = objFindTexture(obj, 0, 0);
                texture->textureId = state;
                ((AppleOnTreeState*)val)->fallScale = lbl_803E37D0 * fallProgress + lbl_803E37CC;
                obj->anim.rootMotionScale =
                    obj->anim.modelInstance->rootMotionScaleBase * ((AppleOnTreeState*)val)->fallScale;
                Obj_SetActiveModelIndex(obj, APPLE_ON_TREE_FALLEN_MODEL_INDEX);
            }
            state = ObjHits_GetPriorityHit(obj, 0x0, 0x0, 0x0);
            if ((state != 0) || ((((AppleOnTreePlacement*)placement)->despawnGameBit != APPLE_ON_TREE_GAME_BIT_NONE &&
                                  (gameBitValue = mainGetBit((int)((AppleOnTreePlacement*)placement)->despawnGameBit),
                                   gameBitValue != 0)))) {
                appleontree_knockLoose(obj, APPLE_ON_TREE_KNOCK_WHILE_FALLING);
            }
            break;
        case APPLE_ON_TREE_STATE_LANDED:
            ((AppleOnTreeState*)state)->elapsedTime = fb - timeDelta;
            if (frac > ((AppleOnTreeState*)state)->landedEnd) {
                appleontree_knockLoose(obj, APPLE_ON_TREE_KNOCK_NATURALLY);
            } else {
                state = ObjHits_GetPriorityHit(obj, 0x0, 0x0, 0x0);
                if ((state != 0) ||
                    ((((AppleOnTreePlacement*)placement)->despawnGameBit != APPLE_ON_TREE_GAME_BIT_NONE &&
                      (gameBitValue = mainGetBit((int)((AppleOnTreePlacement*)placement)->despawnGameBit),
                       gameBitValue != 0)))) {
                    appleontree_knockLoose(obj, APPLE_ON_TREE_KNOCK_WHILE_LANDED);
                }
            }
            break;
        case APPLE_ON_TREE_STATE_KNOCKED:
            if (frac > ((AppleOnTreeState*)state)->fadeEnd) {
                ((AppleOnTreeState*)state)->animState = APPLE_ON_TREE_STATE_FADEOUT;
                ((AppleOnTreeState*)state)->elapsedTime = timeDelta;
            } else {
                int iteration;
                placement = 0;
                iteration = 0;
                fd = lbl_803E37D4;
                while (placement == 0) {
                    f32 t = ((AppleOnTreeState*)state)->flightTime;
                    fb = t * (((AppleOnTreeState*)state)->gravity + ((AppleOnTreeState*)state)->extraAcceleration);
                    fc = t * fb +
                         (((AppleOnTreeState*)state)->bounceVelocity * t + ((AppleOnTreeState*)state)->positionY);
                    if (((AppleOnTreeState*)state)->waterAcceleration > fd) {
                        placement = appleontree_bounceWaterStep(obj, (AppleOnTreeState*)state, fc);
                    } else {
                        placement = appleontree_bounceGroundStep(obj, (AppleOnTreeState*)state, fc);
                    }
                    iteration = iteration + 1;
                    if (!((iteration == 100) || (iteration != 0x66)))
                        break;
                }
                if (lbl_803E37D4 != ((AppleOnTreeState*)state)->dropHeight) {
                    fb = ((AppleOnTreeState*)state)->flightTime / ((AppleOnTreeState*)state)->totalFlightTime;
                    obj->anim.rotX = (f32)((AppleOnTreeState*)state)->rotX * fb;
                    obj->anim.rotY = (f32)((AppleOnTreeState*)state)->rotY * fb;
                    obj->anim.rotZ = (f32)((AppleOnTreeState*)state)->rotZ * fb;
                }
                texture = objFindTexture(obj, 0, 0);
                texture->textureId = (int)(lbl_803E380C * frac);
                appleontree_handleCollectableHit(obj);
            }
            break;
        case APPLE_ON_TREE_STATE_BURST:
            if (fb > lbl_803E3810) {
                placement = *(int*)&obj->extra;
                if (obj->anim.flags & OBJANIM_FLAG_OWNS_PLACEMENT_DATA) {
                    Obj_FreeObject(obj);
                } else {
                    if (obj->anim.hitReactState != NULL) {
                        ObjHits_DisableObject(obj);
                    }
                    ((AppleOnTreeState*)placement)->flags =
                        ((AppleOnTreeState*)placement)->flags | APPLE_ON_TREE_FLAG_INACTIVE;
                }
            }
            break;
        case APPLE_ON_TREE_STATE_FADEOUT:
            frac = lbl_803E3814;
            if (fb > frac) {
                placement = *(int*)&obj->extra;
                if (obj->anim.flags & OBJANIM_FLAG_OWNS_PLACEMENT_DATA) {
                    Obj_FreeObject(obj);
                } else {
                    if (obj->anim.hitReactState != NULL) {
                        ObjHits_DisableObject(obj);
                    }
                    ((AppleOnTreeState*)placement)->flags =
                        ((AppleOnTreeState*)placement)->flags | APPLE_ON_TREE_FLAG_INACTIVE;
                }
            } else {
                placement = (int)(lbl_803E3818 * fb / frac);
                obj->anim.alpha = 0xff - placement;
                appleontree_handleCollectableHit(obj);
            }
        }
    }
}
void AppleOnTree_init(GameObject* obj, AppleOnTreePlacement* placement) {
    AppleOnTreeState* state;
    f32 zeroScale;
    f32 timeScale;
    f32 progress;
    int eventBit;
    ObjTextureRuntimeSlot* texture;

    state = obj->extra;

    state->unk00 = placement->unk18;
    state->phaseDuration = (f32)placement->phaseDuration;
    state->elapsedTime = (f32)placement->initialElapsedTime;
    {
        state->growthEnd = (f32)placement->growthEndFraction / lbl_803E3828;
        progress = (f32)placement->ripeEndFraction / lbl_803E3828;
        state->ripeEnd = progress + state->growthEnd;
        progress = (f32)placement->fallEndFraction / lbl_803E3828;
        state->fallEnd = progress + state->ripeEnd;
        progress = (f32)placement->landedEndFraction / lbl_803E3828;
        state->landedEnd = progress + state->fallEnd;
        state->fadeEnd = (f32)placement->fadeEndFraction / lbl_803E3828;
        state->waterAcceleration = (f32)placement->unk25 / lbl_803E3828;
        state->waterAcceleration = state->waterAcceleration * lbl_803E37DC;
        state->fallScale = lbl_803E37C8;
        state->healthRestore = 0;
        zeroScale = lbl_803E37D4;
        state->extraAcceleration = zeroScale;
        state->gravity = lbl_803E382C;
        state->bounceVelocity = zeroScale;

        timeScale = state->phaseDuration * state->fallEnd;
        timeScale *= timeScale;
        timeScale *= timeScale;
        zeroScale = timeScale * timeScale;
        state->fallBlendDivisor = zeroScale * lbl_803E3830;

        obj->anim.rotX = randomGetRange(-0x8000, 0x7fff);
        obj->anim.rootMotionScale = lbl_803E3834;
        Obj_SetActiveModelIndex(obj, APPLE_ON_TREE_HANGING_MODEL_INDEX);

        eventBit = placement->despawnGameBit;
        if ((eventBit != APPLE_ON_TREE_GAME_BIT_NONE) && (mainGetBit(eventBit) != 0)) {
            state->elapsedTime = lbl_803E3838;
            state->animState = APPLE_ON_TREE_STATE_FADEOUT;
        } else {
            progress = state->elapsedTime / state->phaseDuration;
            if (progress < state->growthEnd) {
                state->animState = APPLE_ON_TREE_STATE_GROWING;
            } else if (progress < state->ripeEnd) {
                obj->anim.rootMotionScale = obj->anim.modelInstance->rootMotionScaleBase;
                state->animState = APPLE_ON_TREE_STATE_RIPE;
            } else if (progress < state->fallEnd) {
                state->animState = APPLE_ON_TREE_STATE_FALLING;
            } else {
                AppleOnTreeState* reread = obj->extra;
                texture = objFindTexture(obj, 0, 0);
                texture->textureId = 0;
                reread->fallScale = lbl_803E37C8;
                obj->anim.rootMotionScale = obj->anim.modelInstance->rootMotionScaleBase;
                Obj_SetActiveModelIndex(obj, APPLE_ON_TREE_FALLEN_MODEL_INDEX);
                state->animState = APPLE_ON_TREE_STATE_LANDED;
            }
        }

        ObjMsg_AllocQueue(obj, APPLE_ON_TREE_MESSAGE_QUEUE_SIZE);
    }
}
