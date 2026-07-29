/*
 * dlla9func0 (DLL 169 / 0xA9) - a modgfx pickup/effect spawner.
 *
 * dll_A9_func03 builds a stack command buffer of GfxCmd primitives (two
 * variant layouts selected by extraArgs) plus a per-effect header (colour,
 * position, scale, hardware-state words copied from the asset table at
 * lbl_80319028) and hands it to gModgfxInterface->spawnEffect. When flag bit
 * 0 is set the effect is positioned at the source object's world position
 * (sourceObj + 0x18..0x20) or, lacking a source object, at the spawn-param
 * packet's position (posSource + 0xc..0x14). func00/func01 are the DLL's
 * unused entry-point stubs.
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/modgfx_types.h"
#include "main/dll/partfx_interface.h"
#include "dlls/object_descriptor.h"

/* effect id spawned by this DLL's modgfx emitter (spawnEffect textureAssetId arg). */
#define DLLA9_EFFECT_ID 0x586

extern u8 lbl_80319028[];

void dll_A9_func03(GameObject* sourceObj, int variant, u8* posSource, u32 flags, u32 modelId, u8* extraArgs)
{
    ModgfxSpawnPacket buf;
    u8* tab = (u8*)(int)lbl_80319028;
    f32 scaleX;
    GfxCmd* e;
    GfxCmd* p;
    u32 effectFlags;
    f32 originOffset = 0.0f;

    if (extraArgs != 0)
    {
        scaleX = -2.0f;
    }
    else
    {
        scaleX = 2.0f;
    }
    e = buf.entries;
    e[0].layer = 0;
    e[0].flags = 0xe;
    e[0].tex = &tab[0xf4];
    e[0].mode = 4;
    e[0].x = originOffset;
    e[0].y = originOffset;
    e[0].z = originOffset;
    if (extraArgs != 0)
    {
        e[1].layer = 0;
        e[1].flags = 7;
        e[1].tex = &tab[0xd4];
        e[1].mode = 2;
        e[1].x = 0.8f;
        e[1].y = 0.006f;
        e[1].z = 0.8f;
        e[2].layer = 0;
        e[2].flags = 7;
        e[2].tex = &tab[0xe4];
        e[2].mode = 2;
        e[2].x = 1.5f;
        e[2].y = 0.006f;
        e[2].z = 1.5f;
        p = e + 3;
    }
    else
    {
        e[1].layer = 0;
        e[1].flags = 7;
        e[1].tex = &tab[0xd4];
        e[1].mode = 2;
        e[1].x = 0.8f;
        e[1].y = 0.028f;
        e[1].z = 0.8f;
        e[2].layer = 0;
        e[2].flags = 7;
        e[2].tex = &tab[0xe4];
        e[2].mode = 2;
        e[2].x = 1.2f;
        e[2].y = 0.028f;
        e[2].z = 1.2f;
        p = e + 3;
    }
    p[0].layer = 1;
    p[0].flags = 0xe;
    p[0].tex = &tab[0xf4];
    p[0].mode = 2;
    p[0].x = 1.0f;
    p[0].y = 130.0f;
    p[0].z = 1.0f;
    p[1].layer = 1;
    p[1].flags = 0xe;
    p[1].tex = &tab[0xf4];
    p[1].mode = 4;
    p[1].x = 255.0f;
    p[1].y = originOffset;
    p[1].z = originOffset;
    p[2].layer = 1;
    p[2].flags = 0xe;
    p[2].tex = &tab[0xf4];
    p[2].mode = 0x4000;
    p[2].x = scaleX;
    p[2].y = originOffset;
    p[2].z = originOffset;
    p[3].layer = 2;
    p[3].flags = 0xe;
    p[3].tex = &tab[0xf4];
    p[3].mode = 0x4000;
    p[3].x = scaleX;
    p[3].y = originOffset;
    p[3].z = originOffset;
    p[4].layer = 3;
    p[4].flags = 1;
    p[4].tex = NULL;
    p[4].mode = 0x2000;
    p[4].x = originOffset;
    p[4].y = originOffset;
    p[4].z = originOffset;
    p[5].layer = 4;
    p[5].flags = 0xe;
    p[5].tex = &tab[0xf4];
    p[5].mode = 4;
    p[5].x = originOffset;
    p[5].y = originOffset;
    p[5].z = originOffset;
    p[6].layer = 4;
    p[6].flags = 0xe;
    p[6].tex = &tab[0xf4];
    p[6].mode = 0x4000;
    p[6].x = scaleX;
    p[6].y = originOffset;
    p[6].z = originOffset;
    p[7].layer = 4;
    p[7].flags = 0xe;
    p[7].tex = &tab[0xf4];
    p[7].mode = 2;
    p[7].x = 1.0f;
    p[7].y = 0.01f;
    p[7].z = 1.0f;

    buf.v58 = 0;
    buf.ctx = (int)sourceObj;
    buf.v44 = variant;
    buf.pos[0] = originOffset;
    buf.pos[1] = originOffset;
    buf.pos[2] = originOffset;
    buf.col[0] = originOffset;
    buf.col[1] = originOffset;
    buf.col[2] = originOffset;
    buf.scale = 1.0f;
    buf.v40 = 1;
    buf.v3c = 0;
    buf.v59 = 0xe;
    buf.v5a = 0;
    buf.v5b = 0x1e;
    buf.count = &p[8] - e;
    buf.hw[0] = *(s16*)&tab[0x110];
    buf.hw[1] = *(s16*)&tab[0x112];
    buf.hw[2] = *(s16*)&tab[0x114];
    buf.hw[3] = *(s16*)&tab[0x116];
    buf.hw[4] = *(s16*)&tab[0x118];
    buf.hw[5] = *(s16*)&tab[0x11a];
    buf.hw[6] = *(s16*)&tab[0x11c];
    buf.cmds = (GfxCmd*)((u8*)&buf + 0x60);
    effectFlags = 0xc010040;
    buf.flags = effectFlags;
    effectFlags |= flags;
    buf.flags = effectFlags;
    if (effectFlags & 1)
    {
        if (sourceObj != 0)
        {
            buf.pos[0] += (sourceObj)->anim.worldPosX;
            buf.pos[1] += (sourceObj)->anim.worldPosY;
            buf.pos[2] += (sourceObj)->anim.worldPosZ;
        }
        else
        {
            buf.pos[0] += ((PartFxSpawnParams*)posSource)->posX;
            buf.pos[1] += ((PartFxSpawnParams*)posSource)->posY;
            buf.pos[2] += ((PartFxSpawnParams*)posSource)->posZ;
        }
    }
    (*gModgfxInterface)->spawnEffect(&buf, 0, 0xe, (u8*)(int)lbl_80319028, 0xc, &tab[0x8c], DLLA9_EFFECT_ID, 0);
}

