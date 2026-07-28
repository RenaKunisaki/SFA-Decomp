#include "main/dll/partfx_interface.h"
#include "main/audio/sfx_play_api.h"
#include "main/audio/sfx_stop_channel_api.h"
#include "dolphin/mtx.h"
#include "main/frame_timing.h"
#include "main/lightmap_api.h"
#include "main/lightmap_text_color_api.h"
#include "track/intersect_render_setup_api.h"
#include "track/intersect_geom_api.h"
#include "main/shader_api.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/dll/modgfx_types.h"
#include "main/dll_000A_expgfx.h"
#include "game/objects/object.h"
#include "sys/objects/lifecycle.h"
#include "sys/objects.h"
#include "main/dll/modgfx.h"
#include "main/resource.h"
#include "main/texture.h"
#include "main/mm.h"
#include "main/vecmath.h"
#include "main/camera.h"
#include "main/dll/dll_000B_dll0b.h"
#include "main/obj_list.h"
#include "dolphin/gx/GXEnum.h"
#include "main/render_mode_api.h"
#include "main/sky.h"
#include "dolphin/gx/GXLegacyDecls.h"
#include "track/intersect_api.h"
#include "string.h"

typedef union Dll0BDescriptorTable
{
    u32 words[30];
    u64 align8;
} Dll0BDescriptorTable;

ModgfxPendingSpawn* gModgfxPendingSpawnStartCursor;
ModgfxPendingSpawn* gModgfxPendingSpawnWriteCursor;
s16 gModgfxSequenceParamIndex;
s16 gModgfxLastSpawnHandle;
f32 gModgfxMotionStep;
u8 lbl_803DD282;
s16 gPartfxSequenceIdCounter;

#define DLL0B_OBJFLAG_RENDERED 0x800

/* Object spawned to back a modgfx effect slot; retail OBJECTS.bin name
   "InvHit" (DLL 0xF1). */
#define DLL0B_CHILD_OBJ_INVHIT 0x66

typedef struct ModgfxEffectSlot
{
    u8 pad0[0x4 - 0x0];
    void* sourceObj;
    u8 pad8[0xC - 0x8];
    s16 unkC;
    u8 padE[0x18 - 0xE];
    f32 posOffsetX;
    f32 posOffsetY;
    f32 posOffsetZ;
    f32 motionOffsetX;
    f32 motionOffsetY;
    f32 motionOffsetZ;
    u8 pad30[0x60 - 0x30];
    f32 posCurX;
    f32 posCurY;
    f32 posCurZ;
    u8 pad6C[0x9C - 0x6C];
    void* unk9C;
    u8 padA0[0xA4 - 0xA0];
    s32 sourceFlags;
    u8 padA8[0xBC - 0xA8];
    f32 alphaDelta;
    f32 alphaCurrent;
    u8 padC4[0xEE - 0xC4];
    s16 frameTimings[7];
    s16 frameIndex;
    s16 frameDuration;
    u8 pad100[0x106 - 0x100];
    s16 rotOffsetZ;
    s16 rotOffsetY;
    s16 rotOffsetX;
    s16 animSlotId;
    u8 pad10E[0x139 - 0x10E];
    s8 emitterCount;
    u8 unk13A;
    u8 pad13B[0x13C - 0x13B];
    u8 pendingFrameIdx;
    u8 pad13D[0x13E - 0x13D];
    u8 unk13E;
    u8 pad13F[0x140 - 0x13F];
} ModgfxEffectSlot;

STATIC_ASSERT(offsetof(ModgfxState, vertexBuffers) == 0x78);
STATIC_ASSERT(offsetof(ModgfxState, alphaValues) == 0xAC);
STATIC_ASSERT(offsetof(ModgfxState, blendColorR) == 0xBC);
STATIC_ASSERT(offsetof(ModgfxState, vertexCount) == 0xEA);
STATIC_ASSERT(offsetof(ModgfxState, posCurX) == 0x60);
STATIC_ASSERT(offsetof(ModgfxState, activeChannel) == 0xFC);
STATIC_ASSERT(offsetof(ModgfxState, rotStepZ) == 0x100);
STATIC_ASSERT(offsetof(ModgfxState, rotOffsetZ) == 0x106);

#define PARTFX_ACTIVE_EFFECT_COUNT 0x32

STATIC_ASSERT(sizeof(ModgfxSpawnContext) == 0x60);
STATIC_ASSERT(offsetof(ModgfxSpawnContext, vecX) == 0x20);
STATIC_ASSERT(offsetof(ModgfxSpawnContext, posX) == 0x2C);
STATIC_ASSERT(offsetof(ModgfxSpawnContext, sequenceParams) == 0x46);
STATIC_ASSERT(offsetof(ModgfxSpawnContext, flags) == 0x54);
STATIC_ASSERT(offsetof(ModgfxSpawnContext, pendingSpawnCount) == 0x5D);

STATIC_ASSERT(sizeof(PartfxEffectState) == 0x140);
STATIC_ASSERT(offsetof(PartfxEffectState, vertexBuffers) == 0x78);
STATIC_ASSERT(offsetof(PartfxEffectState, textureResource) == 0x98);
STATIC_ASSERT(offsetof(PartfxEffectState, flags) == 0xA4);
STATIC_ASSERT(offsetof(PartfxEffectState, drawPosX) == 0x60);
STATIC_ASSERT(offsetof(PartfxEffectState, velocityX) == 0x6C);
STATIC_ASSERT(offsetof(PartfxEffectState, alphaValues) == 0xAC);
STATIC_ASSERT(offsetof(PartfxEffectState, blendColorR) == 0xBC);
STATIC_ASSERT(offsetof(PartfxEffectState, renderScale) == 0xD4);
STATIC_ASSERT(offsetof(PartfxEffectState, vertexCount) == 0xEA);
STATIC_ASSERT(offsetof(PartfxEffectState, colorVertexCount) == 0xEC);
STATIC_ASSERT(offsetof(PartfxEffectState, stageDurations) == 0xEE);
STATIC_ASSERT(offsetof(PartfxEffectState, sequenceId) == 0x10C);
STATIC_ASSERT(offsetof(PartfxEffectState, inlineData) == 0x12C);
STATIC_ASSERT(offsetof(PartfxEffectState, activeVertexBufferIndex) == 0x130);
STATIC_ASSERT(offsetof(PartfxEffectState, emitterCount) == 0x139);
STATIC_ASSERT(offsetof(PartfxEffectState, textureIsBorrowed) == 0x13F);

u8 gModgfxSpawnContextStorage[0x60];
ModgfxPendingSpawn gModgfxPendingSpawnQueue[0x300 / sizeof(ModgfxPendingSpawn)];
void partfx_freeEffectsBySequence(s16 a, int b);
#define MODGFX_ZERO 0.0f
#define MODGFX_ONE  1.0f

s16 dll_0B_spawnEffect(ModgfxSpawnContext* st, int unused, int c, s16* b, int e, s16* d, int textureAssetId,
                  void* textureResource);

#define gModgfxSpawnContext (*(ModgfxSpawnContext*)gModgfxSpawnContextStorage)
s16 dll_0B_getLastSpawnHandle(void)
{
    return gModgfxLastSpawnHandle;
}

void dll_0B_addSequenceFlags(u32 flags)
{
    gModgfxSpawnContext.flags |= flags;
}

void dll_0B_spawnSequence(void* a, void* b, void* c, void* d, void* e, int f, void* g)
{
    gModgfxSpawnContext.pendingSpawns = gModgfxPendingSpawnQueue;
    gModgfxSpawnContext.pendingSpawnCount = gModgfxPendingSpawnWriteCursor - gModgfxPendingSpawnStartCursor;
    if (g == NULL && f == 0)
    {
        gModgfxSpawnContext.flags |= 0x2000000LL;
    }
    else
    {
        gModgfxSpawnContext.flags |= 0x4000000LL;
    }
    if (gModgfxSpawnContext.flags & 1)
    {
        if (gModgfxSpawnContext.attachedSource != NULL)
        {
            gModgfxSpawnContext.posX += ((ObjAnimComponent*)gModgfxSpawnContext.attachedSource)->worldPosX;
            gModgfxSpawnContext.posY += ((ObjAnimComponent*)gModgfxSpawnContext.attachedSource)->worldPosY;
            gModgfxSpawnContext.posZ += ((ObjAnimComponent*)gModgfxSpawnContext.attachedSource)->worldPosZ;
        }
        else
        {
            gModgfxSpawnContext.posX += ((ObjAnimComponent*)a)->localPosX;
            gModgfxSpawnContext.posY += ((ObjAnimComponent*)a)->localPosY;
            gModgfxSpawnContext.posZ += ((ObjAnimComponent*)a)->localPosZ;
        }
    }
    gModgfxLastSpawnHandle = dll_0B_spawnEffect(&gModgfxSpawnContext, 0, (int)c, b, (int)e, d, f, g);
}

void dll_0B_setSequenceParams(void* params)
{
    memcpy(gModgfxSpawnContext.sequenceParams, params, 0xe);
}

void dll_0B_setSequenceParamValue(s16 value)
{
    u8* state = gModgfxSpawnContextStorage;
    state = state + gModgfxSequenceParamIndex * 2;
    *(s16*)(state + 0x46) = value;
}

void dll_0B_setSequenceParamIndex(s16 x)
{
    gModgfxSequenceParamIndex = x;
}

void dll_0B_nextSequenceParam(void)
{
    gModgfxSequenceParamIndex++;
}

void dll_0B_addSequenceSpawn(int modelOrResource, float posX, float posY, float posZ, s16 param14, int param10)
{
    u32 sequenceIndex = gModgfxSequenceParamIndex;
    gModgfxPendingSpawnWriteCursor->sequenceIndex = sequenceIndex;
    gModgfxPendingSpawnWriteCursor->param14 = param14;
    gModgfxPendingSpawnWriteCursor->param10 = param10;
    gModgfxPendingSpawnWriteCursor->modelOrResource = modelOrResource;
    gModgfxPendingSpawnWriteCursor->posX = posX;
    gModgfxPendingSpawnWriteCursor->posY = posY;
    gModgfxPendingSpawnWriteCursor->posZ = posZ;
    gModgfxPendingSpawnWriteCursor++;
}

void dll_0B_resetSequenceSpawns(void)
{
    ModgfxPendingSpawn* cursor = gModgfxPendingSpawnQueue;
    gModgfxPendingSpawnStartCursor = cursor;
    gModgfxPendingSpawnWriteCursor = cursor;
    gModgfxSequenceParamIndex = 0;
}

