/*
 * projcore1 (DLL 191 / 0xBF) - retired projectile-core DLL.
 *
 * The only live entry point reports that projectiles are no longer
 * supported and returns the unsupported sentinel (-1); the DLL's
 * release/initialise lifecycle hooks are empty stubs.
 */
#include "dolphin/os/OSReport.h"
#include "main/dll/dll_descriptor_table.h"
#include "main/dll/dll_00BF_projcore1.h"

int projcore1_doUnsupported(void)
{
    OSReport(sProjcore1DoNoLongerSupported);
    return -1;
}

void projcore1_release(void)
{
}

void projcore1_initialise(void)
{
}

DllDescriptorTable lbl_80319968 = {{(void*)0x00000000, (void*)0x00000000, (void*)0x00000000, (void*)0x00030000,
                                    projcore1_initialise, projcore1_release, (void*)0x00000000,
                                    projcore1_doUnsupported}};

char sProjcore1DoNoLongerSupported[] = "<projcore1 Do>No Longer supported \n";
