/*
 * DIM2PathGen (DLL 0x1D8) - snowball path-generator for Snowhorn Wastes 2.
 * Finds and loads a RomCurve spline near its placement position (curve group 21),
 * then periodically spawns dim2snowball objects (DLL type in spawnTypes[]) from a
 * free pool (object group 47) or via Obj_AllocObjectSetup, alternating between two
 * snowball types per spawn. Spawn rate and type are set from the placement data.
 */
#include "main/dll/dim2pathgeneratorstate_struct.h"
#include "dlls/objects/471_DIM2SnowBal.h"
#include "game/objects/object_setup.h"
#include "dlls/object_descriptor.h"
#include "main/dll/rom_curve_interface.h"
#include "game/objects/object.h"
#include "main/gamebits.h"
#include "sys/objects.h"
#include "sys/objects/lifecycle.h"
#include "main/frame_timing.h"
#include "main/obj_group.h"

#define DIM2PATHGENERATOR_OBJFLAG_HITDETECT_DISABLED 0x2000

typedef struct Dim2pathgeneratorObjectDef
{
    ObjPlacement head; /* 0x00..0x17 (mapId at 0x14) */
    s16 spawnPeriod;
    s16 unk1A;
    s16 unk1C;
    u16 spawnType0; /* 0x1E primary spawn object type -> state->spawnTypes[0] */
    s16 spawnType1; /* 0x20 secondary spawn type (-1 = reuse spawnType0) -> spawnTypes[1] */
    u8 pad22[0x28 - 0x22];
} Dim2pathgeneratorObjectDef;

/* RomCurve definition record (subset) returned by gRomCurveInterface->getById. */
typedef struct Dim2RomCurveDef
{
    u8 pad0[0x8 - 0x0];
    f32 originX;
    f32 originY;
    f32 originZ;
} Dim2RomCurveDef;

typedef struct Dim2pathgeneratorPlacement
{
    ObjPlacement base; /* color/mapActFlagsLo copied into each spawn setup's head */
    s16 spawnPeriod; /* 0x18 */
    s16 unk1A;
    s16 unk1C;
    u16 spawnType0; /* 0x1E */
    s16 spawnType1; /* 0x20 */
    s16 activeGameBit;
    u8 pad24[0x28 - 0x24];
} Dim2pathgeneratorPlacement;

STATIC_ASSERT(sizeof(Dim2PathGeneratorState) == 0x9a8);

#define CURVE_GROUP_SNOWBALL_PATH   21
#define OBJ_GROUP_SNOWBALL_POOL     47

u8 DIM2PathGenerator_getCurveVals(GameObject* obj, int** p1, int** p2, int** p3, int** p4)
{
    int* state = obj->extra;
    *p1 = (int*)((char*)state + 12);
    *p2 = (int*)((char*)state + 812);
    *p3 = (int*)((char*)state + 1612);
    if (p4 != NULL)
    {
        *p4 = (int*)((char*)state + 2412);
    }
    return ((Dim2PathGeneratorState*)state)->curveValid;
}

int DIM2PathGenerator_getExtraSize(void)
{
    return sizeof(Dim2PathGeneratorState);
}

int DIM2PathGenerator_getObjectTypeId(void)
{
    return 0;
}

void DIM2PathGenerator_free(void)
{
}

void DIM2PathGenerator_render(void)
{
}

void DIM2PathGenerator_hitDetect(void)
{
}

