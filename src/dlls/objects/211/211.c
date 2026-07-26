/*
 * staffAction (DLL 0x00D3) - a baddie that hops/crawls along surfaces and
 * chases the player, driven by the shared baddie-control interface
 * (gBaddieControlInterface) and the LandedArwing movement/collision state
 * (LandedArwingState behind GroundBaddieState->control, at +0x40c).
 *
 * Movement is a bounce-walker: landedarwing_moveSurfaceCrawler runs a
 * per-axis bounce machine over surfaceMode 0-5 (X/Y/Z wall planes), while
 * surfaceMode 6 is the swept-surface mode that does collision against a
 * bound mesh object (landedarwing_updateAirborneMotion /
 * landedarwing_moveAlongSurface / landedarwing_resolveSurfaceCollision).
 * flags92 is a packed bit/nibble field (LandedArwingMovementFlags) holding
 * the per-frame movement flags and a bounds-object lookup retry counter
 * (high nibble).
 *
 * dll_D3_update drives target acquisition, contact damage, and per-frame
 * advance through gBaddieControlInterface and gPlayerInterface vtable
 * slots; the object id is 0x49 and its extra block is 0x4a4 bytes. The
 * state handler table gLandedArwingStateHandlers is populated in
 * dll_D3_initialise (slot 0 = landedarwing_updateMovementState).
 */
#include "main/dll/baddie_state.h"
#include "main/dll/baddie_control_interface.h"
#include "sys/objects.h"
#include "main/shader_api.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/dll/path_control_interface.h"
#include "game/objects/object.h"
#include "main/track_bbox_api.h"
#include "main/objprint_api.h"
#include "main/dll/landedArwing.h"
#include "main/dll/dll_00D3_staffAction.h"
#include "main/obj_contact.h"
#include "main/objhits.h"
#include "main/obj_group.h"
#include "main/obj_list.h"
#include "main/frame_timing.h"
#include "main/player_control_interface.h"
#include "main/vecmath.h"
#include "main/track_dolphin_api.h"
#include "main/object_render.h"
#include "main/trig.h"
#include "main/dll/treasurechest_state.h"
#include "main/objseq.h"
#include "main/objfx.h"
#include "dlls/object_descriptor.h"
#include "string.h"
#include "main/dll/dll_00D2_tumbleweed.h"
#include "sys/objects/lifecycle.h"
#include "main/obj_message.h"
#include "main/gamebits.h"

extern const f32 lbl_803E2FD8;
extern const f32 lbl_803E2FDC;
extern const f32 lbl_803E2FE0;
extern const f32 gBackpackBounceDampingHorizontal;
extern const f32 gBackpackBounceDampingVertical;
extern const f32 lbl_803E2FEC;
extern const f32 gBackpackBounceRestitution;
extern const f32 lbl_803E2FF4;
extern const f32 lbl_803E2FF8;
extern const f32 lbl_803E2FFC;
extern const f32 lbl_803E3000;
extern const f32 lbl_803E3004;
extern const f32 lbl_803E3008;
extern const f32 lbl_803E300C;
extern const f32 lbl_803E3010;

#define LANDED_ARWING_SCRIPT_MODE 6

/* obj+0xB8 overlay used only by TriggerLaunchTarget; the named fields line
   up with GroundBaddieState (triggerId/gameBitA/subMode at 0x3F0/0x3F2/0x405). */
typedef struct LandedArwingTriggerLaunchTargetState
{
    u8 pad0[0x3F0 - 0x0];
    s16 launchMoveId;   /* 0x3F0 */
    s16 triggerGameBit; /* 0x3F2 */
    u8 pad3F4[0x405 - 0x3F4];
    u8 subMode;         /* 0x405 */
    u8 pad406[0x408 - 0x406];
} LandedArwingTriggerLaunchTargetState;

/* BaddieState+0x27A (moveJustStartedA): just-collided / move-just-started one-shot */
#define LANDED_ARWING_JUST_COLLIDED 0x27A
/* part of LANDED_ARWING_FLAG_LAUNCHING (0x02004000): mark launch active */
#define LANDED_ARWING_FLAG_BOUNCE 0x4000

int LandedArwing_ReturnZero(void)
{
    return 0;
}

int LandedArwing_TriggerLaunchTarget(GameObject* obj, int target)
{
    int* aux = obj->extra;
    if (*(s8*)(target + LANDED_ARWING_JUST_COLLIDED) != 0)
    {
        (*gBaddieControlInterface)
            ->spawnChild(obj, (int)((LandedArwingTriggerLaunchTargetState*)aux)->launchMoveId, -1, 0);
        (*gPlayerInterface)->spawnPartfx(obj, (void*)target, 0x3c, 0xa, 0);
        mainSetBits((int)((LandedArwingTriggerLaunchTargetState*)aux)->triggerGameBit, 1);
        ((LandedArwingTriggerLaunchTargetState*)aux)->subMode = 0;
    }
    return 0;
}

int LandedArwing_UpdateBounceFade(GameObject* obj, u32* stateWord)
{
    f32 horizontalDamping;
    LandedArwingState* state;
    ObjHitsPriorityState* hitState;

    state = (LandedArwingState*)((GroundBaddieState*)*(int*)&obj->extra)->control;
    ((BaddieState*)stateWord)->stateTag = 3;
    if (*(s8*)((int)stateWord + LANDED_ARWING_JUST_COLLIDED) != 0)
    {
        ObjHits_DisableObject(obj);
        obj->anim.velocityX = -obj->anim.velocityX;
        obj->anim.velocityY = obj->anim.velocityY + lbl_803E2FD8;
        obj->anim.velocityZ = -obj->anim.velocityZ;
        ObjAnim_SetCurrentMove((int)obj, 3, lbl_803E2FDC, 0);
        state->animSpeed = lbl_803E2FE0;
    }
    hitState = (ObjHitsPriorityState*)obj->anim.hitReactState;
    hitState->objectPairHitVolume = 0;
    *stateWord = *stateWord | LANDED_ARWING_FLAG_BOUNCE;
    obj->anim.velocityX =
        obj->anim.velocityX * (horizontalDamping = gBackpackBounceDampingHorizontal);
    obj->anim.velocityY =
        gBackpackBounceDampingVertical * (obj->anim.velocityY - lbl_803E2FEC);
    obj->anim.velocityZ = obj->anim.velocityZ * horizontalDamping;
    objMove(obj, obj->anim.velocityX, obj->anim.velocityY,
            obj->anim.velocityZ);
    if (obj->anim.localPosX < state->boundsMinX)
    {
        obj->anim.localPosX = state->boundsMinX;
        obj->anim.velocityX = gBackpackBounceRestitution * -obj->anim.velocityX;
    }
    if (obj->anim.localPosX > state->boundsMaxX)
    {
        obj->anim.localPosX = state->boundsMaxX;
        obj->anim.velocityX = gBackpackBounceRestitution * -obj->anim.velocityX;
    }
    if (obj->anim.localPosY < state->boundsMinY)
    {
        obj->anim.localPosY = state->boundsMinY;
        obj->anim.velocityY = gBackpackBounceRestitution * -obj->anim.velocityY;
    }
    if (obj->anim.localPosY > state->boundsMaxY)
    {
        obj->anim.localPosY = state->boundsMaxY;
        obj->anim.velocityY = gBackpackBounceRestitution * -obj->anim.velocityY;
    }
    if (obj->anim.localPosZ < state->boundsMinZ)
    {
        obj->anim.localPosZ = state->boundsMinZ;
        obj->anim.velocityZ = gBackpackBounceRestitution * -obj->anim.velocityZ;
    }
    if (obj->anim.localPosZ > state->boundsMaxZ)
    {
        obj->anim.localPosZ = state->boundsMaxZ;
        obj->anim.velocityZ = gBackpackBounceRestitution * -obj->anim.velocityZ;
    }
    if (lbl_803E2FF4 == obj->anim.currentMoveProgress)
    {
        ObjMsg_SendToObjects(0, 3, obj, 0xe0000, (u32)obj);
        Obj_FreeObject(obj);
        return 0;
    }
    else
    {
        obj->anim.alpha = (u8)(255 - (s32)(lbl_803E2FF8 * obj->anim.currentMoveProgress));
    }
    return 0;
}