void dll_0B_beginSequence(int source, u8 mode, u8 flagByte, int word40, int word3C)
{
    f32 fz;
    f32 fz2;
    memset(&gModgfxSpawnContext, 0, sizeof(gModgfxSpawnContext));
    gModgfxSpawnContext.modeByte = mode;
    gModgfxSpawnContext.attachedSource = (void*)source;
    gModgfxSpawnContext.sourceModeCopy = mode;
    fz = MODGFX_ZERO;
    gModgfxSpawnContext.posX = fz;
    gModgfxSpawnContext.posY = fz;
    gModgfxSpawnContext.posZ = fz;
    gModgfxSpawnContext.vecX = fz;
    gModgfxSpawnContext.vecY = fz;
    gModgfxSpawnContext.vecZ = fz;
    fz2 = MODGFX_ONE;
    gModgfxSpawnContext.scale = fz2;
    gModgfxSpawnContext.drawGroupCount = word40;
    gModgfxSpawnContext.drawGroupStride = word3C;
    gModgfxSpawnContext.initialStateByte = flagByte;
    gModgfxSpawnContext.byte5A = 0;
    gModgfxSpawnContext.textureFrameTimer = 0;
}

#define GX_CULL_NONE  0
#define GX_CULL_FRONT 1

/* Per-bone particle vertex update + draw. */

void modgfx_scrollTexCoords(ModgfxState* state, f32* in)
{
    int i;
    s32 dy, dx;
    ModgfxVertexData* slot;
    ModgfxVertexData* cur;
    ModgfxVertexData* prev;
    u8 ovx, ovy;
    int j;

    dx = (s32)(4.0f * (in[1] * gModgfxMotionStep));
    dy = (s32)(4.0f * (in[2] * gModgfxMotionStep));

    cur = state->vertexBuffers[state->activeVertexBufferIndex];
    prev = state->vertexBuffers[1 - state->activeVertexBufferIndex];

    ovx = 0;
    ovy = 0;
    for (i = 0; i < state->vertexCount; i++)
    {
        cur->texCoordS = prev->texCoordS;
        cur->texCoordT = prev->texCoordT;
        cur->texCoordS = (s16)(cur->texCoordS + dx);
        if ((s32)cur->texCoordS > 0x100)
            ovx++;
        if ((s32)cur->texCoordS < -0x100)
            ovx++;
        cur->texCoordT = (s16)(cur->texCoordT + dy);
        if ((s32)cur->texCoordT > 0x100)
            ovy++;
        if ((s32)cur->texCoordT < -0x100)
            ovy++;
        cur++;
        prev++;
    }

    slot = state->vertexBuffers[state->activeVertexBufferIndex];
    for (j = 0; j < state->vertexCount; j++)
    {
        if ((s32)ovx == state->vertexCount)
        {
            if ((s32)slot->texCoordS > 0x100)
            {
                slot->texCoordS -= 0x100;
            }
            else
            {
                slot->texCoordS += 0x100;
            }
        }
        if ((s32)ovy == state->vertexCount)
        {
            if ((s32)slot->texCoordT > 0x100)
            {
                slot->texCoordT -= 0x100;
            }
            else
            {
                slot->texCoordT += 0x100;
            }
        }
        slot++;
    }
}

void* gPartfxActiveEffects[0x32];

void modgfx_captureFrameBaseVertices(ModgfxState* state)
{
    int i;
    ModgfxVertexData* dst;
    ModgfxVertexData* src;
    f32 f1;
    f32 f0;
    src = state->vertexBuffers[1 - state->activeVertexBufferIndex];
    dst = state->baseVertexData;
    for (i = 0; i < state->vertexCount; i++)
    {
        dst->posX = src->posX;
        dst->posY = src->posY;
        dst->posZ = src->posZ;
        dst->colorR = src->colorR;
        dst->colorG = src->colorG;
        dst->colorB = src->colorB;
        dst->alpha = src->alpha;
        dst++;
        src++;
    }
    f1 = MODGFX_ONE;
    state->scaleVectors[0].x = f1;
    state->scaleVectors[0].y = f1;
    state->scaleVectors[0].z = f1;
    f0 = MODGFX_ZERO;
    state->scaleVectors[1].x = f0;
    state->scaleVectors[1].y = f0;
    state->scaleVectors[1].z = f0;
    state->scaleVectors[2].x = f1;
    state->scaleVectors[2].y = f1;
    state->scaleVectors[2].z = f1;
    state->scaleVectors[3].x = f0;
    state->scaleVectors[3].y = f0;
    state->scaleVectors[3].z = f0;
}

void modgfx_stepVertexColor(void* state, void* p, int reinit)
{
    u8* buf = ((u8**)((char*)state + 0x78))[((ModgfxState*)state)->activeVertexBufferIndex];
    int j;

    if (reinit == 1)
    {
        f32 tr = ((ModgfxVertexGroupCmd*)p)->valueX;
        f32 tg = ((ModgfxVertexGroupCmd*)p)->valueY;
        f32 tb = ((ModgfxVertexGroupCmd*)p)->valueZ;
        if (((ModgfxState*)state)->blendFrameCount != 0)
        {
            ((ModgfxState*)state)->blendColorR = (f32)(u32)buf[(((ModgfxVertexGroupCmd*)p)->indices)[0] * 16 + 0xc];
            ((ModgfxState*)state)->blendColorG = (f32)(u32)buf[(((ModgfxVertexGroupCmd*)p)->indices)[0] * 16 + 0xd];
            ((ModgfxState*)state)->blendColorB = (f32)(u32)buf[(((ModgfxVertexGroupCmd*)p)->indices)[0] * 16 + 0xe];
            ((ModgfxState*)state)->blendColorStepR =
                (tr - (f32)(u32)buf[(((ModgfxVertexGroupCmd*)p)->indices)[0] * 16 + 0xc]) / (f32) ((ModgfxState*)state)->blendFrameCount;
            ((ModgfxState*)state)->blendColorStepG =
                (tg - (f32)(u32)buf[(((ModgfxVertexGroupCmd*)p)->indices)[0] * 16 + 0xd]) / (f32) ((ModgfxState*)state)->blendFrameCount;
            ((ModgfxState*)state)->blendColorStepB =
                (tb - (f32)(u32)buf[(((ModgfxVertexGroupCmd*)p)->indices)[0] * 16 + 0xe]) / (f32) ((ModgfxState*)state)->blendFrameCount;
        }
        else
        {
            ((ModgfxState*)state)->blendColorR = tr;
            ((ModgfxState*)state)->blendColorG = tg;
            ((ModgfxState*)state)->blendColorB = tb;
            {
                f32 z = MODGFX_ZERO;
                ((ModgfxState*)state)->blendColorStepR = z;
                ((ModgfxState*)state)->blendColorStepG = z;
                ((ModgfxState*)state)->blendColorStepB = z;
            }
        }
    }
    ((ModgfxState*)state)->blendColorR += ((ModgfxState*)state)->blendColorStepR;
    ((ModgfxState*)state)->blendColorG += ((ModgfxState*)state)->blendColorStepG;
    ((ModgfxState*)state)->blendColorB += ((ModgfxState*)state)->blendColorStepB;
    if (((ModgfxState*)state)->blendColorR < MODGFX_ZERO)
    {
        ((ModgfxState*)state)->blendColorR = MODGFX_ZERO;
    }
    else if (((ModgfxState*)state)->blendColorR > 255.0f)
    {
        ((ModgfxState*)state)->blendColorR = 255.0f;
    }
    if (((ModgfxState*)state)->blendColorG < MODGFX_ZERO)
    {
        ((ModgfxState*)state)->blendColorG = MODGFX_ZERO;
    }
    else if (((ModgfxState*)state)->blendColorG > 255.0f)
    {
        ((ModgfxState*)state)->blendColorG = 255.0f;
    }
    if (((ModgfxState*)state)->blendColorB < MODGFX_ZERO)
    {
        ((ModgfxState*)state)->blendColorB = MODGFX_ZERO;
    }
    else if (((ModgfxState*)state)->blendColorB > 255.0f)
    {
        ((ModgfxState*)state)->blendColorB = 255.0f;
    }
    for (j = 0; j < ((ModgfxVertexGroupCmd*)p)->indexCount; j++)
    {
        buf[(((ModgfxVertexGroupCmd*)p)->indices)[j] * 16 + 0xc] = (int)((ModgfxState*)state)->blendColorR;
        buf[(((ModgfxVertexGroupCmd*)p)->indices)[j] * 16 + 0xd] = (int)((ModgfxState*)state)->blendColorG;
        buf[(((ModgfxVertexGroupCmd*)p)->indices)[j] * 16 + 0xe] = (int)((ModgfxState*)state)->blendColorB;
    }
}

void modgfx_stepPosition(int state, int cmd, int reinit)
{

    if (reinit == 1)
    {
        s16* cf = ((ModgfxState*)state)->channelFrames;
        if (cf[((ModgfxState*)state)->activeChannel] == 0)
        {
            int flags = ((ModgfxState*)state)->flags;
            if ((flags & 0x4) != 0 || (flags & 0x80000) != 0)
            {
                s16 buf[12];
                f32* fbuf = (f32*)&buf[4];
                s16 posBase;
                f32 fill = MODGFX_ZERO;
                fbuf[1] = fill;
                fbuf[2] = fill;
                fbuf[3] = fill;
                fbuf[0] = MODGFX_ONE;
                posBase = *((ModgfxState*)state)->unk04;
                buf[0] = posBase;
                buf[1] = posBase;
                buf[2] = posBase;
                vecRotateZXY(buf, (f32*)(cmd + 0x4));
            }
            ((ModgfxState*)state)->posStepX = ((ModgfxVertexGroupCmd*)cmd)->valueX;
            ((ModgfxState*)state)->posStepY = ((ModgfxVertexGroupCmd*)cmd)->valueY;
            ((ModgfxState*)state)->posStepZ = ((ModgfxVertexGroupCmd*)cmd)->valueZ;
        }
        else
        {
            ((ModgfxState*)state)->posStepX =
                ((ModgfxVertexGroupCmd*)cmd)->valueX / (f32)(s32)((ModgfxState*)state)->blendFrameCount;
            ((ModgfxState*)state)->posStepY =
                ((ModgfxVertexGroupCmd*)cmd)->valueY / (f32)(s32)((ModgfxState*)state)->blendFrameCount;
            ((ModgfxState*)state)->posStepZ =
                ((ModgfxVertexGroupCmd*)cmd)->valueZ / (f32)(s32)((ModgfxState*)state)->blendFrameCount;
        }
        ((ModgfxState*)state)->posCurX = ((ModgfxState*)state)->posCurX + ((ModgfxState*)state)->posStepX;
        ((ModgfxState*)state)->posCurY = ((ModgfxState*)state)->posCurY + ((ModgfxState*)state)->posStepY;
        ((ModgfxState*)state)->posCurZ = ((ModgfxState*)state)->posCurZ + ((ModgfxState*)state)->posStepZ;
    }
    else
    {
        ((ModgfxState*)state)->posCurX =
            ((ModgfxState*)state)->posStepX * gModgfxMotionStep + ((ModgfxState*)state)->posCurX;
        ((ModgfxState*)state)->posCurY =
            ((ModgfxState*)state)->posStepY * gModgfxMotionStep + ((ModgfxState*)state)->posCurY;
        ((ModgfxState*)state)->posCurZ =
            ((ModgfxState*)state)->posStepZ * gModgfxMotionStep + ((ModgfxState*)state)->posCurZ;
    }
}

