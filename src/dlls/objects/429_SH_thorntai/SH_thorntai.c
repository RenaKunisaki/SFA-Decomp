/*
 * SH_thorntail (DLL 0x1AD) - ThornTail Hollow herd-dinosaur behaviour.
 */
#include "dolphin/os/OSReport.h"
#include "main/gamebits.h"
#include "main/dll/SH/shthorntail_ai.h"
#include "main/dll/SH/SHthorntail.h"
#include "main/vecmath.h"
#include "main/frustum.h"
#include "main/frame_timing.h"
#include "main/audio/sfx.h"
#include "dlls/object_descriptor.h"
#include "sys/objects.h"
#include "main/obj_group.h"
#include "main/dll/dll_00C9_enemy.h"
#include "dolphin/os.h"
#include "game/objects/object.h"
#include "main/obj_trigger.h"
#include "main/mapEventTypes.h"
#include "main/dll/partfx_interface.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_trig_api.h"
#include "main/model.h"
#include "main/objprint_character_api.h"
#include "main/object_render.h"
#include "main/obj_path.h"
#include "main/dll/SH/SHroot.h"
#include "main/dll/dll_002E_moveLib.h"
#include "main/newshadows_audio_api.h"
#include "main/dll/path_control_interface.h"

extern u32 gSHthorntailDataTables[][4];
extern char sSHthorntailAngleYawDebug[];
extern f32 SHTHORNTAIL_TIMER_DONE_THRESHOLD;
extern f32 SHTHORNTAIL_LINKED_EVENT_DISTANCE_SQ;
extern f32 SHTHORNTAIL_TAIL_SWING_WINDUP_TIME;
extern f32 SHTHORNTAIL_TAIL_SWING_RECOVER_TIME;
extern f32 SHTHORNTAIL_CLOSE_ATTACK_DISTANCE;
extern u32 lbl_803E5410;

s32 gSHthorntailActiveConfigToken = -1;
u8 gSHthorntailLevelControlMode1ImpactSfxTable[] = {1, 0x10};
u8 gSHthorntailRootControlMode2DefaultImpactSfxTable[] = {1, 0x14};
u8 gSHthorntailLevelControlMode0Locomotion6ImpactSfxTable[] = {3, 0x2D, 0x2E, 0x2F};
u8 gSHthorntailRootControlMode3LocomotionDefaultImpactSfxTable[] = {4, 0x33, 0x34, 0x35, 0x36};
u8 gSHthorntailRootControlMode3Locomotion1ImpactSfxTable[] = {1, 0x37};
u8 gSHthorntailRootControlMode3Locomotion2ImpactSfxTable[] = {1, 0x38};
u8 gSHthorntailRootControlMode3Locomotion3ImpactSfxTable[] = {1, 0x39};
u8 gSHthorntailRootControlMode3Locomotion4ImpactSfxTable[] = {1, 0x3A};
u8 gSHthorntailRootControlMode3Locomotion5IdleImpactSfxTable[] = {1, 0x3B};
u8 gSHthorntailRootControlMode3Locomotion5PlayerImpactSfxTable[] = {1, 0x3C};
u8 gSHthorntailRootControlMode3Locomotion5EventImpactSfxTable[] = {1, 0x3D};
u8 gSHthorntailRootControlMode3Locomotion6ImpactSfxTable[] = {1, 0x3E};
u8 gSHthorntailRootControlMode3Locomotion7ImpactSfxTable[] = {1, 0x3F};
u8 gSHthorntailRootControlMode3Locomotion8ImpactSfxTable[] = {1, 0x40};

#define SHTHORNTAIL_OBJ_TYPE                0x4d7
#define SHTHORNTAIL_OBJ_GROUP               3
#define SHTHORNTAIL_LINKED_CONFIG_ROW_BYTES 0x10
/* player object pos vector lives at +0x18 */
#define PLAYER_POS_OFFSET 0x18

int SHthorntail_HasNearbyPendingEventObject(SHthorntailObject* obj)
{
    SHthorntailObject** objects;
    u32* linkedConfigRow;
    int count;
    int index;
    s8 groupIndex;
    int linkedEventPending;
    s8 matchCount;

    linkedEventPending = 0;
    groupIndex = -1;
    matchCount = 0;
    linkedConfigRow = gSHthorntailDataTables[0];
    for (index = 0; index < 6; index++)
    {
        if (obj->config->configToken == linkedConfigRow[0])
        {
            groupIndex = index;
            break;
        }
        linkedConfigRow = (u32*)((u8*)linkedConfigRow + SHTHORNTAIL_LINKED_CONFIG_ROW_BYTES);
    }
    objects = (SHthorntailObject**)ObjGroup_GetObjects(SHTHORNTAIL_OBJ_GROUP, &count);
    for (index = 0; index < count; index++)
    {
        if ((objects[index]->objType == SHTHORNTAIL_OBJ_TYPE) &&
            ((objects[index]->config->configToken == gSHthorntailDataTables[groupIndex][1]) ||
             (objects[index]->config->configToken == gSHthorntailDataTables[groupIndex][2]) ||
             (objects[index]->config->configToken == gSHthorntailDataTables[groupIndex][3])))
        {
            enemy_setTrackedObj((GameObject*)objects[index], (GameObject*)obj);
            if ((vec3f_distanceSquared((f32*)&objects[index]->pos, (f32*)&obj->pos) < SHTHORNTAIL_LINKED_EVENT_DISTANCE_SQ) &&
                (mainGetBit(SHthorntail_GetLinkedGameBit(objects[index]->config)) == 0u))
            {
                linkedEventPending = 1;
            }
            matchCount++;
            if (matchCount == SHTHORNTAIL_LINKED_CONFIG_COUNT)
            {
                break;
            }
        }
    }
    return linkedEventPending;
}

void SHthorntail_updateTailSwing(u32 objectId, SHthorntailRuntime* runtime)
{
    u8 tailSwingState;
    int moveComplete;

    tailSwingState = runtime->tailSwingState;
    switch (tailSwingState)
    {
    case SHTHORNTAIL_TAIL_SWING_READY:
        runtime->tailSwingTimer = runtime->tailSwingTimer - timeDelta;
        if (runtime->tailSwingTimer <= SHTHORNTAIL_TIMER_DONE_THRESHOLD)
        {
            Sfx_PlayFromObject(objectId, SHTHORNTAIL_TAIL_SWING_WINDUP_VOLUME_ID);
            runtime->tailSwingState = SHTHORNTAIL_TAIL_SWING_WINDUP;
            runtime->tailSwingTimer = SHTHORNTAIL_TAIL_SWING_WINDUP_TIME;
        }
        break;
    case SHTHORNTAIL_TAIL_SWING_WINDUP:
        runtime->tailSwingTimer = runtime->tailSwingTimer - timeDelta;
        if (runtime->tailSwingTimer <= SHTHORNTAIL_TIMER_DONE_THRESHOLD)
        {
            Sfx_PlayFromObject(objectId, SHTHORNTAIL_TAIL_SWING_ACTIVE_VOLUME_ID);
            runtime->tailSwingState = SHTHORNTAIL_TAIL_SWING_ACTIVE;
        }
        break;
    case SHTHORNTAIL_TAIL_SWING_ACTIVE:
        moveComplete = runtime->behaviorFlags & SHTHORNTAIL_FLAG_MOVE_COMPLETE;
        if (moveComplete != 0)
        {
            runtime->tailSwingState = SHTHORNTAIL_TAIL_SWING_READY;
            runtime->tailSwingTimer = SHTHORNTAIL_TAIL_SWING_RECOVER_TIME;
        }
        break;
    default:
        break;
    }
}