int LandedArwing_UpdateRetreatChase(GameObject* obj, int stateWord)
{
    f32 scale;
    int player;
    LandedArwingState* state;
    GameObject* playerObj;
    f32 x;
    f32 y;
    f32 z;

    state = (LandedArwingState*)((GroundBaddieState*)*(int*)&(obj)->extra)->control;
    player = (int)Obj_GetPlayerObject();
    playerObj = (GameObject*)player;
    ((BaddieState*)stateWord)->stateTag = 1;
    if (*(s8*)(stateWord + LANDED_ARWING_JUST_COLLIDED) != 0)
    {
        state->scriptTimer = 0x3c;
        state->speed = lbl_803E2FFC;
        ObjHits_DisableObject(obj);
    }
    if (state->surfaceMode != LANDED_ARWING_SCRIPT_MODE &&
        ((u32)player == 0 || playerObj->anim.worldPosX < state->boundsMinX ||
         (playerObj->anim.worldPosX > state->boundsMaxX && playerObj->anim.worldPosY < state->boundsMinY) ||
         (playerObj->anim.worldPosY > state->boundsMaxY && playerObj->anim.worldPosZ < state->boundsMinZ) ||
         playerObj->anim.worldPosZ > state->boundsMaxZ))
    {
        x = (obj)->anim.localPosX;
        y = (obj)->anim.localPosY;
        z = (obj)->anim.localPosZ;
        scale = lbl_803E2FDC;
    }
    else
    {
        x = (obj)->anim.localPosX - lbl_803E3000 * (playerObj->anim.localPosX - (obj)->anim.localPosX);
        y = (obj)->anim.localPosY - lbl_803E3000 * (playerObj->anim.localPosY - (obj)->anim.localPosY);
        z = (obj)->anim.localPosZ - lbl_803E3000 * (playerObj->anim.localPosZ - (obj)->anim.localPosZ);
        scale = lbl_803E2FF4;
    }
    landedarwing_updateConstrainedChaseVelocity(obj, x, y, z, scale);
    if (state->surfaceMode == LANDED_ARWING_SCRIPT_MODE)
    {
        if ((u32)((state->flags92 >> 2) & 1) != 0U)
        {
            landedarwing_updateAirborneMotion(obj, state);
        }
        else
        {
            landedarwing_moveAlongSurface(obj, state);
        }
    }
    else
    {
        landedarwing_moveSurfaceCrawler(obj, state);
    }
    if ((int)state->scriptTimer <= (int)(u32)framesThisStep)
    {
        return 2;
    }
    state->scriptTimer -= framesThisStep;
    return 0;
}

ObjectDescriptor dll_D3 = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)dll_D3_initialise,
    (ObjectDescriptorCallback)dll_D3_release_nop,
    0,
    (ObjectDescriptorCallback)dll_D3_init,
    (ObjectDescriptorCallback)dll_D3_update,
    (ObjectDescriptorCallback)dll_D3_hitDetect_nop,
    (ObjectDescriptorCallback)dll_D3_render,
    (ObjectDescriptorCallback)dll_D3_free,
    (ObjectDescriptorCallback)dll_D3_getObjectTypeId,
    dll_D3_getExtraSize_ret_1188,
};

/* raw offsets used by the landed-arwing state handlers */
#define BADDIESTATE_HANDLER_TICK_FLAG 0x34d
#define BADDIESTATE_JUST_LAUNCHED 0x27a

#define LANDED_ARWING_OBJECT_PAIR_PRIORITY 9
#define LANDED_ARWING_OBJECT_PAIR_HIT_VOLUME 1

#define LANDED_ARWING_TARGET_PLAYER 0
#define LANDED_ARWING_TARGET_WANDER 1
#define LANDED_ARWING_TARGET_SCRIPT 2

#define LANDED_ARWING_FLAG_SCRIPT_TARGET 0x01
#define LANDED_ARWING_FLAG_LAUNCHING 0x02004000

#define LANDED_ARWING_REVERSE_CHASE_GAMEBIT 0x698
#define LANDED_ARWING_WANDER_TIME_MIN 0x12c
#define LANDED_ARWING_WANDER_TIME_MAX 0x258

typedef struct
{
    u8 high7 : 7;
    u8 bit0 : 1;
} LandedArwingFlags;

