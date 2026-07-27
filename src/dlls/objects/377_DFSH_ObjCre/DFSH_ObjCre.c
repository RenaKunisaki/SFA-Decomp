/*
 * DFSH_ObjCre (DLL 0x179) - shrine SharpClaw encounter spawner.
 *
 * An indexed game bit triggers the activation effects and starts a
 * countdown. Once the countdown expires, this object creates object ID
 * 0x11, which retail OBJINDEX.bin maps to "sharpclawGr" (DLL 0xC9).
 */

#include "dlls/objects/377_DFSH_ObjCre.h"

#include "game/objects/object.h"
#include "main/audio/sfx.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/dll/foodbag.h"
#include "main/frame_timing.h"
#include "main/gamebits_api.h"
#include "main/object_render.h"
#include "main/resource.h"
#include "sys/objects.h"
#include "sys/objects/lifecycle.h"

typedef struct DFSHObjCreatorSharpClawSetup {
    ObjPlacement base;
    s16 gameBit;
    s16 gameBit2;
    s16 unknown1C;
    s16 unknown1E;
    s16 unknown20;
    s16 droppedItemId;
    s16 unknown24;
    u8 unknown26;
    s8 initialWeaponId;
    u8 objectFlagBits;
    u8 aggroRangeByte;
    s8 initialYaw;
    u8 flags;
    s16 respawnEnabled;
    s8 triggerSequenceId;
    u8 healthByte;
    s16 unknown30;
    u8 hitPoints;
    u8 unknown33;
    u16 unknown34;
    u8 unknown36[0x38 - 0x36];
} DFSHObjCreatorSharpClawSetup;

STATIC_ASSERT(sizeof(DFSHObjCreatorSharpClawSetup) == 0x38);
STATIC_ASSERT(offsetof(DFSHObjCreatorSharpClawSetup, base) == 0x00);
STATIC_ASSERT(offsetof(DFSHObjCreatorSharpClawSetup, gameBit) == 0x18);
STATIC_ASSERT(offsetof(DFSHObjCreatorSharpClawSetup, gameBit2) == 0x1A);
STATIC_ASSERT(offsetof(DFSHObjCreatorSharpClawSetup, unknown1C) == 0x1C);
STATIC_ASSERT(offsetof(DFSHObjCreatorSharpClawSetup, unknown1E) == 0x1E);
STATIC_ASSERT(offsetof(DFSHObjCreatorSharpClawSetup, unknown20) == 0x20);
STATIC_ASSERT(offsetof(DFSHObjCreatorSharpClawSetup, droppedItemId) == 0x22);
STATIC_ASSERT(offsetof(DFSHObjCreatorSharpClawSetup, unknown24) == 0x24);
STATIC_ASSERT(offsetof(DFSHObjCreatorSharpClawSetup, unknown26) == 0x26);
STATIC_ASSERT(offsetof(DFSHObjCreatorSharpClawSetup, initialWeaponId) == 0x27);
STATIC_ASSERT(offsetof(DFSHObjCreatorSharpClawSetup, objectFlagBits) == 0x28);
STATIC_ASSERT(offsetof(DFSHObjCreatorSharpClawSetup, aggroRangeByte) == 0x29);
STATIC_ASSERT(offsetof(DFSHObjCreatorSharpClawSetup, initialYaw) == 0x2A);
STATIC_ASSERT(offsetof(DFSHObjCreatorSharpClawSetup, flags) == 0x2B);
STATIC_ASSERT(offsetof(DFSHObjCreatorSharpClawSetup, respawnEnabled) == 0x2C);
STATIC_ASSERT(offsetof(DFSHObjCreatorSharpClawSetup, triggerSequenceId) == 0x2E);
STATIC_ASSERT(offsetof(DFSHObjCreatorSharpClawSetup, healthByte) == 0x2F);
STATIC_ASSERT(offsetof(DFSHObjCreatorSharpClawSetup, unknown30) == 0x30);
STATIC_ASSERT(offsetof(DFSHObjCreatorSharpClawSetup, hitPoints) == 0x32);
STATIC_ASSERT(offsetof(DFSHObjCreatorSharpClawSetup, unknown33) == 0x33);
STATIC_ASSERT(offsetof(DFSHObjCreatorSharpClawSetup, unknown34) == 0x34);
STATIC_ASSERT(offsetof(DFSHObjCreatorSharpClawSetup, unknown36) == 0x36);

#define DFSH_OBJ_CREATOR_SHARPCLAW_OBJECT_ID       0x11
#define DFSH_OBJ_CREATOR_EFFECT_RESOURCE_ID        0x82
#define DFSH_OBJ_CREATOR_DISABLE_GAME_BIT          0x589
#define DFSH_OBJ_CREATOR_TRIGGER_GAME_BIT_BASE     0xF6
#define DFSH_OBJ_CREATOR_DROPPED_ITEM_GAME_BIT     0xFC
#define DFSH_OBJ_CREATOR_SHARPCLAW_GAME_BIT        0x1E7
#define DFSH_OBJ_CREATOR_SHARPCLAW_DROPPED_ITEM_ID 0x49
#define DFSH_OBJ_CREATOR_SHARPCLAW_INITIAL_WEAPON  3
#define DFSH_OBJ_CREATOR_SHARPCLAW_FLAGS           2
#define DFSH_OBJ_CREATOR_SPAWN_TIMER               100

int dfshObjCreator_getExtraSize(void) {
    return sizeof(DFSHObjCreatorState);
}

