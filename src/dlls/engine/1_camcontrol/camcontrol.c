#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "dolphin/mtx/vec.h"
#include "dolphin/os.h"
#include "main/dll/dll_BB.h"
#include "main/dll/dll_0042_unk.h"
#include "main/dll/dll_0044_cameramodeviewfinder.h"
#include "main/dll/dll_0048_cameramodestatic.h"
#include "main/dll/dll_02C0_front_api.h"
#include "main/dll/savegame.h"
#include "main/dll/dll_00C9_enemy.h"
#include "main/mm.h"
#include "main/object_transform.h"
#include "main/obj_query.h"
#include "main/pad.h"
#include "main/voxmaps.h"
#include "string.h"
#include "dlls/objects/261_LargeCrate.h"
#include "main/dll/objfx.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/frame_timing.h"
#include "main/resource.h"
#include "main/dll/dll_0019_dll19func0.h"
#include "main/dll/baddie_control_interface.h"
#include "main/dll/dll_8B.h"
#include "main/dll/dll_B2.h"
#include "main/dll/dll_B3.h"
#include "main/dll/dll_B4.h"
#include "main/dll/dll_B7.h"
#include "main/dll/dll_B8.h"
#include "main/dll/dll_BC.h"
#include "main/dll/player_api.h"
#include "main/dll/tricky_api.h"
#include "main/camera.h"
#include "main/model.h"
#include "main/model_light.h"
#include "main/obj_list.h"
#include "main/object_render.h"
#include "main/rcp_dolphin_api.h"
#include "main/shader_map_api.h"
#include "main/vecmath.h"
#include "sys/objects.h"
#include "sys/objects/lifecycle.h"
#include "track/intersect_api.h"
#include "track/intersect_depth_state_api.h"

typedef struct CamcontrolIconRenderOp
{
    u8 pad00[0x24];
    s32 textureId;
    u8 pad28;
    u8 variantId;
} CamcontrolIconRenderOp;

typedef struct CamcontrolLockIconRenderOp
{
    u8 pad00[0x24];
    s32 textureId;
    u8 pad28;
    u8 distanceTier;
} CamcontrolLockIconRenderOp;

#define RETICLE_BANK_LOCKON  0
#define RETICLE_BANK_DEFAULT 1
#define RETICLE_BANK_CONTEXT 2

#define ICON_VARIANT_PRESS_A        1
#define LOCK_ICON_DIM_ALPHA_SCALE   0x60
#define CAMCONTROL_LOCK_ICON_OBJ    0x1FE
#define CAMCONTROL_OBJFLAG_RENDERED 0x800
#define CAMCONTROL_OBJFLAG_FREED    0x40

s16 gCamcontrolTargetHelpTextId = -1;
u16 gCamcontrolTargetClassMask = 0xFFFF;
char sDllBBTimeDebugFormat[] = "t=%f\n";

#define CAMCONTROL_RETICLE_ROOT_MOTION_SCALE   0.4f
#define CAMCONTROL_NORMALIZED_MAX              1.0f
#define CAMCONTROL_NORMALIZED_MIN              0.0f
#define CAMCONTROL_TARGET_DISTANCE_TIER1       0.25f
#define CAMCONTROL_TARGET_DISTANCE_TIER2       0.5f
#define CAMCONTROL_TARGET_DISTANCE_TIER3       0.75f
#define CAMCONTROL_RETICLE_LIGHT_DIR_Z         -0.78f
#define CAMCONTROL_FOCUS_DELTA_MIN             -100.0f
#define CAMCONTROL_FOCUS_DELTA_MAX             20.0f
#define CAMCONTROL_FOCUS_MOVE_AVERAGE_DAMPING  0.2f
#define CAMCONTROL_BLEND_INTERPOLATE_RATE      0.22f
#define CAMCONTROL_BLEND_MAX_RATE              3.0f
#define CAMCONTROL_RETICLE_FADE_OUT_STEP       -0.04f
#define CAMCONTROL_RETICLE_FADE_IN_STEP        0.04f
#define CAMCONTROL_RETICLE_ALPHA_SCALE         255.0f
#define CAMCONTROL_RETICLE_SPIN_STEP_PER_FRAME 1024.0f
#define CAMCONTROL_MIN_TARGET_DISTANCE         5.0f
#define CAMCONTROL_DEFAULT_FOV_Y               60.0f

u8* pCamera;
u8 gCamcontrolHandlerCount;
CamcontrolHandlerEntry* gCamcontrolCurrentHandler;
s32 gCamcontrolActiveActionId;
int gCamcontrolCurrentHandlerIndex;
s32 gCamcontrolQueuedActionId;
int gCamcontrolActiveActionPriority;
int gCamcontrolActiveActionStartFlags;
void* gCamcontrolQueuedActionData;
u8 gCamcontrolQueuedActionPending;
s8 gCamcontrolQueuedActionStartFlags;
s8 gCamcontrolQueuedActionPriority;
s32 gCamcontrolQueuedActionBlendFrames;
u8 gCamcontrolQueuedActionMode;
int gCamcontrolSavedActionId;
int gCamcontrolSavedActionPriority;
int gCamcontrolSavedActionStartFlags;
f32 gCamcontrolSavedFocusLocalX;
f32 gCamcontrolSavedFocusLocalY;
f32 gCamcontrolSavedFocusLocalZ;
f32 gCamcontrolSavedFocusWorldX;
f32 gCamcontrolSavedFocusWorldY;
f32 gCamcontrolSavedFocusWorldZ;
f32 lbl_803DD4D0;
u32 lbl_803DD4CC;
s8 lbl_803DD4CB;
s8 gCamcontrolTargetState;
u16 gCamcontrolReticleSpin;
void* lbl_803DD4C4;
s16 lbl_803DD4C0;
CamcontrolReticleObject* gCamcontrolTargetReticle;
s8 gCamcontrolTargetChanged;

u8 gCamcontrolStateStorage[0x148];
CamcontrolHandlerEntry* gCamcontrolHandlerEntries[20];

void* lbl_80319A88[35] = {(void*)0x00000000,
                          (void*)0x00000000,
                          (void*)0x00000000,
                          (void*)0x001d0000,
                          Camera_initialise,
                          Camera_release,
                          (void*)0x00000000,
                          Camera_init,
                          Camera_update,
                          Camera_get,
                          Camera_getMode,
                          Camera_GetFollowPos,
                          Camera_getDefaultHandlerEntry,
                          Camera_setMode,
                          Camera_getCamActionsBinEntry,
                          camcontrol_loadTriggeredCamAction,
                          Camera_setFocus,
                          Camera_overridePos,
                          Camera_moveBy,
                          camcontrol_initialise,
                          camcontrol_getRelativePosition,
                          Camera_getOverrideTarget,
                          Camera_getTarget,
                          Camera_setTargetFlag2,
                          Camera_setTarget,
                          Camera_setTargetReticleOverride,
                          Camera_isZooming,
                          camcontrol_updateTargetFeedback,
                          Camera_minimapShowHelpTextForTarget,
                          Camera_setLetterbox,
                          camcontrol_release,
                          Camera_getMinimapInfoText,
                          Camera_applyFrameFlags,
                          Camera_applyTargetFlags,
                          camcontrol_queueSavedAction};

int cameraGetTargetType(void)
{
    return CAMCONTROL_CAMERA->targetKind;
}

int Camera_getMinimapInfoText(void)
{
    return gCamcontrolTargetHelpTextId;
}

