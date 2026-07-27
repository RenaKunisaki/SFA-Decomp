/*
 * projlightning3 (DLL 177 / 0x00B1) - a retired projectile object.
 *
 * The lightning-3 projectile was cut from the shipped game: its object
 * entry point (projlightning3_doUnsupported) only logs that it is "no longer supported"
 * and returns the unsupported sentinel. release/initialise are the empty
 * object lifecycle hooks that remain so the object descriptor stays valid.
 */
#include "dolphin/os.h"
#include "main/dll/dll_descriptor_table.h"
#include "main/dll/dll_00B1_projlightning3.h"

#define PROJECTILE_UNSUPPORTED_RETURN -1

DllDescriptorTable lbl_80319548 = {{(void*)0x00000000, (void*)0x00000000, (void*)0x00000000, (void*)0x00030000,
                                    projlightning3_initialise, projlightning3_release, (void*)0x00000000,
                                    projlightning3_doUnsupported}};

char sProjlightning3DoNoLongerSupported[] = "<projlightning3 Do>No Longer supported \n";

int projlightning3_doUnsupported(void)
{
    OSReport(sProjlightning3DoNoLongerSupported);
    return PROJECTILE_UNSUPPORTED_RETURN;
}

void projlightning3_release(void)
{
}

void projlightning3_initialise(void)
{
}