u32 LandedArwing_UpdateFlightChase(GameObject* obj, int state)
{
    GameObject* playerObj;
    LandedArwingState* sub;
    int targetMode;
    f32 targetX;
    f32 targetY;
    f32 targetZ;
    f32 chaseScale;
    u32 scriptFlags;

    sub = (LandedArwingState*)((GroundBaddieState*)*(int*)&obj->extra)->control;
    playerObj = Obj_GetPlayerObject();
    *(u8*)(state + BADDIESTATE_HANDLER_TICK_FLAG) = 1;

    if (*(s8*)(state + BADDIESTATE_JUST_LAUNCHED) != 0)
    {
        sub->speed = lbl_803E3004;
        ObjHits_EnableObject(obj);
        obj->anim.velocityX =
            -sub->speed * fsin16Precise(obj->anim.rotX & 0xffff);
        obj->anim.velocityY = lbl_803E2FDC;
        obj->anim.velocityZ =
            -sub->speed * fcos16Precise(obj->anim.rotX & 0xffff);
        *(u32*)state |= LANDED_ARWING_FLAG_LAUNCHING;
        ObjAnim_SetCurrentMove((int)obj, 0, lbl_803E2FDC, 0);
        sub->animSpeed = lbl_803E3008;
    }

    ObjHits_SetHitVolumeSlot((ObjAnimComponent*)obj, LANDED_ARWING_OBJECT_PAIR_PRIORITY, LANDED_ARWING_OBJECT_PAIR_HIT_VOLUME, -1);
    ((ObjHitsPriorityState *)obj->anim.hitReactState)->objectPairPriority = LANDED_ARWING_OBJECT_PAIR_PRIORITY;
    ((ObjHitsPriorityState *)obj->anim.hitReactState)->objectPairHitVolume = LANDED_ARWING_OBJECT_PAIR_HIT_VOLUME;
    ObjHits_RegisterActiveHitVolumeObject(obj);

    (*gPathControlInterface)->advance(obj, (void*)(state + 4), timeDelta);

    if (sub->surfaceMode != LANDED_ARWING_SCRIPT_MODE)
    {
        if (playerObj != NULL &&
            playerObj->anim.worldPosX >= sub->boundsMinX &&
            playerObj->anim.worldPosX <= sub->boundsMaxX &&
            playerObj->anim.worldPosY >= sub->boundsMinY &&
            playerObj->anim.worldPosY <= sub->boundsMaxY &&
            playerObj->anim.worldPosZ >= sub->boundsMinZ &&
            playerObj->anim.worldPosZ <= sub->boundsMaxZ)
        {
            targetMode = LANDED_ARWING_TARGET_PLAYER;
        }
        else
        {
            targetMode = LANDED_ARWING_TARGET_WANDER;
        }
    }
    else
    {
        scriptFlags = sub->flags92;
        if ((scriptFlags & LANDED_ARWING_FLAG_SCRIPT_TARGET) != 0)
        {
            targetMode = LANDED_ARWING_TARGET_SCRIPT;
            if ((s32)sub->scriptTimer <= framesThisStep)
            {
                ((LandedArwingFlags*)&sub->flags92)->bit0 = 0;
            }
            else
            {
                sub->scriptTimer -= framesThisStep;
            }
        }
        else
        {
            targetMode = LANDED_ARWING_TARGET_PLAYER;
        }
    }

    switch (targetMode)
    {
    case LANDED_ARWING_TARGET_PLAYER:
        targetX = playerObj->anim.localPosX;
        targetY = playerObj->anim.localPosY - lbl_803E2FD8;
        targetZ = playerObj->anim.localPosZ;
        chaseScale = lbl_803E300C;
        if (mainGetBit(LANDED_ARWING_REVERSE_CHASE_GAMEBIT) != 0)
        {
            chaseScale = -lbl_803E300C;
        }
        break;
    case LANDED_ARWING_TARGET_WANDER:
        if ((s32)sub->wanderTimer <= framesThisStep)
        {
            sub->wanderTargetX = (f32)(s32)randomGetRange((s32)sub->boundsMinX, sub->boundsMaxX);
            sub->wanderTargetY = (f32)(s32)randomGetRange((s32)sub->boundsMinY, sub->boundsMaxY);
            sub->wanderTargetZ = (f32)(s32)randomGetRange((s32)sub->boundsMinZ, sub->boundsMaxZ);
            sub->wanderTimer = randomGetRange(LANDED_ARWING_WANDER_TIME_MIN, LANDED_ARWING_WANDER_TIME_MAX);
        }
        else
        {
            sub->wanderTimer -= framesThisStep;
        }
        targetX = sub->wanderTargetX;
        targetY = sub->wanderTargetY;
        targetZ = sub->wanderTargetZ;
        chaseScale = lbl_803E3010;
        break;
    case LANDED_ARWING_TARGET_SCRIPT:
        targetX = sub->scriptTargetX;
        targetY = sub->scriptTargetY;
        targetZ = sub->scriptTargetZ;
        chaseScale = lbl_803E300C;
        break;
    }

    landedarwing_updateConstrainedChaseVelocity(obj, targetX, targetY, targetZ, chaseScale);

    if (sub->surfaceMode == LANDED_ARWING_SCRIPT_MODE)
    {
        if ((u32)((sub->flags92 >> 2) & 1) != 0)
        {
            landedarwing_updateAirborneMotion(obj, sub);
        }
        else
        {
            landedarwing_moveAlongSurface(obj, sub);
        }
    }
    else
    {
        landedarwing_moveSurfaceCrawler(obj, sub);
    }

    return 0;
}

#define STAFFACTION_HIT_VOLUME_SLOT 9

/* object group this object belongs to */
#define STAFFACTION_OBJGROUP 3
/* bounceFlags: per-wall bounce-allowed bits; each gates a bounce when the
 * surface crawler crosses that bounding-box face (and locks to that axis). */
#define BOUNCE_WALL_MINX 0x01 /* boundsMinX -> surfaceMode 0 */
#define BOUNCE_WALL_MAXX 0x02 /* boundsMaxX -> surfaceMode 1 */
#define BOUNCE_WALL_MAXZ 0x04 /* boundsMaxZ -> surfaceMode 2 */
#define BOUNCE_WALL_MINZ 0x08 /* boundsMinZ -> surfaceMode 3 */
#define BOUNCE_WALL_MAXY 0x10 /* boundsMaxY -> surfaceMode 4 */
#define BOUNCE_WALL_MINY 0x20 /* boundsMinY -> surfaceMode 5 */

typedef struct LandedArwingMovementFlags
{
    u8 boundsLookupRetries : 4;
    u8 surfaceOrientationReady : 1;
    u8 airborne : 1;
    u8 contactCallbackRegistered : 1;
    u8 hitSurfaceType13 : 1;
} LandedArwingMovementFlags;

void landedarwing_resolveSurfaceCollision(GameObject* obj, LandedArwingState* state, f32* hit, f32* end);
void landedarwing_updateConstrainedChaseVelocity(GameObject* obj, f32 targetX, f32 targetY, f32 targetZ, f32 blend);
void landedarwing_buildSurfaceOrientationMatrix(f32* out, f32* forward, f32* up);

u32 landedarwing_updateMovementState(GameObject* obj, u32* params)
{
    LandedArwingState* state;

    state = (LandedArwingState*)((GroundBaddieState*)*(int*)&obj->extra)->control;
    *(u8*)((int)params + 0x34d) = 1;
    if (*(s8*)((int)params + 0x27a) != 0)
    {
        state->speed = lbl_803E3004;
        ObjHits_EnableObject(obj);
        obj->anim.velocityX = -(state->speed) * fsin16Precise(obj->anim.rotX & 0xffff);
        obj->anim.velocityY = lbl_803E2FDC;
        obj->anim.velocityZ = -(state->speed) * fcos16Precise(obj->anim.rotX & 0xffff);
        *params |= 0x2004000;
        ObjAnim_SetCurrentMove((int)obj, 0, lbl_803E2FDC, 0);
        state->animSpeed = lbl_803E2FDC;
    }
    ObjHits_SetHitVolumeSlot((ObjAnimComponent*)obj, STAFFACTION_HIT_VOLUME_SLOT, 1, -1);
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->objectPairPriority = 9;
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->objectPairHitVolume = 1;
    ObjHits_RegisterActiveHitVolumeObject(obj);
    (*gPathControlInterface)->advance(obj, params + 1, timeDelta);
    if (*(s8*)((int)params + 0x27a) != 0)
    {
        if (state->surfaceMode == 6)
        {
            if (((state->flags92 >> 2) & 1) != 0u)
            {
                landedarwing_updateAirborneMotion(obj, state);
            }
            else
            {
                landedarwing_moveAlongSurface(obj, state);
            }
        }
        else
        {
            landedarwing_moveSurfaceCrawler(obj, state);
        }
    }
    return 0;
}

