#define OBJHITS_STATE_INDEX_S8
#define TEX_SETSHADER_U8
#include "main/dll/partfx_interface.h"
#include "dolphin/os/OSReport.h"
#include "dolphin/mtx.h"
#include "main/asset_load.h"
#include "main/gameloop_api.h"
#include "main/pi_data_file_api.h"
#include "main/pi_dolphin_api.h"
#include "main/pi_flush_api.h"
#include "main/rcp_dolphin_api.h"
#include "main/rcp_dolphin_render_api.h"
#include "main/debug.h"
#include "main/frustum.h"
#include "main/shader_api.h"
#include "main/shader_map_api.h"
#include "main/shader_map_text_api.h"
#include "main/map_romlist_page.h"
#include "main/textrender_api.h"
#include "main/texture.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_float_helpers.h"
#include "main/render_envfx_api.h"
#include "main/model_render_instrs_api.h"
#include "main/audio/audio_control_api.h"
#include "main/audio/sfx.h"
#include "main/camera_interface.h"
#include "main/checkpoint_interface.h"
#include "main/dll_000A_expgfx.h"
#include "main/dll/modgfx_interface.h"
#include "main/dll/projgfx_interface.h"
#include "main/dll/cloudaction_interface.h"
#include "main/dll/waterfx_interface.h"
#include "main/dll/rom_curve_interface.h"
#include "main/mapEvent.h"
#include "main/mldf_fileid.h"
#include "main/minimap_api.h"
#include "main/newclouds.h"
#include "main/objseq.h"
#include "main/pad.h"
#include "main/sky_interface.h"
#include "main/sky_api.h"

extern char sTrackLoadBlockOverrunError[];
#include "main/camera.h"
#include "main/object_transform.h"
#include "main/mm.h"
#include "main/voxmaps.h"
#include "main/warpvec.h"
#include "main/dll/dll_0000_gameui_api.h"
#include "main/dll/savegame.h"
#include "main/loaded_file_flags.h"
#include "main/map_load.h"
#include "main/map_texscroll.h"
#include "main/fileio.h"
#include "game/objects/object.h"
#include "sys/objects.h"
#include "main/obj_group.h"
#include "main/obj_list.h"
#include "main/track_dolphin_api.h"
#include "dolphin/os/OSCache.h"
extern char sShaderDebugStrings[];
#define MAP_BLOCK_LAYER_COUNT 5
#define FRUSTUM_PLANE_COUNT   5
void trackLoadBlockEnd(MapBlockData* block, int blockId, int slotIdx, int layer);
/* One 0x20-byte MAPINFO.bin (fileId 0x1f) record, fetched by mapId via getTabEntry. */
typedef struct MapInfoRecord
{
    u8 unk00[0x1c];
    s8 mapType; /* +0x1c: MapType */
    u8 unk1d;
    s16 unk1e; /* +0x1e */
} MapInfoRecord;
extern WarpVec lbl_80386648[];
extern const f32 gMapBlockWorldSize;

#include "sys/objects/lifecycle.h"
#include "game/objects/object_setup.h"
#include "track/intersect_api.h"
#include "main/model.h"
#include "main/pi_dolphin.h"
#include "main/track_dolphin_shadow_api.h"
#include "main/dll/dll_0017_savegame_api.h"
#include "main/objprint_dolphin_api.h"
#include "main/dll/savegame_env_api.h"
#include "main/dll/tricky_api.h"
#include "main/screen_transition.h"
#include "dolphin/gx/GXCull.h"
#include "string.h"

int lbl_803DB620 = -1;
s8 lbl_803DB624[8] = {0, -2, -1, 1, 2, 0, 0, 0};
f32 lbl_803DB62C = 0.5f;
extern int lbl_803822A0[5];
extern f32 lbl_803DEBCC;

/* the ice-mountain snowbike; its map-block residency is tracked separately so the
   ride streams blocks ahead. retail OBJECTS.bin name "IMSnowBike" (DLL 0x255) */
#define SHADER_SNOWBIKE_OBJ 0x72
void defStartFn_8005972c(char* p1, u32* p2, int idx, int flag);
extern f32 gShaderLoadCenterZ;
extern f32 gShaderLoadCenterY;
extern f32 gShaderLoadCenterX;
extern int gShaderCurMapEventId;
int mapCoordsToId(int x, int z, int layer);
extern s16* gMapBlockIds;
extern u8 lbl_803DCE98;
extern u8* gMapBlockRefCounts;
extern u32 lbl_8037E0C0[];
typedef struct ShaderRomListSlot
{
    void* romlist;
    s16 slot;
    s8 flag;
    s8 pad;
} ShaderRomListSlot;
extern int lbl_803DCE7C;
extern int gShaderMapRomBuffers[];
#define INIT_MAP_SLOT(slot)                                                                                 \
    e = (char*)gShaderMapRomBuffers[1] + (slot) * 10 + ofs[0];                                              \
    *(s8*)((char*)gShaderMapRomBuffers[3] + idx + (slot)) = -128;                                          \
    *(s16*)(e + 0) = -32768;                                                                               \
    *(s16*)(e + 2) = -32768;                                                                               \
    *(s16*)(e + 4) = -32768;                                                                               \
    *(s16*)(e + 6) = -32768;                                                                               \
    *(s8*)(e + 8) = -128;                                                                                  \
    *(s8*)(e + 9) = -128;                                                                                  \
    ((s16*)gShaderMapRomBuffers[2])[(idx + (slot)) << 1] = -1;                                             \
    ((s16*)gShaderMapRomBuffers[2])[((idx + (slot)) << 1) + 1] = -1
extern s8* gMapLayerCellStates;
extern int gMapPendingFileFlags;
extern int* gMapBlockIndexList;
extern int gMapBlockIndexCount;
extern s16 lbl_803DCE70;
extern u8 lbl_803DCDED;
extern void* lbl_803DCEA8;
extern int lbl_803DCE74;

typedef struct MapLoadRec
{
    s16 x;
    s16 z;
    s16 blockId;
    s16 layer;
} MapLoadRec;

int mapProcessRomList(int slot);

extern s32 bEnableColorFilter;
extern u8 bEnableViewFinderHud;
extern u8 bEnableSpiritVision;
extern u8 bEnableMonochromeFilter;
extern u8 bEnableMotionBlur;
u32 Rcp_GetColorFilterEnabled(void)
{
    return bEnableColorFilter;
}

void Rcp_SetColorFilterEnabled(u32 x)
{
    bEnableColorFilter = x;
}

void ObjHits_ConvertHitPositionToWorld(GameObject* object, f32* position)
{
    if (object->anim.parent != NULL)
        return;
    position[0] = position[0] + playerMapOffsetX;
    position[2] = position[2] + playerMapOffsetZ;
}

extern u8 bEnableDistortionFilter;
extern u8 bEnableBlurFilter;
void Rcp_DisableDistortionFilter(void)
{
    bEnableDistortionFilter = 0x0;
}

extern f32 distortionFilterVector[];
extern f32 distortionFilterAngle1;
extern f32 distortionFilterAngle2;
extern u8 distortionFilterColor[3];

void turnOnDistortionFilter(f32* vec, f32 angle2, u32* color, f32 angle1)
{
    u8* colorBytes = (u8*)color;

    distortionFilterVector[0] = vec[0];
    distortionFilterVector[1] = vec[1];
    distortionFilterVector[2] = vec[2];
    distortionFilterAngle2 = angle2;
    distortionFilterColor[0] = colorBytes[0];
    distortionFilterColor[1] = colorBytes[1];
    distortionFilterColor[2] = colorBytes[2];
    distortionFilterAngle1 = angle1;
    bEnableDistortionFilter = 1;
}

extern char lbl_803822C8[];
extern int gHeatEffectFadeDirection;

void Rcp_DisableHeatEffect(void)
{
    u8* p = saveGameGetEnvState();
    gHeatEffectFadeDirection = -1;
    p[0x40] = (u8)(p[0x40] & ~0x20);
}

void Rcp_EnableHeatEffect(void)
{
    u8* p = saveGameGetEnvState();
    gHeatEffectFadeDirection = 1;
    p[0x40] = (u8)(p[0x40] | 0x20);
}
void Rcp_DisableBlurFilter(void)
{
    bEnableBlurFilter = 0x0;
}

extern f32 lbl_803DCE50;
extern f32 lbl_803DCE4C;
extern f32 blurFilterArea;
extern u8 bBlurFilterUseArea;
extern u8 bBiggerBlurFilter;

void turnOnBlurFilter(f32 x, f32 y, f32 z, u8 useArea, u8 bigger)
{
    bEnableBlurFilter = 1;
    lbl_803DCE50 = x;
    lbl_803DCE4C = y;
    blurFilterArea = z;
    bBlurFilterUseArea = useArea;
    bBiggerBlurFilter = bigger;
}

u8 Rcp_GetViewFinderHudEnabled(void)
{
    return bEnableViewFinderHud;
}
void Rcp_SetViewFinderHudEnabled(u8 x)
{
    bEnableViewFinderHud = x;
}

void Rcp_SetSpiritVisionEnabled(u8 x)
{
    bEnableSpiritVision = x;
}

void Rcp_SetMonochromeFilterEnabled(u8 x)
{
    bEnableMonochromeFilter = x;
}

int Rcp_GetMotionBlurEnabled(void)
{
    return bEnableMotionBlur;
}

void setMotionBlur(u8 enabled, f32 amount)
{
    bEnableMotionBlur = enabled;
    lbl_803DB62C = amount;
}

void gxSetScissorRect(int p1, int p2, int x, int y, int x2, int y2)
{
    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    if (x2 < 0)
        x2 = 0;
    if (y2 < 0)
        y2 = 0;
    GXSetScissor(x, y, x2 - x, y2 - y);
}

/* Pending warp destination saved by warpToMap from the map-warp tab entry and
 * applied to the player position on map reload (vec3 + a map-layer s16 and a
 * facing-angle s16, each truncated to s8 into the pos map/angle bytes).
 * 16-byte record of WARPTAB.bin (fileId 0x1c). */
typedef struct WarpDestination
{
    f32 x;
    f32 y;
    f32 z;
    s16 layer;
    s16 angle;
} WarpDestination;

extern u8 gRcpPendingWarpDest[];
extern u8* lbl_803DCE78;
extern s16 lbl_803DCEBA;
extern u8 gRcpWarpTransitionType;
extern u8 lbl_803DCEBD;
extern s16 lbl_803DCEB8;
extern u8 lbl_803DCDE0;
extern u8 lbl_803DCA40;

void loadNextMap(void)
{
    u8* pos;
    pos = (*gMapEventInterface)->getCurCharPos();
    if (lbl_803DCEB8 != -1)
    {
        lbl_803DCDE0 -= 1;
        if ((s8)lbl_803DCDE0 < 0)
        {
            if (lbl_803DCEB8 > -1 && (s8)gRcpWarpTransitionType != 0)
            {
                (*gScreenTransitionInterface)->step(3, 1);
            }
            lbl_803DCEB8 = -1;
            Pause_SetDisabled(0);
        }
    }
    if ((s8)lbl_803DCEBD != 0)
    {
        if ((*gScreenTransitionInterface)->isFinished() != 0 || (s8)gRcpWarpTransitionType == 0)
        {
            (*gCloudActionInterface)->freeCloudObjects();
            (*gCloudActionInterface)->onMapSetup();
            (*gSky2Interface)->onMapSetup();
            (*gSkyInterface)->loadLights();
            (*gNewCloudsInterface)->onMapSetup();
            gameUiResetMenuState();
            lbl_803DCEBD = 0;
            *(f32*)(pos + 0) = ((WarpDestination*)gRcpPendingWarpDest)->x;
            *(f32*)(pos + 4) = ((WarpDestination*)gRcpPendingWarpDest)->y;
            *(f32*)(pos + 8) = ((WarpDestination*)gRcpPendingWarpDest)->z;
            *(s8*)(pos + 0xd) = (s8)((WarpDestination*)gRcpPendingWarpDest)->layer;
            *(s8*)(pos + 0xc) = (s8)((WarpDestination*)gRcpPendingWarpDest)->angle;
            mapReload();
            lbl_803DCEB8 = lbl_803DCEBA;
            lbl_803DCEBA = -1;
            lbl_803DCDE0 = 8;
            lbl_803DCA40 = 1;
            blankScreen(1);
        }
    }
}

void warpToMap(int idx, s8 transType)
{
    u8* p = lbl_803DCE78;
    getTabEntry(p, MLDF_FILEID_WARPTAB_BIN, idx << 4, 16);
    ((WarpDestination*)gRcpPendingWarpDest)->x = ((WarpDestination*)p)->x;
    ((WarpDestination*)gRcpPendingWarpDest)->y = ((WarpDestination*)p)->y;
    ((WarpDestination*)gRcpPendingWarpDest)->z = ((WarpDestination*)p)->z;
    ((WarpDestination*)gRcpPendingWarpDest)->layer = ((WarpDestination*)p)->layer;
    ((WarpDestination*)gRcpPendingWarpDest)->angle = ((WarpDestination*)p)->angle;
    lbl_803DCEBA = (s16)idx;
    lbl_803DCEBD = 1;
    *(s8*)&gRcpWarpTransitionType = transType;
    if (transType != 0)
    {
        (*gScreenTransitionInterface)->start(2, 1);
    }
    Pause_SetDisabled(1);
}

static inline int objIsVisibleInAct(u8* def, int act)
{
    if (act == -1)
    {
        return 0;
    }
    if (act != 0)
    {
        if (act < 9)
        {
            if ((def[3] >> (act - 1)) & 1)
                return 0;
        }
        else
        {
            if ((def[5] >> (0x10 - act)) & 1)
                return 0;
        }
    }
    return 1;
}

void mapInstantiateObjects(MapRomListPage* page, int mapId, int index, GameObject* parent)
{
    int* seg = (int*)(lbl_803822C8 + mapId * 0x8c);
    int i;
    char* p;
    char* end;
    char* romBase;
    char* objStart;
    int objIndex;
    char* obj;
    int v;
    int flag;
    int byteIdx;
    int bit;
    s8* vis;

    if (seg[index] == -1)
        return;
    objIndex = 0;
    romBase = (char*)page->objects;
    p = romBase;
    objStart = romBase + seg[index];
    while (p < objStart)
    {
        objIndex++;
        p += *(u8*)(p + 2) * 4;
    }
    for (i = index + 1; i <= 0x20; i++)
    {
        if (seg[i] != -1)
            break;
    }
    obj = objStart;
    end = romBase + seg[i];

    while (obj < end)
    {
        /* i reused below as the object-visible flag */
        if (objIndex < 0)
        {
            i = 0;
        }
        else
        {
            MapRomListPage* bm = gLoadedRomListPages[mapId];
            byteIdx = objIndex >> 3;
            if (byteIdx >= 0xc4)
            {
                i = 0;
            }
            else
            {
                i = 1;
                bit = 1 << (objIndex & 7);
                vis = (s8*)bm->loadedObjectBits;
                if ((bit & vis[byteIdx]) != 0)
                    i = 1;
                else
                    i = 0;
            }
        }
        if (i == 0)
        {
            v = (*gMapEventInterface)->getMapAct(mapId);
            flag = objIsVisibleInAct((u8*)obj, v);
            if (flag != 0)
            {
                if (objIndex >= 0)
                {
                    MapRomListPage* bm2 = gLoadedRomListPages[mapId];
                    byteIdx = objIndex >> 3;
                    bit = 1 << (objIndex & 7);
                    vis = (s8*)bm2->loadedObjectBits;
                    vis[byteIdx] &= ~bit;
                    vis = (s8*)bm2->loadedObjectBits;
                    vis[byteIdx] |= bit;
                }
                Obj_SetupObject((ObjPlacement*)obj, 1, mapId, objIndex, parent);
            }
        }
        objIndex++;
        obj += *(u8*)(obj + 2) * 4;
    }
}

int objShouldUnload(GameObject* obj)
{
    u8* def;
    u8* p;
    u8* src;
    s8** tp;
    int m;
    int keep;
    int bx;
    int bz;
    int k;
    int flags;
    int idx2;
    s8 found;
    f32 x;
    f32 y;
    f32 z;
    f32 dist;

    def = *(u8**)&((GameObject*)obj)->anim.placementData;
    if (def == NULL)
    {
        return 0;
    }
    if (def[4] & 2)
    {
        return 0;
    }
    m = (*gMapEventInterface)->getMapAct(((GameObject*)obj)->anim.mapEventSlot);
    keep = objIsVisibleInAct(def, m);
    if (keep == 0)
    {
        return 1;
    }
    flags = def[4];
    if (flags & 1)
    {
        return 0;
    }
    if (flags & 0x10)
    {
        return !(u8)(*gMapEventInterface)->getObjGroupStatus(((GameObject*)obj)->anim.mapEventSlot, def[6]);
    }
    if (((GameObject*)obj)->pendingParentObj != NULL && ((GameObject*)obj)->seqIndex < 0)
    {
        return 0;
    }
    if (((GameObject*)obj)->ownerObj != NULL)
    {
        return 0;
    }
    if (((GameObject*)obj)->anim.parent == NULL)
    {
        bx = (int)fastFloorf((((GameObject*)obj)->anim.localPosX - playerMapOffsetX) / gMapBlockWorldSize);
        bz = (int)fastFloorf((((GameObject*)obj)->anim.localPosZ - playerMapOffsetZ) / gMapBlockWorldSize);
        if (bx < 0 || bz < 0 || bx >= 0x10 || bz >= 0x10)
        {
            return 1;
        }
        found = 0;
        bx = bx + (bz << 4);
        tp = gMapBlockLayerTables;
        for (k = 0; k < MAP_BLOCK_LAYER_COUNT; k++)
        {
            if ((*tp)[bx] >= 0)
            {
                found = 1;
            }
            tp++;
        }
        if (found == 0)
        {
            return 1;
        }
    }
    flags = def[4];
    if (flags & 0x20)
    {
        return 0;
    }
    if ((flags & 4) && (p = (u8*)Obj_GetPlayerObject()) != NULL && ((GameObject*)obj)->anim.parent == NULL)
    {
        x = ((GameObject*)p)->anim.worldPosX;
        y = ((GameObject*)p)->anim.worldPosY;
        z = ((GameObject*)p)->anim.worldPosZ;
    }
    else
    {
        src = *(u8**)&((GameObject*)obj)->anim.parent;
        if (src != NULL)
        {
            idx2 = (s8)src[0x35] + 1;
        }
        else
        {
            idx2 = 0;
        }
        x = lbl_80386648[idx2].x;
        y = lbl_80386648[idx2].y;
        z = lbl_80386648[idx2].z;
    }
    dist = ((GameObject*)obj)->anim.loadDistance;
    if (((GameObject*)obj)->anim.parent != NULL)
    {
        x -= ((GameObject*)obj)->anim.localPosX;
        y -= ((GameObject*)obj)->anim.localPosY;
        z -= ((GameObject*)obj)->anim.localPosZ;
    }
    else
    {
        x -= ((GameObject*)obj)->anim.worldPosX;
        y -= ((GameObject*)obj)->anim.worldPosY;
        z -= ((GameObject*)obj)->anim.worldPosZ;
    }
    if (x * x + y * y + z * z < (40.0f + dist) * (40.0f + dist))
    {
        return 0;
    }
    return 1;
}

static inline int objVisibleForAct(ObjPlacement* placement, int t)
{
    if (t == -1)
    {
        return 0;
    }
    if (t != 0)
    {
        if (t < 9)
        {
            if ((placement->mapActFlagsLo >> (t - 1)) & 1)
            {
                return 0;
            }
        }
        else
        {
            if ((placement->mapActFlagsHi >> (16 - t)) & 1)
            {
                return 0;
            }
        }
    }
    return 1;
}

int objShouldLoad(ObjPlacement* placement, s8 viewSlot, int mapEventGroup)
{
    char* strs;
    int verbose;
    int useObj;
    f32 y;
    f32 z;
    f32 x;
    int t;
    int bx;
    int bz;
    s8 found;
    s8 i;
    int* tbl;
    GameObject* player;
    int off;
    f32* p;
    f32 d;
    f32 dz;
    f32 dy;
    f32 range;

    strs = sShaderDebugStrings;
    if (placement->mapId == 0x49054)
    {
        verbose = 1;
    }
    else
    {
        verbose = 0;
    }
    t = (*gMapEventInterface)->getMapAct(mapEventGroup);
    if (objVisibleForAct(placement, t) == 0)
    {
        return 0;
    }
    if (placement->loadFlags & 1)
    {
        if (verbose)
        {
            OSReport(strs + 0x1cc);
        }
        return 1;
    }
    if (placement->loadFlags & 2)
    {
        if (verbose)
        {
            OSReport(strs + 0x1e8);
        }
        return 0;
    }
    if (viewSlot == 0)
    {
        bx = fastFloorf((placement->posX - playerMapOffsetX) / gMapBlockWorldSize);
        bz = fastFloorf((placement->posZ - playerMapOffsetZ) / gMapBlockWorldSize);
        if (bx < 0 || bz < 0 || bx >= 16 || bz >= 16)
        {
            if (verbose)
            {
                OSReport(strs + 0x200, &placement->posX, &placement->posY, &placement->posZ);
            }
            return 0;
        }
        found = 0;
        bx += bz << 4;
        for (i = 0; i < MAP_BLOCK_LAYER_COUNT; i++)
        {
            if (gMapBlockLayerTables[i][bx] >= 0)
            {
                found = 1;
            }
        }
        if (found == 0)
        {
            if (verbose)
            {
                OSReport(strs + 0x228);
            }
            return 0;
        }
    }
    if (placement->loadFlags & 0x20)
    {
        if (verbose)
        {
            OSReport(strs + 0x240);
        }
        return 1;
    }
    useObj = 0;
    if ((placement->loadFlags & 4) && viewSlot == 0)
    {
        player = Obj_GetPlayerObject();
        if (player != NULL)
        {
            x = ((GameObject*)player)->anim.worldPosX;
            y = ((GameObject*)player)->anim.worldPosY;
            z = ((GameObject*)player)->anim.worldPosZ;
        }
        else
        {
            useObj = 1;
        }
    }
    else
    {
        useObj = 1;
    }
    if (useObj != 0)
    {
        off = viewSlot << 4;
        x = lbl_80386648[viewSlot].x;
        p = (f32*)((u8*)lbl_80386648 + off);
        y = p[1];
        z = p[2];
    }
    range = (f32)(placement->loadRange << 3);
    d = x - placement->posX;
    dy = y - placement->posY;
    dz = z - placement->posZ;
    d = d * d + dy * dy + dz * dz;
    if (d < range * range)
    {
        if (verbose)
        {
            OSReport(strs + 0x25c, &d);
        }
        return 1;
    }
    if (verbose)
    {
        OSReport(strs + 0x274);
    }
    return 0;
}
void mapLoadUnloadObjects(int flag)
{
    int grpBit;
    u32 objStart;
    GameObject* obj;
    int unload;
    int bit;
    u8 mask;
    u8* bp;
    u32 bits;
    int slot;
    int i;
    int objCount;
    s16 list[8];
    s16* idPtr;
    char* base;
    ObjPlacement* fp;
    int* tp;
    u32 cur;
    u32 end;
    s16 count;
    int vis;
    int idx;

    base = (char*)lbl_8037E0C0;
    count = 0;
    i = 0;
    tp = (int*)(base + 0x41E0);
    for (; i < 5; i++)
    {
        slot = 0;
        idPtr = (s16*)(*tp + 0x594);
        for (; slot < 3; slot++)
        {
            s16 id = *idPtr;
            if (id >= 0 && id < 80 && *(void**)(base + (0x83A8 + id * 4)) != 0)
            {
                s16 dup = 0;
                s16* w = list;
                int j2;
                for (j2 = 0; j2 < count; j2++)
                {
                    if (*w == *idPtr)
                    {
                        dup = 1;
                        break;
                    }
                    w++;
                }
                if (dup == 0)
                    list[count++] = id;
            }
            idPtr++;
        }
        tp++;
    }
    {
        int* objs = (int*)ObjList_GetObjects(&i, &objCount);
        while (i < objCount)
        {
            obj = (GameObject*)objs[i];
            fp = obj->anim.placement;
            i++;
            unload = 0;
            if (obj->anim.mapEventSlot > -1)
            {
                u8 fl = fp->loadFlags;
                if (!(fl & 2))
                {
                    if (fl & 0x10)
                    {
                        if (obj->anim.classId > -1 && objShouldUnload(obj))
                        {
                            unload = 1;
                        }
                        else if (obj->anim.mapEventSlot < 80 &&
                                 *(void**)(base + (0x83A8 + obj->anim.mapEventSlot * 4)) == 0)
                        {
                            unload = 1;
                        }
                    }
                    else
                    {
                        if (obj->anim.classId > -1 && objShouldUnload(obj))
                        {
                            unload = 1;
                        }
                        else if (obj->anim.mapEventSlot < 80 && obj->anim.mapEventSlot != gShaderCurMapEventId)
                        {
                            unload = 1;
                        }
                    }
                }
            }
            if (unload)
            {
                MapRomListPage* page = *(MapRomListPage**)(base + (0x83A8 + obj->anim.mapEventSlot * 4));
                if (page != 0)
                {
                    s16 tbit = obj->romListBit;
                    if (tbit >= 0 && tbit >= 0)
                    {
                        u8* bb = page->loadedObjectBits;
                        *(s8*)&bb[tbit >> 3] = bb[tbit >> 3] & ~(1 << (tbit & 7));
                    }
                }
                if (obj->anim.seqId == SHADER_SNOWBIKE_OBJ)
                {
                    s16 j3 = 0;
                    s16* w2 = list;
                    for (; j3 < count; j3++)
                    {
                        if (obj->anim.mapEventSlot == *w2)
                            break;
                        w2++;
                    }
                }
                Obj_FreeObject(obj);
                i--;
                objCount--;
            }
        }
    }
    if (getLoadedFileFlags(gShaderCurMapEventId) == 0)
    {
        for (i = 0; i < 80; i++)
        {
            if (((void**)(base + 0x83A8))[i] != NULL)
            {
                bits = (*gMapEventInterface)->getObjGroups(i);
                if (bits != 0)
                {
                    grpBit = 0;
                    while (bits != 0)
                    {
                        if ((bits & 1) && (s8)SaveGame_findTransientMapBit(i, grpBit) == -1)
                        {
                            mapInstantiateObjects((MapRomListPage*)((char**)(base + 0x83A8))[i], i, grpBit, NULL);
                            mapClearBit(i, grpBit);
                        }
                        bits >>= 1;
                        grpBit++;
                    }
                }
            }
        }
        for (i = 0; i < count; i++)
        {
            if (gShaderCurMapEventId == list[i])
            {
                MapRomListPage* page = *(MapRomListPage**)(base + (0x83A8 + list[i] * 4));
                if (page != 0)
                {
                    mask = 1;
                    bit = 0;
                    cur = (u32)page->objects;
                    bp = page->loadedObjectBits;
                    end = cur + *(int*)(base + (0x4290 + list[i] * 0x8C));
                    while (cur < end)
                    {
                        objStart = cur;
                        if ((*bp & mask) == 0 && objShouldLoad((ObjPlacement*)cur, 0, list[i]) != 0)
                        {
                            s16 lid = list[i];
                            if (bit >= 0)
                            {
                                MapRomListPage* pg = *(MapRomListPage**)(base + (0x83A8 + lid * 4));
                                int ix2 = bit >> 3;
                                int msk = 1 << (bit & 7);
                                *(s8*)&pg->loadedObjectBits[ix2] = pg->loadedObjectBits[ix2] & ~msk;
                                *(s8*)&pg->loadedObjectBits[ix2] = pg->loadedObjectBits[ix2] | msk;
                            }
                            Obj_SetupObject((ObjPlacement*)objStart, 1, list[i], bit, NULL);
                        }
                        bit++;
                        mask <<= 1;
                        if (mask == 0)
                        {
                            bp++;
                            while (*bp == -1)
                            {
                                bit += 8;
                                cur = objStart + *(u8*)(objStart + 2) * 4;
                                cur += *(u8*)(cur + 2) * 4;
                                cur += *(u8*)(cur + 2) * 4;
                                cur += *(u8*)(cur + 2) * 4;
                                cur += *(u8*)(cur + 2) * 4;
                                cur += *(u8*)(cur + 2) * 4;
                                cur += *(u8*)(cur + 2) * 4;
                                cur += *(u8*)(cur + 2) * 4;
                                objStart = cur;
                                bp++;
                            }
                            mask = 1;
                        }
                        cur = objStart + *(u8*)(objStart + 2) * 4;
                    }
                }
            }
        }
        {
            int* objs2 = (int*)ObjGroup_GetObjects(6, &objCount);
            for (i = 0; i < objCount; i++)
            {
                GameObject* obj2 = (GameObject*)objs2[i];
                u32 mid2 = obj2->anim.pad34;
                MapRomListPage* page2 = ((MapRomListPage**)(base + 0x83A8))[mid2];
                if (page2 != 0)
                {
                    int lp = obj2->anim.transformMatrixIndex + 1;
                    bit = 0;
                    cur = (u32)page2->objects;
                    end = cur + *(int*)(base + (0x4290 + mid2 * 0x8C));
                    bits = (*gMapEventInterface)->getObjGroups(mid2);
                    if (bits != 0)
                    {
                        grpBit = 0;
                        while (bits != 0)
                        {
                            if ((bits & 1) && (s8)SaveGame_findTransientMapBit(mid2, grpBit) == -1)
                            {
                                mapInstantiateObjects(page2, mid2, grpBit, obj2);
                            }
                            bits >>= 1;
                            mapClearBit(mid2, grpBit);
                            grpBit++;
                        }
                    }
                    while (cur < end)
                    {
                        if (bit < 0)
                        {
                            vis = 0;
                        }
                        else
                        {
                            char* pg2 = ((char**)(base + 0x83A8))[mid2];
                            idx = bit >> 3;
                            if (idx >= 0xc4)
                            {
                                vis = 0;
                            }
                            else
                            {
                                switch (((vis = 1) << (bit & 7)) & *(s8*)(*(int*)(pg2 + 0x10) + idx))
                                {
                                case 0:
                                    vis = 0;
                                    break;
                                }
                            }
                        }
                        if (vis == 0 && objShouldLoad((ObjPlacement*)cur, lp, mid2) != 0)
                        {
                            if (bit >= 0)
                            {
                                char* pg3 = ((char**)(base + 0x83A8))[mid2];
                                int ix3 = bit >> 3;
                                int msk3 = 1 << (bit & 7);
                                *(s8*)(*(int*)(pg3 + 0x10) + ix3) = *(u8*)(*(int*)(pg3 + 0x10) + ix3) & ~msk3;
                                *(s8*)(*(int*)(pg3 + 0x10) + ix3) = *(u8*)(*(int*)(pg3 + 0x10) + ix3) | msk3;
                            }
                            Obj_SetupObject((ObjPlacement*)cur, 1, mid2, bit, obj2);
                        }
                        bit++;
                        cur += *(u8*)(cur + 2) * 4;
                    }
                }
            }
        }
    }
}

