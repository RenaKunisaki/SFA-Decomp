/*
 * projdfp1r (DLL 194 / 0xC2) - retired "dfp1r" projectile object.
 *
 * The object is no longer supported: its single behavior entry point just
 * prints the "projdfp1r ... No Longer supported" banner and returns -1, and
 * the load/unload hooks are empty stubs.
 */
#include "dolphin/os.h"
#include "main/dll/dll_BB.h"
#include "main/dll/dll_BC.h"
#include "main/dll/dll_8B.h"
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

/* Explicit length preserves the four-byte retail padding before lbl_80319A88. */
char sProjdfp1rDoNoLongerSupported[40] = "<projdfp1r Do>No Longer supported \n";

/* Camera interface table. */
void* lbl_80319A88[35] = {(void*)0x00000000,
                          (void*)0x00000000,
                          (void*)0x00000000,
                          (void*)0x001d0000,
                          Camera_initialise,
                          Camera_release,
                          (void*)0x00000000,
                          Camera_init,
                          Camera_update,
                          Camera_get,
                          Camera_getMode,
                          Camera_GetFollowPos,
                          Camera_getDefaultHandlerEntry,
                          Camera_setMode,
                          Camera_getCamActionsBinEntry,
                          camcontrol_loadTriggeredCamAction,
                          Camera_setFocus,
                          Camera_overridePos,
                          Camera_moveBy,
                          camcontrol_initialise,
                          camcontrol_getRelativePosition,
                          Camera_getOverrideTarget,
                          Camera_getTarget,
                          Camera_func13,
                          Camera_setTarget,
                          Camera_setTargetReticleOverride,
                          Camera_isZooming,
                          camcontrol_updateTargetFeedback,
                          Camera_minimapShowHelpTextForTarget,
                          Camera_setLetterbox,
                          camcontrol_release,
                          Camera_getMinimapInfoText,
                          Camera_func1C,
                          Camera_func1D,
                          camcontrol_queueSavedAction};
