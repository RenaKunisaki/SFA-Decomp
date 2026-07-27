/*
 * Shared object movement / effect toolkit.
 *
 * A grab-bag of GameObject helpers linked into the main DOL and consumed by
 * the Drakor-area DLLs (bossdrakor, drakorhoverpad, drbarrelgr, drcloudrunner,
 * drlasercannon, hightop), babycloudrunner and the ArwingSquadron DLL: a
 * lightning-cluster spawner, a ballistic intercept predictor, voxel
 * world-line trace wrappers, a hit-light spawner, a velocity steering
 * integrator, the RomCurve follow-velocity drivers and a heading/roll/pitch
 * smoother.
 */
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "dolphin/mtx.h"
#include "main/audio/sfx.h"
#include "main/curve.h"
#include "main/frame_timing.h"
#include "main/gamebits.h"
#include "main/mm.h"
#include "main/model_light.h"
#include "main/objanim.h"
#include "main/obj_group.h"
#include "main/obj_query.h"
#include "main/objseq.h"
#include "main/vecmath.h"
#include "main/voxmaps.h"
#include "main/shader_api.h"
#include "main/dll/rom_curve_interface.h"
#include "main/maketex_timer_api.h"
#include "main/dll/dll_0282_barrelgener.h"
#include "main/dll/barrelgener_state.h"
#include "game/objects/object.h"
#include "sys/objects.h"
#include "main/newclouds.h"
#include "main/audio/sfx_ids.h"
#include "main/audio/sfx_trigger_ids.h"
#include "dlls/object_descriptor.h"
#include "dolphin/mtx/vec.h"
#include "main/objfx.h"

f32 lbl_803DC3A0 = 2.0f;
f32 lbl_803DC3A4 = 0.2f;
f32 lbl_803DC3A8 = 20.0f;
u16 lbl_803DC3AC = 0x40;

typedef struct ObjUpdateRomCurveFollowVelocityState
{
    u8 pad0[0x28C - 0x0];
    f32 velX;
    f32 velZ;
    u8 pad294[0x298 - 0x294];
} ObjUpdateRomCurveFollowVelocityState;

int Obj_UpdateLightningCluster(GameObject* obj, LightningEffect** entries, int count, f32 intensity,
                               ModelLight** light)
{
    int spawned;
    int i;
    f32 pos[3];

    spawned = 0;
    if (lbl_803E6C38 == intensity)
    {
        spawned = 0;
        for (i = 0; i < count; i++)
        {
            if (entries[i] != 0)
            {
                mm_free_(entries[i]);
                entries[i] = 0;
            }
        }
        if (*light != 0)
        {
            modelLightStruct_freeSlot(light);
        }
        return 0;
    }

    for (i = 0; i < count; i++)
    {
        if (entries[i] != 0)
        {
            lightningRender(entries[i]);
            entries[i]->timer += framesThisStep;
            if ((f32)(u32)entries[i]->timer > lbl_803DC3A8)
            {
                mm_free_(entries[i]);
                entries[i] = 0;
            }
        }
        else if (spawned == 0)
        {
            pos[0] = obj->anim.localPosX;
            pos[1] = obj->anim.localPosY;
            pos[2] = obj->anim.localPosZ;
            pos[0] += lbl_803E6C3C * (intensity * (f32)(int)(randomGetRange(0, 0x7d0) - 0x3e8));
            pos[1] += lbl_803E6C3C * (intensity * (f32)(int)(randomGetRange(0, 0x7d0) - 0x3e8));
            pos[2] += lbl_803E6C3C * (intensity * (f32)(int)(randomGetRange(0, 0x7d0) - 0x3e8));
            entries[i] =
                lightningCreate((const Vec3f*)&obj->anim.localPosX, (const Vec3f*)pos, lbl_803DC3A0,
                                           lbl_803DC3A4, lbl_803DC3A8, (u8)lbl_803DC3AC, 0);
            spawned = 1;
        }
    }

    if (*light == 0)
    {
        *light = modelLightStruct_createPointLight(obj, 0x80, 0x80, 0xff, 0);
        if (*light != 0)
        {
            modelLightStruct_setPosition(*light, lbl_803E6C38, intensity * lbl_803E6C40, lbl_803E6C38);
            modelLightStruct_setDistanceAttenuation(*light, intensity, lbl_803E6C44 + intensity);
        }
    }
    return 1;
}