void playerUpdateFn_8005649c(void)
{
    int count;
    int slot;
    GameObject** objs;
    CameraViewSlot* cam;
    int k;
    GameObject** e;
    int i;
    f32 lx, ly, lz;

    objs = (GameObject**)ObjGroup_GetObjects(6, &count);
    cam = Camera_GetCurrentViewSlot();
    Obj_UpdateWorldTransform(cam);
    for (k = 0; k < 31; k++)
        lbl_80386648[k].valid = 0;
    lbl_80386648[0].x = cam->worldX;
    lbl_80386648[0].y = cam->worldY;
    lbl_80386648[0].z = cam->worldZ;
    lbl_80386648[0].valid = 1;
    for (i = 0, e = objs; i < count; e++, i++)
    {
        GameObject* obj = *e;
        slot = obj->anim.transformMatrixIndex + 1;
        if (cam->parentObject == obj)
        {
            lbl_80386648[slot].x = cam->x;
            lbl_80386648[slot].y = cam->y;
            lbl_80386648[slot].z = cam->z;
        }
        else
        {
            Obj_TransformWorldPointToLocal(cam->worldX, cam->worldY, cam->worldZ, &lx, &ly, &lz, (u32)obj);
            lbl_80386648[slot].x = lx;
            lbl_80386648[slot].y = ly;
            lbl_80386648[slot].z = lz;
        }
        lbl_80386648[slot].valid = 1;
    }
}

MapTextureOverride* mapTextureOverrideGetEntry(int idx)
{
    return &lbl_803DCE6C[idx];
}

s16* return0_80056694(MapBlockData* wpad0, int wpad1)
{
    return NULL;
}
int return0_8005669C(int unused)
{
    return 0x0;
}

void mapTextureOverrideRelease(Texture* texture, int type)
{
    int i;
    Texture* entryTexture;

    for (i = 0; i < 80; i++)
    {
        entryTexture = lbl_803DCE6C[i].texture;
        if (entryTexture == texture && lbl_803DCE6C[i].type == type &&
            lbl_803DCE6C[i].refCount > 0)
        {
            lbl_803DCE6C[i].refCount -= 1;
            if (lbl_803DCE6C[i].refCount == 0)
            {
                lbl_803DCE6C[i].frame = 0;
                lbl_803DCE6C[i].type = 0;
                lbl_803DCE6C[i].texture = NULL;
                lbl_803DCE6C[i].flags = 0;
            }
        }
    }
}

extern char sTrackGlobalTexanimOverflowError[];

int mapTextureOverrideAcquire(Texture* texture, u32 flags, int type)
{
    MapTextureOverride* base;
    int idx;
    int found;
    int idx2;

    found = -1;
    idx = 0;
    base = lbl_803DCE6C;
    for (; idx < 80; idx++)
    {
        if (base[idx].refCount != 0)
        {
            Texture* entryTexture = base[idx].texture;
            if (entryTexture == texture && type == base[idx].type)
            {
                found = idx;
                break;
            }
        }
    }
    if (found != -1)
    {
        base[found].refCount += 1;
        return found;
    }
    found = -1;
    idx2 = 0;
    base = lbl_803DCE6C;
    for (; idx2 < 80; idx2++)
    {
        if (base[idx2].refCount == 0)
        {
            found = idx2;
            break;
        }
    }
    if (found != -1)
    {
        base[found].refCount = 1;
        lbl_803DCE6C[found].frame = 0;
        lbl_803DCE6C[found].flags = flags;
        lbl_803DCE6C[found].texture = texture;
        lbl_803DCE6C[found].type = type;
        return found;
    }
    OSReport(sTrackGlobalTexanimOverflowError);
    return 0;
}

extern f32 lbl_803DEBC8;

void mapTextureOverrideSetValue(int type, Texture* texture, int frame)
{
    int i;

    for (i = 0; i < 80; i++)
    {
        if (lbl_803DCE6C[i].refCount > 0 &&
            lbl_803DCE6C[i].texture == texture &&
            type == lbl_803DCE6C[i].type)
        {
            lbl_803DCE6C[i].frame = frame;
        }
    }
}

void mapTextureScrollGetOffset(int idx, float* outX, float* outY)
{
    f32 divisor;
    *outX = lbl_803DCE68[idx].offsetX / (divisor = lbl_803DEBC8);
    *outY = lbl_803DCE68[idx].offsetY / divisor;
}

void mapTextureScrollSetStep(int idx, int xStep, int yStep, int texWidthFixed, int texHeightFixed,
                             int secondaryXStep, int secondaryYStep, int texWidthFixed2, int texHeightFixed2)
{
    MapTextureScroll* e = &lbl_803DCE68[idx];
    e->xStep = (s16)((xStep << 16) / (texWidthFixed >> 6));
    e->yStep = (s16)((yStep << 16) / (texHeightFixed >> 6));
}

typedef struct
{
    u32 romList;
    s16 mapId;
    u16 flags;
} BlockEntry;

typedef struct MapRomListGrid
{
    s16 width;
    u8 unk02[0xa];
    u32* cells;
} MapRomListGrid;

extern BlockEntry gShaderRomListSlots[8];
extern s8 gShaderRomListSlotCount;

static inline int mapFindRomListSlot(char* slots, int id)
{
    int i2 = 0;
    char* q2 = slots;
    int cn = gShaderRomListSlotCount;
    int k;
    for (k = 0; k < cn; k++)
    {
        if (*(void**)q2 != NULL && id == *(s16*)(q2 + 4))
            return i2;
        q2 += 8;
        i2++;
    }
    return -1;
}

static inline int mapFindRomListSlotAndAdvance(char** slots, int id)
{
    int i2 = 0;
    int cn = gShaderRomListSlotCount;
    int k;
    for (k = 0; k < cn; k++)
    {
        if (*(void**)*slots != NULL && id == *(s16*)(*slots + 4))
            return i2;
        *slots += 8;
        i2++;
    }
    return -1;
}

static inline int mapFindRomListSlotByIdAt(char* base, int id)
{
    char* q2;
    int i2;
    int cn;
    int k;
    i2 = 0;
    q2 = base + 0x418C;
    cn = gShaderRomListSlotCount;
    for (k = 0; k < cn; k++)
    {
        if (*(void**)q2 != NULL && id == *(s16*)(q2 + 4))
            return i2;
        q2 += 8;
        i2++;
    }
    return -1;
}

static inline int mapFindRomListSlotById(int id)
{
    char* q2;
    int i2;
    int cn;
    int k;
    i2 = 0;
    q2 = (char*)gShaderRomListSlots;
    cn = gShaderRomListSlotCount;
    for (k = 0; k < cn; k++)
    {
        if (*(void**)q2 != NULL && id == *(s16*)(q2 + 4))
            return i2;
        q2 += 8;
        i2++;
    }
    return -1;
}

int mapTextureScrollAcquire(int xStep, int yStep, int texWidthFixed, int texHeightFixed,
                            int secondaryXStep, int secondaryYStep, int texWidthFixed2, int texHeightFixed2)
{
    MapTextureScroll* base;
    MapTextureScroll* entry;
    int idx;
    int idx2;
    int slot;
    f32 init;

    idx = 0;
    entry = base = lbl_803DCE68;
    for (; idx < 0x3a; idx++)
    {
        if (entry->xStep == xStep && entry->yStep == yStep)
        {
            entry->refCount += 1;
            return idx;
        }
        entry++;
    }
    slot = -1;
    for (idx2 = 0, entry = base; idx2 < 0x3a; entry++, idx2++)
    {
        if (entry->refCount == 0)
        {
            slot = idx2;
            break;
        }
    }
    if (slot == -1)
        return -1;
    entry = &base[slot];
    entry->xStep = (s16)((xStep << 16) / (texWidthFixed >> 6));
    entry->yStep = (s16)((yStep << 16) / (texHeightFixed >> 6));
    init = lbl_803DEBCC;
    entry->offsetX = init;
    entry->offsetY = init;
    entry->refCount += 1;
    return slot;
}

void trackLoadBlockEnd(MapBlockData* block, int blockId, int slotIdx, int layer)
{
    int i;
    s16* arr;
    int count;
    s8* statusArr;

    i = 0;
    arr = gMapBlockIds;
    count = lbl_803DCE98;
    for (; i < count; i++)
    {
        if (*arr == -1)
            break;
        arr++;
    }
    if (i == count)
    {
        lbl_803DCE98++;
        if (lbl_803DCE98 == 0x40)
        {
            OSReport(sTrackLoadBlockOverrunError);
        }
    }
    statusArr = gMapBlockLayerTables[layer];
    statusArr[slotIdx] = i;
    gMapBlocks[i] = block;
    gMapBlockIds[i] = blockId;
    gMapBlockRefCounts[i] = 1;
    setMapBlockFlag();
}




char lbl_803822C8[0x41A0];

void mapBlockFn_80059354(int p1, int p2, MapCellEntry* entry, int layer);


int mapLoadBlock(int cellX, int cellZ, int worldX, int worldZ, int layer)
{
    int j;
    s16* arr;
    void* block[1];
    int textureCursor[2];
    int slotIdx;
    int blockId;
    s8* statusArr;
    MapCellEntry* entry;

    entry = (MapCellEntry*)lbl_803822A0[layer];
    statusArr = gMapBlockLayerTables[layer];
    slotIdx = cellX + (cellZ << 4);
    entry += slotIdx;

    mapBlockFn_80059354(worldX, worldZ, entry, layer);

    blockId = entry->blockId;
    if (mapCheckCurBlocks(entry->romListIndex) == -1)
    {
        statusArr[slotIdx] = -1;
        return 0;
    }
    if (blockId < 0)
    {
        blockId = -1;
    }
    if (blockId < 0)
    {
        statusArr[slotIdx] = blockId;
        return 0;
    }
    statusArr[slotIdx] = -1;

    j = 0;
    arr = gMapBlockIds;
    for (; j < lbl_803DCE98; j++)
    {
        if (blockId == *arr)
        {
            gMapBlockRefCounts[j]++;
            statusArr[slotIdx] = j;
            return 1;
        }
        arr++;
    }

    block[0] = MapBlock_loadFromFile(blockId);
    if (block[0] != NULL)
    {
        MapBlock_init(block[0]);
        textureCursor[0] = 0;
        textureCursor[1] = textureCursor[0];
        while (textureCursor[0] < ((MapBlockData*)block[0])->textureCount)
        {
            int fileId =
                -(int)((u32)((MapTextureRef*)((u8*)((MapBlockData*)block[0])->textures +
                                               textureCursor[1]))->fileId |
                       0x8000);
            ((MapTextureRef*)((u8*)((MapBlockData*)block[0])->textures + textureCursor[1]))->texture =
                textureLoad(fileId, 0);
            textureCursor[1] += sizeof(MapTextureRef);
            textureCursor[0]++;
        }
        MapBlock_initHits(block[0], blockId);
        MapBlock_initShaders(block[0]);
        trackLoadBlockEnd(block[0], blockId, slotIdx, layer);
        ((MapBlockData*)block[0])->unk0 = (void*)return0_80060B90(block[0]);
        DCStoreRange(block[0], ((MapBlockData*)block[0])->size);
    }
    return 1;
}

void unloadMap(void)
{
    MapBlockData* block;
    int shaderIndex;
    MapShaderLayer* shaderLayer;
    int i;
    int layer;
    s8* cur;
    s8 mapType;
    MapShader* shader;
    int layerIndex;
    int textureIndex;
    u32 scrollSlot;

    audioStopByMask(4);
    Sfx_ClearLoopedObjectSounds();
    doNothing_8001F678(1, 0);
    for (layer = 0; layer < MAP_BLOCK_LAYER_COUNT; layer++)
    {
        cur = gMapBlockLayerTables[layer];
        for (i = 0; i < 256; i++)
        {
            mapType = cur[i];
            if (mapType >= 0)
            {
                gMapBlockRefCounts[mapType]--;
                if (gMapBlockRefCounts[mapType] == 0)
                {
                    block = gMapBlocks[mapType];
                    gMapBlockIds[mapType] = -1;
                    gMapBlocks[mapType] = NULL;
                    for (shaderIndex = 0; shaderIndex < block->shaderCount; shaderIndex++)
                    {
                        shader = &block->shaders[shaderIndex];
                        for (layerIndex = 0; layerIndex < shader->layerCount; layerIndex++)
                        {
                            shaderLayer = &shader->layers[layerIndex];
                            scrollSlot = shaderLayer->scrollMtx;
                            if (scrollSlot != 0xff)
                            {
                                if (lbl_803DCE68[scrollSlot].refCount != 0)
                                    lbl_803DCE68[scrollSlot].refCount -= 1;
                            }
                            if (shaderLayer->overrideType != 0)
                                mapTextureOverrideRelease(shaderLayer->texture, shaderLayer->overrideType);
                        }
                    }
                    for (textureIndex = 0; textureIndex < block->textureCount; textureIndex++)
                        textureFree(block->textures[textureIndex].texture);
                    if (block->auxData != NULL)
                        mm_free(block->auxData);
                    if (block->hits != NULL)
                        mm_free(block->hits);
                    setMapBlockFlag();
                    mm_free(block);
                }
            }
        }
    }
    lbl_803DCE98 = 0;
    Obj_ResetObjectSystem();
    for (i = 0; i < ROM_LIST_PAGE_COUNT; i++)
    {
        if (gLoadedRomListPages[i] != NULL)
        {
            mm_free(gLoadedRomListPages[i]);
            gLoadedRomListPages[i] = NULL;
        }
    }
    (*gCheckpointInterface)->reset();
    (*gRomCurveInterface)->initialise();
    gShaderRomListSlotCount = 0;
    playerMapOffsetX = lbl_803DEBCC;
    playerMapOffsetZ = lbl_803DEBCC;
    voxmaps_resetLoadedMaps();
    textureFreeFn_8012fcec();
    fn_80133934();
    (*gNewCloudsInterface)->killSnowCloud(-1, 0);
    (*gCloudActionInterface)->freeCloudObjects();
}

extern s8 curMapLayer;
extern s8 curMapType;
extern s16 lbl_803DCEB4;
extern s16 lbl_803DCEB6;
extern u32 renderFlags;
s32 getCurMapLayer(void)
{
    return curMapLayer;
}

extern int gShaderGameTextLoadedMapId;
extern s8 gShaderMapTextDirTable[];

void gameTextLoadForMap_800571f0(u8 force)
{
    int curVal = gShaderCurMapEventId;
    if (curVal == -1)
        return;
    if (curVal == gShaderGameTextLoadedMapId && force == 0)
        return;
    gShaderGameTextLoadedMapId = curVal;
    if (curVal >= 0x76)
        return;
    {
        s8 entry = gShaderMapTextDirTable[curVal];
        if (entry == -1)
            return;
        gameTextLoadDir(entry);
    }
}

extern u8* lbl_803DCE78;

void mapSetup(int mapType, f32 a, int* outMapId, int* outEvent, f32 b, f32 c)
{
    MapInfoRecord* tabEntry;
    int mapY;
    int mapId;
    int layer;
    int mapCount;
    s8* arr;

    layer = 0;
    arr = (s8*)(int)lbl_803DB624;
    if (arr[0] != mapType)
    {
        layer = 1;
        if (arr[1] != mapType)
        {
            layer = 2;
            if (arr[2] != mapType)
            {
                layer = 3;
                if (arr[3] != mapType)
                {
                    layer = 4;
                    if (arr[4] != mapType)
                    {
                        layer = 5;
                    }
                }
            }
        }
    }
    curMapLayer = 0;
    mapY = fastFloorf(c / gMapBlockWorldSize);
    mapId = mapCoordsToId((s32)fastFloorf(a / gMapBlockWorldSize), mapY, layer);
    mapCount = (s32)((u32)getDataFileSize(MLDF_FILEID_MAPINFO_BIN) >> 5);
    if (mapId < 0 || mapId >= mapCount)
    {
        curMapType = 0;
    }
    else
    {
        getTabEntry(tabEntry = (MapInfoRecord*)lbl_803DCE78, MLDF_FILEID_MAPINFO_BIN, mapId << 5, 0x20);
        curMapType = tabEntry->mapType;
    }
    lbl_803DCEB4 = 0;
    if (curMapType == MAPTYPE_SUBMAP)
    {
        lbl_803DCEB6 = mapId;
        lbl_803DCEB4 = tabEntry->unk1e;
    }
    *outMapId = mapId;
    if (mapId != -1)
    {
        *outEvent = (s32) * (s8*)((*gMapEventInterface)->getCurCharPos() + 0xe);
    }
}

void mapReloadWithFadeout(void)
{
    curMapType = 0;
    lbl_803DCEB6 = 0;
    lbl_803DCEB4 = 0;
}
s32 getCurMapType(void)
{
    return curMapType;
}

typedef struct
{
    Vec v[5];
} PlayerFrustumPlaneDirections;

typedef struct
{
    f32 v[5];
} PlayerFrustumPlaneScales;

STATIC_ASSERT(sizeof(PlayerFrustumPlaneDirections) == 0x3C);
STATIC_ASSERT(sizeof(PlayerFrustumPlaneScales) == 0x14);

const PlayerFrustumPlaneDirections sPlayerFrustumPlaneDirs = {
    {{0.0f, 0.0f, 1.0f},
     {1.0f, 0.0f, 0.0f},
     {-1.0f, 0.0f, 0.0f},
     {0.0f, 1.0f, 0.0f},
     {0.0f, -1.0f, 0.0f}}};
const PlayerFrustumPlaneScales sPlayerFrustumPlaneScales = {
    {0.0f, -25.0f, -25.0f, -25.0f, -25.0f}};

extern s16 lbl_803DCEB8;
extern u8 lbl_803DCDE0;
extern int gMapBlockOriginX;
extern int gMapBlockOriginZ;
extern f32 gMapSavedPlayerOffsetX;
extern f32 gMapSavedPlayerOffsetZ;
extern int gMapCurRomListSlot;
extern u8 gMapLoadDeferred;
extern u8 bEnableBlurFilter;
extern u8 bEnableMotionBlur;
extern int gHeatEffectFadeDirection;
extern u8 lbl_803DCEBD;
extern f32 gShaderDefaultTimeOfDay;
void beginLoadingMap(void)
{
    char* base;
    int i;
    int j;
    s8* a;
    s8* b;
    int k2, k3;
    int mapKind;
    f32* p;
    f32 px, py, pz;
    CameraViewSlot* cam;
    char* player;
    u8* env;
    int bo;
    char buf[0x110];

    base = (char*)lbl_8037E0C0;
    if (lbl_803DCEB8 == -1)
    {
        lbl_803DCEB8 = -2;
        lbl_803DCDE0 = 8;
    }
    (*gObjectTriggerInterface)->onMapSetup();
    mapInitFn_80069990();
    for (i = 0; i < 5; i++)
    {
        a = ((s8**)(base + 0x41F4))[i];
        b = ((s8**)(base + 0x41E0))[i];
        for (j = 0; j < 256; j++)
        {
            a[j] = -1;
            b[j * 12 + 9] = -1;
        }
    }
    for (j = 0; j < 64; j++)
    {
        *(s16*)((char*)gMapBlockIds + j * 2) = -1;
        gMapBlocks[j] = NULL;
    }
    lbl_803DCE98 = 0;
    gShaderRomListSlotCount = 0;
    mapKind = (*gMapEventInterface)->getCurChar();
    p = (f32*)(*gMapEventInterface)->getCurCharPos();
    gMapBlockOriginX = fastFloorf(p[0] / gMapBlockWorldSize);
    gMapBlockOriginZ = fastFloorf(p[2] / gMapBlockWorldSize);
    *(f32*)(base + 0x8588) = p[0];
    *(f32*)(base + 0x858C) = p[1];
    *(f32*)(base + 0x8590) = p[2];
    *(int*)(base + 0x8594) = 1;
    gMapBlockOriginWorldX = gMapBlockOriginX * 640;
    gMapBlockOriginWorldZ = gMapBlockOriginZ * 640;
    playerMapOffsetX = gMapBlockOriginWorldX;
    playerMapOffsetZ = gMapBlockOriginWorldZ;
    gMapSavedPlayerOffsetX = playerMapOffsetX;
    gMapSavedPlayerOffsetZ = playerMapOffsetZ;
    gShaderCurMapEventId = -1;
    gShaderGameTextLoadedMapId = gShaderGameTextLoadedMapId - 1;
    gMapCurRomListSlot = -1;
    curMapLayer = *(s8*)((char*)p + 0xd);
    renderFlags &= 0x82008;
    renderFlags |= 0x481F0LL;
    renderFlags |= 0x804;
    gMapLoadDeferred = 0;
    bEnableBlurFilter = 0;
    bEnableMotionBlur = 0;
    lbl_803DB62C = lbl_803DEBCC;
    gHeatEffectFadeDirection = -1;
    setSaveGameLoadingFlag();
    pz = p[2];
    py = p[1];
    px = p[0];
    if (!(renderFlags & 2) || (renderFlags & 0x800))
    {
        gShaderLoadCenterX = px;
        gShaderLoadCenterY = py;
        gShaderLoadCenterZ = pz;
        renderFlags |= 2;
        if (renderFlags & 0x800)
            doPendingMapLoads();
    }
    renderFlags &= ~4LL;
    trackIntersect();
    cam = Camera_GetCurrentViewSlot();
    cam->x = p[0];
    cam->y = p[1];
    cam->z = p[2];
    mapSetupPlayer();
    lbl_803DCEBD = 0;
    (*gWaterfxInterface)->onMapSetup();
    (*gProjgfxInterface)->onMapSetup();
    (*gModgfxInterface)->onMapSetup();
    (*gExpgfxInterface)->onMapSetup();
    (*gPartfxInterface)->onMapSetup();
    (*gCloudActionInterface)->freeCloudObjects();
    (*gCloudActionInterface)->onMapSetup();
    (*gSky2Interface)->onMapSetup();
    (*gSkyInterface)->loadLights();
    (*gNewCloudsInterface)->onMapSetup();
    waterFxInit();
    player = (char*)Obj_GetPlayerObject();
    if (lbl_803DCEB8 == -2 && player != 0 && (mapKind == 0 || mapKind == 1))
    {
        s16 cam2 = SaveGame_getCamActionNo();
        if (cam2 != -1)
        {
            (*gCameraInterface)->loadTriggeredCamAction(0, cam2, 1);
        }
        env = saveGameGetEnvState();
        {
            s16 v = *(s16*)(env + 4);
            if (v != -1)
                getEnvfxActImmediately(player, player, v & 0xFFFF, 0);
            v = *(s16*)(env + 6);
            if (v != -1)
                getEnvfxActImmediately(player, player, v & 0xFFFF, 0);
            v = *(s16*)(env + 0xa);
            if (v != -1)
                getEnvfxActImmediately(player, player, v & 0xFFFF, 0);
            v = *(s16*)(env + 0xc);
            if (v != -1)
                getEnvfxActImmediately(player, player, v & 0xFFFF, 0);
        }
        skyFn_80088c94(1, (*(u8*)(env + 0x40) & 2) ? 1 : 0);
        skyFn_80088c94(2, (*(u8*)(env + 0x40) & 4) ? 1 : 0);
        skyFn_80088e54((*(u8*)(env + 0x40) & 0x10) ? 1 : 0, lbl_803DEBCC);
        if (*(u8*)(env + 0x40) & 1)
            bo = 1;
        else
            bo = 0;
        {
            u8* e2 = saveGameGetEnvState();
            if (bo)
            {
                renderFlags |= 0x50;
                *(u8*)(e2 + 0x40) = *(u8*)(e2 + 0x40) | 9;
            }
            else
            {
                renderFlags &= ~0x50;
                *(u8*)(e2 + 0x40) = *(u8*)(e2 + 0x40) & ~9;
            }
        }
        if (*(u8*)(env + 0x40) & 8)
            bo = 1;
        else
            bo = 0;
        {
            u8* e3 = saveGameGetEnvState();
            if (bo)
            {
                renderFlags |= 0x40;
                *(u8*)(e3 + 0x40) = *(u8*)(e3 + 0x40) | 8;
            }
            else
            {
                renderFlags &= ~0x40LL;
                *(u8*)(e3 + 0x40) = *(u8*)(e3 + 0x40) & ~8;
            }
        }
        if (*(u8*)(env + 0x40) & 0x20)
            gHeatEffectFadeDirection = 1;
        else
            gHeatEffectFadeDirection = -1;
        *(int*)(buf + 0x30) = 0;
        *(f32*)(buf + 0xc) = lbl_803DEBCC;
        *(f32*)(buf + 0x10) = lbl_803DEBCC;
        *(f32*)(buf + 0x14) = lbl_803DEBCC;
        *(f32*)(buf + 0x18) = lbl_803DEBCC;
        *(f32*)(buf + 0x1c) = lbl_803DEBCC;
        *(f32*)(buf + 0x20) = lbl_803DEBCC;
        {
            s16 a1 = *(s16*)(env + 0xe);
            if (a1 != -1)
            {
                *(f32*)(buf + 0xc) = (f32) * (int*)(env + 0x14);
                *(f32*)(buf + 0x10) = (f32) * (int*)(env + 0x18);
                *(f32*)(buf + 0x14) = (f32) * (int*)(env + 0x1c);
                getEnvfxAct(buf, player, a1 & 0xFFFF, 0);
            }
            a1 = *(s16*)(env + 0x10);
            if (a1 != -1)
            {
                *(f32*)(buf + 0xc) = (f32) * (int*)(env + 0x20);
                *(f32*)(buf + 0x10) = (f32) * (int*)(env + 0x24);
                *(f32*)(buf + 0x14) = (f32) * (int*)(env + 0x28);
                getEnvfxAct(buf, player, a1 & 0xFFFF, 0);
            }
            a1 = *(s16*)(env + 0x12);
            if (a1 != -1)
            {
                *(f32*)(buf + 0xc) = (f32) * (int*)(env + 0x2c);
                *(f32*)(buf + 0x10) = (f32) * (int*)(env + 0x30);
                *(f32*)(buf + 0x14) = (f32) * (int*)(env + 0x34);
                getEnvfxAct(buf, player, a1 & 0xFFFF, 0);
            }
        }
        (*gSkyInterface)->setTimeOfDay(*(f32*)env);
    }
    else
    {
        (*gSkyInterface)->setTimeOfDay(gShaderDefaultTimeOfDay);
        (*gCloudActionInterface)->func09Nop(1);
    }
    clearSaveGameLoadingFlag();
    Pause_SetDisabled(0);
    Pause_ResetMenuFrameCounter();
}

void mapFn_80057d24(int a, int b, int* o0, int* o1, int* o2, int* o3, int f1, int f2, int idx)
{
    int base;
    s16* e2;
    int aa, bb;
    int ptr0;
    int tbl, tbl2;
    int index;
    int idx2;
    u32 v, v2;
    int cellVal;

    if (idx == -1)
    {
        o0[0] = -1;
        o0[1] = 1;
        o0[2] = -1;
        o0[3] = 1;
        o1[0] = 0;
        o1[1] = 0;
        o1[2] = 0;
        o1[3] = -1;
        o2[0] = 0;
        o2[1] = 0;
        o2[2] = 0;
        o2[3] = -1;
        o3[0] = 0;
        o3[1] = 0;
        o3[2] = 0;
        o3[3] = -1;
        if (f1 != 0)
            o0[3] = -2;
        return;
    }
    base = gShaderMapRomBuffers[1];
    e2 = (s16*)(base + gShaderRomListSlots[idx].mapId * 10);
    aa = a - e2[0];
    bb = b - e2[2];
    ptr0 = gShaderRomListSlots[idx].romList;
    if (idx == -1)
    {
        o0[0] = -1;
        o0[1] = 1;
        o0[2] = -1;
        o0[3] = 1;
        o1[0] = 0;
        o1[1] = 0;
        o1[2] = 0;
        o1[3] = -1;
        o2[0] = 0;
        o2[1] = 0;
        o2[2] = 0;
        o2[3] = -1;
        o3[0] = 0;
        o3[1] = 0;
        o3[2] = 0;
        o3[3] = -1;
        if (f1 != 0)
            o0[3] = -2;
        return;
    }
    if (f2 != 0)
    {
        tbl = *(int*)(ptr0 + 0x30);
        tbl2 = *(int*)(ptr0 + 0x34);
    }
    else
    {
        tbl = *(int*)(ptr0 + 0x14);
        tbl2 = *(int*)(ptr0 + 0x2c);
    }
    index = aa + bb * *(s16*)ptr0;
    idx2 = index * 2;
    if (f1 == 0)
    {
        v = *(int*)(tbl + idx2 * 4);
        o0[0] = ((v >> 12) & 0xf) - 7;
        o0[2] = ((v >> 8) & 0xf) - 7;
        o0[1] = ((v >> 4) & 0xf) - 7;
        o0[3] = (v & 0xf) - 7;
        o1[0] = (v >> 28) - 7;
        o1[2] = ((v >> 24) & 0xf) - 7;
        o1[1] = ((v >> 20) & 0xf) - 7;
        o1[3] = ((v >> 16) & 0xf) - 7;
        v2 = *(int*)((tbl + 4) + idx2 * 4);
        o2[0] = ((v2 >> 12) & 0xf) - 7;
        o2[2] = ((v2 >> 8) & 0xf) - 7;
        o2[1] = ((v2 >> 4) & 0xf) - 7;
        o2[3] = (v2 & 0xf) - 7;
        o3[0] = (v2 >> 28) - 7;
        o3[2] = ((v2 >> 24) & 0xf) - 7;
        o3[1] = ((v2 >> 20) & 0xf) - 7;
        o3[3] = ((v2 >> 16) & 0xf) - 7;
    }
    else
    {
        o0[0] = 0;
        o0[1] = -1;
        o0[2] = 0;
        o0[3] = -1;
        o1[0] = 0;
        o1[1] = -1;
        o1[2] = 0;
        o1[3] = -1;
        o2[0] = 0;
        o2[1] = -1;
        o2[2] = 0;
        o2[3] = -1;
        o3[0] = 0;
        o3[1] = -1;
        o3[2] = 0;
        o3[3] = -1;
        cellVal = *(int*)(*(int*)(ptr0 + 0xc) + (idx2 >> 1) * 4) & 0x7f;
        if (cellVal != 127)
        {
            v2 = ((int*)tbl2)[f1 - 1 + cellVal * 4];
            o0[0] = ((v2 >> 12) & 0xf) - 7;
            o0[2] = ((v2 >> 8) & 0xf) - 7;
            o0[1] = ((v2 >> 4) & 0xf) - 7;
            o0[3] = (v2 & 0xf) - 7;
            o1[0] = (v2 >> 28) - 7;
            o1[2] = ((v2 >> 24) & 0xf) - 7;
            o1[1] = ((v2 >> 20) & 0xf) - 7;
            o1[3] = ((v2 >> 16) & 0xf) - 7;
        }
    }
}

/* 16-byte texture-override table entry (array at lbl_803DCE6C, 80 slots). */

void goToPrevMapLayer(void)
{
    curMapLayer--;
    if (curMapLayer < -2)
    {
        curMapLayer = -2;
    }
    renderFlags |= 0x4000;
}

void goToNextMapLayer(void)
{
    curMapLayer++;
    if (curMapLayer > 2)
    {
        curMapLayer = 2;
    }
    renderFlags |= 0x4000;
}
static inline void mapMarkRectRows(char* g3, int* rect)
{
    int xx, zz;
    for (zz = rect[2]; zz <= rect[3]; zz++)
    {
        char* gp;
        xx = rect[0];
        gp = g3 + (zz + 7) * 16 + xx;
        for (; xx <= rect[1]; xx++)
        {
            gp[7] = -3;
            gp++;
        }
    }
}

extern char sTrackPiLockedFormat[];

