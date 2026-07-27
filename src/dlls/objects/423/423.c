#include "main/dll/partfx_interface.h"
#include "main/dll_000A_expgfx.h"
#include "main/objfx.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "game/objects/object.h"
#include "main/track_bbox_api.h"
#include "sys/objects/lifecycle.h"
#include "main/obj_group.h"
#include "main/obj_message.h"
#include "sys/objects.h"
#include "main/dll/objfsa.h"
#include "main/dll/rom_curve_interface.h"
#include "main/dll/SH/SHthorntail_internal.h"
#include "main/audio/sfx_ids.h"
#include "main/audio/sfx.h"
#include "main/dll/ediblemushroom.h"
#include "main/objhits.h"
#include "main/gamebits.h"
#include "main/gameloop_gamebit_api.h"
#include "main/audio/sfx_trigger_ids.h"
#include "dlls/object_descriptor.h"
#include "main/frame_timing.h"
#include "main/gamebit_ids.h"
#include "main/vecmath_distance_api.h"
#include "main/track_dolphin_api.h"
#include "main/dll/NW/dll_01A7_ediblemushroom.h"
#include "main/vecmath.h"

#define EDIBLEMUSHROOM_OBJFLAG_HIDDEN       0x4000
#define EDIBLEMUSHROOM_OBJFLAG_PARENT_SLACK 0x1000
#define EDIBLEMUSHROOM_OBJFLAG_RENDERED     0x800

/* EdibleMushroomState::flags bits */
#define EDIBLEMUSHROOM_FLAG_ANIM_DONE     0x1     /* current move finished this frame */
#define EDIBLEMUSHROOM_FLAG_ON_CURVE      0x2     /* follows a rom-curve path (type 4/5) */
#define EDIBLEMUSHROOM_FLAG_GROUNDED      0x4     /* landed on a floor hit */
#define EDIBLEMUSHROOM_FLAG_MOVING        0x8     /* nonzero root speed this frame */
#define EDIBLEMUSHROOM_FLAG_STRUCK        0x10    /* hit by the player this cycle */
#define EDIBLEMUSHROOM_MSG_IN_RANGE       0x7000a /* sent to player when grab is offered */
#define EDIBLEMUSHROOM_OBJGROUP           0x47

/* the white-mushroom variant, the only one with a pickup particle burst;
   retail OBJECTS.bin name "SH_whitemus" (DLL 0x1A7) */
#define EDIBLEMUSHROOM_OBJ_WHITE 0x658
/* attacker exempt from the struck-by-player reaction; retail OBJECTS.bin name
   "DR_EarthWar" (DLL 0x257) */
#define EDIBLEMUSHROOM_ATTACKER_EARTHWALKER 0x416
#define EDIBLEMUSHROOM_OBJGROUP_SECONDARY 0x31

/* effect emitted on the tailSwingFxTimer tick while idle/rendered */
#define EDIBLEMUSHROOM_PARTFX_TAIL_SWING 0x7f0
/* spore puff emitted on the sporePuffTimer tick during the burrow/attack state */
#define EDIBLEMUSHROOM_PARTFX_SPORE_PUFF 0x51d

void EdibleMushroom_update(GameObject* self);
void EdibleMushroom_hitDetect(GameObject* obj);
s16 EdibleMushroom_findClearApproachAngle(GameObject* obj, GameObject* player, EdibleMushroomState* state, f32 dist);

s16 gEdibleMushroomMoveIdTable[12] = {0, 1, 6, 2, 3, 4, 0, 5, 6, 7, -1, 0};

f32 gEdibleMushroomAnimEventTable[] = {0.005f, 0.01f, 0.005f, 0.01f,  0.01f, 0.015f,
                                       0.005f, 0.01f, 0.005f, 0.012f, 0.0f};

ObjectDescriptor gEdibleMushroomObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    0,
    0,
    0,
    (ObjectDescriptorCallback)EdibleMushroom_init,
    (ObjectDescriptorCallback)EdibleMushroom_update,
    (ObjectDescriptorCallback)EdibleMushroom_hitDetect,
    0,
    (ObjectDescriptorCallback)EdibleMushroom_free,
    0,
    EdibleMushroom_getExtraSize,
};

