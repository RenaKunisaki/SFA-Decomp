#ifndef DLLS_OBJECTS_215_H_
#define DLLS_OBJECTS_215_H_

#include "dlls/object_descriptor.h"
#include "game/objects/object_fwd.h"
#include "main/modellight_api.h"

typedef struct KaldaChompSpitState {
    ModelLightStruct* light; /* 0x00 */
} KaldaChompSpitState;

STATIC_ASSERT(offsetof(KaldaChompSpitState, light) == 0x0);
STATIC_ASSERT(sizeof(KaldaChompSpitState) == 0x4);

int KaldaChompSpit_getExtraSize(void);
int KaldaChompSpit_getObjectTypeId(void);
void KaldaChompSpit_free(GameObject* obj);
void KaldaChompSpit_render(GameObject* obj, int fwdArg2, int fwdArg3, int fwdArg4, int fwdArg5, s8 visible);
void KaldaChompSpit_hitDetect(GameObject* obj);
void KaldaChompSpit_update(GameObject* obj);
void KaldaChompSpit_init(GameObject* obj);
void KaldaChompSpit_release(void);
void KaldaChompSpit_initialise(void);

extern ObjectDescriptor gKaldaChompSpitObjDescriptor;

#endif /* DLLS_OBJECTS_215_H_ */
