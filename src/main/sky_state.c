#include "main/sky_state.h"
#include "main/dll/savegame_env_api.h"
#include "main/dll/savegame_load_api.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/render_envfx_api.h"
#include "main/gamebit_ids.h"
#include "main/gamebits.h"
#include "main/sky_interface.h"
#include "main/dll/cloudaction_interface.h"
#include "game/objects/object.h"
#include "main/gameloop_api.h"
#include "sys/objects.h"
#include "main/objprint_render_api.h"
#include "sys/objects/lifecycle.h"
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
#include "dlls/object_descriptor.h"
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

extern u32 lbl_803DD18C;
extern u32 lbl_803DD188;
extern u8* gSky2State;
extern s8 gSky2DrawMode;
extern SkyColor gSkyCurrentTextureColor;
extern SkyColor gSkyCurrentAmbientColor;
extern SkyColor gSkyCurrentLightColor;
extern u8 gSkySunPositionPrev;
extern ModelLightStruct* gSkyMoonLight;
extern u8 gSkyOverrideLightDirectionEnabled;
extern f32 gSkyOverrideLightIntensity;
extern u8 gSkyOverrideLightColorEnabled;
extern SkyColor gSkyOverrideLightColor;
extern int gSkyObjectsInitialized;
extern Texture* gSkySkyTexture;
extern GameObject* gSkyMoonObject;
extern GameObject* gSkySunObject;
extern ModelLightStruct* gSkySunLight;
extern u8 gSkyEnvFxFlags;
extern void* lbl_803DD13C;
extern void* lbl_803DD138;
extern void* lbl_803DD134;
extern void* lbl_803DD130;
extern u8* gSkyState;
extern u16 gSkyMoonAlpha;
extern u16 gSkySunAlpha;

extern s8 lbl_803DB750;
extern int lbl_803DB754;
extern u8 lbl_803DB758;

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
extern const SkyVec3 gSkyBaseSunDirection;
extern const SkyVec3 gSkyBaseMoonDirection;
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
extern const SkyVec3 lbl_802C1F98;
extern u8 gSkyColorBlendTable[];
extern int lbl_803E8458;
extern const f32 gSkyPi;
extern f32 lbl_803DF190;
extern f32 lbl_803DF194;

int return0_80088758(void);

int getEnvFxBit2BA(void);

void setGameBit2BA(int value);

void doNothing_800887C4(void);

void doNothing_800887C8(void);

void envFxFn_800887cc(void);

void envFxActFn_800887f8(u8 value);

void skySetEnvFxRampTables(void* groupB, void* groupA, void* groupC, void* groupD);

void envFxFn_80088884(void);

void playerEnvFxFn_80088ad4(u8 idx);

void loadSunAndMoon(void);

extern f32 gSkySunDirection[];

extern f32 gSkyMoonDirection[];

extern u8 gSkyColorBlendTable[248];

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

extern SkyDllInterface lbl_8030F414;

void skyFn_80088c94(int flags, u8 mode);

int getSkyColorFn_80088e08(int slot);

int getSkyColorFn_80088e30(int slot);

void skyFn_80088e54(int mode, f32 brightness);

int getSkyStructField24C(void);

void skyGetCurrentTextureColor(u8* red, u8* green, u8* blue);

void skyGetCurrentAmbientAndLightColors(u8* ambientRed, u8* ambientGreen, u8* ambientBlue, u8* lightRed, u8* lightGreen,
                                        u8* lightBlue);

Texture* skyGetSkyTexture(void);

void skyBuildSunModelMatrix(f32 mtx[3][4]);

u8 skyGetSunRenderAlpha(int slot);

void skySetOverrideLightColor(u8 red, u8 green, u8 blue);

void skySetOverrideLightColorEnabled(u8 enabled);

void skySetOverrideLightDirection(f32 x, f32 y, f32 z, f32 intensity);

void skySetOverrideLightDirectionEnabled(u8 enabled);

void skyGetObjectLightDirection(GameObject* obj, f32* x, f32* y, f32* z);

