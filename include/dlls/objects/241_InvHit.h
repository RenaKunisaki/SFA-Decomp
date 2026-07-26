#ifndef DLLS_OBJECTS_241_INVHIT_H_
#define DLLS_OBJECTS_241_INVHIT_H_

#include "dlls/object_descriptor.h"
#include "game/objects/object_fwd.h"

struct InvhitObjectDef;

int InvHit_getExtraSize(void);
int InvHit_getObjectTypeId(void);
void InvHit_free(GameObject* obj);
void InvHit_render(GameObject* obj, int fwdArg2, int fwdArg3, int fwdArg4, int fwdArg5);
void InvHit_hitDetect(void);
void InvHit_update(GameObject* obj);
void InvHit_init(GameObject* obj, struct InvhitObjectDef* def);
void InvHit_release(void);
void InvHit_initialise(void);

extern ObjectDescriptor gInvHitObjDescriptor;

#endif /* DLLS_OBJECTS_241_INVHIT_H_ */
