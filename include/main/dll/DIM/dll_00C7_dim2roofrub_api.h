#ifndef MAIN_DLL_DIM_DLL_00C7_DIM2ROOFRUB_API_H_
#define MAIN_DLL_DIM_DLL_00C7_DIM2ROOFRUB_API_H_

#include "types.h"
#include "dlls/object_descriptor.h"

extern ObjectDescriptor gDIM2RoofRubObjDescriptor;

struct DIM2RoofRubPlacement;

void dim2roofrub_free(GameObject* obj);
int dim2roofrub_getExtraSize(void);
void dim2roofrub_init(GameObject* obj, struct DIM2RoofRubPlacement* params);
void dim2roofrub_render(GameObject* obj, int p2, int p3, int p4, int p5);
void dim2roofrub_update(GameObject* obj);

#endif /* MAIN_DLL_DIM_DLL_00C7_DIM2ROOFRUB_API_H_ */
