#include "main/sky_state.h"
#include "main/dll/savegame_env_api.h"
#include "main/render_envfx_api.h"
#include "main/sky_interface.h"
#include "sys/objects.h"
#include "main/curve_eval.h"
#include "main/frame_timing.h"
#include "main/camera.h"
#include "main/mm.h"
#include "main/model.h"
#include "main/model_light.h"
#include "main/texture.h"
#include "main/textrender_api.h"
#include "main/rcp_dolphin_api.h"
#include "main/sky.h"
#include "main/sky_api.h"
#include "main/lightmap_api.h"
#include "main/lightmap_render_control_api.h"
#include "dlls/object_descriptor.h"
#include "main/loaded_file_flags.h"
#include "track/intersect_api.h"
#include "dolphin/gx/GXLighting.h"
#include "dolphin/gx/GXPixel.h"
#include "dolphin/gx/GXTev.h"
#include "dolphin/mtx/mtx_legacy.h"
#include "main/lightmap.h"
#include "main/track_dolphin_shadow_api.h"
#include "string.h"

u32 lbl_803DD18C;
u32 lbl_803DD188;
u8* gSky2State;
s8 gSky2DrawMode;
extern void* lbl_803DD13C;
extern void* lbl_803DD138;
extern void* lbl_803DD134;
extern void* lbl_803DD130;

s8 lbl_803DB750 = 1;
int lbl_803DB754 = 1;
u8 lbl_803DB758 = 1;

/* gSkyEnvFxFlags: per-group env-FX trigger enables + update state */
#define SKY_ENVFX_GROUP_C        0x01 /* lbl_803DD138 group (GameBit 0x3ab) */
#define SKY_ENVFX_GROUP_A        0x02 /* lbl_803DD130 group (GameBit 0x3ac) */
#define SKY_ENVFX_GROUP_B        0x04 /* lbl_803DD13C group */
#define SKY_ENVFX_GROUP_D        0x08 /* lbl_803DD134 group (weather) */
#define SKY_ENVFX_UPDATE_PENDING 0x10 /* sun position changed; process this frame */
#define SKY_ENVFX_IMMEDIATE      0x20 /* fire acts immediately vs deferred */
/* env-effect ids activated together when the GROUP_D (weather) flag is clear
   (index-style; roles opaque) */
#define SKY_ENVFX_ID_A               0x136
#define SKY_ENVFX_ID_B               0x137
#define SKY_ENVFX_ID_C               0x143
#define SKY_CONFIG_FIELD_COUNT       0xb
#define SKY_CHILD_OBJ_SUN            0x62b /* spawned into gSkySunObject */
#define SKY_CHILD_OBJ_MOON           0x62c /* spawned into gSkyMoonObject */
#define SKY_TEXTURE_SKY              0x5fa /* gSkySkyTexture */
extern u8 gSkyConfigFieldIndices[];
STATIC_ASSERT(sizeof(SkyVec3) == 0xC);
extern const f32 lbl_803DF108;
extern const f32 lbl_803DF10C;
extern const f32 lbl_803DF110;
extern f32 lbl_803DF114;
extern f32 lbl_803DF11C;
extern f32 lbl_803DF120;
extern f32 lbl_803DF138;
extern f32 lbl_803DF13C;
extern f32 lbl_803DF140;
extern const f32 lbl_803DF144;
extern int lbl_803DB610;
extern s8 gSky2DrawMode;
extern u8* gSky2State;
extern f32 lbl_803DF14C;
extern f32 lbl_803DF148;
extern const f32 lbl_803DF15C;
extern const f32 lbl_803DF160;
extern const f32 lbl_803DF168;
extern const f32 lbl_803DF16C;
extern const f32 lbl_803DF170;
extern const f32 lbl_803DF174;
extern const f32 lbl_803DF178;
extern const f32 lbl_803DF17C;
extern const f32 lbl_803DF180;
extern const f32 lbl_803DF184;
extern const f32 lbl_803DF188;
extern const f32 lbl_803DF18C;
extern u16 lbl_803E8460;
extern u8 lbl_803E8462;
extern f32 lbl_8039A7B8[];
const SkyVec3 lbl_802C1F98 = {-1000.0f, -1000.0f, -1000.0f};
extern f32 lbl_803DF190;
extern f32 lbl_803DF194;




void skyGetCurrentAmbientAndLightColors(u8* ambientRed, u8* ambientGreen, u8* ambientBlue, u8* lightRed, u8* lightGreen,
                                        u8* lightBlue);


void getAmbientColor(int slot, u8* red, u8* green, u8* blue);


void skySetLightSlot(int slot, f32 x, f32 y, f32 z, int red, int green, int blue, int ambientIntensity,
                 int lightIntensity, u8 blendAlpha);


void sky2GetFogRange(int* fogNear, int* fogFar)
{
    u8* state;
    f32 value;

    state = gSky2State;
    if (state != NULL)
    {
        value = *(f32*)(state + 0x14);
        *fogNear = value;
        value = *(f32*)(gSky2State + 0x18);
        *fogFar = value;
    }
}

void sky2GetTargetColor(int* red, int* green, int* blue, f32* blend)
{
    u8* state;

    state = gSky2State;
    if (state == NULL)
    {
        return;
    }
    *red = ((Dll06InterpState*)state)->targetX;
    *green = ((Dll06InterpState*)gSky2State)->targetY;
    *blue = ((Dll06InterpState*)gSky2State)->targetZ;
    *blend = ((Dll06InterpState*)gSky2State)->blend;
}

