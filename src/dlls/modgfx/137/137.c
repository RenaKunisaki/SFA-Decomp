/*
 * DLL 137 / 0x89 - one entry of the foodbag/modgfx spawn-effect
 * family covering DLLs 124-144 / 0x7C-0x90. dll_89_func03 fills a stack
 * FbBuf command list with ten FbCmd layers (texture/mode/xyz from float
 * literals and the lbl_80316460 resource block) and hands
 * it to gModgfxInterface->spawnEffect (effect 0x1fd). When flag bit 0 is
 * requested the effect is positioned from sourceObj's transform (+0x18)
 * or, when none, from posSource (+0xc). The two _nop entries are empty
 * DLL entry slots.
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/partfx_interface.h"
#include "game/objects/object.h"
#include "main/dll/fb_cmd.h"
#include "dlls/object_descriptor.h"

u8 lbl_803DB908[8] = {0};

/* effect id spawned by this DLL's modgfx emitter (spawnEffect textureAssetId arg). */
#define DLL89_EFFECT_ID 0x1fd

u8 lbl_80316460[464] = {
    0, 0,   248, 48,  0, 0,   0, 0,   0,   0,  3, 232, 3, 232, 0,   0,   0, 32,  0,   32, 2, 195, 3, 232, 253, 61,
    0, 0,   0,   32,  0, 0,   3, 232, 252, 24, 0, 32,  0, 32,  253, 61,  3, 232, 253, 61, 0, 0,   0, 32,  252, 24,
    3, 232, 0,   0,   0, 32,  0, 32,  253, 61, 3, 232, 2, 195, 0,   0,   0, 32,  0,   0,  3, 232, 3, 232, 0,   32,
    0, 32,  2,   195, 3, 232, 2, 195, 0,   0,  0, 32,  3, 232, 3,   232, 0, 0,   0,   32, 0, 32,  0, 0,   0,   0,
    0, 0,   0,   0,   0, 0,   0, 0,   0,   0,  0, 0,   0, 0,   0,   0,   0, 0,   0,   0,  0, 0,   0, 0,   0,   0,
    0, 0,   0,   0,   0, 0,   0, 0,   0,   0,  0, 0,   0, 0,   0,   0,   0, 0,   0,   0,  0, 0,   0, 0,   0,   0,
    0, 0,   0,   0,   0, 0,   0, 0,   0,   0,  0, 0,   0, 0,   0,   0,   0, 0,   0,   0,  0, 0,   0, 0,   0,   0,
    0, 0,   0,   0,   0, 0,   0, 0,   0,   0,  0, 0,   0, 0,   0,   0,   0, 0,   0,   0,  0, 0,   0, 0,   0,   0,
    0, 0,   0,   0,   0, 0,   0, 0,   0,   0,  0, 0,   0, 0,   0,   0,   0, 0,   0,   0,  0, 0,   0, 0,   0,   0,
    0, 0,   0,   0,   0, 0,   0, 0,   0,   0,  0, 0,   0, 0,   0,   0,   0, 0,   0,   0,  0, 0,   0, 0,   0,   0,
    0, 0,   0,   0,   0, 0,   0, 0,   0,   0,  0, 0,   0, 0,   0,   0,   0, 0,   0,   0,  0, 0,   0, 0,   0,   0,
    0, 0,   0,   0,   0, 0,   0, 0,   0,   0,  0, 0,   0, 0,   0,   0,   0, 0,   0,   0,  0, 0,   0, 0,   0,   0,
    0, 0,   0,   0,   0, 0,   0, 0,   0,   0,  0, 0,   0, 0,   0,   0,   0, 0,   0,   0,  0, 0,   0, 0,   0,   0,
    0, 0,   0,   0,   0, 0,   0, 0,   0,   0,  0, 0,   0, 0,   0,   0,   0, 0,   0,   0,  0, 0,   0, 0,   0,   1,
    0, 2,   0,   0,   0, 2,   0, 3,   0,   0,  0, 3,   0, 4,   0,   0,   0, 4,   0,   5,  0, 0,   0, 5,   0,   6,
    0, 0,   0,   6,   0, 7,   0, 0,   0,   7,  0, 8,   0, 0,   0,   8,   0, 9,   0,   1,  0, 2,   0, 3,   0,   4,
    0, 5,   0,   6,   0, 7,   0, 8,   0,   9,  0, 0,   0, 0,   0,   1,   0, 2,   0,   3,  0, 4,   0, 5,   0,   6,
    0, 7,   0,   8,   0, 9,   0, 0,   0,   90, 0, 200, 0, 90,  0,   0,   0, 0,   0,   0,  0, 0};