u32 SHthorntail_chooseNextState(SHthorntailObject* object, SHthorntailRuntime* runtime, SHthorntailConfig* config)
{
    short angleDelta;
    int value;
    u32 nextState;
    s8 behaviorState;
    f32 dist;

    if (config->leashRadiusByte != '\0')
    {
        value = (int)Obj_GetPlayerObject();
        dist = getXZDistance((f32*)&object->pos, (f32*)(value + PLAYER_POS_OFFSET));
        if (dist < SHTHORNTAIL_CLOSE_ATTACK_DISTANCE)
        {
            behaviorState = runtime->behaviorState;
            if ((SHTHORNTAIL_STATE_MOVE_2 <= behaviorState) && (behaviorState <= SHTHORNTAIL_STATE_MOVE_5))
            {
                nextState = SHTHORNTAIL_STATE_TURN_HOME;
            }
            else
            {
                nextState = SHTHORNTAIL_STATE_CLOSE_ATTACK;
            }
            return nextState;
        }
        dist = getXZDistance((f32*)&object->pos, (f32*)&config->homePos);
        if (dist > (float)(s32)(config->leashRadiusByte * config->leashRadiusByte))
        {
            value = (s16)getAngle(object->modelPos.x - config->homePos.x, object->modelPos.z - config->homePos.z);
            angleDelta = value - (u16)object->facingAngle;
            if (0x8000 < angleDelta)
            {
                angleDelta = angleDelta - 0xFFFF;
            }
            if (angleDelta < -0x8000)
            {
                angleDelta = angleDelta + 0xFFFF;
            }
            value = angleDelta;
            value = (value >= 0) ? value : -value;
            if (0x20 < value)
            {
                OSReport(sSHthorntailAngleYawDebug,
                         (u16)getAngle(object->modelPos.x - config->homePos.x, object->modelPos.z - config->homePos.z),
                         object->facingAngle);
                behaviorState = runtime->behaviorState;
                if ((SHTHORNTAIL_STATE_MOVE_2 <= behaviorState) && (behaviorState <= SHTHORNTAIL_STATE_MOVE_5))
                {
                    return SHTHORNTAIL_STATE_TURN_HOME;
                }
                return SHTHORNTAIL_STATE_CLOSE_ATTACK;
            }
        }
    }
    else
    {
        return SHTHORNTAIL_STATE_CLOSE_ATTACK;
    }
    value = ViewFrustum_IsSphereVisible((float*)&object->modelPos, object->cullRadius * object->modelScale);
    if (value == 0)
    {
        return SHTHORNTAIL_STATE_CLOSE_ATTACK;
    }
    behaviorState = runtime->behaviorState;
    if ((SHTHORNTAIL_STATE_MOVE_2 <= behaviorState) && (behaviorState <= SHTHORNTAIL_STATE_MOVE_5))
    {
        nextState = randomGetRange(SHTHORNTAIL_STATE_MOVE_3, SHTHORNTAIL_STATE_MOVE_5);
        return nextState & 0xff;
    }
    return SHTHORNTAIL_STATE_MOVE_2;
}

u32 gSHthorntailDataTables[][4] = {
    {0x00044318, 0x0004467F, 0x00044677, 0x0004467B}, {0x000442FB, 0x00044641, 0x0004463F, 0x00044640},
    {0x00044309, 0x00044646, 0x00044648, 0x00044649}, {0x00044302, 0x0004432F, 0x0004431C, 0x0004432E},
    {0x000442F4, 0x0004463D, 0x0004463C, 0x0004463E}, {0x00044310, 0x00044636, 0x00044634, 0x00044637},
};

u8 gSHthorntailPathHeaders[] = {
    0xC1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC1, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xC1, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x41, 0x00, 0x00, 0x00, 0xC1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00,
};

u8 gSHthorntailPathData[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02,
    0xC2, 0x00, 0x08, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F,
    0x02, 0xC2, 0x00, 0x08, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02,
    0x3F, 0x02, 0xC2, 0x00, 0x08, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x3F, 0x02, 0xC2, 0x00, 0x08, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00,
    0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x08, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00,
    0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x08, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00,
    0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x08, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A,
    0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x08, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7,
    0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x08, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23,
    0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x08, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C,
    0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x08, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x08, 0xFF, 0xFF, 0x00, 0x00, 0x00,
    0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x08, 0xFF, 0xFF, 0x00, 0x00,
    0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x08, 0xFF, 0xFF, 0x00,
    0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x08, 0xFF, 0xFF,
    0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x08, 0xFF,
    0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x08,
    0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00,
    0x08, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2,
    0x00, 0x08, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02,
    0xC2, 0x00, 0x08, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F,
    0x02, 0xC2, 0x00, 0x08, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02,
    0x3F, 0x02, 0xC2, 0x00, 0x08, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x3F, 0x02, 0xC2, 0x00, 0x08, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00,
    0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x08, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00,
    0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x08, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00,
    0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x09, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A,
    0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x09, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7,
    0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x09, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23,
    0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x09, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C,
    0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x09, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x09, 0xFF, 0xFF, 0x00, 0x00, 0x00,
    0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x09, 0xFF, 0xFF, 0x00, 0x00,
    0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x09, 0xFF, 0xFF, 0x00,
    0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x09, 0xFF, 0xFF,
    0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x09, 0xFF,
    0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x09,
    0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00,
    0x09, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2,
    0x00, 0x09, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02,
    0xC2, 0x00, 0x09, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F,
    0x02, 0xC2, 0x00, 0x09, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02,
    0x3F, 0x02, 0xC2, 0x00, 0x09, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x3F, 0x02, 0xC2, 0x00, 0x09, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00,
    0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x09, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00,
    0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x09, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00,
    0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x09, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7, 0x0A,
    0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x09, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23, 0xD7,
    0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x09, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x23,
    0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x09, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3C,
    0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x09, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x3F, 0x02, 0xC2, 0x00, 0x09, 0xFF, 0xFF, 0x00, 0x00, 0x00,
    0x00, 0x3C, 0x23, 0xD7, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x0D, 0x00, 0x0B,
    0x00, 0x0C, 0x00, 0x0F, 0x00, 0x04, 0x00, 0x05, 0x00, 0x06, 0x00, 0x07, 0x00, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00,
    0x0A, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x3B, 0xC4, 0x9B, 0xA6, 0x3B, 0xC4, 0x9B, 0xA6, 0x3C, 0x44, 0x9B, 0xA6,
    0x3B, 0xC4, 0x9B, 0xA6, 0x3B, 0xC4, 0x9B, 0xA6, 0x3B, 0xC4, 0x9B, 0xA6, 0x3C, 0x23, 0xD7, 0x0A, 0x3B, 0x83, 0x12,
    0x6F, 0x3B, 0xC4, 0x9B, 0xA6, 0x3B, 0x44, 0x9B, 0xA6, 0x3B, 0xC4, 0x9B, 0xA6, 0x3B, 0xC4, 0x9B, 0xA6, 0x3B, 0x23,
    0xD7, 0x0A, 0x3B, 0xC4, 0x9B, 0xA6, 0x3B, 0xC4, 0x9B, 0xA6, 0x3B, 0xC4, 0x9B, 0xA6, 0x3C, 0x23, 0xD7, 0x0A, 0x04,
    0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xB0, 0x02, 0xB0, 0x02,
    0xB0, 0x02, 0xB1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xC2, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

u8 gSHthorntailLevelControlMode0DefaultImpactSfxTable[] = {
    0x0F, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x01, 0x0F,
    0x01, 0x10, 0x01, 0x11, 0x01, 0x12, 0x01, 0x13, 0x00, 0x00, 0x01, 0x16, 0x01, 0x18, 0x01, 0x1A, 0x01, 0x1C,
    0x01, 0x1E, 0x00, 0x00, 0x01, 0x17, 0x01, 0x19, 0x01, 0x1B, 0x01, 0x1D, 0x01, 0x1F, 0x00, 0x00, 0x01, 0x20,
    0x01, 0x22, 0x01, 0x24, 0x01, 0x26, 0x01, 0x28, 0x00, 0x00, 0x01, 0x21, 0x01, 0x23, 0x01, 0x25, 0x01, 0x27,
    0x01, 0x29, 0x00, 0x00, 0x01, 0x2B, 0x01, 0x2C, 0x01, 0x2C, 0x01, 0x2A, 0x01, 0x2B, 0x00, 0x00,
};

u8 gSHthorntailRootControlMode2Locomotion8ImpactSfxTable[] = {
    0x01, 0x31, 0x01, 0x30, 0x01, 0x30, 0x01, 0x32, 0x01, 0x31, 0x00, 0x00,
};

ObjectDescriptor gSH_thorntailObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    0,
    0,
    0,
    (ObjectDescriptorCallback)SHthorntail_init,
    (ObjectDescriptorCallback)SHthorntail_update,
    0,
    (ObjectDescriptorCallback)SHthorntail_render,
    (ObjectDescriptorCallback)SHthorntail_free,
    0,
    SHthorntail_getExtraSize,
};

