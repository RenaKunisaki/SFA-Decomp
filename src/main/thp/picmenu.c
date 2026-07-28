/*
 * picmenu - THP movie loading and audio DMA setup.
 *
 * Opens and validates the attract-mode THP stream, reads its component
 * metadata, and initializes or shuts down the movie audio path.
 */
#include "dolphin/ai.h"
#include "main/dll/FRONT/attract_movie.h"
#include "main/dll/FRONT/n_options.h"
#include "dolphin/os/OSCache.h"
#include "dolphin/os/OSInterrupt.h"
#include "dolphin/os/OSMessage.h"

char sPicMenuThpMagic[] = "THP";

#define THP_VERSION_1_0 0x10000

/* per-frame component kinds in THPHeader::mCompInfoDataOffsets table */
enum
{
    THP_COMPONENT_VIDEO = 0,
    THP_COMPONENT_AUDIO = 1
};

extern char lbl_803A57C0[0x50C];
extern s32 lbl_803DD660;
extern AIDCallback lbl_803DD668;
extern s32 lbl_803DD66C;
extern u32 lbl_803DD670;
extern u32 lbl_803DD674;
extern u32 lbl_803DD678;

BOOL movieLoad(const char* fileName, void* onMemory)
{
    u32 readOff;
    s32 result;
    u32 i;

    if (lbl_803DD660 == 0)
    {
        return 0;
    }

    if (((AttractMoviePlayer*)&lbl_803A5D60)->isOpen != 0)
    {
        return 0;
    }

    memset(&((AttractMoviePlayer*)&lbl_803A5D60)->videoInfo, 0, sizeof(AttractMovieVideoInfo));
    memset(&((AttractMoviePlayer*)&lbl_803A5D60)->audioInfo, 0, sizeof(AttractMovieAudioInfo));

    if (!DVDOpen(fileName, (DVDFileInfo*)&lbl_803A5D60))
    {
        return 0;
    }

    result = DVDRead((DVDFileInfo*)&lbl_803A5D60, gPicMenuDvdReadBuffer, 0x40, 0);
    if (result < 0)
    {
        DVDClose((DVDFileInfo*)&lbl_803A5D60);
        return 0;
    }

    memcpy(&((AttractMoviePlayer*)&lbl_803A5D60)->header, gPicMenuDvdReadBuffer,
           sizeof(((AttractMoviePlayer*)&lbl_803A5D60)->header));

    if (strcmp(((AttractMoviePlayer*)&lbl_803A5D60)->header.mMagic, sPicMenuThpMagic) != 0)
    {
        DVDClose((DVDFileInfo*)&lbl_803A5D60);
        return 0;
    }

    if (((AttractMoviePlayer*)&lbl_803A5D60)->header.mVersion != THP_VERSION_1_0)
    {
        DVDClose((DVDFileInfo*)&lbl_803A5D60);
        return 0;
    }

    {
        u32 compOff = ((AttractMoviePlayer*)&lbl_803A5D60)->header.mCompInfoDataOffsets;

        result = DVDRead((DVDFileInfo*)&lbl_803A5D60, gPicMenuDvdReadBuffer, 0x20, compOff);
        if (result < 0)
        {
            DVDClose((DVDFileInfo*)&lbl_803A5D60);
            return 0;
        }

        memcpy(&((AttractMoviePlayer*)&lbl_803A5D60)->compInfo, gPicMenuDvdReadBuffer, sizeof(THPFrameCompInfo));
        readOff = compOff + sizeof(THPFrameCompInfo);
        ((AttractMoviePlayer*)&lbl_803A5D60)->audioExists = 0;
    }

    for (i = 0; i < ((AttractMoviePlayer*)&lbl_803A5D60)->compInfo.mNumComponents; i++)
    {
        switch (((AttractMoviePlayer*)&lbl_803A5D60)->compInfo.mFrameComp[i])
        {
        case THP_COMPONENT_VIDEO:
            result = DVDRead((DVDFileInfo*)&lbl_803A5D60, gPicMenuDvdReadBuffer, 0x20, readOff);
            if (result < 0)
            {
                DVDClose((DVDFileInfo*)&lbl_803A5D60);
                return 0;
            }
            memcpy(&((AttractMoviePlayer*)&lbl_803A5D60)->videoInfo, gPicMenuDvdReadBuffer,
                   sizeof(AttractMovieVideoInfo));
            readOff += sizeof(AttractMovieVideoInfo);
            break;
        case THP_COMPONENT_AUDIO:
            result = DVDRead((DVDFileInfo*)&lbl_803A5D60, gPicMenuDvdReadBuffer, 0x20, readOff);
            if (result < 0)
            {
                DVDClose((DVDFileInfo*)&lbl_803A5D60);
                return 0;
            }
            memcpy(&((AttractMoviePlayer*)&lbl_803A5D60)->audioInfo, gPicMenuDvdReadBuffer,
                   sizeof(AttractMovieAudioInfo));
            ((AttractMoviePlayer*)&lbl_803A5D60)->audioExists = 1;
            readOff += sizeof(AttractMovieAudioInfo);
            break;
        default:
            return 0;
        }
    }

    ((AttractMoviePlayer*)&lbl_803A5D60)->internalState = 0;
    ((AttractMoviePlayer*)&lbl_803A5D60)->state = 0;
    ((AttractMoviePlayer*)&lbl_803A5D60)->playFlags = 0;
    ((AttractMoviePlayer*)&lbl_803A5D60)->isOnMemory = (s32)onMemory;
    ((AttractMoviePlayer*)&lbl_803A5D60)->isOpen = 1;
    ((AttractMoviePlayer*)&lbl_803A5D60)->curVolume = 127.0f;
    ((AttractMoviePlayer*)&lbl_803A5D60)->targetVolume = 127.0f;
    ((AttractMoviePlayer*)&lbl_803A5D60)->rampCount = 0;

    return 1;
}

