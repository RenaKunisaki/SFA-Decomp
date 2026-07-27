/*
 * projship1 (DLL 183 / 0xB7) - retired "ship projectile 1" object.
 *
 * The object's behaviour was removed during development: its only live
 * entry point, projship1_doUnsupported, just logs a "no longer supported"
 * message and returns -1. release/initialise are empty lifecycle hooks
 * kept so the object descriptor / DLL loader still resolves.
 */
#include "dolphin/os/OSReport.h"
#include "main/dll/dll_descriptor_table.h"
#include "main/dll/dll_00B7_projship1.h"

#define PROJECTILE_UNSUPPORTED_RETURN -1

int projship1_doUnsupported(void)
{
    OSReport(sProjship1DoNoLongerSupported);
    return PROJECTILE_UNSUPPORTED_RETURN;
}

void projship1_release(void)
{
}

void projship1_initialise(void)
{
}

DllDescriptorTable lbl_803197F8 = {{(void*)0x00000000, (void*)0x00000000, (void*)0x00000000, (void*)0x00030000,
                                    projship1_initialise, projship1_release, (void*)0x00000000,
                                    projship1_doUnsupported}};

char sProjship1DoNoLongerSupported[] = "<projship1 Do>No Longer supported \n";
