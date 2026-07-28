#define OBJHITS_STATE_INDEX_S8
#define TEX_SETSHADER_U8
#include "main/map_block.h"
#include "main/texture.h"
#include "track/intersect_depth_state_api.h"
#include "track/intersect_depth_read_api.h"
#include "track/intersect_render_setup_api.h"
#include "main/hud_visibility_api.h"
#include "main/lightmap_api.h"
#include "main/shader_api.h"
#include "main/debug.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_float_helpers.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/frustum.h"
#include "main/asset_load.h"
#include "game/objects/object.h"
#include "main/gameloop_api.h"
#include "sys/objects.h"
#include "main/mm.h"
#include "main/model_light.h"
#include "main/model.h"
#include "main/model_render_instrs_api.h"
#include "main/objHitReact.h"
#include "main/objhits.h"
#undef OBJHITS_STATE_INDEX_S8
#include "main/obj_group.h"
#include "main/object_transform.h"
#include "main/vecmath.h"
#include "dolphin/mtx/mtx_legacy.h"
#include "dolphin/os/OSFastCast.h"
#include "dolphin/gx/GXCull.h"
#include "dolphin/gx/GXGeometry.h"
#include "dolphin/gx/GXLighting.h"
#include "dolphin/gx/GXBump.h"
#include "dolphin/gx/GXPixel.h"
#include "dolphin/gx/GXTev.h"
#include "dolphin/gx/GXTransform.h"
#include "main/camera.h"
#include "main/sky_state.h"
#include "main/track_dolphin.h"
#include "main/track_dolphin_api.h"
#include "main/track_dolphin_shadow_api.h"
#include "main/newshadows_shadow_api.h"
#define TRACK_BBOX_FLAGS_S8
#define TRACK_BBOX_MASK_TYPE s8
#define TRACK_BBOX_ARG10_TYPE s8
#include "main/track_bbox_api.h"
#undef TRACK_BBOX_ARG10_TYPE
#undef TRACK_BBOX_MASK_TYPE
#undef TRACK_BBOX_FLAGS_S8
#include "main/dll/player_api.h"
#include "main/pause_menu_api.h"
#include "main/pi_dolphin.h"
#include "dolphin/os/OSCache.h"
#include "main/voxmaps.h"
#include "track/intersect_api.h"
#include "main/rcp_dolphin_api.h"
#include "main/objmodel.h"
#include "main/newshadows.h"
#include "main/sky.h"
#include "main/newshadows_texture_api.h"
#include "main/acosf_api.h"
#include "main/tex_dolphin.h"
#include "string.h"
#include "main/track_dolphin_sky_api.h"
#include "main/track_dolphin_sky_api.h"

int gShadowVolumeBuffer;
void* lbl_803DCF24[2];
int lbl_803DCF20;
int lbl_803DCF1C;
int lbl_803DCF18;
int lbl_803DCF14;
int lbl_803DCF10;
int lbl_803DCF0C;
void* lbl_803DCF08;
void* lbl_803DCF04;
f32 gSunDotCos;
s16 lbl_803DCEFC;
s16 lbl_803DCEFA;
s16 lbl_803DCEF8;
s16 lbl_803DCEF6;
s16 lbl_803DCEF4;
s16 gShadowVisibleCount;
s16 lbl_803DCEF0;
s8 lbl_803DCEEE;
s8 lbl_803DCEED;
s8 lbl_803DCEEC;
s8 lbl_803DCEEB;
s8 lbl_803DCEEA;
u8 lbl_803DCEE9;
u8 lbl_803DCEE8;
int lbl_803DCEE4;
int lbl_803DCEE0;
f32 gShadowOffsetZ;
f32 gShadowOffsetX;

f32 gShadowOffsetY = 1.0f;
f32 lbl_803DB654 = 1.0f;
s8 gShadowVolumesDirty = 10;
s16 gSunMagnitude = 100;
int gSunDirChanged = 1;

typedef struct TrackShadowTriangle
{
    Vec3f normal;
    f32 planeDistance;
    s8 flags;
    u8 pad11[3];
} TrackShadowTriangle;

typedef struct AngleXf
{
    s16 rotX;
    s16 rotY;
    s16 rotZ;
    s16 pad6;
    f32 scale;
    f32 tx;
    f32 ty;
    f32 tz;
} AngleXf;

typedef union
{
    u8 u8;
    u16 u16;
    u32 u32;
    s16 s16;
    s32 s32;
    f32 f32;
} GolfWGPipe;

extern volatile GolfWGPipe GXWGFifo : (0xCC008000);
extern int renderFlags;

void trackDolphin_buildShadowVolumePlanes(int* obj, void* buf48, void* bufA8);

/* Begin a new shadow-volume frame: clear the per-frame
 * counts, flip the three double-buffer selectors, and rotate the current
 * write pointers to the buffer picked by this frame's flip index. */
void vecGetRanges(f32* pts, f32* base, f32 scale, int* out);

int objShadowFn_80062378(GameObject* obj, u8 param);

