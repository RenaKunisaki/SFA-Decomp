#ifndef MAIN_DLL_CF_CFCHUCKOBJ_H_
#define MAIN_DLL_CF_CFCHUCKOBJ_H_

#include "ghidra_import.h"
#include "game/objects/object.h"
#include "dlls/object_descriptor.h"
#include "game/objects/object_setup.h"
#include "main/objanim_update.h"

extern ObjectDescriptor gAreaFXEmitObjDescriptor;

#define AREAFXEMIT_DLL_ID                    0x0130
#define AREAFXEMIT_CLASS_ID                  0x0067
#define AREAFXEMIT_DEF_ID                    0x05A8
#define TAREAFXEMIT_DEF_ID                   0x05AA
#define AREAFXEMIT_OBJECT_DEF_BYTES          0xA0
#define AREAFXEMIT_PLACEMENT_BYTES           0x28
#define AREAFXEMIT_EXTRA_STATE_BYTES         0x20
#define AREAFXEMIT_SPAWN_LOCAL_WORLD         0
#define AREAFXEMIT_SPAWN_OBJECT_RESOURCE     1
#define AREAFXEMIT_SPAWN_OBJECT_RESOURCE_ALT 2
#define AREAFXEMIT_SPAWN_LOCAL_OBJECT        3
#define AREAFXEMIT_APPROACH_BURST_COUNT      0x23

typedef struct AreaFxEmitObject AreaFxEmitObject;
typedef int (*AreaFxEmitSeqCallback)(AreaFxEmitObject* obj, int unused, ObjAnimUpdateState* animUpdate);

typedef struct AreaFxEmitPlacement
{
    ObjPlacement base;
    s8 triggerRadius;
    s8 initialRoll;
    s8 initialPitch;
    s8 initialYaw;
    u8 extentX;
    u8 extentZ;
    u8 extentY;
    u8 emitType;
    u16 effectId;
    s16 emitCount;
    s16 enableBit;
    s16 stopBit;
} AreaFxEmitPlacement;

/*
 * Per-object extra state for the areafxemit volume particle emitter
 * (AreaFxEmit_getExtraSize == 0x20).
 */
typedef struct AreaFxEmitState
{
    f32 triggerRadius; /* (s8)setup[0x18] << 2; sentinel value = always emit */
    f32 lastDistance;  /* player distance at the last emit check */
    u8 emitType;       /* setup[0x1f]; 4/6 = world-positioned spawn (flag 0x200001), >3 bursts on approach */
    u8 pad09;
    u16 effectId;
    s16 emitCount; /* setup+0x22; >0: spawns per emit; <=0: negated re-emit cooldown; 0 also suppresses */
    s16 enableBit; /* gamebit gate, -1 = always on */
    s16 stopBit;   /* gamebit; once set the emitter suppresses */
    s16 suppressed;
    u16 extentX;       /* setup[0x1c] << 2 -- random offset half-extents */
    u16 extentZ;       /* setup[0x1d] << 2 */
    u16 extentY;       /* setup[0x1e] << 2 */
    s16 emitAngles[3]; /* yaw/pitch/roll, mirrored to obj+0/2/4 */
} AreaFxEmitState;

struct AreaFxEmitObject
{
    ObjAnimComponent objAnim;
    u16 objectFlags;
    u8 padB2[0xB8 - 0xB2];
    AreaFxEmitState* state;
    AreaFxEmitSeqCallback seqCallback;
    u8 padC0[0xF4 - 0xC0];
    s32 emitCooldown;
};

STATIC_ASSERT(sizeof(AreaFxEmitPlacement) == AREAFXEMIT_PLACEMENT_BYTES);
STATIC_ASSERT(offsetof(AreaFxEmitPlacement, triggerRadius) == 0x18);
STATIC_ASSERT(offsetof(AreaFxEmitPlacement, initialRoll) == 0x19);
STATIC_ASSERT(offsetof(AreaFxEmitPlacement, initialPitch) == 0x1A);
STATIC_ASSERT(offsetof(AreaFxEmitPlacement, initialYaw) == 0x1B);
STATIC_ASSERT(offsetof(AreaFxEmitPlacement, extentX) == 0x1C);
STATIC_ASSERT(offsetof(AreaFxEmitPlacement, extentZ) == 0x1D);
STATIC_ASSERT(offsetof(AreaFxEmitPlacement, extentY) == 0x1E);
STATIC_ASSERT(offsetof(AreaFxEmitPlacement, emitType) == 0x1F);
STATIC_ASSERT(offsetof(AreaFxEmitPlacement, effectId) == 0x20);
STATIC_ASSERT(offsetof(AreaFxEmitPlacement, emitCount) == 0x22);
STATIC_ASSERT(offsetof(AreaFxEmitPlacement, enableBit) == 0x24);
STATIC_ASSERT(offsetof(AreaFxEmitPlacement, stopBit) == 0x26);
STATIC_ASSERT(sizeof(AreaFxEmitState) == AREAFXEMIT_EXTRA_STATE_BYTES);
STATIC_ASSERT(offsetof(AreaFxEmitState, lastDistance) == 0x04);
STATIC_ASSERT(offsetof(AreaFxEmitState, emitType) == 0x08);
STATIC_ASSERT(offsetof(AreaFxEmitState, effectId) == 0x0A);
STATIC_ASSERT(offsetof(AreaFxEmitState, emitCount) == 0x0C);
STATIC_ASSERT(offsetof(AreaFxEmitState, enableBit) == 0x0E);
STATIC_ASSERT(offsetof(AreaFxEmitState, stopBit) == 0x10);
STATIC_ASSERT(offsetof(AreaFxEmitState, suppressed) == 0x12);
STATIC_ASSERT(offsetof(AreaFxEmitState, extentX) == 0x14);
STATIC_ASSERT(offsetof(AreaFxEmitState, extentZ) == 0x16);
STATIC_ASSERT(offsetof(AreaFxEmitState, extentY) == 0x18);
STATIC_ASSERT(offsetof(AreaFxEmitState, emitAngles) == 0x1A);
STATIC_ASSERT(offsetof(AreaFxEmitObject, objAnim) == 0x00);
STATIC_ASSERT(offsetof(AreaFxEmitObject, state) == 0xB8);
STATIC_ASSERT(offsetof(AreaFxEmitObject, seqCallback) == 0xBC);
STATIC_ASSERT(offsetof(AreaFxEmitObject, emitCooldown) == 0xF4);

void areafxemit_emitBurst(AreaFxEmitObject* obj, int count);
void areafxemit_emitEffect(AreaFxEmitObject* obj);

int AreaFxEmit_getExtraSize(void);
int AreaFxEmit_getObjectTypeId(void);
void AreaFxEmit_free(AreaFxEmitObject* obj);
void AreaFxEmit_render(int p1, int p2, int p3, int p4, int p5, s8 visible);
void AreaFxEmit_hitDetect(void);
void AreaFxEmit_update(AreaFxEmitObject* obj);
void AreaFxEmit_init(AreaFxEmitObject* obj, AreaFxEmitPlacement* setup);
void AreaFxEmit_release(void);
void AreaFxEmit_initialise(void);

#endif /* MAIN_DLL_CF_CFCHUCKOBJ_H_ */
