/*
 * DIMExplosio (DLL 0x1CA) - the generic explosion/fireball effect object.
 *
 * The extra block (explosion_getExtraSize == 0xA60, ExplosionState) holds a
 * flame pool (50 x ExplosionDebris, 0x30 each, from offset 0) and a gravity
 * debris pool (6 x 0x24 records, from 0x964). Functions are kept in binary
 * (address) order:
 *   explosion_spawnFlame   - seed one flame slot (speed/colour/spin/sfx)
 *   explosion_computeColor - age/lifetime -> RGB ramp via per-channel expf
 *   explosion_render       - draw each live flame as a billboarded quad
 *                            through the GX FIFO
 *   explosion_update       - age the flames, integrate the gravity debris,
 *                            spawn particle fx, fade the light
 *   explosion_init         - seed flames/debris/light from placement flags
 *   explosion_initialise   - precompute the expf falloff scales
 */
#include "dlls/object_descriptor.h"
#include "main/model.h"
#include "main/dll/partfx_interface.h"
#include "main/track_dolphin_api.h"
#include "main/texture.h"
#include "main/dll/explosiondebris_struct.h"
#include "dolphin/mtx/mtx_legacy.h"
#include "main/shader_api.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/dll/fbtextbl_struct.h"
#include "main/dll/fbwgpipe_struct.h"
#include "main/dll/explosionpartfxsource_struct.h"
#include "main/dll/explosion_state.h"
#include "game/objects/object.h"
#include "main/objprint_render_api.h"
#include "main/model_light.h"
#include "sys/objects.h"
#include "sys/objects/lifecycle.h"
#include "main/object_render.h"
#include "main/camera.h"
#include "string.h"
#include "main/audio/sfx.h"
#include "dolphin/gx/GXGeometry.h"
#include "dolphin/gx/GXTransform.h"
#include "track/intersect_render_setup_api.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/frame_timing.h"
#include "main/dll/DIM/dll_01CA_dimexplosion.h"
#include "game/objects/object_setup.h"

typedef void (*ExplosionSpawnFlameSpdFirst)(int obj, f32 spd, int gen, f32 x, f32 y, f32 z);

typedef struct ExplosionPlacement
{
    ObjPlacement base;
    u8 pad18[0x19 - 0x18];
    s8 sfxKind;
    s16 scaleParam;
    s16 configFlags;
} ExplosionPlacement;

STATIC_ASSERT(sizeof(ExplosionPartfxSource) == 0x38);
STATIC_ASSERT(offsetof(ExplosionPartfxSource, rootMotionScale) == 0x08);
STATIC_ASSERT(offsetof(ExplosionPartfxSource, localPosX) == 0x0C);
STATIC_ASSERT(offsetof(ExplosionPartfxSource, worldPosX) == 0x18);
STATIC_ASSERT(offsetof(ExplosionPartfxSource, velocityX) == 0x24);

STATIC_ASSERT(sizeof(ExplosionState) == 0xA60);
STATIC_ASSERT(offsetof(ExplosionState, driftYSpeed) == 0xA3C);
STATIC_ASSERT(sizeof(GravityDebris) == 0x24);
STATIC_ASSERT(offsetof(ExplosionState, debris) == 0x964);
STATIC_ASSERT(offsetof(GravityDebris, active) == 0x20);

#define DIMEXPLOSION_OBJFLAG_HITDETECT_DISABLED 0x2000
#define DIMEXPLOSION_PARTFX                     0x5e

#define GEXPLOSION_TEXTURE_COUNT 4

void* gExplosionTextures[GEXPLOSION_TEXTURE_COUNT];
extern int lbl_803E8468;
f32 gExplosionDebrisSpeedScale;
f32 gExplosionDebrisAlphaScale;
f32 gExplosionDebrisColorScale;
f32 gExplosionFalloffScaleRed;
f32 gExplosionFalloffScaleGreen;
f32 gExplosionFalloffScaleBlue;
u8 gExplosionUpdateTick;
extern f32 gExplosionSpreadDirs[];
const FbTexTbl gExplosionTexTable = {
    {0x5e1, 0x5f7, 0x5f8, 0x5f9}
};

volatile FbWGPipe GXWGFifo : (0xCC008000);

