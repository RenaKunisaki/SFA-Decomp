/*
 * bossdrakor (DLL 0x24D) - the boss dragon "Drakor" encounter object.
 *
 * Drives the flying boss: it follows ROM curve paths to move, smooth-turns
 * toward its velocity or yaws to face the player, advances animation moves,
 * and runs a small move-state machine (BossDrakorState.moveState) that
 * sequences attack/recover animations. b40 in the ByteFlags byte (state
 * +0x198) marks the active "combat/flight" phase; other bits gate hit
 * handling (b04/b08), the first-frame setup (b10), and the air-meter HUD
 * (b20).
 *
 * On first update (b10) it spawns env fx, restores the sky/time-of-day,
 * (re)initialises the curve follower from its saved home position, and
 * creates a glow light (lightObj). Attacks spawn missile/breath objects via
 * Obj_AllocObjectSetup + loadObjectAtObject, aimed at the player with random
 * spread. Hits (priority hit 0xE/0xF) decrement airMeterHandle; when it
 * drops below zero the boss explodes, is removed from the update list, sets
 * map-act 0x1d=3 and game bit 0x83c, and grants the defeat bit stored in the
 * placement (defeatedGameBit). Defeat anim events warp to map 0x79 and restore the HUD.
 */
#include "main/dll/partfx_interface.h"
#include "main/obj_group.h"
#include "main/obj_link.h"
#include "main/obj_path.h"
#include "main/obj_query.h"
#include "main/obj_trigger.h"
#include "sys/objects.h"
#include "main/frame_timing.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/vecmath.h"
#include "main/audio/sfx.h"
#include "main/gamebits.h"
#include "main/game_ui_interface.h"
#include "main/dll/rom_curve_interface.h"
#include "main/mapEventTypes.h"
#include "main/objHitReact.h"
#include "main/objhits.h"
#include "main/objanim.h"
#include "main/objanim_update.h"
#include "dolphin/mtx/vec.h"
#include "main/audio/music_api.h"
#include "main/gametext_show_api.h"
#include "main/rcp_dolphin.h"
#include "main/rcp_dolphin_api.h"
#include "main/maketex_api.h"
#include "main/maketex_random_api.h"
#include "main/maketex_timer_api.h"
#include "main/dll/dll_0282_barrelgener.h"
#include "main/dll/dll_0262_drakormissile.h"
#include "main/dll/dll_0271_drakorhoverpad.h"
#include "main/dll/dll_0243_dbholecontrol1.h"
#include "main/render_envfx_api.h"
#include "sys/objects/lifecycle.h"
#include "main/object_update_list.h"
#include "game/objects/object_setup.h"
#include "main/objprint_anim_api.h"
#include "main/objprint_api.h"
#include "main/objprint_sound_api.h"
#include "main/object_render.h"
#include "game/objects/object.h"
#include "main/model_light.h"
#include "main/modellight_api.h"
#include "main/objfx.h"
#include "main/dll/objfx_api.h"
#include "main/sky_api.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/audio/music_trigger_ids.h"
#include "main/gamebit_ids.h"
#include "main/dll/dll_024D_bossdrakor.h"
#include "dlls/object_descriptor.h"

f32 lbl_803DC188 = 3.0f;
f32 lbl_803DC18C = 8.0f;
f32 lbl_803DC190 = 1.0f;
f32 lbl_803DC194 = 150.0f;
s16 lbl_803DC198 = 0xE38;
s16 lbl_803DC19A = 0x2D8;

#define BOSSDRAKOR_MAP_ARENA          0x1d /* map-event id set to act 3 on boss defeat */
#define BOSSDRAKOR_OBJGROUP           0x45
#define BOSSDRAKOR_PARTFX             0x7ad
#define BOSSDRAKOR_HIT_VOLUME_SLOT    5
#define BOSSDRAKOR_AIRMETER_BGTEXTURE 0x63e /* HUD air-meter background texture id */
#define DRAKORHOVERPAD_OBJGROUP 0x46 /* DLL 0x271 drakorhoverpad */
#define BOSSDRAKOR_CHILD_OBJ_MISSILE 0x70f /* drakormissile (drakormissile_startActiveLaunch) */

#define BOSSDRAKOR_SPELLSTONE_STATE_HELD 2
#define BOSSDRAKOR_SPELLSTONE_STATE_IDLE 0

typedef struct BossDrakorSpellStoneInterface
{
    void* pad00[8];
    int (*setState)(GameObject* spellStone, int state);
} BossDrakorSpellStoneInterface;

STATIC_ASSERT(offsetof(BossDrakorSpellStoneInterface, setState) == 0x20);
#define BOSSDRAKOR_CHILD_OBJ_ATTACK  0x709 /* spawnAttackObjects: BossdrakorPlacement (airMeterMax/curveAdvanceStep) */
#define BOSSDRAKOR_ENVFX_A 0x144
#define BOSSDRAKOR_ENVFX_B 0x10d
#define BOSSDRAKOR_ENVFX_C 0x10e


