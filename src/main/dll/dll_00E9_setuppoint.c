/*
 * DLL 0xE9 - SetupPoint [8016B230-8016B2E0)
 *
 * The DLL's own canonical code is just setuppoint_init, an empty
 * load-time hook. Foreign ObjectDescriptor registration tables are not
 * present in this translation unit; each descriptor is defined by its own DLL.
 */
#include "main/dll/dll_00E9_setuppoint.h"
#include "dlls/object_descriptor.h"

void setuppoint_init(void)
{
}

ObjectDescriptor gSetuppointObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    0,
    0,
    0,
    (ObjectDescriptorCallback)setuppoint_init,
    0,
    0,
    0,
    0,
    0,
    0,
};