void sky2ResetStateFromConfig(u8* cfg, u8 flags)
{
    int i;
    u8* p2;
    int idx;

    if (((Sky2Config*)cfg)->flags & 0x80)
    {
        idx = 1;
    }
    else
    {
        idx = 0;
    }
    *(int*)((&gSky2State)[idx]) = 0;
    (&gSky2State)[idx][0x317] = 1;
    for (i = 0; i < 0x21; i++)
    {
        *(f32*)((&gSky2State)[idx] + i * 4 + 0x178) = lbl_803DF108;
    }
    for (i = 0; i < 0x21; i++)
    {
        *(f32*)((&gSky2State)[idx] + i * 4 + 0x70) = lbl_803DF108;
    }
    for (i = 0; i < 0x16; i++)
    {
        *(f32*)((&gSky2State)[idx] + i * 4 + 0x2ac) = lbl_803DF108;
    }
    for (i = 0; i < SKY_CONFIG_FIELD_COUNT; i++)
    {
        *(f32*)((&gSky2State)[idx] + i * 4 + 0x1fc) = lbl_803DF10C;
        *(f32*)((&gSky2State)[idx] + i * 4 + 0x228) = lbl_803DF110;
    }
    for (i = 0; i < SKY_CONFIG_FIELD_COUNT; i++)
    {
        *(f32*)((&gSky2State)[idx] + i * 4 + 0xf4) = (f32)(u32)cfg[gSkyConfigFieldIndices[i] + 0xc];
        *(f32*)((&gSky2State)[idx] + i * 4 + 0x120) = (f32)(u32)cfg[gSkyConfigFieldIndices[i] + 0x14];
        *(f32*)((&gSky2State)[idx] + i * 4 + 0x14c) = (f32)(u32)cfg[gSkyConfigFieldIndices[i] + 0x1c];
        *(f32*)((&gSky2State)[idx] + i * 4 + 0x254) = (f32)(u32) * (u16*)(cfg + gSkyConfigFieldIndices[i] * 2 + 0x3e);
        *(f32*)((&gSky2State)[idx] + i * 4 + 0x280) = (f32)(u32) * (u16*)(cfg + gSkyConfigFieldIndices[i] * 2 + 0x2e);
    }
    *(u16*)((&gSky2State)[idx] + 4) = cfg[0x58];
    *(u16*)((&gSky2State)[idx] + 6) = ((Sky2Config*)cfg)->flags2;
    *(f32*)((&gSky2State)[idx] + 0x64) = lbl_803DF108;
    *(f32*)((&gSky2State)[idx] + 0x68) = lbl_803DF108;
    *(s8*)((&gSky2State)[idx] + 0x314) = -1;
    *(f32*)((&gSky2State)[idx] + 0x6c) = lbl_803DF108;
    if (((Sky2Config*)cfg)->fadeDurationA == 0)
    {
        ((Sky2Config*)cfg)->fadeDurationA = 1;
    }
    if (((Sky2Config*)cfg)->fadeDurationA != 0)
    {
        *(int*)((&gSky2State)[idx] + 0x3c) = ((Sky2Config*)cfg)->fadeDurationA;
        *(int*)((&gSky2State)[idx] + 0x48) = 1;
        *(int*)((&gSky2State)[idx] + 8) = ((Sky2Config*)cfg)->skyTexId0;
        *(f32*)((&gSky2State)[idx] + 0x5c) = lbl_803DF114 / (f32)(u32)((Sky2Config*)cfg)->fadeDurationA;
    }
    else
    {
        *(int*)((&gSky2State)[idx] + 0x3c) = 0;
        *(f32*)((&gSky2State)[idx] + 0x5c) = lbl_803DF114;
    }
    if (((Sky2Config*)cfg)->fadeDurationB == 0)
    {
        ((Sky2Config*)cfg)->fadeDurationB = 1;
    }
    if (((Sky2Config*)cfg)->fadeDurationB != 0)
    {
        *(int*)((&gSky2State)[idx] + 0x40) = ((Sky2Config*)cfg)->fadeDurationB;
        *(f32*)((&gSky2State)[idx] + 0x58) =
            255.0f / (lbl_803DF11C * ((f32)(u32)((Sky2Config*)cfg)->fadeDurationB / lbl_803DF120));
        *(int*)((&gSky2State)[idx] + 0xc) = 0x5dc;
        *(f32*)((&gSky2State)[idx] + 0x60) = lbl_803DF114 / (f32)(u32)((Sky2Config*)cfg)->fadeDurationB;
    }
    else
    {
        *(int*)((&gSky2State)[idx] + 0x40) = 0;
        *(f32*)((&gSky2State)[idx] + 0x60) = lbl_803DF114;
    }
    *(int*)((&gSky2State)[idx] + 0x44) = 0;
}

