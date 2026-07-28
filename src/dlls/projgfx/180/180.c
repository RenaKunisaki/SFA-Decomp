/*
 * projenergise1 (DLL 180 / 0xB4) - retired "energise projectile" object DLL.
 *
 * The object itself is gone: its only real entry point logs a
 * "no longer supported" message and returns -1, while the standard DLL
 * lifecycle hooks (release/initialise) are empty stubs. Effectively a
 * placeholder that keeps the DLL id valid after the projectile behavior
 * was removed.
 */
#include "dolphin/os/OSReport.h"
#include "main/dll/dll_descriptor_table.h"
#include "main/dll/dll_00B4_projenergise1.h"

#define PROJECTILE_UNSUPPORTED_RETURN -1

int projenergise1_doUnsupported(void)
{
    OSReport(sProjenergise1DoNoLongerSupported);
    return PROJECTILE_UNSUPPORTED_RETURN;
}

void projenergise1_release(void)
{
}

void projenergise1_initialise(void)
{
}

DllDescriptorTable lbl_803196D8 = {{(void*)0x00000000, (void*)0x00000000, (void*)0x00000000, (void*)0x00030000,
                                    projenergise1_initialise, projenergise1_release, (void*)0x00000000,
                                    projenergise1_doUnsupported}};

char sProjenergise1DoNoLongerSupported[] = "<projenergise1 Do>No Longer supported \n";