static const int sExplosionQuadColorA[1] = { -1 };
static const f32 sExplosionBaseScale[1] = { 1.0f };
static const f32 sExplosionLifeScale[1] = { 15.0f };
static const f32 sExplosionFadeInExponent[1] = { 10.0f };
static const f32 sExplosionColorMax[1] = { 255.0f };
static const f32 sExplosionFadeOutExponent[1] = { 25.0f };
static const f32 sExplosionSpawnDelay[1] = { 8.0f };

void explosion_spawnFlame(GameObject* obj, u8 gen, f32 spd, f32 x, f32 y, f32 z)
{
    s16* placement = (obj)->anim.placementData;
    ExplosionState* state = (obj)->extra;
    ExplosionDebris* flames = (ExplosionDebris*)state->flames;
    int idx = state->flameCount++;
    flames[idx].posX = x;
    flames[idx].posY = y;
    flames[idx].posZ = z;
    flames[idx].baseScale = sExplosionBaseScale[0];
    flames[idx].scale = flames[0].baseScale;
    flames[idx].speed = spd;
    flames[idx].generation = gen;
    flames[idx].age = 0;
    flames[idx].lifetime = (int)(sExplosionLifeScale[0] * sqrtf(spd));
    {
        int life = flames[idx].lifetime;
        if (life < 0)
        {
            life = 0;
        }
        else if (life > 0x3c)
        {
            life = 0x3c;
        }
        flames[idx].lifetime = life;
    }
    if (flames[idx].generation < 1)
    {
        s8 c = ((ExplosionPlacement*)placement)->sfxKind;
        if (c != 0)
        {
            if (c == 2)
            {
                Sfx_PlayFromObject((int)obj, SFXTRIG_wp_sexpl2_c_4bf);
            }
            else if (c == 3)
            {
                Sfx_PlayFromObject((int)obj, SFXTRIG_wp_sexpl2_c_4c2);
            }
            else
            {
                s8 m = (obj)->anim.mapEventSlot;
                switch (m)
                {
                case 0x2c:
                case 0x3a:
                case 0x3b:
                case 0x3c:
                case 0x3d:
                case 0x3e:
                    Sfx_PlayFromObjectLimited((int)obj, SFXTRIG_wp_sexpl2_c_4b8, 2);
                    break;
                default:
                    Sfx_PlayFromObject((int)obj, SFXTRIG_sexpl2_c);
                    break;
                }
            }
        }
    }
    flames[idx].spinAngle = randomGetRange(0, 0xffff);
    flames[idx].spinSpeed = randomGetRange(0xc8, 0x12c);
    if ((int)randomGetRange(0, 1) != 0)
    {
        flames[idx].spinSpeed = -flames[idx].spinSpeed;
    }
    flames[idx].texVariant = randomGetRange(0, 3);
    {
        f32 sp = flames[idx].speed;
        f32 ev = expf((sExplosionFadeInExponent[0] * ((f32)flames[idx].lifetime - (f32)flames[idx].age)) / (f32)flames[idx].lifetime);
        f32 d = sp - flames[idx].baseScale;
        ev = d * ev;
        flames[idx].scale = sp - gExplosionDebrisSpeedScale * ev;
        ev = expf((sExplosionFadeOutExponent[0] * (f32)flames[idx].age) / (f32)flames[idx].lifetime);
        ev = sExplosionColorMax[0] * ev;
        flames[idx].alpha = sExplosionColorMax[0] - gExplosionDebrisAlphaScale * ev;
        flames[idx].spawnTimer = sExplosionSpawnDelay[0];
        flames[idx].spawnInterval = flames[idx].spawnTimer;
        flames[idx].active = 1;
    }
}
void explosion_computeColor(f32 age, f32 lifetime, u8 mode, u8* out)
{
    s16 r;
    s16 g;
    s16 b;
    s16 rawR;
    s16 rawG;
    s16 rawB;
    rawR = 0xff - (u8)(int)(gExplosionFalloffScaleRed * (sExplosionColorMax[0] * expf((7.5f * age) / lifetime)));
    rawG = 0xff - (u8)(int)(gExplosionFalloffScaleGreen * (sExplosionColorMax[0] * expf((2.5f * age) / lifetime)));
    rawB = 0xff - (u8)(int)(gExplosionFalloffScaleBlue * (sExplosionColorMax[0] * expf(age / lifetime)));
    r = (rawR < 1) ? 1 : ((rawR > 0xff) ? 0xff : rawR);
    g = (rawG < 1) ? 1 : ((rawG > 0xff) ? 0xff : rawG);
    b = (rawB < 1) ? 1 : ((rawB > 0xff) ? 0xff : rawB);
    switch (mode)
    {
    case 0:
        out[0] = r;
        out[1] = g;
        out[2] = b;
        break;
    case 1:
        out[0] = r;
        out[1] = b;
        out[2] = b;
        break;
    case 2:
        out[0] = b;
        out[1] = r;
        out[2] = b;
        break;
    case 3:
        out[0] = b;
        out[1] = b;
        out[2] = r;
        break;
    }
}