void skySetLightDirection(int flags, f32 x, f32 y, f32 z);

void skySetLightColor(int flags, u8 red, u8 green, u8 blue);

void skySetAmbientColor(int flags, u8 red, u8 green, u8 blue);

void skySetBaseColor(int flags, u8 red, u8 green, u8 blue, u8 ambientScale, u8 lightScale);

void skyFn_80089710(int flags, u8 enabled, int startComplete);

void skyGetSunLightDirection(int slot, f32* x, f32* y, f32* z);

void objGetColor(int slot, u8* red, u8* green, u8* blue);

void getAmbientColor(int slot, u8* red, u8* green, u8* blue);

void textureColorFn_8008991c(int slot, u8* red, u8* green, u8* blue);

void modelTextureFn_80089970(int slot);

ModelLightStruct* skyGetMoonLight(void);

ModelLightStruct* skyGetSunLight(void);

void fn_80089A60(int slot, f32 x, f32 y, f32 z, int red, int green, int blue, int ambientIntensity,
                 int lightIntensity, u8 blendAlpha);

void skyFn_8008a04c(void);

void skyFn_8008a500(void);

void renderSunAndMoon(int a, int b, int c, int d, int visible);

void skyFn_8008aee8(void);

int skyGetVisibility(int slot)
{
    u8* sky;

    sky = gSkyState;
    if (sky != NULL)
    {
        return ((SkyBlendStateFlags*)(sky + slot * 0xa4 + 0xc1))->visibility;
    }
    return 0;
}

void skyTimeToDayHourMinute(f32 time, s16* days, s16* hours, s16* minutes)
{
    s32 remaining;

    remaining = time;
    *days = remaining / 0x34bc0;
    remaining -= *days * 0x34bc0;
    *hours = remaining / 0xe10;
    remaining -= *hours * 0xe10;
    *minutes = remaining / 0x3c;
}

int return0_8008B7E8(void)
{
    return 0x0;
}

int getSunPos(f32* outTime)
{
    f32 time;

    if (gSkyState == NULL)
    {
        if (outTime != NULL)
        {
            *outTime = lbl_803DF058;
        }
        return 0;
    }

    time = ((SkyState*)gSkyState)->timeOfDay;
    if (time >= lbl_803DF088 || time < gSkyDayStartTime)
    {
        if (outTime != NULL)
        {
            if (time >= 75600.0f)
            {
                *outTime = gSkyDayStartTime + (time - 75600.0f);
            }
            else
            {
                *outTime = gSkyDayStartTime - time;
            }
        }
        return 1;
    }

    if (outTime != NULL)
    {
        *outTime = lbl_803DF088 - time;
    }
    return 0;
}

void skyGetTimer(int* outTimer)
{
    u8* sky;

    sky = gSkyState;
    if (sky == NULL)
    {
        *outTimer = 0;
        return;
    }
    *outTimer = ((SkyState*)sky)->timer;
}

void doNothing_8008B8B0(void)
{
}

void skyGetClockTime(f32* time)
{
    u8* sky;

    sky = gSkyState;
    if (sky == NULL)
    {
        *time = lbl_803DF058;
    }
    else
    {
        *time = ((SkyState*)sky)->clockTime;
    }
}

void pDll_Sky_setTimeOfDay_nop(void)
{
}

void getTimeOfDay(f32* time)
{
    u8* sky;

    sky = gSkyState;
    if (sky == NULL)
    {
        *time = lbl_803DF058;
        return;
    }
    *time = ((SkyState*)sky)->timeOfDay;
}

void renderSky(int a, int b, int c, int d, int visible)
{
    if (gSkySunObject != NULL && gSkyMoonObject != NULL)
    {
        renderSunAndMoon(a, b, c, d, visible);
    }
    skyFn_8008a500();
    skyFn_8008a04c();
}

