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

void Obj_SmoothTurnAnglesTowardVelocity(GameObject* obj, const Vec3f* velocity, int turnFrames, f32 rollFactor,
                                        f32 pitchFactor)
{
    ObjAnimComponent* anim = &obj->anim;
    f32 rate;
    f32 delta;
    f32 clamped;
    f32 dist;
    int rotZ;

    rate = timeDelta / (f32)(u32)(u16)turnFrames;
    if (rate > lbl_803E6C6C)
    {
        rate = lbl_803E6C6C;
    }

    delta = (f32)(int)((u16)getAngle(-velocity->x, -velocity->z) - (u16)anim->rotX);
    if (delta > gBarrelGenAngleHalfRange)
    {
        delta = gBarrelGenAngleWrapNeg + delta;
    }
    if (delta < gBarrelGenAngleWrapThreshold)
    {
        delta = gBarrelGenAngleWrapPos + delta;
    }
    delta *= rate;
    clamped = (delta < gBarrelGenTurnRateClampMin)
                  ? gBarrelGenTurnRateClampMin
                  : ((delta > gBarrelGenTurnRateClampMax) ? gBarrelGenTurnRateClampMax : delta);
    anim->rotX += (int)clamped;

    if (rollFactor != lbl_803E6C38)
    {
        anim->rotZ = (s16)(lbl_803E6C98 * (f32)anim->rotZ);
        anim->rotZ = (s16)(oneOverTimeDelta * (lbl_803E6C5C * (clamped * rollFactor)) + (f32)anim->rotZ);
        rotZ = anim->rotZ;
        if (rotZ < -0x2000)
        {
            rotZ = -0x2000;
        }
        else if (rotZ > 0x2000)
        {
            rotZ = 0x2000;
        }
        anim->rotZ = rotZ;
    }

    if (lbl_803E6C38 != pitchFactor)
    {
        {
            f32 xx = velocity->x * velocity->x;
            f32 zz = velocity->z * velocity->z;
            dist = sqrtf(xx + zz);
        }
        delta = (f32)(int)((u16)getAngle(velocity->y * pitchFactor, dist) - (u16)anim->rotY);
        if (delta > gBarrelGenAngleHalfRange)
        {
            delta = gBarrelGenAngleWrapNeg + delta;
        }
        if (delta < gBarrelGenAngleWrapThreshold)
        {
            delta = gBarrelGenAngleWrapPos + delta;
        }
        anim->rotY += (int)(delta * rate);
    }
}