char sSHthorntailAngleYawDebug[] = "angle %d, obj-yaw %d\n";
char sSHthorntailSourceFile[] = "SHthorntail.c";
char sThorntailEnteredInvalidState[] = "Thorntail entered an invalid state\n";

void SHthorntail_updateState(SHthorntailObject* obj, SHthorntailRuntime* runtime)
{
    int alertTriggered;
    int tailSwingQueued;
    int nextState;
    int randomValue;

    switch (runtime->behaviorState)
    {
    case SHTHORNTAIL_STATE_IDLE:
        alertTriggered = RandomTimer_UpdateRangeTrigger(
            &runtime->proximityAlertState, SHTHORNTAIL_PROXIMITY_ALERT_MIN_TIME, SHTHORNTAIL_PROXIMITY_ALERT_MAX_TIME);
        if (alertTriggered != 0)
        {
            Sfx_PlayFromObject((u32)obj, SHTHORNTAIL_ALERT_VOLUME_ID);
        }
        runtime->idleTimer = runtime->idleTimer - timeDelta;
        if (runtime->idleTimer <= SHTHORNTAIL_IDLE_COUNTDOWN_TIME)
        {
            runtime->behaviorState = SHTHORNTAIL_STATE_IDLE_COUNTDOWN;
        }
        break;
    case SHTHORNTAIL_STATE_IDLE_COUNTDOWN:
        runtime->idleTimer = runtime->idleTimer - timeDelta;
        if (runtime->idleTimer <= SHTHORNTAIL_TIMER_DONE_THRESHOLD)
        {
            tailSwingQueued = (*gSkyInterface)->getSunPosition(0);
            if (tailSwingQueued != 0)
            {
                runtime->behaviorState = SHTHORNTAIL_STATE_TAIL_SWING_READY;
            }
            else
            {
                nextState = SHthorntail_chooseNextState(obj, runtime, obj->config);
                runtime->behaviorState = nextState;
            }
        }
        break;
    case SHTHORNTAIL_STATE_MOVE_2:
    case SHTHORNTAIL_STATE_MOVE_3:
    case SHTHORNTAIL_STATE_MOVE_4:
    case SHTHORNTAIL_STATE_MOVE_5:
    case SHTHORNTAIL_STATE_TURN_HOME:
        if ((runtime->behaviorFlags & SHTHORNTAIL_FLAG_MOVE_COMPLETE) != 0)
        {
            tailSwingQueued = (*gSkyInterface)->getSunPosition(0);
            if (tailSwingQueued != 0)
            {
                runtime->behaviorState = SHTHORNTAIL_STATE_TAIL_SWING_READY;
            }
            else
            {
                nextState = SHthorntail_chooseNextState(obj, runtime, obj->config);
                runtime->behaviorState = nextState;
            }
        }
        break;
    case SHTHORNTAIL_STATE_CLOSE_ATTACK:
        if ((runtime->behaviorFlags & SHTHORNTAIL_FLAG_MOVE_COMPLETE) != 0)
        {
            runtime->behaviorState = SHTHORNTAIL_STATE_CLOSE_ATTACK_WAIT;
            randomValue = randomGetRange(SHTHORNTAIL_CLOSE_ATTACK_WAIT_MIN, SHTHORNTAIL_CLOSE_ATTACK_WAIT_MAX);
            runtime->comboTimer = (float)randomValue;
            randomValue = randomGetRange(SHTHORNTAIL_CLOSE_ATTACK_REPEAT_MIN, SHTHORNTAIL_CLOSE_ATTACK_REPEAT_MAX);
            runtime->comboRepeatCount = randomValue;
        }
        break;
    case SHTHORNTAIL_STATE_CLOSE_ATTACK_WAIT:
        runtime->comboTimer = runtime->comboTimer - (float)framesThisStep;
        if (runtime->comboTimer <= SHTHORNTAIL_TIMER_DONE_THRESHOLD)
        {
            if (runtime->comboRepeatCount <= 0)
            {
                runtime->behaviorState = SHTHORNTAIL_STATE_CLOSE_ATTACK_RECOVER;
            }
            else
            {
                runtime->behaviorState = SHTHORNTAIL_STATE_CLOSE_ATTACK_REPEAT;
            }
        }
        break;
    case SHTHORNTAIL_STATE_CLOSE_ATTACK_REPEAT:
        if ((runtime->behaviorFlags & SHTHORNTAIL_FLAG_MOVE_COMPLETE) != 0)
        {
            runtime->behaviorState = SHTHORNTAIL_STATE_CLOSE_ATTACK_WAIT;
            randomValue = randomGetRange(SHTHORNTAIL_CLOSE_ATTACK_WAIT_MIN, SHTHORNTAIL_CLOSE_ATTACK_WAIT_MAX);
            runtime->comboTimer = (float)randomValue;
            runtime->comboRepeatCount--;
        }
        break;
    case SHTHORNTAIL_STATE_CLOSE_ATTACK_RECOVER:
        if ((runtime->behaviorFlags & SHTHORNTAIL_FLAG_MOVE_COMPLETE) != 0)
        {
            runtime->behaviorState = SHTHORNTAIL_STATE_IDLE;
            randomValue = randomGetRange(SHTHORNTAIL_IDLE_WAIT_MIN, SHTHORNTAIL_IDLE_WAIT_MAX);
            runtime->idleTimer = (float)randomValue;
        }
        break;
    case SHTHORNTAIL_STATE_TAIL_SWING_READY:
        if ((runtime->behaviorFlags & SHTHORNTAIL_FLAG_MOVE_COMPLETE) != 0)
        {
            runtime->tailSwingState = SHTHORNTAIL_TAIL_SWING_ACTIVE;
            runtime->behaviorState = SHTHORNTAIL_STATE_TAIL_SWING;
        }
        break;
    case SHTHORNTAIL_STATE_TAIL_SWING:
        SHthorntail_updateTailSwing((u32)obj, runtime);
        if (((runtime->behaviorFlags & SHTHORNTAIL_FLAG_MOVE_COMPLETE) != 0) &&
            (tailSwingQueued = (*gSkyInterface)->getSunPosition(0), tailSwingQueued == 0))
        {
            runtime->behaviorState = SHTHORNTAIL_STATE_TAIL_SWING_RECOVER;
        }
        break;
    case SHTHORNTAIL_STATE_TAIL_SWING_RECOVER:
        if ((runtime->behaviorFlags & SHTHORNTAIL_FLAG_MOVE_COMPLETE) != 0)
        {
            runtime->behaviorState = SHTHORNTAIL_STATE_IDLE;
            randomValue = randomGetRange(SHTHORNTAIL_IDLE_WAIT_MIN, SHTHORNTAIL_IDLE_WAIT_MAX);
            runtime->idleTimer = (float)randomValue;
        }
        break;
    default:
        OSPanic(sSHthorntailSourceFile, SHTHORNTAIL_INVALID_STATE_PANIC_LINE, sThorntailEnteredInvalidState);
    }
    return;
}

