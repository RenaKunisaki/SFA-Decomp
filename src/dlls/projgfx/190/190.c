/*
 * projtesla (DLL 190 / 0xBE) - defunct "tesla" projectile (behaviour cut).
 *
 * Its only non-trivial entry point reports the "no longer supported" string
 * via OSReport and returns 0. The release and initialise descriptor hooks are
 * empty stubs.
 */
#include "dolphin/os/OSReport.h"
#include "main/dll/dll_00BE_projtesla.h"

int projtesla_doUnsupported(void)
{
    OSReport(sProjteslaDoNoLongerSupported);
    return 0;
}

void projtesla_release(void)
{
}

void projtesla_initialise(void)
{
}

void* lbl_80319920[8] = {(void*)0x00000000,    (void*)0x00000000, (void*)0x00000000, (void*)0x00030000,
                         projtesla_initialise, projtesla_release, (void*)0x00000000, projtesla_doUnsupported};

char sProjteslaDoNoLongerSupported[] = "<projtesla Do>No Longer supported \n";
