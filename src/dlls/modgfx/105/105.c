/*
 * DLL 105 / 0x69 entry-point table.
 *
 * dll_69_func03 builds a stack list of GfxCmd records describing a layered
 * 2D/billboard effect and submits it through gModgfxInterface->spawnEffect.
 * The layout is read from the resource table at lbl_803137F8.
 *   - variant selects the effect group passed to spawnEffect (0xc11 for
 *     variant 2, else 0x5e0) and toggles command-list flag 0x40000.
 *   - overrideParams (a/b/c/d) override the default layer-1/4 colour words.
 *   - sourceObj scales entry 1 by the object's rootMotionScale and seeds
 *     entry 2's depth; posSource supplies the spawn position.
 * func00/func01 are the table's no-op slots.
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/partfx_interface.h"
#include "main/dll/modgfx_types.h"
#include "main/dll/dll_0069_dll69func0.h"

/* spawnEffect effect group per variant (docblock: "0xc11 for variant 2, else 0x5e0"). */
#define DLL69_EFFECT_ID_VARIANT2 0xc11
#define DLL69_EFFECT_ID_DEFAULT  0x5e0
#define DLL69_CMDLIST_FLAG_TOGGLE 0x40000

u32 lbl_803137F8[34] = {
    0xfc180000, 0x00000000, 0x00000000, 0x0000fc18, 0x00000000, 0x03e80000, 0x00000040,
    0x00000000, 0x000003e8, 0x00400000, 0xfc180fa0, 0x00000000, 0x00400000, 0x0fa0fc18,
    0x00000040, 0x03e80fa0, 0x00000040, 0x00400000, 0x0fa003e8, 0x00400040, 0x00000002,
    0x00060000, 0x00060004, 0x00010003, 0x00070001, 0x00070005, 0x00000001, 0x00020003,
    0x00040005, 0x00060007, 0x00000104, 0x001e0001, 0x01040000, 0x00000000,
};

