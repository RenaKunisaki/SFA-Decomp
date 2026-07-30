#ifndef MAIN_DLL_DLL_00E2_STAFF_API_H_
#define MAIN_DLL_DLL_00E2_STAFF_API_H_

#include "game/objects/object.h"
#include "dlls/object_descriptor.h"

extern ObjectDescriptor23 gStaffObjDescriptor;

void objSetAnimField48to0(GameObject* obj);
void playerRenderQuakeSpell(GameObject* obj);
void quakeSpellFn_8016cee8(GameObject* obj, GameObject* player);
void staffDrawQuakeSpellRing(void);
void staff_addHitReactValue(GameObject* obj, s32 delta);
void staffDoGrowShrinkAnim(GameObject* obj, u8 grow, u8 alternateRate, int unused);
void staff_free(GameObject* obj);
void staff_func0B(void);
void staff_func0E(void);
void staff_func0F(void);
void staff_func10(GameObject* obj, s32 value);
int staff_getExtraSize(void);
void staff_getHitGeometryPoints(GameObject* obj, f32* outA, f32* outB);
s16 staff_getHitReactValue(GameObject* obj);
int staff_getObjectTypeId(void);
s32 staff_getSwipeTextureIndex(GameObject* obj);
void staff_hitDetect(void);
void staff_hitDetectGeometry(GameObject* obj);
void staff_init(GameObject* obj);
void staff_initialise(void);
void staff_updateSwipe(GameObject* obj, int p4, int p5);
void staff_release(void);
void staff_render(void);
void staffSetGlow(GameObject* obj, u8 attackType, u8 enable);
void staff_func0A(void);
void staff_setHitReactValue(GameObject* obj, s32 value);
void staff_setupSwipe(int p1, u8* swipe, int p3, int p4);
void staff_startSwipe(GameObject* obj, s16 index, f32 arg2, f32 arg3);
void staff_update(GameObject* obj);
void staffStartQuakeSpell(f32* position);

#endif /* MAIN_DLL_DLL_00E2_STAFF_API_H_ */
