#include "main/audio/sfx_ids.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/audio/audio_control_api.h"
#include "main/audio/sfx.h"
#include "main/dll/dll_4D.h"
#include "main/dll/dll_4E.h"
#include "main/dll/dll_02C0_front.h"
#include "main/dll/debug/prof.h"
#include "main/dll/dll_0015_save_settings.h"
#include "main/dll/dll_0037_optionsscreen.h"
#include "main/lightmap_api.h"
#include "main/pad.h"
#include "main/screen_transition.h"
#include "main/rcp_dolphin_render_api.h"
#include "main/gameloop_api.h"
#include "main/dll/dll_003C_tumbleweedbush.h"
#include "main/dll/dll_003D_titlemenuitem.h"
#include "main/dll/dll_0004_dummy04.h"
#include "main/dll/savedata_struct.h"
#include "main/textrender_api.h"
#include "dolphin/os/OSRtc.h"
#include "main/rcp_dolphin_state_api.h"
#include "main/model_engine.h"
#include "main/dll/dll_02C0_front_api.h"
#include "main/dll/front_game_text_box_api.h"
#include "main/frame_timing.h"
#include "main/gametext_box_api.h"
#include "main/gametext_show_api.h"
#include "main/lightmap_render_control_api.h"
#include "main/dll/FRONT/title_menu.h"
#include "main/dll/dll_0015_curves.h"
#include "main/gametext_color_api.h"

typedef struct OptionsScreenPanelConfig {
    u16* items;
    u32 reserved;
    u16 itemLayout;
    u16 selectionTextId;
    u16 headingTextId;
    u16 padding;
} OptionsScreenPanelConfig;

extern OptionsScreenPanelConfig lbl_8031ACB8[4];

/*
 * dll_4e - options-menu setting callbacks (audio panel, gameplay panel,
 * submenu selector).
 *
 * Each callback is driven by the title-menu item widgets in
 * lbl_803A87D0[]: the widget at the option index (the menu row) is
 * queried through the gTitleMenuItemInterface vtable - slot 0x2c tests
 * whether the value changed, slot 0x24 reads the current value, slot
 * 0x28 sets a value, slot 0x10 frees the widget. The action arg selects
 * the menu action (CLOSE vs SELECT); CLOSE plays a back sfx and kicks
 * the screen transition into the next menu state.
 *
 * - applyAudioSetting: sound mode, music/sfx/voice volume, reset to
 *   defaults (reloads the saved volumes from lbl_803DD708).
 * - applyGameplaySetting: widescreen, rumble, roll credits, colour
 *   filter.
 * - openSelectedSubmenu: general / audio / language panels.
 */

#define OPTIONS_MENU_ACTION_CLOSE      0
#define OPTIONS_MENU_ACTION_SELECT     1
#define OPTIONS_MENU_TRANSITION_FRAMES 0x14
#define OPTIONS_MENU_TRANSITION_MODE   5
#define OPTIONS_MENU_NEXT_STATE        0x23
#define OPTIONS_MENU_VOLUME_STEP       10
#define OPTIONS_MENU_ITEM_COUNT        8

#define OPTIONS_SFX_VOLUME_PREVIEW 0x3b9
#define OPTIONS_SFX_CONFIRM        0x418

#define AUDIO_OPTION_SOUND_MODE     0
#define AUDIO_OPTION_MUSIC_VOLUME   1
#define AUDIO_OPTION_SFX_VOLUME     2
#define AUDIO_OPTION_VOICE_VOLUME   3
#define AUDIO_OPTION_RESET_DEFAULTS 4
#define AUDIO_OPTION_EXTRA          5

#define GAMEPLAY_OPTION_WIDESCREEN   0
#define GAMEPLAY_OPTION_RUMBLE       1
#define GAMEPLAY_OPTION_CREDITS      2
#define GAMEPLAY_OPTION_COLOR_FILTER 3

#define OPTIONS_SUBMENU_GENERAL  0
#define OPTIONS_SUBMENU_AUDIO    2
#define OPTIONS_SUBMENU_LANGUAGE 3

extern s8 lbl_803DBA28;
extern u8 lbl_803DD6F8;
extern u8 lbl_803DD6F9;
extern int lbl_803DD6FC;
extern int lbl_803DD700;
extern s8 lbl_803DD704;
extern s8 lbl_803DD705;
extern s8 lbl_803DD706;
extern s8 lbl_803DD70C;

