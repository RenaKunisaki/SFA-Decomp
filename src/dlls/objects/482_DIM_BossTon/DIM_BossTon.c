#include "dlls/object_descriptor.h"
#include "game/objects/object.h"
#include "main/audio/music_api.h"
#include "main/audio/sfx.h"
#include "main/camera_shake_api.h"
#include "main/dll/DIM/DIMbossspit.h"
#include "main/dll/DIM/DIMbosstonsil.h"
#include "main/dll/DIM/dll_223.h"
#include "main/dll/baddie_control_interface.h"
#include "main/dll/baddie_state.h"
#include "main/dll/partfx_interface.h"
#include "main/frame_timing.h"
#include "main/gamebits.h"
#include "main/mapEvent.h"
#include "main/object_render.h"
#include "main/obj_group.h"
#include "main/obj_message.h"
#include "main/obj_path.h"
#include "main/objfx.h"
#include "main/objhits.h"
#include "main/objseq.h"
#include "main/pad.h"
#include "main/player_control_interface.h"
#include "main/render_envfx_api.h"
#include "main/shader_api.h"
#include "main/sky_api.h"
#include "main/vecmath.h"
#include "sys/objects.h"

extern f32 lbl_803DDB98;
extern f32 lbl_803DDB9C;
extern f32 lbl_803DDBA0;
extern f32 lbl_803DDBA4;

#define DIMBOSSTONSIL_HIT_EFFECT_ID     0x4b2
#define DIMBOSSTONSIL_HIT_EFFECT_ALT_ID 0x4b3
#define DIMBOSSTONSIL_PRIMARY_HIT_SFX   0x18a
#define DIMBOSSTONSIL_ALT_HIT_SFX       0x18b
#define DIMBOSSTONSIL_NORMAL_HIT_SFX    0x18c
#define DIMBOSSTONSIL_HIT_GAMEBIT       0x20c
#define DIMBOSSTONSIL_ADVANCE_MSG       0xe0001
/* particle-spawn flag word: bit 0x200000 | bit 0x1 */
#define DIMBOSSTONSIL_HIT_FX_FLAGS 0x200001

int DIMbosstonsil_updateHitReaction(GameObject* obj, DIMbosstonsilState* state, int unused)
{
    if (state->active != 0)
    {
        (*gPlayerInterface)->setState(obj, state, 1);
    }
    if (state->hitResult != 0)
    {
        return 1;
    }
    return 0;
}

int DIMbosstonsil_enableHitReaction(GameObject* obj, DIMbosstonsilState* state)
{
    if (state->stunReady != 0)
    {
        state->active = 1;
        (*gPlayerInterface)->setState(obj, state, 0);
    }
    return 0;
}

int DIMbosstonsil_chooseHitReaction(GameObject* obj, DIMbosstonsilState* state)
{
    u16 moveId;
    u16 unused1;
    u16 unused2;

    if (state->active != 0)
    {
        lbl_803DDB9C = lbl_803DDBA0;
        (*gBaddieControlInterface)
            ->getTargetGeometry(obj, Obj_GetPlayerObject(), 4, &moveId, &unused1, &unused2);
        switch (moveId)
        {
        case 0:
            if (state->active != 0)
            {
                ObjAnim_SetCurrentMove((int)obj, 1, 0.0f, 0);
                state->hitResult = 0;
            }
            break;
        case 1:
            if (state->active != 0)
            {
                ObjAnim_SetCurrentMove((int)obj, 3, 0.0f, 0);
                state->hitResult = 0;
            }
            break;
        case 2:
            if (state->active != 0)
            {
                ObjAnim_SetCurrentMove((int)obj, 2, 0.0f, 0);
                state->hitResult = 0;
            }
            break;
        default:
            if (state->active != 0)
            {
                ObjAnim_SetCurrentMove((int)obj, 4, 0.0f, 0);
                state->hitResult = 0;
            }
            break;
        }
        state->recoveryTimer = 0.005f;
    }
    return 0;
}

