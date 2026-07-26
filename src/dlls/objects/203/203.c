/*
 * DLL 0x00CB - a ground-baddie object (object type id 0x14b, extra size
 * 0x410). Its AI runs through gBaddieControlInterface and gPlayerInterface:
 * dll_CB_init sets up the control state and registers dll_CB_seqFn as the
 * anim-event callback; dll_CB_update advances movement and, while the
 * sub-state's flags400 bit 8 is set, walks a ROM curve path
 * (gRomCurveInterface / Curve_AdvanceAlongPath) copying the curve's
 * position/orientation onto the object. dll_CB_seqFn drives an objseq
 * sub-state machine (subMode 0/1/2) handling player tracking, route paths
 * (routeNav/routeState) and game-bit gating (gameBitC / DllCBPlacement.gameBitId yield).
 * dll_CB_initialise installs the two callback tables gDllCBMoveHandlers /
 * gDllCBStateHandlers used by the player-interface update.
 */
#include "main/dll/chukchukstate_struct.h"
#include "main/dll/baddie_control_interface.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "game/objects/object.h"
#include "main/objprint_character_api.h"
#include "main/obj_group.h"
#include "main/obj_message.h"
#include "sys/objects/lifecycle.h"
#include "sys/objects.h"
#include "main/object_render.h"
#include "main/dll/rom_curve_interface.h"
#include "main/dll/curve_walker.h"
#include "main/objseq.h"
#include "main/player_control_interface.h"
#include "main/gamebits.h"
#include "main/frame_timing.h"
#include "main/dll/dll_00CB_dllcb.h"
#include "main/dll/dll_00CD_iceball.h"
#include "main/voxmaps.h"
#include "main/curve.h"
#include "string.h"

typedef struct DllCBPlacement
{
    u8 pad0[0x4 - 0x0];
    s8 unk4;
    s8 unk5;
    u8 unk6;
    u8 unk7;
    f32 posX;
    f32 posY;
    f32 posZ;
    u8 pad14[0x24 - 0x14];
    s16 trackYieldId;
    u8 pad26[0x2C - 0x26];
    s16 gameBitId;
    s8 trackYieldEnable;
    u8 pad2F[0x30 - 0x2F];
} DllCBPlacement;

/*
 * Per-object extra state for the ChukChuk ice-spitter
 * (ChukChuk_getExtraSize == 0x18).
 */

STATIC_ASSERT(sizeof(ChukChukState) == 0x18);
STATIC_ASSERT(offsetof(ChukChukState, flags) == 0x12);

/* object group this object belongs to */
#define DLLCB_OBJGROUP 3

extern void* gDllCBMoveHandlers[];
void* gDllCBStateHandlers[6];
int dll_CB_stateHandler5(GameObject* obj, GroundBaddieState* state)
{
    GroundBaddieState* sub;
    RouteNav* routePath;
    f32 zero;

    sub = obj->extra;
    if (*(void**)&state->baddie.targetObj != NULL)
    {
        (*gPlayerInterface)->setState((void*)obj, state, 1);
        routePath = &sub->routeNav;
        zero = 0.0f;
        state->baddie.moveInputX = zero;
        state->baddie.moveInputZ = zero;
        memcpy(routePath, &obj->anim.localPosX, 12);
        memcpy((void*)sub->routeNav.curPos, (void*)&((GameObject*)state->baddie.targetObj)->anim.localPosX, 12);
        voxmaps_updateRoutePath(&sub->routeNav, &sub->routeState);
        if (state->baddie.targetDistance < 5e+01f && sub->subMode == 2)
        {
            return 5;
        }
        if (routePath->flag25 == 0)
        {
            (*gPlayerInterface)
                ->moveTowardPoint((void*)obj, state, routePath->tgtPos[0], routePath->tgtPos[2], 0.0f,
                                  0.0f, 6e+01f);
        }
        else
        {
            (*gPlayerInterface)
                ->moveTowardPoint((void*)obj, state, routePath->tgtPos[0], routePath->tgtPos[2], 15.0f,
                                  3e+01f, 6e+01f);
        }
    }
    else
    {
        (*gPlayerInterface)->setState((void*)obj, state, 0);
        state->baddie.moveDone = 0;
    }
    return 0;
}