void buildShadowVolumeBox(f32* direction, f32* out, f32 lowerScale);

f32 lbl_8038D7DC[0x19];
f32 gPrevSunDir[3];

u8 gShadowDrawScratch[0x5DC0];

f32 lbl_8038D77C[0x18];

static inline void GXPosition3s16(const int x, const int y, const int z)
{
    GXWGFifo.s16 = x;
    GXWGFifo.s16 = y;
    GXWGFifo.s16 = z;
}

static inline void GXTexCoord2s16(const s16 x, const s16 y)
{
    GXWGFifo.s16 = x;
    GXWGFifo.s16 = y;
}

static inline void GXPosition3f32(const f32 x, const f32 y, const f32 z)
{
    GXWGFifo.f32 = x;
    GXWGFifo.f32 = y;
    GXWGFifo.f32 = z;
}

static f32 shadowGetSunMagnitude(void)
{
    f32 magnitude = 0.0f;

    if (gSunMagnitude > 0)
    {
        magnitude = (f32)gSunMagnitude;
    }
    return magnitude;
}

void buildShadowVolumeBox(f32* direction, f32* out, f32 lowerScale)
{
    AngleXf xf;
    f32 ax;
    f32 az;
    int i;
    int rotY;

    xf.tx = 0.0f;
    xf.ty = 0.0f;
    xf.tz = 0.0f;
    xf.scale = 1.0f;
    xf.rotZ = 0;
    ax = __fabsf(direction[0]);
    az = __fabsf(direction[2]);
    if (ax > az)
    {
        rotY = (u16)getAngle(ax, direction[1]);
    }
    else
    {
        rotY = (u16)getAngle(az, direction[1]);
    }
    xf.rotY = rotY;
    if (xf.rotY > 0x2000)
    {
        xf.rotY = 0x2000;
    }
    xf.rotX = (s16)getAngle(direction[0], direction[2]);
    for (i = 0; i < 8; i++)
    {
        out[i * 3 + 0] = lbl_8038D7DC[i * 3 + 0];
        if (lbl_8038D7DC[i * 3 + 1] > 0.0f)
        {
            out[i * 3 + 1] = lbl_8038D7DC[i * 3 + 1];
        }
        else
        {
            out[i * 3 + 1] = lowerScale * lbl_8038D7DC[i * 3 + 1];
        }
        out[i * 3 + 2] = lbl_8038D7DC[i * 3 + 2];
        vecRotateZXY(&xf.rotX, &out[i * 3]);
    }
}

void vecGetRanges(f32* pts, f32* base, f32 scale, int* out)
{
    int i;

    out[0] = 0x7fffffff;
    out[3] = 0x80000000;
    out[1] = 0x7fffffff;
    out[4] = 0x80000000;
    out[2] = 0x7fffffff;
    out[5] = 0x80000000;
    for (i = 0; i < 8; i++)
    {
        f32 x = scale * pts[0] + base[0];
        f32 y = scale * pts[1] + base[1];
        f32 z = scale * pts[2] + base[2];
        if (x < out[0])
            out[0] = x;
        if (x > out[3])
            out[3] = x;
        if (y < out[1])
            out[1] = y;
        if (y > out[4])
            out[4] = y;
        if (z < out[2])
            out[2] = z;
        if (z > out[5])
            out[5] = z;
        pts += 3;
    }
}

void buildGroundShadowQuad(s16* out, GameObject* obj)
{
    f32 dist;
    f32 b[3];
    f32 c[3];
    f32 a[3];
    f32 d;
    f32 scale;
    f32 z;
    f32 s;
    f32 nd;

    if (trackGetNearestGroundOffsetAndNormal(obj, obj->anim.localPosX, obj->anim.localPosY,
                                             obj->anim.localPosZ, &dist, a, 0) == 0)
    {
        PSVECNormalize(a, a);
        b[0] = 1.0f;
        b[1] = 0.0f;
        b[2] = 0.0f;
        d = __fabsf(PSVECDotProduct(a, b));
        if (d >= 0.9f)
        {
            b[0] = 0.0f;
            b[2] = 1.0f;
        }
        PSVECCrossProduct(a, b, c);
        PSVECCrossProduct(c, a, b);
        PSVECNormalize(b, b);
        PSVECNormalize(c, c);
        scale = 0.5f * ((ObjAnimComponent*)obj)->modelState->shadowScale;
        PSVECScale(b, b, scale);
        PSVECScale(c, c, scale);
        nd = -dist;
        s = 256.0f;
        z = 0.0f;
        out[0] = (s * ((z - b[0]) - c[0]));
        out[1] = (s * ((nd - b[1]) - c[1]));
        out[2] = (s * ((z - b[2]) - c[2]));
        out[3] = (s * ((z + b[0]) - c[0]));
        out[4] = (s * ((nd + b[1]) - c[1]));
        out[5] = (s * ((z + b[2]) - c[2]));
        out[6] = (s * (c[0] + (z + b[0])));
        out[7] = (s * (c[1] + (nd + b[1])));
        out[8] = (s * (c[2] + (z + b[2])));
        out[9] = (s * (c[0] + (z - b[0])));
        out[10] = (s * (c[1] + (nd - b[1])));
        out[11] = (s * (c[2] + (z - b[2])));
        *(u8*)((char*)out + 0x18) = 1;
    }
    else
    {
        *(u8*)((char*)out + 0x18) = 0xff;
    }
}

