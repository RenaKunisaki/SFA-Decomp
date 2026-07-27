#include "dlls/objects/300_Transporter.h"

#include "game/objects/object.h"
#include "main/dll/partfx_interface.h"
#include "main/dll/tricky_api.h"
#include "main/frame_timing.h"
#include "main/gamebits.h"
#include "main/objfx.h"
#include "main/objseq.h"
#include "main/obj_trigger.h"
#include "main/vecmath.h"
#include "main/vecmath_distance_api.h"
#include "sys/objects.h"

/* recurring shimmer emitted randomly across all warp-pulse stages */
#define WARPPAD_PARTFX_PULSE 0x7CA
/* surge burst emitted at the stage-2 transition and the stage-3 latch release */
#define WARPPAD_PARTFX_SURGE 0x7D2

#define WARP_PAD_A_BUTTON_ICON         0x1B
#define WARP_PAD_INTERACTION_DISTANCE  40.0f
#define WARP_PAD_PROXIMITY_DISTANCE_SQ 409600.0f
#define WARP_PAD_PULSE_RANDOM_LIMIT    0x1E0
#define WARP_PAD_PULSE_STAGE1_TIME     120.0f
#define WARP_PAD_PULSE_STAGE2_TIME     360.0f
#define WARP_PAD_PULSE_STAGE2_DURATION 240.0f
#define WARP_PAD_PULSE_STAGE3_TIME     420.0f
#define WARP_PAD_PULSE_END_TIME        480.0f
#define WARP_PAD_SURGE_SCALE_GROWTH    0.0009f
#define WARP_PAD_SURGE_SCALE_RELEASE   0.00036f

typedef enum TransporterSequenceId {
    TRANSPORTER_SEQUENCE_OUTBOUND = 0,
    TRANSPORTER_SEQUENCE_INBOUND = 1,
    TRANSPORTER_SEQUENCE_TUTORIAL = 2,
} TransporterSequenceId;

typedef struct TransporterEffectParams {
    s16 unk00;
    s16 mode;
    s16 effectId;
    s16 count;
    f32 scale;
    f32 pos[3];
} TransporterEffectParams;

STATIC_ASSERT(offsetof(TransporterEffectParams, unk00) == 0x00);
STATIC_ASSERT(offsetof(TransporterEffectParams, mode) == 0x02);
STATIC_ASSERT(offsetof(TransporterEffectParams, effectId) == 0x04);
STATIC_ASSERT(offsetof(TransporterEffectParams, count) == 0x06);
STATIC_ASSERT(offsetof(TransporterEffectParams, scale) == 0x08);
STATIC_ASSERT(offsetof(TransporterEffectParams, pos) == 0x0C);
STATIC_ASSERT(sizeof(TransporterEffectParams) == 0x18);

extern u8 lbl_803DCDE0;
extern s16 lbl_803DCEB8;

const f32 gTransporterZero[1] = {0.0f};

