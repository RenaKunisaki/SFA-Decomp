/*
 * NW_geyser (DLL 0x1A0) - the erupting geyser of SnowHorn Wastes (map
 * 'nwastes', 0x0A).
 *
 * The geyser plays a pair of looped object sounds and continuously runs
 * its trigger sequence; once GAMEBIT_GEYSER_OFF is set it hides, drops
 * its sounds and collision, and reports completion (GameBit 0x398). Its
 * SeqFn scrolls the geyser texture each frame.
 */
#include "main/mapEvent.h"
#include "game/objects/object.h"
#include "main/objprint_character_api.h"
#include "main/objhits.h"
#include "main/objseq.h"
#include "main/objtexture.h"
#include "main/dll/dll_01A0_nwgeyser.h"
#include "main/gamebits.h"
#include "main/audio/sfx.h"
#include "main/frame_timing.h"
#include "dlls/object_descriptor.h"

/* GameBit that erupts/retires the geyser (hides it, drops its sounds). */
#define GAMEBIT_GEYSER_OFF 0xa

/* looped object sounds played while the geyser is active */
#define SFX_GEYSER_LOOP_A 0x372
#define SFX_GEYSER_LOOP_B 0x373

#define NWGEYSER_OBJFLAG_HIDDEN             0x4000
#define NWGEYSER_OBJFLAG_HITDETECT_DISABLED 0x2000
#define NWGEYSER_OBJFLAG_UPDATE_DISABLED    0x8000

typedef struct NwGeyserTextureScrollParams
{
    f32 unitsPerSecond;
    f32 initialOffset;
} NwGeyserTextureScrollParams;

ObjectDescriptor gNW_geyserObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    0,
    0,
    0,
    (ObjectDescriptorCallback)nw_geyser_init,
    (ObjectDescriptorCallback)nw_geyser_update,
    0,
    0,
    (ObjectDescriptorCallback)nw_geyser_free,
    0,
    0,
};

const NwGeyserTextureScrollParams gNwGeyserTextureScrollParams = {512.0f, 0.0f};

int NW_geyser_SeqFn(GameObject* obj, int unused, ObjAnimUpdateState* animUpdate)
{
    ObjTextureRuntimeSlot* tex0;
    u8* animUpdateBytes;

    animUpdateBytes = (u8*)animUpdate;
    if (mainGetBit(GAMEBIT_GEYSER_OFF) != 0)
    {
        animUpdateBytes[0x90] = (u8)(animUpdateBytes[0x90] | 4);
    }
    tex0 = objFindTexture(obj, 0, 0);
    objFindTexture(obj, 1, 0);
    tex0->offsetT =
        (s16)(tex0->offsetT + (s32)(gNwGeyserTextureScrollParams.unitsPerSecond * timeDelta));
    if (tex0->offsetT > 0x4e80)
    {
        tex0->offsetT -= 0x4e80;
    }
    animUpdate->hitVolumePair = (s16)(animUpdate->activeHitVolumePair & ~0x40);
    animUpdate->sequenceEventActive = 0;
    return 0;
}

void nw_geyser_free(GameObject* obj)
{
    (*gMapEventInterface)->setObjGroupStatus(obj->anim.mapEventSlot, 0x1f, 0);
}

void nw_geyser_update(GameObject* obj)
{
    if (mainGetBit(GAMEBIT_GEYSER_OFF) != 0)
    {
        (obj)->anim.flags = OBJANIM_FLAG_HIDDEN;
        (obj)->objectFlags = (u16)((obj)->objectFlags | NWGEYSER_OBJFLAG_UPDATE_DISABLED);
        Sfx_RemoveLoopedObjectSound((int)obj, SFX_GEYSER_LOOP_A);
        Sfx_RemoveLoopedObjectSound((int)obj, SFX_GEYSER_LOOP_B);
        ObjHits_DisableObject(obj);
        mainSetBits(0x398, 1);
    }
    else
    {
        Sfx_AddLoopedObjectSound((int)obj, SFX_GEYSER_LOOP_A);
        Sfx_AddLoopedObjectSound((int)obj, SFX_GEYSER_LOOP_B);
        (*gObjectTriggerInterface)->runSequence(0, (void*)obj, -1);
        ObjHits_EnableObject(obj);
    }
}

void nw_geyser_init(GameObject* obj)
{
    obj->objectFlags = (u16)(obj->objectFlags | (NWGEYSER_OBJFLAG_HIDDEN | NWGEYSER_OBJFLAG_HITDETECT_DISABLED));
    obj->animEventCallback = NW_geyser_SeqFn;
}