/* Integer-vector lerp setup. On the reinit step, snap or step-interpolate the rotation offset triple
 * toward the rounded params, then advance it by the per-step delta. */
void modgfx_stepS16VectorLerp(ModgfxState* state, f32* params, int reinit)
{
    if (reinit == 1)
    {
        s16 tx = params[1];
        s16 ty = params[2];
        s16 tz = params[3];
        if (state->blendFrameCount != 0)
        {
            state->rotStepZ = (s16)((tx - state->rotOffsetZ) / state->blendFrameCount);
            state->rotStepY = (s16)((ty - state->rotOffsetY) / state->blendFrameCount);
            state->rotStepX = (s16)((tz - state->rotOffsetX) / state->blendFrameCount);
        }
        else
        {
            state->rotOffsetZ = tx;
            state->rotStepZ = 0;
            state->rotOffsetY = ty;
            state->rotStepY = 0;
            state->rotOffsetX = tz;
            state->rotStepX = 0;
        }
    }
    state->rotOffsetZ += state->rotStepZ;
    state->rotOffsetY += state->rotStepY;
    state->rotOffsetX += state->rotStepX;
}

void modgfx_stepVertexAlpha(ModgfxState* state, ModgfxVertexGroupCmd* command, int reinit, u8 channelIndex)
{
    int alphaIndex = channelIndex * 2;
    ModgfxVertexData* vertices = state->vertexBuffers[state->activeVertexBufferIndex];
    ModgfxVertexData* baseVertices = state->baseVertexData;
    int i;

    if (reinit == 1)
    {
        f32 target = command->valueX;
        s16 frames = state->blendFrameCount;

        if (frames != 0)
        {
            state->alphaValues[alphaIndex] =
                (target - (f32)baseVertices[command->indices[0]].alpha) / frames;
            state->alphaValues[alphaIndex + 1] = (f32)baseVertices[command->indices[0]].alpha;
        }
        else
        {
            for (i = 0; i < command->indexCount; i++)
            {
                baseVertices[command->indices[i]].alpha = target;
                vertices[command->indices[i]].alpha = baseVertices[command->indices[i]].alpha;
            }
            return;
        }
    }

    state->alphaValues[alphaIndex + 1] += state->alphaValues[alphaIndex] * gModgfxMotionStep;
    if (state->alphaValues[alphaIndex + 1] < 0.0f)
    {
        state->alphaValues[alphaIndex + 1] = 0.0f;
    }
    else if (state->alphaValues[alphaIndex + 1] > 255.0f)
    {
        state->alphaValues[alphaIndex + 1] = 255.0f;
    }

    for (i = 0; i < command->indexCount; i++)
    {
        vertices[command->indices[i]].alpha = state->alphaValues[alphaIndex + 1];
        baseVertices[command->indices[i]].alpha = vertices[command->indices[i]].alpha;
    }
}

void modgfx_stepVertexScale(ModgfxState* state, ModgfxVertexGroupCmd* command, int reinit, u8 channelIndex)
{
    int scaleIndex = channelIndex * 2;
    int i;
    ModgfxVertexData* vertices;
    ModgfxVertexData* baseVertices;

    if (reinit == 1)
    {
        f32 targetX = command->valueX;
        f32 targetY = command->valueY;
        f32 targetZ = command->valueZ;

        if (state->blendFrameCount != 0)
        {
            state->scaleVectors[scaleIndex + 1].x =
                (targetX - state->scaleVectors[scaleIndex].x) / (f32)state->blendFrameCount;
            state->scaleVectors[scaleIndex + 1].y =
                (targetY - state->scaleVectors[scaleIndex].y) / (f32)state->blendFrameCount;
            state->scaleVectors[scaleIndex + 1].z =
                (targetZ - state->scaleVectors[scaleIndex].z) / (f32)state->blendFrameCount;
        }
        else
        {
            baseVertices = state->baseVertexData;
            vertices = state->vertexBuffers[state->activeVertexBufferIndex];

            for (i = 0; i < command->indexCount; i++)
            {
                baseVertices[command->indices[i]].posX *= targetX;
                baseVertices[command->indices[i]].posY *= targetY;
                baseVertices[command->indices[i]].posZ *= targetZ;
                vertices[command->indices[i]].posX = baseVertices[command->indices[i]].posX;
                vertices[command->indices[i]].posY = baseVertices[command->indices[i]].posY;
                vertices[command->indices[i]].posZ = baseVertices[command->indices[i]].posZ;
            }
            return;
        }
    }

    state->scaleVectors[scaleIndex].x += state->scaleVectors[scaleIndex + 1].x * gModgfxMotionStep;
    state->scaleVectors[scaleIndex].y += state->scaleVectors[scaleIndex + 1].y * gModgfxMotionStep;
    state->scaleVectors[scaleIndex].z += state->scaleVectors[scaleIndex + 1].z * gModgfxMotionStep;

    {
        baseVertices = state->baseVertexData;
        vertices = state->vertexBuffers[state->activeVertexBufferIndex];

        for (i = 0; i < command->indexCount; i++)
        {
            if (state->scaleVectors[scaleIndex].x != 1.0f)
            {
                vertices[command->indices[i]].posX =
                    state->scaleVectors[scaleIndex].x * baseVertices[command->indices[i]].posX;
            }
            if (state->scaleVectors[scaleIndex].y != 1.0f)
            {
                vertices[command->indices[i]].posY =
                    state->scaleVectors[scaleIndex].y * baseVertices[command->indices[i]].posY;
            }
            if (state->scaleVectors[scaleIndex].z != 1.0f)
            {
                vertices[command->indices[i]].posZ =
                    state->scaleVectors[scaleIndex].z * baseVertices[command->indices[i]].posZ;
            }
        }
    }
}

void modgfx_restoreBaseVertices(ModgfxState* state)
{
    int i;
    ModgfxVertexData* src;
    ModgfxVertexData* dst = state->vertexBuffers[state->activeVertexBufferIndex];
    src = state->baseVertexData;
    for (i = 0; i < state->vertexCount; i++)
    {
        dst->posX = src->posX;
        dst->posY = src->posY;
        dst->posZ = src->posZ;
        dst->colorR = src->colorR;
        dst->colorG = src->colorG;
        dst->colorB = src->colorB;
        dst->alpha = src->alpha;
        dst++;
        src++;
    }
}

void partfx_freeEffectsBySequence(s16 sequenceId, int forceAll)
{
    PartfxEffectState** arr = (PartfxEffectState**)gPartfxActiveEffects;
    int i;
    for (i = 0; i < PARTFX_ACTIVE_EFFECT_COUNT; i++)
    {
        if (arr[i] == NULL)
            continue;
        if (sequenceId != arr[i]->sequenceId && forceAll == 0)
            continue;
        if (arr[i]->auxAllocation != NULL)
        {
            mm_free(arr[i]->auxAllocation);
        }
        if (arr[i]->instanceObject != NULL)
        {
            Obj_FreeObject(arr[i]->instanceObject);
        }
        arr[i]->inlineData = NULL;
        if (arr[i]->textureIsBorrowed == 0 && arr[i]->textureResource != NULL)
        {
            textureFree((Texture*)(arr[i]->textureResource));
        }
        if (arr[i]->textureIsBorrowed == 0)
        {
            arr[i]->textureResource = NULL;
        }
        mm_free(arr[i]);
        arr[i] = NULL;
    }
}
/* Flag every active effect whose owner object has the 0x800 state bit
 * by setting its frameUpdated flag. */
void dll_0B_markSourceFrameUpdated(void)
{
    PartfxEffectState* effect;
    GameObject* sourceObject;
    int i;
    PartfxEffectState** effects = (PartfxEffectState**)gPartfxActiveEffects;

    for (i = 0; i < PARTFX_ACTIVE_EFFECT_COUNT; i++)
    {
        effect = effects[i];
        if (effect != NULL)
        {
            sourceObject = effect->sourceObject;
            if (sourceObject != NULL && (sourceObject->objectFlags & DLL0B_OBJFLAG_RENDERED) != 0)
            {
                effect->frameUpdated = 1;
            }
        }
    }
}

void dll_0B_func0D(void* source)
{
    PartfxEffectState** arr = (PartfxEffectState**)gPartfxActiveEffects;
    int i;
    for (i = 0; i < PARTFX_ACTIVE_EFFECT_COUNT; i++)
    {
        if (arr[i] != NULL && arr[i]->sourceObject == source)
        {
            arr[i]->releaseRequested = 1;
        }
    }
}

void dll_0B_func0C(void* source, char value)
{
    PartfxEffectState** arr = (PartfxEffectState**)gPartfxActiveEffects;
    int i;
    for (i = 0; i < PARTFX_ACTIVE_EFFECT_COUNT; i++)
    {
        if (arr[i] != NULL && arr[i]->sourceObject == source)
        {
            arr[i]->byte13B = value;
        }
    }
}
void dll_0B_func0B(void)
{
    lbl_803DD282++;
}

void dll_0B_releaseHandle(s16* p)
{
    PartfxEffectState** arr = (PartfxEffectState**)gPartfxActiveEffects;
    int i;
    for (i = 0; i < PARTFX_ACTIVE_EFFECT_COUNT; i++)
    {
        if (arr[i] != NULL && *p == arr[i]->sequenceId)
        {
            arr[i]->releaseRequested = 1;
        }
    }
    *p = -1;
}

