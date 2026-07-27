#ifndef MAIN_DLL_DLL_01B5_LIGHTFOOT_H_
#define MAIN_DLL_DLL_01B5_LIGHTFOOT_H_

#include "game/objects/object.h"

extern int lbl_80334EE8[];
extern int lbl_803DB0D0[];
extern int lbl_803DB0DC[];
extern u32 lbl_803DC6F0;
extern u32 lbl_803DC6F4[2];
extern u32 lbl_803DC6FC;
extern u32 lbl_803DC700[2];
extern u32 lbl_803DC708;
extern u32 lbl_803DC70C[2];
extern u32 lbl_803DC714;
extern u32 lbl_803DC718[2];
extern u32 lbl_803DC720;
extern u32 lbl_803DC724[2];
extern f32 lbl_803E817C;
extern f32 lbl_803E8180;
extern f32 lbl_803E8188;
extern f32 lbl_803E81C0;
extern f32 lbl_803E81C4;
extern f32 lbl_803E81C8;
extern f32 lbl_803E81D0;
extern const f32 lbl_803E8214;
extern const f32 lbl_803E8218;
extern const f32 lbl_803E821C;
extern const f32 lbl_803E8220;
extern const f32 lbl_803E8224;
extern const f32 lbl_803E8228;

int lightfoot_getExtraSize(void);
int lightfoot_getObjectTypeId(void);
void lightfoot_free(GameObject* obj, int flag);
void lightfoot_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible);
void lightfoot_hitDetect(void);
void lightfoot_update(GameObject* obj);
void lightfoot_init(GameObject* obj, int def, int flag);
void lightfoot_release(void);
void lightfoot_initialise(void);

#endif /* MAIN_DLL_DLL_01B5_LIGHTFOOT_H_ */
