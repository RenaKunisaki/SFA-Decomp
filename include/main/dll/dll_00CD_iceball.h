#ifndef MAIN_DLL_DLL_00CD_ICEBALL_H_
#define MAIN_DLL_DLL_00CD_ICEBALL_H_

#include "dlls/object_descriptor.h"
#include "game/objects/object.h"

void iceBall_handleSurfaceImpact(GameObject* obj);
void iceBall_handleCharacterImpact(GameObject* obj);
int IceBall_getExtraSize(void);
int IceBall_getObjectTypeId(void);
void IceBall_free(GameObject* obj);
void IceBall_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible);
void IceBall_hitDetect(GameObject* obj);
void IceBall_update(GameObject* obj);
void IceBall_init(GameObject* obj);
void IceBall_release(void);
void IceBall_initialise(void);
extern ObjectDescriptor gIceBallObjDescriptor;

#endif /* MAIN_DLL_DLL_00CD_ICEBALL_H_ */
