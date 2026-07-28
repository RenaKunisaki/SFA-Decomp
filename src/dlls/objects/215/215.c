/*
 * Projectile object (DLL slot 215).
 *
 * Flies ballistically with a glow light and bursts after a hit or contact.
 * Sequence 0x869 selects the explosive variant; other sequences use poison
 * particle effects.
 */
#include "dlls/objects/215.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/audio/sfx_channel_volume_api.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/dll/partfx_interface.h"
#include "main/frame_timing.h"
#include "main/model_light.h"
#include "main/object_render.h"
#include "main/objfx.h"
#include "sys/objects/lifecycle.h"

#define KALDACHOMPSPIT_HIT_VOLUME_SLOT_EXPLOSIVE 31
#define KALDACHOMPSPIT_HIT_VOLUME_SLOT_DEFAULT   10

#define KALDACHOMPSPIT_SEQUENCE_ID_EXPLOSIVE 0x869

#define KALDACHOMPSPIT_PARTFX_POISON_TRAIL 0x714
#define KALDACHOMPSPIT_PARTFX_POISON_BURST 0x715

#define KALDACHOMPSPIT_INITIAL_LIFETIME           400
#define KALDACHOMPSPIT_GENERAL_HIT_BURST_LIFETIME 380
#define KALDACHOMPSPIT_FADE_START_LIFETIME        283
#define KALDACHOMPSPIT_BURST_DESPAWN_DELAY        220
#define KALDACHOMPSPIT_POISON_BURST_COUNT         25
#define KALDACHOMPSPIT_EXPLOSION_SCALE_MIN        50
#define KALDACHOMPSPIT_EXPLOSION_SCALE_MAX        60
#define KALDACHOMPSPIT_GLOW_ALPHA_JITTER          25
#define KALDACHOMPSPIT_LIGHT_BASE_DISTANCE        50.0f
#define KALDACHOMPSPIT_LIGHT_ATTENUATION_RANGE    40
#define KALDACHOMPSPIT_GRAVITY                    0.07f
#define KALDACHOMPSPIT_ALPHA_FADE_RATE            4.0f

ObjectDescriptor gKaldaChompSpitObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)KaldaChompSpit_initialise,
    (ObjectDescriptorCallback)KaldaChompSpit_release,
    0,
    (ObjectDescriptorCallback)KaldaChompSpit_init,
    (ObjectDescriptorCallback)KaldaChompSpit_update,
    (ObjectDescriptorCallback)KaldaChompSpit_hitDetect,
    (ObjectDescriptorCallback)KaldaChompSpit_render,
    (ObjectDescriptorCallback)KaldaChompSpit_free,
    (ObjectDescriptorCallback)KaldaChompSpit_getObjectTypeId,
    KaldaChompSpit_getExtraSize,
};

void kaldachompspit_burst(GameObject* obj) {
    int i;
    KaldaChompSpitState* state;
    ObjHitsPriorityState* hitState;
    u8 randomVariant;

    state = obj->extra;
    obj->anim.alpha = 0;
    obj->userData1 = KALDACHOMPSPIT_BURST_DESPAWN_DELAY;
    hitState = (ObjHitsPriorityState*)obj->anim.hitReactState;
    hitState->flags &= ~OBJHITS_PRIORITY_STATE_ENABLED;
    if (state->light != NULL) {
        modelLightStruct_setEnabled(state->light, 0, 1.0f);
    }
    if (obj->anim.seqId == KALDACHOMPSPIT_SEQUENCE_ID_EXPLOSIVE) {
        randomVariant = randomGetRange(0, 1);
        spawnExplosion(obj,
                       (f32)randomGetRange(KALDACHOMPSPIT_EXPLOSION_SCALE_MIN, KALDACHOMPSPIT_EXPLOSION_SCALE_MAX),
                       1, 1, 0, randomVariant, 0, 1, 0);
    } else {
        for (i = 0; i < KALDACHOMPSPIT_POISON_BURST_COUNT; i++) {
            (*gPartfxInterface)->spawnObject((void*)obj, KALDACHOMPSPIT_PARTFX_POISON_BURST, NULL, 1, -1, &i);
        }
        Sfx_PlayFromObject((int)obj, SFXTRIG_lummy311);
    }
}