void landedarwing_updateAirborneMotion(GameObject* obj, LandedArwingState* state)
{
    f32 radius;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 start[3];
    f32 end[3];
    TrackQueryBounds bounds;
    struct
    {
        f32 hit[16];
        f32 hitRadius;
        u8 pad[0x10];
        u8 hitType;
        u8 pad2[0x1f];
    } hitScratch;
    f32 damping;
    int hitFound;

    radius = 100.0f;
    (obj)->anim.velocityY = (obj)->anim.velocityY - lbl_803E2FF4;
    (obj)->anim.velocityX = (obj)->anim.velocityX * (damping = 0.97f);
    (obj)->anim.velocityY = (obj)->anim.velocityY * damping;
    (obj)->anim.velocityZ = (obj)->anim.velocityZ * damping;
    start[0] = (obj)->anim.localPosX;
    start[1] = (obj)->anim.localPosY;
    start[2] = (obj)->anim.localPosZ;
    end[0] = start[0] + (obj)->anim.velocityX;
    end[1] = start[1] + (obj)->anim.velocityY;
    end[2] = start[2] + (obj)->anim.velocityZ;
    hitScratch.hitRadius = lbl_803E2FDC;
    hitScratch.hitType = 3;
    hitDetect_calcSweptSphereBounds(&bounds, start, end, &radius, 1);
    hitDetectFn_800691c0(obj, &bounds, 0, 1);
    hitFound = hitDetectFn_80067958(obj, start, end, 1, hitScratch.hit, 0x20);
    if (hitFound != 0)
    {
        {
            int zero = 0;
            ((LandedArwingMovementFlags*)&state->flags92)->airborne = zero;
        }
        landedarwing_resolveSurfaceCollision(obj, state, hitScratch.hit, end);
    }
    else
    {
        (obj)->anim.localPosX = end[0];
        (obj)->anim.localPosY = end[1];
        (obj)->anim.localPosZ = end[2];
    }
}