void dll_A9_func01_nop(void)
{
}

void dll_A9_func00_nop(void)
{
}

u8 lbl_80319028[288] = {
    0,   0,   0,   0,   3, 232, 0, 0,   0,   0,   3,   98,  0, 0,   1,   244, 0,   31,  0,   0,   3,   98,  0,   0,
    254, 12,  0,   63,  0, 0,   0, 0,   0,   0,   252, 24,  0, 95,  0,   0,   252, 158, 0,   0,   254, 12,  0,   127,
    0,   0,   252, 158, 0, 0,   1, 244, 0,   158, 0,   0,   0, 0,   0,   0,   3,   232, 0,   188, 0,   0,   0,   0,
    3,   232, 3,   232, 0, 0,   0, 31,  3,   98,  3,   232, 1, 244, 0,   31,  0,   31,  3,   98,  3,   232, 254, 12,
    0,   63,  0,   31,  0, 0,   3, 232, 252, 24,  0,   95,  0, 31,  252, 158, 3,   232, 254, 12,  0,   127, 0,   31,
    252, 158, 3,   232, 1, 244, 0, 158, 0,   31,  0,   0,   3, 232, 3,   232, 0,   188, 0,   31,  0,   0,   0,   1,
    0,   8,   0,   0,   0, 8,   0, 7,   0,   1,   0,   2,   0, 9,   0,   1,   0,   9,   0,   8,   0,   2,   0,   3,
    0,   10,  0,   2,   0, 10,  0, 9,   0,   3,   0,   4,   0, 11,  0,   3,   0,   11,  0,   10,  0,   4,   0,   5,
    0,   12,  0,   4,   0, 12,  0, 11,  0,   5,   0,   6,   0, 13,  0,   5,   0,   13,  0,   12,  0,   0,   0,   1,
    0,   2,   0,   3,   0, 4,   0, 5,   0,   6,   0,   0,   0, 7,   0,   8,   0,   9,   0,   10,  0,   11,  0,   12,
    0,   13,  0,   0,   0, 0,   0, 1,   0,   2,   0,   3,   0, 4,   0,   5,   0,   6,   0,   7,   0,   8,   0,   9,
    0,   10,  0,   11,  0, 12,  0, 13,  0,   0,   0,   150, 0, 250, 0,   1,   0,   50,  0,   0,   0,   0,   0,   0};
ObjectDescriptor4 dll_A9_funcs = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_4_SLOTS,
    (ObjectDescriptorCallback)dll_A9_func00_nop,
    (ObjectDescriptorCallback)dll_A9_func01_nop,
    0,
    (ObjectDescriptorCallback)dll_A9_func03,
};