void timeOfDayFn_8008b964(void)
{
    u8* env;
    f32 time;
    int timer;
    int i;
    int count;
    f32 val;
    u8* p;
    int idx;

    time = lbl_803DF058;
    env = saveGameGetEnvState();
    if (gSkyState == NULL || gSkyObjectsInitialized == 0)
    {
        return;
    }
    else
    {
        {
            ((SkyState*)gSkyState)->timeOfDay += ((SkyState*)gSkyState)->timeOfDayRate * timeDelta;
            if (((SkyState*)gSkyState)->timeOfDay >= gSkySecondsPerDay)
            {
                ((SkyState*)gSkyState)->timeOfDay = ((SkyState*)gSkyState)->timeOfDay - gSkySecondsPerDay;
            }
            else if (((SkyState*)gSkyState)->timeOfDay < lbl_803DF058)
            {
                ((SkyState*)gSkyState)->timeOfDay = ((SkyState*)gSkyState)->timeOfDay + gSkySecondsPerDay;
            }
            if (getSunPos(&time) != 0)
            {
                if (((SkyState*)gSkyState)->transitionLatch == 0)
                {
                    ((SkyState*)gSkyState)->transitionLatch = 1;
                }
            }
            else
            {
                if (((SkyState*)gSkyState)->transitionLatch != 0)
                {
                    timer = ((SkyState*)gSkyState)->timer + 1;
                    ((SkyState*)gSkyState)->timer = timer;
                    if (timer > 0x1e)
                    {
                        ((SkyState*)gSkyState)->timer = 0;
                    }
                    ((SkyState*)gSkyState)->transitionLatch = 0;
                }
            }
            if (Obj_GetPlayerObject() != NULL)
            {
                *(f32*)env = ((SkyState*)gSkyState)->timeOfDay;
            }
            i = 0;
            for (count = 2; count != 0; count--)
            {
                p = gSkyState + i;
                *(f32*)&((GameObject*)p)->extra -= *(f32*)(p + 0xb4) * timeDelta;
                val = *(f32*)(gSkyState + (idx = i + 0xb8));
                *(f32*)(gSkyState + idx) =
                    (val < 0.0f) ? 0.0f : ((val > 1.0f) ? 1.0f : val);
                *(f32*)(gSkyState + (idx = i + 0xbc)) -= lbl_803DF0F0 * timeDelta;
                val = *(f32*)(gSkyState + idx);
                *(f32*)(gSkyState + idx) =
                    (val < 0.0f) ? 0.0f : ((val > 1.0f) ? 1.0f : val);
                i += 0xa4;
            }
            ((SkyState*)gSkyState)->fadeFactor -= ((SkyState*)gSkyState)->fadeRate * timeDelta;
            val = ((SkyState*)gSkyState)->fadeFactor;
            ((SkyState*)gSkyState)->fadeFactor =
                (val < lbl_803DF058) ? lbl_803DF058 : ((val > 1.0f) ? 1.0f : val);
            ((SkyState*)gSkyState)->lightBlendFactor += ((SkyState*)gSkyState)->lightBlendRate * timeDelta;
            val = ((SkyState*)gSkyState)->lightBlendFactor;
            ((SkyState*)gSkyState)->lightBlendFactor =
                (val < lbl_803DF058) ? lbl_803DF058 : ((val > 1.0f) ? 1.0f : val);
        }
    }
}

