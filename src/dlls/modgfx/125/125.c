/*
 * DLL 125 / 0x7D - one entry in the foodbag effect-DLL family
 * (DLLs 124-144 / 0x7C-0x90). dll_7D_func03 builds a 10-entry FbBuf model-graphics
 * command list from sub-textures of the global texture blob lbl_80315030,
 * scales the second sprite by an optional caller scale (scaleOverride), positions
 * the effect from either the source object (flags bit 0 set, sourceObj
 * non-null) or a separate position source, then spawns it through
 * gModgfxInterface->spawnEffect. lbl_803DD4B0 is a 0..4 rotating slot
 * counter advanced per spawn. The trailing func01/func00 nops are this
 * DLL's empty lifecycle stubs.
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/partfx_interface.h"
#include "game/objects/object.h"
#include "main/dll/fb_cmd.h"
#include "dlls/object_descriptor.h"

/* effect id spawned by this DLL's modgfx emitter (spawnEffect textureAssetId arg). */
#define DLL7D_EFFECT_ID 0x89

u8 lbl_80315030[520] = {
    0,   0,   13, 72,  0,  0,   0,   0,  0,   0,  3,   98,  11, 84,  0,  0,   0,   8,  0,   0,  3,   98,  7,  108, 0,
    0,   0,   15, 0,   0,  0,   0,   5,  120, 0,  0,   0,   31, 0,   0,  252, 158, 7,  108, 0,  0,   0,   15, 0,   0,
    252, 158, 11, 84,  0,  0,   0,   8,  0,   0,  0,   0,   13, 72,  0,  0,   0,   0,  0,   0,  0,   0,   13, 72,  9,
    196, 0,   0,  0,   31, 3,   98,  11, 84,  9,  196, 0,   8,  0,   31, 3,   98,  7,  108, 9,  196, 0,   15, 0,   31,
    0,   0,   5,  120, 9,  196, 0,   31, 0,   31, 252, 158, 7,  108, 9,  196, 0,   15, 0,   31, 252, 158, 11, 84,  9,
    196, 0,   8,  0,   31, 0,   0,   13, 72,  9,  196, 0,   0,  0,   31, 0,   0,   13, 72,  23, 112, 0,   0,  0,   63,
    3,   98,  11, 84,  23, 112, 0,   8,  0,   63, 3,   98,  7,  108, 23, 112, 0,   15, 0,   63, 0,   0,   5,  120, 23,
    112, 0,   31, 0,   63, 252, 158, 7,  108, 23, 112, 0,   15, 0,   63, 252, 158, 11, 84,  23, 112, 0,   8,  0,   63,
    0,   0,   13, 72,  23, 112, 0,   0,  0,   63, 0,   0,   0,  0,   0,  8,   0,   1,  0,   0,  0,   7,   0,  8,   0,
    1,   0,   9,  0,   2,  0,   1,   0,  8,   0,  9,   0,   2,  0,   10, 0,   3,   0,  2,   0,  9,   0,   10, 0,   3,
    0,   11,  0,  4,   0,  3,   0,   10, 0,   11, 0,   4,   0,  12,  0,  5,   0,   4,  0,   11, 0,   12,  0,  5,   0,
    13,  0,   6,  0,   5,  0,   12,  0,  13,  0,  7,   0,   15, 0,   8,  0,   7,   0,  14,  0,  15,  0,   8,  0,   16,
    0,   9,   0,  8,   0,  15,  0,   16, 0,   9,  0,   17,  0,  10,  0,  9,   0,   16, 0,   17, 0,   10,  0,  18,  0,
    11,  0,   10, 0,   17, 0,   18,  0,  11,  0,  19,  0,   12, 0,   11, 0,   18,  0,  19,  0,  12,  0,   20, 0,   13,
    0,   12,  0,  19,  0,  20,  0,   0,  0,   1,  0,   2,   0,  3,   0,  4,   0,   5,  0,   6,  0,   0,   0,  7,   0,
    8,   0,   9,  0,   10, 0,   11,  0,  12,  0,  13,  0,   0,  0,   14, 0,   15,  0,  16,  0,  17,  0,   18, 0,   19,
    0,   20,  0,  0,   0,  0,   0,   1,  0,   2,  0,   3,   0,  4,   0,  5,   0,   6,  0,   14, 0,   15,  0,  16,  0,
    17,  0,   18, 0,   19, 0,   20,  0,  0,   0,  1,   0,   2,  0,   3,  0,   4,   0,  5,   0,  6,   0,   7,  0,   8,
    0,   9,   0,  10,  0,  11,  0,   12, 0,   13, 0,   14,  0,  15,  0,  16,  0,   17, 0,   18, 0,   19,  0,  20,  0,
    0,   0,   7,  0,   8,  0,   9,   0,  10,  0,  11,  0,   12, 0,   13, 0,   14,  0,  15,  0,  16,  0,   17, 0,   18,
    0,   19,  0,  20,  0,  0,   0,   0,  0,   15, 0,   45,  0,  0,   0,  0,   0,   0,  0,   0};
