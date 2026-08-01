#ifndef MAIN_DLL_BADDIE_TUMBLEWEEDBUSH_H_
#define MAIN_DLL_BADDIE_TUMBLEWEEDBUSH_H_

#include "ghidra_import.h"

u32 Link_update(void);
void setLinkNotRotated(void);
void setLinkIsRotated(void);
u16 linkGetSelectedItemId(void);
void Link_refreshOverlappingItemTimers(void);
void Link_scanItemVerticalBounds(void);

#endif /* MAIN_DLL_BADDIE_TUMBLEWEEDBUSH_H_ */
