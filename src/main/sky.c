#include "main/sky_state.h"
#include "main/dll/savegame_env_api.h"
#include "main/dll/savegame_load_api.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/render_envfx_api.h"
#include "main/gamebit_ids.h"
#include "main/gamebits.h"
#include "main/sky_interface.h"
#include "main/dll/cloudaction_interface.h"
#include "main/game_object.h"
#include "main/gameloop_api.h"
#include "main/object_api.h"
#include "main/objprint_render_api.h"
#include "main/object.h"
#include "main/pad.h"
#include "main/curve_eval.h"
#include "main/frame_timing.h"
#include "main/fileio.h"
#include "main/camera.h"
#include "main/mm.h"
#include "main/model.h"
#include "main/model_light.h"
#include "main/pi_data_file_api.h"
#include "main/pi_frame_api.h"
#include "main/pi_flush_api.h"
#include "main/texture.h"
#include "main/textrender_api.h"
#include "main/rcp_dolphin_api.h"
#include "main/sky.h"
#include "main/sky_api.h"
#include "main/lightmap_api.h"
#include "main/lightmap_render_control_api.h"
#include "main/object_descriptor.h"
#include "main/loaded_file_flags.h"
#include "track/intersect_screen_api.h"
#include "track/intersect_api.h"
#include "dolphin/gx/GXBump.h"
#include "dolphin/gx/GXGeometry.h"
#include "dolphin/gx/GXLighting.h"
#include "dolphin/gx/GXPixel.h"
#include "dolphin/gx/GXTev.h"
#include "dolphin/mtx/mtx_legacy.h"
#include "main/lightmap.h"
#include "main/track_dolphin_sky_api.h"
#include "main/track_dolphin_shadow_api.h"
#include "string.h"

typedef struct SkyColor
{
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} SkyColor;

u32 lbl_803DD18C;
u32 lbl_803DD188;
u8* gSky2State;
s8 gSky2DrawMode;
static u32 sSkyUnusedD;
SkyColor gSkyCurrentTextureColor;
SkyColor gSkyCurrentAmbientColor;
SkyColor gSkyCurrentLightColor;
u8 gSkySunPositionPrev;
ModelLightStruct* gSkyMoonLight;
u8 gSkyOverrideLightDirectionEnabled;
f32 gSkyOverrideLightIntensity;
u8 gSkyOverrideLightColorEnabled;
SkyColor gSkyOverrideLightColor;
int gSkyObjectsInitialized;
Texture* gSkySkyTexture;
GameObject* gSkyMoonObject;
GameObject* gSkySunObject;
ModelLightStruct* gSkySunLight;
u8 gSkyEnvFxFlags;
void* lbl_803DD13C;
void* lbl_803DD138;
void* lbl_803DD134;
void* lbl_803DD130;
u8* gSkyState;
u16 gSkyMoonAlpha;
u16 gSkySunAlpha;

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
extern u8 gSkyEnvFxFlags;
extern u8* gSkyState;
extern GameObject* gSkySunObject;
extern GameObject* gSkyMoonObject;
extern Texture* gSkySkyTexture;
extern int gSkyObjectsInitialized;
extern f32 gSkyOverrideLightIntensity;
extern u8 gSkyOverrideLightDirectionEnabled;
extern f32 gSkyOverrideLightDirection[];
extern const f32 lbl_803DF058;
extern f32 lbl_803DF060;
extern f32 gSkyDayStartTime;
extern f32 lbl_803DF064;
extern f32 lbl_803DF068;
extern f32 gSkySecondsPerDay;
extern const f32 lbl_803DF0F0;
extern const f32 gSkyInitialTimeOfDay;
extern const f32 lbl_803DF0F8;
extern const f32 lbl_803DF0FC;
extern const f32 lbl_803DF100;
extern const f32 lbl_803DF104;
extern f32 lbl_803DF088;
extern u16 gSkySunAlpha;
extern u16 gSkyMoonAlpha;
STATIC_ASSERT(sizeof(SkyVec3) == 0xC);
const SkyVec3 gSkyBaseSunDirection = {0.0f, 0.0f, 4600.0f};
const SkyVec3 gSkyBaseMoonDirection = {0.0f, 0.0f, 4600.0f};
extern const f32 gSkySunMoonFarPlane;
extern const f32 gSkySunArcDuration;
extern const f32 gSkySunFadeInThreshold;
extern const f32 gSkyAlphaFadeScale;
extern const f32 gSkySunFadeOutThreshold;
extern const f32 lbl_803DF0AC;
extern const f32 gSkySunRiseDuration;
extern const f32 lbl_803DF0B4;
extern const f32 lbl_803DF0B8;
extern const f32 gSkySunMoonScale;
extern const f32 lbl_803DF0C0;
extern const f32 lbl_803DF0C4;
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
extern u8 colorScale;
extern int lbl_803DB610;
extern s8 gSky2DrawMode;
extern u8* gSky2State;
extern u8 gSkySunPositionPrev;
extern f32 gSkySunDirection[];
extern f32 gSkyMoonDirection[];
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
extern u8 gSkyColorBlendTable[];
extern int lbl_803E8458;
extern const f32 gSkyPi;
extern f32 lbl_803DF190;
extern f32 lbl_803DF194;

int return0_80088758(void)
{
    return 0x0;
}

int getEnvFxBit2BA(void)
{
    return (u8)mainGetBit(GAMEBIT_ENV_dayNo);
}

void setGameBit2BA(int value)
{
    if ((u8)value >= 0x1c)
    {
        value = 0;
    }
    mainSetBits(GAMEBIT_ENV_dayNo, (u8)value);
}

void doNothing_800887C4(void)
{
}

void doNothing_800887C8(void)
{
}

void envFxFn_800887cc(void)
{
    playerEnvFxFn_80088ad4((u8)mainGetBit(GAMEBIT_ENV_dayNo));
}

void envFxActFn_800887f8(u8 value)
{
    void* player;
    int masked;

    gSkyEnvFxFlags = value;
    masked = value;
    masked &= SKY_ENVFX_GROUP_D;
    if (masked == 0)
    {
        player = Obj_GetPlayerObject();
        getEnvfxAct(player, player, SKY_ENVFX_ID_A, 0);
        getEnvfxAct(player, player, SKY_ENVFX_ID_B, 0);
        getEnvfxAct(player, player, SKY_ENVFX_ID_C, 0);
    }
}

void skySetEnvFxRampTables(void* groupB, void* groupA, void* groupC, void* groupD)
{
    lbl_803DD13C = groupB;
    lbl_803DD130 = groupA;
    lbl_803DD138 = groupC;
    lbl_803DD134 = groupD;
}

