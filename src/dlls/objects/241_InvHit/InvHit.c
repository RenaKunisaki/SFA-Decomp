/*
 * InvHit (DLL 0xF1) - "invisible hit" volume objects of the pushable
 * effect family. One placement type drives several distinct hit-volume
 * behaviours selected by InvHitState.mode (def[0x1a], 0..7):
 *   0  proximity damage: scan the player (and Tricky) and bump the
 *      hit-priority counters once they fall inside userData2 range.
 *   1  attach to an owner object's hit list (ObjList_ContainsObject).
 *   2  passive shape/radius hit volume.
 *   3  publish the object's world position to lbl_803AC780 while the
 *      player exists.
 *   4  homing/tethered projectile: ease toward the owner's target,
 *      clamp to a growing reach around an anchor, spawn fx, and snap to
 *      ground via hitDetectFn_80065e50.
 *   5  like 3 but gated on the player having a lock-on target.
 *   6  fixed primary-radius hit volume.
 *   7  self-free once the owner's hit list no longer references it.
 * InvHit_free releases the expgfx source for mode 4.
 */
#include "dlls/objects/241_InvHit.h"
#include "main/dll/partfx_interface.h"
#include "main/dll_000A_expgfx.h"
#include "main/frame_timing.h"
#include "game/objects/object.h"
#include "game/objects/object_setup.h"
#include "sys/objects/lifecycle.h"
#include "sys/objects.h"
#include "main/object_render.h"
#include "main/obj_list.h"
#include "main/dll/player_target.h"
#include "main/vecmath.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_float_helpers.h"
#include "main/track_dolphin_api.h"

typedef struct InvHitState
{
    f32 anchorX;
    f32 anchorZ;
    u8 mode;
} InvHitState;

typedef struct InvhitObjectDef
{
    ObjPlacement head; /* 0x00 */
    u8 radius;       /* 0x18: primaryRadius / userData2 seed */
    u8 shapeFlags;   /* 0x19 */
    u8 mode;         /* 0x1a: InvHitState.mode selector */
    u8 pad1b[0x1C - 0x1B];
    void* anchorObj; /* 0x1c */
} InvhitObjectDef;

#define INVHIT_OBJFLAG_HIDDEN             0x4000
#define INVHIT_OBJFLAG_HITDETECT_DISABLED 0x2000

#define INVHIT_MODE_PROXIMITY_DAMAGE  0 /* scan player/Tricky, bump hit counters in range */
#define INVHIT_MODE_ATTACH            1 /* attach to owner's hit list */
#define INVHIT_MODE_PASSIVE_VOLUME    2 /* passive shape/radius hit volume */
#define INVHIT_MODE_PUBLISH_POS       3 /* publish world position while player exists */
#define INVHIT_MODE_HOMING_PROJECTILE 4 /* homing/tethered projectile toward owner target */
#define INVHIT_MODE_LOCKON_GATE       5 /* like publish, gated on player lock-on target */
#define INVHIT_MODE_FIXED_RADIUS      6 /* fixed primary-radius hit volume */
#define INVHIT_MODE_SELF_FREE         7 /* self-free once owner hit list drops it */

/* single-precision override for codegen */
f32 lbl_803AC780[4];

int InvHit_getExtraSize(void)
{
    return 0xc;
}
int InvHit_getObjectTypeId(void)
{
    return 0x0;
}
void InvHit_free(GameObject* obj)
{
    InvHitState* inner = obj->extra;
    switch (inner->mode)
    {
    case INVHIT_MODE_HOMING_PROJECTILE:
        (*gExpgfxInterface)->freeSource2((u32)obj);
        break;
    }
}
void InvHit_render(GameObject* obj, int a, int b, int c, int d)
{
    objRenderModelAndHitVolumes(obj, a, b, c, d, 1.0f);
}

void InvHit_hitDetect(void)
{
}


ObjectDescriptor gInvHitObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)InvHit_initialise,
    (ObjectDescriptorCallback)InvHit_release,
    0,
    (ObjectDescriptorCallback)InvHit_init,
    (ObjectDescriptorCallback)InvHit_update,
    (ObjectDescriptorCallback)InvHit_hitDetect,
    (ObjectDescriptorCallback)InvHit_render,
    (ObjectDescriptorCallback)InvHit_free,
    (ObjectDescriptorCallback)InvHit_getObjectTypeId,
    InvHit_getExtraSize,
};

