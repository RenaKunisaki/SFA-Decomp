#ifndef MAIN_DLL_SH_DLL_01B1_SHSTAFF_H_
#define MAIN_DLL_SH_DLL_01B1_SHSTAFF_H_

#include "game/objects/object.h"

int sh_staff_getExtraSize(void);
void sh_staff_free(int* obj, int flag);
void sh_staff_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible);
void sh_staff_update(GameObject* obj);

#endif /* MAIN_DLL_SH_DLL_01B1_SHSTAFF_H_ */