void envFxFn_80088884(void)
{
    u8 a;
    u8 b;
    u8 flags;

    a = (u8)(*gSkyInterface)->getSunPosition(0);
    b = mainGetBit(GAMEBIT_ENV_dayNo);
    if (a != gSkySunPositionPrev)
    {
        gSkySunPositionPrev = a;
        if (a == 0)
        {
            b++;
            if (b == 0x1c)
            {
                b = 0;
            }
            mainSetBits(GAMEBIT_ENV_dayNo, b);
        }
        if (gSkyEnvFxFlags != 0)
        {
            gSkyEnvFxFlags |= SKY_ENVFX_UPDATE_PENDING;
        }
    }
    flags = gSkyEnvFxFlags;
    if ((flags & SKY_ENVFX_UPDATE_PENDING) == 0)
    {
        return;
    }
    flags = (u8)(flags & ~SKY_ENVFX_UPDATE_PENDING);
    gSkyEnvFxFlags = flags;
    if ((u32)lbl_803DD130 != 0 && (flags & SKY_ENVFX_GROUP_A) != 0 && mainGetBit(GAMEBIT_ENV_disableDayFX2) == 0)
    {
        if ((gSkyEnvFxFlags & SKY_ENVFX_IMMEDIATE) != 0)
        {
            getEnvfxActImmediately(0, 0, (u16)((s16*)lbl_803DD130)[b], 0);
        }
        else
        {
            getEnvfxAct(0, 0, (u16)((s16*)lbl_803DD130)[b], 0);
        }
    }
    if ((u32)lbl_803DD13C != 0 && (gSkyEnvFxFlags & SKY_ENVFX_GROUP_B) != 0)
    {
        if ((gSkyEnvFxFlags & SKY_ENVFX_IMMEDIATE) != 0)
        {
            getEnvfxActImmediately(0, 0, (u16)((s16*)lbl_803DD13C)[b], 0);
        }
        else
        {
            getEnvfxAct(0, 0, (u16)((s16*)lbl_803DD13C)[b], 0);
        }
    }
    if ((u32)lbl_803DD138 != 0 && (gSkyEnvFxFlags & SKY_ENVFX_GROUP_C) != 0 &&
        mainGetBit(GAMEBIT_ENV_disableDayFX1) == 0)
    {
        if ((gSkyEnvFxFlags & SKY_ENVFX_IMMEDIATE) != 0)
        {
            getEnvfxActImmediately(0, 0, (u16)((s16*)lbl_803DD138)[b], 0);
        }
        else
        {
            getEnvfxAct(0, 0, (u16)((s16*)lbl_803DD138)[b], 0);
        }
    }
    playerEnvFxFn_80088ad4(b);
    gSkyEnvFxFlags &= ~SKY_ENVFX_IMMEDIATE;
}

void playerEnvFxFn_80088ad4(u8 idx)
{
    void* player;
    int val;
    s8 alt;

    player = Obj_GetPlayerObject();
    if ((void*)lbl_803DD134 == NULL || player == NULL)
    {
        return;
    }
    if ((gSkyEnvFxFlags & SKY_ENVFX_GROUP_D) == 0)
    {
        return;
    }
    if (mainGetBit(GAMEBIT_ENV_isOutdoor) != 0)
    {
        return;
    }
    alt = (s8)(idx - 1);
    if (alt < 0)
    {
        alt = 27;
    }
    if (((s16*)lbl_803DD134)[idx] <= 0 || ((s16*)lbl_803DD134)[alt] != ((s16*)lbl_803DD134)[idx])
    {
        getEnvfxAct(player, player, 310, 0);
        getEnvfxAct(player, player, 311, 0);
        getEnvfxAct(player, player, 323, 0);
    }
    val = ((s16*)lbl_803DD134)[idx];
    if (val > 0)
    {
        if (gSkyEnvFxFlags & SKY_ENVFX_IMMEDIATE)
        {
            getEnvfxActImmediately(player, player, (u16)val, 0);
        }
        else
        {
            getEnvfxAct(player, player, (u16)val, 0);
        }
    }
}

void loadSunAndMoon(void)
{
    GameObject* moonObj;

    if (gSkyObjectsInitialized == 0)
    {
        gSkySunObject = Obj_SetupObject(Obj_AllocObjectSetup(0x20, SKY_CHILD_OBJ_SUN), 4, -1, -1, NULL);
        moonObj = Obj_SetupObject(Obj_AllocObjectSetup(0x20, SKY_CHILD_OBJ_MOON), 4, -1, -1, NULL);
        gSkyMoonObject = moonObj;
        gSkyObjectsInitialized = 1;
        ObjModel_SetRenderCallback((u8*)Obj_GetActiveModel(moonObj), moonFxCb_80074110);
    }
}

f32 gSkySunDirection[] = {0.0f, 1.0f, 0.0f};

f32 gSkyMoonDirection[] = {
    0.0f,   1.0f,   0.0f,   80.0f, 120.0f, 165.0f, 120.0f, 80.0f,  80.0f,
    100.0f, 125.0f, 100.0f, 80.0f, 255.0f, 220.0f, 190.0f, 220.0f, 255.0f,
};

u8 gSkyColorBlendTable[248] = {
    0,   29,  164, 0,   0,   72,  155, 68,  29,  12,  53,  28,  255, 143, 191, 255, 116, 186, 255, 219, 255, 255, 176,
    255, 255, 255, 255, 232, 211, 255, 130, 255, 255, 79,  163, 255, 180, 255, 255, 111, 167, 255, 255, 255, 165, 245,
    183, 140, 255, 205, 0,   255, 152, 0,   255, 129, 36,  242, 96,  33,  153, 53,  107, 104, 38,  102, 11,  0,   69,
    7,   0,   65,  255, 255, 255, 255, 255, 255, 202, 0,   254, 77,  0,   97,  255, 116, 200, 98,  0,   59,  101, 224,
    127, 0,   83,  44,  254, 254, 19,  0,   105, 38,  255, 254, 38,  205, 45,  61,  255, 253, 0,   169, 19,  57,  254,
    254, 254, 8,   121, 208, 206, 0,   0,   255, 161, 0,   255, 254, 226, 92,  131, 63,  255, 255, 147, 180, 91,  67,
    255, 254, 254, 210, 56,  130, 255, 0,   0,   122, 17,  1,   152, 0,   149, 36,  0,   87,  255, 72,  255, 101, 6,
    101, 255, 230, 131, 255, 176, 47,  254, 0,   0,   63,  0,   0,   92,  142, 255, 0,   0,   92,  153, 198, 255, 0,
    37,  172, 255, 255, 255, 53,  53,  255, 128, 128, 83,  122, 96,  70,  201, 0,   254, 77,  0,   97,  30,  65,  85,
    195, 219, 244, 30,  65,  85,  203, 219, 133, 30,  65,  85,  111, 12,  134, 30,  65,  85,  49,  138, 216, 30,  65,
    85,  255, 213, 81,  30,  65,  85,  255, 12,  0,   0,   0,   0,   0,   0,   0,   0,   0,
};

typedef struct SkyDllInterface {
    u32 reserved0;
    u32 reserved1;
    u32 reserved2;
    u32 slotCountAndFlags;
    ObjectDescriptorCallback slot00;
    ObjectDescriptorCallback slot01;
    ObjectDescriptorCallback slot02;
    ObjectDescriptorCallback slot03;
    ObjectDescriptorCallback slot04;
    ObjectDescriptorCallback slot05;
    ObjectDescriptorCallback slot06;
    ObjectDescriptorCallback slot07;
    ObjectDescriptorCallback slot08;
    ObjectDescriptorCallback slot09;
    ObjectDescriptorCallback slot0A;
    ObjectDescriptorCallback slot0B;
    ObjectDescriptorCallback slot0C;
    ObjectDescriptorCallback slot0D;
    ObjectDescriptorCallback slot0E;
    ObjectDescriptorCallback slot0F;
    ObjectDescriptorCallback slot10;
    ObjectDescriptorCallback slot11;
    ObjectDescriptorCallback slot12;
    ObjectDescriptorCallback slot13;
    ObjectDescriptorCallback slot14;
    ObjectDescriptorCallback slot15;
    ObjectDescriptorCallback slot16;
    ObjectDescriptorCallback slot17;
} SkyDllInterface;

