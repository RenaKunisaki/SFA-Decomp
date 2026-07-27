#ifndef MAIN_DLL_CREATOR19D_H_
#define MAIN_DLL_CREATOR19D_H_

#include "ghidra_import.h"

void DFSH_LaserBeam_update(u32 param_1);
void DFSH_LaserBeam_init(void *obj,void *config);
void DFSH_LaserBeam_release(void);
void DFSH_LaserBeam_initialise(void);
void dfshLaserBeam_updateOrbit(void *obj);
int dfshLaserBeam_updateFearSway(void *obj);

#endif /* MAIN_DLL_CREATOR19D_H_ */