void camcontrol_updateTargetReticle(CamcontrolTargetObject* fallbackTarget, int unused2, u32 renderArg2,
                                    u32 renderArg3, u32 renderArg4, u32 renderArg5)
{
    int savedReticleState;
    u8 savedReticleAlpha;
    GameObject* reticle;
    GameObject* targetObject;
    CamcontrolTargetObject* target;
    ObjHitVolumeRuntimeTransform* slot;
    ObjAnimBank* activeBank;
    u8 idx;
    int bank;
    int paletteIdx;

    reticle = (GameObject*)gCamcontrolTargetReticle;
    target = fallbackTarget;
    if ((u32)CAMCONTROL_CAMERA->targetReticleOverride != 0)
    {
        target = (CamcontrolTargetObject*)CAMCONTROL_CAMERA->targetReticleOverride;
        savedReticleState = gCamcontrolTargetState;
        gCamcontrolTargetState = CAMCONTROL_TARGET_RETICLE_STATE_ACTIVE;
        savedReticleAlpha = reticle->anim.alpha;
        reticle->anim.alpha = 0xFF;
    }

    if (target != NULL)
    {
        targetObject = (GameObject*)target;
        if (targetObject->anim.hitVolumeTransforms == NULL)
            return;

        idx = target->targetSetupIndex;
        slot = &targetObject->anim.hitVolumeTransforms[idx];

        switch (targetObject->anim.hitVolumeBounds[idx].flags & CAMCONTROL_TARGET_KIND_MASK)
        {
        case CAMCONTROL_TARGET_KIND_LOCKON:
            bank = RETICLE_BANK_LOCKON;
            break;
        case CAMCONTROL_TARGET_KIND_CONTEXT_A:
        case CAMCONTROL_TARGET_KIND_CONTEXT_B:
            bank = RETICLE_BANK_CONTEXT;
            break;
        default:
            bank = RETICLE_BANK_DEFAULT;
            break;
        }

        paletteIdx = target->targetPaletteIndex;
        if (paletteIdx >= 4)
            paletteIdx = 0;
        gCamcontrolTargetHelpTextId = targetObject->anim.modelInstance->helpTextIds[paletteIdx];

        reticle->anim.worldPosX = slot->jointX;
        reticle->anim.worldPosY = slot->jointY;
        reticle->anim.worldPosZ = slot->jointZ;
        reticle->anim.bankIndex = bank;

        reticle->anim.parent = targetObject->anim.parent;
        if (reticle->anim.parent != NULL)
        {
            Obj_TransformWorldPointToLocal(reticle->anim.worldPosX, reticle->anim.worldPosY, reticle->anim.worldPosZ,
                                           &reticle->anim.localPosX, &reticle->anim.localPosY, &reticle->anim.localPosZ,
                                           (u32)reticle->anim.parent);
        }
        else
        {
            reticle->anim.localPosX = reticle->anim.worldPosX;
            reticle->anim.localPosY = reticle->anim.worldPosY;
            reticle->anim.localPosZ = reticle->anim.worldPosZ;
        }
        reticle->anim.rotY = 0;
        reticle->anim.rotZ = 0;
        reticle->anim.rootMotionScale = CAMCONTROL_RETICLE_ROOT_MOTION_SCALE;
        ((u8*)reticle)[0x37] = reticle->anim.alpha;
        objRenderModelAndHitVolumes(reticle, renderArg2, renderArg3, renderArg4, renderArg5, CAMCONTROL_NORMALIZED_MAX);
    }
    else
    {
        reticle->anim.parent = NULL;
    }

    activeBank = reticle->anim.banks[reticle->anim.bankIndex];
    *(u16*)((u8*)activeBank + 0x18) = (u16)(*(u16*)((u8*)activeBank + 0x18) & ~8);

    if ((u32)CAMCONTROL_CAMERA->targetReticleOverride != 0)
    {
        gCamcontrolTargetState = savedReticleState;
        reticle->anim.alpha = savedReticleAlpha;
    }
}

int aButtonIconTexCb(GameObject* obj, void** objPtr, u32 renderOpIdx)
{
    CamcontrolIconRenderOp* renderOp;
    GXColor color; /* r/g/b intentionally left unset: callee reads only alpha for this op */

    renderOp = (CamcontrolIconRenderOp*)ObjModel_GetRenderOp((ModelFileHeader*)*objPtr, renderOpIdx);
    Rcp_ResetTextureStageState();
    if (renderOp->variantId == ICON_VARIANT_PRESS_A)
    {
        if ((CAMCONTROL_CAMERA->targetFlags & CAMCONTROL_CAMERA_TARGET_FLAG_ACCEPTS_INPUT) == 0)
        {
            color.a = 0;
        }
        else
        {
            color.a = obj->anim.alpha;
        }
    }
    else
    {
        color.a = obj->anim.alpha;
    }
    if (CAMCONTROL_CAMERA->targetKind == CAMCONTROL_TARGET_KIND_SUPPRESSED)
    {
        color.a = 0;
    }
    addTexLayerStageKAlpha(textureIdxToPtr(renderOp->textureId), NULL, 0, &color);
    Rcp_ApplyTextureStageCounts();
    if (color.a < 0xff)
    {
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        gxSetZMode_(1, GX_LEQUAL, 0);
    }
    else
    {
        GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
        gxSetZMode_(1, GX_LEQUAL, 1);
    }
    gxSetPeControl_ZCompLoc_(1);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetCullMode(GX_CULL_BACK);
    return 1;
}

int lockIconTexCb(GameObject* obj, int* modelPtr, int renderOpIdx)
{
    CamcontrolLockIconRenderOp* renderOp;
    u8 tier;
    GXColor color;
    f32 dist;
    int alphaVal;

    renderOp = (CamcontrolLockIconRenderOp*)ObjModel_GetRenderOp((ModelFileHeader*)*modelPtr, renderOpIdx);
    dist = CAMCONTROL_CAMERA->targetDistance;
    if (dist <= CAMCONTROL_NORMALIZED_MIN)
    {
        tier = 4;
    }
    else if (dist <= CAMCONTROL_TARGET_DISTANCE_TIER1)
    {
        tier = 3;
    }
    else if (dist <= CAMCONTROL_TARGET_DISTANCE_TIER2)
    {
        tier = 2;
    }
    else if (dist <= CAMCONTROL_TARGET_DISTANCE_TIER3)
    {
        tier = 1;
    }
    else
    {
        tier = 0;
    }
    Rcp_ResetTextureStageState();
    if (renderOp->distanceTier <= tier)
    {
        color.r = 0;
        color.g = 0;
        color.b = 0;
        alphaVal = ((obj->anim.alpha + 1) * LOCK_ICON_DIM_ALPHA_SCALE) >> 8;
        color.a = alphaVal;
        addTexLayerStageKAlpha(textureIdxToPtr(renderOp->textureId), NULL, 0, &color);
    }
    else
    {
        color.r = 0xff;
        color.g = 0xff;
        color.b = 0xff;
        color.a = obj->anim.alpha;
        addTexLayerStageKAlpha(textureIdxToPtr(renderOp->textureId), NULL, 0, &color);
    }
    Rcp_ApplyTextureStageCounts();
    if (obj->anim.alpha < 0xff || renderOp->distanceTier <= tier)
    {
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        gxSetZMode_(1, GX_LEQUAL, 0);
    }
    else
    {
        GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
        gxSetZMode_(1, GX_LEQUAL, 1);
    }
    gxSetPeControl_ZCompLoc_(1);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetCullMode(GX_CULL_BACK);
    return 1;
}

void lockIconInit(void)
{
    if (gCamcontrolTargetReticle == NULL)
    {
        gCamcontrolTargetReticle = (CamcontrolReticleObject*)Obj_SetupObject(
            Obj_AllocObjectSetup(0x18, CAMCONTROL_LOCK_ICON_OBJ), 4, -1, -1, NULL);
        ObjModel_SetRenderCallback((u8*)Obj_GetActiveModel((GameObject*)gCamcontrolTargetReticle), lockIconTexCb);
        gCamcontrolTargetReticle->anim.bankIndex = CAMCONTROL_RETICLE_ICON_LOCKON;
        ObjModel_SetRenderCallback((u8*)Obj_GetActiveModel((GameObject*)gCamcontrolTargetReticle), aButtonIconTexCb);
        gCamcontrolTargetReticle->anim.bankIndex = CAMCONTROL_RETICLE_ICON_A_BUTTON;
        ObjModel_SetRenderCallback((u8*)Obj_GetActiveModel((GameObject*)gCamcontrolTargetReticle), aButtonIconTexCb);
        lightSetColor(1, 0x32, 0x3C, 0x28);
        lbl_803DD4C4 = objCreateLight(NULL, 1);
        if (lbl_803DD4C4 != NULL)
        {
            modelLightStruct_setLightKind(lbl_803DD4C4, MODEL_LIGHT_KIND_DIRECTIONAL);
            modelLightStruct_setObjectLightMaskIndex(lbl_803DD4C4, 1);
            objSetEventName(lbl_803DD4C4, 1);
            modelLightStruct_setDirection(lbl_803DD4C4, CAMCONTROL_NORMALIZED_MAX, CAMCONTROL_NORMALIZED_MIN,
                                          CAMCONTROL_RETICLE_LIGHT_DIR_Z);
            modelLightStruct_setDiffuseColor(lbl_803DD4C4, 0xB4, 0xC8, 0xFF, 0xFF);
        }
    }
}

static inline int camcontrol_isTargetCandidate(GameObject* obj, ObjHitVolumeRuntimeBounds* data)
{
    int accept;
    if (data != NULL && obj->anim.alpha == 0xff && !(*(u8*)&obj->anim.resetHitboxMode & 0x28) &&
        ((obj->objectFlags & CAMCONTROL_OBJFLAG_RENDERED) || (obj->anim.modelInstance->flags & 1)) &&
        !(obj->anim.flags & OBJANIM_FLAG_HIDDEN) && !(obj->objectFlags & CAMCONTROL_OBJFLAG_FREED) &&
        (gCamcontrolTargetClassMask &
         ((accept = 1) << (data[obj->hitVolumeIndex].flags & CAMCONTROL_TARGET_KIND_MASK))))
    {
        return accept;
    }
    return 0;
}