SkyDllInterface lbl_8030F414 = {
    0,
    0,
    0,
    0x00033FB0,
    0,
    0,
    0,
    (ObjectDescriptorCallback)Sky_func03,
    (ObjectDescriptorCallback)skyLoadLights,
    (ObjectDescriptorCallback)timeOfDayFn_8008b964,
    (ObjectDescriptorCallback)renderSky,
    (ObjectDescriptorCallback)getTimeOfDay,
    (ObjectDescriptorCallback)skyGetClockTime,
    (ObjectDescriptorCallback)doNothing_8008B8B0,
    (ObjectDescriptorCallback)skyGetTimer,
    (ObjectDescriptorCallback)getSunPos,
    (ObjectDescriptorCallback)pDll_Sky_setTimeOfDay_nop,
    (ObjectDescriptorCallback)return0_8008B7E8,
    (ObjectDescriptorCallback)skyTimeToDayHourMinute,
    (ObjectDescriptorCallback)skyGetVisibility,
    (ObjectDescriptorCallback)skyFn_8008aee8,
    (ObjectDescriptorCallback)skyGetCurrentTextureColor,
    (ObjectDescriptorCallback)skyGetCurrentAmbientAndLightColors,
    (ObjectDescriptorCallback)doNothing_800887C8,
    (ObjectDescriptorCallback)doNothing_800887C4,
    (ObjectDescriptorCallback)setGameBit2BA,
    (ObjectDescriptorCallback)getEnvFxBit2BA,
    (ObjectDescriptorCallback)return0_80088758,
};

void* jumptable_8030F484[7] = {
    (void*)((u8*)Sky_func03 + 0x58),
    (void*)((u8*)Sky_func03 + 0x60),
    (void*)((u8*)Sky_func03 + 0x68),
    (void*)((u8*)Sky_func03 + 0x70),
    (void*)((u8*)Sky_func03 + 0x78),
    (void*)((u8*)Sky_func03 + 0x80),
    (void*)((u8*)Sky_func03 + 0x88),
};

void skyFn_80088c94(int flags, u8 mode)
{
    u8* env;
    u8* sky;
    int i;
    u8* entry;

    for (i = 0; i < 2; i++)
    {
        if ((flags & (1 << i)) != 0)
        {
            if (mode != 0)
            {
                ((SkyBlendStateFlags*)(gSkyState + i * 0xa4 + 0xc1))->unused80 = 1;
            }
            else
            {
                entry = gSkyState;
                entry = entry + i * 0xa4;
                ((SkyBlendStateFlags*)(entry + 0xc1))->unused80 = 0;
            }
        }
    }
    sky = gSkyState;
    ((SkyBlendStateFlags*)(sky + 0x209))->unused80 =
        ((SkyBlendStateFlags*)(sky + ((SkyState*)sky)->currentLightIndex * 0xa4 + 0xc1))->unused80;
    env = saveGameGetEnvState();
    if (getSaveGameLoadStatus() == 0)
    {
        for (i = 0; i < 2; i++)
        {
            if (((SkyBlendStateFlags*)(gSkyState + i * 0xa4 + 0xc1))->unused80 != 0)
            {
                env[0x40] |= (2 << i);
            }
            else
            {
                env[0x40] &= ~(2 << i);
            }
        }
    }
}

int getSkyColorFn_80088e08(int slot)
{
    u8* sky;

    sky = gSkyState;
    if (sky != NULL)
    {
        return ((SkyBlendStateFlags*)(sky + slot * 0xa4 + 0xc1))->unused80;
    }
    return 0;
}

int getSkyColorFn_80088e30(int slot)
{
    u8* sky;

    sky = gSkyState;
    if (sky != NULL)
    {
        return sky[slot * 0xa4 + 0xc0];
    }
    return 0xff;
}

void skyFn_80088e54(int mode, f32 brightness)
{
    u8* env;
    u8* env2;
    u32 cloudMode;
    int bit;
    f32 unset;
    f32 fullBlend;
    int idx;

    env = saveGameGetEnvState();
    if (((SkyState*)gSkyState)->currentLightIndex != mode)
    {
        ((SkyState*)gSkyState)->previousLightIndex = ((SkyState*)gSkyState)->currentLightIndex;
        ((SkyState*)gSkyState)->currentLightIndex = mode;
        unset = lbl_803DF058;
        if (brightness != unset)
        {
            ((SkyState*)gSkyState)->lightBlendRate = 1.0f / (lbl_803DF060 * brightness);
            ((SkyState*)gSkyState)->lightBlendFactor = unset;
        }
        else
        {
            fullBlend = 1.0f;
            ((SkyState*)gSkyState)->lightBlendRate = fullBlend;
            ((SkyState*)gSkyState)->lightBlendFactor = fullBlend;
        }
        cloudMode = ((SkyBlendStateFlags*)(gSkyState + (idx = mode * 0xa4) + 0xc1))->cloud;
        if (cloudMode != 0)
        {
            setDrawCloudsAndLights(cloudMode - 1);
        }
        ((SkyBlendStateFlags*)(gSkyState + 0x209))->unused80 =
            ((SkyBlendStateFlags*)(gSkyState + idx + 0xc1))->unused80;
        ((SkyBlendStateFlags*)(gSkyState + 0x209))->visibility =
            ((SkyBlendStateFlags*)(gSkyState + idx + 0xc1))->visibility;
        env2 = saveGameGetEnvState();
        if (getSaveGameLoadStatus() == 0)
        {
            for (bit = 0; bit < 2; bit++)
            {
                if ((u32)((gSkyState[bit * 0xa4 + 0xc1] >> 7) & 1) != 0)
                {
                    env2[0x40] |= 2 << bit;
                }
                else
                {
                    env2[0x40] &= ~(2 << bit);
                }
            }
        }
        if (mode != 0)
        {
            env[0x40] |= 0x10;
        }
        else
        {
            env[0x40] &= ~0x10;
        }
    }
}

int getSkyStructField24C(void)
{
    u8* sky;

    sky = gSkyState;
    if (sky != NULL)
    {
        return ((SkyState*)sky)->currentLightIndex;
    }
    return 0;
}

void skyGetCurrentTextureColor(u8* red, u8* green, u8* blue)
{
    if (gSkyState != NULL)
    {
        *red = gSkyCurrentTextureColor.r;
        *green = gSkyCurrentTextureColor.g;
        *blue = gSkyCurrentTextureColor.b;
        return;
    }
    *red = 0xff;
    *green = 0xff;
    *blue = 0xff;
}

void skyGetCurrentAmbientAndLightColors(u8* ambientRed, u8* ambientGreen, u8* ambientBlue, u8* lightRed, u8* lightGreen,
                                        u8* lightBlue)
{
    u8 red;
    u8 green;
    u8 blue;

    if (gSkyOverrideLightColorEnabled != 0)
    {
        red = gSkyOverrideLightColor.r;
        *ambientRed = red;
        *lightRed = red;
        green = gSkyOverrideLightColor.g;
        *ambientGreen = green;
        *lightGreen = green;
        blue = gSkyOverrideLightColor.b;
        *ambientBlue = blue;
        *lightBlue = blue;
        return;
    }

    if (gSkyState != NULL)
    {
        *ambientRed = gSkyCurrentAmbientColor.r;
        *ambientGreen = gSkyCurrentAmbientColor.g;
        *ambientBlue = gSkyCurrentAmbientColor.b;
        *lightRed = gSkyCurrentLightColor.r;
        *lightGreen = gSkyCurrentLightColor.g;
        *lightBlue = gSkyCurrentLightColor.b;
        return;
    }

    *ambientRed = 0xff;
    *ambientGreen = 0xff;
    *ambientBlue = 0xff;
    *lightRed = 0xff;
    *lightGreen = 0xff;
    *lightBlue = 0xff;
}

