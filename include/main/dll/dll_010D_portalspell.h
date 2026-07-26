#ifndef MAIN_DLL_DLL_010D_PORTALSPELL_H_
#define MAIN_DLL_DLL_010D_PORTALSPELL_H_

#include "dlls/object_descriptor.h"
#include "game/objects/object.h"

int PortalSpellDoor_getExtraSize(void);
int PortalSpellDoor_getObjectTypeId(void);
void PortalSpellDoor_free(void);
void PortalSpellDoor_render(int obj, int p2, int p3, int p4, int p5, s8 visible);
void PortalSpellDoor_hitDetect(void);
void PortalSpellDoor_update(GameObject* obj);
void PortalSpellDoor_init(GameObject* obj, u8* data);
void PortalSpellDoor_release(void);
void PortalSpellDoor_initialise(void);

extern ObjectDescriptor gPortalSpellDoorObjDescriptor;

#endif /* MAIN_DLL_DLL_010D_PORTALSPELL_H_ */
