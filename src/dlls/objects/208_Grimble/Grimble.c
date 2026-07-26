/*
 * grimble (DLL 0x00D0) - the path-following SharpClaw "grimble" baddie
 * (object type id 0x59), built on the shared ground-baddie / player-aware
 * control framework (gBaddieControlInterface, gPlayerInterface).
 *
 * Each grimble locks onto a nearby path object (one of the type-0x17 group
 * scanned in grimble_attachNearestPath) and walks its GrimbleControl sub-state
 * through GroundBaddieState.control: it tracks pathProgress, derives
 * facing from the sampled path tangent (getAngle), and clamps progress to
 * the path's [0.3f, 6.7f] bounds. State handlers A00-A02
 * (registered in gGrimbleStateHandlersA/B and driven by the player
 * interface) implement patrol, edge-turn and pursuit moves; reaching a path
 * edge or losing line-of-sight to the target flips the reversed flag and
 * picks a new randomized targetProgress. The render pass spawns bone/object
 * particle effects from the unk400 fx-flag bits. When the object is asleep
 * (userData1 != 0) update wakes it from the saved map-event time slot.
 */
#include "game/objects/object.h"
#include "main/dll/objfx_api.h"
#include "main/obj_group.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/dll/chukchukstate_struct.h"
#include "main/dll/grimble_state.h"
#include "main/audio/sfx_ids.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/audio/sfx_play_api.h"
#include "main/dll/baddie_state.h"
#include "main/dll/boneparticleeffect_interface.h"
#include "main/dll/dll_00D0_grimble.h"
#include "main/dll/dll_80161130.h"
#include "main/mapEventTypes.h"
#include "sys/objects.h"
#include "sys/objects/lifecycle.h"
#include "main/vecmath.h"
#include "main/object_render.h"
#include "main/audio/sfx.h"
#include "main/objhits.h"
#include "main/gamebits.h"
#include "main/frame_timing.h"
#include "main/player_control_interface.h"
#include "main/dll/baddie_control_interface.h"
#include "main/dll/dll_00CD_iceball.h"

/* object group this object belongs to */
#define GRIMBLE_OBJGROUP    3
#define DFROPENODE_OBJGROUP 0x17 /* DLL 0x175 dfropenode (path nodes) */

STATIC_ASSERT(sizeof(ChukChukState) == 0x18);
STATIC_ASSERT(offsetof(ChukChukState, flags) == 0x12);

extern void* gGrimbleStateHandlersA[10];
extern void* gGrimbleStateHandlersB[6];

int grimble_stateHandlerB05(int* obj, GroundBaddieState* state)
{
    GroundBaddieState* sub = ((GameObject*)obj)->extra;
    if ((s8)state->baddie.moveJustStartedB != 0)
    {
        sub->subMode = 0;
        mainSetBits(sub->gameBitB, 0);
        mainSetBits(sub->gameBitA, 1);
    }
    return 0;
}

int grimble_stateHandlerB04(int* obj, GroundBaddieState* state)
{
    if ((s8)state->baddie.moveJustStartedB != 0)
    {
        (*gPlayerInterface)->setState(obj, state, 8);
        state->baddie.targetObj = NULL;
        state->baddie.physicsActive = 0;
        state->baddie.hasTarget = 0;
        ObjHits_DisableObject((GameObject*)obj);
        *(u8*)&((GameObject*)obj)->anim.resetHitboxMode |= INTERACT_FLAG_DISABLED;
    }
    if (((GameObject*)obj)->anim.alpha == 0)
    {
        if (((GameObject*)obj)->anim.placementData == NULL)
        {
            Obj_FreeObject((GameObject*)obj);
            return 0;
        }
        return 6;
    }
    return 0;
}

int grimble_stateHandlerB03(int obj, GroundBaddieState* state)
{
    if ((s8)state->baddie.hitPoints < 1)
        return 5;
    return 1;
}

int scarab_updateProximityGate(int* obj, GroundBaddieState* state)
{
    int* target;
    f32 dx;
    f32 dz;
    f32 magAbs;
    u32 rel;

    target = *(int**)&state->baddie.targetObj;
    if (target == NULL)
    {
        (*gPlayerInterface)->setState(obj, state, 0);
        return 1;
    }
    if (state->baddie.controlMode != 6)
    {
        dx = ((GameObject*)obj)->anim.localPosX - ((GameObject*)target)->anim.localPosX;
        dz = ((GameObject*)obj)->anim.localPosZ - ((GameObject*)target)->anim.localPosZ;
        rel = (getAngle(dx, dz) - *(s16*)obj) & 0xffff;
        if (rel > 0x4000 && rel < 0xc000)
        {
            dx = -100.0f;
        }
        else
        {
            dx = sqrtf(dx * dx + dz * dz) - 45.0f;
        }
        magAbs = dx < 0.0f ? -dx : dx;
        if (magAbs < 1.0f &&
            (state->baddie.controlMode == 1 || (state->baddie.controlMode == 5 && (s8)state->baddie.moveDone != 0)))
        {
            (*gPlayerInterface)->setState(obj, state, 6);
        }
        else if (state->baddie.controlMode != 1)
        {
            if (dx > 2.5f)
            {
                if (state->baddie.controlMode != 4 && (state->baddie.controlMode != 5 || (s8)state->baddie.moveDone != 0))
                {
                    (*gPlayerInterface)->setState(obj, state, 1);
                }
            }
            if (dx < -2.5f)
            {
                (*gPlayerInterface)->setState(obj, state, 1);
            }
        }
        if (state->baddie.controlMode == 1)
        {
            state->baddie.moveSpeed = (dx > 0.0f) ? 0.04f : -0.07f;
        }
    }
    return 0;
}

