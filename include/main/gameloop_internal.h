#ifndef MAIN_GAMELOOP_INTERNAL_H_
#define MAIN_GAMELOOP_INTERNAL_H_

#include "types.h"

extern int gGameLoopPendingUiDllId;
extern f32 gGameLoopMusicFadeTimer;
extern u8 gGameLoopResetComboDebounce;
extern char sGameLoopResetMessages[0x50];
extern f32 gGameLoopResetFadeOutTimer;
extern void* lbl_803DCAFC;
extern int gGameLoopPendingMapId;
extern int gGameLoopPendingMapDataFileId;
extern u8 gGameLoopPendingMusicId;
extern int gGameLoopButtonObjects[2];
extern u8 gGameLoopProgressiveMode;
extern u8* gGameBitSaveData;
extern u8* gGameBitTable;
extern s16 gGameBitCount;
extern int gGameLoopPlayerTrailIndex;
extern int gGameLoopPlayerTrailTime;
extern f32 gGameLoopResetHoldTimer;
extern u8 gGameLoopHardReset;
extern u8 lbl_803DCAC4;
extern void* lbl_803DCA94;
extern u8 gGameLoopInitComplete;
extern u8 gGameLoopButtonObjectCount;
extern s16 screenBlankFrameCount;
extern u8 gGameLoopMusicActive;
extern u16 gGameLoopMusicRequestCount;
extern u8 gGameLoopMapLoadPending;
extern u8 lbl_803DCA40;
extern u8 lbl_803DCA3F;
extern u8 shouldResetNextFrame;
extern u8 gameState;
extern u8 timeStop;
extern s8 frameCountdown;
extern s8 hudHiddenFrameCount;
extern u8 gGameLoopReloadRequested;
extern u8 lbl_803DCA38;

void gameLoop(void);
void init(void);
void doQueuedLoads(void);
void askProgressiveScanMode(void);

#endif /* MAIN_GAMELOOP_INTERNAL_H_ */