int DIMbosstonsil_startIdleHitReaction(GameObject* obj, DIMbosstonsilState* state)
{
    if (state->active != 0)
    {
        ObjAnim_SetCurrentMove((int)obj, 0, 0.0f, 0);
        state->hitResult = 0;
    }
    state->recoveryTimer = 0.01f;
    return 0;
}

static int DIMbosstonsil_isWithinLightRange(f32 dist)
{
    return dist > 30.0f && dist < 50.0f;
}

void DIMbosstonsil_checkHit(GameObject* obj, DIMbosstonsilState* state)
{
    int hitObj;
    int modelPart;
    u32 hitVolume;
    u32 spawnArgs[7];
    f32* spawnPos;
    int hit;

    hit = ObjHits_GetPriorityHit(obj, &hitObj, &modelPart, &hitVolume);
    if (hit != 0)
    {
        spawnPos = (f32*)((char*)spawnArgs + 0xc);
        {
            f32(*modelPos)[4] =
                (f32(*)[4])(*(int*)(*(int*)(*(int*)&(obj)->anim.banks + ((s8)((u8*)obj)[0xad] << 2)) + 0x50));
            spawnPos[0] = playerMapOffsetX + modelPos[modelPart][1];
            spawnPos[1] = modelPos[modelPart][2];
            spawnPos[2] = playerMapOffsetZ + modelPos[modelPart][3];
        }
        (*gPartfxInterface)
            ->spawnObject(obj, DIMBOSSTONSIL_HIT_EFFECT_ID, spawnArgs, DIMBOSSTONSIL_HIT_FX_FLAGS, -1, NULL);
        (*gPartfxInterface)
            ->spawnObject(obj, DIMBOSSTONSIL_HIT_EFFECT_ALT_ID, spawnArgs, DIMBOSSTONSIL_HIT_FX_FLAGS, -1, NULL);
        objLightFn_8009a1dc(obj, 0.028f, spawnArgs, 3, 0);
        Sfx_PlayFromObject((u32)obj, DIMBOSSTONSIL_PRIMARY_HIT_SFX);
        doRumble(16.0f);
        if (state->hitPointsLeft != 0)
        {
            Sfx_PlayFromObject((u32)obj, DIMBOSSTONSIL_ALT_HIT_SFX);
        }
        else
        {
            Sfx_PlayFromObject((u32)obj, DIMBOSSTONSIL_NORMAL_HIT_SFX);
        }
        CameraShake_SetAllMagnitudes(3.0f);
        if (0.0f == lbl_803DDB98)
        {
            state->active = 1;
            state->hitResult = 0;
            state->hitDamageCount = hit;
            state->hitPointsLeft--;
            gDIMbosstonsilRoutePhase++;
            mainSetBits(DIMBOSSTONSIL_HIT_GAMEBIT, gDIMbosstonsilRoutePhase);
            if (gDIMbosstonsilRoutePhase == 3 || gDIMbosstonsilRoutePhase == 7)
            {
                lbl_803DDB98 = 90.0f;
            }
            else
            {
                lbl_803DDB98 = 0.0f;
            }
            (*gPlayerInterface)->setState(obj, state, 1);
            state->field270 = 1;
            ObjMsg_SendToObject((void*)hitObj, DIMBOSSTONSIL_ADVANCE_MSG, obj, 0);
        }
    }
}

#define DIMBOSSSPIT_MODEL_ACTIVE_FLAG    0x1
#define DIMBOSSSPIT_OBJECT_DISABLED_FLAG 0x8

#define DIMBOSSSPIT_GAMEBIT_ACTIVE     0x20e
#define DIMBOSSSPIT_GAMEBIT_ROUTE_LOW  0x268
#define DIMBOSSSPIT_GAMEBIT_ROUTE_HIGH 0x311

#define DIMBOSSSPIT_ROUTE_HIGH_THRESHOLD  7
#define DIMBOSSSPIT_ROUTE_SPLIT_THRESHOLD 3
#define DIMBOSSSPIT_RUMBLE_SFX            0x189

