/*
 * projwallpower (DLL 187 / 0x00BB) - retired projectile "wall power" object.
 *
 * The object's behavior has been removed: the only live entry point,
 * projwallpower_doUnsupported, just reports that the feature is no longer
 * supported and returns 0 (false). release/initialise are empty lifecycle
 * stubs.
 *
 * Note: unlike the rest of the retired-projectile family (projlightning1,
 * projquakeshock, etc.) which return the -1 unsupported sentinel, wallpower
 * returns 0.
 */
#include "dolphin/os/OSReport.h"
#include "main/dll/dll_descriptor_table.h"
#include "main/dll/dll_00BB_projwallpower.h"

int projwallpower_doUnsupported(void)
{
    OSReport(sProjwallpowerDoNoLongerSupported);
    return 0;
}

void projwallpower_release(void)
{
}

void projwallpower_initialise(void)
{
}

DllDescriptorTable lbl_80319840 = {{(void*)0x00000000, (void*)0x00000000, (void*)0x00000000, (void*)0x00030000,
                                    projwallpower_initialise, projwallpower_release, (void*)0x00000000,
                                    projwallpower_doUnsupported}};

char sProjwallpowerDoNoLongerSupported[] = "<projwallpower Do>No Longer supported \n";
