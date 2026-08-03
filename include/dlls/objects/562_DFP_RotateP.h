#ifndef DLLS_OBJECTS_562_DFP_ROTATEP_H_
#define DLLS_OBJECTS_562_DFP_ROTATEP_H_

#include "types.h"
#include "game/objects/object.h"
#include "main/objseq.h"

typedef struct DFPRotatePStateFlags {
  u8 bit80 : 1;
  u8 bit40 : 1;
  u8 bit20 : 1;
  u8 bit10 : 1;
  u8 lowBits : 4;
} DFPRotatePStateFlags;

typedef struct DFPRotatePState {
  s16 eventId;
  union {
    s16 config20;
    s16 activationEventId;
  };
  union {
    s16 unk4;
    s16 variantSfxTimer;
  };
  u8 config19;
  u8 ringCount;
  DFPRotatePStateFlags flags;
} DFPRotatePState;

extern int gDFP_RotatePEffectHandles[8];

int DFP_RotateP_getExtraSize(void);
int DFP_RotateP_getObjectTypeId(void);
void DFP_RotateP_render(void);
void DFP_RotateP_hitDetect(void);
void DFP_RotateP_update(GameObject* obj);
void DFP_RotateP_init(GameObject* obj,int config);
void DFP_RotateP_free(u32 obj, int arg1);
void DFP_RotateP_release(void);
void DFP_RotateP_initialise(void);
void DFP_RotateP_updateEffectHandleRing(GameObject* obj);
int DFP_RotateP_ensureEffectHandlePair(GameObject* obj, u8 ringIndex);
int DFP_RotateP_activateEffectHandleRing(GameObject* obj, int unused, ObjSeqState* animUpdate);

#endif /* DLLS_OBJECTS_562_DFP_ROTATEP_H_ */