int bossdrakor_seqFn(GameObject* obj, int unused, ObjAnimUpdateState* animUpdate)
{
    int inner = *(int*)&(obj)->extra;
    int i;
    int target;
    int eventId;
    BossDrakorState* s = (BossDrakorState*)inner;
    ((BossDrakorState*)inner)->flags198.b10 = 1;
    if (s->textTimer > lbl_803E6510)
    {
        gameTextShow(0x569);
        s->textTimer -= timeDelta;
        if (s->textTimer < lbl_803E6510)
        {
            s->textTimer = lbl_803E6510;
        }
    }
    for (i = 0; i < animUpdate->eventCount; i++)
    {
        eventId = animUpdate->eventIds[i];
        switch (eventId)
        {
        case 6:
            target = ObjGroup_FindNearestObject(DBHOLE_CONTROL1_OBJECT_GROUP, obj, 0);
            if ((void*)target != NULL && (obj)->childCount != 0)
            {
                (*(BossDrakorSpellStoneInterface**)((GameObject*)target)->anim.dll)
                    ->setState((GameObject*)target, BOSSDRAKOR_SPELLSTONE_STATE_HELD);
                ObjLink_DetachChild(obj, (GameObject*)target);
            }
            break;
        case 7:
            target = ObjGroup_FindNearestObject(DBHOLE_CONTROL1_OBJECT_GROUP, obj, 0);
            if ((void*)target != NULL)
            {
                (*(BossDrakorSpellStoneInterface**)((GameObject*)target)->anim.dll)
                    ->setState((GameObject*)target, BOSSDRAKOR_SPELLSTONE_STATE_IDLE);
                ObjLink_AttachChild(obj, (GameObject*)target, 1);
                s->textTimer = lbl_803E6514;
            }
            break;
        case 9:
            ((BossDrakorState*)inner)->flags198.b02 = 1;
            break;
        case 8:
            mainSetBits(GAMEBIT_DR_ObjGroups, 0);
            (*gMapEventInterface)->setObjGroupStatus(2, 0xf, 1);
            (*gMapEventInterface)->setObjGroupStatus(2, 0x10, 1);
            mainSetBits(GAMEBIT_DRArwingRelated0E7B, 0);
            warpToMap(0x79, 0);
            Rcp_DisableHeatEffect();
            break;
        }
    }
    if (((BossDrakorState*)inner)->flags198.b02)
    {
        objDoParticleFx(obj, lbl_803E6518, 6, lbl_803E651C, NULL);
    }
    return 0;
}
void bossdrakor_updateHeadTracking(GameObject* obj, BossDrakorState* state)
{
    BossDrakorState* drakorState;
    s16* neck;
    s16* upperJaw;
    s16* lowerJaw;
    int neckStep;
    int jawStep;
    s16 jawDelta;
    /* Partfx spawn parameter block (breath/steam emitted from the neck bone). */
    struct
    {
        u8 pad[6];
        s16 mode;
        f32 val;
        Vec vec;
    } partfxParams;

    drakorState = state;
    neck = objModelGetVecFn_800395d8(obj, 0xe);
    if (neck != NULL)
    {
        neckStep = (s16)-neck[0];
        neckStep = (neckStep < -(framesThisStep << 8))
                   ? -(framesThisStep << 8)
                   : (s16)((neckStep > (framesThisStep << 8)) ? (framesThisStep << 8) : neckStep);
        neck[0] += (s16)neckStep;
        PSVECSubtract(&drakorState->homePos, &obj->anim.localPos, &partfxParams.vec);
        partfxParams.val = lbl_803E651C;
        if (timerIsActive(&drakorState->jawAnimAngle) != 0)
        {
            upperJaw = objModelGetVecFn_800395d8(obj, 0xf);
            if (upperJaw != NULL)
            {
                lowerJaw = objModelGetVecFn_800395d8(obj, 0x10);
                if (lowerJaw != NULL)
                {
                    jawDelta = (int)(drakorState->jawAnimAngle * lbl_803DC19A) - (u16)upperJaw[1];
                    if (jawDelta > 0x8000)
                    {
                        jawDelta = (s16)((int)jawDelta - 0xffff);
                    }
                    if (jawDelta < -0x8000)
                    {
                        jawDelta += 0xffff;
                    }
                    jawStep = (jawDelta < -lbl_803DC198 * framesThisStep)
                                ? -lbl_803DC198 * framesThisStep
                                : ((jawDelta > lbl_803DC198 * framesThisStep) ? lbl_803DC198 * framesThisStep : jawDelta);
                    jawDelta = (s16)jawStep;
                    upperJaw[1] += jawDelta;
                    lowerJaw[1] -= jawDelta;
                    if (timerCountDown(&drakorState->jawAnimAngle) != 0)
                    {
                        storeZeroToFloatParam(&drakorState->jawAnimAngle);
                    }
                    if (drakorState->jawAnimAngle > lbl_803E6520)
                    {
                        partfxParams.mode = 45000;
                        (*gPartfxInterface)->spawnObject((void*)obj, BOSSDRAKOR_PARTFX, &partfxParams, 1, -1, NULL);
                    }
                }
            }
        }
    }
}

