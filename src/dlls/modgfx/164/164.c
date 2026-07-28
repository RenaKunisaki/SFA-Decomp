/*
 * dummya4 (DLL 164 / 0xA4) - empty placeholder DLL.
 *
 * Provides only the three required DLL entry points: a no-op
 * release and initialise, plus a func03 stub that returns 0. No game
 * objects, state, or behaviour live here.
 */
#include "dlls/object_descriptor.h"


int DummyA4_func03_ret_0(void)
{
    return 0x0;
}

void DummyA4_release(void)
{
}

void DummyA4_initialise(void)
{
}

ObjectDescriptor4 DummyA4_funcs = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_4_SLOTS,
    (ObjectDescriptorCallback)DummyA4_initialise,
    (ObjectDescriptorCallback)DummyA4_release,
    0,
    (ObjectDescriptorCallback)DummyA4_func03_ret_0,
};