int grimble_stateHandlerB01(int* obj, GroundBaddieState* state)
{
    if ((s8)state->baddie.moveJustStartedB != 0)
    {
        (*gPlayerInterface)->setState(obj, state, 9);
    }
    if ((s8)state->baddie.moveDone != 0)
    {
        return 1;
    }
    return 0;
}

int grimble_stateHandlerB00(int obj, GroundBaddieState* p)
{
    u16 a;
    u16 b;
    u16 c;

    if (p->baddie.targetObj != NULL && p->baddie.controlMode != 2)
    {
        if ((f32)p->baddie.stateTimer > 4.0f * timeDelta)
        {
            (*gBaddieControlInterface)
                ->getTargetGeometry((GameObject*)obj, (GameObject*)p->baddie.targetObj, 16, &a, &b, &c);
            if (a < 4 || a > 11)
            {
                return 3;
            }
            (*gPlayerInterface)->setState((void*)obj, p, 2);
            p->baddie.moveSpeed = 0.028f;
            p->baddie.moveDone = 0;
        }
    }
    return 0;
}
int grimble_stateHandlerA09(GameObject* obj, GroundBaddieState* p)
{
    GroundBaddieState* sub;
    f32 spd;

    sub = (obj)->extra;
    p->baddie.stateTag = 0;
    p->baddie.moveSpeed = 0.01f;
    spd = 0.0f;
    p->baddie.animSpeedA = spd;
    p->baddie.animSpeedB = spd;
    if ((s8)p->baddie.moveJustStartedA != 0)
    {
        Sfx_PlayFromObject((u32)obj, SFXTRIG_dn_seal4_c_27c);
        if ((s8)p->baddie.moveJustStartedA != 0)
        {
            ObjAnim_SetCurrentMove((int)obj, 2, 0.0f, 0);
            p->baddie.moveDone = 0;
        }
        p->baddie.moveSpeed = 0.025f;
        p->baddie.moveDone = 0;
        (obj)->anim.alpha = 0xff;
        sub->flags400 |= 0x100;
    }
    if ((s8)p->baddie.moveDone != 0)
    {
        return 1;
    }
    return 0;
}

int grimble_stateHandlerA08(int* obj, GroundBaddieState* state)
{
    GroundBaddieState* sub = ((GameObject*)obj)->extra;
    if ((s8)state->baddie.moveJustStartedA != 0)
    {
        ObjAnim_SetCurrentMove((int)obj, 8, 0.0f, 0);
        state->baddie.moveDone = 0;
    }
    state->baddie.moveSpeed = 0.048f;
    if (((s32)state->baddie.eventFlags & BADDIE_EVENT_LANDING) != 0)
    {
        Sfx_PlayFromObject((u32)obj, SFXTRIG_wp_iceywindlp16_233);
        state->baddie.eventFlags &= ~BADDIE_EVENT_LANDING;
        (*gBaddieControlInterface)->spawnChild((GameObject*)obj, sub->triggerId, -1, 1);
    }
    return 0;
}

int grimble_stateHandlerA07(short* obj, GroundBaddieState* p)
{
    GrimbleControl* ctrl;
    s16 yaw;
    int diff;
    f32 spd;

    ctrl = ((GroundBaddieState*)((GameObject*)obj)->extra)->control;
    if ((s8)p->baddie.moveJustStartedA != 0)
    {
        ObjAnim_SetCurrentMove((int)obj, 7, 0.0f, 0);
        p->baddie.moveDone = 0;
    }
    if ((s8)p->baddie.moveJustStartedA != 0)
    {
        Sfx_PlayFromObject((u32)obj, SFXTRIG_ms_windlift_loop);
    }
    p->baddie.moveSpeed = 0.018f;
    yaw = ctrl->baseRotX;
    diff = *obj - (yaw & 0xffff);
    if (diff > 0x8000)
    {
        diff -= 0xffff;
    }
    if (diff < -0x8000)
    {
        diff += 0xffff;
    }
    *obj = yaw;
    if (diff > 0x3ffc || diff < -0x3ffc)
    {
        *obj += 0x8000;
    }
    spd = 0.0f;
    p->baddie.animSpeedA = spd;
    p->baddie.animSpeedB = spd;
    if ((s8)p->baddie.moveDone != 0)
    {
        return 1;
    }
    return 0;
}

