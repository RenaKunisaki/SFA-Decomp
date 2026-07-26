/*
 * DLL 0xE9 - Setuppoint.
 *
 * Its only function is setuppoint_init, an empty load-time hook.
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
