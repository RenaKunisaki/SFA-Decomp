/*
 * DLL 136 / 0x88 - one of the foodbag func03 model-effect spawners
 * covering DLLs 124-144 / 0x7C-0x90. dll_88_func03 fills a stack FbBuf
 * command list (9 FbCmd model layers built from the lbl_80316240 asset
 * block) plus the per-effect color/position/scale/flags header, folds
 * the caller's flags in, and when bit 0 is set offsets the spawn
 * position from either the source object (+0x18..0x20) or an explicit
 * position source (+0xC..0x14). The list is handed to the modgfx
 * interface's spawnEffect. func00/func01 are nops.
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/partfx_interface.h"
#include "game/objects/object.h"
#include "main/dll/fb_cmd.h"
#include "dlls/object_descriptor.h"

/* effect id spawned by this DLL's modgfx emitter (spawnEffect textureAssetId arg). */
#define DLL88_EFFECT_ID 0x205

u8 lbl_80316240[512] = {
    254, 12,  1,   244, 0,   0,   0,   0,   0,   31,  255, 6,   1,   244, 255, 176, 0,   7,   0,   31,  0,   0,   1,
    244, 255, 136, 0,   16,  0,   31,  0,   250, 1,   244, 255, 176, 0,   24,  0,   31,  1,   244, 1,   244, 0,   0,
    0,   31,  0,   31,  254, 12,  0,   250, 255, 176, 0,   0,   0,   24,  255, 6,   0,   250, 255, 96,  0,   7,   0,
    24,  0,   0,   0,   250, 255, 56,  0,   16,  0,   24,  0,   250, 0,   250, 255, 96,  0,   24,  0,   24,  1,   244,
    0,   250, 255, 176, 0,   31,  0,   24,  254, 12,  0,   0,   255, 136, 0,   0,   0,   16,  255, 6,   0,   0,   255,
    56,  0,   7,   0,   16,  0,   0,   0,   0,   255, 16,  0,   16,  0,   16,  0,   250, 0,   0,   255, 56,  0,   24,
    0,   16,  1,   244, 0,   0,   255, 136, 0,   31,  0,   16,  254, 12,  255, 6,   255, 176, 0,   0,   0,   7,   255,
    6,   255, 6,   255, 96,  0,   7,   0,   7,   0,   0,   255, 6,   255, 56,  0,   16,  0,   7,   0,   250, 255, 6,
    255, 96,  0,   24,  0,   7,   1,   244, 255, 6,   255, 176, 0,   31,  0,   7,   254, 12,  254, 12,  0,   0,   0,
    0,   0,   0,   255, 6,   254, 12,  255, 176, 0,   7,   0,   0,   0,   0,   254, 12,  255, 136, 0,   16,  0,   0,
    0,   250, 254, 12,  255, 176, 0,   24,  0,   0,   1,   244, 254, 12,  0,   0,   0,   31,  0,   0,   0,   0,   0,
    5,   0,   1,   0,   0,   0,   5,   0,   6,   0,   1,   0,   6,   0,   2,   0,   1,   0,   6,   0,   7,   0,   2,
    0,   7,   0,   3,   0,   2,   0,   7,   0,   8,   0,   3,   0,   8,   0,   4,   0,   3,   0,   8,   0,   9,   0,
    4,   0,   10,  0,   6,   0,   5,   0,   10,  0,   11,  0,   6,   0,   11,  0,   7,   0,   6,   0,   11,  0,   12,
    0,   7,   0,   12,  0,   8,   0,   7,   0,   12,  0,   13,  0,   8,   0,   13,  0,   9,   0,   8,   0,   13,  0,
    14,  0,   9,   0,   15,  0,   11,  0,   10,  0,   15,  0,   16,  0,   11,  0,   16,  0,   12,  0,   11,  0,   16,
    0,   17,  0,   12,  0,   17,  0,   13,  0,   12,  0,   17,  0,   18,  0,   13,  0,   18,  0,   14,  0,   13,  0,
    18,  0,   19,  0,   14,  0,   20,  0,   16,  0,   15,  0,   20,  0,   21,  0,   16,  0,   21,  0,   17,  0,   16,
    0,   21,  0,   22,  0,   17,  0,   22,  0,   18,  0,   17,  0,   22,  0,   23,  0,   18,  0,   23,  0,   19,  0,
    18,  0,   23,  0,   24,  0,   19,  0,   0,   0,   1,   0,   2,   0,   3,   0,   4,   0,   5,   0,   6,   0,   7,
    0,   8,   0,   9,   0,   10,  0,   11,  0,   12,  0,   13,  0,   14,  0,   15,  0,   16,  0,   17,  0,   18,  0,
    19,  0,   20,  0,   21,  0,   22,  0,   23,  0,   24,  0,   0,   0,   0,   0,   30,  0,   30,  0,   80,  0,   0,
    0,   0,   0,   0,   0,   0};

