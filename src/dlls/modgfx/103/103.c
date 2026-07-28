/*
 * DLL 103 / 0x67 - a gameplay-preview effect spawner.
 *
 * dll_67_func03 builds a modgfx command list on the stack (seven GfxCmd
 * layers over the asset table lbl_803133B8) and spawns a
 * gameplay-preview effect via gModgfxInterface; when flag bit 0 is set,
 * the spawn position is offset from either the source object's world
 * position or the caller's spawn-param packet. func00/func01 are the
 * DLL's unused entry-point stubs.
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/partfx_interface.h"
#include "main/dll/modgfx_types.h"
#include "dlls/object_descriptor.h"

/* effect id spawned by this DLL's modgfx emitter (spawnEffect textureAssetId arg). */
#define DLL67_EFFECT_ID 0xe3

u32 lbl_803133B8[123] = {
    0x00000000, 0x03e80000, 0x00000362, 0x000001f4, 0x000b0000, 0x03620000, 0xfe0c0016, 0x00000000, 0x0000fc18,
    0x00200000, 0xfc9e0000, 0xfe0c002a, 0x0000fc9e, 0x000001f4, 0x00340000, 0x00000000, 0x03e8003f, 0x00000000,
    0x0bb803e8, 0x0000003f, 0x03620bb8, 0x01f4000b, 0x003f0362, 0x0bb8fe0c, 0x0016003f, 0x00000bb8, 0xfc180020,
    0x003ffc9e, 0x0bb8fe0c, 0x002a003f, 0xfc9e0bb8, 0x01f40034, 0x003f0000, 0x0bb803e8, 0x003f003f, 0x00001770,
    0x03e80000, 0x007f0362, 0x177001f4, 0x000b007f, 0x03621770, 0xfe0c0016, 0x007f0000, 0x1770fc18, 0x0020007f,
    0xfc9e1770, 0xfe0c002a, 0x007ffc9e, 0x177001f4, 0x0034007f, 0x00001770, 0x03e8003f, 0x007f0000, 0x00000001,
    0x00080000, 0x00080007, 0x00010002, 0x00090001, 0x00090008, 0x00020003, 0x000a0002, 0x000a0009, 0x00030004,
    0x000b0003, 0x000b000a, 0x00040005, 0x000c0004, 0x000c000b, 0x00050006, 0x000d0005, 0x000d000c, 0x00070008,
    0x000f0007, 0x000f000e, 0x00080009, 0x00100008, 0x0010000f, 0x0009000a, 0x00110009, 0x00110010, 0x000a000b,
    0x0012000a, 0x00120011, 0x000b000c, 0x0013000b, 0x00130012, 0x000c000d, 0x0014000c, 0x00140013, 0x00000001,
    0x00020003, 0x00040005, 0x00060000, 0x00070008, 0x0009000a, 0x000b000c, 0x000d0000, 0x000e000f, 0x00100011,
    0x00120013, 0x00140000, 0x00000001, 0x00020003, 0x00040005, 0x0006000e, 0x000f0010, 0x00110012, 0x00130014,
    0x00000001, 0x00020003, 0x00040005, 0x00060007, 0x00080009, 0x000a000b, 0x000c000d, 0x000e000f, 0x00100011,
    0x00120013, 0x00140000, 0x00000032, 0x00640032, 0x00000000, 0x00000000,
};

void dll_67_func03(int sourceObj, int variant, int posSource, u32 flags)
{
    ModgfxSpawnPacket buf;
    u8* base = (u8*)(int)lbl_803133B8;
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
    buf.entries[1].x = 1.8f;
    buf.entries[1].y = 2.0f;
    buf.entries[1].z = 1.8f;
    buf.entries[2].layer = 1;
    buf.entries[2].flags = 7;
    buf.entries[2].tex = &base[372];
    buf.entries[2].mode = 4;
    buf.entries[2].x = 255.0f;
    buf.entries[2].y = 0.0f;
    buf.entries[2].z = 0.0f;
    buf.entries[3].layer = 1;
    buf.entries[3].flags = 0x15;
    buf.entries[3].tex = &base[432];
    buf.entries[3].mode = 0x4000;
    buf.entries[3].x = 0.0f;
    buf.entries[3].y = -8.0f;
    buf.entries[3].z = 0.0f;
    buf.entries[4].layer = 2;
    buf.entries[4].flags = 0x15;
    buf.entries[4].tex = &base[432];
    buf.entries[4].mode = 0x4000;
    buf.entries[4].x = 0.0f;
    buf.entries[4].y = -8.0f;
    buf.entries[4].z = 0.0f;
    buf.entries[5].layer = 3;
    buf.entries[5].flags = 7;
    buf.entries[5].tex = &base[372];
    buf.entries[5].mode = 4;
    buf.entries[5].x = 0.0f;
    buf.entries[5].y = 0.0f;
    buf.entries[5].z = 0.0f;
    buf.entries[6].layer = 3;
    buf.entries[6].flags = 0x15;
    buf.entries[6].tex = &base[432];
    buf.entries[6].mode = 0x4000;
    buf.entries[6].x = 0.0f;
    buf.entries[6].y = -8.0f;
    buf.entries[6].z = 0.0f;
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
    buf.count = 7;
    buf.hw[0] = *(s16*)&base[476];
    buf.hw[1] = *(s16*)&base[478];
    buf.hw[2] = *(s16*)&base[480];
    buf.hw[3] = *(s16*)&base[482];
    buf.hw[4] = *(s16*)&base[484];
    buf.hw[5] = *(s16*)&base[486];
    buf.hw[6] = *(s16*)&base[488];
    buf.cmds = (GfxCmd*)((u8*)&buf + 0x60);
    buf.flags = 0xc010040;
    buf.flags |= flags;
    if ((buf.flags & 1) != 0)
    {
        if (*(void**)&buf.ctx != 0)
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
    (*gModgfxInterface)->spawnEffect(&buf, 0, 0x15, (u8*)(int)lbl_803133B8, 0x18, &base[212], DLL67_EFFECT_ID, 0);
}

void dll_67_func01_nop(void)
{
}

void dll_67_func00_nop(void)
{
}

ObjectDescriptor4WithPadding dll_67_funcs = {
    {
        0,
        0,
        0,
        OBJECT_DESCRIPTOR_FLAGS_4_SLOTS,
        (ObjectDescriptorCallback)dll_67_func00_nop,
        (ObjectDescriptorCallback)dll_67_func01_nop,
        0,
        (ObjectDescriptorCallback)dll_67_func03,
    },
    0,
};