void SHthorntail_updateRootControlMode3(SHthorntailObject* obj, SHthorntailRuntime* runtime)
{
    int randomIdleWait;
    u32 gameBitValue;

    runtime->impactSfxTable = gSHthorntailRootControlMode3LocomotionDefaultImpactSfxTable;
    switch (runtime->locomotionMode)
    {
    case SHTHORNTAIL_LOCOMOTION_1:
        runtime->impactSfxTable = gSHthorntailRootControlMode3Locomotion1ImpactSfxTable;
        break;
    case SHTHORNTAIL_LOCOMOTION_2:
        gameBitValue = mainGetBit(SHTHORNTAIL_ROOT_MODE3_LOCOMOTION2_GAMEBIT);
        if (gameBitValue != 6)
        {
            runtime->impactSfxTable = gSHthorntailRootControlMode3Locomotion2ImpactSfxTable;
        }
        break;
    case SHTHORNTAIL_LOCOMOTION_3:
        gameBitValue = mainGetBit(SHTHORNTAIL_ROOT_MODE3_LOCOMOTION3_GAMEBIT);
        if (gameBitValue == 0)
        {
            runtime->impactSfxTable = gSHthorntailRootControlMode3Locomotion3ImpactSfxTable;
        }
        break;
    case SHTHORNTAIL_LOCOMOTION_4:
        runtime->impactSfxTable = gSHthorntailRootControlMode3Locomotion4ImpactSfxTable;
        break;
    case SHTHORNTAIL_LOCOMOTION_5:
        gameBitValue = mainGetBit(SHTHORNTAIL_ROOT_MODE3_LOCOMOTION5_GATE_GAMEBIT);
        if (gameBitValue == 0)
        {
            gameBitValue = mainGetBit(SHTHORNTAIL_ROOT_MODE3_LOCOMOTION5_EVENT_GAMEBIT);
            if (gameBitValue != 0)
            {
                (*gMapEventInterface)
                    ->setMapAct(SHTHORNTAIL_ROOT_MODE3_TRIGGER_EVENT, SHTHORNTAIL_ROOT_MODE3_TRIGGER_ARG);
                runtime->impactSfxTable = gSHthorntailRootControlMode3Locomotion5EventImpactSfxTable;
            }
            else
            {
                gameBitValue = mainGetBit(SHTHORNTAIL_ROOT_MODE3_LOCOMOTION5_PLAYER_GAMEBIT);
                if (gameBitValue != 0)
                {
                    if (runtime->behaviorState == SHTHORNTAIL_STATE_ROOT_MODE3_WAIT)
                    {
                        runtime->behaviorState = SHTHORNTAIL_STATE_IDLE;
                        randomIdleWait = randomGetRange(SHTHORNTAIL_IDLE_WAIT_MIN, SHTHORNTAIL_IDLE_WAIT_MAX);
                        runtime->idleTimer = (float)randomIdleWait;
                    }
                    runtime->impactSfxTable = gSHthorntailRootControlMode3Locomotion5PlayerImpactSfxTable;
                }
                else
                {
                    runtime->impactSfxTable = gSHthorntailRootControlMode3Locomotion5IdleImpactSfxTable;
                    runtime->behaviorState = SHTHORNTAIL_STATE_ROOT_MODE3_WAIT;
                    return;
                }
            }
        }
        break;
    case SHTHORNTAIL_LOCOMOTION_6:
        gameBitValue = mainGetBit(SHTHORNTAIL_ROOT_MODE3_LOCOMOTION6_GAMEBIT);
        if (gameBitValue == 0)
        {
            runtime->impactSfxTable = gSHthorntailRootControlMode3Locomotion6ImpactSfxTable;
        }
        break;
    case SHTHORNTAIL_LOCOMOTION_7:
        gameBitValue = mainGetBit(SHTHORNTAIL_ROOT_MODE3_LOCOMOTION7_GAMEBIT);
        if (gameBitValue == 0)
        {
            runtime->impactSfxTable = gSHthorntailRootControlMode3Locomotion7ImpactSfxTable;
        }
        break;
    case SHTHORNTAIL_LOCOMOTION_8:
        runtime->impactSfxTable = gSHthorntailRootControlMode3Locomotion8ImpactSfxTable;
    }
    SHthorntail_updateState(obj, runtime);
}

void SHthorntail_updateRootControlMode2(SHthorntailObject* obj, SHthorntailRuntime* runtime)
{
    int linkedEventPending;
    int objectTriggerIsSet;
    u32 triggerIsSet;
    u32 triggerEventId;
    int randomTime;

    runtime->impactSfxTable = gSHthorntailLevelControlMode0DefaultImpactSfxTable;
    switch (runtime->locomotionMode)
    {
    case SHTHORNTAIL_LOCOMOTION_1:
        runtime->impactSfxTable = gSHthorntailRootControlMode2DefaultImpactSfxTable;
        break;
    case SHTHORNTAIL_LOCOMOTION_2:
        runtime->impactSfxTable = gSHthorntailRootControlMode2DefaultImpactSfxTable;
        break;
    case SHTHORNTAIL_LOCOMOTION_3:
        runtime->impactSfxTable = gSHthorntailRootControlMode2DefaultImpactSfxTable;
        break;
    case SHTHORNTAIL_LOCOMOTION_4:
        runtime->impactSfxTable = gSHthorntailRootControlMode2DefaultImpactSfxTable;
        break;
    case SHTHORNTAIL_LOCOMOTION_5:
        runtime->impactSfxTable = gSHthorntailRootControlMode2DefaultImpactSfxTable;
        break;
    case SHTHORNTAIL_LOCOMOTION_6:
        linkedEventPending = SHthorntail_HasNearbyPendingEventObject(obj);
        if (linkedEventPending != 0)
        {
            runtime->behaviorState = SHTHORNTAIL_STATE_EVENT_PAUSE;
            return;
        }
        if (runtime->behaviorState == SHTHORNTAIL_STATE_EVENT_PAUSE)
        {
            Sfx_PlayFromObject(0, SHTHORNTAIL_EVENT_RESUME_VOLUME_ID);
            runtime->behaviorState = SHTHORNTAIL_STATE_IDLE;
            randomTime = randomGetRange(SHTHORNTAIL_IDLE_WAIT_MIN, SHTHORNTAIL_IDLE_WAIT_MAX);
            runtime->idleTimer = (float)randomTime;
        }
        runtime->impactSfxTable = gSHthorntailRootControlMode2DefaultImpactSfxTable;
        break;
    case SHTHORNTAIL_LOCOMOTION_7:
        if (runtime->behaviorState == SHTHORNTAIL_STATE_ROOT_MODE2_EVENT)
        {
            triggerEventId = mainGetBit(SHTHORNTAIL_ROOT_MODE2_TRIGGER_SELECTOR_GAMEBIT);
            triggerIsSet = mainGetBit(triggerEventId);
            if (triggerIsSet != 0)
            {
                (*gMapEventInterface)->setObjGroupStatus((int)obj->animObjId, SHTHORNTAIL_ROOT_MODE2_TRIGGER_ANIM_EVENT, 0);
                runtime->behaviorState = SHTHORNTAIL_STATE_IDLE;
                randomTime = randomGetRange(SHTHORNTAIL_IDLE_WAIT_MIN, SHTHORNTAIL_IDLE_WAIT_MAX);
                runtime->idleTimer = (float)randomTime;
            }
            else
            {
                return;
            }
        }
        else
        {
            triggerIsSet = mainGetBit(SHTHORNTAIL_ROOT_MODE2_TRIGGER_SELECTOR_GAMEBIT);
            if ((triggerIsSet == 0) && (objectTriggerIsSet = ObjTrigger_IsSet((int)obj), objectTriggerIsSet != 0))
            {
                runtime->behaviorFlags = runtime->behaviorFlags | SHTHORNTAIL_FLAG_TRIGGER_EVENT_PENDING;
                runtime->behaviorState = SHTHORNTAIL_STATE_ROOT_MODE2_EVENT;
                (*gMapEventInterface)->setObjGroupStatus((int)obj->animObjId, SHTHORNTAIL_ROOT_MODE2_TRIGGER_ANIM_EVENT, 1);
                mainSetBits(SHTHORNTAIL_ROOT_MODE3_LOCOMOTION7_GAMEBIT, 1);
                return;
            }
        }
        break;
    case SHTHORNTAIL_LOCOMOTION_8:
        runtime->impactSfxTable = gSHthorntailRootControlMode2Locomotion8ImpactSfxTable + 6;
    }
    SHthorntail_updateState(obj, runtime);
}

