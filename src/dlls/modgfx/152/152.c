/*
 * DLL 152 / 0x98 - a model/screen effect emitter sharing foodbag's
 * modgfx command-list pattern (cf. DLL 153 / 0x99). func00/func01 are
 * empty entry stubs (defined below in reverse address order: func01 then
 * func00); func03 fills a ModgfxSpawnPacket of nine command entries from the .sdata2
 * float table at 0.0f.. and the per-entry flag/texture/anim table at
 * lbl_803178B0, then dispatches it through gModgfxInterface->spawnEffect.
 *
 * The extraArgs argument (zero vs. non-zero) selects between two y-offset
 * constants for entries 3/4/6/7 and the spawn-position offset. When effect
 * flag bit 0 is set, the spawn position is offset by the source object's
 * world position (ctx+0x18) or the posSource frame (posSource+0xc).
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/modgfx_types.h"
#include "main/dll/partfx_interface.h"
#include "dlls/object_descriptor.h"

extern u32 lbl_803178B0[];

/* spawnEffect effect ids per variant (textureAssetId arg). */
#define DLL98_EFFECT_ID_VARIANT0 0x3e9
#define DLL98_EFFECT_ID_VARIANT1 0x3f0
#define DLL98_EFFECT_ID_DEFAULT  0x3f3

typedef struct Dll98EffectTable
{
    u8 baseArgs[0xb4];
    u8 variantArgs[0x168 - 0xb4];
    u8 effectArgs[0x1dc - 0x168];
    u8 textureData[0x214 - 0x1dc];
    s16 frameValues[7];
    u8 pad222[2];
} Dll98EffectTable;

STATIC_ASSERT(sizeof(Dll98EffectTable) == 0x224);

