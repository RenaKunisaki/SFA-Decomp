/*
 * DLL 597 - SnowBike vehicle logic, including its trail, route, attachment,
 * collision, movement, and rendering helpers.
 */
#include "dlls/object_descriptor.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "dolphin/gx/GXCull.h"
#include "dolphin/gx/GXEnum.h"
#include "dolphin/gx/GXGeometry.h"
#include "dolphin/gx/GXPixel.h"
#include "dolphin/gx/GXTev.h"
#include "dolphin/gx/GXTransform.h"
#include "dolphin/mtx/mtx_legacy.h"
#include "dolphin/os/OSReport.h"
#include "game/objects/object.h"
#include "game/objects/object_setup.h"
#include "main/audio/sfx.h"
#include "main/audio/sfx_channel_query_api.h"
#include "main/audio/sfx_channel_volume_api.h"
#include "main/audio/sfx_ids.h"
#include "main/audio/sfx_play_api.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/camera.h"
#include "main/camera_interface.h"
#include "main/checkpoint_interface.h"
#include "main/dll/DR/DRcloudcage.h"
#include "main/dll/DR/DRpickup.h"
#include "main/dll/DR/DRshackle.h"
#include "main/dll/DR/drcloudcage_internal.h"
#include "main/dll/SP/dll_0287_spscarab.h"
#include "main/dll/dll_0015_curves.h"
#include "main/dll/dll_0255_snowbike.h"
#include "main/dll/dll_801e991c.h"
#include "main/dll/drhightop.h"
#include "main/dll/objfx_api.h"
#include "main/dll/partfx_interface.h"
#include "main/dll/path_control_interface.h"
#include "main/dll/shwgpipe_struct.h"
#include "main/dll/tricky_api.h"
#include "main/frame_timing.h"
#include "main/game_ui_interface.h"
#include "main/gamebits.h"
#include "main/gametext_show_api.h"
#include "main/lightmap.h"
#include "main/lightmap_api.h"
#include "main/maketex_api.h"
#include "main/mm.h"
#include "main/obj_group.h"
#include "main/obj_path.h"
#include "main/object_render.h"
#include "main/objfx.h"
#include "main/objhits.h"
#include "main/objseq.h"
#include "main/pad.h"
#include "main/rcp_dolphin_api.h"
#include "main/shader_api.h"
#include "main/sky.h"
#include "main/texture.h"
#include "main/track_dolphin_api.h"
#include "main/vecmath.h"
#include "string.h"
#include "sys/objects.h"
#include "sys/objects/lifecycle.h"
#include "track/intersect_depth_state_api.h"
#include "track/intersect_geom_api.h"
#include "track/intersect_render_setup_api.h"

const u32 lbl_803E5AE0 = 0x05050505;
const GXColor lbl_803E5AE4 = {0x20, 0x20, 0x20, 0x80};
const f32 lbl_803E5AE8 = 0.0f;
const f32 lbl_803E5AEC = 1.0f;
const f32 lbl_803E5AF0 = 8.0f;
const f32 lbl_803E5AF4 = 20.0f;
const f32 lbl_803E5AF8 = 0.5f;
const f32 lbl_803E5AFC = -20.0f;
const f32 lbl_803E5B08 = 70.0f;
const f32 lbl_803E5B0C = 11.6f;
const f32 lbl_803E5B10 = 40.0f;
const f32 lbl_803E5B14 = 200.0f;
const f32 lbl_803E5B18 = 18.0f;
const f32 lbl_803E5B1C = 30.0f;
const f32 lbl_803E5B20 = 0.1f;
const f32 lbl_803E5B24 = 30000.0f;
const f32 lbl_803E5B28 = 127.0f;
const f32 lbl_803E5B2C = 1.5f;
const f32 lbl_803E5B30 = 45.0f;
const f32 lbl_803E5B34 = 0.3f;
const f32 lbl_803E5B38 = 0.00390625f;
const f32 lbl_803E5B3C = 60.0f;
const f32 lbl_803E5B40 = 80.0f;
const f32 lbl_803E5B44 = 65.0f;
const f32 lbl_803E5B48 = 100.0f;
const f32 lbl_803E5B4C = -5.3f;
const f32 lbl_803E5B50 = 4.4f;
const f32 lbl_803E5B54 = 24.0f;
const f32 lbl_803E5B58 = 250.0f;
const f32 lbl_803E5B5C = 5.3f;
const f32 lbl_803E5B60 = 7.0f;
const f32 lbl_803E5B64 = 6.5f;
const f32 lbl_803E5B68 = 180.0f;
const f32 lbl_803E5B6C = 56.0f;
const f32 lbl_803E5B70 = -1.0f;
const f32 lbl_803E5B74 = -0.05f;
const f32 lbl_803E5B78 = 2.0f;
const f32 lbl_803E5B7C = 90.0f;
const f32 lbl_803E5B80 = 100000.0f;
const f32 lbl_803E5B84 = 10000.0f;
const f32 lbl_803E5B88 = 0.95f;
const f32 lbl_803E5B8C = 0.01f;
const f32 lbl_803E5B90 = 70000.0f;
const f32 lbl_803E5B94 = 69999.0f;
const f32 lbl_803E5B98 = 4.0f;
const f32 lbl_803E5B9C = -2.0f;
const f32 lbl_803E5BA0 = 850.0f;
const f32 lbl_803E5BA4 = -0.01f;
const f32 lbl_803E5BA8 = 0.2f;
const f32 lbl_803E5BAC = 0.7f;
const f32 lbl_803E5BB0 = 300.0f;
const f32 lbl_803E5BB4 = 120.0f;
const f32 lbl_803E5BB8 = 5.0f;
const f32 lbl_803E5BBC = 0.25f;
const f32 lbl_803E5BC0 = 12.0f;
const f32 lbl_803E5BC4 = 3.0f;
const f32 lbl_803E5BC8 = 16384.0f;
const f32 lbl_803E5BCC = 0.8f;
const f32 gDrHighTopPi = 3.1415927f;
const f32 lbl_803E5BD4 = 32768.0f;
const f32 lbl_803E5BD8 = 0.005f;
const f32 lbl_803E5BDC = 1500.0f;
const f32 lbl_803E5BE0 = 0.98f;
const f32 lbl_803E5BE4 = 0.99f;
const f32 lbl_803E5BE8 = 0.15f;
const f32 lbl_803E5BEC = 0.4f;
const f32 lbl_803E5BF0 = 0.18f;
const f32 lbl_803E5BF4 = 700.0f;
const f32 lbl_803E5BF8 = 0.87f;
const f32 lbl_803E5BFC = 0.97f;
const f32 lbl_803E5C00 = 0.75f;
const f32 lbl_803E5C04 = 0.965f;
const f32 lbl_803E5C08 = 0.985f;
const f32 lbl_803E5C0C = 0.45f;
const f32 lbl_803E5C10 = 15.0f;
const f32 lbl_803E5C14 = 0.04f;
const f32 gSnowBikeBamToDeg = 182.04f;
const f32 lbl_803E5C28 = 0.05f;
const f32 lbl_803E5C2C = -0.002f;
const f32 lbl_803E5C30 = -100.0f;
const f32 lbl_803E5C34 = 50.0f;
const f32 lbl_803E5C38 = 0.85f;
const f32 lbl_803E5C3C = -0.12f;
const f32 lbl_803E5C40 = 0.995f;
const f32 lbl_803E5C44 = 0.08f;
const f32 lbl_803E5C48 = 400.0f;
const f32 lbl_803E5C4C = 25.0f;
const f32 lbl_803E5C50 = 6.0f;
const f32 lbl_803E5C54 = 600.0f;
const f32 lbl_803E5C58 = 1.2f;
const f32 lbl_803E5C5C = 500.0f;
const f32 lbl_803E5C60 = 1.1f;
const f32 lbl_803E5C64 = -0.1f;
const f32 lbl_803E5C68 = 10.0f;

typedef union SnowBikeCheckpointRank
{
    CheckpointRankItem item;
    u8 bytes[0x38];
} SnowBikeCheckpointRank;

STATIC_ASSERT(sizeof(SnowBikeCheckpointRank) == 0x38);

SnowBikeCheckpointRank lbl_803AD088;

f32 lbl_803284E0[19] = {
    -6.5f, 0.0f,  -13.0f, 6.5f, 0.0f, -13.0f, 6.5f, 0.0f, 13.0f, -6.5f,
    0.0f,  13.0f, 1.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 0.0f,
};

int gDrHighTopHitObjectKinds[] = {
    0x72, 0x16D, 0x170, 0x16C, 0x16F, 0x38C, 0x389, 0x38A, 0x4D3, 0x38D, 0x38E, 0x4D4,
};

/* Hittable rider, bike, and scenery object IDs. */
s16 gSnowBikeHitObjectIdTable[26] = {
    0, 365, 0, 368, 0, 364, 0, 367, 0, 905, 0, 906, 0, 1235, 0, 909, 0, 910, 0, 1236, 1175, 1176, 1180, 930, 931, 1180,
};

int gSnowBikeMountRomListTable[6] = {0x30C60, 0x30C60, 0x30C60, 0xC9E, 0xC9F, 0xCB3};

f32 lbl_803DC0B8 = 15.0f;
int lbl_803DC0BC = -1;
f32 lbl_803DC0C0 = 8.5f;
f32 lbl_803DC0C4 = 6.0f;
f32 lbl_803DC0C8 = 1.05f;
int lbl_803DC0CC = 5000;
int lbl_803DC0D0 = 60;
int lbl_803DC0D4 = 20;
f32 lbl_803DC0D8 = 4.7f;
s16 lbl_803DC0DC = 0x4000;
f32 gDrCloudCageRouteDistGate = 2000.0f;
char sSnowBikeVelDebugFmt[] = "vel %f\n";

/* Trail renderer used by the SnowBike effects below. */

#define GX_BM_BLEND       1
#define GX_BL_SRCALPHA    4
#define GX_BL_INVSRCALPHA 5
#define GX_LO_NOOP        5
#define GX_LEQUAL         3
#define GX_ALWAYS         7
#define GX_AOP_AND        0
#define GX_CULL_NONE      0
#define GX_VA_POS         9
#define GX_VA_CLR0        11
#define GX_VA_TEX0        13
#define GX_DIRECT         1
#define GX_QUADS          0x80
#define GX_VTXFMT2        2

Texture* lbl_803DDC60;

#define GXWGFifo (*(volatile ShWGPipe*)0xCC008000)

static inline void shPos3f32(const f32 x, const f32 y, const f32 z)
{
    GXWGFifo.f32 = x;
    GXWGFifo.f32 = y;
    GXWGFifo.f32 = z;
}

static inline void shColor4u8(u8 r, u8 g, u8 b, u8 a)
{
    GXWGFifo.u8 = r;
    GXWGFifo.u8 = g;
    GXWGFifo.u8 = b;
    GXWGFifo.u8 = a;
}

static inline void shTexCoord2f32(const f32 s, const f32 t)
{
    GXWGFifo.f32 = s;
    GXWGFifo.f32 = t;
}