void doPendingMapLoads(void)
{
    MapLoadRec* cellCursor;
    int gx, gz;
    s8** cBase;
    char* base;
    MapLoadRec* savedBlocks;
    int doLoad;
    MapCellEntry** eBase;
    u8 waited;
    int col;
    int slot;
    MapLoadRec* rowCursor;
    int layer;
    int cellIdx;
    int colIdx;
    int i;
    MapLoadRec* recsCursor;
    int cnt;
    f32 dz;
    char** aBase;
    char* cellGrid;
    int row;
    int unusedColumn;
    int gridPass;
    MapLoadRec recs[300];
    int rectA[4], rectB[4], rectC[4], rectD[4];

    base = (char*)lbl_8037E0C0;
    waited = 0;
    if (!(renderFlags & 0x1000))
    {
        gMapSavedPlayerOffsetX = playerMapOffsetX;
        gMapSavedPlayerOffsetZ = playerMapOffsetZ;
        if (gShaderCurMapEventId != -1 && gShaderCurMapEventId != gShaderGameTextLoadedMapId &&
            (gShaderGameTextLoadedMapId = gShaderCurMapEventId, gShaderCurMapEventId < 118) &&
            gShaderMapTextDirTable[gShaderCurMapEventId] != -1)
        {
            gameTextLoadDir(gShaderMapTextDirTable[gShaderCurMapEventId]);
        }
        if (!(renderFlags & 2) && (getLoadedFileFlags(0) != 0 || gMapPendingFileFlags == 0))
        {
            gMapPendingFileFlags = getLoadedFileFlags(0);
        }
        else
        {
            renderFlags &= ~2LL;
            dz = gShaderLoadCenterZ - playerMapOffsetZ;
            gx = fastFloorf((gShaderLoadCenterX - playerMapOffsetX) / gMapBlockWorldSize);
            gz = fastFloorf(dz / gMapBlockWorldSize);
            {
                u32 t = renderFlags;
                doLoad = t & 0x800;
                renderFlags = t & ~0x800LL;
            }
            {
                int ff = getLoadedFileFlags(0);
                if ((ff & ~0x100000) != 0)
                {
                    if (gShaderCurMapEventId != 38 && gShaderCurMapEventId != 58 && gShaderCurMapEventId != 59 &&
                        gShaderCurMapEventId != 60 && gShaderCurMapEventId != 61 && gShaderCurMapEventId != 62 &&
                        gShaderCurMapEventId != 28)
                    {
                        gMapLoadDeferred = 1;
                    }
                }
                else
                {
                    if (gMapLoadDeferred != 0)
                    {
                        gMapLoadDeferred = 0;
                        doLoad = 1;
                    }
                }
            }
            if (gx != 7 || gz != 7 || doLoad != 0 || (renderFlags & 0x4000))
            {
                shadowVolumesSetDirty(1);
                doNothing_8001F678(1, 0);
                cnt = 0;
                layer = 0;
                {
                    MapCellEntry** cellTables;
                    char** gridTables;
                    s8** stateTables;
                    int k8;
                    s8 c;
                    eBase = (MapCellEntry**)(base + 0x41E0);
                    cellTables = eBase;
                    aBase = (char**)(base + 0x41F4);
                    gridTables = aBase;
                    cBase = (s8**)(base + 0x41CC);
                    stateTables = cBase;
                    savedBlocks = recs;
                    recsCursor = savedBlocks;
                    for (; layer < 5; layer++)
                    {
                        MapCellEntry* ent = *cellTables;
                        char* grid = *gridTables;
                        gMapLayerCellStates = *stateTables;
                        cellIdx = 0;
                        row = 0;
                        rowCursor = recsCursor;
                        cellGrid = grid;
                        for (; row < 16; row++)
                        {
                            colIdx = 0;
                            cellCursor = rowCursor;
                            for (k8 = 0; k8 < 8; k8++)
                            {
                                c = cellGrid[0];
                                if (c > -1)
                                {
                                    cellCursor->x = gMapBlockOriginX + colIdx;
                                    cellCursor->z = gMapBlockOriginZ + row;
                                    cellCursor->layer = layer;
                                    cellCursor->blockId = c;
                                    cellCursor++;
                                    rowCursor++;
                                    recsCursor++;
                                    cnt++;
                                }
                                cellGrid[0] = -2;
                                gMapLayerCellStates[cellIdx] = -1;
                                ent[0].blockId = -3;
                                ent[0].mapId = -1;
                                ent[0].adjacentMapId1 = -1;
                                ent[0].adjacentMapId2 = -1;
                                cellIdx = cellIdx + 1;
                                colIdx = colIdx + 1;
                                c = cellGrid[1];
                                if (c > -1)
                                {
                                    cellCursor->x = gMapBlockOriginX + colIdx;
                                    cellCursor->z = gMapBlockOriginZ + row;
                                    cellCursor->layer = layer;
                                    cellCursor->blockId = c;
                                    cellCursor++;
                                    rowCursor++;
                                    recsCursor++;
                                    cnt++;
                                }
                                cellGrid[1] = -2;
                                gMapLayerCellStates[cellIdx] = -1;
                                ent[1].blockId = -3;
                                ent[1].mapId = -1;
                                ent[1].adjacentMapId1 = -1;
                                ent[1].adjacentMapId2 = -1;
                                ent += 2;
                                cellIdx = cellIdx + 1;
                                cellGrid += 2;
                                colIdx = colIdx + 1;
                            }
                        }
                        cellTables++;
                        gridTables++;
                        stateTables++;
                    }
                }
                {
                    int nx = gx + gMapBlockOriginX;
                    int nz;
                    nx -= 7;
                    gMapBlockOriginX = nx;
                    nz = gz + gMapBlockOriginZ;
                    nz -= 7;
                    gMapBlockOriginZ = nz;
                }
                playerMapOffsetX = gMapBlockWorldSize * gMapBlockOriginX;
                playerMapOffsetZ = gMapBlockWorldSize * gMapBlockOriginZ;
                gMapBlockOriginWorldX = playerMapOffsetX;
                gMapBlockOriginWorldZ = playerMapOffsetZ;
                i = 0;
                {
                    int cn = gShaderRomListSlotCount;
                    for (; i < cn; i++)
                    {
                        ((ShaderRomListSlot*)(base + 0x418C))[i].flag = 0;
                    }
                }
                gShaderCurMapEventId = mapCoordsToId(gMapBlockOriginX + 7, gMapBlockOriginZ + 7, 0);
                gMapCurRomListSlot = -1;
                if (gShaderCurMapEventId == -1)
                {
                    int d = mapGetDirIdx(41);
                    setForceLoadImmediately();
                    mapLoadDataFile(d, MLDF_FILEID_TEX1_BIN_A);
                    mapLoadDataFile(d, MLDF_FILEID_TEX0_BIN_A);
                    mapLoadDataFile(d, MLDF_FILEID_ANIM_BIN_A);
                    mapLoadDataFile(d, MLDF_FILEID_MODELS_BIN_A);
                    mapLoadDataFile(d, MLDF_FILEID_TEX1_TAB_A);
                    mapLoadDataFile(d, MLDF_FILEID_MODELS_TAB_A);
                    mapLoadDataFile(d, MLDF_FILEID_ANIM_TAB_A);
                    mapLoadDataFile(d, MLDF_FILEID_TEX0_TAB_A);
                    clearForceLoadImmediately();
                    while (getLoadedFileFlags(0) != 0)
                    {
                        OSReport(sTrackPiLockedFormat, getLoadedFileFlags(0));
                        padUpdate();
                        checkReset();
                        if (waited)
                            waitNextFrame();
                        loadDataFiles();
                        dvdCheckError();
                        if (waited)
                        {
                            mmFreeTick(0);
                            gameTextRun();
                            GXFlush_(1, 0);
                        }
                        if (gDvdErrorPauseActive)
                            waited = 1;
                    }
                }
                else
                {
                    if (gShaderCurMapEventId != -1)
                    {
                        setForceLoadImmediately();
                        slot = mapFindRomListSlotByIdAt(base, gShaderCurMapEventId);
                        if (slot == -1)
                            slot = mapProcessRomList(gShaderCurMapEventId);
                        {
                            int mapId = gShaderCurMapEventId;
                            int sz = (int)((u32)getDataFileSize(MLDF_FILEID_MAPINFO_BIN) >> 5);
                            if (mapId < 0 || mapId >= sz)
                            {
                                curMapType = 0;
                            }
                            else
                            {
                                MapInfoRecord* e = (MapInfoRecord*)lbl_803DCE78;
                                getTabEntry(e, MLDF_FILEID_MAPINFO_BIN, mapId << 5, 0x20);
                                curMapType = e->mapType;
                            }
                        }
                        ((ShaderRomListSlot*)(base + 0x418C))[slot].flag = 1;
                        gMapCurRomListSlot = slot;
                        mapCheckCurBlocks(mapGetDirIdx(gShaderCurMapEventId));
                        mapLoadDataFile(mapGetDirIdx(gShaderCurMapEventId), MLDF_FILEID_BLOCKS_TAB_A);
                        mapLoadDataFile(mapGetDirIdx(gShaderCurMapEventId), MLDF_FILEID_BLOCKS_BIN_A);
                        mapLoadDataFile(mapGetDirIdx(gShaderCurMapEventId), MLDF_FILEID_VOXMAP_TAB_A);
                        mapLoadDataFile(mapGetDirIdx(gShaderCurMapEventId), MLDF_FILEID_VOXMAP_BIN_A);
                        gMapBlockIndexList = getCurrentDataFile(MLDF_FILEID_BLOCKS_TAB_A);
                        gMapBlockIndexCount = 0;
                        {
                            int* blockIndex;
                            for (blockIndex = gMapBlockIndexList; gMapBlockIndexList != 0 && *blockIndex != -1;)
                            {
                                blockIndex++;
                                gMapBlockIndexCount = gMapBlockIndexCount + 1;
                            }
                        }
                        gMapBlockIndexCount = gMapBlockIndexCount - 1;
                        /* Vestigial grid walk over each layer's cell table: writes only dead
                           locals, but retail emits it. MWCC collapses the two innermost levels
                           into the closed-form pointer and row bumps of the mtctr-2 loop. */
                        for (i = 0; i < 5; i++)
                        {
                            cellGrid = (char*)*eBase;
                            row = 0;
                            for (gridPass = 0; gridPass < 2; gridPass++)
                            {
                                for (col = 0; col < 7; col++)
                                {
                                    for (unusedColumn = 0; unusedColumn < 16; unusedColumn++)
                                    {
                                        cellGrid += 12;
                                    }
                                    row++;
                                }
                            }
                            eBase++;
                        }
                        {
                            int mapDir = mapGetDirIdx(gShaderCurMapEventId);
                            mapLoadDataFile(mapDir, MLDF_FILEID_TEX1_BIN_A);
                            mapLoadDataFile(mapDir, MLDF_FILEID_TEX0_BIN_A);
                            mapLoadDataFile(mapDir, MLDF_FILEID_ANIM_BIN_A);
                            mapLoadDataFile(mapDir, MLDF_FILEID_MODELS_BIN_A);
                            mapLoadDataFile(mapDir, MLDF_FILEID_ANIMCURV_BIN_A);
                            mapLoadDataFile(mapDir, MLDF_FILEID_TEX1_TAB_A);
                            mapLoadDataFile(mapDir, MLDF_FILEID_MODELS_TAB_A);
                            mapLoadDataFile(mapDir, MLDF_FILEID_ANIM_TAB_A);
                            mapLoadDataFile(mapDir, MLDF_FILEID_TEX0_TAB_A);
                            mapLoadDataFile(mapDir, MLDF_FILEID_ANIMCURV_TAB_A);
                        }
                        loadModelAndAnimTabs();
                        {
                            for (layer = 0; layer < 5; layer++)
                            {
                                char* g3;
                                s8 cnt2;
                                mapFn_80057d24(gMapBlockOriginX + 7, gMapBlockOriginZ + 7, rectA, rectB, rectC, rectD,
                                               layer, 0, slot);
                                g3 = *aBase;
                                gMapLayerCellStates = *cBase;
                                mapMarkRectRows(g3, rectA);
                                mapMarkRectRows(g3, rectB);
                                mapMarkRectRows(g3, rectC);
                                mapMarkRectRows(g3, rectD);
                                {
                                    int loadedCount = 0;
                                    int zc[2];
                                    int blockX;
                                    char* cellState;
                                    zc[0] = 0;
                                    zc[1] = zc[0];
                                    cellState = g3;
                                    do
                                    {
                                        for (blockX = 0; blockX < 16; blockX++)
                                        {
                                            int bx = gMapBlockOriginX + blockX;
                                            int bz = gMapBlockOriginZ + zc[1];
                                            if (*cellState == -3)
                                            {
                                                if (mapLoadBlock(blockX, zc[1], bx, bz, layer) == 0)
                                                {
                                                    *cellState = -2;
                                                }
                                                else
                                                {
                                                    gMapLayerCellStates[zc[0]] = (s8)loadedCount++;
                                                }
                                            }
                                            zc[0]++;
                                            cellState++;
                                        }
                                        zc[1]++;
                                    } while (zc[1] < 16);
                                }
                                aBase++;
                                cBase++;
                            }
                        }
                        clearForceLoadImmediately();
                    }
                }
                {
                    s8 first = 1;
                    int slotIndex = gShaderRomListSlotCount - 1;
                    ShaderRomListSlot* romListSlot = (ShaderRomListSlot*)(base + 0x418C) + slotIndex;
                    for (; slotIndex >= 0; slotIndex--)
                    {
                        if (romListSlot->flag == 0)
                        {
                            if (romListSlot->romlist != NULL)
                            {
                                s16 sl = romListSlot->slot;
                                defStartFn_8005972c(romListSlot->romlist, (u32*)(base + sl * 0x8C + 0x4208), sl, 1);
                                mm_free(romListSlot->romlist);
                                ((int*)(base + 0x83A8))[sl] = 0;
                            }
                            romListSlot->romlist = NULL;
                            romListSlot->slot = -1;
                        }
                        if (first)
                        {
                            if (romListSlot->romlist == NULL)
                                gShaderRomListSlotCount--;
                            else
                                first = 0;
                        }
                        romListSlot--;
                    }
                }
                {
                    for (i = 0; i < cnt; i++)
                    {
                        s16 blockId = savedBlocks->blockId;
                        if (blockId >= 0)
                        {
                            gMapBlockRefCounts[blockId] -= 1;
                            if (gMapBlockRefCounts[blockId] == 0)
                            {
                                MapBlockData* block = gMapBlocks[blockId];
                                MapShader* shader;
                                MapShaderLayer* shaderLayer;
                                int shaderIndex;
                                int layerIndex;
                                int textureIndex;
                                u32 scrollSlot;
                                gMapBlockIds[blockId] = -1;
                                gMapBlocks[blockId] = NULL;
                                for (shaderIndex = 0; shaderIndex < block->shaderCount; shaderIndex++)
                                {
                                    shader = &block->shaders[shaderIndex];
                                    for (layerIndex = 0; layerIndex < shader->layerCount; layerIndex++)
                                    {
                                        shaderLayer = &shader->layers[layerIndex];
                                        scrollSlot = shaderLayer->scrollMtx;
                                        if (scrollSlot != 0xff)
                                        {
                                            if (lbl_803DCE68[scrollSlot].refCount != 0)
                                                lbl_803DCE68[scrollSlot].refCount -= 1;
                                        }
                                        if (shaderLayer->overrideType != 0)
                                            mapTextureOverrideRelease(shaderLayer->texture,
                                                                      shaderLayer->overrideType);
                                    }
                                }
                                for (textureIndex = 0; textureIndex < block->textureCount; textureIndex++)
                                    textureFree(block->textures[textureIndex].texture);
                                if (block->auxData != NULL)
                                    mm_free(block->auxData);
                                if (block->hits != NULL)
                                    mm_free(block->hits);
                                setMapBlockFlag();
                                mm_free(block);
                            }
                        }
                        savedBlocks++;
                    }
                }
                lbl_803DCE70 = 0;
                lbl_803DCDED = 0;
            }
            mapLoadUnloadObjects(doLoad);
            gMapPendingFileFlags = getLoadedFileFlags(0);
            renderFlags &= ~0x4000LL;
        }
    }
}

void loadMapForCameraPos(float x, float y, float z)
{
    if ((renderFlags & 2) != 0 && (renderFlags & 0x800) == 0)
        return;
    gShaderLoadCenterX = x;
    gShaderLoadCenterY = y;
    gShaderLoadCenterZ = z;
    renderFlags |= 2;
    if ((renderFlags & 0x800) != 0)
    {
        doPendingMapLoads();
    }
}

void mapInitSetRects(s16* rect, u8* bitmap, int originX, int originY, int idx)
{
    u8* self = lbl_803DCE78;
    int tabOff = idx * 7 << 2;
    int offset0 = *(int*)(lbl_803DCE7C + tabOff);

    getTabEntry(self, MLDF_FILEID_MAPS_BIN, offset0, *(int*)((lbl_803DCE7C + 8) + tabOff) - offset0);
    *(int*)(self + 0xc) = (int)self + *(int*)((lbl_803DCE7C + 4) + tabOff) - *(int*)(lbl_803DCE7C + tabOff);
    rect[0] = originX - *(s16*)(self + 4);
    rect[2] = originY - *(s16*)(self + 6);
    rect[1] = rect[0] + *(s16*)(self + 0) - 1;
    rect[3] = rect[2] + *(s16*)(self + 2) - 1;
    *(s8*)((char*)rect + 8) = *(s16*)(self + 4);
    *(s8*)((char*)rect + 9) = *(s16*)(self + 6);
    for (originY = 0; (s16)originY < *(s16*)(self + 2); originY++)
    {
        for (originX = 0; (s16)originX < *(s16*)(self + 0); originX++)
        {
            int pixelIdx = (s16)originX + (s16)originY * *(s16*)(self + 0);
            if ((int)(*(u32*)(*(int*)(self + 0xc) + pixelIdx * 4) >> 23 & 0xff) != 0xff)
            {
                bitmap[pixelIdx >> 3] |= 1 << (pixelIdx & 7);
            }
        }
    }
}

void mapInitSetRects(s16* rect, u8* bitmap, int originX, int originY, int idx);

void initMaps(void)
{
    void* data;
    int total;
    int i;
    int ofs[1];
    int idx;
    char* e;

    data = 0;
    total = getDataFileSize(MLDF_FILEID_GLOBALMA_BIN);
    loadAssetFileById(&data, MLDF_FILEID_GLOBALMA_BIN);
    gShaderMapRomBuffers[0] = -1;
    gShaderMapRomBuffers[1] = (int)mmAlloc(1280, 5, 0);
    gShaderMapRomBuffers[2] = (int)mmAlloc(512, 5, 0);
    gShaderMapRomBuffers[3] = (int)mmAlloc(128, 5, 0);
    gShaderMapRomBuffers[4] = (int)mmAlloc(8192, 5, 0);
    memset((void*)gShaderMapRomBuffers[4], 0, 8192);
    idx = 0;
    ofs[0] = 0;
    for (i = 0; i < 16; i++)
    {
        INIT_MAP_SLOT(0);
        INIT_MAP_SLOT(1);
        INIT_MAP_SLOT(2);
        INIT_MAP_SLOT(3);
        INIT_MAP_SLOT(4);
        INIT_MAP_SLOT(5);
        INIT_MAP_SLOT(6);
        INIT_MAP_SLOT(7);
        ofs[0] += 80;
        idx += 8;
    }
    i = 0;
    total /= 12;
    while (i < total && *(s16*)((char*)data + i * 12 + 6) > -1)
    {
        *(s8*)((char*)gShaderMapRomBuffers[3] + *(s16*)((char*)data + i * 12 + 6)) =
            (s8)*(s16*)((char*)data + i * 12 + 4);
        mapInitSetRects((s16*)((char*)gShaderMapRomBuffers[1] + *(s16*)((char*)data + i * 12 + 6) * 10),
                        (u8*)((char*)gShaderMapRomBuffers[4] + *(s16*)((char*)data + i * 12 + 6) * 64),
                        *(s16*)((char*)data + i * 12), *(s16*)((char*)data + i * 12 + 2),
                        *(s16*)((char*)data + i * 12 + 6));
        ((s16*)gShaderMapRomBuffers[2])[*(s16*)((char*)data + i * 12 + 6) << 1] =
            *(s16*)((char*)data + i * 12 + 8);
        ((s16*)gShaderMapRomBuffers[2])[(*(s16*)((char*)data + i * 12 + 6) << 1) + 1] =
            *(s16*)((char*)data + i * 12 + 0xa);
        i++;
    }
    curMapType = 0;
    lbl_803DCEB6 = 0;
    lbl_803DCEB4 = 0;
    mm_free(data);
}

extern int lbl_803DB648;
extern void* lbl_803DCEA0;

MapRomList* mapBlockFn_800592e4(void)
{
    char* p = (char*)lbl_803822A0[0];
    int v = *(s16*)(p + 0x594);
    if (v < 0)
    {
        v = lbl_803DB648;
    }
    if (v < 0)
    {
        return 0;
    }
    {
        MapRomList* res = gLoadedRomListPages[v];
        if (res == 0)
        {
            return res;
        }
        lbl_803DB648 = v;
        lbl_803DCEA0 = res;
        return res;
    }
}

MapCellEntry* mapGetCellEntry(int x, int z)
{
    int* base = (int*)lbl_803822A0[0];
    return (MapCellEntry*)((char*)base + (x + (z << 4)) * 12);
}

extern s16 lbl_803DCE90;
extern u16* lbl_803DCE84;

void mapBlockFn_80059354(int x, int z, MapCellEntry* out, int layer)
{
    int id;
    char* activeFlags;
    MapRomListGrid* grid;
    int adjacentMapId2;
    char* slots;
    int slot;
    int adjacentMapId1;
    s16* adjacentMapIds;
    s16* mapBounds;
    u32 cell;

    id = mapCoordsToId(x, z, layer);
    if (id != -1)
    {
        slots = (char*)gShaderRomListSlots;
        slot = mapFindRomListSlot(slots, id);
        if (slot == -1)
            slot = mapProcessRomList(id);
        *(s8*)((activeFlags = (char*)gShaderRomListSlots + 6) + slot * 8) = 1;
        grid = (MapRomListGrid*)gShaderRomListSlots[slot].romList;
        adjacentMapIds = (s16*)gShaderMapRomBuffers[2];
        adjacentMapId1 = (s8)adjacentMapIds[id << 1];
        adjacentMapId2 = adjacentMapIds[(id << 1) + 1];
        adjacentMapId2 = (s8)adjacentMapId2;
        out->mapId = id;
        out->adjacentMapId1 = adjacentMapId1;
        out->adjacentMapId2 = adjacentMapId2;
        if (adjacentMapId1 != -1)
        {
            slot = mapFindRomListSlot(slots, adjacentMapId1);
            if (slot == -1)
                slot = mapProcessRomList(adjacentMapId1);
            *(s8*)(activeFlags + slot * 8) = 1;
        }
        if (adjacentMapId2 != -1)
        {
            slot = mapFindRomListSlotAndAdvance(&slots, adjacentMapId2);
            if (slot == -1)
                slot = mapProcessRomList(adjacentMapId2);
            *(s8*)(activeFlags + slot * 8) = 1;
        }
        mapBounds = (s16*)(gShaderMapRomBuffers[1] + id * 10);
        x = x - mapBounds[0];
        z = z - mapBounds[2];
        cell = grid->cells[x + z * grid->width];
        out->cellIndex = (cell >> 0x11) & 0x3f;
        out->romListIndex = (cell >> 0x17) & 0xff;
        if (out->romListIndex == 0xFF)
            out->romListIndex = -1;
        if (out->romListIndex == -1)
        {
            out->blockId = -1;
        }
        else
        {
            if (out->romListIndex >= lbl_803DCE90)
                out->romListIndex = lbl_803DCE90 - 1;
            out->blockId = out->cellIndex + lbl_803DCE84[out->romListIndex];
            if (out->blockId >= lbl_803DCE84[lbl_803DCE90])
                out->blockId = lbl_803DCE84[lbl_803DCE90] - 1;
        }
    }
    else
    {
        out->mapId = -1;
        out->adjacentMapId1 = -1;
        out->adjacentMapId2 = -1;
        out->blockId = -2;
        out->romListIndex = -1;
        out->cellIndex = 0;
    }
}

int mapGetRomListAndOffsets(int p1, int b);

void mapLoadForObject(int mapId, GameObject* obj)
{
    int saved = gShaderCurMapEventId;
    int slot;
    int romList = mapGetRomListAndOffsets(mapId, 1);
    int i;
    slot = 0x50;

    for (i = 0; i < 40; i++)
    {
        if (gLoadedRomListPages[slot] == NULL)
        {
            gLoadedRomListPages[slot] = (MapRomListPage*)romList;
            break;
        }
        slot++;
    }
    obj->anim.pad34 = slot;
    (*gMapEventInterface)->setMapActLut(mapId, slot);
    defStartFn_8005972c((char*)romList, (u32*)&lbl_803822C8[slot * 0x8c], slot, 0);
    (*gMapEventInterface)->updateObjGroups(slot);
    gShaderCurMapEventId = saved;
}

void defStartFn_8005972c(char* p, u32* tbl, int idx, int flag)
{
    char* cur;
    int count;
    int pos;
    u8 found;
    u32 mask;
    int* row;
    int entry;
    s16 t;
    int step;
    int n2;
    int minVal;

    found = 0;
    mask = 0;
    cur = (char*)((MapRomListPage*)p)->objects;
    count = *(u16*)(p + 8);
    if (count != 0)
    {
        pos = 0;
        if (flag == 0)
        {
            tbl[0x21] = -1;
            tbl[0] = -1;
            tbl[1] = -1;
            tbl[2] = -1;
            tbl[3] = -1;
            tbl[4] = -1;
            tbl[5] = -1;
            tbl[6] = -1;
            tbl[7] = -1;
            tbl[8] = -1;
            tbl[9] = -1;
            tbl[10] = -1;
            tbl[11] = -1;
            tbl[12] = -1;
            tbl[13] = -1;
            tbl[14] = -1;
            tbl[15] = -1;
            tbl[16] = -1;
            tbl[17] = -1;
            tbl[18] = -1;
            tbl[19] = -1;
            tbl[20] = -1;
            tbl[21] = -1;
            tbl[22] = -1;
            tbl[23] = -1;
            tbl[24] = -1;
            tbl[25] = -1;
            tbl[26] = -1;
            tbl[27] = -1;
            tbl[28] = -1;
            tbl[29] = -1;
            tbl[30] = -1;
            tbl[31] = -1;
        }
        for (; pos < count;)
        {
            if (flag != 0)
            {
                if (*(s16*)cur == 110)
                    (*gRomCurveInterface)->addCurveDef((RomCurveDef*)cur);
                if (*(s16*)cur == 5)
                    (*gCheckpointInterface)->removeRouteEntry((CheckpointRouteEntry*)cur);
            }
            else
            {
                t = *(s16*)cur;
                if (t == 110 || t == 5)
                {
                    if (t == 110)
                        (*gRomCurveInterface)->remove((RomCurveDef*)cur);
                    else
                        (*gCheckpointInterface)->addRouteEntry((CheckpointRouteEntry*)cur);
                    if (found == 0)
                    {
                        tbl[0x21] = (int)cur - (int)((MapRomListPage*)p)->objects;
                        found = 1;
                    }
                }
                else if (*(u8*)(cur + 4) & 0x10)
                {
                    if ((mask & (1 << *(u8*)(cur + 6))) == 0)
                    {
                        tbl[*(u8*)(cur + 6)] = (int)cur - (int)((MapRomListPage*)p)->objects;
                        mask |= 1 << *(u8*)(cur + 6);
                    }
                }
            }
            step = *(u8*)(cur + 2) * 4;
            pos += step;
            cur += step;
        }
        if (flag == 0)
        {
            minVal = count;
            entry = tbl[0x21];
            if (entry != -1 && entry < count)
                minVal = entry;
            row = (int*)tbl;
            for (n2 = 0; n2 < 32; n2++)
            {
                entry = row[n2];
                if (entry != -1 && entry < minVal)
                    minVal = entry;
            }
            tbl[0x22] = minVal;
            entry = tbl[0x21];
            if (entry != -1)
                tbl[0x20] = entry;
            else
                tbl[0x20] = count;
        }
    }
}

#undef INIT_MAP_SLOT

void mapUnloadRomListPage(int pageIndex)
{
    int idx = pageIndex;
    MapRomListPage* p = gLoadedRomListPages[idx];
    if (p != 0)
    {
        defStartFn_8005972c((char*)p, (u32*)(lbl_803822C8 + idx * 0x8C), idx, 1);
        mm_free(gLoadedRomListPages[idx]);
        gLoadedRomListPages[idx] = 0;
    }
}

int mapCoordsToId(int x, int z, int layerIdx)
{
    int x0, z0;
    s8* layers;
    int x1;
    s16* rects;
    u8* bits;
    int id;
    int layer;
    int idx;

    layer = curMapLayer + lbl_803DB624[layerIdx];
    rects = (s16*)gShaderMapRomBuffers[1];
    bits = (u8*)gShaderMapRomBuffers[4];
    id = 0;
    layers = (s8*)gShaderMapRomBuffers[3];
    for (; id < 128; id++)
    {
        if (layer == layers[0])
        {
            x0 = rects[0];
            if (x >= x0)
            {
                x1 = rects[1];
                if (x <= x1)
                {
                    z0 = rects[2];
                    if (z >= z0 && z <= rects[3])
                    {
                        idx = (x - x0) + (z - z0) * ((x1 - x0) + 1);
                        if ((1 << (idx & 7)) & bits[idx >> 3])
                            return id;
                    }
                }
            }
        }
        rects += 5;
        bits += 0x40;
        layers += 1;
    }
    return -1;
}

char sShaderDebugStrings[172] = {
    0, 0, 0, 52, 0, 0, 0, 52, 0, 0, 0, 52, 0, 0, 0, 52, 0, 0, 0, 52, 0, 0, 0, 52, 0, 0, 0, 56, 0, 0, 0, 52, 0, 0, 0, 60,
    0, 0, 0, 56, 0, 0, 0, 60, 0, 0, 0, 64, 0, 0, 0, 52, 0, 0, 0, 52, 0, 0, 0, 52, 0, 0, 0, 52, 0, 0, 0, 52, 0, 0, 0, 52,
    0, 0, 0, 52, 0, 0, 0, 52, 0, 0, 0, 52, 0, 0, 0, 56, 0, 0, 0, 52, 0, 0, 0, 56, 0, 0, 0, 68, 0, 0, 0, 52, 0, 0, 0, 60,
    0, 0, 0, 56, 0, 0, 0, 52, 0, 0, 0, 56, 0, 0, 0, 60, 0, 0, 0, 52, 0, 0, 0, 52, 0, 0, 0, 52, 0, 0, 0, 52, 0, 0, 0, 52,
    0, 0, 0, 52, 0, 0, 0, 52, 0, 0, 0, 68, 0, 0, 0, 52, 0, 0, 0, 52, 0, 0, 0, 52, 0, 0, 0, 52,
};

s8 gShaderMapTextDirTable[120] = {
    42, 42, 18, -1, 69, -1, -1, 44, 44, 23, 40, 71, 7,  70, 27, -1, 9,  -1, 36, 15, -1, 17, -1, 24,
    24, 24, 0,  16, 5,  8,  25, 14, 37, 20, 22, -1, -1, -1, 1,  12, 39, 72, -1, 10, 4,  -1, -1, -1,
    6,  -1, 13, 43, 19, -1, 38, -1, 29, -1, 1,  1,  1,  1,  1,  -1, -1, -1, -1, 30, 31, 32, 33, 26,
    34, 35, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 4,  -1, -1, -1, -1, -1, -1, 0,  0,
};

f32 sAabbCornerDirections[24] = {
    1.0f,  1.0f, 1.0f, 1.0f,  -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
    -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
};

/* Screen-space (x,y) sample offsets used by the sun occlusion depth probe:
 * center plus the four corners of a 30-pixel box. */
int gSunOcclusionSampleOffsets[10] = {
    0, 0, -15, -15, 15, -15, 15, 15, -15, 15,
};

/* Scene geometry draw-order table (referenced by lightmap.c). */
u8 lbl_8030E65C[16] = {7, 6, 5, 4, 3, 2, 1, 0, 8, 9, 10, 11, 12, 13, 14, 15};

