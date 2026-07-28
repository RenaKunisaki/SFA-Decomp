#ifndef MAIN_DLL_DLL_80136A40_H_
#define MAIN_DLL_DLL_80136A40_H_

#include "game/objects/object.h"
#include "main/dll/tricky_state.h"
#include "types.h"
#include "main/debug.h"

/*
 * Public exports of dll_80136a40 (the Tricky behaviour DLL). Declared here so
 * consumers include this instead of hand-writing a local extern.
 */
void* trickyGetQueuedPathParticlePos(GameObject* obj);
f32 trickyGetAnimSpeed(GameObject* tricky);
GameObject* trickyGetStayPoint(GameObject* tricky);
int trickyGetAimPitchOffset(GameObject* tricky);
int trickyFn_80138f14(GameObject* tricky);
void trickyImpress(GameObject* obj);
void trickySetSoundSuppressed(GameObject* obj, int value);
int trickyTryPlaySound(GameObject* obj, u16 sfxId, int volume);
void debugPrintInit(void);
void debugPrintReset(void);
void debugPrintfxy(int x, int y, char* fmt, ...);
void errDisplayInstallHandlers(void);
void* errDisplayThreadMain(void* unused);
void reportAllocFail(int region0SizeKb, int region0FreeKb, int region1SizeKb, int region1FreeKb, int region2SizeKb,
                     int region2FreeKb, int memoryState, int tickCount, int requestedSize, int largestFree0,
                     int largestFree1);
void debugPrintDraw(int ctx);
void objAnimFreeChildren(GameObject* a, int b, GameObject** c);
GameObject* trickyFindNearestUsableBaddie(GameObject* origin, f32 maxRadius, int allowSpecialTypes);
void Tricky_updateBlendChannelWeight(int obj, u8* state);
void Tricky_emitQueuedPathParticles(u8* obj, u8* state);
int trickySelectQueuedCommandTarget(TrickyState* state, int commandType);
void tricky_updateModelVariantFade(int obj, int state);

#endif
