/*
 * dll99func0 (DLL 0x99) - a model/screen effect emitter sharing foodbag's
 * modgfx command-list pattern (cf. dll_009A_dll9afunc0.c). func00/func01 are
 * empty entry stubs (defined last, in reverse export order: func01 then
 * func00, matching the retail address order); func03 fills a ModgfxSpawnPacket of nine command entries from the
 * .sdata2 float table at 1.0f.. and the per-entry flag/texture/anim
 * table at lbl_80317AF8, then dispatches it through gModgfxInterface->spawnEffect.
 *
 * The "variant" argument (1 vs. other) selects between two x-offset constants
 * for entry 1 and two scale constants for entry 2. When effect flag bit 0 is
 * set, the spawn position is offset by the source object's world position
 * (sourceObj+0x18) and/or the posSource frame (posSource+0xc).
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/modgfx_types.h"
#include "main/dll/partfx_interface.h"
#include "game/objects/object.h"
#include "main/dll/savegame.h"
#include "main/dll/dll_0099_dll99func0.h"

u8 lbl_803DB950[8] = {0, 1, 0, 0, 0, 0, 0, 0};

/* effect id spawned by this DLL's modgfx emitter (spawnEffect textureAssetId arg). */
#define DLL99_EFFECT_ID 0x3c


extern ModgfxEffectResource lbl_80317AF8;


void dll_99_func03(GameObject* sourceObj, int variant, PartFxSpawnParams* posSource, u32 flags,
                   int unused, f32* extraArgs)
{
    ModgfxEffectResource* resource = &lbl_80317AF8;
    ModgfxPointerSpawnPacket buf;
    GfxCmd* entry;
    f32 scale = 1.0f;
    if (extraArgs != NULL)
    {
        scale = *extraArgs;
    }
    entry = buf.entries;
    entry[0].layer = 0;
    entry[0].flags = 5;
    entry[0].tex = resource->primaryTexture;
    entry[0].mode = 4;
    entry[0].x = 0.0f;
    entry[0].y = 0.0f;
    entry[0].z = 0.0f;
    entry[1].layer = 0;
    entry[1].flags = 1;
    entry[1].tex = lbl_803DB950;
    entry[1].mode = 4;
    if (variant == 1)
    {
        entry[1].x = 155.0f;
    }
    else
    {
        entry[1].x = 55.0f;
    }
    entry[1].y = 0.0f;
    entry[1].z = 0.0f;
    entry[2].layer = 0;
    entry[2].flags = 6;
    entry[2].tex = resource->sharedTexture;
    entry[2].mode = 2;
    if (variant == 1)
    {
        entry[2].z = entry[2].y = entry[2].x = 0.15f * scale;
    }
    else
    {
        entry[2].z = entry[2].y = entry[2].x = 0.1f * scale;
    }
    entry[3].layer = 1;
    entry[3].flags = 6;
    entry[3].tex = resource->sharedTexture;
    entry[3].mode = 0x4000;
    entry[3].x = -0.5f;
    entry[3].y = 1.0f;
    entry[3].z = 0.0f;
    entry[4].layer = 1;
    entry[4].flags = 6;
    entry[4].tex = resource->sharedTexture;
    entry[4].mode = 2;
    entry[4].x = 4.0f;
    entry[4].y = 4.0f;
    entry[4].z = 25.0f;
    entry[5].layer = 2;
    entry[5].flags = 6;
    entry[5].tex = resource->sharedTexture;
    entry[5].mode = 0x4000;
    entry[5].x = -0.5f;
    entry[5].y = 1.0f;
    entry[5].z = 0.0f;
    entry[6].layer = 2;
    entry[6].flags = 6;
    entry[6].tex = resource->sharedTexture;
    entry[6].mode = 2;
    entry[6].x = 8.0f;
    entry[6].y = 8.0f;
    entry[6].z = 1.0f;
    entry[7].layer = 3;
    entry[7].flags = 6;
    entry[7].tex = resource->sharedTexture;
    entry[7].mode = 0x4000;
    entry[7].x = -0.5f;
    entry[7].y = 1.0f;
    entry[7].z = 0.0f;
    entry[8].layer = 3;
    entry[8].flags = 1;
    entry[8].tex = lbl_803DB950;
    entry[8].mode = 4;
    entry[8].x = 0.0f;
    entry[8].y = 0.0f;
    entry[8].z = 0.0f;
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
    buf.count = (GfxCmd*)((u8*)entry + 0xd8) - entry;
    buf.hw[0] = resource->sequenceParams[0];
    buf.hw[1] = resource->sequenceParams[1];
    buf.hw[2] = resource->sequenceParams[2];
    buf.hw[3] = resource->sequenceParams[3];
    buf.hw[4] = resource->sequenceParams[4];
    buf.hw[5] = resource->sequenceParams[5];
    buf.hw[6] = resource->sequenceParams[6];
    buf.cmds = (GfxCmd*)((u8*)&buf + 0x60);
    buf.flags = 0x4000410; /* default effect flag set; bit0 enables position offset below */
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
    (*gModgfxInterface)->spawnEffect(&buf, 0, 6, resource, 4, resource->spawnData, DLL99_EFFECT_ID, 0);
}

void dll_99_func01_nop(void)
{
}

void dll_99_func00_nop(void)
{
}
