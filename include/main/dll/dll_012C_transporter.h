#ifndef MAIN_DLL_DLL_012C_TRANSPORTER_H_
#define MAIN_DLL_DLL_012C_TRANSPORTER_H_

#include "global.h"
#include "game/objects/object.h"
#include "main/objanim_update.h"

int Transporter_SeqFn(GameObject* obj, int unused, ObjAnimUpdateState* animUpdate);
int Transporter_getExtraSize(void);
void Transporter_update(int obj);
void Transporter_hitDetect(int obj);
void Transporter_render(void);
void Transporter_init(GameObject* obj, u8* params);

#endif /* MAIN_DLL_DLL_012C_TRANSPORTER_H_ */
