/*
 * DLL 117 / 0x75 - modgfx particle-spawn helper. dll_75_func03 emits a
 * variant-selected effect through gModgfxInterface; variant (0..8) picks the
 * effect parameters and optional hardware tuning. dll_75_func00_nop /
 * dll_75_func01_nop are empty entry-point stubs.
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/modgfx_types.h"
#include "main/dll/dll_0075_dll75func0.h"

s16 gModgfxFxHwTuning[8] = {0, 155, 200, 155, 0, 0, 0, 0};

s16 dll_75_func03(GameObject* sourceObj, int variant, PartFxSpawnParams* posSource, u32 flags, int owner, void* unused)
{
    ModgfxSpawnPacket buf;
    int fl;
    GfxCmd* entries;
    GfxCmd* e;
    f32 originOffset = 0.0f;
    f32 fa = 81.0f;
    f32 fb = 82.0f;
    fl = 100;
    if (variant == 0)
    {
        fl = 0x8c;
    }
    else if (variant == 1)
    {
        fa = 100.0f;
        fb = 101.0f;
        fl = 0x8c;
    }
    else if (variant == 2)
    {
        fa = 102.0f;
        fb = 103.0f;
        fl = 0x8c;
    }
    else if (variant == 3)
    {
        fa = 104.0f;
        fb = 105.0f;
        fl = 0x8c;
    }
    else if (variant == 4)
    {
        fa = 106.0f;
        fb = 107.0f;
        fl = 0x154;
    }
    else if (variant == 5)
    {
        fa = 111.0f;
        fb = 112.0f;
        fl = 0x280;
        gModgfxFxHwTuning[2] = 800;
    }
    else if (variant == 6)
    {
        fa = 125.0f;
        fb = 126.0f;
        fl = 100;
        gModgfxFxHwTuning[2] = 0x14;
    }
    else if (variant == 7)
    {
        fa = 133.0f;
        fb = 134.0f;
        fl = 200;
        gModgfxFxHwTuning[1] = 0x14;
        gModgfxFxHwTuning[2] = 0x14;
        gModgfxFxHwTuning[3] = 0x14;
    }
    else if (variant == 8)
    {
        fa = 157.0f;
        fb = 158.0f;
        fl = 0x41;
        gModgfxFxHwTuning[1] = 0x14;
        gModgfxFxHwTuning[2] = 0x14;
        gModgfxFxHwTuning[3] = 0x14;
    }
    entries = buf.entries;
    entries[0].layer = 0;
    entries[0].flags = fl;
    entries[0].tex = NULL;
    entries[0].mode = 0x20000000;
    entries[0].x = 999.0f;
    entries[0].y = fa;
    entries[0].z = fb;
    e = &entries[1];
    if (variant == 0)
    {
        e[0].layer = 0;
        e[0].flags = 0;
        e[0].tex = NULL;
        e[0].mode = 0x80000;
        e[0].x = originOffset;
        e[0].y = 200.0f;
        e[0].z = originOffset;
        e[1].layer = 1;
        e[1].flags = 0;
        e[1].tex = NULL;
        e[1].mode = 0x80000;
        e[1].x = originOffset;
        e[1].y = originOffset;
        e[1].z = originOffset;
        e[2].layer = 3;
        e[2].flags = 0;
        e[2].tex = NULL;
        e[2].mode = 0x80000;
        e[2].x = originOffset;
        e[2].y = 200.0f;
        e[2].z = originOffset;
        e += 3;
    }
    else if (variant == 6)
    {
        e[0].layer = 3;
        e[0].flags = 1;
        e[0].tex = NULL;
        e[0].mode = 0x2000;
        e[0].x = originOffset;
        e[0].y = originOffset;
        e[0].z = originOffset;
        e += 1;
    }
    else if (variant == 8)
    {
        e[0].layer = 3;
        e[0].flags = 1;
        e[0].tex = NULL;
        e[0].mode = 0x2000;
        e[0].x = originOffset;
        e[0].y = originOffset;
        e[0].z = originOffset;
        e += 1;
    }
    e[0].layer = 4;
    e[0].flags = 0;
    e[0].tex = NULL;
    e[0].mode = 0x20000000;
    e[0].x = 999.0f;
    e[0].y = fa;
    e[0].z = fb;
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
    buf.v40 = 0;
    buf.v3c = 0;
    buf.v59 = 0;
    buf.v5a = 0;
    buf.v5b = 0;
    buf.count = (e + 1) - entries;
    buf.hw[0] = gModgfxFxHwTuning[0];
    buf.hw[1] = gModgfxFxHwTuning[1];
    buf.hw[2] = gModgfxFxHwTuning[2];
    buf.hw[3] = gModgfxFxHwTuning[3];
    buf.hw[4] = gModgfxFxHwTuning[4];
    buf.hw[5] = gModgfxFxHwTuning[5];
    buf.hw[6] = gModgfxFxHwTuning[6];
    buf.cmds = (GfxCmd*)((u8*)&buf + 0x60);
    buf.flags = 0x10800;
    buf.flags |= flags;
    if ((buf.flags & 1) != 0)
    {
        if (sourceObj != 0)
        {
            buf.pos[0] = originOffset + sourceObj->anim.worldPosX;
            buf.pos[1] = originOffset + sourceObj->anim.worldPosY;
            buf.pos[2] = originOffset + sourceObj->anim.worldPosZ;
        }
        else
        {
            buf.pos[0] = originOffset + posSource->posX;
            buf.pos[1] = originOffset + posSource->posY;
            buf.pos[2] = originOffset + posSource->posZ;
        }
    }
    return (*gModgfxInterface)->spawnEffect(&buf, 0, 0, 0, 0, 0, 0, 0);
}

void dll_75_func01_nop(void)
{
}

void dll_75_func00_nop(void)
{
}

void* lbl_80314930[8] = {(void*)0x00000000, (void*)0x00000000, (void*)0x00000000, (void*)0x00030000,
                         dll_75_func00_nop, dll_75_func01_nop, (void*)0x00000000, dll_75_func03};
