#ifndef MAIN_DLL_DLL_0117_APPLEONTREE_INTERNAL_H_
#define MAIN_DLL_DLL_0117_APPLEONTREE_INTERNAL_H_

#include "game/objects/object.h"
#include "game/objects/object_setup.h"
#include "sys/objects/lifecycle.h"
#include "main/objhits.h"
#include "main/dll/dll_0117_appleontree.h"

typedef struct AppleontreeObjectDef
{
    ObjPlacement head; /* 0x00 */
    u32 unk18;
    u16 duration;
    u16 elapsed;
    u8 stage0Frac;
    u8 stage1Frac;
    u8 stage2Frac;
    u8 stage3Frac;
    u8 unk24;
    s8 unk25;
    s16 gameBit;
} AppleontreeObjectDef;

/* AppleOnTree_update animState machine: an apple's lifecycle from hanging on
 * the tree through falling, resting, being knocked loose, and despawning. */
#define APPLEONTREE_STATE_GROWING 0 /* unripe, hanging; scales up toward ripe */
#define APPLEONTREE_STATE_RIPE    1 /* ripe, swaying; ready to drop */
#define APPLEONTREE_STATE_FALLING 2 /* dropping from branch to ground */
#define APPLEONTREE_STATE_LANDED  3 /* settled on the ground, collectable */
#define APPLEONTREE_STATE_KNOCKED 4 /* knocked loose, bouncing/rolling physics */
#define APPLEONTREE_STATE_BURST   5 /* fx-burst despawn (no fade) */
#define APPLEONTREE_STATE_FADEOUT 6 /* alpha fade-out despawn */

/* burst-splat particle spawned 8x when the apple enters APPLEONTREE_STATE_BURST */
#define APPLEONTREE_PARTFX_BURST 0x55a

#define APPLEONTREE_MSG_IN_RANGE 0x7000a /* sent to player when grab is offered */
#define APPLEONTREE_MSG_PICKUP   0x7000b /* player collected: restore health + burst */

extern f32 lbl_803E37C8;
extern f32 gAppleOnTreePickupXZRange;
extern f32 gAppleOnTreePickupRange;
extern f32 lbl_803E37D4;
extern f32 lbl_803E37D8;
extern f32 lbl_803E37DC;
extern f32 lbl_803E37E0;
extern f32 lbl_803E37E4;
extern f32 lbl_803E37E8;
extern f32 lbl_803E37F4;
extern f32 lbl_803E37F8;
extern f32 lbl_803E37FC;
extern f32 lbl_803E3800;
extern const f32 lbl_803E3828;
extern f32 lbl_803E382C;
extern f32 lbl_803E3830;
extern f32 lbl_803E3834;
extern f32 lbl_803E3838;
extern f32 lbl_803E37CC;
extern f32 lbl_803E37D0;
extern f32 lbl_803E3804;
extern f32 lbl_803E3808;
extern f32 lbl_803E380C;
extern f32 lbl_803E3810;
extern f32 lbl_803E3814;
extern f32 lbl_803E3818;

void appleontree_knockLoose(GameObject* obj, int msg);
void appleontree_handleCollectableHit(GameObject* obj);

static inline void appleontree_markFallen(GameObject* obj)
{
    int state = *(int*)&(obj)->extra;
    if (((obj)->anim.flags & OBJANIM_FLAG_OWNS_PLACEMENT_DATA) != 0)
    {
        Obj_FreeObject(obj);
    }
    else
    {
        if ((obj)->anim.hitReactState != NULL)
        {
            ObjHits_DisableObject(obj);
        }
        ((AppleOnTreeState*)state)->flags = (u8)(((AppleOnTreeState*)state)->flags | 2);
    }
}

#endif /* MAIN_DLL_DLL_0117_APPLEONTREE_INTERNAL_H_ */