int bossdrakor_chooseNextMove(GameObject* obj, f32* speedOut)
{
    BossDrakorState* drakorState;
    int idx;
    int v;
    s16 d;
    u16 a;
    Vec dir;

    drakorState = obj->extra;
    PSVECNormalize(&obj->anim.velocity, &dir);
    if (drakorState->moveState != 0)
    {
        *speedOut = lbl_803E6534;
        return drakorState->moveState;
    }
    idx = 0;
    if (dir.y > lbl_803E6538)
    {
        idx = 3;
    }
    else if (dir.y < lbl_803E653C)
    {
        idx = 4;
    }
    else
    {
        a = (u16)(s16)getAngle(dir.x, dir.z);
        d = obj->anim.rotX - a;
        if (d > 0x8000)
        {
            d = (s16)((int)d - 0xffff);
        }
        if (d < -0x8000)
        {
            d += 0xffff;
        }
        v = (d >= 0) ? d : -d;
        if (v > 0x2000)
        {
            v = (d >= 0) ? d : -d;
            if (v < 0x6000)
            {
                if (d > 0)
                {
                    idx = 1;
                }
                else
                {
                    idx = 2;
                }
            }
        }
    }
    v = gBossDrakorMoveStateTable[idx];
    *speedOut = gBossDrakorMoveSpeedTable[idx];
    return v;
}



void bossdrakor_spawnAttackObjects(GameObject* obj, BossDrakorState* state, int action)
{
    GameObject* player;
    int hi;
    int lo;
    GameObject* missile;
    f32 spd;
    f32 prod;
    f32* mstate;
    ObjPlacement* setup;
    Vec target;
    Vec vecA;
    Vec vecB;
    Vec vecC;
    BossDrakorState* s = state;

    if (action < 0 || action >= 4)
    {
        return;
    }
    {
        switch (action)
        {
        case 3:
            break;
        case 1:
            player = Obj_GetPlayerObject();
            if ((state)->flags198.b40)
            {
                if (Obj_IsLoadingLocked() != 0)
                {
                    setup = Obj_AllocObjectSetup(0x20, BOSSDRAKOR_CHILD_OBJ_MISSILE);
                    setup->posX = s->homePosX;
                    setup->posY = s->homePosY;
                    setup->posZ = s->homePosZ;
                    setup->color[0] = 1;
                    setup->color[1] = 1;
                    setup->color[2] = 0xff;
                    setup->color[3] = 0xff;
                    if (player != NULL)
                    {
                        missile = loadObjectAtObject(obj, setup);
                        if (missile != NULL)
                        {
                            prod = lbl_803DC188 * Vec_distance(&(obj)->anim.worldPosX,
                                                               &player->anim.worldPosX);
                            target.x = player->anim.localPosX +
                                        (f32)(s32)randomGetRange(lo = (int)-prod, hi = (int)prod);
                            target.y = player->anim.localPosY + (f32)(s32)randomGetRange(lo, hi);
                            target.z = player->anim.localPosZ + (f32)(s32)randomGetRange(lo, hi);
                            PSVECSubtract(&player->anim.localPos, &s->homePos,
                                          &vecA);
                            PSVECSubtract(&target, &s->homePos, &vecB);
                            PSVECNormalize(&vecA, &vecA);
                            spd = s->missileLeadFactor *
                                      PSVECDotProduct(&player->anim.velocity, &vecA) +
                                  s->missileBaseSpeed;
                            PSVECScale(&vecA, &missile->anim.velocity, spd);
                            mstate = (f32*)missile->extra;
                            PSVECScale(&vecA, &vecC, PSVECDotProduct(&vecA, &vecB));
                            PSVECSubtract(&vecB, &vecC, &vecC);
                            PSVECNormalize(&vecC, &vecC);
                            PSVECScale(&vecC, &missile->anim.velocity,
                                       s->missileBaseSpeed * lbl_803DC18C);
                            *mstate = spd;
                            drakormissile_startActiveLaunch((GameObject*)(missile));
                            storeZeroToFloatParam(&s->jawAnimAngle);
                            s16toFloat(&s->jawAnimAngle, 0x1e);
                            Sfx_PlayFromObject((int)obj, SFXTRIG__UNK);
                            Sfx_PlayFromObject((int)obj, SFXTRIG_cahit2_c);
                        }
                    }
                }
            }
            break;
        case 2:
            if (!(state)->flags198.b40)
            {
                if (Obj_IsLoadingLocked() != 0)
                {
                    setup = Obj_AllocObjectSetup(0x24, BOSSDRAKOR_CHILD_OBJ_ATTACK);
                    setup->color[0] = 2;
                    setup->color[1] = 1;
                    setup->color[2] = 0xff;
                    setup->color[3] = 0xff;
                    setup->posX = s->homePosX;
                    setup->posY = s->homePosY;
                    setup->posZ = s->homePosZ;
                    ((BossdrakorPlacement*)setup)->airMeterMax = 0x3c;
                    ((BossdrakorPlacement*)setup)->unk1C = lbl_803DC194;
                    ((BossdrakorPlacement*)setup)->curveAdvanceStep = lbl_803DC190;
                    loadObjectAtObject(obj, setup);
                    Sfx_PlayFromObject((int)obj, SFXTRIG__UNK);
                }
            }
            break;
        }
    }
}


