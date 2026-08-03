/*
 * DragonRock Palace perch switch. This legacy object is no longer
 * supported; its init and update callbacks only report that fact.
 */
#include "main/dll/DF/dll_0233_dfpstatue1.h"
#include "dolphin/os/OSReport.h"

int DFP_PerchWitch_getExtraSize(void)
{
    return 0x0;
}
int DFP_PerchWitch_getObjectTypeId(void)
{
    return 0x0;
}

void DFP_PerchWitch_free(void)
{
}

void DFP_PerchWitch_render(void)
{
}

void DFP_PerchWitch_hitDetect(void)
{
}

void DFP_PerchWitch_update(void)
{
    OSReport(sDfperchwitchInitNoLongerSupported);
}
void DFP_PerchWitch_init(void)
{
    OSReport(sDfperchwitchInitNoLongerSupported);
}

void DFP_PerchWitch_release(void)
{
}

void DFP_PerchWitch_initialise(void)
{
}

ObjectDescriptor gDfperchwitchObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)DFP_PerchWitch_initialise,
    (ObjectDescriptorCallback)DFP_PerchWitch_release,
    0,
    (ObjectDescriptorCallback)DFP_PerchWitch_init,
    (ObjectDescriptorCallback)DFP_PerchWitch_update,
    (ObjectDescriptorCallback)DFP_PerchWitch_hitDetect,
    (ObjectDescriptorCallback)DFP_PerchWitch_render,
    (ObjectDescriptorCallback)DFP_PerchWitch_free,
    (ObjectDescriptorCallback)DFP_PerchWitch_getObjectTypeId,
    DFP_PerchWitch_getExtraSize,
};

char sDfperchwitchInitNoLongerSupported[] = "<dfperchwitch Init>No Longer supported \n";
