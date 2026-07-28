/*
 * DLL 155 / 0x9B - one of the screenfx scene builders (sibling of
 * DLL 0x9A/0x9C). dll_9B_func03 fills a fixed 14-entry GfxCmd list plus the
 * surrounding ModgfxSpawnPacket describing a multi-state screen effect (texture/
 * model ids, per-part placement offsets and a 7-entry anim table read out of
 * the lbl_80317BD8 resource blob), then hands it to ModgfxInterface
 * spawnEffect (effect 0x15, asset 0x156). When header flag bit 0 is set the
 * base position is offset by either the target object's transform (target,
 * +0x18) or the passed parameter packet (parent, +0x0C). func00/func01 are
 * the DLL's nop lifecycle slots.
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/modgfx_types.h"
#include "main/dll/partfx_interface.h"
#include "dlls/object_descriptor.h"

extern u32 lbl_80317BD8[];

/* effect id spawned by this DLL's modgfx emitter (spawnEffect textureAssetId arg). */
#define DLL9B_EFFECT_ID 0x156


void dll_9B_func03(int target, int variant, int parent, u32 flags)
{
    ModgfxSpawnPacket buf;
    u8* base = (u8*)(int)lbl_80317BD8;
    GfxCmd* entry = buf.entries;

    entry[0].layer = 0;
    entry[0].flags = 0x15;
    entry[0].tex = base + 0x1b0;
    entry[0].mode = 4;
    entry[0].x = 0.0f;
    entry[0].y = 0.0f;
    entry[0].z = 0.0f;
    entry[1].layer = 0;
    entry[1].flags = 0x15;
    entry[1].tex = base + 0x1b0;
    entry[1].mode = 2;
    entry[1].x = 0.01f;
    entry[1].y = 2.0f;
    entry[1].z = 0.01f;
    entry[2].layer = 0;
    entry[2].flags = 0;
    entry[2].tex = 0;
    entry[2].mode = 0x400000;
    entry[2].x = 0.0f;
    entry[2].y = 100.0f;
    entry[2].z = 0.0f;
    entry[3].layer = 0;
    entry[3].flags = 0x124;
    entry[3].tex = 0;
    entry[3].mode = 0x20000;
    entry[3].x = 0.0f;
    entry[3].y = 0.0f;
    entry[3].z = 0.0f;
    entry[4].layer = 1;
    entry[4].flags = 0x15;
    entry[4].tex = base + 0x1b0;
    entry[4].mode = 2;
    entry[4].x = 10.0f;
    entry[4].y = 1.3f;
    entry[4].z = 10.0f;
    entry[5].layer = 1;
    entry[5].flags = 0xe;
    entry[5].tex = base + 0x1dc;
    entry[5].mode = 4;
    entry[5].x = 255.0f;
    entry[5].y = 0.0f;
    entry[5].z = 0.0f;
    entry[6].layer = 1;
    entry[6].flags = 0x15;
    entry[6].tex = base + 0x1b0;
    entry[6].mode = 0x4000;
    entry[6].x = 2.0f;
    entry[6].y = 6.0f;
    entry[6].z = 0.0f;
    entry[7].layer = 1;
    entry[7].flags = 0;
    entry[7].tex = 0;
    entry[7].mode = 0x400000;
    entry[7].x = 0.0f;
    entry[7].y = -100.0f;
    entry[7].z = 0.0f;
    entry[8].layer = 2;
    entry[8].flags = 0x15;
    entry[8].tex = base + 0x1b0;
    entry[8].mode = 0x4000;
    entry[8].x = 2.0f;
    entry[8].y = 6.0f;
    entry[8].z = 0.0f;
    entry[9].layer = 3;
    entry[9].flags = 0x124;
    entry[9].tex = 0;
    entry[9].mode = 0x20000;
    entry[9].x = 0.0f;
    entry[9].y = 0.0f;
    entry[9].z = 0.0f;
    entry[10].layer = 3;
    entry[10].flags = 0xe;
    entry[10].tex = base + 0x1dc;
    entry[10].mode = 4;
    entry[10].x = 0.0f;
    entry[10].y = 0.0f;
    entry[10].z = 0.0f;
    entry[11].layer = 3;
    entry[11].flags = 0x15;
    entry[11].tex = base + 0x1b0;
    entry[11].mode = 0x4000;
    entry[11].x = 2.0f;
    entry[11].y = 6.0f;
    entry[11].z = 0.0f;
    entry[12].layer = 3;
    entry[12].flags = 0x15;
    entry[12].tex = base + 0x1b0;
    entry[12].mode = 2;
    entry[12].x = 0.01f;
    entry[12].y = 1.0f;
    entry[12].z = 0.01f;
    entry[13].layer = 3;
    entry[13].flags = 0;
    entry[13].tex = 0;
    entry[13].mode = 0x400000;
    entry[13].x = 0.0f;
    entry[13].y = 100.0f;
    entry[13].z = 0.0f;

    buf.v58 = 0;
    buf.ctx = target;
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
    buf.count = (GfxCmd*)((u8*)entry + 0x150) - entry;
    buf.hw[0] = *(s16*)(base + 0x1f8);
    buf.hw[1] = *(s16*)(base + 0x1fa);
    buf.hw[2] = *(s16*)(base + 0x1fc);
    buf.hw[3] = *(s16*)(base + 0x1fe);
    buf.hw[4] = *(s16*)(base + 0x200);
    buf.hw[5] = *(s16*)(base + 0x202);
    buf.hw[6] = *(s16*)(base + 0x204);
    buf.cmds = buf.entries;
    buf.flags = 0xc010480;
    buf.flags |= flags;
    if ((buf.flags & 1) != 0)
    {
        if ((u32)target != 0)
        {
            buf.pos[0] += ((GameObject*)(target))->anim.worldPosX;
            buf.pos[1] += ((GameObject*)(target))->anim.worldPosY;
            buf.pos[2] += ((GameObject*)(target))->anim.worldPosZ;
        }
        else
        {
            buf.pos[0] += ((PartFxSpawnParams*)parent)->posX;
            buf.pos[1] += ((PartFxSpawnParams*)parent)->posY;
            buf.pos[2] += ((PartFxSpawnParams*)parent)->posZ;
        }
    }
    (*gModgfxInterface)->spawnEffect(&buf, 0, 0x15, (u8*)(int)lbl_80317BD8, 0x18, base + 0xd4, DLL9B_EFFECT_ID, 0);
}

