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
    vidFree = vidListNodes;
    for (prev = NULL, i = 0; i < 128; prev = &vidListNodes[i], ++i)
    {
        vidListNodes[i].prev = prev;
        if (prev != NULL)
        {
            prev->next = &vidListNodes[i];
        }
    }
    prev->next = NULL;
}
