#include "main/camera_interface.h"
#include "main/dll/dll_0000_gameui_api.h"
#include "main/debug.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "main/vecmath.h"
#include "game/objects/object.h"
#include "main/objprint_api.h"
#include "sys/objects/lifecycle.h"
#include "sys/objects.h"
#include "main/obj_list.h"
#include "main/obj_query.h"
#include "main/objseq.h"
#include "util/carry.h"
#include "string.h"
#include "dolphin/os/OSCache.h"
#include "main/mm.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/printf.h"
#include "main/frame_timing.h"
#include "main/maketex_api.h"
#include "main/maketex_random_api.h"
#include "main/maketex_sequence_api.h"
#include "main/maketex_timer_api.h"
#include "track/intersect_card_api.h"
#include "main/textrender_api.h"
#include "main/objseq_api.h"
#include "main/objanim_update.h"
#include "main/fileio.h"
#include "main/audio/stream_api.h"
#include "main/audio/audio_control_api.h"
#include "dolphin/dvd.h"
#include "main/table_file.h"
#include "main/dll/partfx_interface.h"
#include "main/track_dolphin_api.h"
#include "main/asset_load.h"
#include "main/game_timer_control_api.h"
#include "main/vecmath_distance_api.h"
#include "main/rcp_dolphin_api.h"
#include "main/model.h"
#include "main/render_envfx_api.h"
#include "main/render_sequence_api.h"
#include "main/audio/sfx.h"
#include "game/objects/object_setup.h"
#include "main/camera.h"
#include "main/curve.h"
#include "main/game_ui_interface.h"
#include "main/dll/rom_curve_interface.h"
#include "main/mapEventTypes.h"
#include "main/objtexture.h"
#include "main/resource.h"
#include "main/screen_transition.h"
#include "main/gamebits.h"
#include "main/shader_api.h"
#include "main/sky.h"
#include "main/sky_interface.h"
#include "main/dll/player_api.h"
#include "main/dll/player_status.h"
#include "main/obj_group.h"
#include "main/obj_message.h"
#include "main/pad.h"
#include "main/gamebit_ids.h"
#include "main/mldf_fileid.h"
#include "main/object_transform.h"
#include "main/maketex_yield_api.h"
#include "dolphin/os.h"
#include "main/pi_dolphin_api.h"
#include "main/audio/music_api.h"

extern char sMemoryCardFileNameString[];

volatile u32 gSaveCardState = 0xD;
char* sMemoryCardFileName = sMemoryCardFileNameString;
int lbl_803DB708 = 0x404040FF;

typedef struct SeqRunFlags
{
    u8 useWorldSpace : 1;
} SeqRunFlags;

typedef struct
{
    int key;
    int val;
} SeqSortPair;

#define CARD_RESULT_READY    0
#define CARD_RESULT_IOERROR  -5

void cardSetStatusNoCard2(void);
static inline int maketex_indexOf(int* p, int n, int target)
{
    int i;
    int j;
    i = 0;
    for (j = 0; j < n; j++)
    {
        if (*p++ == target)
        {
            return i;
        }
        i++;
    }
    return -1;
}
void loadMemCardImages(void);
int saveGame(int writeImages);
static inline u64 saveGame_checksum(u64* p, int count)
{
    u64 x[1];
    u16 i[1];
    u64 acc[1];

    x[0] = 0;
    acc[0] = 1;
    for (i[0] = (int)x[0]; (int)i[0] < count; i[0]++)
    {
        x[0] ^= p[i[0]];
        acc[0] += p[i[0]];
    }
    return x[0] ^ (acc[0] + 13);
}


#define MAKETEX_CAMMODE_NPCSPEAK 0x4d /* cameramode DLL dll_004D_cameramodenpcspeak */
#define MAKETEX_CAMMODE_DEFAULT  0x42 /* default gameplay cameramode DLL */

extern s8 seqGlobal3;
extern int gObjSeqTaskTextId;
extern int gObjSeqDeferredTaskTextId;
extern u8 lbl_803DD0F8;
extern f32 lbl_803DD0F4;
extern f32 lbl_803DD0F0;
extern f32 lbl_803DD0EC;
extern s16 lbl_803DD0E8;
extern s16 lbl_803DD0E6;
extern s16 lbl_803DD0E4;
extern f32 lbl_803DD0E0;
extern s16 gObjSeqSlotValues[];
extern GameObject* lbl_803DD07C;
extern u8 lbl_803DD078;
extern u8 lbl_803DD0D9;
extern int gObjSeqSubtitleId;
extern SeqRunFlags lbl_803DD0B4;
extern u8 lbl_803DD124;
extern int gObjSeqPreemptList[][2];
extern void* lbl_803DD0B8;
extern int gObjSeqPreparingStreamSlot;
extern int lbl_803DD064;
extern u64 gSaveCardChecksumHi;
extern u32 gSaveCardChecksumLo;
extern u8* gSaveCardImageBuffer;
extern u64 gSaveCardSerialHi;
extern u8 lbl_803DD059;
extern int gObjSeqStreamResumeOffset;
extern f32 gObjSeqStreamRemainingTime;
extern int gObjSeqTimedStreamSlot;

int saveGameReadSlotCb(u8 idx, int unused, void* dst)
{
    memcpy(dst, (void*)(lbl_803DD044 + idx * 1772 + 2640), 1772);
    return 0;
}

/* Checksums the save buffer, writes it to the memory card, then reads it
 * back and verifies the checksum. */
int saveGame_doWrite(int slot)
{
    u64 x[1];
    u16 i[1];
    u64* p;
    u64 a[1];
    u64 chk;
    u64 chk2;
    int result;
    int offset;

    p = (u64*)lbl_803DD044;
    x[0] = 0;
    a[0] = 1;
    for (i[0] = (int)x[0]; (int)i[0] < 0x3ff; i[0]++)
    {
        x[0] = x[0] ^ p[i[0]];
        a[0] = a[0] + p[i[0]];
    }
    chk = x[0] ^ (a[0] + 13);
    ((u32*)p)[0x7ff] = (u32)chk;
    ((u32*)p)[0x7fe] = (u32)(chk >> 32);
    DCFlushRange((void*)lbl_803DD044, 0x2000);
    result = CARDWrite(&gSaveCardFileInfo.fileInfo, (void*)lbl_803DD044, 0x2000, offset = (u8)slot << 13);
    if (result == -5)
    {
        CARDDelete(0, sMemoryCardFileName);
    }
    if (result == 0)
    {
        DCInvalidateRange((void*)lbl_803DD044, 0x2000);
        result = CARDRead(&gSaveCardFileInfo.fileInfo, (void*)lbl_803DD044, 0x2000, offset);
        if (result == 0)
        {
            u64 x2[1];
            u64 a2[1];
            p = (u64*)lbl_803DD044;
            x2[0] = 0;
            a2[0] = 1;
            for (i[0] = (int)x2[0]; (int)i[0] < 0x3ff; i[0]++)
            {
                x2[0] = x2[0] ^ p[i[0]];
                a2[0] = a2[0] + p[i[0]];
            }
            chk2 = x2[0] ^ (a2[0] + 13);
            if (chk != chk2)
            {
                result = -0x55;
                gSaveCardState = 10;
            }
            else
            {
                gSaveCardChecksumHi = chk2;
            }
        }
    }
    return result;
}

/* Saves the game: verifies the existing save slots' checksums, rewrites
 * stale slots and card images, then runs the caller's callback and maps the
 * result to a status code. */
int saveGame_prepareAndWrite(int writeImages, int cbA, int cbB, void* cbC, void* cbD, SaveGameCallback cb)
{
    u64 chk;
    u64 chk2;
    u64 c;
    u64 t;
    int result;
    void* m;

    m = mmAlloc(0x2000, -1, 0);
    lbl_803DD044 = (char*)m;
    if (m == NULL)
    {
        gSaveCardState = 8;
        return 0;
    }
    if (saveGame(writeImages) == 0)
    {
        mm_free((void*)lbl_803DD044);
        lbl_803DD044 = 0;
        return 0;
    }
    DCInvalidateRange((void*)lbl_803DD044, 0x2000);
    result = CARDRead(&gSaveCardFileInfo.fileInfo, (void*)lbl_803DD044, 0x2000, 0x2000);
    if (result == CARD_RESULT_READY)
    {
        c = saveGame_checksum((u64*)lbl_803DD044, 0x3ff);
        chk = c;
        if (c != *(u64*)(lbl_803DD044 + 0x1ff8))
        {
            DCInvalidateRange((void*)lbl_803DD044, 0x2000);
            result = CARDRead(&gSaveCardFileInfo.fileInfo, (void*)lbl_803DD044, 0x2000, 0x4000);
            if (result == CARD_RESULT_READY)
            {
                c = saveGame_checksum((u64*)lbl_803DD044, 0x3ff);
                chk = c;
                if (c == *(u64*)(lbl_803DD044 + 0x1ff8))
                {
                    result = saveGame_doWrite(1);
                }
                else
                {
                    result = -0x55;
                    gSaveCardState = 10;
                }
            }
        }
    }
    if (result == 0)
    {
        if (lbl_803DD059 != 0)
        {
            if (gSaveCardChecksumHi != 0)
            {
                if (chk != gSaveCardChecksumHi)
                {
                    result = -0x55;
                    gSaveCardState = 0xb;
                }
            }
            else
            {
                gSaveCardChecksumLo = (u32)chk;
                *(u32*)&gSaveCardChecksumHi = (u32)(chk >> 32);
            }
        }
        else
        {
            gSaveCardChecksumLo = (u32)chk;
            *(u32*)&gSaveCardChecksumHi = (u32)(chk >> 32);
        }
    }
    if (result == 0)
    {
        m = gSaveCardImageBuffer = mmAlloc(0x4000, -1, 0);
        if (m == NULL)
        {
            if (lbl_803DD05A != 0)
            {
                lbl_803DD05A = 0;
                CARDClose(&gSaveCardFileInfo.fileInfo);
            }
            CARDUnmount(0);
            mm_free(lbl_803DD040);
            lbl_803DD040 = NULL;
            mm_free((void*)lbl_803DD044);
            lbl_803DD044 = 0;
            gSaveCardState = 8;
            return 0;
        }
        result = CARDRead(&gSaveCardFileInfo.fileInfo, m, 0x2000, 0);
        if (result == CARD_RESULT_READY)
        {
            chk2 = saveGame_checksum((u64*)gSaveCardImageBuffer, 0x400);
            if (chk2 != *(u64*)(lbl_803DD044 + 0xa40))
            {
                if ((u8)writeImages != 0)
                {
                    result = -4;
                    gSaveCardState = 0xc;
                }
                else
                {
                    memset(gSaveCardImageBuffer, 0, 0x4000);
                    loadMemCardImages();
                    result = CARDWrite(&gSaveCardFileInfo.fileInfo, gSaveCardImageBuffer, 0x2000, 0);
                    if (result == CARD_RESULT_IOERROR)
                    {
                        CARDDelete(0, sMemoryCardFileName);
                    }
                    if (result == CARD_RESULT_READY)
                    {
                        t = *(u64*)(gSaveCardImageBuffer + 0x2a40);
                        if (t != *(u64*)(lbl_803DD044 + 0xa40))
                        {
                            int writeResult;
                            *(u64*)(lbl_803DD044 + 0xa40) = t;
                            writeResult = saveGame_doWrite(2);
                            if (writeResult == 0)
                            {
                                writeResult = saveGame_doWrite(1);
                            }
                            result = writeResult;
                        }
                    }
                }
            }
        }
        mm_free(gSaveCardImageBuffer);
    }
    if (result == 0 && cb != NULL)
    {
        result = cb(cbA, cbB, cbC, cbD);
    }
    if (lbl_803DD05A != 0)
    {
        lbl_803DD05A = 0;
        CARDClose(&gSaveCardFileInfo.fileInfo);
    }
    CARDUnmount(0);
    mm_free(lbl_803DD040);
    lbl_803DD040 = NULL;
    mm_free((void*)lbl_803DD044);
    lbl_803DD044 = 0;
    switch (result)
    {
    case -5:
        gSaveCardState = 4;
        break;
    case 0:
        gSaveCardState = 0xd;
        return 1;
    case -4:
        break;
    }
    return 0;
}

/* Builds the memory card comment strings (Shift-JIS title on JP cards),
 * loads the banner/icon images from disc, and checksums both halves of the
 * card image buffer. */
void loadMemCardImages(void)
{
    char* names = sMemoryCardFileNameString;
    DVDFileInfo fi;
    u64* p;
    u16 i[1];
    u64 x[1];
    u64* q;
    u64 a[1];
    u64 chk;
    u64 x2[1];
    u64 a2[1];

    a[0] = 0;
    if (lbl_803DC968 != 0)
    {
        gSaveCardImageBuffer[0x00] = 0x83;
        gSaveCardImageBuffer[0x01] = 0x58;
        gSaveCardImageBuffer[0x02] = 0x83;
        gSaveCardImageBuffer[0x03] = 0x5e;
        gSaveCardImageBuffer[0x04] = 0x81;
        gSaveCardImageBuffer[0x05] = 0x5b;
        gSaveCardImageBuffer[0x06] = 0x83;
        gSaveCardImageBuffer[0x07] = 0x74;
        gSaveCardImageBuffer[0x08] = 0x83;
        gSaveCardImageBuffer[0x09] = 0x48;
        gSaveCardImageBuffer[0x0a] = 0x83;
        gSaveCardImageBuffer[0x0b] = 0x62;
        gSaveCardImageBuffer[0x0c] = 0x83;
        gSaveCardImageBuffer[0x0d] = 0x4e;
        gSaveCardImageBuffer[0x0e] = 0x83;
        gSaveCardImageBuffer[0x0f] = 0x58;
        gSaveCardImageBuffer[0x10] = 0x83;
        gSaveCardImageBuffer[0x11] = 0x41;
        gSaveCardImageBuffer[0x12] = 0x83;
        gSaveCardImageBuffer[0x13] = 0x68;
        gSaveCardImageBuffer[0x14] = 0x83;
        gSaveCardImageBuffer[0x15] = 0x78;
        gSaveCardImageBuffer[0x16] = 0x83;
        gSaveCardImageBuffer[0x17] = 0x93;
        gSaveCardImageBuffer[0x18] = 0x83;
        gSaveCardImageBuffer[0x19] = 0x60;
        gSaveCardImageBuffer[0x1a] = 0x83;
        gSaveCardImageBuffer[0x1b] = 0x83;
        gSaveCardImageBuffer[0x1c] = 0x81;
        gSaveCardImageBuffer[0x1d] = 0x5b;
        gSaveCardImageBuffer[0x1e] = 0x00;
        gSaveCardImageBuffer[0x1f] = 0x00;
        sprintf((char*)(gSaveCardImageBuffer + 0x20), names + 0xa0);
    }
    else
    {
        sprintf((char*)gSaveCardImageBuffer, names);
        sprintf((char*)(gSaveCardImageBuffer + 0x20), names + 0xb4);
    }
    if (DVDOpen(names + 0xc4, &fi))
    {
        DVDRead(&fi, gSaveCardImageBuffer + 0x40, 0x1800, 0x20);
        DVDClose(&fi);
    }
    if (DVDOpen(names + 0xd0, &fi))
    {
        DVDRead(&fi, gSaveCardImageBuffer + 0x1840, 0x400, 0);
        DVDClose(&fi);
    }
    if (DVDOpen(names + 0xe8, &fi))
    {
        DVDRead(&fi, gSaveCardImageBuffer + 0x1c40, 0x400, 0);
        DVDClose(&fi);
    }
    if (DVDOpen(names + 0x100, &fi))
    {
        DVDRead(&fi, gSaveCardImageBuffer + 0x2040, 0x400, 0);
        DVDClose(&fi);
    }
    if (DVDOpen(names + 0x118, &fi))
    {
        DVDRead(&fi, gSaveCardImageBuffer + 0x2440, 0x400, 0);
        DVDClose(&fi);
    }
    if (DVDOpen(names + 0x130, &fi))
    {
        DVDRead(&fi, gSaveCardImageBuffer + 0x2840, 0x200, 0);
        DVDClose(&fi);
    }
    p = (u64*)gSaveCardImageBuffer;
    x[0] = 0;
    a[0] = 1;
    for (i[0] = (int)x[0]; (int)i[0] < 0x400; i[0]++)
    {
        x[0] = x[0] ^ p[i[0]];
        a[0] = a[0] + p[i[0]];
    }
    chk = x[0] ^ (a[0] + 13);
    ((u32*)p)[0xa91] = (u32)chk;
    ((u32*)p)[0xa90] = (u32)(chk >> 32);
    q = (u64*)gSaveCardImageBuffer;
    p = q + 0x400;
    x2[0] = 0;
    a2[0] = 1;
    for (i[0] = (int)x2[0]; (int)i[0] < 0x3ff; i[0]++)
    {
        x2[0] = x2[0] ^ p[i[0]];
        a2[0] = a2[0] + p[i[0]];
    }
    chk = x2[0] ^ (a2[0] + 13);
    ((u32*)q)[0xfff] = (u32)chk;
    ((u32*)q)[0xffe] = (u32)(chk >> 32);
    DCFlushRange(gSaveCardImageBuffer, 0x4000);
}

#define CARD_RESULT_UNLOCKED 1
#define CARD_RESULT_NOCARD   -3
#define CARD_RESULT_NOFILE   -4
#define CARD_RESULT_BROKEN   -6
#define CARD_RESULT_NOENT    -8
#define CARD_RESULT_INSSPACE -9
#define CARD_RESULT_ENCODING -13

/* Mounts the memory card, validates its serial number, opens or creates the
 * save file (writing the card image buffer for a fresh file), and maps any
 * CARD error to a status code. */
int saveGame(int writeImages)
{
    u8 created;
    u8 fresh;
    int result;
    int ok;
    int ret;
    u64 serial;
    CARDStat stat;
    void* m;

    created = 0;
    fresh = 0;
    if (cardProbe(0) == 0)
    {
        ok = 0;
    }
    else
    {
        if ((lbl_803DD040 = mmAlloc(0xa000, -1, 0)) == NULL)
        {
            gSaveCardState = 8;
            ok = 0;
        }
        else
        {
            ok = 1;
        }
    }
    if (ok == 0)
    {
        return 0;
    }
    gSaveCardState = 0;
    result = CARDMount(0, lbl_803DD040, (CARDCallback)cardSetStatusNoCard2);
    if (result == CARD_RESULT_BROKEN)
    {
        result = CARDCheck(0);
    }
    if (result == CARD_RESULT_READY || result == CARD_RESULT_ENCODING)
    {
        int err;
        result = CARDCheck(0);
        err = CARDGetSerialNo(0, &serial);
        if (err == CARD_RESULT_READY)
        {
            if (lbl_803DD059 != 0)
            {
                if (gSaveCardSerialHi != 0)
                {
                    if (serial != gSaveCardSerialHi)
                    {
                        result = -0x55;
                        gSaveCardState = 0xb;
                    }
                }
                else
                {
                    gSaveCardSerialHi = serial;
                }
            }
            else
            {
                gSaveCardSerialHi = serial;
            }
        }
        else
        {
            result = err;
        }
    }
    if (result == CARD_RESULT_READY)
    {
        result = CARDOpen(0, sMemoryCardFileName, &gSaveCardFileInfo.fileInfo);
        if (result == CARD_RESULT_NOFILE && (u8)writeImages == 0)
        {
            created = 1;
            fresh = 1;
        }
        if (result == CARD_RESULT_READY)
        {
            lbl_803DD05A = 1;
        }
    }
    if (result == CARD_RESULT_READY)
    {
        result = CARDGetStatus(0, gSaveCardFileInfo.fileInfo.fileNo, &stat);
        if (result == CARD_RESULT_READY)
        {
            if (stat.iconAddr == 0xffffffff || stat.commentAddr == 0xffffffff)
            {
                if ((u8)writeImages != 0)
                {
                    result = CARD_RESULT_NOFILE;
                }
                else
                {
                    fresh = 1;
                }
            }
        }
    }
    if (fresh != 0)
    {
        m = mmAlloc(0x4000, -1, 0);
        gSaveCardImageBuffer = m;
        if (m != NULL)
        {
            memset(m, 0, 0x4000);
            loadMemCardImages();
        }
        else
        {
            gSaveCardState = 8;
            CARDUnmount(0);
            mm_free(lbl_803DD040);
            lbl_803DD040 = NULL;
            return 0;
        }
    }
    if (created != 0)
    {
        result = CARDCreate(0, sMemoryCardFileName, 0x6000, &gSaveCardFileInfo.fileInfo);
    }
    if (fresh != 0)
    {
        if (result == CARD_RESULT_READY)
        {
            result = CARDWrite(&gSaveCardFileInfo.fileInfo, gSaveCardImageBuffer, 0x4000, 0);
            if (result == CARD_RESULT_READY)
            {
                result = CARDWrite(&gSaveCardFileInfo.fileInfo, gSaveCardImageBuffer + 0x2000, 0x2000, 0x4000);
            }
            if (result == CARD_RESULT_IOERROR)
            {
                CARDDelete(0, sMemoryCardFileName);
            }
            if (created != 0 && result == CARD_RESULT_READY)
            {
                result = CARDGetStatus(0, gSaveCardFileInfo.fileInfo.fileNo, &stat);
            }
            if (result == CARD_RESULT_READY)
            {
                stat.commentAddr = 0;
                stat.bannerFormat = (stat.bannerFormat & ~0x3) | 2;
                stat.iconAddr = 0x40;
                stat.bannerFormat = (stat.bannerFormat & ~0x4) | 4;
                stat.iconFormat = (stat.iconFormat & ~0x3) | 1;
                stat.iconSpeed = (stat.iconSpeed & ~0x3) | 3;
                stat.iconFormat = (stat.iconFormat & ~0xc) | 4;
                stat.iconSpeed = (stat.iconSpeed & ~0xc) | 0xc;
                stat.iconFormat = (stat.iconFormat & ~0x30) | 0x10;
                stat.iconSpeed = (stat.iconSpeed & ~0x30) | 0x30;
                stat.iconFormat = (stat.iconFormat & ~0xc0) | 0x40;
                stat.iconSpeed = (stat.iconSpeed & ~0xc0) | 0xc0;
                stat.iconSpeed = stat.iconSpeed & ~0x300;
                result = CARDSetStatus(0, gSaveCardFileInfo.fileInfo.fileNo, &stat);
                if (result == CARD_RESULT_READY)
                {
                    gSaveCardChecksumHi = *(u64*)(gSaveCardImageBuffer + 0x3ff8);
                }
            }
        }
        mm_free(gSaveCardImageBuffer);
    }
    switch (result)
    {
    case CARD_RESULT_READY:
        if (fresh != 0)
        {
            return 1;
        }
        return 2;
    case CARD_RESULT_UNLOCKED:
        gSaveCardState = 1;
        ret = 0;
        break;
    case CARD_RESULT_NOCARD:
        if ((int)gSaveCardState != 3)
        {
            gSaveCardState = 2;
        }
        ret = 0;
        break;
    case CARD_RESULT_NOFILE:
        gSaveCardState = 0xc;
        ret = 0;
        break;
    case CARD_RESULT_IOERROR:
        gSaveCardState = 4;
        ret = 0;
        break;
    case CARD_RESULT_BROKEN:
        gSaveCardState = 5;
        ret = 0;
        break;
    case CARD_RESULT_ENCODING:
        gSaveCardState = 6;
        ret = 0;
        break;
    case CARD_RESULT_NOENT:
    case CARD_RESULT_INSSPACE:
        gSaveCardState = 9;
        ret = 0;
        break;
    case -0x55:
        ret = 0;
        break;
    default:
        ret = 0;
        break;
    }
    if (lbl_803DD05A != 0)
    {
        lbl_803DD05A = 0;
        CARDClose(&gSaveCardFileInfo.fileInfo);
    }
    CARDUnmount(0);
    mm_free(lbl_803DD040);
    lbl_803DD040 = NULL;
    return ret;
}

void cardSetStatusNoCard2(void)
{
    gSaveCardState = 0x3;
}

int arrayRemoveUnordered(int* array, int* count, int value)
{
    int i;
    int len;
    len = *count;
    i = maketex_indexOf(array, len, value);
    if (i == -1)
        return -1;
    array[i] = array[len - 1];
    (*count)--;
    return i;
}

int arrayIndexOf(int* arr, int count, int target)
{
    int idx = 0;
    int i;
    for (i = 0; i < count; i++)
    {
        int elem = *arr;
        arr++;
        if (elem == target)
            return idx;
        idx++;
    }
    return -1;
}

static inline int seqPairKey(SeqSortPair* pair)
{
    return pair->key;
}

static inline int seqPairVal(SeqSortPair* pair)
{
    return pair->val;
}

void seqPairTableSort(SeqSortPair* arr, int n)
{
    int key;
    int val;
    int limit;
    int i;
    int j;
    int gap;

    gap = 1;
    limit = (n - 1) / 9;
    while (gap <= limit)
    {
        gap = gap * 3 + 1;
    }
    for (; gap > 0; gap /= 3)
    {
        for (i = gap + 1; i < n; i++)
        {
            key = seqPairKey(&arr[i]);
            val = seqPairVal(&arr[i]);
            j = i;
            while (j > gap && arr[j - gap].key > key)
            {
                arr[j].key = arr[j - gap].key;
                arr[j].val = arr[j - gap].val;
                j -= gap;
            }
            arr[j].key = key;
            arr[j].val = val;
        }
    }
    for (i = 1; i < n; i++)
    {
    }
}
void seqPairTableSort(SeqSortPair* arr, int n);

int seqPairTableLookup(void* entries, int count, int key)
{
    int (*arr)[2] = entries;
    int lo, mid;
    int i;
    if (count <= 16)
    {
        for (i = 0; i != count; i++)
        {
            if ((*arr)[0] == key)
                return (*arr)[1];
            arr++;
        }
        return 0;
    }
    lo = 0;
    do
    {
        mid = (count + lo) >> 1;
        if (key > arr[mid][0])
        {
            lo = mid;
        }
        else if (key == arr[mid][0])
        {
            return arr[mid][1];
        }
        else
        {
            count = mid;
        }
    } while (count <= lo);
    return 0;
}

/* Spin-delay then sort when the pair list is large enough. */
void seqPairTablePrepare(void* entries, int n)
{
    SeqSortPair* arr = entries;
    int i;
    int j;

    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
        }
    }
    if (n > 0x10)
    {
        seqPairTableSort(arr, n);
    }
}

int randomChanceOneIn(int n)
{
    return randomGetRange(0, n * 60 / 60) == 0;
}

int timerIsActive(const f32* p)
{
    return 0.0f != *p;
}

void storeZeroToFloatParam(f32* p)
{
    *p = 0.0f;
}

void s16toFloat(f32* p, s16 val)
{
    *p = (f32)val;
}


int timerCountDown(f32* p)
{
    f32 timer = *p;
    f32 zero = 0.0f;
    if (timer != zero)
    {
        *p = timer - timeDelta;
        if (*p <= zero)
        {
            *p = zero;
            return 1;
        }
    }
    return 0;
}

void seqClearTaskTexts(void)
{
    u32 v = -0x1;
    gObjSeqTaskTextId = v;
    gObjSeqDeferredTaskTextId = v;
}

void clearCurSeqNo(void)
{
    curSeqNo = 0x0;
}

int getCurSeqNo(void)
{
    return curSeqNo;
}