void objDrawFn_80061654(GameObject* obj, ObjModel* model)
{
    s16* shadowVerts;
    u8 alpha;
    void* viewMtx;
    GXColor kColor;
    f32 mtx[16];
    f32 outMtx[16];

    shadowVerts = (s16*)model->unk54;
    if (*(u8*)((u8*)shadowVerts + 0x18) == 0)
    {
        buildGroundShadowQuad(shadowVerts, obj);
    }
    if (*(u8*)((u8*)shadowVerts + 0x18) != 0xff)
    {
        alpha = (u8)objShadowFn_80062378(obj, 0x96);
        kColor.a = alpha;
        if (alpha != 0)
        {
            viewMtx = Camera_GetViewMatrix();
            Obj_BuildWorldTransformMatrix(obj, mtx, 0);
            mtx[0] = 1.0f;
            mtx[1] = 0.0f;
            mtx[2] = 0.0f;
            mtx[4] = 0.0f;
            mtx[5] = 1.0f;
            mtx[6] = 0.0f;
            mtx[8] = 0.0f;
            mtx[9] = 0.0f;
            mtx[10] = 1.0f;
            PSMTXConcat(viewMtx, mtx, outMtx);
            GXLoadPosMtxImm((const f32 (*)[4])outMtx, GX_PNMTX9);
            GXClearVtxDesc();
            GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
            GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
            GXSetNumTexGens(1);
            GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
            GXSetTevKColor(GX_KCOLOR0, kColor);
            GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
            GXSetNumTevStages(1);
            GXSetNumIndStages(0);
            GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
            GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
            GXSetNumChans(0);
            GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
            GXSetTevDirect(GX_TEVSTAGE0);
            GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
            GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_KONST, GX_CA_TEXA, GX_CA_ZERO);
            GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV);
            GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV);
            gxSetZMode_(1, GX_LEQUAL, 0);
            GXSetCullMode(GX_CULL_NONE);
            GXSetCurrentMtx(GX_PNMTX9);
            GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
            selectTexture((Texture*)((int)((ObjAnimComponent*)obj)->modelState->shadowTexture), 0);
            GXBegin(GX_QUADS, GX_VTXFMT6, 4);
            GXPosition3s16(shadowVerts[0], shadowVerts[1], shadowVerts[2]);
            GXTexCoord2s16(0, 0);
            GXPosition3s16(shadowVerts[3], shadowVerts[4], shadowVerts[5]);
            GXTexCoord2s16(0x400, 0);
            GXPosition3s16(shadowVerts[6], shadowVerts[7], shadowVerts[8]);
            GXTexCoord2s16(0x400, 0x400);
            GXPosition3s16(shadowVerts[9], shadowVerts[10], shadowVerts[11]);
            GXTexCoord2s16(0, 0x400);
            GXSetCurrentMtx(GX_PNMTX0);
        }
    }
}