void skyLoadLights(void)
{
    u8 done = 0;

    while (getLoadedFileFlags(0) != 0)
    {
        padUpdate();
        checkReset();
        if (done)
        {
            waitNextFrame();
        }
        loadDataFiles();
        dvdCheckError();
        if (done)
        {
            mmFreeTick(0);
            gameTextRun();
            GXFlush_(1, 0);
        }
        if (gDvdErrorPauseActive != 0)
        {
            done = 1;
        }
    }
    gSkyOverrideLightDirectionEnabled = 0;
    gSkyOverrideLightColorEnabled = 0;
    gSkyOverrideLightColor.r = 0xff;
    gSkyOverrideLightColor.g = 0xff;
    gSkyOverrideLightColor.b = 0xff;
    if (gSkySunLight == NULL)
    {
        gSkySunLight = objCreateLight(0, 1);
        if (gSkySunLight != NULL)
        {
            modelLightStruct_setLightKind(gSkySunLight, MODEL_LIGHT_KIND_DIRECTIONAL);
            modelLightStruct_setDirection(gSkySunLight, 0.0f, -1.0f, 0.0f);
            modelLightStruct_setDiffuseColor(gSkySunLight, 0xff, 0xff, 0xff, 0xff);
            modelLightStruct_setSpecularColor(gSkySunLight, 0xff, 0xff, 0xff, 0xff);
        }
        gSkyMoonLight = objCreateLight(0, 1);
        if (gSkyMoonLight != NULL)
        {
            modelLightStruct_setLightKind(gSkyMoonLight, MODEL_LIGHT_KIND_DIRECTIONAL);
            modelLightStruct_setDirection(gSkyMoonLight, 0.0f, 1.0f, 0.0f);
            modelLightStruct_setDiffuseColor(gSkyMoonLight, 0xff, 0xff, 0xff, 0xff);
            modelLightStruct_setSpecularColor(gSkyMoonLight, 0xff, 0xff, 0xff, 0xff);
        }
    }
    skyResetState();
    skyFn_80088c94(7, 0);
    skyFn_80088e54(0, lbl_803DF058);
    skyFn_8008a500();
    skyFn_8008a04c();
    gSkySunDirection[0] = lbl_803DF058;
    gSkySunDirection[1] = (-1.0f);
    gSkySunDirection[2] = lbl_803DF058;
    gSkyMoonDirection[0] = lbl_803DF058;
    gSkyMoonDirection[1] = (-1.0f);
    gSkyMoonDirection[2] = lbl_803DF058;
    gSkySkyTexture = textureLoadAsset(SKY_TEXTURE_SKY);
}

