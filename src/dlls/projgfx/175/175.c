/*
 * projlightning1 (DLL 175 / 0x00AF) - retired "lightning 1" projectile object.
 *
 * The object's behaviour was cut from the shipping game: its "do" entry
 * point now only logs a "no longer supported" message and returns the
 * unsupported sentinel (-1). release/initialise are empty stubs kept so the
 * DLL still exports the standard projectile lifecycle entry points. This is
 * one of a family of identical retired projectile DLLs.
 */
#include "dolphin/os/OSReport.h"
#include "main/dll/dll_descriptor_table.h"
#include "main/dll/dll_00AF_projlightning1.h"
#include "dlls/object_descriptor.h"

#define PROJECTILE_UNSUPPORTED_RETURN -1

int projlightning1_doUnsupported(void)
{
    OSReport(sProjlightning1DoNoLongerSupported);
    return PROJECTILE_UNSUPPORTED_RETURN;
}

void projlightning1_release(void)
{
}

void projlightning1_initialise(void)
{
}

ObjectDescriptor4 projlightning1_funcs = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_4_SLOTS,
    (ObjectDescriptorCallback)projlightning1_initialise,
    (ObjectDescriptorCallback)projlightning1_release,
    0,
    (ObjectDescriptorCallback)projlightning1_doUnsupported,
};

char sProjlightning1DoNoLongerSupported[] = "<projlightning1 Do>No Longer supported \n";