void ObjSeq_SetCameraTransformOverride(f32 x, f32 y, s16 rx, s16 ry, s16 rz, f32 z, f32 w)
{
    lbl_803DD0F8 = 1;
    lbl_803DD0F4 = x;
    lbl_803DD0F0 = y;
    lbl_803DD0EC = z;
    lbl_803DD0E8 = rx;
    lbl_803DD0E6 = ry;
    lbl_803DD0E4 = rz;
    lbl_803DD0E0 = w;
}
GameObject* getFocusedNpc(void)
{
    return focusedNpc;
}

void ObjSeq_ClearModelLookVector(GameObject* obj);

/* Starts the prepared audio stream for a sequence slot and records its
 * subtitle timing. */
int ObjSeq_StartPreparedStream(int slot)
{
    int seqId = gObjSeqSlotSeqIdTable[slot] - 1;
    f32 streamTime;

    if (gObjSeqStreamSuppressed != 0 || AudioStream_IsPreparing() != 0)
    {
        return 0;
    }
    streamTime = gObjSeqSlotStreamTimeTable[slot] - (f32)gObjSeqStreamResumeOffset;
    gObjSeqStreamRemainingTime = streamTime;
    if (0.0f != gObjSeqStreamRemainingTime)
    {
        gObjSeqTimedStreamSlot = slot;
    }
    gObjSeqStreamResumeOffset = -1;
    if (seqId == 0x54b || seqId == 0x550 || seqId == 0x551 || seqId == 0x574 || seqId == 0x579 || seqId == 0x57a)
    {
        gObjSeqStreamRemainingTime = 0.0f;
        gObjSeqTimedStreamSlot = -1;
    }
    gObjSeqPreparingStreamSlot = -1;
    AudioStream_StartPrepared();
    return 1;
}

int animatedObjGetSeqId(ObjAnimUpdateState* state)
{
    return gObjSeqSlotSeqIdTable[state->sequenceSlot] - 1;
}

int ObjSeq_SetSlotValue(ObjAnimUpdateState* state, int value)
{
    gObjSeqSlotValues[(s8)state->sequenceSlot] = (s16)value;
    return 1;
}

void ObjSeq_AudioStreamCallback(void)
{
    AudioStream_IsPreparing();
    doNothing_8000CF54(0);
    if (gObjSeqDeferredTaskTextId != -1)
    {
        gameTextLoadTaskText(gObjSeqDeferredTaskTextId);
        gObjSeqDeferredTaskTextId = -1;
        gObjSeqTaskTextId = -1;
    }
    else if (gObjSeqSubtitleId != -1)
    {
        subtitleFn_8001b700();
        subtitleStart(gObjSeqSubtitleId);
        gObjSeqSubtitleId = -1;
    }
}

int ObjSeq_SetCoordinateSpace(int unused, int space)
{
    switch (space)
    {
    case 0:
        lbl_803DD0B4.useWorldSpace = 1;
        break;
    case 1:
        lbl_803DD0B4.useWorldSpace = 0;
        break;
    }
    return 0;
}

int ObjSeq_setOverridePos(f32 x, f32 y, f32 z)
{
    lbl_803DD0D9 = 1;
    objSeqOverridePos[0] = x;
    objSeqOverridePos[1] = y;
    objSeqOverridePos[2] = z;
    return 1;
}

int ObjSeq_SetObjs(int objs, int arg, int flags)
{
    u8 flagsByte = (u8)flags;
    objSeqObjs = objs;
    lbl_803DD07C = (GameObject*)arg;
    lbl_803DD078 = flagsByte;
    return 1;
}

void cameraFocusNpc(int param1, GameObject* obj)
{
    struct
    {
        f32 vec[3];
        u8 tag;
    } buf;
    ObjHitVolumeRuntimeTransform* hitTransform;

    if ((*gCameraInterface)->getMode() == MAKETEX_CAMMODE_NPCSPEAK)
        return;
    focusedNpc = obj;
    hitTransform = obj->anim.hitVolumeTransforms;
    if (hitTransform == NULL || param1 == 7 || param1 == 6)
    {
        buf.vec[0] = obj->anim.worldPosX;
        buf.vec[1] = obj->anim.worldPosY;
        buf.vec[2] = obj->anim.worldPosZ;
    }
    else
    {
        buf.vec[0] = hitTransform->jointX;
        buf.vec[1] = hitTransform->jointY;
        buf.vec[2] = hitTransform->jointZ;
    }
    buf.tag = (u8)param1;
    (*gCameraInterface)->setMode(MAKETEX_CAMMODE_NPCSPEAK, 1, 0, 0x10, buf.vec, 0, 0xff);
}

void ObjSeq_ClearModelLookVector(GameObject* obj)
{
    s16* v = objModelGetVecFn_800395d8(obj, 0);
    if (v != NULL)
    {
        v[1] = 0;
        v[0] = 0;
    }
}

/* Object-sequence turn-to-face-player step: starts (mode 4) or advances
 * (mode 5) a smooth turn of the object toward the player, blending the model
 * vector and animation as it goes. */
int ObjSeq_TurnToFacePlayer(GameObject* obj, ObjSeqState* state, s16 turnDegrees, s16 yawThreshold,
                            s16 maxAngle, s16 animRight, s16 animLeft)
{
    GameObject* player;
    s16* modelVec;
    int yawd;
    s16 turn;
    int mode;
    f32 out;
    f32 delta[3];
    f32 dist;
    f32 rate;
    f32 yaw;

    player = Obj_GetPlayerObject();
    yawThreshold = (s16)(182.04445f * yawThreshold);
    maxAngle = (s16)(182.04445f * maxAngle);
    turnDegrees = (s16)(182.04445f * turnDegrees);
    mode = (s8)state->movementState;
    if (mode == 4)
    {
        state->flags = state->flags & ~2;
        modelVec = objModelGetVecFn_800395d8(obj, 0);
        if (modelVec != NULL)
        {
            state->flags = state->flags & ~8;
        }
        state->freeCallback = (ObjAnimSequenceFreeCallback)ObjSeq_ClearModelLookVector;
        state->posOffsetX = 0.0f;
        state->posOffsetY = 0.0f;
        state->posOffsetZ = 0.0f;
        yawd = Obj_GetYawDeltaToObject(obj, player, (float*)0);
        if (((s16)yawd >= 0 ? (s16)yawd : -(s16)yawd) < yawThreshold)
        {
            turn = 0;
        }
        else
        {
            turn = (s16)((s16)yawd > 0 ? (s16)yawd - yawThreshold : (s16)yawd + yawThreshold);
        }
        state->rotOffsetX = turn;
        {
            f32* dp = delta;
            ObjHitVolumeRuntimeTransform* ovr = obj->anim.hitVolumeTransforms;
            if (ovr == NULL)
            {
                dp[0] = player->anim.localPosX - obj->anim.localPosX;
                dp[1] = player->anim.localPosY - obj->anim.localPosY;
                dp[2] = player->anim.localPosZ - obj->anim.localPosZ;
            }
            else
            {
                dp[0] = player->anim.localPosX - ovr->jointX;
                dp[1] = player->anim.localPosY - ovr->jointY;
                dp[2] = player->anim.localPosZ - ovr->jointZ;
            }
            dp[1] += 30.0f;
            dist = sqrtf(dp[0] * dp[0] + dp[2] * dp[2]);
            state->rotOffsetY = (s16)getAngle(dp[1], dist);
        }
        state->rotOffsetZ = 0;
        state->movementState = 5;
        state->posOffsetScale = 0.0f;
        if (turn != 0)
        {
            rate = (f32)turnDegrees / (f32)turn;
            state->posOffsetDecay = rate >= 0.0f ? rate : -rate;
        }
        else
        {
            state->posOffsetDecay = 1.0f;
        }
        {
            f32 c = state->posOffsetDecay;
            state->posOffsetDecay = c < 0.0f ? 0.0f : (c > 0.25f ? 0.25f : c);
        }
        if (animRight != -1)
        {
            if (animLeft != -1)
            {
                state->flags = state->flags & ~4;
                if (state->rotOffsetX < 0)
                {
                    if (animLeft != -1)
                    {
                        ObjAnim_SetCurrentMove((int)obj, animLeft, 0.0f, 0);
                    }
                }
                else
                {
                    if (animRight != -1)
                    {
                        ObjAnim_SetCurrentMove((int)obj, animRight, 0.0f, 0);
                    }
                }
            }
        }
        state->freeCallback = (ObjAnimSequenceFreeCallback)ObjSeq_ClearModelLookVector;
        return 1;
    }
    else if (mode == 5)
    {
        state->posOffsetScale = state->posOffsetScale + state->posOffsetDecay;
        if (state->posOffsetScale > 1.0f)
        {
            state->posOffsetScale = 1.0001f;
        }
        obj->anim.rotX +=
            (s16)(state->posOffsetDecay * (f32)state->rotOffsetX);
        modelVec = objModelGetVecFn_800395d8(obj, 0);
        if (modelVec != NULL)
        {
            state->flags = state->flags & ~8;
            yawd = Obj_GetYawDeltaToObject(obj, player, (float*)0);
            yaw = (f32)(s16)yawd;
            {
                f32 cur = (f32)modelVec[1];
                yaw = cur * (1.0f - state->posOffsetScale) + yaw * state->posOffsetScale;
            }
            yaw = (yaw < (f32)-maxAngle) ? (f32)-maxAngle : ((yaw > (f32)maxAngle) ? (f32)maxAngle : yaw);
            modelVec[1] = yaw;
            modelVec[0] = (f32)state->rotOffsetY * state->posOffsetScale;
        }
        if (animRight != -1)
        {
            if (animLeft != -1)
            {
                s16 t50 = state->rotOffsetX;
                f32 fa = (f32)(t50 >= 0 ? t50 : -t50);
                fa = fa * 3.142f / 325767.0f;
                ObjAnim_SampleRootCurvePhase(&obj->anim, fa, &out);
                ObjAnim_AdvanceCurrentMove((int)obj, out, (f32)framesThisStep, NULL);
            }
        }
        if (state->posOffsetScale > 1.0f)
        {
            state->movementState = 0;
            state->flags = state->flags | 8;
            modelVec = objModelGetVecFn_800395d8(obj, 0);
            if (modelVec != NULL)
            {
                state->baseRotY = modelVec[1];
                state->baseRotX = modelVec[0];
            }
            else
            {
                state->baseRotY = 0;
                state->baseRotX = 0;
            }
            if (state->posOffsetScale > 1.0f)
            {
                state->flags = state->flags | 4;
            }
        }
        return 1;
    }
    return 0;
}


/* Shell sort over (key, val) pairs, ascending by key. */
void ObjSeq_setGlobal2(s16 x)
{
    seqGlobal2 = x;
}
s16 ObjSeq_getGlobal2(void)
{
    return seqGlobal2;
}
void ObjSeq_setGlobal1(s16 x)
{
    seqGlobal1 = x;
}
s16 ObjSeq_getGlobal1(void)
{
    return seqGlobal1;
}
void ObjSeq_setGlobal3(s8 x)
{
    seqGlobal3 = x;
}

u8 ObjSeq_getGlobal3(void)
{
    return seqGlobal3;
}

void ObjSeq_yield(ObjSeqState* seq, int value)
{
    seq->savedFrame = value;
    seq->sequenceControlFlags |= OBJSEQ_CONTROL_RESTART_AT_SAVED_FRAME;
}

void ObjSeq_preempt(int key, int value)
{
    u8 count = lbl_803DD124;
    int i = (s8)count;
    if (i >= 40)
        return;
    gObjSeqPreemptList[i][0] = key;
    gObjSeqPreemptList[i][1] = value;
    lbl_803DD124++;
}

void endObjSequence(int seq)
{
    int j;
    int objCount;
    int objIdx;
    GameObject* frees[32];
    GameObject** objs;
    int i;
    int nFree;
    GameObject** ret;

    ret = (GameObject**)ObjList_GetObjects(&objIdx, &objCount);
    nFree = 0;
    i = 0;
    objs = ret;
    for (; i < objCount; i++)
    {
        GameObject* obj = *objs;
        if (obj->seqIndex == seq)
        {
            obj->seqIndex = -1;
        }
        if (obj->anim.classId == 0x10)
        {
            ObjSeqState* st = (ObjSeqState*)obj->extra;
            if ((s8)st->slot == seq)
            {
                if (obj == lbl_803DD0B8)
                {
                    lbl_803DD0B8 = 0;
                }
                frees[nFree++] = obj;
                if (st->freeCallback != NULL)
                {
                    (*(void (**)(void*, GameObject*, int))&st->freeCallback)(st->callbackContext, obj, (int)st);
                    st->freeCallback = NULL;
                }
                if (nFree == 0x10)
                {
                    debugPrintf(sEndObjSequenceMaxFreesError);
                }
            }
        }
        objs++;
    }
    if (curSeqNo == seq)
    {
        curSeqNo = 0;
        Pause_ResetMenuFrameCounter();
    }
    if (seq == gObjSeqPreparingStreamSlot)
    {
        AudioStream_CancelPrepared();
        gObjSeqPreparingStreamSlot = -1;
    }
    for (j = 0; j < nFree; j++)
    {
        Obj_FreeObject(frees[j]);
    }
    if (seq == lbl_803DD064)
    {
        if ((*gCameraInterface)->getMode() == MAKETEX_CAMMODE_NPCSPEAK)
        {
            (*gCameraInterface)->setMode(MAKETEX_CAMMODE_DEFAULT, 0, 3, 0, NULL, 0, 0);
            lbl_803DD064 = 0;
            curSeqNo = 0;
            Pause_ResetMenuFrameCounter();
        }
    }
    lbl_803DD07C = 0;
    gObjSeqSlotSeqIdTable[seq] = 0;
}

u8 gMemoryCardBannerAssetNames[168] = {83,  84,  65,  82,  70,  79,  88,  32,  65,  68,  86,  69,  78,  84,  85,  82,  69,  83,  0,
                        0,   68,  105, 110, 111, 115, 97,  117, 114, 32,  80,  108, 97,  110, 101, 116, 0,   111, 112,
                        101, 110, 105, 110, 103, 46,  98,  110, 114, 0,   99,  97,  114, 100, 47,  109, 101, 109, 99,
                        97,  114, 100, 105, 99,  111, 110, 48,  46,  105, 109, 103, 0,   0,   0,   99,  97,  114, 100,
                        47,  109, 101, 109, 99,  97,  114, 100, 105, 99,  111, 110, 49,  46,  105, 109, 103, 0,   0,
                        0,   99,  97,  114, 100, 47,  109, 101, 109, 99,  97,  114, 100, 105, 99,  111, 110, 50,  46,
                        105, 109, 103, 0,   0,   0,   99,  97,  114, 100, 47,  109, 101, 109, 99,  97,  114, 100, 105,
                        99,  111, 110, 51,  46,  105, 109, 103, 0,   0,   0,   99,  97,  114, 100, 47,  109, 101, 109,
                        99,  97,  114, 100, 105, 99,  111, 110, 48,  46,  112, 97,  108, 0,   0,   0};

f32 gObjSeqCameraFov = 60.0f;
int gObjSeqTaskTextId = -1;
int gObjSeqSubtitleId = -1;
int gObjSeqDeferredTaskTextId = -1;
int gObjSeqPreparingStreamSlot = -1;
int gObjSeqTimedStreamSlot = -1;
int gObjSeqStreamResumeOffset = -1;
int objSeqObjs = -1;
f32 gObjSeqShakeAmplitude = 0.2f;
char sSeqAAnimDataTag[] = "SEQA";
char sSeqBAnimDataTag[] = "SEQB";
int lbl_803DB744[1] = {0};
GXColor gObjSeqDefaultColor = {0x20, 0x20, 0x20, 0xFF};

typedef struct ObjSeqBgCmd
{
    int object;
    s16 param;
    union
    {
        struct
        {
            s8 opcode;
            s8 pad;
        };
        s16 flags;
    };
} ObjSeqBgCmd;

typedef struct RomCurveNode
{
    u8 pad00[0x08];
    f32 x;
    f32 y;
    f32 z;
    u8 pad14[0x07];
    s8 directionMask;
    s32 links[4];
    s8 yaw;
    s8 pitch;
    u8 tangentScale;
} RomCurveNode;

typedef struct RomCurveInterpState
{
    s32 fromNodeId;
    s32 toNodeId;
    f32 fromTime;
    f32 segmentTime1;
    f32 segmentTime2;
    f32 segmentTime3;
    f32 segmentTime4;
    f32 segmentTime5;
    f32 segmentTime6;
    f32 segmentTime7;
    f32 toTime;
} RomCurveInterpState;

#define ROM_CURVE_NODE_ANGLE(v)    ((3.1415927f * (f32)((s32)(v) << 8)) / 32768.0f)
#define ROM_CURVE_NODE_SCALE(node) (2.0f * (f32)(u8)((node)->tangentScale))

typedef struct ObjCurveKey
{
    f32 value;
    s8 tangentAndMode;
    u8 pad05;
    s16 frame;
} ObjCurveKey;

typedef struct ObjSeqPlacement
{
    u8 pad00[8];
    f32 baseX;
    f32 groundOffset;
    f32 baseZ;
    u8 pad14[8];
    s16 targetType;
    u8 pad1E;
    s8 slot;
} ObjSeqPlacement;

typedef struct ObjSeqAnimPlacement
{
    ObjPlacement base;
    s16 animDataIndex;
    s16 unk1A;
    s16 targetType;
    u8 pad1E;
    s8 sequenceSlot;
    u8 unk20;
    u8 unk21;
    s8 startOnLoad;
    u8 pad23;
    u8 unk24;
    u8 pad25[3];
} ObjSeqAnimPlacement;

typedef struct ObjSeqAnimDataHeader
{
    char tag[4];
    s16 dataSize;
    s16 commandCount;
} ObjSeqAnimDataHeader;

typedef struct ObjSeqAnimLookup
{
    s16 baseAnimId;
} ObjSeqAnimLookup;

typedef struct ObjSeqStreamMapEntry
{
    int trackId;
    u32* streamIds;
} ObjSeqStreamMapEntry;

#define OBJSEQ_STREAM_MAP_COUNT 5

STATIC_ASSERT(sizeof(ObjSeqStreamMapEntry) == 8);

STATIC_ASSERT(offsetof(ObjSeqAnimPlacement, animDataIndex) == 0x18);
STATIC_ASSERT(offsetof(ObjSeqAnimPlacement, unk1A) == 0x1A);
STATIC_ASSERT(offsetof(ObjSeqAnimPlacement, targetType) == 0x1C);
STATIC_ASSERT(offsetof(ObjSeqAnimPlacement, sequenceSlot) == 0x1F);
STATIC_ASSERT(offsetof(ObjSeqAnimPlacement, unk20) == 0x20);
STATIC_ASSERT(offsetof(ObjSeqAnimPlacement, unk21) == 0x21);
STATIC_ASSERT(offsetof(ObjSeqAnimPlacement, startOnLoad) == 0x22);
STATIC_ASSERT(offsetof(ObjSeqAnimPlacement, unk24) == 0x24);
STATIC_ASSERT(sizeof(ObjSeqAnimPlacement) == 0x28);
STATIC_ASSERT(sizeof(ObjSeqAnimDataHeader) == 8);

extern int ObjSeq_EvaluateCondition(int condition, u8* seq, int obj);
extern void ObjSeq_ApplyFrameCurves(GameObject* obj, GameObject* seqObj, u8* seq, int frame);
extern void ObjSeq_RebuildCurveStateToFrame(GameObject* obj, GameObject* seqObj, u8* seq, int mode);
extern void ObjSeq_ApplyLinkedObjectTransform(GameObject* obj, GameObject* seqObj, u8* seq);
extern void animatedObjFreeAndSavePlayerPos(GameObject* obj, GameObject* seqObj, u8* seq);
extern s16 gObjSeqBgCmds[];
extern u8 objSeqXrotChanged[];
extern s16 objSeqXrotValues[];
extern s8 gObjSeqBoolFlags[];
extern s8 gObjSeqCondFlags[];
extern s8 gObjSeqSlotResults[];
extern ObjSeqBgCmd lbl_8039A5BC[];
extern u8 gObjSeqRuntimeBuffer[];
extern int gObjSeqScriptedButtonMasks[];
extern ObjSeqStreamMapEntry gObjSeqStreamTableA[];
void ObjSeq_setCamVars(int camA, int camB, int camC, int camD);
int objSeqFindLabel(u8* seq, int label);
int objSeqFindConditional(u8* seq, GameObject* seqState);
void objCallSeqFn(GameObject* obj, GameObject* sourceObj, u8* seq, int action);
void objSeqDoBgCmds0D(u8* seq, GameObject* obj, int skipSpawns);
void ObjSeq_SetupInitialPlaybackState(GameObject* obj, GameObject** seqObj, u8* seq, ObjSeqPlacement* placement, void** outAction);
void ObjSeq_setXrot(int index, int xrot);
int ObjSeq_getBool(int index);
void ObjSeq_setBool(int index, int value);
void ObjSeq_addBgCmd(int index, int xrot, int yrot);
void ObjSeq_seqState_free(u8* seq);
void ObjSeq_seqState_init(u8* seq);
void* ObjSeq_FindTargetObject(GameObject* obj);
void ObjSeq_RefreshActionCursor(void* obj, void* seqFile, u8* seq);
void ObjSeq_onMapSetup(void);
void ObjSeq_release(void);
void ObjSeq_initialise(void);
void ObjSeq_copyDefaultColor(GXColor* out);
void RomCurveInterp_BuildSegmentTimeTable(RomCurveInterpState* out, RomCurveNode* curve, RomCurveNode* next, f32 t,
                                          int flag);
void RomCurveInterp_UpdateSegmentWindow(RomCurveInterpState* state, f32 t);
void RomCurveInterp_InitFromNode(RomCurveInterpState* out, int id);
int RomCurveInterp_EvaluateOffsetPosition(RomCurveInterpState* state, f32* offset, f32* outPos, s16* outAngle,
                                          int ignoreY);
f32 objCurveInterpolate(ObjCurveKey* keys, int count, int frame);

/* Camera mode ids passed to gCameraInterface->setMode; each == cameramode DLL number. */
/* placement stand-in spawned for missing seq actors; retail OBJECTS.bin name
   "Override" (DLL 0xC6) */
#define OBJSEQ_OVERRIDE_OBJ 0x6
/* retail OBJECTS.bin name "VariableObj" (DLL 0xC6) */
#define OBJSEQ_VARIABLE_OBJ 0x443
/* seq actor that carries the cutscene camera; retail OBJECTS.bin name
   "AnimCamera" (DLL 0xC6) */
#define OBJSEQ_ANIMCAMERA_OBJ 0x1e

/* playable-character placement ids; a seq actor carrying either one drives the
   live player object instead of spawning its own. retail OBJECTS.bin names
   "Sabre" and "Krystal" (no owning DLL) */
#define OBJSEQ_SABRE_OBJ   0x0
#define OBJSEQ_KRYSTAL_OBJ 0x1f

#define OBJSEQ_CAMMODE_DEFAULT      0x42 /* default gameplay cameramode DLL */
#define OBJSEQ_CAMMODE_VIEWFINDER   0x44 /* dll_0044 viewfinder */
#define OBJSEQ_CAMMODE_CAMTALK      0x45 /* dll_0045_camTalk */
#define OBJSEQ_CAMMODE_TESTSTRENGTH 0x47 /* dll_0047_cameramodeteststrength */
#define OBJSEQ_CAMMODE_STATIC       0x48 /* dll_0048_cameramodestatic */
#define OBJSEQ_CAMMODE_COMBAT       0x49 /* dll_0049_cameramodecombat */
#define OBJSEQ_CAMMODE_SHIPBATTLE   0x4a /* dll_004A_cameramodeshipbattle */
#define OBJSEQ_CAMMODE_CAMDEBUG     0x4c /* dll_004C_camDebug */
#define OBJSEQ_CAMMODE_CLOUDRUNNER  0x53 /* dll_0053_cameramodecloudrunner */
#define OBJSEQ_CAMMODE_ARWING       0x56 /* dll_0056_cameramodearwing */
#define OBJSEQ_CAMMODE_TITLE        0x57 /* dll_0057_cameramodetitle */

extern char sObjLoadAnimdataNullACRomTabWarning[];

/* GameCube controller button masks */
#define PAD_BUTTON_A 0x100
#define PAD_BUTTON_B 0x200

/* player-target group; its first object is used as the camera focus target */
#define OBJSEQ_TARGET_OBJGROUP 0xf

/* GameObject::objectFlags bit: object is bound to an active sequence (set when
   it becomes a seq callback target, cleared on release; tested elsewhere as the
   "under sequence control / blocked from normal update" gate). */
#define OBJECT_OBJFLAG_SEQ_ATTACHED 0x1000

/* Env-effect ids co-activated by seq opcodes 48/50 (A set) and 49 (B set);
   opaque distinct roles per index. */
#define OBJSEQ_ENVFX_A0 0x134
#define OBJSEQ_ENVFX_A1 0x135
#define OBJSEQ_ENVFX_A2 0x142
#define OBJSEQ_ENVFX_B0 0x136
#define OBJSEQ_ENVFX_B1 0x137
#define OBJSEQ_ENVFX_B2 0x143

extern u8 lbl_80399E50[];
extern char sObjSequenceMissingObjectFormat[];
extern s8 gObjSeqMsgSendModes[];
extern int gObjSeqMsgIds[];
extern s8 gObjSeqJumpLatch[];
int objSeqExecCmd06(GameObject* obj, GameObject* sourceObj, u8* seq, int cmd, s8 flag);

extern ObjSeqStreamMapEntry gObjSeqStreamTableB[];
extern u8 lbl_8039944C[];
extern u8 lbl_803DB411;
int ObjSeq_ExecuteActionCommand(GameObject* obj, u8* action, u8** cmd, s8 flags, void* out);
void* ObjSeq_ToggleCommand3Target(GameObject* obj, u8* seq, ObjSeqPlacement* placement);

typedef struct CamRequest
{
    s16 rot[3];
    u8 pad6[6];
    f32 posB[3];
    f32 pos[3];
    u8 pad24[0x90];
    f32 fov;
    u8 padB8[0x8c];
} CamRequest;

typedef struct CamFloats
{
    f32 a;
    f32 b;
    s16 c;
} CamFloats;

typedef struct CamMode
{
    int mode;
    u8 flag;
} CamMode;

typedef struct SeqByte136
{
    u8 modelSlot : 4;
    u8 pad3 : 1;
    u8 mapEvent : 1;
    u8 rest : 2;
} SeqByte136;

int ObjSeq_update(GameObject* obj, f32 t);

u8 lbl_803DD124;
f32 gObjSeqLinkedSavedPosX;
f32 gObjSeqLinkedSavedPosY;
f32 gObjSeqLinkedSavedPosZ;
s16 gObjSeqLinkedSavedPitch;
u8 gObjSeqLinkedTransformValid;
s8 lbl_803DD113;
u8 lbl_803DD112;
u8 lbl_803DD111;
u8 gObjSeqCameraActive;
int gObjSeqCamMode;
int gObjSeqCamModeArgB;
int gObjSeqCamModeArgC;
int gObjSeqCamModeArgD;
GameObject* focusedNpc;
u8 lbl_803DD0F8;
f32 lbl_803DD0F4;
f32 lbl_803DD0F0;
f32 lbl_803DD0EC;
s16 lbl_803DD0E8;
s16 lbl_803DD0E6;
s16 lbl_803DD0E4;
f32 lbl_803DD0E0;
f32 lbl_803DD0DC;
u8 gObjSeqStop;
u8 lbl_803DD0D9;
u8 lbl_803DD0D8;
ObjSeqAnimLookup* lbl_803DD0D4;
f32 gObjSeqFovOverrideValue;
f32 gObjSeqCurvePosOffsetX;
f32 gObjSeqCurvePosOffsetY;
f32 gObjSeqCurvePosOffsetZ;
int lbl_803DD0C0;
s8 gObjSeqBgCmdCount;
void* lbl_803DD0B8;
u16 lbl_803DD0B6;
SeqRunFlags lbl_803DD0B4;
f32 gObjSeqSavedCamPosX;
f32 gObjSeqSavedCamPosY;
f32 gObjSeqSavedCamPosZ;
f32 gObjSeqSavedCamFov;
int gObjSeqSavedCamPitch;
int gObjSeqSavedCamYaw;
int gObjSeqSavedCamRoll;
int gObjSeqStreamSuppressed;
int gObjSeqInputOverrideActive;
u8 curSeqNo;
s16 lbl_803DD08A;
u8 gObjSeqFovOverrideActive;
int seqGlobal4;
s8 seqGlobal3;
GameObject* lbl_803DD07C;
u8 lbl_803DD078;
f32 gObjSeqStreamRemainingTime;
s16 gObjSeqStreamStopped;
s16 seqGlobal2;
s16 seqGlobal1;
u32 gObjSeqCurrentTrackId;
int lbl_803DD064;
s16 lbl_803DD062;
s16 lbl_803DD060;
typedef struct SeqRunRec
{
    s16 slot;
    s16 flags;
    s16 count;
} SeqRunRec;