CamcontrolTargetObject* camcontrol_findBestTarget(CamcontrolCameraState* cameraState, ObjAnimComponent* focus)
{
    int objIndex;
    int objCount;
    u8 occOut[4];
    f32 worldFrom[3];
    f32 worldTo[3];
    int gridFrom[3];
    int gridTo[3];
    int traceOut[3];
    GameObject* targets[8];
    f32 dist[8];
    GameObject** ptr;
    int bestPri;
    GameObject* obj;
    int idx;
    int count;
    GameObject* player;
    u8 canTarget;
    ObjHitVolumeRuntimeBounds* data;
    ObjHitVolumeRuntimeBounds* entry;
    ObjDefHitVolume* row;
    GameObject* best;
    int i;
    int k;
    int accept;
    f32 dx, dz, dy, distsq, range;
    f32* pDist;
    GameObject** pTarget;

    (void)cameraState;
    bestPri = -1;
    count = 0;
    player = Obj_GetPlayerObject();
    if (player == NULL || focus == NULL || gCamcontrolActiveActionId == 0x44 || objAnimFn_80296328(player) == 0)
    {
        return NULL;
    }
    ptr = (GameObject**)ObjList_GetObjects(&objIndex, &objCount);
    idx = objIndex;
    ptr += idx;
    for (; idx < objCount; ptr++, idx++)
    {
        obj = *ptr;
        data = obj->anim.hitVolumeBounds;
        accept = camcontrol_isTargetCandidate(obj, data);
        if (accept == 0)
        {
            continue;
        }
        if ((int)*(u8*)&obj->anim.modelInstance->hitVolumes[obj->hitVolumeIndex].priority < bestPri)
        {
            continue;
        }
        if ((*(u8*)&obj->anim.resetHitboxMode & 0x80) || (data[obj->hitVolumeIndex].flags & 0x80))
        {
            dy = CAMCONTROL_NORMALIZED_MIN;
        }
        else
        {
            dy = focus->worldPosY - obj->anim.hitVolumeTransforms[obj->hitVolumeIndex].centerY;
        }
        if (!(dy > CAMCONTROL_FOCUS_DELTA_MIN))
        {
            continue;
        }
        if (!(dy < CAMCONTROL_FOCUS_DELTA_MAX))
        {
            continue;
        }
        dx = focus->worldPosX - obj->anim.hitVolumeTransforms[obj->hitVolumeIndex].centerX;
        dz = focus->worldPosZ - obj->anim.hitVolumeTransforms[obj->hitVolumeIndex].centerZ;
        distsq = dx * dx + dz * dz;
        entry = &data[obj->hitVolumeIndex];
        range = (f32)(int)(entry->bounds[2] << 2);
        if (!(distsq < range * range))
        {
            continue;
        }
        canTarget = 1;
        if ((entry->flags & CAMCONTROL_TARGET_KIND_MASK) == CAMCONTROL_TARGET_KIND_A_BUTTON_HINT &&
            playerIsTargetSuppressed((GameObject*)(player)) != 0)
        {
            canTarget = 0;
        }
        if (canTarget == 0)
        {
            continue;
        }
        bestPri = *(u8*)&obj->anim.modelInstance->hitVolumes[obj->hitVolumeIndex].priority;
        i = 0;
        while (i < count &&
               (int)*(u8*)&targets[i]->anim.modelInstance->hitVolumes[targets[i]->hitVolumeIndex].priority > bestPri)
        {
            i++;
        }
        while (i < count && dist[i] < distsq &&
               bestPri == (int)*(u8*)&targets[i]->anim.modelInstance->hitVolumes[targets[i]->hitVolumeIndex].priority)
        {
            i++;
        }
        for (k = count; k > i; k--)
        {
            dist[k] = dist[k - 1];
            targets[k] = targets[k - 1];
        }
        dist[i] = distsq;
        targets[i] = obj;
        count++;
        if (count == 8)
        {
            break;
        }
    }
    if (count > 0)
    {
        best = targets[0];
        row = best->anim.modelInstance->hitVolumes;
        row += best->hitVolumeIndex;
        if (row->flags & 0x20)
        {
            worldFrom[0] = focus->worldPosX;
            worldFrom[1] = CAMCONTROL_FOCUS_DELTA_MAX + focus->worldPosY;
            worldFrom[2] = focus->worldPosZ;
            worldTo[0] = best->anim.hitVolumeTransforms[best->hitVolumeIndex].jointX;
            worldTo[1] = best->anim.hitVolumeTransforms[best->hitVolumeIndex].jointY;
            worldTo[2] = best->anim.hitVolumeTransforms[best->hitVolumeIndex].jointZ;
            voxmaps_worldToGrid(worldFrom, (s16*)gridFrom);
            voxmaps_worldToGrid(worldTo, (s16*)gridTo);
            if ((u8)voxmaps_traceLine((VoxPos*)gridFrom, (VoxPos*)gridTo, (VoxPos*)traceOut, occOut, 0) == 0 &&
                occOut[0] != 1)
            {
                return NULL;
            }
        }
        return (CamcontrolTargetObject*)targets[0];
    }
    return NULL;
}

void camcontrol_updateMoveAverage(CamcontrolCameraState* cameraState, ObjAnimComponent* focus)
{
    Vec3f* velocity;
    f32 mag;
    f32 root;
    f32 minMove;
    f32 average;
    f32 move0;
    f32 move1;
    f32 move2;
    f32 move3;
    f32 move4;

    move1 = cameraState->focusMoveHistory[1];
    cameraState->focusMoveHistory[0] = move1;
    move2 = cameraState->focusMoveHistory[2];
    cameraState->focusMoveHistory[1] = move2;
    move3 = cameraState->focusMoveHistory[3];
    cameraState->focusMoveHistory[2] = move3;
    move4 = cameraState->focusMoveHistory[4];
    cameraState->focusMoveHistory[3] = move4;
    velocity = &focus->velocity;
    mag = PSVECMag(velocity);
    if (mag > CAMCONTROL_NORMALIZED_MIN)
    {
        root = sqrtf(mag);
        mag = root;
    }
    cameraState->focusMoveHistory[4] = mag;
    minMove = CAMCONTROL_NORMALIZED_MIN;
    cameraState->focusMoveAverage = minMove;
    move0 = cameraState->focusMoveHistory[0];
    cameraState->focusMoveAverage += move0;
    move1 = cameraState->focusMoveHistory[1];
    cameraState->focusMoveAverage += move1;
    move2 = cameraState->focusMoveHistory[2];
    cameraState->focusMoveAverage += move2;
    move3 = cameraState->focusMoveHistory[3];
    cameraState->focusMoveAverage += move3;
    move4 = cameraState->focusMoveHistory[4];
    cameraState->focusMoveAverage += move4;
    cameraState->focusMoveAverage *= CAMCONTROL_FOCUS_MOVE_AVERAGE_DAMPING;
    average = cameraState->focusMoveAverage;
    if (average < minMove)
    {
        cameraState->focusMoveAverage = -average;
    }
}


static inline int camcontrol_findHandlerIndex(u16 actionId)
{
    int handlerCount;
    register CamcontrolHandlerEntry** handlerEntry;
    int handlerIndex;

    handlerIndex = 0;
    handlerEntry = gCamcontrolHandlerEntries;
    for (handlerCount = gCamcontrolHandlerCount; 0 < handlerCount; handlerCount--)
    {
        if (actionId == (*handlerEntry)->actionId)
        {
            return handlerIndex;
        }
        handlerEntry++;
        handlerIndex++;
    }
    return -1;
}

void camcontrol_activateHandler(u16 actionId, void* actionData)
{
    CamcontrolHandlerEntry* entry;
    int idx;
    int n;
    int priority;

    if (gCamcontrolCurrentHandler != NULL)
    {
        if (gCamcontrolActiveActionId != actionId)
        {
            gCamcontrolCurrentHandler->handler->vtable->release(pCamera);
            if (gCamcontrolCurrentHandler->priority == CAMCONTROL_HANDLER_PRIORITY_DYNAMIC)
            {
                idx = gCamcontrolCurrentHandlerIndex;
                Resource_Release(gCamcontrolHandlerEntries[idx]->handler);
                mm_free(gCamcontrolHandlerEntries[idx]);
                gCamcontrolHandlerEntries[idx] = gCamcontrolHandlerEntries[gCamcontrolHandlerCount - 1];
                gCamcontrolHandlerCount--;
                gCamcontrolCurrentHandler = NULL;
                gCamcontrolActiveActionId = -1;
                gCamcontrolCurrentHandlerIndex = -1;
            }
        }
    }

    idx = camcontrol_findHandlerIndex(actionId);
    gCamcontrolCurrentHandlerIndex = idx;

    if (idx == -1)
    {
        CamcontrolHandlerEntry* new_entry;
        priority = gCamcontrolQueuedActionPriority;
        new_entry = mmAlloc(CAMCONTROL_HANDLER_ENTRY_SIZE, CAMCONTROL_ACTION_HEAP, 0);
        n = gCamcontrolHandlerCount;
        gCamcontrolHandlerEntries[n] = new_entry;
        gCamcontrolHandlerCount++;
        entry = gCamcontrolHandlerEntries[n];
        entry->actionId = actionId;
        entry->priority = priority;
        entry->handler = Resource_Acquire(actionId, CAMCONTROL_HANDLER_RESOURCE_TYPE);
        gCamcontrolCurrentHandlerIndex = gCamcontrolHandlerCount - 1;
    }

    if (gCamcontrolCurrentHandlerIndex != -1)
    {
        entry = gCamcontrolHandlerEntries[gCamcontrolCurrentHandlerIndex];
        gCamcontrolCurrentHandler = entry;
        gCamcontrolActiveActionId = entry->actionId;
        entry->handler->vtable->activate(pCamera, gCamcontrolQueuedActionStartFlags, actionData);
    }
    else
    {
        gCamcontrolCurrentHandler = NULL;
        gCamcontrolActiveActionId = -1;
    }

    gCamcontrolActiveActionPriority = gCamcontrolQueuedActionPriority;
    gCamcontrolActiveActionStartFlags = gCamcontrolQueuedActionStartFlags;
}