static const f32 sExplosionFlickerExponent[1] = { 3.0f };
static const f32 sExplosionChildOffsetStep[1] = { 0.09f };
static const f32 sExplosionZero[1] = { 0.0f };
static const f64 sExplosionPi[1] = { 3.142 };
static const f32 sExplosionAngleScale[1] = { 32768.0f };
static const f32 sExplosionSpeedScale[1] = { 0.00390625f };

int explosion_getExtraSize(void)
{
    return sizeof(ExplosionState);
}

int explosion_getObjectTypeId(GameObject* obj)
{
    ObjAnimComponent* objAnim = (ObjAnimComponent*)obj;
    int idx = (int)*(short*)(*(int*)&obj->anim.placementData + 0x1c) & 3;
    if (idx >= objAnim->modelInstance->modelCount)
    {
        idx = 0;
    }
    return (idx << 11) | 0x400;
}

void explosion_free(GameObject* obj)
{
    ModelLightStruct* light = ((ExplosionState*)obj->extra)->light;
    if (light != NULL)
    {
        ModelLightStruct_free(light);
    }
}

void explosion_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible)
{
    u32 colA;
    u32 colB;
    u32 colA2;
    u32 colB2;
    f32 mE[12];
    f32 m4[12];
    f32 m3[12];
    f32 m2[12];
    f32 m1[12];
    int state;
    int model;
    int i;
    int cursor;
    colA = sExplosionQuadColorA[0];
    colB = lbl_803E8468;
    state = *(int*)&obj->extra;
    model = (int)Obj_GetActiveModel(obj);
    cursor = state;
    if (visible != 0)
    {
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetCurrentMtx(GX_PNMTX0);
        for (i = 0, cursor = state; i < ((ExplosionState*)state)->flameCount; i++)
        {
            if (((ExplosionDebris*)cursor)->active != 0)
            {
                void** tex;
                int k;
                u8 cv;
                Obj_BuildWorldTransformMatrix(obj, mE, 0);
                PSMTXRotRad(m1, 0x7a, (f32)((6.2832 * (f64)(int)((ExplosionDebris*)cursor)->spinAngle) / 65536.0));
                PSMTXRotRad(m3, 0x78, (f32)((6.2832 * ((f64)(u32)(Camera_GetCurrentViewPitch() & 0xffff) - 0.0)) / 65536.0));
                PSMTXConcat(m3, m1, m3);
                PSMTXRotRad(m2, 0x79, (f32)((6.2832 * (f64)(int)(0x10000 - (Camera_GetCurrentViewYaw() & 0xffff))) / 65536.0));
                PSMTXConcat(m2, m3, m2);
                PSMTXScale(m4, ((ExplosionDebris*)cursor)->scale, ((ExplosionDebris*)cursor)->scale,
                           ((ExplosionDebris*)cursor)->scale);
                PSMTXConcat(m4, m2, m4);
                PSMTXTrans(mE, ((ExplosionDebris*)cursor)->posX - playerMapOffsetX, ((ExplosionDebris*)cursor)->posY,
                           ((ExplosionDebris*)cursor)->posZ - playerMapOffsetZ);
                PSMTXConcat(mE, m4, mE);
                PSMTXConcat(Camera_GetViewMatrix(), mE, mE);
                GXLoadPosMtxImm((const f32(*)[4])mE, GX_PNMTX0);
                ((u8*)&colA)[3] = ((ExplosionDebris*)cursor)->alpha;
                cv = gExplosionDebrisColorScale *
                     (sExplosionColorMax[0] *
                      expf((sExplosionFlickerExponent[0] *
                            ((f32)((ExplosionDebris*)cursor)->lifetime - (f32)((ExplosionDebris*)cursor)->age)) /
                           (f32)((ExplosionDebris*)cursor)->lifetime));
                ((u8*)&colB)[0] = cv;
                ((u8*)&colB)[1] = cv;
                ((u8*)&colB)[2] = cv;
                ((u8*)&colB)[3] = cv;
                explosion_computeColor((f32)((ExplosionDebris*)cursor)->age, (f32)((ExplosionDebris*)cursor)->lifetime,
                                       ((ExplosionState*)state)->modelKind, (u8*)&colA);
                tex = (void**)((int*)gExplosionTextures)[((ExplosionState*)state)->modelKind];
                for (k = 0; k < ((ExplosionDebris*)cursor)->texVariant; k++)
                {
                    tex = (void**)*tex;
                }
                colB2 = colB;
                colA2 = colA;
                fn_80073AAC(tex, &colA2, &colB2);
                GXBegin(GX_QUADS, GX_VTXFMT2, 4);
                {
                    f32 fc, fb, fa;
                    GXWGFifo.f32 = (fa = -1.0f);
                    GXWGFifo.f32 = fa;
                    GXWGFifo.f32 = (fb = sExplosionZero[0]);
                    GXWGFifo.f32 = fb;
                    GXWGFifo.f32 = fb;
                    GXWGFifo.f32 = (fc = sExplosionBaseScale[0]);
                    GXWGFifo.f32 = fa;
                    GXWGFifo.f32 = fb;
                    GXWGFifo.f32 = fc;
                    GXWGFifo.f32 = fb;
                    GXWGFifo.f32 = fc;
                    GXWGFifo.f32 = fc;
                    GXWGFifo.f32 = fb;
                    GXWGFifo.f32 = fc;
                    GXWGFifo.f32 = fc;
                    GXWGFifo.f32 = fa;
                    GXWGFifo.f32 = fc;
                    GXWGFifo.f32 = fb;
                    GXWGFifo.f32 = fb;
                    GXWGFifo.f32 = fc;
                }
            }
            cursor += 0x30;
        }
        if (((ExplosionState*)state)->frameCounter < ((ExplosionState*)state)->lifeFrames &&
            *(u8*)&((ExplosionState*)state)->rayMode != 0)
        {
            for (i = 0, cursor = state; i < ((ExplosionState*)state)->rayMode; cursor += 4, i++)
            {
                obj->anim.rotY = (s16) * (u16*)&((ExplosionState*)cursor)->rayYawA;
                obj->anim.rotX = (s16) * (u16*)&((ExplosionState*)cursor)->rayPitchA;
                objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, visible);
                if (i < ((ExplosionState*)state)->rayMode - 1)
                {
                    ((ObjModel*)model)->bufferFlags &= ~8;
                }
            }
        }
    }
    renderResetFn_8003fc60();
}

