#ifndef MAIN_DLL_DLL_00C6_ANIMATEDOBJ_API_H_
#define MAIN_DLL_DLL_00C6_ANIMATEDOBJ_API_H_

#include "types.h"
#include "dlls/object_descriptor.h"

typedef struct GameObject GameObject;
typedef struct AnimatedObjPlacement AnimatedObjPlacement;

extern ObjectDescriptor gAnimatedObjDescriptor;

void animatedobj_free(GameObject* obj, int seqFlag);
int animatedobj_getExtraSize(void);
void animatedobj_init(GameObject* obj, AnimatedObjPlacement* params);
void animatedobj_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible);
void animatedobj_update(GameObject* obj);

#endif /* MAIN_DLL_DLL_00C6_ANIMATEDOBJ_API_H_ */