void firstPersonZoomOutOnExit(u8 blendFrames, u8 blendFlags)
{
    CameraViewSlot* vs;
    f32 blendProgress;

    Camera_GetCurrentViewSlot();
    blendProgress = CAMCONTROL_NORMALIZED_MAX;
    CAMCONTROL_CAMERA->blendProgress = blendProgress;
    CAMCONTROL_CAMERA->blendStep = blendProgress / (float)blendFrames;
    CAMCONTROL_CAMERA->queuedBlendFlags = blendFlags;

    vs = Camera_GetCurrentViewSlot();
    CAMCONTROL_CAMERA->blendStartX = vs->x;
    CAMCONTROL_CAMERA->blendStartY = vs->y;
    CAMCONTROL_CAMERA->blendStartZ = vs->z;
    CAMCONTROL_CAMERA->blendStartYaw = vs->yaw;
    CAMCONTROL_CAMERA->blendStartPitch = vs->pitch;
    CAMCONTROL_CAMERA->blendStartRoll = vs->roll;

    CAMCONTROL_CAMERA->blendStartFovY = Camera_GetFovY();
}

void cameraSetInterpMode(u8 mode)
{
    CAMCONTROL_CAMERA->blendCurveMode = mode;
}

void camcontrol_applyState(CamcontrolCameraState* camera)
{
    f32 prog;
    f32 clamped;
    CameraViewSlot* view;
    int itmp;
    f32 mag;
    f32 blendFactor;
    f32 delta[3];

    Camera_SetCurrentViewIndex(0);
    view = Camera_GetCurrentViewSlot();
    view->yaw = camera->yaw;
    view->pitch = camera->pitch;
    view->roll = camera->roll;
    if (((camera->smoothingFlags >> 7) & 1) != 0u)
    {
        PSVECSubtract((Vec*)&camera->worldX, (Vec*)&view->x, (Vec*)delta);
        mag = PSVECMag((Vec*)delta);
        if (mag > CAMCONTROL_NORMALIZED_MIN)
        {
            PSVECNormalize((Vec*)delta, (Vec*)delta);
        }
        blendFactor = interpolate(mag, CAMCONTROL_BLEND_INTERPOLATE_RATE, timeDelta);
        mag = (blendFactor < CAMCONTROL_NORMALIZED_MIN)
                  ? CAMCONTROL_NORMALIZED_MIN
                  : ((blendFactor > CAMCONTROL_BLEND_MAX_RATE * timeDelta) ? CAMCONTROL_BLEND_MAX_RATE * timeDelta : blendFactor);
        view->x = mag * delta[0] + view->x;
        view->y = mag * delta[1] + view->y;
        view->z = mag * delta[2] + view->z;
    }
    else
    {
        view->x = camera->worldX;
        view->y = camera->worldY;
        view->z = camera->worldZ;
    }
    lbl_803DD4D0 = camera->fovY;
    if (camera->blendProgress > CAMCONTROL_NORMALIZED_MIN)
    {
        camera->blendProgress = -(camera->blendStep * timeDelta - camera->blendProgress);
        prog = camera->blendProgress;
        clamped = CAMCONTROL_NORMALIZED_MIN;
        clamped = (prog < clamped) ? clamped : ((prog > CAMCONTROL_NORMALIZED_MAX) ? CAMCONTROL_NORMALIZED_MAX : prog);
        camera->blendProgress = clamped;
        if (CAMCONTROL_CAMERA->blendCurveMode == 2)
        {
            mag = CAMCONTROL_NORMALIZED_MAX - camera->blendProgress * camera->blendProgress * camera->blendProgress;
        }
        else if (CAMCONTROL_CAMERA->blendCurveMode == 1)
        {
            mag = CAMCONTROL_NORMALIZED_MAX - camera->blendProgress * camera->blendProgress;
        }
        else
        {
            mag = CAMCONTROL_NORMALIZED_MAX - camera->blendProgress;
        }
        blendFactor = (mag < CAMCONTROL_NORMALIZED_MIN)
                          ? CAMCONTROL_NORMALIZED_MIN
                          : ((mag > CAMCONTROL_NORMALIZED_MAX) ? CAMCONTROL_NORMALIZED_MAX : mag);
        if ((camera->queuedBlendFlags & CAMCONTROL_BLEND_X) != 0)
        {
            view->x = blendFactor * (view->x - camera->blendStartX) + camera->blendStartX;
        }
        if ((camera->queuedBlendFlags & CAMCONTROL_BLEND_Y) != 0)
        {
            view->y = blendFactor * (view->y - camera->blendStartY) + camera->blendStartY;
        }
        if ((camera->queuedBlendFlags & CAMCONTROL_BLEND_Z) != 0)
        {
            view->z = blendFactor * (view->z - camera->blendStartZ) + camera->blendStartZ;
        }
        OSReport(sDllBBTimeDebugFormat, blendFactor);
        if ((camera->queuedBlendFlags & CAMCONTROL_BLEND_YAW) != 0)
        {
            camera->blendDeltaYaw = camera->blendStartYaw - (u16)view->yaw;
            if (0x8000 < camera->blendDeltaYaw)
            {
                camera->blendDeltaYaw = (camera->blendDeltaYaw - 0x10000) + 1;
            }
            if (camera->blendDeltaYaw < -0x8000)
            {
                camera->blendDeltaYaw = (camera->blendDeltaYaw + 0x10000) - 1;
            }
            itmp = (int)((f32)camera->blendDeltaYaw * blendFactor);
            view->yaw = camera->blendStartYaw - itmp;
        }
        if ((camera->queuedBlendFlags & CAMCONTROL_BLEND_PITCH) != 0)
        {
            camera->blendDeltaPitch = camera->blendStartPitch - (u16)view->pitch;
            if (0x8000 < camera->blendDeltaPitch)
            {
                camera->blendDeltaPitch = (camera->blendDeltaPitch - 0x10000) + 1;
            }
            if (camera->blendDeltaPitch < -0x8000)
            {
                camera->blendDeltaPitch = (camera->blendDeltaPitch + 0x10000) - 1;
            }
            itmp = (int)((f32)camera->blendDeltaPitch * blendFactor);
            view->pitch = camera->blendStartPitch - itmp;
        }
        if ((camera->queuedBlendFlags & CAMCONTROL_BLEND_ROLL) != 0)
        {
            camera->blendDeltaRoll = camera->blendStartRoll - (u16)view->roll;
            if (0x8000 < camera->blendDeltaRoll)
            {
                camera->blendDeltaRoll = (camera->blendDeltaRoll - 0x10000) + 1;
            }
            if (camera->blendDeltaRoll < -0x8000)
            {
                camera->blendDeltaRoll = (camera->blendDeltaRoll + 0x10000) - 1;
            }
            itmp = (int)((f32)camera->blendDeltaRoll * blendFactor);
            view->roll = camera->blendStartRoll - itmp;
        }
    }
    Camera_SetFovY(lbl_803DD4D0);
    Obj_UpdateWorldTransform(view);
    loadMapForCameraPos(camera->worldX, camera->worldY, camera->worldZ);
    lbl_803DD4C0 = Camera_GetViewportYOffset();
    if ((int)lbl_803DD4C0 != camera->letterboxTargetOffset)
    {
        if ((int)lbl_803DD4C0 < camera->letterboxTargetOffset)
        {
            lbl_803DD4C0 = lbl_803DD4C0 + camera->letterboxStep * (int)timeDelta;
            if ((int)lbl_803DD4C0 > camera->letterboxTargetOffset)
            {
                lbl_803DD4C0 = camera->letterboxTargetOffset;
            }
        }
        else
        {
            lbl_803DD4C0 = lbl_803DD4C0 - camera->letterboxStep * (int)timeDelta;
            if ((int)lbl_803DD4C0 < camera->letterboxTargetOffset)
            {
                lbl_803DD4C0 = camera->letterboxTargetOffset;
            }
        }
        Camera_SetViewportYOffset(lbl_803DD4C0);
    }
    camera->letterboxTargetOffset = 0;
    Camera_UpdateViewMatrices();
}

