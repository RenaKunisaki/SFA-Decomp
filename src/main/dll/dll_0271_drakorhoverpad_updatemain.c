/*
 * drakorhoverpad (DLL 0x271) - a rideable hover-pad object in the
 * Drakor (DR) levels that follows a ROM spline/curve network.
 *
 * initMain seeds the pad onto its curve and selects a behaviour mode
 * from its placement subtype; updateMain advances the pad along the
 * active curve each step, applying a sinusoidal vertical bob, banking
 * the model toward its travel direction, and steering the object
 * toward the curve sample point. update() picks the next path point in
 * the network (masked vs unmasked branch) and recomputes the per-node
 * velocity/tangent data. handlePathPointEvent dispatches the per-node
 * event ids: speed flips, state changes, camera shake / view offset
 * while the player is riding, and the game bits that gate the ride.
 * render emits the trailing particle spray on a frame cadence.
 *
 * Curve/velocity state lives in the object's extra block
 * (DrakorHoverpadState, 0x17c bytes); the two flag bytes at 0x178/0x179
 * are DrakorHoverpadFlags / DrakorHoverpadPathFlags.
 */
#include "main/dll/dll_0271_drakorhoverpad.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "string.h"
#include "main/curve.h"
#include "dolphin/mtx/mtx_legacy.h"
#include "main/camera.h"
#include "main/camera_shake_api.h"
#include "main/frame_timing.h"
#include "main/gamebits_api.h"
#include "main/obj_group.h"
#include "main/obj_path.h"
#include "main/obj_query.h"
#include "main/objhits.h"
#include "main/object_api.h"
#include "main/object_render.h"
#include "main/objprint_api.h"
#include "main/vecmath.h"
#include "main/audio/sfx_play_api.h"
#include "main/dll/dll_0282_barrelgener.h"
#include "main/dll/rom_curve_interface.h"
#include "main/game_object.h"
#include "main/audio/sfx_ids.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/object_descriptor.h"
#include "main/dll/dll_0271_drakorhoverpad_internal.h"

static inline void drakorhoverpad_initPathCurve(GameObject* obj, u8* p)
{
    int curveArg = 0x2a;

    (*gRomCurveInterface)->initCurve(&((DrakorHoverpadState*)p)->curve, (void*)obj, 300.0f, &curveArg, -1);
    Curve_AdvanceAlongPath((Curve*)(p + 4), 0.01f);
}

