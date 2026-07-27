/*
 * CRrockfall (DLL 0x16A) - a scripted falling-rock / boulder object.
 *
 * On init the per-rock scale is derived from the placement params, the
 * capsule hitbox is sized from the sub-object bounds, and a config-table
 * variant is chosen by seqId (entry 1 of gRockfallCfgTable for seqId 0x600,
 * else entry 0). update() runs the fall state machine:
 *   mode 0 armed   - count down fallDelay while the player is in xz range
 *   mode 1 falling - gravity integrate Y, scrape sfx, until floorY+restOffsetY
 *   mode 2 resting - hitbox stays live
 *   mode 3 shattered - on a hit: stop scrape sfx, play impact sfx and
 *                      (for non-seqId-103 rocks) spawn an explosion
 * The fall is gated by the placement game bit (unk1C); render fades the
 * rock by height fraction and player distance and hides it once shattered.
 */
#include "main/audio/sfx_ids.h"
#include "main/audio/sfx_stop_channel_api.h"
#include "main/vecmath_distance_api.h"
#include "main/audio/sfx_play_api.h"
#include "main/object_render.h"
#include "main/dll/crrockfallplacement_struct.h"
#include "main/dll/crrockfall_types.h"
#include "game/objects/object.h"
#include "sys/objects.h"
#include "main/track_dolphin_api.h"
#include "main/objfx.h"
#include "main/resource.h"
#include "main/gamebits.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/frame_timing.h"
#include "main/objhits.h"
#include "dlls/object_descriptor.h"

STATIC_ASSERT(sizeof(CrRockfallState) == 0x14);

/* anim.seqId rock variants: BIG selects gRockfallCfgTable entry 1;
 * QUARRY has its own scrape/impact sfx and skips the explosion. */
#define CRROCKFALL_SEQ_BIG    0x600
#define CRROCKFALL_SEQ_QUARRY 103

/* CrRockfallState.mode */
#define zcEn3_ROCKFALL_MODE_ARMED     0 /* count down fallDelay while player is in range */
#define zcEn3_ROCKFALL_MODE_FALLING   1 /* gravity integrate Y until floorY+restOffsetY */
#define zcEn3_ROCKFALL_MODE_RESTING   2 /* landed; hitbox stays live */
#define zcEn3_ROCKFALL_MODE_SHATTERED 3 /* hit: stop scrape sfx, play impact, maybe explode */
#define zcEn3_ROCKFALL_MODE_4         4

void* gRockfallResource;
extern u8 gRockfallCfgTable[];

static int crrockfall_isTriggerNear(f32 xz, f32 dy, u32 range)
{
    return xz > 0.0f && xz < 4.0f * (f32)range && dy < 300.0f;
}

f32 crrockfall_findFloorY(GameObject* obj)
{
    CrRockfallState* state = (obj)->extra;
    TrackGroundHit** list;
    TrackGroundHit** p;
    int count;
    int i;
    int bestIdx;
    f32 bestDist;
    count = hitDetectFn_80065e50(obj, (obj)->anim.localPosX, (obj)->anim.localPosY, (obj)->anim.localPosZ, &list,
                                 0, 0);
    bestDist = 100000.0f;
    bestIdx = -1;
    for (i = 0, p = list; i < count; i++)
    {
        f32 dy;
        if ((dy = (obj)->anim.localPosY - (*p)->height) > 20.0f && dy < bestDist)
        {
            bestDist = dy;
            bestIdx = i;
        }
        p++;
    }
    if (bestIdx != -1)
    {
        state->floorFound = 1;
        return list[bestIdx]->height;
    }
    return (obj)->anim.localPosY;
}

int crrockfall_getExtraSize(void)
{
    return 0x14;
}
int crrockfall_getObjectTypeId(void)
{
    return 0x0;
}

void crrockfall_free(void)
{
}

void crrockfall_render(GameObject* obj, int p1, int p2, int p3, int p4, s8 visible)
{
    CrRockfallState* state = obj->extra;
    if (state->mode != zcEn3_ROCKFALL_MODE_SHATTERED && visible != 0)
    {
        objRenderModelAndHitVolumes(obj, p1, p2, p3, p4, 1.0f);
    }
}

void crrockfall_hitDetect(void)
{
}

