#include "main/dll/partfx_interface.h"
#include "dolphin/mtx.h"
#include "track/intersect_depth_state_api.h"
#include "track/intersect_fog_api.h"
#include "track/intersect_render_setup_api.h"
#include "track/intersect_geom_api.h"
#include "main/hud_visibility_api.h"
#include "main/shader_api.h"
#include "main/debug.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "sys/objects/lifecycle.h"
#include "main/camera.h"
#include "main/dll_000A_expgfx.h"
#include "main/dll/waterfx_interface.h"
#include "main/expgfx_internal.h"
#include "game/objects/object.h"
#include "main/dll/player_api.h"
#include "sys/objects.h"
#include "main/objfx.h"
#include "main/lightmap_api.h"
#include "main/lightmap_render_queue_api.h"
#include "main/mm.h"
#include "main/sky.h"
#include "main/tex_dolphin.h"
#include "main/texture.h"
#include "main/dll/objfx_api.h"
#include "dolphin/os/OSFastCast.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/frame_timing.h"
#include "main/render_mode_api.h"
#include "main/dll/objfx.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/trig_float_helpers.h"
#include "main/dll/viewfinder.h"
#include "main/dll/dll_000B_dll0b.h"
#include "track/intersect_api.h"
#include "main/lightmap.h"
#include "main/dll/dll_80136a40.h"


typedef union ExpgfxWGPipe
{
    u8 u8;
    u16 u16;
    u32 u32;
    s8 s8;
    s16 s16;
    s32 s32;
    f32 f32;
    f64 f64;
} ExpgfxWGPipe;

typedef struct ExpgfxRotateParams
{
    s16 angleX;
    s16 angleY;
    s16 angleZ;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
} ExpgfxRotateParams;

typedef struct ExpgfxBillboardAngles
{
    s16 pitch;
    s16 yaw;
} ExpgfxBillboardAngles;

typedef union Dll0BDescriptorTable
{
    u32 words[30];
    u64 align8;
} Dll0BDescriptorTable;

#define GX_BM_NONE        0
#define GX_BM_BLEND       1
#define GX_BL_ZERO        0
#define GX_BL_ONE         1
#define GX_BL_SRCALPHA    4
#define GX_BL_INVSRCALPHA 5
#define GX_LO_NOOP        5
#define GX_GREATER        4
#define GX_ALWAYS         7
#define GX_AOP_AND        0
#define GX_CULL_NONE      0
#define GX_VA_POS         9
#define GX_VA_CLR0        11
#define GX_VA_TEX0        13
#define GX_DIRECT         1
#define GX_QUADS          0x80
#define GX_VTXFMT4        4
#define GX_PNMTX0         0
#define GX_COLOR0         0
#define GX_ALPHA0         2
#define GX_FALSE          0
#define GX_SRC_REG        0
#define GX_SRC_VTX        1
#define GX_DF_NONE        0
#define GX_AF_NONE        2

#define GXWGFifo (*(volatile ExpgfxWGPipe*)0xCC008000)

extern u8 lbl_803DD253;
extern f32 gExpgfxYVelocityPositiveLimit;
extern f32 gExpgfxYVelocityFastStep;
extern f32 gExpgfxYVelocitySlowStep;
extern f32 gExpgfxYVelocityNegativeLimit;
extern const f32 gExpgfxSlotMotionStep;

extern const f32 gExpgfxBoundsInitMin;
extern const f32 gExpgfxBoundsInitMax;
extern const f32 gExpgfxU16ToUnitScale;
extern int gExpgfxSlotType1Count;
extern int lbl_803DD274;
extern int gExpgfxSlotType1Average;

extern u16 gExpgfxPhaseAngleB;
extern u16 gExpgfxPhaseAngleA;
extern f32 gExpgfxFrameTimerC;
extern f32 gExpgfxFrameTimerB;
extern f32 gExpgfxFrameTimerA;
extern f32 gExpgfxNearFadeDepth;

static inline ExpgfxTableEntry* Expgfx_GetTableEntry(int tableIndex)
{
    return &gExpgfxTableEntries[tableIndex];
}

static inline u32 Expgfx_GetSlotTableIndex(const ExpgfxSlot* slot)
{
    return ((u32)slot->encodedTableIndex >> 1) & EXPGFX_SLOT_TABLE_INDEX_MASK;
}

static inline void Expgfx_SetSlotTableIndex(ExpgfxSlot* slot, u8 tableIndex)
{
    slot->encodedTableIndex = (u8)((tableIndex << 1) | (slot->encodedTableIndex & 1));
}

static inline ExpgfxSlot* Expgfx_GetSlot(int poolIndex, int slotIndex)
{
    return (ExpgfxSlot*)(gExpgfxSlotPoolBases[poolIndex] + slotIndex * EXPGFX_SLOT_SIZE);
}

static inline ExpgfxBounds* Expgfx_GetBoundsTemplate(int templateIndex)
{
    return &((ExpgfxBounds*)gExpgfxStaticData)[templateIndex];
}

#define EXPGFX_POOL_ACTIVE_MASK_PTR(runtime, poolIndex) \
    ((u32*)((u8*)(runtime)->poolActiveMasks + (poolIndex) * sizeof(u32)))

int expgfx_addToTable(u32 resourceHandle, u32 sourceId, u32 attachedTableKey, s16 resourceId);

int expgfx_updateSourceFrameFlags(void* sourceObject)
{
    s16 signedPoolIndex;
    int result;
    ExpgfxSourceObject** poolSourceIds;
    int poolIndex;
    result = EXPGFX_SOURCE_FRAME_STATE_NONE;
    lbl_803DD253 = 0;
    poolIndex = 0;
    poolSourceIds = gExpgfxTrackedPoolSourceIds;

    for (; (s16)poolIndex < EXPGFX_POOL_COUNT; poolSourceIds++, poolIndex++)
    {
        if ((((ExpgfxSourceObject*)sourceObject)->objType == EXPGFX_SOURCE_OBJTYPE_MATCH_ALL) ||
            (*poolSourceIds == sourceObject))
        {
            s64 frameBit;

            signedPoolIndex = poolIndex;
            frameBit = 1 << (signedPoolIndex >> 1);
            if ((frameBit & gExpgfxTrackedSourceFrameMasks[signedPoolIndex & 1]) != 0)
            {
                gExpgfxStaticPoolFrameFlags[poolIndex] = EXPGFX_SOURCE_FRAME_STATE_B;
                if ((s8)result == EXPGFX_SOURCE_FRAME_STATE_A)
                {
                    result = EXPGFX_SOURCE_FRAME_STATE_MIXED;
                }
                else
                {
                    result = EXPGFX_SOURCE_FRAME_STATE_B;
                }
            }
            else
            {
                gExpgfxStaticPoolFrameFlags[poolIndex] = EXPGFX_SOURCE_FRAME_STATE_A;
                if ((s8)result == EXPGFX_SOURCE_FRAME_STATE_B)
                {
                    result = EXPGFX_SOURCE_FRAME_STATE_MIXED;
                }
                else
                {
                    result = EXPGFX_SOURCE_FRAME_STATE_A;
                }
            }
        }
        else
        {
            gExpgfxStaticPoolFrameFlags[poolIndex] = EXPGFX_SOURCE_FRAME_STATE_NONE;
        }
    }

    return result;
}

void expgfx_ownerFree3(u32 sourceId)
{
    expgfx_free(sourceId);
    return;
}

void expgfx_func0B_nop(void)
{
}

void expgfx_func0A_nop(void)
{
}

int expgfx_func09(void)
{
    return 0;
}