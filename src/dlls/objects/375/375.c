/*
 * Shared texture animation for the DFSH_Door2S, DFSH_Door3S, and
 * DFSH_Door4S object definitions.
 */

#include "dlls/objects/375.h"

#include "dolphin/MSL_C/PPCEABI/bare/H/math_trig_api.h"
#include "game/objects/object.h"
#include "main/frame_timing.h"
#include "main/gamebits_api.h"
#include "main/object_render.h"
#include "main/objtexture.h"

extern const f32 gDll177TextureValueMaximum;
extern const f32 gDll177TexturePulseAmplitude;
extern const f32 gDll177UnitValue;
extern const f32 gDll177Pi;
extern const f32 gDll177HalfCycleUnits;

int dll_177_updateTextureAnimation(GameObject* obj) {
    ObjTextureRuntimeSlot* texture;
    Dll177State* state;
    const Dll177Placement* placement;
    int alpha;
    u32 phaseStep;
    f32 phase;

    state = obj->extra;
    placement = (const Dll177Placement*)obj->anim.placementData;
    switch (state->textureState) {
    case DLL_177_TEXTURE_STATE_WAIT_FOR_GAME_BIT:
        if (mainGetBit(placement->gameBit) != 0) {
            state->textureState = DLL_177_TEXTURE_STATE_FADE_IN;
        }
        break;
    case DLL_177_TEXTURE_STATE_FADE_IN:
        texture = objFindTexture(obj, 0, 0);
        if (texture != NULL) {
            alpha = texture->textureId + framesThisStep * 0x10;
            if (alpha > 0x100) {
                alpha = 0x100;
                state->textureState = DLL_177_TEXTURE_STATE_PULSE;
            }
            texture->textureId = alpha;
        }
        break;
    case DLL_177_TEXTURE_STATE_PULSE:
    default:
        texture = objFindTexture(obj, 0, 0);
        if (texture != NULL) {
            phaseStep = (state->pulsePhase + framesThisStep * 800) & 0xFFFF;
            state->pulsePhase = phaseStep;
            phase = (gDll177Pi * (f32)(u32)state->pulsePhase) / gDll177HalfCycleUnits;
            texture->textureId = (s32) - (gDll177TexturePulseAmplitude * (gDll177UnitValue - mathCosf(phase)) -
                                          gDll177TextureValueMaximum);
        }
        break;
    }
    return 0;
}

int dll_177_getExtraSize(void) {
    return sizeof(Dll177State);
}

int dll_177_getObjectTypeId(void) {
    return 0;
}

void dll_177_free(void) {
}

void dll_177_render(GameObject* obj, int renderArg2, int renderArg3, int renderArg4, int renderArg5, s8 visible) {
    if (visible != 0) {
        objRenderModelAndHitVolumes(obj, renderArg2, renderArg3, renderArg4, renderArg5, gDll177UnitValue);
    }
}

void dll_177_hitDetect(void) {
}

void dll_177_update(void) {
}

void dll_177_init(GameObject* obj, const Dll177Placement* placement) {
    Dll177State* state;
    ObjTextureRuntimeSlot* texture;

    state = obj->extra;
    obj->animEventCallback = dll_177_updateTextureAnimation;
    if (mainGetBit(placement->gameBit) != 0) {
        state->textureState = DLL_177_TEXTURE_STATE_PULSE;
    } else {
        state->textureState = DLL_177_TEXTURE_STATE_WAIT_FOR_GAME_BIT;
    }
    texture = objFindTexture(obj, 0, 0);
    if (texture != NULL) {
        if (state->textureState == DLL_177_TEXTURE_STATE_PULSE) {
            texture->textureId = 1;
        } else {
            texture->textureId = 0;
        }
    }
    state->pulsePhase = 0;
}

void dll_177_release(void) {
}

void dll_177_initialise(void) {
}

ObjectDescriptor gDll177ObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)dll_177_initialise,
    (ObjectDescriptorCallback)dll_177_release,
    0,
    (ObjectDescriptorCallback)dll_177_init,
    (ObjectDescriptorCallback)dll_177_update,
    (ObjectDescriptorCallback)dll_177_hitDetect,
    (ObjectDescriptorCallback)dll_177_render,
    (ObjectDescriptorCallback)dll_177_free,
    (ObjectDescriptorCallback)dll_177_getObjectTypeId,
    dll_177_getExtraSize,
};