void sky2StepSlotAnim(int slot)
{
    SkySlotAnim* anim;
    f32 dur;
    f32 zero;
    f32 len;
    f32 spd;
    f32 bv;
    int i;
    u16 flags;
    int flag1;

    anim = *(SkySlotAnim**)(&gSky2State + slot);
    if (anim->t >= (dur = lbl_803DF114))
    {
        anim->flags4 &= ~0x100;
        zero = lbl_803DF108;
        (*(SkySlotAnim**)(&gSky2State + slot))->step = zero;
        (*(SkySlotAnim**)(&gSky2State + slot))->t = zero;
        (*(SkySlotAnim**)(&gSky2State + slot))->prevT = dur;
        anim = *(SkySlotAnim**)(&gSky2State + slot);
        if (anim->b316 != 0 && (anim->flags6 & 0x40) == 0)
        {
            anim->b316 = 0;
        }
        for (i = 0; i < 0x21; i++)
        {
            (*(SkySlotAnim**)(&gSky2State + slot))->cur[i] = (*(SkySlotAnim**)(&gSky2State + slot))->target[i];
        }
        for (i = 0; i < 0x16; i++)
        {
            (*(SkySlotAnim**)(&gSky2State + slot))->cur2[i] = (*(SkySlotAnim**)(&gSky2State + slot))->target2[i];
        }
    }
    else
    {
        if (anim->b315 != 0)
        {
            len = lbl_803DF11C * ((f32)anim->frameCount / lbl_803DF120);
            if (lbl_803DF108 == len)
            {
                len = dur;
            }
            anim->step = 1.0f / len;
            for (i = 0; i < 0x21; i++)
            {
                (*(SkySlotAnim**)(&gSky2State + slot))->vel[i] = ((*(SkySlotAnim**)(&gSky2State + slot))->target[i] -
                                                                  (*(SkySlotAnim**)(&gSky2State + slot))->cur[i]) /
                                                                 len;
            }
            for (i = 0; i < 0x16; i++)
            {
                (*(SkySlotAnim**)(&gSky2State + slot))->vel2[i] = ((*(SkySlotAnim**)(&gSky2State + slot))->target2[i] -
                                                                   (*(SkySlotAnim**)(&gSky2State + slot))->cur2[i]) /
                                                                  len;
            }
            (*(SkySlotAnim**)(&gSky2State + slot))->b315 = 0;
        }
        for (i = 0; i < 0x21; i++)
        {
            (*(SkySlotAnim**)(&gSky2State + slot))->cur[i] +=
                timeDelta * (*(SkySlotAnim**)(&gSky2State + slot))->vel[i];
        }
        for (i = 0; i < 0x16; i++)
        {
            (*(SkySlotAnim**)(&gSky2State + slot))->cur2[i] +=
                timeDelta * (*(SkySlotAnim**)(&gSky2State + slot))->vel2[i];
        }
        (*(SkySlotAnim**)(&gSky2State + slot))->t += timeDelta * (*(SkySlotAnim**)(&gSky2State + slot))->step;
        anim = *(SkySlotAnim**)(&gSky2State + slot);
        flags = anim->flags4;
        flag1 = flags & 1;
        if (flag1 != 0 && (bv = anim->blend) > (zero = lbl_803DF108))
        {
            anim->blend = -(255.0f * anim->t - bv);
            if ((*(SkySlotAnim**)(&gSky2State + slot))->blend < zero)
            {
                (*(SkySlotAnim**)(&gSky2State + slot))->blend = zero;
                lbl_803DB750 = 1;
            }
        }
        else if ((flags & 4) != 0 && anim->blend < (spd = 255.0f))
        {
            anim->blend = spd * anim->t;
            if ((*(SkySlotAnim**)(&gSky2State + slot))->blend > spd)
            {
                (*(SkySlotAnim**)(&gSky2State + slot))->blend = spd;
            }
        }
        else if (flag1 == 0 && anim->blend < (spd = 255.0f))
        {
            anim->blend = spd * anim->t;
            if ((*(SkySlotAnim**)(&gSky2State + slot))->blend > spd)
            {
                (*(SkySlotAnim**)(&gSky2State + slot))->blend = spd;
            }
        }
        (*(SkySlotAnim**)(&gSky2State + slot))->prevT = (*(SkySlotAnim**)(&gSky2State + slot))->t;
    }
}

int sky2GetFogFadeAlpha(void)
{
    u8* state;
    f32 y;

    state = gSky2State;
    if (state == NULL)
    {
        return 0xff;
    }
    y = *(f32*)(state + 0x14);
    if (y < lbl_803DF138)
    {
        return 0;
    }
    if (y > lbl_803DF13C)
    {
        return 0xff;
    }
    return (int)(255.0f * ((y - lbl_803DF138) / lbl_803DF140));
}

void dll_06_func0C_nop(void)
{
}

void sky2BlendTowardTargetColor(s32* red, s32* green, s32* blue)
{
    Dll06InterpState* state;
    s32 targetX;
    s32 targetY;
    s32 targetZ;
    s32 oldX;
    s32 oldY;
    s32 oldZ;
    f32 blend;
    f32 fy;
    f32 fz;

    blend = lbl_803DF108;
    state = (Dll06InterpState*)gSky2State;
    if (state == NULL)
    {
        return;
    }
    if (state != NULL && state->active == 0)
    {
        return;
    }

    oldX = *red;
    oldY = *green;
    oldZ = *blue;
    if (state != NULL)
    {
        targetX = state->targetX;
        targetY = state->targetY;
        targetZ = state->targetZ;
        blend = state->blend;
    }

    fy = (f32)(targetY - oldY);
    fz = (f32)(targetZ - oldZ);
    *red = (s32)((f32)(targetX - oldX) * (blend = lbl_803DF144 * blend) + oldX);
    *green = (s32)(fy * blend + oldY);
    *blue = (s32)(fz * blend + oldZ);
}