int grimble_stateHandlerA06(GameObject* obj, GroundBaddieState* p, f32 spd)
{
    GrimbleControl* ctrl;
    f64 d;
    f32 horizRun;
    struct
    {
        f32 x, y, z;
    } b;
    struct
    {
        f32 x, y, z;
    } a;

    ctrl = ((GroundBaddieState*)(obj)->extra)->control;
    ((ObjHitsPriorityState*)(obj)->anim.hitReactState)->hitVolumePriority = 9;
    ((ObjHitsPriorityState*)(obj)->anim.hitReactState)->hitVolumeId = 1;
    ObjHits_RegisterActiveHitVolumeObject(obj);
    if (randomGetRange(0, 100) < 50)
    {
        if ((s8)p->baddie.moveJustStartedA != 0)
        {
            ObjAnim_SetCurrentMove((int)obj, 1, 0.0f, 0);
            p->baddie.moveDone = 0;
        }
    }
    else if ((s8)p->baddie.moveJustStartedA != 0)
    {
        ObjAnim_SetCurrentMove((int)obj, 4, 0.0f, 0);
        p->baddie.moveDone = 0;
    }
    p->baddie.moveSpeed = 0.03f;
    (*gPlayerInterface)->updateAnimRootMotion(obj, p, spd, 1);
    /* advance pathProgress along the path (pathObj vtable +0x28) by
     * animSpeedA, sign-flipped when the path is walked reversed */
    (*(void (**)(void*, void*, f32))(**(int**)(ctrl->pathObj + 0x68) + 0x28))(
        *(void**)&ctrl->pathObj, &ctrl->pathProgress, p->baddie.animSpeedA*(f32)(1 - (ctrl->reversed << 1)));
    if (ctrl->pathProgress < 0.3f)
    {
        ctrl->pathProgress = 0.3f;
    }
    else if (ctrl->pathProgress > 6.7f)
    {
        ctrl->pathProgress = 6.7f;
    }
    /* pitch to the local path slope: sample the curve (pathObj vtable +0x24)
     * just behind and ahead of pathProgress, take the chord, and set rotY to
     * getAngle(rise, horizontal run), mirrored when the path is reversed */
    (*(void (**)(void*, f32, f32*, f32*, f32*))(**(int**)(ctrl->pathObj + 0x68) + 0x24))(
        *(void**)&ctrl->pathObj, ctrl->pathProgress - 0.1f, &a.x, &a.y, &a.z);
    (*(void (**)(void*, f32, f32*, f32*, f32*))(**(int**)(ctrl->pathObj + 0x68) + 0x24))(
        *(void**)&ctrl->pathObj, 0.1f + ctrl->pathProgress, &b.x, &b.y, &b.z);
    a.x = a.x - b.x;
    a.y = a.y - b.y;
    a.z = a.z - b.z;
    horizRun = sqrtf(a.x * a.x + a.z * a.z);
    d = horizRun;
    a.x = horizRun;
    {
        int ang = (s16)getAngle(a.y, d);
        (obj)->anim.rotY = ang * ((ctrl->reversed << 1) - 1);
    }
    if ((s8)p->baddie.moveDone != 0)
    {
        return 5;
    }
    return 0;
}

int grimble_stateHandlerA05(short* obj, GroundBaddieState* p)
{
    GrimbleControl* ctrl;
    f64 d;
    f32 horizRun;
    struct
    {
        f32 x, y, z;
    } b;
    struct
    {
        f32 x, y, z;
    } a;

    ctrl = ((GroundBaddieState*)((GameObject*)obj)->extra)->control;
    if ((s8)p->baddie.moveJustStartedA != 0)
    {
        ObjAnim_SetCurrentMove((int)obj, 6, 0.0f, 0);
        p->baddie.moveDone = 0;
    }
    p->baddie.moveSpeed = 0.03f;
    /* aim rotY along the local path slope (same chord idiom as A06) */
    (*(void (**)(void*, f32, f32*, f32*, f32*))(**(int**)(ctrl->pathObj + 0x68) + 0x24))(
        *(void**)&ctrl->pathObj, ctrl->pathProgress - 0.1f, &a.x, &a.y, &a.z);
    (*(void (**)(void*, f32, f32*, f32*, f32*))(**(int**)(ctrl->pathObj + 0x68) + 0x24))(
        *(void**)&ctrl->pathObj, 0.1f + ctrl->pathProgress, &b.x, &b.y, &b.z);
    a.x = a.x - b.x;
    a.y = a.y - b.y;
    a.z = a.z - b.z;
    horizRun = sqrtf(a.x * a.x + a.z * a.z);
    d = horizRun;
    a.x = horizRun;
    {
        int ang = (s16)getAngle(a.y, d);
        ((GameObject*)obj)->anim.rotY = ang * ((ctrl->reversed << 1) - 1);
    }
    return 0;
}

