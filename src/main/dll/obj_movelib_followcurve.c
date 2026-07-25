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


typedef struct ObjUpdateRomCurveFollowVelocityState
{
    u8 pad0[0x28C - 0x0];
    f32 velX;
    f32 velZ;
    u8 pad294[0x298 - 0x294];
} ObjUpdateRomCurveFollowVelocityState;

int Obj_UpdateRomCurveFollowVelocityIndexed(GameObject* obj, RomCurveWalker* route, f32 advanceStep,
                                            f32 arriveRadius, f32 speed, int flag, int* pickIdx)
{
    int result;
    f32 delta[3];
    f32 dist, ang;

    result = 0;
    delta[0] = obj->anim.localPosX - route->posX;
    delta[2] = obj->anim.localPosZ - route->posZ;
    dist = sqrtf(delta[0] * delta[0] + delta[2] * delta[2]);
    if (dist < arriveRadius)
    {
        if (Curve_AdvanceAlongPath(&route->curve, advanceStep) != 0 || route->atSegmentEnd != 0)
        {
            if ((*gRomCurveInterface)->goNextPointIndexed(route, *pickIdx) != 0)
                result = -1;
            else
                result = *(s8*)((int)route->node9C + 0x18);
            *pickIdx = 0;
        }
        speed = lbl_803E6C78 * advanceStep;
    }
    delta[0] = route->posX - obj->anim.localPosX;
    delta[1] = route->posY - obj->anim.localPosY;
    delta[2] = route->posZ - obj->anim.localPosZ;
    if ((u8)flag == 0)
    {
        ObjUpdateRomCurveFollowVelocityState* state = obj->extra;
        s16 raw;
        delta[0] = obj->anim.localPosX - route->posX;
        delta[2] = obj->anim.localPosZ - route->posZ;
        raw = (s16)getAngle(delta[0], delta[2]);
        ang = gBarrelGenPi * (f32)(-raw) / gBarrelGenAngleHalfRange;
        state->velZ = speed * -mathSinf(ang);
        state->velX = speed * -mathCosf(ang);
    }
    else
    {
        Obj_SteerVelocityTowardVector(obj, (Vec3f*)&obj->anim.velocityX, (Vec3f*)delta, speed,
                                      speed / lbl_803E6C7C, lbl_803E6C80);
    }
    return result;
}

int Obj_UpdateRomCurveFollowVelocity(GameObject* obj, RomCurveWalker* route, f32 advanceStep, f32 arriveRadius,
                                     f32 speed, int flag)
{
    int result;
    f32 delta[3];
    f32 dist, ang;

    result = 0;
    delta[0] = obj->anim.localPosX - route->posX;
    delta[2] = obj->anim.localPosZ - route->posZ;
    dist = sqrtf(delta[0] * delta[0] + delta[2] * delta[2]);
    if (dist < arriveRadius)
    {
        if (Curve_AdvanceAlongPath(&route->curve, advanceStep) != 0 || route->atSegmentEnd != 0)
        {
            if ((*gRomCurveInterface)->goNextPoint(route) != 0)
                result = -1;
            else
                result = *(s8*)((int)route->node9C + 0x18);
        }
        speed = lbl_803E6C78 * advanceStep;
    }
    delta[0] = route->posX - obj->anim.localPosX;
    delta[1] = route->posY - obj->anim.localPosY;
    delta[2] = route->posZ - obj->anim.localPosZ;
    if ((u8)flag == 0)
    {
        ObjUpdateRomCurveFollowVelocityState* state = obj->extra;
        s16 raw;
        delta[0] = obj->anim.localPosX - route->posX;
        delta[2] = obj->anim.localPosZ - route->posZ;
        raw = (s16)getAngle(delta[0], delta[2]);
        ang = gBarrelGenPi * (f32)(-raw) / gBarrelGenAngleHalfRange;
        state->velZ = speed * -mathSinf(ang);
        state->velX = speed * -mathCosf(ang);
    }
    else
    {
        Obj_SteerVelocityTowardVector(obj, (Vec3f*)&obj->anim.velocityX, (Vec3f*)delta, speed,
                                      speed / lbl_803E6C7C, lbl_803E6C80);
    }
    return result;
}
