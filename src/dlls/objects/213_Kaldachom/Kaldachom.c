/*
 * Kaldachom (DLL 0x00D5) - the "Kaldachom" door creature.
 *
 * A chompy baddie embedded in a locked door/wall: it pulls itself up,
 * lunges/bites at the player, and on death frees itself or returns to a
 * placement slot. Behaviour is driven entirely as gPlayerInterface
 * actor-control state handlers (kaldachom_stateHandler{A07,B00..B05}),
 * each returning the next requested control state (0 = stay).
 *
 * State record is the obj extra block (CfDoorlightState) whose tail holds
 * a KaldaChomControl (timers + soundFlags one-shots). The creature reads
 * its per-instance gate game bits (gameBitA/gameBitB) and an aggro chance
 * (placement unk2F vs randomGetRange) to pick its reaction.
 *
 * soundFlags one-shots: SOUNDFLAG_PULLUP_BURST = climb/attack/unlock burst played,
 * SOUNDFLAG_DOOR_CREAK = door-creak played once currentMoveProgress passes lbl_803E3088.
 */
#include "main/audio/sfx.h"
#include "main/audio/sfx_ids.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/gamebits.h"
#include "game/objects/object.h"
#include "sys/objects/lifecycle.h"
#include "main/dll/cf_doorlight_state.h"
#include "main/dll/baddie_control_interface.h"
#include "main/dll/cf_doorlight.h"
#include "main/dll/dll_00D6_kaldachomme_api.h"
#include "main/objhits.h"
#include "main/player_control_interface.h"
#include "main/frame_timing.h"
#include "main/dll/baddie_state.h"
#include "main/dll/kaldachom_state.h"
#include "main/dll/texscroll2.h"
#include "main/dll/partfx_interface.h"
#include "main/dll/objfx_api.h"
#include "main/object_render.h"
#include "main/vecmath.h"
#include "main/obj_path.h"
#include "main/obj_group.h"
#include "main/objprint_api.h"
#include "sys/objects.h"
#include "main/dll/dll_00D5_kaldachom.h"
#include "main/mapEventTypes.h"
#include "game/objects/object_setup.h"
#include "main/objanim.h"
#include "main/objfx.h"
#include "main/objtexture.h"
#include "main/resource.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/dll/player_state_api.h"

typedef struct KaldachomPlacement
{
    u8 pad0[0x2F - 0x0];
    u8 aggroChance; /* 0x2F: 0..99 chance to lunge instead of return */
} KaldachomPlacement;

#define BADDIE_CONTROL_MODE_PULLUP 2
#define BADDIE_CONTROL_MODE_RETURN 6

#define SOUNDFLAG_PULLUP_BURST 0x1
#define SOUNDFLAG_DOOR_CREAK   0x2

#define AGGRO_CHANCE_RANGE 0x63

extern f32 lbl_803E3078;
extern f32 lbl_803E307C;
extern f32 lbl_803E3080;
extern f32 lbl_803E3084;
extern f32 lbl_803E3088;
extern f32 lbl_803E308C;
extern f32 lbl_803E3090;
extern s16 lbl_803203F8[];
extern f32 lbl_80320404[];
extern f32 lbl_803E3094;
extern f32 lbl_803E3098;
extern f32 lbl_803E309C;
extern void* gKaldaChomStateHandlersB[];
extern void* gKaldaChomStateHandlersA[];
extern f32 lbl_803E30A0;
extern f32 lbl_803E30A4;
extern f32 lbl_803E30A8;
extern f32 lbl_803E30AC;
extern f32 lbl_803E30B0;
extern f32 gKaldachomPi;
extern f32 lbl_803E30B8;
extern f32 lbl_803E30BC;
extern f32 lbl_803E30C0;
extern f32 lbl_803E30C4;
extern f32 lbl_803E30C8;
extern f32 lbl_803E30CC;

int kaldachom_stateHandlerB05(int obj, int baddieState)
{
    int state;
    KaldaChomControl* control;
    int def;

    state = *(int*)&((GameObject*)obj)->extra;
    control = ((CfDoorlightState*)state)->control;
    if (((GroundBaddieState*)baddieState)->baddie.controlMode == BADDIE_CONTROL_MODE_PULLUP)
    {
        control->pullupSfxTimer = control->pullupSfxTimer - timeDelta;
        if (control->pullupSfxTimer <= 0.0f)
        {
            ((GroundBaddieState*)baddieState)->baddie.moveDone = 1;
        }
    }
    if ((s8)((GroundBaddieState*)baddieState)->baddie.moveDone != 0 ||
        (s8)((GroundBaddieState*)baddieState)->baddie.moveJustStartedB != 0)
    {
        if ((*gBaddieControlInterface)
                ->shouldDropTarget((GameObject*)obj, (void*)baddieState,
                                   (f32)(u32)((CfDoorlightState*)state)->aggroRange, 1) != 0)
        {
            return 5;
        }
        def = *(int*)&((GameObject*)obj)->anim.placementData;
        if ((int)randomGetRange(0, AGGRO_CHANCE_RANGE) < (int)((KaldachomPlacement*)def)->aggroChance)
        {
            (*gPlayerInterface)->setState((void*)obj, (void*)baddieState, 3);
        }
        else
        {
            control->pullupSfxTimer = (f32)(int)randomGetRange(0x12c, 0x258);
            (*gPlayerInterface)->setState((void*)obj, (void*)baddieState, 2);
        }
    }
    return 0;
}

int kaldachom_stateHandlerB04(int obj, GroundBaddieState* state)
{
    if ((s8)state->baddie.moveJustStartedB != 0)
    {
        (*gPlayerInterface)->setState((void*)obj, state, 1);
    }
    return 0;
}