void camcontrol_applyQueuedAction(void)
{
    CameraViewSlot* view;
    f32 blendStep;

    if (gCamcontrolQueuedActionPending != '\0')
    {
        if (gCamcontrolQueuedActionBlendFrames > 1)
        {
            blendStep = CAMCONTROL_NORMALIZED_MAX / gCamcontrolQueuedActionBlendFrames;
            if ((blendStep <= CAMCONTROL_NORMALIZED_MIN) || (blendStep > CAMCONTROL_NORMALIZED_MAX))
            {
                blendStep = 1.0f;
            }
            CAMCONTROL_CAMERA->blendProgress = 1.0f;
            CAMCONTROL_CAMERA->blendStep = blendStep;
            CAMCONTROL_CAMERA->queuedBlendFlags = gCamcontrolQueuedActionMode;
        }
        else
        {
            CAMCONTROL_CAMERA->blendProgress = CAMCONTROL_NORMALIZED_MIN;
            CAMCONTROL_CAMERA->queuedBlendFlags = 0;
        }
        view = Camera_GetCurrentViewSlot();
        if (CAMCONTROL_NORMALIZED_MAX == CAMCONTROL_CAMERA->blendProgress)
        {
            CAMCONTROL_CAMERA->blendStartX = view->x;
            CAMCONTROL_CAMERA->blendStartY = view->y;
            CAMCONTROL_CAMERA->blendStartZ = view->z;
            CAMCONTROL_CAMERA->blendStartYaw = view->yaw;
            CAMCONTROL_CAMERA->blendStartPitch = view->pitch;
            CAMCONTROL_CAMERA->blendStartRoll = view->roll;
            CAMCONTROL_CAMERA->blendStartFovY = Camera_GetFovY();
        }
        else
        {
            CAMCONTROL_CAMERA->yaw = view->yaw;
            CAMCONTROL_CAMERA->pitch = view->pitch;
            CAMCONTROL_CAMERA->roll = view->roll;
            CAMCONTROL_CAMERA->fovY = Camera_GetFovY();
        }
        gCamcontrolSavedActionId = gCamcontrolActiveActionId;
        gCamcontrolSavedActionPriority = gCamcontrolActiveActionPriority;
        gCamcontrolSavedActionStartFlags = gCamcontrolActiveActionStartFlags;
        camcontrol_activateHandler((u16)gCamcontrolQueuedActionId, gCamcontrolQueuedActionData);
        gCamcontrolQueuedActionPending = '\0';
        if (gCamcontrolQueuedActionData != NULL)
        {
            mm_free(gCamcontrolQueuedActionData);
            gCamcontrolQueuedActionData = NULL;
        }
    }
}

void Camera_applyTargetFlags(int targetFlagMode)
{
    CAMCONTROL_CAMERA->targetFlags = (u8)(CAMCONTROL_CAMERA->targetFlags | ((targetFlagMode << 3) & 0x18));
}

void Camera_setTargetFlag2(int enable)
{
    if (enable != 0)
    {
        CAMCONTROL_CAMERA->targetFlags = (u8)(CAMCONTROL_CAMERA->targetFlags | 2);
    }
    else
    {
        CAMCONTROL_CAMERA->targetFlags = (u8)(CAMCONTROL_CAMERA->targetFlags & ~2);
    }
}

void Camera_applyFrameFlags(int flags)
{
    CAMCONTROL_CAMERA->frameFlags = (u8)(CAMCONTROL_CAMERA->frameFlags | flags);
}

void Camera_setLetterbox(int yOffset, int applyNow)
{
    if (yOffset > CAMCONTROL_CAMERA->letterboxTargetOffset)
    {
        CAMCONTROL_CAMERA->letterboxTargetOffset = yOffset;
        CAMCONTROL_CAMERA->letterboxStep = 2;
        if (applyNow != 0)
        {
            Camera_SetViewportYOffset((s16)yOffset);
        }
    }
}

void Camera_minimapShowHelpTextForTarget(int renderArg2, int renderArg3, int renderArg4, int renderArg5)
{
    if (gameTextFn_80134be8() == 0)
    {
        gCamcontrolTargetHelpTextId = CAMCONTROL_HELP_TEXT_NONE;
        camcontrol_updateTargetReticle((CamcontrolTargetObject*)CAMCONTROL_CAMERA->targetReticleFocus,
                                       gCamcontrolActiveActionId == 0x49, renderArg2, renderArg3, renderArg4, renderArg5);
        CAMCONTROL_CAMERA->targetReticleOverride = 0;
    }
}

void camcontrol_setAButtonIconForTarget(void)
{
    CamcontrolTargetObject* target = (CamcontrolTargetObject*)CAMCONTROL_CAMERA->currentTarget;
    int kind;

    if (gameTextFn_80134be8() != 0)
        return;
    if (target == NULL)
        return;

    kind = target->targetSetup[target->targetSetupIndex].targetKind & CAMCONTROL_TARGET_KIND_MASK;
    if (kind == CAMCONTROL_TARGET_KIND_TALK_ICON)
    {
        if (target->classId == 6)
        {
            setAButtonIcon(CAMCONTROL_A_BUTTON_ICON_TALK_NPC);
        }
        else
        {
            setAButtonIcon(CAMCONTROL_A_BUTTON_ICON_TALK_OBJECT);
        }
    }
    else if (kind == CAMCONTROL_TARGET_KIND_A_BUTTON_HINT)
    {
        setAButtonIcon(CAMCONTROL_A_BUTTON_ICON_HINT);
    }
    else if (kind == CAMCONTROL_TARGET_KIND_CONTEXT_B_ICON)
    {
        setAButtonIcon(CAMCONTROL_A_BUTTON_ICON_CONTEXT_B);
    }
}

static inline u32 camcontrol_GetTargetKind(CamcontrolTargetObject* target)
{
    return target->targetSetup[target->targetSetupIndex].targetKind & CAMCONTROL_TARGET_KIND_MASK;
}