Texture* skyGetSkyTexture(void)
{
    return gSkySkyTexture;
}

void skyBuildSunModelMatrix(f32 mtx[3][4])
{
    f32 scale;
    f32 scaleMtx[3][4];

    scale = 1.0f / gSkySunObject->anim.rootMotionScale;
    PSMTXScale((f32*)scaleMtx, scale, scale, scale);
    Obj_BuildWorldTransformMatrix((GameObject*)gSkySunObject, (f32*)mtx, 0);
    PSMTXConcat((f32*)mtx, (f32*)scaleMtx, (f32*)mtx);
}

u8 skyGetSunRenderAlpha(int slot)
{
    u8* sky;

    sky = gSkyState;
    if (sky == NULL)
    {
        return 0;
    }

    if (((SkyBlendStateFlags*)(sky + slot * 0xa4 + 0xc1))->unused80 != 0)
    {
        return 0;
    }
    return gSkySunObject->anim.renderAlpha;
}

void skySetOverrideLightColor(u8 red, u8 green, u8 blue)
{
    gSkyOverrideLightColor.r = red;
    gSkyOverrideLightColor.g = green;
    gSkyOverrideLightColor.b = blue;
}

void skySetOverrideLightColorEnabled(u8 enabled)
{
    gSkyOverrideLightColorEnabled = enabled;
}

void skySetOverrideLightDirection(f32 x, f32 y, f32 z, f32 intensity)
{
    gSkyOverrideLightDirection[0] = x;
    gSkyOverrideLightDirection[1] = y;
    gSkyOverrideLightDirection[2] = z;
    gSkyOverrideLightIntensity = intensity;
    PSVECNormalize(gSkyOverrideLightDirection, gSkyOverrideLightDirection);
}

void skySetOverrideLightDirectionEnabled(u8 enabled)
{
    gSkyOverrideLightDirectionEnabled = enabled;
}

void skyGetObjectLightDirection(GameObject* obj, f32* x, f32* y, f32* z)
{
    u8* lights[4];
    f32 dir[3];
    int count;
    f32 lx;
    f32 ly;
    f32 lz;
    u8** p;
    int i;
    int slot;
    u8 flag;
    f32 mag;
    u8* sk;
    u8* found;
    u8* cur;
    int offset;

    found = NULL;
    cur = NULL;
    if (gSkyOverrideLightDirectionEnabled != 0)
    {
        *x = gSkyOverrideLightDirection[0];
        *y = gSkyOverrideLightDirection[1];
        *z = gSkyOverrideLightDirection[2];
    }
    else
    {
        slot = obj->lightColorSlot;
        if (gSkyState != NULL)
        {
            flag = ((SkyBlendStateFlags*)(gSkyState + slot * 0xa4 + 0xc1))->unused80;
        }
        else
        {
            flag = 0;
        }
        if (flag != 0)
        {
            modelLightStruct_selectObjectLights(obj, (ModelLightStruct**)lights, 4, (s32*)&count, 2);
            if (count > 0)
            {
                if (*(u8**)&obj->anim.modelState != NULL)
                {
                    found = *(u8**)(*(u8**)&obj->anim.modelState + 0x3c);
                }
                cur = lights[0];
                if (found != lights[0] && found != NULL)
                {
                    p = &lights[1];
                    for (i = count; i > 1; i--)
                    {
                        if (*p == found)
                        {
                            if (-((ModelLightStruct*)cur)->selectionScore <
                                lbl_803DF064 * -((ModelLightStruct*)found)->selectionScore)
                            {
                                cur = found;
                            }
                            break;
                        }
                        p++;
                    }
                }
                modelLightStruct_getWorldPosition((ModelLightStruct*)cur, &lx, &ly, &lz);
                dir[0] = obj->anim.worldPosX - lx;
                dir[1] = obj->anim.worldPosY - ly;
                dir[2] = obj->anim.worldPosZ - lz;
                mag = PSVECMag(dir);
                if (mag > lbl_803DF058)
                {
                    mag = 1.0f / mag;
                    PSVECScale(dir, dir, mag);
                    *x = dir[0];
                    *y = dir[1];
                    *z = dir[2];
                }
            }
            else
            {
                cur = NULL;
                dir[0] = lbl_803DF068;
                dir[1] = (-1.0f);
                dir[2] = lbl_803DF068;
                PSVECNormalize(dir, dir);
                *x = dir[0];
                *y = dir[1];
                *z = dir[2];
            }
        }
        else
        {
            if (gSkyState == NULL)
            {
                *x = lbl_803DF058;
                *y = (-1.0f);
                *z = lbl_803DF058;
            }
            else
            {
                offset = slot * 0xa4;
                sk = gSkyState + offset;
                *x = ((SkyState*)sk)->lights[0].directionX;
                sk = gSkyState + offset;
                *y = ((SkyState*)sk)->lights[0].directionY;
                sk = gSkyState + offset;
                sk = (u8*)sk;
                *z = ((SkyState*)sk)->lights[0].directionZ;
            }
        }
    }
    if (*(u8**)&obj->anim.modelState != NULL)
    {
        *(u8**)(*(u8**)&obj->anim.modelState + 0x3c) = cur;
    }
}

void skySetLightDirection(int flags, f32 x, f32 y, f32 z)
{
    int bit;

    if (gSkyState == NULL)
    {
        return;
    }
    for (bit = 0; bit < 2; bit++)
    {
        if ((flags & (1 << bit)) != 0)
        {
            ((SkyLightSlotView*)(gSkyState + bit * 0xa4))->overrideDirectionX = x;
            ((SkyLightSlotView*)(gSkyState + bit * 0xa4))->overrideDirectionY = y;
            ((SkyLightSlotView*)(gSkyState + bit * 0xa4))->overrideDirectionZ = z;
        }
    }
}

void skySetLightColor(int flags, u8 red, u8 green, u8 blue)
{
    int bit;

    if (gSkyState == NULL)
    {
        return;
    }
    for (bit = 0; bit < 2; bit++)
    {
        if ((flags & (1 << bit)) != 0)
        {
            ((SkyLightSlotView*)(gSkyState + bit * 0xa4))->lightR = red;
            ((SkyLightSlotView*)(gSkyState + bit * 0xa4))->lightG = green;
            ((SkyLightSlotView*)(gSkyState + bit * 0xa4))->lightB = blue;
        }
    }
}

void skySetAmbientColor(int flags, u8 red, u8 green, u8 blue)
{
    int bit;

    if (gSkyState == NULL)
    {
        return;
    }
    for (bit = 0; bit < 2; bit++)
    {
        if ((flags & (1 << bit)) != 0)
        {
            ((SkyLightSlotView*)(gSkyState + bit * 0xa4))->scaledAmbientR = red;
            ((SkyLightSlotView*)(gSkyState + bit * 0xa4))->scaledAmbientG = green;
            ((SkyLightSlotView*)(gSkyState + bit * 0xa4))->scaledAmbientB = blue;
        }
    }
}

