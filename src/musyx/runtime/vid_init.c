#include "musyx/vid_init.h"

VID_LIST* vidFree;
VID_LIST* vidRoot;
u32 vidCurrentId;

void vidInit(void)
{
    int i;
    VID_LIST* prev;

    vidCurrentId = 0;
    vidRoot = 0;
    vidFree = vidList;
    for (prev = NULL, i = 0; i < 128; prev = &vidList[i], ++i)
    {
        vidList[i].prev = prev;
        if (prev != NULL)
        {
            prev->next = &vidList[i];
        }
    }
    prev->next = NULL;
}