void landedarwing_moveSurfaceCrawler(GameObject* obj, LandedArwingState* state)
{
    int headingAngle;

    objMove(obj, obj->anim.velocityX, obj->anim.velocityY,
            obj->anim.velocityZ);
    switch (state->surfaceMode)
    {
    case 0:
        if (obj->anim.localPosY < state->boundsMinY)
        {
            obj->anim.localPosY = state->boundsMinY;
            if ((state->bounceFlags & BOUNCE_WALL_MINY) != 0)
            {
                obj->anim.velocityX = -obj->anim.velocityY;
                state->surfaceMode = 5;
            }
            obj->anim.velocityY = lbl_803E2FDC;
        }
        else if (obj->anim.localPosY > state->boundsMaxY)
        {
            obj->anim.localPosY = state->boundsMaxY;
            if ((state->bounceFlags & BOUNCE_WALL_MAXY) != 0)
            {
                obj->anim.velocityX = obj->anim.velocityY;
                state->surfaceMode = 4;
            }
            obj->anim.velocityY = lbl_803E2FDC;
        }
        else if (obj->anim.localPosZ > state->boundsMaxZ)
        {
            obj->anim.localPosZ = state->boundsMaxZ;
            if ((state->bounceFlags & BOUNCE_WALL_MAXZ) != 0)
            {
                obj->anim.velocityX = obj->anim.velocityZ;
                state->surfaceMode = 2;
            }
            obj->anim.velocityZ = lbl_803E2FDC;
        }
        else if (obj->anim.localPosZ < state->boundsMinZ)
        {
            obj->anim.localPosZ = state->boundsMinZ;
            if ((state->bounceFlags & BOUNCE_WALL_MINZ) != 0)
            {
                obj->anim.velocityX = -obj->anim.velocityZ;
                state->surfaceMode = 3;
            }
            obj->anim.velocityZ = lbl_803E2FDC;
        }
        break;
    case 1:
        if (obj->anim.localPosY < state->boundsMinY)
        {
            obj->anim.localPosY = state->boundsMinY;
            if ((state->bounceFlags & BOUNCE_WALL_MINY) != 0)
            {
                obj->anim.velocityX = obj->anim.velocityY;
                state->surfaceMode = 5;
            }
            obj->anim.velocityY = lbl_803E2FDC;
        }
        else if (obj->anim.localPosY > state->boundsMaxY)
        {
            obj->anim.localPosY = state->boundsMaxY;
            if ((state->bounceFlags & BOUNCE_WALL_MAXY) != 0)
            {
                obj->anim.velocityX = -obj->anim.velocityY;
                state->surfaceMode = 4;
            }
            obj->anim.velocityY = lbl_803E2FDC;
        }
        else if (obj->anim.localPosZ > state->boundsMaxZ)
        {
            obj->anim.localPosZ = state->boundsMaxZ;
            if ((state->bounceFlags & BOUNCE_WALL_MAXZ) != 0)
            {
                obj->anim.velocityX = -obj->anim.velocityZ;
                state->surfaceMode = 2;
            }
            obj->anim.velocityZ = lbl_803E2FDC;
        }
        else if (obj->anim.localPosZ < state->boundsMinZ)
        {
            obj->anim.localPosZ = state->boundsMinZ;
            if ((state->bounceFlags & BOUNCE_WALL_MINZ) != 0)
            {
                obj->anim.velocityX = obj->anim.velocityZ;
                state->surfaceMode = 3;
            }
            obj->anim.velocityZ = lbl_803E2FDC;
        }
        break;
    case 2:
        if (obj->anim.localPosX < state->boundsMinX)
        {
            obj->anim.localPosX = state->boundsMinX;
            if ((state->bounceFlags & BOUNCE_WALL_MINX) != 0)
            {
                obj->anim.velocityZ = obj->anim.velocityX;
                state->surfaceMode = 0;
            }
            obj->anim.velocityX = lbl_803E2FDC;
        }
        else if (obj->anim.localPosX > state->boundsMaxX)
        {
            obj->anim.localPosX = state->boundsMaxX;
            if ((state->bounceFlags & BOUNCE_WALL_MAXX) != 0)
            {
                obj->anim.velocityZ = -obj->anim.velocityX;
                state->surfaceMode = 1;
            }
            obj->anim.velocityX = lbl_803E2FDC;
        }
        else if (obj->anim.localPosY < state->boundsMinY)
        {
            obj->anim.localPosY = state->boundsMinY;
            if ((state->bounceFlags & BOUNCE_WALL_MINY) != 0)
            {
                obj->anim.velocityZ = obj->anim.velocityY;
                state->surfaceMode = 5;
            }
            obj->anim.velocityY = lbl_803E2FDC;
        }
        else if (obj->anim.localPosY > state->boundsMaxY)
        {
            obj->anim.localPosY = state->boundsMaxY;
            if ((state->bounceFlags & BOUNCE_WALL_MAXY) != 0)
            {
                obj->anim.velocityZ = -obj->anim.velocityY;
                state->surfaceMode = 4;
            }
            obj->anim.velocityY = lbl_803E2FDC;
        }
        break;
    case 3:
        if (obj->anim.localPosX < state->boundsMinX)
        {
            obj->anim.localPosX = state->boundsMinX;
            if ((state->bounceFlags & BOUNCE_WALL_MINX) != 0)
            {
                obj->anim.velocityZ = -obj->anim.velocityX;
                state->surfaceMode = 0;
            }
            obj->anim.velocityX = lbl_803E2FDC;
        }
        else if (obj->anim.localPosX > state->boundsMaxX)
        {
            obj->anim.localPosX = state->boundsMaxX;
            if ((state->bounceFlags & BOUNCE_WALL_MAXX) != 0)
            {
                obj->anim.velocityZ = obj->anim.velocityX;
                state->surfaceMode = 1;
            }
            obj->anim.velocityX = lbl_803E2FDC;
        }
        else if (obj->anim.localPosY < state->boundsMinY)
        {
            obj->anim.localPosY = state->boundsMinY;
            if ((state->bounceFlags & BOUNCE_WALL_MINY) != 0)
            {
                obj->anim.velocityZ = -obj->anim.velocityY;
                state->surfaceMode = 5;
            }
            obj->anim.velocityY = lbl_803E2FDC;
        }
        else if (obj->anim.localPosY > state->boundsMaxY)
        {
            obj->anim.localPosY = state->boundsMaxY;
            if ((state->bounceFlags & BOUNCE_WALL_MAXY) != 0)
            {
                obj->anim.velocityZ = obj->anim.velocityY;
                state->surfaceMode = 4;
            }
            obj->anim.velocityY = lbl_803E2FDC;
        }
        break;
    case 5:
        if (obj->anim.localPosX < state->boundsMinX)
        {
            obj->anim.localPosX = state->boundsMinX;
            if ((state->bounceFlags & BOUNCE_WALL_MINX) != 0)
            {
                obj->anim.velocityY = -obj->anim.velocityX;
                state->surfaceMode = 0;
            }
            obj->anim.velocityX = lbl_803E2FDC;
        }
        else if (obj->anim.localPosX > state->boundsMaxX)
        {
            obj->anim.localPosX = state->boundsMaxX;
            if ((state->bounceFlags & BOUNCE_WALL_MAXX) != 0)
            {
                obj->anim.velocityY = obj->anim.velocityX;
                state->surfaceMode = 1;
            }
            obj->anim.velocityX = lbl_803E2FDC;
        }
        else if (obj->anim.localPosZ > state->boundsMaxZ)
        {
            obj->anim.localPosZ = state->boundsMaxZ;
            if ((state->bounceFlags & BOUNCE_WALL_MAXZ) != 0)
            {
                obj->anim.velocityY = obj->anim.velocityZ;
                state->surfaceMode = 2;
            }
            obj->anim.velocityZ = lbl_803E2FDC;
        }
        else if (obj->anim.localPosZ < state->boundsMinZ)
        {
            obj->anim.localPosZ = state->boundsMinZ;
            if ((state->bounceFlags & BOUNCE_WALL_MINZ) != 0)
            {
                obj->anim.velocityY = -obj->anim.velocityZ;
                state->surfaceMode = 3;
            }
            obj->anim.velocityZ = lbl_803E2FDC;
        }
        break;
    case 4:
        if (obj->anim.localPosX < state->boundsMinX)
        {
            obj->anim.localPosX = state->boundsMinX;
            if ((state->bounceFlags & BOUNCE_WALL_MINX) != 0)
            {
                obj->anim.velocityY = obj->anim.velocityX;
                state->surfaceMode = 0;
            }
            obj->anim.velocityX = lbl_803E2FDC;
        }
        else if (obj->anim.localPosX > state->boundsMaxX)
        {
            obj->anim.localPosX = state->boundsMaxX;
            if ((state->bounceFlags & BOUNCE_WALL_MAXX) != 0)
            {
                obj->anim.velocityY = -obj->anim.velocityX;
                state->surfaceMode = 1;
            }
            obj->anim.velocityX = lbl_803E2FDC;
        }
        else if (obj->anim.localPosZ > state->boundsMaxZ)
        {
            obj->anim.localPosZ = state->boundsMaxZ;
            if ((state->bounceFlags & BOUNCE_WALL_MAXZ) != 0)
            {
                obj->anim.velocityY = -obj->anim.velocityZ;
                state->surfaceMode = 2;
            }
            obj->anim.velocityZ = lbl_803E2FDC;
        }
        else if (obj->anim.localPosZ < state->boundsMinZ)
        {
            obj->anim.localPosZ = state->boundsMinZ;
            if ((state->bounceFlags & BOUNCE_WALL_MINZ) != 0)
            {
                obj->anim.velocityY = obj->anim.velocityZ;
                state->surfaceMode = 3;
            }
            obj->anim.velocityZ = lbl_803E2FDC;
        }
        break;
    }

    switch (state->surfaceMode)
    {
    case 0:
        obj->anim.rotX = 0;
        headingAngle = atan2_8002178c(obj->anim.velocityZ, obj->anim.velocityY);
        obj->anim.rotY = (short)(headingAngle + 0x4000);
        obj->anim.rotZ = -0x4000;
        break;
    case 1:
        obj->anim.rotX = 0;
        headingAngle = atan2_8002178c(obj->anim.velocityZ, obj->anim.velocityY);
        obj->anim.rotY = (short)(headingAngle + 0x4000);
        obj->anim.rotZ = 0x4000;
        break;
    case 2:
        obj->anim.rotX = 0x4000;
        headingAngle = atan2_8002178c(obj->anim.velocityX, obj->anim.velocityY);
        obj->anim.rotY = (short)(headingAngle + 0x4000);
        obj->anim.rotZ = -0x4000;
        break;
    case 3:
        obj->anim.rotX = 0x4000;
        headingAngle = atan2_8002178c(obj->anim.velocityX, obj->anim.velocityY);
        obj->anim.rotY = (short)(headingAngle + 0x4000);
        obj->anim.rotZ = 0x4000;
        break;
    case 5:
        headingAngle = atan2_8002178c(obj->anim.velocityX, obj->anim.velocityZ);
        obj->anim.rotX = (short)(headingAngle + 0x8000);
        obj->anim.rotY = 0;
        obj->anim.rotZ = 0;
        break;
    case 4:
        headingAngle = atan2_8002178c(obj->anim.velocityX, obj->anim.velocityZ);
        obj->anim.rotX = (short)(headingAngle + 0x8000);
        obj->anim.rotY = 0;
        obj->anim.rotZ = -0x8000;
        break;
    }
    return;
}