void Transporter_updateEffects(GameObject* obj) {
    TransporterState* state;
    GameObject* player;
    TransporterEffectParams fx;
    u8 flags;
    u8 i;

    state = obj->extra;
    player = Obj_GetPlayerObject();
    fx.pos[0] = gTransporterZero[0];
    fx.pos[1] = 55.0f;
    fx.pos[2] = gTransporterZero[0];
    flags = state->flags;

    if ((flags & TRANSPORTER_FLAG_WARP_A) != 0) {
        if ((flags & TRANSPORTER_FLAG_WARP_B) != 0) {
            fx.effectId = 0xC0E;
            fx.mode = 1;
        } else if ((flags & TRANSPORTER_FLAG_WARP_C) != 0) {
            fx.effectId = 0xC7E;
            fx.mode = 2;
        } else {
            fx.effectId = 0xC13;
            fx.mode = 0;
        }
    } else if ((flags & TRANSPORTER_FLAG_WARP_B) != 0) {
        if (vec3f_distanceSquared(&obj->anim.worldPosX, &player->anim.worldPosX) < WARP_PAD_PROXIMITY_DISTANCE_SQ) {
            if (((state->flags & (TRANSPORTER_FLAG_DISABLED | TRANSPORTER_FLAG_ENABLE_GAMEBIT_OFF)) != 0) &&
                (state->countdownActive == 0)) {
                objfx_spawnArcedBurst(obj, 1, 0.75f, 2, 7, 100, 30.0f, 30.0f, 110.0f, &fx, 0);
            } else {
                objfx_spawnArcedBurst(obj, 1, 0.5f, 1, 6, 100, 30.0f, 30.0f, 110.0f, &fx, 0);
            }
        }
        fx.effectId = 0xC0E;
        fx.mode = 1;
    } else if ((flags & TRANSPORTER_FLAG_WARP_C) != 0) {
        if (vec3f_distanceSquared(&obj->anim.worldPosX, &player->anim.worldPosX) < WARP_PAD_PROXIMITY_DISTANCE_SQ) {
            if (((state->flags & (TRANSPORTER_FLAG_DISABLED | TRANSPORTER_FLAG_ENABLE_GAMEBIT_OFF)) != 0) &&
                (state->countdownActive == 0)) {
                objfx_spawnArcedBurst(obj, 1, 0.75f, 2, 7, 100, 30.0f, 30.0f, 110.0f, &fx, 0);
            } else {
                objfx_spawnArcedBurst(obj, 1, 0.5f, 5, 6, 100, 30.0f, 30.0f, 110.0f, &fx, 0);
            }
        }
        fx.effectId = 0xC7E;
        fx.mode = 2;
    } else {
        if (vec3f_distanceSquared(&obj->anim.worldPosX, &player->anim.worldPosX) < WARP_PAD_PROXIMITY_DISTANCE_SQ) {
            if (((state->flags & (TRANSPORTER_FLAG_DISABLED | TRANSPORTER_FLAG_ENABLE_GAMEBIT_OFF)) != 0) &&
                (state->countdownActive == 0)) {
                objfx_spawnArcedBurst(obj, 1, 0.75f, 2, 7, 100, 30.0f, 30.0f, 110.0f, &fx, 0);
            } else {
                objfx_spawnArcedBurst(obj, 1, 0.5f, 3, 6, 100, 30.0f, 30.0f, 110.0f, &fx, 0);
            }
        }
        fx.effectId = 0xC13;
        fx.mode = 0;
    }

    if ((state->flags & TRANSPORTER_FLAG_PULSE_FX) != 0) {
        if (state->pulseTimer < WARP_PAD_PULSE_STAGE1_TIME) {
            if ((f32)(s32)randomGetRange(0, WARP_PAD_PULSE_RANDOM_LIMIT) < state->pulseTimer / 2.0f) {
                (*gPartfxInterface)->spawnObject((void*)obj, WARPPAD_PARTFX_PULSE, &fx, 2, -1, NULL);
            }
        } else if (state->pulseTimer < WARP_PAD_PULSE_STAGE2_TIME) {
            if ((f32)(s32)randomGetRange(0, WARP_PAD_PULSE_RANDOM_LIMIT) < state->pulseTimer / 3.0f) {
                (*gPartfxInterface)->spawnObject((void*)obj, WARPPAD_PARTFX_PULSE, &fx, 2, -1, NULL);
            }
            fx.count = 0x28;
            fx.unk00 = 0;
            fx.scale = WARP_PAD_SURGE_SCALE_GROWTH *
                       ((state->pulseTimer - WARP_PAD_PULSE_STAGE1_TIME) / WARP_PAD_PULSE_STAGE2_DURATION);
            (*gPartfxInterface)->spawnObject((void*)obj, WARPPAD_PARTFX_SURGE, &fx, 2, -1, NULL);
            state->flags |= TRANSPORTER_FLAG_PULSE_LATCH;
        } else if (state->pulseTimer < WARP_PAD_PULSE_STAGE3_TIME) {
            if ((f32)(s32)randomGetRange(0, WARP_PAD_PULSE_RANDOM_LIMIT) < state->pulseTimer / 2.0f) {
                (*gPartfxInterface)->spawnObject((void*)obj, WARPPAD_PARTFX_PULSE, &fx, 2, -1, NULL);
            }
            if ((state->flags & TRANSPORTER_FLAG_PULSE_LATCH) != 0) {
                state->flags &= ~TRANSPORTER_FLAG_PULSE_LATCH;
                fx.count = 0x46;
                fx.scale = WARP_PAD_SURGE_SCALE_RELEASE;
                for (i = 0xF; i != 0; i--) {
                    (*gPartfxInterface)->spawnObject((void*)obj, WARPPAD_PARTFX_SURGE, &fx, 2, -1, NULL);
                }
            }
        } else if (!(state->pulseTimer < WARP_PAD_PULSE_END_TIME)) {
            state->pulseTimer = gTransporterZero[0];
            state->flags &= ~TRANSPORTER_FLAG_PULSE_FX;
        }
        state->pulseTimer += timeDelta;
    }
}