int kaldachom_stateHandlerB03(GameObject* obj, GroundBaddieState* state)
{
    if ((s8)state->baddie.moveJustStartedB != 0)
    {
        GroundBaddieState* extra = obj->extra;
        extra->subMode = 0;
        mainSetBits(((CfDoorlightState*)extra)->gameBitB, 0);
        mainSetBits(((CfDoorlightState*)extra)->gameBitA, 1);
    }
    return 0;
}

int kaldachom_stateHandlerB02(GameObject* obj, GroundBaddieState* state)
{
    int sub = *(int*)&(obj)->extra;

    if ((s32)(s8)state->baddie.moveJustStartedB != 0)
    {
        ((CfDoorlightState*)sub)->control->soundFlags = 0;
        (*gPlayerInterface)->setState((void*)obj, state, 7);
        ObjHits_DisableObject(obj);
        *(u8*)&(obj)->anim.resetHitboxMode = (u8)(*(u8*)&(obj)->anim.resetHitboxMode | INTERACT_FLAG_DISABLED);
        ((CfDoorlightState*)sub)->flags400 = (u16)(((CfDoorlightState*)sub)->flags400 | 0x20);
        ((CfDoorlightState*)sub)->glowAlpha = lbl_803E3078;
        ((CfDoorlightState*)sub)->unk3EC = lbl_803E307C;
    }
    else if ((s32)(s8)state->baddie.moveDone != 0)
    {
        if ((obj)->anim.placementData == NULL)
        {
            Obj_FreeObject(obj);
            return 0;
        }
        return 4;
    }
    return 0;
}

int kaldachom_stateHandlerB01(int* obj, GroundBaddieState* state)
{
    KaldaChomControl* control = ((CfDoorlightState*)((GameObject*)obj)->extra)->control;
    if (state->baddie.controlMode == BADDIE_CONTROL_MODE_RETURN)
    {
        f32 zero;
        f32 timer;
        if ((s8)state->baddie.moveJustStartedB != 0)
        {
            control->returnStateTimer = lbl_803E3080;
        }
        timer = control->returnStateTimer;
        zero = 0.0f;
        if (timer != zero)
        {
            control->returnStateTimer = timer - timeDelta;
            if (control->returnStateTimer < zero)
            {
                control->returnStateTimer = zero;
            }
        }
        else
        {
            return 6;
        }
    }
    else
    {
        if ((s8)state->baddie.moveDone != 0)
            return 6;
    }
    return 0;
}

int kaldachom_stateHandlerB00(int* obj, GroundBaddieState* state)
{
    if (state->baddie.targetObj != NULL)
    {
        if ((s8)state->baddie.moveJustStartedB != 0)
        {
            f32 fz = 0.0f;
            state->baddie.animSpeedB = fz;
            state->baddie.animSpeedA = fz;
            (*gPlayerInterface)->setState(obj, state, 0);
        }
        else if ((s8)state->baddie.moveDone != 0)
        {
            return 6;
        }
    }
    return 0;
}

int kaldachom_stateHandlerA07(GameObject* obj, int baddieState)
{
    int state;
    KaldaChomControl* control;

    state = *(int*)&(obj)->extra;
    ((GroundBaddieState*)baddieState)->baddie.stateTag = 3;
    ((GroundBaddieState*)baddieState)->baddie.moveSpeed = lbl_803E3084;
    {
        f32 fz = 0.0f;
        ((GroundBaddieState*)baddieState)->baddie.animSpeedA = fz;
        ((GroundBaddieState*)baddieState)->baddie.animSpeedB = fz;
        if (*(char*)&((GroundBaddieState*)baddieState)->baddie.moveJustStartedA != '\0')
        {
            ObjAnim_SetCurrentMove((int)obj, 5, fz, 0);
            ((GroundBaddieState*)baddieState)->baddie.moveDone = 0;
        }
    }
    {
        int v = *(int*)&((GroundBaddieState*)baddieState)->baddie.eventFlags;
        if ((v & 0x1000) != 0)
        {
            *(int*)&((GroundBaddieState*)baddieState)->baddie.eventFlags = v & ~0x1000;
            kaldachompme_setLinkedMouthMode((u8*)obj, 2);
        }
    }
    control = ((CfDoorlightState*)state)->control;
    if ((control->soundFlags & SOUNDFLAG_PULLUP_BURST) == 0)
    {
        Sfx_PlayFromObject((int)obj, SFXTRIG_mn_impyflap16);
        Sfx_PlayFromObject((int)obj, SFXTRIG_dn_boar1_c_277);
        Sfx_PlayFromObject((int)obj, SFXTRIG_en_rfall5_c);
        control->soundFlags |= SOUNDFLAG_PULLUP_BURST;
        {
            GameObject* linkedObj;
            if (((CfDoorlightState*)state)->spawnsLinkedObj != 0)
            {
                linkedObj = (*gBaddieControlInterface)->spawnChild(obj, 6, -1, 0);
            }
            else
            {
                linkedObj = NULL;
            }
            if (linkedObj != NULL)
            {
                f32 fz = 0.0f;
                (**(void (**)(GameObject*, f32, f32, f32))(
                    *(int*)(*(int*)((char*)linkedObj + 0x68)) + 0x2c))(linkedObj, fz, lbl_803E3078, fz);
            }
        }
    }
    if ((control->soundFlags & SOUNDFLAG_DOOR_CREAK) == 0)
    {
        if ((obj)->anim.currentMoveProgress > lbl_803E3088)
        {
            Sfx_PlayFromObject((int)obj, SFXTRIG_wp_iceywindlp16_233);
            control->soundFlags |= SOUNDFLAG_DOOR_CREAK;
        }
    }
    (obj)->anim.alpha = (lbl_803E3078 - (obj)->anim.currentMoveProgress) * lbl_803E308C;
    return 0;
}

