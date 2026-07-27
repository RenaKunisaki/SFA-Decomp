/*
 * projlightning1 (DLL 175 / 0x00AF) - retired "lightning 1" projectile object.
 *
 * The object's behaviour was cut from the shipping game: its "do" entry
 * point now only logs a "no longer supported" message and returns the
 * unsupported sentinel (-1). release/initialise are empty stubs kept so the
 * DLL still exports the standard projectile lifecycle entry points. This is
 * one of a family of identical retired projectile DLLs.
 */
#include "dolphin/os/OSReport.h"
#include "main/dll/dll_descriptor_table.h"
#include "main/dll/dll_00AF_projlightning1.h"

#define PROJECTILE_UNSUPPORTED_RETURN -1

int projlightning1_doUnsupported(void)
{
    OSReport(sProjlightning1DoNoLongerSupported);
    return PROJECTILE_UNSUPPORTED_RETURN;
}

void projlightning1_release(void)
{
}

void projlightning1_initialise(void)
{
}

u32 lbl_803194A8[8] = {0x00000000,
                       0x00000000,
                       0x00000000,
                       0x00030000,
                       (u32)projlightning1_initialise,
                       (u32)projlightning1_release,
                       0x00000000,
                       (u32)projlightning1_doUnsupported};

char sProjlightning1DoNoLongerSupported[] = "<projlightning1 Do>No Longer supported \n";