void Transporter_updateInteraction(GameObject* obj) {
    TransporterPlacement* placement;
    TransporterState* state;
    GameObject* player;
    s16 gameBit;

    placement = (TransporterPlacement*)obj->anim.placement;
    state = obj->extra;
    gameBit = placement->enableGameBit;
    if (gameBit != TRANSPORTER_GAME_BIT_NONE) {
        if (mainGetBit(gameBit) != 0) {
            state->flags &= ~TRANSPORTER_FLAG_ENABLE_GAMEBIT_OFF;
        } else {
            state->flags |= TRANSPORTER_FLAG_ENABLE_GAMEBIT_OFF;
        }
    }

    if ((obj->anim.resetHitboxFlags & INTERACT_FLAG_IN_RANGE) != 0) {
        setAButtonIcon(WARP_PAD_A_BUTTON_ICON);
        if (mainGetBit(GAMEBIT_SawWarpPad) == 0) {
            (*gObjectTriggerInterface)->runSequence(TRANSPORTER_SEQUENCE_TUTORIAL, (void*)obj, -1);
            mainSetBits(GAMEBIT_SawWarpPad, 1);
            return;
        }
    }

    player = Obj_GetPlayerObject();
    if (player == NULL) {
        return;
    }

    if ((state->triggerMode == TRANSPORTER_TRIGGER_PROXIMITY) && (state->countdownActive == 0) &&
        ((obj->objectFlags & OBJECT_OBJFLAG_PARENT_SLACK) == 0)) {
        if ((lbl_803DCEB8 > -1) && (Vec_xzDistance(&obj->anim.worldPosX, &Obj_GetPlayerObject()->anim.worldPosX) <
                                    WARP_PAD_INTERACTION_DISTANCE)) {
            (*gObjectTriggerInterface)->runSequence(TRANSPORTER_SEQUENCE_INBOUND, (void*)obj, -1);
            obj->userData1 = state->activateDelay;
            state->triggerMode = TRANSPORTER_TRIGGER_PROXIMITY;
            state->countdownActive = 1;
            lbl_803DCDE0 = 2;
        } else {
            gameBit = placement->enableGameBit;
            if (((gameBit == TRANSPORTER_GAME_BIT_NONE) ||
                 ((mainGetBit(gameBit) != 0) && ((obj->anim.resetHitboxFlags & INTERACT_FLAG_IN_RANGE) != 0))) &&
                (ObjTrigger_IsSet((int)obj) != 0)) {
                (*gObjectTriggerInterface)->runSequence(TRANSPORTER_SEQUENCE_OUTBOUND, (void*)obj, -1);
                obj->userData1 = state->activateDelay;
                state->triggerMode = TRANSPORTER_TRIGGER_INTERACTION;
                state->countdownActive = 1;
            }
        }
    }

    if (state->countdownActive != 0) {
        if (obj->userData1 > 0) {
            obj->userData1 -= framesThisStep;
        } else {
            obj->userData1 = 0;
            state->countdownActive = 0;
        }
    }
    state->cooldownTimer -= timeDelta;
    if (state->cooldownTimer <= gTransporterZero[0]) {
        state->cooldownTimer = gTransporterZero[0];
        state->unk0A = -1;
    }
}

const f32 gWarpPadRestZero[1] = {0.0f};