/*
 * kaldachom (DLL 0x00D5) attack/movement state handlers, table A.
 *
 * These seven functions provide gKaldaChomStateHandlersA[0..6]; the table runs
 * [0..7] (A07 is defined earlier in this file), which also registers them.
 * Handlers are stepped one per frame while the baddie is active. Each (a) on the
 * first frame of a move (moveJustStartedA) selects the anim move via
 * ObjAnim_SetCurrentMove and may play an attack sfx / toggle the hit volume, and
 * (b) every frame writes the per-mode movement speed and the stateTag mode tag back
 * into the GroundBaddieState. A05/A02 index the move/speed tables (lbl_803203F8
 * move ids, lbl_80320404 speeds) by the control record's climbFxIndex. A00/A01
 * raise/clear gameBitB and drive the linked-mouth mode.
 */
#define KALDACHOM_EVENT_MOUTH_LINK 0x1000

int kaldachom_stateHandlerA06(GameObject* obj, int statePtr)
{
    if ((s32)(s8)((GroundBaddieState*)statePtr)->baddie.moveJustStartedA != 0)
    {
        if ((s32)(s8)((GroundBaddieState*)statePtr)->baddie.moveJustStartedA != 0)
        {
            ObjAnim_SetCurrentMove((int)obj, 8, 0.0f, 0);
            ((GroundBaddieState*)statePtr)->baddie.moveDone = 0;
        }
        Sfx_PlayFromObject((int)obj, SFXTRIG_dn_boar1_c_277);
    }
    (obj)->anim.rotX += 546;
    ((GroundBaddieState*)statePtr)->baddie.stateTag = 1;
    ((GroundBaddieState*)statePtr)->baddie.moveSpeed = lbl_803E3090;
    ((GroundBaddieState*)statePtr)->baddie.animSpeedA = 0.0f;
    return 0;
}

int kaldachom_stateHandlerA05(GameObject* obj, int statePtr)
{
    KaldaChomControl* control = ((GroundBaddieState*)(obj)->extra)->control;

    if ((s32)(s8)((GroundBaddieState*)statePtr)->baddie.moveJustStartedA != 0)
    {
        if ((s32)(s8)((GroundBaddieState*)statePtr)->baddie.moveJustStartedA != 0)
        {
            ObjAnim_SetCurrentMove((int)obj, lbl_803203F8[4], 0.0f, 0);
            ((GroundBaddieState*)statePtr)->baddie.moveDone = 0;
        }
        control->climbFxIndex = 4;
    }
    ((GroundBaddieState*)statePtr)->baddie.moveSpeed = lbl_80320404[control->climbFxIndex];
    ((GroundBaddieState*)statePtr)->baddie.stateTag = 1;
    return 0;
}

int kaldachom_stateHandlerA04(int obj, int statePtr)
{
    if ((s32)(s8)((GroundBaddieState*)statePtr)->baddie.moveJustStartedA != 0)
    {
        if ((s32)(s8)((GroundBaddieState*)statePtr)->baddie.moveJustStartedA != 0)
        {
            ObjAnim_SetCurrentMove(obj, 3, 0.0f, 0);
            ((GroundBaddieState*)statePtr)->baddie.moveDone = 0;
        }
        Sfx_PlayFromObject(obj, SFXTRIG_dn_boar1_c_277);
    }
    ((GroundBaddieState*)statePtr)->baddie.stateTag = 3;
    ((GroundBaddieState*)statePtr)->baddie.moveSpeed = lbl_803E3090;
    ((GroundBaddieState*)statePtr)->baddie.animSpeedA = 0.0f;
    return 0;
}

int kaldachom_stateHandlerA03(int obj, int statePtr)
{
    if ((s32)(s8)((GroundBaddieState*)statePtr)->baddie.moveJustStartedA != 0)
    {
        ObjHits_EnableObject((GameObject*)obj);
        if ((s32)(s8)((GroundBaddieState*)statePtr)->baddie.moveJustStartedA != 0)
        {
            ObjAnim_SetCurrentMove(obj, randomGetRange(6, 7), 0.0f, 0);
            ((GroundBaddieState*)statePtr)->baddie.moveDone = 0;
        }
    }
    ((GroundBaddieState*)statePtr)->baddie.moveSpeed = lbl_803E3094;
    ((GroundBaddieState*)statePtr)->baddie.stateTag = 1;
    return 0;
}

int kaldachom_stateHandlerA02(GameObject* obj, int statePtr)
{
    KaldaChomControl* control = ((GroundBaddieState*)(obj)->extra)->control;

    if ((s32)(s8)((GroundBaddieState*)statePtr)->baddie.moveJustStartedA != 0)
    {
        if ((s32)(s8)((GroundBaddieState*)statePtr)->baddie.moveJustStartedA != 0)
        {
            ObjAnim_SetCurrentMove((int)obj, lbl_803203F8[randomGetRange(0, 4)], 0.0f, 0);
            ((GroundBaddieState*)statePtr)->baddie.moveDone = 0;
        }
        ObjHits_EnableObject(obj);
        control->climbFxIndex = 4;
    }
    ((GroundBaddieState*)statePtr)->baddie.moveSpeed = lbl_80320404[control->climbFxIndex];
    ((GroundBaddieState*)statePtr)->baddie.stateTag = 1;
    return 0;
}

