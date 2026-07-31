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
#include "main/lightmap.h"
#include "main/track_dolphin_shadow_api.h"
#include "string.h"

u32 lbl_803DD18C;
u32 lbl_803DD188;
u8* gSky2State;
s8 gSky2DrawMode;

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
extern int gSky2EnvfxActIndex;
extern s8 gSky2DrawMode;
extern u8* gSky2State;
extern u16 lbl_803E8460;
extern u8 lbl_803E8462;
extern f32 lbl_8039A7B8[];
const SkyVec3 lbl_802C1F98 = {-1000.0f, -1000.0f, -1000.0f};




void skyGetCurrentAmbientAndLightColors(u8* ambientRed, u8* ambientGreen, u8* ambientBlue, u8* lightRed, u8* lightGreen,
                                        u8* lightBlue);




void skySetLightSlot(int slot, f32 x, f32 y, f32 z, int red, int green, int blue, int ambientIntensity,
                 int lightIntensity, u8 blendAlpha);


void sky2GetFogRange(int* fogNear, int* fogFar)
{
    u8* state;
    f32 value;

    state = gSky2State;
    if (state != NULL)
    {
        value = ((SkySlotAnim*)state)->fogNear;
        *fogNear = value;
        value = ((SkySlotAnim*)gSky2State)->fogFar;
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
    *red = ((SkySlotAnim*)state)->colorR;
    *green = ((SkySlotAnim*)gSky2State)->colorG;
    *blue = ((SkySlotAnim*)gSky2State)->colorB;
    *blend = ((SkySlotAnim*)gSky2State)->prevT;
}

void sky2ResetStateFromConfig(u8* cfg, u8 flags)
{
    int i;
    int idx;

    if (((Sky2Config*)cfg)->flags & 0x80)
    {
        idx = 1;
    }
    else
    {
        idx = 0;
    }
    ((SkySlotAnim*)(&gSky2State)[idx])->unk00 = 0;
    ((SkySlotAnim*)(&gSky2State)[idx])->b317 = 1;
    for (i = 0; i < 0x21; i++)
    {
        ((SkySlotAnim*)(&gSky2State)[idx])->vel[i] = 0.0f;
    }
    for (i = 0; i < 0x21; i++)
    {
        ((SkySlotAnim*)(&gSky2State)[idx])->cur[i] = 0.0f;
    }
    for (i = 0; i < 0x16; i++)
    {
        ((SkySlotAnim*)(&gSky2State)[idx])->vel2[i] = 0.0f;
    }
    for (i = 0; i < SKY_CONFIG_FIELD_COUNT; i++)
    {
        ((SkySlotAnim*)(&gSky2State)[idx])->cur2[i] = 1400.0f;
        ((SkySlotAnim*)(&gSky2State)[idx])->cur2[i + 0xb] = 1600.0f;
    }
    for (i = 0; i < SKY_CONFIG_FIELD_COUNT; i++)
    {
        ((SkySlotAnim*)(&gSky2State)[idx])->target[i] = (f32)(u32)((Sky2Config*)cfg)->redKeys[gSkyConfigFieldIndices[i]];
        ((SkySlotAnim*)(&gSky2State)[idx])->target[i + 0xb] = (f32)(u32)((Sky2Config*)cfg)->greenKeys[gSkyConfigFieldIndices[i]];
        ((SkySlotAnim*)(&gSky2State)[idx])->target[i + 0x16] = (f32)(u32)((Sky2Config*)cfg)->blueKeys[gSkyConfigFieldIndices[i]];
        ((SkySlotAnim*)(&gSky2State)[idx])->target2[i] = (f32)(u32)((Sky2Config*)cfg)->fogNearKeys[gSkyConfigFieldIndices[i]];
        ((SkySlotAnim*)(&gSky2State)[idx])->target2[i + 0xb] = (f32)(u32)((Sky2Config*)cfg)->fogFarKeys[gSkyConfigFieldIndices[i]];
    }
    ((SkySlotAnim*)(&gSky2State)[idx])->flags4 = cfg[0x58];
    ((SkySlotAnim*)(&gSky2State)[idx])->flags6 = ((Sky2Config*)cfg)->flags2;
    ((SkySlotAnim*)(&gSky2State)[idx])->wobbleStep = 0.0f;
    ((SkySlotAnim*)(&gSky2State)[idx])->wobbleAmp = 0.0f;
    ((SkySlotAnim*)(&gSky2State)[idx])->b314 = -1;
    ((SkySlotAnim*)(&gSky2State)[idx])->wobbleOffset = 0.0f;
    if (((Sky2Config*)cfg)->fadeDurationA == 0)
    {
        ((Sky2Config*)cfg)->fadeDurationA = 1;
    }
    if (((Sky2Config*)cfg)->fadeDurationA != 0)
    {
        ((SkySlotAnim*)(&gSky2State)[idx])->fadeDurationA = ((Sky2Config*)cfg)->fadeDurationA;
        ((SkySlotAnim*)(&gSky2State)[idx])->unk48 = 1;
        ((SkySlotAnim*)(&gSky2State)[idx])->unk08 = ((Sky2Config*)cfg)->skyTexId0;
        ((SkySlotAnim*)(&gSky2State)[idx])->unk5C = 1.0f / (f32)(u32)((Sky2Config*)cfg)->fadeDurationA;
    }
    else
    {
        ((SkySlotAnim*)(&gSky2State)[idx])->fadeDurationA = 0;
        ((SkySlotAnim*)(&gSky2State)[idx])->unk5C = 1.0f;
    }
    if (((Sky2Config*)cfg)->fadeDurationB == 0)
    {
        ((Sky2Config*)cfg)->fadeDurationB = 1;
    }
    if (((Sky2Config*)cfg)->fadeDurationB != 0)
    {
        ((SkySlotAnim*)(&gSky2State)[idx])->fadeDurationB = ((Sky2Config*)cfg)->fadeDurationB;
        ((SkySlotAnim*)(&gSky2State)[idx])->fadeRate =
            255.0f / (60.0f * ((f32)(u32)((Sky2Config*)cfg)->fadeDurationB / 10.0f));
        ((SkySlotAnim*)(&gSky2State)[idx])->unk0C = 0x5dc;
        ((SkySlotAnim*)(&gSky2State)[idx])->unk60 = 1.0f / (f32)(u32)((Sky2Config*)cfg)->fadeDurationB;
    }
    else
    {
        ((SkySlotAnim*)(&gSky2State)[idx])->fadeDurationB = 0;
        ((SkySlotAnim*)(&gSky2State)[idx])->unk60 = 1.0f;
    }
    ((SkySlotAnim*)(&gSky2State)[idx])->unk44 = 0;
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
    if (anim->t >= (dur = 1.0f))
    {
        anim->flags4 &= ~0x100;
        zero = 0.0f;
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
            len = 60.0f * ((f32)anim->fadeDurationA / 10.0f);
            if (0.0f == len)
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
        if (flag1 != 0 && (bv = anim->blend) > (zero = 0.0f))
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
    int alpha;

    state = gSky2State;
    if (state == NULL)
    {
        return 0xff;
    }
    y = ((SkySlotAnim*)state)->fogNear;
    if (y < 950.0f)
    {
        alpha = 0;
    }
    else if (y > 1210.0f)
    {
        alpha = 0xff;
    }
    else
    {
        alpha = (int)(255.0f * ((y - 950.0f) / 200.0f));
    }
    return alpha;
}

void dll_06_func0C_nop(void)
{
}

void sky2BlendTowardTargetColor(s32* red, s32* green, s32* blue)
{
    SkySlotAnim* state;
    s32 targetR;
    s32 targetG;
    s32 targetB;
    s32 oldR;
    s32 oldG;
    s32 oldB;
    f32 blend;
    f32 fy;
    f32 fz;

    blend = 0.0f;
    state = (SkySlotAnim*)gSky2State;
    if (state == NULL)
    {
        return;
    }
    if (state != NULL && state->b316 == 0)
    {
        return;
    }

    oldR = *red;
    oldG = *green;
    oldB = *blue;
    if (state != NULL)
    {
        targetR = state->colorR;
        targetG = state->colorG;
        targetB = state->colorB;
        blend = state->prevT;
    }

    fy = (f32)(targetG - oldG);
    fz = (f32)(targetB - oldB);
    *red = (s32)((f32)(targetR - oldR) * (blend = 0.25f * blend) + oldR);
    *green = (s32)(fy * blend + oldG);
    *blue = (s32)(fz * blend + oldB);
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

void sky2ApplyModelTint(GameObject* obj)
{
    u8* s;
    f32 v;
    int alpha;

    if (gSky2State == NULL)
    {
        Obj_SetModelColorOverrideRecursive(obj, 0, 0, 0, 0, 0);
    }
    if (lbl_803DB750 == 0 && (*(u16*)((s = gSky2State) + 4) & 1) == 0)
    {
        v = ((SkySlotAnim*)s)->fogNear;
        if (v < 0.0f)
        {
            alpha = 255;
        }
        else if (v > 15.0f)
        {
            alpha = 0;
        }
        else
        {
            alpha = (int)(255.0f - 255.0f * (v / 15.0f));
        }
        Obj_SetModelColorOverrideRecursive(obj, (u8)((SkySlotAnim*)s)->colorR, (u8)((SkySlotAnim*)s)->colorG,
                                           (u8)((SkySlotAnim*)s)->colorB, (u8)alpha, 1);
    }
    else
    {
        Obj_SetModelColorOverrideRecursive(obj, 0, 0, 0, 0, 0);
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
            v = ((SkySlotAnim*)s)->fogNear;
            if (v < 0.0f)
            {
                alpha = 255;
            }
            else if (v > 15.0f)
            {
                alpha = 0;
            }
            else
            {
                alpha = (int)(255.0f - 255.0f * (v / 15.0f));
            }
            setTextColor((void*)obj, (u8)((SkySlotAnim*)s)->colorR, (u8)((SkySlotAnim*)s)->colorG,
                         (u8)((SkySlotAnim*)s)->colorB, (u8)alpha);
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
        setFogColorCallback(obj, (u8)((SkySlotAnim*)s)->colorR, (u8)((SkySlotAnim*)s)->colorG, (u8)((SkySlotAnim*)s)->colorB, 55);
        s = gSky2State;
        if (*(f32*)(s + 0x14) == *(f32*)(s + 0x18))
        {
            *(f32*)(s + 0x14) = *(f32*)(s + 0x14) - 20.0f;
        }
        s = gSky2State;
        if (*(f32*)(s + 0x14) > *(f32*)(s + 0x18))
        {
            *(f32*)(s + 0x14) = *(f32*)(s + 0x18) - 20.0f;
        }
        s = gSky2State;
        fogSetRange(*(f32*)(s + 0x14), *(f32*)(s + 0x18));
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
    Camera* cam;
    u8** pp;
    int i;
    u8* p;
    f32* dst;
    f32 colorMax;
    int k;
    int d;
    int bestKnotOffset;
    int secondKnotOffset;
    u16 a1;
    int range;
    int redInt;
    int greenInt;
    int blueInt;
    u16 flags;
    f32 r;
    f32 g;
    f32 b;
    f32 sa;
    f32 sb;
    f32 step;
    f32 t;
    f32 u;
    f32 directionWeight;
    f32 zero;
    f32 c158;
    f32 c154;
    f32 c150;
    f32 one;
    f32 z;
    f32 zv;
    f32 spd;
    f32 value;
    f32 offset;
    f32 negativeRange;
    f32 ambientScale;

    best = lbl_802C1F98;
    r = 0.0f;
    g = r;
    b = r;
    sa = r;
    sb = r;
    height = r;
    *(u16*)&idx = lbl_803E8460;
    idx.pad = lbl_803E8462;
    skyGetSunColor(0, &red, &green, &blue);
    if (lbl_803DB758 != 0)
    {
        z = 0.0f;
        dst = lbl_8039A7B8;
        dst[0] = z;
        dst[1] = z;
        one = 1.0f;
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
    cam = Camera_GetCurrent();
    zv = 0.0f;
    vec[0] = zv;
    vec[1] = zv;
    vec[2] = (-1.0f);
    q.x = zv;
    q.y = zv;
    q.z = zv;
    q.w = 1.0f;
    q.rx = -cam->yaw;
    q.rz = 0;
    q.ry = 0;
    vecRotateZXY(&q.rx, vec);
    i = 0;
    pp = &gSky2State;
    do
    {
        if (*pp != NULL && ((SkySlotAnim*)*pp)->b317 != 0)
        {
            lbl_803DB750 = 0;
            p = *pp;
            if (((SkySlotAnim*)p)->unk48 != 0)
            {
                if ((((SkySlotAnim*)p)->flags4 & 1) == 0)
                {
                    spd = 255.0f;
                    ((SkySlotAnim*)p)->blend = spd * ((SkySlotAnim*)p)->prevT;
                    if (((SkySlotAnim*)*pp)->blend > spd)
                    {
                        ((SkySlotAnim*)*pp)->blend = spd;
                    }
                }
            }
            else if (((SkySlotAnim*)p)->unk44 != 0)
            {
                ((SkySlotAnim*)p)->prevT = ((SkySlotAnim*)p)->blend / 255.0f;
                p = *pp;
                if ((((SkySlotAnim*)p)->flags4 & 1) == 0) {
                    ((SkySlotAnim*)p)->blend = -(timeDelta * ((SkySlotAnim*)p)->fadeRate - ((SkySlotAnim*)p)->blend);
                    value = ((SkySlotAnim*)*pp)->blend;
                    if (value < 0.0f) {
                        ((SkySlotAnim*)*pp)->blend = 0.0f;
                    }
                }
            }
            if ((((SkySlotAnim*)*pp)->flags4 & 0x100) != 0)
            {
                sky2StepSlotAnim(i);
            }
            p = *pp;
            if ((((SkySlotAnim*)p)->flags4 & 0x10) != 0)
            {
                r = ((SkySlotAnim*)p)->cur[0];
                g = ((SkySlotAnim*)p)->cur[0xb];
                b = ((SkySlotAnim*)p)->cur[0x16];
                sa = ((SkySlotAnim*)p)->cur2[0];
                sb = ((SkySlotAnim*)p)->cur2[0xb];
            }
            else if ((((SkySlotAnim*)p)->flags6 & 0x20) != 0)
            {
                (*gSkyInterface)->getTimeOfDay(&height);
                if ((t = height / 86400.0f) < 0.0f)
                {
                    t = 0.0f;
                }
                if (t > 1.0f)
                {
                    t = 1.0f;
                }
                step = 0.125f;
                if (t <= step)
                {
                    u = t / step;
                    k = 0;
                }
                else if (t <= 0.25f)
                {
                    u = (t - step) / step;
                    k = 1;
                }
                else if (t <= 0.375f)
                {
                    u = (t - 0.25f) / step;
                    k = 2;
                }
                else if (t <= 0.5f)
                {
                    u = (t - 0.375f) / step;
                    k = 3;
                }
                else if (t <= 0.625f)
                {
                    u = (t - 0.5f) / step;
                    k = 4;
                }
                else if (t <= 0.75f)
                {
                    u = (t - 0.625f) / step;
                    k = 5;
                }
                else if (t <= 0.875f)
                {
                    u = (t - 0.75f) / step;
                    k = 6;
                }
                else
                {
                    u = (t - 0.875f) / step;
                    k = 7;
                }
                r = Curve_EvalCatmullRom(&((SkySlotAnim*)*pp)->cur[k], u, 0);
                g = Curve_EvalCatmullRom(&((SkySlotAnim*)*pp)->cur[k + 0xb], u, 0);
                b = Curve_EvalCatmullRom(&((SkySlotAnim*)*pp)->cur[k + 0x16], u, 0);
                sa = Curve_EvalCatmullRom(&((SkySlotAnim*)*pp)->cur2[k], u, 0);
                sb = Curve_EvalCatmullRom(&((SkySlotAnim*)*pp)->cur2[k + 0xb], u, 0);
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
                    directionWeight = (32767.0f - d) / 32767.0f;
                    directionWeight -= 0.75f;
                    directionWeight /= 0.25f;
                    if (directionWeight > best.x)
                    {
                        if (best.x > best.y)
                        {
                            best.y = best.x;
                            idx.second = idx.best;
                        }
                        best.x = directionWeight;
                        idx.best = k;
                    }
                    else if (directionWeight > best.y)
                    {
                        best.y = directionWeight;
                        idx.second = k;
                    }
                    k++;
                } while (k < 8);
                zero = 0.0f;
                if (zero < best.x)
                {
                    p = *pp + (bestKnotOffset = idx.best * 4);
                    r = ((SkySlotAnim*)p)->cur[0] * best.x + r;
                    g = ((SkySlotAnim*)p)->cur[0xb] * best.x + g;
                    b = ((SkySlotAnim*)p)->cur[0x16] * best.x + b;
                    sa = *(f32*)(*pp + bestKnotOffset + 0x1fc) * best.x + sa;
                    sb = ((SkySlotAnim*)p)->cur2[0xb] * best.x + sb;
                }
                if (best.y > zero)
                {
                    p = *pp + (secondKnotOffset = idx.second * 4);
                    r = ((SkySlotAnim*)p)->cur[0] * best.y + r;
                    g = ((SkySlotAnim*)p)->cur[0xb] * best.y + g;
                    b = ((SkySlotAnim*)p)->cur[0x16] * best.y + b;
                    sa = *(f32*)(*pp + secondKnotOffset + 0x1fc) * best.y + sa;
                    sb = ((SkySlotAnim*)p)->cur2[0xb] * best.y + sb;
                }
            }
            if (r > 255.0f)
            {
                r = 255.0f;
            }
            else if (r < 0.0f)
            {
                r = 0.0f;
            }
            colorMax = 255.0f;
            if (g > 255.0f)
            {
                g = colorMax;
            }
            else if (g < 0.0f)
            {
                g = 0.0f;
            }
            if (b > 255.0f)
            {
                b = colorMax;
            }
            else if (b < 0.0f)
            {
                b = 0.0f;
            }
            p = *pp;
            if ((((SkySlotAnim*)p)->flags6 & 0x40) != 0)
            {
                if (((SkySlotAnim*)p)->b314 == -1)
                {
                    ((SkySlotAnim*)p)->b314 = 1;
                    value = 0.0f;
                    ((SkySlotAnim*)*pp)->wobbleOffset = value;
                    negativeRange = -(sb - sa);
                    ((SkySlotAnim*)*pp)->wobbleAmp =
                        randomGetRange((int)(negativeRange * 0.5f), (int)(-negativeRange * 0.5f));
                    ((SkySlotAnim*)*pp)->wobbleStep = 0.05f * randomGetRange(1, 10);
                }
                else if (((SkySlotAnim*)p)->b314 == 1)
                {
                    offset = ((SkySlotAnim*)p)->wobbleOffset;
                    sa = sa + offset;
                    ((SkySlotAnim*)p)->wobbleOffset = offset + ((SkySlotAnim*)p)->wobbleStep;
                    p = *pp;
                    if (((SkySlotAnim*)p)->wobbleOffset > ((SkySlotAnim*)p)->wobbleAmp)
                    {
                        ((SkySlotAnim*)p)->b314 = (s8)(1 - ((SkySlotAnim*)p)->b314);
                    }
                }
                else
                {
                    offset = ((SkySlotAnim*)p)->wobbleOffset;
                    sa = sa + offset;
                    ((SkySlotAnim*)p)->wobbleOffset = offset - ((SkySlotAnim*)p)->wobbleStep;
                    p = *pp;
                    value = ((SkySlotAnim*)p)->wobbleOffset;
                    if (value < 0.0f) {
                        ((SkySlotAnim*)p)->b314 = (s8)(1 - ((SkySlotAnim*)p)->b314);
                        ((SkySlotAnim*)*pp)->wobbleOffset = 0.0f;
                        range = (s16)(int)(sb - sa);
                        ((SkySlotAnim*)*pp)->wobbleAmp = randomGetRange(-range / 2, range / 2);
                        ((SkySlotAnim*)*pp)->wobbleStep = 0.05f * randomGetRange(1, 10);
                    }
                }
            }
            if (sb > 2000.0f)
            {
                sb = 2000.0f;
            }
            if (sa > sb)
            {
                sa = sb - 1.0f;
            }
            if (sa <= 0.0f)
            {
                setStarsHidden(1);
            }
            else
            {
                setStarsHidden(0);
            }
            p = *pp;
            flags = ((SkySlotAnim*)p)->flags4;
            if ((flags & 8) == 0)
            {
                ambientScale = (f32)(red + green + blue) / 765.0f;
                r *= ambientScale;
                g *= ambientScale;
                b *= ambientScale;
            }
            if ((flags & 1) != 0)
            {
                ((SkySlotAnim*)p)->colorR = r;
                *(int*)(*pp + 0x28) = g;
                *(int*)(*pp + 0x2c) = b;
                *(f32*)(*pp + 0x14) = sa;
                *(f32*)(*pp + 0x18) = sb;
                if ((((SkySlotAnim*)*pp)->flags4 & 0x80) == 0)
                {
                    *(int*)(*pp + 0x30) = 0xff;
                    *(int*)(*pp + 0x34) = 0xff;
                    *(int*)(*pp + 0x38) = 0xff;
                    *(f32*)(*pp + 0x1c) = 1950.0f;
                    *(f32*)(*pp + 0x20) = 2005.0f;
                }
            }
            else if ((flags & 4) != 0)
            {
                ((SkySlotAnim*)p)->colorR2 = r;
                *(int*)(*pp + 0x34) = g;
                *(int*)(*pp + 0x38) = b;
                *(f32*)(*pp + 0x1c) = sa;
                *(f32*)(*pp + 0x20) = sb;
                if ((((SkySlotAnim*)*pp)->flags4 & 0x80) == 0)
                {
                    *(int*)(*pp + 0x24) = 0xff;
                    *(int*)(*pp + 0x28) = 0xff;
                    *(int*)(*pp + 0x2c) = 0xff;
                    *(f32*)(*pp + 0x14) = 1950.0f;
                    *(f32*)(*pp + 0x18) = 2005.0f;
                }
            }
            else
            {
                redInt = r;
                ((SkySlotAnim*)p)->colorR = redInt;
                greenInt = g;
                *(int*)(*pp + 0x28) = greenInt;
                blueInt = b;
                *(int*)(*pp + 0x2c) = blueInt;
                *(f32*)(*pp + 0x14) = sa;
                *(f32*)(*pp + 0x18) = sb;
                *(int*)(*pp + 0x30) = redInt;
                *(int*)(*pp + 0x34) = greenInt;
                *(int*)(*pp + 0x38) = blueInt;
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

    gSky2EnvfxActIndex = -1;
    (&gSky2EnvfxActIndex)[1] = -1;
    i = 0;
    slot = (void**)&gSky2State;
    a = 1150.0f;
    b = 1205.0f;
    for (; i < 2; i++)
    {
        if (*slot == NULL)
        {
            *slot = mmAlloc(792, 23, 0);
        }
        memset(*slot, 0, 792);
        ((SkySlotAnim*)*slot)->colorR = 255;
        ((SkySlotAnim*)*slot)->colorG = 255;
        ((SkySlotAnim*)*slot)->colorB = 255;
        ((SkySlotAnim*)*slot)->fogNear = a;
        ((SkySlotAnim*)*slot)->fogFar = b;
        ((SkySlotAnim*)*slot)->colorR2 = 255;
        ((SkySlotAnim*)*slot)->colorG2 = 255;
        ((SkySlotAnim*)*slot)->colorB2 = 255;
        ((SkySlotAnim*)*slot)->fogNear2 = a;
        ((SkySlotAnim*)*slot)->fogFar2 = b;
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
        (&gSky2EnvfxActIndex)[1] = gSky2EnvfxActIndex = (s16)((Sky2Config*)cfg)->envfxActId - 1;
        *(s16*)(env + 0xc) = (s16)((Sky2Config*)cfg)->envfxActId - 1;
        flags58 = ((Sky2Config*)cfg)->flags;
        b1 = (flags58 & 0x80) ? 1 : 0;
        if (((SkySlotAnim*)(&gSky2State)[b1])->b317 == 0)
        {
            if ((flags58 & 0x40) != 0)
            {
                flags |= 0x40;
            }
            sky2ResetStateFromConfig(cfg, flags);
            if ((((Sky2Config*)cfg)->flags & 0x40) != 0)
            {
                ((SkySlotAnim*)(&gSky2State)[b1])->b316 = 1;
            }
            ((SkySlotAnim*)(&gSky2State)[b1])->flags4 = ((Sky2Config*)cfg)->flags | 0x100;
            ((SkySlotAnim*)(&gSky2State)[b1])->b315 = 1;
            ((SkySlotAnim*)(&gSky2State)[b1])->t = 0.0f;
        }
        else if ((flags58 & 0x20) != 0)
        {
            getEnvfxAct(0, 0, 9, 0);
        }
        else
        {
            ((SkySlotAnim*)(&gSky2State)[b1])->flags4 = ((Sky2Config*)cfg)->flags | 0x100;
            ((SkySlotAnim*)(&gSky2State)[b1])->b315 = 1;
            ((SkySlotAnim*)(&gSky2State)[b1])->t = 0.0f;
            for (i = 0; i < SKY_CONFIG_FIELD_COUNT; i++)
            {
                *(f32*)((&gSky2State)[b1] + i * 4 + 0xf4) = (f32)(u32)((Sky2Config*)cfg)->redKeys[gSkyConfigFieldIndices[i]];
                *(f32*)((&gSky2State)[b1] + i * 4 + 0x120) = (f32)(u32)((Sky2Config*)cfg)->greenKeys[gSkyConfigFieldIndices[i]];
                *(f32*)((&gSky2State)[b1] + i * 4 + 0x14c) = (f32)(u32)((Sky2Config*)cfg)->blueKeys[gSkyConfigFieldIndices[i]];
                *(f32*)((&gSky2State)[b1] + i * 4 + 0x254) =
                    (f32)(u32)((Sky2Config*)cfg)->fogNearKeys[gSkyConfigFieldIndices[i]];
                *(f32*)((&gSky2State)[b1] + i * 4 + 0x280) =
                    (f32)(u32)((Sky2Config*)cfg)->fogFarKeys[gSkyConfigFieldIndices[i]];
            }
            ((SkySlotAnim*)(&gSky2State)[b1])->fadeDurationA = ((Sky2Config*)cfg)->fadeDurationA;
            ((SkySlotAnim*)(&gSky2State)[b1])->fadeDurationB = ((Sky2Config*)cfg)->fadeDurationB;
            ((SkySlotAnim*)(&gSky2State)[b1])->b314 = -1;
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
                ((SkySlotAnim*)(&gSky2State)[b1])->flags6 |= 0x40;
                ((SkySlotAnim*)(&gSky2State)[b1])->b316 = 1;
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

    gSky2EnvfxActIndex = -1;
    (&gSky2EnvfxActIndex)[1] = -1;
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

f32 lbl_8039A7B8[0x18];