void dimBossTonsil_newState_hitFightMain(GameObject* obj, ObjAnimUpdateState* animUpdate, DIMbosstonsilState* state,
                                         DIMbosstonsilState* updateState)
{
    f32 timer;

    timer = 0.0f;

    ((ObjHitsPriorityState*)obj->anim.hitReactState)->flags |= DIMBOSSSPIT_MODEL_ACTIVE_FLAG;

    updateState->effectActive = 1;

    (*gBaddieControlInterface)
        ->updateGravity(obj, updateState, timer, 1);

    (*gBaddieControlInterface)
        ->processMessages(obj, updateState, state->animPoints, state->animFrame,
                          &state->hitReactMode, 0, 0, 0);

    if (0.0f != lbl_803DDBA4)
    {
        lbl_803DDBA4 = lbl_803DDBA4 - timeDelta;
        timer = lbl_803DDBA4 / 8.0f;
        if (lbl_803DDBA4 <= 1.0f)
        {
            lbl_803DDBA4 = 0.0f;
            updateState->animFinished = 0;
            ((ObjHitsPriorityState*)obj->anim.hitReactState)->flags &= ~DIMBOSSSPIT_MODEL_ACTIVE_FLAG;
            *(u8*)&obj->anim.resetHitboxMode =
                (u8)(*(u8*)&obj->anim.resetHitboxMode | DIMBOSSSPIT_OBJECT_DISABLED_FLAG);
            mainSetBits(DIMBOSSSPIT_GAMEBIT_ACTIVE, 0);
            if (gDIMbosstonsilRoutePhase >= DIMBOSSSPIT_ROUTE_HIGH_THRESHOLD)
            {
                mainSetBits(DIMBOSSSPIT_GAMEBIT_ROUTE_HIGH, 1);
            }
            else
            {
                mainSetBits(DIMBOSSSPIT_GAMEBIT_ROUTE_LOW, 1);
            }
        }
    }
    else
    {
        timer += 100.0f;
    }

    if (lbl_803DDBA0 >= lbl_803DDB9C)
    {
        Sfx_PlayFromObject((u32)obj, DIMBOSSSPIT_RUMBLE_SFX);
        if (timer > 100.0f)
            timer = 100.0f;
        if (timer < 30.0f)
            timer = 30.0f;
        lbl_803DDB9C = lbl_803DDB9C + timer;
        doRumble(8.0f);
    }

    lbl_803DDBA0 = lbl_803DDBA0 + timeDelta;
    DIMbosstonsil_checkHit(obj, updateState);

    if (0.0f != lbl_803DDB98)
    {
        lbl_803DDB98 = lbl_803DDB98 - timeDelta;
        if (lbl_803DDB98 <= 0.0f)
        {
            lbl_803DDB98 = 0.0f;
            updateState->animFinished = 0;
            ((ObjHitsPriorityState*)obj->anim.hitReactState)->flags &= ~DIMBOSSSPIT_MODEL_ACTIVE_FLAG;
            *(u8*)&obj->anim.resetHitboxMode =
                (u8)(*(u8*)&obj->anim.resetHitboxMode | DIMBOSSSPIT_OBJECT_DISABLED_FLAG);
            mainSetBits(DIMBOSSSPIT_GAMEBIT_ACTIVE, 0);
            if (gDIMbosstonsilRoutePhase == DIMBOSSSPIT_ROUTE_SPLIT_THRESHOLD)
            {
                mainSetBits(DIMBOSSSPIT_GAMEBIT_ROUTE_LOW, 1);
            }
            else
            {
                mainSetBits(DIMBOSSSPIT_GAMEBIT_ROUTE_HIGH, 1);
            }
        }
    }

    state->savedObjFieldC0 = *(u32*)&obj->pendingParentObj;
    *(u32*)&obj->pendingParentObj = 0;

    (*gPlayerInterface)->update(obj, updateState, timeDelta, timeDelta, &gDIMbosstonsilStateHandlers, &gDIMbosstonsilSubstateHandlers);

    *(u32*)&obj->pendingParentObj = state->savedObjFieldC0;
}