void DIM2PathGenerator_update(GameObject* obj)
{
    int* def;
    int* extra = obj->extra;
    int toggle;
    int** objs;
    int i;
    int curveGroup;
    int count;

    def = *(int**)&obj->anim.placementData;
    if (mainGetBit(((Dim2pathgeneratorPlacement*)def)->activeGameBit) == 0)
    {
        return;
    }
    if ((((Dim2PathGeneratorState*)extra)->flags & 4) != 0)
    {
        if ((((Dim2PathGeneratorState*)extra)->flags & 2) == 0)
        {
            int found;
            curveGroup = CURVE_GROUP_SNOWBALL_PATH;
            found = (*gRomCurveInterface)->find(
                obj->anim.localPosX, obj->anim.localPosY,
                obj->anim.localPosZ, &curveGroup, 1, 10);
            if (found != -1)
            {
                int* cv = (int*)(*gRomCurveInterface)->getById(found);
                (*gRomCurveInterface)->countRandomPoints((RomCurveDef*)cv);
                ((Dim2PathGeneratorState*)extra)->curveValid =
                    (*gRomCurveInterface)->buildRandomPoints((RomCurvePlacementDef*)cv, (f32*)((char*)extra + 0xc),
                                                            (f32*)((char*)extra + 0x32c),
                                                            (f32*)((char*)extra + 0x64c),
                                                            (s8*)((char*)extra + 0x96c));
                ((Dim2PathGeneratorState*)extra)->flags |= 2;
                ((Dim2PathGeneratorState*)extra)->originX = ((Dim2RomCurveDef*)cv)->originX;
                ((Dim2PathGeneratorState*)extra)->originY = ((Dim2RomCurveDef*)cv)->originY;
                ((Dim2PathGeneratorState*)extra)->originZ = ((Dim2RomCurveDef*)cv)->originZ;
            }
        }
    }
    else
    {
        ((Dim2PathGeneratorState*)extra)->originX = obj->anim.localPosX;
        ((Dim2PathGeneratorState*)extra)->originY = obj->anim.localPosY;
        ((Dim2PathGeneratorState*)extra)->originZ = obj->anim.localPosZ;
    }
    if ((((Dim2PathGeneratorState*)extra)->spawnTimer -= framesThisStep) > 0)
    {
        return;
    }
    toggle = ((Dim2PathGeneratorState*)extra)->flags & 1;
    ((Dim2PathGeneratorState*)extra)->spawnTimer = ((Dim2PathGeneratorState*)extra)->spawnPeriod;
    ((Dim2PathGeneratorState*)extra)->flags &= ~1;
    objs = (int**)ObjGroup_GetObjects(OBJ_GROUP_SNOWBALL_POOL, &count);
    for (i = 0; i < count; i++)
    {
        if (((Dim2PathGeneratorState*)extra)->spawnTypes[toggle] == ((GameObject*)objs[i])->anim.seqId)
        {
            int* p = *(int**)((char*)objs[i] + 0x4c);
            int j;
            int** o2;
            ((Dim2SnowBallPlacement*)p)->base.posX = ((Dim2PathGeneratorState*)extra)->originX;
            ((Dim2SnowBallPlacement*)p)->base.posY = ((Dim2PathGeneratorState*)extra)->originY;
            ((Dim2SnowBallPlacement*)p)->base.posZ = ((Dim2PathGeneratorState*)extra)->originZ;
            ((Dim2SnowBallPlacement*)p)->base.mapId = ((Dim2pathgeneratorPlacement*)def)->base.mapId;
            (*(void (**)(int*, int*, int))(**(int**)((char*)objs[i] + 0x68) + 4))(objs[i], p, 1);
            ObjGroup_RemoveObject((int)objs[i], OBJ_GROUP_SNOWBALL_POOL);
            o2 = (int**)ObjGroup_GetObjects(OBJ_GROUP_SNOWBALL_POOL, &count);
            for (j = 0; j < count; j++)
            {
            }
            ((Dim2PathGeneratorState*)extra)->flags |= (toggle ^ 1) & 1;
            return;
        }
    }
    if (Obj_IsLoadingLocked())
    {
        Dim2SnowBallPlacement* np = (Dim2SnowBallPlacement*)Obj_AllocObjectSetup(
            36, ((s16*)((Dim2PathGeneratorState*)extra)->spawnTypes)[toggle]);
        np->base.posX = ((Dim2PathGeneratorState*)extra)->originX;
        np->base.posY = ((Dim2PathGeneratorState*)extra)->originY;
        np->base.posZ = ((Dim2PathGeneratorState*)extra)->originZ;
        np->base.color[0] = ((Dim2pathgeneratorPlacement*)def)->base.color[0];
        np->base.color[2] = ((Dim2pathgeneratorPlacement*)def)->base.color[2];
        np->base.color[1] = ((Dim2pathgeneratorPlacement*)def)->base.color[1];
        np->base.color[3] = ((Dim2pathgeneratorPlacement*)def)->base.color[3];
        np->base.color[3] = 255;
        np->base.mapActFlagsLo = ((Dim2pathgeneratorPlacement*)def)->base.mapActFlagsLo;
        np->rotationXByte = (s8) * (u8*)((char*)def + 0x1c);
        np->unknown1A = *(u8*)((char*)def + 0x1a);
        np->unknown1C = *(u8*)((char*)def + 0x1b);
        np->base.mapId = ((Dim2pathgeneratorPlacement*)def)->base.mapId;
        Obj_SetupObject((ObjPlacement*)np, 5, obj->anim.mapEventSlot, -1, NULL);
        ((Dim2PathGeneratorState*)extra)->flags |= (toggle ^ 1) & 1;
    }
}

void DIM2PathGenerator_init(GameObject* obj, int* def)
{
    Dim2PathGeneratorState* state;
    *(s16*)obj = (s16)((u32) * (u8*)((char*)def + 28) << 8);
    state = obj->extra;
    state->spawnPeriod = ((Dim2pathgeneratorObjectDef*)def)->spawnPeriod;
    state->spawnTimer = (s16) * (u8*)((char*)def + 29);
    state->spawnTypes[0] = (s16)((Dim2pathgeneratorObjectDef*)def)->spawnType0;
    {
        s16 v = ((Dim2pathgeneratorObjectDef*)def)->spawnType1;
        if (v == -1)
        {
            state->spawnTypes[1] = (s16)((Dim2pathgeneratorObjectDef*)def)->spawnType0;
        }
        else
        {
            state->spawnTypes[1] = v;
        }
    }
    state->flags = (u8)(state->flags | 4);
    obj->objectFlags = (u16)(obj->objectFlags | DIM2PATHGENERATOR_OBJFLAG_HITDETECT_DISABLED);
}

void DIM2PathGenerator_release(void)
{
}

void DIM2PathGenerator_initialise(void)
{
}

ObjectDescriptor11WithPadding gDIM2PathGeneratorObjDescriptor = {
    {
        0,
        0,
        0,
        OBJECT_DESCRIPTOR_FLAGS_11_SLOTS,
        (ObjectDescriptorCallback)DIM2PathGenerator_initialise,
        (ObjectDescriptorCallback)DIM2PathGenerator_release,
        0,
        (ObjectDescriptorCallback)DIM2PathGenerator_init,
        (ObjectDescriptorCallback)DIM2PathGenerator_update,
        (ObjectDescriptorCallback)DIM2PathGenerator_hitDetect,
        (ObjectDescriptorCallback)DIM2PathGenerator_render,
        (ObjectDescriptorCallback)DIM2PathGenerator_free,
        (ObjectDescriptorCallback)DIM2PathGenerator_getObjectTypeId,
        (ObjectDescriptorExtraSizeCallback)DIM2PathGenerator_getExtraSize,
        (ObjectDescriptorCallback)DIM2PathGenerator_getCurveVals,
    },
    0,
};