void optionsMenu_applyAudioSetting(int action, int option)
{
    int value;

    if (lbl_803A87D0[option] != NULL && gTitleMenuItemInterface->vtable->isChanged(lbl_803A87D0[option]) != 0)
    {
        switch (option)
        {
        case AUDIO_OPTION_SOUND_MODE:
            audioSetSoundMode((u8)gTitleMenuItemInterface->vtable->getValue(lbl_803A87D0[option]), 1);
            break;
        case AUDIO_OPTION_SFX_VOLUME:
            value = gTitleMenuItemInterface->vtable->getValue(lbl_803A87D0[option]);
            audioSetVolumes((u8)value, OPTIONS_MENU_VOLUME_STEP, 0, 1, 0);
            break;
        case AUDIO_OPTION_MUSIC_VOLUME:
            value = gTitleMenuItemInterface->vtable->getValue(lbl_803A87D0[option]);
            audioSetVolumes((u8)value, OPTIONS_MENU_VOLUME_STEP, 1, 0, 0);
            value = gTitleMenuItemInterface->vtable->getValue(lbl_803A87D0[option]);
            gTitleMenuControlInterface->vtable->func0D(value); /* set music control value */
            break;
        case AUDIO_OPTION_VOICE_VOLUME:
            value = gTitleMenuItemInterface->vtable->getValue(lbl_803A87D0[option]);
            audioSetVolumes((u8)value, OPTIONS_MENU_VOLUME_STEP, 0, 0, 1);
            break;
        case AUDIO_OPTION_EXTRA:
            lbl_803DD6FC = gTitleMenuItemInterface->vtable->getValue(lbl_803A87D0[option]);
            break;
        }
    }
    if ((lbl_803A87D0[option] == NULL) ||
        ((option != AUDIO_OPTION_SFX_VOLUME) && (option != AUDIO_OPTION_MUSIC_VOLUME) &&
         (option != AUDIO_OPTION_VOICE_VOLUME)))
    {
        Sfx_StopFromObject(0, OPTIONS_SFX_VOLUME_PREVIEW);
    }
    if (action == OPTIONS_MENU_ACTION_CLOSE)
    {
        Sfx_PlayFromObject(0, SFXTRIG_wmap_name);
        (*gScreenTransitionInterface)->start(OPTIONS_MENU_TRANSITION_FRAMES, OPTIONS_MENU_TRANSITION_MODE);
        lbl_803DD704 = OPTIONS_MENU_NEXT_STATE;
        lbl_803DD705 = 1;
    }
    else if ((action == OPTIONS_MENU_ACTION_SELECT) && (option == AUDIO_OPTION_RESET_DEFAULTS))
    {
        saveFileStruct_resetVolumes();
        gTitleMenuItemInterface->vtable->setValue(lbl_803A87D0[AUDIO_OPTION_MUSIC_VOLUME],
                                                   lbl_803DD708->musicVolume);
        gTitleMenuItemInterface->vtable->setValue(lbl_803A87D0[AUDIO_OPTION_SFX_VOLUME], lbl_803DD708->sfxVolume);
        gTitleMenuItemInterface->vtable->setValue(lbl_803A87D0[AUDIO_OPTION_VOICE_VOLUME],
                                                   lbl_803DD708->speechVolume);
        value = gTitleMenuItemInterface->vtable->getValue(lbl_803A87D0[AUDIO_OPTION_MUSIC_VOLUME]);
        audioSetVolumes((u8)value, OPTIONS_MENU_VOLUME_STEP, 0, 1, 0);
        value = gTitleMenuItemInterface->vtable->getValue(lbl_803A87D0[AUDIO_OPTION_SFX_VOLUME]);
        audioSetVolumes((u8)value, OPTIONS_MENU_VOLUME_STEP, 1, 0, 0);
        value = gTitleMenuItemInterface->vtable->getValue(lbl_803A87D0[AUDIO_OPTION_VOICE_VOLUME]);
        audioSetVolumes((u8)value, OPTIONS_MENU_VOLUME_STEP, 0, 0, 1);
        Sfx_PlayFromObject(0, OPTIONS_SFX_CONFIRM);
    }
}

