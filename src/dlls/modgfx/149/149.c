/*
 * DLL 149 / 0x95 - func00/func01 are empty no-op slots; func03
 * spawns a 7-part modgfx effect via gModgfxInterface->spawnEffect.
 *
 * The effect is described by a ModgfxSpawnPacket with an inline array of
 * per-part draw commands (GfxCmd, 0x18 bytes each). The seven parts use
 * the texture at base+0x80 (base = lbl_80317528) and a table of constant
 * built-in float values. When the effect flags bit 0
 * is set the world-space position is offset by the source object's
 * position (sourceObj+0x18) and/or the posSource transform (posSource+0xc).
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/modgfx_types.h"
#include "main/dll/partfx_interface.h"
#include "dlls/object_descriptor.h"

extern u32 lbl_80317528[];

u8 lbl_803DB940[8] = {0, 4, 0, 5, 0, 6, 0, 7};

/* effect id spawned by this DLL's modgfx emitter (spawnEffect textureAssetId arg). */
#define DLL95_EFFECT_ID 0x46


void dll_95_func03(int sourceObj, int variant, int posSource)
{
    ModgfxSpawnPacket buf;
    u8* base = (u8*)(int)lbl_80317528;
    GfxCmd* entry = buf.entries;
    GameObject* obj = (GameObject*)sourceObj;
    PartFxSpawnParams* params = (PartFxSpawnParams*)posSource;

    entry[0].layer = 0;
    entry[0].flags = 8;
    entry[0].tex = base + 0x80;
    entry[0].mode = 2;
    entry[0].x = 0.014f;
    entry[0].y = 0.03f;
    entry[0].z = 0.014f;
    entry[1].layer = 0;
    entry[1].flags = 4;
    entry[1].tex = lbl_803DB940;
    entry[1].mode = 8;
    entry[1].x = 255.0f;
    entry[1].y = 255.0f;
    entry[1].z = 0.0f;
    entry[2].layer = 0;
    entry[2].flags = 4;
    entry[2].tex = base + 0x80;
    entry[2].mode = 8;
    entry[2].x = 255.0f;
    entry[2].y = 85.0f;
    entry[2].z = 0.0f;
    entry[3].layer = 0;
    entry[3].flags = 0;
    entry[3].tex = NULL;
    entry[3].mode = 0x400000;
    entry[3].x = 0.0f;
    entry[3].y = 80.0f;
    entry[3].z = 0.0f;
    entry[4].layer = 1;
    entry[4].flags = 8;
    entry[4].tex = base + 0x80;
    entry[4].mode = 2;
    entry[4].x = 100.0f;
    entry[4].y = 100.0f;
    entry[4].z = 100.0f;
    entry[5].layer = 1;
    entry[5].flags = 0;
    entry[5].tex = NULL;
    entry[5].mode = 0x400000;
    entry[5].x = 0.0f;
    entry[5].y = -80.0f;
    entry[5].z = 0.0f;
    entry[6].layer = 2;
    entry[6].flags = 8;
    entry[6].tex = base + 0x80;
    entry[6].mode = 4;
    entry[6].x = 0.0f;
    entry[6].y = 0.0f;
    entry[6].z = 0.0f;
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
    buf.v59 = 8;
    buf.v5a = 0;
    buf.v5b = 0x3c;
    buf.count = (GfxCmd*)((u8*)entry + 0xa8) - entry;
    buf.hw[0] = *(s16*)(base + 0x90);
    buf.hw[1] = *(s16*)(base + 0x92);
    buf.hw[2] = *(s16*)(base + 0x94);
    buf.hw[3] = *(s16*)(base + 0x96);
    buf.hw[4] = *(s16*)(base + 0x98);
    buf.hw[5] = *(s16*)(base + 0x9a);
    buf.hw[6] = *(s16*)(base + 0x9c);
    buf.cmds = (GfxCmd*)((u8*)&buf + 0x60);
    buf.flags = 0x4002400;
    if ((buf.flags & 1) != 0)
    {
        if ((u32)sourceObj != 0 && (u32)posSource != 0)
        {
            buf.pos[0] += (obj->anim.worldPosX + params->posX);
            buf.pos[1] += (obj->anim.worldPosY + params->posY);
            buf.pos[2] += (obj->anim.worldPosZ + params->posZ);
        }
        else if ((u32)sourceObj != 0)
        {
            buf.pos[0] += obj->anim.worldPosX;
            buf.pos[1] += ((GameObject*)(buf.ctx))->anim.worldPosY;
            buf.pos[2] += ((GameObject*)(buf.ctx))->anim.worldPosZ;
        }
        else if ((u32)posSource != 0)
        {
            buf.pos[0] += params->posX;
            buf.pos[1] += params->posY;
            buf.pos[2] += params->posZ;
        }
    }
    (*gModgfxInterface)->spawnEffect(&buf, 0, 8, base, 8, base + 0x50, DLL95_EFFECT_ID, 0);
}

void dll_95_func01_nop(void)
{
}

void dll_95_func00_nop(void)
{
}

u32 lbl_80317528[40] = {0xfce001f4, 0xfce00008, 0x001f0320, 0x01f4fce0, 0x0078001f, 0x032001f4, 0x03200008, 0x001ffce0,
                        0x01f40320, 0x0078001f, 0xfc180000, 0xfc180008, 0x000003e8, 0x0000fc18, 0x00780000, 0x03e80000,
                        0x03e80008, 0x0000fc18, 0x000003e8, 0x00780000, 0x00000001, 0x00050000, 0x00050004, 0x00010002,
                        0x00060001, 0x00060005, 0x00020003, 0x00070002, 0x00070006, 0x00030000, 0x00040003, 0x00040007,
                        0x00000001, 0x00020003, 0x00040005, 0x00060007, 0x00000316, 0x000a0000, 0x00000000, 0x00000000};
ObjectDescriptor4 dll_95_funcs = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_4_SLOTS,
    (ObjectDescriptorCallback)dll_95_func00_nop,
    (ObjectDescriptorCallback)dll_95_func01_nop,
    0,
    (ObjectDescriptorCallback)dll_95_func03,
};
