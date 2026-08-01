#include "musyx/vid_get.h"
#include "musyx/mcmd.h"
#include "musyx/vid_init.h"

VID_LIST* get_vidlist(u32 id)
{
    VID_LIST* node;
    u32 value;

    node = vidRoot;
    while (node != NULL)
    {
        value = node->vid;
        if (value == id)
        {
            return node;
        }
        if (value > id)
        {
            break;
        }
        node = node->next;
    }
    return NULL;
}