void dll_88_func03(int sourceObj, int variant, int posSource, u32 flags)
{
    FbBuf buf;
    u8* base = (u8*)(int)lbl_80316240;
    FbCmd* e = buf.entries;
    f32 originOffset = 0.0f;

    e[0].layer = 0;
    e[0].flags = 0x19;
    e[0].tex = base + 0x1bc;
    e[0].mode = 2;
    e[0].x = 20.7f;
    e[0].y = 20.7f;
    e[0].z = 20.7f;
    e[1].layer = 0;
    e[1].flags = 0x19;
    e[1].tex = base + 0x1bc;
    e[1].mode = 0x80;
    e[1].x = originOffset;
    e[1].y = originOffset;
    e[1].z = originOffset;
    e[2].layer = 0;
    e[2].flags = 0x7a;
    e[2].tex = 0;
    e[2].mode = 0x10000;
    e[2].x = originOffset;
    e[2].y = originOffset;
    e[2].z = originOffset;
    e[3].layer = 0;
    e[3].flags = 0x19;
    e[3].tex = base + 0x1bc;
    e[3].mode = 4;
    e[3].x = originOffset;
    e[3].y = originOffset;
    e[3].z = originOffset;
    e[4].layer = 1;
    e[4].flags = 0x19;
    e[4].tex = base + 0x1bc;
    e[4].mode = 4;
    e[4].x = 255.0f;
    e[4].y = originOffset;
    e[4].z = originOffset;
    e[5].layer = 1;
    e[5].flags = 0x19;
    e[5].tex = base + 0x1bc;
    e[5].mode = 2;
    e[5].x = 2.0f;
    e[5].y = 2.0f;
    e[5].z = 1.0f;
    e[6].layer = 2;
    e[6].flags = 0x19;
    e[6].tex = base + 0x1bc;
    e[6].mode = 2;
    e[6].x = 1.5f;
    e[6].y = 1.5f;
    e[6].z = 1.0f;
    e[7].layer = 3;
    e[7].flags = 0x19;
    e[7].tex = base + 0x1bc;
    e[7].mode = 2;
    e[7].x = 1.5f;
    e[7].y = 1.5f;
    e[7].z = 1.0f;
    e[8].layer = 3;
    e[8].flags = 0x19;
    e[8].tex = base + 0x1bc;
    e[8].mode = 4;
    e[8].x = originOffset;
    e[8].y = originOffset;
    e[8].z = originOffset;
    buf.v58 = 0;
    buf.ctx = sourceObj;
    buf.v44 = variant;
    buf.pos[0] = originOffset;
    buf.pos[1] = originOffset;
    buf.pos[2] = originOffset;
    buf.col[0] = originOffset;
    buf.col[1] = originOffset;
    buf.col[2] = originOffset;
    buf.scale = 1.0f;
    buf.v40 = 1;
    buf.v3c = 25;
    buf.v59 = 0x19;
    buf.v5a = 0xff;
    buf.v5b = 16;
    buf.flags = 0x4000480;
    buf.count = (FbCmd*)((u8*)e + 216) - e;
    buf.hw[0] = *(s16*)(base + 0x1f0);
    buf.hw[1] = *(s16*)(base + 0x1f2);
    buf.hw[2] = *(s16*)(base + 0x1f4);
    buf.hw[3] = *(s16*)(base + 0x1f6);
    buf.hw[4] = *(s16*)(base + 0x1f8);
    buf.hw[5] = *(s16*)(base + 0x1fa);
    buf.hw[6] = *(s16*)(base + 0x1fc);
    buf.cmds = e;
    buf.flags |= flags;
    if ((buf.flags & 1) != 0)
    {
        if ((u32)sourceObj != 0)
        {
            GameObject* obj = (GameObject*)sourceObj;
            buf.pos[0] += obj->anim.worldPosX;
            buf.pos[1] += obj->anim.worldPosY;
            buf.pos[2] += obj->anim.worldPosZ;
        }
        else
        {
            PartFxSpawnParams* params = (PartFxSpawnParams*)posSource;
            buf.pos[0] += params->posX;
            buf.pos[1] += params->posY;
            buf.pos[2] += params->posZ;
        }
    }
    (*gModgfxInterface)->spawnEffect(&buf, 0, 0x19, (u8*)(int)lbl_80316240, 0x20, base + 0xfc, DLL88_EFFECT_ID, 0);
}

void dll_88_func01_nop(void)
{
}

void dll_88_func00_nop(void)
{
}

ObjectDescriptor4 dll_88_funcs = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_4_SLOTS,
    (ObjectDescriptorCallback)dll_88_func00_nop,
    (ObjectDescriptorCallback)dll_88_func01_nop,
    0,
    (ObjectDescriptorCallback)dll_88_func03,
};