int kaldachom_stateHandlerA01(GameObject* obj, int statePtr)
{
    GroundBaddieState* state = (obj)->extra;

    if ((s32)(s8)((GroundBaddieState*)statePtr)->baddie.moveJustStartedA != 0)
    {
        if ((s32)(s8)((GroundBaddieState*)statePtr)->baddie.moveJustStartedA != 0)
        {
            ObjAnim_SetCurrentMove((int)obj, 5, 0.0f, 0);
            ((GroundBaddieState*)statePtr)->baddie.moveDone = 0;
        }
        ObjHits_DisableObject(obj);
        ((GroundBaddieState*)statePtr)->baddie.moveSpeed = lbl_803E307C;
        ((GroundBaddieState*)statePtr)->baddie.animSpeedA = 0.0f;
    }
    else if ((s32)(s8)((GroundBaddieState*)statePtr)->baddie.moveDone != 0)
    {
        mainSetBits(state->gameBitB, 0);
        if ((s32)(s8)((GroundBaddieState*)statePtr)->baddie.moveJustStartedA != 0)
        {
            ObjAnim_SetCurrentMove((int)obj, 4, 0.0f, 0);
            ((GroundBaddieState*)statePtr)->baddie.moveDone = 0;
        }
        state->targetState = 0;
    }
    if ((s32)(((GroundBaddieState*)statePtr)->baddie.eventFlags & KALDACHOM_EVENT_MOUTH_LINK) != 0)
    {
        ((GroundBaddieState*)statePtr)->baddie.eventFlags &= ~KALDACHOM_EVENT_MOUTH_LINK;
        kaldachompme_setLinkedMouthMode((u8*)obj, 2);
    }
    return 0;
}

int kaldachom_stateHandlerA00(GameObject* obj, int statePtr)
{
    GroundBaddieState* state = (obj)->extra;

    if ((s32)(s8)((GroundBaddieState*)statePtr)->baddie.moveJustStartedA != 0)
    {
        if ((s32)(s8)((GroundBaddieState*)statePtr)->baddie.moveJustStartedA != 0)
        {
            ObjAnim_SetCurrentMove((int)obj, 4, 0.0f, 0);
            ((GroundBaddieState*)statePtr)->baddie.moveDone = 0;
        }
        kaldachompme_setLinkedMouthMode((u8*)obj, 1);
        ((GroundBaddieState*)statePtr)->baddie.physicsActive = 1;
        mainSetBits(state->gameBitB, 1);
        *(u8*)&(obj)->anim.resetHitboxMode &= ~INTERACT_FLAG_DISABLED;
        (obj)->anim.alpha = 0xff;
        ((GroundBaddieState*)statePtr)->baddie.stateTag = 1;
        ((GroundBaddieState*)statePtr)->baddie.moveSpeed = lbl_803E3098 + ((f32)(u32)state->aggression / lbl_803E309C);
        ObjHits_EnableObject(obj);
    }
    else if ((s32)(s8)((GroundBaddieState*)statePtr)->baddie.moveDone != 0)
    {
        state->targetState = 1;
    }
    return 0;
}

/*
 * kaldachom (DLL 0x00D5, object type 0x49) - the "Kaldachom" / mouth-flytrap
 * ground baddie. Driven through gBaddieControlInterface (movement/combat
 * dispatch) and gPlayerInterface; combat (kaldachom_updateCombat) handles
 * the player's hit response, knockback, hit-point decrement and death
 * transition (substate 1 = stun, 2 = dead). Mouth-point projectiles are
 * spawned from anim events (kaldachom_handleAnimEvents) at the upper/lower
 * mouth path points, and a dust object is spawned while loading is locked.
 * The render path scrolls a texture by a sine-driven phase and refreshes the
 * mouth path points. State-machine handler tables A/B are populated at
 * initialise time and stepped by gPlayerInterface slot 8 each update.
 */
/* object group this object belongs to */
#define KALDACHOM_OBJGROUP 3

#define KALDACHOM_OBJFLAG_HITDETECT_DISABLED 0x2000

/* Dust child; the spawned object is cached in control->spawnedDustObj. */
#define KALDACHOM_CHILD_OBJ_DUST 0x55e
#define KALDACHOM_PARTFX_DUST    0x717 /* dust-puff particle burst kicked up on landing */

/* Mouth-point projectile spawned in kaldaChomFn_80168374 at the upper/lower
 * mouth path points and given target-aimed velocity (docblock: "Mouth-point
 * projectiles are spawned from anim events at the upper/lower mouth path points"). */
#define KALDACHOM_CHILD_OBJ_MOUTH_PROJECTILE 0x51b
#define KALDACHOM_EFFECT_RESOURCE_ID         0x5a /* shared effect resource -> gKaldachomEffectResource */



typedef struct KaldaCombatParams
{
    u32 unk00;
    u32 unk04;
    u32 unk08;
    u32 unk0C;
} KaldaCombatParams;

STATIC_ASSERT(sizeof(KaldaCombatParams) == 0x10);

/* this DLL's data/sdata2 pool: lbl_803E30xx are float constants; the
   gKaldachom*SpawnScratch globals are mutable scratch (fx spawn position / radius). */
const KaldaCombatParams gKaldachomCombatParams = {8, 255, 255, 120};
f32 gKaldachomMouthSpawnScratch;
f32 gKaldachomDustSpawnScratch;
void* gKaldachomEffectResource;
u8 gKaldachomHitLightWork[0x18];