void AttractMovieAudio_Shutdown(void)
{
    u32 saved = OSDisableInterrupts();
    if (lbl_803DD668 != (AIDCallback)0)
    {
        AIRegisterDMACallback(lbl_803DD668);
    }
    OSRestoreInterrupts(saved);
    lbl_803DD660 = 0;
}

BOOL AttractMovieAudio_Init(int audioMode)
{
    u32 saved;
    AIDCallback oldCb;
    register AIDCallback dmaCallback;

    memset((AttractMoviePlayer*)((char*)(int)lbl_803A57C0 + 0x5A0), 0, sizeof(AttractMoviePlayer));
    OSInitMessageQueue((OSMessageQueue*)((char*)(int)lbl_803A57C0 + 0x50C),
                       (void*)((char*)(int)lbl_803A57C0 + ATTRACT_MOVIE_AUDIO_DMA_BUFFER_BYTES), 3);

    if (!THPInit())
    {
        return 0;
    }

    saved = OSDisableInterrupts();
    lbl_803DD66C = audioMode;
    lbl_803DD678 = 0;
    lbl_803DD674 = 0;
    lbl_803DD670 = 0;
    dmaCallback = AttractMovieAudio_DmaCallback;
    oldCb = AIRegisterDMACallback(dmaCallback);
    lbl_803DD668 = oldCb;

    if (oldCb == (AIDCallback)0)
    {
        if (lbl_803DD66C != 0)
        {
            AIRegisterDMACallback((AIDCallback)0);
            OSRestoreInterrupts(saved);
            return 0;
        }
    }

    OSRestoreInterrupts(saved);

    if (lbl_803DD66C == 0)
    {
        memset((char*)(int)lbl_803A57C0, 0, ATTRACT_MOVIE_AUDIO_DMA_BUFFER_BYTES);
        DCFlushRange((char*)(int)lbl_803A57C0, ATTRACT_MOVIE_AUDIO_DMA_BUFFER_BYTES);
        AIInitDMA((u32)((char*)(int)lbl_803A57C0 + lbl_803DD678 * ATTRACT_MOVIE_AUDIO_DMA_BUFFER_SIZE),
                  ATTRACT_MOVIE_AUDIO_DMA_BUFFER_SIZE);
        AIStartDMA();
    }

    lbl_803DD660 = 1;
    return 1;
}