void skySetBaseColor(int flags, u8 red, u8 green, u8 blue, u8 ambientScale, u8 lightScale)
{
    int base;
    int r1, g1, b1, r2, g2, b2;
    int bit;

    if (gSkyState == NULL)
    {
        return;
    }
    bit = 0;
    base = 0;
    r1 = red * ambientScale >> 8;
    g1 = green * ambientScale >> 8;
    b1 = blue * ambientScale >> 8;
    r2 = red * lightScale >> 8;
    g2 = green * lightScale >> 8;
    b2 = blue * lightScale >> 8;
    for (; bit < 2; bit++)
    {
        if ((flags & (1 << bit)) != 0)
        {
            ((SkyLightSlotView*)(gSkyState + base))->overrideAmbientR = red;
            ((SkyLightSlotView*)(gSkyState + base))->overrideAmbientG = green;
            ((SkyLightSlotView*)(gSkyState + base))->overrideAmbientB = blue;
            ((SkyLightSlotView*)(gSkyState + base))->scaledAmbientR = r1;
            ((SkyLightSlotView*)(gSkyState + base))->scaledAmbientG = g1;
            ((SkyLightSlotView*)(gSkyState + base))->scaledAmbientB = b1;
            ((SkyLightSlotView*)(gSkyState + base))->lightR = r2;
            ((SkyLightSlotView*)(gSkyState + base))->lightG = g2;
            ((SkyLightSlotView*)(gSkyState + base))->lightB = b2;
        }
        base += 0xa4;
    }
}

void skyFn_80089710(int flags, u8 enabled, int startComplete)
{
    u8* sky;
    u32 flagBit;
    u8 stateActive;

    sky = gSkyState;
    if (sky == NULL)
    {
        return;
    }

    for (flagBit = 0; flagBit < 2; flagBit++)
    {
        if ((flags & (1 << flagBit)) != 0)
        {
            sky = gSkyState;
            stateActive = ((SkyBlendStateFlags*)(sky + flagBit * 0xa4 + 0xc1))->active;
            if (stateActive != enabled)
            {
                if (startComplete != 0)
                {
                    ((SkyState*)sky)->lights[flagBit].unk9C = 1.0f;
                }
                else
                {
                    ((SkyState*)sky)->lights[flagBit].unk9C = lbl_803DF058;
                }
            }
            sky = gSkyState;
            ((SkyBlendStateFlags*)(sky + flagBit * 0xa4 + 0xc1))->active = enabled;
        }
    }
}

void skyGetSunLightDirection(int slot, f32* x, f32* y, f32* z)
{
    u8* sky;
    int offset;
    f32 fallback;

    if (gSkyState == NULL)
    {
        fallback = lbl_803DF058;
        *x = fallback;
        *y = (-1.0f);
        *z = fallback;
        return;
    }

    offset = slot * 0xa4;
    sky = gSkyState + offset;
    *x = ((SkyState*)sky)->lights[0].directionX;
    sky = gSkyState + offset;
    *y = ((SkyState*)sky)->lights[0].directionY;
    sky = gSkyState + offset;
    sky = (u8*)sky;
    *z = ((SkyState*)sky)->lights[0].directionZ;
}

void objGetColor(int slot, u8* red, u8* green, u8* blue)
{
    u8* sky;
    int offset;

    sky = gSkyState;
    if (sky == NULL)
    {
        *blue = 0xff;
        *green = 0xff;
        *red = 0xff;
    }
    else
    {
        offset = slot * 0xa4;
        *red = gSkyState[offset + 0x78];
        *green = gSkyState[offset + 0x79];
        *blue = gSkyState[offset + 0x7a];
    }

    *red = (u8)((*red * colorScale) >> 8);
    *green = (u8)((*green * colorScale) >> 8);
    *blue = (u8)((*blue * colorScale) >> 8);
}

void getAmbientColor(int slot, u8* red, u8* green, u8* blue)
{
    u8* sky;
    int offset;

    sky = gSkyState;
    if (sky == NULL)
    {
        *blue = 0xff;
        *green = 0xff;
        *red = 0xff;
        return;
    }

    offset = slot * 0xa4;
    *red = gSkyState[offset + 0x78];
    *green = gSkyState[offset + 0x79];
    *blue = gSkyState[offset + 0x7a];
}

void textureColorFn_8008991c(int slot, u8* red, u8* green, u8* blue)
{
    u8* sky;
    int offset;

    sky = gSkyState;
    if (sky == NULL)
    {
        *blue = 0xff;
        *green = 0xff;
        *red = 0xff;
        return;
    }

    offset = slot * 0xa4;
    *red = gSkyState[offset + 0x88];
    *green = gSkyState[offset + 0x89];
    *blue = gSkyState[offset + 0x8a];
}

void modelTextureFn_80089970(int slot)
{
    int offset;
    u8* sky;

    if (gSkySunLight != NULL)
    {
        offset = slot * 0xa4;
        sky = gSkyState + offset;
        modelLightStruct_setDirection(gSkySunLight, ((SkyState*)sky)->lights[0].directionX,
                                      ((SkyState*)sky)->lights[0].directionY, ((SkyState*)sky)->lights[0].directionZ);
        modelLightStruct_setDiffuseColor(gSkySunLight, gSkyState[offset + 0x78], gSkyState[offset + 0x79],
                                         gSkyState[offset + 0x7a], 0xff);
    }
    if (gSkyMoonLight != NULL)
    {
        offset = slot * 0xa4;
        sky = gSkyState + offset;
        modelLightStruct_setDirection(gSkyMoonLight, ((SkyState*)sky)->lights[0].moonDirectionX,
                                      ((SkyState*)sky)->lights[0].moonDirectionY,
                                      ((SkyState*)sky)->lights[0].moonDirectionZ);
        modelLightStruct_setDiffuseColor(gSkyMoonLight, gSkyState[offset + 0x80], gSkyState[offset + 0x81],
                                         gSkyState[offset + 0x82], 0xff);
    }
    lightSetColor(0, gSkyState[slot * 0xa4 + 0x88], gSkyState[slot * 0xa4 + 0x89], gSkyState[slot * 0xa4 + 0x8a]);
}

ModelLightStruct* skyGetMoonLight(void)
{
    return gSkyMoonLight;
}

ModelLightStruct* skyGetSunLight(void)
{
    return gSkySunLight;
}

