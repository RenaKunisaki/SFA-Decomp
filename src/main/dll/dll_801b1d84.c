/* DIM wood door falling debris updater [801B13E8-801B13F0) */

#include "game/objects/object.h"
#include "main/objfx.h"
#include "sys/objects/lifecycle.h"
#include "main/gamebits.h"
#include "main/objhits.h"
#include "main/frame_timing.h"
#include "main/gamebit_ids.h"
#include "main/dll/dll_801b1d84.h"


enum DIMwooddoorDebrisState
{
    DIMWOODDOOR_DEBRIS_STATE_FALLING = 0, /* fall under gravity, spin, watch for impact */
    DIMWOODDOOR_DEBRIS_STATE_EXPLODED = 1 /* exploded + hidden, awaiting despawn timer   */
};

typedef struct DIMwooddoorUpdateFallingDebrisState
{
    u8 pad0[0x1 - 0x0];
    u8 unk1;
    s16 unk2;
    u8 pad4[0x5 - 0x4];
    u8 hitboxRadius;
    u8 hitVolumeSlot;
    u8 unk7;
    u8 state;
    s8 rotZRate;
    s8 rotYRate;
    s8 rotXRate;
    u8 padC[0x10 - 0xC];
} DIMwooddoorUpdateFallingDebrisState;

#define DLL801B1D84_HIT_VOLUME_SLOT 5

extern f32 lbl_803E48A0;
extern f32 lbl_803E48A4;
extern f32 lbl_803E48A8;
extern f32 lbl_803DBEF0;


/* DIMwooddoor_updateFallingDebris: integrate the falling debris under gravity, spin it, and on
 * contact (or scripted trigger) fire the explosion and start the despawn timer. */

void DIMwooddoor_updateFallingDebris(GameObject* obj)
{
    DIMwooddoorUpdateFallingDebrisState* extra = obj->extra;
    switch (extra->state)
    {
    case DIMWOODDOOR_DEBRIS_STATE_FALLING:
    {
        f32 oldvy = obj->anim.velocityY;
        f32 grav = lbl_803E48A4 * -lbl_803DBEF0;
        f32 midVel;
        ObjHitsPriorityState* hitState;
        obj->anim.velocityY = grav * timeDelta + oldvy;
        midVel = lbl_803E48A8 * (oldvy + obj->anim.velocityY);
        objMove(obj, obj->anim.velocityX * timeDelta, midVel * timeDelta,
                obj->anim.velocityZ * timeDelta);
        obj->anim.rotZ =
            obj->anim.rotZ + extra->rotZRate * 10;
        obj->anim.rotY =
            obj->anim.rotY + extra->rotYRate * 10;
        obj->anim.rotX = obj->anim.rotX + extra->rotXRate * 10;
        hitState = (ObjHitsPriorityState*)obj->anim.hitReactState;
        if (hitState != NULL)
        {
            int* vol;
            ObjHits_SetHitVolumeSlot(&obj->anim, DLL801B1D84_HIT_VOLUME_SLOT,
                                     extra->hitVolumeSlot, 0);
            vol = (int*)hitState->lastHitObject;
            if (vol != NULL && vol != *(int**)extra)
            {
                ObjHitbox_SetSphereRadius(&obj->anim,
                                          extra->hitboxRadius);
                spawnExplosion(obj, lbl_803E48A0, 2, 1, 0, 1, 1, 1, 0);
                obj->userData1 = 1180;
                *(s8*)&extra->state = DIMWOODDOOR_DEBRIS_STATE_EXPLODED;
                obj->anim.flags |= OBJANIM_FLAG_HIDDEN;
            }
        }
        if ((mainGetBit(GAMEBIT_DIM2_CannonRelated085E) != 0 && mainGetBit(GAMEBIT_CannonRelated0C2D) == 0) ||
            (mainGetBit(GAMEBIT_DIM2_CannonRelated0874) != 0 && mainGetBit(GAMEBIT_CannonRelated0C2E) == 0))
        {
            obj->userData1 = 1200;
        }
        if (((ObjHitsPriorityState*)obj->anim.hitReactState)->contactFlags != 0)
        {
            ObjHitbox_SetSphereRadius(&obj->anim,
                                      extra->hitboxRadius);
            spawnExplosion(obj, lbl_803E48A0, 2, 1, 0, 1, 1, 1, 0);
            obj->userData1 = 1180;
            *(s8*)&extra->state = DIMWOODDOOR_DEBRIS_STATE_EXPLODED;
            obj->anim.flags |= OBJANIM_FLAG_HIDDEN;
        }
        break;
    }
    case DIMWOODDOOR_DEBRIS_STATE_EXPLODED:
        break;
    }
    obj->userData1 = obj->userData1 + framesThisStep;
    if (obj->userData1 > 1200)
    {
        Obj_FreeObject(obj);
    }
    else if (extra->unk7 != 0)
    {
        *(s8*)&extra->unk7 = 0;
    }
}