void optionsMenu_applyGameplaySetting(int action, int option)
{
    int z[2];
    u8 newState;

    if (lbl_803A87D0[option] != NULL && gTitleMenuItemInterface->vtable->isChanged(lbl_803A87D0[option]) != 0)
    {
        switch (option)
        {
        case GAMEPLAY_OPTION_WIDESCREEN:
            setWidescreen((u8)gTitleMenuItemInterface->vtable->getValue(lbl_803A87D0[option]));
            break;
        case GAMEPLAY_OPTION_RUMBLE:
            newState = !gTitleMenuItemInterface->vtable->getValue(lbl_803A87D0[option]);
            if (newState == 0)
            {
                stopRumble2();
            }
            setRumbleEnabled(newState);
            if (newState != 0)
            {
                doRumble(20.0f);
            }
            break;
        case GAMEPLAY_OPTION_CREDITS:
            if (gTitleMenuItemInterface->vtable->getValue(lbl_803A87D0[option]) == 0)
            {
                creditsStart();
                if (lbl_803DBA28 != -1)
                {
                    gTitleMenuLinkInterface->vtable->free();
                    lbl_803DBA28 = -1;
                }
                z[0] = 0;
                z[1] = z[0];
                for (; z[0] < OPTIONS_MENU_ITEM_COUNT; z[0]++)
                {
                    if (lbl_803A87D0[z[0]] != NULL)
                    {
                        gTitleMenuItemInterface->vtable->free(lbl_803A87D0[z[0]]);
                        lbl_803A87D0[z[0]] = (TitleMenuItem*)z[1];
                    }
                }
            }
            break;
        case GAMEPLAY_OPTION_COLOR_FILTER:
            Rcp_SetColorFilterEnabled(gTitleMenuItemInterface->vtable->getValue(lbl_803A87D0[option]));
            break;
        }
    }
    if (action == OPTIONS_MENU_ACTION_CLOSE)
    {
        Sfx_PlayFromObject(0, SFXTRIG_wmap_name);
        (*gScreenTransitionInterface)->start(OPTIONS_MENU_TRANSITION_FRAMES, OPTIONS_MENU_TRANSITION_MODE);
        lbl_803DD704 = OPTIONS_MENU_NEXT_STATE;
        lbl_803DD705 = 1;
    }
}

int optionsMenu_openSelectedSubmenu(int action, int option)
{
    if (action == OPTIONS_MENU_ACTION_SELECT)
    {
        switch (option)
        {
        case OPTIONS_SUBMENU_GENERAL:
            optionsMenu_openGeneralPanel();
            return 1;
        case OPTIONS_SUBMENU_AUDIO:
            optionsMenu_openAudioPanel();
            return 1;
        case OPTIONS_SUBMENU_LANGUAGE:
            languageMenuInit();
            return 1;
        }
    }
    else if (action == OPTIONS_MENU_ACTION_CLOSE)
    {
        Sfx_PlayFromObject(0, SFXTRIG_wmap_name);
        (*gScreenTransitionInterface)->start(OPTIONS_MENU_TRANSITION_FRAMES, OPTIONS_MENU_TRANSITION_MODE);
        lbl_803DD704 = OPTIONS_MENU_NEXT_STATE;
        lbl_803DD705 = 1;
    }
    return 0;
}

/*
 * dll_4d - language/misc front-end menu setup (UI DLL 0x4D).
 *
 * languageMenuInit() builds the "misc" sub-panel (lbl_8031ACB8) of the
 * options front-end: it tears down any previously-active panel, marks
 * panel 3 (misc) active (lbl_803DBA28), and creates the language menu
 * row through the title-menu item interface. When cheat 3 is unlocked
 * and not already shown (lbl_803DC968), it links in and creates a second
 * row reflecting that cheat's active state; otherwise that row is hidden.
 * The created rows are focused through the title-menu item interface,
 * laid out through the title-menu link interface, and the panel's
 * render-stale countdown (lbl_803DD706) is reset so the new layout draws.
 */

/* misc-panel id stored in lbl_803DBA28 (see dll_0037_optionsscreen.c) */
#define OPTIONS_PANEL_MISC 3

/* the in-game cheat queried for the second menu row */
#define LANGUAGE_MENU_CHEAT_ID CHEAT_DINO_LANGUAGE

/* TitleMenuTextEntry.flags: row is hidden / non-selectable */
#define TITLE_MENU_TEXT_ENTRY_HIDDEN 0x4000

/* lbl_803DBA28 active-panel id and lbl_803DD706 render-stale countdown
   are owned by dll_0037_optionsscreen.c */
extern s8 lbl_803DBA28;
extern s8 lbl_803DD706;