void trackDolphin_buildShadowVolumePlanes(int* obj, void* buf48, void* bufA8)
{
    f32* verts = buf48;
    f32* planes = bufA8;
    f32 nrm[3];

    {
    Vec3f edge1;
    Vec3f edge2;

    edge1.x = verts[6] - verts[9];
    edge1.y = verts[7] - verts[10];
    edge1.z = verts[8] - verts[0xb];
    edge2.x = verts[0x15] - verts[9];
    edge2.y = verts[0x16] - verts[10];
    edge2.z = verts[0x17] - verts[0xb];
    nrm[0] = edge2.y * edge1.z - edge2.z * edge1.y;
    nrm[1] = -(edge2.x * edge1.z - edge2.z * edge1.x);
    nrm[2] = edge2.x * edge1.y - edge2.y * edge1.x;
    PSVECNormalize(nrm, nrm);
    planes[0] = -nrm[0];
    planes[1] = -nrm[1];
    planes[2] = -nrm[2];
    planes[3] = -(planes[0] * verts[9] + planes[1] * verts[10] + planes[2] * verts[0xb]);
    }

    {
    Vec3f edge1;
    Vec3f edge2;

    edge1.x = verts[0x12] - verts[0xf];
    edge1.y = verts[0x13] - verts[0x10];
    edge1.z = verts[0x14] - verts[0x11];
    edge2.x = verts[3] - verts[0xf];
    edge2.y = verts[4] - verts[0x10];
    edge2.z = verts[5] - verts[0x11];
    nrm[0] = edge2.y * edge1.z - edge2.z * edge1.y;
    nrm[1] = -(edge2.x * edge1.z - edge2.z * edge1.x);
    nrm[2] = edge2.x * edge1.y - edge2.y * edge1.x;
    PSVECNormalize(nrm, nrm);
    planes[5] = -nrm[0];
    planes[6] = -nrm[1];
    planes[7] = -nrm[2];
    planes[8] = -(planes[5] * verts[0xf] + planes[6] * verts[0x10] + planes[7] * verts[0x11]);
    }

    {
    Vec3f edge1;
    Vec3f edge2;

    edge1.x = verts[0xf] - verts[0xc];
    edge1.y = verts[0x10] - verts[0xd];
    edge1.z = verts[0x11] - verts[0xe];
    edge2.x = verts[0] - verts[0xc];
    edge2.y = verts[1] - verts[0xd];
    edge2.z = verts[2] - verts[0xe];
    nrm[0] = edge2.y * edge1.z - edge2.z * edge1.y;
    nrm[1] = -(edge2.x * edge1.z - edge2.z * edge1.x);
    nrm[2] = edge2.x * edge1.y - edge2.y * edge1.x;
    PSVECNormalize(nrm, nrm);
    planes[10] = -nrm[0];
    planes[0xb] = -nrm[1];
    planes[0xc] = -nrm[2];
    planes[0xd] = -(planes[10] * verts[0xc] + planes[0xb] * verts[0xd] + planes[0xc] * verts[0xe]);
    }

    {
    Vec3f edge1;
    Vec3f edge2;

    edge1.x = verts[9] - verts[0];
    edge1.y = verts[10] - verts[1];
    edge1.z = verts[0xb] - verts[2];
    edge2.x = verts[0xc] - verts[0];
    edge2.y = verts[0xd] - verts[1];
    edge2.z = verts[0xe] - verts[2];
    nrm[0] = edge2.y * edge1.z - edge2.z * edge1.y;
    nrm[1] = -(edge2.x * edge1.z - edge2.z * edge1.x);
    nrm[2] = edge2.x * edge1.y - edge2.y * edge1.x;
    PSVECNormalize(nrm, nrm);
    planes[0xf] = -nrm[0];
    planes[0x10] = -nrm[1];
    planes[0x11] = -nrm[2];
    planes[0x12] = -(planes[0xf] * verts[0] + planes[0x10] * verts[1] + planes[0x11] * verts[2]);
    }

    {
    Vec3f edge1;
    Vec3f edge2;

    edge1.x = verts[0x12] - verts[0x15];
    edge1.y = verts[0x13] - verts[0x16];
    edge1.z = verts[0x14] - verts[0x17];
    edge2.x = verts[0xc] - verts[0x15];
    edge2.y = verts[0xd] - verts[0x16];
    edge2.z = verts[0xe] - verts[0x17];
    nrm[0] = edge2.y * edge1.z - edge2.z * edge1.y;
    nrm[1] = -(edge2.x * edge1.z - edge2.z * edge1.x);
    nrm[2] = edge2.x * edge1.y - edge2.y * edge1.x;
    PSVECNormalize(nrm, nrm);
    planes[0x14] = -nrm[0];
    planes[0x15] = -nrm[1];
    planes[0x16] = -nrm[2];
    planes[0x17] = -(planes[0x14] * verts[0x15] + planes[0x15] * verts[0x16] + planes[0x16] * verts[0x17]);
    }

    {
    Vec3f edge1;
    Vec3f edge2;

    edge1.x = verts[3] - verts[0];
    edge1.y = verts[4] - verts[1];
    edge1.z = verts[5] - verts[2];
    edge2.x = verts[9] - verts[0];
    edge2.y = verts[10] - verts[1];
    edge2.z = verts[0xb] - verts[2];
    nrm[0] = edge2.y * edge1.z - edge2.z * edge1.y;
    nrm[1] = -(edge2.x * edge1.z - edge2.z * edge1.x);
    nrm[2] = edge2.x * edge1.y - edge2.y * edge1.x;
    PSVECNormalize(nrm, nrm);
    planes[0x19] = -nrm[0];
    planes[0x1a] = -nrm[1];
    planes[0x1b] = -nrm[2];
    planes[0x1c] = -(planes[0x19] * verts[0] + planes[0x1a] * verts[1] + planes[0x1b] * verts[2]);
    }
}

int cullVisibleShadowTriangles(GameObject* obj, void* u1, void* u2, int count, Vec3f* vertices, Vec3f* outVertices,
                TrackShadowTriangle* triangles, int limit)
{
    int vertexIndex = 0;
    int outCount = 0;
    int i = 0;
    int j;
    ObjModelState* modelState = obj->anim.modelState;

    gShadowVisibleCount = 0;
    for (; i < count; i++, vertexIndex += 3, triangles++)
    {
        int vis = 1;
        f32 dot = modelState->shadowOffsetX * triangles->normal.x + modelState->shadowOffsetY * triangles->normal.y +
                  modelState->shadowOffsetZ * triangles->normal.z;
        if (dot < 0.0f)
        {
            vis = -1;
        }
        if (vis == 1)
        {
            gShadowVisibleCount++;
            for (j = 0; j < 3; j++)
            {
                outVertices->x = vertices[vertexIndex + j].x;
                outVertices->y = vertices[vertexIndex + j].y;
                outVertices->z = vertices[vertexIndex + j].z;
                outVertices++;
                if (++outCount >= limit)
                {
                    return 0;
                }
            }
        }
    }
    return gShadowVisibleCount > 0;
}