int grimble_stateHandlerA04(short* obj, GroundBaddieState* p)
{
    GrimbleControl* ctrl;
    f64 d;
    f32 horizRun;
    struct
    {
        f32 x, y, z;
    } b;
    struct
    {
        f32 x, y, z;
    } a;

    ctrl = ((GroundBaddieState*)((GameObject*)obj)->extra)->control;
    if ((s8)p->baddie.moveJustStartedA != 0)
    {
        ObjAnim_SetCurrentMove((int)obj, 5, 0.0f, 0);
        p->baddie.moveDone = 0;
    }
    p->baddie.moveSpeed = 0.03f;
    /* aim rotY along the local path slope (same chord idiom as A06) */
    (*(void (**)(void*, f32, f32*, f32*, f32*))(**(int**)(ctrl->pathObj + 0x68) + 0x24))(
        *(void**)&ctrl->pathObj, ctrl->pathProgress - 0.1f, &a.x, &a.y, &a.z);
    (*(void (**)(void*, f32, f32*, f32*, f32*))(**(int**)(ctrl->pathObj + 0x68) + 0x24))(
        *(void**)&ctrl->pathObj, 0.1f + ctrl->pathProgress, &b.x, &b.y, &b.z);
    a.x = a.x - b.x;
    a.y = a.y - b.y;
    a.z = a.z - b.z;
    horizRun = sqrtf(a.x * a.x + a.z * a.z);
    d = horizRun;
    a.x = horizRun;
    {
        int ang = (s16)getAngle(a.y, d);
        ((GameObject*)obj)->anim.rotY = ang * ((ctrl->reversed << 1) - 1);
    }
    if ((s8)p->baddie.moveDone != 0)
    {
        return 6;
    }
    return 0;
}

int grimble_stateHandlerA03(short* obj, GroundBaddieState* p)
{
    GrimbleControl* ctrl;
    f64 d;
    f32 horizRun;
    struct
    {
        f32 x, y, z;
    } b;
    struct
    {
        f32 x, y, z;
    } a;

    ctrl = ((GroundBaddieState*)((GameObject*)obj)->extra)->control;
    if ((s8)p->baddie.moveJustStartedA != 0)
    {
        ObjAnim_SetCurrentMove((int)obj, 2, 0.0f, 0);
        p->baddie.moveDone = 0;
    }
    p->baddie.moveSpeed = 0.025f;
    /* aim rotY along the local path slope (same chord idiom as A06) */
    (*(void (**)(void*, f32, f32*, f32*, f32*))(**(int**)(ctrl->pathObj + 0x68) + 0x24))(
        *(void**)&ctrl->pathObj, ctrl->pathProgress - 0.1f, &a.x, &a.y, &a.z);
    (*(void (**)(void*, f32, f32*, f32*, f32*))(**(int**)(ctrl->pathObj + 0x68) + 0x24))(
        *(void**)&ctrl->pathObj, 0.1f + ctrl->pathProgress, &b.x, &b.y, &b.z);
    a.x = a.x - b.x;
    a.y = a.y - b.y;
    a.z = a.z - b.z;
    horizRun = sqrtf(a.x * a.x + a.z * a.z);
    d = horizRun;
    a.x = horizRun;
    {
        int ang = (s16)getAngle(a.y, d);
        ((GameObject*)obj)->anim.rotY = ang * ((ctrl->reversed << 1) - 1);
    }
    if ((s8)p->baddie.moveDone != 0)
    {
        return 1;
    }
    return 0;
}

int grimble_stateHandlerA02(GameObject* obj, char* state, f32 arg)
{
    u16 zone;
    u16 pad;
    u16 dist;
    f32 z2, y2, x2, z, y, x;
    f32 spd;
    f32 vel;
    s16 angle;
    double d;
    f32 r;
    char* sub;

    sub = (char*)((GroundBaddieState*)obj->extra)->control;
    if (*(s8*)&((GroundBaddieState*)state)->baddie.moveJustStartedA != 0)
    {
        ObjAnim_SetCurrentMove((int)obj, 3, 0.0f, 0);
        ((GroundBaddieState*)state)->baddie.moveDone = 0;
    }
    ((GroundBaddieState*)state)->baddie.moveSpeed = 0.03f;
    (*gPlayerInterface)->updateAnimRootMotion(obj, state, arg, 9);
    (*(void (**)(int, char*, f32))(*(int*)(*(int*)(((GrimbleControl*)sub)->pathObj + 0x68)) + 0x28))(
        ((GrimbleControl*)sub)->pathObj, sub + 0x48,
        ((GroundBaddieState*)state)->baddie.animSpeedA * (f32)(1 - (((GrimbleControl*)sub)->reversed << 1)));
    if (((GrimbleControl*)sub)->pathProgress < 0.3f)
    {
        ((GrimbleControl*)sub)->pathProgress = 0.3f;
    }
    else if (((GrimbleControl*)sub)->pathProgress > 6.7f)
    {
        ((GrimbleControl*)sub)->pathProgress = 6.7f;
    }
    (*(void (**)(int, f32, f32*, f32*, f32*))(*(int*)(*(int*)(((GrimbleControl*)sub)->pathObj + 0x68)) + 0x24))(
        ((GrimbleControl*)sub)->pathObj, ((GrimbleControl*)sub)->pathProgress - 0.1f, &x, &y, &z);
    (*(void (**)(int, f32, f32*, f32*, f32*))(*(int*)(*(int*)(((GrimbleControl*)sub)->pathObj + 0x68)) + 0x24))(
        ((GrimbleControl*)sub)->pathObj, 0.1f + ((GrimbleControl*)sub)->pathProgress, &x2, &y2, &z2);
    x = x - x2;
    y = y - y2;
    z = z - z2;
    r = sqrtf(x * x + z * z);
    d = r;
    x = r;
    angle = getAngle(y, d);
    obj->anim.rotY = (1.0f - 2.0f * obj->anim.currentMoveProgress) *
                     (f32)(s16)(angle * ((((GrimbleControl*)sub)->reversed << 1) - 1));
    if (*(s8*)&((GroundBaddieState*)state)->baddie.moveDone != 0)
    {
        (*gBaddieControlInterface)
            ->getTargetGeometry(obj, (GameObject*)((GroundBaddieState*)state)->baddie.targetObj, 0x10,
                                &zone, &pad, &dist);
        ((GrimbleControl*)sub)->reversed = 1 - *(u8*)&((GrimbleControl*)sub)->reversed;
        obj->anim.rotX = ((GrimbleControl*)sub)->baseRotX + (!((GrimbleControl*)sub)->reversed << 15);
        spd = (f32)(int)randomGetRange(0x32, 0x64) / 100.0f;
        vel = (f32)((((GrimbleControl*)sub)->reversed << 1) - 1) * spd;
        if (zone < 4 || zone > 0xb)
        {
            if (dist > 0x1f4)
            {
                vel *= 1.0f + dist / 100.0f;
            }
            else
            {
                vel *= 1.0f + dist / 300.0f;
            }
        }
        ((GrimbleControl*)sub)->targetProgress = ((GrimbleControl*)sub)->pathProgress - vel;
        spd = ((GrimbleControl*)sub)->targetProgress;
        spd = (spd > 1.0f) ? spd : 1.0f;
        ((GrimbleControl*)sub)->targetProgress = spd;
        spd = ((GrimbleControl*)sub)->targetProgress;
        spd = (spd < 6.0f) ? spd : 6.0f;
        ((GrimbleControl*)sub)->targetProgress = spd;
        return 4;
    }
    return 0;
}

