/*
 * dlla6func0 (DLL 166 / 0xA6) - a modgfx effect spawner (sibling of DLL 0xA8).
 *
 * dll_A6_func03 builds a stack command buffer of GfxCmd primitives on the
 * stack, with a small variant-0/variant-1 prefix command, then a fixed set
 * of mode/layer commands (several randomised per spawn via randomGetRange),
 * plus a per-effect header (colour, position, scale, hardware-state words
 * copied from the asset table at gDllA6EffectHwWords) and hands it to
 * gModgfxInterface->spawnEffect. When flag bit 0 is set the effect is
 * positioned from the source object's world position and/or the spawn-param
 * packet's position (posSource + 0xc..0x14). func00/func01 are the DLL's
 * unused entry-point stubs.
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/modgfx_types.h"
#include "main/dll/partfx_interface.h"
#include "game/objects/object.h"
#include "main/dll/dll_00A6_dlla6func0.h"
#include "main/vecmath.h"

u8 lbl_803DB980[8] = {0, 0, 0, 1, 0, 2, 0, 0};
u8 gDllA6EffectTex[8] = {0, 0, 0, 1, 0, 2, 0, 0};

/* effect id spawned by this DLL's modgfx emitter (spawnEffect textureAssetId arg). */
#define DLLA6_EFFECT_ID 0x26a

extern u8 lbl_80318DF0[];
extern s16 gDllA6EffectHwWords[];

