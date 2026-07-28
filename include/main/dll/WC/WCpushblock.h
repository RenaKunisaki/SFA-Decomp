#ifndef MAIN_DLL_WC_WCPUSHBLOCK_H_
#define MAIN_DLL_WC_WCPUSHBLOCK_H_

#include "ghidra_import.h"

typedef struct WCPushBlockState WCPushBlockState;

void WCPushBlock_SpawnFromPath(GameObject *path, u8* unusedState);
void WCPushBlock_UpdateCloudAction(int obj, WCPushBlockState *state);
void WCPushBlock_UpdateRideTilt(GameObject *obj, WCPushBlockState *state);

#endif /* MAIN_DLL_WC_WCPUSHBLOCK_H_ */