void crrockfall_update(GameObject* obj)
{
    CrRockfallState* state = obj->extra;
    ObjHitsPriorityState* hitState = *(ObjHitsPriorityState**)&obj->anim.hitReactState;
    ObjModelState* modelState = obj->anim.modelState;
    CrrockfallPlacement* placement = *(CrrockfallPlacement**)&obj->anim.placementData;

    if (gRockfallResource == NULL)
    {
        gRockfallResource = Resource_Acquire(91, 1);
    }

    if (state->floorFound == 0)
    {
        state->floorY = crrockfall_findFloorY(obj);
        if (state->floorFound != 0 && modelState != NULL)
        {
            modelState->overrideWorldPosY = state->floorY;
            objShadowInvalidate(obj);
        }
        return;
    }
    else
    {
        if (modelState != NULL)
        {
            f32 frac;
            f32 height;
            f32 dist;
            int alphaScale;
            GameObject* player;
            frac = (obj->anim.localPosY - state->floorY) / (state->startY - state->floorY);
            if (frac > 1.0f)
            {
                frac = 1.0f;
            }
            else if (frac < 0.0f)
            {
                frac = 0.0f;
            }
            height = (1.0f) - frac;
            player = Obj_GetPlayerObject();
            if (player != NULL)
            {
                dist = Vec_distance(&obj->anim.worldPosX, &player->anim.worldPosX);
                if (dist > 350.0f)
                {
                    dist = 350.0f;
                }
                else if (dist < 250.0f)
                {
                    dist = 250.0f;
                }
            }
            else
            {
                dist = 350.0f;
            }
            dist = (dist - 250.0f) / 100.0f;
            dist = 1.0f - dist;
            alphaScale = (int)(120.0f * height) + 0x40;
            modelState->shadowAlpha =
                (int)(((f32)(u32) * (u8*)((char*)obj + 0x37) / 255.0f) * ((f32)alphaScale * dist));
        }

        if (placement->gameBitId != -1 &&
            mainGetBit(placement->gameBitId) == 0)
        {
            return;
        }

        switch (state->mode)
        {
        case zcEn3_ROCKFALL_MODE_ARMED:
        {
            CrrockfallPlacement* desc;
            f32 xz;
            f32 dy;
            int inRange;
            GameObject* player = Obj_GetPlayerObject();
            if (player == NULL)
            {
                inRange = 0;
            }
            else
            {
                desc = *(CrrockfallPlacement**)&obj->anim.placementData;
                xz = Vec_xzDistance(&obj->anim.worldPosX,
                                    &player->anim.worldPosX);
                dy = obj->anim.localPosY - player->anim.localPosY;
                if (dy < 0.0f)
                {
                    dy = 0.0f;
                }
                if (xz < 4.0f * (f32)(u32)desc->triggerRange && dy < 300.0f)
                {
                    inRange = 1;
                }
                else
                {
                    inRange = 0;
                }
            }
            if (inRange != 0)
            {
                if ((state->fallDelay -= framesThisStep) <= 0)
                {
                    state->mode = zcEn3_ROCKFALL_MODE_FALLING;
                }
            }
            break;
        }
        case zcEn3_ROCKFALL_MODE_FALLING:
            if (state->fallStarted == 0)
            {
                state->fallStarted = 1;
                obj->anim.velocityY = 0.0f;
                if (obj->anim.seqId == CRROCKFALL_SEQ_QUARRY)
                {
                    Sfx_PlayFromObject((u32)obj, SFXTRIG_dn_boar1_c_155);
                }
                Sfx_PlayFromObject((u32)obj, SFXTRIG_wp_swdwood16);
                hitState->flags |= 1;
            }
            *(int*)&hitState->objectHitMask = 16;
            *(int*)&hitState->skeletonHitMask = 16;
            hitState->hitVolumeId = 1;
            hitState->hitVolumePriority = 13;
            obj->anim.velocityY = -0.15f * timeDelta + obj->anim.velocityY;
            obj->anim.localPosY =
                obj->anim.velocityY * timeDelta + obj->anim.localPosY;
            if (obj->anim.localPosY < state->floorY + state->cfg->restOffsetY)
            {
                obj->anim.localPosY =
                    state->cfg->restOffsetY * obj->anim.rootMotionScale + state->floorY;
                state->mode = zcEn3_ROCKFALL_MODE_RESTING;
                if (state->cfg->landSfx != 0)
                {
                    Sfx_PlayFromObject((u32)obj, (u16)state->cfg->landSfx);
                }
            }
            break;
        case zcEn3_ROCKFALL_MODE_RESTING:
            *(int*)&hitState->objectHitMask = 16;
            *(int*)&hitState->skeletonHitMask = 16;
            hitState->hitVolumeId = 1;
            hitState->hitVolumePriority = 13;
            break;
        case zcEn3_ROCKFALL_MODE_SHATTERED:
            break;
        }

        if (*(void**)&hitState->lastHitObject != NULL)
        {
            hitState->flags &= ~1;
            state->mode = zcEn3_ROCKFALL_MODE_SHATTERED;
            Sfx_StopObjectChannel((int)obj, 8);
            if (obj->anim.seqId == CRROCKFALL_SEQ_QUARRY)
            {
                Sfx_PlayFromObject((u32)obj, SFXTRIG_mv_dinostomp1);
            }
            else
            {
                Sfx_PlayFromObject((u32)obj, SFXTRIG_jbike_bombbeep);
                spawnExplosion(obj, (f32)(u32)placement->scaleByte, 1, 1, 0, 1,
                                     1, 1, 1);
            }
        }
    }

    {
        f32 z = 0.0f;
        obj->anim.velocityX = z;
        obj->anim.velocityZ = z;
    }
}