typedef struct SHthorntailTailSwingEffectScratch
{
    u8 particleParams[12];
    Vec position;
} SHthorntailTailSwingEffectScratch;

#define THORNTAIL_OBJGROUP 0x4d

#define SHTHORNTAIL_OBJFLAG_RENDERED 0x800
#define SHTHORNTAIL_PARTFX_TAILSWING 0x7f0 /* tail-swing effect (SHthorntailTailSwingEffectScratch) */

#define SHTHORNTAIL_LEVEL_MODE1_GATE_OPEN_GAMEBIT            0x13E
#define SHTHORNTAIL_LEVEL_MODE1_FREEZE_GAMEBIT               0x168
#define SHTHORNTAIL_LEVEL_MODE1_PRIMARY_TRIGGER_GAMEBIT      0xCD5
#define SHTHORNTAIL_LEVEL_MODE1_SECONDARY_TRIGGER_GAMEBIT    0xCD6
#define SHTHORNTAIL_LEVEL_MODE1_CLOSE_ATTACK_DISABLE_GAMEBIT 0x1AB
#define SHTHORNTAIL_LEVEL_MODE0_LOCOMOTION2_GAMEBIT          0x09E
#define SHTHORNTAIL_LEVELCONTROL_AUDIO_CHANNEL               0x7F
#define SHTHORNTAIL_LEVELCONTROL_COLLISION_FLAG              0x40

#define SHTHORNTAIL_NORMAL_HIT_REACT_ENTRIES_OFFSET 0x0A0
#define SHTHORNTAIL_HEAVY_HIT_REACT_ENTRIES_OFFSET  0x294
#define SHTHORNTAIL_STATE_MOVE_IDS_OFFSET           0x488
#define SHTHORNTAIL_STATE_MOVE_STEP_SCALES_OFFSET   0x4AC
#define SHTHORNTAIL_STATE_FLAGS_OFFSET              0x4F0
#define SHTHORNTAIL_STATE_TRIGGER0_SFX_OFFSET       0x504
#define SHTHORNTAIL_STATE_TRIGGER7_SFX_OFFSET       0x528

#define SHTHORNTAIL_NORMAL_HIT_REACT_ENTRIES(tables)                                                                   \
    ((ObjHitReactEntry*)((tables) + SHTHORNTAIL_NORMAL_HIT_REACT_ENTRIES_OFFSET))
#define SHTHORNTAIL_HEAVY_HIT_REACT_ENTRIES(tables)                                                                    \
    ((ObjHitReactEntry*)((tables) + SHTHORNTAIL_HEAVY_HIT_REACT_ENTRIES_OFFSET))
#define SHTHORNTAIL_STATE_MOVE_IDS(tables)         ((s16*)((tables) + SHTHORNTAIL_STATE_MOVE_IDS_OFFSET))
#define SHTHORNTAIL_STATE_MOVE_STEP_SCALES(tables) ((f32*)((tables) + SHTHORNTAIL_STATE_MOVE_STEP_SCALES_OFFSET))
#define SHTHORNTAIL_STATE_FLAGS(tables)            ((u8*)((tables) + SHTHORNTAIL_STATE_FLAGS_OFFSET))
#define SHTHORNTAIL_STATE_TRIGGER0_SFX(tables)     ((u16*)((tables) + SHTHORNTAIL_STATE_TRIGGER0_SFX_OFFSET))
#define SHTHORNTAIL_STATE_TRIGGER7_SFX(tables)     ((u8*)((tables) + SHTHORNTAIL_STATE_TRIGGER7_SFX_OFFSET))

void SHthorntail_updateLevelControlMode1(u32 objectId, SHthorntailRuntime* runtime, SHthorntailConfig* config)
{
    int playerObj;
    int randomIdleWait;
    u8 closeToPlayer;
    u32 gameBit;
    int triggerIsSet;

    runtime->impactSfxTable = gSHthorntailLevelControlMode1ImpactSfxTable;
    playerObj = (int)Obj_GetPlayerObject();
    {
        int cmp = (double)getXZDistance((f32*)(objectId + 0x18), (f32*)(playerObj + 0x18)) < (double)SHTHORNTAIL_CLOSE_ATTACK_DISTANCE;
        closeToPlayer = cmp;
    }
    if (config->impactSfxVariant == 0)
    {
        gameBit = mainGetBit(SHTHORNTAIL_LEVEL_MODE1_GATE_OPEN_GAMEBIT);
        if (gameBit != 0)
        {
            gameBit = mainGetBit(SHTHORNTAIL_LEVEL_MODE1_FREEZE_GAMEBIT);
            if (gameBit != 0)
            {
                runtime->behaviorFlags = runtime->behaviorFlags | SHTHORNTAIL_FLAG_FREEZE_MOTION;
                runtime->freezeFrameCounter = 0;
                closeToPlayer = FALSE;
            }
            else
            {
                triggerIsSet = ObjTrigger_IsSet(objectId);
                if (triggerIsSet != 0)
                {
                    runtime->behaviorFlags = runtime->behaviorFlags | SHTHORNTAIL_FLAG_TRIGGER_EVENT_PENDING;
                    mainSetBits(SHTHORNTAIL_LEVEL_MODE1_SECONDARY_TRIGGER_GAMEBIT, 1);
                }
            }
        }
        else
        {
            triggerIsSet = ObjTrigger_IsSet(objectId);
            if (triggerIsSet != 0)
            {
                runtime->behaviorFlags = runtime->behaviorFlags | SHTHORNTAIL_FLAG_TRIGGER_EVENT_PENDING;
                mainSetBits(SHTHORNTAIL_LEVEL_MODE1_PRIMARY_TRIGGER_GAMEBIT, 1);
            }
        }
    }
    else
    {
        gameBit = mainGetBit(SHTHORNTAIL_LEVEL_MODE1_CLOSE_ATTACK_DISABLE_GAMEBIT);
        if (gameBit != 0)
        {
            closeToPlayer = FALSE;
        }
    }
    switch (runtime->behaviorState)
    {
    case SHTHORNTAIL_STATE_IDLE:
        if (!closeToPlayer)
        {
            runtime->idleTimer = SHTHORNTAIL_IDLE_COUNTDOWN_TIME;
            runtime->behaviorState = SHTHORNTAIL_STATE_IDLE_COUNTDOWN;
        }
        break;
    case SHTHORNTAIL_STATE_IDLE_COUNTDOWN:
        if (closeToPlayer)
        {
            runtime->behaviorState = SHTHORNTAIL_STATE_IDLE;
        }
        else
        {
            runtime->idleTimer = runtime->idleTimer - timeDelta;
            if (runtime->idleTimer <= SHTHORNTAIL_TIMER_DONE_THRESHOLD)
            {
                runtime->behaviorState = SHTHORNTAIL_STATE_TAIL_SWING_READY;
            }
        }
        break;
    case SHTHORNTAIL_STATE_TAIL_SWING_READY:
        if ((runtime->behaviorFlags & SHTHORNTAIL_FLAG_MOVE_COMPLETE) != 0)
        {
            if (closeToPlayer)
            {
                runtime->behaviorState = SHTHORNTAIL_STATE_TAIL_SWING_RECOVER;
            }
            else
            {
                runtime->tailSwingState = SHTHORNTAIL_TAIL_SWING_ACTIVE;
                runtime->behaviorState = SHTHORNTAIL_STATE_TAIL_SWING;
            }
        }
        break;
    case SHTHORNTAIL_STATE_TAIL_SWING:
        if (closeToPlayer)
        {
            runtime->behaviorState = SHTHORNTAIL_STATE_TAIL_SWING_RECOVER;
        }
        else
        {
            SHthorntail_updateTailSwing(objectId, runtime);
        }
        break;
    case SHTHORNTAIL_STATE_TAIL_SWING_RECOVER:
        if ((runtime->behaviorFlags & SHTHORNTAIL_FLAG_MOVE_COMPLETE) != 0)
        {
            runtime->behaviorState = SHTHORNTAIL_STATE_IDLE;
            randomIdleWait = randomGetRange(SHTHORNTAIL_IDLE_WAIT_MIN, SHTHORNTAIL_IDLE_WAIT_MAX);
            runtime->idleTimer = (float)randomIdleWait;
        }
        break;
    }
}