void fn_80089A60(int slot, f32 x, f32 y, f32 z, int red, int green, int blue, int ambientIntensity,
                 int lightIntensity, u8 blendAlpha)
{
    f32 dir[3];
    int ambientR;
    int ambientG;
    int ambientB;
    int lightR;
    int lightG;
    int lightB;
    u32 previousComponent;
    int lightScale;
    int entryOffset;
    SkyLightSlotView* skyEntry;
    f32 blend;
    int ambientScale;
    SkyLight* previous;
    SkyLight* current;

    dir[0] = -x;
    dir[1] = -y;
    dir[2] = -z;
    if (slot == 2)
    {
        previous = (SkyLight*)(gSkyState + ((SkyState*)gSkyState)->previousLightIndex * 0xa4 + 0x20);
        current = (SkyLight*)(gSkyState + ((SkyState*)gSkyState)->currentLightIndex * 0xa4 + 0x20);
        dir[0] = previous->directionX +
                 ((SkyState*)gSkyState)->lightBlendFactor * (current->directionX - previous->directionX);
        dir[1] = previous->directionY +
                 ((SkyState*)gSkyState)->lightBlendFactor * (current->directionY - previous->directionY);
        dir[2] = previous->directionZ +
                 ((SkyState*)gSkyState)->lightBlendFactor * (current->directionZ - previous->directionZ);
        blend = ((SkyState*)gSkyState)->lightBlendFactor;
        previousComponent = previous->ambientR;
        red = (int)(blend * ((f32)current->ambientR - (f32)previousComponent) + (f32)previousComponent);
        previousComponent = previous->ambientG;
        green = (int)(blend * ((f32)current->ambientG - (f32)previousComponent) + (f32)previousComponent);
        previousComponent = previous->ambientB;
        blue = (int)(blend * ((f32)current->ambientB - (f32)previousComponent) + (f32)previousComponent);
        previousComponent = previous->scaledAmbientR;
        ambientR = (int)(blend * ((f32)current->scaledAmbientR - (f32)previousComponent) + (f32)previousComponent);
        previousComponent = previous->scaledAmbientG;
        ambientG = (int)(blend * ((f32)current->scaledAmbientG - (f32)previousComponent) + (f32)previousComponent);
        previousComponent = previous->scaledAmbientB;
        ambientB = (int)(blend * ((f32)current->scaledAmbientB - (f32)previousComponent) + (f32)previousComponent);
        previousComponent = previous->lightR;
        lightR = (int)(blend * ((f32)current->lightR - (f32)previousComponent) + (f32)previousComponent);
        previousComponent = previous->lightG;
        lightG = (int)(blend * ((f32)current->lightG - (f32)previousComponent) + (f32)previousComponent);
        previousComponent = previous->lightB;
        lightB = (int)(blend * ((f32)current->lightB - (f32)previousComponent) + (f32)previousComponent);
        previousComponent = previous->blendAlpha;
        blendAlpha = blend * ((f32)current->blendAlpha - (f32)previousComponent) + (f32)previousComponent;
    }
    else
    {
        if (((SkyBlendStateFlags*)(gSkyState + slot * 0xa4 + 0xc1))->unused80 != 0)
        {
            dir[0] = (-1.0f);
            dir[1] = (-1.0f);
            dir[2] = (-1.0f);
            PSVECNormalize(dir, dir);
            PSMTXMultVecSR(Camera_GetInverseViewMatrix(), dir, dir);
        }
        entryOffset = slot * 0xa4;
        if (((SkyBlendStateFlags*)(gSkyState + slot * 0xa4 + 0xc1))->active != 0)
        {
            skyEntry = (SkyLightSlotView*)(gSkyState + entryOffset);
            dir[0] = skyEntry->overrideDirectionX;
            dir[1] = skyEntry->overrideDirectionY;
            dir[2] = skyEntry->overrideDirectionZ;
            red = skyEntry->overrideAmbientR;
            green = skyEntry->overrideAmbientG;
            blue = skyEntry->overrideAmbientB;
            ambientR = skyEntry->scaledAmbientR;
            ambientG = skyEntry->scaledAmbientG;
            ambientB = skyEntry->scaledAmbientB;
            lightR = skyEntry->lightR;
            lightG = skyEntry->lightG;
            lightB = skyEntry->lightB;
            blendAlpha = 0xff;
        }
        else
        {
            ambientScale = ambientIntensity + 1;
            ambientR = red * ambientScale >> 8;
            ambientG = green * ambientScale >> 8;
            ambientB = blue * ambientScale >> 8;
            lightScale = lightIntensity + 1;
            lightR = red * lightScale >> 8;
            lightG = green * lightScale >> 8;
            lightB = blue * lightScale >> 8;
        }
    }
    ((SkyLight*)(gSkyState + 0x20))[slot].directionX = dir[0];
    ((SkyLight*)(gSkyState + 0x20))[slot].directionY = dir[1];
    ((SkyLight*)(gSkyState + 0x20))[slot].directionZ = dir[2];
    gSkyState[slot * 0xa4 + 0x78] = red;
    gSkyState[slot * 0xa4 + 0x79] = green;
    gSkyState[slot * 0xa4 + 0x7a] = blue;
    ((SkyLight*)(gSkyState + 0x20))[slot].moonDirectionX = -dir[0];
    ((SkyLight*)(gSkyState + 0x20))[slot].moonDirectionY = -dir[1];
    ((SkyLight*)(gSkyState + 0x20))[slot].moonDirectionZ = -dir[2];
    gSkyState[slot * 0xa4 + 0x80] = (u8)(ambientR * (colorScale + 1) >> 8);
    gSkyState[slot * 0xa4 + 0x81] = (u8)(ambientG * (colorScale + 1) >> 8);
    gSkyState[slot * 0xa4 + 0x82] = (u8)(ambientB * (colorScale + 1) >> 8);
    gSkyState[slot * 0xa4 + 0x88] = lightR;
    gSkyState[slot * 0xa4 + 0x89] = lightG;
    gSkyState[slot * 0xa4 + 0x8a] = lightB;
    gSkyState[slot * 0xa4 + 0xc0] = blendAlpha;
}

void skyFn_8008a04c(void)
{
    int part;
    int red;
    int green;
    f32* blendAlphaCurve;
    f32* ambientIntensityCurve;
    f32* lightIntensityCurve;
    int greenCurveOffset;
    int blueCurveOffset;
    int i;
    int off;
    f32* vec;
    int rawR;
    int blue;
    int rawG;
    int lightIntensity;
    int ambientIntensity;
    u8 blendAlpha;
    f32 tc;
    f32 blend;
    f32 time2;
    SkyColorBlendView* slot;
    f32 zero;
    f32 frac;
    f32 dayStart;

    vec = gSkySunDirection;
    if (gSkyState == NULL)
    {
        for (blue = 0; blue < 3; blue++)
        {
            fn_80089A60(blue, vec[0], vec[1], vec[2], 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);
        }
    }
    else
    {
        tc = (((SkyState*)gSkyState)->timeOfDay / gSkySecondsPerDay < 0.0f)
                 ? 0.0f
                 : ((((SkyState*)gSkyState)->timeOfDay / gSkySecondsPerDay > 1.0f)
                        ? 1.0f
                        : ((SkyState*)gSkyState)->timeOfDay / gSkySecondsPerDay);
        if (tc <= 0.25f)
        {
            frac = tc / 0.25f;
            part = 0;
        }
        else if (tc <= 0.5f)
        {
            frac = (tc - 0.25f) / 0.25f;
            part = 1;
        }
        else if (tc <= 0.75f)
        {
            frac = (tc - 0.5f) / 0.25f;
            part = 2;
        }
        else
        {
            frac = (tc - 0.75f) / 0.25f;
            part = 3;
        }
        for (i = 0; i < 2; i++)
        {
            blendAlphaCurve = &((f32*)((u8*)vec + 0x40))[part];
            ambientIntensityCurve = &((f32*)((u8*)vec + 0x18))[part];
            lightIntensityCurve = &((f32*)((u8*)vec + 0x2c))[part];
            greenCurveOffset = (part + 7) * 4;
            blueCurveOffset = (part + 0xe) * 4;
            zero = lbl_803DF058;
            dayStart = gSkyDayStartTime;
            off = i * 0xa4;
            if ((u32)((gSkyState[off + 0xc1] >> 7) & 1) != 0)
            {
                blendAlpha = 0xc8;
                ambientIntensity = 0;
                lightIntensity = 0x60;
            }
            else
            {
                blendAlpha = (int)Curve_EvalLinear(blendAlphaCurve, frac, 0);
                ambientIntensity = Curve_EvalLinear(ambientIntensityCurve, frac, 0);
                lightIntensity = Curve_EvalLinear(lightIntensityCurve, frac, 0);
            }
            rawR = Curve_EvalCatmullRom(gSkyState + off + part * 4 + 0x20, frac, 0);
            rawG = Curve_EvalCatmullRom(gSkyState + off + greenCurveOffset + 0x20, frac, 0);
            blue = Curve_EvalCatmullRom(gSkyState + off + blueCurveOffset + 0x20, frac, 0);
            slot = (SkyColorBlendView*)(gSkyState + off);
            blend = slot->factor;
            if (blend != zero)
            {
                rawR = (int)(blend * ((f32)slot->targetR - rawR) + rawR);
                rawG = (int)(blend * ((f32)slot->targetG - rawG) + rawG);
                blue = (int)(blend * ((f32)slot->targetB - blue) + blue);
            }
            if (rawR < 0)
            {
                red = 0;
            }
            else if (rawR > 0xff)
            {
                red = 0xff;
            }
            else
            {
                red = rawR;
            }
            if (rawG < 0)
            {
                green = 0;
            }
            else if (rawG > 0xff)
            {
                green = 0xff;
            }
            else
            {
                green = rawG;
            }
            if (blue < 0)
            {
                blue = 0;
            }
            else if (blue > 0xff)
            {
                blue = 0xff;
            }
            if (i == 0)
            {
                gSkyCurrentTextureColor.r = red;
                gSkyCurrentTextureColor.g = green;
                gSkyCurrentTextureColor.b = blue;
            }
            time2 = ((SkyState*)gSkyState)->timeOfDay;
            if (time2 >= dayStart && time2 <= lbl_803DF088)
            {
                fn_80089A60(i, vec[0], vec[1], vec[2], red, green, blue, ambientIntensity, lightIntensity,
                            blendAlpha);
            }
            else
            {
                fn_80089A60(i, -vec[3], vec[4], -vec[5], red, green, blue, ambientIntensity, lightIntensity,
                            blendAlpha);
            }
        }
        fn_80089A60(2, 0.0f, 0.0f, 0.0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);
    }
}

