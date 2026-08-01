/*
 * DLL 0x00DA - the homing pollen-cloud projectile/fragment
 * spawned by the pollen object. Each fragment picks one of five
 * PollenFragmentConfig presets by its pollen type (0..5), spawns a burst of
 * particle fx and a loop sfx on init, then per-frame steers toward the
 * nearest object in its target group, applies velocity damping/gravity,
 * optionally smooth-turns to face its velocity (or free-spins for the
 * 0x482 fragment object), and bursts (explosion fx + sfx) on contact with a
 * non-owner object. Timed variants fade their alpha out and self-free.
 */
#include "main/dll/partfx_interface.h"
#include "dolphin/mtx/vec.h"
#include "main/frame_timing.h"
#include "main/object_render.h"
#include "main/dll/dll_0282_barrelgener.h"
#include "main/dll/dll_00DA_pollenfragment_api.h"
#include "main/dll_000A_expgfx.h"
#include "main/obj_path.h"
#include "main/objfx.h"
#include "dlls/object_descriptor.h"
#include "main/model_light.h"
#include "main/objhits.h"
#include "main/objtype.h"
#include "sys/objects.h"
#include "main/audio/sfx_keep_alive_api.h"
#include "main/audio/sfx_limited_object_api.h"
#include "main/maketex_timer_api.h"
#include "main/objseq_api.h"
#include "main/vecmath.h"
#include "sys/objects/lifecycle.h"

f32 gPollenFragmentSpinRateX = 1024.0f;
f32 gPollenFragmentSpinRateY = 512.0f;

typedef struct
{
    s16 unk00;         /* 0x00 */
    s16 loopSfx;       /* 0x02 */
    s16 explodeSfx;    /* 0x04 */
    s16 unk06;         /* 0x06 */
    s16 burstFx;       /* 0x08 */
    s16 auraFx;        /* 0x0A */
    s16 unk0C;         /* 0x0C */
    s16 unk0E;         /* 0x0E */
    s16 targetGroup;   /* 0x10 */
    u8 noVertical : 1; /* 0x12 bit 7 */
    u8 timed : 1;      /* 0x12 bit 6 */
    u8 smoothTurn : 1; /* 0x12 bit 5 */
    u8 usePath : 1;    /* 0x12 bit 4 */
} PollenFragmentDef;

PollenFragmentConfig gPollenFragmentConfig0 = {
    0x0000, 0x049F, 0x00B9, 0x04BA, 0x04BA, -1, 0.2f, 0x0000, 0xC000,
};

PollenFragmentConfig gPollenFragmentConfig1 = {
    0x02FA, 0x02FB, 0x0496, 0x068F, 0x068F, 0x068F, 0.4f, 0x0026, 0x7000,
};

PollenFragmentConfig gPollenFragmentConfig2 = {
    0x02FA, 0x02FB, 0x0496, 0x068F, 0x068F, 0x068F, 0.4f, 0x0026, 0x2000,
};

PollenFragmentConfig gPollenFragmentConfig3 = {
    0x02FA, 0x02FB, 0x0496, 0x068F, 0x068F, -1, 0.2f, 0x0000, 0x2000,
};

PollenFragmentConfig gPollenFragmentConfig4 = {
    0x02FA, 0x02FB, 0x0496, 0x068F, 0x068F, 0x068F, 0.4f, 0x0026, 0x3000,
};

PollenFragmentConfig* gPollenFragmentConfigs[] = {
    &gPollenFragmentConfig0, &gPollenFragmentConfig1, &gPollenFragmentConfig2, &gPollenFragmentConfig3, &gPollenFragmentConfig4,
};

typedef struct PollenFragmentExtra
{
    int ownerObj; /* 0x00: owner captured on first update */
    f32 speed;    /* 0x04: steering speed factor */
    f32 timer;    /* 0x08: lifetime/strength timer */
    union {
        struct {
            f32 velX; /* 0x0C */
            f32 velY; /* 0x10 */
            f32 velZ; /* 0x14 */
        };
        Vec velocity;
    };
    ModelLightStruct* modelLight; /* 0x18 */
    PollenFragmentDef* def; /* 0x1C */
    f32 deathTimer;         /* 0x20 */
    f32 lifetimeTimer;      /* 0x24 */
} PollenFragmentExtra;

