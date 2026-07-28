/*
 * DLL 108 / 0x6C - empty placeholder DLL.
 *
 * Provides only the three required object-DLL entry points: a no-op
 * release and initialise, plus a func03 stub that returns 0. No game
 * objects, state, or behaviour live here.
 */
#include "main/dll/dll_006C_dummy6c.h"
#include "dlls/object_descriptor.h"

ObjectDescriptor4 Dummy6C_funcs = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_4_SLOTS,
    (ObjectDescriptorCallback)Dummy6C_initialise,
    (ObjectDescriptorCallback)Dummy6C_release,
    0,
    (ObjectDescriptorCallback)Dummy6C_func03_ret_0,
};

int Dummy6C_func03_ret_0(void)
{
    return 0x0;
}

void Dummy6C_release(void)
{
}

void Dummy6C_initialise(void)
{
}