void languageMenuInit(void)
{
    MenuPanelGroup* panel;

    if ((s8)lbl_803DBA28 != -1)
    {
        gTitleMenuLinkInterface->vtable->free();
    }
    lbl_803DBA28 = OPTIONS_PANEL_MISC;

    panel = (MenuPanelGroup*)lbl_8031ACB8;
    lbl_803A87D0[0] =
        gTitleMenuItemInterface->vtable->createWithWindow(0x36b, 0x22, 0, 1,
                                                         (s16)(lbl_803DD708->subtitlesEnabled == 0));

    if (isCheatUnlocked(LANGUAGE_MENU_CHEAT_ID) != 0 && lbl_803DC968 == 0)
    {
        panel->entries[panel->count - 2].pad18[3] = panel->count - 1;
        panel->entries[panel->count - 1].flags &= ~TITLE_MENU_TEXT_ENTRY_HIDDEN;

        lbl_803A87D0[1] = gTitleMenuItemInterface->vtable->createWithWindow(
            0x36b, 0x23, 0, 1, (s16)(saveFileStruct_isCheatActive(LANGUAGE_MENU_CHEAT_ID) == 0));
    }
    else
    {
        panel->entries[panel->count - 2].pad18[3] = -1;
        panel->entries[panel->count - 1].flags |= TITLE_MENU_TEXT_ENTRY_HIDDEN;
    }

    gTitleMenuItemInterface->vtable->setEnabled(lbl_803A87D0[0], 1);

    gTitleMenuLinkInterface->vtable->setup(panel->entries, panel->count, 0, NULL, 0, 0, 0x14, 0xc8, 0xff, 0xff, 0xff,
                                           0xff);

    lbl_803DD706 = 2;
}

/*
 * prof - title-screen Options menu panel builders.
 *
 * Two entry points populate the Options sub-menus through the title-menu
 * link/item interfaces (gTitleMenuLink/Item). openAudioPanel builds the
 * Audio panel (surround/stereo/mono toggle, music/sfx/voice sliders, and
 * a cheat-gated extra entry); openGeneralPanel builds the General panel,
 * unlocking option/cheat entries based on isCheatUnlocked() and toggling
 * the per-entry "disabled" flag (0x4000) accordingly.
 *
 * lbl_803DBA28 tracks which panel is currently open (-1 = none); a switch
 * away first tears down the previous link (slot +8). Built item handles
 * are cached in lbl_803A87D0[]. lbl_803DD706 is set to 2 by both builders;
 * its exact role is unconfirmed.
 */
extern s8 lbl_803DBA28;
extern s8 lbl_803DD706;

typedef struct OptionsMenuPanels
{
    u8 pad00[0x10];
    s8* audioEntries;
    u32 unk_14;
    u8 audioCount;
    u8 pad19[0x20 - 0x19];
    s8* optionEntries;
    u32 unk_24;
    u8 optionCount;
} OptionsMenuPanels;

/* per-entry flag word (entry+0x16): set to grey-out / disable an entry */
#define OPTION_ENTRY_DISABLED 0x4000

void optionsMenu_openAudioPanel(void)
{
    OptionsMenuPanels* panels;
    TitleMenuItem* item;

    if (lbl_803DBA28 != -1)
    {
        gTitleMenuLinkInterface->vtable->free();
    }
    lbl_803DBA28 = 1;
    panels = (OptionsMenuPanels*)lbl_8031ACB8;

    if (isCheatUnlocked(2) != 0)
    {
        panels->audioEntries[0x10b] = 5;
        *(u16*)(panels->audioEntries + 0x142) = (u16)(*(u16*)(panels->audioEntries + 0x142) & ~OPTION_ENTRY_DISABLED);
        panels->audioEntries[0x146] = 4;
    }
    else
    {
        panels->audioEntries[0x10b] = -1;
        *(u16*)(panels->audioEntries + 0x142) = (u16)(*(u16*)(panels->audioEntries + 0x142) | OPTION_ENTRY_DISABLED);
    }

    gTitleMenuLinkInterface->vtable->setup(panels->audioEntries, panels->audioCount, 0, NULL, 0, 0, 0x14, 0xc8, 0xff,
                                           0xff, 0xff, 0xff);

    if (OSGetSoundMode() == 1)
    {
        item = gTitleMenuItemInterface->vtable->createWithWindow(0x36c, 0x22, 0, 3, lbl_803DD708->soundMode);
    }
    else
    {
        item = gTitleMenuItemInterface->vtable->createWithWindow(0x36c, 0x22, 0, 3, 2);
    }
    lbl_803A87D0[0] = item;
    lbl_803A87D0[1] =
        gTitleMenuItemInterface->vtable->createWithText(0x124, 0xb2, 0, 0x7f, lbl_803DD708->musicVolume, 0x3e);
    lbl_803A87D0[2] =
        gTitleMenuItemInterface->vtable->createWithText(0x124, 0xcc, 0, 0x7f, lbl_803DD708->sfxVolume, 0x3e);
    lbl_803A87D0[3] =
        gTitleMenuItemInterface->vtable->createWithText(0x124, 0xe6, 0, 0x7f, lbl_803DD708->speechVolume, 0x3e);
    lbl_803A87D0[3]->flags = (u8)(lbl_803A87D0[3]->flags | 0x40);
    lbl_803A87D0[4] = NULL;
    lbl_803A87D0[5] = NULL;

    if (isCheatUnlocked(2) != 0)
    {
        lbl_803A87D0[5] = gTitleMenuItemInterface->vtable->createWithWindow(
            0x3cb, 0x27, 0, (s16)(return0x64_8000A378() - 1), 0);
        lbl_803A87D0[5]->flags = (u8)(lbl_803A87D0[5]->flags | 0x80);
    }

    gTitleMenuItemInterface->vtable->setEnabled(lbl_803A87D0[0], 1);
    lbl_803DD706 = 2;
}