#define POLLENFRAGMENT_HIT_VOLUME_SLOT 0x16

int pollenfragment_getExtraSize(void)
{
    return 0x28;
}

int pollenfragment_getObjectTypeId(void)
{
    return 0x0;
}

void pollenfragment_free(GameObject* obj)
{
    PollenFragmentExtra* state = obj->extra;
    if (state->modelLight != NULL)
    {
        ModelLightStruct_free(state->modelLight);
        state->modelLight = NULL;
    }
    (*gExpgfxInterface)->freeSource2((u32)obj);
}

void pollenfragment_render(GameObject* obj, int p2, int p3, int p4, int p5)
{
    PollenFragmentExtra* state = obj->extra;
    if (timerIsActive(&state->deathTimer) != 0)
        return;
    objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, 1.0f);
}

void pollenfragment_hitDetect(GameObject* obj)
{
    u8* extra;
    int hitType;
    int hitObject;

    extra = *(u8**)&(obj)->extra;
    if (timerIsActive(&((PollenFragmentExtra*)extra)->deathTimer) == 0)
    {
        hitType = ObjHits_GetPriorityHit(obj, &hitObject, 0, 0);
        if (hitType == 0xe || hitType == 0xf)
        {
            if ((((PollenFragmentExtra*)extra)->def)->explodeSfx != -1)
            {
                spawnExplosion((GameObject*)(int)obj, 30.0f, 0, 1, 0, 1, 0, 1, 0);
                Sfx_PlayFromObjectLimited(
                    obj, (u16)(((PollenFragmentExtra*)extra)->def)->explodeSfx, 3);
            }
            ObjHits_DisableObject(obj);
            s16toFloat((f32*)(extra + 0x20), 0x78);
        }
        if (((ObjHitsPriorityState*)(obj)->anim.hitReactState)->contactFlags != 0)
        {
            ObjHits_DisableObject(obj);
            ((PollenFragmentExtra*)extra)->timer = 0.0f;
            if ((((PollenFragmentExtra*)extra)->def)->explodeSfx != -1)
            {
                spawnExplosion((GameObject*)(int)obj, 30.0f, 0, 1, 0, 1, 0, 1, 0);
                Sfx_PlayFromObjectLimited(
                    obj, (u16)(((PollenFragmentExtra*)extra)->def)->explodeSfx, 3);
            }
            s16toFloat((f32*)(extra + 0x20), 0x78);
        }
    }
}