int KaldaChompSpit_getExtraSize(void) {
    return sizeof(KaldaChompSpitState);
}

int KaldaChompSpit_getObjectTypeId(void) {
    return 0;
}

void KaldaChompSpit_free(GameObject* obj) {
    KaldaChompSpitState* state = obj->extra;
    ModelLightStruct* light = state->light;
    if (light != NULL) {
        ModelLightStruct_free(light);
    }
}

void KaldaChompSpit_render(GameObject* obj, int fwdArg2, int fwdArg3, int fwdArg4, int fwdArg5, s8 visible) {
    KaldaChompSpitState* state = obj->extra;
    ModelLightStruct* light = state->light;
    if (light != NULL && light->glowType != 0 && light->enabled != 0) {
        queueGlowRender(light);
    }
    if (visible != 0) {
        objRenderModelAndHitVolumes(obj, fwdArg2, fwdArg3, fwdArg4, fwdArg5, 1.0f);
    }
}

void KaldaChompSpit_hitDetect(GameObject* obj) {
    (void)obj;
}

void KaldaChompSpit_update(GameObject* obj) {
    ObjAnimComponent* objAnim;
    KaldaChompSpitState* state;
    f32 moveX;
    ModelLightStruct* light;
    int alphaJitter;
    f32 moveY;
    f32 moveZ;
    s16 glowAlpha;
    f32 alphaDecay;

    objAnim = &obj->anim;
    state = obj->extra;
    obj->userData1 = (int)((f32)obj->userData1 - timeDelta);
    if (obj->userData1 < 0) {
        Sfx_StopObjectChannel((int)obj, 0x7f);
        Obj_FreeObject(obj);
    } else if (objAnim->alpha != 0) {
        if (obj->userData1 < KALDACHOMPSPIT_FADE_START_LIFETIME) {
            obj->anim.velocityY = -(KALDACHOMPSPIT_GRAVITY * timeDelta - obj->anim.velocityY);
            if ((f32)(u32)objAnim->alpha - (alphaDecay = KALDACHOMPSPIT_ALPHA_FADE_RATE * timeDelta) > 0.0f) {
                objAnim->alpha = (f32)(u32)objAnim->alpha - alphaDecay;
            } else {
                Sfx_StopObjectChannel((int)obj, 0x7f);
                objAnim->alpha = 0;
            }
            Sfx_SetObjectChannelVolume((u32)obj, 0x40, (u8)(objAnim->alpha >> 1), 0.5f);
        }
        moveX = obj->anim.velocityX * timeDelta;
        moveY = obj->anim.velocityY * timeDelta;
        moveZ = obj->anim.velocityZ * timeDelta;
        objMove(obj, moveX, moveY, moveZ);
        if (obj->anim.seqId == KALDACHOMPSPIT_SEQUENCE_ID_EXPLOSIVE) {
            ObjHits_SetHitVolumeSlot((ObjAnimComponent*)obj, KALDACHOMPSPIT_HIT_VOLUME_SLOT_EXPLOSIVE, 1, 0);
            obj->anim.rotX += 0x100;
            obj->anim.rotY += 0x800;
        } else {
            ObjHits_SetHitVolumeSlot((ObjAnimComponent*)obj, KALDACHOMPSPIT_HIT_VOLUME_SLOT_DEFAULT, 1, 0);
            obj->anim.rotX = getAngle(moveX, moveZ) - 0x8000;
            obj->anim.rotY = 0x4000 - getAngle(sqrtf(moveX * moveX + moveZ * moveZ), moveY);
        }
        ObjHits_EnableObject(obj);
        if (((ObjHitsPriorityState*)obj->anim.hitReactState)->lastHitObject != 0) {
            if (obj->userData1 < KALDACHOMPSPIT_GENERAL_HIT_BURST_LIFETIME) {
                kaldachompspit_burst(obj);
                return;
            }
            if ((((ObjHitsPriorityState*)obj->anim.hitReactState)->lastHitObject == (u32)Obj_GetPlayerObject()) ||
                (((ObjHitsPriorityState*)obj->anim.hitReactState)->lastHitObject == (u32)getTrickyObject())) {
                kaldachompspit_burst(obj);
                return;
            }
        }
        if (((ObjHitsPriorityState*)obj->anim.hitReactState)->contactFlags != 0) {
            kaldachompspit_burst(obj);
        } else {
            if (obj->anim.seqId == KALDACHOMPSPIT_SEQUENCE_ID_EXPLOSIVE) {
                objfx_spawnPulseBurst((void*)obj, 1.0f, 1, 0, 0, NULL);
            } else {
                (*gPartfxInterface)
                    ->spawnObject((void*)obj, KALDACHOMPSPIT_PARTFX_POISON_TRAIL, NULL, 2, -1, &objAnim->alpha);
                (*gPartfxInterface)->spawnObject((void*)obj, KALDACHOMPSPIT_PARTFX_POISON_BURST, NULL, 1, -1, NULL);
                (*gPartfxInterface)->spawnObject((void*)obj, KALDACHOMPSPIT_PARTFX_POISON_BURST, NULL, 1, -1, NULL);
            }
            light = state->light;
            if (light != NULL && light->glowType != 0 && light->enabled != 0) {
                alphaJitter = randomGetRange(-KALDACHOMPSPIT_GLOW_ALPHA_JITTER, KALDACHOMPSPIT_GLOW_ALPHA_JITTER);
                light = state->light;
                glowAlpha = light->glowAlpha + light->glowAlphaStep + alphaJitter;
                if (glowAlpha < 0) {
                    glowAlpha = 0;
                    light->glowAlphaStep = 0;
                } else if (glowAlpha > 0xff) {
                    glowAlpha = 0xff;
                    light->glowAlphaStep = 0;
                }
                state->light->glowAlpha = glowAlpha;
            }
        }
    }
}