struct
{
    u8 drawOrder[16];
    char passLevelObject[28];
    char failManualLoad[24];
    char failOutsideMap[40];
    char failNoBlock[24];
    char passBlockObject[28];
    char passInRange[24];
    char failOutOfRange[28];
} lbl_8030E66C = {
    {0, 15, 1, 14, 2, 13, 3, 12, 4, 11, 5, 10, 6, 9, 8, 7},
    "LOAD PASS: Level object\n",
    "LOAD FAIL: Manual load\n",
    "LOAD FAIL: Outside map x=%f y=%f z=%f\n",
    "LOAD FAIL: No block\n",
    "LOAD PASS: Block object\n",
    "LOAD PASS: In range %f\n",
    "LOAD FAIL: Out of range\n",
};

char sTrackGlobalTexanimOverflowError[] = "TRACK ERROR: Global texanim overflow\n";

char sTrackLoadBlockOverrunError[] = "trackLoadBlockEnd: track block overrun\n";

char sTrackPiLockedFormat[] = "track piLocked %x\n";

char sTrackCellCoordFormat[] = " cellx %i celly %i cellz %i ";

void mapBlockFn_80059c2c(u8* outFlags)
{
    int i;
    int outer;
    for (outer = 0; outer < 0x78; outer++)
    {
        i = mapFindRomListSlotById(outer);
        if (i == -1)
        {
            outFlags[outer] = 0;
        }
        else
        {
            outFlags[outer] = 1;
        }
    }
}

int mapProcessRomList(int slot)
{
    char* base;
    int j;
    char* obj;
    int i;
    char* cur;
    u8 flag;
    ShaderRomListSlot* p;
    int count;
    ShaderRomListSlot* slots;
    ShaderRomListSlot* entry;
    s16* rects;
    int step;
    int rl;
    f32 dx, dz;

    base = (char*)lbl_8037E0C0;
    flag = 0;
    while (isRomListLoading())
    {
        padUpdate();
        checkReset();
        if (flag)
            waitNextFrame();
        loadDataFiles();
        dvdCheckError();
        if (flag)
        {
            mmFreeTick(0);
            gameTextRun();
            GXFlush_(1, 0);
        }
        if (gDvdErrorPauseActive)
            flag = 1;
    }
    i = 0;
    p = (ShaderRomListSlot*)(base + 0x418C);
    count = gShaderRomListSlotCount;
    while (i < count && p->romlist != 0)
    {
        p++;
        i++;
    }
    if (i == count)
        gShaderRomListSlotCount++;
    rl = mapGetRomListAndOffsets(slot, 0);
    slots = (ShaderRomListSlot*)(base + 0x418C);
    entry = &slots[i];
    entry->romlist = (void*)rl;
    *(int*)(slot * 4 + 0x83A8 + (char*)base) = rl;
    ((s16*)((char*)base + 0x4190))[i * 4] = slot;
    lbl_803DCEA0 = entry->romlist;
    rects = (s16*)(*(int*)(base + 0x417C) + slot * 10);
    *(u8*)((char*)lbl_803DCEA0 + 0x19) = *(u8*)(*(int*)(base + 0x4184) + slot);
    *(f32*)((char*)lbl_803DCEA0 + 0x24) = gMapBlockWorldSize * (f32)(rects[0] + *(s16*)((char*)lbl_803DCEA0 + 4));
    *(f32*)((char*)lbl_803DCEA0 + 0x28) = gMapBlockWorldSize * (f32)(rects[2] + *(s16*)((char*)lbl_803DCEA0 + 6));
    cur = lbl_803DCEA0;
    dz = *(f32*)(cur + 0x28);
    dx = *(f32*)(cur + 0x24);
    if (cur != 0)
    {
        obj = (char*)((MapRomListPage*)cur)->objects;
        for (j = 0; j < *(u16*)(cur + 8);)
        {
            if (saveGame_restoreObjectPosToRomList(obj) == 0)
            {
                ((GameObject*)obj)->anim.rootMotionScale += dx;
                ((GameObject*)obj)->anim.localPosY += dz;
            }
            step = *(u8*)(obj + 2) * 4;
            j += step;
            obj += step;
        }
    }
    lbl_803DB620 = slot;
    return i;
}

int mapGetRomListAndOffsets(int p1, int flag)
{
    int words = p1 * 7;
    int offset0 = *(int*)(lbl_803DCE7C + (words << 2));
    int tailLen = *(int*)((lbl_803DCE7C + 0x1c) + ((u32)words << 2)) - offset0;
    int v0, v1, v2;
    int i;

    mapsBinGetRomlistSize(offset0, &v0, &v1, &v2, words);
    lbl_803DCEA0 = mmAlloc(tailLen + (v0 + 7 >> 3) + 0x401 + v2, 5, 0);
    fileLoadToBufferOffset(MLDF_FILEID_MAPS_BIN, lbl_803DCEA0, offset0, tailLen);

    *(int*)((char*)lbl_803DCEA0 + 0xc) = (int)lbl_803DCEA0 + *(int*)((lbl_803DCE7C + 4) + (words << 2)) - offset0;
    *(int*)((char*)lbl_803DCEA0 + 0x14) = (int)lbl_803DCEA0 + *(int*)((lbl_803DCE7C + 8) + (words << 2)) - offset0;
    *(int*)((char*)lbl_803DCEA0 + 0x30) = (int)lbl_803DCEA0 + *(int*)((lbl_803DCE7C + 0xc) + (words << 2)) - offset0;
    *(int*)((char*)lbl_803DCEA0 + 0x2c) = (int)lbl_803DCEA0 + *(int*)((lbl_803DCE7C + 0x10) + (words << 2)) - offset0;
    *(int*)((char*)lbl_803DCEA0 + 0x34) = (int)lbl_803DCEA0 + *(int*)((lbl_803DCE7C + 0x14) + (words << 2)) - offset0;
    *(int*)((char*)lbl_803DCEA0 + 0x20) = (int)lbl_803DCEA0 + *(int*)((lbl_803DCE7C + 0x18) + (words << 2)) - offset0;

    piRomLoadSection(*(int*)((lbl_803DCE7C + 0x18) + (words << 2)), p1, *(int*)((char*)lbl_803DCEA0 + 0x20));
    *(int*)((char*)lbl_803DCEA0 + 0x10) = (*(int*)((lbl_803DCE7C + 0x1c) + (words << 2)) + v2) + (int)lbl_803DCEA0 - offset0;

    for (i = 0; i < (v0 + 7 >> 3) + 1; i++)
    {
        *(u8*)(*(int*)((char*)lbl_803DCEA0 + 0x10) + i) = 0;
    }
    {
        f32 fillVal = lbl_803DEBCC;
        *(f32*)((char*)lbl_803DCEA0 + 0x24) = fillVal;
        *(f32*)((char*)lbl_803DCEA0 + 0x28) = fillVal;
    }
    *(u8*)((char*)lbl_803DCEA0 + 0x18) = 0;
    *(u8*)((char*)lbl_803DCEA0 + 0x19) = 0;
    if (flag == 0)
    {
        defStartFn_8005972c(lbl_803DCEA0, (u32*)&lbl_803822C8[p1 * 0x8c], p1, 0);
        (*gMapEventInterface)->updateObjGroups(p1);
    }
    return (int)lbl_803DCEA0;
}

extern f32 lbl_803DEBEC;
extern f32 lbl_803DEBF0;
extern FrustumPlane gViewFrustumPlanes[];

int ViewFrustum_IsSphereVisible(float* center, float radius)
{
    FrustumPlane* plane;
    u8 i = 0;
    f32 offZ = playerMapOffsetZ;
    f32 offX = playerMapOffsetX;
    for (; i < FRUSTUM_PLANE_COUNT; i++)
    {
        float dot;
        plane = &gViewFrustumPlanes[i];
        dot = plane->distance + (plane->normalZ * (center[2] - offZ) +
                                 (center[1] * plane->normalY + plane->normalX * (center[0] - offX)));
        if (radius + dot < *(f32*)&lbl_803DEBCC)
            return 0;
    }
    return 1;
}

extern f32 lbl_803DEBB8;
extern f32 lbl_803DEBD4;
extern f32 lbl_803DEBD8;
extern f32 lbl_803DEBDC;

int objUpdateOpacity(GameObject* obj)
{
    u8 op;
    char* ptr;
    int alpha;
    f32 range;
    f32 d;
    f32 near;
    GameObject* player;
    u8 i;
    f32 o1, o2, o3;
    f32 sz;
    f32 o5, o6;
    f32 prod;
    f32 offZ, offX;

    op = ((GameObject*)obj)->anim.alpha;
    if (op == 0)
    {
        ((GameObject*)obj)->anim.renderAlpha = 0;
        return 0;
    }
    ptr = (void*)((GameObject*)obj)->anim.placementData;
    if (ptr != 0 && (*(u8*)(ptr + 5) & 1))
    {
        ((GameObject*)obj)->anim.renderAlpha = (u8)(((op + 1) * 255) >> 8);
    }
    else
    {
        range = ((GameObject*)obj)->anim.cullDistance2;
        if (range < lbl_803DEBB8)
        {
            ((GameObject*)obj)->anim.renderAlpha = 0;
            return 0;
        }
        player = Obj_GetPlayerObject();
        if (ptr != 0 && (*(u8*)(ptr + 5) & 2) && player != 0)
        {
            d = Vec_distance(&((GameObject*)obj)->anim.worldPosX, &((GameObject*)player)->anim.worldPosX);
        }
        else
        {
            d = Camera_DistanceToCurrentViewPosition(((GameObject*)obj)->anim.worldPosX,
                                                     ((GameObject*)obj)->anim.worldPosY,
                                                     ((GameObject*)obj)->anim.worldPosZ);
        }
        if (d > range)
        {
            ((GameObject*)obj)->anim.renderAlpha = 0;
            return 0;
        }
        alpha = 255;
        near = range - lbl_803DEBD4;
        if (d > near)
        {
            range = range - near;
            d = d - near;
            alpha = (int)(lbl_803DEBD8 * (lbl_803DEBDC - d / range));
        }
        Camera_ProjectWorldSphere(((GameObject*)obj)->anim.worldPosX - playerMapOffsetX,
                                  ((GameObject*)obj)->anim.worldPosY,
                                  ((GameObject*)obj)->anim.worldPosZ - playerMapOffsetZ,
                                  ((GameObject*)obj)->anim.hitboxScale * ((GameObject*)obj)->anim.rootMotionScale, &o1,
                                  &o2, &o3, &sz, &o5, &o6);
        sz = __fabsf(sz);
        sz = sz * gMapBlockWorldSize;
        if (sz < 10.0f)
        {
            ((GameObject*)obj)->anim.renderAlpha = 0;
            return 0;
        }
        if (sz < 15.0f)
        {
            alpha = (int)(((f32)alpha * (sz - 10.0f)) / 5.0f);
        }
        ((GameObject*)obj)->anim.renderAlpha = (u8)((alpha * (((GameObject*)obj)->anim.alpha + 1)) >> 8);
    }
    if (((GameObject*)obj)->anim.renderAlpha == 0)
    {
        return 0;
    }
    else
    {
        prod = ((GameObject*)obj)->anim.hitboxScale * ((GameObject*)obj)->anim.rootMotionScale;
        i = 0;
        offZ = playerMapOffsetZ;
        offX = playerMapOffsetX;
        for (; i < FRUSTUM_PLANE_COUNT; i++)
        {
            FrustumPlane* plane = &gViewFrustumPlanes[i];
            if (prod + (plane->distance + (plane->normalZ * (((GameObject*)obj)->anim.worldPosZ - offZ) +
                                           (((GameObject*)obj)->anim.worldPosY * plane->normalY +
                                            plane->normalX * (((GameObject*)obj)->anim.worldPosX - offX)))) <
                *(f32*)&lbl_803DEBCC)
                return 0;
        }
    }
    return 1;
}
void mapDebugRender(int* state)
{
    int y1;
    int y0;
    int sz;
    MapBlockData* blk;
    int dy;
    int sx;
    int y0a;
    int bz;
    int ci;
    int wx;
    f32 cy;
    int bx;
    int yy;
    s8* tbl;
    int h;
    int step;
    int row;
    int cx;
    int cz;
    int cell;
    int v;
    int n;
    int wz;

    if (lbl_803DCDED != 0)
    {
        bx = fastFloorf((*(f32*)((char*)lbl_803DCEA8 + 0xc) - playerMapOffsetX) / gMapBlockWorldSize);
        bz = fastFloorf((*(f32*)((char*)lbl_803DCEA8 + 0x14) - playerMapOffsetZ) / gMapBlockWorldSize);
        tbl = gMapBlockLayerTables[0];
        if (bx < 0 || bz < 0 || bx >= 16 || bz >= 16)
        {
            blk = 0;
        }
        else
        {
            ci = tbl[bx + bz * 16];
            if (ci < 0 || ci >= lbl_803DCE98)
            {
                blk = 0;
            }
            else
            {
                blk = gMapBlocks[ci];
            }
        }
        sx = (int)(gMapBlockWorldSize * fastFloorf(*(f32*)((char*)lbl_803DCEA8 + 0xc) / gMapBlockWorldSize));
        sz = (int)(gMapBlockWorldSize * fastFloorf(*(f32*)((char*)lbl_803DCEA8 + 0x14) / gMapBlockWorldSize));
        wx = (int)(*(f32*)((char*)lbl_803DCEA8 + 0xc) - sx);
        wz = (int)(*(f32*)((char*)lbl_803DCEA8 + 0x14) - sz);
        if (blk != 0)
        {
            y0 = blk->minY;
            y0a = y0;
            if (y0 & 1)
                y0a = y0 - 1;
            cy = *(f32*)((char*)lbl_803DCEA8 + 0x10);
            y1 = blk->maxY;
            if (cy > y1)
                cy = (f32)(y1 - 1);
            yy = cy;
            dy = yy - y0a;
            h = y1 - y0;
            if (h / 80 < 8)
                step = h / 8;
            else
                step = 80;
            row = dy / step;
            cx = wx / 80;
            cz = wz / 80;
            cell = row * 0x40;
            cell += cz * 8;
            cell += cx;
            logPrintf(sTrackCellCoordFormat, cx, row, cz);
            v = lbl_803DCE70;
            n = v >> 3;
            if (v & 7)
                n = n + 1;
            modelRenderInstrsState_init((ModelRenderInstrsState*)state, (void*)(lbl_803DCE74 + n * cell), v, v);
        }
    }
}

int mapRectFn_8005a728(int bx, int bz, MapBlockData* block)
{
    f32 a1, a2, b1, b2, c1, c2;
    f32 p3;
    f32 fx, fz, x2, z2, y0, y1;
    f32 v;
    FrustumPlane* plane;
    int i;
    int j;
    int hit;

    fx = gMapBlockWorldSize * bx;
    fz = gMapBlockWorldSize * bz;
    x2 = gMapBlockWorldSize + fx;
    z2 = gMapBlockWorldSize + fz;
    if (block)
    {
        y0 = block->minY;
        y1 = block->maxY;
    }
    else
    {
        y0 = lbl_803DEBEC;
        y1 = lbl_803DEBF0;
    }
    plane = gViewFrustumPlanes;
    for (i = 0; i < FRUSTUM_PLANE_COUNT; i++)
    {
        f32 p0 = plane[i].normalX;
        f32 p1 = plane[i].normalY;
        f32 p2 = plane[i].normalZ;
        p3 = plane[i].distance;
        j = 0;
        hit = 0;
        a1 = fx * p0;
        a2 = x2 * p0;
        b1 = fz * p2;
        b2 = z2 * p2;
        c1 = y0 * p1;
        c2 = y1 * p1;
        while (j < 8 && hit == 0)
        {
            if (j & 1)
                v = a1;
            else
                v = a2;
            if (j & 2)
                v += b1;
            else
                v += b2;
            if (j & 4)
                v += c1;
            else
                v += c2;
            v += p3;
            if (v > lbl_803DEBCC)
                hit = 1;
            j++;
        }
        if (j == 8 && hit == 0)
            return 0;
    }
    return 1;
}

void frustumPlanes_updateAabbCornerIndices(FrustumPlane* planes, int count)
{
    int k;
    int j;
    int bi;
    f32 best;
    f32 v;

    for (k = 0; k < count; k++)
    {
        best = lbl_803DEBCC;
        j = 0;
        while (j < 24)
        {
            v = planes->normalX * sAabbCornerDirections[j++];
            v += planes->normalY * sAabbCornerDirections[j++];
            v += planes->normalZ * sAabbCornerDirections[j++];
            if (v > best)
            {
                best = v;
                bi = j - 3;
            }
        }
        switch (bi)
        {
        case 0:
            planes->aabbCornerIndex = 0;
            break;
        case 3:
            planes->aabbCornerIndex = 2;
            break;
        case 6:
            planes->aabbCornerIndex = 5;
            break;
        case 9:
            planes->aabbCornerIndex = 7;
            break;
        case 0xc:
            planes->aabbCornerIndex = 1;
            break;
        case 0xf:
            planes->aabbCornerIndex = 3;
            break;
        case 0x12:
            planes->aabbCornerIndex = 4;
            break;
        case 0x15:
            planes->aabbCornerIndex = 6;
            break;
        }
        planes++;
    }
}

extern FrustumPlane gPlayerRelativeFrustumPlanes[];
extern f32 lbl_803DEBF4;
void playerVecFn_8005a9b0(void)
{
    Vec tmp;
    Vec camPos;
    PlayerFrustumPlaneScales scales;
    PlayerFrustumPlaneDirections planes;
    GameObject* player;
    CameraViewSlot* viewSlot;
    FrustumPlane* outPtr;
    int i;
    f32* invRotMtx;
    f32 clipDist;

    planes = sPlayerFrustumPlaneDirs;
    scales = sPlayerFrustumPlaneScales;
    player = Obj_GetPlayerObject();
    viewSlot = Camera_GetCurrentViewSlot();
    camPos.x = viewSlot->worldX - playerMapOffsetX;
    camPos.y = viewSlot->worldY;
    camPos.z = viewSlot->worldZ - playerMapOffsetZ;
    invRotMtx = Camera_GetInverseViewRotationMatrix();
    if (player != NULL)
    {
        clipDist = -Camera_DistanceToCurrentViewPosition(((GameObject*)player)->anim.worldPosX,
                                                         ((GameObject*)player)->anim.worldPosY,
                                                         ((GameObject*)player)->anim.worldPosZ);
    }
    else
    {
        clipDist = lbl_803DEBF4;
    }
    scales.v[0] = clipDist;

    outPtr = gPlayerRelativeFrustumPlanes;
    for (i = 0; i < FRUSTUM_PLANE_COUNT; i++)
    {
        PSMTXMultVec((const f32 (*)[4])invRotMtx, &planes.v[i], (Vec*)&outPtr[i].normalX);
        PSVECScale(&outPtr[i].normal, &tmp, scales.v[i]);
        PSVECAdd(&camPos, &tmp, &tmp);
        outPtr[i].distance = -PSVECDotProduct(&tmp, &outPtr[i].normal);
    }
    frustumPlanes_updateAabbCornerIndices(gPlayerRelativeFrustumPlanes, FRUSTUM_PLANE_COUNT);
}

u8 lbl_80380209[0x1DFF];
u8 lbl_80382008[0x30];
ModelLightStruct* gGlowLightList[100];
f32 distortionFilterVector[0x1c];
int gShaderMapRomBuffers[0x5];
BlockEntry gShaderRomListSlots[8];
int lbl_8038228C[5];
int lbl_803822A0[5];
s8* gMapBlockLayerTables[MAP_BLOCK_LAYER_COUNT];

MapRomListPage* gLoadedRomListPages[ROM_LIST_PAGE_COUNT];
WarpVec lbl_80386648[0x29];

#include "main/dll/player_api.h"
#include "main/frame_timing.h"
#include "main/hud_visibility_api.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/lightmap_api.h"
#include "main/lightmap_lifecycle_api.h"
#include "main/lightmap_render_control_api.h"
#include "main/lightmap_render_queue_api.h"
#include "main/lightmap_text_color_api.h"
#include "main/modellight_api.h"
#include "main/objprint_render_api.h"
#include "main/vecmath.h"
#include "dolphin/gx/GXLighting.h"
#include "dolphin/gx/GXManage.h"
#include "main/sky_state.h"
#include "main/newshadows.h"
#include "main/newshadows_shadow_api.h"
#include "main/rcp_dolphin.h"
#include "main/dll/dll_0000_gameui.h"
#include "main/dll/dll_0031_minimap.h"
#include "main/dll/dll_00E2_staff_api.h"
#include "main/sky.h"
#include "track/intersect_render_setup_api.h"
#include "main/dll/cloudaction.h"
#include "main/trig.h"
#include "main/tex_dolphin.h"
#include "main/acosf_api.h"
#include "dolphin/gx/GXGeometry.h"
#include "dolphin/gx/GXTransform.h"

u8 colorFilterColor[4] = {0xFF, 0x70, 0x40, 0};
u8 colorScale = 0xFF;
extern f32 widescreenAspect_803DEC1C;
extern f32 lbl_803DB670;

void sceneDraw(void);
void sceneDrawTransparentPolys(void);

typedef struct
{
    f32 lo;
    f32 hi;
} F32Pair;

typedef struct EnvironmentUpdateInterface
{
    void (*create)(void);
    void (*destroy)(void);
    void (*update)(void);
} EnvironmentUpdateInterface;

extern u32 renderFlags;
/* Global renderFlags bits (decoded by the accessor fns below: shouldDrawShadows,
 * shouldDrawClouds, getDrawDistanceFlag, isOvercast, setPendingMapLoad,
 * setStarsHidden). */
#define RENDERFLAG_WIDESCREEN      0x8
#define RENDERFLAG_DRAW_CLOUDS     0x10
#define RENDERFLAG_DRAW_SHADOWS    0x80
#define RENDERFLAG_PENDING_MAP_LOAD 0x1000
#define RENDERFLAG_DRAW_DISTANCE   0x10000
#define RENDERFLAG_OVERCAST        0x40000
#define RENDERFLAG_HIDE_STARS      0x80000

extern f32 lbl_803DEBF8;
extern const f32 lbl_803DEBFC;
extern f32 lbl_803DEBCC;
extern f32 lbl_803DEBDC;
extern f32 lbl_803DEC00;
extern f32 gLightmapDegToBamScale;
extern F32Pair lbl_803DEC08;
extern f32 lbl_803DEC0C;
extern FrustumPlane gViewFrustumPlanes[];

extern u8 lbl_803DCE98; /* count of allocated blocks */
extern f32 lbl_803DEC18;
extern u32 lbl_803DCE34;
extern f32 lbl_803DEC10;
extern u16 lbl_803DCEAC;
extern u8 lbl_803DCE06;
extern s32 heatEffectIntensity;
extern u8 gLightmapScreenImageEnabled;
extern s8* gMapLayerCellStates;
extern int gMapCurRomListSlot;
extern f32 lbl_803DCE58;
extern f32 lbl_803DCE54;
typedef struct
{
    u32 a;
    u32 b;
    u32 key;
    u32 d;
} LightSortEntry;
extern s16* gMapBlockIds;
extern u8* gMapBlockRefCounts;
extern u8* lbl_803DCE78;
extern int lbl_803DCE7C;
extern void* lbl_803DCE80;
extern u16* lbl_803DCE84;
extern s16 lbl_803DCE90;
extern s16 lbl_803DCEBA;
extern s16 lbl_803DCEB8;
extern EnvironmentUpdateInterface** lbl_803DCAB0;
extern int gHeatEffectFadeDirection;

typedef union
{
    u8 u8;
    u16 u16;
    u32 u32;
    s8 s8;
    s16 s16;
    s32 s32;
    f32 f32;
} PPCWGPipe;

volatile PPCWGPipe GXWGFifo : (0xCC008000);

void renderShadowType3(u8* obj, u32 b, s32 offset);
static inline void GXPosition3s16(const s16 x, const s16 y, const s16 z)
{
    GXWGFifo.s16 = x;
    GXWGFifo.s16 = y;
    GXWGFifo.s16 = z;
}
static inline void GXColor4u8(const u8 r, const u8 g, const u8 b, const u8 a)
{
    GXWGFifo.u8 = r;
    GXWGFifo.u8 = g;
    GXWGFifo.u8 = b;
    GXWGFifo.u8 = a;
}
static inline void GXTexCoord2s16(const s16 s, const s16 t)
{
    GXWGFifo.s16 = s;
    GXWGFifo.s16 = t;
}
static inline void GXPosition1x8(const u8 x) { GXWGFifo.u8 = x; }

void updateVisibleGeometry(void)
{
    CameraViewSlot* cam;
    int n;
    int i;
    f32 tt, ff, ss;
    f32 scale;
    f32 xx, yy, zz;
    f32 ratio, ratio2;
    u16 fov;
    f32 ox, oy, oz;
    f32 dd;
    f32* pw;
    MatrixTransform st;
    f32 m[17];

    cam = Camera_GetCurrentViewSlot();
    if ((renderFlags & RENDERFLAG_WIDESCREEN) != 0 || (renderFlags & RENDERFLAG_DRAW_DISTANCE) != 0)
    {
        scale = Camera_GetFovY() / lbl_803DEBF8;
    }
    else
    {
        scale = Camera_GetFovY();
        scale *= lbl_803DEBFC;
    }
    xx = cam->worldX - playerMapOffsetX;
    yy = cam->worldY;
    zz = cam->worldZ - playerMapOffsetZ;
    st.x = lbl_803DEBCC;
    st.y = lbl_803DEBCC;
    st.z = lbl_803DEBCC;
    st.scale = lbl_803DEBDC;
    st.rotX = 0x8000 - cam->worldYaw;
    st.rotY = -cam->worldPitch;
    st.rotZ = cam->worldRoll;
    setMatrixFromObjectPos(m, &st);
    Matrix_TransformPoint(m, lbl_803DEBCC, *(f32*)&lbl_803DEBCC, lbl_803DEC00, &ox, &oy, &oz);
    gViewFrustumPlanes[0].normalX = ox;
    gViewFrustumPlanes[n = 0].normalY = oy;
    gViewFrustumPlanes[n = 0].normalZ = oz;
    dd = -(zz * oz + (xx * ox + yy * oy));
    pw = &gViewFrustumPlanes[0].distance;
    i = 0;
    pw[i * 5] = dd;
    fov = (int)(gLightmapDegToBamScale * scale) & 0xffff;
    tt = fcos16HighPrecision(fov);
    ratio = fsin16HighPrecision(fov) / tt;
    ratio2 = ratio * ratio;
    ff = lbl_803DEC08.lo;
    tt = ff * ratio2;
    tt = fn_80292248(sqrtf(ff * tt + ratio2));
    ff = mathSinfHighPrecision(tt);
    ss = mathCosfHighPrecision(tt);
    Matrix_TransformPoint(m, ss, lbl_803DEBCC, -ff, &ox, &oy, &oz);
    gViewFrustumPlanes[n = 1].normalX = ox;
    gViewFrustumPlanes[n].normalY = oy;
    gViewFrustumPlanes[n].normalZ = oz;
    pw[n * 5] = -(zz * oz + (xx * ox + yy * oy));
    Matrix_TransformPoint(m, -ss, lbl_803DEBCC, -ff, &ox, &oy, &oz);
    gViewFrustumPlanes[n = 2].normalX = ox;
    gViewFrustumPlanes[n].normalY = oy;
    gViewFrustumPlanes[n].normalZ = oz;
    pw[n * 5] = -(zz * oz + (xx * ox + yy * oy));
    Matrix_TransformPoint(m, lbl_803DEBCC, -ss, -ff, &ox, &oy, &oz);
    gViewFrustumPlanes[n = 3].normalX = ox;
    gViewFrustumPlanes[n].normalY = oy;
    gViewFrustumPlanes[n].normalZ = oz;
    pw[n * 5] = -(zz * oz + (xx * ox + yy * oy));
    Matrix_TransformPoint(m, lbl_803DEBCC, ss, -ff, &ox, &oy, &oz);
    gViewFrustumPlanes[n = 4].normalX = ox;
    gViewFrustumPlanes[n].normalY = oy;
    gViewFrustumPlanes[n].normalZ = oz;
    pw[n * 5] = -(zz * oz + (xx * ox + yy * oy));
    frustumPlanes_updateAabbCornerIndices((FrustumPlane*)gViewFrustumPlanes, 5);
}

MapBlockData* mapGetBlock(int i)
{
    if (i < 0 || i >= lbl_803DCE98) return 0;
    return gMapBlocks[i];
}

extern s32 lbl_803DCE30;

s8* mapGetBlockIdx(int layer)
{
    return gMapBlockLayerTables[layer];
}

MapBlockData* mapGetBlockAtPos(int x, int y, int layer)
{
    s8* table = gMapBlockLayerTables[layer];
    s32 idx;
    if (x < 0 || y < 0 || x >= 0x10 || y >= 0x10) return 0;
    idx = table[x + (y << 4)];
    if (idx < 0 || idx >= lbl_803DCE98) return 0;
    return gMapBlocks[idx];
}

void* RomList_GetLoadedPages(void)
{
    return gLoadedRomListPages;
}

u32 gVisibleObjectSortKeys[0x400];
extern int gLightmapDeferredObjectCount;
extern s16 gVisibleObjectSortKeyCount;

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

extern int gMapBlockOriginX;
extern int gMapBlockOriginZ;

int coordsToMapCell(f32 x, f32 z)
{
    int ix = (int)(fastFloorf(x / gMapBlockWorldSize) - (f32)gMapBlockOriginX);
    int iz = (int)(fastFloorf(z / gMapBlockWorldSize) - (f32)gMapBlockOriginZ);
    if (ix < 0 || ix >= 16) return -1;
    if (iz < 0 || iz >= 16) return -1;
    return *(s16*)((char*)lbl_803822A0[0] + (ix + iz * 16) * 12);
}

void mapGetBlockOriginForPos(f32 x, f32 y, f32 z, f32* outX, f32* outZ)
{
    s32 ix, iz;
    f32 s;
    ix = fastFloorf(x / gMapBlockWorldSize);
    iz = fastFloorf(z / gMapBlockWorldSize);
    s = gMapBlockWorldSize;
    *outX = s * ix;
    *outZ = s * iz;
}

#define MAP_BLOCK_LAYER_COUNT 5

int isInBounds(f32 x, f32 z)
{
    int ix = (int)(fastFloorf(x / gMapBlockWorldSize) - (f32)gMapBlockOriginX);
    int iz = (int)(fastFloorf(z / gMapBlockWorldSize) - (f32)gMapBlockOriginZ);
    int linear;
    s8** p;
    if (ix < 0 || ix >= 16) return -1;
    if (iz < 0 || iz >= 16) return -1;
    linear = ix + (iz << 4);
    {
        int i;
        p = gMapBlockLayerTables;
        for (i = 0; i < MAP_BLOCK_LAYER_COUNT; i++)
        {
            if ((*p)[linear] > -1) return 1;
            p++;
        }
    }
    return 0;
}


int objPosToMapBlockIdx(f32 x, f32 y, f32 z)
{
    s8** tp;
    int ix = (int)(fastFloorf(x / gMapBlockWorldSize) - (f32)gMapBlockOriginX);
    int iz = (int)(fastFloorf(z / gMapBlockWorldSize) - (f32)gMapBlockOriginZ);
    int i;
    if (ix < 0 || ix >= 16) return -1;
    if (iz < 0 || iz >= 16) return -1;
    ix = ix + (iz << 4);
    for (tp = gMapBlockLayerTables, i = 0; i < MAP_BLOCK_LAYER_COUNT; tp++, i++)
    {
        s8* table = *tp;
        int idx = table[ix];
        if (idx > -1)
        {
            MapBlockData* block = gMapBlocks[idx];
            if (y > (f32)(block->minY - 50) &&
                y < (f32)(block->maxY + 50))
            {
                return table[ix];
            }
        }
    }
    return -1;
}

