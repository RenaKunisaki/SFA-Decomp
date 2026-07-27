/*
 * projdfp1r (DLL 194 / 0xC2) - retired "dfp1r" projectile object.
 *
 * The object is no longer supported: its single behavior entry point just
 * prints the "projdfp1r ... No Longer supported" banner and returns -1, and
 * the load/unload hooks are empty stubs.
 */
#include "dolphin/os.h"
#include "main/dll/dll_descriptor_table.h"
#include "main/dll/dll_00C2_projdfp1r.h"

#define PROJECTILE_UNSUPPORTED_RETURN -1

int projdfp1r_doUnsupported(void)
{
    OSReport(sProjdfp1rDoNoLongerSupported);
    return PROJECTILE_UNSUPPORTED_RETURN;
}

void projdfp1r_release(void)
{
}

void projdfp1r_initialise(void)
{
}

DllDescriptorTable lbl_80319A40 = {{(void*)0x00000000, (void*)0x00000000, (void*)0x00000000, (void*)0x00030000,
                                    projdfp1r_initialise, projdfp1r_release, (void*)0x00000000,
                                    projdfp1r_doUnsupported}};

/* Explicit length preserves the four-byte retail tail padding. */
char sProjdfp1rDoNoLongerSupported[] = "<projdfp1r Do>No Longer supported \n";