int grimble_stateHandlerA01(GameObject* obj, char* state, f32 arg)
{
    f32 z2, y2, x2, z, y, x;
    u8 hitEdge;
    s16 angle;
    double d;
    f32 r;
    char* sub;

    sub = (char*)((GroundBaddieState*)obj->extra)->control;
    if (*(s8*)&((GroundBaddieState*)state)->baddie.moveJustStartedA != 0)
    {
        ObjAnim_SetCurrentMove((int)obj, 0, 0.0f, 0);
        ((GroundBaddieState*)state)->baddie.moveDone = 0;
    }
    (*gPlayerInterface)->updateAnimRootMotion(obj, state, arg, 0);
    if ((*(int*)&((GroundBaddieState*)state)->baddie.eventFlags & 1) != 0)
    {
        *(int*)&((GroundBaddieState*)state)->baddie.eventFlags =
            *(int*)&((GroundBaddieState*)state)->baddie.eventFlags & ~1;
        Sfx_PlayFromObject((int)obj, SFXTRIG_mv_persquk1);
    }
    (*(void (**)(int, char*, f32))(*(int*)(*(int*)(((GrimbleControl*)sub)->pathObj + 0x68)) + 0x28))(
        ((GrimbleControl*)sub)->pathObj, sub + 0x48,
        50.4f *
            (((GroundBaddieState*)state)->baddie.moveSpeed * (f32)(1 - (((GrimbleControl*)sub)->reversed << 1))));
    if (((GrimbleControl*)sub)->pathProgress < 0.3f)
    {
        ((GrimbleControl*)sub)->pathProgress = 0.3f;
        hitEdge = 1;
    }
    else if (((GrimbleControl*)sub)->pathProgress > 6.7f)
    {
        ((GrimbleControl*)sub)->pathProgress = 6.7f;
        hitEdge = 1;
    }
    else
    {
        hitEdge = 0;
    }
    if (hitEdge != 0)
    {
        return 7;
    }
    (*(void (**)(int, f32, f32*, f32*, f32*))(*(int*)(*(int*)(((GrimbleControl*)sub)->pathObj + 0x68)) + 0x24))(
        ((GrimbleControl*)sub)->pathObj, ((GrimbleControl*)sub)->pathProgress - 0.1f, &x, &y, &z);
    (*(void (**)(int, f32, f32*, f32*, f32*))(*(int*)(*(int*)(((GrimbleControl*)sub)->pathObj + 0x68)) + 0x24))(
        ((GrimbleControl*)sub)->pathObj, 0.1f + ((GrimbleControl*)sub)->pathProgress, &x2, &y2, &z2);
    x = x - x2;
    y = y - y2;
    z = z - z2;
    r = sqrtf(x * x + z * z);
    d = r;
    x = r;
    angle = getAngle(y, d);
    (obj)->anim.rotY = angle * ((((GrimbleControl*)sub)->reversed << 1) - 1);
    return 0;
}

