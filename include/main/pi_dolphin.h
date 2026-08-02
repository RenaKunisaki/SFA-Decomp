#ifndef MAIN_PI_DOLPHIN_H_
#define MAIN_PI_DOLPHIN_H_

#include "ghidra_import.h"
#include "dolphin/gx/GXStruct.h"
#include "main/pi_dolphin_api.h"

void piRomLoadSection(int param_1,int param_2,int param_3);

/* extern-cleanup: defining-file public prototypes */
void setDisplayCopyFilter(void);
void gxDisableGpuHangRecovery(void);
void allocSomething32bytes(void);
void initViewport(void);
void tvInit(void);
void loadModelsBin(int fileOffset, int* animCount, int* headerSize, int* amapFlag, int* dataLen, int id);
void* fileLoad(int id, int heap);
void videoInit(void* rmode, int arg);
int fileLoadToBuffer(int id, void* buffer);
u8 initLoadFiles(void);
void videoBlackScreenForFrames(int frameCount);
void checkLoadBlock(int a, int* compressedSize, int* decompressedSize);

extern void** gDvdFileInfoPool;
extern GXRenderModeObj* gRenderModeObj;
extern s32 gObjLevelLockSlots[2];


#include "main/mldf_fileid.h"

#endif /* MAIN_PI_DOLPHIN_H_ */