void kaldaChomFn_8016821c(GameObject* obj, KaldaChomControl* control)
{
    u8 loadLocked;
    int placement;
    int work;

    placement = *(int*)&(obj)->anim.placementData;
    gKaldachomDustSpawnScratch = lbl_803E30A0 + (float)(int)*(char*)(placement + 0x28) / lbl_803E30A4;
    control->hitFlashTimer = lbl_803E308C;
    Sfx_PlayFromObject((int)obj, SFXTRIG_wp_beamgenlp16_276);
    work = 0x28;
    do
    {
        (*gPartfxInterface)
            ->spawnObject((void*)obj, KALDACHOM_PARTFX_DUST, 0, 4, 0xffffffff, &gKaldachomDustSpawnScratch);
        work--;
    } while (work != 0);
    if ((control->spawnedDustObj == NULL) && (loadLocked = Obj_IsLoadingLocked(), loadLocked != '\0'))
    {
        work = (int)Obj_AllocObjectSetup(0x24, KALDACHOM_CHILD_OBJ_DUST);
        ((ObjPlacement*)work)->posX = (obj)->anim.localPosX;
        ((ObjPlacement*)work)->posY = lbl_803E30A8 + (obj)->anim.localPosY;
        ((ObjPlacement*)work)->posZ = (obj)->anim.localPosZ;
        ((ObjPlacement*)work)->color[0] = ((ObjPlacement*)placement)->color[0];
        ((ObjPlacement*)work)->color[1] = ((ObjPlacement*)placement)->color[1];
        ((ObjPlacement*)work)->color[2] = ((ObjPlacement*)placement)->color[2];
        ((ObjPlacement*)work)->color[3] = ((ObjPlacement*)placement)->color[3];
        work = (int)Obj_SetupObject((ObjPlacement*)work, 5, 0xffffffff, 0xffffffff, 0);
        control->spawnedDustObj = (void*)work;
        ((GameObject*)control->spawnedDustObj)->anim.rootMotionScale = gKaldachomDustSpawnScratch;
    }
}

void kaldaChomFn_80168374(GameObject* obj, int state, u8 useUpperMouthPoint)
{
    KaldaChomControl* control;
    int ref;
    u8* setup;
    f32 yJitter;
    f32 spd;
    f32 heightOffset;
    f32 mouthY;

    control = ((CampfireState*)state)->control;
    ref = *(int*)&obj->anim.placementData;
    if (Obj_IsLoadingLocked() != 0)
    {
        heightOffset = lbl_803E30A0 + (f32)(s32) * (s8*)(ref + 0x28) / lbl_803E30A4;
        ref = (int)Obj_AllocObjectSetup(0x24, KALDACHOM_CHILD_OBJ_MOUTH_PROJECTILE);
        if (useUpperMouthPoint != 0)
        {
            ((ObjPlacement*)ref)->posX = control->upperMouthPosX;
            ((ObjPlacement*)ref)->posY = control->upperMouthPosY;
            ((ObjPlacement*)ref)->posZ = control->upperMouthPosZ;
        }
        else
        {
            ((ObjPlacement*)ref)->posX = control->lowerMouthPosX;
            ((ObjPlacement*)ref)->posY = control->lowerMouthPosY;
            ((ObjPlacement*)ref)->posZ = control->lowerMouthPosZ;
        }
        ((ObjPlacement*)ref)->color[0] = 1;
        ((ObjPlacement*)ref)->color[1] = 4;
        ((ObjPlacement*)ref)->color[2] = 0xff;
        ((ObjPlacement*)ref)->color[3] = 0xff;
        setup = (u8*)Obj_SetupObject((ObjPlacement*)ref, 5, 0xffffffff, 0xffffffff, 0);
        if (setup != NULL)
        {
            spd = lbl_803E30AC * (((GroundBaddieState*)state)->baddie.targetDistance /
                                  (f32)(u32)((GroundBaddieState*)state)->aggroRange);
            ((GameObject*)setup)->anim.velocityX =
                (((GameObject*)((GroundBaddieState*)state)->baddie.targetObj)->anim.localPosX -
                 ((ObjPlacement*)ref)->posX) /
                spd;
            yJitter = (f32)(s32)randomGetRange(-0xa, 0xa);
            mouthY = lbl_803E30A8 * heightOffset +
                     ((GameObject*)((GroundBaddieState*)state)->baddie.targetObj)->anim.localPosY;
            ((GameObject*)setup)->anim.velocityY = (mouthY + yJitter - ((ObjPlacement*)ref)->posY) / spd;
            ((GameObject*)setup)->anim.velocityZ =
                (((GameObject*)((GroundBaddieState*)state)->baddie.targetObj)->anim.localPosZ -
                 ((ObjPlacement*)ref)->posZ) /
                spd;
        }
    }
}

void kaldachom_handleAnimEvents(GameObject* obj, int state, int eventStateArg)
{
    KaldaChomControl* control = ((CampfireState*)state)->control;
    GroundBaddieState* eventState = (GroundBaddieState*)eventStateArg;
    int spawnCount;

    gKaldachomMouthSpawnScratch =
        lbl_803E30A0 + (f32)(s32)(s8) * (u8*)(*(int*)&(obj)->anim.placementData + 0x28) / lbl_803E30A4;

    if (((s32)eventState->baddie.eventFlags & BADDIE_EVENT_FOOTSTEP) != 0)
    {
        eventState->baddie.eventFlags &= ~BADDIE_EVENT_FOOTSTEP;
        Sfx_PlayFromObject((int)obj, SFXTRIG_mn_lummy211_273);
    }
    if (((s32)eventState->baddie.eventFlags & 0x80) != 0)
    {
        control->climbFxIndex = randomGetRange(0, 2);
        eventState->baddie.eventFlags &= ~0x80;
        Sfx_PlayFromObject((int)obj, SFXTRIG_mn_impyflap16);
        for (spawnCount = (2 - control->climbFxIndex) * 10; spawnCount != 0; spawnCount--)
        {
            (*gPartfxInterface)->spawnObject((void*)obj, 1809, 0, 4, -1, &gKaldachomMouthSpawnScratch);
        }
    }
    if (((s32)eventState->baddie.eventFlags & 0x40) != 0)
    {
        eventState->baddie.eventFlags &= ~0x40;
        kaldaChomFn_80168374(obj, state, 0);
    }
    if (((s32)eventState->baddie.eventFlags & 0x800) != 0)
    {
        eventState->baddie.eventFlags &= ~0x800;
        kaldaChomFn_80168374(obj, state, 1);
    }
    if (((s32)eventState->baddie.eventFlags & BADDIE_EVENT_LANDING) != 0)
    {
        eventState->baddie.eventFlags &= ~BADDIE_EVENT_LANDING;
        Sfx_PlayFromObject((int)obj, SFXTRIG_mn_cling03);
    }
    if (((s32)eventState->baddie.eventFlags & 0x400) != 0)
    {
        control->climbFxIndex = 3;
        spawnCount = 10;
        do
        {
            (*gPartfxInterface)->spawnObject((void*)obj, 1808, 0, 4, -1, &gKaldachomMouthSpawnScratch);
            spawnCount--;
        } while (spawnCount != 0);
        eventState->baddie.eventFlags &= ~0x400;
    }
}

