#include "dolphin.h"
#include "dolphin/TRK_MINNOW_DOLPHIN/MWTrace.h"
#include "main/gametext_box_api.h"
#include "main/textrender_api.h"

extern GameTextBox gTextBoxes[];
extern void* gCurTextBox;

void MWTRACE(int boxId)
{
    int i = gGameTextCommandCount;
    GameTextSlot* cmd;
    void* box;

    gGameTextCommandCount = i + 1;
    cmd = &gGameTextCommandSlots[i];
    if (boxId == 0xff)
    {
        box = NULL;
    }
    else
    {
        box = &gTextBoxes[boxId];
    }
    gCurTextBox = box;
    cmd->opcode = 8;
    cmd->arg0 = boxId;
}