void dll_98_func03(int sourceObj, int variant, int posSource, u32 flags, int arg5, int extraArgs)
{
    ModgfxSpawnPacket buf;
    u8* table = (u8*)(int)lbl_803178B0;
    Dll98EffectTable* effectTable = (Dll98EffectTable*)table;
    GfxCmd* entry;
    int anim;
    effectTable->frameValues[1] = randomGetRange(0, 0x1e) + 0x1e;
    effectTable->frameValues[2] = (s32)effectTable->frameValues[1];
    entry = buf.entries;
    entry[0].layer = 0;
    entry[0].flags = 0x12;
    entry[0].tex = table + 0x1dc;
    entry[0].mode = 4;
    entry[0].x = 0.0f;
    entry[0].y = 0.0f;
    entry[0].z = 0.0f;
    entry[1].layer = 0;
    entry[1].flags = 0x12;
    entry[1].tex = table + 0x1dc;
    entry[1].mode = 2;
    entry[1].z = entry[1].x = 0.22f;
    entry[1].y = 0.3f;
    entry[2].layer = 1;
    entry[2].flags = 0x12;
    entry[2].tex = table + 0x1dc;
    entry[2].mode = 4;
    entry[2].x = 255.0f;
    entry[2].y = 0.0f;
    entry[2].z = 0.0f;
    entry[3].layer = 1;
    entry[3].flags = 0x12;
    entry[3].tex = table + 0x1dc;
    entry[3].mode = 0x400000;
    entry[3].x = 0.0f;
    if ((u32)extraArgs != 0)
    {
        entry[3].y = -7.0f;
    }
    else
    {
        entry[3].y = 7.0f;
    }
    entry[3].z = 0.0f;
    entry[4].layer = 1;
    entry[4].flags = 0x12;
    entry[4].tex = table + 0x1dc;
    entry[4].mode = 0x4000;
    entry[4].x = 0.0f;
    if ((u32)extraArgs != 0)
    {
        entry[4].y = 1.0f;
    }
    else
    {
        entry[4].y = -1.0f;
    }
    entry[4].z = 0.0f;
    entry[5].layer = 2;
    entry[5].flags = 0x12;
    entry[5].tex = table + 0x1dc;
    entry[5].mode = 4;
    entry[5].x = 0.0f;
    entry[5].y = 0.0f;
    entry[5].z = 0.0f;
    entry[6].layer = 2;
    entry[6].flags = 0x12;
    entry[6].tex = table + 0x1dc;
    entry[6].mode = 0x400000;
    entry[6].x = 0.0f;
    if ((u32)extraArgs != 0)
    {
        entry[6].y = -7.0f;
    }
    else
    {
        entry[6].y = 7.0f;
    }
    entry[6].z = 0.0f;
    entry[7].layer = 2;
    entry[7].flags = 0x12;
    entry[7].tex = table + 0x1dc;
    entry[7].mode = 0x4000;
    entry[7].x = 0.0f;
    if ((u32)extraArgs != 0)
    {
        entry[7].y = 1.0f;
    }
    else
    {
        entry[7].y = -1.0f;
    }
    entry[7].z = 0.0f;
    entry[8].layer = 2;
    entry[8].flags = 0x12;
    entry[8].tex = table + 0x1dc;
    entry[8].mode = 2;
    entry[8].x = 1.0f;
    entry[8].y = 1.0f;
    entry[8].z = 1.0f;
    buf.v58 = 0;
    buf.ctx = sourceObj;
    buf.v44 = variant;
    buf.pos[0] = 0.0f;
    if ((u32)extraArgs != 0)
    {
        buf.pos[1] = -2.0f;
    }
    else
    {
        buf.pos[1] = 2.0f;
    }
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
    buf.flags = 0x4080400; /* bit 0 enables position offset below */
    buf.count = (GfxCmd*)((u8*)entry + 0xd8) - entry;
    buf.hw[0] = effectTable->frameValues[0];
    buf.hw[1] = effectTable->frameValues[1];
    buf.hw[2] = effectTable->frameValues[2];
    buf.hw[3] = effectTable->frameValues[3];
    buf.hw[4] = effectTable->frameValues[4];
    buf.hw[5] = effectTable->frameValues[5];
    buf.hw[6] = effectTable->frameValues[6];
    buf.cmds = (GfxCmd*)((u8*)&buf + 0x60);
    buf.flags |= flags;
    if ((buf.flags & 1) != 0)
    {
        if ((u32)buf.ctx != 0)
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
    if (variant == 0)
    {
        anim = DLL98_EFFECT_ID_VARIANT0;
    }
    else if (variant == 1)
    {
        anim = DLL98_EFFECT_ID_VARIANT1;
    }
    else
    {
        anim = DLL98_EFFECT_ID_DEFAULT;
    }
    (*gModgfxInterface)
        ->spawnEffect(&buf, 0, 0x12,
                      (u32)extraArgs != 0 ? table + 0xb4 : (u8*)(int)lbl_803178B0,
                      0x10, table + 0x168,
                      anim, 0);
}

void dll_98_func01_nop(void)
{
}

void dll_98_func00_nop(void)
{
}

u32 lbl_803178B0[137] = {
    0x00000000, 0x03e80000, 0x0000fd3d, 0x000002c3, 0x000f0000, 0xfc180000, 0x0000001f, 0x0000fd3d, 0x0000fd3d,
    0x002f0000, 0x00000000, 0xfc18003f, 0x000002c3, 0x0000fd3d, 0x004f0000, 0x03e80000, 0x0000005f, 0x000002c3,
    0x000002c3, 0x006f0000, 0x00000000, 0x03e8007f, 0x00000000, 0x07d003e8, 0x0000001f, 0xfd3d07d0, 0x02c3000f,
    0x001ffc18, 0x07d00000, 0x001f001f, 0xfd3d07d0, 0xfd3d002f, 0x001f0000, 0x07d0fc18, 0x003f001f, 0x02c307d0,
    0xfd3d004f, 0x001f03e8, 0x07d00000, 0x005f001f, 0x02c307d0, 0x02c3006f, 0x001f0000, 0x07d003e8, 0x007f001f,
    0x00000000, 0x03e80000, 0x0000fd3d, 0x000002c3, 0x000f0000, 0xfc180000, 0x0000001f, 0x0000fd3d, 0x0000fd3d,
    0x002f0000, 0x00000000, 0xfc18003f, 0x000002c3, 0x0000fd3d, 0x004f0000, 0x03e80000, 0x0000005f, 0x000002c3,
    0x000002c3, 0x006f0000, 0x00000000, 0x03e8007f, 0x00000000, 0xf83003e8, 0x0000001f, 0xfd3df830, 0x02c3000f,
    0x001ffc18, 0xf8300000, 0x001f001f, 0xfd3df830, 0xfd3d002f, 0x001f0000, 0xf830fc18, 0x003f001f, 0x02c3f830,
    0xfd3d004f, 0x001f03e8, 0xf8300000, 0x005f001f, 0x02c3f830, 0x02c3006f, 0x001f0000, 0xf83003e8, 0x007f001f,
    0x00000001, 0x000a0000, 0x000a0009, 0x00010002, 0x000b0001, 0x000b000a, 0x00020003, 0x000c0002, 0x000c000b,
    0x00030004, 0x000d0003, 0x000d000c, 0x00040005, 0x000e0004, 0x000e000d, 0x00050006, 0x000f0005, 0x000f000e,
    0x00060007, 0x00100006, 0x0010000f, 0x00070008, 0x00110007, 0x00110010, 0x00000001, 0x00020003, 0x00040005,
    0x00060007, 0x00080000, 0x00000001, 0x00020003, 0x00040005, 0x00060007, 0x00080009, 0x000a000b, 0x000c000d,
    0x000e000f, 0x00100011, 0x0009000a, 0x000b000c, 0x000d000e, 0x000f0010, 0x00110000, 0x00000064, 0x00640000,
    0x00000000, 0x00000000};
ObjectDescriptor4WithPadding dll_98_funcs = {
    {
        0,
        0,
        0,
        OBJECT_DESCRIPTOR_FLAGS_4_SLOTS,
        (ObjectDescriptorCallback)dll_98_func00_nop,
        (ObjectDescriptorCallback)dll_98_func01_nop,
        0,
        (ObjectDescriptorCallback)dll_98_func03,
    },
    0,
};