#define DIMBOSSTONSIL_OBJGROUP 3
#define DIMBOSSTONSIL_PARTFX   0x4bd

DIMbosstonsilStateHandlerTable gDIMbosstonsilStateHandlers;
DIMbosstonsilSubstateHandlerTable gDIMbosstonsilSubstateHandlers;
f32 lbl_803DDBA4;
f32 lbl_803DDBA0;
f32 lbl_803DDB9C;
f32 lbl_803DDB98;
s8 gDIMbosstonsilRoutePhase;
ModelLightStruct* gDIMbosstonsilLight;

int DIMbosstonsil_SeqFn(GameObject* obj, u32 unused, ObjAnimUpdateState* animUpdate)
{
    DIMbosstonsilState* state;
    DIMbosstonsilConfig* config;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
    s16 lightValue;
    int eventIndex;
    int eventId;
    int hitReactMode;
    int animOk;

    state = (obj)->extra;
    config = (DIMbosstonsilConfig*)(obj)->anim.placementData;

    if (gDIMbosstonsilLight != NULL)
    {
        modelLightStruct_getSpecularColor((ModelLightStruct*)gDIMbosstonsilLight, &red, &green, &blue, &alpha);
        modelLightStruct_setGlowColor((ModelLightStruct*)gDIMbosstonsilLight, red, green, blue, 0xc0);
        if (gDIMbosstonsilLight->glowType != 0 && gDIMbosstonsilLight->enabled != 0)
        {
            lightValue = gDIMbosstonsilLight->glowAlpha + gDIMbosstonsilLight->glowAlphaStep;
            if (lightValue < 0)
            {
                lightValue = 0;
                gDIMbosstonsilLight->glowAlphaStep = 0;
            }
            else if (lightValue > 0xc)
            {
                lightValue = lightValue + randomGetRange(-0xc, 0xc);
                if (lightValue > 0xff)
                {
                    lightValue = 0xff;
                    gDIMbosstonsilLight->glowAlphaStep = 0;
                }
            }
            gDIMbosstonsilLight->glowAlpha = lightValue;
        }
    }

    if ((obj)->userData1 != 0)
    {
        return 0;
    }

    for (eventIndex = 0; eventIndex < (int)(u32)animUpdate->eventCount; eventIndex++)
    {
        eventId = animUpdate->eventIds[eventIndex];
        switch (eventId)
        {
        case DIMBOSSTONSIL_ANIM_EVENT_START_STEAM:
            skyFn_80089710(7, 1, 0);
            skySetLightDirection(7, -0.1f, -0.1f, 1.0f);
            skySetBaseColor(7, 0xff, 0xb4, 0xb4, 0x7f, 0x28);
            getEnvfxAct(obj, obj, DIMBOSSTONSIL_STEAM_ENVFX, 0);
            Music_Trigger(DIMBOSSTONSIL_STEAM_MUSIC, 1);
            break;
        case DIMBOSSTONSIL_ANIM_EVENT_ENABLE_AREA:
            (*gMapEventInterface)->setObjGroupStatus(DIMBOSSTONSIL_MAP_DIR, DIMBOSSTONSIL_MAP_AREA, 1);
            break;
        case DIMBOSSTONSIL_ANIM_EVENT_DISABLE_AREA:
            (*gMapEventInterface)->setObjGroupStatus(DIMBOSSTONSIL_MAP_DIR, DIMBOSSTONSIL_MAP_AREA, 0);
            break;
        case DIMBOSSTONSIL_ANIM_EVENT_ENABLE_LIGHT:
            if (gDIMbosstonsilLight != NULL)
            {
                modelLightStruct_setEnabled(gDIMbosstonsilLight, 1, 1.0f);
            }
            break;
        case DIMBOSSTONSIL_ANIM_EVENT_DISABLE_LIGHT:
            if (gDIMbosstonsilLight != NULL)
            {
                modelLightStruct_setEnabled(gDIMbosstonsilLight, 0, 1.0f);
            }
            break;
        }
    }

    if (lbl_803DDBA0 >= lbl_803DDB9C)
    {
        Sfx_PlayFromObject((u32)obj, DIMBOSSTONSIL_RUMBLE_SFX);
        lbl_803DDB9C += 100.0f;
        doRumble(8.0f);
    }
    lbl_803DDBA0 += timeDelta;

    if ((obj)->seqIndex != -1)
    {
        animOk = (*gBaddieControlInterface)->isObjectValid((GameObject*)obj, state, 1);
        if (animOk == 0)
        {
            return 1;
        }
        if ((state->eventGameBit != -1) && (mainGetBit(state->eventGameBit) != 0))
        {
            (*gObjectTriggerInterface)->yield((ObjSeqState*)animUpdate, config->eventId);
            state->eventGameBit = -1;
        }

        hitReactMode = state->hitReactMode;
        switch (hitReactMode)
        {
        case 2:
            animUpdate->hitVolumePair = 0;
            dimBossTonsil_newState_hitFightMain(obj, animUpdate, state, state);
            if (state->hitReactMode == 1)
            {
                state->field270 = 0;
                (*gPlayerInterface)
                    ->update(obj, state, 1.0f, 1.0f, &gDIMbosstonsilStateHandlers, &gDIMbosstonsilSubstateHandlers);
                animUpdate->sequenceEventActive = 0;
            }
            break;
        case 1:
            animOk = (*gBaddieControlInterface)
                         ->updateSequenceMovement((GameObject*)obj, (ObjSeqState*)animUpdate, (char*)state,
                                                  &gDIMbosstonsilStateHandlers, &gDIMbosstonsilSubstateHandlers, 0);
            if (animOk != 0)
            {
                (*gBaddieControlInterface)
                    ->updateGravity((GameObject*)obj, state, 0.0f, 1);
            }
            break;
        case 0:
        default:
            animUpdate->hitVolumePair = -1;
            animUpdate->hitVolumePair &= ~0x40;
            break;
        }
    }

    if ((obj)->seqIndex == -1)
    {
        state->stateFlags |= DIMBOSSTONSIL_STATE_FLAG_START_MOVE;
        return 0;
    }

    return 0;
}