int EdibleMushroom_SeqFn(GameObject* obj)
{
    ((EdibleMushroomState*)obj->extra)->seqResetPending = 1;
    return 0;
}

void edibleMushroomFn_801d083c(GameObject* obj, EdibleMushroomState* state, u8* other)
{
    GameObject* player;
    int sval;
    u32 animState;
    int curMove;
    int moveId;
    int bit;
    f32 dz;
    f32 dx;
    f32 speed;
    f32 rangeSq;
    f32 timer;
    s16 ang;
    f32 animOut[7];
    struct
    {
        u8 pad[0xc];
        f32 x;
        f32 y;
        f32 z;
    } fx;
    f32 sunTime;

    player = Obj_GetPlayerObject();

    if (state->flags & EDIBLEMUSHROOM_FLAG_GROUNDED)
    {
        state->animState = 6;
    }

    speed = oneOverTimeDelta * (state->previousTargetDistance -
                                state->currentTargetDistance);

    sval = state->animState;
    switch (sval)
    {
    case 0:
        if (state->flags & EDIBLEMUSHROOM_FLAG_STRUCK)
        {
            state->animState = 9;
        }
        else if ((*gSkyInterface)->getSunPosition(&sunTime) == 0)
        {
            if (state->currentTargetDistance < other[0x19])
            {
                if (state->flags & EDIBLEMUSHROOM_FLAG_ON_CURVE)
                {
                    rangeSq = state->lungeRange * state->lungeRange;
                    while (1)
                    {
                        dx = state->curve.posX - obj->anim.localPosX;
                        dz = state->curve.posZ - obj->anim.localPosZ;
                        if (dx * dx + dz * dz < rangeSq)
                        {
                            if (Curve_AdvanceAlongPath(&state->curve.curve,
                                                       state->curveAdvanceStep) != 0 ||
                                state->curve.atSegmentEnd != 0)
                            {
                                (*gRomCurveInterface)->goNextPoint(&state->curve);
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                    ang = getAngle(-dx, -dz);
                    state->moveAngle = ang;
                }
                else
                {
                    state->moveAngle =
                        EdibleMushroom_findClearApproachAngle(obj, player, state, state->lungeRange);
                }
                state->animState = 1;
                Sfx_PlayFromObject((u32)obj, SFXTRIG_mushrele16);
                obj->anim.rotX = (s16)(state->moveAngle - 0x4000);
            }
            else if (state->currentTargetDistance < other[0x1f])
            {
                state->animState = 3;
            }
        }
        else
        {
            timer = (state->tailSwingFxTimer -= timeDelta);
            if (timer <= 0.0f)
            {
                if (obj->objectFlags & EDIBLEMUSHROOM_OBJFLAG_RENDERED)
                {
                    fx.x = obj->anim.worldPosX;
                    fx.y = 18.0f + obj->anim.worldPosY;
                    fx.z = obj->anim.worldPosZ;
                    (*gPartfxInterface)->spawnObject(obj, EDIBLEMUSHROOM_PARTFX_TAIL_SWING, &fx, 0x200001, -1, NULL);
                }
                state->tailSwingFxTimer = 30.0f;
            }
        }
        break;
    case 1:
        if (state->flags & EDIBLEMUSHROOM_FLAG_STRUCK)
        {
            state->animState = 9;
        }
        else if (state->flags & EDIBLEMUSHROOM_FLAG_ANIM_DONE)
        {
            state->animState = 0;
        }
        break;
    case 3:
    case 7:
        if (state->flags & EDIBLEMUSHROOM_FLAG_STRUCK)
        {
            state->animState = 9;
            break;
        }
        if (state->flags & EDIBLEMUSHROOM_FLAG_ANIM_DONE)
        {
            if (sval == 3u)
            {
                state->animState = 4;
            }
            else
            {
                state->animState = 0;
            }
            break;
        }
        /* fall through */
    case 4:
        if (state->flags & EDIBLEMUSHROOM_FLAG_STRUCK)
        {
            state->animState = 9;
        }
        else
        {
            ang = getAngle(-(obj->anim.localPosX - player->anim.localPosX),
                           -(obj->anim.localPosZ - player->anim.localPosZ));
            obj->anim.rotX = ang;
            if (state->currentTargetDistance > 10.0f + other[0x1f])
            {
                state->animState = 7;
            }
            else if (state->currentTargetDistance < other[0x19])
            {
                Sfx_PlayFromObject((u32)obj, SFXTRIG_mushrele16);
                if (speed >= 0.54f)
                {
                    if (state->flags & EDIBLEMUSHROOM_FLAG_ON_CURVE)
                    {
                        rangeSq = state->lungeRange * state->lungeRange;
                        while (1)
                        {
                            dx = state->curve.posX - obj->anim.localPosX;
                            dz = state->curve.posZ - obj->anim.localPosZ;
                            if (dx * dx + dz * dz < rangeSq)
                            {
                                if (Curve_AdvanceAlongPath(&state->curve.curve,
                                                           state->curveAdvanceStep) != 0 ||
                                    state->curve.atSegmentEnd != 0)
                                {
                                    (*gRomCurveInterface)->goNextPoint(&state->curve);
                                }
                            }
                            else
                            {
                                break;
                            }
                        }
                        ang = getAngle(-dx, -dz);
                        state->moveAngle = ang;
                    }
                    else
                    {
                        state->moveAngle =
                            EdibleMushroom_findClearApproachAngle(obj, player, state, state->lungeRange);
                    }
                    state->animState = 1;
                    obj->anim.rotX = (s16)(state->moveAngle - 0x4000);
                }
                else
                {
                    if (state->flags & EDIBLEMUSHROOM_FLAG_ON_CURVE)
                    {
                        rangeSq =
                            state->retreatRange * state->retreatRange;
                        while (1)
                        {
                            dx = state->curve.posX - obj->anim.localPosX;
                            dz = state->curve.posZ - obj->anim.localPosZ;
                            if (dx * dx + dz * dz < rangeSq)
                            {
                                if (Curve_AdvanceAlongPath(&state->curve.curve,
                                                           state->curveAdvanceStep) != 0 ||
                                    state->curve.atSegmentEnd != 0)
                                {
                                    (*gRomCurveInterface)->goNextPoint(&state->curve);
                                }
                            }
                            else
                            {
                                break;
                            }
                        }
                        ang = getAngle(-dx, -dz);
                        state->moveAngle = ang;
                    }
                    else
                    {
                        state->moveAngle =
                            EdibleMushroom_findClearApproachAngle(obj, player, state, state->retreatRange);
                    }
                    state->animState = 5;
                    obj->anim.rotX = state->moveAngle;
                }
            }
        }
        break;
    case 5:
        if ((state->flags & (EDIBLEMUSHROOM_FLAG_STRUCK | EDIBLEMUSHROOM_FLAG_ANIM_DONE)) ==
            (EDIBLEMUSHROOM_FLAG_STRUCK | EDIBLEMUSHROOM_FLAG_ANIM_DONE))
        {
            state->animState = 9;
        }
        if (state->currentTargetDistance > 10.0f + other[0x19] &&
            (state->flags & EDIBLEMUSHROOM_FLAG_ANIM_DONE))
        {
            state->animState = 4;
        }
        else if (speed >= 0.54f)
        {
            if (state->flags & EDIBLEMUSHROOM_FLAG_ON_CURVE)
            {
                rangeSq = state->lungeRange * state->lungeRange;
                while (1)
                {
                    dx = state->curve.posX - obj->anim.localPosX;
                    dz = state->curve.posZ - obj->anim.localPosZ;
                    if (dx * dx + dz * dz < rangeSq)
                    {
                        if (Curve_AdvanceAlongPath(&state->curve.curve,
                                                   state->curveAdvanceStep) != 0 ||
                            state->curve.atSegmentEnd != 0)
                        {
                            (*gRomCurveInterface)->goNextPoint(&state->curve);
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                ang = getAngle(-dx, -dz);
                state->moveAngle = ang;
            }
            else
            {
                state->moveAngle =
                    EdibleMushroom_findClearApproachAngle(obj, player, state, state->lungeRange);
            }
            state->animState = 1;
            Sfx_PlayFromObject((u32)obj, SFXTRIG_mushrele16);
            obj->anim.rotX = (s16)(state->moveAngle - 0x4000);
        }
        break;
    case 9:
        ObjHits_ClearSourceMask((ObjAnimComponent*)obj, 1);
        Sfx_KeepAliveLoopedObjectSound((u32)obj, SFXTRIG_cagelp_c);
        if (state->burrowAttackTimer <= 0.0f)
        {
            state->burrowAttackTimer = (f32)(int)randomGetRange(0xf0, 0x12c);
        }
        timer = state->burrowAttackTimer - timeDelta;
        state->burrowAttackTimer = timer;
        if (timer <= 0.0f)
        {
            ObjHits_SetSourceMask((ObjAnimComponent*)obj, 1);
            (*gExpgfxInterface)->freeSource((int)obj);
            state->animState = 0;
            state->flags &= ~EDIBLEMUSHROOM_FLAG_STRUCK;
        }
        else
        {
            timer = state->sporePuffTimer - timeDelta;
            state->sporePuffTimer = timer;
            if (timer <= 0.0f)
            {
                fx.x = 10.0f;
                fx.y = 12.0f;
                if (obj->objectFlags & EDIBLEMUSHROOM_OBJFLAG_RENDERED)
                {
                    (*gPartfxInterface)->spawnObject(obj, EDIBLEMUSHROOM_PARTFX_SPORE_PUFF, &fx, 2, -1, NULL);
                }
                state->sporePuffTimer = 20.0f;
            }
            if (mainGetBit(GAMEBIT_ITEM_TrickyFood_GrabInProgress) == 0)
            {
                if (!(player->objectFlags & EDIBLEMUSHROOM_OBJFLAG_PARENT_SLACK))
                {
                    if (Vec_xzDistance(&player->anim.worldPosX, &obj->anim.worldPosX) < 25.0f)
                    {
                        (*gExpgfxInterface)->freeSource((int)obj);
                        if (obj->anim.seqId == EDIBLEMUSHROOM_OBJ_WHITE)
                        {
                            state->pickupMsgBitId = 0x18a;
                            itemPickupDoParticleFx(obj, 1.0f, 0xff, 0x28);
                        }
                        else
                        {
                            state->pickupMsgBitId = 0x119;
                            itemPickupDoParticleFx(obj, 1.0f, 6, 0x28);
                        }
                        state->pickupMsgValue = 0;
                        state->pickupMsgDelay = 0.4f;
                        ObjMsg_SendToObject(player, EDIBLEMUSHROOM_MSG_IN_RANGE, obj, (u32)&state->pickupMsgBitId);
                        bit = *(s16*)(other + 0x1a);
                        if (bit != -1)
                        {
                            mainSetBits(bit, 1);
                        }
                        state->animState = 8;
                        mainSetBits(GAMEBIT_ITEM_TrickyFood_GrabInProgress, 1);
                    }
                }
            }
        }
        break;
    case 6:
        if (state->flags & EDIBLEMUSHROOM_FLAG_STRUCK)
        {
            state->animState = 9;
        }
        break;
    case 2:
    case 8:
    case 10:
        break;
    }

    curMove = obj->anim.currentMove;
    moveId = gEdibleMushroomMoveIdTable[state->animState];
    if (curMove != moveId && moveId != -1)
    {
        ObjAnim_SetCurrentMove((int)obj, moveId, 0.25f, 0);
    }

    if (ObjAnim_AdvanceCurrentMove(
            (int)obj, gEdibleMushroomAnimEventTable[state->animState], timeDelta,
            (ObjAnimEventList*)animOut) != 0)
    {
        state->flags |= EDIBLEMUSHROOM_FLAG_ANIM_DONE;
    }
    else
    {
        state->flags &= ~EDIBLEMUSHROOM_FLAG_ANIM_DONE;
    }

    animState = state->animState;
    if (animState == 1)
    {
        speed = state->lungeRootSpeedScale * (animOut[0] * oneOverTimeDelta);
    }
    else if (animState == 5)
    {
        speed = animOut[2] * oneOverTimeDelta;
    }
    else
    {
        speed = 0.0f;
    }

    if (0.0f != speed)
    {
        state->flags |= EDIBLEMUSHROOM_FLAG_MOVING;
    }
    else
    {
        state->flags &= ~EDIBLEMUSHROOM_FLAG_MOVING;
    }

    obj->anim.velocityX =
        speed * mathSinf((3.1415927f * (f32)state->moveAngle) / 32768.0f);
    obj->anim.velocityZ =
        speed * mathCosf((3.1415927f * (f32)state->moveAngle) / 32768.0f);

    objMove(obj, obj->anim.velocityX * timeDelta, 0.0f, obj->anim.velocityZ * timeDelta);
}

s16 EdibleMushroom_findClearApproachAngle(GameObject* obj, GameObject* player, EdibleMushroomState* state, f32 dist)
{
    s16 angle;
    s16 anglePlus;
    s16 angleMinus;
    int i;
    f32 rad;
    f32 cos0;
    f32 sin0;
    f32 cosM;
    f32 cosP;
    f32 sinM;
    f32 sinP;
    f32 cosStepP;
    f32 cosStepM;
    f32 sinStepP;
    f32 sinStepM;
    f32 vec[3];

    angle = getAngle(-(obj->anim.localPosX - player->anim.localPosX),
                     -(obj->anim.localPosZ - player->anim.localPosZ));
    rad = (3.1415927f * angle) / 32768.0f;
    cos0 = mathSinf(rad);
    sin0 = mathCosf(rad);
    vec[0] = obj->anim.localPosX - dist * cos0;
    vec[1] = obj->anim.localPosY;
    vec[2] = obj->anim.localPosZ - dist * sin0;
    if (objBboxFn_800640cc(&obj->anim.localPosX, vec, 0.1f, 3, NULL, obj, 8, -1, 0xff, 0) != 0)
    {
        anglePlus = angle;
        angleMinus = angle;
        cosM = cos0;
        cosP = cos0;
        cosStepP = mathSinf(0.34898064f);
        cosStepM = mathSinf(-0.34898064f);
        sinP = sin0;
        sinM = sin0;
        sinStepP = mathCosf(0.34898064f);
        sinStepM = mathCosf(-0.34898064f);
        for (i = 0; i < 8; i++)
        {
            f32 cosNext;

            anglePlus += 0xe38;
            cosNext = cosP * sinStepP + sinP * cosStepP;
            sinP = sinP * sinStepP - cosP * cosStepP;
            cosP = cosNext;
            vec[0] = obj->anim.localPosX - dist * cosNext;
            vec[2] = obj->anim.localPosZ - dist * sinP;
            if (objBboxFn_800640cc(&obj->anim.localPosX, vec, 0.1f, 1, NULL, obj, 8, -1, 0xff, 0) == 0)
            {
                return anglePlus;
            }
            angleMinus -= 0xe38;
            cosNext = cosM * sinStepM + sinM * cosStepM;
            sinM = sinM * sinStepM - cosM * cosStepM;
            cosM = cosNext;
            vec[0] = obj->anim.localPosX - dist * cosNext;
            vec[2] = obj->anim.localPosZ - dist * sinM;
            if (objBboxFn_800640cc(&obj->anim.localPosX, vec, 0.1f, 1, NULL, obj, 8, -1, 0xff, 0) == 0)
            {
                return angleMinus;
            }
        }
    }
    return angle;
}

int EdibleMushroom_getExtraSize(void)
{
    return 0x144;
}

void EdibleMushroom_free(int obj)
{
    ObjGroup_RemoveObject(obj, EDIBLEMUSHROOM_OBJGROUP);
    ObjGroup_RemoveObject(obj, EDIBLEMUSHROOM_OBJGROUP_SECONDARY);
}

void EdibleMushroom_hitDetect(GameObject* obj)
{
    EdibleMushroomState* state;
    u8* mapObj;
    int hitCount;
    TrackGroundHit** hits;
    int i;
    TrackBBoxHit bboxHit;

    state = obj->extra;
    mapObj = *(u8**)&obj->anim.placementData;

    if (((obj->objectFlags & EDIBLEMUSHROOM_OBJFLAG_PARENT_SLACK) == 0) &&
        (((state->flags & EDIBLEMUSHROOM_FLAG_MOVING) != 0) ||
         ((((ObjHitsPriorityState*)obj->anim.hitReactState)->flags & EDIBLEMUSHROOM_FLAG_MOVING) != 0)))
    {
        hitCount = hitDetectFn_80065e50(obj, obj->anim.localPosX, obj->anim.localPosY,
                                        obj->anim.localPosZ, &hits, 0, 0);
        for (i = 0; i < hitCount; i++)
        {
            if (hits[i]->height < 10.0f + obj->anim.localPosY)
            {
                obj->anim.localPosY = hits[i]->height;
                break;
            }
        }

        hitCount = objBboxFn_800640cc(&obj->anim.previousLocalPosX, &obj->anim.localPosX, 6.0f, 2, &bboxHit,
                                      obj, 8, -1, 0xff, 0x14);
        if ((mapObj[0x18] == 4) && (hitCount != 0) && (bboxHit.surfaceType == 13))
        {
            state->flags |= EDIBLEMUSHROOM_FLAG_GROUNDED;
        }
    }
}

void EdibleMushroom_update(GameObject* self)
{
    EdibleMushroomState* state;
    u8* other;
    GameObject* player;
    GameObject* enemy;
    int hitObj;
    int msg;
    int hitKind;
    f32 distState;
    f32 distEnemy;

    state = self->extra;
    other = *(u8**)&self->anim.placementData;
    player = Obj_GetPlayerObject();
    enemy = getTrickyObject();

    if (objIsFrozen((u8*)self) != 0)
        return;

    if (state->animState == 8)
    {
        while (ObjMsg_Pop(self, (u32*)&msg, 0, 0) != 0)
        {
            if (((u32)msg - 0x70000) != 0xB)
                continue;
            self->anim.flags = (s16)(self->anim.flags | OBJANIM_FLAG_HIDDEN);
            ObjHits_DisableObject(self);
            gameBitIncrement(state->collectedGameBitId);
            mainSetBits(GAMEBIT_ITEM_TrickyFood_GrabInProgress, 0);
            if (self->anim.seqId == EDIBLEMUSHROOM_OBJ_WHITE)
            {
                itemPickupDoParticleFx(self, 1.0f, 0xFF, 0x28);
            }
            else
            {
                itemPickupDoParticleFx(self, 1.0f, 6, 0x28);
            }
            Sfx_PlayFromObject((u32)self, SFXTRIG_cam90_c);
        }
        return;
    }

    if (state->seqResetPending != 0)
    {
        self->anim.localPosX = ((EnemyMushroomMapData*)other)->base.posX;
        self->anim.localPosY = ((EnemyMushroomMapData*)other)->base.posY;
        self->anim.localPosZ = ((EnemyMushroomMapData*)other)->base.posZ;
        self->anim.alpha = 0xFF;
        state->seqResetPending = 0;
    }

    state->previousTargetDistance = state->currentTargetDistance;
    distState = vec3f_distanceSquared(&player->anim.worldPosX, &self->anim.worldPosX);
    if (enemy == NULL)
    {
        state->currentTargetDistance = sqrtf(distState);
    }
    else
    {
        distEnemy = vec3f_distanceSquared(&enemy->anim.worldPosX, &self->anim.worldPosX);
        if (distState < distEnemy)
        {
            state->currentTargetDistance = sqrtf(distState);
        }
        else
        {
            state->currentTargetDistance = sqrtf(distEnemy);
        }
        if (state->currentTargetDistance < (f32)(u32)other[0x1F])
        {
            (*(void (**)(GameObject*, GameObject*, int, int))(*(int*)*(int*)&enemy->anim.dll + 0x28))(enemy, self, 0, 1);
        }
    }

    hitKind = ObjHits_GetPriorityHit(self, &hitObj, 0, 0);
    if (hitKind != 0)
    {
        if (hitKind == 0x10)
        {
            Obj_StartModelFadeIn(self, 0x12C);
        }
        else
        {
            Obj_SetModelColorFadeRecursive(self, 0xF, 0xC8, 0, 0, 1);
            if (((GameObject*)hitObj)->anim.seqId != EDIBLEMUSHROOM_ATTACKER_EARTHWALKER)
            {
                if ((state->flags & EDIBLEMUSHROOM_FLAG_STRUCK) == 0)
                {
                    Sfx_PlayFromObject((u32)self, SFXTRIG_mv_ladderslide16);
                }
                state->flags =
                    (u8)(state->flags | EDIBLEMUSHROOM_FLAG_STRUCK);
            }
        }
    }
    edibleMushroomFn_801d083c(self, state, other);
}

void EdibleMushroom_init(GameObject* obj, int aux)
{
    EdibleMushroomState* state;
    GameObject* player;
    int curveInitParam;
    ObjAnimEventList animEvents;
    f32 dist;

    state = obj->extra;
    curveInitParam = 0x19;
    player = Obj_GetPlayerObject();

    (obj)->animEventCallback = EdibleMushroom_SeqFn;
    (obj)->objectFlags = (u16)((obj)->objectFlags | EDIBLEMUSHROOM_OBJFLAG_HIDDEN);

    if (mainGetBit(((EdibleMushroomPlacement*)aux)->gameBitId) != 0)
    {
        state->animState = 8;
        ObjHits_DisableObject(obj);
        (obj)->anim.flags = (short)((obj)->anim.flags | OBJANIM_FLAG_HIDDEN);
    }

    (obj)->anim.modelState->flags |= 0x810;

    state->lungeRootSpeedScale = 0.5f;
    state->mapParamScale = 0.2f * ((f32)((EdibleMushroomPlacement*)aux)->paramByte / 255.0f);

    ObjAnim_SetCurrentMove((int)obj, 1, 0.0f, 0);
    ObjAnim_AdvanceCurrentMove((int)obj, 1.0f, 1.0f, &animEvents);
    state->lungeRange = animEvents.rootDeltaX;
    if (state->lungeRange < 0.0f)
    {
        state->lungeRange = -state->lungeRange;
    }
    state->lungeRange =
        state->lungeRange * state->lungeRootSpeedScale;
    state->lungeRange += 20.0f;

    ObjAnim_SetCurrentMove((int)obj, 4, 0.0f, 0);
    ObjAnim_AdvanceCurrentMove((int)obj, 1.0f, 1.0f, &animEvents);
    state->retreatRange = animEvents.rootDeltaZ;
    if (state->retreatRange < 0.0f)
    {
        state->retreatRange = -state->retreatRange;
    }
    state->retreatRange += 20.0f;

    ObjMsg_AllocQueue(obj, 1);

    {
        int v = ((EdibleMushroomPlacement*)aux)->objectTypeParam;
        switch (v)
        {
        case 4:
        case 5:
            state->flags |= EDIBLEMUSHROOM_FLAG_ON_CURVE;
            (*gRomCurveInterface)->initCurve((void*)state, (void*)obj, 1000.0f, &curveInitParam, -1);
            (obj)->anim.localPosX = state->curve.posX;
            (obj)->anim.localPosZ = state->curve.posZ;
            break;
        }
    }

    state->curveAdvanceStep = 5.0f;

    if (player != NULL)
    {
        dist = Vec_distance(&player->anim.worldPosX, &obj->anim.worldPosX);
        state->currentTargetDistance = dist;
        state->previousTargetDistance = dist;
    }
    else
    {
        f32 z = 200.0f;
        state->currentTargetDistance = z;
        state->previousTargetDistance = z;
    }

    ObjGroup_AddObject((int)obj, EDIBLEMUSHROOM_OBJGROUP_SECONDARY);
    ObjGroup_AddObject((int)obj, EDIBLEMUSHROOM_OBJGROUP);

    if ((obj)->anim.seqId == EDIBLEMUSHROOM_OBJ_WHITE)
    {
        state->collectedGameBitId = 0x66d;
    }
    else
    {
        state->collectedGameBitId = 0xc1;
    }
}