extern void* lbl_803DCEA0;

int* mapRomListFindItem(int needle, int* out_idx, int* out_outer, int* out_type, int* out_lastpage)
{
    MapRomListPage* page;
    MapRomListPage** pp;
    int inner_idx;
    int outer;
    int total_offset;
    int* p;
    u16 limit;
    int sz;

    for (outer = 0, pp = gLoadedRomListPages; outer < ROM_LIST_PAGE_COUNT; pp++, outer++)
    {
        page = *pp;
        if (page == NULL) continue;

        lbl_803DCEA0 = page;
        p = (int*)page->objects;
        inner_idx = 0;
        total_offset = 0;
        limit = page->objectDataSize;

        while (total_offset < limit)
        {
            if (*(u32*)((char*)p + 0x14) == (u32)needle)
            {
                if (out_idx != NULL) *out_idx = inner_idx;
                if (out_outer != NULL) *out_outer = outer;
                if (out_type != NULL)
                {
                    *out_type = (int)*(s8*)((char*)lbl_803DCEA0 + 0x19);
                }
                if (out_lastpage != NULL)
                {
                    *out_lastpage = (outer >= 0x50) ? 1 : 0;
                }
                return p;
            }
            sz = (int)*(u8*)((char*)p + 0x2) << 2;
            total_offset += sz;
            p = (int*)((char*)p + sz);
            inner_idx++;
        }
    }
    return NULL;
}

void sortVisibleObjectKeysDescending(u32* arr, int n)
{
    int i, j;
    int gap = 1;
    u32 tmp;
    while (gap <= n / 9)
        gap = gap * 3 + 1;
    while (gap > 0)
    {
        for (i = gap + 1; i <= n; i++)
        {
            tmp = arr[i - 1];
            j = i;
            while (j > gap && arr[j - gap - 1] < tmp)
            {
                arr[j - 1] = arr[j - gap - 1];
                j -= gap;
            }
            arr[j - 1] = tmp;
        }
        gap /= 3;
    }
}

void sortVisibleObjectKeysDescending(u32* arr, int n);
void getVisibleObjects(s8* opacity)
{
    int part;
    int* objects;
    int* p;
    u8* o;
    int i;
    u32 key;
    int depthInt;
    s8* cur;
    u8* sub;
    u8* att;
    int j;
    u8* interactState;
    int* model;
    ObjModelInstance* modelDef;
    u32 tf;
    u32 mode;
    s16 t;
    int sortDepth;
    int count;
    f32 a, b;
    f32 depth;

    maybeHudFn_8006c91c();
    objects = ObjList_GetObjects((int*)0, 0);
    part = ObjList_PartitionForRender(&count);
    i = 0;
    p = objects;
    cur = opacity;
    for (; i < count; i++, cur++)
    {
        o = (u8*)*p;

        ((GameObject*)o)->objectFlags &= ~OBJECT_OBJFLAG_RENDERED;
        j = 0;
        sub = o;
        for (; j < ((GameObject*)o)->childCount; j++)
        {
            att = *(u8**)(sub + 0xc8);
            if (att != NULL)
            {
                ((GameObject*)att)->objectFlags &= ~OBJECT_OBJFLAG_RENDERED;
            }
            sub += 4;
        }
        if (i >= part)
        {
            *cur = objUpdateOpacity((GameObject*)o);
            if (*cur != 0 || (((ObjAnimComponent*)o)->modelInstance->flags & 0x200000) != 0)
            {
                if ((((ObjAnimComponent*)o)->modelInstance->flags & 0x80000) != 0)
                {
                    *(f32*)&((GameObject*)o)->anim.targetObj =
                        (f32)(((GameObject*)o)->anim.modelInstance->fixedSortDepth * 100);
                    depthInt = (int)*(f32*)&((GameObject*)o)->anim.targetObj;
                }
                else
                {
                    if (((GameObject*)o)->anim.parent != NULL)
                    {
                        Camera_ProjectWorldPoint(((GameObject*)o)->anim.worldPosX, ((GameObject*)o)->anim.worldPosY,
                                                 ((GameObject*)o)->anim.worldPosZ, &a, &b, &depth,
                                                 (f32*)&((GameObject*)o)->anim.targetObj);
                    }
                    else
                    {
                        Camera_ProjectWorldPoint(((GameObject*)o)->anim.localPosX - playerMapOffsetX,
                                                 ((GameObject*)o)->anim.localPosY,
                                                 ((GameObject*)o)->anim.localPosZ - playerMapOffsetZ, &a, &b,
                                                 &depth, (f32*)&((GameObject*)o)->anim.targetObj);
                    }
                    depthInt = (int)(lbl_803DEC0C * (lbl_803DEBDC + depth));
                }
                if ((((GameObject*)o)->anim.flags & OBJANIM_FLAG_HIDDEN) == 0 &&
                    ((GameObject*)o)->anim.modelState != NULL &&
                    (((GameObject*)o)->anim.modelState->flags & OBJ_MODEL_STATE_SHADOW_VISIBLE) != 0)
                {
                    t = ((ObjAnimComponent*)o)->modelInstance->shadowType;
                    if (t == 2 || t == 1)
                    {
                        queueObjectShadow((GameObject*)o);
                    }
                    else if (t == 4)
                    {
                        renderObjectShadowTexture((GameObject*)o);
                    }
                }
                if (gVisibleObjectSortKeyCount < 1000)
                {
                    key = 0;
                    model = (int*)Obj_GetActiveModel((GameObject*)o);
                    if (((GameObject*)o)->anim.renderAlpha == 0xff && (((GameObject*)o)->anim.flags & 0x80) == 0 &&
                        ((tf = ((ObjAnimComponent*)o)->modelInstance->flags) & 0x40000) == 0 &&
                        *(void**)(model + 0x16) == NULL)
                    {
                        key |= 0x80000000;
                        sortDepth = 1000 - (depthInt & 0xffff);
                        if ((tf & 0x800000) != 0 && (((GameObject*)o)->colorFadeFlags & OBJ_COLOR_FADE_FLAG_ACTIVE) == 0)
                        {
                            key |= 0x40000000LL;
                            key |= (((GameObject*)o)->anim.seqId & 0x3ff) << 20;
                        }
                        gVisibleObjectSortKeys[gVisibleObjectSortKeyCount] =
                            (i & 0x3ff) | (((sortDepth & 0x3ff) << 10) | key);
                        gVisibleObjectSortKeyCount++;
                        if ((((ObjAnimComponent*)o)->modelInstance->renderFlags & 0x20) != 0 &&
                            (((GameObject*)o)->objectFlags & 0x400) == 0 &&
                            (((GameObject*)o)->anim.flags & OBJANIM_FLAG_HIDDEN) == 0)
                        {
                            renderShadowType3(o, 7, 0x50);
                            lbl_8037E0C0[lbl_803DCE30 * 4 + 3] = 1;
                            lbl_803DCE30++;
                        }
                    }
                    else
                    {
                        if ((((ObjAnimComponent*)o)->modelInstance->flags & OBJDEF_FLAG_DEFERRED_RENDER) != 0 ||
                            (((ObjAnimComponent*)o)->modelInstance->renderFlags & OBJDEF_RENDERFLAG_DEFERRED_RENDER) != 0)
                        {
                            mode = 0x1f;
                        }
                        else
                        {
                            mode = 7;
                        }
                        renderShadowType3(o, mode, 0);
                        lbl_8037E0C0[lbl_803DCE30 * 4 + 3] = 0;
                        lbl_803DCE30++;
                        if ((((ObjAnimComponent*)o)->modelInstance->renderFlags & 0x20) != 0 &&
                            (((GameObject*)o)->anim.flags & OBJANIM_FLAG_HIDDEN) == 0)
                        {
                            renderShadowType3(o, 7, 0x50);
                            lbl_8037E0C0[lbl_803DCE30 * 4 + 3] = 1;
                            lbl_803DCE30++;
                        }
                    }
                }
            }
            else
            {
                interactState = (void*)((GameObject*)o)->anim.hitReactState;
                if (interactState != NULL && (interactState[0x62] & 0x30) != 0)
                {
                    interactState[0xaf] = 2;
                }
            }
        }
        p++;
    }
    if (gVisibleObjectSortKeyCount > 1)
    {
        sortVisibleObjectKeysDescending(gVisibleObjectSortKeys, gVisibleObjectSortKeyCount);
    }
    renderShadows(0, 0, 0);
}

void renderObjects(s8* opacity)
{
    u32* kp;
    int i;
    u32 flags;
    int idx;
    u8* obj;
    u8* state;
    int* p;
    int slot;
    int* objects;
    LightmapDrawQueue* qbase;
    LightmapQEnt* q;

    qbase = (LightmapDrawQueue*)lbl_8037E0C0;
    q = (LightmapQEnt*)lbl_8037E0C0;
    objects = ObjList_GetObjects((int*)0, 0);
    for (i = 1, kp = (u32*)((u8*)qbase + 0x8818) + 1; i < gVisibleObjectSortKeyCount; kp++, i++)
    {
        idx = *kp & 0x3ff;
        obj = (u8*)objects[idx];
        flags = ((GameObject*)obj)->anim.modelInstance->flags;
        if ((flags & OBJDEF_FLAG_DEFERRED_RENDER) != 0 || ((((GameObject*)obj)->anim.modelInstance->renderFlags & OBJDEF_RENDERFLAG_DEFERRED_RENDER) != 0))
        {
            if (opacity[idx] != 0 && gLightmapDeferredObjectCount < 0x14)
            {
                slot = gLightmapDeferredObjectCount;
                gLightmapDeferredObjectCount = slot + 1;
                *(u32*)((u8*)qbase->deferred + slot * 4) = (u32)obj;
            }
        }
        else
        {
            if ((flags & 0x800000) == 0)
            {
                (*gModgfxInterface)->renderEffects(NULL, 0, 0, 1, obj);
            }
            objRender(0, 0, 0, 0, (GameObject*)obj, 1);
            p = (int*)((GameObject*)obj)->anim.modelState;
            if (p != NULL && ((GameObject*)obj)->anim.modelState->shadowCastSlot != NULL)
            {
                renderShadowType3(obj, 0x13, 0);
                *(u32*)((u8*)&q->d + lbl_803DCE30 * 16) = 2;
                lbl_803DCE30++;
            }
            else if (((GameObject*)obj)->anim.modelInstance->shadowType == OBJ_SHADOW_TYPE_CRASH && (((GameObject*)obj)->anim.flags
                & OBJANIM_FLAG_HIDDEN) == 0 && (((GameObject*)obj)->anim.modelState->flags &
                OBJ_MODEL_STATE_SHADOW_VISIBLE))
            {
                renderShadowType3(obj, 0x13, 0);
                *(u32*)((u8*)&q->d + lbl_803DCE30 * 16) = 3;
                lbl_803DCE30++;
            }
        }
    }
}
static inline void fillBoxRows(u8* map, int* box)
{
    int y, x0;
    int xs, xe;
    u8* p;
    for (y = box[2]; y <= box[3]; y++)
    {
        xs = box[0];
        p = map + (y + 7) * 0x10 + xs;
        xe = box[1];
        for (x0 = xs; x0 <= xe; x0++)
        {
            p[7] = 1;
            p++;
        }
    }
}

void renderSceneGeometry(u8 renderType, s8* order)
{
    u8 map[256];
    int box0[4];
    int box1[4];
    int box2[4];
    int box3[4];
    s8** layerTablePtr;
    int* layerFlagPtr;
    int idx;
    int k;
    int row, col;
    int oi, ii;
    int layer;
    MapBlockData* block;
    s8* table;
    f32 worldSize;
    f32 rowF, colF;
    int cell;

    layer = 4;
    layerTablePtr = &gMapBlockLayerTables[4];
    layerFlagPtr = &lbl_8038228C[4];
    worldSize = gMapBlockWorldSize;
    do
    {
        table = *layerTablePtr;
        gMapLayerCellStates = (s8*)*layerFlagPtr;
        mapFn_80057d24(gMapBlockOriginX + 7, gMapBlockOriginZ + 7, box0, box1, box2, box3, layer, 1,
                       gMapCurRomListSlot);
        for (k = 0; k < ARRAY_COUNT(map); k++)
        {
            map[k] = 0;
        }
        fillBoxRows(map, box0);
        fillBoxRows(map, box1);
        fillBoxRows(map, box2);
        fillBoxRows(map, box3);
        for (oi = 0; oi < 16; oi++)
        {
            row = order[oi];
            ii = 0;
            rowF = worldSize * (f32)row;
            for (; ii < 16; ii++)
            {
                col = order[ii];
                cell = row + col * 0x10;
                idx = table[cell];
                if (idx < 0)
                {
                    block = NULL;
                }
                else
                {
                    block = gMapBlocks[idx];
                    block->flags4 ^= 1;
                    if (map[cell] == 0)
                    {
                        continue;
                    }
                }
                if (idx > -1 && mapRectFn_8005a728(row, col, block) != 0)
                {
                    lbl_803DCE58 = rowF;
                    colF = gMapBlockWorldSize * (f32)col;
                    lbl_803DCE54 = colF;
                    PSMTXTrans((MtxPtr)block->transform, rowF, (f32)block->collisionYOffset, colF);
                    renderMapBlock(block, renderType);
                }
            }
        }
        layerTablePtr--;
        layerFlagPtr--;
        layer--;
    }
    while (layer >= 0);
}
extern u8 bEnableMotionBlur;
extern f32 lbl_803DB62C;

extern u8 bEnableBlurFilter;
extern f32 lbl_803DCE50;
extern f32 lbl_803DCE4C;
extern f32 blurFilterArea;
extern u8 bBlurFilterUseArea;
extern u8 bBiggerBlurFilter;
extern u8 bEnableDistortionFilter;
extern f32 distortionFilterAngle1;
extern f32 distortionFilterAngle2;
extern u8 distortionFilterColor[3];
extern u8 bEnableMonochromeFilter;
extern u8 bEnableSpiritVision;
extern u8 bEnableViewFinderHud;
extern f32 lbl_803DEC14;
extern s32 bEnableColorFilter;

