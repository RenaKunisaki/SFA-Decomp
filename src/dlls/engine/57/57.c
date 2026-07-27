#include "types.h"
#include "main/rcp_dolphin_api.h"
#include "main/model_engine.h"
#include "sys/objects.h"
#include "main/frame_timing.h"
#include "main/texture.h"
#include "main/dll/dll_0039_dummy39.h"

#define DUMMY39_COUNTDOWN_FRAMES 0x28
#define DUMMY39_WARP_MAP         0x60
#define DUMMY39_MAX_STEP_FRAMES  3

extern u8 lbl_803DD728;
extern u8* lbl_803DD72C;

void Dummy39_render(void)
{
}

void Dummy39_frameEnd(void)
{
}

int Dummy39_run(void)
{
    s32 step;
    u8 cur;
    s8 next;
    Obj_GetPlayerObject();
    step = framesThisStep;
    if (step > DUMMY39_MAX_STEP_FRAMES)
        step = DUMMY39_MAX_STEP_FRAMES;
    cur = lbl_803DD728;
    if ((s8)cur > 0)
    {
        next = cur - step;
        *(s8*)&lbl_803DD728 = next;
        if ((s8)(u8)next <= 0)
        {
            loadUiDll(1);
            warpToMap(DUMMY39_WARP_MAP, 1);
        }
    }
    return 0;
}

void Dummy39_release(void)
{
    textureFree((Texture*)(lbl_803DD72C));
}

void Dummy39_initialise(void)
{
    lbl_803DD728 = DUMMY39_COUNTDOWN_FRAMES;
}

u32 lbl_8031ADD0[10] = {0x00000000, 0x00000000, 0x00000000, 0x00050000,
        (u32)Dummy39_initialise, (u32)Dummy39_release,
        0x00000000, (u32)Dummy39_run, (u32)Dummy39_frameEnd,
        (u32)Dummy39_render};
