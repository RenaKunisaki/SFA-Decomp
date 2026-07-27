#ifndef MAIN_DLL_DB_DBROCKFALL_H_
#define MAIN_DLL_DB_DBROCKFALL_H_

#include "game/objects/object.h"
#include "ghidra_import.h"
#include "main/dll/paymentkiosk.h"
#include "dlls/object_descriptor.h"
#include "main/objanim_update.h"

typedef struct FEseqobjectState
{
    u8 state;
    u8 pad01;
    u8 promptState;
} FEseqobjectState;

void PaymentKiosk_init(int obj, PaymentKioskMapData* initData);
int FEseqobject_SeqFn(int obj, int unused, ObjAnimUpdateState* animUpdate);
void FEseqobject_init(GameObject* obj);
void FEseqobject_update(int obj);
void PaymentKiosk_release(void);
void PaymentKiosk_initialise(void);
void FUN_801df784(u64 param_1, u64 param_2, u64 param_3, u64 param_4, u64 param_5, u64 param_6, u64 param_7,
                  u64 param_8, u32 param_9, u32 param_10, int param_11);
int FEseqobject_getExtraSize(void);
int FEseqobject_getObjectTypeId(void);
void FEseqobject_free(void);
void FEseqobject_render(GameObject* obj, int p2, int p3, int p4, int p5, s8 visible);
void FEseqobject_hitDetect(void);
void FEseqobject_release(void);
void FEseqobject_initialise(void);
extern ObjectDescriptor gFEseqobjectObjDescriptor;

#endif /* MAIN_DLL_DB_DBROCKFALL_H_ */