int Obj_PredictInterceptPoint(GameObject* obj, f32 dt, const Vec3f* targetPos, Vec3f* outPos)
{
    f32 pos[3];
    f32 step[3];
    f32 vel[3];
    int gridOut[2];
    int gridB[2];
    int gridA[2];
    int i;

    if ((GameObject*)obj != Obj_GetPlayerObject())
    {
        PSVECSubtract((const Vec*)&(obj)->anim.localPosX, (const Vec*)&(obj)->anim.previousLocalPosX, (Vec*)vel);
    }
    else
    {
        vel[0] = (obj)->anim.velocityX;
        vel[1] = (obj)->anim.velocityY;
        vel[2] = (obj)->anim.velocityZ;
    }
    PSVECScale((const Vec*)vel, (Vec*)vel, oneOverTimeDelta);
    pos[0] = (obj)->anim.localPosX;
    pos[1] = lbl_803E6C58 + (obj)->anim.localPosY;
    pos[2] = (obj)->anim.localPosZ;
    for (i = 0; i < 5; i++)
    {
        PSVECScale((const Vec*)vel, (Vec*)step,
                   PSVECDistance((const Vec*)pos, (const Vec*)targetPos) / dt);
        PSVECAdd((const Vec*)&obj->anim.localPosX, (const Vec*)step, (Vec*)pos);
    }
    outPos->x = pos[0];
    outPos->y = pos[1];
    outPos->z = pos[2];
    voxmaps_worldToGrid((void*)targetPos, (s16*)gridA);
    voxmaps_worldToGrid(pos, (s16*)gridB);
    return voxmaps_traceLine((VoxPos*)gridA, (VoxPos*)gridB, (VoxPos*)gridOut, NULL, 0) != 0;
}

int voxmaps_traceWorldLine(void* startPos, void* endPos)
{
    int grid1[2];
    int grid2[2];
    int out[2];

    voxmaps_worldToGrid(startPos, (s16*)grid1);
    voxmaps_worldToGrid(endPos, (s16*)grid2);
    return voxmaps_traceLine((VoxPos*)grid1, (VoxPos*)grid2, (VoxPos*)out, NULL, 0);
}

void voxmaps_traceScaledVectorEnd(f32* out, void* origin, f32* dir, f32 scale)
{
    f32 endPos[3];
    f32 scaled[3];
    int gridA[2];
    int gridB[2];
    int gridOut[2];
    int e0;
    int e1;

    PSVECNormalize((const Vec*)dir, (Vec*)dir);
    PSVECScale((const Vec*)dir, (Vec*)scaled, scale);
    PSVECAdd((const Vec*)scaled, (const Vec*)origin, (Vec*)endPos);
    voxmaps_worldToGrid(origin, (s16*)gridA);
    voxmaps_worldToGrid(endPos, (s16*)gridB);
    if (voxmaps_traceLine((VoxPos*)gridA, (VoxPos*)gridB, (VoxPos*)gridOut, NULL, 0) == 0)
        voxmaps_gridToWorld(endPos, (s16*)gridOut);
    *(Vec3f*)out = *(Vec3f*)endPos;
}

void Obj_SpawnHitLightAndFade(GameObject* obj, const Vec3f* pos, f32 scale)
{
    struct
    {
        f32 _pad[3];
        f32 vec[3];
    } s;

    s.vec[0] = pos->x + playerMapOffsetX;
    s.vec[1] = pos->y;
    s.vec[2] = pos->z + playerMapOffsetZ;
    objLightFn_8009a1dc(obj, lbl_803E6C68, &s, 1, 0);
    Obj_SetModelColorFadeRecursive(obj, 0x5a, 0xc8, 0, 0, 1);
}