void sky2SetDrawMode1(void)
{
    if (gSky2State == NULL)
    {
        return;
    }
    if (gSky2DrawMode != 1)
    {
        gSky2DrawMode = 1;
    }
}

void sky2SetDrawMode2(void)
{
    if (gSky2State == NULL)
    {
        return;
    }
    if (gSky2DrawMode != 2)
    {
        gSky2DrawMode = 2;
    }
}

void sky2ApplyModelTint(int obj)
{
    u8* s;
    f32 v;
    int alpha;

    if (gSky2State == NULL)
    {
        Obj_SetModelColorOverrideRecursive((GameObject*)obj, 0, 0, 0, 0, 0);
    }
    if (lbl_803DB750 == 0 && (*(u16*)((s = gSky2State) + 4) & 1) == 0)
    {
        v = *(f32*)(s + 0x14);
        if (v < lbl_803DF108)
        {
            alpha = 255;
        }
        else if (v > lbl_803DF148)
        {
            alpha = 0;
        }
        else
        {
            alpha = (int)(255.0f - 255.0f * (v / lbl_803DF148));
        }
        Obj_SetModelColorOverrideRecursive((GameObject*)obj, (u8) * (int*)(s + 0x24), (u8) * (int*)(s + 0x28),
                                           (u8) * (int*)(s + 0x2c), (u8)alpha, 1);
    }
    else
    {
        Obj_SetModelColorOverrideRecursive((GameObject*)obj, 0, 0, 0, 0, 0);
    }
}

void sky2ApplyTextColor(int obj)
{
    u8* s = gSky2State;
    f32 v;
    int alpha;

    if (s != NULL)
    {
        if (lbl_803DB750 == 0 && (*(u16*)(s + 4) & 1) == 0)
        {
            v = *(f32*)(s + 0x14);
            if (v < lbl_803DF108)
            {
                alpha = 255;
            }
            else if (v > lbl_803DF148)
            {
                alpha = 0;
            }
            else
            {
                alpha = (int)(255.0f - 255.0f * (v / lbl_803DF148));
            }
            setTextColor((void*)obj, (u8) * (int*)(s + 0x24), (u8) * (int*)(s + 0x28),
                         (u8) * (int*)(s + 0x2c), (u8)alpha);
        }
        else
        {
            setTextColor((void*)obj, 255, 255, 255, 0);
        }
    }
}

int dll_06_func07_ret_0(void)
{
    return 0x0;
}

void sky2ApplyFog(int obj)
{
    u8* s = gSky2State;

    if (s != NULL)
    {
        gSky2DrawMode = 2;
        setFogColorCallback(obj, (u8) * (int*)(s + 0x24), (u8) * (int*)(s + 0x28), (u8) * (int*)(s + 0x2c), 55);
        s = gSky2State;
        if (*(f32*)(s + 0x14) == *(f32*)(s + 0x18))
        {
            *(f32*)(s + 0x14) = *(f32*)(s + 0x14) - lbl_803DF14C;
        }
        s = gSky2State;
        if (*(f32*)(s + 0x14) > *(f32*)(s + 0x18))
        {
            *(f32*)(s + 0x14) = *(f32*)(s + 0x18) - lbl_803DF14C;
        }
        s = gSky2State;
        fogFn_80070404(*(f32*)(s + 0x14), *(f32*)(s + 0x18));
    }
}

