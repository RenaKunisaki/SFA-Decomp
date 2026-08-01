#include "main/game_ui_interface.h"
#include "main/model_engine.h"
#include "main/dll/dll_0010_uicontroller.h"

void UIController_render(void* context, int arg1, int arg2)
{
    if (gameTimerIsRunning() != 0)
    {
        gameTimerRun(context);
    }
    hudNumberRender(context);
    (*gGameUIInterface)->render(context, arg1, arg2);
}

void UIController_frameEnd(void)
{
    (*gGameUIInterface)->frameEnd();
}

void UIController_frameStart(void)
{
    (*gGameUIInterface)->frameStart();
}

void UIController_release(void)
{
}

void UIController_initialise(void)
{
}

u32 UIController_funcs[10] = {
    0, 0, 0, 0x00050000,
    (u32)UIController_initialise, (u32)UIController_release, 0, (u32)UIController_frameStart,
    (u32)UIController_frameEnd, (u32)UIController_render,
};
