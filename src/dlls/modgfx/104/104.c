/*
 * DLL 104 / 0x68 - a thin gameplay-effect DLL.
 *
 * Real exports (per the DLL's .text):
 *   dll_68_func00_nop / dll_68_func01_nop - empty entry-point stubs.
 *   dll_68_func03 - builds an 11-command Modgfx effect command list on the
 *     stack (textures/half-words sourced from lbl_803135C8) and submits it via
 *     gModgfxInterface->spawnEffect. When the caller's flags bit 0 is set the
 *     effect is positioned from the source object (offset 0x18..0x20) or, if
 *     none, from the PartFxSpawnParams pos fields.
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/partfx_interface.h"
#include "game/objects/object.h"
#include "main/dll/modgfx_types.h"
#include "main/dll/dll_0068_dll68func0.h"

/* effect id spawned by this DLL's modgfx emitter (spawnEffect textureAssetId arg). */
#define DLL68_EFFECT_ID 0x41

extern u32 lbl_803135C8[];

void dll_68_func03(int sourceObj, int variant, int posSource, u32 flags)
{
    ModgfxSpawnPacket buf;
    u8* base = (u8*)(int)lbl_803135C8;
    int ctx;
    buf.entries[0].layer = 0;
    buf.entries[0].flags = 0x15;
    buf.entries[0].tex = &base[432];
    buf.entries[0].mode = 4;
    buf.entries[0].x = 0.0f;
    buf.entries[0].y = 0.0f;
    buf.entries[0].z = 0.0f;
    buf.entries[1].layer = 0;
    buf.entries[1].flags = 0x15;
    buf.entries[1].tex = &base[432];
    buf.entries[1].mode = 2;
    buf.entries[1].x = 2.1f;
    buf.entries[1].y = 2.0f;
    buf.entries[1].z = 2.1f;
    buf.entries[2].layer = 1;
    buf.entries[2].flags = 7;
    buf.entries[2].tex = &base[356];
    buf.entries[2].mode = 2;
    buf.entries[2].x = 2.0f;
    buf.entries[2].y = 1.0f;
    buf.entries[2].z = 2.0f;
    buf.entries[3].layer = 1;
    buf.entries[3].flags = 7;
    buf.entries[3].tex = &base[372];
    buf.entries[3].mode = 4;
    buf.entries[3].x = 255.0f;
    buf.entries[3].y = 0.0f;
    buf.entries[3].z = 0.0f;
    buf.entries[4].layer = 1;
    buf.entries[4].flags = 0x15;
    buf.entries[4].tex = &base[432];
    buf.entries[4].mode = 0x4000;
    buf.entries[4].x = 0.0f;
    buf.entries[4].y = -2.0f;
    buf.entries[4].z = 0.0f;
    buf.entries[5].layer = 1;
    buf.entries[5].flags = 0;
    buf.entries[5].tex = 0;
    buf.entries[5].mode = 0x100;
    buf.entries[5].x = 0.0f;
    buf.entries[5].y = 0.0f;
    buf.entries[5].z = 150.0f;
    buf.entries[6].layer = 2;
    buf.entries[6].flags = 0x15;
    buf.entries[6].tex = &base[432];
    buf.entries[6].mode = 0x4000;
    buf.entries[6].x = 0.0f;
    buf.entries[6].y = -2.0f;
    buf.entries[6].z = 0.0f;
    buf.entries[7].layer = 2;
    buf.entries[7].flags = 0;
    buf.entries[7].tex = 0;
    buf.entries[7].mode = 0x100;
    buf.entries[7].x = 0.0f;
    buf.entries[7].y = 0.0f;
    buf.entries[7].z = 150.0f;
    buf.entries[8].layer = 3;
    buf.entries[8].flags = 7;
    buf.entries[8].tex = &base[372];
    buf.entries[8].mode = 4;
    buf.entries[8].x = 0.0f;
    buf.entries[8].y = 0.0f;
    buf.entries[8].z = 0.0f;
    buf.entries[9].layer = 3;
    buf.entries[9].flags = 0x15;
    buf.entries[9].tex = &base[432];
    buf.entries[9].mode = 0x4000;
    buf.entries[9].x = 0.0f;
    buf.entries[9].y = -2.0f;
    buf.entries[9].z = 0.0f;
    buf.entries[10].layer = 3;
    buf.entries[10].flags = 0;
    buf.entries[10].tex = 0;
    buf.entries[10].mode = 0x100;
    buf.entries[10].x = 0.0f;
    buf.entries[10].y = 0.0f;
    buf.entries[10].z = 150.0f;
    buf.v58 = 0;
    ctx = sourceObj;
    buf.ctx = ctx;
    buf.v44 = variant;
    buf.pos[0] = 0.0f;
    buf.pos[1] = 0.0f;
    buf.pos[2] = 0.0f;
    buf.col[0] = 0.0f;
    buf.col[1] = 0.0f;
    buf.col[2] = 0.0f;
    buf.scale = 1.0f;
    buf.v40 = 2;
    buf.v3c = 7;
    buf.v59 = 0xe;
    buf.v5a = 0;
    buf.v5b = 0x1e;
    buf.count = 11;
    buf.hw[0] = *(s16*)&base[476];
    buf.hw[1] = *(s16*)&base[478];
    buf.hw[2] = *(s16*)&base[480];
    buf.hw[3] = *(s16*)&base[482];
    buf.hw[4] = *(s16*)&base[484];
    buf.hw[5] = *(s16*)&base[486];
    buf.hw[6] = *(s16*)&base[488];
    buf.cmds = (GfxCmd*)((u8*)&buf + 0x60);
    buf.flags = 0xc0100c0;
    buf.flags |= flags;
    if ((buf.flags & 1) != 0)
    {
        if (*(void**)&buf.ctx != 0)
        {
            buf.pos[0] = ((GameObject*)(buf.ctx))->anim.worldPosX;
            buf.pos[1] = ((GameObject*)(buf.ctx))->anim.worldPosY;
            buf.pos[2] = ((GameObject*)(buf.ctx))->anim.worldPosZ;
        }
        else
        {
            buf.pos[0] = ((PartFxSpawnParams*)posSource)->posX;
            buf.pos[1] = ((PartFxSpawnParams*)posSource)->posY;
            buf.pos[2] = ((PartFxSpawnParams*)posSource)->posZ;
        }
    }
    (*gModgfxInterface)->spawnEffect(&buf, 0, 0x15, (u8*)(int)lbl_803135C8, 0x18, &base[212], DLL68_EFFECT_ID, 0);
}

void dll_68_func01_nop(void)
{
}

void dll_68_func00_nop(void)
{
}
