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

void Obj_SteerVelocityTowardVector(GameObject* obj, Vec3f* currentVelocity, Vec3f* desiredDirection, f32 maxSpeed,
                                   f32 maxSpeedDelta, f32 maxTurnAngle)
{
    f32 mtx[12];
    f32 n1[3];
    f32 n2[3];
    f32 cross[3];
    f32 mag1, mag2, t, ang;
    int gt;
    f64 gtf;

    mag1 = PSVECMag((const Vec*)currentVelocity);
    if (mag1 > lbl_803E6C38)
    {
        f32 inv = lbl_803E6C6C / mag1;
        n1[0] = currentVelocity->x * inv;
        n1[1] = currentVelocity->y * inv;
        n1[2] = currentVelocity->z * inv;
        PSVECNormalize((const Vec*)n1, (Vec*)n1);
    }
    else
    {
        n1[0] = lbl_803E6C38;
        n1[1] = lbl_803E6C38;
        n1[2] = lbl_803E6C38;
    }
    mag2 = PSVECMag((const Vec*)desiredDirection);
    if (mag2 > lbl_803E6C38)
    {
        f32 inv = lbl_803E6C6C / mag2;
        n2[0] = desiredDirection->x * inv;
        n2[1] = desiredDirection->y * inv;
        n2[2] = desiredDirection->z * inv;
    }
    else
    {
        n2[0] = lbl_803E6C38;
        n2[1] = lbl_803E6C38;
        n2[2] = lbl_803E6C38;
    }
    PSVECCrossProduct((const Vec*)n1, (const Vec*)n2, (Vec*)cross);
    if (PSVECMag((const Vec*)cross) > lbl_803E6C38)
    {
        ang = acosf_fast(PSVECDotProduct((const Vec*)n1, (const Vec*)n2));
        gt = (ang > maxTurnAngle);
        gtf = __fabs((f32)gt);
        if (gtf != lbl_803E6C38)
        {
            PSMTXRotAxisRad((MtxP)mtx, (const Vec*)cross,
                            maxTurnAngle * (ang > lbl_803E6C38 ? lbl_803E6C6C : lbl_803E6C70));
            PSMTXMultVecSR((MtxP)mtx, (const Vec*)n1, (Vec*)n2);
        }
    }
    t = mag2 * lbl_803E6C74;
    if (t > mag1 + maxSpeedDelta)
        t = mag1 + maxSpeedDelta;
    else if (t < mag1 - maxSpeedDelta)
        t = mag1 - maxSpeedDelta;
    if (t > maxSpeed)
        t = maxSpeed;
    obj->anim.velocityX = n2[0] * t;
    obj->anim.velocityY = n2[1] * t;
    obj->anim.velocityZ = n2[2] * t;
}
