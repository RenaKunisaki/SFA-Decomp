/*
 * projdummy (DLL 171 / 0xAB) - retired projectile object.
 *
 * One of the stubbed-out projectile DLLs. The object has
 * no behaviour left: release/initialise are empty and doUnsupported just
 * logs the "no longer supported" string and returns the failure sentinel.
 * The slot is kept so the DLL id stays valid.
 */
#include "dolphin/os/OSReport.h"
#include "main/dll/dll_00AB_projdummy.h"
#include "dlls/object_descriptor.h"

#define PROJECTILE_UNSUPPORTED_RETURN -1

ObjectDescriptor4 projdummy_funcs = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_4_SLOTS,
    (ObjectDescriptorCallback)projdummy_initialise,
    (ObjectDescriptorCallback)projdummy_release,
    0,
    (ObjectDescriptorCallback)projdummy_doUnsupported,
};

char sProjdummyDoNoLongerSupported[] = "<projdummy Do>No Longer supported \n";

int projdummy_doUnsupported(void)
{
    OSReport(sProjdummyDoNoLongerSupported);
    return PROJECTILE_UNSUPPORTED_RETURN;
}

void projdummy_release(void)
{
}

void projdummy_initialise(void)
{
}
