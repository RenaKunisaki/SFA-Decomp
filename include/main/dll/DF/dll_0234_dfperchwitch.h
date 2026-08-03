#ifndef MAIN_DLL_DF_DLL_0234_DFPERCHWITCH_H_
#define MAIN_DLL_DF_DLL_0234_DFPERCHWITCH_H_

#include "types.h"
#include "dlls/object_descriptor.h"

extern char sDfperchwitchInitNoLongerSupported[];
extern ObjectDescriptor gDfperchwitchObjDescriptor;

int DFP_PerchWitch_getExtraSize(void);
int DFP_PerchWitch_getObjectTypeId(void);
void DFP_PerchWitch_free(void);
void DFP_PerchWitch_render(void);
void DFP_PerchWitch_hitDetect(void);
void DFP_PerchWitch_update(void);
void DFP_PerchWitch_init(void);
void DFP_PerchWitch_release(void);
void DFP_PerchWitch_initialise(void);

#endif /* MAIN_DLL_DF_DLL_0234_DFPERCHWITCH_H_ */