typedef struct KaldaCombatStack
{
    f32 dx;
    f32 dy;
    f32 dz;
    KaldaCombatParams p;
} KaldaCombatStack;

void kaldachom_updateCombat(GameObject* obj, int stateWithBaddieData, int state)
{
    KaldaChomControl* control;
    GameObject* playerObj;
    int result;
    u8 rnd;
    KaldaCombatStack st;
    u16 hitType;
    u16 hitAux1;
    u16 hitAux2;

    control = ((CampfireState*)stateWithBaddieData)->control;
    st.p = gKaldachomCombatParams;
    playerObj = Obj_GetPlayerObject();
    if (((GroundBaddieState*)state)->baddie.targetObj != NULL)
    {
        GameObject* target = (GameObject*)((GroundBaddieState*)state)->baddie.targetObj;
        st.dx = target->anim.worldPosX - obj->anim.worldPosX;
        st.dy = target->anim.worldPosY - obj->anim.worldPosY;
        st.dz = target->anim.worldPosZ - obj->anim.worldPosZ;
        ((GroundBaddieState*)state)->baddie.targetDistance = sqrtf(st.dz * st.dz + (st.dx * st.dx + st.dy * st.dy));
    }
    (*gBaddieControlInterface)
        ->processMessages(obj, (void*)state, (void*)(stateWithBaddieData + 0x35c),
                          ((GroundBaddieState*)stateWithBaddieData)->gameBitB, NULL, 0, 0, 4);
    (*gBaddieControlInterface)
        ->getTargetGeometry(obj, playerObj, 4, &hitType, &hitAux1, &hitAux2);
    if ((hitType == 1) || (hitType == 2))
    {
        result = (*gBaddieControlInterface)
                     ->updateHitReaction(obj, (void*)state, (char*)stateWithBaddieData + 0x35c,
                                         ((GroundBaddieState*)stateWithBaddieData)->gameBitB, NULL, NULL, 1,
                                         gKaldachomHitLightWork);
        if (result != 0)
        {
            if ((result != 0x10) && (result != 0x11))
            {
                objLightFn_8009a1dc((void*)obj, lbl_803E30BC, gKaldachomHitLightWork, 3, 0);
                (*gPlayerInterface)->setState(obj, (void*)state, 4);
                ((GroundBaddieState*)state)->baddie.hitPoints -= 1;
                Obj_SetModelColorFadeRecursive(obj, 0xf, 200, 0, 0, 1);
                Sfx_PlayFromObject((int)obj, SFXTRIG_stftest);
            }
            if (*(s8*)&((GroundBaddieState*)state)->baddie.hitPoints < 1)
            {
                ((GroundBaddieState*)state)->baddie.substate = 2;
            }
        }
    }
    else
    {
        result = (*gBaddieControlInterface)
                     ->updateHitReaction(obj, (void*)state, (char*)stateWithBaddieData + 0x35c,
                                         ((GroundBaddieState*)stateWithBaddieData)->gameBitB, NULL, NULL, 1,
                                         gKaldachomHitLightWork);
        if (result != 0)
        {
            if (result != 0x11)
            {
                if ((result != 0x10) && (control->hitFlashTimer < lbl_803E30C0))
                {
                    kaldaChomFn_8016821c(obj, control);
                    *(f32*)(gKaldachomHitLightWork + 8) = lbl_803E3078;
                    *(u16*)(gKaldachomHitLightWork + 4) = 0;
                    *(u16*)(gKaldachomHitLightWork + 2) = 0;
                    *(u16*)(gKaldachomHitLightWork + 0) = 0;
                    (*(void (**)(int, int, void*, int, int, void*))(*(int*)gKaldachomEffectResource + 4))(
                        0, 1, gKaldachomHitLightWork, 0x401, -1, (KaldaCombatParams*)((u8*)&st + 0xc));
                    fn_802961FC(playerObj, 2);
                    (*gPlayerInterface)->setState(obj, (void*)state, 5);
                    objLightFn_8009a1dc((void*)obj, lbl_803E30BC, gKaldachomHitLightWork, 4, 0);
                    Sfx_PlayFromObject((int)obj, SFXTRIG_swdout1);
                }
            }
            else
            {
                if (((GroundBaddieState*)state)->baddie.substate != 1)
                {
                    (*gPlayerInterface)->setState(obj, (void*)state, 6);
                    ((GroundBaddieState*)state)->baddie.moveJustStartedB = 1;
                    ((GroundBaddieState*)state)->baddie.moveJustStartedA = 1;
                    ((GroundBaddieState*)state)->baddie.substate = 1;
                    objLightFn_8009a1dc((void*)obj, lbl_803E30BC, gKaldachomHitLightWork, 1, 0);
                    Sfx_PlayFromObject((int)obj, SFXTRIG_stftest);
                    Sfx_PlayFromObject((int)obj, SFXTRIG_baddie_rach_call3);
                }
            }
        }
        if (*(s8*)&((GroundBaddieState*)state)->baddie.hitPoints < 1)
        {
            ((GroundBaddieState*)state)->baddie.substate = 2;
        }
    }

    if (control->spawnedDustObj != NULL)
    {
        if (control->hitFlashTimer <= 0.0f)
        {
            f32 zeroConst = 0.0f;
            ((GameObject*)control->spawnedDustObj)->anim.alpha = 0;
            control->hitFlashTimer = zeroConst;
        }
        else
        {
            rnd = randomGetRange(0, (u8)(s32)control->hitFlashTimer);
            ((GameObject*)control->spawnedDustObj)->anim.alpha = rnd;
            ((GameObject*)control->spawnedDustObj)->anim.rotZ = obj->anim.rotZ;
            ((GameObject*)control->spawnedDustObj)->anim.rotY = obj->anim.rotY;
            ((GameObject*)control->spawnedDustObj)->anim.rotX = obj->anim.rotX;
            control->hitFlashTimer = control->hitFlashTimer - lbl_803E30C4 * timeDelta;
        }
    }
}

