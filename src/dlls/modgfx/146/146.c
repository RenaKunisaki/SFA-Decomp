/*
 * DLL 146 / 0x92 - mod-gfx effect spawner.
 *
 * func00/func01 are no-op slots. func03 builds a modgfx effect: a stack
 * ModgfxSpawnPacket holding a fixed list of GfxCmd entries (textures from
 * lbl_803171C0 / lbl_803DB930, modes, per-entry positions and a shared
 * scale), then hands it to gModgfxInterface->spawnEffect.
 *
 * `variant` selects between two sets of x/scale constants (155.0f
 * vs 55.0f, 0.15f vs 0.1f). When flags bit 0 is
 * set the effect origin is offset by the source object's world position
 * (sourceObj +0x18..+0x20) and/or the position source (posSource
 * +0x0c..+0x14). extraArgs, when non-NULL, overrides the base scale.
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/modgfx_types.h"
#include "main/dll/partfx_interface.h"
#include "game/objects/object.h"
#include "main/dll/dll_0092_dll92func0.h"

extern u32 lbl_803171C0[];

u8 lbl_803DB930[8] = {0, 1, 0, 0, 0, 0, 0, 0};

/* effect id spawned by this DLL's modgfx emitter (spawnEffect textureAssetId arg). */
#define DLL92_EFFECT_ID 0x3c

void dll_92_func03(GameObject* sourceObj, int variant, PartFxSpawnParams* posSource, u32 flags,
                   u32 unused, f32* extraArgs)
{
    ModgfxEffectResource* resource[1];
    ModgfxPointerSpawnPacket buf;
    GfxCmd* e;
    f32 s;
    resource[0] = (ModgfxEffectResource*)lbl_803171C0;
    s = 1.0f;
    if (extraArgs != NULL)
    {
        s = *extraArgs;
    }
    e = buf.entries;
    e[0].layer = 0;
    e[0].flags = 5;
    e[0].tex = resource[0]->primaryTexture;
    e[0].mode = 4;
    e[0].x = 0.0f;
    e[0].y = 0.0f;
    e[0].z = 0.0f;
    e[1].layer = 0;
    e[1].flags = 1;
    e[1].tex = lbl_803DB930;
    e[1].mode = 4;
    if (variant == 1)
    {
        e[1].x = 155.0f;
    }
    else
    {
        e[1].x = 55.0f;
    }
    e[1].y = 0.0f;
    e[1].z = 0.0f;
    e[2].layer = 0;
    e[2].flags = 6;
    e[2].tex = resource[0]->sharedTexture;
    e[2].mode = 2;
    if (variant == 1)
    {
        e[2].z = e[2].y = e[2].x = 0.15f * s;
    }
    else
    {
        e[2].z = e[2].y = e[2].x = 0.1f * s;
    }
    e[3].layer = 1;
    e[3].flags = 6;
    e[3].tex = resource[0]->sharedTexture;
    e[3].mode = 0x4000;
    e[3].x = -0.5f;
    e[3].y = 1.0f;
    e[3].z = 0.0f;
    e[4].layer = 1;
    e[4].flags = 6;
    e[4].tex = resource[0]->sharedTexture;
    e[4].mode = 2;
    e[4].x = 4.0f;
    e[4].y = 4.0f;
    e[4].z = 25.0f;
    e[5].layer = 2;
    e[5].flags = 6;
    e[5].tex = resource[0]->sharedTexture;
    e[5].mode = 0x4000;
    e[5].x = -0.5f;
    e[5].y = 1.0f;
    e[5].z = 0.0f;
    e[6].layer = 2;
    e[6].flags = 6;
    e[6].tex = resource[0]->sharedTexture;
    e[6].mode = 2;
    e[6].x = 8.0f;
    e[6].y = 8.0f;
    e[6].z = 1.0f;
    e[7].layer = 3;
    e[7].flags = 6;
    e[7].tex = resource[0]->sharedTexture;
    e[7].mode = 0x4000;
    e[7].x = -0.5f;
    e[7].y = 1.0f;
    e[7].z = 0.0f;
    e[8].layer = 3;
    e[8].flags = 1;
    e[8].tex = lbl_803DB930;
    e[8].mode = 4;
    e[8].x = 0.0f;
    e[8].y = 0.0f;
    e[8].z = 0.0f;
    buf.v58 = 0;
    buf.ctx = sourceObj;
    buf.v44 = variant;
    buf.pos[0] = 0.0f;
    buf.pos[1] = 0.0f;
    buf.pos[2] = 0.0f;
    buf.col[0] = 0.0f;
    buf.col[1] = 0.0f;
    buf.col[2] = 0.0f;
    buf.scale = 2.0f;
    buf.v40 = 1;
    buf.v3c = 0;
    buf.v59 = 6;
    buf.v5a = 0;
    buf.v5b = 0;
    buf.count = (GfxCmd*)((u8*)e + 0xd8) - e;
    buf.hw[0] = resource[0]->sequenceParams[0];
    buf.hw[1] = resource[0]->sequenceParams[1];
    buf.hw[2] = resource[0]->sequenceParams[2];
    buf.hw[3] = resource[0]->sequenceParams[3];
    buf.hw[4] = resource[0]->sequenceParams[4];
    buf.hw[5] = resource[0]->sequenceParams[5];
    buf.hw[6] = resource[0]->sequenceParams[6];
    buf.cmds = (GfxCmd*)((u8*)&buf + 0x60);
    buf.flags = 0x4000400;
    buf.flags |= flags;
    if ((buf.flags & 1) != 0)
    {
        if (sourceObj != NULL && posSource != NULL)
        {
            buf.pos[0] += sourceObj->anim.worldPosX + posSource->posX;
            buf.pos[1] += sourceObj->anim.worldPosY + posSource->posY;
            buf.pos[2] += sourceObj->anim.worldPosZ + posSource->posZ;
        }
        else if (sourceObj != NULL)
        {
            buf.pos[0] += sourceObj->anim.worldPosX;
            buf.pos[1] += buf.sourceObj->anim.worldPosY;
            buf.pos[2] += buf.sourceObj->anim.worldPosZ;
        }
        else if (posSource != NULL)
        {
            buf.pos[0] += posSource->posX;
            buf.pos[1] += posSource->posY;
            buf.pos[2] += posSource->posZ;
        }
    }
    (*gModgfxInterface)->spawnEffect(&buf, 0, 6, resource[0], 4, resource[0]->spawnData, DLL92_EFFECT_ID, 0);
}

void dll_92_func01_nop(void)
{
}

void dll_92_func00_nop(void)
{
}

u32 lbl_803171C0[31] = {0xff1a0000, 0x00000000, 0x000f0000, 0x00000000, 0x007f000f, 0x00e60000, 0x000000ff, 0x000fff1a,
                        0x000003e8, 0x00000000, 0x00000000, 0x03e8007f, 0x000000e6, 0x000003e8, 0x00ff0000, 0x00000004,
                        0x00030000, 0x00010004, 0x00010002, 0x00040002, 0x00050004, 0x00000001, 0x00020003, 0x00040005,
                        0x00000002, 0x00030004, 0x00050000, 0x00000006, 0x0014001a, 0x00000000, 0x00000000};
u32 lbl_8031723C[9] = {
    0x00000000, 0x00000000,         0x00000000, 0x00030000, (u32)dll_92_func00_nop, (u32)dll_92_func01_nop,
    0x00000000, (u32)dll_92_func03, 0x00000000};
