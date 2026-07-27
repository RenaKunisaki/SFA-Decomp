/*
 * THPRead - attract-movie DVD reader thread and message queues.
 */
#include "global.h"
#include "main/dll/FRONT/attract_movie.h"
#include "main/dll/FRONT/picmenu.h"
#include "main/fileio.h"
#include "dolphin/os/OSMessage.h"
#include "dolphin/os/OSThread.h"

char gPicMenuReadThreadArea[0x13E8];
s32 gPicMenuReadThreadCreated;

#define gPicMenuReadThread             (*(OSThread*)(gPicMenuReadThreadArea + 0x1000))
#define gPicMenuReadedBuffer2Queue     (*(OSMessageQueue*)(gPicMenuReadThreadArea + 0x1388))
#define gPicMenuReadedBufferQueue      (*(OSMessageQueue*)(gPicMenuReadThreadArea + 0x13A8))
#define gPicMenuFreeReadBufferQueue    (*(OSMessageQueue*)(gPicMenuReadThreadArea + 0x13C8))

void PushReadedBuffer2(OSMessage msg)
{
    OSSendMessage(&gPicMenuReadedBuffer2Queue, msg, OS_MESSAGE_BLOCK);
}

OSMessage PopReadedBuffer2(void)
{
    OSMessage msg;
    OSReceiveMessage(&gPicMenuReadedBuffer2Queue, &msg, OS_MESSAGE_BLOCK);
    return msg;
}

void PushFreeReadBuffer(OSMessage msg)
{
    OSSendMessage(&gPicMenuFreeReadBufferQueue, msg, OS_MESSAGE_BLOCK);
}

OSMessage PopReadedBuffer(void)
{
    OSMessage msg;
    OSReceiveMessage(&gPicMenuReadedBufferQueue, &msg, OS_MESSAGE_BLOCK);
    return msg;
}

void* THPRead_Reader(void* unused)
{
    AttractMovieReadBuffer* req;
    u32 readOff;
    u32 readSize;
    char* base;
    int i;

    base = gPicMenuReadThreadArea;
    i = 0;
    readOff = lbl_803A5D60.initOffset;
    readSize = lbl_803A5D60.initReadSize;

    while (1)
    {
        OSMessage msgVal;
        s32 res;

        OSReceiveMessage((OSMessageQueue*)(base + 0x13C8), &msgVal, OS_MESSAGE_BLOCK);
        req = (AttractMovieReadBuffer*)msgVal;

        res = DVDReadPrio(&lbl_803A5D60.fileInfo, req->ptr, readSize, readOff, 2);
        if (res != (s32)readSize)
        {
            if (res == -1)
            {
                lbl_803A5D60.dvdError = -1;
            }
            if (i == 0)
            {
                PrepareReady(0);
            }
            OSSuspendThread((OSThread*)(base + 0x1000));
        }

        req->frameNumber = i;
        OSSendMessage((OSMessageQueue*)(base + 0x13A8), (OSMessage)req, OS_MESSAGE_BLOCK);

        readOff += readSize;
        readSize = *(u32*)req->ptr;

        {
            u32 cols = lbl_803A5D60.header.mNumFrames;
            u32 bOff = lbl_803A5D60.initReadFrame;
            u32 pos = (i + bOff) % cols;
            if (pos == cols - 1)
            {
                if (lbl_803A5D60.playFlags & 1)
                {
                    readOff = lbl_803A5D60.header.mMovieDataOffsets;
                }
                else
                {
                    OSSuspendThread((OSThread*)(base + 0x1000));
                }
            }
        }
        i++;
    }
}

void ReadThreadCancel(void)
{
    if (gPicMenuReadThreadCreated != 0)
    {
        OSCancelThread(&gPicMenuReadThread);
        gPicMenuReadThreadCreated = 0;
    }
}

void ReadThreadStart(void)
{
    if (gPicMenuReadThreadCreated != 0)
    {
        OSResumeThread(&gPicMenuReadThread);
    }
}

BOOL CreateReadThread(OSPriority priority)
{
    char* base = gPicMenuReadThreadArea;
    char* stack = base + 0x1000;

    if (!OSCreateThread((OSThread*)stack, THPRead_Reader, NULL, stack, 0x1000, priority, 1))
    {
        return 0;
    }

    OSInitMessageQueue((OSMessageQueue*)(base + 0x13C8), (void*)(base + 0x1360), 10);
    OSInitMessageQueue((OSMessageQueue*)(base + 0x13A8), (void*)(base + 0x1338), 10);
    OSInitMessageQueue((OSMessageQueue*)(base + 0x1388), (void*)(base + 0x1310), 10);
    gPicMenuReadThreadCreated = 1;
    return 1;
}