void SnowBike_DrawTrails(int p1, char* table)
{
    u8 r;
    u8 g;
    u8 b;
    GXColor color;
    f32* verts;
    char* p;
    int i;
    int j;
    f32 texT;
    f32 texS;

    color = lbl_803E5AE4;
    selectTexture((Texture*)lbl_803DDC60, 0);
    textureSetupFn_800799c0();
    geomDrawFn_800796f0();
    textRenderSetupFn_80079804();
    GXSetTevColor(GX_TEVREG1, color);
    gxSetZMode_(1, GX_LEQUAL, 0);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    gxSetPeControl_ZCompLoc_(1);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetCullMode(GX_CULL_NONE);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXLoadPosMtxImm((const f32(*)[4])Camera_GetViewMatrix(), GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
    getAmbientColor(0, &r, &g, &b);
    i = 0;
    p = table;
    for (; i < 9; i++)
    {
        if (((*(u8*)(p + 0x4ce) & 1) != 0) && (*(s16*)(p + 0x4cc) >= 4))
        {
            j = 0;
            verts = *(f32**)(p + 0x4c8);
            texS = lbl_803E5AE8;
            texT = lbl_803E5AEC;
            while (j < *(s16*)(p + 0x4cc) - 2)
            {
                GXBegin(GX_QUADS, GX_VTXFMT2, 4);
                shPos3f32(verts[0] - playerMapOffsetX, verts[0 + 1], verts[0 + 2] - playerMapOffsetZ);
                shColor4u8(*(u8*)&r, *(u8*)&g, *(u8*)&b, (u8) * (s16*)((char*)verts + 0xc));
                shTexCoord2f32(texS, texS);
                shPos3f32(verts[4] - playerMapOffsetX, verts[4 + 1], verts[4 + 2] - playerMapOffsetZ);
                shColor4u8(*(u8*)&r, *(u8*)&g, *(u8*)&b, (u8) * (s16*)((char*)verts + 0x1c));
                shTexCoord2f32(texT, texS);
                shPos3f32(verts[0xc] - playerMapOffsetX, verts[0xc + 1], verts[0xc + 2] - playerMapOffsetZ);
                shColor4u8(*(u8*)&r, *(u8*)&g, *(u8*)&b, (u8) * (s16*)((char*)verts + 0x3c));
                shTexCoord2f32(texT, texS);
                shPos3f32(verts[8] - playerMapOffsetX, verts[8 + 1], verts[8 + 2] - playerMapOffsetZ);
                shColor4u8(*(u8*)&r, *(u8*)&g, *(u8*)&b, (u8) * (s16*)((char*)verts + 0x2c));
                shTexCoord2f32(texS, texS);
                verts += 8;
                j += 2;
            }
        }
        p += 8;
    }
}

/*
 * Cloud-cage trail and audio effects used by the SnowBike.
 *
 * Provides three routines for the vehicle's trail, sound, and pitch state:
 *   fn_801E9C00  builds and fades the swirling cloud-trail ribbons. Each of the
 *                three emitters casts a transformed segment, ray-tests it
 *                (hitDetectFn_80065e50, mask 0x20), and when it strikes ground
 *                inserts a new fully-opaque point pair at the head of one of the
 *                nine trail buffers; every existing pair's alpha decays by
 *                timeDelta and exhausted trails are freed.
 *   drcloudcage_updateEngineFx  drives the wind/engine sfx channels (8,1,2,4) by distance and
 *                rotZ, clamps each channel volume, and spawns two light pulses;
 *                then advances the trails via fn_801E9C00.
 *   fn_801EA678  returns a distance/route-rank weighted scalar (pitch/intensity)
 *                from the checkpoint route rank, falling back to player distance
 *                when no rank gate (lbl_803DC0BC) is set.
 *
 * State is addressed through raw byte offsets into the owning object's extra
 * block; trail buffers begin at DRCLOUDCAGE_TRAILS_OFFSET (DRCLOUDCAGE_TRAIL_COUNT
 * records of DRCLOUDCAGE_TRAIL_STRIDE bytes), with the three active head-trail
 * pointers immediately following at +0x510/+0x514/+0x518.
 */

/* Shared route-rank state used by the trail, pitch, and vehicle routines. */
struct DRCloudCagePoints;

#define DRCLOUDCAGE_TRAIL_COUNT       9
#define DRCLOUDCAGE_TRAIL_STRIDE      8
#define DRCLOUDCAGE_TRAILS_OFFSET     0x4c8
#define DRCLOUDCAGE_PAIR_SIZE         0x10
#define DRCLOUDCAGE_TRAIL_FLAG_ACTIVE 1

typedef struct DRCloudCagePointPair
{
    f32 startX;
    f32 startY;
    f32 startZ;
    s16 startAlpha;
    u8 startColorByte;
    u8 pad0F;
    f32 endX;
    f32 endY;
    f32 endZ;
    s16 endAlpha;
    u8 endColorByte;
    u8 pad1F;
} DRCloudCagePointPair;

typedef struct DRCloudCageTrail
{
    DRCloudCagePointPair* points;
    s16 count;
    u8 flags;
    u8 pad07;
} DRCloudCageTrail;

typedef struct DRCloudCagePoints
{
    f32 m[18];
} DRCloudCagePoints;

const DRCloudCagePoints gDrCloudCagePointTemplate = {
    {-6.0f, 1.0f, 15.0f, 6.0f, 1.0f, 15.0f, -7.5f, 1.0f, 15.0f, -4.0f, 1.0f,
     15.0f, 4.0f, 1.0f, 15.0f, 7.5f, 1.0f, 15.0f}};

void fn_801E9C00(GameObject* obj, int state)
{
    f32 endZ;
    f32 endY;
    f32 endX;
    f32 startZ;
    f32 startY;
    f32 startX;
    TrackGroundHit** hits;
    MatrixTransform transform;
    f32 matrix[16];
    DRCloudCagePoints localPoints;
    u8* p;
    int trailIndex;
    DRCloudCageTrail* trail;
    int pairIndex;
    u8* points;
    DRCloudCagePointPair* pair;
    s32 a;
    f32 fade;
    int copyOffset;
    int activeOffset;
    f32* endpoint;
    u8* slot;
    f32* pStartZ;
    f32* pStartY;
    f32* pEndZ;
    f32* pEndY;
    f32* pEndX;
    int endpointIndex;
    DRCloudCageTrail* selectedTrail;
    int activeIndex;
    int nextOffset;
    int scanIndex;
    int hitIndex;
    int hitCount;
    int copyIndex;
    u8 hitDetected;
    f32 deltaY;
    f32 maxDelta;
    f32 zero;
    f32 scaleV;
    f32 minDelta;
    int baseOffset;
    int baseOffset2;

    localPoints = gDrCloudCagePointTemplate;

    for (trailIndex = 0, p = (u8*)state; trailIndex < DRCLOUDCAGE_TRAIL_COUNT;
         p += DRCLOUDCAGE_TRAIL_STRIDE, trailIndex++)
    {
        trail = (DRCloudCageTrail*)(p + DRCLOUDCAGE_TRAILS_OFFSET);
        if (trail->flags & DRCLOUDCAGE_TRAIL_FLAG_ACTIVE)
        {
            pairIndex = trail->count - 2;
            points = (u8*)trail->points;
            pair = (DRCloudCagePointPair*)((u8*)trail->points + pairIndex * DRCLOUDCAGE_PAIR_SIZE);
            fade = lbl_803E5AF0;
            for (; pairIndex >= 0; pair--, pairIndex -= 2)
            {
                pair->startAlpha = -(fade * timeDelta - pair->startAlpha);
                pair->endAlpha = pair->startAlpha;
                a = pair->startAlpha;
                if (a < 0)
                {
                    a = 0;
                }
                else if (a > 0xff)
                {
                    a = 0xff;
                }
                pair->startAlpha = a;
                a = pair->endAlpha;
                if (a < 0)
                {
                    a = 0;
                }
                else if (a > 0xff)
                {
                    a = 0xff;
                }
                pair->endAlpha = a;
            }

            pairIndex = trail->count - 2;
            pair = (DRCloudCagePointPair*)(points + pairIndex * DRCLOUDCAGE_PAIR_SIZE);
            for (; pairIndex >= 0; pair--, pairIndex -= 2)
            {
                if (pairIndex >= 2)
                {
                    if ((pair->startAlpha <= 0) && (pair->endAlpha <= 0) && (*(s16*)((u8*)pair - 4) <= 0) &&
                        (*(s16*)((u8*)pair - 0x14) <= 0))
                    {
                        trail->count -= 2;
                    }
                }
                else
                {
                    if ((pair->startAlpha <= 0) && (pair->endAlpha <= 0))
                    {
                        trail->count -= 2;
                    }
                }
            }

            /* The three active head-trail pointer slots (0x510/0x514/0x518)
             * stay raw: the spawn loop below walks them via a running `slot`
             * base (slot += 4), so naming them as fixed struct fields shifts
             * the walker's addressing/CSE. */
            if ((trail != *(DRCloudCageTrail**)(state + 0x510)) && (trail != *(DRCloudCageTrail**)(state + 0x514)) &&
                (trail != *(DRCloudCageTrail**)(state + 0x518)) && (trail->count == 0))
            {
                trail->flags &= ~DRCLOUDCAGE_TRAIL_FLAG_ACTIVE;
            }
        }
    }

    activeIndex = 0;
    baseOffset = -4;
    baseOffset2 = 8;
    slot = (u8*)state;
    pStartZ = &startZ;
    pStartY = &startY;
    pEndZ = &endZ;
    pEndY = &endY;
    pEndX = &endX;
    zero = lbl_803E5AE8;
    maxDelta = lbl_803E5AF4;
    minDelta = lbl_803E5AFC;
    scaleV = lbl_803E5AEC;
    for (; activeIndex < 3; baseOffset += 0x18, baseOffset2 += 0x18, slot += 4, activeIndex++)
    {
        activeOffset = baseOffset + 4;
        nextOffset = baseOffset2 + 4;
        transform.x = obj->anim.worldPosX;
        transform.y = obj->anim.worldPosY;
        transform.z = obj->anim.worldPosZ;
        transform.rotX = obj->anim.rotX;
        transform.rotY = obj->anim.rotY;
        transform.rotZ = (s16)(obj->anim.rotZ + ((DRCloudCageState*)state)->rotZOffset);
        transform.scale = scaleV;
        setMatrixFromObjectPos(matrix, &transform);

        Matrix_TransformPoint(matrix, ((f32*)((u8*)&localPoints + activeOffset))[0],
                              ((f32*)((u8*)&localPoints + activeOffset))[1],
                              ((f32*)((u8*)&localPoints + activeOffset))[2], &startX, pStartY, pStartZ);
        Matrix_TransformPoint(matrix, ((f32*)((u8*)&localPoints + nextOffset))[0],
                              ((f32*)((u8*)&localPoints + nextOffset))[1], ((f32*)((u8*)&localPoints + nextOffset))[2],
                              pEndX, pEndY, pEndZ);

        hitDetected = 0;
        endpointIndex = 0;
        endpoint = &startX;
        for (; endpointIndex < 2; endpoint += 3, endpointIndex++)
        {
            hitCount = hitDetectFn_80065e50(obj, endpoint[0], endpoint[1], endpoint[2], &hits, 0, 0x20);
            for (hitIndex = 0; hitIndex < hitCount; hitIndex++)
            {
                deltaY = hits[hitIndex]->height - endpoint[1];
                if (activeIndex > 0)
                {
                    if ((deltaY > zero) && (deltaY < maxDelta))
                    {
                        hitDetected = 1;
                        endpoint[1] = lbl_803E5AF8 + hits[hitIndex]->height;
                        break;
                    }
                }
                else if ((deltaY >= minDelta) && (deltaY < maxDelta))
                {
                    hitDetected = 1;
                    endpoint[1] = lbl_803E5AF8 + hits[hitIndex]->height;
                    break;
                }
            }
        }

        if (!((DRCloudCageState*)state)->stateFlags.hidden && hitDetected)
        {
            selectedTrail = *(DRCloudCageTrail**)(slot + 0x510);
            if (selectedTrail == NULL)
            {
                for (scanIndex = 0; scanIndex < DRCLOUDCAGE_TRAIL_COUNT; scanIndex++)
                {
                    selectedTrail =
                        (DRCloudCageTrail*)(state + scanIndex * DRCLOUDCAGE_TRAIL_STRIDE + DRCLOUDCAGE_TRAILS_OFFSET);
                    if (!(selectedTrail->flags & DRCLOUDCAGE_TRAIL_FLAG_ACTIVE))
                    {
                        break;
                    }
                }
                if (scanIndex >= DRCLOUDCAGE_TRAIL_COUNT)
                {
                    break;
                }
                selectedTrail->flags |= DRCLOUDCAGE_TRAIL_FLAG_ACTIVE;
                selectedTrail->count = 0;
                *(DRCloudCageTrail**)(slot + 0x510) = selectedTrail;
            }
            else
            {
                copyIndex = selectedTrail->count - 1;
                copyOffset = copyIndex * DRCLOUDCAGE_PAIR_SIZE;
                while (copyIndex >= 0)
                {
                    memcpy((u8*)selectedTrail->points + (copyIndex + 2) * DRCLOUDCAGE_PAIR_SIZE,
                           (u8*)selectedTrail->points + copyOffset, DRCLOUDCAGE_PAIR_SIZE);
                    copyOffset -= DRCLOUDCAGE_PAIR_SIZE;
                    copyIndex--;
                }
            }

            selectedTrail->points[0].startX = startX;
            selectedTrail->points[0].startY = startY;
            selectedTrail->points[0].startZ = startZ;
            selectedTrail->points[0].endX = endX;
            selectedTrail->points[0].endY = endY;
            selectedTrail->points[0].endZ = endZ;
            selectedTrail->points[0].startAlpha = 0xff;
            selectedTrail->points[0].endAlpha = 0xff;
            selectedTrail->points[0].startColorByte = ((DRCloudCageState*)state)->trailColorByte;
            selectedTrail->points[0].endColorByte = ((DRCloudCageState*)state)->trailColorByte;
            selectedTrail->count += 2;
            ((DRCloudCageState*)state)->lastSpawnPosX = obj->anim.worldPosX;
            ((DRCloudCageState*)state)->lastSpawnPosY = obj->anim.worldPosY;
            ((DRCloudCageState*)state)->lastSpawnPosZ = obj->anim.worldPosZ;
        }
        else
        {
            *(DRCloudCageTrail**)(slot + 0x510) = 0;
        }
    }
}

f32 gDrCloudCageWindVolume;


typedef struct DRCloudCagePulseParams
{
    u8 pad[8];
    f32 unk8;
    f32 unkC;
    f32 unk10;
    f32 unk14;
} DRCloudCagePulseParams;

void drcloudcage_updateEngineFx(GameObject* obj, void* state, f32 distanceScale, int intensity, u8* unused,
                                u8 channelFlags)
{
    f32 clamped;
    f32 windVol;
    f32 fv;
    int vol;
    f32 channelVol;
    f32 channelVol4;
    DRCloudCagePulseParams pulse;

    clamped =
        (distanceScale < lbl_803E5AE8) ? lbl_803E5AE8 : ((distanceScale > lbl_803E5B08) ? lbl_803E5B08 : distanceScale);
    if (channelFlags & 1)
    {
        if (Sfx_IsPlayingFromObjectChannel((int)obj, 8))
        {
            gDrCloudCageWindVolume = lbl_803E5B0C * clamped;
            if (gDrCloudCageWindVolume < lbl_803E5AE8)
            {
                gDrCloudCageWindVolume = -gDrCloudCageWindVolume;
            }
            if (gDrCloudCageWindVolume < lbl_803E5B10)
            {
                gDrCloudCageWindVolume = lbl_803E5B10;
            }
            if (gDrCloudCageWindVolume > lbl_803E5B14)
            {
                gDrCloudCageWindVolume = lbl_803E5B14;
            }
            if (((DRCloudCageState*)state)->distanceGate < lbl_803E5B18)
            {
                vol = (int)(lbl_803E5B1C * clamped);
                if (vol < 0)
                {
                    vol = -vol;
                }
                if (vol > 0x7f)
                {
                    vol = 0x7f;
                }
            }
            else
            {
                vol = 0;
            }
            Sfx_SetObjectChannelVolume((u32)obj, 8, vol & 0xff,
                                       lbl_803E5B20 + gDrCloudCageWindVolume / lbl_803E5B08);
        }
    }
    if (channelFlags & 2)
    {
        if (Sfx_IsPlayingFromObjectChannel((int)obj, 1))
        {
            if (((DRCloudCageState*)state)->distanceGate < lbl_803E5B18)
            {
                windVol = 0.0f;
                if (windVol != clamped)
                {
                    windVol = clamped * (f32)obj->anim.rotZ / lbl_803E5B24;
                }
                gDrCloudCageWindVolume = windVol;
                fv = (f32)(f64)windVol;
                if (fv < 0.0f)
                {
                    gDrCloudCageWindVolume = -fv;
                }
                else if (fv > lbl_803E5AEC)
                {
                    gDrCloudCageWindVolume = lbl_803E5AEC;
                }
                vol = (int)(lbl_803E5B28 * gDrCloudCageWindVolume);
                if ((f32)vol > lbl_803E5B28)
                {
                    vol = 0x7f;
                }
                else if ((f32)vol < 0.0f)
                {
                    vol = 0;
                }
                Sfx_SetObjectChannelVolume((u32)obj, 1, vol & 0xff, lbl_803E5B20 + gDrCloudCageWindVolume);
            }
        }
    }
    if (channelFlags & 4)
    {
        Sfx_PlayFromObject((u32)obj, ((DRCloudCageState*)state)->windSfxId);
        Sfx_PlayFromObject((u32)obj, SFXTRIG_tr_gal_rumblelp11);
        if (intensity > 5)
        {
            ((DRCloudCageState*)state)->channel2Vol = ((DRCloudCageState*)state)->channel2Vol + timeDelta;
        }
        else
        {
            if (((DRCloudCageState*)state)->channel2Vol > lbl_803E5B10)
            {
                ((DRCloudCageState*)state)->channel2Vol =
                    -(lbl_803E5B2C * timeDelta - ((DRCloudCageState*)state)->channel2Vol);
            }
        }
        if (((DRCloudCageState*)state)->channel2Vol > *(f32*)&lbl_803E5B08)
        {
            ((DRCloudCageState*)state)->channel2Vol = *(f32*)&lbl_803E5B08;
        }
        if (((DRCloudCageState*)state)->channel2Vol < *(f32*)&lbl_803E5B30)
        {
            ((DRCloudCageState*)state)->channel2Vol = *(f32*)&lbl_803E5B30;
        }
        channelVol = ((DRCloudCageState*)state)->channel2Vol;
        ((void (*)(GameObject*, u32, u8, f32))Sfx_SetObjectChannelVolume)(obj, 2, channelVol, channelVol * lbl_803E5B38 + lbl_803E5B34);
        if (intensity > 5)
        {
            ((DRCloudCageState*)state)->channel4Vol = lbl_803E5B3C + intensity;
        }
        else
        {
            if (((DRCloudCageState*)state)->channel4Vol > lbl_803E5B3C)
            {
                ((DRCloudCageState*)state)->channel4Vol =
                    -(lbl_803E5AF8 * timeDelta - ((DRCloudCageState*)state)->channel4Vol);
            }
        }
        if (((DRCloudCageState*)state)->channel4Vol > *(f32*)&lbl_803E5B40)
        {
            ((DRCloudCageState*)state)->channel4Vol = *(f32*)&lbl_803E5B40;
        }
        if (((DRCloudCageState*)state)->channel4Vol < *(f32*)&lbl_803E5B44)
        {
            ((DRCloudCageState*)state)->channel4Vol = *(f32*)&lbl_803E5B44;
        }
        channelVol4 = ((DRCloudCageState*)state)->channel4Vol;
        ((void (*)(GameObject*, u32, u8, f32))Sfx_SetObjectChannelVolume)(obj, 4, channelVol4, channelVol4 / lbl_803E5B48);
        pulse.unkC = lbl_803E5B4C;
        pulse.unk10 = lbl_803E5B50;
        pulse.unk14 = lbl_803E5B54;
        pulse.unk8 = lbl_803E5AE8;
        objfx_spawnLightPulse(obj, lbl_803E5AF8, 2, 0, 1, ((DRCloudCageState*)state)->channel4Vol / lbl_803E5B58,
                              &pulse);
        pulse.unkC = lbl_803E5B5C;
        objfx_spawnLightPulse(obj, lbl_803E5AF8, 2, 0, 1, ((DRCloudCageState*)state)->channel4Vol / lbl_803E5B58,
                              &pulse);
    }
    fn_801E9C00(obj, (int)state);
}

f32 fn_801EA678(GameObject* obj, int state)
{
    f32 result;
    f32 d;
    f32 templateMetric;
    f32 stateMetric;
    int rank;

    if ((lbl_803DC0BC == -1) ||
        (rank = (*gCheckpointInterface)->getRouteRank((CheckpointRankItem*)(state + 0x28)), lbl_803DC0BC > rank))
    {
        if (lbl_803DC0BC == -1)
        {
            rank = (int)Obj_GetPlayerObject();
            d = Vec_distance(&obj->anim.worldPosX, (f32*)(rank + 0x18));
            d = d * lbl_803E5AF8;
        }
        else
        {
            /* state+0x28 is the CheckpointRankItem passed to getRouteRank;
             * its linkDepth (+0x1C = 0x44) and routeProgress (+0xC = 0x34) are
             * read here. These stay raw: spelling them as nested-struct members
             * (rankItem.linkDepth / rankItem.routeProgress) shifts codegen. */
            templateMetric = lbl_803E5B48 * (f32) * (s32*)(lbl_803AD088.bytes + 0x1c) +
                             lbl_803E5B48 * *(f32*)(lbl_803AD088.bytes + 0xc);
            stateMetric = lbl_803E5B48 * (f32) * (s32*)(state + 0x44) + lbl_803E5B48 * *(f32*)(state + 0x34);
            d = templateMetric - stateMetric;
            d = (d >= lbl_803E5AE8) ? d : -d;
        }
        if (d <= ((DRCloudCageState*)state)->distNear)
        {
            result = ((DRCloudCageState*)state)->valNear;
        }
        else if (d >= ((DRCloudCageState*)state)->distFar)
        {
            result = ((DRCloudCageState*)state)->valFar;
        }
        else
        {
            f32 ratio = (d - ((DRCloudCageState*)state)->distNear) /
                        (((DRCloudCageState*)state)->distFar - ((DRCloudCageState*)state)->distNear);
            d = ((DRCloudCageState*)state)->valNear;
            result = ratio * (((DRCloudCageState*)state)->valFar - d) + d;
        }
        if (((DRCloudCageState*)state)->routeGateActive == 0)
        {
            d = stateMetric - templateMetric;
            d = (d >= lbl_803E5AE8) ? d : -d;
            if (d > gDrCloudCageRouteDistGate)
            {
                result = *(f32*)&lbl_803E5AE8;
            }
        }
    }
    else
    {
        rank = (*gCheckpointInterface)->getRouteRank((CheckpointRankItem*)(state + 0x28));
        if (rank == 2)
        {
            result = lbl_803E5B60;
        }
        else
        {
            result = lbl_803E5B64;
        }
    }
    return result;
}

/*
 * SnowBike shackle swing and attachment math.
 *
 * drshackle_updateAttachedPosition rides the shackle along its checkpoint
 * route while it tracks the player: on first contact it anchors to the
 * route (snapping yaw, seeding swing accel and floor offset), thereafter
 * it advances the route and blends the swing each frame.
 * drshackle_updateSwingBlend computes the per-frame swing-blend factor
 * from the yaw delta between the object and its anchor, clamps it, and
 * decides the return direction.
 *
 * Several `lbl_803E5Bxx` are plain float constants (see the inline value
 * comments).
 */

STATIC_ASSERT(offsetof(ShackleSwingState, anchorX) == 0x0C);
STATIC_ASSERT(offsetof(ShackleSwingState, collider) == 0x28);
STATIC_ASSERT(offsetof(ShackleSwingState, colliderMode) == 0x5D);
STATIC_ASSERT(offsetof(ShackleSwingState, attachment) == 0x178);
STATIC_ASSERT(offsetof(ShackleSwingState, distanceFade) == 0x3E4);
STATIC_ASSERT(offsetof(ShackleSwingState, yaw) == 0x40C);
STATIC_ASSERT(offsetof(ShackleSwingState, targetYaw) == 0x40E);
STATIC_ASSERT(offsetof(ShackleSwingState, flags) == 0x428);
STATIC_ASSERT(offsetof(ShackleSwingState, swingAccel) == 0x430);
STATIC_ASSERT(offsetof(ShackleSwingState, floorAdjustFlag) == 0x434);
STATIC_ASSERT(offsetof(ShackleSwingState, swingCommand) == 0x44C);
STATIC_ASSERT(offsetof(ShackleSwingState, swingReturn) == 0x458);
STATIC_ASSERT(offsetof(ShackleSwingState, swingBlend) == 0x45C);
STATIC_ASSERT(offsetof(ShackleSwingState, unk494) == 0x494);
STATIC_ASSERT(offsetof(ShackleSwingState, lastPitch) == 0x49C);

#define DRSHACKLE_ANGLE_STEP         0xb6
#define DRSHACKLE_SWING_BLEND_LIMIT  0x41
#define DRSHACKLE_SWING_RETURN_LEFT  0x100
#define DRSHACKLE_ANGLE_RETURN_LIMIT 0x2aaa

int drshackle_updateSwingBlend(GameObject* obj, ShackleSwingState* state)
{
    ShackleSwingState* s = state;
    GameObject* o = (GameObject*)obj;
    int hitResult;
    int yawDelta;
    f32 fade;

    {
        f32 dx = o->anim.localPosX;
        f32 dz = o->anim.localPosZ;
        dx = dx - s->anchorX;
        dz = dz - s->anchorZ;
        fade = lbl_803E5B68 - sqrtf(dx * dx + dz * dz);
    }

    if (s->distanceFade != lbl_803E5AE8)
    {
        fade = fade + (((fade - lbl_803E5B10) < lbl_803E5AE8)
                           ? lbl_803E5AE8
                           : (((fade - lbl_803E5B10) > lbl_803E5B08) ? lbl_803E5B08 : (fade - lbl_803E5B10)));
    }
    if (fade < *(f32*)&lbl_803E5AE8)
    {
        fade = *(f32*)&lbl_803E5AE8;
    }

    hitResult = (*gCheckpointInterface)->advanceRoute((u8*)state, &s->collider, fade, s->colliderMode, 1, 0);

    (*gCheckpointInterface)->getRouteHeading((GameObject*)obj, &s->collider);

    (*gCheckpointInterface)->queueRouteRankItem((CheckpointRankItem*)&s->collider);

    if (hitResult != 0)
    {
        s->swingBlend = lbl_803E5AE8;
        return 0;
    }

    yawDelta = (s32)(u16)getAngle(o->anim.localPosX - s->anchorX, o->anim.localPosZ - s->anchorZ) - (s32)(u16)s->yaw;
    if (0x8000 < yawDelta)
    {
        yawDelta = yawDelta + -0xffff;
    }
    if (yawDelta < -0x8000)
    {
        yawDelta = yawDelta + 0xffff;
    }
    {
        s32 blendStep = yawDelta / DRSHACKLE_ANGLE_STEP;
        if (blendStep < -DRSHACKLE_SWING_BLEND_LIMIT)
        {
            blendStep = -DRSHACKLE_SWING_BLEND_LIMIT;
        }
        else if (blendStep > DRSHACKLE_SWING_BLEND_LIMIT)
        {
            blendStep = DRSHACKLE_SWING_BLEND_LIMIT;
        }
        s->swingBlend = (f32)(-blendStep);
    }
    s->swingCommand = 0;
    s->swingBlend = s->swingBlend / lbl_803E5B6C;

    {
        f32 blend = s->swingBlend;
        s->swingBlend = (blend < lbl_803E5B70) ? lbl_803E5B70 : ((blend > lbl_803E5AEC) ? lbl_803E5AEC : blend);
    }

    {
        f32 ang = fn_801EA678(o, (int)state);
        ang = -ang;
        if (s->lastPitch < ang || yawDelta > DRSHACKLE_ANGLE_RETURN_LIMIT || yawDelta < -DRSHACKLE_ANGLE_RETURN_LIMIT)
        {
            s->swingReturn = 0;
        }
        else if (s->lastPitch > ang)
        {
            s->swingReturn = DRSHACKLE_SWING_RETURN_LEFT;
        }
    }
    return 1;
}

int drshackle_updateAttachedPosition(GameObject* obj, ShackleSwingState* state)
{
    ShackleSwingState* s = state;
    ShackleFlags* flags;
    int mapBlockIdx;
    int hitResult;
    s16 angle;
    f32 floorOffset;

    flags = &s->flags;
    if (flags->active == 0)
    {
        return 0;
    }
    mapBlockIdx = objPosToMapBlockIdx(obj->anim.localPosX, obj->anim.localPosY, obj->anim.localPosZ);
    if (mapBlockIdx > -1)
    {
        if (flags->positionAnchored == 0)
        {
            {
                f32 zero = lbl_803E5AE8;
                s->unk494 = zero;
                s->unk498 = zero;
            }
            s->lastPitch = -fn_801EA678(obj, (int)state);
            hitResult = (*gCheckpointInterface)
                            ->advanceRoute((u8*)state, &s->collider, -s->lastPitch * timeDelta, s->colliderMode, 1, 0);
            (*gCheckpointInterface)->getRouteHeading(obj, &s->collider);
            (*gCheckpointInterface)->queueRouteRankItem((CheckpointRankItem*)&s->collider);
            if (hitResult != 0)
            {
                return 0;
            }

            SnowBike_ResetDynamics((int)obj, (int)state);
            angle = (s16)getAngle(obj->anim.localPosX - s->anchorX, obj->anim.localPosZ - s->anchorZ);
            obj->anim.rotX = angle;
            s->targetYaw = angle;
            s->yaw = angle;
            s->swingAccel = lbl_803E5B74;
            obj->anim.localPosX = s->anchorX;
            obj->anim.localPosY = s->anchorY;
            obj->anim.localPosZ = s->anchorZ;
            (*gPathControlInterface)->attachObject((void*)obj, (void*)s->attachment);
            ((ObjHitsPriorityState*)obj->anim.hitReactState)->localPosX = obj->anim.localPosX;
            ((ObjHitsPriorityState*)obj->anim.hitReactState)->localPosY = obj->anim.localPosY;
            ((ObjHitsPriorityState*)obj->anim.hitReactState)->localPosZ = obj->anim.localPosZ;
            ((ObjHitsPriorityState*)obj->anim.hitReactState)->worldPosX = obj->anim.worldPosX;
            ((ObjHitsPriorityState*)obj->anim.hitReactState)->worldPosY = obj->anim.worldPosY;
            ((ObjHitsPriorityState*)obj->anim.hitReactState)->worldPosZ = obj->anim.worldPosZ;

            if (s->floorAdjustFlag == 0)
            {
                hitDetectFn_800658a4(obj, obj->anim.localPosX, obj->anim.localPosY, obj->anim.localPosZ, &floorOffset,
                                     0);
                obj->anim.localPosY = obj->anim.localPosY - floorOffset;
                obj->anim.localPosY = obj->anim.localPosY + lbl_803E5B78;
            }
            flags->positionAnchored = 1;
            return 0;
        }
        return drshackle_updateSwingBlend(obj, state) != 0;
    }

    hitResult = (*gCheckpointInterface)
                    ->advanceRoute((u8*)state, &s->collider, timeDelta * fn_801EA678(obj, (int)state), s->colliderMode,
                                   1, 0);
    (*gCheckpointInterface)->getRouteHeading(obj, &s->collider);
    (*gCheckpointInterface)->queueRouteRankItem((CheckpointRankItem*)&s->collider);
    if (hitResult != 0)
    {
        return 0;
    }

    angle = (s16)getAngle(obj->anim.localPosX - s->anchorX, obj->anim.localPosZ - s->anchorZ);
    obj->anim.rotX = angle;
    obj->anim.localPosX = s->anchorX;
    obj->anim.localPosY = s->anchorY;
    obj->anim.localPosZ = s->anchorZ;
    (*gPathControlInterface)->attachObject((void*)obj, (void*)s->attachment);
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->localPosX = obj->anim.localPosX;
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->localPosY = obj->anim.localPosY;
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->localPosZ = obj->anim.localPosZ;
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->worldPosX = obj->anim.worldPosX;
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->worldPosY = obj->anim.worldPosY;
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->worldPosZ = obj->anim.worldPosZ;
    flags->positionAnchored = 0;
    return 0;
}

/*
 * SnowBike "Hightop" vehicle helpers.
 *
 * Implements the per-frame logic of the snowbike vehicle: route following
 * along a checkpoint path (fn_801EAE4C / gCheckpointInterface), the air /
 * fuel meter and its UI + shutdown sequence (SnowBike_UpdateAirMeter), spawn / reset
 * latching (SnowBike_onSeqFree), the animation-event/sequence callback that seeds
 * the launch impulse from per-step velocity (SnowBike_SeqFn),
 * collision response and impact particle bursts (fn_801EB634), steering /
 * pitch-roll integration with rumble + camera shake (SnowBike_UpdateSteering), and the
 * exhaust/contrail particle drivers blended toward per-state targets
 * (SnowBike_UpdateExhaustFx). State lives in SnowBikeState (dll_0255_snowbike.h); flags428 is
 * a bitfield overlay byte read via the Hightop* flag structs below.
 */

typedef struct HightopFlags3
{
    u8 hi : 4;
    u8 active : 1;
    u8 lo : 3;
} HightopFlags3;

typedef struct HightopFlags
{
    u8 resetLatch : 1;
    u8 flags : 7;
} HightopFlags;

/* particle spray spawned in a burst loop (~0x32/framesThisStep) on a bike collision */
#define DRHIGHTOP_PARTFX_COLLISION_SPRAY 0x553
#define DRHIGHTOP_HIT_VOLUME_SLOT        0x15


void fn_801EAE4C(GameObject* obj, SnowBikeState* st)
{
    f32 pathStep;
    u32 gameBitSet;
    u32 absoluteHeadingDelta;
    s16 headingDelta;
    u16 routeHeading;
    s8 routeRank;

    if ((u32)(st->flags428 >> 3 & 1) == 0)
    {
        st->routeState.startCheckpointId = -1;
        st->routeState.matchedCheckpointId = -1;
        st->routeState.currentCheckpointId = -1;
        st->routeState.linkDepth = 0;
        lbl_803DC0BC = -1;
        gameBitSet = mainGetBit(st->gameBitPtr[0]);
        if (gameBitSet != 0)
        {
            st->routeFlags.active = 1;
        }
        if ((u32)(st->flags428 >> 3 & 1) != 0)
        {
            if ((u32)(st->flags428 >> 1 & 1) != 0)
            {
                SnowBike_resetToRomListPosition(obj);
            }
            else
            {
                (*gCheckpointInterface)
                    ->findRouteForObject(obj, &st->routeState, st->routeFilter);
            }
            (*gCheckpointInterface)->rewindRoute(&st->routeState);
        }
    }
    else
    {
        if ((u32)(st->flags428 >> 1 & 1) == 0)
        {
            routeHeading = (*gCheckpointInterface)->getRouteHeading(obj, &st->routeState);
            headingDelta = obj->anim.rotX - routeHeading;
            if (0x8000 < headingDelta)
            {
                headingDelta = headingDelta - 0xffff;
            }
            if (headingDelta < -0x8000)
            {
                headingDelta = headingDelta + 0xffff;
            }
            absoluteHeadingDelta = ((int)headingDelta >= 0) ? headingDelta : -headingDelta;
            if ((int)((u32)(((int)(absoluteHeadingDelta ^ lbl_803DC0DC) >> 1) -
                             ((absoluteHeadingDelta ^ lbl_803DC0DC) & absoluteHeadingDelta)) >> 0x1f) ==
                0)
            {
                pathStep = timeDelta;
            }
            else
            {
                pathStep = -timeDelta;
            }
            st->pathProgress = st->pathProgress + pathStep;
            pathStep = st->pathProgress;
            st->pathProgress =
                (pathStep < lbl_803E5AE8) ? lbl_803E5AE8 : ((pathStep > lbl_803E5B68) ? lbl_803E5B68 : pathStep);
            if (st->pathProgress > lbl_803E5B7C)
            {
                gameTextShow(0x475);
            }
            (*gCheckpointInterface)->queueRouteRankItem(&st->rankItem);
            st->routeRank = (s8)(*gCheckpointInterface)->getRouteRank(&st->rankItem);
            routeRank = st->routeRank;
            if ((routeRank == 1) && (lbl_803DC0BC == -1))
            {
                lbl_803DC0BC = -1;
            }
            else
            {
                lbl_803DC0BC = routeRank;
                lbl_803AD088.item.linkDepth = st->routeState.linkDepth;
                lbl_803AD088.item.routeProgress = st->routeState.routeProgress;
            }
        }
        gameBitSet = mainGetBit(st->gameBitPtr[1]);
        if (gameBitSet != 0)
        {
            st->routeFlags.active = 0;
        }
    }
}

void SnowBike_UpdateAirMeter(u32 obj, int stateRaw)
{
    SnowBikeState* st = (SnowBikeState*)stateRaw;
    f32 rate;
    f32 lim;
    f32 td;

    if ((u32)(st->flags428 >> 5 & 1) != 0)
    {
        if (st->airMeterCurrent >= lbl_803E5AE8)
        {
            td = timeDelta;
            st->airMeterCurrent -= td * lbl_803DC0D8 + (f32)(s32)(st->airDrainRate * (td * PSVECMag(&st->localVelX)));
            lim = lbl_803E5AE8;
            if (lim != st->airMeterRefillTimer)
            {
                rate = 200.0f;
                st->airMeterCurrent = rate * timeDelta + st->airMeterCurrent;
                st->airMeterRefillTimer = st->airMeterRefillTimer - (f32)(s32)(rate * timeDelta);
                st->airMeterRefillTimer =
                    (st->airMeterRefillTimer < lim)
                        ? lim
                        : ((st->airMeterRefillTimer > lbl_803E5B80) ? lbl_803E5B80 : st->airMeterRefillTimer);
                st->airMeterCurrent =
                    (st->airMeterCurrent < lbl_803E5AE8)
                        ? lbl_803E5AE8
                        : ((st->airMeterCurrent > st->airMeterMax) ? st->airMeterMax : st->airMeterCurrent);
            }
            if (st->airMeterCurrent < lbl_803E5B84)
            {
                Sfx_KeepAliveLoopedObjectSound((u32)obj, SFXTRIG_ar_bomb_pickup);
            }
            (*gGameUIInterface)->runAirMeter((s32)st->airMeterCurrent);
        }
        else
        {
            Sfx_StopObjectChannel((u32)obj, 0x7f);
            if (st->velLimitX > lbl_803E5B20)
            {
                if (randomGetRange(0, 10) == 0)
                {
                    Sfx_PlayFromObject(0, SFXTRIG_dn_boar1_c_117);
                }
                PSVECScale(&st->velLimitX, &st->velLimitX, lbl_803E5B88);
                if ((u32)(st->flags428 >> 7 & 1) != 0)
                {
                    if (st->velLimitX < lbl_803E5B20)
                    {
                        st->velLimitX = lbl_803E5B20;
                    }
                }
            }
            else
            {
                (*gGameUIInterface)->airMeterSetShutdown();
                (*gObjectTriggerInterface)->runSequence(0, (void*)obj, -1);
                lim = lbl_803E5B8C;
                st->velLimitX = lbl_803E5B8C;
                st->velLimitY = lim;
                st->velLimitZ = lim;
            }
        }
    }
}

void SnowBike_onSeqFree(GameObject* obj)
{
    SnowBikeState* state = obj->extra;
    if ((u32)((state->flags428 >> 1) & 1) == 0)
    {
        s16 sv;
        f32 fz = lbl_803E5AE8;
        state->localVelX = fz;
        state->localVelY = fz;
        state->distanceScale = lbl_803E5B9C;
        ((HightopFlags*)&state->flags428)->resetLatch = 0;
        state->impactShakeTimer = fz;
        sv = obj->anim.rotX;
        state->yaw = sv;
        state->yawCurrent = sv;
        state->engineFxLevel = lbl_803E5B74;
    }
    ObjHits_EnableObject(obj);
    (*gPathControlInterface)->attachObject(obj, (char*)state + 0x178);
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->localPosX = obj->anim.localPosX;
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->localPosY = obj->anim.localPosY;
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->localPosZ = obj->anim.localPosZ;
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->worldPosX = obj->anim.worldPosX;
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->worldPosY = obj->anim.worldPosY;
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->worldPosZ = obj->anim.worldPosZ;
}

int SnowBike_SeqFn(GameObject* obj, int unused, ObjSeqState* seq)
{
    typedef struct HightopMatrixSeed
    {
        s16 rotX;
        s16 rotY;
        s16 rotZ;
        s16 pad;
        f32 unused;
        f32 x;
        f32 y;
        f32 z;
    } HightopMatrixSeed;

    u8 triggerType;
    int i;
    int state;
    SnowBikeState* st;
    f32 matrix[16];
    HightopMatrixSeed transform;
    f64 xSpeed;
    f64 ySpeed;
    f64 zSpeed;

    state = *(int*)&obj->extra;
    st = (SnowBikeState*)state;
    seq->freeCallback = (ObjAnimSequenceFreeCallback)SnowBike_onSeqFree;
    ObjHits_DisableObject(obj);

    for (i = 0; i < (int)(u32)seq->eventCount; i++)
    {
        triggerType = seq->eventIds[i];
        switch (triggerType)
        {
        case 2:
            if (obj->anim.seqId != 0x16c && obj->anim.seqId != 0x16f)
            {
                mainSetBits(0x499, 1);
            }
            break;
        case 3:
            (*gGameUIInterface)->airMeterSetShutdown();
            break;
        }
    }

    if (st->riderMode == 2)
    {
        xSpeed = (double)(float)(oneOverTimeDelta * (obj->anim.localPosX - st->refPosX));
        ySpeed = (double)(float)(oneOverTimeDelta * (obj->anim.localPosY - st->refPosY));
        zSpeed = (double)(float)(oneOverTimeDelta * (obj->anim.localPosZ - st->refPosZ));

        transform.x = lbl_803E5AE8;
        transform.y = lbl_803E5AE8;
        transform.z = lbl_803E5AE8;
        transform.unused = lbl_803E5AEC;
        transform.rotX = -obj->anim.rotX;
        transform.rotY = 0;
        transform.rotZ = 0;
        mtxRotateByVec3s(matrix, &transform);
        Matrix_TransformPoint(matrix, xSpeed, ySpeed, zSpeed, (float*)(state + 0x494), (float*)(state + 0x498),
                              (float*)(state + 0x49c));

        st->stickY = st->stickY + (framesThisStep << 3);
        if (st->stickY > 0x46)
        {
            st->stickY = 0x46;
        }

        drcloudcage_updateEngineFx((GameObject*)obj, (void*)state, st->distanceScale,
                                   (int)(lbl_803E5BA0 * -st->engineFxLevel), (u8*)(state + 0x461), 4);
    }

    ((HightopFlags3*)&st->flags428)->active = 0;
    return 0;
}

void fn_801EB634(GameObject* obj, int stateRaw)
{
    SnowBikeState* st = (SnowBikeState*)stateRaw;
    int hitKind;
    int hitReact;
    int burstCount;
    u32 hit;
    f32 dot;
    int hitOutB;
    u32 hitOutC;
    int hitObj;
    f32 velNrm[3];

    hitReact = *(int*)&obj->anim.hitReactState;
    if (ObjHits_IsObjectEnabled((ObjAnimComponent*)obj) != 0)
    {
        if ((u32)(st->flags428 >> 1 & 1) == 0)
        {
            ObjHits_SetHitVolumeSlot((ObjAnimComponent*)obj, DRHIGHTOP_HIT_VOLUME_SLOT, 1, 0);
        }
        else
        {
            ObjHits_ClearHitVolumes((ObjAnimComponent*)obj);
            ObjHits_SyncObjectPositionIfDirty(obj);
        }
        hitKind = ObjHits_GetPriorityHit(obj, &hitObj, &hitOutB, &hitOutC);
        switch (hitKind)
        {
        case 0xd:
            if ((u32)(st->flags428 >> 1 & 1) == 0)
            {
                st->linkedObj = hitObj;
                st->collisionFxDamping = lbl_803E5AEC;
            }
            break;
        case 0x15:
            if (st->collisionFxTimer == lbl_803E5AE8)
            {
                PSVECNormalize((float*)&obj->anim.velocityX, velNrm);
                dot = PSVECDotProduct(velNrm, (float*)(hitObj + 0x24));
                PSVECScale(&st->localVelX, &st->localVelX, dot * st->collisionBounceScale + lbl_803E5AEC);
                st->localVelY = st->localVelY * lbl_803E5BA8;
                st->collisionFxTimer = lbl_803E5AF4;
                st->collisionFxDamping = lbl_803E5AEC;
            }
            break;
        case 0x1d:
            if ((u32)(st->flags428 >> 1 & 1) == 0)
            {
                setMotionBlur(1, lbl_803E5BAC);
                st->collisionFxTimer = (f32)(s32)lbl_803DC0D0;
                st->collisionFxDamping = lbl_803DC0C8;
                st->airMeterRefillTimer = (f32)(s32)lbl_803DC0CC;
            }
            break;
        }
        hit = *(u32*)(hitReact + 0x50);
        if (((hit != 0) && (hitObj = hit, *(u32*)& st->linkedObj = hit, st->collisionFxTimer == lbl_803E5AE8)) &&
            (hitKind = arrayIndexOf(gDrHighTopHitObjectKinds, 0xc, (int)*(short*)(hitObj + 0x46)), hitKind != -1))
        {
            fn_8009A8C8((GameObject*)obj, lbl_803E5BB0);
            (*gPartfxInterface)->spawnObject((void*)obj, 0x551, NULL, 4, -1, NULL);
            (*gPartfxInterface)->spawnObject((void*)obj, 0x552, NULL, 4, -1, NULL);
            (*gPartfxInterface)->spawnObject((void*)obj, 0x554, NULL, 4, -1, NULL);
            burstCount = 0x32 / framesThisStep;
            while (burstCount-- != 0)
            {
                (*gPartfxInterface)->spawnObject((void*)obj, DRHIGHTOP_PARTFX_COLLISION_SPRAY, NULL, 2, -1, NULL);
            }
            st->collisionFxTimer = lbl_803E5AF4;
            st->collisionFxDamping = lbl_803E5AEC;
            if ((u32)(st->flags428 >> 1 & 1) == 0)
            {
                st->collisionFxTimer = (f32)(s32)lbl_803DC0D4;
            }
        }
    }
}

void SnowBike_UpdateSteering(short* obj, int stateRaw)
{
    SnowBikeState* st = (SnowBikeState*)stateRaw;
    void* pathState = (void*)(stateRaw + 0x178);
    f32 fa;
    f32 fb;
    int rotClamped;
    int yawDelta;
    int ival;

    (*gPathControlInterface)->update(obj, pathState, timeDelta);
    (*gPathControlInterface)->apply(obj, pathState);
    (*gPathControlInterface)->advance(obj, pathState, timeDelta);
    ival = 2;
    if (st->unk3D9 == '\0')
    {
        st->impactShakeTimer = st->impactShakeTimer + timeDelta;
        fa = st->impactShakeTimer;
        st->impactShakeTimer = (fa < lbl_803E5AE8) ? lbl_803E5AE8 : ((fa > lbl_803E5BB4) ? lbl_803E5BB4 : fa);
        if (st->impactShakeTimer >= lbl_803E5BB8)
        {
            if ((u32)(st->flags428 >> 7 & 1) == 0)
            {
                st->unk584 = lbl_803E5AE8;
            }
            ((HightopFlags*)&st->flags428)->resetLatch = 1;
        }
    }
    else
    {
        if ((u32)(st->flags428 >> 7 & 1) != 0)
        {
            ival = 0;
            fa = lbl_803E5BBC;
            st->haloYawDrift = fa * (f32)(s32)obj[1];
            st->haloDriftAmpB = fa * (f32)(s32)obj[2];
            st->haloDriftPhaseA = ival;
            st->haloDriftPhaseB = ival;
            if ((u32)(st->flags428 >> 1 & 1) == 0)
            {
                doRumble(st->impactShakeTimer * fa);
                Camera_EnableViewYOffset();
                CameraShake_SetAllMagnitudes(st->impactShakeTimer / lbl_803E5BC0);
                Sfx_PlayFromObject((u32)obj, SFXTRIG_tr_jbike_bombbeep);
                fb = (lbl_803E5B40 < lbl_803E5BC4 * st->impactShakeTimer) ? lbl_803E5B40
                                                                          : lbl_803E5BC4 * st->impactShakeTimer;
                {
                    Sfx_SetObjectSfxVolume((u32)obj, SFXTRIG_tr_jbike_bombbeep, fb, lbl_803E5B20);
                }
            }
        }
        ((HightopFlags*)&st->flags428)->resetLatch = 0;
        st->impactShakeTimer = lbl_803E5AE8;
        st->dampPresetMode = st->dampPresetModeRaw;
    }
    fa = lbl_803E5BC8;
    st->haloDriftPhaseA = fa * timeDelta + (f32)(s32)st->haloDriftPhaseA;
    st->haloDriftPhaseB = fa * timeDelta + (f32)(s32)st->haloDriftPhaseB;
    st->haloYawDrift = st->haloYawDrift * powfBitEstimate(lbl_803E5BCC, timeDelta);
    st->haloDriftAmpB = st->haloDriftAmpB * powfBitEstimate(lbl_803E5BCC, timeDelta);
    st->haloPitchDrift = st->haloYawDrift * mathSinf((gDrHighTopPi * (f32)(s32)st->haloDriftPhaseA) / lbl_803E5BD4);
    st->haloDriftB = st->haloDriftAmpB * mathSinf((gDrHighTopPi * (f32)(s32)st->haloDriftPhaseB) / lbl_803E5BD4);
    yawDelta = (int)*obj - ((int)st->yaw & 0xffffU);
    if (0x8000 < yawDelta)
    {
        yawDelta = yawDelta + -0xffff;
    }
    if (yawDelta < -0x8000)
    {
        yawDelta = yawDelta + 0xffff;
    }
    st->yaw = *(s16*)((char*)st + 0x40e) + yawDelta;
    st->yawCurrent = st->yawCurrent + yawDelta;
    obj[1] = obj[1] + ((int)st->unk310 >> ival);
    obj[2] = obj[2] + ((int)st->unk312 >> ival);
    rotClamped = obj[1];
    if (rotClamped < -0x2000)
    {
        rotClamped = -0x2000;
    }
    else if (0x2000 < rotClamped)
    {
        rotClamped = 0x2000;
    }
    obj[1] = rotClamped;
    rotClamped = obj[2];
    if (rotClamped < -0x2000)
    {
        rotClamped = -0x2000;
    }
    else if (0x2000 < rotClamped)
    {
        rotClamped = 0x2000;
    }
    obj[2] = rotClamped;
}

void SnowBike_UpdateExhaustFx(GameObject* obj, int stateRaw)
{
    typedef struct HightopPartfxTransform
    {
        s16 rotX;
        s16 rotY;
        s16 rotZ;
        s16 pad;
        f32 scale;
        f32 x;
        f32 y;
        f32 z;
    } HightopPartfxTransform;

    SnowBikeState* st = (SnowBikeState*)stateRaw;
    u8 flags;
    s16 motionFrame;
    f32 fa;
    f32 fb;
    f32 speed;
    f32 target558;
    f32 target530;
    f32 target534;
    f32 target548;
    f32 target54c;
    f32 target540;
    f32 target544;
    f32 k;
    HightopPartfxTransform effect;

    speed =
        sqrtf(st->distanceScale * st->distanceScale + (st->localVelX * st->localVelX + st->localVelY * st->localVelY));
    st->timer -= timeDelta;
    fa = st->timer;
    st->timer = (fa < lbl_803E5AE8) ? lbl_803E5AE8 : ((fa > lbl_803E5B1C) ? lbl_803E5B1C : fa);

    flags = st->flags428;
    if ((u32)(flags >> 7 & 1) == 0)
    {
        switch (st->dampPresetMode)
        {
        case 0xd:
            target558 = lbl_803E5BD8;
            target534 = lbl_803E5BDC;
            target530 = lbl_803E5B88;
            target548 = lbl_803E5BE0;
            target54c = lbl_803E5BE4;
            target540 = lbl_803E5BE8;
            target544 = lbl_803E5AF8;
            if (((u32)(flags >> 1 & 1) == 0) && (st->timer <= lbl_803E5AE8))
            {
                st->timer = (f32)(s32)randomGetRange(5, 10);
                if (PSVECMag((void*)&obj->anim.velocityX) > lbl_803E5BC4)
                {
                    doRumble((f32)(s32)randomGetRange(1, 3));
                }
            }
            if (speed > lbl_803E5BEC)
            {
                (*gPartfxInterface)->spawnObject((void*)obj, 0x80b, NULL, 2, -1, NULL);
            }
            break;
        case 3:
        default:
            target558 = lbl_803E5BF0;
            target534 = lbl_803E5BF4;
            target530 = lbl_803E5BF8;
            target548 = lbl_803E5BFC;
            target54c = lbl_803E5BE4;
            target540 = lbl_803E5BE8;
            target544 = lbl_803E5AF8;
            break;
        case 9:
            target558 = lbl_803E5BEC;
            target534 = lbl_803E5BF4;
            target530 = lbl_803E5C00;
            target548 = lbl_803E5C04;
            target54c = lbl_803E5C08;
            target540 = lbl_803E5B20;
            target544 = lbl_803E5C0C;
            if (speed > lbl_803E5B34)
            {
                effect.scale = lbl_803E5AEC;
                effect.rotZ = 0;
                effect.rotY = 0;
                effect.rotX = 0;
                effect.x = obj->anim.localPosX;
                effect.y = lbl_803E5C10 + obj->anim.localPosY;
                effect.z = obj->anim.localPosZ;
                (*gPartfxInterface)->spawnObject((void*)obj, 0x80a, &effect, 1, -1, NULL);
            }
            break;
        }

        motionFrame = st->steerAngleDeg;
        if (((motionFrame >= 0x1e) && (motionFrame <= 0x3c)) || ((motionFrame >= 0x12c) && (motionFrame <= 0x14a)))
        {
            target558 *= lbl_803E5B20;
            target534 *= lbl_803E5B2C;
            target530 += lbl_803E5B20;
            if (target530 < lbl_803E5AE8)
            {
                target530 = lbl_803E5AE8;
            }
            else if (target530 > lbl_803E5B88)
            {
                target530 = lbl_803E5B88;
            }
        }
    }
    else
    {
        target558 = st->unk578;
        target534 = st->unk574;
        target530 = st->unk56C;
        target548 = st->localVelXDampTarget;
        target54c = st->distanceScaleDampTarget;
        target540 = lbl_803E5B20;
        target544 = lbl_803E5AF8;
    }

    if ((u32)((st->flags428 >> 1) & 1) != 0)
    {
        target558 = lbl_803E5AF8;
    }
    fb = timeDelta;
    speed = lbl_803E5C14;
    st->unk558 +=
        fb * (speed *
              (((target558 < lbl_803E5BD8) ? lbl_803E5BD8 : ((target558 > lbl_803E5AEC) ? lbl_803E5AEC : target558)) -
               st->unk558));
    st->unk534 += timeDelta * (lbl_803E5BBC * (target534 - st->unk534));
    st->unk530 += timeDelta * (lbl_803E5C14 * (target530 - st->unk530));
    st->localVelXDamp += timeDelta * ((k = lbl_803E5B20) * (target548 - st->localVelXDamp));
    st->distanceScaleDamp += timeDelta * (k * (target54c - st->distanceScaleDamp));
    st->turnVelScale += timeDelta * (k * (target540 - st->turnVelScale));
    st->turnForceGain += timeDelta * (k * (target544 - st->turnForceGain));
}

void SnowBike_UpdateLiftSway(int obj, int state)
{
    PickupFlags* flags;
    int origBit4;
    f32 rate;
    f32 target;
    f32 clampedRate;
    f32 out[3];
    f32 vec_args[4];

    flags = (PickupFlags*)&((DRPickupState*)state)->flags;
    origBit4 = flags->b4;

    if ((((DRPickupState*)state)->flags458 & 0x100) != 0)
    {
        flags->b6 = 1;
    }
    else
    {
        flags->b6 = 0;
    }

    if ((((DRPickupState*)state)->flags458 & 0x200) != 0)
    {
        flags->b4 = 1;
    }
    else
    {
        flags->b4 = 0;
    }

    if ((origBit4 == 0) && (flags->b4 != 0))
    {
        Sfx_PlayFromObject(obj, SFXTRIG_bblast16);
    }

    target = lbl_803E5AE8;
    if (flags->b6 != 0)
    {
        target = ((DRPickupState*)state)->liftZVelTarget;
    }
    rate = (target - ((DRPickupState*)state)->liftZVel) * lbl_803E5C28;
    clampedRate = (rate < lbl_803E5C2C) ? lbl_803E5C2C : ((rate > lbl_803E5B8C) ? lbl_803E5B8C : rate);
    *(f32*)(state + 0x430) = clampedRate * timeDelta + *(f32*)((int)state + 0x430);

    target = lbl_803E5AE8;
    if (flags->b4 != 0)
    {
        f32 vy53c = ((DRPickupState*)state)->settleVelMax;
        f32 v49c = ((DRPickupState*)state)->accumZ;
        if (v49c >= target)
        {
            f32 nv = -vy53c;
            target = (nv < -v49c * oneOverTimeDelta) ? -v49c * oneOverTimeDelta : ((nv > target) ? target : nv);
        }
        else
        {
            target =
                (vy53c < target) ? target : ((vy53c > -v49c * oneOverTimeDelta) ? -v49c * oneOverTimeDelta : vy53c);
        }
    }
    {
        f32 fz = *(f32*)&lbl_803E5AE8;
        ((DRPickupState*)state)->localOffsetX = fz;
        ((DRPickupState*)state)->localOffsetY = fz;
    }
    ((DRPickupState*)state)->localOffsetZ = (*(f32*)(state + 0x430) + target) * timeDelta;

    Matrix_TransformPoint((f32*)(state + 0x6c), ((DRPickupState*)state)->localOffsetX,
                          ((DRPickupState*)state)->localOffsetY, ((DRPickupState*)state)->localOffsetZ, &out[0],
                          &out[1], &out[2]);
    Matrix_TransformPoint((f32*)(state + 0x12c), out[0], out[1], out[2], &out[0], &out[1], &out[2]);
    PSVECAdd(out, (void*)(state + 0x494), (void*)(state + 0x494));

    ((DRPickupState*)state)->angVel414 =
        (-((DRPickupState*)state)->angAccelGain * ((DRPickupState*)state)->angAccelScale) * timeDelta +
        ((DRPickupState*)state)->angVel414;
    ((DRPickupState*)state)->angVel414 =
        powfBitEstimate(((DRPickupState*)state)->angVelDamping, timeDelta) * ((DRPickupState*)state)->angVel414;

    {
        f32 lim;
        f32 v;
        v = ((DRPickupState*)state)->angVel414;
        lim = ((DRPickupState*)state)->angVelLimit;
        ((DRPickupState*)state)->angVel414 = (v < -lim) ? -lim : ((v > lim) ? lim : v);
    }

    {
        f32 newF = (f32)(s32) * (s16*)(state + 0x40e) + ((DRPickupState*)state)->angVel414 * timeDelta;
        s32 delta;
        ((DRPickupState*)state)->angle40E = newF;
        delta = (s32)(((DRPickupState*)state)->angVel414 * ((DRPickupState*)state)->angleScale);
        delta -= (s32)(u16) * (u32*)(state + 0x410);
        if (delta > 0x8000)
        {
            delta = delta - 0xFFFF;
        }
        if (delta < -0x8000)
        {
            delta = delta + 0xFFFF;
        }
        ((DRPickupState*)state)->angAccum410 =
            (u32)(s32)((f32)delta * ((DRPickupState*)state)->angAccumGain + (f32)(s32) * (u32*)((int)state + 0x410));
    }
    {
        s32 delta = (s32) * (s16*)(state + 0x40e) - (s32)(u16) * (s16*)(state + 0x40c);
        if (delta > 0x8000)
        {
            delta = delta - 0xFFFF;
        }
        if (delta < -0x8000)
        {
            delta = delta + 0xFFFF;
        }
        ((DRPickupState*)state)->angle40C =
            (s16)((f32)delta * ((DRPickupState*)state)->angleGain + (f32)(s32) * (s16*)((int)state + 0x40c));
    }

    if (flags->b7 != 0)
    {
        ((DRPickupState*)state)->spinVel =
            (-((DRPickupState*)state)->spinDecel) * timeDelta + ((DRPickupState*)state)->spinVel;
        {
            f32 v = ((DRPickupState*)state)->spinVel;
            ((DRPickupState*)state)->spinVel =
                (v < lbl_803E5C30) ? lbl_803E5C30 : ((v > lbl_803E5B48) ? lbl_803E5B48 : v);
        }
        *(s16*)(obj + 0x2) = (f32)(s32) * (s16*)(obj + 0x2) + ((DRPickupState*)state)->spinVel * timeDelta;
    }

    if (flags->b1 == 0)
    {
        vec_args[0] = ((DRPickupState*)state)->angVel414;
        vec_args[1] = ((DRPickupState*)state)->accumZ;
        vec_args[2] = (f32)(s32) * (s16*)(obj + 0x4);
        vec_args[3] = (f32)(s32) * (s16*)(obj + 0x2);
        (*gCameraInterface)->releaseAction(vec_args, 0x10);
    }

    {
        f32 lim;
        f32 v;
        v = ((DRPickupState*)state)->accumX;
        lim = ((DRPickupState*)state)->clampLimitX;
        ((DRPickupState*)state)->accumX = (v < -lim) ? -lim : ((v > lim) ? lim : v);
        v = ((DRPickupState*)state)->accumX;
        if (v < lbl_803E5B8C)
        {
            if (v > lbl_803E5BA4)
            {
                ((DRPickupState*)state)->accumX = lbl_803E5AE8;
            }
        }
    }

    {
        f32 v = ((DRPickupState*)state)->accumY;
        f32 lim = -((DRPickupState*)state)->clampLimitY;
        ((DRPickupState*)state)->accumY = (v < lim) ? lim : ((v > lbl_803E5AEC) ? lbl_803E5AEC : v);
        v = ((DRPickupState*)state)->accumY;
        if (v < lbl_803E5B8C)
        {
            if (v > lbl_803E5BA4)
            {
                ((DRPickupState*)state)->accumY = lbl_803E5AE8;
            }
        }
    }

    {
        f32 lim;
        f32 v;
        v = ((DRPickupState*)state)->accumZ;
        lim = ((DRPickupState*)state)->clampLimitZ;
        ((DRPickupState*)state)->accumZ = (v < -lim) ? -lim : ((v > lim) ? lim : v);
        v = ((DRPickupState*)state)->accumZ;
        if (v < lbl_803E5B8C)
        {
            if (v > lbl_803E5BA4)
            {
                ((DRPickupState*)state)->accumZ = lbl_803E5AE8;
            }
        }
    }
}


#define SNOWBIKE_OBJGROUP           0xa
#define SNOWBIKE_AIRMETER_BGTEXTURE 0x5cd

/* SnowBike defNos (anim.seqId), names from retail OBJECTS.bin at def+0x91; all gate
   to this DLL. The three CRSnowClawB / two IMSnowClawB entries share
   one truncated bin name apiece and are told apart by the bikeVariant ordinal that
   SnowBike_init assigns them below. IM = Ice Mountain, CR = CloudRunner Fortress. */
#define SNOWBIKE_IM_BIKE_OBJ           0x72
#define SNOWBIKE_IM_CLAWBIKE_V0_OBJ    0x16c
#define SNOWBIKE_IM_CLAWBIKE_V1_OBJ    0x16f
#define SNOWBIKE_CR_BIKE_OBJ           0x38c
#define SNOWBIKE_CR_CLAWBIKE_V0_OBJ    0x38d
#define SNOWBIKE_CR_CLAWBIKE_V1_OBJ    0x38e
#define SNOWBIKE_CR_CLAWBIKE_V2_OBJ    0x4d4


typedef struct SnowBikeSetTypeState
{
    s16 savedRotX;
    u8 pad2[0xC - 0x2];
    f32 savedPosX;
    f32 savedPosY;
    f32 savedPosZ;
    u8 pad18[0x3D3 - 0x18];
    s8 unk3D3;
    u8 pad3D4[0x3E8 - 0x3D4];
    f32 modelMtxPosX;
    f32 modelMtxPosY;
    f32 modelMtxPosZ;
    u8 pad3F4[0x400 - 0x3F4];
    f32 mountPosX;
    f32 mountPosY;
    f32 mountPosZ;
    u8 pad40C[0x414 - 0x40C];
    f32 unk414;
    u8 pad418[0x420 - 0x418];
    u8 unk420;
    s8 bikeType;
    u8 pad422[0x428 - 0x422];
    u8 flags;
    u8 pad429[0x434 - 0x429];
    u8 romListGroupIndex;
    u8 romListItemIndex;
    u8 pad436[0x448 - 0x436];
    s16 completionGameBit;
    u8 pad44A[0x494 - 0x44A];
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    u8 pad4A0[0x4B8 - 0x4A0];
    f32 airMeterMax;     /* 0x4B8 */
    f32 airMeterCurrent; /* 0x4BC */
    f32 airDrainRate;    /* 0x4C0 */
    u8 pad4C4[0x4C8 - 0x4C4];
} SnowBikeSetTypeState;

typedef struct DRcradleSnowBikeFlags
{
    u8 resetLatch : 1;   /* 0x80 */
    u8 pathActive : 1;   /* 0x40 */
    u8 uiPrompt : 1;     /* 0x20 */
    u8 impulseLatch : 1; /* 0x10 */
    u8 flags : 4;
} DRcradleSnowBikeFlags;

void SnowBike_buildOrientationMatrices(GameObject* obj, int state)
{
    MatrixTransform v;
    SnowBikeState* s = (SnowBikeState*)state;

    v.x = 0.0f;
    v.y = 0.0f;
    v.z = 0.0f;
    v.scale = lbl_803E5AEC;

    v.rotX = s->yaw;
    v.rotY = 0;
    v.rotZ = 0;
    setMatrixFromObjectPos((f32*)(state + 0x6c), &v);

    v.rotX = -s->yaw;
    v.rotY = 0;
    v.rotZ = 0;
    mtxRotateByVec3s((f32*)(state + 0xac), &v);

    v.rotX = s->yawCurrent;
    v.rotY = 0;
    v.rotZ = 0;
    setMatrixFromObjectPos((f32*)(state + 0xec), &v);

    v.rotX = -s->yawCurrent;
    v.rotY = 0;
    v.rotZ = 0;
    mtxRotateByVec3s((f32*)(state + 0x12c), &v);
}

void SnowBike_ResetDynamics(int obj, register int state)
{
    f32 fz, fa, fb, fc;
    DRcradleSnowBikeFlags* flags;
    SnowBikeState* s = (SnowBikeState*)state;
    s->unk52C = lbl_803E5C34;
    s->unk530 = lbl_803E5C38;
    s->unk534 = lbl_803E5BF4;
    fz = 0.0f;
    ((SnowBikeSetTypeState*)state)->unk414 = fz;
    s->unk584 = fz;
    s->localVelXDamp = lbl_803E5BFC;
    s->distanceScaleDamp = lbl_803E5BE4;
    s->turnVelScale = lbl_803E5B20;
    s->turnForceGain = lbl_803E5AF8;
    s->unk558 = lbl_803E5BA8;
    s->unk56C = lbl_803E5C00;
    flags = (DRcradleSnowBikeFlags*)(state + 0x428);
    flags->resetLatch = 0;
    s->engineFxLevel = fz;
    fa = s->baseVelLimitX;
    s->velLimitX = fa;
    s->localVelXLimit = fa;
    fb = s->baseVelLimitY;
    s->velLimitY = fb;
    s->localVelYLimit = fb;
    fc = s->baseVelLimitZ;
    s->velLimitZ = fc;
    s->distanceScaleLimit = fc;
    flags->pathActive = 0;
    flags->impulseLatch = 0;
    *(u32*)(state + 0x42c) = 0;
    s->collisionFxTimer = fz;
    s->collisionFxDamping = lbl_803E5AEC;
}

void SnowBike_InitTuning(GameObject* obj, int state)
{
    f32 fa, fz;
    SnowBikeState* s = (SnowBikeState*)state;
    s->liftAccel = lbl_803E5C3C;
    s->unk530 = lbl_803E5C38;
    s->unk534 = lbl_803E5BF4;
    s->unk538 = lbl_803E5B74;
    s->unk53C = lbl_803E5C14;
    s->localVelXDamp = lbl_803E5BFC;
    s->distanceScaleDamp = lbl_803E5BE4;
    s->turnVelScale = lbl_803E5B20;
    s->turnForceGain = lbl_803E5AF8;
    fa = lbl_803E5C40;
    s->localVelXDampTarget = fa;
    s->distanceScaleDampTarget = fa;
    s->unk554 = lbl_803E5C44;
    s->unk550 = lbl_803E5C10;
    s->unk570 = lbl_803E5BB8;
    fz = lbl_803E5BA8;
    s->unk558 = fz;
    s->unk578 = lbl_803E5B8C;
    s->unk574 = lbl_803E5BB0;
    s->unk56C = lbl_803E5C00;
    s->collisionBounceScale = fz;
}
typedef struct SnowBikeMountState
{
    s16 savedRotX;
    u8 pad2[0xC - 0x2];
    f32 savedPosX;
    f32 savedPosY;
    f32 savedPosZ;
    u8 pad18[0x3D3 - 0x18];
    s8 unk3D3;
    u8 pad3D4[0x3E8 - 0x3D4];
    f32 modelMtxPosX;
    f32 modelMtxPosY;
    f32 modelMtxPosZ;
    u8 pad3F4[0x400 - 0x3F4];
    f32 mountPosX;
    f32 mountPosY;
    f32 mountPosZ;
    u8 pad40C[0x414 - 0x40C];
    f32 unk414;
    u8 pad418[0x420 - 0x418];
    u8 unk420;
    u8 pad421[0x428 - 0x421];
    u8 flags;
    u8 pad429[0x434 - 0x429];
    u8 romListGroupIndex;
    u8 romListItemIndex;
    u8 pad436[0x494 - 0x436];
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
} SnowBikeMountState;


/* texture asset loaded into lbl_803DDC60 (this DLL's only texture) */
#define SNOWBIKE_TEXTURE_ID 0x186
typedef struct SnowBikeRomListItem
{
    ObjPlacement base;
    u8 pad18[0x29 - 0x18];
    u8 yawByte;
} SnowBikeRomListItem;
typedef struct
{
    u8 pad0 : 2;
    u8 b20 : 1;
    u8 pad1 : 2;
    u8 b04 : 1;
    u8 b02 : 1;
    u8 b01 : 1;
} HaloSnowBikeFlags;
typedef struct
{
    u8 pad0 : 2;
    u8 b20 : 1;
    u8 pad1 : 2;
    u8 b04 : 1;
    u8 b02 : 1;
    u8 b01 : 1;
} SnowBikeFlags;
typedef struct SnowBikePlacement
{
    ObjPlacement base;
    u8 yawByte;
    u8 startFlag;
    s16 unk1a;
    u8 param1c;
    u8 param1d;
    s16 gameBitId;
    u8 pad20[0x24 - 0x20];
} SnowBikePlacement;

typedef struct
{
    s16 rot[3];
    f32 quad[4];
} SBRotQuad;

s32 SnowBike_getRouteRank(GameObject* obj)
{
    return (*gCheckpointInterface)->getRouteRank((CheckpointRankItem*)(*(int*)&obj->extra + 0x28));
}

s32 SnowBike_isAtRankGate(GameObject* obj)
{
    int result = (*gCheckpointInterface)->getRouteRank((CheckpointRankItem*)(*(int*)&obj->extra + 0x28));
    if (result == 3)
    {
        if (lbl_803DC0BC == -1)
        {
            return 1;
        }
    }
    return (u32)__cntlzw(lbl_803DC0BC - 1 - result) >> 5;
}

void SnowBike_func17(void)
{
}

void SnowBike_func16(void)
{
}

void SnowBike_resetToRomListPosition(GameObject* obj)
{
    int state = *(int*)&obj->extra;
    int* table;
    SnowBikeRomListItem* found;
    f32 zero;

    table = (int*)((int)gSnowBikeMountRomListTable + (int)(((SnowBikeMountState*)state)->romListGroupIndex) * 12);
    found = (SnowBikeRomListItem*)mapRomListFindItem(table[((SnowBikeMountState*)state)->romListItemIndex], 0, 0, 0, 0);
    if (found != NULL)
    {
        if (((SnowBikeMountState*)state)->romListGroupIndex != 0)
        {
            obj->anim.localPosX = found->base.posX;
            obj->anim.localPosY = found->base.posY;
            obj->anim.localPosZ = found->base.posZ;
            obj->anim.rotX = (s16)((found->yawByte) << 8);
        }
        (*gCheckpointInterface)->findRouteForObject(obj, (CheckpointRouteState*)(state + 0x28), 0);
        ((SnowBikeMountState*)state)->savedPosX = obj->anim.localPosX;
        ((SnowBikeMountState*)state)->savedPosY = obj->anim.localPosY;
        ((SnowBikeMountState*)state)->savedPosZ = obj->anim.localPosZ;
        ((SnowBikeMountState*)state)->savedRotX = obj->anim.rotX;
        zero = 0.0f;
        ((SnowBikeMountState*)state)->velocityX = zero;
        ((SnowBikeMountState*)state)->velocityY = zero;
        ((SnowBikeMountState*)state)->velocityZ = zero;
        (*gPathControlInterface)->attachObject((void*)obj, (void*)&((ShackleSwingState*)state)->attachment);
        ((ObjHitsPriorityState*)obj->anim.hitReactState)->localPosX = obj->anim.localPosX;
        ((ObjHitsPriorityState*)obj->anim.hitReactState)->localPosY = obj->anim.localPosY;
        ((ObjHitsPriorityState*)obj->anim.hitReactState)->localPosZ = obj->anim.localPosZ;
        ((ObjHitsPriorityState*)obj->anim.hitReactState)->worldPosX = obj->anim.worldPosX;
        ((ObjHitsPriorityState*)obj->anim.hitReactState)->worldPosY = obj->anim.worldPosY;
        ((ObjHitsPriorityState*)obj->anim.hitReactState)->worldPosZ = obj->anim.worldPosZ;
        ((SnowBikeMountState*)state)->unk3D3 = 1;
    }
}

s32 SnowBike_func14(GameObject* obj)
{
    return ((SnowBikeState*)obj->extra)->routeRank;
}

f32 SnowBike_func13(GameObject* obj, f32* out)
{
    int state = *(int*)&obj->extra;
    f32 speed;
    *out = lbl_803E5BB8;
    speed = sqrtf(((SnowBikeMountState*)state)->velocityZ * ((SnowBikeMountState*)state)->velocityZ +
                  (((SnowBikeMountState*)state)->velocityX * ((SnowBikeMountState*)state)->velocityX +
                   ((SnowBikeMountState*)state)->velocityY * ((SnowBikeMountState*)state)->velocityY));
    speed = speed * lbl_803E5BA8;
    if (speed > lbl_803E5AEC)
    {
        speed = lbl_803E5AEC;
    }
    return speed;
}

void SnowBike_func12(GameObject* obj, f32* outFloat, s32* outBool)
{
    int state = *(int*)&obj->extra;
    f32 value, r;
    *outFloat = ((SnowBikeMountState*)state)->unk414 / lbl_803E5C48;
    value = *outFloat;
    *outFloat = (value < lbl_803E5B70) ? lbl_803E5B70 : ((value > lbl_803E5AEC) ? lbl_803E5AEC : value);
    *outBool = ((SnowBikeMountState*)state)->unk414 < 0.0f;
}

void SnowBike_setRiderMode(GameObject* obj, int type)
{
    int state = *(int*)&obj->extra;
    u32 bit;
    ((SnowBikeSetTypeState*)state)->bikeType = type;
    if (type == 2)
    {
        mainSetBits(((SnowBikeSetTypeState*)state)->completionGameBit, 1);
        SnowBike_ResetDynamics((int)obj, state);
        bit = (((SnowBikeSetTypeState*)state)->flags >> 5) & 1;
        if (bit != 0)
        {
            ((SnowBikeSetTypeState*)state)->airMeterMax = lbl_803E5B90;
            ((SnowBikeSetTypeState*)state)->airDrainRate = lbl_803E5AEC;
            ((SnowBikeSetTypeState*)state)->airMeterCurrent = lbl_803E5B94;
            if (((SnowBikeSetTypeState*)state)->bikeType == 2)
            {
                (*gGameUIInterface)
                    ->initAirMeter((int)((SnowBikeSetTypeState*)state)->airMeterMax, SNOWBIKE_AIRMETER_BGTEXTURE);
                (*gGameUIInterface)->airMeterSetRatio(lbl_803E5B98);
            }
        }
        if (obj->anim.seqId == SNOWBIKE_IM_BIKE_OBJ)
        {
            ((ObjHitsPriorityState*)obj->anim.hitReactState)->lateralResponseWeight = 0x14;
            ((ObjHitsPriorityState*)obj->anim.hitReactState)->axialResponseWeight = 0x14;
        }
    }
}

s32 SnowBike_getRiderMode(GameObject* obj)
{
    return ((SnowBikeState*)obj->extra)->riderMode;
}

void SnowBike_mount(GameObject* obj, f32* x, f32* y, f32* z)
{
    int state = *(int*)&obj->extra;
    ((SnowBikeMountState*)state)->mountPosX = obj->anim.localPosX;
    ((SnowBikeMountState*)state)->mountPosY = obj->anim.localPosY;
    ((SnowBikeMountState*)state)->mountPosZ = obj->anim.localPosZ;
    *x = ((SnowBikeMountState*)state)->mountPosX;
    *y = ((SnowBikeMountState*)state)->mountPosY;
    *z = ((SnowBikeMountState*)state)->mountPosZ;
}

int SnowBike_func0E(void)
{
    return 0x2;
}

int SnowBike_render2(void)
{
    return 0x0;
}

void SnowBike_modelMtxFn(GameObject* obj, f32* x, f32* y, f32* z)
{
    int state = *(int*)&obj->extra;
    *x = ((SnowBikeMountState*)state)->modelMtxPosX;
    *y = ((SnowBikeMountState*)state)->modelMtxPosY;
    *z = ((SnowBikeMountState*)state)->modelMtxPosZ;
}

u8 SnowBike_func0B(GameObject* obj)
{
    return ((SnowBikeState*)obj->extra)->playerInRange;
}

u32 SnowBike_setScale(GameObject* obj)
{
    int state = *(int*)&obj->extra;
    u32 bit = (((SnowBikeMountState*)state)->flags >> 1) & 1;
    if (bit != 0)
    {
        return 0;
    }
    return ((SnowBikeMountState*)state)->unk420;
}

int SnowBike_getExtraSize(void)
{
    return 0x59c;
}

int SnowBike_getObjectTypeId(void)
{
    return 0x3;
}

void SnowBike_free(GameObject* obj)
{
    char* p;
    int i;
    u32 bit;
    int state;

    state = *(int*)&obj->extra;
    ObjGroup_RemoveObject((int)obj, SNOWBIKE_OBJGROUP);
    i = 0;
    p = (char*)state;
    for (; i < 9; i++)
    {
        mm_free(*(void**)(p + 0x4c8));
        p += 8;
    }
    bit = (((SnowBikeMountState*)state)->flags >> 5) & 1;
    if (bit != 0)
    {
        (*gGameUIInterface)->airMeterSetShutdown();
    }
}

void SnowBike_render(GameObject* obj, u32 p2, u32 p3, u32 p4, u32 p5, char visible)
{
    void* path;

    path = (obj)->extra;
    SnowBike_DrawTrails((int)obj, (char*)path);
    if (visible == -1)
    {
        objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, (double)lbl_803E5AEC);
        ObjPath_GetPointWorldPosition(obj, 0, (f32*)((char*)path + 0x3e8),
                                      (f32*)((char*)path + 0x3ec), (f32*)((char*)path + 0x3f0), 0);
    }
    else
    {
        objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, (double)lbl_803E5AEC);
        ObjPath_GetPointWorldPosition(obj, 0, (f32*)((char*)path + 0x3e8),
                                      (f32*)((char*)path + 0x3ec), (f32*)((char*)path + 0x3f0), 0);
    }
}

