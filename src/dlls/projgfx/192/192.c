/*
 * projcore2 (DLL 192 / 0xC0) - retired projectile-core DLL.
 *
 * The only live entry point reports that projectiles are no longer
 * supported and returns the unsupported sentinel (-1); the DLL's
 * release/initialise lifecycle hooks are empty stubs.
 */
#include "dolphin/os/OSReport.h"
#include "main/dll/dll_descriptor_table.h"
#include "main/dll/dll_00C0_projcore2.h"

#define PROJECTILE_UNSUPPORTED_RETURN -1

int projcore2_doUnsupported(void)
{
    OSReport(sProjcore2DoNoLongerSupported);
    return PROJECTILE_UNSUPPORTED_RETURN;
}

void projcore2_release(void)
{
}

void projcore2_initialise(void)
{
}

DllDescriptorTable lbl_803199B0 = {{(void*)0x00000000, (void*)0x00000000, (void*)0x00000000, (void*)0x00030000,
                                    projcore2_initialise, projcore2_release, (void*)0x00000000,
                                    projcore2_doUnsupported}};

char sProjcore2DoNoLongerSupported[] = "<projcore2 Do>No Longer supported \n";