int gBossDrakorMoveStateTable[5] = {1, 2, 3, 4, 5};
int gBossDrakorMoveSpeedTable[5] = {400, 400, 400, 600, 600};

BossDrakorTuning gBossDrakorTurnMoveStates = {
    {18, 18, 19, 20, 21},
    {0.005f, 0.005f, 0.005f, 0.005f, 0.005f, 0.005f, 0.005f, 0.005f, 0.005f},
    {1, 7, 6, 7, 7, 1, 1, 3, 11},
    {0.2f, 0.2f, 0.2f},
    {0.08f, 0.06f, 0.06f},
    {50, 100, 200},
};

ObjectDescriptor gBossDrakorObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)bossdrakor_initialise,
    (ObjectDescriptorCallback)bossdrakor_release,
    0,
    (ObjectDescriptorCallback)bossdrakor_init,
    (ObjectDescriptorCallback)bossdrakor_update,
    (ObjectDescriptorCallback)bossdrakor_hitDetect,
    (ObjectDescriptorCallback)bossdrakor_render,
    (ObjectDescriptorCallback)bossdrakor_free,
    0,
    (ObjectDescriptorExtraSizeCallback)bossdrakor_getExtraSize,
};

void bossdrakor_handleActionEvent(GameObject* obj, BossDrakorState* state, int action)
{
    int* tbl = gBossDrakorMoveStateTable;
    BossDrakorState* s = state;
    f32 t;
    int found;
    if (action >= 26 || action <= -1)
    {
        return;
    }
    switch (action)
    {
    case 1:
        if ((state)->flags198.b40)
        {
            s->moveState = 0x12;
            if (s->lightObj != NULL)
            {
                modelLightStruct_setEnabled(s->lightObj, 0, lbl_803E651C);
            }
        }
        else
        {
            (state)->flags198.b40 = 1;
            if (s->lightObj != NULL)
            {
                modelLightStruct_setEnabled(s->lightObj, 1, lbl_803E651C);
            }
        }
        break;
    case 2:
        storeZeroToFloatParam(&s->attackTimer);
        s16toFloat(&s->attackTimer, 0x1e);
        s->attackType = 2;
        s->attackTimerDuration = lbl_803E6510;
        break;
    case 3:
        storeZeroToFloatParam(&s->attackTimer);
        s16toFloat(&s->attackTimer, 0x5a);
        s->attackTimerDuration = lbl_803E6540;
        s->attackType = 1;
        s->missileBaseSpeed = *(f32*)((char*)tbl + 0x84);
        s->missileLeadFactor = *(f32*)((char*)tbl + 0x90);
        break;
    case 4:
        storeZeroToFloatParam(&s->attackTimer);
        s16toFloat(&s->attackTimer, 0x3c);
        s->attackTimerDuration = lbl_803E6544;
        s->attackType = 1;
        s->missileBaseSpeed = *(f32*)((char*)tbl + 0x88);
        s->missileLeadFactor = *(f32*)((char*)tbl + 0x94);
        break;
    case 5:
        storeZeroToFloatParam(&s->attackTimer);
        s16toFloat(&s->attackTimer, 0x1e);
        s->attackTimerDuration = lbl_803E6548;
        s->attackType = 1;
        s->missileBaseSpeed = *(f32*)((char*)tbl + 0x8c);
        s->missileLeadFactor = *(f32*)((char*)tbl + 0x98);
        break;
    case 6:
        t = lbl_803E6510;
        s->attackTimerDuration = t;
        s->attackTimer = t;
        storeZeroToFloatParam(&s->attackTimer);
        break;
    case 7:
        s->moveState = 0x13;
        s->moveSpeed = lbl_803E654C;
        (state)->flags198.b08 = 0;
        break;
    case 25:
        s->moveState = 0x14;
        s->moveSpeed = lbl_803E654C;
        break;
    case 8:
        s->moveState = 0x11;
        break;
    case 9:
        s->moveState = 0;
        break;
    case 10:
    case 11:
    case 12:
        if (s->airMeterHandle < (tbl + action)[0x1d])
        {
            s->curveFollowState = 1;
        }
        break;
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
        s->repeatCount++;
        if (s->repeatCount > action - 0xd)
        {
            s->repeatCount = 0;
            s->curveFollowState = 1;
        }
        break;
    case 20:
    case 21:
    case 22:
    case 23:
        if (mainGetBit((s16)(action + 0xbe5)) != 0)
        {
            s->curveFollowState = 1;
        }
    case 24:
        found = ObjGroup_FindNearestObject(DRAKORHOVERPAD_OBJGROUP, obj, 0);
        if ((void*)found != NULL)
        {
            drakorhoverpad_resetPendingMotion((GameObject*)(found));
        }
        break;
    }
}

