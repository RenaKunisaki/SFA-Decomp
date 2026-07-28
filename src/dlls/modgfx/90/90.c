/*
 * DLL 90 / 0x5A - a staff-collision particle spawner.
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/modgfx_types.h"
#include "main/dll/dll_005A_staffcollisionfunc03.h"

u8 lbl_803DB898[8] = {0, 0, 0, 1, 0, 2, 0, 0};
u8 lbl_803DB8A0[8] = {0, 0, 0, 1, 0, 2, 0, 0};
u8 lbl_803DB8A8[8] = {0, 0, 0, 1, 0, 2, 0, 3};

extern u8 lbl_80311DA8[];

void StaffCollision_func03(u8* sourceObj, int variant, PartFxSpawnParams* spawnParams, u32 spawnFlags, int modelId,
                           StaffCollisionColorArgs* colorArgs)
{
    struct
    {
        s16 rotZ, rotX, rotY;
        f32 x;
        f32 y, z, w;
    } m;
    struct
    {
        GfxCmd* cmds;
        u8* ctx;
        u8 pad0[0x18];
        f32 col[3];
        f32 pos[3];
        f32 scale;
        u32 v3c;
        u32 v40;
        s16 effectVariant;
        s16 hw[7];
        u32 flags;
        u8 v58, kindFlags, v5a, v5b, pad_5c;
        s8 count;
        u8 pad1[2];
    } buf;
    GfxCmd ents[32];
    GfxCmd* e = ents;
    int cnt;
    StaffFxDesc* base = (StaffFxDesc*)lbl_80311DA8;
    s16 r, g, b;
    int i;
    r = 0xff;
    g = 0xff;
    b = 0xff;
    cnt = 1;
    if (colorArgs != NULL)
    {
        cnt = colorArgs->count;
        r = colorArgs->red;
        g = colorArgs->green;
        b = colorArgs->blue;
    }
    for (i = 0; i < cnt; i++)
    {
        f32 ra, rb;
        if (variant == 0)
        {
            r += randomGetRange(-0x1b, 0x1b);
            if (r > 0xff)
            {
                r = 0xff;
            }
            else if (r < 0)
            {
                r = 0;
            }
            g += randomGetRange(-0x1b, 0x1b);
            if (g > 0xff)
            {
                g = 0xff;
            }
            else if (g < 0)
            {
                g = 0;
            }
            b += randomGetRange(-0x1b, 0x1b);
            if (b > 0xff)
            {
                b = 0xff;
            }
            else if (b < 0)
            {
                b = 0;
            }
        }
        e[0].layer = 0;
        e[0].flags = variant != 0 ? 4 : 3;
        e[0].tex = variant != 0 ? lbl_803DB8A8 : lbl_803DB8A0;
        e[0].mode = 8;
        e[0].x = r;
        e[0].y = g;
        e[0].z = b;
        ra = (f32)(int)randomGetRange(0, 0xfffe);
        rb = (f32)(int)randomGetRange(-0xbb8, -0x2ee0);
        e[1].layer = 0;
        e[1].flags = 0;
        e[1].tex = NULL;
        e[1].mode = 0x80;
        e[1].x = 0.0f;
        e[1].y = rb;
        e[1].z = ra;
        e[2].layer = 0;
        e[2].flags = variant != 0 ? 4 : 3;
        e[2].tex = variant != 0 ? lbl_803DB8A8 : lbl_803DB8A0;
        e[2].mode = 2;
        e[2].x = 1.0f;
        e[2].y = 0.5f;
        e[2].z = 1.5f;
        e[3].layer = 1;
        e[3].flags = 0;
        e[3].tex = NULL;
        e[3].mode = 0x400000;
        e[3].x = 0.0f;
        e[3].y = 0.0f;
        e[3].z = 400.0f;
        m.y = 0.0f;
        m.z = 0.0f;
        m.w = 0.0f;
        m.x = 1.0f;
        m.rotY = 0;
        m.rotX = rb;
        m.rotZ = ra;
        vecRotateZXY((s16*)&m, &e[3].x);
        buf.v58 = 0;
        buf.ctx = sourceObj;
        buf.effectVariant = variant;
        buf.pos[0] = 0.0f;
        buf.pos[1] = 0.0f;
        buf.pos[2] = 0.0f;
        buf.col[0] = 0.0f;
        buf.col[1] = 0.0f;
        buf.col[2] = 0.0f;
        buf.scale = 1.0f;
        buf.v40 = 1;
        buf.v3c = 0;
        buf.kindFlags = variant != 0 ? 4 : 3;
        buf.v5a = 0;
        buf.v5b = 0x10;
        buf.count = 4;
        buf.hw[0] = base->hw[0];
        buf.hw[1] = base->hw[1];
        buf.hw[2] = base->hw[2];
        buf.hw[3] = base->hw[3];
        buf.hw[4] = base->hw[4];
        buf.hw[5] = base->hw[5];
        buf.hw[6] = base->hw[6];
        buf.cmds = ents;
        buf.flags = 0x2000490;
        buf.flags |= spawnFlags;
        if ((buf.flags & 1) != 0)
        {
            if (buf.ctx != 0 && spawnParams != 0)
            {
                buf.pos[0] += ((GameObject*)buf.ctx)->anim.worldPosX + spawnParams->posX;
                buf.pos[1] += ((GameObject*)buf.ctx)->anim.worldPosY + spawnParams->posY;
                buf.pos[2] += ((GameObject*)buf.ctx)->anim.worldPosZ + spawnParams->posZ;
            }
            else if (buf.ctx != 0)
            {
                buf.pos[0] += ((GameObject*)buf.ctx)->anim.worldPosX;
                buf.pos[1] += ((GameObject*)buf.ctx)->anim.worldPosY;
                buf.pos[2] += ((GameObject*)buf.ctx)->anim.worldPosZ;
            }
            else if (spawnParams != 0)
            {
                buf.pos[0] += spawnParams->posX;
                buf.pos[1] += spawnParams->posY;
                buf.pos[2] += spawnParams->posZ;
            }
        }
        (*gModgfxInterface)
            ->spawnEffect(&buf, 0, variant != 0 ? 4 : 3, variant != 0 ? (void*)base->vtx1 : (void*)base->vtx0,
                          variant != 0 ? 2 : 1, variant != 0 ? (void*)base->col : (void*)lbl_803DB898, 0, 0);
    }
}

u8 lbl_80311DA8[100] = {0, 30, 0, 0, 0, 0,   0, 0,  0, 31, 255, 226, 0, 0, 0, 0,   0,   15,  0, 31, 0, 0, 0, 0,  3, 232,
                        0, 8,  0, 0, 0, 0,   0, 15, 0, 0,  0,   0,   0, 0, 0, 31,  255, 241, 0, 0,  0, 0, 0, 15, 0, 31,
                        0, 15, 0, 0, 7, 208, 0, 8,  0, 0,  255, 241, 0, 0, 7, 208, 0,   8,   0, 0,  0, 0, 0, 1,  0, 2,
                        0, 1,  0, 3, 0, 2,   0, 0,  0, 80, 0,   0,   0, 0, 0, 0,   0,   0,   0, 0,  0, 0};

void* lbl_80311E0C[9] = {(void*)0x00000000, (void*)0x00000000,     (void*)0x00000000,
                         (void*)0x00030000, (void*)0x00000000,     (void*)0x00000000,
                         (void*)0x00000000, StaffCollision_func03, (void*)0x00000000};