void skyFn_8008a500(void)
{
    f32 dot;
    f32 len;
    f32 time;

    if (gSkyState != NULL)
    {
        dot = gSkySunDirection[2] * gSkySunDirection[2] +
              (gSkySunDirection[0] * gSkySunDirection[0] + gSkySunDirection[1] * gSkySunDirection[1]);
        if (lbl_803DF058 != dot)
        {
            len = sqrtf(dot);
        }
        else
        {
            len = 1.0f;
        }
        *gSkySunDirection = *gSkySunDirection / len;
        gSkySunDirection[1] = gSkySunDirection[1] / len;
        gSkySunDirection[2] = gSkySunDirection[2] / len;
        dot = gSkyMoonDirection[2] * gSkyMoonDirection[2] +
              (gSkyMoonDirection[0] * gSkyMoonDirection[0] + gSkyMoonDirection[1] * gSkyMoonDirection[1]);
        if (lbl_803DF058 != dot)
        {
            len = sqrtf(dot);
        }
        else
        {
            len = 1.0f;
        }
        *gSkyMoonDirection = *gSkyMoonDirection / len;
        gSkyMoonDirection[1] = gSkyMoonDirection[1] / len;
        gSkyMoonDirection[2] = gSkyMoonDirection[2] / len;
        time = ((SkyState*)gSkyState)->timeOfDay;
        if (time >= gSkyDayStartTime && time <= lbl_803DF088)
        {
            if (gSkyOverrideLightDirectionEnabled != 0)
            {
                shadowSetLightDirection(gSkyOverrideLightDirection[0], gSkyOverrideLightDirection[1],
                               gSkyOverrideLightDirection[2], gSkyOverrideLightIntensity);
            }
            else
            {
                shadowSetLightDirection(*gSkySunDirection, gSkySunDirection[1], gSkySunDirection[2], 100);
            }
            (*gCloudActionInterface)->func08Nop(*gSkySunDirection, gSkySunDirection[1], gSkySunDirection[2], 1);
        }
        else
        {
            if (gSkyOverrideLightDirectionEnabled != 0)
            {
                shadowSetLightDirection(gSkyOverrideLightDirection[0], gSkyOverrideLightDirection[1],
                               gSkyOverrideLightDirection[2], gSkyOverrideLightIntensity);
            }
            else
            {
                shadowSetLightDirection(-(*gSkyMoonDirection), gSkyMoonDirection[1], -gSkyMoonDirection[2], 100);
            }
            (*gCloudActionInterface)->func08Nop(-(*gSkyMoonDirection), gSkyMoonDirection[1], -gSkyMoonDirection[2], 0);
        }
    }
}