void SHthorntail_updateLevelControlMode0(SHthorntailObject* obj, SHthorntailRuntime* runtime, SHthorntailConfig* config)
{
    int linkedEventPending;
    u32 gameBit;
    int randomIdleWait;
    SHthorntailDataTables* dataTables;

    dataTables = (SHthorntailDataTables*)&gSHthorntailDataTables;
    runtime->impactSfxTable = dataTables->levelMode0DefaultImpactSfxTable;
    switch (runtime->locomotionMode)
    {
    case SHTHORNTAIL_LOCOMOTION_1:
        runtime->impactSfxTable =
            (u8*)dataTables->levelMode0Locomotion1ImpactSfxVariants + config->impactSfxVariant * 2;
        break;
    case SHTHORNTAIL_LOCOMOTION_2:
        gameBit = mainGetBit(SHTHORNTAIL_LEVEL_MODE0_LOCOMOTION2_GAMEBIT);
        if (gameBit != 0)
        {
            runtime->impactSfxTable =
                (u8*)dataTables->levelMode0Locomotion2SetImpactSfxVariants + config->impactSfxVariant * 2;
        }
        else
        {
            runtime->impactSfxTable =
                (u8*)dataTables->levelMode0Locomotion2ClearImpactSfxVariants + config->impactSfxVariant * 2;
        }
        break;
    case SHTHORNTAIL_LOCOMOTION_3:
        gameBit = mainGetBit(SHTHORNTAIL_ROOT_MODE3_LOCOMOTION3_GAMEBIT);
        if (gameBit != 0)
        {
            runtime->impactSfxTable =
                (u8*)dataTables->levelMode0Locomotion3SetImpactSfxVariants + config->impactSfxVariant * 2;
        }
        else
        {
            runtime->impactSfxTable =
                (u8*)dataTables->levelMode0Locomotion3ClearImpactSfxVariants + config->impactSfxVariant * 2;
        }
        break;
    case SHTHORNTAIL_LOCOMOTION_5:
        gameBit = mainGetBit(SHTHORNTAIL_ROOT_MODE3_LOCOMOTION5_PLAYER_GAMEBIT);
        if (gameBit == 0)
        {
            runtime->impactSfxTable =
                (u8*)dataTables->levelMode0Locomotion5ClearImpactSfxVariants + config->impactSfxVariant * 2;
        }
        break;
    case SHTHORNTAIL_LOCOMOTION_6:
        linkedEventPending = SHthorntail_HasNearbyPendingEventObject(obj);
        if (linkedEventPending != 0)
        {
            runtime->behaviorState = SHTHORNTAIL_STATE_EVENT_PAUSE;
            return;
        }
        if (runtime->behaviorState == SHTHORNTAIL_STATE_EVENT_PAUSE)
        {
            Sfx_PlayFromObject(0, SHTHORNTAIL_EVENT_RESUME_VOLUME_ID);
            runtime->behaviorState = SHTHORNTAIL_STATE_IDLE;
            randomIdleWait = randomGetRange(SHTHORNTAIL_IDLE_WAIT_MIN, SHTHORNTAIL_IDLE_WAIT_MAX);
            runtime->idleTimer = (float)randomIdleWait;
        }
        gameBit = mainGetBit(SHTHORNTAIL_ROOT_MODE3_LOCOMOTION6_GAMEBIT);
        if (gameBit == 0)
        {
            runtime->impactSfxTable = gSHthorntailLevelControlMode0Locomotion6ImpactSfxTable;
        }
        break;
    case SHTHORNTAIL_LOCOMOTION_8:
        runtime->impactSfxTable =
            (u8*)dataTables->levelMode0Locomotion8ImpactSfxVariants + config->impactSfxVariant * 2;
        break;
    }
    SHthorntail_updateState(obj, runtime);
}

u32 SHthorntail_updateLevelControlState(SHthorntailObject* obj, int unused, ObjAnimUpdateState* animUpdate)
{
    SHthorntailRuntime* runtime;
    int randomIdleWait;
    int impactHandled;
    int levelControlReady;
    int impactPending;

    runtime = obj->runtime;
    levelControlReady = (int)(runtime->behaviorFlags & SHTHORNTAIL_FLAG_LEVELCONTROL_READY);
    if (levelControlReady == 0)
    {
        Sfx_StopObjectChannel((u32)obj, SHTHORNTAIL_LEVELCONTROL_AUDIO_CHANNEL);
        runtime->behaviorState = SHTHORNTAIL_STATE_IDLE;
        randomIdleWait = randomGetRange(SHTHORNTAIL_IDLE_WAIT_MIN, SHTHORNTAIL_IDLE_WAIT_MAX);
        runtime->idleTimer = (float)randomIdleWait;
        runtime->behaviorFlags = runtime->behaviorFlags & ~SHTHORNTAIL_FLAG_TRIGGER_EVENT_PENDING;
        runtime->behaviorFlags =
            runtime->behaviorFlags | (SHTHORNTAIL_FLAG_LEVELCONTROL_READY | SHTHORNTAIL_FLAG_FREEZE_MOTION);
        runtime->freezeFrameCounter = 0;
        obj->statusFlags = obj->statusFlags | SHTHORNTAIL_OBJECT_STATUS_FREEZE_FRAME;
    }
    impactPending = (int)(runtime->behaviorFlags & SHTHORNTAIL_FLAG_IMPACT_PENDING);
    if (impactPending != 0)
    {
        impactHandled = dll_2E_func07((GameObject*)obj, (ObjSeqState*)animUpdate, (MoveLibState*)runtime, 0, 0);
        if (impactHandled != 0)
        {
            return 0;
        }
        animUpdate->hitVolumePair &= ~SHTHORNTAIL_LEVELCONTROL_COLLISION_FLAG;
        characterDoEyeAnims((GameObject*)obj, runtime->collisionShapeState);
    }
    runtime->activeMoveValid = 0;
    objAudioFn_8006ef38((GameObject*)obj, &animUpdate->animEvents, 8, runtime->renderPathPoints,
                        runtime->moveScratch, 1.0f, 1.0f);
    return 0;
}

int SHthorntail_getExtraSize(void)
{
    return SHTHORNTAIL_EXTRA_STATE_BYTES;
}

void SHthorntail_free(SHthorntailObject* obj)
{
    u32 activeConfigToken;

    activeConfigToken = gSHthorntailActiveConfigToken;
    if (activeConfigToken == obj->config->configToken)
    {
        gSHthorntailActiveConfigToken = SHTHORNTAIL_CONFIG_TOKEN_NONE;
    }
    ObjGroup_RemoveObject((int)obj, THORNTAIL_OBJGROUP);
}

