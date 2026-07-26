#ifndef MAIN_DLL_DLL_01FB_DLL1FB_H_
#define MAIN_DLL_DLL_01FB_DLL1FB_H_

#include "types.h"
#include "game/objects/object.h"

int dll_1FB_getExtraSize_ret_12(void);
int dll_1FB_getObjectTypeId(void);
void dll_1FB_free_nop(void);
void dll_1FB_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible);
void dll_1FB_hitDetect_nop(void);
void dll_1FB_update(GameObject* obj);
void dll_1FB_init(GameObject* obj, u8* def);
void dll_1FB_release_nop(void);
void dll_1FB_initialise_nop(void);

#endif /* MAIN_DLL_DLL_01FB_DLL1FB_H_ */