void skyResetState(void)
{
    u8* tex0;
    int iofs;
    int jofs;
    int i;
    int j;

    if (gSkyState != NULL)
    {
        if (gSkyState != NULL)
        {
            if (*(u8**)gSkyState != NULL)
            {
                textureFree((Texture*)(*(u8**)gSkyState));
            }
            if (((SkyState*)gSkyState)->handle != NULL)
            {
                textureFree((Texture*)(((SkyState*)gSkyState)->handle));
            }
            mm_free(((SkyState*)gSkyState)->texture0);
            mm_free(((SkyState*)gSkyState)->texture1);
            mm_free(gSkyState);
        }
        gSkyState = NULL;
    }
    gSkyState = mmAlloc(sizeof(SkyState), 0x17, 0);
    memset(gSkyState, 0, sizeof(SkyState));
    ((SkyState*)gSkyState)->unk250 = -1;
    ((SkyState*)gSkyState)->timer = randomGetRange(0, 0x1c);
    ((SkyState*)gSkyState)->unk252 = 0xc;
    ((SkyState*)gSkyState)->unk253 = 0;
    ((SkyState*)gSkyState)->timeOfDay = gSkyInitialTimeOfDay;
    ((SkyState*)gSkyState)->clockTime = 0xb4;
    ((SkyState*)gSkyState)->sunYaw = lbl_803DF0F8;
    ((SkyState*)gSkyState)->timeOfDayRate = (f32)((SkyState*)gSkyState)->clockTime / lbl_803DF060;
    ((SkyState*)gSkyState)->skyTextureIds[0] = 0xc38;
    ((SkyState*)gSkyState)->skyTextureIds[1] = 0xc38;
    ((SkyState*)gSkyState)->skyTextureIds[2] = 0xc38;
    ((SkyState*)gSkyState)->skyTextureIds[3] = 0xc38;
    ((SkyState*)gSkyState)->skyTextureIds[4] = 0xc38;
    ((SkyState*)gSkyState)->skyTextureIds[5] = 0xc38;
    ((SkyState*)gSkyState)->skyTextureIds[6] = 0xc38;
    ((SkyState*)gSkyState)->skyTextureIds[7] = 0xc38;
    *(u8**)gSkyState = textureLoadAsset(((SkyState*)gSkyState)->skyTextureIds[0]);
    ((SkyState*)gSkyState)->handle = textureLoadAsset(((SkyState*)gSkyState)->skyTextureIds[1]);
    ((SkyState*)gSkyState)->textureId0 = 0xc38;
    ((SkyState*)gSkyState)->textureId1 = 0xc38;
    tex0 = *(u8**)gSkyState;
    ((SkyState*)gSkyState)->texture0 = textureAlloc(((Texture*)tex0)->width, ((Texture*)tex0)->height, 6, 0, 0, 1, 0, 1, 1);
    ((SkyState*)gSkyState)->texture1 = textureAlloc(((Texture*)tex0)->width, ((Texture*)tex0)->height, 6, 0, 0, 1, 0, 1, 1);
    i = 0;
    iofs = 0;
    do
    {
        jofs = 0;
        for (j = 0; j < 3; j++)
        {
            *(f32*)(iofs + (int)gSkyState + jofs + 0x20) = lbl_803DF0FC;
            *(f32*)(iofs + (int)gSkyState + jofs + 0x24) = lbl_803DF0FC;
            *(f32*)(iofs + (int)gSkyState + jofs + 0x28) = lbl_803DF0FC;
            *(f32*)(iofs + (int)gSkyState + jofs + 0x2c) = lbl_803DF0FC;
            *(f32*)(iofs + (int)gSkyState + jofs + 0x30) = lbl_803DF0FC;
            *(f32*)(iofs + (int)gSkyState + jofs + 0x34) = lbl_803DF0FC;
            *(f32*)(iofs + (int)gSkyState + jofs + 0x38) = lbl_803DF0FC;
            jofs += 0x1c;
        }
        gSkyState[iofs + 0x74] = 0xff;
        gSkyState[iofs + 0x75] = 0xff;
        gSkyState[iofs + 0x76] = 0xff;
        gSkyState[iofs + 0x78] = 0xff;
        gSkyState[iofs + 0x79] = 0xff;
        gSkyState[iofs + 0x7a] = 0xff;
        gSkyState[iofs + 0x80] = 0xff;
        gSkyState[iofs + 0x81] = 0xff;
        gSkyState[iofs + 0x82] = 0xff;
        gSkyState[iofs + 0x88] = 0xff;
        gSkyState[iofs + 0x89] = 0xff;
        gSkyState[iofs + 0x8a] = 0xff;
        *(f32*)(gSkyState + iofs + 0x90) = lbl_803DF058;
        *(f32*)(gSkyState + iofs + 0x94) = (-1.0f);
        *(f32*)(gSkyState + iofs + 0x98) = lbl_803DF058;
        *(f32*)(gSkyState + iofs + 0x9c) = lbl_803DF058;
        *(f32*)(gSkyState + iofs + 0xa0) = (-1.0f);
        *(f32*)(gSkyState + iofs + 0xa4) = lbl_803DF058;
        ((SkyBlendStateFlags*)(gSkyState + iofs + 0xc1))->active = 0;
        *(f32*)(gSkyState + iofs + 0xa8) = lbl_803DF100;
        *(f32*)(gSkyState + iofs + 0xac) = 1.0f;
        *(f32*)(gSkyState + iofs + 0xb0) = lbl_803DF100;
        gSkyState[iofs + 0x7c] = 0xff;
        gSkyState[iofs + 0x7d] = 0xff;
        gSkyState[iofs + 0x7e] = 0xff;
        gSkyState[iofs + 0x84] = 0xff;
        gSkyState[iofs + 0x85] = 0xff;
        gSkyState[iofs + 0x86] = 0xff;
        gSkyState[iofs + 0x8c] = 0xff;
        gSkyState[iofs + 0x8d] = 0xff;
        gSkyState[iofs + 0x8e] = 0xff;
        gSkyState[iofs + 0xc0] = 0x80;
        iofs += 0xa4;
        i++;
    } while (i < 3);
}

