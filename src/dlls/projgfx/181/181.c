/*
 * projenergise2 (DLL 181 / 0x00B5) - retired "energise" projectile object.
 *
 * Its entry point reports that the projectile is no longer supported and
 * returns the unsupported sentinel; release/initialise are empty stubs.
 */
#include "dolphin/os.h"
#include "main/dll/dll_00B5_projenergise2.h"
#include "dlls/object_descriptor.h"

#define PROJENERGISE2_UNSUPPORTED -1

ObjectDescriptor4 projenergise2_funcs = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_4_SLOTS,
    (ObjectDescriptorCallback)projenergise2_initialise,
    (ObjectDescriptorCallback)projenergise2_release,
    0,
    (ObjectDescriptorCallback)projenergise2_doUnsupported,
};

char sProjenergise2DoNoLongerSupported[] = "<projenergise2 Do>No Longer supported \n";

int projenergise2_doUnsupported(void)
{
    OSReport(sProjenergise2DoNoLongerSupported);
    return PROJENERGISE2_UNSUPPORTED;
}

void projenergise2_release(void)
{
}

void projenergise2_initialise(void)
{
}