int dll_CB_stateHandler4(GameObject* obj, GroundBaddieState* state)
{
    if ((s8)state->baddie.moveJustStartedB != 0)
    {
        f32 fz;
        (*gPlayerInterface)->setState(obj, state, 0);
        fz = 5.0f;
        obj->anim.velocityY = fz;
        state->baddie.animSpeedA = fz;
        state->baddie.animSpeedC = fz;
    }
    if (obj->anim.velocityY < 0.25)
    {
        f32 fz = 0.0f;
        obj->anim.velocityY = fz;
        state->baddie.animSpeedA = fz;
        state->baddie.animSpeedC = fz;
        return 6;
    }
    {
        f32 d = 1.1f;
        obj->anim.velocityY = obj->anim.velocityY / d;
        state->baddie.animSpeedA = state->baddie.animSpeedA / d;
        state->baddie.animSpeedC = state->baddie.animSpeedC / d;
    }
    return 0;
}

int dll_CB_stateHandler3(GameObject* obj, u8* obj2)
{
    GroundBaddieState* state = obj->extra;
    if ((s8)obj2[0x27b] != 0)
    {
        (*gBaddieControlInterface)->spawnChild(obj, state->triggerId, -1, 0);
    }
    return 0;
}

int dll_CB_stateHandler2(GameObject* obj, GroundBaddieState* state)
{
    ObjHitsPriorityState* hitState;

    if (*(char*)&state->baddie.moveJustStartedB != '\0')
    {
        (*gPlayerInterface)->setState((void*)obj, state, 3);
        *(int*)&state->baddie.targetObj = 0;
        state->baddie.physicsActive = 0;
        state->baddie.hasTarget = 0;
        hitState = (ObjHitsPriorityState*)(obj)->anim.hitReactState;
        hitState->flags &= ~1;
        *(u8*)&(obj)->anim.resetHitboxMode |= INTERACT_FLAG_DISABLED;
    }
    else
    {
        ObjMsg_SendToObject(Obj_GetPlayerObject(), 0xe0000, obj, 0);
        if ((obj)->anim.placementData == NULL)
        {
            Obj_FreeObject(obj);
            return 0;
        }
        return 4;
    }
    return 0;
}

int dll_CB_stateHandler1(int p1, u8* obj)
{
    if ((s8)obj[0x354] < 1)
        return 3;
    return 6;
}

int dll_CB_stateHandler0(void)
{
    return 0x6;
}

int dll_CB_moveHandler3(GameObject* obj)
{
    GroundBaddieState* sub = obj->extra;
    u8 step;
    if (obj->anim.alpha >= (step = framesThisStep))
    {
        obj->anim.alpha -= step;
    }
    else
    {
        obj->anim.alpha = 0;
    }
    if (obj->anim.alpha == 0)
    {
        mainSetBits(sub->gameBitB, 0);
        mainSetBits(sub->gameBitA, 1);
    }
    return 0;
}

int dll_CB_moveHandler2(short* out, BaddieState* obj)
{
    f32 f = 0.0f;
    obj->animSpeedA = f;
    obj->animSpeedB = f;
    obj->physicsActive = 1;
    out[2] = obj->spawnRotZ;
    out[1] = obj->spawnRotY;
    return 0;
}

int dll_CB_moveHandler1(GameObject* obj, GroundBaddieState* def)
{
    GroundBaddieState* state = obj->extra;
    if ((s8)def->baddie.moveJustStartedA != 0)
    {
        ObjAnim_SetCurrentMove((int)obj, 0, 0.0f, 0);
        def->baddie.moveDone = 0;
    }
    def->baddie.physicsActive = 1;
    obj->anim.rotZ = def->baddie.spawnRotZ;
    obj->anim.rotY = def->baddie.spawnRotY;
    (*gBaddieControlInterface)
        ->updateMovementBlend(obj, def, state, 1.0f, 12.0f);
    def->baddie.moveSpeed = 0.075f * def->baddie.animSpeedA;
    return 0;
}