typedef struct SeqRunTables
{
    u8 pad0[0x2a80];
    SeqRunRec recs[0x1e];
    u8 pad1[0x800];
    u8 marks[0xb0];
    int handles[0x55];
    u8 cmdFlags[0x58];
    u8 counts[0x58];
    s16 headings[0x55];
    u8 pad2[0xae];
    f32 dists[0x55];
    f32 frames[0x55];
    u8 pad3[0xb0];
    s16 modes[0x55];
} SeqRunTables;

typedef struct ObjSeqQueuedBgCmd
{
    s16 index;
    s16 xrot;
    s16 yrot;
} ObjSeqQueuedBgCmd;

typedef struct ObjSeqRunBgState
{
    u8 pad0000[0x2A80];
    ObjSeqQueuedBgCmd queuedCmds[0x1E];
    u8 pad2B34[0x338C - 0x2B34];
    u8 slotMarks[0x55];
    u8 pad33E1[0x3740 - 0x33E1];
    f32 slotDistances[0x55];
    f32 previousSlotDistances[0x55];
    s8 pendingFrames[0x55];
    u8 pad3A3D[3];
    u8 slotStates[0x55];
    u8 pad3A95[0x3B44 - 0x3A95];
    s8 boolFlags[0x58];
    s8 conditionFlags[0x58];
    u8 slotResults[0x58];
    u8 previousSlotResults[0x58];
} ObjSeqRunBgState;

STATIC_ASSERT(offsetof(ObjSeqRunBgState, slotMarks) == 0x338C);
STATIC_ASSERT(offsetof(ObjSeqRunBgState, pendingFrames) == 0x39E8);
STATIC_ASSERT(offsetof(ObjSeqRunBgState, boolFlags) == 0x3B44);
STATIC_ASSERT(offsetof(ObjSeqRunBgState, slotResults) == 0x3BF4);

void RomCurveInterp_BuildSegmentTimeTable(RomCurveInterpState* out, RomCurveNode* curve, RomCurveNode* next, f32 t,
                                          int flag);
int RomCurveInterp_EvaluateOffsetPosition(RomCurveInterpState* state, f32* offset, f32* outPos, s16* outAngle,
                                          int ignoreY);



static inline u8* ObjSeq_GetActiveModel(GameObject* obj)
{
    ObjAnimComponent* objAnim = &obj->anim;
    return (u8*)objAnim->banks[objAnim->bankIndex];
}

static inline int objSeqIsObjMonitored(u8* walk, GameObject* obj)
{
    int i;
    int n;

    n = (s8)lbl_803DD124;
    for (i = 0; i < n; i++)
    {
        if (*(GameObject**)walk == obj)
        {
            return 1;
        }
        walk += 8;
    }
    return 0;
}

static inline int objSeqRemoveMonitoredObj(u8* base, u8** monp, GameObject* obj)
{
    int v;
    int j;
    int k;
    int n;
    int flags;
    u8* p;

    n = (s8)lbl_803DD124;
    for (j = 0; j < n; j++)
    {
        if (*(GameObject**)*monp == obj)
        {
            flags = *(int*)(base + j * 8 + 0x3d50);
            lbl_803DD124 -= 1;
            p = base + j * 8 + 0x3d4c;
            v = *(int*)(p + 8);
            for (k = j; k < (s8)lbl_803DD124; k++)
            {
                *(int*)p = v;
                *(int*)(p + 4) = v;
                p += 8;
            }
            return flags;
        }
        *monp += 8;
    }
    return 0;
}

int ObjSeq_start(int seqIdx, GameObject* obj, int flags)
{
    u8* base;
    SeqRunTables* st;
    u8* walk2;
    u8* walk;
    int packed;
    u8* mon;
    int i;
    int idx;
    int count;
    int first;
    int bit;
    int objId;
    int slot;
    u8* hdr;
    GameObject* parent;
    u8* srcSeq;
    ObjSeqAnimPlacement* setup;
    u8* seq;
    int size;
    s16 heading;
    int camArg;
    GameObject* player;
    int doCam;
    GameObject* newObj;
    s16* slotPtr;
    u8* buf;
    u8* blk;
    u8* p;
    s16* mapTbl;
    int j;
    int seqFlags;
    int found;
    int cur;
    s16 val;
    u32 objIdU;
    u32 mapFlags;
    u32 trackId;
    f32 x;
    f32 y;
    f32 z;

    base = gObjSeqRuntimeBuffer;
    st = (SeqRunTables*)base;
    srcSeq = (u8*)obj->anim.placementData;
    camArg = 0;
    doCam = 0;
    player = Obj_GetPlayerObject();

    if (seqIdx == -1)
    {
        return -1;
    }
    if (seqIdx < 0 || seqIdx >= obj->anim.modelInstance->sequenceCount)
    {
        return -1;
    }

    for (i = 0x19; i < 0x55; i++)
    {
        p = base + i * 2;
        p = (u8*)((int)p + 0x3a98);
        if (*(s16*)p == 0)
        {
            slot = i;
            *(s16*)p = 1;
            blk = base + i * 0x80;
            for (j = 0; j < 16; j++)
            {
                *(u8**)blk = NULL;
                blk += 8;
            }
            i = 0x56;
        }
    }
    if (i == 0x55)
    {
        return -1;
    }

    mapTbl = obj->anim.modelInstance->sequenceMap;
    if (mapTbl != NULL)
    {
        seqIdx = mapTbl[seqIdx];
    }

    cur = obj->seqIndex;
    if (cur != -1 && lbl_803DD07C == NULL)
    {
        endObjSequence(cur);
    }

    val = seqIdx + 1;
    *(slotPtr = (s16*)(base + 0x3a98) + slot) = val;
    gObjSeqTaskTextId = -1;
    gObjSeqSubtitleId = -1;

    mon = base + 0x3d4c;
    found = objSeqIsObjMonitored(mon, obj);
    if (found == 0)
    {
        gObjSeqTaskTextId = seqIdx;
    }

    hdr = mmAlloc(0x20, 0x11, 0);
    getTabEntry(hdr, MLDF_FILEID_OBJSEQ_TAB, seqIdx * 2, 8);
    first = *(s16*)hdr;
    count = *(s16*)(hdr + 2) - first;
    size = count << 3;
    buf = mmAlloc(size, 0x11, 0);
    getTabEntry(buf, MLDF_FILEID_OBJSEQ_BIN, first * 8, size);
    mm_free(hdr);

    if (lbl_803DD07C != NULL)
    {
        obj = lbl_803DD07C;
    }
    obj->seqIndex = slot;
    parent = obj->anim.parent;
    x = obj->anim.localPosX;
    y = obj->anim.localPosY;
    z = obj->anim.localPosZ;
    if (lbl_803DD0B4.useWorldSpace)
    {
        parent = NULL;
        x = obj->anim.worldPosX;
        y = obj->anim.worldPosY;
        z = obj->anim.worldPosZ;
    }
    heading = obj->anim.rotX;
    if (lbl_803DD078 != 0)
    {
        x -= obj->anim.rootMotionScale *
             (obj->anim.hitboxScale * mathSinf((3.1415927f * (f32)obj->anim.rotX) / 32768.0f));
        z -= obj->anim.rootMotionScale *
             (obj->anim.hitboxScale * mathCosf((3.1415927f * (f32)obj->anim.rotX) / 32768.0f));
    }

    i = 0;
    st->cmdFlags[obj->seqIndex] = 0;
    base[obj->seqIndex + 0x3334] = 0;
    gObjSeqSlotValues[obj->seqIndex] = 0;
    st->handles[obj->seqIndex] = obj->anim.romDefNo;

    walk = buf;
    bit = 1;
    for (; i < count; i++)
    {
        if ((flags & (bit << i)) && (*(u16*)(walk + 4) & 0x4000))
        {
            objIdU = *(u16*)(walk + 6);
            if (objIdU == OBJSEQ_KRYSTAL_OBJ || objIdU == OBJSEQ_SABRE_OBJ)
            {
                if (playerStatusIsPositive(Obj_GetPlayerObject()) == 0)
                {
                    return -1;
                }
            }
        }
        walk += 8;
    }

    idx = 0;
    walk2 = buf;
    packed = ((seqIdx & 0x7ff) << 4) | 0x8000;
    for (; idx < count; idx++)
    {
        if (flags & (1 << idx))
        {
            setup = (ObjSeqAnimPlacement*)Obj_AllocObjectSetup(0x28, OBJSEQ_OVERRIDE_OBJ);
            objId = *(u16*)(walk2 + 6);
            if (objId == OBJSEQ_KRYSTAL_OBJ || objId == OBJSEQ_SABRE_OBJ)
            {
                GameObject* pp = Obj_GetPlayerObject();
                pp->objectFlags |= OBJECT_OBJFLAG_SEQ_ATTACHED;
            }
            if (objId == 0xffff)
            {
                setup->base.objectId = OBJSEQ_OVERRIDE_OBJ;
                setup->targetType = obj->anim.romDefNo + 4;
                if (obj->anim.romDefNo == OBJSEQ_VARIABLE_OBJ && objSeqObjs != -1)
                {
                    setup->targetType = objSeqObjs + 4;
                }
                *(u16*)(walk2 + 4) |= 0x8000;
            }
            else if (objId == 0xfffe)
            {
                setup->base.objectId = OBJSEQ_ANIMCAMERA_OBJ;
                setup->targetType = 3;
                curSeqNo = slot;
            }
            else
            {
                if (*(u16*)(walk2 + 4) & 0x4000)
                {
                    setup->base.objectId = OBJSEQ_OVERRIDE_OBJ;
                    if (objId == OBJSEQ_VARIABLE_OBJ)
                    {
                        if (objSeqObjs != -1)
                        {
                            setup->targetType = objSeqObjs + 4;
                        }
                        else
                        {
                            setup->targetType = objId + 4;
                        }
                    }
                    else
                    {
                        setup->targetType = objId + 4;
                    }
                }
                else
                {
                    setup->base.objectId = objId;
                    setup->targetType = 0;
                }
            }
            if (*(u16*)(walk2 + 4) & 0x8000)
            {
                setup->unk20 = 0;
                setup->unk21 = 0;
            }
            else
            {
                setup->unk20 = 1;
                setup->unk21 = 1;
            }
            if (idx == 0 && (*(u16*)(walk2 + 4) & 0x1000) && player != NULL)
            {
                playerSetOverrideParentSlack(player);
            }
            setup->animDataIndex = packed | (idx & 0xf);
            setup->unk1A = -1;
            if (idx != 0)
            {
                if (lbl_803DD0D9 != 0 && setup->base.objectId == OBJSEQ_ANIMCAMERA_OBJ)
                {
                    setup->base.posX = x + *(f32*)(base + 0x2bd4);
                    setup->base.posY = y + *(f32*)(base + 0x2bd8);
                    setup->base.posZ = z + *(f32*)(base + 0x2bdc);
                    lbl_803DD0D9 = 0;
                }
                else
                {
                    setup->base.posX = x;
                    setup->base.posY = y;
                    setup->base.posZ = z;
                }
            }
            else
            {
                setup->base.posX = obj->anim.localPosX;
                setup->base.posY = obj->anim.localPosY;
                setup->base.posZ = obj->anim.localPosZ;
            }
            setup->sequenceSlot = slot;
            setup->startOnLoad = 1;
            setup->unk24 = (*(u16*)(walk2 + 4) & 0xf00) >> 8;
            setup->base.color[0] = 2;
            setup->base.color[1] = 1;
            if (srcSeq != NULL)
            {
                setup->base.color[1] = setup->base.color[1] | (srcSeq[5] & 0x18);
            }
            if (setup->base.objectId == OBJSEQ_ANIMCAMERA_OBJ)
            {
                setup->base.color[0] = 1;
            }
            if (setup->base.objectId == OBJSEQ_VARIABLE_OBJ && objSeqObjs != -1)
            {
                setup->base.objectId = objSeqObjs;
            }
            newObj = Obj_SetupObject(&setup->base, 5, -1, -1, parent);
            newObj->seqIndex = -2;
            seq = newObj->extra;
            ((ObjSeqState*)seq)->heading = heading;
            ((ObjSeqState*)seq)->flags = -1;
            ((ObjSeqState*)seq)->flags = ((ObjSeqState*)seq)->flags & ~0x400;
            ((ObjSeqState*)seq)->conditionOpcodes[0] = 0;
            ((ObjSeqState*)seq)->conditionOpcodes[1] = 0;
            ((ObjSeqState*)seq)->conditionOpcodes[2] = 0;
            ((ObjSeqState*)seq)->conditionOpcodes[3] = 0;
            if (*(u16*)(walk2 + 4) & 1)
            {
                ((ObjSeqState*)seq)->flags = ((ObjSeqState*)seq)->flags & ~1;
            }
            if (*(u16*)(walk2 + 4) & 2)
            {
                ((ObjSeqState*)seq)->flags = ((ObjSeqState*)seq)->flags & ~2;
            }
            if (*(u16*)(walk2 + 4) & 4)
            {
                ((ObjSeqState*)seq)->heading = 0;
            }
            if (*(u16*)(walk2 + 4) & 8)
            {
                ((ObjSeqState*)seq)->flags = ((ObjSeqState*)seq)->flags & ~0x100;
            }
            if (*(u16*)(walk2 + 4) & 0x80)
            {
                ((ObjSeqState*)seq)->stateFlags = ((ObjSeqState*)seq)->stateFlags | 4;
            }
            if (*(u16*)(walk2 + 4) & 0x40)
            {
                ((ObjSeqState*)seq)->stateFlags = ((ObjSeqState*)seq)->stateFlags | 2;
            }
            if (*(u16*)(walk2 + 4) & 0x2000)
            {
                if (idx == 0 && player != NULL)
                {
                    playerSetCutsceneCameraFlag(player);
                }
                if (lbl_803DD064 == 0 || lbl_803DD064 == obj->seqIndex)
                {
                    lbl_803DD064 = obj->seqIndex;
                    curSeqNo = slot;
                }
                ((ObjSeqState*)seq)->movementState = 4;
                if (camArg == 0)
                {
                    camArg = *(u16*)(walk2 + 4) & 0xf00;
                    camArg >>= 8;
                }
                doCam = 1;
            }
            else
            {
                ((ObjSeqState*)seq)->movementState = -1;
            }
            if ((objId == OBJSEQ_KRYSTAL_OBJ || objId == OBJSEQ_SABRE_OBJ) && (((ObjSeqState*)seq)->flags & 1))
            {
                playerSetInCutscene(player);
            }
            ((ObjSeqState*)seq)->targetObjId = *(int*)walk2;
            ((ObjSeqState*)seq)->savedFlags = ((ObjSeqState*)seq)->flags;
            if (idx == 0)
            {
                *(u8*)((u8*)&st->cmdFlags[0] + obj->seqIndex) = *(u16*)(walk2 + 4);
                *(int*)((u8*)&st->handles[0] + obj->seqIndex * 4) =
                    *(int*)((u8*)newObj->anim.placementData + 0x14);
                mapFlags = obj->anim.modelInstance->flags;
                if ((mapFlags & OBJMODEL_FLAG_SKIP_RESET_UPDATE) && !(mapFlags & 0x8000))
                {
                    parent = obj;
                    z = y = x = 0.0f;
                    heading = 0;
                }
            }
        }
        walk2 += 8;
    }

    st->headings[obj->seqIndex] = heading;
    base[obj->seqIndex + 0x3590] = 0;
    base[obj->seqIndex + 0x338c] = 0;
    seqFlags = objSeqRemoveMonitoredObj(base, &mon, obj);
    if (seqFlags != 0)
    {
        st->cmdFlags[obj->seqIndex] |= 0x10;
    }
    else
    {
        gObjSeqStreamStopped = 0;
        trackId = (u32)(*slotPtr - 1) & 0x3fff;
        gObjSeqCurrentTrackId = trackId;
        if (AudioStream_Play(trackId, ObjSeq_AudioStreamCallback) == 0)
        {
            if (gObjSeqTaskTextId != -1)
            {
                gameTextLoadTaskText(gObjSeqTaskTextId);
                gObjSeqTaskTextId = -1;
            }
        }
        else
        {
            gObjSeqPreparingStreamSlot = slot;
            gObjSeqDeferredTaskTextId = gObjSeqTaskTextId;
            gObjSeqTimedStreamSlot = -1;
            gObjSeqStreamRemainingTime = 0.0f;
            gObjSeqStreamResumeOffset = -1;
        }
    }

    st->dists[obj->seqIndex] = seqFlags;
    st->frames[obj->seqIndex] = seqFlags;

    if (slot >= 0 && slot < 0x55)
    {
        if (gObjSeqBgCmdCount < 0x1e)
        {
            st->recs[gObjSeqBgCmdCount].slot = slot;
            st->recs[gObjSeqBgCmdCount].count = count;
            st->recs[gObjSeqBgCmdCount++].flags = seqFlags;
        }
    }

    if (doCam != 0)
    {
        cameraFocusNpc(camArg, obj);
    }
    mm_free(buf);
    lbl_803DD078 = 0;
    lbl_803DD0B4.useWorldSpace = 0;
    return slot;
}


void ObjSeq_func13(void)
{
}


int ObjSeq_func12(void)
{
    return 0;
}

int ObjSeq_func0E(void)
{
    return 0;
}

void ObjSeq_setGlobal4(int value)
{
    seqGlobal4 = value;
}

int ObjSeq_getGlobal4(void)
{
    return seqGlobal4;
}

int ObjSeq_func0F(void)
{
    return 1;
}

static inline GameObject* objSeqFindLinkedObject(u8* seqObj, GameObject* candidate)
{
    u8* slotBase;
    u8* entry;
    int j;

    j = 0;
    slotBase = gObjSeqRuntimeBuffer + (s8)seqObj[0x57] * 0x80;
    entry = slotBase;
    for (; j < 16; j++)
    {
        if (*(GameObject**)entry == candidate)
        {
            return *(GameObject**)(slotBase + j * 8 + 4);
        }
        entry += 8;
    }
    return NULL;
}

int ObjSeq_resolveTargetObject(GameObject* obj)
{
    int objectCount;
    int unused;
    void** objects;
    u8* seqObj;
    ObjSeqPlacement* model;
    GameObject* found;
    GameObject* candidate;
    int objType;
    int i;
    GameObject* linked;
    f32 bestDist;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 distSq;

    objects = (void**)ObjList_GetObjects(&unused, &objectCount);
    seqObj = obj->extra;
    model = (ObjSeqPlacement*)obj->anim.placementData;
    if (obj->anim.classId == 0x11)
    {
        ((ObjSeqState*)seqObj)->targetObj = NULL;
        return -1;
    }

    switch (model->targetType)
    {
    case 0:
        ((ObjSeqState*)seqObj)->targetObj = NULL;
        break;
    case 1:
        ((ObjSeqState*)seqObj)->targetObj = Obj_GetPlayerObject();
        break;
    case 2:
        ((ObjSeqState*)seqObj)->targetObj = getTrickyObject();
        break;
    case 3:
        ((ObjSeqState*)seqObj)->targetObj = NULL;
        ((ObjSeqState*)seqObj)->unk7B = (s8)(model->targetType - 2);
        if (lbl_803DD064 != 0)
        {
            lbl_803DD064 = 0;
        }
        if ((lbl_80399E50[(s8)seqObj[0x57]] & 0x10) == 0)
        {
            (*gCameraInterface)->setLetterbox(0x41, 1);
        }
        break;
    default:
        ((ObjSeqState*)seqObj)->targetObj = NULL;
        objType = model->targetType - 4;
        if (objType == OBJSEQ_KRYSTAL_OBJ || objType == OBJSEQ_SABRE_OBJ)
        {
            ((ObjSeqState*)seqObj)->targetObj = Obj_GetPlayerObject();
        }
        else if (((ObjSeqState*)seqObj)->targetObjId != 0)
        {
            ((ObjSeqState*)seqObj)->targetObj = ObjList_FindObjectById(((ObjSeqState*)seqObj)->targetObjId);
        }
        else
        {
            bestDist = -1.0f;
            for (i = 0; i < objectCount; i++)
            {
                candidate = objects[i];
                linked = objSeqFindLinkedObject(seqObj, candidate);
                if (linked == obj)
                {
                    ((ObjSeqState*)seqObj)->targetObj = candidate;
                    break;
                }
                if (linked == NULL)
                {
                    if (candidate->anim.romDefNo == objType)
                    {
                        dx = obj->anim.localPosX - candidate->anim.localPosX;
                        dy = obj->anim.localPosY - candidate->anim.localPosY;
                        dz = obj->anim.localPosZ - candidate->anim.localPosZ;
                        distSq = dx * dx + dy * dy + dz * dz;
                        if (bestDist < 0.0f || distSq < bestDist)
                        {
                            bestDist = distSq;
                            ((ObjSeqState*)seqObj)->targetObj = candidate;
                        }
                    }
                }
            }
        }
        break;
    }

    found = *(GameObject**)seqObj;
    if (found != NULL)
    {
        if ((s8)seqObj[0x57] < 0x19)
        {
            if (found->seqIndex != -1)
            {
                endObjSequence(found->seqIndex);
            }
        }
        return (*(GameObject**)seqObj)->anim.defId;
    }
    return -1;
}

void* ObjSeq_FindTargetObject(GameObject* obj)
{
    int objectCount;
    int unused;
    void** objects;
    int targetId;
    int objectType;
    GameObject* candidate;
    void* bestObj;
    int i;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 distSq;
    f32 bestDistSq;

    targetId = *(int*)((u8*)obj->extra + 0x10c);
    if (targetId != 0)
    {
        return ObjList_FindObjectById(targetId);
    }

    objects = (void**)ObjList_GetObjects(&unused, &objectCount);
    objectType = *(s16*)((u8*)obj->anim.placementData + 0x1c) - 4;
    if (objectType == 0x1f || objectType == 0)
    {
        return Obj_GetPlayerObject();
    }
    if (objectType == 0x24 || objectType == 0x25)
    {
        return getTrickyObject();
    }

    {
        bestDistSq = -1.0f;
        bestObj = NULL;
        for (i = 0; i < objectCount; i++)
        {
            candidate = objects[i];
            if (candidate->anim.romDefNo == objectType)
            {
                dx = obj->anim.localPosX - candidate->anim.localPosX;
                dy = obj->anim.localPosY - candidate->anim.localPosY;
                dz = obj->anim.localPosZ - candidate->anim.localPosZ;
                distSq = dx * dx + dy * dy + dz * dz;
                if (bestDistSq < 0.0f || distSq < bestDistSq)
                {
                    bestDistSq = distSq;
                    bestObj = candidate;
                }
            }
        }
    }
    return bestObj;
}

#define ObjSeq_GetObjects(unused, count) ((GameObject**)ObjList_GetObjects((unused), (count)))

void ObjSeq_runBgCmds(void)
{
    int ok;
    int keepCount;
    GameObject** objects;
    int matchCount;
    GameObject** objPtr;
    u8* base;
    ObjSeqRunBgState* state;
    ObjSeqQueuedBgCmd* cmd;
    ObjSeqQueuedBgCmd* keepWalk;
    ObjSeqQueuedBgCmd* keepBase;
    int count;
    int i;
    int index;
    int xrot;
    ObjSeqPlacement* model;
    ObjSeqState* seqp;
    GameObject* candidate;
    GameObject** mp;
    int n;
    s8* pending;
    u8* results;
    u8* actions;
    f32* dists;
    f32* frames;
    u8* marks;
    s8 frames8;
    GameObject* matched[0x28];
    ObjSeqQueuedBgCmd keepBuf[0x1e];
    int objectCount;
    int unused;

    base = gObjSeqRuntimeBuffer;
    state = (ObjSeqRunBgState*)base;
    objects = ObjSeq_GetObjects(&unused, &objectCount);
    if (lbl_803DD060 != lbl_803DD062)
    {
        lbl_803DD062 = lbl_803DD060;
    }

    pending = state->pendingFrames;
    results = state->slotResults;
    actions = state->previousSlotResults;
    dists = state->slotDistances;
    frames = state->previousSlotDistances;
    marks = state->slotMarks;
    frames8 = framesThisStep;

    for (i = 0; i < 0x55; i++)
    {
        *pending = 0;
        if ((s8)*results != 0 && (s8)*actions == 0)
        {
            *pending = frames8;
        }
        *actions = *results;
        *results = 0;
        *frames = *dists;
        *dists = -1.0f;
        if (*marks == 2)
        {
            *marks = 1;
        }
        else
        {
            *marks = 0;
        }
        pending++;
        results++;
        actions++;
        dists++;
        frames++;
        marks++;
    }

    count = gObjSeqBgCmdCount;
    keepCount = 0;
    cmd = (ObjSeqQueuedBgCmd*)(base + 0x2a80) + count;
    keepBase = keepBuf;
    keepWalk = keepBase;
    while (count > 0)
    {
        cmd--;
        count--;
        index = cmd->index;
        xrot = cmd->xrot;
        i = 0;
        state->boolFlags[index] = 0;
        state->conditionFlags[index] = 0;
        state->slotStates[index] = 0;
        matchCount = 0;
        ok = 1;
        objPtr = objects;
        for (; i < objectCount; i++)
        {
            candidate = *objPtr;
            if (candidate->anim.classId == 0x10)
            {
                model = (ObjSeqPlacement*)candidate->anim.placementData;
                seqp = candidate->extra;
                if (model != NULL && model->slot == index)
                {
                    if (model->targetType >= 4 && ObjSeq_FindTargetObject(candidate) == NULL)
                    {
                        ok = 0;
                        logPrintf(sObjSequenceMissingObjectFormat, model->targetType - 4);
                    }
                    else
                    {
                        seqp->targetObj = NULL;
                    }
                    if (matchCount < 0x28)
                    {
                        matched[matchCount++] = candidate;
                    }
                }
            }
            objPtr++;
        }

        n = 0;
        mp = matched;
        for (; n < matchCount; n++)
        {
            candidate = *mp;
            model = (ObjSeqPlacement*)candidate->anim.placementData;
            if (model != NULL && model->slot == index)
            {
                seqp = candidate->extra;
                if (ok != 0)
                {
                    seqp->runState = 2;
                    seqp->pendingStartFrame = xrot;
                    ObjSeq_update(candidate, 1.0f);
                    Obj_GetWorldPosition(candidate, &candidate->anim.worldPosX,
                                         &candidate->anim.worldPosY, &candidate->anim.worldPosZ);
                }
                else
                {
                    seqp->runState = 3;
                }
            }
            mp++;
        }

        if (ok == 0)
        {
            keepWalk->index = index;
            keepWalk++;
            keepBuf[keepCount++].xrot = xrot;
        }
    }

    for (i = 0; i < keepCount; i++)
    {
        ((ObjSeqQueuedBgCmd*)(base + 0x2a80))[i].index = keepBase->index;
        ((ObjSeqQueuedBgCmd*)(base + 0x2a80))[i].xrot = keepBase->xrot;
        keepBase++;
    }
    gObjSeqBgCmdCount = keepCount;
}