void explosion_hitDetect(void)
{
}

void explosion_update(GameObject* obj)
{
    ExplosionPartfxSource fake;
    u16 ang[6];
    f32 vpos[3];
    f32 m[12];
    u8 rgb[3];
    int state = *(int*)&(obj)->extra;
    int i;
    int cursor;
    gExplosionUpdateTick += 1;
    cursor = state;
    ((ExplosionState*)state)->frameCounter += framesThisStep;
    for (i = 0, cursor = state; i < ((ExplosionState*)state)->flameCount; i++)
    {
        ((ExplosionDebris*)cursor)->age += framesThisStep;
        if (((ExplosionDebris*)cursor)->active != 0)
        {
            f32 sp = ((ExplosionDebris*)cursor)->speed;
            f32 ev = expf(
                (sExplosionFadeInExponent[0] * ((f32)((ExplosionDebris*)cursor)->lifetime - (f32)((ExplosionDebris*)cursor)->age)) /
                (f32)(int)((ExplosionDebris*)cursor)->lifetime);
            f32 d = sp - ((ExplosionDebris*)cursor)->baseScale;
            ev = d * ev;
            ((ExplosionDebris*)cursor)->scale = sp - gExplosionDebrisSpeedScale * ev;
            ev =
                expf((sExplosionFadeOutExponent[0] * (f32)((ExplosionDebris*)cursor)->age) / (f32)((ExplosionDebris*)cursor)->lifetime);
            ev = sExplosionColorMax[0] * ev;
            *(s8*)&((ExplosionDebris*)cursor)->alpha = sExplosionColorMax[0] - gExplosionDebrisAlphaScale * ev;
            if (((ExplosionDebris*)cursor)->age >= ((ExplosionDebris*)cursor)->lifetime)
            {
                ((ExplosionDebris*)cursor)->active = 0;
            }
            else
            {
                *(s16*)&((ExplosionDebris*)cursor)->spinAngle +=
                    framesThisStep * *(s16*)&((ExplosionDebris*)cursor)->spinSpeed;
                if (((ExplosionDebris*)cursor)->texVariant >= 4)
                {
                    ((ExplosionDebris*)cursor)->texVariant -= 4;
                }
                if (((ExplosionDebris*)cursor)->generation < 5)
                {
                    if ((f32)((ExplosionDebris*)cursor)->age / (f32)((ExplosionDebris*)cursor)->lifetime <
                        0.2f)
                    {
                        ((ExplosionDebris*)cursor)->spawnTimer -= framesThisStep;
                        if (((ExplosionDebris*)cursor)->spawnTimer <= 0)
                        {
                            int st2;
                            u8 gen;
                            f32 sp2;
                            f32 sv;
                            gen = ((ExplosionDebris*)cursor)->generation;
                            sp2 = ((ExplosionDebris*)cursor)->speed;
                            st2 = *(int*)&(obj)->extra;
                            vpos[0] = ((ExplosionDebris*)cursor)->scale *
                                      (sExplosionChildOffsetStep[0] * (f32)(int)randomGetRange(-5, 3) + sExplosionBaseScale[0]);
                            vpos[1] = sExplosionZero[0];
                            vpos[2] = sExplosionZero[0];
                            PSMTXRotRad(
                                m, 0x7a,
                                (f32)(sExplosionPi[0] * (f64)((f32)(int)randomGetRange(0, 0xffff) / sExplosionAngleScale[0])));
                            PSMTXConcat(Camera_GetInverseViewRotationMatrix(), m, m);
                            PSMTXMultVecSR(m, vpos, vpos);
                            vpos[0] += ((ExplosionDebris*)cursor)->posX;
                            vpos[1] += ((ExplosionDebris*)cursor)->posY;
                            vpos[2] += ((ExplosionDebris*)cursor)->posZ;
                            sv = sp2 * (f32)(int)randomGetRange(0xc0, 0x100);
                            sv = sv * sExplosionSpeedScale[0];
                            if (((ExplosionState*)st2)->flameCount < 0x32)
                            {
                                explosion_spawnFlame(obj, (u8)(gen + 1), sv, vpos[0], vpos[1], vpos[2]);
                            }
                            ((ExplosionDebris*)cursor)->spawnTimer = ((ExplosionDebris*)cursor)->spawnInterval;
                        }
                    }
                }
            }
        }
        cursor += 0x30;
    }
    memcpy(&fake, (void*)obj, sizeof(fake));
    fake.rootMotionScale = sExplosionBaseScale[0];
    fake.velocityX = sExplosionZero[0];
    fake.velocityY = sExplosionZero[0];
    fake.velocityZ = sExplosionZero[0];
    for (i = 0, cursor = state; i < ((ExplosionState*)state)->debrisCount; i++)
    {
        GravityDebris* d = (GravityDebris*)((char*)cursor + 0x964);
        if (d->active != 0)
        {
            d->age += framesThisStep;
            if (d->age >= d->lifetime)
            {
                d->active = 0;
            }
            else
            {
                f32 grav = ((ExplosionState*)state)->driftYSpeed;
                u32 ft = framesThisStep;
                f32 n974 = -(grav * (f32)(u32)ft - d->velY);
                d->posY = -(0.5f * (grav * (f32)(int)(ft * ft)) - (d->velY * (f32)(u32)ft + d->posY));
                d->velY = n974;
                d->posX += d->velX * (f32)(u32)framesThisStep;
                d->posZ += d->velZ * (f32)(u32)framesThisStep;
                if (((ExplosionState*)state)->nearGround != 0 && d->posY < ((ExplosionState*)state)->groundY &&
                    d->velY < sExplosionZero[0])
                {
                    d->velY = 0.95f * -d->velY;
                }
                fake.localPosX = d->posX;
                fake.localPosY = d->posY;
                fake.localPosZ = d->posZ;
                fake.worldPosX = fake.localPosX;
                fake.worldPosY = fake.localPosY;
                fake.worldPosZ = fake.localPosZ;
                if (gExplosionUpdateTick & 1)
                {
                    int t = d->age;
                    if (t < 0x40)
                    {
                        int v = t << 6;
                        ang[0] = 0xffff - v;
                        ang[1] = ang[0];
                        ang[2] = 0x8000;
                        ang[3] = 0xc000 - v;
                        ang[4] = 0xa000 - v;
                        ang[5] = 0;
                    }
                    else if (t < 0x80)
                    {
                        int v = t << 6;
                        ang[0] = 0xc000 - v;
                        ang[1] = 0xa000 - v;
                        ang[2] = 0;
                        ang[3] = 0x8000;
                        ang[4] = 0;
                        ang[5] = 0;
                    }
                    else
                    {
                        ang[0] = 0xa000;
                        ang[1] = 0;
                        ang[2] = 0;
                        ang[3] = 0;
                        ang[4] = 0;
                        ang[5] = 0;
                    }
                    {
                        u8 md;
                        md = ((ExplosionState*)state)->modelKind;
                        switch (md)
                        {
                        case 0:
                            break;
                        case 1:
                            ang[1] = ang[2];
                            ang[4] = ang[5];
                            break;
                        case 2:
                            ang[1] = ang[0];
                            ang[4] = ang[3];
                            ang[0] = ang[2];
                            ang[3] = ang[5];
                            break;
                        case 3:
                        {
                            u16 sv5;
                            u16 sv = ang[2];
                            ang[1] = sv;
                            sv5 = ang[5];
                            ang[4] = sv5;
                            ang[2] = ang[0];
                            ang[5] = ang[3];
                            ang[0] = sv;
                            ang[3] = sv5;
                        }
                        break;
                        }
                    }
                    (*gPartfxInterface)->spawnObject((void*)obj, DIMEXPLOSION_PARTFX, &fake, 0x200001, -1, ang);
                }
            }
        }
        cursor += 0x24;
    }
    {
        int e = ((ExplosionState*)state)->frameCounter;
        int d = ((ExplosionState*)state)->lifeFrames;
        if (e > d << 1)
        {
            Obj_FreeObject(obj);
        }
        else
        {
            if (e > d)
            {
                if (*(void**)&((ExplosionState*)state)->light != NULL)
                {
                    modelLightStruct_setEnabled(((ExplosionState*)state)->light, 0, sExplosionZero[0]);
                }
            }
            else
            {
                explosion_computeColor((f32)e, (f32)d, ((ExplosionState*)state)->modelKind, rgb);
                if (*(void**)&((ExplosionState*)state)->light != NULL)
                {
                    modelLightStruct_setDiffuseColor(((ExplosionState*)state)->light, rgb[0], rgb[1], rgb[2], 0xff);
                }
            }
            {
                f32 frac = (f32)((ExplosionState*)state)->frameCounter / (f32)((ExplosionState*)state)->lifeFrames;
                (obj)->anim.rootMotionScale = 0.1f * (frac * ((ExplosionState*)state)->scale);
                (obj)->anim.alpha = sExplosionColorMax[0] - sExplosionColorMax[0] * frac;
            }
            if (((ExplosionState*)state)->halfLifeFired == 0 &&
                ((ExplosionState*)state)->frameCounter >= (((ExplosionState*)state)->lifeFrames >> 1))
            {
                u32 k;
                u16 r0v = randomGetRange(0x1000, 0x6000);
                ang[0] = r0v;
                ang[1] = r0v;
                ang[2] = r0v;
                ang[3] = ((ExplosionDebris*)state)->lifetime;
                k = 0;
                while ((f32)(int)k < ((ExplosionState*)state)->scale)
                {
                    k++;
                }
                ((ExplosionState*)state)->halfLifeFired = 1;
            }
        }
    }
}