int dll_0B_renderEffects(void* a0, int a1, int a2, u8 a3, void* a4)
{
    u8 ar;
    u8 ag;
    u8 ab;
    f32 pos[3];
    f32 rot[3];
    MatrixTransform xf;
    f32 mtxB[16];
    f32 mtxA[12];
    int** p;
    int slot;
    CameraViewSlot* view;
    u8 texCount;
    void* buf1;
    void* buf2;
    u8 aligned;
    void* tex;
    int n131p1;
    int n131;
    int j;
    f32 dirX;
    f32 dirZ;
    f32 dscale;

    n131p1 = 0;
    n131 = 0;
    if (a4 != NULL)
    {
        getAmbientColor(((GameObject*)a4)->lightColorSlot, &ar, &ag, &ab);
    }
    else
    {
        getAmbientColor(0, &ar, &ag, &ab);
    }
    GXSetCullMode(GX_CULL_NONE);
    if (renderModeSetOrGet(-1) == 1)
    {
        return 1;
    }
    view = Camera_GetCurrentViewSlot();
    p = (int**)gPartfxActiveEffects;
    for (slot = 0; slot < PARTFX_ACTIVE_EFFECT_COUNT; slot++)
    {
        if (p[slot] == NULL)
            continue;
        if (((PartfxEffectState*)p[slot])->sequenceId == -1)
            continue;
        if (a3)
        {
            if (((int)((PartfxEffectState*)p[slot])->flags & 0x2000) == 0)
                continue;
        }
        if (a3)
        {
            if (((PartfxEffectState*)p[slot])->sourceObject != a4)
                continue;
        }
        if (!a3)
        {
            if ((int)((PartfxEffectState*)p[slot])->flags & 0x2000)
                continue;
        }
        if ((int)((PartfxEffectState*)p[slot])->flags & 0x800)
        {
            ((PartfxEffectState*)p[slot])->frameUpdated = 0;
        }
        aligned = 0;
        buf1 = ((PartfxEffectState*)p[slot])->vertexBuffers[((PartfxEffectState*)p[slot])->activeVertexBufferIndex];
        buf2 = ((PartfxEffectState*)p[slot])->colorBuffers[((PartfxEffectState*)p[slot])->activeVertexBufferIndex];
        xf.x = MODGFX_ZERO;
        xf.y = MODGFX_ZERO;
        xf.z = MODGFX_ZERO;
        xf.scale = MODGFX_ONE;
        xf.rotZ = 0;
        xf.rotY = 0;
        pos[0] = ((PartfxEffectState*)p[slot])->drawPosX;
        pos[1] = ((PartfxEffectState*)p[slot])->drawPosY;
        pos[2] = ((PartfxEffectState*)p[slot])->drawPosZ;
        if ((int)((PartfxEffectState*)p[slot])->flags & 0x4)
        {
            if (MODGFX_ZERO == pos[2] + (pos[0] + pos[1]))
            {
                aligned = 1;
            }
        }
        if ((int)((PartfxEffectState*)p[slot])->flags & 0x4)
        {
            if (!aligned)
            {
                if (((PartfxEffectState*)p[slot])->sourceObject != NULL)
                {
                    xf.rotX = ((GameObject*)((PartfxEffectState*)p[slot])->sourceObject)->anim.rotX;
                    xf.rotY = ((GameObject*)((PartfxEffectState*)p[slot])->sourceObject)->anim.rotY;
                    xf.rotZ = ((GameObject*)((PartfxEffectState*)p[slot])->sourceObject)->anim.rotZ;
                    vecRotateZXY(&xf.rotX, &pos[0]);
                }
            }
        }
        rot[0] = MODGFX_ZERO;
        rot[1] = MODGFX_ZERO;
        rot[2] = MODGFX_ZERO;
        if (((int)((PartfxEffectState*)p[slot])->flags & 1) == 0)
        {
            if (((PartfxEffectState*)p[slot])->sourceObject != NULL)
            {
                rot[0] = ((GameObject*)((PartfxEffectState*)p[slot])->sourceObject)->anim.worldPosX;
                rot[1] = ((GameObject*)((PartfxEffectState*)p[slot])->sourceObject)->anim.worldPosY;
                rot[2] = ((GameObject*)((PartfxEffectState*)p[slot])->sourceObject)->anim.worldPosZ;
            }
            else
            {
                rot[0] = ((PartfxEffectState*)p[slot])->sourcePosX;
                rot[1] = ((PartfxEffectState*)p[slot])->sourcePosY;
                rot[2] = ((PartfxEffectState*)p[slot])->sourcePosZ;
                Obj_RotateLocalOffsetByYaw(&((PartfxEffectState*)p[slot])->sourcePosX, &rot[0],
                                           ((PartfxEffectState*)p[slot])->sourceYawIndex);
            }
        }
        if (rot[0] > 65534.0f || rot[0] < -65534.0f)
        {
            rot[0] = -playerMapOffsetX;
        }
        if (rot[1] > 65534.0f || rot[1] < -65534.0f)
        {
            rot[1] = MODGFX_ZERO;
        }
        if (rot[2] > 65534.0f || rot[2] < -65534.0f)
        {
            rot[2] = -playerMapOffsetZ;
        }
        xf.x = rot[0] + pos[0];
        xf.y = rot[1] + pos[1];
        xf.z = rot[2] + pos[2];
        if ((int)((PartfxEffectState*)p[slot])->flags & 0x400000)
        {
            dscale = 0.5f * ((PartfxEffectState*)p[slot])->renderScale;
            xf.scale = dscale + dscale / randomGetRange(1, 10);
        }
        else
        {
            xf.scale = 0.01f * ((PartfxEffectState*)p[slot])->renderScale;
        }
        if ((int)((PartfxEffectState*)p[slot])->flags & 0x80000)
        {
            xf.rotZ = ((GameObject*)((PartfxEffectState*)p[slot])->sourceObject)->anim.rotZ;
            xf.rotY = ((GameObject*)((PartfxEffectState*)p[slot])->sourceObject)->anim.rotY;
            xf.rotX = ((GameObject*)((PartfxEffectState*)p[slot])->sourceObject)->anim.rotX;
        }
        else if (aligned && ((PartfxEffectState*)p[slot])->sourceObject != NULL)
        {
            xf.rotZ = ((PartfxEffectState*)p[slot])->rotOffsetZ +
                        ((GameObject*)((PartfxEffectState*)p[slot])->sourceObject)->anim.rotZ;
            xf.rotY = ((PartfxEffectState*)p[slot])->rotOffsetY +
                        ((GameObject*)((PartfxEffectState*)p[slot])->sourceObject)->anim.rotY;
            xf.rotX = ((PartfxEffectState*)p[slot])->rotOffsetX +
                        ((GameObject*)((PartfxEffectState*)p[slot])->sourceObject)->anim.rotX;
        }
        else if (aligned)
        {
            xf.rotZ = ((PartfxEffectState*)p[slot])->rotOffsetZ + ((PartfxEffectState*)p[slot])->sourceRotZ;
            xf.rotY = ((PartfxEffectState*)p[slot])->rotOffsetY + ((PartfxEffectState*)p[slot])->sourceRotY;
            xf.rotX = ((PartfxEffectState*)p[slot])->rotOffsetX + ((PartfxEffectState*)p[slot])->sourceRotX;
        }
        else
        {
            xf.rotZ = ((PartfxEffectState*)p[slot])->rotOffsetZ;
            xf.rotY = ((PartfxEffectState*)p[slot])->rotOffsetY;
            xf.rotX = ((PartfxEffectState*)p[slot])->rotOffsetX;
        }
        if ((int)((PartfxEffectState*)p[slot])->flags & 0x1000)
        {
            if (((PartfxEffectState*)p[slot])->sourceObject != NULL)
            {
                dirX = view->worldX - ((GameObject*)((PartfxEffectState*)p[slot])->sourceObject)->anim.worldPosX;
                dirZ = view->worldZ -
                       ((GameObject*)((PartfxEffectState*)p[slot])->sourceObject)->anim.worldPosZ;
                dscale = sqrtf(dirX * dirX + dirZ * dirZ);
                if (dscale)
                {
                    dirX = dirX / dscale;
                    dirZ = dirZ / dscale;
                }
                dscale = (u16)getAngle(dirX, dirZ);
                xf.rotX += (s16)dscale;
            }
        }
        xf.x = xf.x - playerMapOffsetX;
        xf.z = xf.z - playerMapOffsetZ;
        setMatrixFromObjectPos(mtxB, &xf);
        mtx44Transpose(mtxB, mtxA);
        PSMTXConcat((MtxPtr)Camera_GetViewMatrix(), (MtxPtr)mtxA, (MtxPtr)mtxA);
        GXLoadPosMtxImm(mtxA, GX_PNMTX0);
        tex = ((PartfxEffectState*)p[slot])->textureResource;
        if (tex != NULL)
        {
            texCount = (u8)(((Texture*)tex)->animationFrameCount >> 8);
        }
        if (tex != NULL && ((PartfxEffectState*)p[slot])->textureFrameTimer != 0)
        {
            ((PartfxEffectState*)p[slot])->textureFrameStep -= 1;
            if (((PartfxEffectState*)p[slot])->textureFrameStep == 0)
            {
                ((PartfxEffectState*)p[slot])->textureFrameStep =
                    0x3c / ((PartfxEffectState*)p[slot])->textureFrameTimer;
                ((PartfxEffectState*)p[slot])->textureFrame += 1;
                if (((PartfxEffectState*)p[slot])->textureFrame >= (u32)texCount)
                {
                    ((PartfxEffectState*)p[slot])->textureFrame = 0;
                }
            }
        }
        if ((int)((PartfxEffectState*)p[slot])->flags & 0x10000000)
        {
            setTextColor(a0, ar, ag, ab, 0xff);
        }
        else if (((PartfxEffectState*)p[slot])->sourceObject != NULL &&
                 ((int)((PartfxEffectState*)p[slot])->flags & 0x4000))
        {
            setTextColor(a0, 0xff, 0xff, 0xff,
                         ((GameObject*)((PartfxEffectState*)p[slot])->sourceObject)->anim.renderAlpha);
        }
        else
        {
            setTextColor(a0, 0xff, 0xff, 0xff, 0xff);
        }
        tex = ((PartfxEffectState*)p[slot])->textureResource;
        if (tex != NULL)
        {
            n131 = ((PartfxEffectState*)p[slot])->textureFrame;
            n131p1 = (n131 + 1) & 0xff;
            if (n131p1 > texCount - 1)
            {
                n131p1 = 0;
            }
        }
        if (((int)((PartfxEffectState*)p[slot])->flags & 0x1000000) &&
            (((PartfxEffectState*)p[slot])->frameUpdated != 0 || ((int)((PartfxEffectState*)p[slot])->flags & 0x400)))
        {
            {
                for (j = 0; j < (u8)n131p1; j++)
                {
                    tex = *(void**)tex;
                }
                _textSetColor(a0, 0xff, 0xff, 0xff,
                              (u8)(0xff - ((PartfxEffectState*)p[slot])->textureFrameStep *
                                              ((PartfxEffectState*)p[slot])->textureFrameFadeStep));
                gxTevResetStages();
                gxTevAddTextureFrameBlendStages();
                gxTevModulateRasStage();
                gxTevCommitStages();
                selectTexture((Texture*)tex, 1);
            }
        }
        else if ((int)((PartfxEffectState*)p[slot])->flags & 0x2000000)
        {
            gxTevResetStages();
            gxTevRasTimesColor1Stage();
            gxTevCommitStages();
        }
        else if ((int)((PartfxEffectState*)p[slot])->flags & 0x4000000)
        {
            gxTevResetStages();
            gxTevTextureTimesRasStage();
            gxTevModulateColor1Stage();
            gxTevCommitStages();
        }
        if (((int)((PartfxEffectState*)p[slot])->flags & 0x05000000) &&
            (((PartfxEffectState*)p[slot])->frameUpdated != 0 || ((int)((PartfxEffectState*)p[slot])->flags & 0x400)))
        {
            {
                tex = ((PartfxEffectState*)p[slot])->textureResource;
                for (j = 0; j < (u8)n131; j++)
                {
                    tex = *(void**)tex;
                }
                selectTexture((Texture*)tex, 0);
            }
        }
        if ((int)((PartfxEffectState*)p[slot])->flags & 0x100)
        {
            gxSetAlphaBlendZTest();
        }
        else if (((int)((PartfxEffectState*)p[slot])->flags & 0x10) &&
                 ((int)((PartfxEffectState*)p[slot])->flags & 0x80))
        {
            gxSetAlphaBlendNoZTest();
        }
        else if ((int)((PartfxEffectState*)p[slot])->flags & 0x80)
        {
            gxSetAlphaBlendZTest();
        }
        else if ((int)((PartfxEffectState*)p[slot])->flags & 0x10)
        {
            gxSetAlphaBlendNoZTest();
        }
        else
        {
            gxSetAlphaBlendZTest();
        }
        if ((int)((PartfxEffectState*)p[slot])->flags & 0x40)
        {
            GXSetCullMode(GX_CULL_FRONT);
        }
        else
        {
            GXSetCullMode(GX_CULL_NONE);
        }
        if (((PartfxEffectState*)p[slot])->frameUpdated != 0 || ((int)((PartfxEffectState*)p[slot])->flags & 0x400))
        {
            int di;
            for (di = 0; di < ((PartfxEffectState*)p[slot])->drawGroupCount; di++)
            {
                if ((int)((PartfxEffectState*)p[slot])->flags & 0x8000000)
                {
                    lightmapDrawTriangleList(buf1, (u8*)buf2,
                                    ((PartfxEffectState*)p[slot])->colorVertexCount /
                                        ((PartfxEffectState*)p[slot])->drawGroupCount);
                }
                else
                {
                    lightmapDrawTriangleList(buf1, (u8*)buf2,
                                    ((PartfxEffectState*)p[slot])->colorVertexCount);
                }
                buf1 = (char*)buf1 + (((PartfxEffectState*)p[slot])->drawGroupStride << 4);
                if ((int)((PartfxEffectState*)p[slot])->flags & 0x8000000)
                {
                    buf2 = (char*)buf2 + ((((PartfxEffectState*)p[slot])->colorVertexCount /
                                           ((PartfxEffectState*)p[slot])->drawGroupCount)
                                          << 4);
                }
            }
            Rcp_ResetRenderState();
            ((PartfxEffectState*)p[slot])->activeVertexBufferIndex =
                1 - ((PartfxEffectState*)p[slot])->activeVertexBufferIndex;
        }
    }
    return 0;
}

