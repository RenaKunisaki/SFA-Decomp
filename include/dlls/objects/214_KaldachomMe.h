#ifndef DLLS_OBJECTS_214_KALDACHOMME_H_
#define DLLS_OBJECTS_214_KALDACHOMME_H_

#include "dlls/object_descriptor.h"
#include "game/objects/object_fwd.h"
#include "game/objects/object_setup.h"

typedef struct KaldaChompMeState {
    f32 progress;       /* 0x00 */
    f32 step;           /* 0x04 */
    f32 targetProgress; /* 0x08 */
    u8 moveId;          /* 0x0C */
    u8 pad0D[3];        /* 0x0D */
} KaldaChompMeState;

typedef struct KaldaChompMePlacement {
    ObjPlacement base; /* 0x00 */
    u8 rotZByte;       /* 0x18 */
    u8 rotYByte;       /* 0x19 */
    u8 rotXByte;       /* 0x1A */
    u8 pad1B;          /* 0x1B */
} KaldaChompMePlacement;

typedef u8 KaldaChompMeLinkedMode;

enum {
    KALDACHOMPME_LINKED_MODE_MOVE_0 = 1,
    KALDACHOMPME_LINKED_MODE_MOVE_1 = 2
};

STATIC_ASSERT(offsetof(KaldaChompMeState, progress) == 0x0);
STATIC_ASSERT(offsetof(KaldaChompMeState, step) == 0x4);
STATIC_ASSERT(offsetof(KaldaChompMeState, targetProgress) == 0x8);
STATIC_ASSERT(offsetof(KaldaChompMeState, moveId) == 0xC);
STATIC_ASSERT(sizeof(KaldaChompMeState) == 0x10);

STATIC_ASSERT(offsetof(KaldaChompMePlacement, base) == 0x0);
STATIC_ASSERT(offsetof(KaldaChompMePlacement, rotZByte) == 0x18);
STATIC_ASSERT(offsetof(KaldaChompMePlacement, rotYByte) == 0x19);
STATIC_ASSERT(offsetof(KaldaChompMePlacement, rotXByte) == 0x1A);
STATIC_ASSERT(sizeof(KaldaChompMePlacement) == 0x1C);

void kaldachompme_setLinkedMouthMode(GameObject* obj, KaldaChompMeLinkedMode mode);
int KaldaChompMe_getExtraSize(void);
int KaldaChompMe_getObjectTypeId(void);
void KaldaChompMe_free(GameObject* obj);
void KaldaChompMe_render(GameObject* obj, int fwdArg2, int fwdArg3, int fwdArg4, int fwdArg5, s8 visible);
void KaldaChompMe_hitDetect(GameObject* obj);
void KaldaChompMe_update(GameObject* obj);
void KaldaChompMe_init(GameObject* obj, KaldaChompMePlacement* placement);
void KaldaChompMe_release(void);
void KaldaChompMe_initialise(void);

extern ObjectDescriptor gKaldaChompMeObjDescriptor;

#endif /* DLLS_OBJECTS_214_KALDACHOMME_H_ */
