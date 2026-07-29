/*
 * projtesla (DLL 190 / 0xBE) - defunct "tesla" projectile (behaviour cut).
 *
 * Its only non-trivial entry point reports the "no longer supported" string
 * via OSReport and returns 0. The release and initialise descriptor hooks are
 * empty stubs.
 */
#include "dolphin/os/OSReport.h"
#include "main/dll/dll_00BE_projtesla.h"
#include "dlls/object_descriptor.h"

int projtesla_doUnsupported(void)
{
    OSReport(sProjteslaDoNoLongerSupported);
    return 0;
}

void projtesla_release(void)
{
}

void projtesla_initialise(void)
{
}

ObjectDescriptor4 projtesla_funcs = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_4_SLOTS,
    (ObjectDescriptorCallback)projtesla_initialise,
    (ObjectDescriptorCallback)projtesla_release,
    0,
    (ObjectDescriptorCallback)projtesla_doUnsupported,
};

char sProjteslaDoNoLongerSupported[] = "<projtesla Do>No Longer supported \n";
