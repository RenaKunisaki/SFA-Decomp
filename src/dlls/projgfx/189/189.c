/*
 * projsunshock (DLL 189 / 0xBD) - a deprecated projectile object DLL.
 *
 * Only the standard DLL lifecycle entry points survive. The object is
 * no longer functional: doUnsupported logs a "no longer supported"
 * message and returns the failure sentinel, while release/initialise are
 * empty stubs that keep the DLL loadable.
 */
#include "dolphin/os/OSReport.h"
#include "main/dll/dll_descriptor_table.h"
#include "main/dll/dll_00BD_projsunshock.h"

int projsunshock_doUnsupported(void)
{
    OSReport(sProjsunshockDoNoLongerSupported);
    return -1;
}

void projsunshock_release(void)
{
}

void projsunshock_initialise(void)
{
}

DllDescriptorTable lbl_803198D8 = {{(void*)0x00000000, (void*)0x00000000, (void*)0x00000000, (void*)0x00030000,
                                    projsunshock_initialise, projsunshock_release, (void*)0x00000000,
                                    projsunshock_doUnsupported}};

char sProjsunshockDoNoLongerSupported[] = "<projsunshock Do>No Longer supported \n";
