/*
 * projlightning4 (DLL 179 / 0x00B3) - a retired projectile object.
 *
 * The lightning-4 projectile was cut from the shipped game: its object
 * entry point (projlightning4_doUnsupported) only logs that it is "no longer
 * supported" and returns the unsupported sentinel. release/initialise are the
 * empty object lifecycle hooks that remain so the object descriptor stays valid.
 */
#include "dolphin/os.h"
#include "main/dll/dll_descriptor_table.h"
#include "main/dll/dll_00B3_projlightning4.h"

#define PROJECTILE_UNSUPPORTED_RETURN -1

DllDescriptorTable lbl_80319598 = {{(void*)0x00000000, (void*)0x00000000, (void*)0x00000000, (void*)0x00030000,
                                    projlightning4_initialise, projlightning4_release, (void*)0x00000000,
                                    projlightning4_doUnsupported}};

char sProjlightning4DoNoLongerSupported[] = "<projlightning4 Do>No Longer supported \n";

int projlightning4_doUnsupported(void)
{
    OSReport(sProjlightning4DoNoLongerSupported);
    return PROJECTILE_UNSUPPORTED_RETURN;
}

void projlightning4_release(void)
{
}

void projlightning4_initialise(void)
{
}