void optionsMenu_openGeneralPanel(void)
{
    OptionsMenuPanels* panels;
    int lastUnlocked;
    int entryOffset;
    int cheatId;
    TitleMenuItem** slot[1];
    int cheatId2;
    int entryOffset2;
    int lastUnlocked2;

    if (lbl_803DBA28 != -1)
    {
        gTitleMenuLinkInterface->vtable->free();
    }
    lbl_803DBA28 = 2;
    panels = (OptionsMenuPanels*)lbl_8031ACB8;

    lastUnlocked = -1;
    cheatId = 3;
    entryOffset = 0xb4;
    do
    {
        if (isCheatUnlocked((u8)(cheatId - 2)) != 0)
        {
            panels->optionEntries[entryOffset - 0x21] = cheatId;
            *(u16*)(panels->optionEntries + entryOffset + 0x16) &= ~OPTION_ENTRY_DISABLED;
            lastUnlocked = cheatId;
        }
        else
        {
            panels->optionEntries[entryOffset - 0x21] = lastUnlocked;
            *(u16*)(panels->optionEntries + entryOffset + 0x16) |= OPTION_ENTRY_DISABLED;
        }
        entryOffset -= 0x3c;
        cheatId--;
    } while (cheatId > 1);

    lastUnlocked2 = 1;
    cheatId2 = 2;
    entryOffset2 = 0x78;
    do
    {
        if (isCheatUnlocked((u8)(cheatId2 - 2)) != 0)
        {
            panels->optionEntries[entryOffset2 + 0x1a] = lastUnlocked2;
            *(u16*)(panels->optionEntries + entryOffset2 + 0x16) &= ~OPTION_ENTRY_DISABLED;
            lastUnlocked2 = cheatId2;
        }
        entryOffset2 += 0x3c;
        cheatId2++;
    } while (cheatId2 < 4);

    gTitleMenuLinkInterface->vtable->setup(panels->optionEntries, panels->optionCount, 0, NULL, 0, 0, 0x14, 0xc8, 0xff,
                                           0xff, 0xff, 0xff);

    lbl_803A87D0[0] =
        gTitleMenuItemInterface->vtable->createWithWindow(0x366, 0x22, 0, 1, lbl_803DD708->widescreenEnabled);
    lbl_803A87D0[1] =
        gTitleMenuItemInterface->vtable->createWithWindow(0x36b, 0x23, 0, 1,
                                                         (s16)(lbl_803DD708->rumbleEnabled == 0));
    slot[0] = lbl_803A87D0;

    cheatId = 0;
    do
    {
        if (isCheatUnlocked((u8)cheatId) != 0)
        {
            if (cheatId == CHEAT_SEPIA_MODE)
            {
                slot[0][2] = gTitleMenuItemInterface->vtable->createWithWindow(
                    0x507, cheatId + 0x24, 0, 1, Rcp_GetColorFilterEnabled());
            }
            else
            {
                slot[0][2] = gTitleMenuItemInterface->vtable->createWithWindow(
                    0x36b, cheatId + 0x24, 0, 1, (s16)(saveFileStruct_isCheatActive((u8)cheatId) == 0));
            }
        }
        slot[0]++;
        cheatId++;
    } while (cheatId <= 1);

    gTitleMenuItemInterface->vtable->setEnabled(lbl_803A87D0[0], 1);
    lbl_803DD706 = 2;
}

#define OPTIONSSCREEN_MENU_ITEM_COUNT 8