static inline f32 ObjSeq_SampleTrackCurve(u8* seq, int track, int frame)
{
    f32 val;
    if (((ObjSeqState*)seq)->animEntries == NULL)
    {
        return 0.0f;
    }
    val = 0.0f;
    if (((ObjSeqState*)seq)->trackRunLength[track] != 0)
    {
        val = objCurveInterpolate(
            (ObjCurveKey*)(((ObjSeqState*)seq)->animEntries + ((ObjSeqState*)seq)->trackAnimStart[track] * 8),
            ((ObjSeqState*)seq)->trackRunLength[track] & 0xfff, frame);
    }
    return val;
}


void ObjSeq_seqState_free(u8* seq)
{
    void* ptr;

    ptr = ((ObjSeqState*)seq)->cmds;
    if (ptr != NULL)
    {
        mm_free(ptr);
        ((ObjSeqState*)seq)->cmds = NULL;
        ((ObjSeqState*)seq)->animEntries = NULL;
    }
    ptr = ((ObjSeqState*)seq)->curveInterp;
    if (ptr != NULL)
    {
        mm_free(ptr);
        ((ObjSeqState*)seq)->curveInterp = NULL;
    }
}

void ObjSeq_seqState_init(u8* seq)
{
    int animIndex;
    int runLength;
    int track;
    int animCount;
    int commandIndex;
    u8* command;

    for (animCount = 0; animCount < 0x13; animCount++)
    {
        ((ObjSeqState*)seq)->trackRunLength[animCount] = 0;
    }

    track = 0;
    animIndex = 0;
    while (animIndex < ((ObjSeqState*)seq)->animCount)
    {
        runLength = 0;
        commandIndex = ((ObjSeqState*)seq)->animCount;
        while (animIndex + runLength < commandIndex &&
               track == ((s8)(((ObjSeqState*)seq)->animEntries + (animIndex + runLength) * 8)[5] & 0x1f))
        {
            runLength++;
        }
        ((ObjSeqState*)seq)->trackRunLength[track] = runLength;
        ((ObjSeqState*)seq)->trackAnimStart[track] = animIndex;
        track++;
        animIndex += runLength;
    }

    ((ObjSeqState*)seq)->endFrame = 1000;
    commandIndex = 0;
    while (commandIndex < 2 && commandIndex < ((ObjSeqState*)seq)->cmdCount)
    {
        command = ((ObjSeqState*)seq)->cmds + commandIndex * 4;
        if ((s8)command[0] == -1)
        {
            ((ObjSeqState*)seq)->endFrame = *(s16*)(command + 2) + 1;
        }
        commandIndex++;
    }
}



void ObjSeq_objLoadAnimdata(ObjSeqState* seq, ObjSeqAnimPlacement* placement)
{
    ObjSeqRunBgState* runBgState = (ObjSeqRunBgState*)gObjSeqRuntimeBuffer;
    s16 size;
    int animId;
    int fileOffset;
    ObjSeqAnimDataHeader hdr;

    if (placement->animDataIndex == -1)
    {
        return;
    }

    seq->animCount = 0;
    seq->cmdCount = 0;
    animId = placement->animDataIndex;
    if ((animId & 0x8000) != 0)
    {
        getTabEntry(lbl_803DD0D4, MLDF_FILEID_OBJSEQ2C_TAB, ((animId & 0x7ff0) >> 4) * 2, 8);
        animId = lbl_803DD0D4->baseAnimId + (animId & 0xf);
    }
    else
    {
        animId = animId + 1;
    }

    if (getTableFileEntry(MLDF_FILEID_ANIMCURV_TAB_A, animId, &fileOffset) == 0)
    {
        logPrintf(sObjLoadAnimdataNullACRomTabWarning);
        return;
    }

    loadAndDecompressDataFile(MLDF_FILEID_ANIMCURV_BIN_A, &hdr, fileOffset, 8, 0, 0, 0);
    if (strncmp(hdr.tag, sSeqAAnimDataTag, 4) != 0 && strncmp(hdr.tag, sSeqBAnimDataTag, 4) != 0)
    {
        logPrintf(sObjLoadAnimdataNullACRomTabWarning);
        return;
    }

    size = hdr.dataSize;
    seq->cmdCount = hdr.commandCount;
    if (size == 0)
    {
        logPrintf(sObjLoadAnimdataNullACRomTabWarning);
        return;
    }

    seq->cmds = mmAlloc(size, 0x11, 0);
    if (seq->cmds == NULL)
    {
        logPrintf(sObjLoadAnimdataNullACRomTabWarning);
        return;
    }

    loadAndDecompressDataFile(MLDF_FILEID_ANIMCURV_BIN_A, seq->cmds, fileOffset + 8, hdr.dataSize, 0, 0, 0);
    seq->animCount = (s16)(((hdr.dataSize >> 2) - hdr.commandCount) >> 1);
    seq->animEntries = seq->cmds + hdr.commandCount * 4;

    seq->slot = placement->sequenceSlot;
    if (seq->slot > -1)
    {
        runBgState->conditionFlags[seq->slot] = 0;
        runBgState->boolFlags[seq->slot] = 0;
        runBgState->slotStates[seq->slot] = 0;
    }

    if (placement->startOnLoad != 0)
    {
        seq->runState = 2;
    }
    else
    {
        seq->runState = 0;
    }
    ObjSeq_seqState_init((u8*)seq);
}


void ObjSeq_updateCamera(void);

u32 lbl_8030EC00[4] = {0x28E5, 0x28E6, 0x28E7, 0x28E8};
u32 lbl_8030EC10[3] = {0x501C, 0x501D, 0x501E};
u32 lbl_8030EC1C[3] = {0x51A1, 0x51A2, -1};
u32 lbl_8030EC28[7] = {0x51A4, 0x51A5, 0x51A7, 0x51A8, 0x51A9, 0x51AA, 0x51AB};
u32 lbl_8030EC44[4] = {0x51AC, 0x51AD, 0x51AE, 0x51AF};
u32 lbl_8030EC54[4] = {0x2A, 0x25, 0x21, 0x2B};
u32 lbl_8030EC64[3] = {-1, -1, -1};
u32 lbl_8030EC70[3] = {-1, -1, 0x525};
u32 lbl_8030EC7C[7] = {0x2E5, 0x2E6, 0x2E8, 0x2EA, 0x2EA, 0x2E8, 0x2E9};
u32 lbl_8030EC98[4] = {0x2ED, 0x2EE, 0x2EF, 0x2F0};

ObjSeqStreamMapEntry gObjSeqStreamTableA[OBJSEQ_STREAM_MAP_COUNT] = {
    {0x35F, lbl_8030EC00}, {0x45A, lbl_8030EC10}, {0x117, lbl_8030EC1C},
    {0xC3, lbl_8030EC28},  {0x122, lbl_8030EC44},
};
ObjSeqStreamMapEntry gObjSeqStreamTableB[OBJSEQ_STREAM_MAP_COUNT] = {
    {0x35F, lbl_8030EC54}, {0x45A, lbl_8030EC64}, {0x117, lbl_8030EC70},
    {0xC3, lbl_8030EC7C},  {0x122, lbl_8030EC98},
};

s16 gObjSeqSlotValues[86] = {0};

int gObjSeqScriptedButtonMasks[7] = {0x100, 0x200, 0x40000, 0x80000, 0x20000, 0x10000, -1};

int gObjSeqMsgIds[] = {
    0x00050001, 0x00050002, 0x00050003, 0x00060001, 0x00060002, 0x000A0001, 0x000A0002, 0x000A0003,
    8,          9,          0x00030002, 0x00030003, 0x000A0004, 0x000A0005, 0x000A0006, 0x000F000B,
    0x000F000C, 0x000F000D, 0x000F000E, 0x000F000F, 0x000F0010, 0x00130001, 0x00130002,
};

s8 gObjSeqMsgSendModes[24] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0};

void* lbl_8030EE34[40] = {(void*)0,
                          (void*)0,
                          (void*)0,
                          (void*)0x230000,
                          (void*)ObjSeq_initialise,
                          (void*)ObjSeq_release,
                          (void*)0,
                          (void*)ObjSeq_onMapSetup,
                          (void*)ObjSeq_addBgCmd,
                          (void*)ObjSeq_setBool,
                          (void*)ObjSeq_getBool,
                          (void*)ObjSeq_update,
                          (void*)ObjSeq_updateCamera,
                          (void*)ObjSeq_objLoadAnimdata,
                          (void*)ObjSeq_seqState_init,
                          (void*)ObjSeq_seqState_free,
                          (void*)ObjSeq_runBgCmds,
                          (void*)ObjSeq_resolveTargetObject,
                          (void*)ObjSeq_func0E,
                          (void*)ObjSeq_func0F,
                          (void*)ObjSeq_getGlobal4,
                          (void*)ObjSeq_setGlobal4,
                          (void*)ObjSeq_func12,
                          (void*)ObjSeq_func13,
                          (void*)ObjSeq_start,
                          (void*)endObjSequence,
                          (void*)ObjSeq_setCamVars,
                          (void*)ObjSeq_preempt,
                          (void*)ObjSeq_yield,
                          (void*)ObjSeq_getGlobal3,
                          (void*)ObjSeq_setGlobal3,
                          (void*)ObjSeq_getGlobal1,
                          (void*)ObjSeq_setGlobal1,
                          (void*)ObjSeq_getGlobal2,
                          (void*)ObjSeq_setGlobal2,
                          (void*)ObjSeq_setXrot,
                          (void*)ObjSeq_TurnToFacePlayer,
                          (void*)ObjSeq_SetObjs,
                          (void*)ObjSeq_setOverridePos,
                          (void*)ObjSeq_SetCoordinateSpace};

char sEndObjSequenceMaxFreesError[41] = "endObjSequence: max number of obj frees\n\000";
char sObjSequenceMissingObjectFormat[38] = " SEQUENCE: Could not Find Object %i \n\000";
char sObjLoadAnimdataNullACRomTabWarning[45] = "<objLoadAnimdata>  Warning ACRomTab is NULL\n\000";

void ObjSeq_updateCamera(void)
{
    CamRequest block;
    CamFloats fblock;
    CamMode mode47;
    CamMode mode48;
    int groupObjCount;
    GameObject* obj;
    u8* model;
    u8* camObj;
    f32 x;
    f32 y;
    f32 z;
    s16 pitch;
    s16 yaw;
    s16 roll;
    int code;

    obj = lbl_803DD0B8;
    if (obj != NULL)
    {
        model = (u8*)obj->anim.placementData;
        if (lbl_803DD0F8 != 0)
        {
            x = lbl_803DD0F4;
            y = lbl_803DD0F0;
            z = lbl_803DD0EC;
        }
        else
        {
            x = obj->anim.worldPosX;
            y = obj->anim.worldPosY;
            z = obj->anim.worldPosZ;
        }
        pitch = obj->anim.rotX;
        yaw = obj->anim.rotY;
        roll = obj->anim.rotZ;
        if (obj->anim.parent != NULL)
        {
            pitch = (s16)(pitch + obj->anim.parentAnim->rotX);
        }
        lbl_803DD0DC = 1.0f;
        if ((s8)gObjSeqCameraActive == 0)
        {
            block.pos[0] = x;
            block.pos[1] = y;
            block.pos[2] = z;
            block.rot[0] = (s16)(0x8000 - pitch);
            block.rot[1] = (s16)-yaw;
            block.rot[2] = roll;
            if ((s8)gObjSeqFovOverrideActive != 0)
            {
                block.fov = gObjSeqFovOverrideValue;
                gObjSeqCameraFov = gObjSeqFovOverrideValue;
            }
            else
            {
                block.fov = gObjSeqCameraFov;
            }
            (*gCameraInterface)->setMode(OBJSEQ_CAMMODE_CAMDEBUG, 0, 1, 0x144, &block, model[0x24], 0xff);
            gObjSeqCameraActive = 1;
        }
        else
        {
            camObj = (*gCameraInterface)->getCamera();
            *(f32*)(camObj + 0x18) = x;
            *(f32*)(camObj + 0x1c) = y;
            *(f32*)(camObj + 0x20) = z;
            Obj_TransformWorldPointToLocal(*(f32*)(camObj + 0x18), *(f32*)(camObj + 0x1c), *(f32*)(camObj + 0x20),
                                           (f32*)(camObj + 0xc), (f32*)(camObj + 0x10), (f32*)(camObj + 0x14),
                                           (GameObject*)*(void**)(camObj + 0x30));
            *(s16*)camObj = (s16)(0x8000 - pitch);
            *(s16*)(camObj + 2) = (s16)-yaw;
            *(s16*)(camObj + 4) = roll;
            if ((s8)gObjSeqFovOverrideActive != 0)
            {
                *(f32*)(camObj + 0xb4) = gObjSeqFovOverrideValue;
                gObjSeqCameraFov = gObjSeqFovOverrideValue;
            }
            else
            {
                *(f32*)(camObj + 0xb4) = gObjSeqCameraFov;
            }
            gObjSeqSavedCamPosX = *(f32*)(camObj + 0x18);
            gObjSeqSavedCamPosY = *(f32*)(camObj + 0x1c);
            gObjSeqSavedCamPosZ = *(f32*)(camObj + 0x20);
            gObjSeqSavedCamPitch = *(s16*)camObj;
            gObjSeqSavedCamYaw = *(s16*)(camObj + 2);
            gObjSeqSavedCamRoll = *(s16*)(camObj + 4);
            gObjSeqSavedCamFov = *(f32*)(camObj + 0xb4);
        }
    }
    else
    {
        if ((s8)gObjSeqCameraActive != 0)
        {
            if (lbl_803DD064 == 0)
            {
                switch (gObjSeqCamMode)
                {
                case 0x47:
                    mode47.mode = gObjSeqCamModeArgB;
                    mode47.flag = gObjSeqCamModeArgC;
                    (*gCameraInterface)
                        ->setMode(OBJSEQ_CAMMODE_TESTSTRENGTH, 1, 3, 8, &mode47, gObjSeqCamModeArgD, 0xff);
                    break;
                case 0x48:
                    mode48.mode = gObjSeqCamModeArgB;
                    if ((code = gObjSeqCamModeArgD) == 0)
                    {
                        mode48.flag = 1;
                    }
                    (*gCameraInterface)->setMode(OBJSEQ_CAMMODE_STATIC, 1, 3, 8, &mode48, code, 0xff);
                    break;
                case 0x4a:
                    (*gCameraInterface)->setMode(OBJSEQ_CAMMODE_SHIPBATTLE, 1, 0, 0, NULL, gObjSeqCamModeArgD, 0xff);
                    break;
                case 0x4c:
                    block.posB[0] = gObjSeqSavedCamPosX;
                    block.posB[1] = gObjSeqSavedCamPosY;
                    block.posB[2] = gObjSeqSavedCamPosZ;
                    block.rot[0] = gObjSeqSavedCamPitch;
                    block.rot[1] = gObjSeqSavedCamYaw;
                    block.rot[2] = gObjSeqSavedCamRoll;
                    block.fov = gObjSeqSavedCamFov;
                    (*gCameraInterface)->setMode(OBJSEQ_CAMMODE_CAMDEBUG, 1, 0, 0x144, &block, 0, 0xff);
                    break;
                case 0x45:
                    (*gCameraInterface)->setMode(OBJSEQ_CAMMODE_CAMTALK, 1, 0, 0, NULL, gObjSeqCamModeArgD, 0xff);
                    break;
                case 0x44:
                    if (gObjSeqCamModeArgB != 0)
                    {
                        fblock.a = 90.0f;
                        fblock.b = 20.0f;
                        fblock.c = 5;
                        (*gCameraInterface)->setMode(OBJSEQ_CAMMODE_VIEWFINDER, 1, 1, 0xc, &fblock, 0, 0xff);
                    }
                    else
                    {
                        fblock.a = 90.0f;
                        fblock.b = 20.0f;
                        fblock.c = 0x1e;
                        (*gCameraInterface)->setMode(OBJSEQ_CAMMODE_VIEWFINDER, 1, 0, 0xc, &fblock, 0, 0xff);
                    }
                    break;
                case 0x49:
                    (*gCameraInterface)
                        ->setMode(OBJSEQ_CAMMODE_COMBAT, 1, 0, gObjSeqCamModeArgB, &gObjSeqCamModeArgC,
                                  gObjSeqCamModeArgD, 0xff);
                    break;
                case 0x53:
                    (*gCameraInterface)->setMode(OBJSEQ_CAMMODE_CLOUDRUNNER, 1, 0, 0, NULL, 0, 0xff);
                    break;
                case 0x56:
                    (*gCameraInterface)->setMode(OBJSEQ_CAMMODE_ARWING, 1, gObjSeqCamModeArgB, 0, NULL, 0, 0);
                    break;
                case 0x57:
                    (*gCameraInterface)->setMode(OBJSEQ_CAMMODE_TITLE, 0, 3, 0, NULL, 0, 0);
                    (*gCameraInterface)
                        ->setFocus(*(void**)(void*)ObjGroup_GetObjects(OBJSEQ_TARGET_OBJGROUP, &groupObjCount), 0);
                    break;
                default:
                    if (gObjSeqCamModeArgB == 0)
                    {
                        gObjSeqCamModeArgB = 1;
                    }
                    (*gCameraInterface)
                        ->setMode(OBJSEQ_CAMMODE_DEFAULT, 0, gObjSeqCamModeArgB, 0, NULL, gObjSeqCamModeArgD, 0xff);
                    break;
                }
            }
            gObjSeqCameraActive = 0;
            gObjSeqCameraFov = 60.0f;
            gObjSeqCamModeArgB = 1;
            gObjSeqCamModeArgD = 0x5a;
            gObjSeqCamMode = 0x42;
            curSeqNo = 0;
        }
        else
        {
            gObjSeqCamModeArgB = 1;
            gObjSeqCamModeArgD = 0x5a;
            gObjSeqCamMode = 0x42;
        }
    }

    gObjSeqFovOverrideActive = 0;
    lbl_803DD0B8 = NULL;
    lbl_803DD0F8 = 0;
}

void animatedObjFreeAndSavePlayerPos(GameObject* obj, GameObject* seqObj, u8* seq)
{
    void (*callback)(void* ctx, u8* obj);
    GameObject* player;
    int clearBit;

    callback = ((ObjSeqState*)seq)->freeCallback;
    if (callback != NULL)
    {
        callback(((ObjSeqState*)seq)->callbackContext, (u8*)obj);
        ((ObjSeqState*)seq)->freeCallback = NULL;
    }

    if ((s8)((ObjSeqState*)seq)->slot == gObjSeqPreparingStreamSlot)
    {
        AudioStream_CancelPrepared();
        gObjSeqPreparingStreamSlot = -1;
    }

    if (((ObjSeqState*)seq)->runState != 0)
    {
        if ((s8)((ObjSeqState*)seq)->unk7B != 0)
        {
            ((ObjSeqState*)seq)->unk7B = 0;
        }
        if (((ObjSeqState*)seq)->targetObj != NULL)
        {
            seqObj->pendingParentObj = NULL;
            seqObj->objectFlags &= ~OBJECT_OBJFLAG_SEQ_ATTACHED;
            ((ObjSeqState*)seq)->targetObj = NULL;
        }
    }

    if ((((u32)((ObjSeqState*)seq)->flags136[0] >> 2) & 1U) != 0U)
    {
        player = Obj_GetPlayerObject();
        (*gMapEventInterface)->savePoint((int)&player->anim.localPosX, player->anim.rotX, 0, getCurMapLayer());
        clearBit = 0;
        {
            struct SeqByte136
            {
                u8 b80 : 1, b40 : 1, b20 : 1, b10 : 1, b08 : 1, b04 : 1, b02 : 1, b01 : 1;
            };
            ((struct SeqByte136*)&((ObjSeqState*)seq)->flags136[0])->b04 = clearBit;
        }
    }

    ((ObjSeqState*)seq)->runState = 0;
}


f32 objCurveInterpolate(ObjCurveKey* keys, int count, int frame)
{
    int index;
    int mode;
    int prevIndex;
    int keyIndex;
    ObjCurveKey* key;
    ObjCurveKey* prev;
    f32 values[4];
    f32 deltaNext;
    f32 deltaPrev;
    f32 span;
    f32 t;

    if (count <= 0)
    {
        return 0.0f;
    }

    index = 0;
    while (index < count && keys[index].frame < frame)
    {
        index++;
    }

    if (index == count)
    {
        return keys[count - 1].value;
    }
    if (index == 0)
    {
        return keys[0].value;
    }
    if (frame == keys[index].frame)
    {
        t = keys[index].value;
        mode = keys[index].tangentAndMode & 3;
        if (mode > 1 && index < count - 1)
        {
            t = keys[index + 1].value;
        }
        return t;
    }

    prevIndex = index - 1;
    prev = &keys[prevIndex];
    mode = prev->tangentAndMode & 3;
    values[0] = prev->value;
    if (mode == 0)
    {
        deltaNext = prev[1].value - values[0];
        if (prevIndex > 0)
        {
            deltaPrev = values[0] - prev[-1].value;
        }
        else
        {
            deltaPrev = deltaNext;
        }
        if (deltaNext < 0.0f)
        {
            deltaNext = -deltaNext;
        }
        if (deltaPrev < 0.0f)
        {
            deltaPrev = -deltaPrev;
        }
        deltaPrev = deltaNext + deltaPrev;
        t = deltaPrev / 16.0f;
        values[2] = t * (f32)(prev->tangentAndMode >> 2);
    }

    span = (f32)(keys[prevIndex + 1].frame - keys[prevIndex].frame);
    keyIndex = index;
    if (index < count)
    {
        key = &keys[keyIndex];
        values[1] = key->value;
        if (mode == 0)
        {
            index++;
            if (index < count)
            {
                deltaPrev = key[1].value - values[1];
            }
            else
            {
                deltaPrev = deltaNext;
            }
            if (deltaPrev < 0.0f)
            {
                deltaPrev = -deltaPrev;
            }
            deltaPrev = deltaNext + deltaPrev;
            t = deltaPrev / 16.0f;
            values[3] = t * (f32)(keys[keyIndex].tangentAndMode >> 2);
        }
    }

    if (span > 0.0f)
    {
        t = (f32)(frame - keys[keyIndex - 1].frame) / span;
        if (mode == 0)
        {
            return Curve_EvalHermite(values, t, NULL);
        }
        if (mode == 1)
        {
            return t * (values[1] - values[0]) + values[0];
        }
        return values[1];
    }
    return values[1];
}