int dll_CB_moveHandler0(short* out, BaddieState* obj, f32 timeDelta)
{
    f32 f = 0.0f;
    obj->animSpeedA = f;
    obj->animSpeedB = f;
    obj->moveSpeed = f;
    obj->physicsActive = 1;
    out[2] = obj->spawnRotZ;
    out[1] = obj->spawnRotY;
    (*gPlayerInterface)->rotateTowardTarget(out, obj, timeDelta, 5);
    return 0;
}

const f32 gDllCbDefaultAnimSpeed = 0.1f;

void dll_CB_seekAndUpdate(GameObject* obj, void* seq, GroundBaddieState* sub, GroundBaddieState* state)
{
    DllCBPlacement* setup;

    setup = *(DllCBPlacement**)&obj->anim.placementData;
    state->baddie.moveDone = 1;
    if ((*gBaddieControlInterface)
            ->shouldDropTarget(obj, state, (f32)(u32)sub->aggroRange, 1) != 0)
    {
        *(int*)&state->baddie.targetObj = sub->savedObjC0;
        state->baddie.hasTarget = 0;
        if (setup->trackYieldEnable != -1)
        {
            if (seq != NULL)
            {
                (*gObjectTriggerInterface)->yield((ObjSeqState*)seq, setup->trackYieldId);
            }
            sub->subMode = 1;
        }
        else
        {
            *(int*)&state->baddie.targetObj = 0;
        }
    }
    (*gBaddieControlInterface)->updateGravity(obj, state, 0.17f, 1);
    sub->savedObjC0 = *(int*)&obj->pendingParentObj;
    *(int*)&obj->pendingParentObj = 0;
    (*gPlayerInterface)->update(obj, state, timeDelta, timeDelta, gDllCBMoveHandlers, gDllCBStateHandlers);
    *(int*)&obj->pendingParentObj = sub->savedObjC0;
}

void dll_CB_advanceAI(GameObject* obj, GroundBaddieState* sub, GroundBaddieState* state)
{
    GameObject* targetObj;
    int stateResult;
    struct
    {
        f32 x, y, z;
    } d;
    f32* dp = &d.x;

    if (obj->childObjs[0] != NULL)
    {
        ((GameObject*)obj->childObjs[0])->anim.parent = obj->anim.parent;
    }
    targetObj = *(GameObject**)&state->baddie.targetObj;
    if (targetObj != NULL)
    {
        d.x = targetObj->anim.worldPosX - obj->anim.worldPosX;
        d.y = targetObj->anim.worldPosY - obj->anim.worldPosY;
        d.z = targetObj->anim.worldPosZ - obj->anim.worldPosZ;
        state->baddie.targetDistance = sqrtf(d.z * d.z + (d.x * d.x + d.y * d.y));
    }
    characterDoEyeAnims(obj, sub->eyeAnimState);
    if ((sub->configFlags & 1) == 0)
    {
        (*gBaddieControlInterface)
            ->pollCameraTarget(obj, state, &sub->flags400, 2, 3, sub->soundIdB, sub->soundIdA);
    }
    (*gBaddieControlInterface)
        ->processMessages(obj, state, &sub->routeNav, sub->gameBitB, &sub->subMode, 0, 0, 0);
    stateResult = (*gBaddieControlInterface)
                      ->updateHitReaction(obj, state, &sub->routeNav, sub->gameBitB,
                                          lbl_80320008, lbl_80320080, 1, NULL);
    if (stateResult >= 4)
    {
        sub->subMode = 2;
        *(int*)&state->baddie.targetObj = (int)Obj_GetPlayerObject();
    }
}