#define OPTIONSSCREEN_PANEL_NONE     (-1)
#define OPTIONSSCREEN_PANEL_TOP      0
#define OPTIONSSCREEN_PANEL_AUDIO    1
#define OPTIONSSCREEN_PANEL_GAMEPLAY 2
#define OPTIONSSCREEN_PANEL_MISC     3

s8 lbl_803DBA28 = -1;
u8 lbl_803DD6F8;
u8 lbl_803DD6F9;
int lbl_803DD6FC;
int lbl_803DD700;
s8 lbl_803DD704;
s8 lbl_803DD705;
s8 lbl_803DD706;
SaveData* lbl_803DD708;
s8 lbl_803DD70C;

u16 lbl_8031A8F8[90] = {
    0x035a, 0x0012, 0x0140, 0x00a4, 0x0000, 0x0140, 0x0034, 0x0000, 0xffff, 0xffff, 0x00c8, 0x0200, 0x0000, 0xff01, 0xffff, 0xff00,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x035c, 0x0013,
    0x0140, 0x0110, 0x0000, 0x0140, 0x00a0, 0x0000, 0xffff, 0xffff, 0x00c8, 0x0200, 0x0000, 0x01ff, 0xffff, 0xff00, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x035b, 0x0014, 0x0140, 0x00da,
    0x0000, 0x0140, 0x006a, 0x0000, 0xffff, 0xffff, 0x00c8, 0x0200, 0x0000, 0x0002, 0xffff, 0xff00, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};

u16 lbl_8031A9AC[120] = {
    0x035e, 0x0017, 0x005a, 0x00cb, 0x0000, 0x005a, 0x0116, 0x0000, 0xffff, 0xffff, 0x0000, 0x0021, 0x0000, 0xff01, 0xffff, 0xff00,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0360, 0x0018,
    0x005a, 0x0119, 0x0000, 0x005a, 0x0146, 0x0000, 0xffff, 0xffff, 0x0000, 0x0021, 0x0000, 0x0002, 0xffff, 0xff00, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0500, 0x0019, 0x005a, 0x0119,
    0x0000, 0x005a, 0x0146, 0x0000, 0xffff, 0xffff, 0x0000, 0x0021, 0x0000, 0x0103, 0xffff, 0xff00, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0502, 0x001a, 0x005a, 0x0119, 0x0000, 0x005a,
    0x0146, 0x0000, 0xffff, 0xffff, 0x0000, 0x0021, 0x0000, 0x02ff, 0xffff, 0xff00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};

u16 lbl_8031AA9C[180] = {
    0x0361, 0x0017, 0x005a, 0x00b1, 0x0000, 0x005a, 0x00fe, 0x0000, 0xffff, 0xffff, 0x0000, 0x0021, 0x0000, 0xff01, 0xffff, 0xff00,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0362, 0x0018,
    0x005a, 0x00e5, 0x0000, 0x005a, 0x0116, 0x0000, 0xffff, 0xffff, 0x0000, 0x0021, 0x0000, 0x0002, 0xffff, 0xff00, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0363, 0x0019, 0x005a, 0x00ff,
    0x0000, 0x005a, 0x012e, 0x0000, 0xffff, 0xffff, 0x0000, 0x0021, 0x0000, 0x0103, 0xffff, 0xff00, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x03a8, 0x001a, 0x005a, 0x00ff, 0x0000, 0x005a,
    0x012e, 0x0000, 0xffff, 0xffff, 0x0000, 0x0021, 0x0000, 0x0204, 0xffff, 0xff00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0364, 0x001b, 0x005a, 0x00ff, 0x0000, 0x005a, 0x012e, 0x0000,
    0xffff, 0xffff, 0x0000, 0x0001, 0x0000, 0x0305, 0xffff, 0xff00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x03ca, 0x001c, 0x005a, 0x00ff, 0x0000, 0x005a, 0x012e, 0x0000, 0xffff, 0xffff,
    0x0000, 0x0001, 0x0000, 0x04ff, 0xffff, 0xff00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000};

u16 lbl_8031AC04[90] = {
    0x035f, 0x0017, 0x0140, 0x0170, 0x0000, 0x0140, 0x00bb, 0x0000, 0xffff, 0xffff, 0x00b4, 0x0000, 0x0000, 0xff01, 0xffff, 0xff00,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0506, 0x0018,
    0x0140, 0x0170, 0x0000, 0x0140, 0x00bb, 0x0000, 0xffff, 0xffff, 0x00b4, 0x0000, 0x0000, 0x00ff, 0xffff, 0xff00, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0369, 0x0002, 0x0109, 0x0188,
    0x0000, 0x0109, 0x017c, 0x0000, 0xffff, 0xffff, 0x0000, 0x0000, 0x0000, 0xffff, 0xffff, 0xff00, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};