void SHthorntail_render(SHthorntailObject* obj, int p2, int p3, int p4, int p5, s8 visible)
{
    SHthorntailRuntime* runtime;
    int pointIndex;

    runtime = obj->runtime;
    objRenderModelAndHitVolumes((GameObject*)obj, p2, p3, p4, p5, 1.0f);
    dll_2E_func06((GameObject*)obj, (MoveLibState*)runtime, 0);
    pointIndex = 0;
    do
    {
        ObjPath_GetPointWorldPosition((GameObject*)obj, pointIndex, &runtime->renderPathPoints[0].x,
                                      &runtime->renderPathPoints[0].y,
                                      &runtime->renderPathPoints[0].z, 0);
        runtime = (SHthorntailRuntime*)((int)runtime + sizeof(Vec));
        pointIndex = pointIndex + 1;
    } while (pointIndex < SHTHORNTAIL_RENDER_PATH_POINT_COUNT);
}

static void SHthorntail_applyGravity(SHthorntailObject* obj)
{
    obj->velocityY = -(0.17f * timeDelta - obj->velocityY);
}

void SHthorntail_update(int obj)
{
    u8* stateTables;
    SHthorntailRuntime* runtime;
    SHthorntailConfig* config;
    int i;
    s8* eventId;
    u8 hitResult;
    u8 mode;
    ObjHitReactEntry* hitReactEntries;
    int val;
    u32 uval;
    int ref;
    s32 activeConfigToken;
    f32 negSinFacing;
    f32 negCosFacing;
    f32 leashDistance;
    ObjAnimEventList animEvents;
    SHthorntailTailSwingEffectScratch effectScratch;

    stateTables = (u8*)&gSHthorntailDataTables;
    runtime = ((SHthorntailObject*)obj)->runtime;
    config = ((SHthorntailObject*)obj)->config;
    if (runtime->behaviorState == '\f')
    {
        if (runtime->effectTimer <= SHTHORNTAIL_TIMER_DONE_THRESHOLD)
        {
            if ((((SHthorntailObject*)obj)->objectFlags & SHTHORNTAIL_OBJFLAG_RENDERED) != 0)
            {
                ObjPath_GetPointWorldPosition((GameObject*)obj, 4, &effectScratch.position.x, &effectScratch.position.y,
                                              &effectScratch.position.z, 0);
                (*gPartfxInterface)
                    ->spawnObject((void*)obj, SHTHORNTAIL_PARTFX_TAILSWING, effectScratch.particleParams, 0x200001, -1, NULL);
            }
            runtime->effectTimer = 30.0f;
        }
        runtime->effectTimer = runtime->effectTimer - timeDelta;
    }
    runtime->behaviorFlags = runtime->behaviorFlags & ~SHTHORNTAIL_FLAG_LEVELCONTROL_READY;
    if ((SHTHORNTAIL_STATE_FLAGS(stateTables)[runtime->behaviorState] & SHTHORNTAIL_STATE_FLAG_HEAVY_HIT_REACT) != 0)
    {
        hitReactEntries = SHTHORNTAIL_HEAVY_HIT_REACT_ENTRIES(stateTables);
    }
    else
    {
        hitReactEntries = SHTHORNTAIL_NORMAL_HIT_REACT_ENTRIES(stateTables);
    }
    val = 0x19;
    hitResult = runtime->hitReactState =
        ObjHitReact_Update(obj, hitReactEntries, val, runtime->hitReactState, (float*)runtime->hitReactScratch);
    if (hitResult == 0)
    {
        mode = (*gMapEventInterface)->getMapAct((int)((SHthorntailObject*)obj)->animObjId);
        runtime->locomotionMode = mode;
        switch (config->controlMode)
        {
        case SHTHORNTAIL_CONTROL_MODE_LEVEL_0:
            SHthorntail_updateLevelControlMode0((SHthorntailObject*)obj, runtime, config);
            break;
        case SHTHORNTAIL_CONTROL_MODE_LEVEL_1:
            SHthorntail_updateLevelControlMode1(obj, runtime, config);
            break;
        case SHTHORNTAIL_CONTROL_MODE_ROOT_2:
            SHthorntail_updateRootControlMode2((SHthorntailObject*)obj, runtime);
            break;
        case SHTHORNTAIL_CONTROL_MODE_ROOT_3:
            SHthorntail_updateRootControlMode3((SHthorntailObject*)obj, runtime);
            break;
        }
        if ((SHTHORNTAIL_STATE_FLAGS(stateTables)[runtime->behaviorState] & SHTHORNTAIL_STATE_FLAG_STATUS_ACTIVE) != 0)
        {
            ((SHthorntailObject*)obj)->statusFlags |= SHTHORNTAIL_OBJECT_STATUS_ACTIVE;
        }
        else
        {
            ((SHthorntailObject*)obj)->statusFlags &= ~SHTHORNTAIL_OBJECT_STATUS_ACTIVE;
            ((SHthorntailObject*)obj)->statusFlags &= ~SHTHORNTAIL_OBJECT_STATUS_FREEZE_FRAME;
        }
        if ((runtime->behaviorFlags & SHTHORNTAIL_FLAG_FREEZE_MOTION) != 0)
        {
            if (++runtime->freezeFrameCounter > 0xa)
            {
                runtime->behaviorFlags = runtime->behaviorFlags & ~SHTHORNTAIL_FLAG_FREEZE_MOTION;
            }
            else
            {
                ((SHthorntailObject*)obj)->statusFlags |= SHTHORNTAIL_OBJECT_STATUS_FREEZE_FRAME;
            }
        }
        if ((int)((SHthorntailObject*)obj)->currentMove != SHTHORNTAIL_STATE_MOVE_IDS(stateTables)[runtime->behaviorState])
        {
            ObjAnim_SetCurrentMove(obj, SHTHORNTAIL_STATE_MOVE_IDS(stateTables)[runtime->behaviorState],
                                   SHTHORNTAIL_TIMER_DONE_THRESHOLD, 0);
            runtime->storedFacingAngle = ((SHthorntailObject*)obj)->facingAngle;
        }
        val = ObjAnim_AdvanceCurrentMove(
            obj, SHTHORNTAIL_STATE_MOVE_STEP_SCALES(stateTables)[runtime->behaviorState], timeDelta, &animEvents);
        if (val != 0)
        {
            runtime->behaviorFlags = runtime->behaviorFlags | SHTHORNTAIL_FLAG_MOVE_COMPLETE;
        }
        else
        {
            runtime->behaviorFlags = runtime->behaviorFlags & ~SHTHORNTAIL_FLAG_MOVE_COMPLETE;
        }
        if ((SHTHORNTAIL_STATE_FLAGS(stateTables)[runtime->behaviorState] & SHTHORNTAIL_STATE_FLAG_APPLY_ROOT_MOTION) !=
            0)
        {
            if ((runtime->behaviorFlags & SHTHORNTAIL_FLAG_MOVE_COMPLETE) != 0)
            {
                runtime->storedFacingAngle = ((SHthorntailObject*)obj)->facingAngle;
            }
            negSinFacing = -mathSinf((3.1415927f * (f32)(s32)runtime->storedFacingAngle) / 32768.0f);
            negCosFacing = -mathCosf((3.1415927f * (f32)(s32)runtime->storedFacingAngle) / 32768.0f);
            ((SHthorntailObject*)obj)->modelPos.x = negSinFacing * -animEvents.rootDeltaZ + ((SHthorntailObject*)obj)->modelPos.x;
            ((SHthorntailObject*)obj)->modelPos.z = negCosFacing * -animEvents.rootDeltaZ + ((SHthorntailObject*)obj)->modelPos.z;
            ((SHthorntailObject*)obj)->modelPos.x = negCosFacing * -animEvents.rootDeltaX + ((SHthorntailObject*)obj)->modelPos.x;
            ((SHthorntailObject*)obj)->modelPos.z = negSinFacing * animEvents.rootDeltaX + ((SHthorntailObject*)obj)->modelPos.z;
            ((SHthorntailObject*)obj)->facingAngle += animEvents.rootPitch;
        }
        for (i = 0, eventId = (s8*)&animEvents; i < animEvents.triggerCount; i = i + 1)
        {
            if (eventId[0x13] == '\0')
            {
                if (SHTHORNTAIL_STATE_TRIGGER0_SFX(stateTables)[runtime->behaviorState] != 0)
                {
                    Sfx_PlayFromObject(obj, SHTHORNTAIL_STATE_TRIGGER0_SFX(stateTables)[runtime->behaviorState]);
                }
            }
            else if ((eventId[0x13] == '\a') &&
                     (SHTHORNTAIL_STATE_TRIGGER7_SFX(stateTables)[runtime->behaviorState] != 0))
            {
                Sfx_PlayFromObject(obj, SHTHORNTAIL_STATE_TRIGGER7_SFX(stateTables)[runtime->behaviorState]);
            }
            eventId++;
        }
        objAudioFn_8006ef38((GameObject*)obj, &animEvents, 8, runtime->renderPathPoints, runtime->moveScratch, 1.0f,
                            1.0f);
        if ((SHTHORNTAIL_STATE_FLAGS(stateTables)[runtime->behaviorState] &
             SHTHORNTAIL_STATE_FLAG_DISABLE_MOVE_CONTROL) != 0)
        {
            runtime->movementControlFlags = runtime->movementControlFlags & ~1;
        }
        else
        {
            runtime->movementControlFlags = runtime->movementControlFlags | 1;
        }
        dll_2E_func03((GameObject*)obj, (MoveLibState*)runtime);
        if ((SHTHORNTAIL_STATE_FLAGS(stateTables)[runtime->behaviorState] & SHTHORNTAIL_STATE_FLAG_HEAVY_HIT_REACT) !=
            0)
        {
            characterCloseEyes((GameObject*)obj, runtime->collisionShapeState);
        }
        else
        {
            characterDoEyeAnims((GameObject*)obj, runtime->collisionShapeState);
        }
        runtime->behaviorFlags = runtime->behaviorFlags & ~2;
        if (((runtime->behaviorFlags & 4) == 0) && (val = ObjTrigger_IsSet(obj), val != 0))
        {
            uval = randomGetRange(1, (u32)*runtime->impactSfxTable);
            runtime->behaviorFlags = runtime->behaviorFlags | SHTHORNTAIL_FLAG_IMPACT_PENDING;
            (*gObjectTriggerInterface)->runSequence(*(u8*)(runtime->impactSfxTable + uval), (void*)obj, -1);
        }
        if (config->leashRadiusByte != '\0')
        {
            leashDistance = getXZDistance(&((SHthorntailObject*)obj)->pos.x, (float*)&config->homePos);
            if ((leashDistance > (f32)(s32)((u32)config->leashRadiusByte * (u32)config->leashRadiusByte)) &&
                (ref = ViewFrustum_IsSphereVisible((f32*)(obj + 0xC), ((SHthorntailObject*)obj)->cullRadius * ((SHthorntailObject*)obj)->modelScale), ref == 0))
            {
                ref = getAngle(((SHthorntailObject*)obj)->modelPos.x - config->homePos.x, ((SHthorntailObject*)obj)->modelPos.z - config->homePos.z);
                ((SHthorntailObject*)obj)->facingAngle = ref;
            }
        }
        runtime->activeMoveValid = 1;
        activeConfigToken = gSHthorntailActiveConfigToken;
        if (activeConfigToken == SHTHORNTAIL_CONFIG_TOKEN_NONE)
        {
            gSHthorntailActiveConfigToken = ((SHthorntailObject*)obj)->config->configToken;
            ((SHthorntailObject*)obj)->velocityY = -(0.17f * timeDelta - ((SHthorntailObject*)obj)->velocityY);
            (*gPathControlInterface)->update((void*)obj, runtime->moveScratch, timeDelta);
            (*gPathControlInterface)->apply((void*)obj, runtime->moveScratch);
            (*gPathControlInterface)->advance((void*)obj, runtime->moveScratch, timeDelta);
            ((SHthorntailObject*)obj)->pitch = runtime->moveControlPitch;
            ((SHthorntailObject*)obj)->roll = runtime->moveControlRoll;
        }
        else
        {
            if ((u32)activeConfigToken == (u32)((SHthorntailObject*)obj)->config->configToken)
            {
                gSHthorntailActiveConfigToken = SHTHORNTAIL_CONFIG_TOKEN_NONE;
            }
            if (('\x02' <= runtime->behaviorState) && (runtime->behaviorState <= '\x06'))
            {
                ((SHthorntailObject*)obj)->velocityY = -(0.17f * timeDelta - ((SHthorntailObject*)obj)->velocityY);
                (*gPathControlInterface)->update((void*)obj, runtime->moveScratch, timeDelta);
                (*gPathControlInterface)->apply((void*)obj, runtime->moveScratch);
                (*gPathControlInterface)->advance((void*)obj, runtime->moveScratch, timeDelta);
                ((SHthorntailObject*)obj)->pitch = runtime->moveControlPitch;
                ((SHthorntailObject*)obj)->roll = runtime->moveControlRoll;
            }
            else
            {
                (*gPathControlInterface)->attachObject((void*)obj, runtime->moveScratch);
            }
        }
    }
    return;
}

