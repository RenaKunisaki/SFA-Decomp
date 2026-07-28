#ifndef MAIN_GAMETEXT_SHARED_INTERNAL_H_
#define MAIN_GAMETEXT_SHARED_INTERNAL_H_

#include "types.h"

struct TextDisplayState;

extern char* lbl_803DC9C4;
extern int gGameTextBoundsMaxX;
extern int gGameTextBoundsMinX;
extern int gGameTextBoundsMaxY;
extern int gGameTextBoundsMinY;
extern int gGameTextMeasureOnly;
extern u8 gGameTextColorA;
extern u8 gGameTextColorB;
extern u8 gGameTextColorG;
extern u8 gGameTextColorR;
extern int lbl_803DC9C0;
extern u16 lbl_803DC9AA;
extern u16 lbl_803DC9A8;
extern void* gGameTextStringStore;
extern f32 lbl_803DE704;
extern f32 lbl_803DE708;
extern int lbl_803DC984;
extern f32 gGameTextScale;
extern u8 lbl_803DC990;
extern u8 lbl_803DC991;
extern u8 lbl_803DC992;
extern u8 sGameTextFallbackDefs[];
extern f32 gGameTextRevealProgress;
extern int gGameTextDrawnCharIndex;
extern int gGameTextRevealActive;
extern f32 lbl_803DE700;
extern f32 gGameTextRevealSpeed;

int utf8GetNextChar(u8* str, int* outLen);
char* gameStrcpy(char* dst, char* src);
void* gameTextGetCurBox(void);
void gameTextMeasureStringBounds(char* str, int boxIdx, int* outMinX, int* outMaxX, int* outMinY, int* outMaxY);
void gameTextRenderStrs(char* str, int boxIdx);
void gameTextTickReveal(int textId, struct TextDisplayState* state);
void gameTextRenderById(int textId, int x, int y);

#endif /* MAIN_GAMETEXT_SHARED_INTERNAL_H_ */