void objDrawFn_80061f0c(Vec3f* vertices, ObjModelState* modelState, GameObject* obj, int triangleCount, void* p7,
                       void* buf48, f32 f)
{
    u8 col[4];
    Vec3f savedWorldPos;
    Vec3f savedLocalPos;
    f32 mtx[4][4];
    f32 outMtx[4][4];
    f32 f31, f30;
    f32 kf;
    s16 s31, s30, s29;
    u32 h2;
    void* viewMtx;

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    col[0] = 0;
    col[1] = 0;
    col[2] = 0;
    col[3] = modelState->shadowCastSlot->alpha;
    f31 = obj->anim.rootMotionScale;
    s31 = obj->anim.rotX;
    s30 = obj->anim.rotZ;
    s29 = obj->anim.rotY;
    if (modelState->shadowRenderResource == NULL ||
        modelState->shadowRenderResource != OBJECT_SHADOW_MESH_UNCACHED)
        obj->anim.rootMotionScale = 0.05f;
    else
        obj->anim.rootMotionScale = 1.0f;
    obj->anim.rotX = 0;
    obj->anim.rotY = 0;
    if ((modelState->flags & 0x2000) == 0)
        obj->anim.rotZ = 0;
    if (modelState->flags & 0x20)
    {
        memcpy(&savedLocalPos, &obj->anim.localPos, sizeof(Vec3f));
        memcpy(&savedWorldPos, &obj->anim.worldPos, sizeof(Vec3f));
        memcpy(&obj->anim.worldPos, &modelState->overrideWorldPos, sizeof(Vec3f));
        memcpy(&obj->anim.localPos, &modelState->overrideWorldPos, sizeof(Vec3f));
    }
    Obj_BuildWorldTransformMatrix(obj, (f32*)mtx, 0);
    viewMtx = Camera_GetViewMatrix();
    PSMTXConcat(viewMtx, (f32*)mtx, (f32*)outMtx);
    GXLoadPosMtxImm((const f32 (*)[4])outMtx, GX_PNMTX0);
    if (obj->anim.modelInstance->renderFlags & OBJDEF_RENDERFLAG_PROJECTED_SHADOW)
    {
        u32 c = *(u32*)col;
        objectShadow_setupSwappedProjectedTexture(modelState->shadowCastSlot, &c, mtx);
    }
    else
    {
        if (obj == Obj_GetPlayerObject())
            f30 = 10.0f;
        else
            f30 = obj->anim.hitboxScale * obj->anim.rootMotionScale;
        if (modelState->shadowRenderResource != OBJECT_SHADOW_MESH_UNCACHED ||
            (h2 = getNewShadowSmallDiskTexture(), (u32)modelState->shadowCastSlot->texture == h2))
        {
            u32 c = *(u32*)col;
            objectShadow_setupProjectedTexture(modelState->shadowCastSlot, &c, mtx);
        }
        else if (modelState->shadowCastSlot->mode == 0xff)
        {
            u32 c = *(u32*)col;
            objectShadow_setupProjectedTextureDepthFade(modelState->shadowCastSlot, &c, mtx, f30);
        }
        else
        {
            u32 c = *(u32*)col;
            objectShadow_setupProjectedTextureChannel(modelState->shadowCastSlot, &c, mtx, f30);
        }
    }
    GXSetCullMode(GX_CULL_FRONT);
    GXSetCurrentMtx(GX_PNMTX0);
    obj->anim.rootMotionScale = f31;
    obj->anim.rotX = s31;
    obj->anim.rotY = s29;
    obj->anim.rotZ = s30;
    if (modelState->shadowRenderResource == NULL)
    {
        u32 i;
        modelState->shadowRenderResource = mmAlloc(triangleCount * 0x12 + sizeof(ObjectShadowMesh), 0x18, 0);
        if (modelState->shadowRenderResource == NULL)
            return;
        modelState->shadowRenderResource->vertices =
            (Vec3s*)((u8*)modelState->shadowRenderResource + sizeof(ObjectShadowMesh));
        modelState->shadowRenderResource->vertexCount = triangleCount * 3;
        i = 0;
        kf = 20.0f;
        for (; i < modelState->shadowRenderResource->vertexCount; i++)
        {
            modelState->shadowRenderResource->vertices[i].x = kf * vertices[i].x;
            modelState->shadowRenderResource->vertices[i].y = kf * vertices[i].y;
            modelState->shadowRenderResource->vertices[i].z = kf * vertices[i].z;
        }
    }
    if (modelState->shadowRenderResource != OBJECT_SHADOW_MESH_UNCACHED)
    {
        u32 i;
        GXBegin(GX_TRIANGLES, GX_VTXFMT0, modelState->shadowRenderResource->vertexCount & 0xffff);
        for (i = 0; i < modelState->shadowRenderResource->vertexCount; i++)
        {
            Vec3s* vertex = &modelState->shadowRenderResource->vertices[i];
            s16 z = vertex->z;
            s16 y = vertex->y;
            s16 x = vertex->x;
            GXWGFifo.s16 = x;
            GXWGFifo.s16 = y;
            GXWGFifo.s16 = z;
        }
    }
    else
    {
        int i;
        int w0;
        GXBegin(GX_TRIANGLES, GX_VTXFMT2, (triangleCount * 3) & 0xffff);
        w0 = 0;
        for (i = 0; i < triangleCount; i++)
        {
            int k;
            for (k = 0; k < 3; k++)
            {
                Vec3f* v1 = &vertices[w0 + k];
                f32 b2 = v1->z;
                f32 b1 = v1->y;
                f32 b0 = v1->x;
                GXWGFifo.f32 = b0;
                GXWGFifo.f32 = b1;
                GXWGFifo.f32 = b2;
            }
            w0 += 3;
        }
    }
    if (modelState->flags & 0x20)
    {
        memcpy(&obj->anim.localPos, &savedLocalPos, sizeof(Vec3f));
        memcpy(&obj->anim.worldPos, &savedWorldPos, sizeof(Vec3f));
    }
}