void SnowBike_hitDetect(GameObject* obj)
{
    SnowBikeState* state;
    u8* other;
    int vol;
    f32 mag;
    f32 velScale;
    f32 velScaleDefault;
    f32 value;
    f32 clamped;
    f32 limit;
    f32 dummy;

    state = obj->extra;
    other = *(u8**)obj->anim.hitReactState;
    if (obj->pendingParentObj != NULL)
    {
        return;
    }
    if (state->riderMode == 2)
    {
        SnowBike_UpdateSteering((short*)obj, (int)state);
        state->savedRotY = obj->anim.rotY;
        state->savedRotZ = obj->anim.rotZ;
        obj->anim.rotY = (f32)obj->anim.rotY + state->haloPitchDrift;
        obj->anim.rotZ = (f32)obj->anim.rotZ + (state->unk410 + state->haloDriftB);
    }
    if (state->unk3D9 == 4 || state->unk3D6 != 0)
    {
        obj->anim.velocityY =
            oneOverTimeDelta * (obj->anim.localPosY - obj->anim.previousLocalPosY);
        state->localVelY = obj->anim.velocityY;
    }
    if (state->unk3D6 != 0 ||
        ((((ObjHitsPriorityState*)obj->anim.hitReactState)->flags & 8) != 0 &&
         arrayIndexOf((int*)gSnowBikeHitObjectIdTable, 10, ((GameObject*)other)->anim.seqId) == -1) ||
        (*(void**)&state->linkedObj != NULL && state->collisionFxDamping <= lbl_803E5AEC))
    {
    mag = PSVECMag((f32*)((int)obj + 0x24));
    if (mag > lbl_803E5AEC)
    {
        if (!((HaloSnowBikeFlags*)&state->flags428)->b02)
        {
            doRumble(lbl_803E5BC4 * mag);
        }
        state->engineFxLevel = state->engineFxLevel * lbl_803E5BBC;
        if (obj->anim.seqId == SNOWBIKE_IM_BIKE_OBJ || obj->anim.seqId == SNOWBIKE_CR_BIKE_OBJ)
        {
            vol = (int)(lbl_803E5C4C * mag);
            if (vol > 80)
            {
                vol = 80;
            }
            else if (vol < 30)
            {
                vol = 30;
            }
            if (Sfx_IsPlayingFromObjectChannel((int)obj, 32) == 0)
            {
                Sfx_PlayFromObject((int)obj, SFXTRIG_tr_jbike_bombbeep);
                Sfx_SetObjectSfxVolume((int)obj, SFXTRIG_tr_jbike_bombbeep, vol, lbl_803E5B28);
            }
        }
    }
    if (!((HaloSnowBikeFlags*)&state->flags428)->b02 && mag > lbl_803E5BC4)
    {
        Camera_EnableViewYOffset();
        CameraShake_SetAllMagnitudes(mag * lbl_803E5AF8);
    }
    if (*(void**)&state->linkedObj != NULL)
    {
        velScale = lbl_803E5C00;
        OSReport(sSnowBikeVelDebugFmt, mag);
        if (((GameObject*)state->linkedObj)->anim.seqId == SNOWBIKE_CR_CLAWBIKE_V0_OBJ || ((GameObject*)state->linkedObj)->anim.seqId == SNOWBIKE_CR_CLAWBIKE_V1_OBJ ||
            ((GameObject*)state->linkedObj)->anim.seqId == SNOWBIKE_CR_CLAWBIKE_V2_OBJ)
        {
            velScale = lbl_803E5B88;
        }
        obj->anim.velocityX =
            velScale *
            (oneOverTimeDelta * (obj->anim.localPosX - obj->anim.previousLocalPosX));
        obj->anim.velocityZ =
            velScale *
            (oneOverTimeDelta * (obj->anim.localPosZ - obj->anim.previousLocalPosZ));
    }
    else
    {
        velScaleDefault = lbl_803E5B88;
        obj->anim.velocityX =
            velScaleDefault *
            (oneOverTimeDelta * (obj->anim.localPosX - obj->anim.previousLocalPosX));
        obj->anim.velocityZ =
            velScaleDefault *
            (oneOverTimeDelta * (obj->anim.localPosZ - obj->anim.previousLocalPosZ));
    }
    Matrix_TransformPoint((f32*)((u8*)state + 0x12c), obj->anim.velocityX, 0.0f,
                          obj->anim.velocityZ, &state->localVelX, &dummy, &state->distanceScale);
    }
{
    f32 limit;
    f32 value = state->localVelX;
    f32 clamped;
    limit = state->localVelXLimit;
    if (value < -limit)
    {
        clamped = -limit;
    }
    else if (value > limit)
    {
        clamped = limit;
    }
    else
    {
        clamped = value;
    }
    state->localVelX = clamped;
}
    if (state->localVelX < lbl_803E5B8C && state->localVelX > lbl_803E5BA4)
    {
        state->localVelX = 0.0f;
    }
    value = state->localVelY;
    limit = state->localVelYLimit;
    if (value < -limit)
    {
        clamped = -limit;
    }
    else if (value > lbl_803E5AEC)
    {
        clamped = lbl_803E5AEC;
    }
    else
    {
        clamped = value;
    }
    state->localVelY = clamped;
    if (state->localVelY < lbl_803E5B8C && state->localVelY > lbl_803E5BA4)
    {
        state->localVelY = 0.0f;
    }
    {
        f32 limit;
        f32 value = state->distanceScale;
        f32 clamped;
        limit = state->distanceScaleLimit;
        if (value < -limit)
        {
            clamped = -limit;
        }
        else if (value > limit)
        {
            clamped = limit;
        }
        else
        {
            clamped = value;
        }
        state->distanceScale = clamped;
    }
    if (state->distanceScale < lbl_803E5B8C && state->distanceScale > lbl_803E5BA4)
    {
        state->distanceScale = 0.0f;
    }
    state->refPosX = obj->anim.localPosX;
    state->refPosY = obj->anim.localPosY;
    state->refPosZ = obj->anim.localPosZ;
    state->linkedObj = 0;
}