void dll_9B_func01_nop(void)
{
}

void dll_9B_func00_nop(void)
{
}

u32 lbl_80317BD8[130] = {
    0x00000000, 0x03e80000, 0x00000362, 0x000001f4, 0x00420000, 0x03620000, 0xfe0c0084, 0x00000000, 0x0000fc18,
    0x00c00000, 0xfc9e0000, 0xfe0c0084, 0x0000fc9e, 0x000001f4, 0x00420000, 0x00000000, 0x03e80000, 0x00000000,
    0x01f403e8, 0x0000005a, 0x036201f4, 0x01f40042, 0x005a0362, 0x01f4fe0c, 0x0084005a, 0x000001f4, 0xfc1800c0,
    0x005afc9e, 0x01f4fe0c, 0x0084005a, 0xfc9e01f4, 0x01f40042, 0x005a0000, 0x01f403e8, 0x0000005a, 0x00001770,
    0x03e80000, 0x02fa0362, 0x177001f4, 0x004202fa, 0x03621770, 0xfe0c0084, 0x02fa0000, 0x1770fc18, 0x00c002fa,
    0xfc9e1770, 0xfe0c0084, 0x02fafc9e, 0x177001f4, 0x004202fa, 0x00001770, 0x03e80000, 0x02fa0000, 0x00000001,
    0x00080000, 0x00080007, 0x00010002, 0x00090001, 0x00090008, 0x00020003, 0x000a0002, 0x000a0009, 0x00030004,
    0x000b0003, 0x000b000a, 0x00040005, 0x000c0004, 0x000c000b, 0x00050006, 0x000d0005, 0x000d000c, 0x00070008,
    0x000f0007, 0x000f000e, 0x00080009, 0x00100008, 0x0010000f, 0x0009000a, 0x00110009, 0x00110010, 0x000a000b,
    0x0012000a, 0x00120011, 0x000b000c, 0x0013000b, 0x00130012, 0x000c000d, 0x0014000c, 0x00140013, 0x00000001,
    0x00020003, 0x00040005, 0x00060000, 0x00070008, 0x0009000a, 0x000b000c, 0x000d0000, 0x000e000f, 0x00100011,
    0x00120013, 0x00140000, 0x00000001, 0x00020003, 0x00040005, 0x0006000e, 0x000f0010, 0x00110012, 0x00130014,
    0x00000001, 0x00020003, 0x00040005, 0x00060007, 0x00080009, 0x000a000b, 0x000c000d, 0x000e000f, 0x00100011,
    0x00120013, 0x00140000, 0x00070008, 0x0009000a, 0x000b000c, 0x000d000e, 0x000f0010, 0x00110012, 0x00130014,
    0x00000096, 0x044c0032, 0x00010000, 0x00000000};
ObjectDescriptor4 dll_9B_funcs = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_4_SLOTS,
    (ObjectDescriptorCallback)dll_9B_func00_nop,
    (ObjectDescriptorCallback)dll_9B_func01_nop,
    0,
    (ObjectDescriptorCallback)dll_9B_func03,
};