int objShadowFn_80062378(GameObject* obj, u8 param)
{
    int lo;
    int hi;
    f32 inv;
    void* p;

    p = (obj)->anim.modelInstance;
    if (((ObjDef*)p)->renderFlags & OBJDEF_RENDERFLAG_PROJECTED_SHADOW)
    {
        lo = 1000;
        hi = 2000;
    }
    else
    {
        lo = 400;
        hi = 500;
    }
    inv = (Camera_DistanceToCurrentViewPosition((obj)->anim.worldPosX, (obj)->anim.worldPosY, (obj)->anim.worldPosZ) -
           lo) /
          (f32)(hi - lo);
    if (inv < 0.0f)
    {
        inv = 0.0f;
    }
    else if (inv > 1.0f)
    {
        inv = 1.0f;
    }
    inv = 1.0f - inv;
    {
        int n = (int)((f32)param * inv);
        return (n * (*(u8*)((char*)obj + 0x37) + 1)) >> 8;
    }
}

int objShadowFn_80062498(GameObject* obj, int renderMode, int unused, int frameCount)
{
    ObjModelState* modelState;
    u8* cache;
    f32 yOff;
    int idxOut = 0;
    int drawScratch;
    u32* vtx;
    int alphaOut = 0;
    int alpha;
    ObjectShadowMesh* shadowMesh;
    f32 vec[3];
    f32 base[3];
    TrackQueryBounds ranges;
    u8 buf48[96];
    u8 bufA8[304];

    cache = getCache();
    modelState = obj->anim.modelState;
    if ((s32)shouldDrawShadows() == 0)
    {
        obj->anim.modelState->shadowCastSlot = NULL;
        return 0;
    }

    shadowMesh = modelState->shadowRenderResource;
    if (shadowMesh == NULL || shadowMesh == OBJECT_SHADOW_MESH_UNCACHED)
    {
        vec[0] = modelState->shadowOffsetX;
        vec[1] = modelState->shadowOffsetY;
        vec[2] = modelState->shadowOffsetZ;
        buildShadowVolumeBox(vec, (f32*)buf48, modelState->shadowModelScale);

        {
            void* p54 = obj->anim.hitReactState;
            if (p54 != NULL)
            {
                yOff = (f32)((int)((ObjHitsPriorityState*)p54)->primaryCapsuleOffsetB / 2);
            }
            else
            {
                yOff = 0.0f;
            }
        }

        base[0] = obj->anim.worldPosX;
        base[1] = obj->anim.worldPosY + yOff;
        base[2] = obj->anim.worldPosZ;
        vecGetRanges((f32*)buf48, base, modelState->shadowScale, (int*)&ranges);

        hitDetectFn_800691c0(obj, &ranges, 0x81, 0);
        trackGetGridOrigin((int**)&vtx);
        trackGetTriangleBuffer(&idxOut, &alphaOut);

        alpha = alphaOut;
        idxOut = collectShadowTrackTriangles((int*)obj, alpha, gShadowDrawScratch, gShadowVolumeBuffer, idxOut, (f32)(int)vtx[0],
                             (f32)(int)vtx[2], renderMode, modelState->flags & 0x40000);
        lbl_803DCEE0 = alpha;
        lbl_803DCEF0 = idxOut;
        lbl_803DCEE4 = (int)vtx;
        trackDolphin_buildShadowVolumePlanes((int*)obj, buf48, bufA8);
        cullVisibleShadowTriangles(obj, buf48, bufA8, idxOut, (Vec3f*)gShadowVolumeBuffer, (Vec3f*)cache,
                    (TrackShadowTriangle*)gShadowDrawScratch, 0x555);
    }
    objDrawFn_80061f0c((Vec3f*)cache, modelState, obj, gShadowVisibleCount, &drawScratch, buf48, yOff);
    return 0;
}