void camcontrol_updateTargetFeedback(void)
{
    u32 targetKind;
    s16 objType;
    f32 alphaScale;
    CamcontrolTargetObject* target;
    ObjAnimComponent* reticle;
    u8 buttonPressed;
    int result;
    u32 buttons;
    u32 buttonMask;
    f32 targetDistance;

    target = (CamcontrolTargetObject*)CAMCONTROL_CAMERA->currentTarget;
    reticle = &gCamcontrolTargetReticle->anim;
    buttonPressed = false;
    if (reticle == NULL)
    {
        return;
    }
    result = gameTextFn_80134be8();
    switch (result)
    {
    case 0:
        if ((gCamcontrolTargetChanged != '\0') && (gCamcontrolTargetChanged = '\0', target != NULL))
        {
            targetKind = CAMCONTROL_CAMERA->targetKind;
            if (targetKind == CAMCONTROL_TARGET_KIND_LOCKON)
            {
                Sfx_PlayFromObject(0, SFXTRIG_headcam_out);
                objShowButtonGlow(reticle, CAMCONTROL_NORMALIZED_MAX, 2);
            }
            else if ((targetKind == CAMCONTROL_TARGET_KIND_CONTEXT_A) ||
                     (targetKind == CAMCONTROL_TARGET_KIND_CONTEXT_B))
            {
                Sfx_PlayFromObject(0, SFXTRIG_lockon2_on);
                objShowButtonGlow(reticle, CAMCONTROL_NORMALIZED_MAX, 3);
            }
            else if (targetKind != CAMCONTROL_TARGET_KIND_SUPPRESSED)
            {
                Sfx_PlayFromObject(0, SFXTRIG_sc_scabshortish32);
                objShowButtonGlow(reticle, CAMCONTROL_NORMALIZED_MAX, 1);
            }
        }
        if (target != NULL)
        {
            target->targetFlags = target->targetFlags | CAMCONTROL_TARGET_FLAG_RETICLE_TOUCHING;
            buttons = getButtonsJustPressed(0);
            buttonMask = CAMCONTROL_TARGET_BUTTON_PRIMARY;
            targetKind = camcontrol_GetTargetKind(target);
            if ((targetKind == CAMCONTROL_TARGET_KIND_CONTEXT_A) || (targetKind == CAMCONTROL_TARGET_KIND_CONTEXT_B))
            {
                buttonMask = CAMCONTROL_TARGET_BUTTON_CONTEXT;
            }
            if ((buttons & buttonMask) != 0)
            {
                buttonPressed = true;
            }
            if ((target->targetFlags & CAMCONTROL_TARGET_FLAG_ACCEPTS_INPUT) == 0)
            {
                if (buttonPressed)
                {
                    target->targetFlags = target->targetFlags | CAMCONTROL_TARGET_FLAG_INPUT_PRESSED;
                }
            }
            else if ((buttonPressed) && (result = isTalkingToNpc(), result == 0))
            {
                Sfx_PlayFromObject(0, SFXTRIG_sc_clock_timesup);
            }
        }
        if (gCamcontrolTargetState == '\0')
        {
            if (reticle->currentMoveProgress <= CAMCONTROL_NORMALIZED_MIN)
            {
                if (target != NULL)
                {
                    CAMCONTROL_CAMERA->targetReticleFocus = (int)target;
                    CAMCONTROL_CAMERA->targetKind = camcontrol_GetTargetKind(target);
                    gCamcontrolTargetState = CAMCONTROL_TARGET_RETICLE_STATE_ACTIVE;
                    gCamcontrolTargetChanged = true;
                }
                else
                {
                    CAMCONTROL_CAMERA->targetReticleFocus = 0;
                }
            }
            else
            {
                ObjAnim_AdvanceCurrentMove((int)reticle, CAMCONTROL_RETICLE_FADE_OUT_STEP,
                                                                            timeDelta, NULL);
            }
        }
        else if (((u32)CAMCONTROL_CAMERA->targetReticleFocus != (u32)target) &&
                 (reticle->currentMoveProgress >= CAMCONTROL_NORMALIZED_MAX))
        {
            gCamcontrolTargetState = CAMCONTROL_TARGET_RETICLE_STATE_INACTIVE;
            if (target != NULL)
            {
                ObjAnim_SetMoveProgress((ObjAnimComponent*)reticle, CAMCONTROL_NORMALIZED_MIN);
            }
            if (target == NULL)
            {
                targetKind = CAMCONTROL_CAMERA->targetKind;
                if (targetKind == CAMCONTROL_TARGET_KIND_LOCKON)
                {
                    Sfx_PlayFromObject(0, SFXTRIG_strafe_active);
                }
                else if ((targetKind == CAMCONTROL_TARGET_KIND_CONTEXT_A) ||
                         (targetKind == CAMCONTROL_TARGET_KIND_CONTEXT_B))
                {
                    Sfx_PlayFromObject(0, SFXTRIG_lockon2_off);
                }
                else if (targetKind != CAMCONTROL_TARGET_KIND_SUPPRESSED)
                {
                    Sfx_PlayFromObject(0, SFXTRIG_sc_gemrun1022);
                }
            }
        }
        else
        {
            ObjAnim_AdvanceCurrentMove((int)reticle, CAMCONTROL_RETICLE_FADE_IN_STEP,
                                                                        timeDelta, NULL);
        }
        result = Obj_IsObjectAlive((GameObject*)CAMCONTROL_CAMERA->targetReticleFocus);
        if (result == 0)
        {
            CAMCONTROL_CAMERA->targetReticleFocus = 0;
        }
        if ((gCamcontrolTargetState == CAMCONTROL_TARGET_RETICLE_STATE_ACTIVE) &&
            ((u32)CAMCONTROL_CAMERA->targetReticleFocus != 0))
        {
            target = (CamcontrolTargetObject*)CAMCONTROL_CAMERA->targetReticleFocus;
            if ((target->targetFlags & CAMCONTROL_TARGET_FLAG_ACCEPTS_INPUT) != 0)
            {
                CAMCONTROL_CAMERA->targetFlags =
                    CAMCONTROL_CAMERA->targetFlags | CAMCONTROL_CAMERA_TARGET_FLAG_ACCEPTS_INPUT;
            }
            else
            {
                CAMCONTROL_CAMERA->targetFlags =
                    CAMCONTROL_CAMERA->targetFlags & ~CAMCONTROL_CAMERA_TARGET_FLAG_ACCEPTS_INPUT;
            }
            target = (CamcontrolTargetObject*)CAMCONTROL_CAMERA->targetReticleFocus;
            objType = target->objType;
            switch (objType)
            {
            case 0x11:
            case 0xd8:
            case 0x13a:
            case 0x251:
            case 0x25d:
            case 0x281:
            case 0x369:
            case 0x3fe:
            case 0x427:
            case 0x457:
            case 0x458:
            case 0x4ac:
            case 0x4d7:
            case 0x58b:
            case 0x5b7:
            case 0x5b8:
            case 0x5b9:
            case 0x5e1:
            case 0x613:
            case 0x642:
            case 0x6a2:
            case 0x6a3:
            case 0x6a4:
            case 0x6a5:
            case 0x842:
            case 0x84b:
            case 0x851:
                targetDistance = enemy_getHealthFraction((GameObject*)target);
                break;
            case 0x3de:
            case 0x49f:
                targetDistance = LargeCrate_getReticleDistance((GameObject*)target);
                break;
            case 0x31:
                targetDistance = CAMCONTROL_NORMALIZED_MAX;
                break;
            default:
                result = dll_19_isBaddieControlObject((GameObject*)target);
                if (result != 0)
                {
                    targetDistance = (*gBaddieControlInterface)->getHealthFraction((GameObject*)target);
                }
                else
                {
                    targetDistance = CAMCONTROL_NORMALIZED_MAX;
                }
                break;
            }
            if (targetDistance <= CAMCONTROL_NORMALIZED_MIN &&
                CAMCONTROL_CAMERA->targetDistance > CAMCONTROL_NORMALIZED_MIN)
            {
                objShowButtonGlow(reticle, CAMCONTROL_NORMALIZED_MAX, 4);
            }
            else if (targetDistance <= CAMCONTROL_TARGET_DISTANCE_TIER1 &&
                     CAMCONTROL_CAMERA->targetDistance > CAMCONTROL_TARGET_DISTANCE_TIER1)
            {
                objShowButtonGlow(reticle, CAMCONTROL_NORMALIZED_MAX, 4);
            }
            else if (targetDistance <= CAMCONTROL_TARGET_DISTANCE_TIER2 &&
                     CAMCONTROL_CAMERA->targetDistance > CAMCONTROL_TARGET_DISTANCE_TIER2)
            {
                objShowButtonGlow(reticle, CAMCONTROL_NORMALIZED_MAX, 4);
            }
            else if (targetDistance <= CAMCONTROL_TARGET_DISTANCE_TIER3 &&
                     CAMCONTROL_CAMERA->targetDistance > CAMCONTROL_TARGET_DISTANCE_TIER3)
            {
                objShowButtonGlow(reticle, CAMCONTROL_NORMALIZED_MAX, 4);
            }
            CAMCONTROL_CAMERA->targetDistance = targetDistance;
        }
        alphaScale = CAMCONTROL_RETICLE_ALPHA_SCALE * reticle->currentMoveProgress;
        alphaScale = (alphaScale < CAMCONTROL_NORMALIZED_MIN)
                         ? CAMCONTROL_NORMALIZED_MIN
                         : ((alphaScale > CAMCONTROL_RETICLE_ALPHA_SCALE) ? CAMCONTROL_RETICLE_ALPHA_SCALE : alphaScale);
        reticle->alpha = alphaScale;
        gCamcontrolReticleSpin = CAMCONTROL_RETICLE_SPIN_STEP;
        *(s16*)&reticle->rotX = (CAMCONTROL_RETICLE_SPIN_STEP_PER_FRAME * timeDelta + (float)reticle->rotX);
        break;
    }
}

int Camera_isZooming(void)
{
    return CAMCONTROL_CAMERA->blendProgress > CAMCONTROL_NORMALIZED_MIN;
}

void Camera_setTargetReticleOverride(int target)
{
    CAMCONTROL_CAMERA->targetReticleOverride = target;
}

void Camera_setTarget(int target)
{
    CAMCONTROL_CAMERA->overrideTarget = target;
    CAMCONTROL_CAMERA->currentTarget = target;
}

int Camera_getTarget(void)
{
    return CAMCONTROL_CAMERA->currentTarget;
}

int Camera_getOverrideTarget(void)
{
    return CAMCONTROL_CAMERA->overrideTarget;
}

void camcontrol_getRelativePosition(void* targetObj, f32* outX, f32* outY, f32* outZ, f32* outDistanceXZ,
                                    f32 heightOffset, int useLocalPosition)
{
    ObjAnimComponent* focusObj;
    ObjAnimComponent* target;

    focusObj = CAMCONTROL_CAMERA->focusObj;
    target = targetObj;
    if (useLocalPosition != 0)
    {
        *outX = target->localPosX - focusObj->localPosX;
        *outY = target->localPosY - (focusObj->localPosY + heightOffset);
        *outZ = target->localPosZ - focusObj->localPosZ;
    }
    else
    {
        *outX = target->worldPosX - focusObj->worldPosX;
        *outY = target->worldPosY - (focusObj->worldPosY + heightOffset);
        *outZ = target->worldPosZ - focusObj->worldPosZ;
    }
    if (outDistanceXZ != NULL)
    {
        *outDistanceXZ = *outX * *outX + *outZ * *outZ;
        if (*outDistanceXZ > CAMCONTROL_NORMALIZED_MIN)
        {
            *outDistanceXZ = sqrtf(*outDistanceXZ);
        }
        if (*outDistanceXZ < CAMCONTROL_MIN_TARGET_DISTANCE)
        {
            *outDistanceXZ = CAMCONTROL_MIN_TARGET_DISTANCE;
        }
    }
    return;
}

void camcontrol_initialise(f32 numerator, f32* dst, f32 denominator, f32 minValue, f32 y, f32 z)
{
    f32 ratio;

    ratio = numerator / denominator;
    if (ratio < minValue)
    {
        ratio = minValue;
    }
    dst[0] = ratio;
    dst[1] = y;
    dst[2] = CAMCONTROL_NORMALIZED_MIN;
    dst[3] = z;
}

void Camera_moveBy(f32 x, f32 y, f32 z)
{
    CAMCONTROL_CAMERA->localX += x;
    CAMCONTROL_CAMERA->localY += y;
    CAMCONTROL_CAMERA->localZ += z;
}