int dll_CB_seqFn(GameObject* obj, int p2, ObjSeqState* e)
{
    DllCBPlacement* setup;
    RomCurveWalker* path;
    GroundBaddieState* sub;

    setup = *(DllCBPlacement**)&obj->anim.placementData;
    sub = *(GroundBaddieState**)&obj->extra;
    if (obj->userData1 != 0)
    {
        return 0;
    }
    if (obj->seqIndex != -1)
    {
        if ((*gBaddieControlInterface)
                ->isObjectValid(obj, sub, 1) == 0)
        {
            return 1;
        }
        dll_CB_advanceAI(obj, sub, sub);
        if (sub->gameBitC != -1 && mainGetBit(sub->gameBitC) != 0)
        {
            (*gObjectTriggerInterface)->yield(e, setup->gameBitId);
            sub->gameBitC = -1;
        }
        switch (sub->subMode)
        {
        case 2:
            e->flags = 0;
            dll_CB_seekAndUpdate(obj, e, sub, sub);
            if (sub->subMode == 1)
            {
                sub->baddie.substate = 5;
                (*gPlayerInterface)
                    ->update(obj, sub, 1.0f, 1.0f, gDllCBMoveHandlers,
                             gDllCBStateHandlers);
                e->movementState = 0;
            }
            break;
        case 1:
            if ((*gBaddieControlInterface)
                    ->updateSequenceMovement(obj, e, (char*)sub, gDllCBMoveHandlers,
                                             gDllCBStateHandlers, 0) != 0)
            {
                (*gBaddieControlInterface)
                    ->updateGravity(obj, sub, 0.17f, 1);
            }
            break;
        case 0:
        default:
            e->flags = -1;
            e->flags &= ~0x40;
            path = (RomCurveWalker*)sub->path;
            if ((sub->flags400 & BADDIE_FLAG400_PATH_ACTIVE) != 0)
            {
                if ((Curve_AdvanceAlongPath((Curve*)path, sub->baddie.animSpeedA) != 0 ||
                     path->atSegmentEnd != 0) &&
                    (*gRomCurveInterface)->goNextPoint(path) != 0)
                {
                    sub->flags400 &= ~BADDIE_FLAG400_PATH_ACTIVE;
                }
                sub->baddie.animSpeedA = 0.1f;
                obj->anim.rotX = getAngle(path->tangentX, path->tangentZ) + 0x8000;
                obj->anim.rotY = getAngle(path->tangentZ, path->tangentY) + 0x4000;
                obj->anim.rotZ = getAngle(path->tangentY, path->tangentX) + 0x4000;
                obj->anim.localPosX = path->posX;
                obj->anim.localPosY = path->posY;
                obj->anim.localPosZ = path->posZ;
            }
            break;
        }
    }
    if (obj->seqIndex == -1)
    {
        sub->flags400 |= 2;
        return 0;
    }
    return sub->subMode != 0;
}

void dll_CB_func0B_nop(void)
{
}

s16 dll_CB_setScale(GameObject* obj)
{
    return ((BaddieState*)obj->extra)->controlMode;
}

int dll_CB_getExtraSize_ret_1040(void)
{
    return 0x410;
}

int dll_CB_getObjectTypeId(void)
{
    return 0x14b;
}

void dll_CB_free(GameObject* obj)
{
    GroundBaddieState* state = obj->extra;
    ObjGroup_RemoveObject((int)obj, DLLCB_OBJGROUP);
    {
        GameObject* sub = obj->childObjs[0];
        if (sub != NULL)
        {
            Obj_FreeObject(sub);
            obj->childObjs[0] = NULL;
        }
    }
    (*gBaddieControlInterface)->releaseState(obj, state, 1);
}

void dll_CB_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible)
{
    s32 v = visible;
    if (v != 0)
    {
        switch (obj->userData1)
        {
        case 0:
            objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, 1.0f);
            break;
        }
    }
}

void dll_CB_hitDetect(GameObject* obj)
{
    void* a = obj->extra;
    (*gPlayerInterface)->updateVelocityState(obj, a, gDllCBMoveHandlers);
}

