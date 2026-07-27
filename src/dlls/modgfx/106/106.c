/*
 * DLL 106 / 0x6A - particle/gfx spawn helper.
 *
 * The unit owns three entry points:
 *   dll_6A_func00_nop / dll_6A_func01_nop - empty stubs.
 *   dll_6A_func03 - builds a 10-command graphics command list (GfxCmd[])
 *     on the stack from a layout table (lbl_803138A0), then submits it via
 *     gModgfxInterface->spawnEffect.
 *     Bit 0 of the spawn-context flags word enables
 *     world-position override: non-null sourceObj uses the GameObject
 *     world position, null uses posSource as a PartFxSpawnParams packet.
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/partfx_interface.h"
#include "game/objects/object.h"
#include "main/dll/modgfx_types.h"
#include "main/dll/dll_006A_dll6afunc0.h"

/* effect id spawned by this DLL's modgfx emitter (spawnEffect textureAssetId arg). */
#define DLL6A_EFFECT_ID 0x3e

extern u32 lbl_803138A0[];

s16 dll_6A_func03(void* sourceObj, int variant, PartFxSpawnParams* spawnParams, u32 spawnFlags, int modelId,
                  void* extraArg)
{
    ModgfxPointerSpawnPacket buf;
    u8* tab = (u8*)(int)lbl_803138A0;
    GfxCmd* e = buf.entries;
    e[0].layer = 0;
    e[0].flags = 0x12;
    e[0].tex = &tab[296];
    e[0].mode = 4;
    e[0].x = 0.0f;
    e[0].y = 0.0f;
    e[0].z = 0.0f;
    e[1].layer = 0;
    e[1].flags = 0x12;
    e[1].tex = &tab[296];
    e[1].mode = 2;
    e[1].x = 0.2f;
    e[1].y = 4.0f;
    e[1].z = 0.2f;
    e[2].layer = 0;
    e[2].flags = 9;
    e[2].tex = &tab[276];
    e[2].mode = 8;
    e[2].x = (f32)(u32)tab[variant * 3 + 368];
    e[2].y = (f32)(u32)tab[variant * 3 + 369];
    e[2].z = (f32)(u32)tab[variant * 3 + 370];
    e[3].layer = 1;
    e[3].flags = 0x12;
    e[3].tex = &tab[296];
    e[3].mode = 4;
    e[3].x = 85.0f;
    e[3].y = 0.0f;
    e[3].z = 0.0f;
    e[4].layer = 1;
    e[4].flags = 0x12;
    e[4].tex = &tab[296];
    e[4].mode = 2;
    e[4].x = 15.0f;
    e[4].y = 0.15f;
    e[4].z = 15.0f;
    e[5].layer = 3;
    e[5].flags = 0x12;
    e[5].tex = &tab[296];
    e[5].mode = 0x100;
    e[5].x = 0.0f;
    e[5].y = 0.0f;
    e[5].z = 30.0f;
    e[6].layer = 4;
    e[6].flags = 2;
    e[6].tex = 0;
    e[6].mode = 0x2000;
    e[6].x = 0.0f;
    e[6].y = 0.0f;
    e[6].z = 0.0f;
    e[7].layer = 5;
    e[7].flags = 0x12;
    e[7].tex = &tab[296];
    e[7].mode = 4;
    e[7].x = 0.0f;
    e[7].y = 0.0f;
    e[7].z = 0.0f;
    e[8].layer = 5;
    e[8].flags = 0x12;
    e[8].tex = &tab[296];
    e[8].mode = 2;
    e[8].x = 0.1f;
    e[8].y = 10.0f;
    e[8].z = 0.1f;
    e[9].layer = 5;
    e[9].flags = 0x7a;
    e[9].tex = 0;
    e[9].mode = 0x10000;
    e[9].x = 0.0f;
    e[9].y = 0.0f;
    e[9].z = 0.0f;
    buf.v58 = 0;
    buf.ctx = (u8*)sourceObj;
    buf.v44 = variant;
    buf.pos[0] = 0.0f;
    buf.pos[1] = 0.0f;
    buf.pos[2] = 0.0f;
    buf.col[0] = 0.0f;
    buf.col[1] = 0.0f;
    buf.col[2] = 0.0f;
    buf.scale = 1.0f;
    buf.v40 = 1;
    buf.v3c = 0;
    buf.v59 = 0x12;
    buf.v5a = 0;
    buf.v5b = 0x10;
    buf.flags = 0x5000004;
    buf.count = (e + 10) - buf.entries;
    buf.hw[0] = *(s16*)&tab[352];
    buf.hw[1] = *(s16*)&tab[354];
    buf.hw[2] = *(s16*)&tab[356];
    buf.hw[3] = *(s16*)&tab[358];
    buf.hw[4] = *(s16*)&tab[360];
    buf.hw[5] = *(s16*)&tab[362];
    buf.hw[6] = *(s16*)&tab[364];
    buf.cmds = buf.entries;
    buf.flags |= spawnFlags;
    if ((buf.flags & 1) != 0)
    {
        if (sourceObj != 0)
        {
            buf.pos[0] = ((GameObject*)sourceObj)->anim.worldPosX;
            buf.pos[1] = ((GameObject*)sourceObj)->anim.worldPosY;
            buf.pos[2] = ((GameObject*)sourceObj)->anim.worldPosZ;
        }
        else
        {
            buf.pos[0] = spawnParams->posX;
            buf.pos[1] = spawnParams->posY;
            buf.pos[2] = spawnParams->posZ;
        }
    }
    return (*gModgfxInterface)
        ->spawnEffect(&buf, 0, 0x12, (u8*)(int)lbl_803138A0, 0x10, &((u8*)(int)lbl_803138A0)[180], DLL6A_EFFECT_ID, 0);
}

void dll_6A_func01_nop(void)
{
}

void dll_6A_func00_nop(void)
{
}
