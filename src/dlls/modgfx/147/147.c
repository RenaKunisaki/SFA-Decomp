/*
 * DLL 147 / 0x93 - mod-gfx effect spawner.
 *
 * func00/func01 are empty stub entry points (kept to align this DLL's
 * exported function set with the v1.0 assembly). func03 builds a six-entry
 * GfxCmd display list on the stack from the resource blob at
 * lbl_80317260, fills out the surrounding ModgfxSpawnPacket parameters, optionally
 * offsets the effect position by the source/posSource object's world
 * position (when the caller-supplied flag bit 0 is set), then hands the
 * buffer to the mod-gfx interface's spawnEffect.
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/modgfx_types.h"
#include "main/dll/partfx_interface.h"
#include "dlls/object_descriptor.h"

extern u32 lbl_80317260[];

/* effect id spawned by this DLL's modgfx emitter (spawnEffect textureAssetId arg). */
#define DLL93_EFFECT_ID 0x89

void dll_93_func03(int sourceObj, int variant, int posSource, u32 flags)
{
    ModgfxSpawnPacket buf;
    u8* base = (u8*)(int)lbl_80317260;
    GfxCmd* e = buf.entries;
    f32 rval;

    e[0].layer = 0;
    e[0].flags = 0x15;
    e[0].tex = base + 0x1b0;
    e[0].mode = 4;
    e[0].x = 0.0f;
    e[0].y = 0.0f;
    e[0].z = 0.0f;
    e[1].layer = 0;
    e[1].flags = 0x15;
    e[1].tex = base + 0x1b0;
    e[1].mode = 2;
    rval = 0.1f * (f32)randomGetRange(0, 10) + 1.0f;
    e[1].x = rval;
    e[1].y = 10.5f;
    e[1].z = rval;
    e[2].layer = 1;
    e[2].flags = 0x15;
    e[2].tex = base + 0x1b0;
    e[2].mode = 4;
    e[2].x = 255.0f;
    e[2].y = 0.0f;
    e[2].z = 0.0f;
    e[3].layer = 1;
    e[3].flags = 0x15;
    e[3].tex = base + 0x1b0;
    e[3].mode = 0x4000;
    e[3].x = 1.1f;
    e[3].y = 0.0f;
    e[3].z = 0.0f;
    e[4].layer = 2;
    e[4].flags = 0x15;
    e[4].tex = base + 0x1b0;
    e[4].mode = 4;
    e[4].x = 0.0f;
    e[4].y = 0.0f;
    e[4].z = 0.0f;
    e[5].layer = 2;
    e[5].flags = 0x15;
    e[5].tex = base + 0x1b0;
    e[5].mode = 0x4000;
    e[5].x = 1.1f;
    e[5].y = 0.0f;
    e[5].z = 0.0f;
    buf.v58 = 0;
    buf.ctx = sourceObj;
    buf.v44 = variant;
    buf.pos[0] = 0.0f;
    buf.pos[1] = 0.0f;
    buf.pos[2] = 0.0f;
    buf.col[0] = 0.0f;
    buf.col[1] = 0.0f;
    buf.col[2] = 0.0f;
    buf.scale = 1.2f;
    buf.v40 = 2;
    buf.v3c = 7;
    buf.v59 = 0xe;
    buf.v5a = 0;
    buf.v5b = 0x1e;
    buf.count = (GfxCmd*)((u8*)e + 0x90) - e;
    buf.hw[0] = *(s16*)(base + 0x1f8);
    buf.hw[1] = *(s16*)(base + 0x1fa);
    buf.hw[2] = *(s16*)(base + 0x1fc);
    buf.hw[3] = *(s16*)(base + 0x1fe);
    buf.hw[4] = *(s16*)(base + 0x200);
    buf.hw[5] = *(s16*)(base + 0x202);
    buf.hw[6] = *(s16*)(base + 0x204);
    buf.cmds = (GfxCmd*)((u8*)&buf + 0x60);
    buf.flags = 0xc0104c0;
    buf.flags |= flags;
    if ((buf.flags & 1) != 0)
    {
        if ((u32)sourceObj != 0)
        {
            GameObject* obj = (GameObject*)sourceObj;
            buf.pos[0] += obj->anim.localPosX;
            buf.pos[1] += obj->anim.localPosY;
            buf.pos[2] += obj->anim.localPosZ;
        }
        else
        {
            PartFxSpawnParams* params = (PartFxSpawnParams*)posSource;
            buf.pos[0] += params->posX;
            buf.pos[1] += params->posY;
            buf.pos[2] += params->posZ;
        }
    }
    (*gModgfxInterface)->spawnEffect(&buf, 0, 0x15, (u8*)(int)lbl_80317260, 0x18, base + 0xd4, DLL93_EFFECT_ID, 0);
}