void kaldachom_func0B(void)
{
}

s16 kaldachom_setScale(int* obj)
{
    return ((CampfireState*)((GameObject*)obj)->extra)->controlMode;
}
int kaldachom_getExtraSize(void)
{
    return sizeof(CampfireState);
}
int kaldachom_getObjectTypeId(void)
{
    return 0x49;
}

void kaldachom_free(GameObject* obj)
{
    u32 state;

    state = *(u32*)&(obj)->extra;
    ObjGroup_RemoveObject((int)obj, KALDACHOM_OBJGROUP);
    (*gBaddieControlInterface)->releaseState(obj, (void*)state, 0x20);
}

void kaldachom_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible)
{
    int state;
    KaldaChomControl* control;

    state = *(int*)&obj->extra;
    if (visible != 0)
    {
        switch (obj->userData1)
        {
        case 0:
            if (((GroundBaddieState*)state)->glowAlpha != 0.0f)
            {
                fn_8003B5E0(200, 0, 0, (int)((GroundBaddieState*)state)->glowAlpha);
            }
            objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, lbl_803E3078);
            if ((((GroundBaddieState*)state)->flags400 & 0x60) != 0)
            {
                objParticleFn_80099d84(obj, lbl_803E3078, 3, ((GroundBaddieState*)state)->glowAlpha, 0);
            }
            control = ((CampfireState*)state)->control;
            ObjPath_GetPointWorldPosition(obj, 2, &control->upperMouthPosX, &control->upperMouthPosY,
                                          &control->upperMouthPosZ, 0);
            ObjPath_GetPointWorldPosition(obj, 1, &control->lowerMouthPosX, &control->lowerMouthPosY,
                                          &control->lowerMouthPosZ, 0);
            break;
        }
    }
}

void kaldachom_hitDetect(void)
{
}

void kaldachom_update(GameObject* obj)
{
    int cond;
    u32 player;
    int texture;
    int ref;
    int state;
    f32 scrollPhase;

    state = *(int*)&obj->extra;
    ref = *(int*)&obj->anim.placementData;
    if (obj->userData1 != 0)
    {
        if ((((CampfireState*)state)->substate != 3) &&
            (cond = (*gMapEventInterface)->shouldNotSaveTime(((ObjPlacement*)ref)->mapId), cond != 0))
        {
            (*gBaddieControlInterface)
                ->initGroundBaddie(obj, (u8*)ref, (u8*)state, 8, 6, 0, 0x26, lbl_803E30C8);
            ((GroundBaddieState*)state)->targetState = 0;
            Sfx_PlayFromObject((int)obj, SFXTRIG_mn_lummy211);
            ObjAnim_SetCurrentMove((int)obj, 4, 0.0f, OBJANIM_MOVE_CONTROL_SKIP_EVENT_COUNTDOWN);
            ((GroundBaddieState*)state)->baddie.moveDone = 0;
            obj->anim.alpha = 0xff;
            *(u8*)&obj->anim.resetHitboxMode =
                *(u8*)&obj->anim.resetHitboxMode | INTERACT_FLAG_DISABLED;
        }
    }
    else
    {
        ref = (*gBaddieControlInterface)->isObjectValid(obj, (void*)state, 0);
        if (ref == 0)
        {
            *(u16*)&((GroundBaddieState*)state)->targetState = 0;
        }
        else
        {
            kaldachom_updateCombat(obj, state, state);
            if (((CampfireState*)state)->targetState == 0)
            {
                texture = (int)((CampfireState*)state)->control;
                ((KaldaChomControl*)texture)->pullupSfxTimer = ((KaldaChomControl*)texture)->pullupSfxTimer - timeDelta;
                if (((KaldaChomControl*)texture)->pullupSfxTimer <= 0.0f)
                {
                    Sfx_PlayFromObject((int)obj, SFXTRIG_mn_lummy111);
                    ((KaldaChomControl*)texture)->pullupSfxTimer = (f32)(int)randomGetRange(300, 600);
                }
                player = (u32)Obj_GetPlayerObject();
                *(u32*)&((GroundBaddieState*)state)->baddie.targetObj = player;
                if (((CampfireState*)state)->controlMode != 6)
                {
                    (*gPlayerInterface)->rotateTowardTarget(obj, (void*)state, timeDelta, 5);
                }
                ref = (int)(*gBaddieControlInterface)
                          ->findAggroTarget(obj, (void*)state, (f32)(u32)((CampfireState*)state)->aggroRange, 0x8000);
                if ((void*)ref != NULL)
                {
                    (*gBaddieControlInterface)
                        ->startHitReaction(obj, (void*)state, (char*)state + 0x35c,
                                           ((CampfireState*)state)->gameBitB, NULL, 0, 0, 4, -1);
                    *(u8*)&((GroundBaddieState*)state)->baddie.hasTarget = 0;
                    *(u16*)&((GroundBaddieState*)state)->targetState = 1;
                }
            }
            else
            {
                ref = (int)((CampfireState*)state)->control;
                texture = (int)objFindTexture(obj, 0, 0);
                ((KaldaChomControl*)ref)->textureScrollAngle += 0x1000;
                scrollPhase =
                    mathSinf((gKaldachomPi * (f32)(s32)((KaldaChomControl*)ref)->textureScrollAngle) / lbl_803E30B8);
                scrollPhase = lbl_803E3078 + scrollPhase;
                ((ObjTextureRuntimeSlot*)texture)->textureId = (int)(lbl_803E30B0 * scrollPhase);
                player = (u32)Obj_GetPlayerObject();
                *(u32*)&((GroundBaddieState*)state)->baddie.targetObj = player;
                kaldachom_handleAnimEvents(obj, state, state);
                (*gBaddieControlInterface)
                    ->updateGravity(obj, (void*)state, 0.0f, -1);
                if (((CampfireState*)state)->controlMode != 6)
                {
                    (*gPlayerInterface)->rotateTowardTarget(obj, (void*)state, timeDelta, 5);
                }
                ((GroundBaddieState*)state)->savedObjC0 = *(int*)&obj->pendingParentObj;
                *(u32*)&obj->pendingParentObj = 0;
                (*gPlayerInterface)->update(obj, (void*)state, timeDelta, timeDelta, &gKaldaChomStateHandlersA,
                                            &gKaldaChomStateHandlersB);
                *(u32*)&obj->pendingParentObj = ((GroundBaddieState*)state)->savedObjC0;
            }
        }
    }
}