void Sky_func03(int a, int b, u8* cfg)
{
    s16* envp;
    u8* env2;
    u8 mask;
    int iofs;
    int i;
    u8* p4;
    u32 cloudMode;
    int vis;
    int tmp;

    envp = (s16*)saveGameGetEnvState();
    if (cfg != NULL && ((int)((Sky2Config*)cfg)->flags & 2) != 0)
    {
        switch (((Sky2Config*)cfg)->cloudMode)
        {
        case 0:
        default:
            mask = 0xf;
            break;
        case 1:
            mask = 1;
            break;
        case 2:
            mask = 2;
            break;
        case 3:
            mask = 4;
            break;
        case 4:
            mask = 5;
            break;
        case 5:
            mask = 3;
            break;
        case 6:
            mask = 6;
            break;
        }
        for (i = 0, iofs = 0; i < 2; i++)
        {
            if ((mask & (1 << i)) != 0)
            {
                envp[2] = (s16)((Sky2Config*)cfg)->envfxActId - 1;
                *(f32*)(gSkyState + iofs + 0x20) = (f32)(u32)((Sky2Config*)cfg)->lightColorR;
                *(f32*)(gSkyState + iofs + 0x24) = (f32)(u32)((Sky2Config*)cfg)->lightColorR;
                *(f32*)(gSkyState + iofs + 0x28) = (f32)(u32)((Sky2Config*)cfg)->lightColorG;
                *(f32*)(gSkyState + iofs + 0x2c) = (f32)(u32)((Sky2Config*)cfg)->lightColorB;
                *(f32*)(gSkyState + iofs + 0x30) = (f32)(u32)((Sky2Config*)cfg)->lightColorA;
                *(f32*)(gSkyState + iofs + 0x34) = (f32)(u32)((Sky2Config*)cfg)->lightColorR;
                *(f32*)(gSkyState + iofs + 0x38) = (f32)(u32)((Sky2Config*)cfg)->lightColorR;
                *(f32*)(gSkyState + iofs + 0x3c) = (f32)(u32)((Sky2Config*)cfg)->color2R;
                *(f32*)(gSkyState + iofs + 0x40) = (f32)(u32)((Sky2Config*)cfg)->color2R;
                *(f32*)(gSkyState + iofs + 0x44) = (f32)(u32)((Sky2Config*)cfg)->color2G;
                *(f32*)(gSkyState + iofs + 0x48) = (f32)(u32)((Sky2Config*)cfg)->color2B;
                *(f32*)(gSkyState + iofs + 0x4c) = (f32)(u32)((Sky2Config*)cfg)->color2A;
                *(f32*)(gSkyState + iofs + 0x50) = (f32)(u32)((Sky2Config*)cfg)->color2R;
                *(f32*)(gSkyState + iofs + 0x54) = (f32)(u32)((Sky2Config*)cfg)->color2R;
                *(f32*)(gSkyState + iofs + 0x58) = (f32)(u32)((Sky2Config*)cfg)->color3R;
                *(f32*)(gSkyState + iofs + 0x5c) = (f32)(u32)((Sky2Config*)cfg)->color3R;
                *(f32*)(gSkyState + iofs + 0x60) = (f32)(u32)((Sky2Config*)cfg)->color3G;
                *(f32*)(gSkyState + iofs + 0x64) = (f32)(u32)((Sky2Config*)cfg)->color3B;
                *(f32*)(gSkyState + iofs + 0x68) = (f32)(u32)((Sky2Config*)cfg)->color3A;
                *(f32*)(gSkyState + iofs + 0x6c) = (f32)(u32)((Sky2Config*)cfg)->color3R;
                *(f32*)(gSkyState + iofs + 0x70) = (f32)(u32)((Sky2Config*)cfg)->color3R;
                *(f32*)(gSkyState + iofs + 0xb8) = 1.0f;
                if (((Sky2Config*)cfg)->fadeDurationA != 0)
                {
                    *(f32*)(gSkyState + iofs + 0xb4) =
                        1.0f / (lbl_803DF104 * (f32)(u32)((Sky2Config*)cfg)->fadeDurationA);
                }
                else
                {
                    *(f32*)(gSkyState + iofs + 0xb4) = 1.0f;
                }
                p4 = gSkyState + iofs;
                if (gSkyState == NULL)
                {
                    p4[0x76] = 0xff;
                    p4[0x75] = 0xff;
                    p4[0x74] = 0xff;
                }
                else
                {
                    p4[0x74] = p4[0x78];
                    p4[0x75] = gSkyState[iofs + 0x79];
                    p4[0x76] = gSkyState[iofs + 0x7a];
                }
                if (((Sky2Config*)cfg)->cloudBlendMode != 0)
                {
                    ((SkyBlendStateFlags*)(gSkyState + iofs + 0xc1))->cloud =
                        (((Sky2Config*)cfg)->cloudBlendMode & 1) + 1;
                }
                else
                {
                    ((SkyBlendStateFlags*)(gSkyState + iofs + 0xc1))->cloud = 0;
                }
            }
            envp++;
            iofs += 0xa4;
        }
        if (((Sky2Config*)cfg)->cloudBlendMode != 0)
        {
            skyFn_80088c94(mask, (u8)(((Sky2Config*)cfg)->cloudBlendMode > 2 ? 1 : 0));
        }
        vis = ((Sky2Config*)cfg)->visibility;
        for (i = 0; i < 2; i++)
        {
            if ((mask & (1 << i)) != 0)
            {
                ((SkyBlendStateFlags*)(gSkyState + i * 0xa4 + 0xc1))->visibility = vis;
            }
        }
        ((SkyBlendStateFlags*)(gSkyState + 0x209))->visibility =
            ((SkyBlendStateFlags*)(gSkyState + ((SkyState*)gSkyState)->currentLightIndex * 0xa4 + 0xc1))->visibility;
        if ((((Sky2Config*)cfg)->flags & 1) == 0)
        {
            ((SkyState*)gSkyState)->skyTextureIds[0] = ((Sky2Config*)cfg)->skyTexId0 + 0xc38;
            ((SkyState*)gSkyState)->skyTextureIds[1] = ((Sky2Config*)cfg)->skyTexId1 + 0xc38;
            ((SkyState*)gSkyState)->skyTextureIds[2] = ((Sky2Config*)cfg)->skyTexId2 + 0xc38;
            ((SkyState*)gSkyState)->skyTextureIds[3] = ((Sky2Config*)cfg)->skyTexId3 + 0xc38;
            ((SkyState*)gSkyState)->skyTextureIds[4] = ((Sky2Config*)cfg)->skyTexId4 + 0xc38;
            ((SkyState*)gSkyState)->skyTextureIds[5] = ((Sky2Config*)cfg)->skyTexId5 + 0xc38;
            ((SkyState*)gSkyState)->skyTextureIds[6] = ((Sky2Config*)cfg)->skyTexId6 + 0xc38;
            ((SkyState*)gSkyState)->skyTextureIds[7] = ((Sky2Config*)cfg)->skyTexId7 + 0xc38;
            tmp = *(int*)&((SkyState*)gSkyState)->texture1;
            *(int*)&((SkyState*)gSkyState)->texture1 =
                *(int*)((u8*)&((SkyState*)gSkyState)->texture0 + ((SkyState*)gSkyState)->swapTexIndex * 4);
            *(int*)((u8*)&((SkyState*)gSkyState)->texture0 + ((SkyState*)gSkyState)->swapTexIndex * 4) = tmp;
            ((SkyState*)gSkyState)->unk250 = -1;
            if ((((u32)(u8)((SkyState*)gSkyState)->flags255 >> 7) & 1) != 0)
            {
                ((SkyState*)gSkyState)->fadeFactor = 1.0f;
                if (((Sky2Config*)cfg)->fadeDurationA != 0)
                {
                    ((SkyState*)gSkyState)->fadeRate =
                        1.0f / (lbl_803DF104 * (f32)(u32)((Sky2Config*)cfg)->fadeDurationA);
                }
                else
                {
                    ((SkyState*)gSkyState)->fadeRate = 1.0f;
                }
            }
            else
            {
                ((SkyState*)gSkyState)->fadeFactor = lbl_803DF058;
            }
        }
        cloudMode = ((SkyBlendStateFlags*)(gSkyState + ((SkyState*)gSkyState)->currentLightIndex * 0xa4 + 0xc1))->cloud;
        if (cloudMode != 0)
        {
            setDrawCloudsAndLights(cloudMode - 1);
        }
        ((SkyBlendStateFlags*)(gSkyState + 0x209))->unused80 =
            ((SkyBlendStateFlags*)(gSkyState + ((SkyState*)gSkyState)->currentLightIndex * 0xa4 + 0xc1))->unused80;
        ((SkyBlendStateFlags*)(gSkyState + 0x209))->visibility =
            ((SkyBlendStateFlags*)(gSkyState + ((SkyState*)gSkyState)->currentLightIndex * 0xa4 + 0xc1))->visibility;
        env2 = saveGameGetEnvState();
        if (getSaveGameLoadStatus() == 0)
        {
            for (i = 0; i < 2; i++)
            {
                if (((SkyBlendStateFlags*)(gSkyState + i * 0xa4 + 0xc1))->unused80 != 0)
                {
                    env2[0x40] |= (2 << i);
                }
                else
                {
                    env2[0x40] &= ~(2 << i);
                }
            }
        }
    }
}