int grimble_stateHandlerA00(GameObject* obj, char* state, f32 arg)
{
    u16 zone;
    u16 pad;
    u16 dist;
    f32 z2, y2, x2, z, y, x;
    s16 angle;
    double d;
    f32 r;
    char* sub;

    sub = (char*)((GroundBaddieState*)obj->extra)->control;
    if (*(s8*)&((GroundBaddieState*)state)->baddie.moveJustStartedA != 0)
    {
        ObjAnim_SetCurrentMove((int)obj, 0, 0.0f, 0);
        ((GroundBaddieState*)state)->baddie.moveDone = 0;
    }
    ((GroundBaddieState*)state)->baddie.moveSpeed = 0.03f;
    (*gPlayerInterface)->updateAnimRootMotion(obj, state, arg, 1);
    (*(void (**)(int, char*, f32))(*(int*)(*(int*)(((GrimbleControl*)sub)->pathObj + 0x68)) + 0x28))(
        ((GrimbleControl*)sub)->pathObj, sub + 0x48,
        ((GroundBaddieState*)state)->baddie.animSpeedA * (f32)(1 - (((GrimbleControl*)sub)->reversed << 1)));
    if (((GrimbleControl*)sub)->pathProgress < 0.3f)
    {
        ((GrimbleControl*)sub)->pathProgress = 0.3f;
    }
    else if (((GrimbleControl*)sub)->pathProgress > 6.7f)
    {
        ((GrimbleControl*)sub)->pathProgress = 6.7f;
    }
    (*gBaddieControlInterface)
        ->getTargetGeometry(obj, (GameObject*)((GroundBaddieState*)state)->baddie.targetObj, 0x10,
                            &zone, &pad, &dist);
    if (zone > 3 && zone < 0xc && dist > 0x190 && ((GrimbleControl*)sub)->pathProgress > 2.0f &&
        ((GrimbleControl*)sub)->pathProgress < 5.0f)
    {
        return 3;
    }
    if ((((GrimbleControl*)sub)->reversed ^
         (((GrimbleControl*)sub)->pathProgress >= ((GrimbleControl*)sub)->targetProgress)) != 0 &&
        *(s8*)&((GroundBaddieState*)state)->baddie.moveDone != 0)
    {
        return 3;
    }
    if ((*(int*)&((GroundBaddieState*)state)->baddie.eventFlags & 1) != 0)
    {
        *(int*)&((GroundBaddieState*)state)->baddie.eventFlags =
            *(int*)&((GroundBaddieState*)state)->baddie.eventFlags & ~1;
        Sfx_PlayFromObject((int)obj, SFXTRIG_mv_persquk1);
    }
    (*(void (**)(int, f32, f32*, f32*, f32*))(*(int*)(*(int*)(((GrimbleControl*)sub)->pathObj + 0x68)) + 0x24))(
        ((GrimbleControl*)sub)->pathObj, ((GrimbleControl*)sub)->pathProgress - 0.1f, &x, &y, &z);
    (*(void (**)(int, f32, f32*, f32*, f32*))(*(int*)(*(int*)(((GrimbleControl*)sub)->pathObj + 0x68)) + 0x24))(
        ((GrimbleControl*)sub)->pathObj, 0.1f + ((GrimbleControl*)sub)->pathProgress, &x2, &y2, &z2);
    x = x - x2;
    y = y - y2;
    z = z - z2;
    r = sqrtf(x * x + z * z);
    d = r;
    x = r;
    angle = getAngle(y, d);
    obj->anim.rotY = angle * ((((GrimbleControl*)sub)->reversed << 1) - 1);
    return 0;
}

int grimble_animEventCallback(void)
{
    return 0x0;
}


