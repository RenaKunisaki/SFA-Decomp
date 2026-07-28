#include "main/audio/sfx_ids.h"
#include "main/texture.h"
#include "main/pad_api.h"
#include "main/rcp_dolphin_api.h"
#include "main/model_engine.h"
#include "main/gameloop_api.h"
#include "main/audio/sfx_trigger_ids.h"
#include "ghidra_import.h"
#include "main/audio/sfx.h"
#include "main/frame_timing.h"
#include "main/textrender_api.h"
#include "main/dll/dll_0038_weirdunusedmenu.h"
#include "main/dll/savegame.h"
#include "main/dll/dll_003C_tumbleweedbush.h"

#define WIDGET_FLAG_SAVING 0x1000

#define WEIRDMENU_TEXTURE_A_ID 0x31e
#define WEIRDMENU_TEXTURE_B_ID 0x310
#define WEIRDMENU_TEXTURE_C_ID 0x31f

#define SFX_MENU_CANCEL 0x419

#define PAD_BUTTON_A 0x100
#define PAD_BUTTON_B 0x200
#define PAD_CONFIRM_MASK (PAD_BUTTON_A | PAD_BUTTON_B)


void* gWeirdMenuTextHandle[2];
Texture* gWeirdMenuTextureA;
Texture* gWeirdMenuTextureB;
Texture* gWeirdMenuTextureC;
u8 gWeirdMenuPhase;
s8 gWeirdMenuSaveTimer;
s16 gWeirdMenuScrollOffset;

WeirdMenuWork gWeirdMenuWidgetWork = {
    {0x00, 0x00, 0x00, 0xa1, 0x00, 0x56, 0x00, 0x00, 0x00, 0xa1, 0x00, 0x56,
     0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80},
    0x0000,
    {0x00, 0xff, 0x01, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x01, 0x00, 0xa1, 0x00, 0x6d, 0x00, 0x00, 0x00, 0xa1, 0x00, 0x56,
     0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80},
    0x0000,
    {0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

u32 gWeirdMenuWidgetLayout[3] = {0x000000f9, 0xffffffff, 0x00000102};

u32 lbl_8031ADA4[11] = {0x00000000, 0x00000000, 0x00000000, 0x00050000,
        (u32)WeirdUnusedMenu_initialise, (u32)WeirdUnusedMenu_release,
        0x00000000, (u32)WeirdUnusedMenu_run, (u32)WeirdUnusedMenu_frameEnd,
        (u32)WeirdUnusedMenu_render, 0x00000000};

void WeirdUnusedMenu_render(void)
{
}

void WeirdUnusedMenu_frameEnd(void)
{
}

int WeirdUnusedMenu_run(void)
{
    int selection;
    int action;

    if (gWeirdMenuPhase == 0)
    {
        action = gTitleMenuLinkInterface->vtable->update();
        selection = gTitleMenuLinkInterface->vtable->getSelected();
        if (action == 1)
        {
            if (selection == 0)
            {
                Sfx_PlayFromObject(0, SFXTRIG_dn_boar1_c_103);
                loadUiDll(1);
                cutsceneExit();
                buttonDisable(0, PAD_CONFIRM_MASK);
            }
            else
            {
                Sfx_PlayFromObject(0, SFXTRIG_dn_boar1_c_104);
                gWeirdMenuSaveTimer = 0;
                gWeirdMenuPhase = 1;
                gWeirdMenuWidgetWork.widgetFlagsA = (u16)(gWeirdMenuWidgetWork.widgetFlagsA | WIDGET_FLAG_SAVING);
                gWeirdMenuWidgetWork.widgetFlagsB = (u16)(gWeirdMenuWidgetWork.widgetFlagsB | WIDGET_FLAG_SAVING);
                gTitleMenuLinkInterface->vtable->copyItems(&gWeirdMenuWidgetWork);
            }
        }
        else if (action == 0)
        {
            Sfx_PlayFromObject(0, SFX_MENU_CANCEL);
            loadUiDll(1);
            cutsceneExit();
            buttonDisable(0, PAD_CONFIRM_MASK);
        }
    }
    else if (gWeirdMenuPhase == 1)
    {
        if (gWeirdMenuSaveTimer == 0)
        {
            saveGame_save();
        }
        if ((f32)(s8)(gWeirdMenuSaveTimer = ((f32)gWeirdMenuSaveTimer + timeDelta)) >= 120.0f)
        {
            gWeirdMenuPhase = 0;
            gWeirdMenuWidgetWork.widgetFlagsA = (u16)(gWeirdMenuWidgetWork.widgetFlagsA & ~WIDGET_FLAG_SAVING);
            gWeirdMenuWidgetWork.widgetFlagsB = (u16)(gWeirdMenuWidgetWork.widgetFlagsB & ~WIDGET_FLAG_SAVING);
            gTitleMenuLinkInterface->vtable->copyItems(&gWeirdMenuWidgetWork);
            gTitleMenuLinkInterface->vtable->setSelected(0);
        }
    }

    gWeirdMenuScrollOffset = (s16)(gWeirdMenuScrollOffset + (framesThisStep << 3));
    if (gWeirdMenuScrollOffset > 0x8c)
    {
        gWeirdMenuScrollOffset = 0x8c;
    }
    return 0;
}

void WeirdUnusedMenu_release(void)
{
    textureFree((Texture*)((u8*)gWeirdMenuTextureA));
    textureFree((Texture*)((u8*)gWeirdMenuTextureB));
    textureFree((Texture*)((u8*)gWeirdMenuTextureC));
    warpToMap(0, 1);
    gTitleMenuLinkInterface->vtable->free();
}

void WeirdUnusedMenu_initialise(void)
{
    gWeirdMenuTextureA = textureLoadAsset(WEIRDMENU_TEXTURE_A_ID);
    gWeirdMenuTextureB = textureLoadAsset(WEIRDMENU_TEXTURE_B_ID);
    gWeirdMenuTextureC = textureLoadAsset(WEIRDMENU_TEXTURE_C_ID);
    gWeirdMenuTextHandle[0] = gameTextGet(0);
    gTitleMenuLinkInterface->vtable->setup(&gWeirdMenuWidgetWork, 2, 0, gWeirdMenuWidgetLayout, 0, 0, 0x5b, 0x45,
                                          0x30, 0xff, 0xd7, 0x3d);
    gWeirdMenuScrollOffset = 0;
    gWeirdMenuPhase = 0;
}