void crrockfall_init(GameObject* obj, CrrockfallPlacement* params)
{
    CrRockfallState* state = obj->extra;
    ObjHitsPriorityState* hitState;
    ObjModelState* modelState;

    state->mode = zcEn3_ROCKFALL_MODE_ARMED;
    state->startY = obj->anim.localPosY;
    state->fallDelay = params->fallDelay;
    obj->anim.rootMotionScale = (f32)(u32)params->scaleByte / 127.0f;

    hitState = *(ObjHitsPriorityState**)&obj->anim.hitReactState;
    if (hitState != NULL)
    {
        f32 scale = obj->anim.rootMotionScale;
        ObjHitbox_SetCapsuleBounds((ObjAnimComponent*)obj, (int)((f32)hitState->primaryRadius * scale),
                                   (int)((f32)hitState->primaryCapsuleOffsetA * scale),
                                   (int)((f32)hitState->primaryCapsuleOffsetB * scale));
        ObjHits_DisableObject(obj);
    }

    modelState = obj->anim.modelState;
    if (modelState != NULL)
    {
        modelState->flags |= 0xb0;
        modelState->flags |= 0xc00;
        modelState->overrideWorldPosX = obj->anim.localPosX;
        modelState->overrideWorldPosZ = obj->anim.localPosZ;
        modelState->shadowScale = modelState->shadowScale * obj->anim.rootMotionScale;
    }

    if (obj->anim.seqId == CRROCKFALL_SEQ_BIG)
    {
        state->cfg = (CrRockfallCfgEntry*)&gRockfallCfgTable[0xc];
    }
    else
    {
        state->cfg = (CrRockfallCfgEntry*)gRockfallCfgTable;
    }
}

void crrockfall_release(void)
{
    if (gRockfallResource != NULL)
    {
        Resource_Release(gRockfallResource);
    }
    gRockfallResource = NULL;
}

void crrockfall_initialise(void)
{
    gRockfallResource = NULL;
}


u8 gRockfallCfgTable[] = {
    0x00, 0x00, 0x00, 0x67, 0x00, 0x00, 0x00, 0x00, 0x41, 0xA0, 0x00, 0x00,
    0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x03, 0xE3, 0x41, 0xF0, 0x00, 0x00,
};

ObjectDescriptor gCRrockfallObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)crrockfall_initialise,
    (ObjectDescriptorCallback)crrockfall_release,
    0,
    (ObjectDescriptorCallback)crrockfall_init,
    (ObjectDescriptorCallback)crrockfall_update,
    (ObjectDescriptorCallback)crrockfall_hitDetect,
    (ObjectDescriptorCallback)crrockfall_render,
    (ObjectDescriptorCallback)crrockfall_free,
    (ObjectDescriptorCallback)crrockfall_getObjectTypeId,
    crrockfall_getExtraSize,
};