void Camera_overridePos(f32 x, f32 y, f32 z)
{
    CAMCONTROL_CAMERA->overrideWorldPosPending = 1;
    CAMCONTROL_CAMERA->overrideWorldX = x;
    CAMCONTROL_CAMERA->overrideWorldY = y;
    CAMCONTROL_CAMERA->overrideWorldZ = z;
}

void Camera_setFocus(void* target, int flags)
{
    if (target == CAMCONTROL_CAMERA->focusObj)
    {
        return;
    }
    CAMCONTROL_CAMERA->focusObj = target;
}

static inline CamcontrolHandlerEntry* camcontrol_findDefaultHandler(void)
{
    int handlerCount;
    register CamcontrolHandlerEntry** handlerEntry;
    int handlerIndex;

    handlerIndex = 0;
    handlerEntry = gCamcontrolHandlerEntries;
    for (handlerCount = gCamcontrolHandlerCount; 0 < handlerCount; handlerCount--)
    {
        if ((*handlerEntry)->actionId == CAMCONTROL_ACTION_DEFAULT)
        {
            return gCamcontrolHandlerEntries[handlerIndex];
        }
        handlerEntry++;
        handlerIndex++;
    }
    return NULL;
}

void camcontrol_loadTriggeredCamAction(int triggerType, int actionNo, int triggerMode)
{
    CamcontrolHandlerEntry* defaultHandler;
    int blendFrames;
    CamcontrolTriggeredAction* camAction;
    int actionOffset;
    CamcontrolQueuedActionParam triggerType1Param;
    CamcontrolQueuedActionParam triggerType2Param;

    switch (triggerType)
    {
    case CAMCONTROL_TRIGGER_KIND_LOAD_ACTION:
        break;
    case CAMCONTROL_TRIGGER_KIND_QUEUE_TYPE1:
        triggerType1Param.actionIndex = actionNo & CAMCONTROL_ACTION_INDEX_MASK;
        triggerType1Param.noBlendFlag = actionNo & CAMCONTROL_ACTION_FLAG_NO_BLEND;
        CAMCONTROL_CAMERA->blendCurveMode = 1;
        if (triggerType1Param.noBlendFlag != 0)
        {
            blendFrames = 0;
        }
        else
        {
            blendFrames = CAMCONTROL_DEFAULT_BLEND_FRAMES;
        }
        Camera_setMode(CAMCONTROL_ACTION_TRIGGER_TYPE1, 1, 0, CAMCONTROL_QUEUED_ACTION_PARAM_SIZE, &triggerType1Param,
                       blendFrames, CAMCONTROL_QUEUE_SENTINEL);
        return;
    case CAMCONTROL_TRIGGER_KIND_QUEUE_TYPE2:
        triggerType2Param.actionIndex = actionNo & CAMCONTROL_ACTION_INDEX_MASK;
        triggerType2Param.noBlendFlag = (u8)(actionNo & CAMCONTROL_ACTION_FLAG_NO_BLEND);
        if (triggerType2Param.noBlendFlag != 0)
        {
            blendFrames = 0;
        }
        else
        {
            blendFrames = CAMCONTROL_DEFAULT_BLEND_FRAMES;
        }
        Camera_setMode(CAMCONTROL_ACTION_TRIGGER_TYPE2, 1, 0, CAMCONTROL_QUEUED_ACTION_PARAM_SIZE, &triggerType2Param,
                       blendFrames, CAMCONTROL_QUEUE_SENTINEL);
        return;
    case CAMCONTROL_TRIGGER_KIND_DEFAULT_ACTION:
        Camera_setMode(CAMCONTROL_ACTION_DEFAULT, 0, 1, 0, 0, CAMCONTROL_DEFAULT_BLEND_FRAMES,
                       CAMCONTROL_QUEUE_SENTINEL);
        return;
    case CAMCONTROL_TRIGGER_KIND_DEFAULT_ACTION_OFFSET:
        Camera_setMode(actionNo + CAMCONTROL_ACTION_DEFAULT, 1, 0, 0, 0, CAMCONTROL_DEFAULT_BLEND_FRAMES,
                       CAMCONTROL_QUEUE_SENTINEL);
        return;
    }
    if (actionNo != CAMCONTROL_ACTION_NO_NONE)
    {
        if (actionNo == CAMCONTROL_ACTION_NO_NONE)
        {
            camAction = NULL;
        }
        else
        {
            camAction = (CamcontrolTriggeredAction*)mmAlloc(CAMCONTROL_ACTION_RECORD_SIZE, CAMCONTROL_ACTION_HEAP, 0);
            if (camAction != NULL)
            {
                actionOffset = (actionNo - 1) * CAMCONTROL_ACTION_RECORD_SIZE;
                getTabEntry(camAction, CAMCONTROL_ACTION_FILE_ID, actionOffset, CAMCONTROL_ACTION_RECORD_SIZE);
            }
        }
        if (camAction == NULL)
        {
            return;
        }
        camAction->triggerMode = triggerMode;
        SaveGame_setCamActionNo((short)actionNo);
        if (((((int)gCamcontrolActiveActionId != CAMCONTROL_ACTION_DEFAULT) &&
              ((int)gCamcontrolActiveActionId != CAMCONTROL_ACTION_TRIGGERED)) &&
             ((int)gCamcontrolActiveActionId != CAMCONTROL_ACTION_TRIGGER_TYPE1)) &&
            ((int)gCamcontrolActiveActionId != CAMCONTROL_ACTION_TRIGGER_TYPE2))
        {
            defaultHandler = camcontrol_findDefaultHandler();
            defaultHandler->handler->vtable->actionCallback(camAction, CAMCONTROL_ACTION_RECORD_SIZE);
        }
        else
        {
            switch (camAction->actionKind)
            {
            case CAMCONTROL_TRIGGERED_ACTION_KIND_DEFAULT:
            default:
                Camera_setMode(CAMCONTROL_ACTION_DEFAULT, 0, 2, CAMCONTROL_ACTION_RECORD_SIZE, camAction, 0,
                               CAMCONTROL_QUEUE_SENTINEL);
                break;
            case CAMCONTROL_TRIGGERED_ACTION_KIND_TRIGGERED:
                Camera_setMode(CAMCONTROL_ACTION_TRIGGERED, 1, 2, CAMCONTROL_ACTION_RECORD_SIZE, camAction, 0,
                               CAMCONTROL_QUEUE_SENTINEL);
                break;
            }
        }
        mm_free(camAction);
    }
    else
    {
        OSReport(sCamcontrolTriggeredCamActionLoadWarning, actionNo);
        camAction = (CamcontrolTriggeredAction*)mmAlloc(CAMCONTROL_ACTION_RECORD_SIZE, CAMCONTROL_ACTION_HEAP, 0);
        if (camAction != NULL)
        {
            getTabEntry(camAction, CAMCONTROL_ACTION_FILE_ID, CAMCONTROL_FALLBACK_ACTION_FILE_OFFSET,
                        CAMCONTROL_ACTION_RECORD_SIZE);
        }
        if (camAction == NULL)
        {
            return;
        }
        camAction->triggerMode = triggerMode;
        SaveGame_setCamActionNo(CAMCONTROL_FALLBACK_ACTION_NO);
        if (((((int)gCamcontrolActiveActionId != CAMCONTROL_ACTION_DEFAULT) &&
              ((int)gCamcontrolActiveActionId != CAMCONTROL_ACTION_TRIGGERED)) &&
             ((int)gCamcontrolActiveActionId != CAMCONTROL_ACTION_TRIGGER_TYPE1)) &&
            ((int)gCamcontrolActiveActionId != CAMCONTROL_ACTION_TRIGGER_TYPE2))
        {
            defaultHandler = camcontrol_findDefaultHandler();
            defaultHandler->handler->vtable->actionCallback(camAction, CAMCONTROL_ACTION_RECORD_SIZE);
        }
        else
        {
            switch (camAction->actionKind)
            {
            case CAMCONTROL_TRIGGERED_ACTION_KIND_DEFAULT:
            default:
                Camera_setMode(CAMCONTROL_ACTION_DEFAULT, 0, 2, CAMCONTROL_ACTION_RECORD_SIZE, camAction, 0,
                               CAMCONTROL_QUEUE_SENTINEL);
                break;
            case CAMCONTROL_TRIGGERED_ACTION_KIND_TRIGGERED:
                Camera_setMode(CAMCONTROL_ACTION_TRIGGERED, 1, 2, CAMCONTROL_ACTION_RECORD_SIZE, camAction, 0,
                               CAMCONTROL_QUEUE_SENTINEL);
                break;
            }
        }
        mm_free(camAction);
    }
    return;
}

CamcontrolTriggeredAction* Camera_getCamActionsBinEntry(int actionNo)
{
    CamcontrolTriggeredAction* camAction;

    if (actionNo == CAMCONTROL_ACTION_NO_NONE)
    {
        return NULL;
    }
    camAction = mmAlloc(CAMCONTROL_ACTION_RECORD_SIZE, CAMCONTROL_ACTION_HEAP, 0);
    if (camAction != NULL)
    {
        getTabEntry(camAction, CAMCONTROL_ACTION_FILE_ID, (actionNo - 1) * CAMCONTROL_ACTION_RECORD_SIZE,
                    CAMCONTROL_ACTION_RECORD_SIZE);
    }
    return camAction;
}