void pollenfragment_update(GameObject* obj)
{
    u8* extra;
    GameObject* nearObj;
    PollenFragmentDef* def;
    void* hit;
    int i;
    f32 horizDamping;
    f32 t;
    Vec dir;
    Vec sc;
    Vec pos;

    extra = *(u8**)&obj->extra;
    if (getCurSeqNo() != 0)
    {
        Obj_FreeObject(obj);
        return;
    }
    if (timerIsActive(&((PollenFragmentExtra*)extra)->deathTimer) != 0)
    {
        if (timerCountDown((f32*)(extra + 0x20)) != 0)
        {
            Obj_FreeObject(obj);
        }
        return;
    }
    if (timerCountDown(&((PollenFragmentExtra*)extra)->lifetimeTimer) != 0)
    {
        s16toFloat((f32*)(extra + 0x20), 0x78);
    }
    if (*(void**)&obj->ownerObj != NULL)
    {
        ((PollenFragmentExtra*)extra)->ownerObj = *(int*)&obj->ownerObj;
        *(int*)&obj->ownerObj = 0;
    }
    if ((((PollenFragmentExtra*)extra)->def)->timed)
    {
        ((PollenFragmentExtra*)extra)->timer -= timeDelta;
        if (((PollenFragmentExtra*)extra)->timer <= 0.0f)
        {
            if (obj->anim.alpha == 0xff)
            {
                i = 2;
                do
                {
                    (*gPartfxInterface)
                        ->spawnObject((void*)obj, (int)(((PollenFragmentExtra*)extra)->def)->burstFx, NULL, 1, -1,
                                      NULL);
                } while (i-- != 0);
            }
            ((PollenFragmentExtra*)extra)->timer = 0.0f;
            if (obj->anim.alpha >= framesThisStep << 3)
            {
                obj->anim.alpha -= framesThisStep << 3;
            }
            else
            {
                obj->anim.alpha = 0;
                Obj_FreeObject(obj);
                return;
            }
        }
    }
    if ((((PollenFragmentExtra*)extra)->def)->auraFx != -1)
    {
        (*gPartfxInterface)
            ->spawnObject((void*)obj, (int)(((PollenFragmentExtra*)extra)->def)->auraFx, NULL, 1, -1, NULL);
    }
    nearObj = (GameObject*)((u8*)objGetNearestTypeTo((int)(((PollenFragmentExtra*)extra)->def)->targetGroup, obj, 0));
    if (nearObj != NULL &&
        (!(def = ((PollenFragmentExtra*)extra)->def)->timed || ((PollenFragmentExtra*)extra)->timer < 210.0f))
    {
        if (def->usePath)
        {
            ObjPath_GetPointWorldPosition(nearObj, 0, &pos.x, &pos.y, &pos.z, 0);
        }
        else
        {
            f32 prod;
            f32 quarter = 0.25f;
            pos.x = nearObj->anim.worldPosX;
            prod = nearObj->anim.hitboxScale * nearObj->anim.rootMotionScale;
            pos.y = prod * quarter + nearObj->anim.worldPosY;
            pos.z = nearObj->anim.worldPosZ;
        }
        PSVECSubtract(&pos, &obj->anim.worldPos, &dir);
        PSVECMag(&dir);
        PSVECNormalize(&dir, &dir);
        PSVECSubtract(&dir, &((PollenFragmentExtra*)extra)->velocity, &sc);
        *(f32*)((int)extra + 0xC) = dir.x;
        *(f32*)((int)extra + 0x10) = dir.y;
        *(f32*)((int)extra + 0x14) = dir.z;
        PSVECScale(&sc, &sc, 30.0f);
        PSVECAdd(&dir, &sc, &dir);
        obj->anim.velocityX =
            obj->anim.velocityX +
            ((30.0f + ((PollenFragmentExtra*)extra)->timer) * (dir.x * ((PollenFragmentExtra*)extra)->speed)) /
                210.0f;
        obj->anim.velocityZ =
            obj->anim.velocityZ +
            ((30.0f + ((PollenFragmentExtra*)extra)->timer) * (dir.z * ((PollenFragmentExtra*)extra)->speed)) /
                210.0f;
        if (!(((PollenFragmentExtra*)extra)->def)->noVertical)
        {
            obj->anim.velocityY =
                obj->anim.velocityY + ((30.0f + ((PollenFragmentExtra*)extra)->timer) *
                                                      (2.0f * (dir.y * ((PollenFragmentExtra*)extra)->speed))) /
                                                         210.0f;
        }
    }
    obj->anim.velocityX = obj->anim.velocityX * (horizDamping = 0.97f);
    obj->anim.velocityZ = obj->anim.velocityZ * horizDamping;
    obj->anim.velocityY *= 0.95f;
    if ((((PollenFragmentExtra*)extra)->def)->noVertical)
    {
        t = 0.04f * timeDelta;
        obj->anim.velocityY =
            obj->anim.velocityY - (t * ((PollenFragmentExtra*)extra)->timer) / 300.0f;
    }
    if ((((PollenFragmentExtra*)extra)->def)->smoothTurn)
    {
        Obj_SmoothTurnAnglesTowardVelocity(obj, (const Vec3f*)((int)obj + 0x24), 10, 0.0f,
                                           1.0f);
        obj->anim.rotZ = obj->anim.rotZ + framesThisStep * 0x500;
    }
    else if (obj->anim.romDefNo == POLLEN_FRAGMENT_OBJECT_ID)
    {
        t = 5.0f * gPollenFragmentSpinRateX;
        obj->anim.rotX = t * (f32)(u32)framesThisStep + (f32)(int)obj->anim.rotX;
        obj->anim.rotY =
            gPollenFragmentSpinRateY * (f32)(u32)framesThisStep + (f32)(int)obj->anim.rotY;
    }
    Sfx_KeepAliveLoopedObjectSound((int)obj, (u16)(((PollenFragmentExtra*)extra)->def)->loopSfx);
    objMove(obj, obj->anim.velocityX * timeDelta, obj->anim.velocityY * timeDelta,
            obj->anim.velocityZ * timeDelta);
    ObjHits_SetHitVolumeSlot(&obj->anim, POLLENFRAGMENT_HIT_VOLUME_SLOT, 1, 0);
    ObjHits_EnableObject(obj);
    hit = (void*)((ObjHitsPriorityState*)obj->anim.hitReactState)->lastHitObject;
    if (hit != NULL && ((GameObject*)hit)->anim.romDefNo != obj->anim.romDefNo &&
        hit != *(void**)&((PollenFragmentExtra*)extra)->ownerObj)
    {
        ((PollenFragmentExtra*)extra)->timer = 0.0f;
        ObjHits_DisableObject(obj);
        if ((((PollenFragmentExtra*)extra)->def)->explodeSfx != -1)
        {
            spawnExplosion(obj, 30.0f, 0, 1, 0, 1, 0, 1, 0);
            Sfx_PlayFromObjectLimited(
                obj, (u16)(((PollenFragmentExtra*)extra)->def)->explodeSfx, 3);
        }
        s16toFloat((f32*)(extra + 0x20), 0x78);
    }
}

