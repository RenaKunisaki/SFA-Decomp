#ifndef MAIN_DLL_DFBARRELANIM_H_
#define MAIN_DLL_DFBARRELANIM_H_

#include "ghidra_import.h"
#include "main/dll/DF/DFbarrel.h"

DFRope* DFRope_Create(f32 startX, f32 startY, f32 startZ, f32 endX, f32 endY, f32 endZ, f32 unused, s32 count,
                      f32 tickScale);
void dfropenode_setMinY(GameObject* obj, float value);
int dfropenode_isVisible(GameObject* obj);
void dfropenode_setVisible(GameObject* obj, int value);
void dfropenode_clearLinkedObj(GameObject* obj);
int dfropenode_getAngle(GameObject* obj);
f32 DFRope_projectPointOntoSegment(f32* x, f32* y, f32* z, f32 startX, f32 startY, f32 startZ, f32 endX, f32 endY, f32 endZ);

#endif /* MAIN_DLL_DFBARRELANIM_H_ */