void sky2_run(void)
{
    SkyRotQ q;
    f32 vec[3];
    SkyVec3 best;
    f32 height;
    SkyBestIdx idx;
    u8 red;
    u8 green;
    u8 blue;
    CameraViewSlot* cam;
    u8** pp;
    int i;
    u8* p;
    f32* dst;
    f32 cmax;
    int k;
    int d;
    int off1;
    int off2;
    u16 a1;
    int amp;
    int ri;
    int gi;
    int bi;
    u16 flags;
    f32 r;
    f32 g;
    f32 b;
    f32 sa;
    f32 sb;
    f32 step;
    f32 t;
    f32 u;
    f32 att;
    f32 z2;
    f32 c158;
    f32 c154;
    f32 c150;
    f32 one;
    f32 z;
    f32 zv;
    f32 spd;
    f32 frzero;
    f32 hv;
    f32 diff;
    f32 scale;

    best = lbl_802C1F98;
    r = lbl_803DF108;
    g = r;
    b = r;
    sa = r;
    sb = r;
    height = r;
    *(u16*)&idx = lbl_803E8460;
    idx.pad = lbl_803E8462;
    getAmbientColor(0, &red, &green, &blue);
    if (lbl_803DB758 != 0)
    {
        z = lbl_803DF108;
        dst = lbl_8039A7B8;
        dst[0] = z;
        dst[1] = z;
        one = lbl_803DF114;
        dst[2] = one;
        c150 = -0.707f;
        dst[3] = c150;
        dst[4] = z;
        c154 = 0.707f;
        dst[5] = c154;
        c158 = (-1.0f);
        dst[6] = c158;
        dst[7] = z;
        dst[8] = z;
        dst[9] = c150;
        dst[10] = z;
        dst[11] = c150;
        dst[12] = z;
        dst[13] = z;
        dst[14] = c158;
        dst[15] = c154;
        dst[16] = z;
        dst[17] = c150;
        dst[18] = one;
        dst[19] = z;
        dst[20] = z;
        dst[21] = c154;
        dst[22] = z;
        dst[23] = c154;
        lbl_803DB758 = 0;
    }
    cam = Camera_GetCurrentViewSlot();
    zv = lbl_803DF108;
    vec[0] = zv;
    vec[1] = zv;
    vec[2] = (-1.0f);
    q.x = zv;
    q.y = zv;
    q.z = zv;
    q.w = lbl_803DF114;
    q.rx = -cam->yaw;
    q.rz = 0;
    q.ry = 0;
    vecRotateZXY(&q.rx, vec);
    i = 0;
    pp = &gSky2State;
    do
    {
        if (*pp != NULL && *(s8*)(*pp + 0x317) != 0)
        {
            lbl_803DB750 = 0;
            p = *pp;
            if (*(int*)(p + 0x48) != 0)
            {
                if ((*(u16*)&((GameObject*)p)->anim.rotZ & 1) == 0)
                {
                    spd = 255.0f;
                    ((SkySlotAnim*)p)->blend = spd * ((SkySlotAnim*)p)->prevT;
                    if (((SkySlotAnim*)*pp)->blend > spd)
                    {
                        ((SkySlotAnim*)*pp)->blend = spd;
                    }
                }
            }
            else if (*(int*)(p + 0x44) != 0)
            {
                ((SkySlotAnim*)p)->prevT = ((SkySlotAnim*)p)->blend / 255.0f;
                p = *pp;
                if ((*(u16*)&((GameObject*)p)->anim.rotZ & 1) == 0) {
                    ((SkySlotAnim*)p)->blend = -(timeDelta * *(f32*)(p + 0x58) - ((SkySlotAnim*)p)->blend);
                    frzero = ((SkySlotAnim*)*pp)->blend;
                    if (frzero < lbl_803DF108) {
                        ((SkySlotAnim*)*pp)->blend = lbl_803DF108;
                    }
                }
            }
            if ((*(u16*)(*pp + 4) & 0x100) != 0)
            {
                sky2StepSlotAnim(i);
            }
            p = *pp;
            if ((*(u16*)&((GameObject*)p)->anim.rotZ & 0x10) != 0)
            {
                r = *(f32*)&((GameObject*)p)->anim.textureSlots;
                g = ((GameObject*)p)->anim.activeMoveProgress;
                b = *(f32*)&((GameObject*)p)->childObjs[0];
                sa = ((SkySlotAnim*)p)->cur2[0];
                sb = ((SkySlotAnim*)p)->cur2[0xb];
            }
            else if ((*(u16*)&((GameObject*)p)->anim.flags & 0x20) != 0)
            {
                (*gSkyInterface)->getTimeOfDay(&height);
                if ((t = height / lbl_803DF15C) < lbl_803DF108)
                {
                    t = lbl_803DF108;
                }
                if (t > lbl_803DF114)
                {
                    t = lbl_803DF114;
                }
                step = lbl_803DF160;
                if (t <= step)
                {
                    u = t / step;
                    k = 0;
                }
                else if (t <= lbl_803DF144)
                {
                    u = (t - step) / step;
                    k = 1;
                }
                else if (t <= 0.375f)
                {
                    u = (t - lbl_803DF144) / step;
                    k = 2;
                }
                else if (t <= lbl_803DF168)
                {
                    u = (t - 0.375f) / step;
                    k = 3;
                }
                else if (t <= lbl_803DF16C)
                {
                    u = (t - lbl_803DF168) / step;
                    k = 4;
                }
                else if (t <= lbl_803DF170)
                {
                    u = (t - lbl_803DF16C) / step;
                    k = 5;
                }
                else if (t <= lbl_803DF174)
                {
                    u = (t - lbl_803DF170) / step;
                    k = 6;
                }
                else
                {
                    u = (t - lbl_803DF174) / step;
                    k = 7;
                }
                r = Curve_EvalCatmullRom(*pp + (off1 = k * 4) + 0x70, u, 0);
                g = Curve_EvalCatmullRom(*pp + (off2 = (k + 0xb) * 4) + 0x70, u, 0);
                b = Curve_EvalCatmullRom(*pp + (k + 0x16) * 4 + 0x70, u, 0);
                sa = Curve_EvalCatmullRom(*pp + off1 + 0x1fc, u, 0);
                sb = Curve_EvalCatmullRom(*pp + off2 + 0x1fc, u, 0);
            }
            else
            {
                k = 0;
                do
                {
                    a1 = getAngle(lbl_8039A7B8[k * 3], lbl_8039A7B8[k * 3 + 2]);
                    d = a1 - (u16)getAngle(vec[0], vec[2]);
                    if (d < 0)
                    {
                        d *= -1;
                    }
                    if (d > 0x7fff)
                    {
                        d = 0xffff - d;
                    }
                    att = (lbl_803DF178 - d) / lbl_803DF178;
                    att -= lbl_803DF170;
                    att /= lbl_803DF144;
                    if (att > best.x)
                    {
                        if (best.x > best.y)
                        {
                            best.y = best.x;
                            idx.second = idx.best;
                        }
                        best.x = att;
                        idx.best = k;
                    }
                    else if (att > best.y)
                    {
                        best.y = att;
                        idx.second = k;
                    }
                    k++;
                } while (k < 8);
                z2 = lbl_803DF108;
                if (best.x > z2)
                {
                    p = *pp + (off1 = idx.best * 4);
                    r = *(f32*)&((GameObject*)p)->anim.textureSlots * best.x + r;
                    g = ((GameObject*)p)->anim.activeMoveProgress * best.x + g;
                    b = *(f32*)&((GameObject*)p)->childObjs[0] * best.x + b;
                    sa = *(f32*)(*pp + off1 + 0x1fc) * best.x + sa;
                    sb = ((SkySlotAnim*)p)->cur2[0xb] * best.x + sb;
                }
                if (best.y > z2)
                {
                    p = *pp + (off2 = idx.second * 4);
                    r = *(f32*)&((GameObject*)p)->anim.textureSlots * best.y + r;
                    g = ((GameObject*)p)->anim.activeMoveProgress * best.y + g;
                    b = *(f32*)&((GameObject*)p)->childObjs[0] * best.y + b;
                    sa = *(f32*)(*pp + off2 + 0x1fc) * best.y + sa;
                    sb = ((SkySlotAnim*)p)->cur2[0xb] * best.y + sb;
                }
            }
            if (r > 255.0f)
            {
                r = 255.0f;
            }
            else if (r < lbl_803DF108)
            {
                r = lbl_803DF108;
            }
            cmax = 255.0f;
            if (g > 255.0f)
            {
                g = cmax;
            }
            else if (g < lbl_803DF108)
            {
                g = lbl_803DF108;
            }
            if (b > 255.0f)
            {
                b = cmax;
            }
            else if (b < lbl_803DF108)
            {
                b = lbl_803DF108;
            }
            p = *pp;
            if ((*(u16*)&((GameObject*)p)->anim.flags & 0x40) != 0)
            {
                if (((SkySlotAnim*)p)->b314 == -1)
                {
                    ((SkySlotAnim*)p)->b314 = 1;
                    frzero = lbl_803DF108;
                    *(f32*)(*pp + 0x6c) = frzero;
                    diff = sb - sa;
                    *(f32*)(*pp + 0x68) = randomGetRange((int)(-diff * lbl_803DF168), (int)(diff * lbl_803DF168));
                    *(f32*)(*pp + 0x64) = lbl_803DF17C * randomGetRange(1, 10);
                }
                else if (((SkySlotAnim*)p)->b314 == 1)
                {
                    hv = *(f32*)&((GameObject*)p)->anim.jointPoseData;
                    sa = sa + hv;
                    *(f32*)&((GameObject*)p)->anim.jointPoseData = hv + *(f32*)&((GameObject*)p)->anim.modelState;
                    p = *pp;
                    if (*(f32*)&((GameObject*)p)->anim.jointPoseData > *(f32*)&((GameObject*)p)->anim.dll)
                    {
                        ((SkySlotAnim*)p)->b314 = (s8)(1 - ((SkySlotAnim*)p)->b314);
                    }
                }
                else
                {
                    hv = *(f32*)&((GameObject*)p)->anim.jointPoseData;
                    sa = sa + hv;
                    *(f32*)&((GameObject*)p)->anim.jointPoseData = hv - *(f32*)&((GameObject*)p)->anim.modelState;
                    p = *pp;
                    frzero = *(f32*)&((GameObject*)p)->anim.jointPoseData;
                    if (frzero < lbl_803DF108) {
                        ((SkySlotAnim*)p)->b314 = (s8)(1 - ((SkySlotAnim*)p)->b314);
                        *(f32*)(*pp + 0x6c) = lbl_803DF108;
                        amp = (s16)(int)(sb - sa);
                        *(f32*)(*pp + 0x68) = randomGetRange(-amp / 2, amp / 2);
                        *(f32*)(*pp + 0x64) = lbl_803DF17C * randomGetRange(1, 10);
                    }
                }
            }
            if (sb > lbl_803DF180)
            {
                sb = lbl_803DF180;
            }
            if (sa > sb)
            {
                sa = sb - lbl_803DF114;
            }
            if (sa <= lbl_803DF108)
            {
                setStarsHidden(1);
            }
            else
            {
                setStarsHidden(0);
            }
            p = *pp;
            flags = *(u16*)&((GameObject*)p)->anim.rotZ;
            if ((flags & 8) == 0)
            {
                scale = (f32)(red + green + blue) / lbl_803DF184;
                r *= scale;
                g *= scale;
                b *= scale;
            }
            if ((flags & 1) != 0)
            {
                *(int*)&((GameObject*)p)->anim.velocityX = r;
                *(int*)(*pp + 0x28) = g;
                *(int*)(*pp + 0x2c) = b;
                *(f32*)(*pp + 0x14) = sa;
                *(f32*)(*pp + 0x18) = sb;
                if ((*(u16*)(*pp + 4) & 0x80) == 0)
                {
                    *(int*)(*pp + 0x30) = 0xff;
                    *(int*)(*pp + 0x34) = 0xff;
                    *(int*)(*pp + 0x38) = 0xff;
                    *(f32*)(*pp + 0x1c) = lbl_803DF188;
                    *(f32*)(*pp + 0x20) = lbl_803DF18C;
                }
            }
            else if ((flags & 4) != 0)
            {
                *(int*)&((GameObject*)p)->anim.parent = r;
                *(int*)(*pp + 0x34) = g;
                *(int*)(*pp + 0x38) = b;
                *(f32*)(*pp + 0x1c) = sa;
                *(f32*)(*pp + 0x20) = sb;
                if ((*(u16*)(*pp + 4) & 0x80) == 0)
                {
                    *(int*)(*pp + 0x24) = 0xff;
                    *(int*)(*pp + 0x28) = 0xff;
                    *(int*)(*pp + 0x2c) = 0xff;
                    *(f32*)(*pp + 0x14) = lbl_803DF188;
                    *(f32*)(*pp + 0x18) = lbl_803DF18C;
                }
            }
            else
            {
                ri = r;
                *(int*)&((GameObject*)p)->anim.velocityX = ri;
                gi = g;
                *(int*)(*pp + 0x28) = gi;
                bi = b;
                *(int*)(*pp + 0x2c) = bi;
                *(f32*)(*pp + 0x14) = sa;
                *(f32*)(*pp + 0x18) = sb;
                *(int*)(*pp + 0x30) = ri;
                *(int*)(*pp + 0x34) = gi;
                *(int*)(*pp + 0x38) = bi;
                *(f32*)(*pp + 0x1c) = sa;
                *(f32*)(*pp + 0x20) = sb;
            }
        }
        pp++;
        i++;
    } while (i < 2);
}