void SHthorntail_init(SHthorntailObject* obj, SHthorntailConfig* config)
{
    SHthorntailRuntime* runtime;
    ObjModel* model;
    u32 randomTime;
    u8* moveScratch;
    u32 outA[2];
    u32 outB;
    u32 stackPad;

    runtime = obj->runtime;
    outA[0] = lbl_803E5410;
    obj->facingAngle = (short)((int)config->initialFacingByte << 8);
    switch (config->controlMode)
    {
    case SHTHORNTAIL_CONTROL_MODE_LEVEL_0:
        runtime->behaviorState = SHTHORNTAIL_STATE_IDLE;
        randomTime = randomGetRange(SHTHORNTAIL_IDLE_WAIT_MIN, SHTHORNTAIL_IDLE_WAIT_MAX);
        runtime->idleTimer = (f32)(s32)randomTime;
        break;
    case SHTHORNTAIL_CONTROL_MODE_LEVEL_1:
        runtime->tailSwingState = SHTHORNTAIL_TAIL_SWING_ACTIVE;
        runtime->behaviorState = SHTHORNTAIL_STATE_TAIL_SWING;
        break;
    case SHTHORNTAIL_CONTROL_MODE_ROOT_2:
        runtime->behaviorState = SHTHORNTAIL_STATE_IDLE;
        randomTime = randomGetRange(SHTHORNTAIL_IDLE_WAIT_MIN, SHTHORNTAIL_IDLE_WAIT_MAX);
        runtime->idleTimer = (f32)(s32)randomTime;
        break;
    case SHTHORNTAIL_CONTROL_MODE_ROOT_3:
        runtime->behaviorState = SHTHORNTAIL_STATE_IDLE;
        randomTime = randomGetRange(SHTHORNTAIL_IDLE_WAIT_MIN, SHTHORNTAIL_IDLE_WAIT_MAX);
        runtime->idleTimer = (f32)(s32)randomTime;
        break;
    }
    obj->modelScale = *(float*)((int)obj->anim.modelInstance + 4) * ((float)config->initScale / 1000.0f);
    model = Obj_GetActiveModel((GameObject*)obj);
    modelInitBones(obj->modelScale, model);
    moveScratch = runtime->moveScratch;
    (*gPathControlInterface)->init(moveScratch, SHTHORNTAIL_PATH_CONTROL_MODE, SHTHORNTAIL_PATH_CONTROL_FLAGS, 0);
    (*gPathControlInterface)
        ->setup(moveScratch, SHTHORNTAIL_PATH_CHANNEL, gSHthorntailPathHeaders, gSHthorntailPathData, outA);
    (*gPathControlInterface)->attachObject(obj, moveScratch);
    obj->animEventCallback = SHthorntail_updateLevelControlState;
    dll_2E_func05((GameObject*)obj, (MoveLibState*)runtime, 0xffffdc72, 0x2aaa, 3);
    dll_2E_func08((MoveLibState*)runtime, 400, 0x78);
    ObjGroup_AddObject((int)obj, THORNTAIL_OBJGROUP);
}