void pollenfragment_init(GameObject* obj, int config)
{
    s8 pollenType;
    u32 randomValue;
    int spawnCount;
    u32* state;

    state = *(u32**)&(obj)->extra;
    if (*(char*)(config + 0x19) == '\x01')
    {
        ((PollenFragmentExtra*)state)->timer = 155.0f;
    }
    else
    {
        randomValue = randomGetRange(0xb4, 300);
        ((PollenFragmentExtra*)state)->timer = (f32)(int)randomValue;
    }
    pollenType = *(s8*)(config + 0x19);
    pollenType = (pollenType < 0) ? 0 : ((pollenType > 5u) ? 5 : pollenType);
    *(s8*)(config + 0x19) = pollenType;
    state[7] = (u32)gPollenFragmentConfigs[*(char*)(config + 0x19)];
    if ((int)*(short*)state[7] != 0)
    {
        Sfx_PlayFromObjectLimited(obj, (int)*(short*)state[7] & 0xffff, 3);
    }
    spawnCount = 4;
    do
    {
        (*gPartfxInterface)->spawnObject((void*)obj, (int)*(short*)(state[7] + 6), NULL, 1, -1, NULL);
    } while (spawnCount-- != 0);
    if (!((PollenFragmentDef*)state[7])->timed)
    {
        ((PollenFragmentExtra*)state)->timer = 60.0f;
    }
    ObjHits_SetTargetMask(obj, 4);
    ((PollenFragmentExtra*)state)->modelLight = NULL;
    ((PollenFragmentExtra*)state)->speed = *(f32*)(state[7] + 0xc);
    ((PollenFragmentExtra*)state)->ownerObj = 0;
    s16toFloat((f32*)(state + 9), 0xe10);
    storeZeroToFloatParam(&((PollenFragmentExtra*)state)->deathTimer);
}

void pollenfragment_release(void)
{
}

void pollenfragment_initialise(void)
{
}

ObjectDescriptor gPollenFragmentObjDescriptor = {
    0, 0, 0, OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)pollenfragment_initialise, (ObjectDescriptorCallback)pollenfragment_release, 0,
    (ObjectDescriptorCallback)pollenfragment_init, (ObjectDescriptorCallback)pollenfragment_update,
    (ObjectDescriptorCallback)pollenfragment_hitDetect, (ObjectDescriptorCallback)pollenfragment_render,
    (ObjectDescriptorCallback)pollenfragment_free, (ObjectDescriptorCallback)pollenfragment_getObjectTypeId,
    pollenfragment_getExtraSize,
};