void InvHit_update(GameObject* obj)
{
    InvHitState* state;
    GameObject* targetObj;

    state = obj->extra;
    obj->anim.previousLocalPosX = obj->anim.localPosX;
    obj->anim.previousLocalPosY = obj->anim.localPosY;
    obj->anim.previousLocalPosZ = obj->anim.localPosZ;
    switch (state->mode)
    {
    case INVHIT_MODE_PROXIMITY_DAMAGE:
    {
        GameObject* victim = Obj_GetPlayerObject();
        while (victim != NULL)
        {
            f32 dx = obj->anim.localPosX - victim->anim.localPosX;
            f32 dy = obj->anim.localPosY - victim->anim.localPosY;
            f32 dz = obj->anim.localPosZ - victim->anim.localPosZ;
            f32 dist = sqrtf(dx * dx + dy * dy + dz * dz);
            if (dist < (f32)obj->userData2)
            {
                ObjHitsPriorityState* victimHits = *(ObjHitsPriorityState**)&victim->anim.hitReactState;
                victimHits->priorityHitCount += 1;
                victimHits->flags = victimHits->flags & ~1;
                (*(ObjHitsPriorityState**)&obj->anim.hitReactState)->priorityHitCount += 1;
            }
            if (victim->anim.classId == 1)
            {
                victim = getTrickyObject();
            }
            else
            {
                victim = NULL;
            }
        }
        break;
    }
    case INVHIT_MODE_PUBLISH_POS:
        if (Obj_GetPlayerObject() != NULL)
        {
            lbl_803AC780[0] = obj->anim.worldPosX;
            lbl_803AC780[1] = obj->anim.worldPosY;
            lbl_803AC780[2] = obj->anim.worldPosZ;
        }
        break;
    case INVHIT_MODE_LOCKON_GATE:
    {
        void* pl = Obj_GetPlayerObject();
        u32 v = Player_GetTargetObject((int)pl);
        if (pl != NULL && v != 0)
        {
            lbl_803AC780[0] = obj->anim.worldPosX;
            lbl_803AC780[1] = obj->anim.worldPosY;
            lbl_803AC780[2] = obj->anim.worldPosZ;
        }
        break;
    }
    case INVHIT_MODE_ATTACH:
        ObjList_ContainsObject(obj->userData1);
        break;
    case INVHIT_MODE_SELF_FREE:
    {
        ObjHitsPriorityState* hitState = *(ObjHitsPriorityState**)&obj->anim.hitReactState;
        char* ownerHitSlot;
        char* ownerHitState = *(char**)(obj->userData1 + 0x54);
        int j;

        j = 0;
        ownerHitSlot = ownerHitState;
        for (; j < *(s8*)(ownerHitState + 0x71); j++)
        {
            if (*(GameObject**)(ownerHitSlot + 0x7c) == obj)
            {
                hitState->flags = hitState->flags & ~1;
                Obj_FreeObject(obj);
            }
            ownerHitSlot += 4;
        }
        break;
    }
    case INVHIT_MODE_HOMING_PROJECTILE:
    {
        ObjHitsPriorityState* hitState = *(ObjHitsPriorityState**)&obj->anim.hitReactState;
        TrackGroundHit** hits[2];
        f32 dx2;
        f32 dz2;
        f32 reach;
        int cnt;
        f32 thr;
        int i;

        obj->userData2 -= framesThisStep;
        if (*(void**)&hitState->lastHitObject != NULL)
        {
            hitState->flags = 0;
        }
        targetObj = *(GameObject**)&obj->userData1;
        if (targetObj != NULL)
        {
            f32 dx;
            f32 dz;
            f32 smoothTime;
            f32 qt;
            f32 dist;

            if (ObjList_ContainsObject((int)targetObj) == 0)
                break;
            dx = targetObj->anim.localPosX - obj->anim.localPosX;
            dz = targetObj->anim.localPosZ - obj->anim.localPosZ;
            smoothTime = 48.0f;
            qt = dx / smoothTime;
            obj->anim.localPosX = qt * timeDelta + obj->anim.localPosX;
            qt = dz / smoothTime;
            obj->anim.localPosZ = qt * timeDelta + obj->anim.localPosZ;
            dx = targetObj->anim.localPosX - state->anchorX;
            dz = targetObj->anim.localPosZ - state->anchorZ;
            reach = 10.0f + sqrtf(dx * dx + dz * dz);
            dx2 = obj->anim.localPosX - state->anchorX;
            dz2 = obj->anim.localPosZ - state->anchorZ;
            dist = sqrtf(dx2 * dx2 + dz2 * dz2);
            if (dist > reach)
            {
                f32 r = reach / dist;
                dx2 = dx2 * r;
                dz2 = dz2 * r;
                obj->anim.localPosX = state->anchorX + dx2;
                obj->anim.localPosZ = state->anchorZ + dz2;
            }
            (*gPartfxInterface)->spawnObject(obj, 0x25, NULL, 0, -1, NULL);
            (*gPartfxInterface)->spawnObject(obj, 0x56, NULL, 0, -1, NULL);
        }
        {
            s8 tmp =
                (s8)hitDetectFn_80065e50(obj, obj->anim.localPosX, obj->anim.localPosY,
                                         obj->anim.localPosZ, hits, 0, 0);
            i = 0;
            cnt = tmp;
        }
        thr = 20.0f;
        for (; i < cnt; i++)
        {
            f32 h = hits[0][i]->height;
            f32 oy = obj->anim.localPosY;
            if (h < thr + oy && h > oy - thr)
            {
                obj->anim.localPosY = h;
                i = cnt;
            }
        }
        break;
    }
    }
}