void DIMbosstonsil_func0B(void)
{
}

int DIMbosstonsil_setScale(GameObject* obj)
{
    return ((DIMbosstonsilState*)obj->extra)->scale;
}

int DIMbosstonsil_getExtraSize(void)
{
    return DIMBOSSTONSIL_STATE_SIZE;
}

int DIMbosstonsil_getObjectTypeId(void)
{
    return DIMBOSSTONSIL_OBJECT_TYPE;
}

void DIMbosstonsil_free(GameObject* obj)
{
    DIMbosstonsilState* state;

    state = (obj)->extra;
    ObjGroup_RemoveObject((int)obj, DIMBOSSTONSIL_OBJGROUP);
    (*gBaddieControlInterface)->releaseState((GameObject*)obj, state, 1);
    if (gDIMbosstonsilLight != NULL)
    {
        ModelLightStruct_free(gDIMbosstonsilLight);
    }
}

void DIMbosstonsil_render(GameObject* obj, u32 p2, u32 p3, u32 p4, u32 p5, char visible)
{
    struct
    {
        f32 x;
        f32 y;
        f32 z;
    } pathPoint;
    int partfxArgs[3];
    f32* pp;

    if (visible != 0)
    {
        switch ((obj)->userData1)
        {
        case 0:
        {
            objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, (double)1.0f);

            ObjPath_GetPointWorldPosition(obj, 1, (pp = &pathPoint.x), &pathPoint.y, &pathPoint.z, 0);
            (*gPartfxInterface)->spawnObject(obj, DIMBOSSTONSIL_PARTFX, partfxArgs, 0x200001, -1, NULL);

            ObjPath_GetPointWorldPosition(obj, 0, pp, &pathPoint.y, &pathPoint.z, 0);
            (*gPartfxInterface)->spawnObject(obj, DIMBOSSTONSIL_PARTFX, partfxArgs, 0x200001, -1, NULL);

            if (gDIMbosstonsilLight != 0 && gDIMbosstonsilLight->glowType != 0 && gDIMbosstonsilLight->enabled != 0)
            {
                modelLightStruct_setPosition(gDIMbosstonsilLight, pathPoint.x, pathPoint.y, pathPoint.z);
                queueGlowRender(gDIMbosstonsilLight);
            }
            break;
        }
        }
    }
}