int objSeqExecCmd06(GameObject* obj, GameObject* sourceObj, u8* seq, int cmd, s8 flag)
{
    u8* base = gObjSeqRuntimeBuffer;
    ObjAnimComponent* sourceAnim = &sourceObj->anim;
    u32 cmdByte;
    int cmdArg = (cmd >> 8) & 0xff;
    int pair[2];
    GameObject* player;
    u8 flags;
    u8* slotFlags;
    int trackId;
    int slot;
    int off;
    int* streams;
    f32 dist;
    f32 strength;

    cmdByte = cmd & 0xff;
    switch (cmdByte)
    {
    case 2:
        if (flag != 0)
        {
            break;
        }
        pair[0] = 0x19;
        pair[1] = 0x15;
        if (((ObjSeqState*)seq)->curveId < 0)
        {
            ((ObjSeqState*)seq)->curveId = (*gRomCurveInterface)->find(
                obj->anim.localPosX, obj->anim.localPosY,
                obj->anim.localPosZ, pair, 2, cmdArg);
            if (((ObjSeqState*)seq)->curveId > -1)
            {
                if (((ObjSeqState*)seq)->curveInterp != NULL)
                {
                    mm_free(((ObjSeqState*)seq)->curveInterp);
                    ((ObjSeqState*)seq)->curveInterp = NULL;
                }
                ((ObjSeqState*)seq)->curveInterp = mmAlloc(0x2c, 0x11, 0);
                if (((ObjSeqState*)seq)->curveInterp != NULL)
                {
                    RomCurveInterp_InitFromNode(((ObjSeqState*)seq)->curveInterp, ((ObjSeqState*)seq)->curveId);
                }
                else
                {
                    ((ObjSeqState*)seq)->curveId = -1;
                }
            }
        }
        break;
    case 9:
        if (flag != 0)
        {
            break;
        }
        ((ObjSeqState*)seq)->stateFlags |= 1;
        break;
    case 18:
        if (flag != 0)
        {
            break;
        }
        slotFlags = base + (s8)((ObjSeqState*)seq)->slot;
        flags = *(slotFlags += 0x3538);
        if ((flags & 0x10) != 0)
        {
            *slotFlags = flags & ~0x10;
        }
        else
        {
            *slotFlags = flags | 0x10;
        }
        break;
    case 14:
        if (flag != 0)
        {
            break;
        }
        if ((s8)(base + (s8)((ObjSeqState*)seq)->slot)[0x3a40] == 0)
        {
            (*gScreenTransitionInterface)->start(cmdArg, 1);
        }
        break;
    case 15:
        if (flag != 0)
        {
            break;
        }
        if ((s8)(base + (s8)((ObjSeqState*)seq)->slot)[0x3a40] == 0)
        {
            (*gScreenTransitionInterface)->step(cmdArg, 1);
        }
        break;
    case 20:
        gObjSeqCamMode = 0x47;
        gObjSeqCamModeArgB = cmdArg & 0x7f;
        gObjSeqCamModeArgC = 1;
        gObjSeqCamModeArgD = 0x78;
        break;
    case 23:
        if (flag != 0)
        {
            break;
        }
        if (cmdArg >= sourceAnim->modelInstance->modelCount)
        {
            break;
        }
        if (sourceObj->anim.classId == 1)
        {
            if (((s16*)(base + 0x3a98))[(s8)((ObjSeqState*)seq)->slot] - 1 != 0x45)
            {
                break;
            }
            if (cmdArg == 1)
            {
                cmdArg = 0;
            }
            playerSetDisguised(sourceObj, cmdArg);
        }
        else
        {
            Obj_SetActiveModelIndex(sourceObj, cmdArg);
        }
        break;
    case 24:
        if (sourceObj->anim.classId == 1)
        {
            playerPullOutStaff(sourceObj, cmdArg);
        }
        break;
    case 25:
        if (sourceObj->anim.classId == 1)
        {
            playerPutAwayStaff(sourceObj, cmdArg);
        }
        break;
    case 26:
        gObjSeqCamMode = 0x42;
        gObjSeqCamModeArgB = 4;
        gObjSeqCamModeArgC = 0;
        gObjSeqCamModeArgD = 0;
        break;
    case 33:
        ((ObjSeqState*)seq)->flags = ((ObjSeqState*)seq)->flags | 0x400;
        ((SeqByte136*)&((ObjSeqState*)seq)->flags136[0])->modelSlot = cmdArg;
        break;
    case 34:
        ((ObjSeqState*)seq)->flags = ((ObjSeqState*)seq)->flags & ~0x400;
        ((SeqByte136*)&((ObjSeqState*)seq)->flags136[0])->modelSlot = 0;
        break;
    case 35:
        ((SeqByte136*)&((ObjSeqState*)seq)->flags136[0])->mapEvent = 1;
        break;
    case 36:
        (*gMapEventInterface)->savePoint(0, 0, 1, getCurMapLayer());
        break;
    case 38:
        playerLock(Obj_GetPlayerObject(), cmdArg);
        break;
    case 44:
        setMotionBlur(1, cmdArg / 10.0f);
        break;
    case 45:
        setMotionBlur(0, 0.0f);
        break;
    case 46:
        Rcp_SetMonochromeFilterEnabled(1);
        break;
    case 47:
        Rcp_SetMonochromeFilterEnabled(0);
        break;
    case 48:
        mainSetBits(GAMEBIT_ENV_isOutdoor, 1);
        getEnvfxActVoid(Obj_GetPlayerObject(), Obj_GetPlayerObject(), OBJSEQ_ENVFX_A0, 0);
        getEnvfxActVoid(Obj_GetPlayerObject(), Obj_GetPlayerObject(), OBJSEQ_ENVFX_A1, 0);
        getEnvfxActVoid(Obj_GetPlayerObject(), Obj_GetPlayerObject(), OBJSEQ_ENVFX_A2, 0);
        break;
    case 49:
        mainSetBits(GAMEBIT_ENV_isOutdoor, 1);
        getEnvfxActVoid(Obj_GetPlayerObject(), Obj_GetPlayerObject(), OBJSEQ_ENVFX_B0, 0);
        getEnvfxActVoid(Obj_GetPlayerObject(), Obj_GetPlayerObject(), OBJSEQ_ENVFX_B1, 0);
        getEnvfxActVoid(Obj_GetPlayerObject(), Obj_GetPlayerObject(), OBJSEQ_ENVFX_B2, 0);
        break;
    case 50:
        mainSetBits(GAMEBIT_ENV_isOutdoor, 0);
        getEnvfxActVoid(Obj_GetPlayerObject(), Obj_GetPlayerObject(), OBJSEQ_ENVFX_A0, 0);
        getEnvfxActVoid(Obj_GetPlayerObject(), Obj_GetPlayerObject(), OBJSEQ_ENVFX_A1, 0);
        getEnvfxActVoid(Obj_GetPlayerObject(), Obj_GetPlayerObject(), OBJSEQ_ENVFX_A2, 0);
        skyRefreshPlayerEnvFx();
        break;
    }

    switch (cmdByte)
    {
    case 0:
        gObjSeqStop = 1;
        return 0;
    case 7:
        if (flag != 0)
        {
            break;
        }
        CameraShake_Enable();
        player = Obj_GetPlayerObject();
        if (player == NULL)
        {
            break;
        }
        dist = Vec_xzDistance(&player->anim.worldPosX, &obj->anim.worldPosX);
        strength = 2.0f * (f32)(cmdArg - 7) + 1.0f;
        if (dist < 200.0f)
        {
            if (dist > 50.0f)
            {
                strength *= 1.0f - (dist - 50.0f) / 150.0f;
            }
            CameraShake_StartDampened(gObjSeqShakeAmplitude * strength, gObjSeqShakeAmplitude * strength,
                              gObjSeqShakeAmplitude);
        }
        break;
    case 10:
        gameTimerInit(0x12, cmdArg);
        break;
    case 11:
        gameTimerInit(0x11, cmdArg);
        break;
    case 12:
        timerSetToCountUp();
        break;
    case 37:
        gameTimerStop();
        break;
    case 13:
        Sfx_StopObjectChannel((u32)sourceObj, 0x7f);
        break;
    case 16:
        *(s8*)&((ObjSeqState*)seq)->unk7D = cmdArg;
        break;
    case 21:
        gObjSeqCamMode = 0x48;
        gObjSeqCamModeArgB = cmdArg & 0x7f;
        gObjSeqCamModeArgC = 1;
        gObjSeqCamModeArgD = 0x78;
        break;
    case 51:
        gObjSeqCamModeArgD = cmdArg;
        break;
    case 23:
        if (flag != 0)
        {
            break;
        }
        if (sourceObj->anim.classId == 1)
        {
            break;
        }
        if (cmdArg >= sourceAnim->modelInstance->modelCount)
        {
            break;
        }
        Obj_SetActiveModelIndex(sourceObj, cmdArg);
        break;
    case 27:
        (*gMapEventInterface)->setObjGroupStatus(sourceAnim->mapEventSlot, cmdArg, 1);
        break;
    case 28:
        (*gMapEventInterface)->setObjGroupStatus(sourceAnim->mapEventSlot, cmdArg, 0);
        break;
    case 29:
        (*gMapEventInterface)->setMapAct(sourceAnim->mapEventSlot, cmdArg);
        break;
    case 19:
        if (flag != 0)
        {
            break;
        }
        (base + (s8)((ObjSeqState*)seq)->slot)[0x3538] &= ~0x10;
        break;
    case 30:
        if (flag != 0)
        {
            break;
        }
        (base + (s8)((ObjSeqState*)seq)->slot)[0x3538] |= 0x10;
        break;
    case 31:
        (*gMapEventInterface)->clearRestartPoint();
        break;
    case 32:
        (*gMapEventInterface)->gotoRestartPoint();
        break;
    case 39:
        if (gObjSeqPreparingStreamSlot == (s8)((ObjSeqState*)seq)->slot)
        {
            gObjSeqStreamResumeOffset = (int)((f32*)(base + 0x3894))[(s8)((ObjSeqState*)seq)->slot];
            gObjSeqStreamStopped = ObjSeq_StartPreparedStream(((ObjSeqState*)seq)->slot) == 0;
        }
        break;
    case 40:
        slot = ((ObjSeqState*)seq)->slot;
        if (base[slot + 0x3334] == 0)
        {
            trackId = (u32)(((s16*)(base + 0x3a98))[slot] - 1) & 0x3fff;
            gObjSeqCurrentTrackId = trackId;
            streams = (int*)seqPairTableLookup(gObjSeqStreamTableA, OBJSEQ_STREAM_MAP_COUNT, trackId);
            if (streams != NULL)
            {
                off = cmdArg * 4;
                if (AudioStream_Play(*(int*)((u8*)streams + off), ObjSeq_AudioStreamCallback) != 0)
                {
                    gObjSeqPreparingStreamSlot = slot;
                }
                streams = (int*)seqPairTableLookup(gObjSeqStreamTableB, OBJSEQ_STREAM_MAP_COUNT, trackId);
                if (streams != NULL)
                {
                    gObjSeqSubtitleId = *(int*)((u8*)streams + off);
                }
            }
        }
        break;
    }
    return 1;
}

void ObjSeq_setCamVars(int camA, int camB, int camC, int camD)
{
    gObjSeqCamMode = camA;
    gObjSeqCamModeArgB = camB;
    gObjSeqCamModeArgC = camC;
    gObjSeqCamModeArgD = camD;
}

int seqDoSubCmd0B(GameObject* obj, GameObject* sourceObj, u8* seq, u8* cmdsArg, s16 xrot, s16 countArg, s8 flag1, s8 flag2)
{
    u8* cmds;
    int count;
    int opcode;
    int operand;
    int top16;
    int subId;
    int i;
    int freeSlot;
    u32 packed;
    int result;
    int j;
    int found;
    int eventIdx;
    u8 eventId;
    u8 slotVal;
    int slot;

    i = 0;
    cmds = cmdsArg;
    count = countArg;
    for (; i < count; i++)
    {
        packed = *(u32*)cmds;
        opcode = packed & 0x3f;
        operand = (packed >> 6) & 0x3ff;
        top16 = packed >> 16;
        if (opcode == 2 || opcode == 3)
        {
            if ((top16 & 0x8000) != 0)
            {
                top16 |= 0xffff0000;
            }
            subId = operand;
            operand = 0;
        }

        result = 0;
        switch (opcode)
        {
        case 6:
            if (objSeqExecCmd06(obj, sourceObj, seq, operand | (top16 << 8), flag2) == 0)
            {
                return 1;
            }
            result = -1;
            operand = 0;
            break;
        case 7:
            if (sourceObj != obj)
            {
                switch ((s8)gObjSeqMsgSendModes[operand])
                {
                case 1:
                    ObjMsg_SendToObjects(0, 2, obj, gObjSeqMsgIds[operand], (u32)obj);
                    break;
                case 2:
                    ObjMsg_SendToNearbyObjects(0, 600.0f, 2, obj, gObjSeqMsgIds[operand], (u32)obj);
                    break;
                default:
                    ObjMsg_SendToObject(sourceObj, gObjSeqMsgIds[operand], obj, 0);
                    break;
                }
            }
            result = -1;
            operand = 0;
            break;
        case 8:
            if (flag2 == 0)
            {
                found = 0;
                freeSlot = -1;
                for (j = 0; j < 10; j++)
                {
                    slotVal = seq[j + 0x12c];
                    if (slotVal == operand)
                    {
                        found = 1;
                    }
                    if (slotVal == 0)
                    {
                        freeSlot = j;
                    }
                }
                if (found == 0 && freeSlot != -1)
                {
                    seq[freeSlot + 0x12c] = operand;
                    *(s16*)(seq + freeSlot * 2 + 0x118) = objSeqFindLabel(seq, top16);
                }
                result = 0;
            }
            break;
        case 9:
            break;
        default:
            result = ObjSeq_EvaluateCondition(operand, seq, obj->anim.placementDataAddress);
            break;
        }

        if (result > 0 && flag1 == 0)
        {
            switch (opcode)
            {
            case 1:
                if (flag2 != 0)
                {
                    break;
                }
                slot = (s8)((ObjSeqState*)seq)->slot;
                if ((s8)gObjSeqJumpLatch[slot] == 0)
                {
                    gObjSeqJumpLatch[slot] = 1;
                    ((ObjSeqState*)seq)->curFrame = top16;
                    ((ObjSeqState*)seq)->prevFrame = ((ObjSeqState*)seq)->curFrame;
                }
                return 1;
            case 10:
                if (flag2 != 0)
                {
                    break;
                }
                slot = (s8)((ObjSeqState*)seq)->slot;
                if ((s8)gObjSeqJumpLatch[slot] == 0)
                {
                    gObjSeqJumpLatch[slot] = 1;
                    ((ObjSeqState*)seq)->curFrame = objSeqFindLabel(seq, top16);
                    ((ObjSeqState*)seq)->prevFrame = ((ObjSeqState*)seq)->curFrame;
                }
                return 1;
            case 2:
                switch (subId)
                {
                case 0:
                    eventId = top16;
                    ((ObjSeqState*)seq)->unk80 = eventId;
                    eventIdx = ((ObjSeqState*)seq)->eventCount;
                    if ((u32)eventIdx < 10)
                    {
                        ((ObjSeqState*)seq)->eventCount += 1;
                        ((ObjSeqState*)seq)->eventIds[eventIdx] = eventId;
                    }
                    break;
                case 1:
                    ((ObjSeqState*)seq)->seqCounter = top16;
                    break;
                case 3:
                    seqGlobal1 = top16;
                    break;
                case 4:
                    seqGlobal2 = top16;
                    break;
                case 5:
                    gObjSeqBoolFlags[(s8)((ObjSeqState*)seq)->slot] = top16;
                    break;
                case 6:
                    mainSetBits(((ObjSeqState*)seq)->gameBit, top16 != 0);
                    break;
                case 2:
                    break;
                }
                break;
            case 3:
                if (flag2 != 0)
                {
                    break;
                }
                switch (subId)
                {
                case 0:
                    ((ObjSeqState*)seq)->seqCounter = ((ObjSeqState*)seq)->seqCounter + top16;
                    break;
                case 1:
                    break;
                }
                break;
            case 4:
                if (flag2 != 0)
                {
                    break;
                }
                ((ObjSeqState*)seq)->curFrame = xrot;
                ((ObjSeqState*)seq)->prevFrame = xrot;
                ((ObjSeqState*)seq)->pendingConditionId = (s8)(operand + 1);
                gObjSeqJumpLatch[(s8)((ObjSeqState*)seq)->slot] = 1;
                return 1;
            case 5:
                if (flag2 != 0)
                {
                    break;
                }
                return 0;
            case 0:
            case 6:
            case 7:
            case 8:
            case 9:
                break;
            }
        }
        cmds += 4;
    }
    return 0;
}

int ObjSeq_EvaluateCondition(int condition, u8* seq, int obj)
{
    f32 sunTime;
    int result;

    result = 0;

    switch (condition)
    {
    case OBJSEQ_COND_SEQCOUNTER_LT1:
        if (((ObjSeqState*)seq)->seqCounter <= 0)
        {
            result = 1;
        }
        break;
    case OBJSEQ_COND_SEQCOUNTER_GT0:
        if (((ObjSeqState*)seq)->seqCounter > 0)
        {
            result = 1;
        }
        break;
    case OBJSEQ_COND_DAYTIME:
        if ((*gSkyInterface)->getSunPosition(&sunTime) == 0)
        {
            result = 1;
        }
        break;
    case OBJSEQ_COND_NIGHTTIME:
        if ((*gSkyInterface)->getSunPosition(&sunTime) != 0)
        {
            result = 1;
        }
        break;
    case OBJSEQ_COND_BOOL_EQ0:
        if (gObjSeqBoolFlags[(s8)((ObjSeqState*)seq)->slot] == 0)
        {
            result = 1;
        }
        break;
    case OBJSEQ_COND_BOOL_EQ1:
        if (gObjSeqBoolFlags[(s8)((ObjSeqState*)seq)->slot] == 1)
        {
            result = 1;
        }
        break;
    case OBJSEQ_COND_VAR1_EQ0:
        if (gObjSeqCondFlags[(s8)((ObjSeqState*)seq)->slot] == 0)
        {
            result = 1;
        }
        break;
    case OBJSEQ_COND_VAR1_NE0:
        if (gObjSeqCondFlags[(s8)((ObjSeqState*)seq)->slot] != 0)
        {
            result = 1;
        }
        break;
    case OBJSEQ_COND_GLOBAL1_LE0:
        if (seqGlobal1 <= 0)
        {
            result = 1;
        }
        break;
    case OBJSEQ_COND_GLOBAL1_GT0:
        if (seqGlobal1 > 0)
        {
            result = 1;
        }
        break;
    case OBJSEQ_COND_GLOBAL2_LE0:
        if (seqGlobal2 <= 0)
        {
            result = 1;
        }
        break;
    case OBJSEQ_COND_GLOBAL2_GT0:
        if (seqGlobal2 > 0)
        {
            result = 1;
        }
        break;
    case OBJSEQ_COND_TIMER_DISABLED:
        if (isGameTimerDisabled() != 0)
        {
            result = 1;
        }
        break;
    case OBJSEQ_COND_TIMER_ENABLED:
        if (isGameTimerDisabled() == 0)
        {
            result = 1;
        }
        break;
    case OBJSEQ_COND_GLOBAL3_NE0:
        if (seqGlobal3 != 0)
        {
            result = 1;
        }
        break;
    case OBJSEQ_COND_GLOBAL3_EQ0:
        if (seqGlobal3 == 0)
        {
            result = 1;
        }
        break;
    case 16:
    case 17:
    default:
        result = 1;
        break;
    }
    return result;
}

void RomCurveInterp_BuildSegmentTimeTable(RomCurveInterpState* out, RomCurveNode* curve, RomCurveNode* next, f32 t,
                                          int flag)
{
    f32 curveScale;
    f32 nextScale;
    f32 xPoints[4];
    f32 yPoints[4];
    f32 zPoints[4];
    f32 xSamples[9];
    f32 ySamples[9];
    f32 zSamples[9];
    f32* times;
    f32 dx;
    f32 dy;
    f32 dz;
    int i;

    curveScale = ROM_CURVE_NODE_SCALE(curve);
    nextScale = ROM_CURVE_NODE_SCALE(next);

    xPoints[0] = curve->x;
    xPoints[2] = curveScale * mathSinf(ROM_CURVE_NODE_ANGLE(curve->yaw));
    xPoints[1] = next->x;
    xPoints[3] = nextScale * mathSinf(ROM_CURVE_NODE_ANGLE(next->yaw));

    yPoints[0] = curve->y;
    yPoints[2] = curveScale * mathSinf(ROM_CURVE_NODE_ANGLE(curve->pitch));
    yPoints[1] = next->y;
    yPoints[3] = nextScale * mathSinf(ROM_CURVE_NODE_ANGLE(next->pitch));

    zPoints[0] = curve->z;
    zPoints[2] = curveScale * mathCosf(ROM_CURVE_NODE_ANGLE(curve->yaw));
    zPoints[1] = next->z;
    zPoints[3] = nextScale * mathCosf(ROM_CURVE_NODE_ANGLE(next->yaw));

    Curve_SampleSegmentPoints(xPoints, yPoints, zPoints, xSamples, ySamples, zSamples, 8, Curve_BuildHermiteCoeffs);

    times = (f32*)out;
    times[2] = 0.0f;
    for (i = 0; i < 8; i++)
    {
        dx = xSamples[i + 1] - xSamples[i];
        dy = ySamples[i + 1] - ySamples[i];
        dz = zSamples[i + 1] - zSamples[i];
        times[i + 3] = times[i + 2] + sqrtf(dx * dx + dy * dy + dz * dz);
    }
    if ((s8)flag == 1)
    {
        t -= out->toTime;
    }
    for (i = 0; i <= 8; i++)
    {
        times[i + 2] += t;
    }
}

void RomCurveInterp_UpdateSegmentWindow(RomCurveInterpState* state, f32 t)
{
    RomCurveNode* prev;
    RomCurveNode* fromNode;
    RomCurveNode* toNode;
    int found;
    int i;
    int mask;
    int val;
    f32 thr;

    fromNode = NULL;
    if (t < state->fromTime)
    {
        fromNode = (RomCurveNode*)(*gRomCurveInterface)->getById(state->fromNodeId);
    }
    if (fromNode != NULL)
    {
        while (t < (thr = state->fromTime))
        {
            mask = 1;
            for (i = 0; i < 4; i++)
            {
                val = fromNode->links[i];
                if (val > -1 && (fromNode->directionMask & mask) != 0)
                {
                    found = val;
                    i = 5;
                }
                mask <<= 1;
            }
            if (i != 6)
            {
                state->toTime = thr;
                state->toNodeId = state->fromNodeId;
                state->fromNodeId = -1;
                return;
            }
            state->toNodeId = state->fromNodeId;
            state->fromNodeId = found;
            prev = fromNode;
            fromNode = (RomCurveNode*)(*gRomCurveInterface)->getById(state->fromNodeId);
            RomCurveInterp_BuildSegmentTimeTable(state, fromNode, prev, state->fromTime, 1);
        }
    }
    toNode = (RomCurveNode*)(*gRomCurveInterface)->getById(state->toNodeId);
    if (toNode == NULL)
    {
        return;
    }
    while (t >= (thr = state->toTime))
    {
        mask = 1;
        for (i = 0; i < 4; i++)
        {
            val = toNode->links[i];
            if (val > -1 && (toNode->directionMask & mask) == 0)
            {
                found = val;
                i = 5;
            }
            mask <<= 1;
        }
        if (i != 6)
        {
            state->fromTime = thr;
            state->fromNodeId = state->toNodeId;
            state->toNodeId = -1;
            return;
        }
        state->fromNodeId = state->toNodeId;
        state->toNodeId = found;
        prev = toNode;
        toNode = (RomCurveNode*)(*gRomCurveInterface)->getById(state->toNodeId);
        RomCurveInterp_BuildSegmentTimeTable(state, prev, toNode, state->toTime, 0);
    }
}

void RomCurveInterp_InitFromNode(RomCurveInterpState* out, int id)
{
    RomCurveNode* curve;
    int i;
    int mask;
    int found;
    int val;

    out->fromNodeId = id;
    out->toNodeId = -1;
    curve = (RomCurveNode*)(*gRomCurveInterface)->getById(out->fromNodeId);
    mask = 1;
    for (i = 0; i < 4; i++)
    {
        val = curve->links[i];
        if (val > -1 && (curve->directionMask & mask) == 0)
        {
            found = val;
            i = 5;
        }
        mask <<= 1;
    }
    if (i != 6)
    {
        out->fromNodeId = -1;
    }
    else
    {
        out->toNodeId = found;
        RomCurveInterp_BuildSegmentTimeTable(out, curve, (RomCurveNode*)(*gRomCurveInterface)->getById(out->toNodeId),
                                             0.0f, 0);
    }
}

int RomCurveInterp_EvaluateOffsetPosition(RomCurveInterpState* state, f32* offset, f32* outPos, s16* outAngle,
                                          int ignoreY)
{
    RomCurveNode* from;
    RomCurveNode* to;
    f32 segmentT;
    f32 t;
    f32 fromScale;
    f32 toScale;
    f32 xPoints[4];
    f32 yPoints[4];
    f32 zPoints[4];
    f32 xTangent;
    f32 yTangent;
    f32 zTangent;
    f32 length;
    f32 scale;
    int segment;
    int i;

    t = offset[2];
    RomCurveInterp_UpdateSegmentWindow(state, t);
    from = (RomCurveNode*)(*gRomCurveInterface)->getById(state->fromNodeId);
    if (from != NULL && state->toNodeId > -1)
    {
        to = (RomCurveNode*)(*gRomCurveInterface)->getById(state->toNodeId);
        i = 0;
        while (i <= 8 && t >= *(f32*)((u8*)state + (i << 2) + 8))
        {
            i++;
        }
        segment = i - 1;
        {
            f32* times = (f32*)state;
            segmentT = segment;
            segmentT += (t - times[segment + 2]) / (times[segment + 3] - times[segment + 2]);
        }
        segmentT = segmentT / 8.0f;

        fromScale = ROM_CURVE_NODE_SCALE(from);
        toScale = ROM_CURVE_NODE_SCALE(to);

        xPoints[0] = from->x;
        xPoints[2] = fromScale * mathSinf(ROM_CURVE_NODE_ANGLE(from->yaw));
        xPoints[1] = to->x;
        xPoints[3] = toScale * mathSinf(ROM_CURVE_NODE_ANGLE(to->yaw));

        yPoints[0] = from->y;
        yPoints[2] = fromScale * mathSinf(ROM_CURVE_NODE_ANGLE(from->pitch));
        yPoints[1] = to->y;
        yPoints[3] = toScale * mathSinf(ROM_CURVE_NODE_ANGLE(to->pitch));

        zPoints[0] = from->z;
        zPoints[2] = fromScale * mathCosf(ROM_CURVE_NODE_ANGLE(from->yaw));
        zPoints[1] = to->z;
        zPoints[3] = toScale * mathCosf(ROM_CURVE_NODE_ANGLE(to->yaw));

        {
            outPos[0] = Curve_EvalHermite(xPoints, segmentT, &xTangent);
            if ((s8)ignoreY == 0)
            {
                outPos[1] = Curve_EvalHermite(yPoints, segmentT, &yTangent);
            }
            outPos[2] = Curve_EvalHermite(zPoints, segmentT, &zTangent);
        }

        length = sqrtf(xTangent * xTangent + zTangent * zTangent);
        if (length > 0.1f)
        {
            scale = offset[0] / length;
            *outAngle = (s16)(getAngle(xTangent, zTangent) + 0x8000);
            xTangent *= scale;
            zTangent *= scale;
            outPos[0] += zTangent;
            outPos[2] -= xTangent;
            if ((s8)ignoreY == 0)
            {
                outPos[1] += offset[1];
            }
        }
    }
    else
    {
        if (from == NULL)
        {
            from = (RomCurveNode*)(*gRomCurveInterface)->getById(state->toNodeId);
        }
        if (from != NULL)
        {
            outPos[0] = from->x;
            if ((s8)ignoreY == 0)
            {
                outPos[1] = from->y + offset[1];
            }
            outPos[2] = from->z;
            outPos[0] += offset[0] * mathCosf(ROM_CURVE_NODE_ANGLE(from->yaw));
            outPos[2] += offset[0] * mathSinf(ROM_CURVE_NODE_ANGLE(from->yaw));
            *outAngle = (s16)(((s32)from->yaw << 8) + 0x8000);
        }
        else
        {
            return 0;
        }
    }
    return 1;
}

void ObjSeq_UpdateCurvePosition(GameObject* obj, u8* seq)
{
    GameObject* object;
    ObjSeqState* state;
    ObjSeqPlacement* placement;
    RomCurveNode* node;
    f32 outPos[3];
    f32 offset[3];
    f32 dx;
    f32 dy;
    f32 dz;
    f32 angleCos;
    f32 angleSin;

    object = obj;
    state = (ObjSeqState*)seq;
    placement = (ObjSeqPlacement*)object->anim.placementData;
    if (placement == NULL)
    {
        return;
    }

    if (state->curveId < 0)
    {
        dx = object->anim.localPosX - placement->baseX;
        dz = object->anim.localPosZ - placement->baseZ;
        angleSin = mathSinf((3.1415927f * (f32)state->heading) / 32768.0f);
        angleCos = mathCosf((3.1415927f * (f32)state->heading) / 32768.0f);
        object->anim.localPosX = angleSin * dz + (angleCos * dx + placement->baseX);
        object->anim.localPosZ = -(angleSin * dx - (angleCos * dz + placement->baseZ));
        return;
    }

    node = (RomCurveNode*)(*gRomCurveInterface)->getById(state->curveId);
    if (node == NULL)
    {
        return;
    }

    dx = object->anim.localPosX - placement->baseX;
    dy = object->anim.localPosY - placement->groundOffset;
    dz = object->anim.localPosZ - placement->baseZ;
    offset[0] = dx;
    offset[1] = dy;
    offset[2] = dz;
    outPos[0] = object->anim.localPosX;
    outPos[1] = object->anim.localPosY;
    outPos[2] = object->anim.localPosZ;

    if (node->links[0] < 0)
    {
        object->anim.localPosX = outPos[0];
        object->anim.localPosY = outPos[1];
        object->anim.localPosZ = outPos[2];
        return;
    }

    if (RomCurveInterp_EvaluateOffsetPosition(state->curveInterp, offset, outPos, &state->heading,
                                              state->groundSnapEnabled) != 0)
    {
        object->anim.localPosX = outPos[0];
        object->anim.localPosY = outPos[1];
        object->anim.localPosZ = outPos[2];
        return;
    }

    angleSin = mathSinf((3.1415927f * (f32)state->heading) / 32768.0f);
    angleCos = mathCosf((3.1415927f * (f32)state->heading) / 32768.0f);
    object->anim.localPosX = angleSin * dz + (angleCos * dx + placement->baseX);
    object->anim.localPosZ = -(angleSin * dx - (angleCos * dz + placement->baseZ));
}
int objSeqFindLabel(u8* seq, int label)
{
    int commandCount;
    int commandIndex;
    u32 packed;
    int currentLabel;
    u8* command;

    currentLabel = 0;
    commandIndex = 0;
    commandCount = ((ObjSeqState*)seq)->cmdCount;
    while (commandIndex < commandCount)
    {
        command = ((ObjSeqState*)seq)->cmds + commandIndex * 4;
        if ((s8)command[0] == 0)
        {
            currentLabel = *(s16*)(command + 2);
        }
        else if ((s8)command[0] == 0xb)
        {
            if (*(s16*)(command + 2) > 0)
            {
                packed = *(u32*)(command + 4);
                if ((int)(packed & 0x3f) == 9 && (int)(packed >> 16) == label)
                {
                    return currentLabel;
                }
                commandIndex += *(s16*)(command + 2);
            }
        }
        currentLabel += command[1];
        commandIndex++;
    }
    return -1;
}