void explosion_init(GameObject* obj, int def)
{
    f32 vsp[3];
    f32 mB[12];
    f32 mA[12];
    int cursor;
    int state = *(int*)&obj->extra;
    f32 scale;
    int i;
    int debrisCount;
    ((ExplosionState*)state)->flameCount = 0;
    if (((ExplosionPlacement*)def)->scaleParam == 0)
    {
        scale = 100.0f;
    }
    else
    {
        scale = (f32)(int)((ExplosionPlacement*)def)->scaleParam * sExplosionSpeedScale[0];
        if (scale > 100.0f)
        {
            scale = 100.0f;
        }
    }
    ((ExplosionSpawnFlameSpdFirst)explosion_spawnFlame)((int)obj, 0.4f * scale, 0, obj->anim.localPosX,
                                                        obj->anim.localPosY, obj->anim.localPosZ);
    obj->objectFlags |= DIMEXPLOSION_OBJFLAG_HITDETECT_DISABLED;
    ((ExplosionState*)state)->modelKind = ((ExplosionPlacement*)def)->configFlags & 3;
    Obj_SetActiveModelIndex(obj, ((ExplosionState*)state)->modelKind);
    if (((ExplosionPlacement*)def)->configFlags & 4)
    {
        ((ExplosionState*)state)->driftYSpeed = 0.1f;
    }
    else
    {
        ((ExplosionState*)state)->driftYSpeed = sExplosionZero[0];
    }
    ((ExplosionState*)state)->nearGround = 0;
    if (hitDetectFn_800658a4(obj, obj->anim.localPosX, 5.0f + obj->anim.localPosY,
                            obj->anim.localPosZ, (f32*)(state + 0x960), 0) == 0)
    {
        if (((ExplosionState*)state)->groundY < 20.0f)
        {
            ((ExplosionState*)state)->nearGround = 1;
        }
        ((ExplosionState*)state)->groundY = obj->anim.localPosY - ((ExplosionState*)state)->groundY;
    }
    else
    {
        ((ExplosionState*)state)->groundY = obj->anim.localPosY;
    }
    if (((ExplosionPlacement*)def)->configFlags & 0x10)
    {
        debrisCount = (int)((f32)(6.0f * scale) / 100.0f);
        for (i = 0, cursor = state; i < debrisCount; i++)
        {
            if (((ExplosionState*)state)->nearGround != 0)
            {
                f32 mag = 2.0f * ((f32)(int)randomGetRange(0x14, 0x28) * 0.01f) + 2.0f;
                vsp[0] = mag;
                vsp[1] = sExplosionZero[0];
                vsp[2] = sExplosionZero[0];
                PSMTXRotRad(mB, 0x7a,
                            (f32)(sExplosionPi[0] * (f64)((f32)(int)randomGetRange(0x2000, 0x6000) / 65535.0f)));
                PSMTXRotRad(mA, 0x79, (f32)(sExplosionPi[0] * (f64)((f32)(int)randomGetRange(0, 0xffff) / sExplosionAngleScale[0])));
                PSMTXConcat(mA, mB, mB);
                PSMTXMultVecSR(mB, vsp, vsp);
            }
            else
            {
                f32 mag = 2.0f * ((f32)(int)randomGetRange(0x14, 0x28) * 0.01f) + 2.0f;
                u8 idx;
                idx = i % 4;
                vsp[0] = mag * gExplosionSpreadDirs[idx * 3];
                vsp[1] = mag * gExplosionSpreadDirs[idx * 3 + 1];
                vsp[2] = mag * gExplosionSpreadDirs[idx * 3 + 2];
                PSMTXRotRad(
                    mB, 0x7a,
                    (f32)(sExplosionPi[0] * (f64)(((f32)(int)randomGetRange(0, 0x8000) - 16384.0f) / 65535.0f)));
                PSMTXRotRad(
                    mA, 0x78,
                    (f32)(sExplosionPi[0] * (f64)(((f32)(int)randomGetRange(0, 0x8000) - 16384.0f) / 65535.0f)));
                PSMTXConcat(mA, mB, mB);
                PSMTXMultVecSR(mB, vsp, vsp);
            }
            {
                GravityDebris* d = (GravityDebris*)((char*)cursor + 0x964);
                d->posX = obj->anim.localPosX;
                d->posY = obj->anim.localPosY;
                d->posZ = obj->anim.localPosZ;
                d->velX = vsp[0];
                d->velY = vsp[1];
                d->velZ = vsp[2];
                d->age = 0;
                d->lifetime = randomGetRange(0x28, 0x32);
                d->active = 1;
            }
            cursor += 0x24;
        }
        ((ExplosionState*)state)->debrisCount = i;
    }
    else
    {
        ((ExplosionState*)state)->debrisCount = 0;
    }
    ((ExplosionState*)state)->light = 0;
    if (((ExplosionPlacement*)def)->configFlags & 0x20)
    {
        ((ExplosionState*)state)->light = objCreateLight(0, 1);
        if (*(void**)&((ExplosionState*)state)->light != NULL)
        {
            modelLightStruct_setLightKind(((ExplosionState*)state)->light, MODEL_LIGHT_KIND_POINT);
            modelLightStruct_setPosition(((ExplosionState*)state)->light, obj->anim.worldPosX, obj->anim.worldPosY,
                                         obj->anim.worldPosZ);
            modelLightStruct_setAffectsAabbLightSelection((ModelLightStruct*)((ExplosionState*)state)->light, 1);
            modelLightStruct_setEnabled(((ExplosionState*)state)->light, 1, sExplosionZero[0]);
            modelLightStruct_setDistanceAttenuation(((ExplosionState*)state)->light, (f32)(1.5f * scale),
                                                    (f32)(sExplosionFlickerExponent[0] * scale));
            modelLightStruct_setDiffuseColor(((ExplosionState*)state)->light, 0xff, 0xeb, 0xa0, 0xff);
        }
    }
    obj->anim.alpha = 0xff;
    if (((ExplosionPlacement*)def)->configFlags & 8)
    {
        if (((ExplosionState*)state)->nearGround == 0)
        {
            ((ExplosionState*)state)->rayMode = 2;
            *(u16*)&((ExplosionState*)state)->rayYawA = randomGetRange(0, 0x4000);
            *(u16*)&((ExplosionState*)state)->rayPitchA = randomGetRange(0, 0x8000);
            *(u16*)&((ExplosionState*)state)->rayYawB = *(u16*)&((ExplosionState*)state)->rayYawA + 0x4000;
            *(u16*)&((ExplosionState*)state)->rayPitchB = *(u16*)&((ExplosionState*)state)->rayPitchA;
        }
        else
        {
            ((ExplosionState*)state)->rayMode = 1;
            ((ExplosionState*)state)->rayYawA = 0;
            ((ExplosionState*)state)->rayPitchA = 0;
        }
    }
    else
    {
        ((ExplosionState*)state)->rayMode = 0;
    }
    ((ExplosionState*)state)->halfLifeFired = 0;
    ((ExplosionState*)state)->frameCounter = 0;
    ((ExplosionState*)state)->lifeFrames = (int)(sExplosionLifeScale[0] * sqrtf(scale));
    {
        int v = ((ExplosionState*)state)->lifeFrames;
        if (v < 0)
        {
            v = 0;
        }
        else if (v > 0x3c)
        {
            v = 0x3c;
        }
        ((ExplosionState*)state)->lifeFrames = v;
    }
    ((ExplosionState*)state)->scale = scale;
    obj->anim.rootMotionScale = sExplosionZero[0];
}