int lbl_803DD4B0;

s16 dll_7D_func03(int sourceObj, int variant, int posSource, u32 flags, u32 arg5, f32* scaleOverride)
{
    s16 ret;
    FbBuf buf;
    u8* base = (u8*)(int)lbl_80315030;
    f32 scale = 1.0f;
    FbCmd* entry;
    if (scaleOverride != NULL)
    {
        scale = *scaleOverride;
    }
    entry = buf.entries;
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
    entry[1].y = entry[1].x = 0.15f * scale;
    entry[1].z = 0.9f * scale;
    entry[2].layer = 1;
    entry[2].flags = 7;
    entry[2].tex = base + 0x184;
    entry[2].mode = 2;
    entry[2].x = 3.0f;
    entry[2].y = 3.0f;
    entry[2].z = 1.0f;
    entry[3].layer = 2;
    entry[3].flags = 7;
    entry[3].tex = base + 0x164;
    entry[3].mode = 4;
    entry[3].x = 255.0f;
    entry[3].y = 0.0f;
    entry[3].z = 0.0f;
    entry[4].layer = 2;
    entry[4].flags = 7;
    entry[4].tex = base + 0x174;
    entry[4].mode = 4;
    entry[4].x = 255.0f;
    entry[4].y = 0.0f;
    entry[4].z = 0.0f;
    entry[5].layer = 2;
    entry[5].flags = 7;
    entry[5].tex = base + 0x174;
    entry[5].mode = 2;
    entry[5].x = 2.0f;
    entry[5].y = 2.0f;
    entry[5].z = 1.0f;
    entry[6].layer = 2;
    entry[6].flags = 0x15;
    entry[6].tex = base + 0x1b0;
    entry[6].mode = 0x4000;
    entry[6].x = 4.0f;
    entry[6].y = -6.0f;
    entry[6].z = 0.0f;
    entry[7].layer = 3;
    entry[7].flags = 0x15;
    entry[7].tex = base + 0x1b0;
    entry[7].mode = 0x4000;
    entry[7].x = 4.0f;
    entry[7].y = -6.0f;
    entry[7].z = 0.0f;
    entry[8].layer = 3;
    entry[8].flags = 7;
    entry[8].tex = base + 0x164;
    entry[8].mode = 4;
    entry[8].x = 0.0f;
    entry[8].y = 0.0f;
    entry[8].z = 0.0f;
    entry[9].layer = 3;
    entry[9].flags = 7;
    entry[9].tex = base + 0x174;
    entry[9].mode = 4;
    entry[9].x = 0.0f;
    entry[9].y = 0.0f;
    entry[9].z = 0.0f;
    buf.v58 = 0;
    buf.ctx = sourceObj;
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
    buf.v5b = 0xa;
    buf.count = (FbCmd*)((u8*)entry + 0xf0) - entry;
    buf.hw[0] = *(s16*)(base + 0x1f8);
    buf.hw[1] = *(s16*)(base + 0x1fa);
    buf.hw[2] = *(s16*)(base + 0x1fc);
    buf.hw[3] = *(s16*)(base + 0x1fe);
    buf.hw[4] = *(s16*)(base + 0x200);
    buf.hw[5] = *(s16*)(base + 0x202);
    buf.hw[6] = *(s16*)(base + 0x204);
    buf.cmds = entry;
    buf.flags = 0xc010080;
    buf.flags |= flags;
    if ((buf.flags & 1) != 0)
    {
        if ((u32)sourceObj != 0)
        {
            buf.pos[0] += ((GameObject*)(sourceObj))->anim.worldPosX;
            buf.pos[1] += ((GameObject*)(sourceObj))->anim.worldPosY;
            buf.pos[2] += ((GameObject*)(sourceObj))->anim.worldPosZ;
        }
        else
        {
            buf.pos[0] += ((PartFxSpawnParams*)posSource)->posX;
            buf.pos[1] += ((PartFxSpawnParams*)posSource)->posY;
            buf.pos[2] += ((PartFxSpawnParams*)posSource)->posZ;
        }
    }
    ret =
        (*gModgfxInterface)->spawnEffect(&buf, 0, 0x15, (u8*)(int)lbl_80315030, 0x18, base + 0xd4, DLL7D_EFFECT_ID, 0);
    lbl_803DD4B0 += 1;
    if (lbl_803DD4B0 == 5)
    {
        lbl_803DD4B0 = 0;
    }
    return ret;
}

void dll_7D_func01_nop(void)
{
}

void dll_7D_func00_nop(void)
{
}

ObjectDescriptor4 dll_7D_funcs = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_4_SLOTS,
    (ObjectDescriptorCallback)dll_7D_func00_nop,
    (ObjectDescriptorCallback)dll_7D_func01_nop,
    0,
    (ObjectDescriptorCallback)dll_7D_func03,
};