int objSeqFindConditional(u8* seq, GameObject* seqState)
{
    int currentLabel;
    int commandIndex;
    u8* command;
    u32 packed;

    currentLabel = -1;
    commandIndex = 0;
    while (commandIndex < ((ObjSeqState*)seq)->cmdCount)
    {
        command = ((ObjSeqState*)seq)->cmds + commandIndex * 4;
        if ((s8)command[0] == 0)
        {
            currentLabel = *(s16*)(command + 2);
        }
        else if ((s8)command[0] == 0xb)
        {
            if (*(s16*)(command + 2) > 0)
            {
                packed = *(u32*)(command + 4);
                if ((int)(packed & 0x3f) == 4 &&
                    ObjSeq_EvaluateCondition((packed >> 6) & 0x3ff, seq, seqState->anim.placementDataAddress) != 0)
                {
                    currentLabel -= 10;
                    if (currentLabel < 0)
                    {
                        currentLabel = 0;
                    }
                    return currentLabel;
                }
                commandIndex += *(s16*)(command + 2);
            }
        }
        currentLabel += command[1];
        commandIndex++;
    }
    return -1;
}
void objCallSeqFn(GameObject* obj, GameObject* sourceObj, u8* seq, int action)
{
    int callbackResult;
    s8 actionSlot;
    int movementState;
    int flags;
    u8* sourceModel;

    (void)action;

    sourceModel = (u8*)sourceObj->anim.placementData;
    obj->anim.previousLocalPosX = obj->anim.localPosX;
    obj->anim.previousLocalPosY = obj->anim.localPosY;
    obj->anim.previousLocalPosZ = obj->anim.localPosZ;
    obj->anim.previousWorldPosX = obj->anim.worldPosX;
    obj->anim.previousWorldPosY = obj->anim.worldPosY;
    obj->anim.previousWorldPosZ = obj->anim.worldPosZ;

    if (obj->animEventCallback != NULL)
    {
        callbackResult = (*(int (**)(GameObject*, GameObject*, u8*, int))&obj->animEventCallback)(obj, sourceObj, seq, action);
        if (callbackResult == 4)
        {
            gObjSeqStop = 1;
        }
        else if (callbackResult != 0)
        {
            actionSlot = ((ObjSeqState*)seq)->slot;
            if (gObjSeqSlotResults[actionSlot] < 2)
            {
                gObjSeqSlotResults[actionSlot] = callbackResult;
            }
        }
        ((ObjSeqState*)seq)->eventCount = 0;
        ((ObjSeqState*)seq)->unk80 = 0;
    }
    else
    {
        if ((s8)((ObjSeqState*)seq)->unk7B != 0)
        {
            ((ObjSeqState*)seq)->movementState = 0;
            return;
        }

        movementState = (s8)((ObjSeqState*)seq)->movementState;
        if (movementState >= 4)
        {
            if (ObjSeq_TurnToFacePlayer(obj, (ObjSeqState*)seq, 6, 0x1e, 0x50, -1, -1) != 0)
            {
                actionSlot = ((ObjSeqState*)seq)->slot;
                if (gObjSeqSlotResults[actionSlot] < 2)
                {
                    gObjSeqSlotResults[actionSlot] = 1;
                }
            }
        }
        else if (movementState != 0)
        {
            if (movementState != 2)
            {
                ((ObjSeqState*)seq)->posOffsetScale = 1.0f;
                ((ObjSeqState*)seq)->posOffsetX =
                    obj->anim.localPosX - sourceObj->anim.localPosX;
                ((ObjSeqState*)seq)->posOffsetY =
                    obj->anim.localPosY - sourceObj->anim.localPosY;
                ((ObjSeqState*)seq)->posOffsetZ =
                    obj->anim.localPosZ - sourceObj->anim.localPosZ;
                ((ObjSeqState*)seq)->movementState = 2;
            }
            if ((s8)sourceModel[0x20] == 1)
            {
                ((ObjSeqState*)seq)->posOffsetDecay = 0.016666668f;
                actionSlot = ((ObjSeqState*)seq)->slot;
                if (gObjSeqSlotResults[actionSlot] < 2)
                {
                    gObjSeqSlotResults[actionSlot] = 1;
                }
            }
            ((ObjSeqState*)seq)->posOffsetScale =
                ((ObjSeqState*)seq)->posOffsetScale - ((ObjSeqState*)seq)->posOffsetDecay * timeDelta;
            if (((ObjSeqState*)seq)->posOffsetScale <= 0.0f)
            {
                ((ObjSeqState*)seq)->movementState = 0;
            }
        }
    }

    flags = obj->anim.resetHitboxFlags;
    flags &= ~7;
    obj->anim.resetHitboxFlags = flags;
    Obj_GetWorldPosition(obj, &obj->anim.worldPosX, &obj->anim.worldPosY, &obj->anim.worldPosZ);
    if (obj->anim.hitReactState != NULL)
    {
        ((ObjHitsPriorityState*)obj->anim.hitReactState)->lastHitObject = 0;
        ((ObjHitsPriorityState*)obj->anim.hitReactState)->priorityHitCount = 0;
    }
    if (obj->anim.hitboxTransformState != NULL)
    {
        obj->anim.hitboxTransformState->contactObjectCount = 0;
    }
}

void objSeqDoBgCmds0D(u8* seq, GameObject* obj, int skipSpawns)
{
    ObjSeqBgCmd* cmd;
    int cmdObj;
    int cmdParam;
    void* resource;
    int transitionSlot;
    int scriptedButtons;

    if (gObjSeqInputOverrideActive != 0 && obj->seqIndex != (s8)((ObjSeqState*)seq)->slot)
    {
        (*gGameUIInterface)->setInputOverride(0, 0, 0);
    }

    while (lbl_803DD113 > 0)
    {
        lbl_803DD113--;
        cmd = &lbl_8039A5BC[lbl_803DD113];
        cmdParam = cmd->param;
        cmdObj = cmd->object;

        switch (cmd->opcode)
        {
        case 3:
            if ((u8)skipSpawns == 0)
            {
                (*gPartfxInterface)->spawnObject((void*)cmdObj, cmdParam, NULL, 0x10000, -1, NULL);
            }
            break;
        case 4:
            if ((u8)skipSpawns == 0)
            {
                return0xFFFF_80008B6C(cmdObj, 0, 0, 1, -1, (u8)cmdParam, 0);
            }
            break;
        case 5:
            if ((u8)skipSpawns == 0)
            {
                resource = Resource_Acquire((u16)(cmdParam + 0xab), 1);
                if (resource != NULL)
                {
                    (*(void (**)(int, int, int, int, int, int, int))((char*)*(int**)resource + 0x4))(
                        cmdObj, 0, 0, 1, -1, (u8)cmdParam, 0);
                }
                if (resource != NULL)
                {
                    Resource_Release(resource);
                }
            }
            break;
        case 9:
            if ((u8)skipSpawns == 0)
            {
                switch (cmdParam & 0x2f)
                {
                case 6:
                    transitionSlot = (cmdParam & 0xfc0) >> 4;
                    (*gScreenTransitionInterface)->start(transitionSlot, 3);
                    break;
                case 7:
                    transitionSlot = (cmdParam & 0xfc0) >> 4;
                    (*gScreenTransitionInterface)->step(transitionSlot, 3);
                    break;
                case 8:
                    transitionSlot = (cmdParam & 0xfc0) >> 4;
                    (*gScreenTransitionInterface)->start(transitionSlot, 2);
                    break;
                case 9:
                    transitionSlot = (cmdParam & 0xfc0) >> 4;
                    (*gScreenTransitionInterface)->step(transitionSlot, 2);
                    break;
                case 0xb:
                    transitionSlot = (cmdParam & 0xfc0) >> 4;
                    (*gScreenTransitionInterface)->start(transitionSlot, 4);
                    break;
                case 0xc:
                    transitionSlot = (cmdParam & 0xfc0) >> 4;
                    (*gScreenTransitionInterface)->stepWithBlend(transitionSlot, 4, 0.2f);
                    break;
                }
            }
            break;
        case 0xb:
            mainSetBits(cmdParam, 1);
            break;
        case 0xc:
            mainSetBits(cmdParam, 0);
            break;
        case 0xd:
            if ((u8)skipSpawns == 0)
            {
                scriptedButtons = gObjSeqScriptedButtonMasks[cmdParam];
                (*gGameUIInterface)->setInputOverride(scriptedButtons, 0, 0);
                if (gObjSeqScriptedButtonMasks[cmdParam] != -1)
                {
                    gObjSeqInputOverrideActive = 1;
                }
                else
                {
                    gObjSeqInputOverrideActive = 0;
                }
            }
            break;
        }
    }
}

int ObjSeq_ExecuteActionCommand(GameObject* obj, u8* action, u8** cmdPtr, s8 flags, void* out)
{
    u8* base = gObjSeqRuntimeBuffer;
    s8 noExec;
    s8 doUpdate;
    s8 flag8;
    s8 f;
    u8* seq;
    GameObject* activeObj;
    u8* cmd;
    ObjSeqPlacement* model;
    u8* animState;
    u8* act2;
    u8* st2;
    u8* entry;
    int opcode;
    int sub;
    int restart;
    int reps;
    int val;
    int slot;
    int minRot;
    f32 blend;
    f32 t;

    (void)out;

    cmd = *cmdPtr;
    f = (s8)flags;
    noExec = (s8)(f & 1);
    doUpdate = (s8)(f & 2);
    flag8 = (s8)(f & 8);
    if (noExec == 0)
    {
        doUpdate = 1;
    }
    seq = obj->extra;
    model = (ObjSeqPlacement*)obj->anim.placementData;
    activeObj = *(GameObject**)seq;
    if (activeObj == NULL)
    {
        activeObj = obj;
    }

    opcode = (s8)cmd[0];
    switch (opcode)
    {
    case SEQACT_ANIM:
        if (flag8 != 0)
        {
            break;
        }
        ((ObjSeqState*)seq)->moveId = (s16)(*(s16*)(cmd + 2) & 0xfff);
        if (activeObj->anim.classId == 1 && ((ObjSeqState*)seq)->moveId < 4)
        {
            ((ObjSeqState*)seq)->moveId += 0x531;
        }
        ((ObjSeqState*)seq)->moveBlendParam = (*(s16*)(cmd + 2) >> 8) & 0xf0;
        if (action == NULL)
        {
            break;
        }
        animState = (u8*)((ObjAnimBank*)action)->currentState;
        if (activeObj->anim.currentMove == ((ObjSeqState*)seq)->moveId)
        {
            if ((s8)animState[0x60] != 0)
            {
                restart = 0;
            }
            else
            {
                restart = 1;
            }
        }
        else
        {
            restart = 1;
        }
        if (doUpdate == 0)
        {
            break;
        }
        if (restart == 0)
        {
            break;
        }
        if ((((ObjSeqState*)seq)->flags & 4) == 0)
        {
            break;
        }
        if (action == NULL)
        {
            break;
        }
        ((ObjAnimState*)animState)->framePhase =
            activeObj->anim.currentMoveProgress * ((ObjAnimState*)animState)->frameLength;
        if (((ObjSeqState*)seq)->trackRunLength[10] != 0)
        {
            sub = ((ObjSeqState*)seq)->curFrame - 1;
            if (((ObjSeqState*)seq)->animEntries != NULL && ((ObjSeqState*)seq)->trackRunLength[10] != 0)
            {
                objCurveInterpolate(
                    (ObjCurveKey*)(((ObjSeqState*)seq)->animEntries + ((ObjSeqState*)seq)->trackAnimStart[10] * 8),
                    ((ObjSeqState*)seq)->trackRunLength[10] & 0xfff, sub);
            }
        }
        if (activeObj->anim.classId == 1)
        {
            act2 = ObjSeq_GetActiveModel(activeObj);
            animState = *(u8**)(act2 + 0x2c);
            ((ObjAnimState*)animState)->lastBlendMoveIndex = -1;
            ((ObjAnimState*)animState)->eventState = 0;
            ((ObjAnimState*)animState)->prevEventState = 0;
            st2 = *(u8**)(act2 + 0x30);
            if (st2 != NULL)
            {
                *(s16*)(st2 + 0x64) = -1;
                *(s16*)(st2 + 0x58) = 0;
                *(s16*)(st2 + 0x5a) = 0;
                *(s16*)(st2 + 0x5c) = 0;
            }
        }
        ((ObjSeqState*)seq)->fade = 1.0f;
        ObjAnim_SetCurrentMove((int)activeObj, ((ObjSeqState*)seq)->moveId,
                               (f32)((ObjSeqState*)seq)->moveBlendParam / 256.0f, 0);
        break;
    case SEQACT_MOVEMODE:
        if (flag8 != 0)
        {
            break;
        }
        if ((s8)((ObjSeqState*)seq)->unk7B != 0 && (s8)(base + (s8)((ObjSeqState*)seq)->slot)[0x3a40] != 0)
        {
            ((ObjSeqState*)seq)->useRootMotionSpeed = 0;
            break;
        }
        ((ObjSeqState*)seq)->useRootMotionSpeed = 1 - ((ObjSeqState*)seq)->useRootMotionSpeed;
        break;
    case SEQACT_GROUND_MODE:
        *(s8*)&((ObjSeqState*)seq)->groundSnapEnabled = 1 - ((ObjSeqState*)seq)->groundSnapEnabled;
        break;
    case SEQACT_OVERRIDE:
        if (flag8 != 0)
        {
            break;
        }
        if ((f & 4) != 0)
        {
            break;
        }
        activeObj = ObjSeq_ToggleCommand3Target(obj, seq, model);
        activeObj->anim.activeMove = -1;
        break;
    case SEQACT_CONDITION:
        if (doUpdate != 0 && *(s16*)(cmd + 2) > 0 && lbl_803DD0C0 < 0x14)
        {
            *(u8**)((entry = base + lbl_803DD0C0 * 8) + 0x2b34) = cmd + 4;
            *(s16*)(entry + 0x2b3a) = ((ObjSeqState*)seq)->curFrame;
            reps = *(s16*)(cmd + 2);
            lbl_803DD0C0 = lbl_803DD0C0 + 1;
            *(s16*)(entry + 0x2b38) = reps;
        }
        ((ObjSeqState*)seq)->cmdCursor += *(s16*)(cmd + 2);
        break;
    case SEQACT_VTXANIM:
        if (flag8 != 0)
        {
            break;
        }
        if (doUpdate == 0)
        {
            break;
        }
        if (action == NULL)
        {
            break;
        }
        if (*(u8*)(*(u8**)action + 0xf9) == 0)
        {
            break;
        }
        blend = (f32)(int)((*(s16*)(cmd + 2) >> 8) & 0xff);
        if (0.0f != blend)
        {
            t = 1.0f / blend;
        }
        else
        {
            t = 1.0f;
        }
        sub = *(s16*)(cmd + 2) & 0xff;
        if (sub < 0xf)
        {
            ObjModel_SetBlendChannelTargets((ObjModel*)action, 2, *(s8*)(*(u8**)(action + 0x28) + 0x2d), sub - 1,
                                            t, 0);
        }
        else
        {
            ObjModel_SetBlendChannelTargets((ObjModel*)action, 0, *(s8*)(*(u8**)(action + 0x28) + 0xd), sub - 1,
                                            t, 0);
        }
        break;
    case SEQACT_STORYBOARD:
        if (flag8 != 0)
        {
            break;
        }
        (*gGameUIInterface)->showNpcDialogue(*(s16*)(cmd + 2), 0x14, 0x8c, 0);
        break;
    case SEQACT_ENVFX:
        if (noExec != 0)
        {
            break;
        }
        if (((*(s16*)(cmd + 2) >> 12) & 0xf) == 8)
        {
            break;
        }
        if ((s8)lbl_803DD113 < 10)
        {
            entry = base + lbl_803DD113 * 8;
            *(GameObject**)(entry + 0x3ca4) = activeObj;
            *(s8*)((int)entry + 0x3caa) = (s8)((*(s16*)(cmd + 2) >> 12) & 0xf);
            if (*(s8*)((int)entry + 0x3caa) == 0xb || *(s8*)((int)entry + 0x3caa) == 0xc)
            {
                u8* entry2;
                val = *(s16*)(cmd + 6);
                entry2 = base + (s8)(lbl_803DD113++) * 8;
                *(s16*)(entry2 + 0x3ca8) = val;
            }
            else
            {
                val = (s16)(*(s16*)(cmd + 2) & 0xfff);
                lbl_803DD113++;
                *(s16*)(entry + 0x3ca8) = val;
            }
        }
        break;
    case SEQACT_SETTIME:
        break;
    }

    if (noExec != 0)
    {
        return 0;
    }

    if ((s8)lbl_803DD112 != 0 || (s8)lbl_803DD111 != 0)
    {
        if ((s8)cmd[0] == 0xd)
        {
            switch ((*(s16*)(cmd + 2) >> 12) & 0xf)
            {
            case 2:
                getEnvfxActVoid(activeObj, activeObj, *(s16*)(cmd + 2) & 0xfff, 0);
                break;
            case 6:
                warpToMap(*(s16*)(cmd + 2) & 0xfff, 0);
                break;
            case 5:
                break;
            }
        }
        return 0;
    }

    switch ((s8)cmd[0])
    {
    case SEQACT_SFX:
        if (flag8 != 0)
        {
            break;
        }
        if (((base + (s8)((ObjSeqState*)seq)->slot)[0x3538] & 0x20) == 0)
        {
            break;
        }
        if ((s8)(base + (s8)((ObjSeqState*)seq)->slot)[0x3c4c] == 3)
        {
            break;
        }
        if (((*(s16*)(cmd + 2) >> 12) & 0xf) != 0xf)
        {
            Sfx_PlayFromObject((u32)obj, (u16)(*(s16*)(cmd + 2) & 0xfff));
        }
        else
        {
            Sfx_PlayFromObject((u32)obj, (u16)(*(s16*)(cmd + 2) & 0xfff));
            ((ObjSeqState*)seq)->sfxTimer[3] = -1;
            ((ObjSeqState*)seq)->sfxId[3] = (s16)(*(s16*)(cmd + 2) & 0xfff);
        }
        break;
    case SEQACT_ENVFX:
        switch ((*(s16*)(cmd + 2) >> 12) & 0xf)
        {
        case 0:
            if (((base + (s8)((ObjSeqState*)seq)->slot)[0x3538] & 0x20) != 0)
            {
                val = (*(s16*)(cmd + 2) & 0xfff) + 1;
                if (val == 0xd9 || val == 0x92)
                {
                    Music_Trigger(val, 1);
                }
            }
            break;
        case 2:
            getEnvfxActVoid(activeObj, activeObj, *(s16*)(cmd + 2) & 0xfff, 0);
            break;
        case 6:
            if (flag8 != 0)
            {
                break;
            }
            warpToMap(*(s16*)(cmd + 2) & 0xfff, 0);
            break;
        case 7:
            if (flag8 != 0)
            {
                break;
            }
            break;
        case 8:
            if (flag8 != 0)
            {
                break;
            }
            ((ObjSeqState*)seq)->texId5 = (u8)(*(s16*)(cmd + 2) & 0xfff);
            ((ObjSeqState*)seq)->texId4 = ((ObjSeqState*)seq)->texId5;
            break;
        case 0xe:
            if (flag8 != 0)
            {
                break;
            }
            ((ObjSeqState*)seq)->texId5 = (u8)(*(s16*)(cmd + 2) & 0xfff);
            break;
        case 0xf:
            if (flag8 != 0)
            {
                break;
            }
            ((ObjSeqState*)seq)->texId4 = (u8)(*(s16*)(cmd + 2) & 0xfff);
            break;
        }
        break;
    case SEQACT_SFX_WITH_DURATION:
        if (flag8 != 0)
        {
            break;
        }
        if (((base + (s8)((ObjSeqState*)seq)->slot)[0x3538] & 0x20) == 0)
        {
            break;
        }
        if ((s8)(base + (s8)((ObjSeqState*)seq)->slot)[0x3c4c] == 3)
        {
            break;
        }
        if (((*(s16*)(cmd + 2) >> 12) & 0xf) != 0xf)
        {
            minRot = 0x7fff;
            slot = 0;
            for (val = 0; val < 3; val++)
            {
                if (((ObjSeqState*)seq)->sfxTimer[val] < (s16)minRot)
                {
                    slot = val;
                    minRot = ((ObjSeqState*)seq)->sfxTimer[val];
                }
            }
        }
        else
        {
            slot = 3;
        }
        entry = seq + slot * 2;
        if (*(s16*)(entry + 0x30) > 0)
        {
            Sfx_RemoveLoopedObjectSound((u32)obj, (u16) * (s16*)(entry + 0x38));
        }
        cmd[1] = cmd[5];
        cmd[4] = 0x63;
        *(s16*)(entry + 0x30) = *(s16*)(cmd + 6);
        *(s16*)(seq + slot * 2 + 0x38) = (s16)(*(s16*)(cmd + 2) & 0xfff);
        Sfx_AddLoopedObjectSound((u32)obj, (u16) * (s16*)(seq + slot * 2 + 0x38));
        break;
    }
    return 0;
}




void ObjSeq_SetupInitialPlaybackState(GameObject* obj, GameObject** seqObj, u8* seq, ObjSeqPlacement* placement, void** outAction)
{
    GameObject* activeObj;
    s16* modelVec;
    f32 groundY[2];
    long long time;
    u8* historyBase;

    historyBase = gObjSeqRuntimeBuffer;
    if ((s8)((ObjSeqState*)seq)->unk7B != 0)
    {
        gObjSeqCamModeArgB = 1;
        gObjSeqCamModeArgD = 0x5a;
        gObjSeqCamMode = 0x42;
    }

    ((ObjSeqState*)seq)->curFrame = ((ObjSeqState*)seq)->pendingStartFrame;
    ((ObjSeqState*)seq)->prevFrame = -0x3c;
    ObjSeq_ApplyFrameCurves(obj, *seqObj, seq, 0);
    ObjSeq_RebuildCurveStateToFrame(obj, *seqObj, seq, 1);

    activeObj = *(GameObject**)obj->extra;
    if (activeObj == NULL)
    {
        activeObj = obj;
    }
    *outAction = ObjSeq_GetActiveModel(activeObj);
    *seqObj = activeObj;

    ObjSeq_UpdateCurvePosition(obj, seq);
    if ((s8)((ObjSeqState*)seq)->groundSnapEnabled == 1 &&
        hitDetectFn_800658a4(obj, obj->anim.localPosX, obj->anim.localPosY,
                             obj->anim.localPosZ, groundY, 0) == 0)
    {
        obj->anim.localPosY =
            obj->anim.localPosY +
            ((obj->anim.localPosY - groundY[0]) - placement->groundOffset);
    }

    obj->anim.rotX += ((ObjSeqState*)seq)->heading;
    if (*seqObj != obj && (s8)lbl_803DD0D8 == 0)
    {
        objCallSeqFn(*seqObj, obj, seq, ((u8*)(historyBase + 0x3c4c))[(s8)((ObjSeqState*)seq)->slot]);
    }

    ObjSeq_ApplyLinkedObjectTransform(obj, *seqObj, seq);
    ((ObjSeqState*)seq)->texId5 = 0;
    ((ObjSeqState*)seq)->texId4 = 0;
    ((ObjSeqState*)seq)->runState = 1;
    ((ObjSeqState*)seq)->prevFrame = ((ObjSeqState*)seq)->curFrame;
    if ((s8)gObjSeqStop != 0)
    {
        animatedObjFreeAndSavePlayerPos(obj, *seqObj, seq);
    }

    ((f32*)(historyBase + 0x3740))[(s8)((ObjSeqState*)seq)->slot] = (f32)((ObjSeqState*)seq)->curFrame;
    ((s16*)(historyBase + 0x2be0))[(s8)((ObjSeqState*)seq)->slot] = ((ObjSeqState*)seq)->curFrame;
    time = OSGetTime();
    ((long long*)(historyBase + 0x2f38))[(s8)((ObjSeqState*)seq)->slot] = time;
    time = OSGetTime();
    ((long long*)(historyBase + 0x2c90))[(s8)((ObjSeqState*)seq)->slot] = time;

    if (*seqObj != NULL)
    {
        objModelClearVecFn_8003aa40(*seqObj);
        if ((*seqObj)->anim.classId == 1)
        {
            modelVec = objModelGetVecFn_800395d8(obj, 1);
            if (modelVec != NULL)
            {
                modelVec[0] = 0;
                modelVec[1] = 0;
                modelVec[2] = 0;
            }
        }
    }
}

void* ObjSeq_ToggleCommand3Target(GameObject* obj, u8* seq, ObjSeqPlacement* placement)
{
    void* result;
    GameObject* activeObj;
    u8* entry;
    int j;
    u8* slotBase;
    int slotOff;
    GameObject* seqObj;
    f32 groundY[2];

    result = obj;
    *(s8*)&((ObjSeqState*)seq)->unk79 = (s8)(((ObjSeqState*)seq)->unk79 ^ 1);
    if ((s8)((ObjSeqState*)seq)->unk79 != 0)
    {
        ObjSeq_resolveTargetObject(obj);
        seqObj = *(GameObject**)seq;
        if (seqObj != NULL)
        {
            result = seqObj;
            seqObj->pendingParentObj = obj;
            seqObj->objectFlags |= OBJECT_OBJFLAG_SEQ_ATTACHED;
            ((ObjSeqState*)seq)->callbackContext = seqObj;

            activeObj = *(GameObject**)seq;
            j = 0;
            slotOff = (s8)((ObjSeqState*)seq)->slot * 0x80;
            slotBase = gObjSeqRuntimeBuffer + slotOff;
            entry = slotBase;
            for (; j < 16; j++)
            {
                if (*(GameObject**)entry == NULL || *(GameObject**)entry == activeObj)
                {
                    break;
                }
                entry += 8;
            }
            *(GameObject**)(slotBase + j * 8) = activeObj;
            *(GameObject**)((u8*)(int)gObjSeqRuntimeBuffer + slotOff + j * 8 + 4) = obj;
        }
    }
    else
    {
        if (((ObjSeqState*)seq)->targetObj != NULL)
        {
            if ((((ObjSeqState*)seq)->flags & 1) != 0)
            {
                obj->anim.localPosX = obj->anim.localPosX;
                obj->anim.localPosY = obj->anim.localPosY;
                obj->anim.localPosZ = obj->anim.localPosZ;
                ObjSeq_UpdateCurvePosition(obj, seq);
            }
            if ((s8)((ObjSeqState*)seq)->groundSnapEnabled == 1 &&
                hitDetectFn_800658a4(obj, obj->anim.localPosX, obj->anim.localPosY,
                                     obj->anim.localPosZ, groundY, 0) == 0)
            {
                obj->anim.localPosY =
                    obj->anim.localPosY +
                    ((obj->anim.localPosY - groundY[0]) - placement->groundOffset);
            }
            if ((((ObjSeqState*)seq)->flags & 2) != 0)
            {
                obj->anim.rotX += ((ObjSeqState*)seq)->heading;
            }
            obj->pendingParentObj = NULL;
            obj->objectFlags &= ~OBJECT_OBJFLAG_SEQ_ATTACHED;
            ((ObjSeqState*)seq)->targetObj = NULL;
            result = obj;
        }
    }
    return result;
}

