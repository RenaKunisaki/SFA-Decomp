#ifndef MAIN_LIGHTMAP_INTERNAL_H_
#define MAIN_LIGHTMAP_INTERNAL_H_

#include "main/dll/ppcwgpipe_struct.h"
#include "types.h"

typedef struct EnvironmentUpdateInterface
{
    void (*create)(void);
    void (*destroy)(void);
    void (*update)(void);
} EnvironmentUpdateInterface;

typedef struct
{
    u32 a;
    u32 b;
    u32 key;
    u32 d;
} LightSortEntry;

typedef struct
{
    u32 a, b, c, d;
} LightmapQEnt;

typedef struct MapLayerBuffers
{
    u8 reserved[0x41cc];
    u8* cellStates[5];
    u8* blockDescriptors[5];
    u8* blockIndices[5];
} MapLayerBuffers;

typedef struct
{
    u8 pad[0x4114];
    u32 deferred[20];
} LightmapDrawQueue;

typedef union
{
    double d;

    struct
    {
        u32 hi;
        u32 lo;
    } u;
} F64Cvt;

#endif /* MAIN_LIGHTMAP_INTERNAL_H_ */