void KaldaChompSpit_init(GameObject* obj) {
    KaldaChompSpitState* state;

    state = obj->extra;
    obj->userData1 = KALDACHOMPSPIT_INITIAL_LIFETIME;
    ObjHits_DisableObject(obj);
    obj->anim.alpha = 0xff;
    Sfx_PlayFromObject((int)obj, SFXTRIG_whiz3_c);
    obj->objectFlags |= OBJECT_OBJFLAG_HITDETECT_DISABLED;
    if (state->light == NULL) {
        state->light = objCreateLight(obj, 1);
        if (state->light != NULL) {
            modelLightStruct_setLightKind(state->light, MODEL_LIGHT_KIND_POINT);
        }
    }
    if (state->light != NULL) {
        f32 lightPos = 0.0f;
        modelLightStruct_setPosition(state->light, lightPos, lightPos, lightPos);
        if (obj->anim.seqId == KALDACHOMPSPIT_SEQUENCE_ID_EXPLOSIVE) {
            modelLightStruct_setDiffuseColor(state->light, 0xff, 0xc0, 0, 0xff);
            modelLightStruct_setSpecularColor(state->light, 0xff, 0xc0, 0, 0xff);
            modelLightStruct_setupGlow(state->light, 0, 0xff, 0xc0, 0, 0x7f,
                                       0.6f * (KALDACHOMPSPIT_LIGHT_BASE_DISTANCE * obj->anim.rootMotionScale));
            modelLightStruct_setDiffuseTargetColor(state->light, 0xff, 0xd2, 0, 0xff);
        } else {
            modelLightStruct_setDiffuseColor(state->light, 0, 0xff, 0, 0xff);
            modelLightStruct_setSpecularColor(state->light, 0, 0xff, 0, 0xff);
            modelLightStruct_setupGlow(state->light, 0, 0, 0xff, 0, 0x28,
                                       KALDACHOMPSPIT_LIGHT_BASE_DISTANCE * obj->anim.rootMotionScale);
            modelLightStruct_setDiffuseTargetColor(state->light, 0, 0xff, 0, 0xff);
        }
        {
            int nearDistance = (int)(KALDACHOMPSPIT_LIGHT_BASE_DISTANCE * obj->anim.rootMotionScale);
            modelLightStruct_setDistanceAttenuation(state->light, nearDistance,
                                                    (f32)(nearDistance + KALDACHOMPSPIT_LIGHT_ATTENUATION_RANGE));
        }
        lightSetField4D(state->light, 1);
        modelLightStruct_setEnabled(state->light, 1, 1.0f);
        modelLightStruct_startColorFade(state->light, 1, 3);
    }
}

void KaldaChompSpit_release(void) {
}

void KaldaChompSpit_initialise(void) {
}