void ObjSeq_RefreshActionCursor(void* obj, void* seqFile, u8* seq)
{
    int actionIndex;
    u8* command;
    u8 opcode;
    int stop;

    if (((ObjSeqState*)seq)->cmds == NULL)
    {
        return;
    }

    ((ObjSeqState*)seq)->retriggerFrame = -1;
    ((ObjSeqState*)seq)->cmdCursor = 0;
    ((ObjSeqState*)seq)->fade = 0.0f;
    stop = 0;
    while (stop == 0 && ((ObjSeqState*)seq)->cmdCursor < ((ObjSeqState*)seq)->cmdCount)
    {
        actionIndex = ((ObjSeqState*)seq)->cmdCursor;
        command = ((ObjSeqState*)seq)->cmds + actionIndex * 4;
        opcode = command[0];
        if ((s8)opcode == 0)
        {
            if (((ObjSeqState*)seq)->curFrame >= *(s16*)(command + 2))
            {
                ((ObjSeqState*)seq)->retriggerFrame = *(s16*)(command + 2);
                ((ObjSeqState*)seq)->cmdCursor++;
            }
            else
            {
                stop = 1;
            }
        }
        else if ((s8)opcode == 0xb && *(s16*)(command + 2) > 0)
        {
            if (((ObjSeqState*)seq)->curFrame >= ((ObjSeqState*)seq)->retriggerFrame)
            {
                ((ObjSeqState*)seq)->retriggerFrame += command[1];
                ((ObjSeqState*)seq)->cmdCursor = (s16)(((ObjSeqState*)seq)->cmdCursor + (*(s16*)(command + 2) + 1));
            }
            else
            {
                stop = 1;
            }
        }
        else if (((ObjSeqState*)seq)->curFrame >= ((ObjSeqState*)seq)->retriggerFrame)
        {
            if ((s8)command[0] != 0xf)
            {
                ((ObjSeqState*)seq)->retriggerFrame += command[1];
            }
            ((ObjSeqState*)seq)->cmdCursor++;
        }
        else
        {
            stop = 1;
        }
    }
}
void ObjSeq_RebuildCurveStateToFrame(GameObject* obj, GameObject* seqObj, u8* seq, int mode)
{
    struct
    {
        f32 x;
        f32 y;
        f32 z;
    } pos;
    f32* posp;
    int out[3];
    u8* cmd;
    f32 speed;
    ObjSeqPlacement* model;
    u8* action;
    int found;
    int i;
    int targetFrame;
    s8 flags;
    int stop;
    int frame;
    f32 val;
    f32 rate;
    f32 prevX;
    f32 prevZ;
    int opcode;
    u8* entry;

    ObjSeqState* state = (ObjSeqState*)seq;

    if (state->cmds == NULL)
    {
        return;
    }

    flags = 1;
    if (mode != 0)
    {
        flags |= 2;
    }

    model = (ObjSeqPlacement*)obj->anim.placementData;
    targetFrame = state->curFrame;
    lbl_803DD08A = targetFrame;
    state->cmdCursor = 0;
    state->retriggerFrame = -0x32;
    state->useRootMotionSpeed = 0;
    state->groundSnapEnabled = 0;
    state->unk79 = 0;
    state->targetObj = NULL;
    state->unk7B = 0;
    state->fade = 0.0f;
    state->curFrame = -1;

    found = -1;
    seqObj = obj;
    i = 0;
    while (i < state->cmdCount && state->curFrame <= targetFrame)
    {
        cmd = state->cmds + i * 4;
        opcode = cmd[0];
        switch ((s8)opcode)
        {
        case 3:
            flags = (s8)(flags | 4);
            seqObj = ObjSeq_ToggleCommand3Target(obj, seq, model);
            seqObj->anim.activeMove = -1;
            break;
        case 0:
            state->curFrame = *(s16*)(cmd + 2);
            break;
        case 9:
            found = state->curFrame;
            break;
        case 11:
            if (*(s16*)(cmd + 2) > 0)
            {
                i += *(s16*)(cmd + 2);
            }
            break;
        default:
            if ((s8)opcode != 0xf)
            {
                state->curFrame += cmd[1];
            }
            break;
        }
        i++;
    }

    state->curFrame = found;
    action = (u8*)seqObj->anim.banks[seqObj->anim.bankIndex];
    if (action != NULL)
    {
        val = ObjSeq_SampleTrackCurve(seq, 13, -1);
        prevX = model->baseX + val;
        val = ObjSeq_SampleTrackCurve(seq, 11, -1);
        prevZ = model->baseZ + val;
    }

    posp = &pos.x;
    entry = lbl_8039944C;
    while (state->curFrame < targetFrame)
    {
        state->curFrame += 1;
        frame = state->curFrame;
        val = ObjSeq_SampleTrackCurve(seq, 13, frame);
        pos.x = model->baseX + val;
        frame = state->curFrame;
        val = ObjSeq_SampleTrackCurve(seq, 12, frame);
        pos.y = model->groundOffset + val;
        frame = state->curFrame;
        val = ObjSeq_SampleTrackCurve(seq, 11, frame);
        pos.z = model->baseZ + val;

        if (state->curFrame > 0 && mode != 0)
        {
            if ((s8)state->useRootMotionSpeed == 1 && (s8)state->unk7B == 0 &&
                action != NULL)
            {
                f32 dx = posp[0] - prevX;
                if (ObjAnim_SampleRootCurvePhase(&seqObj->anim,
                                                 sqrtf(dx * dx + (posp[2] - prevZ) * (posp[2] - prevZ)),
                                                 &speed) == 0)
                {
                    frame = state->curFrame - 1;
                    val = ObjSeq_SampleTrackCurve(seq, 9, frame);
                    speed = 0.0004f * val;
                }
            }
            else
            {
                frame = state->curFrame - 1;
                val = ObjSeq_SampleTrackCurve(seq, 9, frame);
                speed = 0.0004f * val;
            }

            if (action != NULL)
            {
                ObjAnim_AdvanceCurrentMove((int)seqObj, speed, 1.0f,
                                                                            &state->animEvents);
                if (mode != 0)
                {
                    if (state->fade > 0.0f)
                    {
                        if (state->trackRunLength[10] != 0)
                        {
                            frame = state->curFrame - 1;
                            rate = ObjSeq_SampleTrackCurve(seq, 10, frame);
                        }
                        else
                        {
                            rate = 8.0f;
                        }
                        if (rate < 1.0f)
                        {
                            rate = 1.0f;
                        }
                        rate = 1.0f / rate;
                        state->fade -= rate;
                        if (state->fade < 0.0f)
                        {
                            state->fade = 0.0f;
                        }
                    }
                }
            }
            else
            {
                seqObj->anim.currentMoveProgress += speed;
                val = 1.0f;
                while (seqObj->anim.currentMoveProgress > val)
                {
                    seqObj->anim.currentMoveProgress -= val;
                }
                rate = 1.0f;
                while (seqObj->anim.currentMoveProgress < 0.0f)
                {
                    seqObj->anim.currentMoveProgress += rate;
                }
            }
        }

        prevX = posp[0];
        prevZ = posp[2];

        stop = 0;
        lbl_803DD0C0 = 0;
        while (stop == 0 && state->cmdCursor < state->cmdCount)
        {
            cmd = state->cmds + state->cmdCursor * 4;
            opcode = (s8)cmd[0];
            if (opcode == 0)
            {
                if (state->curFrame >= *(s16*)(cmd + 2))
                {
                    state->retriggerFrame = *(s16*)(cmd + 2);
                    state->cmdCursor += 1;
                }
                else
                {
                    stop = 1;
                }
            }
            else
            {
                if (state->curFrame >= state->retriggerFrame)
                {
                    if (opcode != 0xf)
                    {
                        state->retriggerFrame += cmd[1];
                    }
                    state->cmdCursor += 1;
                    if (ObjSeq_ExecuteActionCommand(obj, action, &cmd, flags, out) != 0)
                    {
                        return;
                    }
                    {
                        GameObject* t = *(GameObject**)obj->extra;
                        if (t == NULL)
                        {
                            t = obj;
                        }
                        action = ObjSeq_GetActiveModel(t);
                        seqObj = t;
                    }
                }
                else
                {
                    stop = 1;
                }
            }
        }

        for (i = 0; i < lbl_803DD0C0; i++)
        {
            if (seqDoSubCmd0B(obj, seqObj, seq, *(u8**)(entry + i * 8), *(s16*)(entry + i * 8 + 6),
                              *(s16*)(entry + i * 8 + 4), 1, 0) != 0)
            {
                i = lbl_803DD0C0;
            }
            {
                GameObject* t = *(GameObject**)obj->extra;
                if (t == NULL)
                {
                    t = obj;
                }
                action = ObjSeq_GetActiveModel(t);
                seqObj = t;
            }
        }
        lbl_803DD0C0 = 0;
    }
}

void ObjSeq_ApplyFrameCurves(GameObject* obj, GameObject* seqObj, u8* seq, int frame)
{
    ObjSeqPlacement* model;
    s16* vec;
    s16* vec2;
    ObjTextureRuntimeSlot* tex1;
    ObjTextureRuntimeSlot* tex2;
    ObjTextureRuntimeSlot* tex5;
    int slots;
    int k;
    int* modelIds;
    int i;
    int vol;
    s16 scroll;
    f32 val;

    model = (ObjSeqPlacement*)obj->anim.placementData;
    obj->anim.localPosX = model->baseX;
    obj->anim.localPosY = model->groundOffset;
    obj->anim.localPosZ = model->baseZ;
    obj->anim.rotY = 0;
    obj->anim.rotX = 0;
    obj->anim.rotZ = 0;
    if ((((ObjSeqState*)seq)->flags & 0x20) != 0)
    {
        seqObj->anim.alpha = 0xff;
    }
    gObjSeqCurvePosOffsetX = 0.0f;
    gObjSeqCurvePosOffsetY = 0.0f;
    gObjSeqCurvePosOffsetZ = 0.0f;

    if (((ObjSeqState*)seq)->animEntries != NULL)
    {
        val = ObjSeq_SampleTrackCurve(seq, 18, frame);
        vol = val;

        for (i = 0; i < 3; i++)
        {
            if (*(s16*)(seq + i * 2 + 0x30) != 0)
            {
                Sfx_IsPlayingFromObject((u32)seqObj, (u16) * (s16*)(seq + i * 2 + 0x38));
            }
        }

        if (vol > 0 && ((ObjSeqState*)seq)->sfxTimer[3] != 0)
        {
            if (Sfx_IsPlayingFromObject((u32)seqObj, (u16)((ObjSeqState*)seq)->sfxId[3]) != 0)
            {
                Sfx_SetObjectSfxVolume((u32)seqObj, (u16)((ObjSeqState*)seq)->sfxId[3], vol, 0.5f);
            }
        }

        if (((ObjSeqState*)seq)->animEntries == NULL)
        {
            val = 0.0f;
        }
        else
        {
            val = 0.0f;
            if (((ObjSeqState*)seq)->trackRunLength[7] != 0)
            {
                val = objCurveInterpolate(
                    (ObjCurveKey*)(((ObjSeqState*)seq)->animEntries + ((ObjSeqState*)seq)->trackAnimStart[7] * 8),
                    ((ObjSeqState*)seq)->trackRunLength[7] & 0xfff, frame);
            }
        }
        obj->anim.rotX = 182.044f * val;

        if (((ObjSeqState*)seq)->animEntries == NULL)
        {
            val = 0.0f;
        }
        else
        {
            val = 0.0f;
            if (((ObjSeqState*)seq)->trackRunLength[8] != 0)
            {
                val = objCurveInterpolate(
                    (ObjCurveKey*)(((ObjSeqState*)seq)->animEntries + ((ObjSeqState*)seq)->trackAnimStart[8] * 8),
                    ((ObjSeqState*)seq)->trackRunLength[8] & 0xfff, frame);
            }
        }
        obj->anim.rotY = 182.044f * val;

        if (((ObjSeqState*)seq)->animEntries == NULL)
        {
            val = 0.0f;
        }
        else
        {
            val = 0.0f;
            if (((ObjSeqState*)seq)->trackRunLength[6] != 0)
            {
                val = objCurveInterpolate(
                    (ObjCurveKey*)(((ObjSeqState*)seq)->animEntries + ((ObjSeqState*)seq)->trackAnimStart[6] * 8),
                    ((ObjSeqState*)seq)->trackRunLength[6] & 0xfff, frame);
            }
        }
        obj->anim.rotZ = 182.044f * val;

        if (((ObjSeqState*)seq)->animEntries == NULL)
        {
            val = 0.0f;
        }
        else
        {
            val = 0.0f;
            if (((ObjSeqState*)seq)->trackRunLength[13] != 0)
            {
                val = objCurveInterpolate(
                    (ObjCurveKey*)(((ObjSeqState*)seq)->animEntries + ((ObjSeqState*)seq)->trackAnimStart[13] * 8),
                    ((ObjSeqState*)seq)->trackRunLength[13] & 0xfff, frame);
            }
        }
        gObjSeqCurvePosOffsetX = val;

        if (((ObjSeqState*)seq)->animEntries == NULL)
        {
            val = 0.0f;
        }
        else
        {
            val = 0.0f;
            if (((ObjSeqState*)seq)->trackRunLength[12] != 0)
            {
                val = objCurveInterpolate(
                    (ObjCurveKey*)(((ObjSeqState*)seq)->animEntries + ((ObjSeqState*)seq)->trackAnimStart[12] * 8),
                    ((ObjSeqState*)seq)->trackRunLength[12] & 0xfff, frame);
            }
        }
        gObjSeqCurvePosOffsetY = val;

        if (((ObjSeqState*)seq)->animEntries == NULL)
        {
            val = 0.0f;
        }
        else
        {
            val = 0.0f;
            if (((ObjSeqState*)seq)->trackRunLength[11] != 0)
            {
                val = objCurveInterpolate(
                    (ObjCurveKey*)(((ObjSeqState*)seq)->animEntries + ((ObjSeqState*)seq)->trackAnimStart[11] * 8),
                    ((ObjSeqState*)seq)->trackRunLength[11] & 0xfff, frame);
            }
        }
        gObjSeqCurvePosOffsetZ = val;

        gObjSeqLinkedSavedPosX = gObjSeqCurvePosOffsetX;
        gObjSeqLinkedSavedPosY = gObjSeqCurvePosOffsetY;
        gObjSeqLinkedSavedPosZ = gObjSeqCurvePosOffsetZ;
        gObjSeqLinkedSavedPitch = obj->anim.rotX;
        gObjSeqLinkedTransformValid = 1;
        obj->anim.localPosX = model->baseX + gObjSeqCurvePosOffsetX;
        obj->anim.localPosY = model->groundOffset + gObjSeqCurvePosOffsetY;
        obj->anim.localPosZ = model->baseZ + gObjSeqCurvePosOffsetZ;

        if (((ObjSeqState*)seq)->trackRunLength[14] != 0)
        {
            if (((ObjSeqState*)seq)->animEntries == NULL)
            {
                val = 0.0f;
            }
            else
            {
                val = 0.0f;
                if (((ObjSeqState*)seq)->trackRunLength[14] != 0)
                {
                    val = objCurveInterpolate(
                        (ObjCurveKey*)(((ObjSeqState*)seq)->animEntries + ((ObjSeqState*)seq)->trackAnimStart[14] * 8),
                        ((ObjSeqState*)seq)->trackRunLength[14] & 0xfff, frame);
                }
            }
            if ((s8)((ObjSeqState*)seq)->unk7B != 0)
            {
                if (val < 35.0f)
                {
                    val = 35.0f;
                }
                if (val > 120.0f)
                {
                    val = 125.0f;
                }
                gObjSeqFovOverrideActive = 1;
                gObjSeqFovOverrideValue = val;
            }
            else
            {
                ((ObjSeqState*)seq)->unk10 = val;
            }
        }

        if ((((ObjSeqState*)seq)->flags & 0x20) != 0 && ((ObjSeqState*)seq)->trackRunLength[3] != 0)
        {
            if (((ObjSeqState*)seq)->animEntries == NULL)
            {
                val = 0.0f;
            }
            else
            {
                val = 0.0f;
                if (((ObjSeqState*)seq)->trackRunLength[3] != 0)
                {
                    val = objCurveInterpolate(
                        (ObjCurveKey*)(((ObjSeqState*)seq)->animEntries + ((ObjSeqState*)seq)->trackAnimStart[3] * 8),
                        ((ObjSeqState*)seq)->trackRunLength[3] & 0xfff, frame);
                }
            }
            if (val < 0.0f)
            {
                val = 0.0f;
            }
            if (val > 255.0f)
            {
                val = 255.0f;
            }
            seqObj->anim.alpha = val;
        }

        if (((ObjSeqState*)seq)->trackRunLength[4] != 0)
        {
            if (((ObjSeqState*)seq)->animEntries == NULL)
            {
                val = 0.0f;
            }
            else
            {
                val = 0.0f;
                if (((ObjSeqState*)seq)->trackRunLength[4] != 0)
                {
                    val = objCurveInterpolate(
                        (ObjCurveKey*)(((ObjSeqState*)seq)->animEntries + ((ObjSeqState*)seq)->trackAnimStart[4] * 8),
                        ((ObjSeqState*)seq)->trackRunLength[4] & 0xfff, frame);
                }
            }
            (*gSkyInterface)->setTimeOfDay(60.0f * val);
        }

        if ((((ObjSeqState*)seq)->flags & 0x10) != 0 && ((ObjSeqState*)seq)->trackRunLength[5] != 0)
        {
            if (((ObjSeqState*)seq)->animEntries == NULL)
            {
                val = 0.0f;
            }
            else
            {
                val = 0.0f;
                if (((ObjSeqState*)seq)->trackRunLength[5] != 0)
                {
                    val = objCurveInterpolate(
                        (ObjCurveKey*)(((ObjSeqState*)seq)->animEntries + ((ObjSeqState*)seq)->trackAnimStart[5] * 8),
                        ((ObjSeqState*)seq)->trackRunLength[5] & 0xfff, frame);
                }
            }
            seqObj->anim.rootMotionScale = val * seqObj->anim.modelInstance->rootMotionScaleBase;
        }

        if ((((ObjSeqState*)seq)->flags & 8) != 0)
        {
            vec = objModelGetVecFn_800395d8(seqObj, 0);
            if (vec != NULL)
            {
                if (((ObjSeqState*)seq)->trackRunLength[1] != 0)
                {
                    if (((ObjSeqState*)seq)->animEntries == NULL)
                    {
                        val = 0.0f;
                    }
                    else
                    {
                        val = 0.0f;
                        if (((ObjSeqState*)seq)->trackRunLength[1] != 0)
                        {
                            val = objCurveInterpolate((ObjCurveKey*)(((ObjSeqState*)seq)->animEntries +
                                                                     ((ObjSeqState*)seq)->trackAnimStart[1] * 8),
                                                      ((ObjSeqState*)seq)->trackRunLength[1] & 0xfff, frame);
                        }
                    }
                }
                else
                {
                    val = 0.0f;
                }
                vec[0] = (s16)(((ObjSeqState*)seq)->baseRotX + (int)(182.044f * val));

                if (((ObjSeqState*)seq)->trackRunLength[2] != 0)
                {
                    if (((ObjSeqState*)seq)->animEntries == NULL)
                    {
                        val = 0.0f;
                    }
                    else
                    {
                        val = 0.0f;
                        if (((ObjSeqState*)seq)->trackRunLength[2] != 0)
                        {
                            val = objCurveInterpolate((ObjCurveKey*)(((ObjSeqState*)seq)->animEntries +
                                                                     ((ObjSeqState*)seq)->trackAnimStart[2] * 8),
                                                      ((ObjSeqState*)seq)->trackRunLength[2] & 0xfff, frame);
                        }
                    }
                }
                else
                {
                    val = 0.0f;
                }
                vec[1] = (s16)(((ObjSeqState*)seq)->baseRotY + (int)(182.044f * val));

                if (((ObjSeqState*)seq)->trackRunLength[0] != 0)
                {
                    if (((ObjSeqState*)seq)->animEntries == NULL)
                    {
                        val = 0.0f;
                    }
                    else
                    {
                        val = 0.0f;
                        if (((ObjSeqState*)seq)->trackRunLength[0] != 0)
                        {
                            val = objCurveInterpolate((ObjCurveKey*)(((ObjSeqState*)seq)->animEntries +
                                                                     ((ObjSeqState*)seq)->trackAnimStart[0] * 8),
                                                      ((ObjSeqState*)seq)->trackRunLength[0] & 0xfff, frame);
                        }
                    }
                }
                else
                {
                    val = 0.0f;
                }
                vec[2] = 182.044f * val;

                if ((((ObjSeqState*)seq)->flags & 0x400) != 0)
                {
                    slots = ((SeqByte136*)&((ObjSeqState*)seq)->flags136[0])->modelSlot;
                    modelIds = objGetLookAtJointKeys();
                    if (slots == 0)
                    {
                        slots = 9;
                    }
                    if (vec != NULL)
                    {
                        for (k = 1, modelIds++; k < slots; modelIds++, k++)
                        {
                            vec2 = objModelGetVecFn_800395d8(seqObj, *modelIds);
                            if (vec2 != NULL)
                            {
                                vec2[1] = vec[1];
                                vec2[0] = vec[0];
                                vec2[2] = vec[2];
                            }
                        }
                    }
                }
            }
        }

        if ((((ObjSeqState*)seq)->flags & 0x200) != 0)
        {
            vec = objModelGetVecFn_800395d8(seqObj, 1);
            if (vec != NULL)
            {
                if (((ObjSeqState*)seq)->trackRunLength[17] != 0)
                {
                    if (((ObjSeqState*)seq)->animEntries == NULL)
                    {
                        val = 0.0f;
                    }
                    else
                    {
                        val = 0.0f;
                        if (((ObjSeqState*)seq)->trackRunLength[17] != 0)
                        {
                            val = objCurveInterpolate((ObjCurveKey*)(((ObjSeqState*)seq)->animEntries +
                                                                     ((ObjSeqState*)seq)->trackAnimStart[17] * 8),
                                                      ((ObjSeqState*)seq)->trackRunLength[17] & 0xfff, frame);
                        }
                    }
                }
                else
                {
                    val = 0.0f;
                }
                vec[0] = 182.044f * val;
            }
        }

        if ((((ObjSeqState*)seq)->flags & 0x40) != 0)
        {
            tex1 = objFindTexture(seqObj, 1, 0);
            tex2 = objFindTexture(seqObj, 0, 0);
            if (tex1 != NULL || tex2 != NULL)
            {
                if (((ObjSeqState*)seq)->trackRunLength[15] != 0)
                {
                    if (((ObjSeqState*)seq)->animEntries == NULL)
                    {
                        val = 0.0f;
                    }
                    else
                    {
                        val = 0.0f;
                        if (((ObjSeqState*)seq)->trackRunLength[15] != 0)
                        {
                            val = objCurveInterpolate((ObjCurveKey*)(((ObjSeqState*)seq)->animEntries +
                                                                     ((ObjSeqState*)seq)->trackAnimStart[15] * 8),
                                                      ((ObjSeqState*)seq)->trackRunLength[15] & 0xfff, frame);
                        }
                    }
                }
                else
                {
                    val = 0.0f;
                }
                scroll = 10.0f * val;
                if (tex1 != NULL)
                {
                    tex1->offsetS = scroll;
                }
                if (tex2 != NULL)
                {
                    tex2->offsetS = (s16)-scroll;
                }

                if (((ObjSeqState*)seq)->trackRunLength[16] != 0)
                {
                    if (((ObjSeqState*)seq)->animEntries == NULL)
                    {
                        val = 0.0f;
                    }
                    else
                    {
                        val = 0.0f;
                        if (((ObjSeqState*)seq)->trackRunLength[16] != 0)
                        {
                            val = objCurveInterpolate((ObjCurveKey*)(((ObjSeqState*)seq)->animEntries +
                                                                     ((ObjSeqState*)seq)->trackAnimStart[16] * 8),
                                                      ((ObjSeqState*)seq)->trackRunLength[16] & 0xfff, frame);
                        }
                    }
                }
                else
                {
                    val = 0.0f;
                }
                scroll = (s16) - (int)(10.0f * val);
                if (tex1 != NULL)
                {
                    tex1->offsetT = scroll;
                }
                if (tex2 != NULL)
                {
                    tex2->offsetT = scroll;
                }
            }

            tex5 = objFindTexture(seqObj, 5, 0);
            tex2 = objFindTexture(seqObj, 4, 0);
            if (tex5 != NULL)
            {
                tex5->textureId = (s16)((ObjSeqState*)seq)->texId5 << 8;
            }
            if (tex2 != NULL)
            {
                tex2->textureId = (s16)((ObjSeqState*)seq)->texId4 << 8;
            }
        }
    }
    else
    {
        gObjSeqLinkedSavedPosX = 0.0f;
        gObjSeqLinkedSavedPosY = 0.0f;
        gObjSeqLinkedSavedPosZ = 0.0f;
        gObjSeqLinkedSavedPitch = 0;
        gObjSeqLinkedTransformValid = 1;
    }
}

void ObjSeq_ApplyLinkedObjectTransform(GameObject* obj, GameObject* seqObj, u8* seq)
{
    s16 basePitch;
    int baseYaw;
    int baseRoll;
    f32 baseX;
    f32 baseY;
    f32 baseZ;

    if (seqObj->anim.parent == obj->anim.parent || (s8)gObjSeqLinkedTransformValid == 0)
    {
        baseX = obj->anim.localPosX;
        baseY = obj->anim.localPosY;
        baseZ = obj->anim.localPosZ;
        basePitch = obj->anim.rotX;
    }
    else
    {
        baseX = gObjSeqLinkedSavedPosX;
        baseY = gObjSeqLinkedSavedPosY;
        baseZ = gObjSeqLinkedSavedPosZ;
        basePitch = gObjSeqLinkedSavedPitch;
    }

    baseYaw = obj->anim.rotY;
    baseRoll = obj->anim.rotZ;
    if (seqObj != obj)
    {
        if ((((ObjSeqState*)seq)->flags & 1) != 0)
        {
            if ((s8)((ObjSeqState*)seq)->movementState == 2)
            {
                seqObj->anim.localPosX =
                    ((ObjSeqState*)seq)->posOffsetX * ((ObjSeqState*)seq)->posOffsetScale + baseX;
                seqObj->anim.localPosY =
                    ((ObjSeqState*)seq)->posOffsetY * ((ObjSeqState*)seq)->posOffsetScale + baseY;
                seqObj->anim.localPosZ =
                    ((ObjSeqState*)seq)->posOffsetZ * ((ObjSeqState*)seq)->posOffsetScale + baseZ;
            }
            else
            {
                seqObj->anim.localPosX = baseX;
                seqObj->anim.localPosY = baseY;
                seqObj->anim.localPosZ = baseZ;
            }
        }
        if ((((ObjSeqState*)seq)->flags & 2) != 0)
        {
            if ((s8)((ObjSeqState*)seq)->movementState == 2)
            {
                seqObj->anim.rotX = (s16)((s32)basePitch + (s32)((f32)((ObjSeqState*)seq)->rotOffsetX *
                                                                   ((ObjSeqState*)seq)->posOffsetScale));
                seqObj->anim.rotY =
                    (s16)(baseYaw + (s32)((f32)((ObjSeqState*)seq)->rotOffsetY * ((ObjSeqState*)seq)->posOffsetScale));
                seqObj->anim.rotZ =
                    (s16)(baseRoll + (s32)((f32)((ObjSeqState*)seq)->rotOffsetZ * ((ObjSeqState*)seq)->posOffsetScale));
            }
            else
            {
                seqObj->anim.rotX = basePitch;
                seqObj->anim.rotY = baseYaw;
                seqObj->anim.rotZ = baseRoll;
            }
        }
    }

    if ((s8)((ObjSeqState*)seq)->unk7B != 0 && (s8)((ObjSeqState*)seq)->useRootMotionSpeed != 0)
    {
        lbl_803DD0B8 = obj;
        lbl_803DD0B6 = framesThisStep;
    }
    Obj_GetWorldPosition(seqObj, &seqObj->anim.worldPosX, &seqObj->anim.worldPosY,
                         &seqObj->anim.worldPosZ);
}
static inline int ObjSeq_CheckConditionOpcode(ObjSeqState* state, GameObject* obj, u8 conditionOpcode)
{
    ObjAnimSequenceConditionCallback cb;

    switch (conditionOpcode)
    {
    case 0x12:
        if (getButtonsJustPressed(0) & PAD_BUTTON_A)
        {
            return 1;
        }
        break;
    case 0x13:
        if (getButtonsJustPressed(0) & PAD_BUTTON_B)
        {
            return 1;
        }
        break;
    case 0x14:
    case 0x15:
    case 0x16:
    case 0x17:
    case 0x18:
    case 0x19:
        cb = state->conditionCallback;
        if (cb != NULL)
        {
            return cb(state->callbackContext, (u8*)obj);
        }
        break;
    case 0x1a:
        return isTalkingToNpc() == 0;
    }
    return 0;
}