void sceneDraw(void)
{
    char* q;
    int i;
    u8* cursor;
    GameObject* player;
    u8 flag;
    int t;
    GXColor c;
    f32 skyA;
    f32 skyB;
    s8 buf[616];

    q = (char*)lbl_8037E0C0;
    lbl_803DCE34 = (u32)cloudGetLayerTextureSize(&skyA, &skyB);
    if (lbl_803DCE34 != 0)
    {
        *(f32*)(q + 0x3f48) = lbl_803DEC10;
        *(f32*)(q + 0x3f4c) = lbl_803DEBCC;
        *(f32*)(q + 0x3f50) = lbl_803DEBCC;
        *(f32*)(q + 0x3f54) = lbl_803DEC10 * playerMapOffsetX + skyA;
        *(f32*)(q + 0x3f58) = lbl_803DEBCC;
        *(f32*)(q + 0x3f5c) = lbl_803DEBCC;
        *(f32*)(q + 0x3f60) = lbl_803DEC10;
        *(f32*)(q + 0x3f64) = lbl_803DEC10 * playerMapOffsetZ + skyB;
        *(f32*)(q + 0x3f68) = lbl_803DEBCC;
        *(f32*)(q + 0x3f6c) = lbl_803DEBCC;
        *(f32*)(q + 0x3f70) = lbl_803DEBCC;
        *(f32*)(q + 0x3f74) = lbl_803DEBDC;
        PSMTXConcat((MtxPtr)(q + 0x3f48), (MtxPtr)Camera_GetInverseViewMatrix(),
                    (MtxPtr)(q + 0x3f48));
    }
    mapDebugRender((int*)(q + 0x4164));
    fn_80062894();
    shadowVolumeBeginFrame();
    gVisibleObjectSortKeyCount = 1;
    lbl_803DCEAC = 0;
    lbl_803DCE06 = 0;
    drawReflectionTexture();
    lbl_803DCE30 = 0;
    getVisibleObjects(buf);
    Rcp_UpdateDistortionTextures();
    perspectiveFn_80129db4();
    GXPixModeSync();
    Camera_UpdateProjection(NULL, 0);
    Camera_UpdateViewMatrices();
    Camera_RebuildProjectionMatrix();
    t = 0;
    if ((renderFlags & 0x40) != 0 && (renderFlags & RENDERFLAG_HIDE_STARS) == 0)
    {
        t = 1;
    }
    flag = t;
    if ((renderFlags & RENDERFLAG_OVERCAST) != 0)
    {
        (*gSkyInterface)->renderTimeOfDayBackdrop(0, 0);
        if (flag != 0)
        {
            drawSkyStars();
        }
        (*gSkyInterface)->render(0, 0, 0, 0, flag);
        if ((renderFlags & RENDERFLAG_DRAW_CLOUDS) != 0)
        {
            (*gCloudActionInterface)->renderClouds(0, 0, 0, 0);
        }
    }
    else
    {
        (*gSkyInterface)->render(0, 0, 0, 0, flag);
        (*gCloudActionInterface)->renderClouds(0, 0, 0, 0);
        drawSkyStars();
    }
    if (gLightmapScreenImageEnabled != 0)
    {
        screenImageDraw(gLightmapScreenImageEnabled);
    }
    lightningRenderActive();
    (*gSky2Interface)->applyFogColor(0);
    gLightmapDeferredObjectCount = 0;
    getAmbientColor(0, (u8*)&c, (u8*)&c + 1, (u8*)&c + 2);
    GXSetChanCtrl(GX_COLOR0, GX_TRUE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_ALPHA0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanAmbColor(GX_COLOR0, c);
    GXSetNumChans(1);
    renderSceneGeometry(0, (s8*)lbl_8030E65C);
    renderResetFn_8003fc60();
    renderObjects(buf);
    if (CameraShake_IsActive() != 0 || (int)bEnableMotionBlur != 0)
    {
        renderMotionBlur(lbl_803DB62C);
    }
    if (getHudHiddenFrameCount() == 0)
    {
        updateReflectionTextures();
    }
    if (bEnableBlurFilter != 0)
    {
        doBlurFilter(lbl_803DCE50, lbl_803DCE4C, blurFilterArea, bBlurFilterUseArea,
                     bBiggerBlurFilter);
    }
    if (heatEffectIntensity != 0)
    {
        doHeatEffect(heatEffectIntensity & 0xff);
    }
    i = 0;
    cursor = (u8*)(q + 0x4114);
    for (; i < gLightmapDeferredObjectCount; i++)
    {
        (*gModgfxInterface)->renderEffects(NULL, 0, 0, 1, (void*)*(u32*)cursor);
        objRender(0, 0, 0, 0, (GameObject*)*(u32*)cursor, 1);
        cursor += 4;
    }
    renderParticles();
    renderSceneGeometry(1, (s8*)&lbl_8030E66C);
    renderSceneGeometry(2, (s8*)&lbl_8030E66C);
    if (lbl_803DCE30 == 1000)
    {
        sceneDrawTransparentPolys();
        lbl_803DCE30 = 0;
    }
    *(u32*)(((int)q + 8) + lbl_803DCE30 * 16) = 0x78000000;
    *(u32*)(((int)q + 12) + lbl_803DCE30 * 16) = 8;
    lbl_803DCE30 = lbl_803DCE30 + 1;
    if (lbl_803DCE30 == 1000)
    {
        sceneDrawTransparentPolys();
        lbl_803DCE30 = 0;
    }
    *(u32*)(((int)q + 8) + lbl_803DCE30 * 16) = 0x50000000;
    *(u32*)(((int)q + 12) + lbl_803DCE30 * 16) = 9;
    lbl_803DCE30 = lbl_803DCE30 + 1;
    sceneDrawTransparentPolys();
    (*gModgfxInterface)->markSourceFrameUpdated(buf);
    (*gModgfxInterface)->renderEffects(NULL, 0, 0, 0, NULL);
    player = Obj_GetPlayerObject();
    if (player != NULL)
    {
        i = 0;
        cursor = (u8*)player;
        for (; i < ((GameObject*)player)->childCount; i++)
        {
            u8* m = *(u8**)(cursor + 200);
            if (((GameObject*)m)->anim.classId == 45)
            {
                (*(void (***)(void))*(int*)(m + 0x68))[11]();
            }
            cursor += 4;
        }
    }
    quakeSpellTextureFn_8016dbf4();
    (*gNewCloudsInterface)->renderSnowClouds(0);
    if (bEnableDistortionFilter != 0)
    {
        updateReflectionTextures();
        doDistortionFilter((f32*)(q + 0x4108), distortionFilterAngle2,
                           distortionFilterColor, distortionFilterAngle1);
    }
    renderGlows();
    (*gCameraInterface)->minimapShowHelpTextForTarget(0, 0, 0, 0);
    if (bEnableMonochromeFilter != 0)
    {
        doColorFilter(colorFilterColor);
    }
    else if (bEnableSpiritVision != 0)
    {
        doSpiritVisionFilter();
    }
    if (bEnableViewFinderHud != 0)
    {
        drawViewFinderAperture(lbl_803DEC14, lbl_803DEC18, 0x40, 0);
    }
    if (bEnableColorFilter == 1)
    {
        doColorFilter(colorFilterColor);
    }
    shadowVolumesSetDirty(0);
}

extern s8 curMapType;

void sceneRender(int wpad0, int wpad1, int wpad2, int wpad3, int wpad4, int wpad5)
{
    renderFlags |= 0x21;
    if (curMapType == MAPTYPE_SUBMAP || curMapType == MAPTYPE_SUBMAP_UNUSED)
    {
        renderFlags &= ~1LL;
    }
    Camera_UpdateProjection(NULL, 0);
    updateVisibleGeometry();
    playerVecFn_8005a9b0();
    Camera_EnableViewYOffset();
    Camera_UpdateViewMatrices();
    Camera_RebuildProjectionMatrix();
    updateLights();
    lbl_803DCEA8 = (void*)Camera_GetCurrentViewSlot();
    sceneDraw();
    screenFn_8000e944(NULL);
    renderFlags &= ~2LL;
}

void doNothing_beforeTitleScreen(void)
{
}
void updateEnvironment(int mode)
{
    if (mode == 0)
    {
        MapTextureOverride* textureOverride;
        MapTextureScroll* textureScroll;
        Texture* tex;
        int i;
        f32 x;
        f32 deltaY;
        f32 deltaX;
        f32 deltaTime;

        envFxFn_80088884();
        (*gCloudActionInterface)->scrollTexture();
        (*gSky2Interface)->run();
        (*gSkyInterface)->updateTimeOfDay();
        (*gNewCloudsInterface)->run();

        for (i = 0; i < 80; i++)
        {
            textureOverride = &lbl_803DCE6C[i];
            if (textureOverride->refCount != 0 && (tex = textureOverride->texture) != NULL &&
                tex->animationFrameCount != 0x100 && tex->animationFrameStep != 0)
            {
                textureUpdateAnimationFrame(tex, &textureOverride->flags, &textureOverride->frame);
            }
        }

        for (i = 0; i < 58; i++)
        {
            textureScroll = &lbl_803DCE68[i];
            if (textureScroll->refCount != 0)
            {
                deltaY = textureScroll->yStep * (deltaTime = timeDelta);
                x = textureScroll->offsetX;
                deltaX = textureScroll->xStep * deltaTime;
                textureScroll->offsetX = x + deltaX;
                textureScroll->offsetY = textureScroll->offsetY + deltaY;
            }
        }

        loadNextMap();
        if (lbl_803DCAB0 != NULL)
        {
            (*lbl_803DCAB0)->update();
        }
        gMinimapInterface->vtable->frameStart();

        if (gHeatEffectFadeDirection != 0)
        {
            heatEffectIntensity += gHeatEffectFadeDirection;
            if (heatEffectIntensity < 0)
            {
                heatEffectIntensity = 0;
                gHeatEffectFadeDirection = 0;
            }
            else if (heatEffectIntensity > 255)
            {
                heatEffectIntensity = 255;
                gHeatEffectFadeDirection = 0;
            }
        }
    }
}
void initMapBlocks(void);

void gameFlagFn_8005cd24(int v);

int getDrawDistanceFlag_8005cd48(void);

extern f32 widescreenAspect_803DEC1C;
extern f32 lbl_803DB670;

int setWidescreen(u8 v);
int isWidescreen(void);
u32 shouldDrawShadows(void);
int shouldDrawClouds(void);

void titleScreenFn_8005cdd4(int v);

void setDrawLights(int v);

void gameFlagFn_8005ce6c(int v);

u8 isOvercast(void);

void setIsOvercast(int v);

void setStarsHidden(int v);

void setDrawCloudsAndLights(int v);

void setPendingMapLoad(int v);

void drawFn_8005cf8c(const void* vertexBase, u8* triList, int triCount);


void setFogColorCallback(int unused, u8 red, u8 green, u8 blue, int wpad0);


void _textSetColor(void* context, int red, int green, int blue, int alpha);

void setTextColor(void* context, int a, int b, int c, int d);

void doNothing_8005D148(int arg0, int arg1);


void objDrawFn_8005da48(GameObject* obj);
void modelRenderFn_8005d4ec(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx);
void modelRenderFn_8005d69c(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx);
void modelRenderFn_8005d894(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx);
void lightmap_sortTransparentDrawQueue(void);

void getVisibleObjects(s8 * opacity);


void renderSceneGeometry(u8 renderType, s8* order);

void doNothing_8005D14C(int arg0, int arg1);
void renderShadowType3(u8* obj, u32 b, s32 offset);

void lightmap_sortTransparentDrawQueue(void);


typedef union
{
    double d;

    struct
    {
        u32 hi;
        u32 lo;
    } u;
} F64Cvt;




extern const f32 lbl_803DEC20;

asm void fn_8005D3B4(MapBlockBoundsRec* bounds, MapBlockData* block, s32 selector);


void sortVisibleObjectKeysDescending(u32* arr, int n);


void modelRenderFn_8005d4ec(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx);
void modelRenderFn_8005d69c(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx);
void modelRenderFn_8005d894(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx);


void objDrawFn_8005da48(GameObject* obj);

void sceneDrawTransparentPolys(void);


void lightmap_queueExternalRenderEntry(u32 a, u32 b, f32* p);


extern u8 colorFilterColor[4];
extern u8 colorScale;
extern f32 widescreenAspect_803DEC1C;
extern f32 lbl_803DB670;

void sceneDraw(void);
void sceneDrawTransparentPolys(void);



extern u32 renderFlags;
/* Global renderFlags bits (decoded by the accessor fns below: shouldDrawShadows,
 * shouldDrawClouds, getDrawDistanceFlag, isOvercast, setPendingMapLoad,
 * setStarsHidden). */
#define RENDERFLAG_WIDESCREEN      0x8
#define RENDERFLAG_DRAW_CLOUDS     0x10
#define RENDERFLAG_DRAW_SHADOWS    0x80
#define RENDERFLAG_PENDING_MAP_LOAD 0x1000
#define RENDERFLAG_DRAW_DISTANCE   0x10000
#define RENDERFLAG_OVERCAST        0x40000
#define RENDERFLAG_HIDE_STARS      0x80000

extern f32 lbl_803DEBF8;
extern const f32 lbl_803DEBFC;
extern f32 lbl_803DEBCC;
extern f32 lbl_803DEBDC;
extern f32 lbl_803DEC00;
extern f32 gLightmapDegToBamScale;
extern F32Pair lbl_803DEC08;
extern f32 lbl_803DEC0C;
extern FrustumPlane gViewFrustumPlanes[];

extern u8 lbl_803DCE98; /* count of allocated blocks */
extern f32 lbl_803DEC18;
extern u32 lbl_803DCE34;
extern f32 lbl_803DEC10;
extern u16 lbl_803DCEAC;
extern u8 lbl_803DCE06;
extern s32 heatEffectIntensity;
extern u8 gLightmapScreenImageEnabled;
extern s8* gMapLayerCellStates;
extern int gMapCurRomListSlot;
extern f32 lbl_803DCE58;
extern f32 lbl_803DCE54;
extern s16* gMapBlockIds;
extern u8* gMapBlockRefCounts;
extern u8* lbl_803DCE78;
extern int lbl_803DCE7C;
extern void* lbl_803DCE80;
extern u16* lbl_803DCE84;
extern s16 lbl_803DCE90;
extern s16 lbl_803DCEBA;
extern s16 lbl_803DCEB8;
extern EnvironmentUpdateInterface** lbl_803DCAB0;
extern int gHeatEffectFadeDirection;


void updateVisibleGeometry(void);

MapBlockData* mapGetBlock(int i);

extern s32 lbl_803DCE30;

s8* mapGetBlockIdx(int layer);

MapBlockData* mapGetBlockAtPos(int x, int y, int layer);

void* RomList_GetLoadedPages(void);

extern u32 gVisibleObjectSortKeys[0x400];
extern int gLightmapDeferredObjectCount;
extern s16 gVisibleObjectSortKeyCount;




extern int gMapBlockOriginX;
extern int gMapBlockOriginZ;

int coordsToMapCell(f32 x, f32 z);

void mapGetBlockOriginForPos(f32 x, f32 y, f32 z, f32* outX, f32* outZ);

#define MAP_BLOCK_LAYER_COUNT 5

int isInBounds(f32 x, f32 z);


int objPosToMapBlockIdx(f32 x, f32 y, f32 z);

extern void* lbl_803DCEA0;

int* mapRomListFindItem(int needle, int* out_idx, int* out_outer, int* out_type, int* out_lastpage);

void sortVisibleObjectKeysDescending(u32* arr, int n);

void sortVisibleObjectKeysDescending(u32* arr, int n);
void getVisibleObjects(s8* opacity);

void renderObjects(s8* opacity);


void renderSceneGeometry(u8 renderType, s8* order);
extern u8 bEnableMotionBlur;
extern f32 lbl_803DB62C;

extern u8 bEnableBlurFilter;
extern f32 lbl_803DCE50;
extern f32 lbl_803DCE4C;
extern f32 blurFilterArea;
extern u8 bBlurFilterUseArea;
extern u8 bBiggerBlurFilter;
extern u8 bEnableDistortionFilter;
extern f32 distortionFilterAngle1;
extern f32 distortionFilterAngle2;
extern u8 distortionFilterColor[3];
extern u8 bEnableMonochromeFilter;
extern u8 bEnableSpiritVision;
extern u8 bEnableViewFinderHud;
extern f32 lbl_803DEC14;
extern s32 bEnableColorFilter;

void sceneDraw(void);

extern s8 curMapType;

void sceneRender(int wpad0, int wpad1, int wpad2, int wpad3, int wpad4, int wpad5);

void doNothing_beforeTitleScreen(void);
void updateEnvironment(int mode);
void initMapBlocks(void)
{
    u8* mb = (u8*)lbl_8037E0C0;
    MapLayerBuffers* buffers = (MapLayerBuffers*)lbl_8037E0C0;
    u32 zero;
    u32* q;
    u16* p;
    void* tmp;
    int i;

    renderFlags = 0;
    gMapBlocks = mmAlloc(0x100, 5, 0);
    gMapBlockIds = mmAlloc(0x80, 5, 0);
    gMapBlockRefCounts = mmAlloc(0x40, 5, 0);
    lbl_803DCE78 = mmAlloc(0xd48, 5, 0);
    buffers->blockIndices[0] = mmAlloc(0x500, 5, 0);
    buffers->blockDescriptors[0] = mmAlloc(0x3c00, 5, 0);
    buffers->cellStates[0] = mmAlloc(0x500, 5, 0);

    for (i = 0; i < 16; i += 4)
    {
        *(u32*)(mb + 0x41f8 + i) = *(u32*)(mb + 0x41f4 + i) + 0x100;
        *(u32*)(mb + 0x41e4 + i) = *(u32*)(mb + 0x41e0 + i) + 0xc00;
        *(u32*)(mb + 0x41d0 + i) = *(u32*)(mb + 0x41cc + i) + 0x100;
    }

    loadAssetFileById((void**)&lbl_803DCE7C, MLDF_FILEID_MAPS_TAB);
    loadAssetFileById(&lbl_803DCE80, MLDF_FILEID_HITS_TAB);

    q = (u32*)((u8*)(mb + 0x10000) - 0x7c58);
    zero = 0;
    for (i = 0; i < 3; i++)
    {
        q[0] = zero;
        q[1] = zero;
        q[2] = zero;
        q[3] = zero;
        q[4] = zero;
        q[5] = zero;
        q[6] = zero;
        q[7] = zero;
        q[8] = zero;
        q[9] = zero;
        q[10] = zero;
        q[11] = zero;
        q[12] = zero;
        q[13] = zero;
        q[14] = zero;
        q[15] = zero;
        q[16] = zero;
        q[17] = zero;
        q[18] = zero;
        q[19] = zero;
        q[20] = zero;
        q[21] = zero;
        q[22] = zero;
        q[23] = zero;
        q[24] = zero;
        q[25] = zero;
        q[26] = zero;
        q[27] = zero;
        q[28] = zero;
        q[29] = zero;
        q[30] = zero;
        q[31] = zero;
        q[32] = zero;
        q[33] = zero;
        q[34] = zero;
        q[35] = zero;
        q[36] = zero;
        q[37] = zero;
        q[38] = zero;
        q[39] = zero;
        q += 40;
    }

    loadAssetFileById(&lbl_803DCE84, MLDF_FILEID_TRKBLK_TAB);

    lbl_803DCE90 = 0;
    p = lbl_803DCE84;
    while (*p != 0xffff)
    {
        p++;
        lbl_803DCE90++;
    }
    lbl_803DCE90--;
    lbl_803DCEBA = -1;
    lbl_803DCEB8 = -2;

    tmp = mmAlloc(0x500, 5, 0);
    lbl_803DCE6C = tmp;
    memset(tmp, 0, 0x500);

    tmp = mmAlloc(0x3a0, 5, 0);
    lbl_803DCE68 = tmp;
    memset(tmp, 0, 0x3a0);

    memset(mb + 0x8818, 0, 0xfa0);
    *(u32*)(mb + 0x8818) = -1;
}

void gameFlagFn_8005cd24(int v);

int getDrawDistanceFlag_8005cd48(void);

extern f32 widescreenAspect_803DEC1C;
extern f32 lbl_803DB670;

int setWidescreen(u8 v);
int isWidescreen(void);
u32 shouldDrawShadows(void);
int shouldDrawClouds(void);

void titleScreenFn_8005cdd4(int v);

void setDrawLights(int v);

void gameFlagFn_8005ce6c(int v);

u8 isOvercast(void);

void setIsOvercast(int v);

void setStarsHidden(int v);

void setDrawCloudsAndLights(int v);

void setPendingMapLoad(int v);

void drawFn_8005cf8c(const void* vertexBase, u8* triList, int triCount);


void setFogColorCallback(int unused, u8 red, u8 green, u8 blue, int wpad0);


void _textSetColor(void* context, int red, int green, int blue, int alpha);

void setTextColor(void* context, int a, int b, int c, int d);

void doNothing_8005D148(int arg0, int arg1);


void objDrawFn_8005da48(GameObject* obj);
void modelRenderFn_8005d4ec(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx);
void modelRenderFn_8005d69c(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx);
void modelRenderFn_8005d894(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx);
void lightmap_sortTransparentDrawQueue(void);

void getVisibleObjects(s8 * opacity);


void renderSceneGeometry(u8 renderType, s8* order);

void doNothing_8005D14C(int arg0, int arg1);
void renderShadowType3(u8* obj, u32 b, s32 offset);

void lightmap_sortTransparentDrawQueue(void);






extern const f32 lbl_803DEC20;

asm void fn_8005D3B4(MapBlockBoundsRec* bounds, MapBlockData* block, s32 selector);


void sortVisibleObjectKeysDescending(u32* arr, int n);


void modelRenderFn_8005d4ec(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx);
void modelRenderFn_8005d69c(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx);
void modelRenderFn_8005d894(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx);


void objDrawFn_8005da48(GameObject* obj);

void sceneDrawTransparentPolys(void);


void lightmap_queueExternalRenderEntry(u32 a, u32 b, f32* p);


extern u8 colorFilterColor[4];
extern u8 colorScale;
extern f32 widescreenAspect_803DEC1C;
extern f32 lbl_803DB670;

void sceneDraw(void);
void sceneDrawTransparentPolys(void);



extern u32 renderFlags;
/* Global renderFlags bits (decoded by the accessor fns below: shouldDrawShadows,
 * shouldDrawClouds, getDrawDistanceFlag, isOvercast, setPendingMapLoad,
 * setStarsHidden). */
#define RENDERFLAG_WIDESCREEN      0x8
#define RENDERFLAG_DRAW_CLOUDS     0x10
#define RENDERFLAG_DRAW_SHADOWS    0x80
#define RENDERFLAG_PENDING_MAP_LOAD 0x1000
#define RENDERFLAG_DRAW_DISTANCE   0x10000
#define RENDERFLAG_OVERCAST        0x40000
#define RENDERFLAG_HIDE_STARS      0x80000

extern f32 lbl_803DEBF8;
extern const f32 lbl_803DEBFC;
extern f32 lbl_803DEBCC;
extern f32 lbl_803DEBDC;
extern f32 lbl_803DEC00;
extern f32 gLightmapDegToBamScale;
extern F32Pair lbl_803DEC08;
extern f32 lbl_803DEC0C;
extern FrustumPlane gViewFrustumPlanes[];

extern u8 lbl_803DCE98; /* count of allocated blocks */
extern f32 lbl_803DEC18;
extern u32 lbl_803DCE34;
extern f32 lbl_803DEC10;
extern u16 lbl_803DCEAC;
extern u8 lbl_803DCE06;
extern s32 heatEffectIntensity;
extern u8 gLightmapScreenImageEnabled;
extern s8* gMapLayerCellStates;
extern int gMapCurRomListSlot;
extern f32 lbl_803DCE58;
extern f32 lbl_803DCE54;
extern s16* gMapBlockIds;
extern u8* gMapBlockRefCounts;
extern u8* lbl_803DCE78;
extern int lbl_803DCE7C;
extern void* lbl_803DCE80;
extern u16* lbl_803DCE84;
extern s16 lbl_803DCE90;
extern s16 lbl_803DCEBA;
extern s16 lbl_803DCEB8;
extern EnvironmentUpdateInterface** lbl_803DCAB0;
extern int gHeatEffectFadeDirection;


void updateVisibleGeometry(void);

MapBlockData* mapGetBlock(int i);

extern s32 lbl_803DCE30;

s8* mapGetBlockIdx(int layer);

MapBlockData* mapGetBlockAtPos(int x, int y, int layer);

void* RomList_GetLoadedPages(void);

extern u32 gVisibleObjectSortKeys[0x400];
extern int gLightmapDeferredObjectCount;
extern s16 gVisibleObjectSortKeyCount;

typedef struct LightmapDrawEntry
{
    union
    {
        u32 value;
        GameObject* object;
        MapBlockBoundsRec* bounds;
    } arg0;
    union
    {
        u32 value;
        MapBlockData* block;
    } arg1;
    u32 sortKey;
    s32 type;
} LightmapDrawEntry;

typedef union LightmapDrawItem
{
    GameObject* object;
    MapBlockData* block;
} LightmapDrawItem;



extern int gMapBlockOriginX;
extern int gMapBlockOriginZ;

int coordsToMapCell(f32 x, f32 z);

void mapGetBlockOriginForPos(f32 x, f32 y, f32 z, f32* outX, f32* outZ);

#define MAP_BLOCK_LAYER_COUNT 5

int isInBounds(f32 x, f32 z);


int objPosToMapBlockIdx(f32 x, f32 y, f32 z);

extern void* lbl_803DCEA0;

int* mapRomListFindItem(int needle, int* out_idx, int* out_outer, int* out_type, int* out_lastpage);

void sortVisibleObjectKeysDescending(u32* arr, int n);

void sortVisibleObjectKeysDescending(u32* arr, int n);
void getVisibleObjects(s8* opacity);

void renderObjects(s8* opacity);


void renderSceneGeometry(u8 renderType, s8* order);
extern u8 bEnableMotionBlur;
extern f32 lbl_803DB62C;

extern u8 bEnableBlurFilter;
extern f32 lbl_803DCE50;
extern f32 lbl_803DCE4C;
extern f32 blurFilterArea;
extern u8 bBlurFilterUseArea;
extern u8 bBiggerBlurFilter;
extern u8 bEnableDistortionFilter;
extern f32 distortionFilterAngle1;
extern f32 distortionFilterAngle2;
extern u8 distortionFilterColor[3];
extern u8 bEnableMonochromeFilter;
extern u8 bEnableSpiritVision;
extern u8 bEnableViewFinderHud;
extern f32 lbl_803DEC14;
extern s32 bEnableColorFilter;

void sceneDraw(void);

extern s8 curMapType;

void sceneRender(int wpad0, int wpad1, int wpad2, int wpad3, int wpad4, int wpad5);

void doNothing_beforeTitleScreen(void);
void updateEnvironment(int mode);
void initMapBlocks(void);

void gameFlagFn_8005cd24(int v)
{
    renderFlags = (v != 0) ? (renderFlags | 0x20000) : (renderFlags & ~0x20000);
}

int getDrawDistanceFlag_8005cd48(void) { return renderFlags & RENDERFLAG_DRAW_DISTANCE; }

extern f32 widescreenAspect_803DEC1C;
extern f32 lbl_803DB670;

int setWidescreen(u8 v)
{
    if (v != 0)
    {
        renderFlags |= RENDERFLAG_WIDESCREEN;
        Camera_SetAspectRatio(widescreenAspect_803DEC1C);
    }
    else
    {
        renderFlags &= ~(u64)RENDERFLAG_WIDESCREEN;
        Camera_SetAspectRatio(lbl_803DB670);
    }
    return 0;
}
int isWidescreen(void) { return renderFlags & RENDERFLAG_WIDESCREEN; }
u32 shouldDrawShadows(void) { return renderFlags & RENDERFLAG_DRAW_SHADOWS; }
int shouldDrawClouds(void) { return renderFlags & RENDERFLAG_DRAW_CLOUDS; }

void titleScreenFn_8005cdd4(int v)
{
    if (v != 0) renderFlags &= ~0x2000;
    else renderFlags |= 0x2000;
}

void setDrawLights(int v)
{
    void* env = saveGameGetEnvState();
    if (v != 0)
    {
        renderFlags |= 0x40;
        *(u8*)((char*)env + 0x40) |= 0x8;
    }
    else
    {
        renderFlags &= ~0x40LL;
        *(u8*)((char*)env + 0x40) &= ~0x8;
    }
}

void gameFlagFn_8005ce6c(int v)
{
    renderFlags = (v != 0) ? (renderFlags | 0x20) : (renderFlags & ~0x20);
}

u8 isOvercast(void)
{
    u32 v = renderFlags & RENDERFLAG_OVERCAST;
    u32 t = ((u32) - (s32)v | v) >> 31;
    return t;
}

void setIsOvercast(int v)
{
    renderFlags = (v != 0) ? (renderFlags | RENDERFLAG_OVERCAST) : (renderFlags & ~RENDERFLAG_OVERCAST);
}

void setStarsHidden(int v)
{
    renderFlags = (v != 0) ? (renderFlags | RENDERFLAG_HIDE_STARS) : (renderFlags & ~RENDERFLAG_HIDE_STARS);
}

void setDrawCloudsAndLights(int v)
{
    void* env = saveGameGetEnvState();
    if (v != 0)
    {
        renderFlags |= 0x50;
        *(u8*)((char*)env + 0x40) |= 0x9;
    }
    else
    {
        renderFlags &= ~0x50;
        *(u8*)((char*)env + 0x40) &= ~0x9;
    }
}

void setPendingMapLoad(int v)
{
    renderFlags = (v != 0) ? (renderFlags | RENDERFLAG_PENDING_MAP_LOAD) : (renderFlags & ~RENDERFLAG_PENDING_MAP_LOAD);
}

void drawFn_8005cf8c(const void* vertexBase, u8* triList, int triCount)
{
    const LightmapVertex* vertices = vertexBase;
    const LightmapVertex* vertex;
    int tri, vtx;

    /* Emit triCount triangles as GX_TRIANGLES; each vertex is 16 bytes:
       s16 pos[3] @0x0, u8 color[4] @0xc, s16 texcoord[2] @0x8. */
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXBegin(GX_TRIANGLES, GX_VTXFMT0, triCount * 3 & 0xffff);
    for (tri = 0; tri < triCount; tri++)
    {
        u8* list = triList;
        for (vtx = 0; vtx < 3; vtx++)
        {
            GXPosition1x8(0);
            vertex = &vertices[list[vtx + 1]];
            GXPosition3s16(vertex->x, vertex->y, vertex->z);
            vertex = &vertices[list[vtx + 1]];
            GXColor4u8(vertex->r, vertex->g, vertex->b, vertex->a);
            vertex = &vertices[list[vtx + 1]];
            GXTexCoord2s16(vertex->s, vertex->t);
        }
        triList = triList + 0x10;
    }
}


void setFogColorCallback(int unused, u8 red, u8 green, u8 blue, int wpad0)
{
    setFogColorRgb(red, green, blue);
}


void _textSetColor(void* context, int red, int green, int blue, int alpha)
{
    _gxSetTevColor1(red, green, blue, alpha);
}

void setTextColor(void* context, int a, int b, int c, int d)
{
    _gxSetTevColor2(a, b, c, d);
}

void doNothing_8005D148(int arg0, int arg1)
{
}


void objDrawFn_8005da48(GameObject* obj);
void modelRenderFn_8005d4ec(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx);
void modelRenderFn_8005d69c(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx);
void modelRenderFn_8005d894(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx);
void lightmap_sortTransparentDrawQueue(void);

void getVisibleObjects(s8 * opacity);


void renderSceneGeometry(u8 renderType, s8* order);

void doNothing_8005D14C(int arg0, int arg1)
{
}
void renderShadowType3(u8* obj, u32 b, s32 offset)
{
    f32 stk[3];
    s32 t;
    if (lbl_803DCE30 == 1000)
    {
        sceneDrawTransparentPolys();
        lbl_803DCE30 = 0;
    }
    if (((GameObject*)obj)->anim.parent != NULL)
    {
        stk[0] = ((GameObject*)obj)->anim.worldPosX;
        stk[1] = ((GameObject*)obj)->anim.worldPosY;
        stk[2] = ((GameObject*)obj)->anim.worldPosZ;
    }
    else
    {
        stk[0] = ((GameObject*)obj)->anim.worldPosX - playerMapOffsetX;
        stk[1] = ((GameObject*)obj)->anim.worldPosY;
        stk[2] = ((GameObject*)obj)->anim.worldPosZ - playerMapOffsetZ;
    }
    PSMTXMultVec((MtxPtr)Camera_GetViewMatrix(), (Vec*)stk, (Vec*)stk);
    t = (s32) - stk[2] + offset;
    t = t < 0 ? 0 : (t > 0x7ffffff ? 0x7ffffff : t);
    lbl_8037E0C0[lbl_803DCE30 * 4] = (u32)obj;
    lbl_8037E0C0[lbl_803DCE30 * 4 + 2] = t | ((b & 0xff) << 27);
}

void lightmap_sortTransparentDrawQueue(void)
{
    int i, j;
    int gap = 1;
    LightSortEntry tmp;
    while (gap <= (lbl_803DCE30 - 1) / 9)
        gap = gap * 3 + 1;
    while (gap > 0)
    {
        for (i = gap + 1; i <= lbl_803DCE30; i++)
        {
            tmp = ((LightSortEntry*)lbl_8037E0C0)[i - 1];
            j = i;
            while (j > gap && ((LightSortEntry*)lbl_8037E0C0)[j - gap - 1].key < tmp.key)
            {
                ((LightSortEntry*)lbl_8037E0C0)[j - 1] = ((LightSortEntry*)lbl_8037E0C0)[j - gap - 1];
                j -= gap;
            }
            ((LightSortEntry*)lbl_8037E0C0)[j - 1] = tmp;
        }
        gap /= 3;
    }
}






extern const f32 lbl_803DEC20;

asm void fn_8005D3B4(MapBlockBoundsRec* bounds, MapBlockData* block, s32 selector)
{
    nofralloc
    stwu r1, -48(r1)
    mflr r0
    stw r0, 52(r1)
    stw r31, 44(r1)
    stw r30, 40(r1)
    stw r29, 36(r1)
    mr r29, r3
    mr r30, r4
    mr r31, r5
    lwz r0, lbl_803DCE30
    cmpwi r0, 1000
    bne _psq
    bl sceneDrawTransparentPolys
    li r0, 0
    stw r0, lbl_803DCE30
_psq:
    psq_l f0, 12(r29), 1, 5
    psq_l f1, 6(r29), 1, 5
    psq_l f2, 14(r29), 1, 5
    lfs f3, lbl_803DEC20
    lfs f6, 40(r30)
    fmadds f9, f2, f3, f6
    psq_l f4, 8(r29), 1, 5
    psq_l f2, 16(r29), 1, 5
    lfs f7, 56(r30)
    fmadds f10, f2, f3, f7
    psq_l f5, 10(r29), 1, 5
    lfs f2, lbl_803DEBFC
    lfs f8, 24(r30)
    fmadds f1, f1, f3, f8
    fmadds f0, f0, f3, f8
    fadds f0, f1, f0
    fmuls f0, f2, f0
    stfs f0, 8(r1)
    fmadds f0, f4, f3, f6
    fadds f0, f0, f9
    fmuls f0, f2, f0
    stfs f0, 12(r1)
    fmadds f0, f5, f3, f7
    fadds f0, f0, f10
    fmuls f0, f2, f0
    stfs f0, 16(r1)
    bl Camera_GetViewMatrix
    addi r4, r1, 8
    mr r5, r4
    bl PSMTXMultVec
    lfs f0, 16(r1)
    fneg f0, f0
    fctiwz f0, f0
    stfd f0, 24(r1)
    lwz r0, 28(r1)
    cmpwi r0, 0
    bge _pos
    li r4, 0
    b _store
_pos:
    lis r3, 2048
    addi r4, r3, -1
    cmpw r0, r4
    ble _clamp
    b _store
_clamp:
    mr r4, r0
_store:
    lwz r0, lbl_803DCE30
    slwi r0, r0, 4
    lis r3, lbl_8037E0C0@ha
    addi r3, r3, lbl_8037E0C0@l
    stwx r29, r3, r0
    add r3, r3, r0
    stw r30, 4(r3)
    clrlwi r0, r31, 24
    slwi r0, r0, 27
    or r0, r4, r0
    stw r0, 8(r3)
    lwz r31, 44(r1)
    lwz r30, 40(r1)
    lwz r29, 36(r1)
    lwz r0, 52(r1)
    mtlr r0
    addi r1, r1, 48
    blr
}


void sortVisibleObjectKeysDescending(u32* arr, int n);


void modelRenderFn_8005d4ec(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx)
{
    int state[5];
    int countShifted;
    int cursor;
    u32 v;
    int* base;
    struct MapShader* newR;
    int nibble;
    int i;
    u8* s0;

    countShifted = block->nRenderInstrsMain << 3;
    modelRenderInstrsState_init((ModelRenderInstrsState*)state, block->renderInstrsMain, countShifted, countShifted);
    modelRenderInstrsState_setBit((ModelRenderInstrsState*)state, bounds->renderBitOffset);
    state[4] += 4;
    mapBlockRender_drawDimmedAabbLights(bounds, block, viewMtx);
    newR = mapBlockRender_setLightmapShader(block, (ModelRenderInstrsState*)state);
    state[4] += 4;
    mapBlockRender_setVtxDcrs(1, block, newR, (ModelRenderInstrsState*)state);
    cursor = state[4] + 4;
    state[4] = cursor;
    countShifted = cursor >> 3;
    s0 = (u8*)state[0];
    v = s0[countShifted];
    base = (int*)(state[0] + countShifted);
    v = v | ((u32)*(u8*)((char*)base + 1) << 8);
    v = v | ((u32)*(u8*)((char*)base + 2) << 16);
    state[4] += 4;
    nibble = (v >> (cursor & 7)) & 0xf;
    for (i = 0; i < nibble; i++)
    {
        *(int*)&state[4] = state[4] + 8;
    }
    state[4] += 4;
    mapBlockRender_drawLightmapIndirectPasses(block, newR, (ModelRenderInstrsState*)state,
                                               (float (*)[4])viewMtx);
}
void modelRenderFn_8005d69c(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx)
{
    int state[5];
    f32 m[12];
    int countShifted;
    struct MapShader* newR;
    int cursor;
    u32 v;
    int* base;
    int nibble;
    int i;
    u8* s0;

    PSMTXConcat((MtxPtr)lbl_80396850, (MtxPtr)viewMtx, (MtxPtr)m);
    GXLoadTexMtxImm((const f32 (*)[4])m, GX_TEXMTX0, GX_MTX3x4);
    PSMTXConcat((MtxPtr)lbl_80396820, (MtxPtr)viewMtx, (MtxPtr)m);
    GXLoadTexMtxImm((const f32 (*)[4])m, GX_TEXMTX1, GX_MTX3x4);
    gxTextureSetupFn_8007cf7c();
    countShifted = block->nRenderInstrsWater << 3;
    modelRenderInstrsState_init((ModelRenderInstrsState*)state, block->renderInstrsWater, countShifted, countShifted);
    modelRenderInstrsState_setBit((ModelRenderInstrsState*)state, bounds->renderBitOffset);
    state[4] += 4;
    newR = mapBlockRender_setShader(1, block, (ModelRenderInstrsState*)state);
    state[4] += 4;
    mapBlockRender_setVtxDcrs(1, block, newR, (ModelRenderInstrsState*)state);
    cursor = state[4] + 4;
    state[4] = cursor;
    countShifted = cursor >> 3;
    s0 = (u8*)state[0];
    v = s0[countShifted];
    base = (int*)(state[0] + countShifted);
    v = v | ((u32)*(u8*)((char*)base + 1) << 8);
    v = v | ((u32)*(u8*)((char*)base + 2) << 16);
    state[4] += 4;
    nibble = (v >> (cursor & 7)) & 0xf;
    for (i = 0; i < nibble; i++)
    {
        *(int*)&state[4] = state[4] + 8;
    }
    state[4] += 4;
    mapBlockRender_callList(1, 1, block, newR, (ModelRenderInstrsState*)state, viewMtx);
}
void modelRenderFn_8005d894(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx)
{
    int state[5];
    int countShifted;
    struct MapShader* newR;
    int cursor;
    u32 v;
    int* base;
    int nibble;
    int i;
    u8* s0;

    Camera_ApplyTransparentViewport();
    countShifted = block->nRenderInstrsTransp << 3;
    modelRenderInstrsState_init((ModelRenderInstrsState*)state, block->renderInstrsTransp, countShifted, countShifted);
    modelRenderInstrsState_setBit((ModelRenderInstrsState*)state, bounds->renderBitOffset);
    state[4] += 4;
    newR = mapBlockRender_setShader(1, block, (ModelRenderInstrsState*)state);
    state[4] += 4;
    mapBlockRender_setVtxDcrs(1, block, newR, (ModelRenderInstrsState*)state);
    cursor = state[4] + 4;
    state[4] = cursor;
    countShifted = cursor >> 3;
    s0 = (u8*)state[0];
    v = s0[countShifted];
    base = (int*)(state[0] + countShifted);
    v = v | ((u32)*(u8*)((char*)base + 1) << 8);
    v = v | ((u32)*(u8*)((char*)base + 2) << 16);
    state[4] += 4;
    nibble = (v >> (cursor & 7)) & 0xf;
    for (i = 0; i < nibble; i++)
    {
        *(int*)&state[4] = state[4] + 8;
    }
    state[4] += 4;
    mapBlockRender_callList(1, 1, block, newR, (ModelRenderInstrsState*)state, viewMtx);
    Camera_ApplyFullViewport();
}


void objDrawFn_8005da48(GameObject* obj)
{
    int* model = (int*)Obj_GetActiveModel(obj);
    if (*(void**)((char*)model + 0x58) != NULL)
    {
        objRenderFn_8003d980((u8*)obj, model);
    }
    else
    {
        void* shadow;
        (*gModgfxInterface)->renderEffects(NULL, 0, 0, 1, obj);
        renderResetFn_8003fc60();
        objRender(0, 0, 0, 0, obj, 1);
        Camera_ApplyDecalViewport();
        shadow = obj->anim.modelState;
        if (shadow != NULL && ((ObjModelState*)shadow)->shadowCastSlot != NULL)
        {
            objShadowFn_80062498(obj, 0, 0, framesThisStep);
        }
        else if (((ObjAnimComponent*)obj)->modelInstance->shadowType == OBJ_SHADOW_TYPE_CRASH)
        {
            objDrawFn_80061654(obj, (ObjModel*)model);
        }
        Camera_ApplyFullViewport();
    }
}

void sceneDrawTransparentPolys(void)
{
    GXColor c5;
    int i;
    LightmapDrawItem item;
    GameObject* player;
    GXColor c4;
    LightmapDrawEntry* entries;
    GXColor c6;
    f32 m[16];

    lightmap_sortTransparentDrawQueue();
    i = 0;
    entries = (LightmapDrawEntry*)lbl_8037E0C0;
    for (; i < lbl_803DCE30; i++)
    {
        switch (entries[i].type)
        {
        case 0:
            expgfx_renderSourcePools(entries[i].arg0.value, 0);
            objDrawFn_8005da48(entries[i].arg0.object);
            expgfx_renderSourcePools(entries[i].arg0.value, 1);
            break;
        case 1:
            item.object = entries[i].arg0.object;
            Obj_GetActiveModel(item.object);
            player = Obj_GetPlayerObject();
            if (item.object == player)
            {
                if (playerIsDisguised(item.object) == 0)
                {
                    fn_802B4ED8(item.object, 1, 1);
                }
            }
            else
            {
                objRenderFuzz((int*)item.object);
            }
            break;
        case 2:
            Camera_ApplyDecalViewport();
            objShadowFn_80062498(entries[i].arg0.object, 0, 0, framesThisStep);
            Camera_ApplyFullViewport();
            break;
        case 3:
            Camera_ApplyDecalViewport();
            objDrawFn_80061654(entries[i].arg0.object, Obj_GetActiveModel(entries[i].arg0.object));
            Camera_ApplyFullViewport();
            break;
        case 4:
            item.block = entries[i].arg1.block;
            GXSetChanCtrl(GX_COLOR0, GX_TRUE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
            GXSetChanCtrl(GX_ALPHA0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
            objGetColor(0, (u8*)&c4, (u8*)&c4 + 1, (u8*)&c4 + 2);
            GXSetChanAmbColor(GX_COLOR0, c4);
            GXSetNumChans(1);
            PSMTXConcat((MtxPtr)Camera_GetViewMatrix(), (MtxPtr)item.block->transform, (MtxPtr)m);
            setupToRenderMapBlock(item.block, m);
            modelRenderFn_8005d894(entries[i].arg0.bounds, entries[i].arg1.block, m);
            break;
        case 5:
            item.block = entries[i].arg1.block;
            GXSetChanCtrl(GX_COLOR0, GX_TRUE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
            GXSetChanCtrl(GX_ALPHA0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
            objGetColor(0, (u8*)&c5, (u8*)&c5 + 1, (u8*)&c5 + 2);
            GXSetChanAmbColor(GX_COLOR0, c5);
            GXSetNumChans(1);
            PSMTXConcat((MtxPtr)Camera_GetViewMatrix(), (MtxPtr)item.block->transform, (MtxPtr)m);
            setupToRenderMapBlock(item.block, m);
            modelRenderFn_8005d69c(entries[i].arg0.bounds, entries[i].arg1.block, m);
            break;
        case 6:
            item.block = entries[i].arg1.block;
            GXSetChanCtrl(GX_COLOR0, GX_TRUE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
            GXSetChanCtrl(GX_ALPHA0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
            objGetColor(0, (u8*)&c6, (u8*)&c6 + 1, (u8*)&c6 + 2);
            GXSetChanAmbColor(GX_COLOR0, c6);
            GXSetNumChans(1);
            PSMTXConcat((MtxPtr)Camera_GetViewMatrix(), (MtxPtr)item.block->transform, (MtxPtr)m);
            setupToRenderMapBlock(item.block, m);
            modelRenderFn_8005d4ec(entries[i].arg0.bounds, entries[i].arg1.block, m);
            break;
        case 7:
            drawGlow(entries[i].arg0.value, entries[i].arg1.value);
            break;
        case 8:
            drawFn_8006f500();
            break;
        case 9:
            (*gWaterfxInterface)->render(0, 0);
        }
    }
}


void lightmap_queueExternalRenderEntry(u32 a, u32 b, f32* p)
{
    s32 t;
    if (lbl_803DCE30 == 1000)
    {
        sceneDrawTransparentPolys();
        lbl_803DCE30 = 0;
    }
    t = (s32) - p[2];
    t = t < 0 ? 0 : (t > 0x7ffffff ? 0x7ffffff : t);
    lbl_8037E0C0[lbl_803DCE30 * 4] = a;
    lbl_8037E0C0[lbl_803DCE30 * 4 + 1] = b;
    lbl_8037E0C0[lbl_803DCE30 * 4 + 2] = t | 0x38000000;
    lbl_8037E0C0[lbl_803DCE30 * 4 + 3] = 7;
    lbl_803DCE30++;
}

#include "main/map_block.h"
#include "track/intersect_depth_state_api.h"
#include "track/intersect_depth_read_api.h"
#include "main/model_light.h"
#include "main/objHitReact.h"
#include "main/objhits.h"
#undef OBJHITS_STATE_INDEX_S8
#include "dolphin/os/OSFastCast.h"
#include "dolphin/gx/GXBump.h"
#include "dolphin/gx/GXPixel.h"
#include "dolphin/gx/GXTev.h"
#include "main/track_dolphin.h"
#define TRACK_BBOX_FLAGS_S8
#define TRACK_BBOX_MASK_TYPE s8
#define TRACK_BBOX_ARG10_TYPE s8
#include "main/track_bbox_api.h"
#undef TRACK_BBOX_ARG10_TYPE
#undef TRACK_BBOX_MASK_TYPE
#undef TRACK_BBOX_FLAGS_S8
#include "main/pause_menu_api.h"
#include "main/objmodel.h"
#include "main/newshadows_texture_api.h"
#include "main/atan2f_api.h"
#include "dolphin/gx/GXDispList.h"
#include "track/intersect_fog_api.h"
#include "main/objseq_api.h"
#include "main/dll/FRONT/n_options.h"

u8 lbl_803DB638[4] = {0x20, 0x20, 0x20, 0};
int gTexShaderAmbColor = -1;
int gTexLightmapAmbColor = -1;
s8 gTexIndMtxScaleExp = -2;

extern f32 lbl_803DEBCC;
extern const f32 lbl_803DEBFC;
extern const f32 lbl_803DEC20;
extern const f32 gTexIndMtxScale;
extern f32 lbl_803DEC28;
extern const f32 lbl_803DEC2C;
extern int lbl_803DEBB0;
extern ModelLightStruct* gTexDimmedLightList[2];
extern ModelLightStruct* gTexBlockLightList[2];
extern s32 lbl_803DCE30;
extern u32 lbl_803DCE34;
extern int gTexIndMtxTable[];
#define FRUSTUM_PLANE_COUNT 5
extern FrustumPlane gViewFrustumPlanes[FRUSTUM_PLANE_COUNT];
extern int gTexShaderFogColor;
extern int gTexLightmapFogColor;

/*
 * TexShadowRow - 0x10-stride rows of the pending-shadow queue at the head of
 * lbl_8037E0C0 (indexed by lbl_803DCE30, bumped after each fn_8005D3B4 push).
 * mapBlockRender_callList writes type (4/5 = object shadow, 6 = indirect
 * lightmap) into the queued shadow row.
 */
typedef struct TexShadowRow
{
    int unk0;
    int unk4;
    int unk8;
    int type;
} TexShadowRow;

u8 mapBlockBounds_HasCornerPastDepthThreshold(MapBlockBoundsRec* bounds, float* xform)
{
    float v[3];
    u32 i;
    f32 fbset;
    f32 timing;

    i = 0;
    timing = lbl_803DEC20;
    fbset = lbl_803DEC28;
    while (1)
    {
        {
            switch (i)
            {
            case 0:
                v[0] = (f32)bounds->minX;
                v[1] = (f32)bounds->minY;
                v[2] = (f32)bounds->minZ;
                break;
            case 1:
                v[0] = (f32)bounds->maxX;
                v[1] = (f32)bounds->minY;
                v[2] = (f32)bounds->minZ;
                break;
            case 2:
                v[0] = (f32)bounds->minX;
                v[1] = (f32)bounds->maxY;
                v[2] = (f32)bounds->minZ;
                break;
            case 3:
                v[0] = (f32)bounds->maxX;
                v[1] = (f32)bounds->maxY;
                v[2] = (f32)bounds->minZ;
                break;
            case 4:
                v[0] = (f32)bounds->minX;
                v[1] = (f32)bounds->minY;
                v[2] = (f32)bounds->maxZ;
                break;
            case 5:
                v[0] = (f32)bounds->maxX;
                v[1] = (f32)bounds->minY;
                v[2] = (f32)bounds->maxZ;
                break;
            case 6:
                v[0] = (f32)bounds->minX;
                v[1] = (f32)bounds->maxY;
                v[2] = (f32)bounds->maxZ;
                break;
            case 7:
                v[0] = (f32)bounds->maxX;
                v[1] = (f32)bounds->maxY;
                v[2] = (f32)bounds->maxZ;
                break;
            }
        }
        v[0] = v[0] * timing;
        v[1] = v[1] * timing;
        v[2] = v[2] * timing;
        PSMTXMultVec((MtxPtr)xform, (Vec*)v, (Vec*)v);
        if (v[2] >= fbset)
        {
            return 1;
        }
        i = i + 1;
        if ((int)i < 8)
        {
            continue;
        }
        return 0;
    }
}

typedef struct IndMtxCopy
{
    int w[6];
} IndMtxCopy;

#define SHADER_FLAGS(s) ((s)->flags)

void mapBlockRender_drawLightmapIndirectPasses(struct MapBlockData* blockData, MapShader* shader,
                                               ModelRenderInstrsState* state, f32 (*viewMtx)[4])
{
    f32 passMtx[3][4];
    float indMtx[2][3];
    int noiseFrameCount;
    Texture** noiseTextures;
    MapBlockBoundsRec* bounds[1];
    u8 passCount;
    int byteBase;
    u32 bits;
    int bitPos;
    u32 flags;
    u8* mtxSrc;
    int i;

    bitPos = state->bit;
    {
        int off = bitPos >> 3;
        byteBase = (int)state->instrs;
        bits = *(u8*)(byteBase + off);
        byteBase += off;
        bits = bits | (u32)(*(u8*)(byteBase + 1) << 8);
        bits = bits | (u32)(*(u8*)(byteBase + 2) << 16);
    }
    state->bit = bitPos + 8;
    /* extract this cursor's 8-bit field (LSB-first: shift out the bits already
     * consumed within the byte, then mask the width) -> bounds-record index */
    bounds[0] = &blockData->displayLists[(bits >> (bitPos & 7)) & 0xff];
    flags = SHADER_FLAGS(shader);
    if ((flags & 0x4000) != 0)
    {
        passCount = 4;
    }
    else if ((flags & 0x8000) != 0)
    {
        passCount = 8;
    }
    else if ((flags & 0x10000) != 0)
    {
        passCount = 0x10;
    }
    else
    {
        return;
    }
    i = 0;
    for (; i < passCount; i = i + 1)
    {
        PSMTXTrans((MtxPtr)passMtx, 0.0f, lbl_803DEC2C * (f32)(i + 1), 0.0f);
        PSMTXConcat((MtxPtr)viewMtx, (MtxPtr)passMtx, (MtxPtr)passMtx);
        GXLoadPosMtxImm(passMtx, GX_PNMTX0);
        mtxSrc = (u8*)gTexIndMtxTable;
        *(IndMtxCopy*)indMtx = *(IndMtxCopy*)mtxSrc;
        getNewShadowNoiseTextureFrames(&noiseTextures, &noiseFrameCount);
        selectTexture(noiseTextures[(u8)i], 1);
        {
            f32 s = (f32)((i & 0xff) + 1) * gTexIndMtxScale;
            indMtx[0][0] = s * lbl_803DEBFC;
        }
        indMtx[1][1] = indMtx[0][0];
        GXSetIndTexMtx(GX_ITM_0, (const float (*)[3])indMtx, gTexIndMtxScaleExp);
        GXCallDisplayList(bounds[0]->dlist, bounds[0]->dlistSize);
    }
}

MapShader* mapBlockRender_setLightmapShader(struct MapBlockData* blockData, ModelRenderInstrsState* state)
{
    MapShader* shader;
    u32 shaderIdx;
    int byteBase;
    int fogColor;
    u32 bits;
    u32 bitPos;
    u8 ambColor[3];

    fogColor = gTexLightmapFogColor;
    bitPos = state->bit;
    {
        int off = (int)bitPos >> 3;
        byteBase = (int)state->instrs;
        bits = *(u8*)(byteBase + off);
        byteBase += off;
        bits |= (u32) * (u8*)(byteBase + 1) << 8;
        bits |= (u32) * (u8*)(byteBase + 2) << 16;
        state->bit = bitPos + 6;
        shaderIdx = (bits >> (bitPos & 7)) & 0x3f;
        shader = &blockData->shaders[shaderIdx];
    }
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
    selectTexture(((MapShaderLayer*)Shader_getLayer(shader, 0))->texture, 0);
    if ((SHADER_FLAGS(shader) & 4) != 0)
    {
        _gxSetFogParams();
    }
    else
    {
        GXSetFog(GX_FOG_NONE, 0.0f, 0.0f, 0.0f, 0.0f, *(GXColor*)&fogColor);
    }
    if ((SHADER_FLAGS(shader) & 1) != 0 || (SHADER_FLAGS(shader) & 0x40000) != 0 ||
        (SHADER_FLAGS(shader) & 0x800) != 0 || (SHADER_FLAGS(shader) & 0x1000) != 0)
    {
        GXSetChanAmbColor(GX_COLOR0, *(GXColor*)&gTexLightmapAmbColor);
        if ((SHADER_FLAGS(shader) & 0x40000) != 0)
        {
            GXSetChanCtrl(GX_COLOR0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        }
        else
        {
            GXSetChanCtrl(GX_COLOR0, GX_ENABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        }
    }
    else
    {
        objGetColor(0, &ambColor[0], &ambColor[1], &ambColor[2]);
        GXSetChanCtrl(GX_COLOR0, GX_ENABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        GXSetChanAmbColor(GX_COLOR0, *(GXColor*)&ambColor[0]);
    }
    return shader;
}

void mapBlockRender_drawDimmedAabbLights(MapBlockBoundsRec* bounds, MapBlockData* block, float* viewMtx)
{
    ModelLightStruct** lightPtr;
    f32 posZ;
    f32 posY;
    f32 posX;
    int lightCount;
    u8 colorA;
    u8 colorB;
    u8 colorG;
    u8 colorR;

    {
        f32 fz = *(f32*)&playerMapOffsetZ;
        f32 fldZ = block->transform[2][3];
        f32 fldY = block->transform[1][3];
        f32 fx = *(f32*)&playerMapOffsetX;
        f32 fldX = block->transform[0][3];
        f32 ax0 = (f32)(bounds->minX >> 3) + fldX;
        f32 az0 = (f32)(bounds->minZ >> 3) + fldZ;
        f32 ax1 = (f32)(bounds->maxX >> 3) + fldX;
        f32 az1 = (f32)(bounds->maxZ >> 3) + fldZ;
        modelLightStruct_selectBrightestAabbLights(ax0 + fx, (f32)(bounds->minY >> 3) + fldY, az0 + fz, ax1 + fx,
                                                   (f32)(bounds->maxY >> 3) + fldY, az1 + fz,
                                                   gTexDimmedLightList, 2, &lightCount);
    }
    Rcp_ResetTextureStageState();
    fn_8004CE0C(viewMtx);
    {
        u8* pColorA;
        u8* pColorB;
        u8* pColorG;
        f32* pPosZ;
        f32* pPosY;
        int i;

        i = 0;
        lightPtr = gTexDimmedLightList;
        pColorA = &colorA;
        pColorB = &colorB;
        pColorG = &colorG;
        pPosZ = &posZ;
        pPosY = &posY;
        for (; i < lightCount; lightPtr = lightPtr + 1, i = i + 1)
        {
            modelLightStruct_getDiffuseColor(*lightPtr, &colorR, pColorG, pColorB, pColorA);
            colorR = ((int)colorR >> 1) + ((int)colorR >> 2);
            colorG = ((int)colorG >> 1) + ((int)colorG >> 2);
            colorB = ((int)colorB >> 1) + ((int)colorB >> 2);
            modelLightStruct_getPosition(*lightPtr, &posX, pPosY, pPosZ);
            fn_8004FA30(modelLightStruct_getRadius(*lightPtr), (int*)&colorR, &posX);
        }
    }
    Rcp_ApplyTextureStageCounts();
    GXSetNumChans(1);
    GXSetCullMode(GX_CULL_BACK);
    gxSetZMode_(1, GX_LEQUAL, 0);
    gxSetPeControl_ZCompLoc_(1);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    return;
}

u32 frustumTestAabbWithPlaneOffsets(f32 minX, f32 maxX, f32 minY, f32 maxY, f32 minZ, f32 maxZ, f32* planeOffsets)
{
    FrustumPlane* plane;
    int cornerIndex;
    int i;
    float nearX;
    float nearY;
    float nearZ;
    float farX;
    float farY;
    float farZ;

    plane = gViewFrustumPlanes;
    for (i = 0; i < FRUSTUM_PLANE_COUNT; i++)
    {
        cornerIndex = plane[i].aabbCornerIndex;
        if ((cornerIndex & 1) != 0)
        {
            nearX = maxX;
            farX = minX;
        }
        else
        {
            nearX = minX;
            farX = maxX;
        }
        if ((cornerIndex & 2) != 0)
        {
            nearY = maxY;
            farY = minY;
        }
        else
        {
            nearY = minY;
            farY = maxY;
        }
        if ((cornerIndex & 4) != 0)
        {
            nearZ = maxZ;
            farZ = minZ;
        }
        else
        {
            nearZ = minZ;
            farZ = maxZ;
        }
        if ((nearX * plane[i].normalX + nearY * plane[i].normalY + nearZ * plane[i].normalZ + plane[i].distance +
                 planeOffsets[i] <
             lbl_803DEBCC) &&
            (farX * plane[i].normalX + farY * plane[i].normalY + farZ * plane[i].normalZ + plane[i].distance +
                 planeOffsets[i] <
             lbl_803DEBCC))
            return 0;
    }
    return 1;
}

u8 mapBlockBounds_ComputeAndTestPlanes(MapBlockBoundsRec* bounds, struct MapBlockData* block,
                                       FrustumPlane* planes, int planeCount, f32* minX, f32* minY, f32* minZ,
                                       f32* maxX, f32* maxY, f32* maxZ)
{
    u8 cornerIndex;
    float nearX;
    float nearY;
    float nearZ;
    float farX;
    float farY;
    float farZ;
    int i;
    *maxX = (f32)(bounds->maxX >> 3) + block->transform[0][3];
    *minX = (f32)(bounds->minX >> 3) + block->transform[0][3];
    *maxY = (f32)(bounds->maxY >> 3) + block->transform[1][3];
    *minY = (f32)(bounds->minY >> 3) + block->transform[1][3];
    *maxZ = (f32)(bounds->maxZ >> 3) + block->transform[2][3];
    *minZ = (f32)(bounds->minZ >> 3) + block->transform[2][3];
    for (i = 0; i < planeCount; i = i + 1)
    {
        cornerIndex = planes->aabbCornerIndex;
        if ((cornerIndex & 1) != 0)
        {
            nearX = *maxX;
            farX = *minX;
        }
        else
        {
            nearX = *minX;
            farX = *maxX;
        }
        if ((cornerIndex & 2) != 0)
        {
            nearY = *maxY;
            farY = *minY;
        }
        else
        {
            nearY = *minY;
            farY = *maxY;
        }
        if ((cornerIndex & 4) != 0)
        {
            nearZ = *maxZ;
            farZ = *minZ;
        }
        else
        {
            nearZ = *minZ;
            farZ = *maxZ;
        }
        if ((planes->distance + (nearX * planes->normalX + nearY * planes->normalY + nearZ * planes->normalZ) <
             lbl_803DEBCC) &&
            (planes->distance + (farX * planes->normalX + farY * planes->normalY + farZ * planes->normalZ) <
             lbl_803DEBCC))
        {
            return 0;
        }
        planes++;
    }
    return 1;
}

void mapBlockRender_callList(u32 passSelect, u32 visArg, MapBlockData* block, MapShader* shader,
                             ModelRenderInstrsState* state, float* mtx)
{
    int lightPos[3];
    int count;
    float minX;
    float minY;
    float minZ;
    float maxX;
    float maxY;
    float maxZ;
    u8 lightColor[4];
    GXColor chanColor;
    int i;
    u32 visible;
    u32 flags;
    u32 bits;
    int bitPos;
    int byteBase;

    {
        u8* texGlobals;
        MapBlockBoundsRec* bounds[1];

        texGlobals = (u8*)lbl_8037E0C0;
        bitPos = state->bit;
        {
            int off = bitPos >> 3;
            byteBase = (int)state->instrs;
            bits = *(u8*)(byteBase + off);
            byteBase += off;
            bits = bits | (u32)(*(u8*)(byteBase + 1) << 8);
            bits = bits | (u32)(*(u8*)(byteBase + 2) << 16);
        }
        state->bit = bitPos + 8;
        bounds[0] = &block->displayLists[(bits >> (bitPos & 7)) & 0xff];
        if ((shader != NULL) && ((SHADER_FLAGS(shader) & 2) != 0))
        {
            return;
        }
        if (mapBlockBounds_ComputeAndTestPlanes(bounds[0], block, (FrustumPlane*)(texGlobals + 0x987c),
                                                FRUSTUM_PLANE_COUNT, &minX, &minY, &minZ, &maxX, &maxY, &maxZ) == 0)
        {
            return;
        }
        if ((u8)passSelect == 0)
        {
            flags = SHADER_FLAGS(shader);
            if ((flags & 0x80000000) != 0)
            {
                int shadowType;

                fn_8005D3B4(bounds[0], block, bounds[0]->selector);
                shadowType = 5;
                *(int*)((u8*)&((TexShadowRow*)texGlobals)->type +
                         lbl_803DCE30 * sizeof(TexShadowRow)) = shadowType;
                lbl_803DCE30 = lbl_803DCE30 + 1;
            }
            else if (((flags & 0x40000000) != 0) || ((flags & 0x2000) != 0))
            {
                int shadowType;

                fn_8005D3B4(bounds[0], block, bounds[0]->selector);
                shadowType = 4;
                *(int*)((u8*)&((TexShadowRow*)texGlobals)->type +
                         lbl_803DCE30 * sizeof(TexShadowRow)) = shadowType;
                lbl_803DCE30 = lbl_803DCE30 + 1;
            }
        }
        else
        {
            if (shader != NULL)
            {
                flags = SHADER_FLAGS(shader);
                if (((flags & 0x80000000) == 0) && ((flags & 0x20000) == 0))
                {
                    if ((shader != NULL) && ((flags & 0x80000) != 0))
                    {
                        count = 0;
                    }
                    else
                    {
                        modelLightStruct_selectBrightestAabbLights(
                            minX + playerMapOffsetX, minY, minZ + playerMapOffsetZ, maxX + playerMapOffsetX, maxY,
                            maxZ + playerMapOffsetZ, gTexBlockLightList, 2, &count);
                    }
                    if ((shader != NULL) &&
                        (((SHADER_FLAGS(shader) & 0x800) != 0 || ((SHADER_FLAGS(shader) & 0x1000) != 0))))
                    {
                        ObjSeq_copyDefaultColor(&chanColor);
                        chanColor.a = 0;
                        chanColor.b = 0;
                        chanColor.g = 0;
                        chanColor.r = 0;
                        if (count == 0)
                        {
                            if ((shader != NULL) && ((SHADER_FLAGS(shader) & 0x800) != 0))
                            {
                                fn_8004EF9C((int*)&chanColor);
                            }
                            else
                            {
                                fn_8004EECC((u8*)&chanColor);
                            }
                        }
                        else
                        {
                            modelLightStruct_getDiffuseColor(gTexBlockLightList[0], &lightColor[0], &lightColor[1],
                                                             &lightColor[2], &lightColor[3]);
                            modelLightStruct_getPosition(gTexBlockLightList[0], (f32*)&lightPos[0],
                                                         (f32*)&lightPos[1], (f32*)&lightPos[2]);
                            fn_8004F6D8(modelLightStruct_getRadius(gTexBlockLightList[0]),
                                        (int*)lightColor, (f32*)&lightPos[0], (u8*)&chanColor);
                            for (i = 1; i < count; i = i + 1)
                            {
                                modelLightStruct_getDiffuseColor(gTexBlockLightList[i], &lightColor[0],
                                                                 &lightColor[1], &lightColor[2], &lightColor[3]);
                                modelLightStruct_getPosition(gTexBlockLightList[i], (f32*)&lightPos[0],
                                                             (f32*)&lightPos[1], (f32*)&lightPos[2]);
                                fn_8004F380(modelLightStruct_getRadius(gTexBlockLightList[i]),
                                            (int*)lightColor, (f32*)&lightPos[0]);
                            }
                            if ((shader != NULL) && ((SHADER_FLAGS(shader) & 0x800) != 0))
                            {
                                fn_8004F2B0();
                            }
                            else
                            {
                                fn_8004F080();
                            }
                        }
                    }
                    else
                    {
                        for (i = 0; i < count; i = i + 1)
                        {
                            modelLightStruct_getDiffuseColor(gTexBlockLightList[i], &lightColor[0],
                                                             &lightColor[1], &lightColor[2], &lightColor[3]);
                            modelLightStruct_getPosition(gTexBlockLightList[i], (f32*)&lightPos[0],
                                                         (f32*)&lightPos[1], (f32*)&lightPos[2]);
                            fn_8004FA30(modelLightStruct_getRadius(gTexBlockLightList[i]),
                                        (int*)lightColor, (f32*)&lightPos[0]);
                        }
                    }
                    if ((shader != NULL) && ((SHADER_FLAGS(shader) & 0x2000) != 0))
                    {
                        if ((shader != NULL) && ((SHADER_FLAGS(shader) & 0x40000000) != 0))
                        {
                            visible = visArg;
                        }
                        else
                        {
                            u8 mirrorVisible = mapBlockBounds_ComputeAndTestPlanes(
                                bounds[0], block, (FrustumPlane*)(texGlobals + 0x9818), FRUSTUM_PLANE_COUNT, &minX, &minY,
                                &minZ, &maxX, &maxY, &maxZ);
                            if ((mirrorVisible != 0 && (u8)visArg != 0) || (mirrorVisible == 0 && (u8)visArg == 0))
                            {
                                visible = 1;
                            }
                            else
                            {
                                visible = 0;
                            }
                            if ((u8)visArg != 0)
                            {
                                GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
                                gxSetZMode_(1, GX_LEQUAL, 0);
                                gxSetPeControl_ZCompLoc_(1);
                                GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
                            }
                        }
                        if ((u8)visible == 0)
                        {
                            return;
                        }
                        fn_8004D230();
                    }
                    Rcp_ApplyTextureStageCounts();
                }
            }
            GXCallDisplayList(bounds[0]->dlist, bounds[0]->dlistSize);
            flags = SHADER_FLAGS(shader);
            if ((((flags & 0x4000) != 0) || ((flags & 0x8000) != 0) || ((flags & 0x10000) != 0)) &&
                (mapBlockBounds_HasCornerPastDepthThreshold(bounds[0], mtx) != 0))
            {
                int shadowType;

                fn_8005D3B4(bounds[0], block, 0x17);
                shadowType = 6;
                *(int*)((u8*)&((TexShadowRow*)texGlobals)->type +
                         lbl_803DCE30 * sizeof(TexShadowRow)) = shadowType;
                lbl_803DCE30 = lbl_803DCE30 + 1;
            }
        }
    }
}

void mapBlockRender_setupShaderTextures(MapShader* shader, int mode)
{
    int layerIdx;
    MapShaderLayer* layer;
    int texture;
    f32 (*texMtx)[4];
    int overrideIdx;
    int remain;
    MapTextureOverride* overrideEntry;
    u8 layerByte;
    u32 kColor;
    f32 tx;
    f32 texMatrix[3][4];

    kColor = lbl_803DEBB0;
    if ((shader->layerCount == 2) &&
        (texture = (int)Shader_getLayer(shader, 1),
         (((MapShaderLayer*)texture)->typeBits & 0x7f) == 9u))
    {
        layer = Shader_getLayer(shader, 0);
        {
            u8 overrideType;
            if ((overrideType = layer->overrideType) != '\0')
            {
                int layerTextureId = layer->textureIndex;
                MapTextureOverride* overrides;
                overrideIdx = 0;
                overrides = (MapTextureOverride*)(int)lbl_803DCE6C;
                overrideEntry = overrides;
                for (remain = 0x50; remain != 0 || (texture = layerTextureId, 0); remain--)
                {
                    if (((0 < overrideEntry->refCount) &&
                         ((u32)overrideEntry->textureId == layerTextureId)) &&
                        ((int)overrideType == overrideEntry->type))
                    {
                        texture = (int)textureGetAnimationFrame((Texture*)layerTextureId,
                                                                overrides[overrideIdx].frame);
                        break;
                    }
                    overrideEntry = overrideEntry + 1;
                    overrideIdx = overrideIdx + 1;
                }
            }
            else
            {
                texture = layer->textureIndex;
            }
        }
        if (layer->scrollMtx != 0xff)
        {
            tx = *(float*)((int)lbl_803DCE68 + ((u32)layer->scrollMtx << 4)) / 1048576.0f;
            PSMTXTrans((MtxPtr)texMatrix, tx,
                       *(float*)((int)lbl_803DCE68 + 4 + ((u32)layer->scrollMtx << 4)) /
                           1048576.0f,
                       lbl_803DEBCC);
            texMtx = texMatrix;
        }
        else
        {
            texMtx = NULL;
        }
        fn_80051B00((Texture*)texture, texMtx, 0, (GXColor*)&kColor);
        if ((SHADER_FLAGS(shader) & 0x100) != 0)
        {
            fn_8004D928();
        }
        layer = Shader_getLayer(shader, 1);
        {
            u8 overrideType;
            if ((overrideType = layer->overrideType) != '\0')
            {
                int layerTextureId = layer->textureIndex;
                MapTextureOverride* overrides;
                overrideIdx = 0;
                overrides = (MapTextureOverride*)(int)lbl_803DCE6C;
                overrideEntry = overrides;
                for (remain = 0x50; remain != 0 || (texture = layerTextureId, 0); remain--)
                {
                    if (((0 < overrideEntry->refCount) &&
                         ((u32)overrideEntry->textureId == layerTextureId)) &&
                        ((int)overrideType == overrideEntry->type))
                    {
                        texture = (int)textureGetAnimationFrame((Texture*)layerTextureId,
                                                                overrides[overrideIdx].frame);
                        break;
                    }
                    overrideEntry = overrideEntry + 1;
                    overrideIdx = overrideIdx + 1;
                }
            }
            else
            {
                texture = layer->textureIndex;
            }
        }
        if (layer->scrollMtx != 0xff)
        {
            tx = *(float*)((int)lbl_803DCE68 + ((u32)layer->scrollMtx << 4)) / 1048576.0f;
            PSMTXTrans((MtxPtr)texMatrix, tx,
                       *(float*)((int)lbl_803DCE68 + 4 + ((u32)layer->scrollMtx << 4)) /
                           1048576.0f,
                       lbl_803DEBCC);
            texMtx = texMatrix;
        }
        else
        {
            texMtx = NULL;
        }
        fn_80051868((Texture*)texture, texMtx, 9);
        textureFn_800524ec((GXColor*)&kColor);
    }
    else
    {
        for (layerIdx = 0; layerIdx < (int)(u32)shader->layerCount; layerIdx = layerIdx + 1)
        {
            int layerTextureId;
            layer = Shader_getLayer(shader, layerIdx);
            layerTextureId = layer->textureIndex;
            if ((u32)layerTextureId != 0)
            {
                u8 overrideType;
                {
                    if ((overrideType = layer->overrideType) != '\0')
                    {
                        MapTextureOverride* overrides;
                        overrideIdx = 0;
                        overrides = (MapTextureOverride*)(int)lbl_803DCE6C;
                        overrideEntry = overrides;
                        for (remain = 0x50; remain != 0 || (texture = layerTextureId, 0); remain--)
                        {
                            if (((0 < overrideEntry->refCount) &&
                                 ((u32)overrideEntry->textureId == layerTextureId)) &&
                                ((int)overrideType == overrideEntry->type))
                            {
                                texture = (int)textureGetAnimationFrame(
                                    (Texture*)layerTextureId, overrides[overrideIdx].frame);
                                break;
                            }
                            overrideEntry = overrideEntry + 1;
                            overrideIdx = overrideIdx + 1;
                        }
                    }
                    else
                    {
                        texture = layerTextureId;
                    }
                    if (layer->scrollMtx != 0xff)
                    {
                        int scrollOffset = (u32)layer->scrollMtx * 0x10;
                        tx = *(float*)((u8*)lbl_803DCE68 + scrollOffset) / 1048576.0f;
                        PSMTXTrans((MtxPtr)texMatrix, tx,
                                   *(float*)((u8*)lbl_803DCE68 + scrollOffset + 4) / 1048576.0f,
                                   lbl_803DEBCC);
                        texMtx = texMatrix;
                    }
                    else
                    {
                        texMtx = NULL;
                    }
                    layerByte = layer->typeBits & 0x7f;
                    if ((SHADER_FLAGS(shader) & 0x40000) != 0)
                    {
                        fn_80051528((void*)texture, texMtx);
                    }
                    else
                    {
                        fn_80051868((Texture*)texture, texMtx, layerByte);
                    }
                }
            }
            else
            {
                gxColorFn_800523d0();
            }
        }
        if ((SHADER_FLAGS(shader) & 0x100) != 0)
        {
            fn_8004D928();
        }
    }
    return;
}

MapShader* mapBlockRender_setShader(u8 doSetup, MapBlockData* blockData, ModelRenderInstrsState* state)
{
    MapShader* shader;
    u32 shaderIdx;
    int fogColor;
    int byteBase;
    u32 flags;
    int* lightList;
    u8 ambColor[3];
    u8 fogRgba[4];
    u32 bits;
    u32 bitPos;

    fogColor = gTexShaderFogColor;
    bitPos = state->bit;
    {
        int off = (int)bitPos >> 3;
        byteBase = (int)state->instrs;
        bits = *(u8*)(byteBase + off);
        byteBase += off;
        bits |= (u32) * (u8*)(byteBase + 1) << 8;
        bits |= (u32) * (u8*)(byteBase + 2) << 16;
        state->bit = bitPos + 6;
        shaderIdx = (bits >> (bitPos & 7)) & 0x3f;
        shader = &blockData->shaders[shaderIdx];
    }

    if (doSetup == 0)
    {
        return shader;
    }

    if ((SHADER_FLAGS(shader) & 4) != 0)
    {
        _gxSetFogParams();
    }
    else
    {
        GXSetFog(GX_FOG_NONE, 0.0f, 0.0f, 0.0f, 0.0f, *(GXColor*)&fogColor);
    }
    if ((shader != 0) && ((SHADER_FLAGS(shader) & 0x80000000) != 0))
    {
        return shader;
    }
    if ((shader != 0) && ((SHADER_FLAGS(shader) & 0x20000) != 0))
    {
        u32 res;
        res = AttractMovie_DrawTextureCallback(0, 0, 0);
        if ((res & 0xff) != 0)
        {
            return shader;
        }
    }
    Rcp_ResetTextureStageState();
    if ((SHADER_FLAGS(shader) & 0x80) != 0)
    {
        fn_8004DA54((char*)shader);
    }
    else
    {
        mapBlockRender_setupShaderTextures(shader, 0x80);
    }
    flags = SHADER_FLAGS(shader);
    if ((flags & 0x20) != 0 && (lightList = (int*)lbl_803DCE34) != 0)
    {
        fn_8004FDA0((u8*)lightList, &lbl_80382008, lbl_803DB638);
    }
    else if ((flags & 0x40) != 0)
    {
        fn_8004E0FC();
    }
    else if (isHeavyFogEnabled())
    {
        getColor803dd01c(fogRgba);
        renderHeavyFog(fogRgba);
    }
    if (((SHADER_FLAGS(shader) & 0x40000000) != 0) || ((SHADER_FLAGS(shader) & 0x20000000) != 0))
    {
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        gxSetZMode_(1, GX_LEQUAL, 0);
        gxSetPeControl_ZCompLoc_(1);
        GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    }
    else if ((SHADER_FLAGS(shader) & 0x400) != 0 && (SHADER_FLAGS(shader) & 0x80) == 0)
    {
        GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
        gxSetZMode_(1, GX_LEQUAL, 1);
        gxSetPeControl_ZCompLoc_(0);
        GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_GREATER, 0);
    }
    else
    {
        GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
        gxSetZMode_(1, GX_LEQUAL, 1);
        gxSetPeControl_ZCompLoc_(1);
        GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    }
    if ((SHADER_FLAGS(shader) & 1) != 0 || (SHADER_FLAGS(shader) & 0x40000) != 0 ||
        (SHADER_FLAGS(shader) & 0x800) != 0 || (SHADER_FLAGS(shader) & 0x1000) != 0)
    {
        GXSetChanAmbColor(GX_COLOR0, *(GXColor*)&gTexShaderAmbColor);
        if ((SHADER_FLAGS(shader) & 0x40000) != 0)
        {
            GXSetChanCtrl(GX_COLOR0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        }
        else
        {
            GXSetChanCtrl(GX_COLOR0, GX_ENABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        }
    }
    else
    {
        objGetColor(0, &ambColor[0], &ambColor[1], &ambColor[2]);
        GXSetChanCtrl(GX_COLOR0, GX_ENABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        GXSetChanAmbColor(GX_COLOR0, *(GXColor*)&ambColor[0]);
    }
    if ((SHADER_FLAGS(shader) & 0x8) != 0)
    {
        GXSetCullMode(GX_CULL_BACK);
    }
    else
    {
        GXSetCullMode(GX_CULL_NONE);
    }
    return shader;
}

typedef struct TrackP6Entry
{
    f32 relX0;
    f32 relY0;
    f32 relZ0;
    f32 relX1;
    f32 relY1;
    f32 relZ1;
    f32 relX2;
    f32 relY2;
    f32 relZ2;
} TrackP6Entry;

typedef struct TrackBlockDescriptor
{
    void* object;
    s16 firstTriangle;
    u8 pad06[2];
    void* currentMatrix;
    void* currentCollisionMatrix;
    void* alternateMatrix;
    void* alternateCollisionMatrix;
} TrackBlockDescriptor;

/* TrackTriangle -- the 0x4c-byte collision triangle record packed into
 * gTrackTriangleBuffer.  Plane and edge-plane normals are prebaked f32;
 * vertex coordinates are stored as s16 triplets grouped by axis
 * (x0 x1 x2 / y0 y1 y2 / z0 z1 z2), which the hit-detect code reads both
 * by field and as an s16 index off the record base. */
typedef struct TrackTriangle
{
    f32 planeD;     /* 0x00 plane equation constant */
    f32 planeN[3];  /* 0x04 plane normal xyz */
    s16 vx[3];      /* 0x10 vertex x coords */
    s16 vy[3];      /* 0x16 vertex y coords */
    s16 vz[3];      /* 0x1c vertex z coords */
    u8 pad22[2];    /* 0x22 */
    f32 edgeN0[3];  /* 0x24 edge 0 outward normal */
    f32 edgeN1[3];  /* 0x30 edge 1 outward normal */
    f32 edgeN2[3];  /* 0x3c edge 2 outward normal */
    u8 surfaceType; /* 0x48 copied into intersect-line records */
    s8 flags;       /* 0x49 0x10 = disabled, 0x4 = force */
    u8 minMaxY;     /* 0x4a lo/hi nibble: s16 index (base 0xb) of min/max height */
    u8 edgeOutBits; /* 0x4b per-edge outside bits from last query */
} TrackTriangle;


extern const f32 lbl_803DEC50;
extern u32 gSunFlareScissorX;
extern u32 gSunFlareScissorY;
extern u32 gSunFlareScissorWidth;
extern u32 gSunFlareScissorHeight;
extern u8 lbl_803DCE06;
extern u8 lbl_803DCE98;
extern void* lbl_803DCE80;
extern int gMapBlockIndexCount;
extern int* gMapBlockIndexList;
extern int sSynthFadeUnit;
extern u32 renderFlags;
extern u8 colorScale;
extern f32 gSunFlareFade;
extern int gSunOcclusionSampleOffsets[];
extern f32 lbl_803DEBD4, lbl_803DEBD8, lbl_803DEBDC;
extern f32 lbl_803DEBE4;
extern f32 lbl_803DEC30, lbl_803DEC34, lbl_803DEC38;
extern f32 lbl_803DEC3C, lbl_803DEC40;

void* trackGetBlockDescriptors(u32* outVal);

void mapBlockRender_setVtxDcrs(u8 doSetup, MapBlockData* block, MapShader* shader,
                               ModelRenderInstrsState* state)
{
    int* stateWords;
    u32 val;
    int pos;
    int off;
    u8* p;
    int bit;
    u32 val2;
    int pos2;
    int off2;
    u8* q;
    int bit2;
    u32 val3;
    int pos3;
    int off3;
    u8* r;
    int bit3;
    int i;

    stateWords = (int*)state;
    if (doSetup != 0)
    {
        GXClearVtxDesc();
    }
    pos = stateWords[4];
    off = pos >> 3;
    val = *(u8*)(stateWords[0] + off);
    p = (u8*)stateWords[0] + off;
    val |= p[1] << 8;
    val |= p[2] << 16;
    stateWords[4] = pos + 1;
    bit = (val >> (pos & 7)) & 1;
    if (doSetup != 0)
    {
        GXSetVtxDesc(GX_VA_POS, bit ? GX_INDEX16 : GX_INDEX8);
    }
    pos2 = stateWords[4];
    off2 = pos2 >> 3;
    val2 = *(u8*)(stateWords[0] + off2);
    q = (u8*)stateWords[0] + off2;
    val2 |= q[1] << 8;
    val2 |= q[2] << 16;
    stateWords[4] = pos2 + 1;
    bit2 = (val2 >> (pos2 & 7)) & 1;
    if (doSetup != 0)
    {
        GXSetVtxDesc(GX_VA_CLR0, bit2 ? GX_INDEX16 : GX_INDEX8);
    }
    pos3 = stateWords[4];
    off3 = pos3 >> 3;
    val3 = *(u8*)(stateWords[0] + off3);
    r = (u8*)stateWords[0] + off3;
    val3 |= r[1] << 8;
    val3 |= r[2] << 16;
    stateWords[4] = pos3 + 1;
    bit3 = (val3 >> (pos3 & 7)) & 1;
    if (doSetup != 0)
    {
        if (shader != NULL && (shader->flags & 0x80000000) == 0)
        {
            for (i = 0; i < shader->layerCount; i++)
            {
                GXSetVtxDesc(i + GX_VA_TEX0, bit3 ? GX_INDEX16 : GX_INDEX8);
            }
        }
        else
        {
            GXSetVtxDesc(GX_VA_TEX0, bit3 ? GX_INDEX16 : GX_INDEX8);
        }
    }
}

void setupToRenderMapBlock(MapBlockData* block, void* posMtx);

void setupToRenderMapBlock(MapBlockData* block, void* posMtx)
{
    f32 out[12];
    f32 tmp[12];
    f32 fc;

    GXLoadPosMtxImm((const f32 (*)[4])posMtx, GX_PNMTX0);
    PSMTXCopy((MtxPtr)posMtx, (MtxPtr)tmp);
    fc = lbl_803DEBCC;
    tmp[3] = fc;
    tmp[7] = fc;
    tmp[11] = fc;
    GXLoadNrmMtxImm((const f32 (*)[4])tmp, GX_PNMTX0);
    PSMTXConcat((MtxPtr)lbl_803967F0, (MtxPtr)posMtx, (MtxPtr)out);
    GXLoadTexMtxImm((const f32 (*)[4])out, GX_TEXMTX2, GX_MTX3x4);
    GXSetArray(GX_VA_POS, block->vertices, 6);
    GXSetArray(GX_VA_CLR0, block->vertexColors, 2);
    GXSetArray(GX_VA_TEX0, block->vertexTexCoords, 4);
    GXSetArray(GX_VA_TEX1, block->vertexTexCoords, 4);
}

void renderMapBlock(MapBlockData* block, u8 type)
{
    ModelRenderInstrsState state;
    f32 m[16];
    u16 instructionCount;
    void* instructions;
    MapShader* shader;
    u8 doSetup;
    int done;
    void* viewMtx;

    shader = NULL;
    doSetup = FALSE;
    if (type == 1)
    {
        instructions = block->renderInstrsTransp;
        instructionCount = block->nRenderInstrsTransp;
    }
    else if (type == 2)
    {
        instructions = block->renderInstrsWater;
        instructionCount = block->nRenderInstrsWater;
    }
    else
    {
        instructions = block->renderInstrsMain;
        instructionCount = block->nRenderInstrsMain;
        doSetup = TRUE;
    }
    if (instructionCount == 0)
        return;
    viewMtx = Camera_GetViewMatrix();
    PSMTXConcat((MtxPtr)viewMtx, (MtxPtr)block->transform, (MtxPtr)m);
    if (doSetup)
        setupToRenderMapBlock(block, m);
    modelRenderInstrsState_init(&state, instructions, instructionCount << 3, instructionCount << 3);
    done = FALSE;
    while (!done)
    {
        u32 word;
        int op;
        int pos = state.bit;
        u8* bp;

        bp = state.instrs;
        bp += pos >> 3;
        word = bp[0];
        word |= bp[1] << 8;
        word |= bp[2] << 16;
        state.bit = pos + 4;
        op = (word >> (pos & 7)) & 0xf;
        switch (op)
        {
        case 3:
            mapBlockRender_setVtxDcrs(doSetup, block, shader, &state);
            break;
        case 1:
            shader = mapBlockRender_setShader(doSetup, block, &state);
            break;
        case 2:
            mapBlockRender_callList(doSetup, 0, block, shader, &state, m);
            break;
        case 4:
        {
            u32 word2;
            int cnt;
            int i;
            u8* bp2;
            int pos2 = pos + 4;
            bp2 = state.instrs;
            bp2 += pos2 >> 3;
            word2 = bp2[0];
            word2 |= bp2[1] << 8;
            word2 |= bp2[2] << 16;
            state.bit = pos2 + 4;
            cnt = (word2 >> (pos2 & 7)) & 0xf;
            for (i = 0; i < cnt; i++)
                modelRenderInstrsState_advance(&state, 8);
            break;
        }
        case 5:
            done = TRUE;
            break;
        }
    }
}

void renderGlows(void)
{
    f32 px, py, pz;
    s32 sx, sy, sz;
    u8 amb[3];
    GXColor fogCol;
    f32 sunMtx[12];
    f32 dir[3];
    f32 cam[3];
    void* viewMtx;
    u8 alpha;
    u8 sunAlpha;
    f32 sunDot;
    f32 cx, cy, cz;
    int i;
    ModelLightStruct* e;

    fogCol = *(GXColor*)&sSynthFadeUnit;
    GXSetCullMode(GX_CULL_NONE);
    Camera_RebuildProjectionMatrix();
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    textureSetupFn_800799c0();
    gxTextureFn_800794e0();
    textRenderSetupFn_80079804();
    GXSetFog(GX_FOG_NONE, 0.0f, 0.0f, 0.0f, 0.0f, fogCol);
    gxBlendFn_800789ac();
    alpha = 0xff;
    gSunFlareScissorWidth = 0;
    gSunFlareScissorHeight = 0;
    sunAlpha = skyGetSunRenderAlpha(2);
    if (sunAlpha != 0 && (renderFlags & 0x40))
    {
        viewMtx = Camera_GetViewMatrix();
        skyGetSunLightDirection(0, &dir[0], &dir[1], &dir[2]);
        cam[0] = *(f32*)((char*)viewMtx + 0x20);
        cam[1] = *(f32*)((char*)viewMtx + 0x24);
        cam[2] = *(f32*)((char*)viewMtx + 0x28);
        sunDot = PSVECDotProduct((Vec*)dir, (Vec*)cam);
        if (sunDot > lbl_803DEBCC)
        {
            int occ;
            f32 fade;
            skyBuildSunModelMatrix((f32(*)[4])sunMtx);
            Camera_ProjectWorldPointWithOffset(sunMtx[3], sunMtx[7], sunMtx[11], lbl_803DEBD4, &px, &py, &pz);
            Camera_NdcToScreen(px, py, pz, &sx, &sy, &sz);
            gSunFlareScissorX = sx - 0x10;
            gSunFlareScissorWidth = 0x20;
            gSunFlareScissorY = sy - 0x10;
            gSunFlareScissorHeight = 0x20;
            if ((int)gSunFlareScissorX < 0)
                gSunFlareScissorX = 0;
            else if ((int)gSunFlareScissorX > 0x280)
                gSunFlareScissorX = 0x280;
            if ((int)gSunFlareScissorY < 0)
                gSunFlareScissorY = 0;
            else if ((int)gSunFlareScissorY > 0x1e0)
                gSunFlareScissorY = 0x1e0;
            if ((int)gSunFlareScissorX + 0x20 > 0x280)
                gSunFlareScissorWidth = 0x280 - gSunFlareScissorX;
            if ((int)gSunFlareScissorY + 0x20 > 0x1e0)
                gSunFlareScissorHeight = 0x1e0 - gSunFlareScissorY;
            occ = 0;
            for (i = 0; i < 5; i++)
            {
                int d = depthReadRequestPoll(sx + gSunOcclusionSampleOffsets[i * 2],
                                             sy + gSunOcclusionSampleOffsets[i * 2 + 1], (void*)i);
                if (sz <= d && pauseMenuGetState() == 0)
                    occ++;
            }
            fade = (f32)(u32)occ / lbl_803DEBE4 - gSunFlareFade;
            if (fade > lbl_803DEC30)
                fade = lbl_803DEC30;
            else if (fade < lbl_803DEC34)
                fade = lbl_803DEC34;
            gSunFlareFade = gSunFlareFade + fade;
            sunDot = sunDot * gSunFlareFade;
            if (sunDot > lbl_803DEBCC)
            {
                PSMTXConcat((MtxPtr)viewMtx, (MtxPtr)sunMtx, (MtxPtr)sunMtx);
                GXLoadPosMtxImm((const f32 (*)[4])sunMtx, GX_PNMTX0);
                GXSetCurrentMtx(GX_PNMTX0);
                selectTexture((Texture*)((int)skyGetSkyTexture()), 0);
                getAmbientColor(0, &amb[0], &amb[1], &amb[2]);
                sunDot = (f32)(u32)sunAlpha * sunDot;
                _gxSetTevColor2(amb[0], amb[1], amb[2], (int)(lbl_803DEBFC * sunDot));
                alpha = lbl_803DEBD8 - lbl_803DEC38 * sunDot;
                fade = lbl_803DEC3C * sunDot;
                sunDot = fade * lbl_803DEC40;
                GXBegin(GX_QUADS, GX_VTXFMT2, 4);
                GXWGFifo.f32 = -sunDot;
                GXWGFifo.f32 = -sunDot;
                GXWGFifo.f32 = lbl_803DEBCC;
                GXWGFifo.f32 = lbl_803DEBCC;
                GXWGFifo.f32 = lbl_803DEBCC;
                GXWGFifo.f32 = sunDot;
                GXWGFifo.f32 = -sunDot;
                GXWGFifo.f32 = lbl_803DEBCC;
                GXWGFifo.f32 = lbl_803DEBDC;
                GXWGFifo.f32 = lbl_803DEBCC;
                GXWGFifo.f32 = sunDot;
                GXWGFifo.f32 = sunDot;
                GXWGFifo.f32 = lbl_803DEBCC;
                GXWGFifo.f32 = lbl_803DEBDC;
                GXWGFifo.f32 = lbl_803DEBDC;
                GXWGFifo.f32 = -sunDot;
                GXWGFifo.f32 = sunDot;
                GXWGFifo.f32 = lbl_803DEBCC;
                GXWGFifo.f32 = lbl_803DEBCC;
                GXWGFifo.f32 = lbl_803DEBDC;
            }
        }
    }
    colorScale = alpha;
    if (lbl_803DCE06 != 0)
    {
        for (i = 0; i < lbl_803DCE06; i++)
        {
            int d;
            e = gGlowLightList[i];
            Camera_ProjectWorldPointWithOffset(e->worldX - playerMapOffsetX, e->worldY, e->worldZ - playerMapOffsetZ,
                                               e->glowProjectionRadius, &px, &py, &pz);
            Camera_NdcToScreen(px, py, pz, &sx, &sy, &sz);
            d = depthReadRequestPoll(sx, sy, e);
            if (sz <= d && pauseMenuGetState() == 0)
                e->glowAlphaStep = 0x10;
            else
                e->glowAlphaStep = -0x10;
        }
        GXSetCurrentMtx(GX_IDENTITY);
        gxTextureFn_800794e0();
        gxBlendFn_800789ac();
        for (i = 0; i < lbl_803DCE06; i++)
        {
            e = gGlowLightList[i];
            if (e->glowAlpha != 0)
            {
                f32 f = e->activeIntensity;
                selectTexture((Texture*)((int)e->glowTexture), 0);
                _gxSetTevColor2((int)((f32)(u32)e->glowColor[0] * e->activeIntensity),
                                (int)((f32)(u32)e->glowColor[1] * e->activeIntensity),
                                (int)((f32)(u32)e->glowColor[2] * e->activeIntensity),
                                (u8)((int)(e->glowColor[3] * e->glowAlpha) >> 8));
                GXBegin(GX_QUADS, GX_VTXFMT2, 4);
                cz = e->viewZ;
                cy = e->viewY - e->glowScale;
                cx = e->viewX - e->glowScale;
                GXWGFifo.f32 = cx;
                GXWGFifo.f32 = cy;
                GXWGFifo.f32 = cz;
                GXWGFifo.f32 = lbl_803DEBCC;
                GXWGFifo.f32 = lbl_803DEBCC;
                cz = e->viewZ;
                cy = e->viewY - e->glowScale;
                cx = e->viewX + e->glowScale;
                GXWGFifo.f32 = cx;
                GXWGFifo.f32 = cy;
                GXWGFifo.f32 = cz;
                GXWGFifo.f32 = lbl_803DEBDC;
                GXWGFifo.f32 = lbl_803DEBCC;
                cz = e->viewZ;
                cy = e->viewY + e->glowScale;
                cx = e->viewX + e->glowScale;
                GXWGFifo.f32 = cx;
                GXWGFifo.f32 = cy;
                GXWGFifo.f32 = cz;
                GXWGFifo.f32 = lbl_803DEBDC;
                GXWGFifo.f32 = lbl_803DEBDC;
                cz = e->viewZ;
                cy = e->viewY + e->glowScale;
                cx = e->viewX - e->glowScale;
                GXWGFifo.f32 = cx;
                GXWGFifo.f32 = cy;
                GXWGFifo.f32 = cz;
                GXWGFifo.f32 = lbl_803DEBCC;
                GXWGFifo.f32 = lbl_803DEBDC;
            }
        }
        GXSetCurrentMtx(GX_PNMTX0);
    }
}

void getSunFlareScissorRect(int* outX, int* outY, int* outWidth, int* outHeight)
{
    *outX = gSunFlareScissorX;
    *outY = gSunFlareScissorY;
    *outWidth = gSunFlareScissorWidth;
    *outHeight = gSunFlareScissorHeight;
}

static inline int isGlowInFrustum(ModelLightStruct* light)
{
    FrustumPlane* plane;
    u8 i;
    f32 offsetX;
    f32 offsetZ;
    f32 bias;

    i = 0;
    offsetZ = playerMapOffsetZ;
    offsetX = playerMapOffsetX;
    bias = lbl_803DEBCC;
    for (; i < 5; i++)
    {
        f32 dot;
        plane = &gViewFrustumPlanes[i];
        dot = light->worldY * plane->normalY + plane->normalX * (light->worldX - offsetX) +
                  plane->normalZ * (light->worldZ - offsetZ) + plane->distance + bias;
        if (dot < bias)
        {
            return 0;
        }
    }
    return 1;
}

void queueGlowRender(ModelLightStruct* light)
{
    int visible;
    u8 idx;

    if (lbl_803DCE06 >= 100)
        return;

    visible = isGlowInFrustum(light);
    {
        u8 vis = visible;
        if (vis == 0 && light->glowAlpha == 0)
            return;
        if (vis == 0)
        {
            light->glowAlphaStep = -0x10;
        }
    }
    idx = lbl_803DCE06++;
    gGlowLightList[idx] = light;
}

void trackPackVector(short* out, float* vec)
{
    int yScaled;
    int zScaled;

    yScaled = (int)(lbl_803DEC50 * vec[1]);
    zScaled = (int)(lbl_803DEC50 * vec[2]);
    *out = (short)(int)(lbl_803DEC50 * *vec);
    out[1] = yScaled;
    out[2] = zScaled;
}

void trackUnpackVector(s16* in, f32* out)
{
    out[0] = (f32)(s32)in[0] * lbl_803DEC20;
    out[1] = (f32)(s32)in[1] * lbl_803DEC20;
    out[2] = (f32)(s32)in[2] * lbl_803DEC20;
}

/* fn_80067B84 -- gather model triangles overlapping a swept bbox into the
 * hit-detect triangle buffer at cur (0x4c-byte records); returns advanced
 * cursor. */
u32 trackGetPackedSurfaceType(int* obj)
{
    u32 v = obj[4];
    v &= 0x00FF0000;
    return v >> 16;
}

int mapBlockGetPolygonGroupType(void* obj)
{
    return (*(u32*)&((GameObject*)obj)->anim.localPosY & 0xff000000) >> 24;
}

int mapBlockCountTrianglesByType(MapBlockData* block, int type)
{
    int entry;
    int offset;
    int total;
    int i;
    int count;
    total = 0;
    offset = 0;
    count = block->polyGroupCount;
    for (i = 0; i < count; i++)
    {
        entry = (int)block->polygonGroups + offset;
        if (type == (int)((*(u32*)(entry + 0x10) & 0xff000000) >> 24))
        {
            total += *(u16*)(entry + 0x14) - *(u16*)entry;
        }
        offset += 0x14;
    }
    return total;
}

void* mapBlockGetPolygon(int* obj, int idx)
{
    return (char*)((int**)obj)[0x4c / 4] + idx * 8;
}

void* mapBlockGetPolygonGroup(void* obj, int idx)
{
    return (char*)((int**)obj)[0x50 / 4] + idx * 0x14;
}

void* mapBlockGetEdge(int* obj, int idx)
{
    return (char*)((int**)obj)[0x68 / 4] + idx * 0x1c;
}

MapShader* mapBlockGetShader(MapBlockData* obj, int idx)
{
    return obj->shaders + idx;
}

void MapBlock_initShaders(MapBlockData* block)
{
    int i;
    int j;
    int ref;
    MapShader* sh;
    for (i = 0; i < block->shaderCount; i++)
    {
        sh = &block->shaders[i];
        for (j = 0; j < sh->layerCount; j++)
        {
            ref = sh->layers[j].textureIndex;
            if (ref != -1)
            {
                sh->layers[j].texture = block->textures[ref].texture;
                ref = sh->layers[j].overrideType;
                if ((u32)ref != 0u)
                {
                    mapTextureOverrideAcquire(sh->layers[j].texture, 0, ref);
                }
            }
            else
            {
                sh->layers[j].texture = NULL;
            }
            sh->layers[j].scrollMtx = 0xff;
        }
        ref = sh->auxTextureIndex;
        if (ref != -1)
        {
            sh->auxTexture = block->textures[ref].texture;
        }
        else
        {
            sh->auxTexture = NULL;
        }
    }
}

static inline void* mapBlockRelocatePointer(MapBlockData* block, void* offset)
{
    return (u8*)block + (u32)offset;
}

void MapBlock_init(MapBlockData* block)
{
    int i;

    if (block->textures != NULL)
        block->textures = mapBlockRelocatePointer(block, block->textures);
    if (block->gcPolygons != NULL)
        block->gcPolygons = mapBlockRelocatePointer(block, block->gcPolygons);
    if (block->polygonGroups != NULL)
        block->polygonGroups = mapBlockRelocatePointer(block, block->polygonGroups);
    block->vertices = mapBlockRelocatePointer(block, block->vertices);
    block->vertexColors = mapBlockRelocatePointer(block, block->vertexColors);
    block->vertexTexCoords = mapBlockRelocatePointer(block, block->vertexTexCoords);
    if (block->renderInstrsMain != NULL)
        block->renderInstrsMain = mapBlockRelocatePointer(block, block->renderInstrsMain);
    if (block->renderInstrsTransp != NULL)
        block->renderInstrsTransp = mapBlockRelocatePointer(block, block->renderInstrsTransp);
    if (block->renderInstrsWater != NULL)
        block->renderInstrsWater = mapBlockRelocatePointer(block, block->renderInstrsWater);
    block->displayLists = mapBlockRelocatePointer(block, block->displayLists);
    if (block->shaders != NULL)
        block->shaders = mapBlockRelocatePointer(block, block->shaders);

    for (i = 0; i < block->edgeCount; i++)
    {
        block->displayLists[i].dlist = mapBlockRelocatePointer(block, block->displayLists[i].dlist);
    }
}

void MapBlock_initHits(MapBlockData* block, int index)
{
    int i;
    int* table = (int*)lbl_803DCE80;
    int fileOff = table[index];
    int size = table[index + 1] - fileOff;
    MapHitLine* entry;
    s16 value;

    if (size > 0)
    {
        block->hits = mmAlloc(size, 5, 0);
        fileLoadToBufferOffset(MLDF_FILEID_HITS_BIN, block->hits, fileOff, size);
    }
    block->hitCount = (u32)size / sizeof(MapHitLine);
    i = 0;
    while (i < block->hitCount)
    {
        entry = &block->hits[i];
        if (entry->x[0] < 0 || (value = entry->x[1]) < 0 || entry->x[0] > 0x280 || value > 0x280)
        {
            entry->kind = 0x40;
        }
        entry = &block->hits[i];
        if (entry->z[0] < 0 || (value = entry->z[1]) < 0 || entry->z[0] > 0x280 || value > 0x280)
        {
            entry->kind = 0x40;
        }
        i++;
    }
    block->auxData = NULL;
    block->unk9E = 0;
    block->flags4 &= ~0x40;
}

MapBlockData* MapBlock_loadFromFile(int blockId)
{
    int compressedLen;
    int decompressedSize;
    void* buf;
    int blockOff = 0;
    int* table;
    int tableEntry;
    if (blockId <= gMapBlockIndexCount)
    {
        table = gMapBlockIndexList;
        if (table != 0)
        {
            tableEntry = table[blockId];
            if (tableEntry != -1)
            {
                if (tableEntry != 0 || table[blockId + 1] != 0)
                {
                    blockOff = tableEntry;
                    checkLoadBlock(tableEntry, &compressedLen, &decompressedSize);
                }
                else
                {
                    return 0;
                }
            }
        }
    }
    else
    {
        return 0;
    }
    if (compressedLen <= 0)
    {
        return 0;
    }
    if (decompressedSize > 0x32000)
    {
        return 0;
    }
    buf = mmAlloc(decompressedSize, 5, 0);
    if (buf == 0)
    {
        return 0;
    }
    loadAndDecompressDataFile(MLDF_FILEID_BLOCKS_BIN_A, buf, blockOff, compressedLen, 0, 0, 0);
    return buf;
}

void gxErrorFn_80060b40(void)
{
    int n;
    int i;

    i = 0;
    n = lbl_803DCE98;
    for (; i < n; i++)
    {
    }
}

int return0_80060B90(void* wpad0)
{
    return 0x0;
}

void mapGetBlocks(void** outLayerTables, u32* outBlocks)
{
    *outLayerTables = gMapBlockLayerTables;
    *outBlocks = (u32)gMapBlocks;
}

void mapClearBlockEdgeFlags(void)
{
    int i;
    int j;
    MapBlockData* block;

    for (i = 0; i < lbl_803DCE98; i++)
    {
        block = gMapBlocks[i];
        if (block != NULL)
        {
            for (j = 0; j < block->edgeCount; j++)
            {
                block->displayLists[j].flags = 0;
            }
        }
    }
}

int fn_80060C14(int* obj, int triBuf, void* planesOut, int vertsOut, int p7, f32 offX, f32 offZ, int p8, int kindMask)
{
    int j;
    f32 lm[12];
    u8* descBytes = trackGetBlockDescriptors((u32*)&j);
    u8* end = descBytes + j * 0x18;
    int total;
    int grp;
    int outOff;

    grp = 0;
    outOff = 0;
    j = grp;
    total = 0;
    kindMask = kindMask ? 4 : 8;
    for (; descBytes < end; descBytes += 0x18)
    {
        u32 id = *(u32*)descBytes;
        if (id == 0 || id == *(u32*)&((GameObject*)obj)->anim.parent)
        {
            f32 fx = ((GameObject*)obj)->anim.localPosX;
            f32 fz = ((GameObject*)obj)->anim.localPosZ;
            f32* outA;

            if (id == 0)
            {
                fx -= offX;
                fz -= offZ;
            }
            j = (s16)((TrackBlockDescriptor*)descBytes)->firstTriangle;
            outA = (f32*)((char*)planesOut + outOff);
            while (j < (s16)((TrackBlockDescriptor*)descBytes)[1].firstTriangle && grp < 0x4b0 && total < 0xe10)
            {
                if (kindMask & ((TrackTriangle*)triBuf + j)->flags)
                {
                    ((TrackP6Entry*)vertsOut)->relX0 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vx[0]) - fx;
                    ((TrackP6Entry*)vertsOut)->relY0 =
                        __OSs16tof32(&((TrackTriangle*)triBuf + j)->vy[0]) - ((GameObject*)obj)->anim.localPosY;
                    ((TrackP6Entry*)vertsOut)->relZ0 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vz[0]) - fz;
                    ((TrackP6Entry*)vertsOut)->relX1 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vx[1]) - fx;
                    ((TrackP6Entry*)vertsOut)->relY1 =
                        __OSs16tof32(&((TrackTriangle*)triBuf + j)->vy[1]) - ((GameObject*)obj)->anim.localPosY;
                    ((TrackP6Entry*)vertsOut)->relZ1 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vz[1]) - fz;
                    ((TrackP6Entry*)vertsOut)->relX2 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vx[2]) - fx;
                    ((TrackP6Entry*)vertsOut)->relY2 =
                        __OSs16tof32(&((TrackTriangle*)triBuf + j)->vy[2]) - ((GameObject*)obj)->anim.localPosY;
                    ((TrackP6Entry*)vertsOut)->relZ2 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vz[2]) - fz;
                    outA[0] = ((TrackTriangle*)triBuf + j)->planeN[0];
                    outA[1] = ((TrackTriangle*)triBuf + j)->planeN[1];
                    outA[2] = ((TrackTriangle*)triBuf + j)->planeN[2];
                    *(u8*)((char*)outA + 0x10) = *(u8*)&((TrackTriangle*)triBuf + j)->flags;
                    vertsOut += 0x24;
                    total += 3;
                    outA = (f32*)((char*)outA + 0x14);
                    grp += 1;
                    outOff += 0x14;
                }
                j++;
            }
        }
        else
        {
            f32* m = *(f32**)((char*)descBytes + 0xc);
            f32* p6start;
            int totalStart;
            f32* outA;

            lm[0] = m[0];
            lm[1] = m[4];
            lm[2] = m[8];
            lm[3] = m[12] - ((GameObject*)obj)->anim.localPosX;
            lm[4] = m[1];
            lm[5] = m[5];
            lm[6] = m[9];
            lm[7] = m[13] - ((GameObject*)obj)->anim.localPosY;
            lm[8] = m[2];
            lm[9] = m[6];
            lm[10] = m[10];
            lm[11] = m[14] - ((GameObject*)obj)->anim.localPosZ;
            p6start = (f32*)vertsOut;
            totalStart = total;
            j = (s16)((TrackBlockDescriptor*)descBytes)->firstTriangle;
            outA = (f32*)((char*)planesOut + outOff);
            while (j < (s16)((TrackBlockDescriptor*)descBytes)[1].firstTriangle && grp < 0x4b0 && total < 0xe10)
            {
                if (kindMask & ((TrackTriangle*)triBuf + j)->flags)
                {
                    ((TrackP6Entry*)vertsOut)->relX0 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vx[0]);
                    ((TrackP6Entry*)vertsOut)->relY0 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vy[0]);
                    ((TrackP6Entry*)vertsOut)->relZ0 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vz[0]);
                    ((TrackP6Entry*)vertsOut)->relX1 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vx[1]);
                    ((TrackP6Entry*)vertsOut)->relY1 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vy[1]);
                    ((TrackP6Entry*)vertsOut)->relZ1 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vz[1]);
                    ((TrackP6Entry*)vertsOut)->relX2 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vx[2]);
                    ((TrackP6Entry*)vertsOut)->relY2 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vy[2]);
                    ((TrackP6Entry*)vertsOut)->relZ2 = __OSs16tof32(&((TrackTriangle*)triBuf + j)->vz[2]);
                    outA[0] = ((TrackTriangle*)triBuf + j)->planeN[0];
                    outA[1] = ((TrackTriangle*)triBuf + j)->planeN[1];
                    outA[2] = ((TrackTriangle*)triBuf + j)->planeN[2];
                    *(u8*)((char*)outA + 0x10) = *(u8*)&((TrackTriangle*)triBuf + j)->flags;
                    vertsOut += 0x24;
                    total += 3;
                    outA = (f32*)((char*)outA + 0x14);
                    grp += 1;
                    outOff += 0x14;
                }
                j++;
            }
            if (totalStart < total)
            {
                PSMTXMultVecArray((MtxPtr)lm, (Vec*)p6start, (Vec*)p6start, total - totalStart);
            }
        }
    }
    return grp;
}

u32 lbl_8037E0C0[0x853];
