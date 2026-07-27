#include "dolphin/os/OSReport.h"
#include "main/screen_transition.h"
#include "main/gamebits.h"
#include "main/frame_timing.h"
#include "main/model_engine.h"
#include "main/dll/FRONT/dll_39.h"
extern int gNrarewareFrameCounter;
extern f32 gNrarewareStage3Timer;
extern f32 gNrarewareStage1Timer;
extern u8 gNrarewareTransitionStarted;
extern u8 gNrarewareTimeoutFlag;
extern s8 gNrarewareExitDelay;
extern s8 gNrarewareStage;

void n_rareware_render(void)
{
    int frame;

    if (((s8)gNrarewareTransitionStarted != 0) && ((s8)gNrarewareExitDelay <= 10))
    {
        return;
    }

    frame = gNrarewareFrameCounter;
    if ((frame > 40) && ((s8)gNrarewareStage == 0))
    {
        gNrarewareStage = 1;
        gNrarewareStage1Timer = 5e+02f;
    }
    if ((frame > 50) && ((s8)gNrarewareStage == 1))
    {
        gNrarewareStage = 2;
    }
    if ((frame > 285) && ((s8)gNrarewareStage == 2))
    {
        gNrarewareStage = 3;
        gNrarewareStage3Timer = 145.0f;
    }
}

void n_rareware_frameEnd(void)
{
}

u32 lbl_8031A1A0[10] = {0x00000000,
                        0x00000000,
                        0x00000000,
                        0x00050000,
                        (u32)n_rareware_initialise,
                        (u32)n_rareware_release,
                        0x00000000,
                        (u32)n_rareware_frameStart,
                        (u32)n_rareware_frameEnd,
                        (u32)n_rareware_render};

static char sNRarewareReportTag[] = "n_rareware\n";

int n_rareware_frameStart(void)
{
    int frameStep;

    frameStep = framesThisStep;
    OSReport(sNRarewareReportTag);
    if (frameStep > 3)
    {
        frameStep = 3;
    }
    if ((s8)gNrarewareExitDelay > 0)
    {
        gNrarewareExitDelay = (s8)(gNrarewareExitDelay - frameStep);
    }
    if ((s8)gNrarewareTransitionStarted != 0)
    {
        mainSetBits(GAMEBIT_MenuRelated044F, 0);
        loadUiDll(4);
    }
    gNrarewareFrameCounter += framesThisStep;
    if (gNrarewareFrameCounter > 0x26c)
    {
        gNrarewareTimeoutFlag = 1;
    }
    if ((s8)gNrarewareTimeoutFlag != 0)
    {
        (*gScreenTransitionInterface)->start(0x1e, 1);
        gNrarewareExitDelay = 0x2d;
        gNrarewareTransitionStarted = 1;
    }
    if (gNrarewareStage > 0)
    {
        gNrarewareStage1Timer -= timeDelta;
    }
    if (gNrarewareStage > 2)
    {
        gNrarewareStage3Timer -= timeDelta;
    }
    return 0;
}

void n_rareware_release(void)
{
}

void n_rareware_initialise(void)
{
    fn_8001404C(0);
    gNrarewareFrameCounter = 0;
    gNrarewareStage = 0;
    gNrarewareTimeoutFlag = 0;
    gNrarewareExitDelay = 0;
    gNrarewareTransitionStarted = 0;
}
