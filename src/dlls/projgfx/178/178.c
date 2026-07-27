/*
 * projrobotfire (DLL 178 / 0x00B2) - the robot fire projectile object.
 *
 * The entire retail DLL is a stub: doUnsupported logs "no longer supported"
 * via OSReport and returns -1; release/initialise are empty.
 */
#include "dolphin/os.h"
#include "main/dll/dll_descriptor_table.h"
#include "main/dll/dll_00B2_projrobotfire.h"

#define PROJECTILE_UNSUPPORTED_RETURN -1

int projrobotfire_doUnsupported(void)
{
    OSReport(sProjrobotfireDoNoLongerSupported);
    return PROJECTILE_UNSUPPORTED_RETURN;
}

void projrobotfire_release(void)
{
}

void projrobotfire_initialise(void)
{
}

DllDescriptorTable lbl_80319768 = {{(void*)0x00000000, (void*)0x00000000, (void*)0x00000000, (void*)0x00030000,
                                    projrobotfire_initialise, projrobotfire_release, (void*)0x00000000,
                                    projrobotfire_doUnsupported}};

char sProjrobotfireDoNoLongerSupported[] = "<projrobotfire Do>No Longer supported \n";