void camcontrol_release(void* camAction, int recordSize)
{
    CamcontrolHandlerEntry* currentHandler;

    currentHandler = gCamcontrolCurrentHandler;
    if (currentHandler != NULL)
    {
        currentHandler->handler->vtable->actionCallback(camAction, recordSize);
    }
}

void camcontrol_queueSavedAction(int blendFrames, u8 queueMode)
{
    if (gCamcontrolSavedActionId != CAMCONTROL_SAVED_ACTION_NONE)
    {
        Camera_setMode(gCamcontrolSavedActionId, gCamcontrolSavedActionPriority, gCamcontrolSavedActionStartFlags, 0, 0,
                       blendFrames, queueMode);
    }
    return;
}

void Camera_setMode(s32 actionId, int priority, int startFlags, int dataSize, void* data, int blendFrames, u8 queueMode)
{
    if (gCamcontrolQueuedActionData != NULL)
    {
        mm_free(gCamcontrolQueuedActionData);
        gCamcontrolQueuedActionData = NULL;
        gCamcontrolQueuedActionPending = 0;
    }
    gCamcontrolQueuedActionId = actionId;
    gCamcontrolQueuedActionBlendFrames = blendFrames;
    if (data != NULL)
    {
        gCamcontrolQueuedActionData = mmAlloc(dataSize, CAMCONTROL_ACTION_HEAP, 0);
        memcpy(gCamcontrolQueuedActionData, data, dataSize);
    }
    else
    {
        gCamcontrolQueuedActionData = NULL;
    }
    if (actionId == CAMCONTROL_ACTION_DEFAULT)
    {
        gCamcontrolQueuedActionPriority = 0;
    }
    else
    {
        gCamcontrolQueuedActionPriority = priority;
    }
    gCamcontrolQueuedActionStartFlags = startFlags;
    gCamcontrolQueuedActionPending = 1;
    gCamcontrolQueuedActionMode = queueMode;
    return;
}



void* Camera_getDefaultHandlerEntry(void)
{
    int i;

    i = 0;
    for (; i < gCamcontrolHandlerCount; i++)
    {
        if (gCamcontrolHandlerEntries[i]->actionId == CAMCONTROL_ACTION_DEFAULT)
        {
            return gCamcontrolHandlerEntries[i];
        }
    }
    return NULL;
}

void* Camera_GetFollowPos(void)
{
    return gCamcontrolCurrentHandler;
}

int Camera_getMode(void)
{
    return gCamcontrolActiveActionId;
}

void* Camera_get(void)
{
    return pCamera;
}

#define camera CAMCONTROL_CAMERA
void Camera_update(u8 framesThisStep)
{
    ObjAnimComponent* focus;
    u8 textActive;
    CamcontrolTargetObject* target;

    if (gameTextFn_80134be8() != 0)
    {
        textActive = 1;
    }
    else
    {
        textActive = 0;
    }
    focus = camera->focusObj;
    if (focus == NULL)
    {
        camera->currentTarget = 0;
        camera->overrideTarget = 0;
    }
    else
    {
        gCamcontrolSavedFocusLocalX = focus->localPosX;
        gCamcontrolSavedFocusLocalY = focus->localPosY;
        gCamcontrolSavedFocusLocalZ = focus->localPosZ;
        gCamcontrolSavedFocusWorldX = focus->worldPosX;
        gCamcontrolSavedFocusWorldY = focus->worldPosY;
        gCamcontrolSavedFocusWorldZ = focus->worldPosZ;
        camcontrol_updateMoveAverage(camera, focus);
        if (camera->overrideWorldPosPending != 0)
        {
            focus->worldPosX = camera->overrideWorldX;
            focus->worldPosY = camera->overrideWorldY;
            focus->worldPosZ = camera->overrideWorldZ;
            Obj_TransformWorldPointToLocal(focus->worldPosX, focus->worldPosY, focus->worldPosZ, &focus->localPosX,
                                           &focus->localPosY, &focus->localPosZ, (u32)focus->parent);
            camera->overrideWorldPosPending = 0;
        }
        if (camera->localFrameObj != focus->parent)
        {
            Obj_TransformLocalPointToWorld(camera->localX, camera->localY, camera->localZ, &camera->worldX,
                                           &camera->worldY, &camera->worldZ, (u32)camera->localFrameObj);
            Obj_TransformLocalPointToWorld(camera->prevLocalX, camera->prevLocalY, camera->prevLocalZ,
                                           &camera->prevWorldX, &camera->prevWorldY, &camera->prevWorldZ,
                                           (u32)camera->localFrameObj);
            Obj_TransformWorldPointToLocal(camera->worldX, camera->worldY, camera->worldZ, &camera->localX,
                                           &camera->localY, &camera->localZ, (u32)focus->parent);
            Obj_TransformWorldPointToLocal(camera->prevWorldX, camera->prevWorldY, camera->prevWorldZ,
                                           &camera->prevLocalX, &camera->prevLocalY, &camera->prevLocalZ,
                                           (u32)focus->parent);
            camera->localFrameObj = focus->parent;
        }
        if (focus->parent != NULL)
        {
            focus->rotX += ((ObjAnimComponent*)focus->parent)->rotX;
        }
        camcontrol_applyQueuedAction();
        if (gCamcontrolCurrentHandler != 0)
        {
            gCamcontrolCurrentHandler->handler->vtable->update((void*)pCamera);
            Obj_TransformLocalPointToWorld(camera->localX, camera->localY, camera->localZ, &camera->worldX,
                                           &camera->worldY, &camera->worldZ, (u32)camera->localFrameObj);
            camcontrol_applyState(camera);
        }
        camcontrol_applyQueuedAction();
        if (textActive == 0)
        {
            if (camera->overrideTarget == 0u)
            {
                target = camcontrol_findBestTarget(camera, focus);
                camera->currentTarget = (int)target;
            }
            else
            {
                camera->currentTarget = camera->overrideTarget;
            }
        }
        camera->prevLocalX = camera->localX;
        camera->prevLocalY = camera->localY;
        camera->prevLocalZ = camera->localZ;
        camera->prevWorldX = camera->worldX;
        camera->prevWorldY = camera->worldY;
        camera->prevWorldZ = camera->worldZ;
        camera->frameFlags = 0;
        focus->localPosX = gCamcontrolSavedFocusLocalX;
        focus->localPosY = gCamcontrolSavedFocusLocalY;
        focus->localPosZ = gCamcontrolSavedFocusLocalZ;
        focus->worldPosX = gCamcontrolSavedFocusWorldX;
        focus->worldPosY = gCamcontrolSavedFocusWorldY;
        focus->worldPosZ = gCamcontrolSavedFocusWorldZ;
        if (focus->parent != NULL)
        {
            focus->rotX -= ((ObjAnimComponent*)focus->parent)->rotX;
        }
    }
    return;
}
#undef camera

void Camera_init(void* focus, f32 x, f32 y, f32 z)
{
    memset((void*)pCamera, 0, sizeof(CamcontrolCameraState));
    CAMCONTROL_CAMERA->localX = x;
    CAMCONTROL_CAMERA->localY = y;
    CAMCONTROL_CAMERA->localZ = z;
    CAMCONTROL_CAMERA->worldX = x;
    CAMCONTROL_CAMERA->worldY = y;
    CAMCONTROL_CAMERA->worldZ = z;
    CAMCONTROL_CAMERA->prevLocalX = x;
    CAMCONTROL_CAMERA->prevLocalY = y;
    CAMCONTROL_CAMERA->prevLocalZ = z;
    CAMCONTROL_CAMERA->prevWorldX = x;
    CAMCONTROL_CAMERA->prevWorldY = y;
    CAMCONTROL_CAMERA->prevWorldZ = z;
    CAMCONTROL_CAMERA->focusObj = focus;
    CAMCONTROL_CAMERA->fovY = CAMCONTROL_DEFAULT_FOV_Y;
    gCamcontrolTargetState = CAMCONTROL_TARGET_RETICLE_STATE_INACTIVE;
}

void Camera_release(void)
{
    voxmaps_resetLoadedMaps();
    lbl_803DD4CB = -1;
}

void Camera_initialise(void)
{
    pCamera = gCamcontrolStateStorage;
    memset((void*)pCamera, 0, sizeof(CamcontrolCameraState));
    voxmaps_initialise();
    gCamcontrolActiveActionId = -1;
    gCamcontrolCurrentHandlerIndex = -1;
    gCamcontrolQueuedActionId = -1;
    lbl_803DD4CC = 0;
    lbl_803DD4CB = -1;
    gCamcontrolTargetClassMask = 0xffff;
}


char sCamcontrolTriggeredCamActionLoadWarning[] = "<camcontrol.c>  failed to load triggered camaction actionno %d\n";