int bossdrakor_getExtraSize(void)
{
    return 0x1a4;
}

void bossdrakor_free(GameObject* obj)
{
    BossDrakorState* inner = (BossDrakorState*)(obj)->extra;
    BossDrakorState* s = inner;
    ObjGroup_RemoveObject((int)obj, BOSSDRAKOR_OBJGROUP);
    if ((obj)->childObjs[0] != NULL)
    {
        ObjLink_DetachChild(obj, obj->childObjs[0]);
    }
    if (s->lightObj != NULL)
    {
        ModelLightStruct_free(s->lightObj);
    }
    Music_Trigger(MUSICTRIG_LVF_Tracking, 0);
    Music_Trigger(MUSICTRIG_citytombs, 0);
}

void bossdrakor_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 vis)
{
    BossDrakorState* inner = (BossDrakorState*)obj->extra;
    f32 pos2;
    f32 pos1;
    f32 pos0;
    ModelLightStruct* light;
    int val;
    BossDrakorState* s = inner;
    objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, lbl_803E651C);
    ObjPath_GetPointWorldPosition(obj, 0, &s->homePosX, &s->homePosY,
                                  &s->homePosZ, 0);
    if (s->lightObj != NULL)
    {
        ObjPath_GetPointWorldPosition(obj, 5, &pos0, &pos1, &pos2, 0);
        modelLightStruct_setPosition(s->lightObj, pos0, pos1, pos2);
        light = s->lightObj;
        if (light->glowType != 0 && light->enabled != 0)
        {
            val = light->glowAlpha + light->glowAlphaStep;
            if (val < 0)
            {
                val = 0;
                light->glowAlphaStep = 0;
            }
            else if (val > 0xc)
            {
                val += randomGetRange(-0xc, 0xc);
                if (val > 0xff)
                {
                    val = 0xff;
                    s->lightObj->glowAlphaStep = 0;
                }
            }
            s->lightObj->glowAlpha = val;
        }
        light = s->lightObj;
        if (light->glowType != 0 && light->enabled != 0)
        {
            queueGlowRender(light);
        }
    }
}