OptionsScreenPanelConfig lbl_8031ACB8[4] = {
    {lbl_8031A8F8, 0, 0x0304, 0x0330, 0x0367, 0},
    {lbl_8031AA9C, 0, 0x0603, 0x035c, 0x0368, 0},
    {lbl_8031A9AC, 0, 0x0403, 0x035a, 0x0368, 0},
    {lbl_8031AC04, 0, 0x0203, 0x035b, 0x0368, 0},
};

u32 lbl_8031ACF8[10] = {0x00000000, 0x00000000, 0x00000000, 0x00050000,
        (u32)OptionsScreen_initialise, (u32)OptionsScreen_release,
        0x00000000, (u32)OptionsScreen_frameStart, (u32)OptionsScreen_frameEnd,
        (u32)OptionsScreen_render};


void OptionsScreen_render(int arg)
{
    int alpha;
    int fade;
    TitleMenuItem** item;
    int i;
    u16* panel = (u16*)lbl_8031ACB8 + lbl_803DBA28 * 8;

    if (shouldShowCredits() != 0)
    {
        creditsStart_();
        return;
    }

    alpha = (int)(255.0f - (*gScreenTransitionInterface)->getProgress());
    gameTextSetDrawFunc(titleScreenTextDrawFunc);

    if ((u8)alpha < 0x80)
    {
        f32 conv = (f32)((u8)alpha * 0x86);
        titleScreenPositionElements(40.0f, 254.0f - conv * 0.0078125f);
        fade = 0;
    }
    else
    {
        titleScreenPositionElements(40.0f, 120.0f);
        fade = ((int)((u8)alpha & 0x7f) << 1) & 0xff;
    }

    gameTextBoxFn_80134d40(fade, 0, 0);
    if (panel[5] != 0xffff)
    {
        gameTextSetColor(0xff, 0xff, 0xff, 0xff);
        *(u8*)((char*)gameTextGetBox(*(u8*)((char*)gameTextGet(panel[5]) + 4)) + 0x1e) = fade;
        gameTextShow(panel[5]);
    }
    if (panel[6] != 0xffff)
    {
        gameTextSetColor(0xff, 0xff, 0xff, fade);
        gameTextShow(panel[6]);
    }

    item = lbl_803A87D0;
    for (i = 0; i < OPTIONSSCREEN_MENU_ITEM_COUNT; i++)
    {
        if (item[i] != NULL)
        {
            gTitleMenuItemInterface->vtable->render(item[i], arg, fade);
        }
    }
    gTitleMenuLinkInterface->vtable->setOpacity(fade);
    gTitleMenuLinkInterface->vtable->render(arg);
    gameTextSetDrawFunc(0);
    titleScreenShowCopyright(0);
    if ((s8)--lbl_803DD706 < 0)
    {
        lbl_803DD706 = 0;
    }
}

void OptionsScreen_frameEnd(void)
{
}

static inline void optionsScreenFreeMenuItems(void)
{
    int i;

    for (i = 0; i < OPTIONSSCREEN_MENU_ITEM_COUNT; i++)
    {
        if (lbl_803A87D0[i] != NULL)
        {
            gTitleMenuItemInterface->vtable->free(lbl_803A87D0[i]);
            lbl_803A87D0[i] = NULL;
        }
    }
}