void grimble_attachNearestPath(GameObject* obj)
{
    int count;
    f32 pathDistance;
    f32 candidateProgress;
    f32 pathQueryAux;
    f32 targetProgress;
    int* pathObjects;
    char* extraState;
    int i;
    int rotationDelta;
    int sameDirection;
    char* controlData;

    extraState = obj->extra;
    pathObjects = (void*)ObjGroup_GetObjects(DFROPENODE_OBJGROUP, &count);
    if (count != 0)
    {
        controlData = (char*)((GroundBaddieState*)extraState)->control;
        ((GrimbleControl*)controlData)->candidatePathObj = 0;
        ((GrimbleControl*)controlData)->nearestDist = 200.0f;
        for (i = 0; i < count; i++)
        {
            if ((*(int (**)(int, f32, f32, f32, f32*, f32*, f32*))(
                    *(int*)(*(int*)(pathObjects[i] + 0x68)) + 0x30))(
                    pathObjects[i], obj->anim.localPosX, obj->anim.localPosY, obj->anim.localPosZ, &pathDistance,
                    &candidateProgress, &pathQueryAux) != 0 &&
                pathDistance < ((GrimbleControl*)controlData)->nearestDist)
            {
                ((GrimbleControl*)controlData)->candidatePathObj = pathObjects[i];
                ((GrimbleControl*)controlData)->nearestDist = pathDistance;
                ((GrimbleControl*)controlData)->candidateProgress = candidateProgress;
            }
        }
        if (*(void**)&((GrimbleControl*)controlData)->candidatePathObj != NULL)
        {
            ((GrimbleControl*)controlData)->pathObj = ((GrimbleControl*)controlData)->candidatePathObj;
            ((GrimbleControl*)controlData)->pathProgress = ((GrimbleControl*)controlData)->candidateProgress;
            (*(void (**)(int, char*))(
                *(int*)(*(int*)(((GrimbleControl*)controlData)->pathObj + 0x68)) + 0x20))(
                ((GrimbleControl*)controlData)->pathObj, controlData + 0xc);
            (*(void (**)(int, f32, f32*, f32*, f32*))(
                *(int*)(*(int*)(((GrimbleControl*)controlData)->pathObj + 0x68)) + 0x24))(
                ((GrimbleControl*)controlData)->pathObj, ((GrimbleControl*)controlData)->pathProgress,
                (f32*)(controlData + 0x1c), (f32*)(controlData + 0x20), (f32*)(controlData + 0x24));
            ((GrimbleControl*)controlData)->baseRotX = (*(s16(**)(int))(
                *(int*)(*(int*)(((GrimbleControl*)controlData)->pathObj + 0x68)) + 0x34))(
                ((GrimbleControl*)controlData)->pathObj);
            ((GrimbleControl*)controlData)->savedPathProgress = ((GrimbleControl*)controlData)->pathProgress;
            ((GrimbleControl*)controlData)->unk46 = 0;
            ((GrimbleControl*)controlData)->anchorPosY = ((GrimbleControl*)controlData)->pathPosY;
            ((GrimbleControl*)controlData)->currentPosY = obj->anim.localPosY;
            ((GrimbleControl*)controlData)->posYDelta =
                ((GrimbleControl*)controlData)->anchorPosY - ((GrimbleControl*)controlData)->currentPosY;
            rotationDelta = obj->anim.rotX - (u16)((GrimbleControl*)controlData)->baseRotX;
            if (rotationDelta > 0x8000)
            {
                rotationDelta -= 0xffff;
            }
            if (rotationDelta < -0x8000)
            {
                rotationDelta += 0xffff;
            }
            sameDirection = 0;
            if (rotationDelta <= 0x3ffc && rotationDelta >= -0x3ffc)
            {
                sameDirection = 1;
            }
            ((GrimbleControl*)controlData)->reversed = sameDirection;
            obj->anim.rotX = ((GrimbleControl*)controlData)->baseRotX +
                             (!((GrimbleControl*)controlData)->reversed << 15);
            targetProgress =
                ((GrimbleControl*)controlData)->pathProgress -
                (f32)((((GrimbleControl*)controlData)->reversed << 1) - 1) *
                    ((f32)(int)randomGetRange(0xa, 0x3c) / 10.0f);
            ((GrimbleControl*)controlData)->targetProgress = targetProgress;
            targetProgress = ((GrimbleControl*)controlData)->targetProgress;
            targetProgress = (targetProgress > 1.0f) ? targetProgress : 1.0f;
            ((GrimbleControl*)controlData)->targetProgress = targetProgress;
            targetProgress = ((GrimbleControl*)controlData)->targetProgress;
            targetProgress = (targetProgress < 6.0f) ? targetProgress : 6.0f;
            ((GrimbleControl*)controlData)->targetProgress = targetProgress;
        }
    }
}

int grimble_getExtraSize(void)
{
    return sizeof(GroundBaddieState) + sizeof(GrimbleControl);
}
int grimble_getObjectTypeId(void)
{
    return 0x59;
}

void grimble_free(GameObject* obj)
{
    int* state = obj->extra;
    ObjGroup_RemoveObject((u32)obj, GRIMBLE_OBJGROUP);
    (*gBaddieControlInterface)->releaseState(obj, state, 0);
}

void grimble_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible)
{
    char* state = obj->extra;
    char* sub = *(char**)&((GroundBaddieState*)state)->control;

    if (visible == 0 || obj->userData1 != 0)
    {
        return;
    }
    objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, 1.0f);
    if (((GrimbleControl*)sub)->unk50 > 0.0f)
    {
        (*gBoneParticleEffectInterface)->spawnEffect((void*)obj, 0x52a, NULL, 0x64, NULL);
    }
    if ((((GroundBaddieState*)state)->flags400 & 0x60) != 0)
    {
        objParticleFn_80099d84((GameObject*)obj, 1.0f, 3, ((GroundBaddieState*)state)->glowAlpha, 0);
    }
    if ((((GroundBaddieState*)state)->flags400 & 0x100) != 0)
    {
        objParticleFn_80099d84((GameObject*)obj, 1.0f, 4, ((GroundBaddieState*)state)->glowAlpha, 0);
        ((GroundBaddieState*)state)->flags400 = ((GroundBaddieState*)state)->flags400 & ~0x100;
    }
}

void grimble_hitDetect(int obj)
{
    (*gPlayerInterface)->updateVelocityState((void*)obj, ((GameObject*)obj)->extra, gGrimbleStateHandlersA);
}

