#include "musyx/vid_get.h"
#include "musyx/mcmd.h"
#include "musyx/vid_init.h"

McmdVidListNode* get_vidlist(u32 id)
{
    McmdVidListNode* node;
    u32 value;

    node = vidRoot;
    while (node != NULL)
    {
        value = node->id;
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
