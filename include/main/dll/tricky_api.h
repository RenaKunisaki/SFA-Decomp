#ifndef MAIN_DLL_TRICKY_API_H_
#define MAIN_DLL_TRICKY_API_H_

#include "global.h"

enum AButtonIcon {
    A_BUTTON_ICON_THROW_CARRYABLE = 4,
    A_BUTTON_ICON_PLACE_CARRYABLE = 5,
    A_BUTTON_ICON_HINT = 7,
    A_BUTTON_ICON_TALK_NPC = 8,
    A_BUTTON_ICON_TALK_OBJECT = 9,
    A_BUTTON_ICON_CONTEXT_B = 0x0F
};

void setAButtonIcon(int icon);
void setBButtonIcon(int icon);
void cutSceneFn_8011dd30(void);
void drawViewFinderHud(void);
void hudSetMagicCostPreview(u8 value);
void fearTestMeterSetFadeIn(u32 value);
void hudFn_8011f38c(u8 value);
void showDeathMenu(void);
void resetYbutton(void);
int getYButtonItem(s16* out);
void gameUiResetMenuState(void);
void hudFn_8011f6f0(u8 value);

#endif /* MAIN_DLL_TRICKY_API_H_ */
