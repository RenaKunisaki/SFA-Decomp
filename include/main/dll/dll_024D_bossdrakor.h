#ifndef MAIN_DLL_DLL_024D_BOSSDRAKOR_H_
#define MAIN_DLL_DLL_024D_BOSSDRAKOR_H_

#include "types.h"
#include "game/objects/object.h"
#include "main/byte_flags.h"
#include "main/dll/curve_walker.h"
#include "main/objprint_sound_api.h"
#include "main/model_light.h"
#include "main/objanim_update.h"
#include "global.h"
#include "game/objects/object_setup.h"

typedef struct BossdrakorPlacement
{
    ObjPlacement base;
    u8 pad18[0x19 - 0x18];
    u8 curveAdvanceStep;
    s16 airMeterMax;
    s16 unk1C;
    s16 defeatedGameBit;
} BossdrakorPlacement;

typedef struct BossDrakorState
{
    f32 curveAdvanceStep;
    u8 pad04[8];
    int unk0C;
    f32 attackTimer;
    f32 attackTimerDuration;
    f32 jawAnimAngle;
    union {
        struct {
            f32 homePosX;
            f32 homePosY;
            f32 homePosZ;
        };
        Vec3f homePos;
    };
    RomCurveWalker curveWalker; /* 0x28: the rom-curve walker this boss follows */
    ObjSoundState soundState; /* 0x130 */
    ModelLightStruct* lightObj; /* 0x160 */
    f32 moveSpeed;
    int moveState; /* 0x168 */
    int unk16C;
    int airMeterHandle;
    int attackType;
    f32 shakeAmount;
    f32 shakeVel;
    f32 shakeScaleZ;
    f32 missileBaseSpeed;  /* 0x184: base missile speed (constant term of spd); also scales missile lateral vel */
    f32 missileLeadFactor; /* 0x188: coefficient on dot(playerVel, dir) added to base speed (target-lead) */
    f32 textTimer;
    u8 repeatCount;
    u8 pad191[3];
    int curveFollowState;
    ByteFlags flags198;
    u8 pad199[3];
    f32 hitSfxCooldown;
    f32 hurtSfxCooldown;
} BossDrakorState;

STATIC_ASSERT(offsetof(BossDrakorState, curveWalker) == 0x28);
STATIC_ASSERT(offsetof(BossDrakorState, soundState) == 0x130);
STATIC_ASSERT(sizeof(BossDrakorState) == 0x1a4);

extern f32 lbl_803E6510;
extern f32 lbl_803E6514;
extern f32 lbl_803E6518;
extern f32 lbl_803E651C;
extern f32 lbl_803E6520;
extern f32 gBossDrakorDegToAngle;
extern f32 lbl_803E6534;
extern f32 lbl_803E6538;
extern f32 lbl_803E653C;
extern f32 lbl_803E6540;
extern f32 lbl_803E6544;
extern f32 lbl_803E6548;
extern f32 lbl_803E654C;
extern f32 lbl_803E6550;
extern f32 lbl_803E6554;
extern f32 lbl_803E6558;
extern f32 lbl_803E655C;
extern f32 lbl_803E6560;
extern f32 lbl_803E6564;
extern f32 lbl_803E6568;
extern f32 lbl_803E656C;
extern f32 lbl_803E6570;
extern f32 lbl_803E6574;
extern f32 lbl_803E6578;
extern f32 lbl_803E657C;

extern f32 lbl_803DC188;
extern f32 lbl_803DC18C;
extern f32 lbl_803DC190;
extern f32 lbl_803DC194;
extern s16 lbl_803DC198;
extern s16 lbl_803DC19A;

extern int gBossDrakorMoveStateTable[];
extern int gBossDrakorMoveSpeedTable[];
typedef struct BossDrakorTuning
{
    int turnMoveStates[5];
    f32 unk14[9];
    int unk38[9];
    f32 missileBaseSpeeds[3];
    f32 missileLeadFactors[3];
    int unk74[3];
} BossDrakorTuning;

STATIC_ASSERT(sizeof(BossDrakorTuning) == 0x80);
STATIC_ASSERT(offsetof(BossDrakorTuning, unk14) == 0x14);
STATIC_ASSERT(offsetof(BossDrakorTuning, unk38) == 0x38);
STATIC_ASSERT(offsetof(BossDrakorTuning, missileBaseSpeeds) == 0x5c);
STATIC_ASSERT(offsetof(BossDrakorTuning, missileLeadFactors) == 0x68);
STATIC_ASSERT(offsetof(BossDrakorTuning, unk74) == 0x74);

extern BossDrakorTuning gBossDrakorTurnMoveStates;

void bossdrakor_release(void);
void bossdrakor_initialise(void);
int bossdrakor_getExtraSize(void);
void bossdrakor_update(GameObject* obj);
void bossdrakor_free(GameObject* obj);
void bossdrakor_hitDetect(GameObject* obj);
int bossdrakor_seqFn(GameObject* obj, int unused, ObjAnimUpdateState* animUpdate);
void bossdrakor_handleActionEvent(GameObject* obj, BossDrakorState* state, int action);
void bossdrakor_updateHeadTracking(GameObject* obj, BossDrakorState* state);
int bossdrakor_chooseNextMove(GameObject* obj, f32* speedOut);
void bossdrakor_spawnAttackObjects(GameObject* obj, BossDrakorState* state, int action);
void bossdrakor_init(GameObject* obj, BossdrakorPlacement* init);
void bossdrakor_render(GameObject* p1, int p2, int p3, int p4, int p5, s8 vis);

#endif