void bossdrakor_hitDetect(GameObject* obj)
{
    int inner = *(int*)&(obj)->extra;
    BossdrakorPlacement* setup = (BossdrakorPlacement*)(obj)->anim.placementData;
    f32 hz;
    f32 hy;
    f32 hx;
    f32 shakeInit;
    BossDrakorState* s = (BossDrakorState*)inner;
    int hit = ObjHits_GetPriorityHitWithPosition(obj, 0, 0, 0, &hx, &hy, &hz);
    if (hit == 0xf || hit == 0xe)
    {
        if (((BossDrakorState*)inner)->flags198.b40)
        {
            s->airMeterHandle -= 1;
            ((BossDrakorState*)inner)->flags198.b08 = 1;
            if (s->airMeterHandle < 0)
            {
                mainSetBits(setup->defeatedGameBit, 1);
                spawnExplosion((GameObject*)(int*)obj, lbl_803E6550, 1, 1, 1, 1, 1, 1, 1);
                Obj_RemoveFromUpdateList(obj);
                (*gMapEventInterface)->setMapAct(BOSSDRAKOR_MAP_ARENA, 3);
                mainSetBits(GAMEBIT_ITEM_WaterSpellStone2_Got, 1);
            }
            else
            {
                Obj_SpawnHitLightAndFade(obj, (const Vec3f*)&hx, lbl_803E6554);
            }
            if (s->hitSfxCooldown <= lbl_803E6510)
            {
                s->hitSfxCooldown = lbl_803E6558;
                Sfx_PlayFromObject((int)obj, SFXTRIG__UNK_var);
            }
            if (s->hurtSfxCooldown <= lbl_803E6510)
            {
                s->hurtSfxCooldown = lbl_803E6520;
                Sfx_PlayFromObject((int)obj, SFXTRIG_mpwru1);
            }
            shakeInit = lbl_803E6518;
            s->shakeVel = shakeInit;
            s->shakeAmount = shakeInit;
            s->shakeScaleZ = (f32)(s32)randomGetRange(-0x32, 0x32) / lbl_803E655C;
        }
        else
        {
            if (s->hurtSfxCooldown < lbl_803E6510)
            {
                s->hurtSfxCooldown = lbl_803E6520;
                Sfx_PlayFromObject((int)obj, SFXTRIG_sc_npu_216);
            }
        }
    }
    s->hitSfxCooldown -= timeDelta;
    s->hurtSfxCooldown -= timeDelta;
}
void bossdrakor_update(GameObject* obj)
{
    int state;
    s8* p;
    int i;
    int state2;
    int moveResult;
    int adv;
    GameObject* player;
    int moveId;
    s16* uvec;
    s16 shakeX;
    s16 shakeY;
    int* tbl;
    int* tblRes;
    f32 shake;
    f32 shakeScaleZ;
    f32 t;
    s16 d;
    int step;
    s16* vec;
    s8 buf[0x1c];
    f32 hz;
    f32 hy;
    f32 hx;
    int curveArg;
    BossDrakorState* drakorState;
    BossDrakorState* s2;

    state = *(int*)&obj->extra;
    drakorState = (BossDrakorState*)state;
    curveArg = 0x29;
    if (((BossDrakorState*)state)->flags198.b10)
    {
        getEnvfxActImmediately(obj, obj, BOSSDRAKOR_ENVFX_A, 0);
        getEnvfxActImmediately(obj, obj, BOSSDRAKOR_ENVFX_B, 0);
        getEnvfxActImmediately(obj, obj, BOSSDRAKOR_ENVFX_C, 0);
        skySetLightIndex(1, lbl_803E6510);
        Rcp_EnableHeatEffect();
        if ((*gRomCurveInterface)->initCurve(&drakorState->curveWalker, (void*)obj, lbl_803E6560, &curveArg, 0xd) !=
            0)
        {
            (*gRomCurveInterface)->initCurve(&drakorState->curveWalker, (void*)obj, lbl_803E6560, &curveArg, 0);
        }
        obj->anim.localPosX = drakorState->curveWalker.posX;
        obj->anim.localPosZ = drakorState->curveWalker.posZ;
        obj->anim.localPosY = drakorState->curveWalker.posY;
        ((BossDrakorState*)state)->flags198.b20 = 1;
        drakorState->repeatCount = 0;
        state2 = *(int*)&obj->extra;
        s2 = (BossDrakorState*)state2;
        ((BossDrakorState*)state2)->flags198.b20 = 1;
        (*gGameUIInterface)->initAirMeter(s2->airMeterHandle, BOSSDRAKOR_AIRMETER_BGTEXTURE);
        (*gGameUIInterface)->runAirMeter(s2->airMeterHandle);
        ((BossDrakorState*)state)->flags198.b10 = 0;
        drakorState->lightObj = objCreateLight(NULL, 1);
        if (drakorState->lightObj != NULL)
        {
            modelLightStruct_setLightKind(drakorState->lightObj, MODEL_LIGHT_KIND_POINT);
            modelLightStruct_setDiffuseColor(drakorState->lightObj, 0x40, 0, 0xff, 0xff);
            modelLightStruct_setSpecularColor(drakorState->lightObj, 0x40, 0, 0xff, 0xff);
            modelLightStruct_setupGlow(drakorState->lightObj, 0, 0x40, 0, 0x80, 0x5a, lbl_803E6564);
            modelLightStruct_setDistanceAttenuation(drakorState->lightObj, lbl_803E6544, lbl_803E6540);
            lightSetField4D((ModelLightStruct*)drakorState->lightObj, 0);
            modelLightStruct_setEnabled(drakorState->lightObj, 1, lbl_803E6520);
            modelLightStruct_setDiffuseTargetColor(drakorState->lightObj, 0x40, 0, 0x80, 0x40);
            modelLightStruct_setSpecularTargetColor((ModelLightStruct*)drakorState->lightObj, 0x40, 0,
                                                     0x80, 0x40);
            modelLightStruct_startColorFade(drakorState->lightObj, 2, 0x28);
            modelLightStruct_setAffectsAabbLightSelection((ModelLightStruct*)drakorState->lightObj, 1);
            modelLightStruct_setGlowProjectionRadius((ModelLightStruct*)drakorState->lightObj,
                                                      lbl_803E6550);
        }
    }
    moveResult = Obj_UpdateRomCurveFollowVelocityIndexed(
        obj, &drakorState->curveWalker, drakorState->curveAdvanceStep,
        lbl_803E6568, lbl_803E6520, 1, &drakorState->curveFollowState);
    if (((BossDrakorState*)state)->flags198.b40)
    {
        player = Obj_GetPlayerObject();
        if (player != NULL)
        {
            step = Obj_GetYawDeltaToObject(obj, player, 0);
            obj->anim.rotX +=
                (s16)(((s16)step < -0x200) ? -0x200 : (((s16)step > 0x200) ? 0x200 : (s16)step));
            step = obj->anim.rotY;
            if (step != 0)
            {
                if (step < -0x100)
                {
                    step = -0x100;
                }
                else if (step > 0x100)
                {
                    step = 0x100;
                }
                obj->anim.rotY -= (s16)step;
            }
            step = obj->anim.rotZ;
            if (step != 0)
            {
                if (step < -0x100)
                {
                    step = -0x100;
                }
                else if (step > 0x100)
                {
                    step = 0x100;
                }
                obj->anim.rotZ -= (s16)step;
            }
        }
    }
    else
    {
        Obj_SmoothTurnAnglesTowardVelocity(obj, (const Vec3f*)&obj->anim.velocityX, 0x2d,
                                           lbl_803E6548, lbl_803E656C);
    }
    if (moveResult != 0)
    {
        bossdrakor_handleActionEvent(obj, drakorState, moveResult);
    }
    adv = ObjAnim_AdvanceCurrentMove(
        (int)obj,
        (t = PSVECMag(&obj->anim.velocity) / drakorState->moveSpeed, t + lbl_803E6570),
        timeDelta, (ObjAnimEventList*)buf);
    if (adv != 0)
    {
        if (drakorState->moveState == 0)
        {
            ObjHits_ClearHitVolumes((ObjAnimComponent*)obj);
            ((BossDrakorState*)state)->flags198.b04 = 0;
            ((BossDrakorState*)state)->flags198.b08 = 0;
            if (!((BossDrakorState*)state)->flags198.b40)
            {
                drakorState->moveSpeed = lbl_803E6534;
                ObjAnim_SetCurrentEventStepFrames((ObjAnimComponent*)obj, 0x28);
                moveId = 0x10;
            }
            else
            {
                moveId = bossdrakor_chooseNextMove(obj, &drakorState->moveSpeed);
            }
            ObjAnim_SetCurrentMove((int)obj, moveId, lbl_803E6510, 0);
        }
        else
        {
            ObjAnim_SetCurrentMove((int)obj, drakorState->moveState, lbl_803E6510, 0);
        }
        if (arrayIndexOf(gBossDrakorTurnMoveStates.turnMoveStates, 5, drakorState->moveState) != -1)
        {
            switch (drakorState->moveState)
            {
            case 0x12:
                ((BossDrakorState*)state)->flags198.b40 = 0;
                drakorState->moveState = 0;
                break;
            case 0x13:
                drakorState->moveState = 0x16;
                drakorState->moveSpeed = lbl_803E6534;
                break;
            case 0x16:
                drakorState->moveState = 0x16;
                drakorState->moveSpeed = lbl_803E6574;
                break;
            case 0x14:
                if (((BossDrakorState*)state)->flags198.b08)
                {
                    drakorState->moveState = 0;
                }
                else
                {
                    ObjHits_SetHitVolumeSlot((ObjAnimComponent*)obj, BOSSDRAKOR_HIT_VOLUME_SLOT, 1, 0);
                    drakorState->moveState = 0x15;
                    drakorState->moveSpeed = lbl_803E6574;
                }
                break;
            case 0x15:
                drakorState->moveState = 0;
                drakorState->moveSpeed = lbl_803E6514;
                ((BossDrakorState*)state)->flags198.b04 = 1;
                break;
            }
        }
    }
    for (i = 0, p = buf; i < buf[0x1b]; i++)
    {
        switch (p[0x13])
        {
        case 0:
            Sfx_PlayFromObject((u32)obj, SFXTRIG_mv_sliftloop11);
            break;
        case 7:
            Sfx_PlayFromObject((u32)obj, SFXTRIG_mv_sliftloop11);
            break;
        }
        p++;
    }
    if (timerCountDown(&drakorState->attackTimer) != 0)
    {
        bossdrakor_spawnAttackObjects(obj, drakorState, drakorState->attackType);
        if (drakorState->attackTimerDuration != lbl_803E6510)
        {
            s16toFloat(&drakorState->attackTimer,
                       drakorState->attackTimerDuration);
        }
    }
    if ((obj->objectFlags & OBJECT_OBJFLAG_RENDERED) == 0)
    {
        drakorState->homePosX = obj->anim.localPosX;
        drakorState->homePosY = obj->anim.localPosY - lbl_803E655C;
        drakorState->homePosZ = obj->anim.localPosZ;
    }
    objMove(obj, obj->anim.velocityX, obj->anim.velocityY,
            obj->anim.velocityZ);
    if (((BossDrakorState*)state)->flags198.b20)
    {
        (*gGameUIInterface)->runAirMeter(drakorState->airMeterHandle);
    }
    t = lbl_803E6510;
    if (t != drakorState->shakeAmount)
    {
        drakorState->shakeVel = -(lbl_803E6578 * timeDelta - drakorState->shakeVel);
        drakorState->shakeAmount =
            drakorState->shakeAmount + drakorState->shakeVel;
        t = (drakorState->shakeAmount < t)
                ? t
                : ((drakorState->shakeAmount > lbl_803E6550) ? lbl_803E6550
                                                                           : drakorState->shakeAmount);
        drakorState->shakeAmount = t;
        shakeScaleZ = drakorState->shakeScaleZ;
        shake = drakorState->shakeAmount;
        tblRes = objGetLookAtJointKeys();
        shakeX = (s16)(gBossDrakorDegToAngle * shake);
        shakeY = (s16)(gBossDrakorDegToAngle * (shake * shakeScaleZ));
        i = 0;
        tbl = tblRes;
        do
        {
            uvec = (s16*)objModelGetVecFn_800395d8(obj, tbl[0]);
            if (uvec != NULL)
            {
                uvec[1] = shakeY;
                uvec[0] = shakeX;
                uvec[2] = 0;
            }
            tbl++;
            i++;
        } while (i < 5);
    }
    if (randomChanceOneIn(200) != 0 && ((BossDrakorState*)state)->flags198.b40)
    {
    objSoundStart((u32)obj, &drakorState->soundState, 0x2ff);
    }
    objSoundUpdateMouth(obj, &drakorState->soundState);
    if (((BossDrakorState*)state)->flags198.b04)
    {
        player = Obj_GetPlayerObject();
        vec = objModelGetVecFn_800395d8(obj, 0xe);
        if (vec != NULL)
        {
            f32 hxsq;
            f32 hzsq;
            ObjPath_GetPointWorldPosition(obj, 4, &hx, &hy, &hz, 0);
            PSVECSubtract(&player->anim.localPos, (Vec*)&hx, (Vec*)&hx);
            hxsq = hx * hx;
            hzsq = hz * hz;
            d = (s16)getAngle(hy, sqrtf(hxsq + hzsq)) - (u16)vec[0];
            if (d > 0x8000)
            {
                d = (s16)((int)d - 0xffff);
            }
            if (d < -0x8000)
            {
                d += 0xffff;
            }
            step = (d < -(framesThisStep << 8)) ? -(framesThisStep << 8)
                                                : ((d > (framesThisStep << 8)) ? (framesThisStep << 8) : d);
            vec[0] += (s16)step;
        }
    }
    else
    {
        bossdrakor_updateHeadTracking(obj, drakorState);
    }
}
void bossdrakor_init(GameObject* obj, BossdrakorPlacement* init)
{
    int inner = *(int*)&(obj)->extra;
    f32 fz;
    BossDrakorState* s = (BossDrakorState*)inner;
    if (init->curveAdvanceStep == 0)
    {
        init->curveAdvanceStep = 0xa;
    }
    if (init->airMeterMax <= 0)
    {
        init->airMeterMax = 0x1e;
    }
    s->unk0C = 0;
    ((BossDrakorState*)inner)->flags198.b80 = 0;
    s->curveAdvanceStep = (f32)(u32)init->curveAdvanceStep;
    s->airMeterHandle = init->airMeterMax;
    fz = lbl_803E6510;
    s->attackTimerDuration = fz;
    s->moveState = 0;
    s->unk16C = -1;
    s->attackType = 0;
    s->moveSpeed = lbl_803E657C;
    ((BossDrakorState*)inner)->flags198.b40 = 1;
    s->shakeAmount = fz;
    s->shakeVel = fz;
    s->curveFollowState = 0;
    s->textTimer = fz;
    ((BossDrakorState*)inner)->flags198.b10 = 1;
    storeZeroToFloatParam(&s->attackTimer);
    ObjGroup_AddObject((int)obj, BOSSDRAKOR_OBJGROUP);
    storeZeroToFloatParam(&s->jawAnimAngle);
    (obj)->animEventCallback = bossdrakor_seqFn;
    Music_Trigger(MUSICTRIG_LVF_Tracking, 1);
    Music_Trigger(MUSICTRIG_citytombs, 1);
    s->lightObj = 0;
}

void bossdrakor_release(void)
{
}

void bossdrakor_initialise(void)
{
}