void SnowBike_buildOrientationMatrices(GameObject* obj, int state);

void SnowBike_update(GameObject* obj)
{
    u8* state = obj->extra;
    f32 mtx1[16];
    f32 mtx2[16];
    SBRotQuad rq1;
    SBRotQuad rq2;
    f32 vec1[3];
    f32 vec2[3];
    f32 dummy1;
    f32 dummy2;
    s8 mode;
    int t;
    f32 fz;
    f32 damp;
    f32 value;
    f32 clamped;
    SnowBikeState* s = (SnowBikeState*)state;

    if (obj->anim.mapEventSlot == -1)
    {
        if (mainGetBit(GAMEBIT_DIM_CrossedBlizzard) != 0)
        {
            s->playerInRange = 0;
        }
        if (mainGetBit(GAMEBIT_SnowBikeRelated01FB) != 0)
        {
            Obj_SetModelSlotIndex(obj, 0x13);
        }
    }
    *(u8*)&obj->anim.resetHitboxMode |= INTERACT_FLAG_DISABLED;
    obj->anim.rotY = s->savedRotY;
    obj->anim.rotZ = s->savedRotZ;
    if (((SnowBikeFlags*)(state + 0x428))->b04 || mainGetBit(s->gameBitId) != 0)
    {
        ((SnowBikeFlags*)(state + 0x428))->b04 = 1;
        return;
    }
    mode = s->riderMode;
    switch (mode)
    {
    case 0:
    {
        {
            *(u8*)&obj->anim.resetHitboxMode &= ~INTERACT_FLAG_DISABLED;
            if ((*(u8*)&obj->anim.resetHitboxMode & INTERACT_FLAG_IN_RANGE) != 0)
            {
                s->playerInRange = 1;
            }
            else
            {
                s->playerInRange = 0;
            }
            Sfx_StopObjectChannel((int)obj, 0x57);
        }
    }
    break;
    case 2:
    {
        fn_801EAE4C(obj, (SnowBikeState*)state);
        if (((SnowBikeFlags*)(state + 0x428))->b02)
        {
            if (drshackle_updateAttachedPosition(obj, (ShackleSwingState*)state) != 0)
            {
                SnowBike_UpdateExhaustFx(obj, (int)state);
                SnowBike_buildOrientationMatrices(obj, (int)state);
                if (s->collisionFxTimer)
                {
                    PSVECScale((f32*)(state + 0x464), (f32*)(state + 0x47c),
                               s->collisionFxDamping);
                    PSVECScale((f32*)(state + 0x494), (f32*)(state + 0x494),
                               s->collisionFxDamping);
                    s->collisionFxTimer -= timeDelta;
                    if (s->collisionFxTimer <= 0.0f)
                    {
                        if (Rcp_GetMotionBlurEnabled() != 0)
                        {
                            setMotionBlur(0, 0.0f);
                        }
                        s->collisionFxTimer = 0.0f;
                    }
                }
                else
                {
                    s->localVelXLimit = s->velLimitX;
                    s->localVelYLimit = s->velLimitY;
                    s->distanceScaleLimit = s->velLimitZ;
                }
                fz = 0.0f;
                rq1.quad[1] = fz;
                rq1.quad[2] = fz;
                rq1.quad[3] = fz;
                rq1.quad[0] = lbl_803E5AEC;
                rq1.rot[0] = -s->yaw;
                rq1.rot[1] = -obj->anim.rotY;
                rq1.rot[2] = -obj->anim.rotZ;
                mtxRotateByVec3s(mtx1, rq1.rot);
                Matrix_TransformPoint(mtx1, 0.0f,
                                      s->liftAccel * s->turnForceGain,
                                      0.0f, &vec1[0], &dummy1, &vec1[2]);
                vec1[0] = vec1[0] * s->turnVelScale;
                vec1[1] = 0.0f;
                PSVECScale(vec1, vec1, timeDelta);
                PSVECAdd((f32*)(state + 0x494), vec1, (f32*)(state + 0x494));
                s->localVelY =
                    s->liftAccel * timeDelta + s->localVelY;
                damp = powfBitEstimate(s->localVelXDamp, timeDelta);
                s->localVelX *= damp;
                damp = powfBitEstimate(s->distanceScaleDamp, timeDelta);
                s->distanceScale *= damp;
                SnowBike_UpdateLiftSway((int)obj, (int)state);
                Matrix_TransformPoint((f32*)(state + 0xec), s->localVelX,
                                      s->localVelY, s->distanceScale,
                                      &obj->anim.velocityX, &obj->anim.velocityY,
                                      &obj->anim.velocityZ);
                objApplyVelocity(obj);
            }
        }
        else
        {
            setAButtonIcon(0x10);
            setBButtonIcon(0x11);
            s->stickX = padGetStickX(0);
            s->stickY = (f32)padGetStickY(0);
            s->buttonsHeld = getButtonsHeld(0);
            s->buttonsJustPressed = getButtonsJustPressed(0);
            s->buttonsJustPressedIfNotBusy = getButtonsJustPressedIfNotBusy(0);
            s->steerAngleDeg =
                (f32)(u16)getAngle(s->stickX, (f32) - (int)s->stickY) /
                gSnowBikeBamToDeg;
            s->stickX = s->stickX / lbl_803E5B6C;
            value = s->stickX;
            if (value < lbl_803E5B70)
            {
                clamped = lbl_803E5B70;
            }
            else if (value > lbl_803E5AEC)
            {
                clamped = lbl_803E5AEC;
            }
            else
            {
                clamped = value;
            }
            s->stickX = clamped;
            SnowBike_UpdateExhaustFx(obj, (int)state);
            SnowBike_buildOrientationMatrices(obj, (int)state);
            if (s->collisionFxTimer)
            {
                PSVECScale((f32*)(state + 0x464), (f32*)(state + 0x47c), s->collisionFxDamping);
                PSVECScale((f32*)(state + 0x494), (f32*)(state + 0x494), s->collisionFxDamping);
                s->collisionFxTimer -= timeDelta;
                if (s->collisionFxTimer <= 0.0f)
                {
                    if (Rcp_GetMotionBlurEnabled() != 0)
                    {
                        setMotionBlur(0, 0.0f);
                    }
                    s->collisionFxTimer = 0.0f;
                }
            }
            else
            {
                s->localVelXLimit = s->velLimitX;
                s->localVelYLimit = s->velLimitY;
                s->distanceScaleLimit = s->velLimitZ;
            }
            fz = 0.0f;
            rq2.quad[1] = fz;
            rq2.quad[2] = fz;
            rq2.quad[3] = fz;
            rq2.quad[0] = lbl_803E5AEC;
            rq2.rot[0] = -s->yaw;
            rq2.rot[1] = -obj->anim.rotY;
            rq2.rot[2] = -obj->anim.rotZ;
            mtxRotateByVec3s(mtx2, rq2.rot);
            Matrix_TransformPoint(mtx2, 0.0f,
                                  s->liftAccel * s->turnForceGain,
                                  0.0f, &vec2[0], &dummy2, &vec2[2]);
            vec2[0] = vec2[0] * s->turnVelScale;
            vec2[1] = 0.0f;
            PSVECScale(vec2, vec2, timeDelta);
            PSVECAdd((f32*)(state + 0x494), vec2, (f32*)(state + 0x494));
            s->localVelY =
                s->liftAccel * timeDelta + s->localVelY;
            damp = powfBitEstimate(s->localVelXDamp, timeDelta);
            s->localVelX *= damp;
            damp = powfBitEstimate(s->distanceScaleDamp, timeDelta);
            s->distanceScale *= damp;
            SnowBike_UpdateLiftSway((int)obj, (int)state);
            Matrix_TransformPoint((f32*)(state + 0xec), s->localVelX,
                                  s->localVelY, s->distanceScale,
                                  &obj->anim.velocityX, &obj->anim.velocityY,
                                  &obj->anim.velocityZ);
            objApplyVelocity(obj);
        }
        SnowBike_UpdateAirMeter((int)obj, (int)state);
        drcloudcage_updateEngineFx(obj, state, s->distanceScale,
                                   (int)(lbl_803E5BA0 * -s->engineFxLevel), state + 0x461, 7);
        fn_801EB634(obj, (int)state);
        obj->anim.rotX = s->yaw;
    }
    break;
    }
}