void dll_A6_func03(short* sourceObj, int variant, u8* posSource, u32 flags)
{
    ModgfxSpawnPacket buf;
    GfxCmd* p;
    GfxCmd* e = buf.entries;
    f32 zr;
    f32 yr;
    u32 fl;
    p = e;

    if (variant == 0)
    {
        p->layer = 0;
        p->flags = 3;
        p->tex = gDllA6EffectTex;
        p->mode = 8;
        p->x = (f32)(int)(randomGetRange(0, 0x1e) + 0xe1);
        p->y = (f32)(int)(randomGetRange(0, 0x14) + 0x87);
        p->z = (f32)(int)(randomGetRange(0, 0x14) + 0x41);
        p++;
    }
    else if (variant == 1)
    {
        p->layer = 0;
        p->flags = 3;
        p->tex = gDllA6EffectTex;
        p->mode = 8;
        p->y = p->x = (f32)(int)(randomGetRange(0, 0x5a) + 0x87);
        p->z = (f32)(int)(randomGetRange(0, 0x1e) + 0xe1);
        p++;
    }
    zr = (f32)randomGetRange(0, 0xfffe);
    yr = (f32)randomGetRange(-3000, -12000);
    p[0].layer = 0;
    p[0].flags = 0;
    p[0].tex = NULL;
    p[0].mode = 0x80;
    p[0].x = 0.0f;
    p[0].y = yr;
    p[0].z = zr;
    p[1].layer = 0;
    p[1].flags = 3;
    p[1].tex = gDllA6EffectTex;
    p[1].mode = 4;
    p[1].x = 0.0f;
    p[1].y = 0.0f;
    p[1].z = 0.0f;
    p[2].layer = 0;
    p[2].flags = 3;
    p[2].tex = gDllA6EffectTex;
    p[2].mode = 2;
    p[2].x = 1.0f;
    p[2].y = 0.01f * (f32)randomGetRange(0, 0x19) + 0.25f;
    p[2].z = 0.01f * (f32)randomGetRange(0, 10) + 0.4f;
    p[3].layer = 1;
    p[3].flags = 3;
    p[3].tex = gDllA6EffectTex;
    p[3].mode = 4;
    if (randomGetRange(0, 10) == 0)
    {
        p[3].x = 145.0f + (f32)randomGetRange(0, 0x1e);
    }
    else
    {
        p[3].x = 25.0f + (f32)randomGetRange(0, 10);
    }
    p[3].y = 0.0f;
    p[3].z = 0.0f;
    p[4].layer = 1;
    p[4].flags = 0;
    p[4].tex = NULL;
    p[4].mode = 0x80;
    p[4].x = 0.0f;
    p[4].y = 0.0f;
    p[4].z = (f32)randomGetRange(0, 0xfffe);
    p[5].layer = 1;
    p[5].flags = 3;
    p[5].tex = gDllA6EffectTex;
    p[5].mode = 2;
    p[5].x = 9.0f;
    p[5].y = 12.0f;
    p[5].z = 21.0f;
    p[6].layer = 2;
    p[6].flags = 0;
    p[6].tex = NULL;
    p[6].mode = 0x80;
    p[6].x = 0.0f;
    p[6].y = 0.0f;
    p[6].z = (f32)randomGetRange(0, 0xfffe);
    p[7].layer = 2;
    p[7].flags = 3;
    p[7].tex = gDllA6EffectTex;
    p[7].mode = 4;
    p[7].x = 0.0f;
    p[7].y = 0.0f;
    p[7].z = 0.0f;
    p[8].layer = 2;
    p[8].flags = 3;
    p[8].tex = gDllA6EffectTex;
    p[8].mode = 2;
    p[8].x = 0.1f;
    p[8].y = 14.0f;
    p[8].z = 0.05f;

    buf.v58 = 0;
    buf.ctx = (int)sourceObj;
    buf.v44 = variant;
    buf.pos[0] = 0.0f;
    buf.pos[1] = 0.0f;
    buf.pos[2] = 0.0f;
    buf.col[0] = 0.0f;
    buf.col[1] = 0.0f;
    buf.col[2] = 0.0f;
    buf.scale = 4.0f;
    buf.v40 = 1;
    buf.v3c = 0;
    buf.v59 = 3;
    buf.v5a = 0;
    buf.v5b = 0;
    buf.count = &p[9] - e;
    buf.hw[0] = gDllA6EffectHwWords[0];
    buf.hw[1] = gDllA6EffectHwWords[1];
    buf.hw[2] = gDllA6EffectHwWords[2];
    buf.hw[3] = gDllA6EffectHwWords[3];
    buf.hw[4] = gDllA6EffectHwWords[4];
    buf.hw[5] = gDllA6EffectHwWords[5];
    buf.hw[6] = gDllA6EffectHwWords[6];
    buf.cmds = (GfxCmd*)((u8*)&buf + 0x60);
    fl = 0x4000400;
    buf.flags = fl;
    fl |= flags;
    buf.flags = fl;
    if (fl & 1)
    {
        if (sourceObj != 0 && posSource != 0)
        {
            buf.pos[0] +=
                ((GameObject*)sourceObj)->anim.worldPosX + ((PartFxSpawnParams*)posSource)->posX;
            buf.pos[1] +=
                ((GameObject*)sourceObj)->anim.worldPosY + ((PartFxSpawnParams*)posSource)->posY;
            buf.pos[2] +=
                ((GameObject*)sourceObj)->anim.worldPosZ + ((PartFxSpawnParams*)posSource)->posZ;
        }
        else if (sourceObj != 0)
        {
            buf.pos[0] = buf.pos[0] + ((GameObject*)sourceObj)->anim.worldPosX;
            buf.pos[1] = buf.pos[1] + ((GameObject*)(buf.ctx))->anim.worldPosY;
            buf.pos[2] = buf.pos[2] + ((GameObject*)(buf.ctx))->anim.worldPosZ;
        }
        else if (posSource != 0)
        {
            buf.pos[0] = buf.pos[0] + ((PartFxSpawnParams*)posSource)->posX;
            buf.pos[1] = buf.pos[1] + ((PartFxSpawnParams*)posSource)->posY;
            buf.pos[2] = buf.pos[2] + ((PartFxSpawnParams*)posSource)->posZ;
        }
    }
    (*gModgfxInterface)->spawnEffect(&buf, 0, 3, lbl_80318DF0, 1, lbl_803DB980, DLLA6_EFFECT_ID, 0);
}

void dll_A6_func01_nop(void)
{
}

void dll_A6_func00_nop(void)
{
}

u8 lbl_80318DF0[32] = {0, 0,  0, 230, 5, 20, 0, 0, 0, 31, 0, 0,  255, 26, 5, 20,
                       0, 31, 0, 31,  0, 0,  0, 0, 0, 0,  0, 15, 0,   16, 0, 0};
s16 gDllA6EffectHwWords[8] = {0, 0x46, 0x46, 0, 0, 0, 0, 0};

void* lbl_80318E20[8] = {(void*)0x00000000, (void*)0x00000000, (void*)0x00000000, (void*)0x00030000,
                         dll_A6_func00_nop, dll_A6_func01_nop, (void*)0x00000000, dll_A6_func03};