void grimble_update(GameObject* obj)
{
    char* state;
    char* sub;
    int def;

    state = obj->extra;
    sub = *(char**)&((GroundBaddieState*)state)->control;
    def = *(int*)&obj->anim.placementData;
    if (obj->userData1 != 0)
    {
        if ((*gMapEventInterface)->shouldNotSaveTime(((ObjPlacement*)def)->mapId) != 0)
        {
            (*gBaddieControlInterface)
                ->initGroundBaddie(obj, (u8*)def, (u8*)state, 0xa, 6, 0x10e, 0x36, 20.0f);
            ((GroundBaddieState*)state)->baddie.substate = 1;
            ((GroundBaddieState*)state)->baddie.moveJustStartedB = 1;
            obj->anim.alpha = 0;
        }
    }
    else
    {
        if (*(void**)&((GrimbleControl*)sub)->candidatePathObj != NULL)
        {
            void* target;
            int r;
            (*gPlayerInterface)->update(obj, state, 1.0f, 1.0f, gGrimbleStateHandlersA, gGrimbleStateHandlersB);
            (*(void (**)(int, f32, f32*, f32*, f32*))(
                *(int*)(*(int*)(((GrimbleControl*)sub)->pathObj + 0x68)) + 0x24))(
                ((GrimbleControl*)sub)->pathObj, ((GrimbleControl*)sub)->pathProgress, &obj->anim.localPosX,
                &obj->anim.localPosY, &obj->anim.localPosZ);
            (*gBaddieControlInterface)
                ->processMessages(obj, state, state + 0x35c, ((GroundBaddieState*)state)->gameBitB,
                                  (u8*)(state + 0x405), 0, 0, 0);
            r = (*gBaddieControlInterface)
                    ->updateHitReaction(obj, state, state + 0x35c, ((GroundBaddieState*)state)->gameBitB,
                                        lbl_803200E0, lbl_80320158, 3, NULL);
            if (r == 0xe)
            {
                ((GroundBaddieState*)state)->subMode = 2;
                ((GroundBaddieState*)state)->baddie.targetObj = Obj_GetPlayerObject();
            }
            if (((GroundBaddieState*)state)->baddie.targetObj != NULL ||
                *(s8*)&((GroundBaddieState*)state)->baddie.hitPoints == 0)
            {
                ((ObjHitsPriorityState*)obj->anim.hitReactState)->flags |= 1;
                if ((*gBaddieControlInterface)
                        ->shouldDropTarget(obj, state, (f32)((GroundBaddieState*)state)->aggroRange, 1) != 0)
                {
                    *(int*)&((GroundBaddieState*)state)->baddie.targetObj = 0;
                }
            }
            else
            {
                ((ObjHitsPriorityState*)obj->anim.hitReactState)->flags &= ~1;
                target = (*gBaddieControlInterface)
                             ->findAggroTarget(obj, state, (f32)((GroundBaddieState*)state)->aggroRange, 0x8000);
                if (target != NULL)
                {
                    ((GroundBaddieState*)state)->baddie.targetObj = target;
                    ((GroundBaddieState*)state)->baddie.hasTarget = 0;
                }
            }
        }
        else
        {
            grimble_attachNearestPath(obj);
        }
    }
}

void grimble_init(int obj, int def, int flag)
{
    char* state = ((GameObject*)obj)->extra;
    u8 flags = 2;

    if (flag != 0)
    {
        flags |= 1;
    }
    (*gBaddieControlInterface)
        ->initGroundBaddie((GameObject*)obj, (u8*)def, (u8*)state, 0, 0, 0, flags, 20.0f);
    ((GameObject*)obj)->animEventCallback = grimble_animEventCallback;
    (*gPlayerInterface)->setState((void*)obj, state, 0);
    ((GroundBaddieState*)state)->baddie.substate = 0;
    ((GroundBaddieState*)state)->baddie.animSpeedA = 0.0f;
    ((GrimbleControl*)((GroundBaddieState*)state)->control)->candidatePathObj = 0;
}

void grimble_release(void)
{
}

void grimble_initialise(void)
{
    grimble_initialiseStateHandlerTables();
}

void grimble_initialiseStateHandlerTables(void)
{
    gGrimbleStateHandlersA[0] = grimble_stateHandlerA00;
    gGrimbleStateHandlersA[1] = grimble_stateHandlerA01;
    gGrimbleStateHandlersA[2] = grimble_stateHandlerA02;
    gGrimbleStateHandlersA[3] = grimble_stateHandlerA03;
    gGrimbleStateHandlersA[4] = grimble_stateHandlerA04;
    gGrimbleStateHandlersA[5] = grimble_stateHandlerA05;
    gGrimbleStateHandlersA[6] = grimble_stateHandlerA06;
    gGrimbleStateHandlersA[7] = grimble_stateHandlerA07;
    gGrimbleStateHandlersA[8] = grimble_stateHandlerA08;
    gGrimbleStateHandlersA[9] = grimble_stateHandlerA09;
    gGrimbleStateHandlersB[0] = grimble_stateHandlerB00;
    gGrimbleStateHandlersB[1] = grimble_stateHandlerB01;
    gGrimbleStateHandlersB[2] = scarab_updateProximityGate;
    gGrimbleStateHandlersB[3] = grimble_stateHandlerB03;
    gGrimbleStateHandlersB[4] = grimble_stateHandlerB04;
    gGrimbleStateHandlersB[5] = grimble_stateHandlerB05;
}

ObjectDescriptor gGrimbleObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)grimble_initialise,
    (ObjectDescriptorCallback)grimble_release,
    0,
    (ObjectDescriptorCallback)grimble_init,
    (ObjectDescriptorCallback)grimble_update,
    (ObjectDescriptorCallback)grimble_hitDetect,
    (ObjectDescriptorCallback)grimble_render,
    (ObjectDescriptorCallback)grimble_free,
    (ObjectDescriptorCallback)grimble_getObjectTypeId,
    grimble_getExtraSize,
};
