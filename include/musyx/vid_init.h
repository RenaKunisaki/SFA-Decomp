#ifndef MUSYX_VID_INIT_H_
#define MUSYX_VID_INIT_H_

#include "ghidra_import.h"
#include "musyx/mcmd.h"

extern McmdVidListNode vidListNodes[128];
extern u32 vidCurrentId;
extern McmdVidListNode* vidRoot;
extern McmdVidListNode* vidFree;

void vidInit(void);

#endif /* MUSYX_VID_INIT_H_ */
