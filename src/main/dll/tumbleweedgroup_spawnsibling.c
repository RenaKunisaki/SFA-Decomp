#include "main/frame_timing.h"
#include "main/object_render.h"
#include "main/audio/sfx.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/audio/sfx_trigger_ids.h"
#include "game/objects/object.h"
#include "main/obj_list.h"
#include "sys/objects/lifecycle.h"
#include "sys/objects.h"
#include "main/dll/dll_00D1_tumbleweedbush.h"
#include "main/dll/dll_00D2_tumbleweed.h"
#include "game/objects/object_setup.h"
#include "main/objhits.h"
#include "main/sky_interface.h"
#include "main/objfx.h"
#include "main/vecmath_distance_api.h"
#include "main/vecmath.h"
#include "main/obj_group.h"
#include "main/track_dolphin_api.h"
#include "string.h"


#define TUMBLEWEED_BUSH_PIECE_SCALE 64.0f

s8 tumbleweedbush_spawnSibling(int* obj)
{
    u8* state;
    u8* placementData;
    int siblingType;
    int idx;
    int outCount;
    f32 sunTime;
    int freeSlot;
    u8* scan;
    int** list;
    int count;
    int* newObj;

    state = ((GameObject*)obj)->extra;
    placementData = *(u8**)&((GameObject*)obj)->anim.placementData;
    switch (((GameObject*)obj)->anim.seqId)
    {
    case TUMBLEWEEDBUSH_SEQ_A:
        if ((*gSkyInterface)->getSunPosition(&sunTime) == 0)
            return -1;
        siblingType = TUMBLEWEEDBUSH_SIBLING_A;
        break;
    case TUMBLEWEEDBUSH_SEQ_B:
        siblingType = TUMBLEWEEDBUSH_SIBLING_B;
        break;
    case TUMBLEWEEDBUSH_SEQ_C:
        siblingType = TUMBLEWEEDBUSH_SIBLING_C;
        break;
    case TUMBLEWEEDBUSH_SEQ_D:
        siblingType = TUMBLEWEEDBUSH_SIBLING_D;
        break;
    }

    idx = 0;
    freeSlot = -1;
    scan = state;
    while (idx < (int)((TumbleweedBushState*)state)->pieceCount && freeSlot == -1)
    {
        if (((TumbleweedBushState*)scan)->pieceObjects[0] == NULL)
            freeSlot = idx;
        scan += 4;
        idx++;
    }
    if (freeSlot == -1)
        return -1;

    list = ObjList_GetObjects(&idx, &outCount);
    count = 0;
    while (idx < outCount)
    {
        int j = *(int*)&idx;
        idx = j + 1;
        if (siblingType == ((GameObject*)list[j])->anim.seqId)
            count++;
    }
    if (count >= 7)
        return -1;
    if (Obj_IsLoadingLocked() == 0)
        return -1;

    newObj = (int*)Obj_AllocObjectSetup(0x20, siblingType);
    ((ObjPlacement*)newObj)->posX =
        ((GameObject*)obj)->anim.localPosX + ((TumbleweedBushState*)state)->pieceOffsets[freeSlot][0];
    ((ObjPlacement*)newObj)->posY =
        ((GameObject*)obj)->anim.localPosY + ((TumbleweedBushState*)state)->pieceOffsets[freeSlot][1];
    ((ObjPlacement*)newObj)->posZ =
        ((GameObject*)obj)->anim.localPosZ + ((TumbleweedBushState*)state)->pieceOffsets[freeSlot][2];
    ((ObjPlacement*)newObj)->color[0] = ((TumbleweedBushPlacement*)placementData)->base.color[0];
    ((ObjPlacement*)newObj)->color[1] = ((TumbleweedBushPlacement*)placementData)->base.color[1];
    ((ObjPlacement*)newObj)->color[2] = ((TumbleweedBushPlacement*)placementData)->base.color[2];
    ((ObjPlacement*)newObj)->color[3] = ((TumbleweedBushPlacement*)placementData)->base.color[3];
    ((TumbleweedBushPlacement*)newObj)->scale = TUMBLEWEED_BUSH_PIECE_SCALE;

    if ((((TumbleweedBushState*)state)->variant & 1) != 0)
    {
        switch (((ObjPlacement*)((GameObject*)obj)->anim.placementData)->mapId)
        {
        case 0x292c:
            if (((TumbleweedBushState*)state)->spawnedCount == 6)
            {
                ((TumbleweedBushPlacement*)newObj)->radiusByte = 1;
                list = ObjList_GetObjects(&idx, &outCount);
                while (idx < outCount)
                {
                    int* child = list[idx];
                    if (((GameObject*)child)->anim.seqId == 0x27f)
                    {
                        ((ObjPlacement*)newObj)->posX = ((GameObject*)child)->anim.localPosX;
                        ((ObjPlacement*)newObj)->posY = ((GameObject*)list[idx])->anim.localPosY;
                        ((ObjPlacement*)newObj)->posZ = ((GameObject*)list[idx])->anim.localPosZ;
                        idx = outCount;
                    }
                    idx++;
                }
            }
            break;
        }
    }

    {
        int* setup = (int*)Obj_SetupObject((ObjPlacement*)newObj, 5, ((GameObject*)obj)->anim.mapEventSlot, -1,
                                           ((GameObject*)obj)->anim.parent);
        u8* slotBase = (u8*)((TumbleweedBushState*)state)->pieceObjects;
        *(int**)(slotBase + freeSlot * 4) = setup;
        {
            int* spawned = *(int**)(slotBase + freeSlot * 4);
            ((void (*)(int*, f64, f64)) * (int*)(*(int*)(*(int*)((char*)spawned + 0x68)) + 0x24))(
                spawned, (f64)((GameObject*)obj)->anim.localPosX, (f64)((GameObject*)obj)->anim.localPosZ);
        }
    }
    ((TumbleweedBushState*)state)->spawnedCount += 1;
    return freeSlot;
}