u8 objShadowUpdateAlpha(GameObject* obj, int delta)
{
    ObjModelState* modelState;
    s16* alphaStep;
    f32 f31;
    int v;

    modelState = obj->anim.modelState;
    alphaStep = &modelState->shadowAlphaStep;
    if (modelState->flags & OBJ_MODEL_STATE_SHADOW_FADE_OUT)
    {
        *alphaStep = *alphaStep - (delta << 9);
        if (*alphaStep <= 0)
        {
            *alphaStep = 0;
        }
        if (*alphaStep == 0)
        {
            modelState->shadowCastSlot = NULL;
            return 0;
        }
    }
    else if (!(modelState->flags & OBJ_MODEL_STATE_SHADOW_ALPHA_HOLD))
    {
        *alphaStep = *alphaStep + (delta << 9);
        if (*alphaStep >= 0x4000)
        {
            *alphaStep = 0x4000;
        }
    }
    f31 = (1.0f / 16384.0f) * (f32)*alphaStep;
    f31 = lbl_803DB654 * f31;
    {
        f32 tint = objShadowFn_80062378(obj, modelState->shadowTintA);
        v = (s16)(int)(tint * f31);
    }
    if (v > 0xff)
    {
        v = 0xff;
    }
    else if (v < 0)
    {
        v = 0;
    }
    return v & 0xff;
}

void shadowVolumeBeginFrame(void)
{
    void* bufPtr;
    s16 zero;
    if ((s8)gShadowVolumesDirty == 0)
    {
        return;
    }
    zero = 0;
    lbl_803DCEF8 = zero;
    lbl_803DCEFC = zero;
    lbl_803DCEF4 = zero;
    lbl_803DCEEC = 1 - lbl_803DCEEC;
    lbl_803DCEED = 1 - lbl_803DCEED;
    lbl_803DCEEE = 1 - lbl_803DCEEE;
    bufPtr = lbl_803DCF24[lbl_803DCEEC];
    lbl_803DCF08 = bufPtr;
    lbl_803DCEF4 = zero;
    lbl_803DCF10 = lbl_803DCF20;
    lbl_803DCF18 = lbl_803DCF1C;
    lbl_803DCF04 = bufPtr;
    lbl_803DCF14 = lbl_803DCF1C;
    lbl_803DCF0C = lbl_803DCF20;
}

void shadowBeginFrame(void)
{
    lbl_803DCEF6 = 0;
    lbl_803DCEFA = 0;
    lbl_803DCEEA = (s8)(1 - lbl_803DCEEA);
    lbl_803DCEEB = (s8)(1 - lbl_803DCEEB);
    lbl_803DCEE9 = 0;
    lbl_803DCEE8 = 0;
}

void objShadowInvalidate(GameObject* obj)
{
    gShadowVolumesDirty = 0x1;
}

void shadowVolumesSetDirty(s32 dirty)
{
    gShadowVolumesDirty = dirty;
}

void* shadowInit(int* obj, int size, int wpad0)
{
    int rounded;
    ObjModelState* modelState;
    s16 texId;

    rounded = roundUpTo4(size);
    *(int*)&((ObjAnimComponent*)obj)->modelState = rounded;
    modelState = ((ObjAnimComponent*)obj)->modelState;
    texId = ((ObjAnimComponent*)obj)->modelInstance->shadowTextureId;
    if (texId != -1 && ((ObjAnimComponent*)obj)->modelInstance->shadowType != OBJ_SHADOW_TYPE_MODEL_GEOMETRIC)
    {
        modelState->shadowTexture = (void*)textureLoad(-texId, 0);
    }
    else if (((ObjAnimComponent*)obj)->modelInstance->renderFlags & OBJDEF_RENDERFLAG_PROJECTED_SHADOW)
    {
        modelState->shadowTexture = (void*)textureAlloc512();
    }
    else if (((ObjAnimComponent*)obj)->modelInstance->renderFlags & 0x2)
    {
        modelState->shadowTexture = NULL;
        modelState->shadowWorkBuffer = NULL;
    }
    else
    {
        modelState->shadowTexture = (void*)getNewShadowSmallDiskTexture();
    }
    if (((ObjAnimComponent*)obj)->modelInstance->shadowType == OBJ_SHADOW_TYPE_BIG_BOX)
    {
        modelState->shadowRenderResource = NULL;
    }
    else
    {
        modelState->shadowRenderResource = OBJECT_SHADOW_MESH_UNCACHED;
    }
    modelState->shadowScale = *(f32*)((ObjAnimComponent*)obj)->modelInstance;
    modelState->shadowModelScale = *(f32*)((char*)((ObjAnimComponent*)obj)->modelInstance + 0x88);
    modelState->shadowOffsetX = gShadowOffsetX;
    modelState->shadowOffsetY = gShadowOffsetY;
    modelState->shadowOffsetZ = gShadowOffsetZ;
    modelState->shadowAlphaStep = 0x4000;
    modelState->flags = OBJ_MODEL_STATE_SHADOW_VISIBLE;
    modelState->pad38[0] = 0x19;
    modelState->pad38[1] = 0x4b;
    modelState->shadowTintA = 0x96;
    modelState->shadowTintB = 0x64;
    gShadowVolumesDirty = 1;
    return (char*)rounded + 0x44;
}