void dll_0B_detachSource(void* param)
{
    PartfxEffectState** arr = (PartfxEffectState**)gPartfxActiveEffects;
    int i;

    for (i = 0; i < PARTFX_ACTIVE_EFFECT_COUNT; i++)
    {
        if (arr[i] != NULL && arr[i]->sourceObject == param)
        {
            if ((int)arr[i]->flags & 0x10000)
            {
                partfx_freeEffectsBySequence(arr[i]->sequenceId, 0);
            }
            else
            {
                arr[i]->sourcePosX = ((GameObject*)arr[i]->sourceObject)->anim.worldPosX;
                arr[i]->sourcePosY = ((GameObject*)arr[i]->sourceObject)->anim.worldPosY;
                arr[i]->sourcePosZ = ((GameObject*)arr[i]->sourceObject)->anim.worldPosZ;
                arr[i]->sourceScale = ((GameObject*)arr[i]->sourceObject)->anim.rootMotionScale;
                arr[i]->sourceRotZ = ((GameObject*)arr[i]->sourceObject)->anim.rotZ;
                arr[i]->sourceRotY = ((GameObject*)arr[i]->sourceObject)->anim.rotY;
                arr[i]->sourceRotX = ((GameObject*)arr[i]->sourceObject)->anim.rotX;
                if ((int)arr[i]->flags & 0x2)
                {
                    arr[i]->velocityX += ((GameObject*)arr[i]->sourceObject)->anim.velocityX;
                    arr[i]->velocityY += ((GameObject*)arr[i]->sourceObject)->anim.velocityY;
                    arr[i]->velocityZ += ((GameObject*)arr[i]->sourceObject)->anim.velocityZ;
                }
                if (!((int)arr[i]->flags & 0x200000))
                {
                    arr[i]->flags |= 0x200000;
                }
                *(int*)&arr[i]->sourceObject = 0;
            }
        }
    }
}

void dll_0B_freeSourceEffects(void* source)
{
    PartfxEffectState** arr = (PartfxEffectState**)gPartfxActiveEffects;
    int i;
    for (i = 0; i < PARTFX_ACTIVE_EFFECT_COUNT; i++)
    {
        if (arr[i] == NULL)
            continue;
        if (arr[i]->sourceObject != source)
            continue;
        if (arr[i]->instanceObject != NULL)
        {
            Obj_FreeObject(arr[i]->instanceObject);
        }
        arr[i]->inlineData = NULL;
        if (arr[i]->textureIsBorrowed == 0 && arr[i]->textureResource != NULL)
        {
            textureFree((Texture*)(arr[i]->textureResource));
        }
        if (arr[i]->textureIsBorrowed == 0)
        {
            arr[i]->textureResource = NULL;
        }
        mm_free(arr[i]);
        arr[i] = NULL;
    }
}

static inline int modgfx_findFreeEffectSlot(void** p, int found, int i)
{
    for (; i < PARTFX_ACTIVE_EFFECT_COUNT && found == 0; p++, i++)
    {
        if (*p == NULL)
            found = 1;
    }
    if (found)
    {
        return i - 1;
    }
    return -1;
}

void dll_0B_releaseAll(void)
{
    partfx_freeEffectsBySequence(0, 1);
}

typedef void (*ExpFn2)(void*, int);
typedef void (*ExpFn3)(void*, void*, int);
typedef void (*ExpFn4)(void*, void*, int, int);
typedef void (*ExpResFn6)(void*, int, void*, int, int, void*);

#define PENDING_SPAWNS ((char*)*(int**)((char*)eff + 0x9c))

