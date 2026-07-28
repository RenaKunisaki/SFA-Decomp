/*
 * projquakeshock (DLL 188 / 0xBC) - retired "quake shock" projectile object.
 *
 * The DLL's lifecycle hooks (release/initialise) are empty and its single
 * entry point logs a "no longer supported" message and returns a failure
 * code, so this projectile type has been disabled in retail.
 */
#include "dolphin/os.h"
#include "main/dll/dll_00BC_projquakeshock.h"
#include "dlls/object_descriptor.h"

#define PROJECTILE_UNSUPPORTED_RETURN -1

int projquakeshock_doUnsupported(void)
{
    OSReport(sProjquakeshockDoNoLongerSupported);
    return PROJECTILE_UNSUPPORTED_RETURN;
}

void projquakeshock_release(void)
{
}

void projquakeshock_initialise(void)
{
}

ObjectDescriptor4 projquakeshock_funcs = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_4_SLOTS,
    (ObjectDescriptorCallback)projquakeshock_initialise,
    (ObjectDescriptorCallback)projquakeshock_release,
    0,
    (ObjectDescriptorCallback)projquakeshock_doUnsupported,
};

char sProjquakeshockDoNoLongerSupported[] = "<projquakeshock Do>No Longer supported \n";