void renderSunAndMoon(int a, int b, int c, int d, int visible)
{
    SkyRotQ q1;
    f32 moonTC;
    f32 vec[3];
    SkyVec3 sunDir;
    SkyVec3 moonDir;
    int v;
    CameraViewSlot* cam;
    f32 far;
    f32 yaw;
    f32 scale;
    f32 sunT;
    f32 moonT;
    SkyRotQ q2;
    f32 riseT;
    f32 time2;
    u8 vis;
    u8* model;
    SkyState* sky;

    cam = Camera_GetCurrentViewSlot();
    sunDir = gSkyBaseSunDirection;
    moonDir = gSkyBaseMoonDirection;
    v = 0;
    q1.x = lbl_803DF058;
    q1.y = lbl_803DF058;
    q1.z = lbl_803DF058;
    q1.w = 1.0f;
    q1.rz = 0;
    q1.ry = 0;
    q1.rx = 0;
    q2.x = lbl_803DF058;
    q2.y = lbl_803DF058;
    q2.z = lbl_803DF058;
    q2.w = 1.0f;
    q2.rz = 0;
    q2.ry = 0;
    q2.rx = 0;
    (*gSkyInterface)->getTransitionTimer(&v);
    if (cam != NULL && gSkyState != NULL)
    {
        far = Camera_GetFarPlane();
        Camera_SetFarPlane(gSkySunMoonFarPlane, 0);
        Camera_RebuildProjectionMatrix();
        sky = (SkyState*)gSkyState;
        sunT = (sky->timeOfDay - gSkyDayStartTime) / gSkySunArcDuration;
        if (sunT < lbl_803DF058)
        {
            sunT = lbl_803DF058;
        }
        else if (sunT > 1.0f)
        {
            sunT = 1.0f;
        }
        if (sunT < gSkySunFadeInThreshold)
        {
            if (sunT < lbl_803DF058)
            {
                gSkySunAlpha = 0;
            }
            else
            {
                *(s16*)&gSkySunAlpha = (gSkyAlphaFadeScale * sunT);
            }
        }
        else
        {
            if (sunT > gSkySunFadeOutThreshold)
            {
                if (sunT > 1.0f)
                {
                    gSkySunAlpha = 0;
                }
                else
                {
                    *(s16*)&gSkySunAlpha =
                        (gSkyAlphaFadeScale * (gSkySunFadeInThreshold - (sunT - gSkySunFadeOutThreshold)));
                }
            }
            else
            {
                gSkySunAlpha = 0xff;
            }
        }
        sunT *= lbl_803DF0AC;
        riseT = (sky->timeOfDay - gSkyDayStartTime) / gSkySunRiseDuration;
        if (riseT < lbl_803DF058)
        {
            riseT = lbl_803DF058;
        }
        else if (riseT > 1.0f)
        {
            riseT = 1.0f - (riseT - 1.0f);
        }
        scale = -(lbl_803DF0B4 * riseT - 1.0f);
        vec[0] = lbl_803DF0B8 * sunDir.x;
        vec[1] = lbl_803DF0B8 * sunDir.y;
        vec[2] = lbl_803DF0B8 * sunDir.z;
        yaw = sky->sunYaw;
        q1.rx = sunT;
        vecRotateZXY(&q1.rx, vec);
        q1.w = 1.0f;
        q1.rz = yaw;
        q1.ry = 0;
        q1.rx = 0;
        vecRotateZXY(&q1.rx, vec);
        gSkySunDirection[0] = vec[0];
        gSkySunDirection[1] = vec[1];
        gSkySunDirection[2] = vec[2];
        ((GameObject*)gSkySunObject)->anim.localPosX = cam->worldX + (f32)(s16)(int)vec[0];
        ((GameObject*)gSkySunObject)->anim.localPosY = cam->worldY + (f32)(s16)(int)vec[1];
        ((GameObject*)gSkySunObject)->anim.localPosZ = cam->worldZ + (f32)(s16)(int)vec[2];
        ((GameObject*)gSkySunObject)->anim.rootMotionScale = gSkySunMoonScale * scale;
        *(s16*)gSkySunObject = 0x10000 - cam->yaw;
        ((GameObject*)gSkySunObject)->anim.rotY = cam->pitch;
        ((GameObject*)gSkySunObject)->anim.rotZ = 0;
        gSkySunObject->anim.renderAlpha = *(s16*)&gSkySunAlpha;
        time2 = ((SkyState*)gSkyState)->timeOfDay;
        if (time2 >= lbl_803DF088)
        {
            moonT = time2 - lbl_803DF088;
        }
        else
        {
            moonT = time2 + lbl_803DF0C0;
        }
        moonTC = moonT / gSkySunRiseDuration;
        if (moonTC < lbl_803DF058)
        {
            moonTC = lbl_803DF058;
        }
        else if (moonTC > 1.0f)
        {
            moonTC = 1.0f;
        }
        if (moonTC < gSkySunFadeInThreshold)
        {
            if (moonTC < lbl_803DF058)
            {
                gSkyMoonAlpha = 0;
            }
            else
            {
                *(s16*)&gSkyMoonAlpha = (gSkyAlphaFadeScale * moonTC);
            }
        }
        else
        {
            if (moonTC > gSkySunFadeOutThreshold)
            {
                if (moonTC > 1.0f)
                {
                    gSkyMoonAlpha = 0;
                }
                else
                {
                    *(s16*)&gSkyMoonAlpha =
                        (gSkyAlphaFadeScale * (gSkySunFadeInThreshold - (moonTC - gSkySunFadeOutThreshold)));
                }
            }
            else
            {
                gSkyMoonAlpha = 0xff;
            }
        }
        moonTC *= lbl_803DF0AC;
        riseT = moonT / lbl_803DF0C4;
        if (riseT < lbl_803DF058)
        {
            riseT = lbl_803DF058;
        }
        else if (riseT > 1.0f)
        {
            riseT = 1.0f - (riseT - 1.0f);
        }
        scale = -(lbl_803DF0B4 * riseT - 1.0f);
        vec[0] = lbl_803DF0B8 * moonDir.x;
        vec[1] = lbl_803DF0B8 * moonDir.y;
        vec[2] = lbl_803DF0B8 * moonDir.z;
        q2.rx = moonTC;
        vecRotateZXY(&q2.rx, vec);
        q2.w = 1.0f;
        q2.rz = yaw;
        q2.ry = 0;
        q2.rx = 0;
        vecRotateZXY(&q2.rx, vec);
        gSkyMoonDirection[0] = vec[0];
        gSkyMoonDirection[1] = vec[1];
        gSkyMoonDirection[2] = vec[2];
        ((GameObject*)gSkyMoonObject)->anim.localPosX = cam->worldX + (f32)(s16)(int)vec[0];
        ((GameObject*)gSkyMoonObject)->anim.localPosY = cam->worldY + (f32)(s16)(int)vec[1];
        ((GameObject*)gSkyMoonObject)->anim.localPosZ = cam->worldZ + (f32)(s16)(int)vec[2];
        ((GameObject*)gSkyMoonObject)->anim.rootMotionScale = gSkySunMoonScale * scale;
        *(s16*)gSkyMoonObject = 0x10000 - cam->yaw;
        ((GameObject*)gSkyMoonObject)->anim.rotY = cam->pitch;
        vis = 0;
        ((GameObject*)gSkyMoonObject)->anim.rotZ = 0;
        ((u8*)gSkyMoonObject)[0x37] = *(s16*)&gSkyMoonAlpha;
        if (gSkySunObject->anim.renderAlpha != 0)
        {
            if (gSkyState != NULL)
            {
                vis = ((SkyBlendStateFlags*)(gSkyState + 0x209))->unused80;
            }
            if (vis == 0 && (u8)visible != 0)
            {
                model = (u8*)Obj_GetActiveModel((GameObject*)gSkySunObject);
                ((ObjModel*)model)->bufferFlags &= ~8;
                objRender(a, b, c, d, (GameObject*)gSkySunObject, 1);
            }
        }
        if (((u8*)gSkyMoonObject)[0x37] != 0)
        {
            if (gSkyState != NULL)
            {
                vis = ((SkyBlendStateFlags*)(gSkyState + 0x209))->unused80;
            }
            else
            {
                vis = 0;
            }
            if (vis == 0 && (u8)visible != 0)
            {
                model = (u8*)Obj_GetActiveModel(gSkyMoonObject);
                ((ObjModel*)model)->bufferFlags &= ~8;
                objRender(a, b, c, d, (GameObject*)gSkyMoonObject, 1);
            }
        }
        Camera_SetFarPlane(far, 0);
        Camera_RebuildProjectionMatrix();
    }
}

void skyFn_8008aee8(void);

int skyGetVisibility(int slot);

void skyTimeToDayHourMinute(f32 time, s16* days, s16* hours, s16* minutes);

int return0_8008B7E8(void);

int getSunPos(f32* outTime);

void skyGetTimer(int* outTimer);

void doNothing_8008B8B0(void);

void skyGetClockTime(f32* time);

void pDll_Sky_setTimeOfDay_nop(void);

void getTimeOfDay(f32* time);

void renderSky(int a, int b, int c, int d, int visible);

void timeOfDayFn_8008b964(void);

void skyLoadLights(void);

void skyResetState(void);

void Sky_func03(int a, int b, u8* cfg);

void dll_06_func0B(int* x, int* y);

void dll_06_func0A(int* a, int* b, int* c, f32* scale);

void fn_8008C9F4(u8* cfg, u8 flags);

void fn_8008D088(int slot);

int dll_06_func0F(void);

void dll_06_func0C_nop(void);

void dll_06_func09(s32* x, s32* y, s32* z);

void dll_06_func0E(void);

void dll_06_func0D(void);

void fn_8008DAE8(int obj);

void dll_06_func08(int obj);

int dll_06_func07_ret_0(void);

void dll_06_func06(int obj);

void sky2_run(void);

void sky2_onMapSetup(void);

void sky2_update(int a, int b, u8* cfg);

void sky2_release(void);

void sky2_initialise(void);

u8 gSkyConfigFieldIndices[] = {0, 0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0};

ObjectDescriptor17 lbl_8030F4AC = {
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
    (ObjectDescriptorCallback)dll_06_func06,
    (ObjectDescriptorCallback)dll_06_func07_ret_0,
    (ObjectDescriptorCallback)dll_06_func08,
    (ObjectDescriptorExtraSizeCallback)dll_06_func09,
    (ObjectDescriptorCallback)dll_06_func0A,
    (ObjectDescriptorCallback)dll_06_func0B,
    (ObjectDescriptorCallback)dll_06_func0C_nop,
    (ObjectDescriptorCallback)dll_06_func0D,
    (ObjectDescriptorCallback)dll_06_func0E,
    (ObjectDescriptorCallback)dll_06_func0F,
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
f32 gSkyOverrideLightDirection[4];