void explosion_release(u32 obj)
{
    int i;

    for (i = 0; i < GEXPLOSION_TEXTURE_COUNT; i++)
    {
        if (gExplosionTextures[i] != NULL)
        {
            textureFree((Texture*)gExplosionTextures[i]);
            gExplosionTextures[i] = NULL;
        }
    }
}

void explosion_initialise(void)
{
    FbTexTbl t;
    int i;
    t = gExplosionTexTable;
    gExplosionDebrisSpeedScale = sExplosionBaseScale[0] / expf(sExplosionFadeInExponent[0]);
    gExplosionDebrisAlphaScale = sExplosionBaseScale[0] / expf(sExplosionFadeOutExponent[0]);
    gExplosionDebrisColorScale = sExplosionBaseScale[0] / expf(sExplosionFlickerExponent[0]);
    gExplosionFalloffScaleRed = sExplosionBaseScale[0] / expf(7.5f);
    gExplosionFalloffScaleGreen = sExplosionBaseScale[0] / expf(2.5f);
    gExplosionFalloffScaleBlue = sExplosionBaseScale[0] / expf(sExplosionBaseScale[0]);
    for (i = 0; i < GEXPLOSION_TEXTURE_COUNT; i++)
    {
        gExplosionTextures[i] = textureLoadAsset(t.v[i]);
    }
}

f32 gExplosionSpreadDirs[] = {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
};

ObjectDescriptor gExplosionObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)explosion_initialise,
    (ObjectDescriptorCallback)explosion_release,
    0,
    (ObjectDescriptorCallback)explosion_init,
    (ObjectDescriptorCallback)explosion_update,
    (ObjectDescriptorCallback)explosion_hitDetect,
    (ObjectDescriptorCallback)explosion_render,
    (ObjectDescriptorCallback)explosion_free,
    (ObjectDescriptorCallback)explosion_getObjectTypeId,
    explosion_getExtraSize,
};
