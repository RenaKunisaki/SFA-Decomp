/*
 * DLL 133 / 0x85 - func03 builds a ModgfxInterface
 * effect command list (FbBuf) and spawns it. Two layouts are emitted:
 * variant 4 (a self-contained burst, base flags 0x4004400) and the
 * default variant (scaled off the source object's field 8 and a child
 * object at field 0x50, base flags 0x4006410). The caller's `flags` are
 * OR'd in; flag bit 0 adds the source/position-source world offsets to
 * buf.pos before the spawn. Several command slots seed x with a random
 * angle from randomGetRange. The two trailing _nop entry points are the
 * DLL's unused func00/func01 slots.
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/partfx_interface.h"
#include "game/objects/object.h"
#include "main/dll/fb_cmd.h"

u8 lbl_803DB8F0[4] = {0, 0, 0, 1};
u8 lbl_803DB8F4[8] = {0, 0, 0, 1, 0, 2, 0, 3};
u8 lbl_803DB8FC[4] = {0, 2, 0, 3};

#define FX_VARIANT_BURST 4

extern u8 gFoodbagEffectTemplate[];

void dll_85_func03(int sourceObj, int variant, int posSource, u32 flags)
{
    FbBuf buf;
    u8* base = (u8*)(int)gFoodbagEffectTemplate;
    s16* tableHw = (s16*)base;
    FbCmd* p;
    FbCmd* e = buf.entries;
    f32 rv;

    if (variant == FX_VARIANT_BURST)
    {
        e[0].layer = 0;
        e[0].flags = 0;
        e[0].tex = NULL;
        e[0].mode = 0x400000;
        e[0].x = 10.0f;
        e[0].y = 0.0f;
        e[0].z = 0.0f;
        e[1].layer = 0;
        e[1].flags = 2;
        e[1].tex = lbl_803DB8FC;
        e[1].mode = 2;
        e[1].x = 9.0f;
        e[1].y = 2.0f;
        e[1].z = 9.0f;
        e[2].layer = 0;
        e[2].flags = 4;
        e[2].tex = lbl_803DB8FC;
        e[2].mode = 0x80;
        e[2].x = (f32)randomGetRange(-0x7ff8, 0x7ff8);
        e[2].y = 0.0f;
        e[2].z = 16383.0f;
        p = &e[3];
    }
    else
    {
        GameObject* src = (GameObject*)sourceObj;
        e[0].layer = 0;
        e[0].flags = 2;
        e[0].tex = lbl_803DB8F0;
        e[0].mode = 2;
        e[0].x = 190.0f * src->anim.rootMotionScale;
        e[0].y = 6.0f * src->anim.rootMotionScale;
        e[0].z = 1.0f;
        e[1].layer = 0;
        e[1].flags = 2;
        e[1].tex = lbl_803DB8FC;
        e[1].mode = 2;
        e[1].x = 40.0f * (src->anim.rootMotionScale /
                          src->anim.modelInstance->rootMotionScaleBase);
        e[1].y = 6.0f * (src->anim.rootMotionScale /
                         src->anim.modelInstance->rootMotionScaleBase);
        e[1].z = 1.0f;
        rv = (f32)randomGetRange(0, 0xfffe);
        e[2].layer = 0;
        e[2].flags = 0;
        e[2].tex = NULL;
        e[2].mode = 0x80;
        e[2].x = rv;
        e[2].y = 1000.0f;
        e[2].z = 0.0f;
        p = &e[3];
    }
    p[0].layer = 0;
    p[0].flags = 4;
    p[0].tex = lbl_803DB8F4;
    p[0].mode = 4;
    p[0].x = 0.0f;
    p[0].y = 0.0f;
    p[0].z = 0.0f;
    rv = (f32)randomGetRange(0, 0xfffe);
    p[1].layer = 1;
    p[1].flags = 2;
    p[1].tex = lbl_803DB8F0;
    p[1].mode = 4;
    p[1].x = 255.0f;
    p[1].y = 0.0f;
    p[1].z = 0.0f;
    if (variant == FX_VARIANT_BURST)
    {
        p[2].layer = 2;
        p[2].flags = 0;
        p[2].tex = NULL;
        p[2].mode = 0x100;
        p[2].x = 100.0f;
        p[2].y = 0.0f;
        p[2].z = 0.0f;
        p += 3;
    }
    else
    {
        p[2].layer = 1;
        p[2].flags = 0;
        p[2].tex = NULL;
        p[2].mode = 0x80;
        p[2].x = rv;
        p[2].y = 1000.0f;
        p[2].z = 0.0f;
        p += 3;
    }
    rv = (f32)randomGetRange(0, 0xfffe);
    if (variant == FX_VARIANT_BURST)
    {
        p->layer = 2;
        p->flags = 0;
        p->tex = NULL;
        p->mode = 0x100;
        p->x = 100.0f;
        p->y = 0.0f;
        p->z = 0.0f;
        p++;
    }
    else
    {
        p->layer = 2;
        p->flags = 0;
        p->tex = NULL;
        p->mode = 0x80;
        p->x = rv;
        p->y = 1000.0f;
        p->z = 0.0f;
        p++;
    }
    if (variant == FX_VARIANT_BURST)
    {
        p->layer = 3;
        p->flags = 0;
        p->tex = NULL;
        p->mode = 0x100;
        p->x = 100.0f;
        p->y = 0.0f;
        p->z = 0.0f;
        p++;
    }
    else
    {
        p->layer = 3;
        p->flags = 0;
        p->tex = NULL;
        p->mode = 0x80;
        p->x = rv;
        p->y = 1000.0f;
        p->z = 0.0f;
        p++;
    }
    p[0].layer = 3;
    p[0].flags = 2;
    p[0].tex = lbl_803DB8F0;
    p[0].mode = 4;
    p[0].x = 100.0f;
    p[0].y = 0.0f;
    p[0].z = 0.0f;
    p[1].layer = 3;
    p[1].flags = 4;
    p[1].tex = lbl_803DB8F4;
    p[1].mode = 2;
    p[1].x = 2.0f;
    p[1].y = 0.1f;
    p[1].z = 1.0f;
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
    buf.v3c = 0;
    buf.v59 = 4;
    buf.v5a = 0;
    buf.v5b = 0x20;
    buf.count = (FbCmd*)((u8*)p + 0x30) - e;
    buf.hw[0] = *(s16*)(base + 0x34);
    buf.hw[1] = *(s16*)(base + 0x36);
    buf.hw[2] = *(s16*)(base + 0x38);
    buf.hw[3] = *(s16*)(base + 0x3a);
    buf.hw[4] = *(s16*)(base + 0x3c);
    buf.hw[5] = *(s16*)(base + 0x3e);
    buf.hw[6] = *(s16*)(base + 0x40);
    buf.cmds = (FbCmd*)((u8*)&buf + 0x60);
    if (variant == FX_VARIANT_BURST)
    {
        buf.flags = 0x4004400;
    }
    else
    {
        buf.flags = 0x4006410;
    }
    buf.flags |= flags;
    if ((buf.flags & 1) != 0)
    {
        if ((u32)buf.ctx != 0 && (u32)posSource != 0)
        {
            buf.pos[0] += ((GameObject*)buf.ctx)->anim.worldPosX + ((PartFxSpawnParams*)posSource)->posX;
            buf.pos[1] += ((GameObject*)buf.ctx)->anim.worldPosY + ((PartFxSpawnParams*)posSource)->posY;
            buf.pos[2] += ((GameObject*)buf.ctx)->anim.worldPosZ + ((PartFxSpawnParams*)posSource)->posZ;
        }
        else if ((u32)buf.ctx != 0)
        {
            buf.pos[0] += ((GameObject*)buf.ctx)->anim.worldPosX;
            buf.pos[1] += ((GameObject*)buf.ctx)->anim.worldPosY;
            buf.pos[2] += ((GameObject*)buf.ctx)->anim.worldPosZ;
        }
        else if ((u32)posSource != 0)
        {
            buf.pos[0] += ((PartFxSpawnParams*)posSource)->posX;
            buf.pos[1] += ((PartFxSpawnParams*)posSource)->posY;
            buf.pos[2] += ((PartFxSpawnParams*)posSource)->posZ;
        }
    }
    (*gModgfxInterface)
        ->spawnEffect(&buf, 0, 4, (u8*)(int)gFoodbagEffectTemplate, 2, base + 0x28,
                      tableHw[variant * 2 + randomGetRange(0, 1) + 0x22], 0);
}

void dll_85_func01_nop(void)
{
}

void dll_85_func00_nop(void)
{
}

u8 gFoodbagEffectTemplate[88] = {
    0, 30, 0, 0,   0, 0, 0, 0, 0, 0,  255, 226, 0, 0,   0, 0,   0, 15,  0, 0, 255, 226, 3, 232, 0, 0,   0, 15, 0, 15,
    0, 30, 3, 232, 0, 0, 0, 0, 0, 15, 0,   0,   0, 1,   0, 2,   0, 0,   0, 2, 0,   3,   0, 0,   0, 10,  0, 15, 0, 80,
    0, 0,  0, 0,   0, 0, 0, 0, 5, 39, 5,   40,  0, 223, 0, 222, 0, 223, 2, 0, 1,   251, 1, 251, 0, 223, 0, 222};

/* DLL entry and resource tables. */
void* lbl_80316000[8] = {(void*)0x00000000, (void*)0x00000000, (void*)0x00000000, (void*)0x00030000,
                         dll_85_func00_nop, dll_85_func01_nop, (void*)0x00000000, dll_85_func03};
