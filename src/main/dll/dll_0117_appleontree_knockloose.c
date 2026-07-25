/* DLL 0x0117 - appleontree / groundAnimator group. TU: 0x8017D854-0x8017DAF0. */
#include "main/dll/partfx_interface.h"
#include "main/audio/sfx_ids.h"
#include "main/vecmath_distance_api.h"
#include "main/audio/sfx_play_api.h"
#include "main/objfx.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/vecmath.h"
#include "main/dll/groundAnimator.h"
#include "main/dll_000A_expgfx.h"
#include "main/dll/waterfx_interface.h"
#include "game/objects/object.h"
#include "game/objects/object_setup.h"
#include "main/obj_message.h"
#include "sys/objects.h"
#include "sys/objects/lifecycle.h"
#include "main/objseq.h"
#include "main/objtexture.h"
#include "main/dll/baddie_state.h"
#include "main/sky_interface.h"
#include "main/gamebits.h"
#include "main/frame_timing.h"
#include "main/track_dolphin_api.h"
#include "main/objhits.h"
#include "main/dll/dll_00FC_babycloudrunner.h"
#include "main/dll/dll_0117_appleontree.h"
#include "main/dll/player_api.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/object_render.h"
#include "main/dll/dll_0117_appleontree_internal.h"

void appleontree_knockLoose(GameObject* obj, int msg)
{
    int state = *(int*)&obj->extra;
    int healthRestore;

    switch (msg)
    {
    case 0:
        healthRestore = 2;
        break;
    case 1:
        healthRestore = 2;
        break;
    case 2:
        healthRestore = 2;
        break;
    default:
        healthRestore = 0;
        break;
    }
    ((AppleOnTreeState*)state)->healthRestore = healthRestore;
    ((AppleOnTreeState*)state)->animState = APPLEONTREE_STATE_KNOCKED;
    ((AppleOnTreeState*)state)->elapsedTime = timeDelta;
    ((AppleOnTreeState*)state)->flightTime = timeDelta;
    ((AppleOnTreeState*)state)->rotX = randomGetRange(-0x8000, 0x7fff);
    ((AppleOnTreeState*)state)->rotY = randomGetRange(-0x8000, 0x7fff);
    ((AppleOnTreeState*)state)->rotZ = 0x2000;

    if (fn_80065684(obj, obj->anim.localPosX, obj->anim.localPosY,
                    obj->anim.localPosZ, (f32*)(state + 0x30), 0) == 0)
    {
        appleontree_markFallen(obj);
    }
    else
    {
        f32 m = ((AppleOnTreeState*)state)->gravity;
        f32 g = lbl_803E37D8 * m;
        f32 q = sqrtf(-(g * ((AppleOnTreeState*)state)->dropHeight - lbl_803E37D4));
        f32 t = lbl_803E37DC * m;
        f32 r;

        if (t >= lbl_803E37D4)
        {
            r = t;
        }
        else
        {
            r = -t;
        }
        if (r <= lbl_803E37E0)
        {
            r = lbl_803E37C8;
        }
        else
        {
            f32 r2;
            r = (lbl_803E37E4 - q) / t;
            r2 = (lbl_803E37E4 + q) / t;
            r = (r > *(f32*)&lbl_803E37D4) ? r : r2;
        }
        ((AppleOnTreeState*)state)->totalFlightTime = r;

        if (((AppleOnTreeState*)state)->velY < lbl_803E37D4)
        {
            ((AppleOnTreeState*)state)->dropHeight =
                -(lbl_803E37D8 * ((AppleOnTreeState*)state)->fallScale - ((AppleOnTreeState*)state)->dropHeight);
        }
        else
        {
            ((AppleOnTreeState*)state)->dropHeight = lbl_803E37E8 * (lbl_803E37D8 * ((AppleOnTreeState*)state)->fallScale) +
                                                     ((AppleOnTreeState*)state)->dropHeight;
        }

        if (((AppleOnTreeState*)state)->dropHeight <= lbl_803E37D4)
        {
            state = *(int*)&obj->extra;
            if ((obj->anim.flags & OBJANIM_FLAG_OWNS_PLACEMENT_DATA) != 0)
            {
                Obj_FreeObject(obj);
            }
            else
            {
                if (obj->anim.hitReactState != NULL)
                {
                    ObjHits_DisableObject(obj);
                }
                ((AppleOnTreeState*)state)->flags = (u8)(((AppleOnTreeState*)state)->flags | 2);
            }
        }
        else
        {
            ((AppleOnTreeState*)state)->posY = obj->anim.localPosY;
            ((AppleOnTreeState*)state)->splashPosY =
                obj->anim.localPosY - ((AppleOnTreeState*)state)->dropHeight;
            if (obj->anim.hitReactState != NULL)
            {
                ObjHits_DisableObject(obj);
            }
            Sfx_PlayFromObject((int)obj, SFXTRIG_en_tranch_6);
        }
    }
}

