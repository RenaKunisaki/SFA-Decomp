#ifndef MUSYX_VID_INIT_H_
#define MUSYX_VID_INIT_H_

#include "ghidra_import.h"
#include "musyx/mcmd.h"

extern VID_LIST vidList[128];
extern u32 vidCurrentId;
extern VID_LIST* vidRoot;
extern VID_LIST* vidFree;

void vidInit(void);

#endif /* MUSYX_VID_INIT_H_ */