void dll_06_func0B(int* x, int* y)
{
    u8* state;
    f32 value;

    state = gSky2State;
    if (state != NULL)
    {
        value = *(f32*)(state + 0x14);
        *x = value;
        value = *(f32*)(gSky2State + 0x18);
        *y = value;
    }
}

void dll_06_func0A(int* a, int* b, int* c, f32* scale)
{
    u8* state;

    state = gSky2State;
    if (state == NULL)
    {
        return;
    }
    *a = ((Dll06InterpState*)state)->targetX;
    *b = ((Dll06InterpState*)gSky2State)->targetY;
    *c = ((Dll06InterpState*)gSky2State)->targetZ;
    *scale = ((Dll06InterpState*)gSky2State)->blend;
}

void fn_8008C9F4(u8* cfg, u8 flags)
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

void fn_8008D088(int slot)
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

int dll_06_func0F(void)
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

void dll_06_func09(s32* x, s32* y, s32* z)
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

    oldX = *x;
    oldY = *y;
    oldZ = *z;
    if (state != NULL)
    {
        targetX = state->targetX;
        targetY = state->targetY;
        targetZ = state->targetZ;
        blend = state->blend;
    }

    fy = (f32)(targetY - oldY);
    fz = (f32)(targetZ - oldZ);
    *x = (s32)((f32)(targetX - oldX) * (blend = lbl_803DF144 * blend) + oldX);
    *y = (s32)(fy * blend + oldY);
    *z = (s32)(fz * blend + oldZ);
}