void sky2_onMapSetup(void)
{
    void** slot;
    int i;
    f32 b;
    f32 a;

    lbl_803DB610 = -1;
    (&lbl_803DB610)[1] = -1;
    i = 0;
    slot = (void**)&gSky2State;
    a = lbl_803DF190;
    b = lbl_803DF194;
    for (; i < 2; i++)
    {
        if (*slot == NULL)
        {
            *slot = mmAlloc(792, 23, 0);
        }
        memset(*slot, 0, 792);
        *(int*)((char*)*slot + 0x24) = 255;
        *(int*)((char*)*slot + 0x28) = 255;
        *(int*)((char*)*slot + 0x2c) = 255;
        *(f32*)((char*)*slot + 0x14) = a;
        *(f32*)((char*)*slot + 0x18) = b;
        *(int*)((char*)*slot + 0x30) = 255;
        *(int*)((char*)*slot + 0x34) = 255;
        *(int*)((char*)*slot + 0x38) = 255;
        *(f32*)((char*)*slot + 0x1c) = a;
        *(f32*)((char*)*slot + 0x20) = b;
        if (lbl_803DB754 != 0)
        {
            getEnvfxAct(NULL, NULL, 9, 0);
            lbl_803DB754 = 0;
        }
        slot++;
    }
}

