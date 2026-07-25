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

void skyFn_8008aee8(void)
{
    int* sky;
    int texA;
    int texB;
    u8* texC;
    CameraViewSlot* cam;
    GameObject* player;
    int cell;
    u8* tbl;
    u8* channel;
    int idxA;
    int idxB;
    int phase;
    int gradA;
    int gradB;
    int texW;
    u32 screenRes;
    int texHandle;
    f32 u;
    f32 frac;
    f32 t;
    f32 tc;
    f32 sinProd;
    f32 widthF;
    f32 angle;
    f32 blend;
    f32 v;
    f32 ang0;
    GXColor fogColor;

    fogColor = *(GXColor*)&lbl_803E8458;
    if (gSkyState != NULL)
    {
        if ((player = Obj_GetPlayerObject()) != NULL &&
            (((cell = coordsToMapCell(player->anim.localPosX, player->anim.localPosZ)) ==
              0x30) ||
             cell == 0x2b))
        {
            return;
        }
        sky = *(int**)&gSkyState;
        frac = ((SkyTimeBlend*)sky)->time / gSkySecondsPerDay;
        t = (frac < 0.0f) ? 0.0f : ((frac > 1.0f) ? 1.0f : frac);
        u = 0.0f;
        if (t >= u && t < 0.125f)
        {
            u = t / 0.125f;
            ((SkyTimeBlend*)sky)->phase = 0;
        }
        else if (t >= 0.125f && t < 0.25f)
        {
            u = (t - 0.125f) / 0.125f;
            ((SkyTimeBlend*)sky)->phase = 1;
        }
        else if (t >= 0.25f && t < 0.375f)
        {
            u = (t - 0.25f) / 0.125f;
            ((SkyTimeBlend*)sky)->phase = 2;
        }
        else if (t >= 0.375f && t < 0.5f)
        {
            u = (t - 0.375f) / 0.125f;
            ((SkyTimeBlend*)sky)->phase = 3;
        }
        else if (t >= 0.5f && t < 0.625f)
        {
            u = (t - 0.5f) / 0.125f;
            ((SkyTimeBlend*)sky)->phase = 4;
        }
        else if (t >= 0.625f && t < 0.75f)
        {
            u = (t - 0.625f) / 0.125f;
            ((SkyTimeBlend*)sky)->phase = 5;
        }
        else if (t >= 0.75f && t < 0.875f)
        {
            u = (t - 0.75f) / 0.125f;
            ((SkyTimeBlend*)sky)->phase = 6;
        }
        else if (t >= 0.875f && t <= 1.0f)
        {
            u = (t - 0.875f) / 0.125f;
            ((SkyTimeBlend*)sky)->phase = 7;
        }
        tc = (u < 0.0f) ? 0.0f : ((u > 1.0f) ? 1.0f : u);
        sky = *(int**)&gSkyState;
        phase = ((SkyTimeBlend*)sky)->phase;
        if (phase != ((SkyTimeBlend*)sky)->prevPhase)
        {
            texA = sky[phase + 0x87];
            texB = sky[(phase + 1) % 8 + 0x87];
            if (((SkyTimeBlend*)sky)->texAId != texA)
            {
                textureFree((Texture*)((void*)sky[0]));
                *(void**)gSkyState = textureLoadAsset(texA);
                ((SkyTimeBlend*)gSkyState)->texAId = texA;
            }
            sky = *(int**)&gSkyState;
            if (((SkyTimeBlend*)sky)->texBId != texB)
            {
                textureFree((Texture*)((void*)sky[1]));
                ((SkyTimeBlend*)gSkyState)->texB = textureLoadAsset(texB);
                ((SkyTimeBlend*)gSkyState)->texBId = texB;
            }
            ((SkyTimeBlend*)gSkyState)->prevPhase = (s8)((SkyTimeBlend*)gSkyState)->phase;
        }
        blendTextures(((SkyTimeBlend*)gSkyState)->texB, ((SkyTimeBlend*)gSkyState)->texA, tc,
                      (void*)(*(int**)&gSkyState)[((SkyTimeBlend*)gSkyState)->texSel + 2]);
        ((SkyBlendStateFlags*)(gSkyState + 0x255))->unused80 = 1;
        sky = *(int**)&gSkyState;
        blend = ((SkyTimeBlend*)sky)->blend;
        if (blend)
        {
            texHandle = sky[((SkyTimeBlend*)sky)->texSel + 2];
            blendTextures((void*)sky[4], (void*)texHandle, blend, (void*)texHandle);
        }
        sky = *(int**)&gSkyState;
        idxA = (s16)(sky[((SkyTimeBlend*)sky)->phase + 0x87] - 0xc38) * 6;
        tbl = gSkyColorBlendTable;
        gradA = tbl[idxA];
        idxB = (s16)(sky[(((SkyTimeBlend*)sky)->phase + 1) % 8 + 0x87] - 0xc38) * 6;
        gradB = tbl[idxB];
        gSkyCurrentLightColor.r = (u8)(int)(tc * (f32)(gradB - gradA) + (f32)(u32)gradA);
        channel = tbl + 1;
        gradA = channel[idxA];
        gradB = channel[idxB];
        gSkyCurrentLightColor.g = (u8)(int)(tc * (f32)(gradB - gradA) + (f32)(u32)gradA);
        channel = tbl + 2;
        gradA = channel[idxA];
        gradB = channel[idxB];
        gSkyCurrentLightColor.b = (u8)(int)(tc * (f32)(gradB - gradA) + (f32)(u32)gradA);
        channel = tbl + 3;
        gradA = channel[idxA];
        gradB = channel[idxB];
        gSkyCurrentAmbientColor.r = (u8)(int)(tc * (f32)(gradB - gradA) + (f32)(u32)gradA);
        channel = tbl + 4;
        gradA = channel[idxA];
        gradB = channel[idxB];
        gSkyCurrentAmbientColor.g = (u8)(int)(tc * (f32)(gradB - gradA) + (f32)(u32)gradA);
        channel = tbl + 5;
        gradA = channel[idxA];
        gradB = channel[idxB];
        gSkyCurrentAmbientColor.b = (u8)(int)(tc * (f32)(gradB - gradA) + (f32)(u32)gradA);
        texC = (u8*)sky[((SkyTimeBlend*)sky)->texSel + 2];
        cam = Camera_GetCurrentViewSlot();
        frac = Camera_GetFovY();
        frac = frac / 2.0f;
        widthF = (f32)(u32) * (u16*)(texC + 0xc);
        sinProd = widthF * frac / 180.0f;
        sinProd *= 3.0f;
        sinProd *= mathCosf(gSkyPi * (f32)-cam->worldRoll / 32768.0f);
        ang0 = widthF / 2.0f - 6.0f - 3.0f * (widthF * cam->worldPitch) / 32768.0f;
        angle = ang0 + sinProd;
        angle *= 32.0f;
        (*gSky2Interface)->applyTextColor(0);
        GXSetFog(GX_FOG_NONE, 0.0f, 0.0f, 0.0f, 0.0f, fogColor);
        selectTexture((Texture*)texC, 0);
        fn_8007880C();
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevDirect(GX_TEVSTAGE0);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_C1, GX_CC_A1, GX_CC_ZERO);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
        GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
        GXSetNumIndStages(0);
        GXSetNumChans(0);
        GXSetNumTexGens(1);
        GXSetNumTevStages(1);
        screenRes = getScreenResolution();
        sinProd *= 2.0f;
        texW = ((Texture*)texC)->height;
        v = angle / (32.0f * (f32)(u32)texW);
        skyDrawFn_80075d5c(0, 0, (screenRes & 0xffff) << 2, (screenRes >> 16) << 2, 0.0f, v,
                           1.0f, v - sinProd / (f32)(u32)texW, -0x18f);
    }
}

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

extern u8 gSkyConfigFieldIndices[];

extern ObjectDescriptor17 lbl_8030F4AC;

extern u8 lbl_8030F500[160];

extern f32 lbl_8039A7B8[0x18];
extern f32 gSkyOverrideLightDirection[4];
