#ifndef MAIN_OBJPRINT_DOLPHIN_INTERNAL_H_
#define MAIN_OBJPRINT_DOLPHIN_INTERNAL_H_

#include "global.h"
#include "types.h"

/*
 * One render op ("shader") record from the model file's renderOps array,
 * bound by opcode 1 of the render-instruction stream.  Layer records
 * (Shader_getLayer) precede these fields; byte 0x41 holds the layer count
 * and byte 0x40 the layer blend flags (0x10 = additive path).
 * flags (+0x3C) bits seen in this file: 8 = backface cull, 0x100 = extra
 * projected-texture pass, 0x400 = alpha-test opaque, 0x200 = fuzz overlay
 * eligible, 0x20000 = water/caustic hook, 0x100000 = decal second layer,
 * 0x40000000 = force blend.
 */
typedef struct ObjModelRenderOp
{
    u8 pad0[0x18 - 0x0];
    u32 textureId;
    u32 unk1C;
    u8 pad20[0x24 - 0x20];
    u32 indirectTextureId;
    u8 pad28[0x34 - 0x28];
    u32 envTextureId;
    u8 pad38[0x3C - 0x38];
    u32 flags;
} ObjModelRenderOp;

// ObjModelRenderOp.flags (+0x3C) bits
#define SHADER_FLAG_BACKFACE_CULL      0x8
#define SHADER_FLAG_PROJECTED_TEX_PASS 0x100
#define SHADER_FLAG_ALPHA_TEST_OPAQUE  0x400
#define SHADER_FLAG_WATER_CAUSTIC      0x20000
#define SHADER_FLAG_DECAL_LAYER        0x100000
#define SHADER_FLAG_FORCE_BLEND        0x40000000

typedef struct
{
    u8* data;
    int pad[3];
    int pos;
} MtxBitStream;

typedef u8 (*ObjModelRenderCb)(int* obj, int* am, int p3);

typedef struct IndTexMtx23
{
    f32 m[2][3];
} IndTexMtx23;

STATIC_ASSERT(sizeof(IndTexMtx23) == 0x18);

#endif /* MAIN_OBJPRINT_DOLPHIN_INTERNAL_H_ */
