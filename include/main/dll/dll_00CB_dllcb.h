#ifndef MAIN_DLL_DLL_00CB_DLLCB_H_
#define MAIN_DLL_DLL_00CB_DLLCB_H_

#include "game/objects/object.h"
#include "main/dll/baddie_state.h"

struct ObjSeqState;

int dll_CB_stateHandler5(GameObject* obj, GroundBaddieState* p);
int dll_CB_stateHandler2(GameObject* obj, GroundBaddieState* p);
void dll_CB_seekAndUpdate(GameObject* obj, void* p2, GroundBaddieState* sub, GroundBaddieState* p);
void dll_CB_advanceAI(GameObject* obj, GroundBaddieState* sub, GroundBaddieState* p);
int dll_CB_seqFn(GameObject* obj, int p2, struct ObjSeqState* e);
void dll_CB_func0B_nop(void);
void dll_CB_release_nop(void);
void dll_CB_init(GameObject* obj, u8* params, int extra);
void dll_CB_update(GameObject* obj);
int dll_CB_stateHandler0(void);
int dll_CB_getExtraSize_ret_1040(void);
int dll_CB_getObjectTypeId(void);
s16 dll_CB_setScale(GameObject* obj);
int dll_CB_stateHandler1(int p1, u8* obj);
int dll_CB_stateHandler3(GameObject* obj, u8* obj2);
void dll_CB_hitDetect(GameObject* obj);
void dll_CB_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible);
int dll_CB_moveHandler2(short* out, BaddieState* obj);
int dll_CB_moveHandler0(short* out, BaddieState* obj, f32 timeDelta);
int dll_CB_stateHandler4(GameObject* obj, GroundBaddieState* state);
int dll_CB_moveHandler1(GameObject* obj, GroundBaddieState* def);
void dll_CB_initialise(void);
int dll_CB_moveHandler3(GameObject* obj);
void dll_CB_free(GameObject* obj);

#endif /* MAIN_DLL_DLL_00CB_DLLCB_H_ */