s16 dll_69_func03(GameObject* sourceObj, int variant, void* posSource, u32 flags, int unused,
                  Dll69EffectParams* overrideParams)
{
    ModgfxPointerSpawnPacket buf;
    GfxCmd* e;
    GfxCmd* entries;
    u8* base = (u8*)(int)lbl_803137F8;
    int b = 0x30;
    int c = 0x31;
    int a = 1;
    int d = 0x50;
    entries = buf.entries;
    if (overrideParams != NULL)
    {
        a = overrideParams->param0;
        b = overrideParams->param1;
        c = overrideParams->param2;
        d = overrideParams->param3;
    }
    entries[0].layer = 0;
    entries[0].flags = 8;
    entries[0].tex = &base[0x68];
    entries[0].mode = 4;
    entries[0].x = 0.0f;
    entries[0].y = 0.0f;
    entries[0].z = 0.0f;
    entries[1].layer = 0;
    entries[1].flags = 8;
    entries[1].tex = &base[0x68];
    entries[1].mode = 2;
    if (sourceObj != NULL)
    {
        entries[1].x = 7.0f * sourceObj->anim.rootMotionScale;
        entries[1].y = 6.0f * sourceObj->anim.rootMotionScale;
        entries[1].z = 7.0f * sourceObj->anim.rootMotionScale;
    }
    else
    {
        entries[1].x = 7.0f;
        entries[1].y = 6.0f;
        entries[1].z = 7.0f;
    }
    entries[2].layer = 0;
    entries[2].flags = 0;
    entries[2].tex = NULL;
    entries[2].mode = 0x80;
    entries[2].x = 0.0f;
    entries[2].y = 0.0f;
    if (sourceObj != NULL)
    {
        entries[2].z = (f32) * (s16*)sourceObj;
    }
    else
    {
        entries[2].z = 0.0f;
    }
    entries[3].layer = 1;
    entries[3].flags = 8;
    entries[3].tex = &base[0x68];
    entries[3].mode = 4;
    entries[3].x = 255.0f;
    entries[3].y = 0.0f;
    entries[3].z = 0.0f;
    entries[4].layer = 1;
    entries[4].flags = d;
    entries[4].tex = NULL;
    entries[4].mode = 0x20000000;
    entries[4].x = a;
    entries[4].y = b;
    entries[4].z = c;
    e = &entries[5];
    if (variant == 0)
    {
        e->layer = 2;
        e->flags = 0x3b;
        e->tex = NULL;
        e->mode = 0x1800000;
        e->x = 1.0f;
        e->y = 0.0f;
        e->z = 10.0f;
        e++;
    }
    e[0].layer = 2;
    e[0].flags = 0;
    e[0].tex = NULL;
    e[0].mode = 0x100;
    e[0].x = 0.0f;
    e[0].y = 0.0f;
    e[0].z = 50.0f;
    e[1].layer = 3;
    e[1].flags = 1;
    e[1].tex = NULL;
    e[1].mode = 0x2000;
    e[1].x = 0.0f;
    e[1].y = 0.0f;
    e[1].z = 0.0f;
    e[2].layer = 4;
    e[2].flags = 8;
    e[2].tex = &base[0x68];
    e[2].mode = 4;
    e[2].x = 0.0f;
    e[2].y = 0.0f;
    e[2].z = 0.0f;
    e[3].layer = 4;
    e[3].flags = 0;
    e[3].tex = NULL;
    e[3].mode = 0x20000000;
    e[3].x = a;
    e[3].y = b;
    e[3].z = c;
    buf.v58 = variant;
    buf.ctx = (u8*)sourceObj;
    buf.v44 = variant;
    buf.pos[0] = 0.0f;
    if (posSource != NULL)
    {
        buf.pos[1] = ((PartFxSpawnParams*)posSource)->posY;
    }
    else
    {
        buf.pos[1] = 0.0f;
    }
    buf.pos[2] = 0.0f;
    buf.col[0] = 0.0f;
    buf.col[1] = 0.0f;
    buf.col[2] = 0.0f;
    buf.scale = 1.0f;
    buf.v40 = 1;
    buf.v3c = 0;
    buf.v59 = 8;
    buf.v5a = 0;
    buf.v5b = 0x1e;
    buf.count = (e + 4) - entries;
    buf.hw[0] = *(s16*)&base[0x78];
    buf.hw[1] = *(s16*)&base[0x7a];
    buf.hw[2] = *(s16*)&base[0x7c];
    buf.hw[3] = *(s16*)&base[0x7e];
    buf.hw[4] = *(s16*)&base[0x80];
    buf.hw[5] = *(s16*)&base[0x82];
    buf.hw[6] = *(s16*)&base[0x84];
    buf.cmds = (GfxCmd*)((u8*)&buf + 0x60);
    {
        u32 packetFlags = 0x4000000;
        buf.flags = packetFlags;
        packetFlags |= flags | 0x80;
        buf.flags = packetFlags;
        if (variant == 2)
        {
            u32 mask = DLL69_CMDLIST_FLAG_TOGGLE;
            buf.flags = packetFlags ^ mask;
        }
        else
        {
            u32 mask = DLL69_CMDLIST_FLAG_TOGGLE;
            buf.flags = packetFlags | mask;
        }
    }
    if ((buf.flags & 1) != 0)
    {
        if (buf.ctx != NULL)
        {
            buf.pos[0] += ((GameObject*)(buf.ctx))->anim.worldPosX;
            buf.pos[1] += ((GameObject*)(buf.ctx))->anim.worldPosY;
            buf.pos[2] += ((GameObject*)(buf.ctx))->anim.worldPosZ;
        }
        else
        {
            buf.pos[0] += ((PartFxSpawnParams*)posSource)->posX;
            buf.pos[1] += ((PartFxSpawnParams*)posSource)->posY;
            buf.pos[2] += ((PartFxSpawnParams*)posSource)->posZ;
        }
    }
    return (*gModgfxInterface)
        ->spawnEffect(&buf, 0, 8, (u8*)(int)lbl_803137F8, 4, &base[0x50],
                      variant == 2 ? DLL69_EFFECT_ID_VARIANT2 : DLL69_EFFECT_ID_DEFAULT, 0);
}

void dll_69_func01_nop(void)
{
}

void dll_69_func00_nop(void)
{
}

u32 lbl_80313880[8] = {
    0x00000000, 0x00000000,        0x00000000, 0x00030000, (u32)dll_69_func00_nop, (u32)dll_69_func01_nop,
    0x00000000, (u32)dll_69_func03,
};