void kaldachom_init(GameObject* obj, int data, int skip_alloc)
{
    int state;
    KaldaChomControl* control;
    int player;
    u8 initMode;

    state = *(int*)&(obj)->extra;
    initMode = 6;
    if (skip_alloc != 0)
    {
        initMode |= 1;
    }
    (*gBaddieControlInterface)
        ->initGroundBaddie(obj, (u8*)data, (u8*)state, 8, 6, 0, initMode, lbl_803E30C8);
    (obj)->animEventCallback = NULL;
    control = ((CampfireState*)state)->control;
    ObjAnim_SetCurrentMove((int)obj, 4, 0.0f, OBJANIM_MOVE_CONTROL_SKIP_EVENT_COUNTDOWN);
    (obj)->anim.currentMoveProgress = lbl_803E307C;
    *(u8*)&(obj)->anim.resetHitboxMode = *(u8*)&(obj)->anim.resetHitboxMode | INTERACT_FLAG_DISABLED;
    (*gPlayerInterface)->setState(obj, (void*)state, 0);
    *(u16*)&((GroundBaddieState*)state)->baddie.substate = 0;
    ((GroundBaddieState*)state)->baddie.moveSpeed = lbl_803E307C;
    ((GroundBaddieState*)state)->baddie.animSpeedA = 0.0f;
    player = (int)Obj_GetPlayerObject();
    ((CampfireState*)state)->targetObj = player;
    ((GroundBaddieState*)state)->baddie.physicsActive = 0;
    ObjHits_DisableObject(obj);
    control->pullupSfxTimer = (f32)(int)randomGetRange(300, 600);
    control->idleAnimTimer = (f32)(int)randomGetRange(0, 499);
    control->unk3C = 0.0f;
    control->spawnedDustObj = NULL;
    (obj)->objectFlags = (obj)->objectFlags | KALDACHOM_OBJFLAG_HITDETECT_DISABLED;
    (obj)->anim.rootMotionScale = lbl_803E30A0 + (f32)(s32) * (s8*)(data + 0x28) / lbl_803E30A4;
    ObjHitbox_SetSphereRadius((ObjAnimComponent*)obj, (int)(lbl_803E30CC * (obj)->anim.rootMotionScale));
    if (skip_alloc == 0)
    {
        gKaldachomEffectResource = Resource_Acquire(KALDACHOM_EFFECT_RESOURCE_ID, 1);
    }
}

void kaldachom_release(void)
{
}

void kaldachom_initialise(void)
{
    gKaldaChomStateHandlersA[0] = kaldachom_stateHandlerA00;
    gKaldaChomStateHandlersA[1] = kaldachom_stateHandlerA01;
    gKaldaChomStateHandlersA[2] = kaldachom_stateHandlerA02;
    gKaldaChomStateHandlersA[3] = kaldachom_stateHandlerA03;
    gKaldaChomStateHandlersA[4] = kaldachom_stateHandlerA04;
    gKaldaChomStateHandlersA[5] = kaldachom_stateHandlerA05;
    gKaldaChomStateHandlersA[6] = kaldachom_stateHandlerA06;
    gKaldaChomStateHandlersA[7] = kaldachom_stateHandlerA07;
    gKaldaChomStateHandlersB[0] = kaldachom_stateHandlerB00;
    gKaldaChomStateHandlersB[1] = kaldachom_stateHandlerB01;
    gKaldaChomStateHandlersB[2] = kaldachom_stateHandlerB02;
    gKaldaChomStateHandlersB[3] = kaldachom_stateHandlerB03;
    gKaldaChomStateHandlersB[4] = kaldachom_stateHandlerB04;
    gKaldaChomStateHandlersB[5] = kaldachom_stateHandlerB05;
}

ObjectDescriptor12 gKaldaChomObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_12_SLOTS,
    (ObjectDescriptorCallback)kaldachom_initialise,
    (ObjectDescriptorCallback)kaldachom_release,
    0,
    (ObjectDescriptorCallback)kaldachom_init,
    (ObjectDescriptorCallback)kaldachom_update,
    (ObjectDescriptorCallback)kaldachom_hitDetect,
    (ObjectDescriptorCallback)kaldachom_render,
    (ObjectDescriptorCallback)kaldachom_free,
    (ObjectDescriptorCallback)kaldachom_getObjectTypeId,
    kaldachom_getExtraSize,
    (ObjectDescriptorCallback)kaldachom_setScale,
    (ObjectDescriptorCallback)kaldachom_func0B,
};