int ObjSeq_update(GameObject* obj, f32 t)
{
    u8* base = gObjSeqRuntimeBuffer;
    GameObject* activeObj;
    u8* action;
    u8* cmd;
    f32 moveProgress;
    f32 groundY;
    ObjSeqPlacement* placement;
    u8* seq;
    ObjSeqState* state;
    u8* p;
    ObjSeqBgCmd* entry;
    int runs;
    int step;
    int slot;
    int i;
    int k;
    int targetFrame;
    int stop;
    int opcode;
    int found;
    int pressed;
    int restart;
    s8 rewindStep;
    u8 conditionOpcode;
    int aInt;
    f32 val;
    f32 rate;
    f32 px;
    f32 pz;
    f32 fval;
    f32 prevX;
    f32 prevZ;
    ObjAnimSequenceConditionCallback cb;

    (void)t;

    runs = 0;
    step = lbl_803DB411;
    placement = (ObjSeqPlacement*)obj->anim.placementData;
    if (placement == NULL)
    {
        return 1;
    }

    seq = obj->extra;
    state = (ObjSeqState*)seq;
    if ((state->stateFlags & 2) != 0)
    {
        setJoypadDisabled();
    }
    activeObj = state->targetObj;
    gObjSeqStop = 0;
    gObjSeqLinkedTransformValid = 0;
    lbl_803DD112 = 0;
    lbl_803DD111 = 0;

    if (state->runState == 3)
    {
        if (state->targetObj != NULL)
        {
            activeObj->pendingParentObj = obj;
            activeObj->objectFlags |= OBJECT_OBJFLAG_SEQ_ATTACHED;
        }
        return 0;
    }

    slot = state->slot;
    if (base[slot + 0x338c] == 1)
    {
        state->curFrame = ((s16*)(base + 0x3694))[slot];
        state->prevFrame = state->curFrame;
        ObjSeq_RefreshActionCursor(obj, activeObj, seq);
    }
    else
    {
        state->curFrame = ((f32*)(base + 0x3894))[slot];
    }

    i = 3;
    p = seq + 6;
    while (p -= 2, i-- != 0)
    {
        if (*(s16*)(p + 0x30) > 0)
        {
            *(s16*)(p + 0x30) -= framesThisStep;
            if (*(s16*)(p + 0x30) <= 0)
            {
                *(s16*)(p + 0x30) = 0;
                Sfx_RemoveLoopedObjectSound((u32)obj, *(s16*)(p + 0x38));
            }
        }
    }
    ((u8*)(base + 0x3cf4))[state->slot] = 0;

    do
    {
        lbl_803DD113 = 0;
        if (state->runState == 0)
        {
            obj->anim.alpha = 0;
            return 1;
        }

        activeObj = obj;
        if (state->targetObj != NULL)
        {
            activeObj = state->targetObj;
            activeObj->pendingParentObj = obj;
            activeObj->objectFlags |= OBJECT_OBJFLAG_SEQ_ATTACHED;
        }
        else if (state->unk7B == 0 && state->movementState < 4)
        {
            state->movementState = -1;
        }

        slot = state->slot;
        if ((s8)base[slot + 0x3c4c] != 0 && (rewindStep = (s8)base[slot + 0x39e8]) != 0)
        {
            state->curFrame -= rewindStep;
            if (state->curFrame < 0)
            {
                state->curFrame = 0;
            }
            state->prevFrame = (s16)(state->curFrame - 1);
            ObjSeq_RebuildCurveStateToFrame(obj, activeObj, seq, 1);
        }

        lbl_803DD0D8 = 0;
        if (activeObj != obj)
        {
            objCallSeqFn(activeObj, obj, seq, ((u8*)(base + 0x3c4c))[state->slot]);
            lbl_803DD0D8 = 1;
        }

        if ((state->sequenceControlFlags & OBJSEQ_CONTROL_SET_LATCH_B) != 0)
        {
            ((u8*)(base + 0x3b9c))[state->slot] = 1;
        }
        if ((state->sequenceControlFlags & OBJSEQ_CONTROL_CLEAR_LATCH_B) != 0)
        {
            ((u8*)(base + 0x3b9c))[state->slot] = 0;
        }
        if ((state->sequenceControlFlags & OBJSEQ_CONTROL_SET_LATCH_A) != 0)
        {
            ((u8*)(base + 0x3b44))[state->slot] = 1;
        }
        if ((state->sequenceControlFlags & OBJSEQ_CONTROL_CLEAR_LATCH_A) != 0)
        {
            ((u8*)(base + 0x3b44))[state->slot] = 0;
        }
        if ((state->sequenceControlFlags & OBJSEQ_CONTROL_SET_STATE_LATCH) != 0)
        {
            ((u8*)(base + 0x3a40))[state->slot] = 1;
        }
        if ((state->sequenceControlFlags & OBJSEQ_CONTROL_CLEAR_STATE_LATCH) != 0)
        {
            ((u8*)(base + 0x3a40))[state->slot] = 0;
        }

        if (state->runState == 2)
        {
            ObjSeq_SetupInitialPlaybackState(obj, &activeObj, seq, placement, (void**)&action);
            return 0;
        }

        if ((s8)((u8*)(base + 0x3c4c))[state->slot] == 1)
        {
            step = 0;
        }
        else if ((s8)((u8*)(base + 0x3c4c))[state->slot] == 2)
        {
            state->curFrame = state->endFrame;
            lbl_803DD112 = 1;
        }
        else if ((s8)((u8*)(base + 0x3c4c))[state->slot] == 3)
        {
            found = objSeqFindConditional(seq, obj);
            if (found > -1)
            {
                ((u8*)(base + 0x3cf4))[state->slot] = 1;
                state->curFrame = found;
                state->prevFrame = state->curFrame;
            }
        }

        if (state->targetObj != NULL && ((GameObject*)state->targetObj)->seqIndex != -1 &&
            (((u8*)(base + 0x3538))[state->slot] & 0x10) == 0)
        {
            (*gCameraInterface)->setLetterbox(0x41, 1);
        }

        slot = state->slot;
        if (((u8*)(base + 0x3590))[slot] != 0)
        {
            state->heading = ((s16*)(base + 0x35e8))[slot];
        }

        if (state->pendingConditionId != 0)
        {
            if (ObjSeq_EvaluateCondition(state->pendingConditionId - 1, seq, (int)placement) == 0)
            {
                state->pendingConditionId = 0;
            }
            else
            {
                ((f32*)(base + 0x3740))[state->slot] = (f32)state->curFrame;
                return 0;
            }
        }

        state->curFrame = (s16)(state->curFrame + step);
        if (state->curFrame > state->endFrame)
        {
            state->curFrame = state->endFrame;
        }
        targetFrame = state->curFrame;
        ObjSeq_ApplyFrameCurves(obj, activeObj, seq, targetFrame);
        obj->anim.localPosX = obj->anim.localPosX + state->posStepX;
        obj->anim.localPosY = obj->anim.localPosY + state->posStepY;
        obj->anim.localPosZ = obj->anim.localPosZ + state->posStepZ;
        obj->anim.rotZ += state->rotStepZ;
        obj->anim.rotY += state->rotStepY;
        obj->anim.rotX += state->rotStepX;

        action = ObjSeq_GetActiveModel(activeObj);
        lbl_803DD0C0 = 0;
        if (action != NULL)
        {
            val = ObjSeq_SampleTrackCurve(seq, 13, state->prevFrame);
            prevX = placement->baseX + val;
            val = ObjSeq_SampleTrackCurve(seq, 11, state->prevFrame);
            prevZ = placement->baseZ + val;
        }
        state->curFrame = state->prevFrame;

        while (state->curFrame < targetFrame)
        {
            state->curFrame += 1;
            val = ObjSeq_SampleTrackCurve(seq, 13, state->curFrame);
            px = placement->baseX + val;
            val = ObjSeq_SampleTrackCurve(seq, 11, state->curFrame);
            pz = placement->baseZ + val;

            if (state->curFrame > 0 && (state->flags & 4) != 0)
            {
                if (state->useRootMotionSpeed == 1 && state->unk7B == 0 &&
                    action != NULL)
                {
                    f32 dx = px - prevX;
                    f32 dz = pz - prevZ;
                    if (ObjAnim_SampleRootCurvePhase(&activeObj->anim, sqrtf(dx * dx + dz * dz),
                                                     &moveProgress) == 0)
                    {
                        i = state->curFrame - 1;
                        val = ObjSeq_SampleTrackCurve(seq, 9, i);
                        moveProgress = 0.0004f * val;
                    }
                }
                else
                {
                    i = state->curFrame - 1;
                    val = ObjSeq_SampleTrackCurve(seq, 9, i);
                    moveProgress = 0.0004f * val;
                }

                if (action != NULL)
                {
                    ObjAnim_AdvanceCurrentMove(
                        (int)activeObj, moveProgress, 1.0f, &state->animEvents);
                    if (state->fade > 0.0f)
                    {
                        if (state->trackRunLength[10] != 0)
                        {
                            i = state->curFrame - 1;
                            rate = ObjSeq_SampleTrackCurve(seq, 10, i);
                        }
                        else
                        {
                            rate = 8.0f;
                        }
                        if (rate < 1.0f)
                        {
                            rate = 1.0f;
                        }
                        rate = 1.0f / rate;
                        state->fade = state->fade - rate;
                        fval = state->fade;
                        if (fval < 0.0f)
                        {
                            state->fade = 0.0f;
                        }
                    }
                }
                else
                {
                    activeObj->anim.currentMoveProgress += moveProgress;
                    fval = 1.0f;
                    while (activeObj->anim.currentMoveProgress > fval)
                    {
                        activeObj->anim.currentMoveProgress -= fval;
                    }
                    rate = 1.0f;
                    val = 0.0f;
                    while (activeObj->anim.currentMoveProgress < val)
                    {
                        activeObj->anim.currentMoveProgress += rate;
                    }
                }
            }

            prevX = px;
            prevZ = pz;

            stop = 0;
            while (stop == 0 && state->cmdCursor < state->cmdCount)
            {
                cmd = state->cmds + state->cmdCursor * 4;
                opcode = (s8)cmd[0];
                if (opcode == 0)
                {
                    if (state->curFrame >= *(s16*)(cmd + 2))
                    {
                        state->retriggerFrame = *(s16*)(cmd + 2);
                        state->cmdCursor += 1;
                    }
                    else
                    {
                        stop = 1;
                    }
                }
                else
                {
                    if (state->curFrame >= state->retriggerFrame)
                    {
                        if (opcode != 0xf)
                        {
                            state->retriggerFrame += cmd[1];
                        }
                        state->cmdCursor += 1;
                        if (ObjSeq_ExecuteActionCommand(obj, action, &cmd, 0, 0) != 0)
                        {
                            targetFrame = state->curFrame;
                        }
                        {
                            GameObject* t = *(GameObject**)obj->extra;
                            if (t == NULL)
                            {
                                t = obj;
                            }
                            action = ObjSeq_GetActiveModel(t);
                            activeObj = t;
                        }
                    }
                    else
                    {
                        stop = 1;
                    }
                }
            }
        }

        for (k = 0; k < 10; k++)
        {
            conditionOpcode = state->conditionOpcodes[k];
            if (conditionOpcode == 0)
            {
                continue;
            }
            pressed = ObjSeq_CheckConditionOpcode(state, obj, conditionOpcode);
            if (pressed != 0)
            {
                ((u8*)(base + 0x3cf4))[state->slot] = 1;
                state->curFrame = ((ObjSeqState*)seq)->conditionFrames[k];
                state->prevFrame = state->curFrame;
                state->conditionOpcodes[0] = 0;
                state->conditionOpcodes[1] = 0;
                state->conditionOpcodes[2] = 0;
                state->conditionOpcodes[3] = 0;
                state->conditionOpcodes[4] = 0;
                state->conditionOpcodes[5] = 0;
                state->conditionOpcodes[6] = 0;
                state->conditionOpcodes[7] = 0;
                state->conditionOpcodes[8] = 0;
                state->conditionOpcodes[9] = 0;
                break;
            }
        }

        if ((s8)lbl_803DD0D8 == 0 && activeObj != obj)
        {
            objCallSeqFn(activeObj, obj, seq, ((u8*)(base + 0x3c4c))[state->slot]);
        }

        if (state->sequenceControlFlags != 0)
        {
            restart = 0;
            if ((state->sequenceControlFlags & OBJSEQ_CONTROL_RESTART_AT_SAVED_FRAME) != 0)
            {
                restart = 1;
                state->sequenceControlFlags =
                    state->sequenceControlFlags & ~OBJSEQ_CONTROL_RESTART_AT_SAVED_FRAME;
                state->curFrame = (s16)state->savedFrame;
                state->prevFrame = state->curFrame;
            }
            state->sequenceControlFlags = 0;
            ((s8*)(base + 0x3cf4))[state->slot] = restart;
        }

        state->eventCount = 0;
        state->unk80 = 0;
        if (action != NULL && (state->flags & 4) != 0)
        {
            ((ObjAnimBank*)action)->currentState->eventCountdown = (u16)(int)(16384.0f * state->fade);
        }
        ObjSeq_UpdateCurvePosition(obj, seq);
        if ((s8)state->groundSnapEnabled == 1 &&
            hitDetectFn_800658a4(obj, obj->anim.localPosX, obj->anim.localPosY,
                                 obj->anim.localPosZ, &groundY, 0) == 0)
        {
            obj->anim.localPosY =
                obj->anim.localPosY +
                ((obj->anim.localPosY - groundY) - placement->groundOffset);
        }
        obj->anim.rotX += state->heading;
        ObjSeq_ApplyLinkedObjectTransform(obj, activeObj, seq);
        objSeqDoBgCmds0D(seq, activeObj, 0);

        for (k = 0; k < lbl_803DD0C0; k++)
        {
            entry = (ObjSeqBgCmd*)(base + k * 8);
            entry = (ObjSeqBgCmd*)((int)entry + 0x2b34);
            if (seqDoSubCmd0B(obj, activeObj, seq, (u8*)entry->object, entry->flags, entry->param, 0, 0) != 0)
            {
                k = lbl_803DD0C0;
            }
            {
                GameObject* t = *(GameObject**)obj->extra;
                if (t == NULL)
                {
                    t = obj;
                }
                action = ObjSeq_GetActiveModel(t);
                activeObj = t;
            }
        }

        if (gObjSeqStreamStopped != 0)
        {
            gObjSeqStreamStopped = ObjSeq_StartPreparedStream(gObjSeqPreparingStreamSlot) == 0;
        }
        state->prevFrame = state->curFrame;

        if ((s8)gObjSeqStop != 0)
        {
            {
                GameObject* t = *(GameObject**)obj->extra;
                if (t == NULL)
                {
                    t = obj;
                }
                action = ObjSeq_GetActiveModel(t);
                activeObj = t;
                animatedObjFreeAndSavePlayerPos(obj, t, seq);
            }
        }
        else
        {
            slot = state->slot;
            if (((s8*)(base + 0x3cf4))[slot] != 0)
            {
                ((s16*)(base + 0x3694))[slot] = state->curFrame;
                ((u8*)(base + 0x338c))[state->slot] = 2;
                ((f32*)(base + 0x3740))[state->slot] = (f32)state->curFrame;
            }
            if (-1.0f == ((f32*)(base + 0x3740))[slot = state->slot])
            {
                if (gObjSeqTimedStreamSlot == slot)
                {
                    fval = gObjSeqStreamRemainingTime;
                    aInt = fval;
                    gObjSeqStreamRemainingTime = fval - 0.16666667f;
                    fval = gObjSeqStreamRemainingTime;
                    if (aInt != (int)fval)
                    {
                        step--;
                        if (fval <= 0.0f)
                        {
                            gObjSeqTimedStreamSlot = -1;
                        }
                    }
                }
                ((f32*)(base + 0x3740))[state->slot] =
                    step + ((f32*)(base + 0x3894))[state->slot];
            }
        }

        if ((s8)gObjSeqStop != 0)
        {
            break;
        }
        if (state->curFrame >= state->endFrame)
        {
            break;
        }
    } while (runs-- != 0);

    return 0;
}

int ObjSeq_takeXrotChanged(int index)
{
    int changed;

    changed = objSeqXrotChanged[index];
    objSeqXrotChanged[index] = 0;
    return changed;
}

void ObjSeq_setXrot(int index, int xrot)
{
    s16 xrot16;

    objSeqXrotChanged[index] = 1;
    xrot16 = xrot;
    objSeqXrotValues[index] = xrot16;
}

int ObjSeq_getBool(int index)
{
    if (index < 0 || index >= 0x55)
    {
        return 0;
    }
    return gObjSeqBoolFlags[index];
}

void ObjSeq_setBool(int index, int value)
{
    s8 flag;

    if (index < 0 || index >= 0x55)
    {
        return;
    }
    flag = value;
    gObjSeqBoolFlags[index] = flag;
}

void ObjSeq_addBgCmd(int index, int xrot, int yrot)
{
    s8 count;
    s16 shortIndex;
    s16 shortXrot;
    s16 shortYrot;

    if (index < 0 || index >= 0x55)
    {
        return;
    }

    count = gObjSeqBgCmdCount;
    if (count >= 0x1e)
    {
        return;
    }

    shortIndex = index;
    shortYrot = yrot;
    shortXrot = xrot;
    gObjSeqBgCmds[count * 3] = shortIndex;
    gObjSeqBgCmds[count * 3 + 2] = shortYrot;
    gObjSeqBgCmds[gObjSeqBgCmdCount++ * 3 + 1] = shortXrot;
}
u8 gObjSeqRuntimeBuffer[0x2A80];
s16 gObjSeqBgCmds[0x5A];
u8 lbl_8039944C[0xA0];
f32 objSeqOverridePos[0x259];
u8 lbl_80399E50[0x58];
u8 objSeqXrotChanged[0x58];
s16 objSeqXrotValues[0x156];
f32 gObjSeqSlotStreamTimeTable[0x81];
s16 gObjSeqSlotSeqIdTable[0x56];
s8 gObjSeqBoolFlags[0x58];
s8 gObjSeqCondFlags[0x58];
s8 gObjSeqSlotResults[0xB0];
ObjSeqBgCmd lbl_8039A5BC[0x50 / sizeof(ObjSeqBgCmd)];
s8 gObjSeqJumpLatch[0x58];
int gObjSeqPreemptList[40][2];


extern u8 gObjSeqCameraActive;
extern int gObjSeqCamMode;
extern int gObjSeqCamModeArgB;
extern int gObjSeqCamModeArgD;
extern s8 gObjSeqBgCmdCount;

#define OBJSEQ_SLOT_COUNT 85

typedef struct ObjSeqRuntimeStorage {
    u8 _reserved0000[0x338c];
    u8 marks[0x58];
    int handles[OBJSEQ_SLOT_COUNT];
    u8 _reserved3538[0x58];
    u8 counts[0x58];
    u8 _reserved35e8[0x158];
    f32 distances[OBJSEQ_SLOT_COUNT];
    f32 frames[OBJSEQ_SLOT_COUNT];
    u8 pending[0x58];
    u8 states[0x58];
    s16 modes[0x56];
    u8 flagsA[0x58];
    u8 flagsB[0x58];
    u8 results[0x58];
    u8 actions[0x58];
} ObjSeqRuntimeStorage;

STATIC_ASSERT(offsetof(ObjSeqRuntimeStorage, marks) == 0x338c);
STATIC_ASSERT(offsetof(ObjSeqRuntimeStorage, handles) == 0x33e4);
STATIC_ASSERT(offsetof(ObjSeqRuntimeStorage, counts) == 0x3590);
STATIC_ASSERT(offsetof(ObjSeqRuntimeStorage, distances) == 0x3740);
STATIC_ASSERT(offsetof(ObjSeqRuntimeStorage, frames) == 0x3894);
STATIC_ASSERT(offsetof(ObjSeqRuntimeStorage, pending) == 0x39e8);
STATIC_ASSERT(offsetof(ObjSeqRuntimeStorage, states) == 0x3a40);
STATIC_ASSERT(offsetof(ObjSeqRuntimeStorage, modes) == 0x3a98);
STATIC_ASSERT(offsetof(ObjSeqRuntimeStorage, flagsA) == 0x3b44);
STATIC_ASSERT(offsetof(ObjSeqRuntimeStorage, flagsB) == 0x3b9c);
STATIC_ASSERT(offsetof(ObjSeqRuntimeStorage, results) == 0x3bf4);
STATIC_ASSERT(offsetof(ObjSeqRuntimeStorage, actions) == 0x3c4c);

void ObjSeq_onMapSetup(void)
{
    u8* base = gObjSeqRuntimeBuffer;
    u8* flagsA;
    s16* modes;
    u8* actions;
    u8* results;
    u8* states;
    u8* pending;
    f32* frames;
    f32* dists;
    int* handles;
    u8* counts;
    u8* marks;
    u8* flagsB;
    int i = 0;
    f32 neg1;
    f32 zero;

    flagsB = base + offsetof(ObjSeqRuntimeStorage, flagsB);
    flagsA = base + offsetof(ObjSeqRuntimeStorage, flagsA);
    modes = (s16*)(base + offsetof(ObjSeqRuntimeStorage, modes));
    actions = base + offsetof(ObjSeqRuntimeStorage, actions);
    results = base + offsetof(ObjSeqRuntimeStorage, results);
    states = base + offsetof(ObjSeqRuntimeStorage, states);
    pending = base + offsetof(ObjSeqRuntimeStorage, pending);
    frames = (f32*)(base + offsetof(ObjSeqRuntimeStorage, frames));
    dists = (f32*)(base + offsetof(ObjSeqRuntimeStorage, distances));
    counts = base + offsetof(ObjSeqRuntimeStorage, counts);
    handles = (int*)(base + offsetof(ObjSeqRuntimeStorage, handles));
    marks = base + offsetof(ObjSeqRuntimeStorage, marks);

    {
        zero = 0.0f;
        neg1 = -1.0f;
        for (; i < 0x50; i += 8)
        {
            flagsB[0] = 0;
            flagsA[0] = 0;
            modes[0] = 0;
            actions[0] = 0;
            results[0] = 0;
            states[0] = 0;
            pending[0] = 0;
            frames[0] = zero;
            dists[0] = neg1;
            counts[0] = 0;
            handles[0] = 0;
            marks[0] = 0;
            flagsB[1] = 0;
            flagsA[1] = 0;
            modes[1] = 0;
            actions[1] = 0;
            results[1] = 0;
            states[1] = 0;
            pending[1] = 0;
            frames[1] = zero;
            dists[1] = neg1;
            counts[1] = 0;
            handles[1] = 0;
            marks[1] = 0;
            flagsB[2] = 0;
            flagsA[2] = 0;
            modes[2] = 0;
            actions[2] = 0;
            results[2] = 0;
            states[2] = 0;
            pending[2] = 0;
            frames[2] = zero;
            dists[2] = neg1;
            counts[2] = 0;
            handles[2] = 0;
            marks[2] = 0;
            flagsB[3] = 0;
            flagsA[3] = 0;
            modes[3] = 0;
            actions[3] = 0;
            results[3] = 0;
            states[3] = 0;
            pending[3] = 0;
            frames[3] = zero;
            dists[3] = neg1;
            counts[3] = 0;
            handles[3] = 0;
            marks[3] = 0;
            flagsB[4] = 0;
            flagsA[4] = 0;
            modes[4] = 0;
            actions[4] = 0;
            results[4] = 0;
            states[4] = 0;
            pending[4] = 0;
            frames[4] = zero;
            dists[4] = neg1;
            counts[4] = 0;
            handles[4] = 0;
            marks[4] = 0;
            flagsB[5] = 0;
            flagsA[5] = 0;
            modes[5] = 0;
            actions[5] = 0;
            results[5] = 0;
            states[5] = 0;
            pending[5] = 0;
            frames[5] = zero;
            dists[5] = neg1;
            counts[5] = 0;
            handles[5] = 0;
            marks[5] = 0;
            flagsB[6] = 0;
            flagsA[6] = 0;
            modes[6] = 0;
            actions[6] = 0;
            results[6] = 0;
            states[6] = 0;
            pending[6] = 0;
            frames[6] = zero;
            dists[6] = neg1;
            counts[6] = 0;
            handles[6] = 0;
            marks[6] = 0;
            flagsB[7] = 0;
            flagsA[7] = 0;
            modes[7] = 0;
            actions[7] = 0;
            results[7] = 0;
            states[7] = 0;
            pending[7] = 0;
            frames[7] = zero;
            dists[7] = neg1;
            counts[7] = 0;
            handles[7] = 0;
            marks[7] = 0;
            flagsB += 8;
            flagsA += 8;
            modes += 8;
            actions += 8;
            results += 8;
            states += 8;
            pending += 8;
            frames += 8;
            dists += 8;
            counts += 8;
            handles += 8;
            marks += 8;
        }
    }

    {
        marks = base + i;
        modes = (s16*)(base + i * 2);
        modes += 0x3a98 / 2;
        handles = (int*)(base + i * 4);
        handles += 0x33e4 / 4;
        marks += 0x338c;
        zero = 0.0f;
        neg1 = -1.0f;
        while (i < 85)
        {
            frames = (f32*)(handles + 300);
            dists = (f32*)(handles + 215);
            flagsA = marks + 0x810;
            flagsB = marks + 0x7b8;
            actions = marks + 0x8c0;
            results = marks + 0x868;
            states = marks + 0x6b4;
            pending = marks + 0x65c;
            counts = marks + 0x204;
            *flagsA++ = 0;
            *flagsB++ = 0;
            modes[0] = 0;
            *actions++ = 0;
            *results++ = 0;
            *states++ = 0;
            *pending++ = 0;
            *frames++ = zero;
            *dists++ = neg1;
            *counts++ = 0;
            handles[0] = 0;
            marks[0] = 0;
            marks[0] = 0;
            modes++;
            handles++;
            marks++;
            i++;
        }
    }

    lbl_803DD124 = 0;
    gObjSeqCamMode = 0;
    gObjSeqCameraActive = 0;
    lbl_803DD0DC = 0.0f;
    lbl_803DD0B8 = NULL;
    lbl_803DD0F8 = 0;
    gObjSeqBgCmdCount = 0;
}

void ObjSeq_release(void)
{
    mm_free(lbl_803DD0D4);
}

void ObjSeq_initialise(void)
{
    lbl_803DD0D4 = mmAlloc(0x10, 0x11, 0);
    ObjSeq_onMapSetup();
    gObjSeqCamModeArgB = 1;
    gObjSeqCamModeArgD = 0x5a;
    gObjSeqCamMode = 0x42;
    seqPairTablePrepare(gObjSeqStreamTableA, 5);
}

void ObjSeq_copyDefaultColor(GXColor* out)
{
    GXColor* src;

    out->r = gObjSeqDefaultColor.r;
    src = &gObjSeqDefaultColor;
    out->g = src->g;
    out->b = src->b;
    out->a = src->a;
}
