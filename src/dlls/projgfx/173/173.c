/*
 * projmagicemmit1 (DLL 173 / 0xAD) - retired projectile object.
 *
 * One of the stubbed-out projectile DLLs. The object has
 * no behaviour left: release/initialise are empty and doUnsupported just
 * logs the "no longer supported" string and returns the failure sentinel.
 * The slot is kept so the DLL id stays valid.
 */
#include "dolphin/os/OSReport.h"
#include "main/dll/dll_descriptor_table.h"
#include "main/dll/dll_00AD_projmagicemmit1.h"

int projmagicemmit1_doUnsupported(void)
{
    OSReport(sProjmagicemmit1DoNoLongerSupported);
    return -1; /* failure sentinel */
}

void projmagicemmit1_release(void)
{
}

void projmagicemmit1_initialise(void)
{
}

DllDescriptorTable lbl_80319410 = {{(void*)0x00000000, (void*)0x00000000, (void*)0x00000000, (void*)0x00030000,
                                    projmagicemmit1_initialise, projmagicemmit1_release, (void*)0x00000000,
                                    projmagicemmit1_doUnsupported}};

char sProjmagicemmit1DoNoLongerSupported[] = "<projmagicemmit1 Do>No Longer supported \n";
