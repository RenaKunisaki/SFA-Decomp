/*
 * THPRead - attract-movie DVD reader thread and message queues.
 */
#include "global.h"
#include "main/dll/FRONT/attract_movie.h"
#include "dolphin/os/OSThread.h"
#include "dolphin/thp/THPPlayer.h"

char gPicMenuReadThreadArea[0x1000];
OSThread gPicMenuReadThread;
OSMessageQueue gPicMenuReadedBuffer2Queue;
OSMessageQueue gPicMenuReadedBufferQueue;
OSMessageQueue gPicMenuFreeReadBufferQueue;
OSMessage gPicMenuReadedBuffer2Messages[10];
OSMessage gPicMenuReadedBufferMessages[10];
OSMessage gPicMenuFreeReadBufferMessages[10];

s32 gPicMenuReadThreadCreated;

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
    readOff = gAttractMoviePlayer.initOffset;
    readSize = gAttractMoviePlayer.initReadSize;

    while (1)
    {
        OSMessage msgVal;
        s32 res;

        OSReceiveMessage((OSMessageQueue*)(base + 0x13C8), &msgVal, OS_MESSAGE_BLOCK);
        req = (AttractMovieReadBuffer*)msgVal;

        res = DVDReadPrio(&gAttractMoviePlayer.fileInfo, req->ptr, readSize, readOff, 2);
        if (res != (s32)readSize)
        {
            if (res == -1)
            {
                gAttractMoviePlayer.dvdError = -1;
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
            u32 cols = gAttractMoviePlayer.header.mNumFrames;
            u32 bOff = gAttractMoviePlayer.initReadFrame;
            u32 pos = (i + bOff) % cols;
            if (pos == cols - 1)
            {
                if (gAttractMoviePlayer.playFlags & 1)
                {
                    readOff = gAttractMoviePlayer.header.mMovieDataOffsets;
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