void dll_06_func0E(void)
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

void dll_06_func0D(void)
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

void fn_8008DAE8(int obj)
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

void dll_06_func08(int obj)
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

void dll_06_func06(int obj)
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
                if ((*(u16*)&((GameObject*)p)->anim.rotZ & 1) == 0)
                {
                    ((SkySlotAnim*)p)->blend = -(timeDelta * *(f32*)(p + 0x58) - ((SkySlotAnim*)p)->blend);
                    if (((SkySlotAnim*)*pp)->blend < (frzero = lbl_803DF108))
                    {
                        ((SkySlotAnim*)*pp)->blend = frzero;
                    }
                }
            }
            if ((*(u16*)(*pp + 4) & 0x100) != 0)
            {
                fn_8008D088(i);
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
                    if (*(f32*)&((GameObject*)p)->anim.jointPoseData < (frzero = lbl_803DF108))
                    {
                        ((SkySlotAnim*)p)->b314 = (s8)(1 - ((SkySlotAnim*)p)->b314);
                        *(f32*)(*pp + 0x6c) = frzero;
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
            fn_8008C9F4(cfg, flags);
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

extern u8 gSkyConfigFieldIndices[];

extern ObjectDescriptor17 lbl_8030F4AC;

extern u8 lbl_8030F500[160];

extern f32 lbl_8039A7B8[0x18];
extern f32 gSkyOverrideLightDirection[4];