int dfshObjCreator_getObjectTypeId(void) {
    return 0;
}

void dfshObjCreator_free(void) {
}

void dfshObjCreator_render(GameObject* obj, int renderArg2, int renderArg3, int renderArg4, int renderArg5,
                           s8 visible) {
    s32 isVisible;

    isVisible = visible;
    if (isVisible != 0) {
        objRenderModelAndHitVolumes(obj, renderArg2, renderArg3, renderArg4, renderArg5, 1.0f);
    }
}

void dfshObjCreator_hitDetect(void) {
}

void dfshObjCreator_update(GameObject* obj) {
    const DFSHObjCreatorPlacement* placement;
    DFSHObjCreatorState* state;
    Dll82Interface** effectResource;
    DFSHObjCreatorSharpClawSetup* sharpClawSetup;

    placement = (const DFSHObjCreatorPlacement*)obj->anim.placementData;
    state = obj->extra;
    if (mainGetBit(DFSH_OBJ_CREATOR_DISABLE_GAME_BIT) != 0) {
        obj->userData2 = 0;
        return;
    }

    if (obj->userData2 == 0 &&
        mainGetBit(placement->triggerGameBitOffset + DFSH_OBJ_CREATOR_TRIGGER_GAME_BIT_BASE) != 0) {
        effectResource = Resource_Acquire(DFSH_OBJ_CREATOR_EFFECT_RESOURCE_ID, 1);
        (*effectResource)->spawn(obj, 0, NULL, 1, -1, NULL);
        (*effectResource)->spawn(obj, 1, NULL, 1, -1, NULL);
        Sfx_PlayFromObject((int)obj, SFXTRIG_hitpos_6);
        Resource_Release(effectResource);
        state->spawnTimerRate = 1;
        obj->userData2 = 1;
    }

    if (state->spawnTimerRate != 0) {
        state->spawnTimer = (s16)(state->spawnTimer - state->spawnTimerRate * (int)timeDelta);
    }

    if (Obj_IsLoadingLocked() != 0 && state->spawnTimer <= 0) {
        sharpClawSetup = (DFSHObjCreatorSharpClawSetup*)Obj_AllocObjectSetup(sizeof(DFSHObjCreatorSharpClawSetup),
                                                                             DFSH_OBJ_CREATOR_SHARPCLAW_OBJECT_ID);
        sharpClawSetup->base.posX = placement->base.posX;
        sharpClawSetup->base.posY = placement->base.posY;
        sharpClawSetup->base.posZ = placement->base.posZ;
        sharpClawSetup->base.mapId = placement->base.mapId;
        sharpClawSetup->base.color[0] = placement->base.color[0];
        sharpClawSetup->base.color[1] = placement->base.color[1];
        sharpClawSetup->base.color[2] = placement->base.color[2];
        sharpClawSetup->base.color[3] = placement->base.color[3];
        sharpClawSetup->initialWeaponId = DFSH_OBJ_CREATOR_SHARPCLAW_INITIAL_WEAPON;
        sharpClawSetup->gameBit = DFSH_OBJ_CREATOR_SHARPCLAW_GAME_BIT;
        sharpClawSetup->unknown30 = -1;
        sharpClawSetup->gameBit2 = -1;
        sharpClawSetup->unknown1C = -1;
        sharpClawSetup->initialYaw = (s8)(obj->anim.rotX >> 8);
        sharpClawSetup->flags = DFSH_OBJ_CREATOR_SHARPCLAW_FLAGS;
        if (mainGetBit(DFSH_OBJ_CREATOR_DROPPED_ITEM_GAME_BIT) != 0) {
            sharpClawSetup->droppedItemId = DFSH_OBJ_CREATOR_SHARPCLAW_DROPPED_ITEM_ID;
        } else {
            sharpClawSetup->droppedItemId = -1;
        }
        sharpClawSetup->aggroRangeByte = 0xFF;
        sharpClawSetup->triggerSequenceId = -1;
        sharpClawSetup->unknown34 = 0xFFFF;
        Obj_SetupObject(&sharpClawSetup->base, 5, obj->anim.mapEventSlot, -1, obj->anim.parent);
        state->spawnTimer = DFSH_OBJ_CREATOR_SPAWN_TIMER;
        state->spawnTimerRate = 0;
    }
}

void dfshObjCreator_init(GameObject* obj, const DFSHObjCreatorPlacement* placement) {
    DFSHObjCreatorState* state;

    state = obj->extra;
    obj->anim.rotX = (s16)((s32)placement->initialYaw << 8);
    obj->userData2 = 0;
    state->spawnTimer = DFSH_OBJ_CREATOR_SPAWN_TIMER;
    state->spawnTimerRate = 0;
    obj->anim.renderAlpha = 0xFF;
    obj->anim.alpha = 0xFF;
}

void dfshObjCreator_release(void) {
}

void dfshObjCreator_initialise(void) {
}

ObjectDescriptor gDFSHObjCreatorObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)dfshObjCreator_initialise,
    (ObjectDescriptorCallback)dfshObjCreator_release,
    0,
    (ObjectDescriptorCallback)dfshObjCreator_init,
    (ObjectDescriptorCallback)dfshObjCreator_update,
    (ObjectDescriptorCallback)dfshObjCreator_hitDetect,
    (ObjectDescriptorCallback)dfshObjCreator_render,
    (ObjectDescriptorCallback)dfshObjCreator_free,
    (ObjectDescriptorCallback)dfshObjCreator_getObjectTypeId,
    dfshObjCreator_getExtraSize,
};
