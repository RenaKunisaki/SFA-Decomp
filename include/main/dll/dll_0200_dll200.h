#ifndef MAIN_DLL_DLL_0200_DLL200_H_
#define MAIN_DLL_DLL_0200_DLL200_H_

#include "game/objects/object.h"
#include "game/objects/object_setup.h"
#include "global.h"
#include "main/objanim_update.h"

typedef struct Dll200Placement
{
    ObjPlacement head;
    s8 rotXByte;
} Dll200Placement;

/* Set of 3 item ids copied from a placement's item-set table and passed
 * (as an s32[3]) to isOneOfItemsBeingUsed to test the player's held item. */
typedef struct ItemIdSet3
{
    int itemId0;
    int itemId1;
    int itemId2;
} ItemIdSet3;

STATIC_ASSERT(sizeof(ItemIdSet3) == 0xC);

extern const ItemIdSet3 gArwingAttachmentItemSetWander;
extern const ItemIdSet3 gArwingAttachmentItemSetIdle;

typedef struct ArwAttachTarget
{
    f32 x;
    f32 y;
    f32 moveId;
    f32 altMoveId;
    f32 speed;
} ArwAttachTarget;

void dll_200_updateAct6Idle(GameObject* obj);
void dll_200_updateAct1Interact(GameObject* obj);
void dll_200_free_nop(void);
void dll_200_hitDetect_nop(void);
void dll_200_release_nop(void);
void dll_200_initialise_nop(void);
int dll_200_getExtraSize_ret_40(void);
int dll_200_getObjectTypeId(void);
void dll_200_render(GameObject* obj, int p1, int p2, int p3, int p4, s8 visible);
void dll_200_init(GameObject* obj, Dll200Placement* def);
int dll_200_SeqFn(GameObject* obj, int unused, ObjAnimUpdateState* animUpdate, int arg3);
int dll_200_unlockFireBlasterSpell(GameObject* obj, int unused, ObjAnimUpdateState* animUpdate, int arg3);
void dll_200_update(int obj);
void dll_200_updateAct2Wander(GameObject* obj);

#endif /* MAIN_DLL_DLL_0200_DLL200_H_ */