void dll_CB_update(GameObject* obj)
{
    RomCurveWalker* path;
    GroundBaddieState* sub;
    DllCBPlacement* def;

    sub = obj->extra;
    def = *(DllCBPlacement**)&obj->anim.placementData;
    if (obj->userData1 != 0)
        return;
    if (obj->userData2 == 0)
    {
        obj->anim.localPosX = def->posX;
        obj->anim.localPosY = def->posY;
        obj->anim.localPosZ = def->posZ;
        obj->userData2 = 1;
        return;
    }
    if ((sub->flags400 & 2) != 0)
    {
        (*gBaddieControlInterface)
            ->startHitReaction(obj, sub, &sub->routeNav, sub->gameBitB, &sub->subMode, 0, 0, 0, 1);
        sub->flags400 = (u16)(sub->flags400 & ~2);
    }
    if ((*gBaddieControlInterface)->isObjectValid(obj, sub, 1) == 0)
        return;
    dll_CB_advanceAI(obj, sub, sub);
    path = (RomCurveWalker*)sub->path;
    if ((sub->flags400 & BADDIE_FLAG400_PATH_ACTIVE) == 0)
        return;
    if (Curve_AdvanceAlongPath((Curve*)path, sub->baddie.animSpeedA) != 0 || path->atSegmentEnd != 0)
    {
        if ((*gRomCurveInterface)->goNextPoint(path) != 0)
        {
            sub->flags400 = (u16)(sub->flags400 & ~BADDIE_FLAG400_PATH_ACTIVE);
        }
    }
    sub->baddie.animSpeedA = 0.1f;
    obj->anim.rotX = (s16)(getAngle(path->tangentX, path->tangentZ) + 0x8000);
    obj->anim.rotY = (s16)(getAngle(path->tangentZ, path->tangentY) + 0x4000);
    obj->anim.rotZ = (s16)(getAngle(path->tangentY, path->tangentX) + 0x4000);
    obj->anim.localPosX = path->posX;
    obj->anim.localPosY = path->posY;
    obj->anim.localPosZ = path->posZ;
}

void dll_CB_init(GameObject* obj, u8* params, int extra)
{
    GroundBaddieState* sub;
    u8 flags;

    sub = obj->extra;
    flags = 0x16;
    if (extra != 0)
        flags |= 1;
    if ((params[0x2b] & 1) == 0)
        flags |= 8;
    obj->anim.rotY = (s16)((s8)params[0x28] << 8);
    obj->anim.rotZ = (s16)((s8)params[0x27] << 8);
    (*gBaddieControlInterface)
        ->initGroundBaddie(obj, params, (u8*)sub, 4, 6, 0x82, flags, 2e+01f);
    obj->animEventCallback = dll_CB_seqFn;
    (*gPlayerInterface)->setState(obj, sub, 0);
    sub->baddie.substate = 0;
    if (sub->aggroRange < 0x32)
    {
        sub->aggroRange = 0x32;
    }
}

void dll_CB_release_nop(void)
{
}

void dll_CB_initialise(void)
{
    ((void**)gDllCBMoveHandlers)[0] = dll_CB_moveHandler0;
    ((void**)gDllCBMoveHandlers)[1] = dll_CB_moveHandler1;
    ((void**)gDllCBMoveHandlers)[2] = dll_CB_moveHandler2;
    ((void**)gDllCBMoveHandlers)[3] = dll_CB_moveHandler3;
    gDllCBStateHandlers[0] = dll_CB_stateHandler0;
    gDllCBStateHandlers[1] = dll_CB_stateHandler1;
    gDllCBStateHandlers[2] = dll_CB_stateHandler2;
    gDllCBStateHandlers[3] = dll_CB_stateHandler3;
    gDllCBStateHandlers[4] = dll_CB_stateHandler4;
    gDllCBStateHandlers[5] = dll_CB_stateHandler5;
}