void landedarwing_moveAlongSurface(GameObject* obj, LandedArwingState* state)
{
    f32 one;
    f32 distanceRemaining;
    int hitFound;
    int stepCount;
    f32 traveled;
    f32 segmentLen;
    f32 stepScale;
    f32 radius;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 start[3];
    f32 end[3];
    TrackQueryBounds bounds;
    struct
    {
        f32 hit[16];
        f32 hitRadius;
        u8 pad[0x10];
        u8 hitType;
        u8 pad2[0x10];
    } hitScratch;
    f32 speed;

    distanceRemaining = sqrtf(obj->anim.velocityZ * obj->anim.velocityZ +
                              (obj->anim.velocityX * obj->anim.velocityX +
                               obj->anim.velocityY * obj->anim.velocityY));
    traveled = lbl_803E2FDC;
    stepCount = 0;
    hitScratch.hitRadius = traveled;
    hitScratch.hitType = 3;
    start[0] = obj->anim.localPosX;
    start[1] = obj->anim.localPosY;
    start[2] = obj->anim.localPosZ;
    end[0] = start[0] + obj->anim.velocityX;
    end[1] = start[1] + obj->anim.velocityY;
    end[2] = start[2] + obj->anim.velocityZ;
    radius = 100.0f;
    hitDetect_calcSweptSphereBounds(&bounds, start, end, &radius, 1);
    hitDetectFn_800691c0(obj, &bounds, 0, 1);
    one = lbl_803E2FF4;
    while ((traveled < distanceRemaining) && (++stepCount < 10))
    {
        start[0] = obj->anim.localPosX;
        start[1] = obj->anim.localPosY;
        start[2] = obj->anim.localPosZ;
        stepScale = one - (traveled / distanceRemaining);
        end[0] = obj->anim.velocityX * stepScale + start[0];
        end[1] = obj->anim.velocityY * stepScale + start[1];
        end[2] = obj->anim.velocityZ * stepScale + start[2];
        hitFound = hitDetectFn_80067958(obj, start, end, 1, hitScratch.hit, 0x20);
        if (hitFound != 0)
        {
            dx = end[0] - start[0];
            dy = end[1] - start[1];
            dz = end[2] - start[2];
            segmentLen = sqrtf(dz * dz + (dx * dx + dy * dy));
            traveled = (f32)(traveled + segmentLen);
            landedarwing_resolveSurfaceCollision(obj, state, hitScratch.hit, end);
        }
        else
        {
            traveled = distanceRemaining;
            obj->anim.localPosX = end[0];
            obj->anim.localPosY = end[1];
            obj->anim.localPosZ = end[2];
        }
    }
    start[0] = obj->anim.localPosX;
    start[1] = obj->anim.localPosY;
    start[2] = obj->anim.localPosZ;
    end[0] = -(4.0f * state->surfaceNormalX - start[0]);
    end[1] = -(4.0f * state->surfaceNormalY - start[1]);
    end[2] = -(4.0f * state->surfaceNormalZ - start[2]);
    hitScratch.hitRadius = lbl_803E2FDC;
    hitScratch.hitType = 3;
    hitFound = hitDetectFn_80067958(obj, start, end, 1, hitScratch.hit, 0x20);
    if (hitFound != 0)
    {
        if ((((hitScratch.hit[0] != state->surfaceNormalX) ||
              (hitScratch.hit[1] != state->surfaceNormalY)) ||
             (hitScratch.hit[2] != state->surfaceNormalZ)) ||
            (hitScratch.hit[3] != state->surfacePlaneD))
        {
            landedarwing_resolveSurfaceCollision(obj, state, hitScratch.hit, end);
        }
        else
        {
            obj->anim.localPosX = end[0];
            obj->anim.localPosY = end[1];
            obj->anim.localPosZ = end[2];
        }
    }
    else
    {
        start[0] = end[0];
        start[1] = end[1];
        start[2] = end[2];
        end[0] = -obj->anim.velocityX;
        end[1] = -obj->anim.velocityY;
        end[2] = -obj->anim.velocityZ;
        Vec3_Normalize(end);
        end[0] = 10.0f * end[0] + start[0];
        end[1] = 10.0f * end[1] + start[1];
        end[2] = 10.0f * end[2] + start[2];
        hitScratch.hitRadius = lbl_803E2FDC;
        hitScratch.hitType = 3;
        hitFound = hitDetectFn_80067958(obj, start, end, 1, hitScratch.hit, 0x20);
        if (hitFound != 0)
        {
            landedarwing_resolveSurfaceCollision(obj, state, hitScratch.hit, end);
        }
        else
        {
            speed = 6.0f;
            obj->anim.velocityX = speed * state->surfaceNormalX;
            obj->anim.velocityY = speed * state->surfaceNormalY;
            obj->anim.velocityZ = speed * state->surfaceNormalZ;
            ((LandedArwingMovementFlags*)&state->flags92)->airborne = 1;
        }
    }
    ((LandedArwingMovementFlags*)&state->flags92)->surfaceOrientationReady = 1;
}

void landedarwing_resolveSurfaceCollision(GameObject* obj, LandedArwingState* state, f32* hit, f32* end)
{
    f32 speed;
    f32 planeX;
    f32 planeZ;
    f32 planeY;
    f32 planeW;
    f32 response[3];
    f32 plane[4];
    f32 scale;
    f32 velX;
    f32 velY;
    f32 velZ;
    f32 stateX;
    f32 stateY;
    f32 stateZ;
    f32 objX;
    f32 objY;
    f32 objZ;
    f32 len;

    scale = 100.0f;
    stateX = scale * state->surfaceNormalX + (objX = obj->anim.localPosX);
    stateY = scale * state->surfaceNormalY + (objY = obj->anim.localPosY);
    stateZ = scale * state->surfaceNormalZ + (objZ = obj->anim.localPosZ);
    velX = scale * obj->anim.velocityX + objX;
    velY = scale * obj->anim.velocityY + objY;
    velZ = scale * obj->anim.velocityZ + objZ;
    planeX = objY * (stateZ - velZ) + (stateY * (velZ - objZ) + velY * (objZ - stateZ));
    planeY = objZ * (stateX - velX) + (stateZ * (velX - objX) + velZ * (objX - stateX));
    planeZ = objX * (stateY - velY) + (stateX * (velY - objY) + velX * (objY - stateY));
    len = sqrtf(planeZ * planeZ + (planeX * planeX + planeY * planeY));
    if (len > lbl_803E2FDC)
    {
        len = lbl_803E2FF4 / len;
        planeX *= len;
        planeY *= len;
        planeZ *= len;
    }
    planeW = -(stateZ * planeZ + (stateX * planeX + stateY * planeY));
    plane[0] = planeX;
    plane[1] = planeY;
    plane[2] = planeZ;
    plane[3] = planeW;
    Vec3_Cross(plane, hit, response);
    Vec3_Normalize(response);
    speed = lbl_803E3004;
    obj->anim.velocityX = lbl_803E3004 * response[0];
    obj->anim.velocityY = speed * response[1];
    obj->anim.velocityZ = speed * response[2];
    state->surfaceNormalX = hit[0];
    state->surfaceNormalY = hit[1];
    state->surfaceNormalZ = hit[2];
    state->surfacePlaneD = hit[3];
    obj->anim.localPosX = end[0] + state->surfaceNormalX;
    obj->anim.localPosY = end[1] + state->surfaceNormalY;
    obj->anim.localPosZ = end[2] + state->surfaceNormalZ;
}