void playerShadowClearPositionOverride(GameObject* obj)
{
    ObjModelState* modelState = obj->anim.modelState;
    if (modelState == NULL)
        return;
    modelState->flags &= ~0x2020;
}

void doNothing_80062A50(GameObject* obj, f32 x, f32 y, f32 z)
{
}

void shadowSetLightDirection(f32 directionX, f32 directionY, f32 directionZ, int magnitude)
{
    f32 normalizedDirection[3];
    f32 directionSimilarity;
    f32 directionMagnitudeSquared;
    f32 previousMagnitudeSquared;
    f32 combinedMagnitudeSquared;

    normalizedDirection[0] = directionX;
    normalizedDirection[1] = directionY;
    normalizedDirection[2] = directionZ;
    PSVECNormalize(normalizedDirection, normalizedDirection);
    gSunMagnitude = magnitude;
    gShadowOffsetX = directionX * magnitude;
    gShadowOffsetY = directionY * magnitude;
    lbl_803DB654 = 1.0f;
    if (gShadowOffsetY < 80.0f)
    {
        gShadowOffsetY = 80.0f;
    }
    gShadowOffsetZ = directionZ * magnitude;
    directionSimilarity = normalizedDirection[0] * gPrevSunDir[0] + normalizedDirection[1] * gPrevSunDir[1] +
                          normalizedDirection[2] * gPrevSunDir[2];
    directionMagnitudeSquared = normalizedDirection[0] * normalizedDirection[0] +
                                normalizedDirection[1] * normalizedDirection[1];
    directionMagnitudeSquared += normalizedDirection[2] * normalizedDirection[2];
    previousMagnitudeSquared = gPrevSunDir[0] * gPrevSunDir[0] + gPrevSunDir[1] * gPrevSunDir[1] +
                               gPrevSunDir[2] * gPrevSunDir[2];
    combinedMagnitudeSquared = directionMagnitudeSquared * previousMagnitudeSquared;
    if (combinedMagnitudeSquared)
    {
        previousMagnitudeSquared = sqrtf(combinedMagnitudeSquared);
    }
    if (previousMagnitudeSquared)
    {
        gSunDotCos = directionSimilarity / previousMagnitudeSquared;
    }
    else
    {
        gSunDotCos = 0.0f;
    }
    if (gSunDotCos < 0.0f)
    {
        gSunDotCos *= -1.0f;
    }
    if (gSunDotCos <= 0.99619f)
    {
        gSunDirChanged = 1;
    }
    if (gSunDirChanged != 0)
    {
        gPrevSunDir[0] = normalizedDirection[0];
        gPrevSunDir[1] = normalizedDirection[1];
        gPrevSunDir[2] = normalizedDirection[2];
        gSunDirChanged = 0;
        gShadowVolumesDirty = 1;
    }
}

void initTextures(void)
{
    f32* a = lbl_8038D77C;
    f32* b = lbl_8038D7DC;

    gShadowVolumesDirty = 10;
    gShadowVolumeBuffer = (int)mmAlloc(0xa8c0, 0x18, 0);
    a[0] = -1.0f;
    b[0] = -1.0f;
    a[1] = -1.0f;
    b[1] = -1.0f;
    a[2] = -1.0f;
    b[2] = -1.0f;
    a[3] = -1.0f;
    b[3] = -1.0f;
    a[4] = 0.0f;
    b[4] = 0.0f;
    a[5] = -1.0f;
    b[5] = -1.0f;
    a[6] = 1.0f;
    b[6] = 1.0f;
    a[7] = 0.0f;
    b[7] = 0.0f;
    a[8] = -1.0f;
    b[8] = -1.0f;
    a[9] = 1.0f;
    b[9] = 1.0f;
    a[10] = -1.0f;
    b[10] = -1.0f;
    a[11] = -1.0f;
    b[11] = -1.0f;
    b[12] = -1.0f;
    b[13] = -1.0f;
    b[14] = 1.0f;
    b[15] = -1.0f;
    b[16] = 0.0f;
    b[17] = 1.0f;
    b[18] = 1.0f;
    b[19] = 0.0f;
    b[20] = 1.0f;
    b[21] = 1.0f;
    b[22] = -1.0f;
    b[23] = 1.0f;
    a[12] = -6.0f;
    a[13] = 0.0f;
    a[14] = 55.0f;
    a[15] = -6.0f;
    a[16] = 16.0f;
    a[17] = 55.0f;
    a[18] = 6.0f;
    a[19] = 16.0f;
    a[20] = 55.0f;
    a[21] = 6.0f;
    a[22] = 0.0f;
    a[23] = 55.0f;
    allocLotsOfTextures();
}