void sky2_update(int a, int b, u8* cfg)
{
    u8* env;
    u16 bits;
    u8* st;
    int m40;
    u8 flags;
    u8 flags58;
    u8 b1;
    u8 i;

    flags = 0;
    env = saveGameGetEnvState();
    if (cfg != NULL)
    {
        (&lbl_803DB610)[1] = lbl_803DB610 = (s16)((Sky2Config*)cfg)->envfxActId - 1;
        *(s16*)(env + 0xc) = (s16)((Sky2Config*)cfg)->envfxActId - 1;
        flags58 = ((Sky2Config*)cfg)->flags;
        b1 = (flags58 & 0x80) ? 1 : 0;
        if (*(s8*)((&gSky2State)[b1] + 0x317) == 0)
        {
            if ((flags58 & 0x40) != 0)
            {
                flags |= 0x40;
            }
            sky2ResetStateFromConfig(cfg, flags);
            if ((((Sky2Config*)cfg)->flags & 0x40) != 0)
            {
                (&gSky2State)[b1][0x316] = 1;
            }
            *(u16*)((&gSky2State)[b1] + 4) = ((Sky2Config*)cfg)->flags | 0x100;
            (&gSky2State)[b1][0x315] = 1;
            *(f32*)((&gSky2State)[b1] + 0x304) = lbl_803DF108;
        }
        else if ((flags58 & 0x20) != 0)
        {
            getEnvfxAct(0, 0, 9, 0);
        }
        else
        {
            *(u16*)((&gSky2State)[b1] + 4) = ((Sky2Config*)cfg)->flags | 0x100;
            (&gSky2State)[b1][0x315] = 1;
            *(f32*)((&gSky2State)[b1] + 0x304) = lbl_803DF108;
            for (i = 0; i < SKY_CONFIG_FIELD_COUNT; i++)
            {
                *(f32*)((&gSky2State)[b1] + i * 4 + 0xf4) = (f32)(u32)cfg[gSkyConfigFieldIndices[i] + 0xc];
                *(f32*)((&gSky2State)[b1] + i * 4 + 0x120) = (f32)(u32)cfg[gSkyConfigFieldIndices[i] + 0x14];
                *(f32*)((&gSky2State)[b1] + i * 4 + 0x14c) = (f32)(u32)cfg[gSkyConfigFieldIndices[i] + 0x1c];
                *(f32*)((&gSky2State)[b1] + i * 4 + 0x254) =
                    (f32)(u32) * (u16*)(cfg + gSkyConfigFieldIndices[i] * 2 + 0x3e);
                *(f32*)((&gSky2State)[b1] + i * 4 + 0x280) =
                    (f32)(u32) * (u16*)(cfg + gSkyConfigFieldIndices[i] * 2 + 0x2e);
            }
            *(int*)((&gSky2State)[b1] + 0x3c) = ((Sky2Config*)cfg)->fadeDurationA;
            *(int*)((&gSky2State)[b1] + 0x40) = ((Sky2Config*)cfg)->fadeDurationB;
            *(s8*)((&gSky2State)[b1] + 0x314) = -1;
            if ((((Sky2Config*)cfg)->flags2 & 0x20) != 0)
            {
                st = (&gSky2State)[b1];
                bits = *(u16*)(st + 6);
                if ((bits & 0x20) == 0)
                {
                    *(u16*)(st + 6) = bits | 0x20;
                }
            }
            if ((((Sky2Config*)cfg)->flags2 & 0x20) == 0)
            {
                st = (&gSky2State)[b1];
                bits = *(u16*)(st + 6);
                if ((bits & 0x20) != 0)
                {
                    *(u16*)(st + 6) = bits ^ 0x20;
                }
            }
            if ((((Sky2Config*)cfg)->flags & 0x40) != 0)
            {
                *(u16*)((&gSky2State)[b1] + 6) |= 0x40;
                (&gSky2State)[b1][0x316] = 1;
            }
            else
            {
                st = (&gSky2State)[b1];
                bits = *(u16*)(st + 6);
                if ((bits & 0x40) != 0)
                {
                    *(u16*)(st + 6) = bits ^ 0x40;
                }
            }
            m40 = ((Sky2Config*)cfg)->flags2 & 0x40;
            if (m40 != 0)
            {
                st = (&gSky2State)[b1];
                bits = *(u16*)(st + 6);
                if ((bits & 0x40) == 0)
                {
                    *(u16*)(st + 6) = bits | 0x40;
                    return;
                }
            }
            if (m40 == 0)
            {
                st = (&gSky2State)[b1];
                bits = *(u16*)(st + 6);
                if ((bits & 0x40) != 0)
                {
                    *(u16*)(st + 6) = bits ^ 0x40;
                }
            }
        }
    }
}