int OptionsScreen_frameStart(void)
{
    int step = framesThisStep;
    s8 oldFade = lbl_803DD704;
    int selection;
    int item;
    int i;

    if (shouldShowCredits())
    {
        return 0;
    }
    if (step > 3)
    {
        step = 3;
    }
    if (lbl_803DD704 > 0)
    {
        lbl_803DD704 = (s8)(lbl_803DD704 - step);
    }
    if ((*gScreenTransitionInterface)->isFinished() == 0)
    {
        gTitleMenuLinkInterface->vtable->resetTimers();
        lbl_803DD706 = 2;
    }

    if (lbl_803DD705 != 0)
    {
        if ((oldFade <= 0xc || lbl_803DD704 > 0xc) && lbl_803DD704 <= 0)
        {
            if ((s8)lbl_803DBA28 != OPTIONSSCREEN_PANEL_NONE)
            {
                gTitleMenuLinkInterface->vtable->free();
                lbl_803DBA28 = OPTIONSSCREEN_PANEL_NONE;
            }
            optionsScreenFreeMenuItems();
            titleScreenFn_8005cdd4(1);
            setDrawCloudsAndLights(1);
            loadUiDll(4);
        }
        return lbl_803DD704 <= 12;
    }

    selection = gTitleMenuLinkInterface->vtable->update();
    item = gTitleMenuLinkInterface->vtable->getSelected();
    if (item != lbl_803DD700)
    {
        Sfx_PlayFromObject(0, SFXTRIG_warningloop);
    }
    lbl_803DD700 = item;

    switch ((s8)lbl_803DBA28)
    {
    case OPTIONSSCREEN_PANEL_TOP:
        lbl_803DD70C = item;
        if (optionsMenu_openSelectedSubmenu(selection, item) != 0)
        {
            return 0;
        }
        break;
    case OPTIONSSCREEN_PANEL_GAMEPLAY:
        optionsMenu_applyGameplaySetting(selection, item);
        if (selection == 0)
        {
            lbl_803DD708->widescreenEnabled = gTitleMenuItemInterface->vtable->getValue(lbl_803A87D0[0]);
            lbl_803DD708->rumbleEnabled = !gTitleMenuItemInterface->vtable->getValue(lbl_803A87D0[1]);
            setWidescreen(lbl_803DD708->widescreenEnabled);
            setRumbleEnabled(lbl_803DD708->rumbleEnabled);
        }
        break;
    case OPTIONSSCREEN_PANEL_AUDIO:
        optionsMenu_applyAudioSetting(selection, item);
        if (selection == 0)
        {
            lbl_803DD708->soundMode = gTitleMenuItemInterface->vtable->getValue(lbl_803A87D0[0]);
            lbl_803DD708->musicVolume = gTitleMenuItemInterface->vtable->getValue(lbl_803A87D0[1]);
            lbl_803DD708->sfxVolume = gTitleMenuItemInterface->vtable->getValue(lbl_803A87D0[2]);
            lbl_803DD708->speechVolume = gTitleMenuItemInterface->vtable->getValue(lbl_803A87D0[3]);
        }
        break;
    case OPTIONSSCREEN_PANEL_MISC:
        if (selection == 0)
        {
            Sfx_PlayFromObject(0, SFXTRIG_wmap_name);
            (*gScreenTransitionInterface)->start(0x14, 5);
            lbl_803DD704 = 0x23;
            lbl_803DD705 = 1;
        }
        if (lbl_803A87D0[item] != NULL && gTitleMenuItemInterface->vtable->isChanged(lbl_803A87D0[item]) != 0)
        {
            switch (item)
            {
            case 0:
                lbl_803DD708->subtitlesEnabled = !gTitleMenuItemInterface->vtable->getValue(lbl_803A87D0[0]);
                setSubtitlesEnabled(lbl_803DD708->subtitlesEnabled);
                break;
            default:
                saveFileStruct_setCheatActive(CHEAT_DINO_LANGUAGE,
                                              !gTitleMenuItemInterface->vtable->getValue(lbl_803A87D0[item]));
                break;
            }
        }
        break;
    }

    if ((s8)lbl_803DBA28 != OPTIONSSCREEN_PANEL_TOP)
    {
        for (i = 0; i < OPTIONSSCREEN_MENU_ITEM_COUNT; i++)
        {
            if (lbl_803A87D0[i] != NULL)
            {
                if (i == item)
                {
                    gTitleMenuItemInterface->vtable->setEnabled(lbl_803A87D0[i], 1);
                }
                else
                {
                    gTitleMenuItemInterface->vtable->setEnabled(lbl_803A87D0[i], 0);
                }
                gTitleMenuItemInterface->vtable->update(lbl_803A87D0[i]);
            }
        }
    }
    return 0;
}

void OptionsScreen_release(void)
{
}

void OptionsScreen_initialise(void)
{
    (*gScreenTransitionInterface)->step(20, 5);
    gameTextLoadDir(21);
    lbl_803DD70C = 0;
    lbl_803DD708 = getSaveFileStruct();
    if (lbl_803DD6F8 == 0)
    {
        optionsMenu_openGeneralPanel();
    }
    else if (lbl_803DD6F8 == 1)
    {
        optionsMenu_openAudioPanel();
    }
    else
    {
        languageMenuInit();
    }
    lbl_803DD706 = 2;
    lbl_803DD705 = 0;
    lbl_803DD6F9 = 0;
}