void landedarwing_updateConstrainedChaseVelocity(GameObject* obj, f32 targetX, f32 targetY, f32 targetZ, f32 blend)
{
    LandedArwingState* state;
    int mode;
    f32 vx;
    f32 vy;
    f32 vz;
    f32 len;
    f32 scale;
    f32 dot;

    state = (LandedArwingState*)((GroundBaddieState*)*(int*)&(obj)->extra)->control;
    if ((u32)(state->flags92 >> 2 & 1) == 0)
    {
        vx = targetX - (obj)->anim.localPosX;
        vy = targetY - (obj)->anim.localPosY;
        vz = targetZ - (obj)->anim.localPosZ;
        len = sqrtf(vz * vz + (vx * vx + vy * vy));
        if (len >= lbl_803E2FDC)
        {
            scale = state->speed / len;
            vx *= scale;
            vy *= scale;
            vz *= scale;
        }
        vx = blend * (vx - (obj)->anim.velocityX) + (obj)->anim.velocityX;
        vy = blend * (vy - (obj)->anim.velocityY) + (obj)->anim.velocityY;
        vz = blend * (vz - (obj)->anim.velocityZ) + (obj)->anim.velocityZ;
        mode = state->surfaceMode;
        switch (mode)
        {
        case 0:
        case 1:
            vx = *(f32*)&lbl_803E2FDC;
            len = sqrtf(vy * vy + vz * vz);
            if (len != lbl_803E2FDC)
            {
                scale = state->speed / len;
                vy *= scale;
                vz *= scale;
            }
            break;
        case 2:
        case 3:
            vz = *(f32*)&lbl_803E2FDC;
            len = sqrtf(vx * vx + vy * vy);
            if (len != lbl_803E2FDC)
            {
                scale = state->speed / len;
                vx *= scale;
                vy *= scale;
            }
            break;
        case 4:
        case 5:
            vy = *(f32*)&lbl_803E2FDC;
            len = sqrtf(vx * vx + vz * vz);
            if (len != lbl_803E2FDC)
            {
                scale = state->speed / len;
                vx *= scale;
                vz *= scale;
            }
            break;
        case 6:
            dot = vz * state->surfaceNormalZ + (vx * state->surfaceNormalX + vy * state->surfaceNormalY);
            vx = -(dot * state->surfaceNormalX - vx);
            vy = -(dot * state->surfaceNormalY - vy);
            vz = -(dot * state->surfaceNormalZ - vz);
            len = sqrtf(vz * vz + (vx * vx + vy * vy));
            if (len != lbl_803E2FDC)
            {
                scale = state->speed / len;
                vx *= scale;
                vy *= scale;
                vz *= scale;
            }
            break;
        }
        (obj)->anim.velocityX = vx;
        (obj)->anim.velocityY = vy;
        (obj)->anim.velocityZ = vz;
    }
}

int dll_D3_getExtraSize_ret_1188(void)
{
    return 0x4a4;
}

int dll_D3_getObjectTypeId(void)
{
    return 0x49;
}

void dll_D3_free(GameObject* obj)
{
    int* inner = obj->extra;
    ObjGroup_RemoveObject((int)obj, STAFFACTION_OBJGROUP);
    if (obj->childObjs[0] != NULL)
    {
        Obj_FreeObject(obj->childObjs[0]);
        *(int*)&obj->childObjs[0] = 0;
    }
    (*gBaddieControlInterface)->releaseState(obj, inner, 0);
}

void dll_D3_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible)
{
    LandedArwingState* state;
    f32* slideMtx;
    f32 mtx[15];
    f32 scale;

    state = (LandedArwingState*)((GroundBaddieState*)*(int*)&(obj)->extra)->control;
    slideMtx = &state->unk_04;
    if (visible != 0)
    {
        switch ((obj)->userData1)
        {
        case 0:
            if ((state->surfaceMode == 6) &&
                ((((u32)state->flags92 >> 3) & 1) != 0))
            {
                if ((((u32)state->flags92 >> 2) & 1) == 0)
                {
                    landedarwing_buildSurfaceOrientationMatrix(
                        slideMtx, &(obj)->anim.velocityX, &state->surfaceNormalX);
                }
                scale = (obj)->anim.rootMotionScale;
                initRotationMtx(mtx, scale, scale, scale);
                mtx44_mult(mtx, slideMtx, mtx);
                mtx[12] = (obj)->anim.localPosX - playerMapOffsetX;
                mtx[13] = (obj)->anim.localPosY;
                mtx[14] = (obj)->anim.localPosZ - playerMapOffsetZ;
                objSetModelMatrixOverride((f32*)mtx);
                objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, lbl_803E2FF4);
                objSetModelMatrixOverride(NULL);
            }
            else
            {
                objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, lbl_803E2FF4);
            }
            break;
        }
    }
}

void landedarwing_buildSurfaceOrientationMatrix(f32* out, f32* forward, f32* up)
{
    f32 rt[3];
    f32 upRecomputed[3];
    f32 fwd[3];
    fwd[0] = forward[0];
    fwd[1] = forward[1];
    fwd[2] = forward[2];
    Vec3_Normalize(fwd);
    Vec3_Cross(up, fwd, rt);
    Vec3_Normalize(rt);
    Vec3_Cross(rt, fwd, upRecomputed);
    Vec3_Normalize(upRecomputed);
    {
        f32(*mat)[4] = (f32(*)[4])out;
        mat[0][0] = -rt[0];
        mat[0][1] = -rt[1];
        mat[0][2] = -rt[2];
        mat[1][0] = -upRecomputed[0];
        mat[1][1] = -upRecomputed[1];
        mat[1][2] = -upRecomputed[2];
        mat[2][0] = -fwd[0];
        mat[2][1] = -fwd[1];
        mat[2][2] = -fwd[2];
    }
}

void dll_D3_hitDetect_nop(void)
{
}

typedef struct DllD3Placement
{
    u8 pad0[0x8 - 0x0];
    f32 posX;
    f32 posY;
    f32 posZ;
    u8 pad14[0x2E - 0x14];
    s8 seqIndex;
    u8 pad2F[0x30 - 0x2F];
} DllD3Placement;

void* gLandedArwingStateHandlers[6];
int gStaffActionHitLightParams[6];
void* gLandedArwingDefaultStateHandler;