void sky2_release(void)
{
}

void sky2_initialise(void)
{
    u8* state;

    lbl_803DB610 = -1;
    (&lbl_803DB610)[1] = -1;
    if (gSky2State != NULL)
    {
        mm_free(gSky2State);
    }
    state = (&gSky2State)[1];
    if (state != NULL)
    {
        mm_free(state);
    }
    gSky2State = NULL;
    (&gSky2State)[1] = NULL;
}

u8 gSkyConfigFieldIndices[] = {0, 0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0};

ObjectDescriptor17 sky2_funcs = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_16_SLOTS,
    (ObjectDescriptorCallback)sky2_initialise,
    (ObjectDescriptorCallback)sky2_release,
    0,
    (ObjectDescriptorCallback)sky2_update,
    (ObjectDescriptorCallback)sky2_onMapSetup,
    (ObjectDescriptorCallback)sky2_run,
    (ObjectDescriptorCallback)sky2ApplyFog,
    (ObjectDescriptorCallback)dll_06_func07_ret_0,
    (ObjectDescriptorCallback)sky2ApplyTextColor,
    (ObjectDescriptorExtraSizeCallback)sky2BlendTowardTargetColor,
    (ObjectDescriptorCallback)sky2GetTargetColor,
    (ObjectDescriptorCallback)sky2GetFogRange,
    (ObjectDescriptorCallback)dll_06_func0C_nop,
    (ObjectDescriptorCallback)sky2SetDrawMode2,
    (ObjectDescriptorCallback)sky2SetDrawMode1,
    (ObjectDescriptorCallback)sky2GetFogFadeAlpha,
};

u8 lbl_8030F500[160] = {255, 206, 0,   0,   255, 206, 255, 206, 0, 100, 255, 206, 0, 50,  0, 100, 255, 206, 0, 50,
                        0,   0,   255, 206, 255, 206, 0,   0,   0, 50,  255, 206, 0, 100, 0, 50,  0,   50,  0, 100,
                        0,   50,  0,   50,  0,   0,   0,   50,  0, 0,   0,   0,   0, 0,   0, 6,   0,   0,   0, 2,
                        0,   0,   0,   8,   0,   0,   0,   2,   0, 0,   0,   16,  0, 0,   0, 8,   0,   0,   0, 32,
                        0,   0,   0,   40,  0,   0,   0,   48,  0, 0,   0,   1,   0, 0,   0, 2,   0,   0,   0, 2,
                        0,   0,   0,   4,   0,   0,   0,   3,   0, 0,   0,   6,   0, 0,   0, 6,   0,   0,   0, 12,
                        0,   0,   0,   12,  0,   0,   0,   24,  0, 0,   0,   24,  0, 0,   0, 32,  0,   0,   0, 32,
                        0,   0,   0,   40,  0,   0,   0,   40,  0, 0,   0,   48,  0, 0,   0, 48,  0,   0,   0, 56};

f32 lbl_8039A7B8[0x18];