void drakorhoverpad_updateMain(GameObject* obj)
{
    u8* p = (obj)->extra;
    RomCurveWalker* curve;
    DrakorHoverpadUpdateMainPlacement* q = (DrakorHoverpadUpdateMainPlacement*)(obj)->anim.placementData;
    DrakorHoverpadFlags* f = (DrakorHoverpadFlags*)(p + 0x178);
    DrakorHoverpadPathFlags* g = (DrakorHoverpadPathFlags*)(p + 0x179);
    int evOut;
    f32 diff[3];
    f32 curvePos[3];
    int curveArg;
    f32 phase;
    f32 wobbleY;
    f32 limit;
    f32 absH;
    f32 absV;
    GameObject* nearest;
    s16 yawDelta;
    int c;
    int angle;
    int clamped;
    f32 spd;

    Obj_GetPlayerObject();
    if (drakorhoverpad_init(obj) != 0)
    {
        return;
    }
    if (f->bit20 == 0)
    {
        f->bit20 = mainGetBit(q->activateGameBit);
        ((DrakorHoverpadUpdateMainState*)p)->targetSpeed = 0.0f;
        if (f->bit20 != 0)
        {
            drakorhoverpad_initPathCurve(obj, p);
            (obj)->anim.localPosX = ((DrakorHoverpadState*)p)->curve.posX;
            (obj)->anim.localPosY = ((DrakorHoverpadState*)p)->curve.posY;
            (obj)->anim.localPosZ = ((DrakorHoverpadState*)p)->curve.posZ;
            ((DrakorHoverpadState*)p)->commandSpeed = (*(f32*)&gDrakorHoverpadSpeedStep);
            Sfx_PlayFromObject((int)obj, SFXTRIG_id_308);
            Sfx_PlayFromObject((int)obj, SFXTRIG_id_30a);
        }
        return;
    }
    curve = &((DrakorHoverpadState*)p)->curve;
    if (g->f08 != 0)
    {
        angle = (s16)getAngle(sqrtf(curve->tangentX * curve->tangentX + curve->tangentZ * curve->tangentZ),
                              curve->tangentY);
        phase = 3.1415927f * (f32)angle / 32768.0f;
        wobbleY = -0.7f * mathCosf(phase);
        limit = 0.1f * (0.7f * mathSinf(phase));
        if (f->b40 != 0)
        {
            absH = (((DrakorHoverpadState*)p)->commandSpeed >= 0.0f) ? ((DrakorHoverpadState*)p)->commandSpeed : -((DrakorHoverpadState*)p)->commandSpeed;
            absV = (((DrakorHoverpadUpdateMainState*)p)->verticalVel >= 0.0f)
                       ? ((DrakorHoverpadUpdateMainState*)p)->verticalVel
                       : -((DrakorHoverpadUpdateMainState*)p)->verticalVel;
            if (absV > (*(f32*)&gDrakorHoverpadSpeedStep) + absH)
            {
                limit = limit + (*(f32*)&gDrakorHoverpadSpeedStep);
            }
        }
        if (f->state != 0)
        {
            limit = limit + (*(f32*)&gDrakorHoverpadSpeedStep);
        }
        ((DrakorHoverpadUpdateMainState*)p)->verticalVel = ((DrakorHoverpadUpdateMainState*)p)->targetSpeed +
                                                           (((DrakorHoverpadUpdateMainState*)p)->verticalVel + wobbleY);
        absV = ((DrakorHoverpadUpdateMainState*)p)->verticalVel;
        absH = (absV >= 0.0f) ? absV : -absV;
        if (absH < limit)
        {
            ((DrakorHoverpadUpdateMainState*)p)->verticalVel = ((DrakorHoverpadState*)p)->commandSpeed;
        }
        else
        {
            ((DrakorHoverpadUpdateMainState*)p)->verticalVel += (absV > ((DrakorHoverpadState*)p)->commandSpeed) ? -limit : limit;
        }
        ObjHits_SetHitVolumeSlot((ObjAnimComponent*)obj, DRAKORHOVERPAD_HIT_VOLUME_SLOT, 1, 0);
    }
    else
    {
        ObjHits_DisableObject(obj);
        ((DrakorHoverpadUpdateMainState*)p)->verticalVel = ((DrakorHoverpadState*)p)->commandSpeed;
        lbl_803DC2F8 = (*(f32*)&gDrakorHoverpadSpeedStep) * ((DrakorHoverpadState*)p)->commandSpeed;
    }
    if (((DrakorHoverpadUpdateMainState*)p)->verticalVel < 0.0f)
    {
        (*gRomCurveInterface)->setClosed((RomCurveWalker*)(p + 4), 1);
    }
    else
    {
        (*gRomCurveInterface)->setClosed((RomCurveWalker*)(p + 4), 0);
    }
    ((DrakorHoverpadUpdateMainState*)p)->targetSpeed = 0.0f;
    if (0.0f != ((DrakorHoverpadUpdateMainState*)p)->verticalVel)
    {
        Curve_AdvanceAlongPath(&curve->curve, ((DrakorHoverpadUpdateMainState*)p)->verticalVel);
        c = curve->reverse;
        if ((c == 0 && curve->atSegmentEnd != 0) || (c != 0 && curve->atSegmentEnd == 0))
        {
            if (drakorhoverpad_handlePathPointEvent(obj, *(u8*)((u8*)curve->nodeA0 + 0x18),
                                                    *(u8*)((u8*)curve->nodeA4 + 0x18), &evOut) != 0)
            {
                drakorhoverpad_update(curve, evOut);
            }
        }
    }
    curvePos[0] = curve->posX;
    curvePos[1] = curve->posY;
    curvePos[2] = curve->posZ;
    curvePos[1] = curvePos[1] + (1.0f + mathSinf(3.1415927f *
                                                         (f32)(int)((DrakorHoverpadUpdateMainState*)p)->anglePhase /
                                                         32768.0f));
    ((DrakorHoverpadUpdateMainState*)p)->anglePhase =
        (s16)(((DrakorHoverpadUpdateMainState*)p)->anglePhase + framesThisStep * 0x320);
    if (g->f10 != 0)
    {
        nearest = (GameObject*)ObjGroup_FindNearestObject(BOSSDRAKOR_OBJGROUP, obj, 0);
        if (nearest != NULL)
        {
            yawDelta = Obj_GetYawDeltaToObject(obj, nearest, 0);
            if (yawDelta < -0x200)
            {
                yawDelta = -0x200;
            }
            else if (yawDelta > 0x200)
            {
                yawDelta = 0x200;
            }
            c = (s16)yawDelta;
            (obj)->anim.rotX += (s16)c;
            if ((obj)->anim.rotY != 0)
            {
                yawDelta = (obj)->anim.rotY;
                if (yawDelta < -0x100)
                {
                    yawDelta = -0x100;
                }
                else if (yawDelta > 0x100)
                {
                    yawDelta = 0x100;
                }
                (obj)->anim.rotY -= (s16)yawDelta;
            }
            (obj)->anim.rotZ = (s16)(c * lbl_803DC2FC);
        }
    }
    else
    {
        s16 yawDelta;
        phase = sqrtf(curve->tangentX * curve->tangentX + curve->tangentZ * curve->tangentZ);
        yawDelta = (s16)(getAngle(curve->tangentX, curve->tangentZ) + 0x8000) - (obj)->anim.rotX;
        (obj)->anim.rotY = (s16)getAngle(curve->tangentY, phase);
        if (yawDelta < -0x800)
        {
            clamped = -0x800;
        }
        else if (yawDelta > 0x800)
        {
            clamped = 0x800;
        }
        else
        {
            clamped = yawDelta;
        }
        c = (s16)clamped;
        (obj)->anim.rotZ = (s16)((((DrakorHoverpadUpdateMainState*)p)->verticalVel < 0.0f) ? c : -c);
        (obj)->anim.rotX += (s16)((c < -0x100) ? -0x100 : (c > 0x100) ? 0x100 : c);
        c = (obj)->anim.rotY;
        if (c < -0x64)
        {
            c = -0x64;
        }
        else if (c > 0x64)
        {
            c = 0x64;
        }
        (obj)->anim.rotY = c;
    }
    PSVECSubtract(curvePos, &(obj)->anim.localPosX, diff);
    /* snapshot the shared steer speed before building the call args (the
     * through-pointer read keeps the load at this statement) */
    spd = lbl_803DC2F8;
    Obj_SteerVelocityTowardVector(obj, (Vec3f*)&obj->anim.velocityX, (Vec3f*)diff, spd, spd / 30.0f,
                                  0.3f);
    PSVECAdd(&(obj)->anim.localPosX, &(obj)->anim.velocityX, &(obj)->anim.localPosX);
}