void SnowBike_InitTuning(GameObject* obj, int state);

void SnowBike_init(GameObject* obj, SnowBikePlacement* params, int flag)
{
    f32 fv;
    f32 fz;
    s16 rot;
    u8* path;
    int i;
    u8* alloc;
    u32 pathParam;
    char* base[1];
    u8* state;
    SnowBikeState* s;

    base[0] = (char*)lbl_803284E0;
    pathParam = lbl_803E5AE0;
    state = obj->extra;
    s = (SnowBikeState*)state;

    if (obj->anim.mapEventSlot == 0x13)
    {
        alloc = mmAlloc(36, 5, 0);
        memcpy(alloc, params, 36);
        *(u8**)&obj->anim.placementData = alloc;
        obj->anim.flags |= OBJANIM_FLAG_OWNS_PLACEMENT_DATA;
        Obj_ClearModelSlotIndex(obj);
    }
    rot = params->yawByte << 8;
    s->yawCurrent = rot;
    s->yaw = rot;
    obj->anim.rotX = rot;
    SnowBike_InitTuning(obj, (int)state);
    if (flag == 0)
    {
        if (((SnowBikeFlags*)(state + 0x428))->b20)
        {
            s->airMeterMax = lbl_803E5B90;
            s->airDrainRate = lbl_803E5AEC;
            s->airMeterCurrent = lbl_803E5B94;
            if (s->riderMode == 2)
            {
                (*gGameUIInterface)
                    ->initAirMeter((int)s->airMeterMax, SNOWBIKE_AIRMETER_BGTEXTURE);
                (*gGameUIInterface)->airMeterSetRatio(lbl_803E5B98);
            }
        }
    }
    if (params->startFlag != 0)
    {
        ((SnowBikeFlags*)(state + 0x428))->b02 = 1;
    }
    s->checkpointIndexA = -1;
    s->checkpointIndexB = -1;
    s->checkpointIndexC = -1;
    s->routeFilter = params->param1c;
    s->unk05D = params->param1d;
    s->posSnapshotX = obj->anim.localPosX;
    s->posSnapshotY = obj->anim.localPosY;
    s->posSnapshotZ = obj->anim.localPosZ;
    obj->animEventCallback = SnowBike_SeqFn;
    ObjGroup_AddObject((int)obj, SNOWBIKE_OBJGROUP);
    if (flag == 0)
    {
        i = 0;
        for (path = state; i < 9; i++)
        {
            *(u8**)(path + 0x4c8) = mmAlloc(1600, 26, 0);
            path += 8;
        }
    }
    s->homePosX = obj->anim.worldPosX;
    s->homePosY = obj->anim.worldPosY;
    s->homePosZ = obj->anim.worldPosZ;
    s->pathProgress = 0.0f;
    s->unk448 = params->unk1a;
    s->gameBitId = params->gameBitId;
    if (mainGetBit(s->gameBitId) != 0)
    {
        ((SnowBikeFlags*)(state + 0x428))->b04 = 1;
    }
    s->unk438 = lbl_803E5B1C;
    fz = 0.0f;
    s->unk3F4 = fz;
    s->unk3F8 = fz;
    s->unk018 = lbl_803E5C48;
    s->unk01C = fz;
    s->unk020 = lbl_803E5BC4;
    s->unk024 = lbl_803E5C50;
    s->collisionHitType = -1;
    fv = lbl_803E5B98;
    s->velLimitX = fv;
    s->velLimitY = fv;
    s->modelId = 0x436;
    switch (obj->anim.seqId)
    {
    case SNOWBIKE_IM_BIKE_OBJ:
    default:
        s->bikeType = 1;
        s->velLimitZ = lbl_803E5C50;
        s->modelId = 282;
        break;
    case SNOWBIKE_IM_CLAWBIKE_V0_OBJ:
        s->bikeType = 1;
        s->bikeVariant = 0;
        s->unk01C = lbl_803E5B14;
        s->unk018 = lbl_803E5C54;
        s->collisionHitType = 1;
        s->velLimitZ = lbl_803E5AF0;
        break;
    case SNOWBIKE_IM_CLAWBIKE_V1_OBJ:
        s->bikeType = 1;
        s->unk058 = 1;
        s->bikeVariant = 1;
        s->collisionHitType = 2;
        s->velLimitZ = lbl_803E5AF0;
        break;
    case SNOWBIKE_CR_BIKE_OBJ:
        s->bikeType = 0;
        s->velLimitZ = lbl_803DC0C4;
        s->modelId = 282;
        break;
    case SNOWBIKE_CR_CLAWBIKE_V0_OBJ:
        s->bikeType = 0;
        s->bikeVariant = 0;
        s->unk01C = lbl_803E5B14;
        s->unk018 = lbl_803E5C54;
        s->velLimitZ = lbl_803E5C58 * lbl_803DC0C0;
        break;
    case SNOWBIKE_CR_CLAWBIKE_V1_OBJ:
        s->bikeType = 0;
        s->bikeVariant = 1;
        s->unk01C = lbl_803E5B48;
        s->unk018 = lbl_803E5C5C;
        s->velLimitZ = lbl_803E5C60 * lbl_803DC0C0;
        break;
    case SNOWBIKE_CR_CLAWBIKE_V2_OBJ:
        s->bikeType = 0;
        s->bikeVariant = 2;
        s->unk01C = lbl_803E5B48;
        s->unk018 = lbl_803E5C5C;
        s->velLimitZ = lbl_803DC0C0;
        break;
    }
    fv = s->velLimitX;
    s->localVelXLimit = fv;
    s->baseVelLimitX = fv;
    fv = s->velLimitY;
    s->localVelYLimit = fv;
    s->baseVelLimitY = fv;
    fv = s->velLimitZ;
    s->distanceScaleLimit = fv;
    s->baseVelLimitZ = fv;
    s->gameBitPtr = (s16*)((int)base[0] + 0xa4 + s->bikeType * 6);
    if (s->bikeType == 0)
    {
        if (!((SnowBikeFlags*)(state + 0x428))->b02)
        {
            ((SnowBikeFlags*)(state + 0x428))->b20 = 1;
            s->airMeterRefillTimer = 0.0f;
        }
        s->unk538 = lbl_803E5C64;
    }
    else
    {
        s->unk538 = lbl_803E5B74;
    }
    path = state + 0x178;
    path[0x25b] = 1;
    (*gPathControlInterface)->init(path, 0, 0x48607, 1);
    (*gPathControlInterface)->setup(path, 4, base[0], base[0] + 0x30, &pathParam);
    if (((SnowBikeFlags*)(state + 0x428))->b02 && s->collisionHitType != -1)
    {
        curves_setLocalPointCollisionEx((CurvesCollisionState*)path, 1, (f32*)(base[0] + 0x40), &lbl_803DC0B8, 8,
                                        s->collisionHitType);
    }
    else
    {
        (*gPathControlInterface)->setLocalPointCollision(path, 1, base[0] + 0x40, &lbl_803DC0B8, 8);
    }
    path[0x264] = lbl_803E5C68 + lbl_803DC0B8;
    (*gPathControlInterface)->attachObject((void*)obj, path);
}