void dll_D3_update(GameObject* obj)
{
    int trans;
    int* state;
    LandedArwingState* extra;
    GameObject* player;
    int hitCount;
    int rc;
    int hits;
    f32 vec[4];
    int hitResult[21];
#define searchRadius vec[0]
#define dx           vec[1]
#define dy           vec[2]
#define dz           vec[3]

    trans = *(int*)&obj->anim.placementData;
    state = obj->extra;
    extra = (LandedArwingState*)((GroundBaddieState*)state)->control;
    player = Obj_GetPlayerObject();
    searchRadius = 1e+06f;

    if (extra->boundsObj == NULL)
    {
        extra->surfaceMode = 6;
        if (((u32)extra->flags92 >> 4 & 0xF) != 0u)
        {
            if ((extra->boundsObj = ObjList_FindNearestObjectByDefNo(obj, 0x4ad, &searchRadius)) != NULL)
            {
                (*(void (**)(int, int, int))(*(int**)*(int**)(*(int*)&extra->boundsObj + 0x68) + 0x20 / 4))(
                    *(int*)&extra->boundsObj, (int)&extra->boundsMinX, (int)&extra->bounceFlags);
                extra->surfaceMode = 5;
            }
            ((LandedArwingMovementFlags*)&extra->flags92)->boundsLookupRetries -= 1;
        }
    }

    if (obj->userData1 != 0)
        return;

    if (obj->userData2 == 0)
    {
        obj->anim.localPosX = ((DllD3Placement*)trans)->posX;
        obj->anim.localPosY = ((DllD3Placement*)trans)->posY;
        obj->anim.localPosZ = ((DllD3Placement*)trans)->posZ;
        (*gObjectTriggerInterface)->runSequence(((DllD3Placement*)trans)->seqIndex, obj, -1);
        obj->userData2 = 1;
        return;
    }

    rc = (*gBaddieControlInterface)->isObjectValid(obj, state, 0);
    if (rc == 0)
        return;

    if (((LandedArwingMovementFlags*)&extra->flags92)->contactCallbackRegistered == 0u)
    {
    if (ObjContact_AddCallback(obj, player, fn_80167550) != 0)
        {
            ((LandedArwingMovementFlags*)&extra->flags92)->contactCallbackRegistered = 1;
        }
    }

    ObjAnim_AdvanceCurrentMove((int)obj, extra->animSpeed, timeDelta, NULL);

    if (((TreasureChestState*)state)->targetState != 1)
    {
        rc = (int)(*gBaddieControlInterface)
                 ->findAggroTarget(obj, state,
                                   (f32)(u32)((TreasureChestState*)state)->aggroRange, 0x8000);
        if (rc != 0u)
        {
            (*gBaddieControlInterface)
                ->startHitReaction(obj, state, (char*)state + 0x35c,
                                   ((TreasureChestState*)state)->gameBitB, NULL, 0, 1, 0, -1);
            ((TreasureChestState*)state)->targetObj = rc;
            ((TreasureChestState*)state)->hasTarget = 0;
            ((TreasureChestState*)state)->targetState = 1;
            ((TreasureChestState*)state)->subMode = 2;
        }

        if ((u32)((TreasureChestState*)state)->targetObj != 0 && ((TreasureChestState*)state)->targetState == 2)
        {
            if (((TreasureChestState*)state)->targetDistance <= (f32)(u32)((TreasureChestState*)state)->aggroRange)
            {
                ((TreasureChestState*)state)->targetState = 1;
            }
        }
    }

    if (((TreasureChestState*)state)->targetObj != 0u)
    {
        dx = ((GameObject*)(((TreasureChestState*)state)->targetObj))->anim.worldPosX -
             obj->anim.worldPosX;
        dy = ((GameObject*)(((TreasureChestState*)state)->targetObj))->anim.worldPosY -
             obj->anim.worldPosY;
        dz = ((GameObject*)(((TreasureChestState*)state)->targetObj))->anim.worldPosZ -
             obj->anim.worldPosZ;
        ((TreasureChestState*)state)->targetDistance = sqrtf(dz * dz + (dx * dx + dy * dy));
    }

    (*gBaddieControlInterface)
        ->processMessages(obj, state, (char*)state + 0x35c,
                          ((TreasureChestState*)state)->gameBitB, NULL, 0, 0, 0);

    hits = (int)((TreasureChestState*)state)->hitPoints;
    if (hits > 0)
    {
        (*gBaddieControlInterface)
            ->updateHitReaction(obj, state, (void*)((int)state + 0x35c),
                                ((TreasureChestState*)state)->gameBitB, lbl_803202E8, lbl_80320360, 0,
                                gStaffActionHitLightParams);
        if ((int)((TreasureChestState*)state)->hitPoints < hits)
        {
            (*(void (**)(int))(*(int**)*(int**)(*(int*)&player->childObjs[0] + 0x68) + 0x50 / 4))(
                *(int*)&player->childObjs[0]);
            *(f32*)((char*)gStaffActionHitLightParams + 0xc) = obj->anim.localPosX;
            *(f32*)((char*)gStaffActionHitLightParams + 0x10) = obj->anim.localPosY;
            *(f32*)((char*)gStaffActionHitLightParams + 0x14) = obj->anim.localPosZ;
            objLightFn_8009a1dc(obj, 0.014f, gStaffActionHitLightParams, 1, 0);
        }
    }

    (*gBaddieControlInterface)
        ->updateGravity(obj, state, lbl_803E2FDC, -1);

    ((TreasureChestState*)state)->savedObjC0 = *(int*)&obj->pendingParentObj;
    *(int*)&obj->pendingParentObj = 0;

    (*gPlayerInterface)->update(obj, state, timeDelta, timeDelta, gLandedArwingStateHandlers,
                                &gLandedArwingDefaultStateHandler);

    *(int*)&obj->pendingParentObj = ((TreasureChestState*)state)->savedObjC0;

    if (((LandedArwingMovementFlags*)&extra->flags92)->hitSurfaceType13 == 0u && extra->surfaceMode == 6)
    {
        hitCount = objBboxFn_800640cc(&obj->anim.previousLocalPosX, &obj->anim.localPosX,
                                      6.0f, 0, (TrackBBoxHit*)hitResult, obj, -0x7c, -1, 0xff,
                                      0);
        if (hitCount != 0 && *(s8*)((char*)hitResult + 0x50) == 13)
        {
            ((LandedArwingMovementFlags*)&extra->flags92)->hitSurfaceType13 = 1;
            extra->scriptTimer = (u16)(randomGetRange(10, 0xf) * 0x3c);
        }
    }
}
#undef searchRadius
#undef dx
#undef dy
#undef dz

void dll_D3_init(GameObject* obj, int def, int flag)
{
    int state;
    LandedArwingState* extra;
    u8 setupFlags;
    f32 fz;
    int ftag;

    state = *(int*)&(obj)->extra;
    setupFlags = 6;
    if (flag != 0)
    {
        setupFlags |= 1;
    }
    (*gBaddieControlInterface)
        ->initGroundBaddie(obj, (u8*)def, (u8*)state, 5, 1, 0x108, setupFlags, 20.0f);
    (obj)->animEventCallback = NULL;

    extra = (LandedArwingState*)((GroundBaddieState*)state)->control;
    memset((void*)extra, 0, 0x94);
    extra->surfaceMode = 5;
    ((LandedArwingMovementFlags*)&extra->flags92)->boundsLookupRetries = 3;
    fz = lbl_803E2FDC;
    extra->surfaceNormalX = fz;
    extra->surfaceNormalY = lbl_803E2FF4;
    extra->surfaceNormalZ = fz;
    extra->surfacePlaneD = -(obj)->anim.localPosY;
    extra->scriptTargetX = (obj)->anim.localPosX;
    extra->scriptTargetY = (obj)->anim.localPosY;
    extra->scriptTargetZ = (obj)->anim.localPosZ;

    ObjAnim_SetCurrentMove((int)obj, 0, fz, 0);
    if (*(u8*)(def + 0x2b) == 0)
    {
        ftag = 0;
    }
    else
    {
        ftag = 1;
    }
    ((TreasureChestState*)state)->controlMode = ftag;
    ((TreasureChestState*)state)->substate = 0;
    ((TreasureChestState*)state)->targetState = 0;
    ((TreasureChestState*)state)->subMode = 0;
    ((TreasureChestState*)state)->physicsActive = 0;
    ObjHits_DisableObject(obj);

    fz = lbl_803E2FF4;
    extra->unk_04 = fz;
    extra->unk_18 = fz;
    extra->unk_2C = fz;
    extra->unk_40 = fz;
}

void fn_80167550(GameObject* obj, GameObject* otherObj)
{
    int* state = obj->extra;
    (*gPlayerInterface)->setState(obj, state, 2);
}

void dll_D3_release_nop(void)
{
}

void dll_D3_initialise(void)
{
    gLandedArwingStateHandlers[0] = landedarwing_updateMovementState;
    gLandedArwingStateHandlers[1] = LandedArwing_UpdateFlightChase;
    gLandedArwingStateHandlers[2] = LandedArwing_UpdateRetreatChase;
    gLandedArwingStateHandlers[3] = LandedArwing_UpdateBounceFade;
    gLandedArwingStateHandlers[4] = LandedArwing_TriggerLaunchTarget;
    gLandedArwingDefaultStateHandler = LandedArwing_ReturnZero;
}