void dll_0B_updateActiveEffects(void)
{
    int emOff;
    int emIdx;
    int* eff;
    int reprocess;
    int active;
    int** pp;
    int slot;
    int feFlag;
    int cntC;
    int cntA;
    int k;
    void* res;
    PartFxSpawnParams tmpl;
    MatrixTransform rot;
    int objCount;
    int objIdx;

    emIdx = 0;
    gExpgfxUpdatingActivePools = 2;
    if (renderModeSetOrGet(-1) == 1)
    {
        return;
    }
    gModgfxMotionStep = timeDelta;
    pp = (int**)gPartfxActiveEffects;
    for (slot = 0; slot < PARTFX_ACTIVE_EFFECT_COUNT; slot++)
    {
        reprocess = 1;
        while (reprocess)
        {
            reprocess = 0;
            eff = pp[slot];
            if (eff == NULL)
                break;
            if (((ModgfxEffectSlot*)eff)->animSlotId == -1)
                break;
            active = 0;
            ((ModgfxEffectSlot*)eff)->unk13E = 0;
            if (((ModgfxEffectSlot*)eff)->frameDuration < 0 || ((ModgfxEffectSlot*)eff)->frameIndex == -1)
            {
                ((ModgfxEffectSlot*)eff)->frameIndex += 1;
                if (((ModgfxEffectSlot*)eff)->frameIndex > 6)
                {
                    partfx_freeEffectsBySequence(((ModgfxEffectSlot*)eff)->animSlotId, 0);
                    break;
                }
                ((ModgfxEffectSlot*)eff)->frameDuration =
                    ((ModgfxEffectSlot*)eff)->frameTimings[((ModgfxEffectSlot*)eff)->frameIndex];
                active = 1;
                ((ExpFn2)modgfx_captureFrameBaseVertices)(eff, 0);
            }
            else if (((ModgfxEffectSlot*)eff)->pendingFrameIdx != 0)
            {
                ((ModgfxEffectSlot*)eff)->frameIndex = ((ModgfxEffectSlot*)eff)->pendingFrameIdx;
                ((ModgfxEffectSlot*)eff)->pendingFrameIdx = 0;
                if (((ModgfxEffectSlot*)eff)->frameIndex > 6)
                {
                    partfx_freeEffectsBySequence(((ModgfxEffectSlot*)eff)->animSlotId, 0);
                    break;
                }
                ((ModgfxEffectSlot*)eff)->frameDuration =
                    ((ModgfxEffectSlot*)eff)->frameTimings[((ModgfxEffectSlot*)eff)->frameIndex];
                active = 1;
                ((ExpFn2)modgfx_captureFrameBaseVertices)(eff, 0);
            }
            cntC = 0;
            cntA = 0;
            ((ExpFn3)modgfx_restoreBaseVertices)(eff, PENDING_SPAWNS + emIdx * 0x18, active);
            feFlag = 0;
            emIdx = 0;
            emOff = 0;
            for (; emIdx < ((ModgfxEffectSlot*)eff)->emitterCount; emOff += 0x18, emIdx++)
            {
                int flags;
                if (((ModgfxEffectSlot*)eff)->frameIndex != ((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->sequenceIndex)
                    continue;
                flags = ((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->modelOrResource;
                if ((flags & 0x1000) && ((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->posX > MODGFX_ZERO &&
                    ((ModgfxEffectSlot*)eff)->frameIndex > 0)
                {
                    ((ModgfxEffectSlot*)eff)->frameIndex = ((ModgfxPendingSpawn*)(PENDING_SPAWNS + emIdx * 0x18))->param14;
                    ((ModgfxPendingSpawn*)(PENDING_SPAWNS + emIdx * 0x18))->posX =
                        ((ModgfxPendingSpawn*)(PENDING_SPAWNS + emIdx * 0x18))->posX - MODGFX_ONE;
                    ((ModgfxEffectSlot*)eff)->frameDuration = -1;
                    break;
                }
                if (flags & 0x2000)
                {
                    if (((ModgfxEffectSlot*)eff)->unk13A != 0)
                    {
                        ((ModgfxEffectSlot*)eff)->unk13A = 0;
                        ((ModgfxPendingSpawn*)(PENDING_SPAWNS + emIdx * 0x18))->modelOrResource = 0;
                        ((ModgfxPendingSpawn*)(PENDING_SPAWNS + emIdx * 0x18))->modelOrResource = 0x20;
                        ((ModgfxEffectSlot*)eff)->frameDuration = -1;
                        reprocess = 1;
                        feFlag = 0;
                        break;
                    }
                    if (((ModgfxEffectSlot*)eff)->frameIndex > 0)
                    {
                        feFlag = 1;
                        ((ModgfxEffectSlot*)eff)->frameIndex = ((ModgfxPendingSpawn*)(PENDING_SPAWNS + emIdx * 0x18))->param14;
                        ((ModgfxEffectSlot*)eff)->frameDuration = -1;
                        reprocess = 1;
                        break;
                    }
                }
                if (flags & 0x10000000)
                {
                    tmpl.posX = ((ModgfxEffectSlot*)eff)->posCurX;
                    tmpl.posY = ((ModgfxEffectSlot*)eff)->posCurY;
                    tmpl.posZ = ((ModgfxEffectSlot*)eff)->posCurZ;
                    rot.x = MODGFX_ZERO;
                    rot.y = MODGFX_ZERO;
                    rot.z = MODGFX_ZERO;
                    rot.scale = MODGFX_ONE;
                    if (((ModgfxEffectSlot*)eff)->sourceFlags & 1)
                    {
                        rot.rotX = ((ModgfxEffectSlot*)eff)->unkC;
                    }
                    else
                    {
                        rot.rotX = *(s16*)(*(int**)&((ModgfxEffectSlot*)eff)->sourceObj);
                    }
                    rot.rotY = 0;
                    rot.rotZ = 0;
                    vecRotateZXY(&rot.rotX, &tmpl.posX);
                    if (*(void**)eff == NULL && Obj_IsLoadingLocked())
                    {
                        int* o;
                        if ((((ModgfxEffectSlot*)eff)->sourceFlags & 1) == 0)
                        {
                            tmpl.posX = ((GameObject*)((ModgfxEffectSlot*)eff)->sourceObj)->anim.worldPosX + tmpl.posX;
                            tmpl.posY = ((GameObject*)((ModgfxEffectSlot*)eff)->sourceObj)->anim.worldPosY + tmpl.posY;
                            tmpl.posZ = ((GameObject*)((ModgfxEffectSlot*)eff)->sourceObj)->anim.worldPosZ + tmpl.posZ;
                        }
                        else
                        {
                            tmpl.posX = ((ModgfxEffectSlot*)eff)->posOffsetX + tmpl.posX;
                            tmpl.posY = ((ModgfxEffectSlot*)eff)->posOffsetY + tmpl.posY;
                            tmpl.posZ = ((ModgfxEffectSlot*)eff)->posOffsetZ + tmpl.posZ;
                        }
                        o = (int*)Obj_AllocObjectSetup(0x20, DLL0B_CHILD_OBJ_INVHIT);
                        ((ObjPlacement*)o)->posX = tmpl.posX;
                        ((ObjPlacement*)o)->posY = tmpl.posY;
                        ((ObjPlacement*)o)->posZ = tmpl.posZ;
                        *(int*)eff = (int)Obj_SetupObject((ObjPlacement*)o, 5, -1, -1, NULL);
                        *(int*)(*(int*)eff + 0xf8) = 1;
                    }
                    else if (*(void**)eff != NULL)
                    {
                        if ((((ModgfxEffectSlot*)eff)->sourceFlags & 1) == 0)
                        {
                            tmpl.posX = ((GameObject*)((ModgfxEffectSlot*)eff)->sourceObj)->anim.worldPosX + tmpl.posX;
                            tmpl.posY = ((GameObject*)((ModgfxEffectSlot*)eff)->sourceObj)->anim.worldPosY + tmpl.posY;
                            tmpl.posZ = ((GameObject*)((ModgfxEffectSlot*)eff)->sourceObj)->anim.worldPosZ + tmpl.posZ;
                        }
                        else
                        {
                            tmpl.posX = ((ModgfxEffectSlot*)eff)->posOffsetX + tmpl.posX;
                            tmpl.posY = ((ModgfxEffectSlot*)eff)->posOffsetY + tmpl.posY;
                            tmpl.posZ = ((ModgfxEffectSlot*)eff)->posOffsetZ + tmpl.posZ;
                        }
                        *(f32*)(*(int*)eff + 0x18) = tmpl.posX;
                        *(f32*)(*(int*)eff + 0x1c) = tmpl.posY;
                        *(f32*)(*(int*)eff + 0x20) = tmpl.posZ;
                    }
                    if (*(void**)eff != NULL)
                    {
                        int* o = *(int**)eff;
                        int* list = *(int**)((char*)*(int**)&((GameObject*)o)->anim.hitReactState + 0x50);
                        if (list != NULL)
                        {
                            if (*(s16*)((char*)list + 0x44) == (int)((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->posX)
                            {
                                Obj_FreeObject((GameObject*)o);
                                *(int*)eff = 0;
                                ((ModgfxPendingSpawn*)(PENDING_SPAWNS + emIdx * 0x18))->modelOrResource ^= 0x10000000;
                                if (((ModgfxPendingSpawn*)(PENDING_SPAWNS + emIdx * 0x18))->posZ >= MODGFX_ZERO &&
                                    *(int**)&((ModgfxEffectSlot*)eff)->sourceObj != NULL)
                                {
                                    (*gPartfxInterface)
                                        ->spawnObject(*(int**)&((ModgfxEffectSlot*)eff)->sourceObj,
                                                      (int)((ModgfxPendingSpawn*)(PENDING_SPAWNS + emIdx * 0x18))->posZ, &tmpl,
                                                      0x200001, -1, 0);
                                }
                                ((ModgfxEffectSlot*)eff)->pendingFrameIdx =
                                    ((ModgfxPendingSpawn*)(PENDING_SPAWNS + emIdx * 0x18))->posY;
                                break;
                            }
                        }
                    }
                }
                ObjList_GetObjects(&objIdx, &objCount);
                if (((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->modelOrResource & 0x2)
                {
                    modgfx_stepVertexScale((ModgfxState*)eff,
                                           (ModgfxVertexGroupCmd*)(PENDING_SPAWNS + emOff), active, cntC);
                    cntC++;
                }
                if (((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->modelOrResource & 0x4)
                {
                    modgfx_stepVertexAlpha((ModgfxState*)eff,
                                           (ModgfxVertexGroupCmd*)(PENDING_SPAWNS + emOff), active, cntA);
                    cntA++;
                }
                if (((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->modelOrResource & 0x8)
                {
                    ((ExpFn4)modgfx_stepVertexColor)(eff, PENDING_SPAWNS + emOff, active, 0);
                }
                if (((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->modelOrResource & 0x100)
                {
                    ModgfxPendingSpawn* em = (ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff);
                    ((ModgfxEffectSlot*)eff)->rotOffsetZ += (s16)(em->posX * gModgfxMotionStep);
                    ((ModgfxEffectSlot*)eff)->rotOffsetY += (s16)(em->posY * gModgfxMotionStep);
                    ((ModgfxEffectSlot*)eff)->rotOffsetX += (s16)(em->posZ * gModgfxMotionStep);
                }
                if (((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->modelOrResource & 0x80)
                {
                    ((ExpFn4)modgfx_stepS16VectorLerp)(eff, PENDING_SPAWNS + emOff, active, 0);
                }
                if (((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->modelOrResource & 0x8000000)
                {
                    ((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->posZ = randomGetRange(0, 0xffff);
                    ((ExpFn4)modgfx_stepS16VectorLerp)(eff, PENDING_SPAWNS + emOff, active, 0);
                }
                if (((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->modelOrResource & 0x4000)
                {
                    ((ExpFn4)modgfx_scrollTexCoords)(eff, PENDING_SPAWNS + emOff, active, 0);
                }
                if ((((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->modelOrResource & 0x10000) && active != 0)
                {
                    if (((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->param14 == -1)
                    {
                        Sfx_StopObjectChannel((int)*(int**)&((ModgfxEffectSlot*)eff)->sourceObj, 0x40);
                    }
                    else
                    {
                        Sfx_PlayFromObject((u32)((ModgfxEffectSlot*)eff)->sourceObj,
                                           (u16) * (s16*)(PENDING_SPAWNS + emOff + 0x14));
                    }
                }
                if (((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->modelOrResource & 0x100000)
                {
                    if (active == 1)
                    {
                        if (((ModgfxEffectSlot*)eff)->frameDuration != 0)
                        {
                            ((ModgfxEffectSlot*)eff)->alphaDelta =
                                (((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->posX -
                                 (f32)(u32)(*(GameObject**)&((ModgfxEffectSlot*)eff)->sourceObj)->anim.alpha) /
                                (f32)((ModgfxEffectSlot*)eff)->frameDuration;
                            ((ModgfxEffectSlot*)eff)->alphaCurrent =
                                (f32)(u32)(*(GameObject**)&((ModgfxEffectSlot*)eff)->sourceObj)->anim.alpha;
                        }
                        else
                        {
                            ((ModgfxEffectSlot*)eff)->alphaDelta =
                                ((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->posX -
                                (f32)(u32)(*(GameObject**)&((ModgfxEffectSlot*)eff)->sourceObj)->anim.alpha;
                            ((ModgfxEffectSlot*)eff)->alphaCurrent = MODGFX_ZERO;
                        }
                    }
                    ((ModgfxEffectSlot*)eff)->alphaCurrent =
                        ((ModgfxEffectSlot*)eff)->alphaCurrent + ((ModgfxEffectSlot*)eff)->alphaDelta;
                    if (((ModgfxEffectSlot*)eff)->alphaCurrent > 255.0f)
                    {
                        ((ModgfxEffectSlot*)eff)->alphaCurrent = 255.0f;
                    }
                    else if (((ModgfxEffectSlot*)eff)->alphaCurrent < MODGFX_ZERO)
                    {
                        ((ModgfxEffectSlot*)eff)->alphaCurrent = MODGFX_ZERO;
                    }
                    (*(GameObject**)&((ModgfxEffectSlot*)eff)->sourceObj)->anim.alpha =
                        ((ModgfxEffectSlot*)eff)->alphaCurrent;
                }
                if (((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->modelOrResource & 0x400000)
                {
                    ((ExpFn4)modgfx_stepPosition)(eff, PENDING_SPAWNS + emOff, active, 0);
                }
                if (((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->modelOrResource & 0x80000000)
                {
                    ModgfxPendingSpawn* em = (ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff);
                    ((ModgfxEffectSlot*)eff)->motionOffsetX =
                        em->posX * gModgfxMotionStep + ((ModgfxEffectSlot*)eff)->motionOffsetX;
                    ((ModgfxEffectSlot*)eff)->motionOffsetY =
                        em->posY * gModgfxMotionStep + ((ModgfxEffectSlot*)eff)->motionOffsetY;
                    ((ModgfxEffectSlot*)eff)->motionOffsetZ =
                        em->posZ * gModgfxMotionStep + ((ModgfxEffectSlot*)eff)->motionOffsetZ;
                }
                if (((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->modelOrResource & 0x800000)
                {
                    if ((((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->modelOrResource & 0x1000000) &&
                        MODGFX_ZERO == ((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->posY)
                    {
                        for (k = 0; k < (int)((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->posX; k++)
                        {
                            if (randomGetRange(0, (int)((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->posZ) == 0)
                            {
                                if (((ModgfxEffectSlot*)eff)->sourceFlags & 1)
                                {
                                    (*gPartfxInterface)
                                        ->spawnObject(*(int**)&((ModgfxEffectSlot*)eff)->sourceObj,
                                                      ((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->param14, NULL, 0x10001, -1,
                                                      NULL);
                                }
                                else
                                {
                                    (*gPartfxInterface)
                                        ->spawnObject(*(int**)&((ModgfxEffectSlot*)eff)->sourceObj,
                                                      ((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->param14, NULL, 0x10001, -1,
                                                      NULL);
                                }
                            }
                        }
                    }
                    else if (MODGFX_ZERO == ((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->posY)
                    {
                        for (k = 0; k < (int)((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->posX; k++)
                        {
                            if (((ModgfxEffectSlot*)eff)->sourceFlags & 1)
                            {
                                (*gPartfxInterface)
                                    ->spawnObject(*(int**)&((ModgfxEffectSlot*)eff)->sourceObj,
                                                  ((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->param14, eff + 3, 0x10002, -1,
                                                  NULL);
                            }
                            else
                            {
                                (*gPartfxInterface)
                                    ->spawnObject(*(int**)&((ModgfxEffectSlot*)eff)->sourceObj,
                                                  ((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->param14, NULL, 0x10002, -1,
                                                  NULL);
                            }
                        }
                    }
                    else if (MODGFX_ONE == ((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->posY)
                    {
                        if ((((ModgfxEffectSlot*)eff)->sourceFlags & 1) == 0)
                        {
                            tmpl.posX = ((GameObject*)((ModgfxEffectSlot*)eff)->sourceObj)->anim.worldPosX +
                                     ((ModgfxEffectSlot*)eff)->posCurX;
                            tmpl.posY = ((GameObject*)((ModgfxEffectSlot*)eff)->sourceObj)->anim.worldPosY +
                                     ((ModgfxEffectSlot*)eff)->posCurY;
                            tmpl.posZ = ((GameObject*)((ModgfxEffectSlot*)eff)->sourceObj)->anim.worldPosZ +
                                     ((ModgfxEffectSlot*)eff)->posCurZ;
                            if (*(int**)&((ModgfxEffectSlot*)eff)->sourceObj != NULL)
                            {
                                (*gPartfxInterface)
                                    ->spawnObject(*(int**)&((ModgfxEffectSlot*)eff)->sourceObj,
                                                  ((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->param14, &tmpl, 0x10001, -1,
                                                  NULL);
                            }
                        }
                        else
                        {
                            tmpl.posX = ((ModgfxEffectSlot*)eff)->posCurX;
                            tmpl.posY = ((ModgfxEffectSlot*)eff)->posCurY;
                            tmpl.posZ = ((ModgfxEffectSlot*)eff)->posCurZ;
                            if (*(int**)&((ModgfxEffectSlot*)eff)->sourceObj != NULL)
                            {
                                (*gPartfxInterface)
                                    ->spawnObject(*(int**)&((ModgfxEffectSlot*)eff)->sourceObj,
                                                  ((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->param14, &tmpl, 0x10001, -1,
                                                  NULL);
                            }
                        }
                    }
                }
                if (((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->modelOrResource & 0x4000000)
                {
                    res = Resource_Acquire((u16)(((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->param14 + 0x58), 1);
                    if (((ModgfxPendingSpawn*)(PENDING_SPAWNS + emOff))->modelOrResource & 0x1000000)
                    {
                        for (k = 0; k < (int)*(f32*)((emOff + (int)PENDING_SPAWNS) + 0x4); k++)
                        {
                            if (randomGetRange(0, 5) == 0)
                            {
                                if (((ModgfxEffectSlot*)eff)->sourceFlags & 1)
                                {
                                    (*(ExpResFn6*)(*(int*)res + 4))(NULL, 0, eff + 3, 1, -1, NULL);
                                }
                                else
                                {
                                    (*(ExpResFn6*)(*(int*)res + 4))(*(int**)&((ModgfxEffectSlot*)eff)->sourceObj, 0,
                                                                    NULL, 1, -1, NULL);
                                }
                            }
                        }
                    }
                    else
                    {
                        for (k = 0; k < (int)*(f32*)((emOff + (int)PENDING_SPAWNS) + 0x4); k++)
                        {
                            if (((ModgfxEffectSlot*)eff)->sourceFlags & 1)
                            {
                                (*(ExpResFn6*)(*(int*)res + 4))(NULL, 0, eff + 3, 1, -1, NULL);
                            }
                            else
                            {
                                (*(ExpResFn6*)(*(int*)res + 4))(*(int**)&((ModgfxEffectSlot*)eff)->sourceObj, 0, NULL,
                                                                1, -1, NULL);
                            }
                        }
                    }
                    Resource_Release(res);
                }
            }
            if (feFlag == 0)
            {
                ((ModgfxEffectSlot*)eff)->frameDuration = ((ModgfxEffectSlot*)eff)->frameDuration - framesThisStep;
            }
        }
        gExpgfxUpdatingActivePools = 0;
    }
}

s16 dll_0B_spawnEffect(ModgfxSpawnContext* st, int unused, int c, s16* b, int e, s16* d, int textureAssetId,
                  void* textureResource)
{
    int base0;
    int total = 0;
    int off;
    int i = 0;
    int found = i;
    int spawnCount;
    int divThresh;
    int slot = modgfx_findFreeEffectSlot(gPartfxActiveEffects, found, i);
    f32 fz434;
    f32 fz430;
    PartfxEffectState** arr;

    if (slot == -1)
    {
        return 0;
    }
    {
        off = 0;
        spawnCount = st->pendingSpawnCount;
        for (i = 0; i < spawnCount; i++, off += 0x18)
        {
            ModgfxPendingSpawn* item = (ModgfxPendingSpawn*)((u8*)st->pendingSpawns + off);
            if ((item->modelOrResource & 0xf7fff180) == 0 && item->param14 != 0)
            {
                total += item->param14;
            }
        }
    }

    base0 = 0;
    if ((st->flags & 0x800) == 0)
    {
        base0 = (int)(long)((e * 3) << 4) + ((c * 3) << 4);
    }

    arr = (PartfxEffectState**)gPartfxActiveEffects;
    arr[slot] =
        (PartfxEffectState*)mmAlloc(base0 + 0x240 + spawnCount * 0x18 + total * 2, 0x15, 0);
    if (arr[slot] == NULL)
    {
        partfx_freeEffectsBySequence(0, 0);
        return -1;
    }

    arr[slot]->inlineData =
        (u8*)arr[slot] + sizeof(PartfxEffectState);
    {
        u8* bufp = arr[slot]->inlineData;
        if ((st->flags & 0x800) == 0)
        {
            arr[slot]->colorBuffers[0] = bufp;
            bufp += e * 16;
            arr[slot]->colorBuffers[1] = bufp;
            bufp += e * 16;
            arr[slot]->colorBuffers[2] = bufp;
            bufp += e * 16;
            arr[slot]->vertexBuffers[0] = bufp;
            bufp += c * 16;
            arr[slot]->vertexBuffers[1] = bufp;
            bufp += c * 16;
            arr[slot]->vertexBuffers[2] = bufp;
            bufp += c * 16;
        }
        arr[slot]->baseVertexBuffer = bufp;
        arr[slot]->baseColorBuffer = bufp + 0x80;
    }

    if (st->drawGroupCount != 0)
    {
        divThresh = e / st->drawGroupCount;
    }
    else
    {
        divThresh = e;
    }
    if ((st->flags & 0x800) == 0)
    {
        int k;
        for (k = 0; k < 3; k++)
        {
            u8* dstc = (u8*)arr[slot]->colorBuffers[k];
            int j = 0;
            int bias = 0;
            s16* sd = d;
            for (; j < e; j++)
            {
                if ((st->flags & 0x8000000) && j == divThresh)
                {
                    bias = st->drawGroupStride;
                }
                dstc[1] = sd[0] - bias;
                dstc[2] = sd[1] - bias;
                dstc[3] = sd[2] - bias;
                sd += 3;
                dstc += 0x10;
            }
        }
    }

    arr[slot]->textureResource = NULL;
    arr[slot]->textureIsBorrowed = 0;
    if (textureResource != NULL)
    {
        arr[slot]->textureResource = textureResource;
        arr[slot]->textureIsBorrowed = 1;
    }
    else if (textureAssetId != 0)
    {
        arr[slot]->textureResource = textureLoadAsset(textureAssetId);
        arr[slot]->textureIsBorrowed = 0;
    }

    if ((st->flags & 0x800) == 0)
    {
        int k;
        for (k = 0; k < 3; k++)
        {
            u8* dstv = (u8*)arr[slot]->vertexBuffers[k];
            int j;
            s16* sb = b;
            for (j = 0; j < c; j++)
            {
                *(s16*)(dstv + 0) = sb[0];
                *(s16*)(dstv + 2) = sb[1];
                *(s16*)(dstv + 4) = sb[2];
                if (arr[slot]->textureResource != NULL)
                {
                    *(s16*)(dstv + 8) =
                        128.0f *
                        ((f32)sb[3] / (f32)((Texture*)arr[slot]->textureResource)->width);
                    *(s16*)(dstv + 0xa) =
                        128.0f *
                        ((f32)sb[4] / (f32)((Texture*)arr[slot]->textureResource)->height);
                }
                dstv[0xc] = 0xff;
                dstv[0xd] = 0xff;
                dstv[0xe] = 0xff;
                dstv[0xf] = 0xff;
                dstv += 0x10;
                sb += 5;
            }
        }
    }

    arr[slot]->emitterCount = st->pendingSpawnCount;
    arr[slot]->word114 = 0;
    arr[slot]->word118 = 0;
    arr[slot]->word11C = 0;
    arr[slot]->auxAllocation = NULL;
    arr[slot]->releaseRequested = 0;
    arr[slot]->byte13D = 0;
    arr[slot]->stageTimer = 0;
    arr[slot]->nextStage = -1;
    arr[slot]->requestedStage = 0;
    arr[slot]->stageDurations[0] = st->sequenceParams[0];
    arr[slot]->stageDurations[1] = st->sequenceParams[1];
    arr[slot]->stageDurations[2] = st->sequenceParams[2];
    arr[slot]->stageDurations[3] = st->sequenceParams[3];
    arr[slot]->stageDurations[4] = st->sequenceParams[4];
    arr[slot]->stageDurations[5] = st->sequenceParams[5];
    arr[slot]->stageDurations[6] = st->sequenceParams[6];
    arr[slot]->emitterCommands =
        (u8*)(base0 + (int)arr[slot]->inlineData) + 0x100;
    arr[slot]->auxSequenceBuffer = NULL;
    if (total != 0)
    {
        arr[slot]->auxSequenceBuffer =
            (u8*)arr[slot]->emitterCommands +
            st->pendingSpawnCount * 0x18;
    }

    {
        u8* dst = arr[slot]->auxSequenceBuffer;
        for (i = 0, off = 0; i < arr[slot]->emitterCount; i++, off += 0x18)
        {
            ((ModgfxPendingSpawn*)((u8*)arr[slot]->emitterCommands + off))
                ->sequenceIndex = ((ModgfxPendingSpawn*)((u8*)st->pendingSpawns + off))->sequenceIndex;
            ((ModgfxPendingSpawn*)((u8*)arr[slot]->emitterCommands + off))
                ->param14 = ((ModgfxPendingSpawn*)((u8*)st->pendingSpawns + off))->param14;
            ((ModgfxPendingSpawn*)((u8*)arr[slot]->emitterCommands + off))
                ->param10 = 0;
            ((ModgfxPendingSpawn*)((u8*)arr[slot]->emitterCommands + off))
                ->modelOrResource = ((ModgfxPendingSpawn*)((u8*)st->pendingSpawns + off))->modelOrResource;
            if ((((ModgfxPendingSpawn*)((u8*)arr[slot]->emitterCommands + off))
                     ->modelOrResource &
                 0xf7fff180) == 0 &&
                ((ModgfxPendingSpawn*)((u8*)arr[slot]->emitterCommands + off))
                        ->param14 != 0)
            {
                int k;
                ((ModgfxPendingSpawn*)((u8*)arr[slot]->emitterCommands + off))
                    ->param10 = 0;
                *(u8**)&((ModgfxPendingSpawn*)((u8*)arr[slot]->emitterCommands +
                                               off))
                     ->param10 = dst;
                dst += ((ModgfxPendingSpawn*)((u8*)arr[slot]->emitterCommands +
                                              off))
                           ->param14 *
                       2;
                for (k = 0;
                     k <
                     ((ModgfxPendingSpawn*)((u8*)arr[slot]->emitterCommands +
                                            off))
                         ->param14;
                     k++)
                {
                    *(s16*)(*(u8**)&((ModgfxPendingSpawn*)((u8*)arr[slot]
                                                               ->emitterCommands +
                                                           off))
                                 ->param10 +
                            k * 2) =
                        *(s16*)(*(u8**)((u8*)&((ModgfxPendingSpawn*)st->pendingSpawns)->param10 + off) + k * 2);
                }
            }
            ((ModgfxPendingSpawn*)((u8*)arr[slot]->emitterCommands + off))
                ->posX = ((ModgfxPendingSpawn*)((u8*)st->pendingSpawns + off))->posX;
            ((ModgfxPendingSpawn*)((u8*)arr[slot]->emitterCommands + off))
                ->posY = ((ModgfxPendingSpawn*)((u8*)st->pendingSpawns + off))->posY;
            ((ModgfxPendingSpawn*)((u8*)arr[slot]->emitterCommands + off))
                ->posZ = ((ModgfxPendingSpawn*)((u8*)st->pendingSpawns + off))->posZ;
        }
    }

    arr[slot]->currentStage = -1;
    arr[slot]->stageFrameCountdown =
        arr[slot]
            ->stageDurations[arr[slot]->currentStage];
    arr[slot]->flags = st->flags;
    arr[slot]->drawPosX = st->posX;
    arr[slot]->drawPosY = st->posY;
    arr[slot]->drawPosZ = st->posZ;
    arr[slot]->renderScale = st->scale;
    if ((int)arr[slot]->flags & 1)
    {
        arr[slot]->sourcePosX = st->posX;
        arr[slot]->sourcePosY = st->posY;
        arr[slot]->sourcePosZ = st->posZ;
    }
    fz430 = MODGFX_ZERO;
    arr[slot]->posStepX = fz430;
    arr[slot]->posStepY = fz430;
    arr[slot]->posStepZ = fz430;
    fz434 = MODGFX_ONE;
    arr[slot]->scaleVectors[0].x = fz434;
    arr[slot]->scaleVectors[0].y = fz434;
    arr[slot]->scaleVectors[0].z = fz434;
    arr[slot]->scaleVectors[1].x = fz430;
    arr[slot]->scaleVectors[1].y = fz430;
    arr[slot]->scaleVectors[1].z = fz430;
    arr[slot]->scaleVectors[2].x = fz434;
    arr[slot]->scaleVectors[2].y = fz434;
    arr[slot]->scaleVectors[2].z = fz434;
    arr[slot]->scaleVectors[3].x = fz430;
    arr[slot]->scaleVectors[3].y = fz430;
    arr[slot]->scaleVectors[3].z = fz430;
    arr[slot]->rotOffsetZ = 0;
    arr[slot]->rotOffsetY = 0;
    arr[slot]->rotOffsetX = 0;
    arr[slot]->vec120 = 0;
    arr[slot]->vec122 = 0;
    arr[slot]->vec124 = 0;
    arr[slot]->alphaValues[0] = fz430;
    arr[slot]->alphaValues[1] = fz430;
    arr[slot]->alphaValues[2] = fz430;
    arr[slot]->alphaValues[3] = fz430;
    arr[slot]->blendColorR = fz430;
    arr[slot]->blendColorG = fz430;
    arr[slot]->blendColorB = fz430;
    arr[slot]->blendColorStepR = fz430;
    arr[slot]->blendColorStepG = fz430;
    arr[slot]->blendColorStepB = fz430;
    arr[slot]->velocityX = st->vecX;
    arr[slot]->velocityY = st->vecY;
    arr[slot]->velocityZ = st->vecZ;
    gPartfxSequenceIdCounter += 1;
    if (gPartfxSequenceIdCounter > 0x4e20)
    {
        gPartfxSequenceIdCounter = 0;
    }
    arr[slot]->sequenceId = gPartfxSequenceIdCounter;
    arr[slot]->byte126 = lbl_803DD282;
    arr[slot]->vertexCount = c;
    arr[slot]->colorVertexCount = e;
    arr[slot]->sourceObject = st->attachedSource;
    arr[slot]->instanceObject = NULL;
    *(u8*)&arr[slot]->sourceYawIndex = st->sourceYawIndex;
    arr[slot]->drawGroupCount = st->drawGroupCount;
    arr[slot]->drawGroupStride = st->drawGroupStride;
    arr[slot]->initialStateByte = st->initialStateByte;
    arr[slot]->soundHandle = 0;
    arr[slot]->activeVertexBufferIndex = 0;
    arr[slot]->byte13B = 0;
    arr[slot]->frameUpdated = 0;
    arr[slot]->textureFrameTimer = st->textureFrameTimer;
    if (arr[slot]->textureFrameTimer != 0)
    {
        arr[slot]->textureFrameStep =
            0x3c / arr[slot]->textureFrameTimer;
    }
    else
    {
        arr[slot]->textureFrameStep = 0;
    }
    if (arr[slot]->textureFrameStep != 0)
    {
        arr[slot]->textureFrameFadeStep =
            0xff / arr[slot]->textureFrameStep;
    }
    else
    {
        arr[slot]->textureFrameFadeStep = 0;
    }
    arr[slot]->textureFrame = 0;
    arr[slot]->initialDelayFrames = st->sourceModeCopy;
    return arr[slot]->sequenceId;
}

void dll_0B_onMapSetup(void)
{
    int i;

    partfx_freeEffectsBySequence(0, 1);
    for (i = 0; i < PARTFX_ACTIVE_EFFECT_COUNT; i++)
    {
        gPartfxActiveEffects[i] = NULL;
    }
}

void dll_0B_release(void)
{
    partfx_freeEffectsBySequence(0, 1);
}

void dll_0B_initialise(void)
{
    PartfxEffectState** arr = (PartfxEffectState**)gPartfxActiveEffects;
    int i;
    for (i = 0; i < PARTFX_ACTIVE_EFFECT_COUNT; i++)
    {
        arr[i] = NULL;
    }
}

Dll0BDescriptorTable lbl_8030FCA8 = {{0x00000000,
                                      0x00000000,
                                      0x00000000,
                                      0x00180000,
                                      (u32)dll_0B_initialise,
                                      (u32)dll_0B_release,
                                      0x00000000,
                                      (u32)dll_0B_onMapSetup,
                                      (u32)dll_0B_spawnEffect,
                                      (u32)dll_0B_updateActiveEffects,
                                      (u32)dll_0B_releaseAll,
                                      (u32)dll_0B_freeSourceEffects,
                                      (u32)dll_0B_detachSource,
                                      (u32)dll_0B_renderEffects,
                                      (u32)dll_0B_releaseHandle,
                                      (u32)dll_0B_func0B,
                                      (u32)dll_0B_func0C,
                                      (u32)dll_0B_func0D,
                                      (u32)dll_0B_markSourceFrameUpdated,
                                      (u32)dll_0B_beginSequence,
                                      (u32)dll_0B_resetSequenceSpawns,
                                      (u32)dll_0B_addSequenceSpawn,
                                      (u32)dll_0B_nextSequenceParam,
                                      (u32)dll_0B_setSequenceParamIndex,
                                      (u32)dll_0B_setSequenceParamValue,
                                      (u32)dll_0B_setSequenceParams,
                                      (u32)dll_0B_spawnSequence,
                                      (u32)dll_0B_addSequenceFlags,
                                      (u32)dll_0B_getLastSpawnHandle,
                                      0x00000000}};