void DIMbosstonsil_hitDetect(GameObject* obj)
{
    (*gPlayerInterface)->updateVelocityState(obj, (obj)->extra, &gDIMbosstonsilStateHandlers);
}

void DIMbosstonsil_update(GameObject* obj)
{
    DIMbosstonsilState* state;
    DIMbosstonsilConfig* config;
    u8 red, green, blue, alpha;

    state = (obj)->extra;
    config = (DIMbosstonsilConfig*)(obj)->anim.placementData;

    if ((obj)->userData1 != 0)
        return;

    if ((obj)->userData2 == 0)
    {
        (obj)->anim.localPosX = config->spawnX;
        (obj)->anim.localPosY = config->spawnY;
        (obj)->anim.localPosZ = config->spawnZ;
        (*gObjectTriggerInterface)->runSequence((int)config->animObjId, obj, -1);
        (obj)->userData2 = 1;
        return;
    }

    if ((state->stateFlags & DIMBOSSTONSIL_STATE_FLAG_START_MOVE) != 0)
    {
        lbl_803DDBA4 = 780.0f;
        (*gBaddieControlInterface)
            ->startHitReaction(obj, state, state->animPoints, state->animFrame, &state->hitReactMode, 0, 0, 0, 1);
        state->stateFlags &= ~DIMBOSSTONSIL_STATE_FLAG_START_MOVE;
    }

    if ((*gBaddieControlInterface)->isObjectValid(obj, state, 1) == 0)
        return;

    state->targetObject = Obj_GetPlayerObject();
    dimBossTonsil_newState_hitFightMain(obj, NULL, state, state);

    if (gDIMbosstonsilLight == 0)
        return;

    modelLightStruct_getSpecularColor((ModelLightStruct*)gDIMbosstonsilLight, &red, &green, &blue, &alpha);
    modelLightStruct_setGlowColor((ModelLightStruct*)gDIMbosstonsilLight, red, green, blue, 0xc0);

    if (gDIMbosstonsilLight->glowType == 0)
        return;
    if (gDIMbosstonsilLight->enabled == 0)
        return;

    {
        s16 r30_local;
        int sum;
        sum = gDIMbosstonsilLight->glowAlpha + gDIMbosstonsilLight->glowAlphaStep;
        r30_local = sum;
        if (r30_local < 0)
        {
            r30_local = 0;
            gDIMbosstonsilLight->glowAlphaStep = 0;
        }
        else if (r30_local > 0xc)
        {
            int rnd = randomGetRange(-0xc, 0xc);
            r30_local = (s16)(r30_local + rnd);
            if (r30_local > 0xff)
            {
                r30_local = 0xff;
                gDIMbosstonsilLight->glowAlphaStep = 0;
            }
        }
        gDIMbosstonsilLight->glowAlpha = r30_local;
    }
}