void dll_89_func03(int sourceObj, int variant, int posSource, u32 flags)
{
    FbBuf buf;
    u8* base = (u8*)(int)lbl_80316460;
    FbCmd* e = buf.entries;

    e[0].layer = 0;
    e[0].flags = 10;
    e[0].tex = base + 0x1ac;
    e[0].mode = 2;
    e[0].x = 1.1f;
    e[0].y = 1.2f;
    e[0].z = 1.1f;
    e[1].layer = 0;
    e[1].flags = 10;
    e[1].tex = base + 0x1ac;
    e[1].mode = 4;
    e[1].x = 0.0f;
    e[1].y = 0.0f;
    e[1].z = 0.0f;
    e[2].layer = 0;
    e[2].flags = 0;
    e[2].tex = NULL;
    e[2].mode = 0x400000;
    e[2].x = 8.0f;
    e[2].y = 72.0f;
    e[2].z = 5.0f;
    e[3].layer = 1;
    e[3].flags = 10;
    e[3].tex = base + 0x1ac;
    e[3].mode = 0x4000;
    e[3].x = 1.0f;
    e[3].y = 1.0f;
    e[3].z = 0.0f;
    e[4].layer = 0;
    e[4].flags = 9;
    e[4].tex = base + 0x198;
    e[4].mode = 2;
    e[4].x = 32.1f;
    e[4].y = 1.2f;
    e[4].z = 32.1f;
    e[5].layer = 2;
    e[5].flags = 1;
    e[5].tex = lbl_803DB908;
    e[5].mode = 4;
    e[5].x = 255.0f;
    e[5].y = 0.0f;
    e[5].z = 0.0f;
    e[6].layer = 2;
    e[6].flags = 10;
    e[6].tex = base + 0x1ac;
    e[6].mode = 0x4000;
    e[6].x = 1.0f;
    e[6].y = 1.0f;
    e[6].z = 0.0f;
    e[7].layer = 3;
    e[7].flags = 10;
    e[7].tex = base + 0x1ac;
    e[7].mode = 0x4000;
    e[7].x = 1.0f;
    e[7].y = 1.0f;
    e[7].z = 0.0f;
    e[8].layer = 4;
    e[8].flags = 10;
    e[8].tex = base + 0x1ac;
    e[8].mode = 0x4000;
    e[8].x = 1.0f;
    e[8].y = 1.0f;
    e[8].z = 0.0f;
    e[9].layer = 4;
    e[9].flags = 10;
    e[9].tex = base + 0x1ac;
    e[9].mode = 4;
    e[9].x = 0.0f;
    e[9].y = 0.0f;
    e[9].z = 0.0f;
    buf.v58 = 0;
    buf.ctx = sourceObj;
    buf.v44 = variant;
    buf.pos[0] = 0.0f;
    buf.pos[1] = 0.0f;
    buf.pos[2] = 0.0f;
    buf.col[0] = 0.0f;
    buf.col[1] = 0.0f;
    buf.col[2] = 0.0f;
    buf.scale = 0.0f;
    buf.v40 = 1;
    buf.v3c = 10;
    buf.v59 = 10;
    buf.v5a = 0;
    buf.v5b = 16;
    buf.flags = 0x4000494;
    buf.count = (FbCmd*)((u8*)e + 240) - e;
    buf.hw[0] = *(s16*)(base + 0x1c0);
    buf.hw[1] = *(s16*)(base + 0x1c2);
    buf.hw[2] = *(s16*)(base + 0x1c4);
    buf.hw[3] = *(s16*)(base + 0x1c6);
    buf.hw[4] = *(s16*)(base + 0x1c8);
    buf.hw[5] = *(s16*)(base + 0x1ca);
    buf.hw[6] = *(s16*)(base + 0x1cc);
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
    (*gModgfxInterface)->spawnEffect(&buf, 0, 10, (u8*)(int)lbl_80316460, 8, base + 0x168, DLL89_EFFECT_ID, 0);
}

void dll_89_func01_nop(void)
{
}

void dll_89_func00_nop(void)
{
}

ObjectDescriptor4 dll_89_funcs = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_4_SLOTS,
    (ObjectDescriptorCallback)dll_89_func00_nop,
    (ObjectDescriptorCallback)dll_89_func01_nop,
    0,
    (ObjectDescriptorCallback)dll_89_func03,
};