void drakorhoverpad_initMain(GameObject* obj, void* desc)
{
    u8* p = (obj)->extra;
    DrakorHoverpadFlags* f = (DrakorHoverpadFlags*)(p + 0x178);
    DrakorHoverpadPathFlags* g = (DrakorHoverpadPathFlags*)(p + 0x179);
    DrakorHoverpadUpdateMainPlacement* d = (DrakorHoverpadUpdateMainPlacement*)desc;
    f32 initialSpeed;

    (obj)->anim.rotX = (s16)(d->rotXByte << 8);
    ((DrakorHoverpadState*)p)->unk118 = (f32)d->unk1a;
    initialSpeed = 0.0f;
    ((DrakorHoverpadState*)p)->speed = initialSpeed;
    f->bit20 = 0;
    f->b40 = 1;
    ((DrakorHoverpadState*)p)->unk170 = 0;
    ((DrakorHoverpadState*)p)->unk11C = initialSpeed;
    ((DrakorHoverpadState*)p)->unk120 = initialSpeed;
    ((DrakorHoverpadState*)p)->frameCounter = 0;
    switch (d->subtype)
    {
    case DRAKORHOVERPAD_SUBTYPE_TRACKING:
        g->f10 = 1;
        g->f04 = 1;
        g->f08 = 0;
        break;
    case DRAKORHOVERPAD_SUBTYPE_FREE:
        g->f10 = 0;
        g->f04 = 0;
        g->f08 = 1;
        break;
    }
    ObjGroup_AddObject((int)obj, DRAKORHOVERPAD_OBJGROUP);
    ObjGroup_AddObject((int)obj, DRAKORHOVERPAD_OBJGROUP_SECONDARY);
}

void drakorhoverpad_release(void)
{
}

void drakorhoverpad_initialise(void)
{
}

u8 lbl_8032AAB0[0x80] = {
    0x04, 0x30, 0x0B, 0x00, 0x03, 0x00, 0x04, 0x31, 0x05, 0x00, 0x02, 0x00, 0x04, 0x32, 0x0B, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x02,
    0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x05,
    0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03,
    0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x05,
    0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x02,
    0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x02,
    0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x05,
};