void InvHit_init(GameObject* obj, InvhitObjectDef* def)
{
    InvHitState* state = obj->extra;
    ObjHitsPriorityState* sub;

    state->mode = def->mode;
    sub = *(ObjHitsPriorityState**)&obj->anim.hitReactState;
    sub->flags = sub->flags & ~1;
    switch (state->mode)
    {
    case INVHIT_MODE_PROXIMITY_DAMAGE:
        obj->userData2 = def->radius;
        break;
    case INVHIT_MODE_FIXED_RADIUS:
        sub->shapeFlags = 1;
        sub->primaryRadius = 0x23;
        sub->flags = sub->flags | 0x45;
        sub->hitVolumePriority = 0xb;
        sub->hitVolumeId = 1;
        sub->activeHitboxMode = 0;
        sub->resetHitboxMode = 0;
        *(int*)&sub->objectHitMask = 0x10;
        *(int*)&sub->skeletonHitMask = 0x10;
        sub->lateralResponseWeight = 0;
        sub->axialResponseWeight = 0;
        break;
    case INVHIT_MODE_PUBLISH_POS:
        obj->userData2 = def->radius;
        obj->userData1 = 0;
        break;
    case INVHIT_MODE_LOCKON_GATE:
        obj->userData2 = def->radius;
        obj->userData1 = 0;
        break;
    case INVHIT_MODE_SELF_FREE:
        sub->shapeFlags = 1;
        sub->primaryRadius = def->radius;
        sub->flags = sub->flags | 0x45;
        sub->activeHitboxMode = 0;
        sub->hitVolumePriority = 0xa;
        sub->hitVolumeId = 0;
        sub->resetHitboxMode = 0;
        *(int*)&sub->objectHitMask = 0x10;
        *(int*)&sub->skeletonHitMask = 0x10;
        sub->lateralResponseWeight = 0;
        sub->axialResponseWeight = 0;
        break;
    case INVHIT_MODE_ATTACH:
        sub->shapeFlags = 1;
        sub->primaryRadius = def->radius;
        sub->flags = sub->flags | 0x45;
        sub->activeHitboxMode = 0;
        sub->hitVolumePriority = 0xb;
        sub->hitVolumeId = 1;
        sub->resetHitboxMode = 0;
        sub->hitVolumePriority = 0x11;
        sub->hitVolumeId = 1;
        *(int*)&sub->objectHitMask = 0x10;
        *(int*)&sub->skeletonHitMask = 0x10;
        sub->lateralResponseWeight = 0;
        sub->axialResponseWeight = 0;
        break;
    case INVHIT_MODE_PASSIVE_VOLUME:
        sub->shapeFlags = def->shapeFlags;
        sub->primaryRadius = def->radius;
        sub->flags = sub->flags | 1;
        sub->activeHitboxMode = 0;
        sub->resetHitboxMode = 0;
        sub->lateralResponseWeight = 0;
        sub->axialResponseWeight = 0;
        break;
    case INVHIT_MODE_HOMING_PROJECTILE:
        sub->shapeFlags = 1;
        sub->primaryRadius = 0xa;
        sub->flags = 3;
        *(int*)&sub->objectHitMask = 0x10;
        obj->userData2 = 0x78;
        {
            GameObject* anchorObj = *(GameObject**)&def->anchorObj;
            if (anchorObj != NULL)
            {
                state->anchorX = anchorObj->anim.localPosX;
                state->anchorZ = (*(GameObject**)&def->anchorObj)->anim.localPosZ;
            }
        }
        break;
    }
    obj->objectFlags =
        obj->objectFlags | (INVHIT_OBJFLAG_HIDDEN | INVHIT_OBJFLAG_HITDETECT_DISABLED);
}



void InvHit_release(void)
{
}


void InvHit_initialise(void)
{
}