void SnowBike_release(void)
{
    if (lbl_803DDC60 != 0)
    {
        textureFree((Texture*)((u8*)lbl_803DDC60));
        lbl_803DDC60 = 0;
    }
}

void SnowBike_initialise(void)
{
    if (lbl_803DDC60 == 0)
    {
        lbl_803DDC60 = textureLoadAsset(SNOWBIKE_TEXTURE_ID);
    }
}

ObjectDescriptor24 gSnowBikeObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_24_SLOTS,
    (ObjectDescriptorCallback)SnowBike_initialise,
    (ObjectDescriptorCallback)SnowBike_release,
    NULL,
    (ObjectDescriptorCallback)SnowBike_init,
    (ObjectDescriptorCallback)SnowBike_update,
    (ObjectDescriptorCallback)SnowBike_hitDetect,
    (ObjectDescriptorCallback)SnowBike_render,
    (ObjectDescriptorCallback)SnowBike_free,
    (ObjectDescriptorCallback)SnowBike_getObjectTypeId,
    (ObjectDescriptorExtraSizeCallback)SnowBike_getExtraSize,
    (ObjectDescriptorCallback)SnowBike_setScale,
    (ObjectDescriptorCallback)SnowBike_func0B,
    (ObjectDescriptorCallback)SnowBike_modelMtxFn,
    (ObjectDescriptorCallback)SnowBike_render2,
    (ObjectDescriptorCallback)SnowBike_func0E,
    (ObjectDescriptorCallback)SnowBike_mount,
    (ObjectDescriptorCallback)SnowBike_getRiderMode,
    (ObjectDescriptorCallback)SnowBike_setRiderMode,
    (ObjectDescriptorCallback)SnowBike_func12,
    (ObjectDescriptorCallback)SnowBike_func13,
    (ObjectDescriptorCallback)SnowBike_func14,
    (ObjectDescriptorCallback)SnowBike_resetToRomListPosition,
    (ObjectDescriptorCallback)SnowBike_func16,
    (ObjectDescriptorCallback)SnowBike_func17,
};