void DIMbosstonsil_init(GameObject* obj, u32 def, int isAltVariant)
{
    u8 variant;
    int state;

    state = *(int*)&obj->extra;
    variant = 6;
    if (isAltVariant != 0)
    {
        variant = variant | 1;
    }
    (*gBaddieControlInterface)
        ->initGroundBaddie(obj, (u8*)def, (u8*)state, 2, 2, 0x102, variant, 20.0f);
    obj->animEventCallback = DIMbosstonsil_SeqFn;
    (*gPlayerInterface)->setState(obj, (void*)state, 0);
    ((BaddieState*)state)->substate = 0;
    gDIMbosstonsilRoutePhase = mainGetBit(0x20c);
    if (gDIMbosstonsilRoutePhase < 3)
    {
        *(s8*)(state + DIMBOSSTONSIL_HEALTH_PHASE_OFFSET) = 3 - gDIMbosstonsilRoutePhase;
    }
    else
    {
        *(s8*)(state + DIMBOSSTONSIL_HEALTH_PHASE_OFFSET) = 7 - gDIMbosstonsilRoutePhase;
    }
    lbl_803DDBA4 = 0.0f;
    lbl_803DDBA0 = 0.0f;
    lbl_803DDB98 = 0.0f;
    lbl_803DDB9C = 30.0f;
    gDIMbosstonsilLight = objCreateLight(0, 1);
    if (gDIMbosstonsilLight != 0)
    {
        modelLightStruct_setLightKind(gDIMbosstonsilLight, MODEL_LIGHT_KIND_POINT);
        modelLightStruct_setDiffuseColor(gDIMbosstonsilLight, 0xff, 0, 0, 0x7f);
        modelLightStruct_setSpecularColor(gDIMbosstonsilLight, 0xff, 0, 0, 0x7f);
        modelLightStruct_setDistanceAttenuation(gDIMbosstonsilLight, 30.0f, 50.0f);
        lightSetField4D((ModelLightStruct*)gDIMbosstonsilLight, 1);
        modelLightStruct_setEnabled(gDIMbosstonsilLight, 1, 0.0f);
        modelLightStruct_setGlowProjectionRadius((ModelLightStruct*)gDIMbosstonsilLight, 50.0f);
        modelLightStruct_setDiffuseTargetColor(gDIMbosstonsilLight, 0xff, 0x7f, 0, 0x40);
        modelLightStruct_setSpecularTargetColor((ModelLightStruct*)gDIMbosstonsilLight, 0xff, 0x7f, 0, 0x40);
        modelLightStruct_startColorFade(gDIMbosstonsilLight, 2, 0x3c);
        modelLightStruct_setAffectsAabbLightSelection((ModelLightStruct*)gDIMbosstonsilLight, 1);
        modelLightStruct_setupGlow(gDIMbosstonsilLight, 0, 0xff, 0, 0, 0x7f, 50.0f);
    }
    return;
}

void DIMbosstonsil_release(void)
{
}

void DIMbosstonsil_initialise(void)
{
    gDIMbosstonsilStateHandlers.startIdle = DIMbosstonsil_startIdleHitReaction;
    gDIMbosstonsilStateHandlers.choose = DIMbosstonsil_chooseHitReaction;
    gDIMbosstonsilSubstateHandlers.enable = DIMbosstonsil_enableHitReaction;
    gDIMbosstonsilSubstateHandlers.update = DIMbosstonsil_updateHitReaction;
}

ObjectDescriptor12 gDIM_BossTonsilObjDescriptor = {
    0, 0, 0, OBJECT_DESCRIPTOR_FLAGS_12_SLOTS,
    (ObjectDescriptorCallback)DIMbosstonsil_initialise,
    (ObjectDescriptorCallback)DIMbosstonsil_release,
    0,
    (ObjectDescriptorCallback)DIMbosstonsil_init,
    (ObjectDescriptorCallback)DIMbosstonsil_update,
    (ObjectDescriptorCallback)DIMbosstonsil_hitDetect,
    (ObjectDescriptorCallback)DIMbosstonsil_render,
    (ObjectDescriptorCallback)DIMbosstonsil_free,
    (ObjectDescriptorCallback)DIMbosstonsil_getObjectTypeId,
    DIMbosstonsil_getExtraSize,
    (ObjectDescriptorCallback)DIMbosstonsil_setScale,
    (ObjectDescriptorCallback)DIMbosstonsil_func0B,
};