void dll_93_func01_nop(void)
{
}

void dll_93_func00_nop(void)
{
}

u32 lbl_80317260[130] = {
    0x00000000, 0x03e80000, 0x00ff0362, 0x000001f4, 0x000b00ff, 0x03620000, 0xfe0c0016, 0x00ff0000, 0x0000fc18,
    0x002000ff, 0xfc9e0000, 0xfe0c0016, 0x00fffc9e, 0x000001f4, 0x000b00ff, 0x00000000, 0x03e80000, 0x00ff0000,
    0x0bb803e8, 0x0000007f, 0x03620bb8, 0x01f4000b, 0x007f0362, 0x0bb8fe0c, 0x0016007f, 0x00000bb8, 0xfc180020,
    0x007ffc9e, 0x0bb8fe0c, 0x0016007f, 0xfc9e0bb8, 0x01f4000b, 0x007f0000, 0x0bb803e8, 0x0000007f, 0x00001770,
    0x03e80000, 0x00000362, 0x177001f4, 0x000b0000, 0x03621770, 0xfe0c0016, 0x00000000, 0x1770fc18, 0x00200000,
    0xfc9e1770, 0xfe0c0016, 0x0000fc9e, 0x177001f4, 0x000b0000, 0x00001770, 0x03e80000, 0x00000000, 0x00000001,
    0x00080000, 0x00080007, 0x00010002, 0x00090001, 0x00090008, 0x00020003, 0x000a0002, 0x000a0009, 0x00030004,
    0x000b0003, 0x000b000a, 0x00040005, 0x000c0004, 0x000c000b, 0x00050006, 0x000d0005, 0x000d000c, 0x00070008,
    0x000f0007, 0x000f000e, 0x00080009, 0x00100008, 0x0010000f, 0x0009000a, 0x00110009, 0x00110010, 0x000a000b,
    0x0012000a, 0x00120011, 0x000b000c, 0x0013000b, 0x00130012, 0x000c000d, 0x0014000c, 0x00140013, 0x00000001,
    0x00020003, 0x00040005, 0x00060000, 0x00070008, 0x0009000a, 0x000b000c, 0x000d0000, 0x000e000f, 0x00100011,
    0x00120013, 0x00140000, 0x00000001, 0x00020003, 0x00040005, 0x0006000e, 0x000f0010, 0x00110012, 0x00130014,
    0x00000001, 0x00020003, 0x00040005, 0x00060007, 0x00080009, 0x000a000b, 0x000c000d, 0x000e000f, 0x00100011,
    0x00120013, 0x00140000, 0x00070008, 0x0009000a, 0x000b000c, 0x000d000e, 0x000f0010, 0x00110012, 0x00130014,
    0x00000032, 0x00320000, 0x00000000, 0x00000000};
ObjectDescriptor4 dll_93_funcs = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_4_SLOTS,
    (ObjectDescriptorCallback)dll_93_func00_nop,
    (ObjectDescriptorCallback)dll_93_func01_nop,
    0,
    (ObjectDescriptorCallback)dll_93_func03,
};
