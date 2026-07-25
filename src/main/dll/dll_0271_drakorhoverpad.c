/*
 * drakorhoverpad (DLL 0x271) - a rideable hover-pad object in the
 * Drakor (DR) levels that follows a ROM spline/curve network.
 *
 * initMain seeds the pad onto its curve and selects a behaviour mode
 * from its placement subtype; updateMain advances the pad along the
 * active curve each step, applying a sinusoidal vertical bob, banking
 * the model toward its travel direction, and steering the object
 * toward the curve sample point. update() picks the next path point in
 * the network (masked vs unmasked branch) and recomputes the per-node
 * velocity/tangent data. handlePathPointEvent dispatches the per-node
 * event ids: speed flips, state changes, camera shake / view offset
 * while the player is riding, and the game bits that gate the ride.
 * render emits the trailing particle spray on a frame cadence.
 *
 * Curve/velocity state lives in the object's extra block
 * (DrakorHoverpadState, 0x17c bytes); the two flag bytes at 0x178/0x179
 * are DrakorHoverpadFlags / DrakorHoverpadPathFlags.
 */
#include "main/dll/dll_0271_drakorhoverpad.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "string.h"
#include "main/curve.h"
#include "dolphin/mtx/mtx_legacy.h"
#include "main/camera.h"
#include "main/camera_shake_api.h"
#include "main/frame_timing.h"
#include "main/gamebits_api.h"
#include "main/obj_group.h"
#include "main/obj_path.h"
#include "main/obj_query.h"
#include "main/objhits.h"
#include "main/object_api.h"
#include "main/object_render.h"
#include "main/objprint_api.h"
#include "main/vecmath.h"
#include "main/audio/sfx_play_api.h"
#include "main/dll/dll_0282_barrelgener.h"
#include "main/dll/rom_curve_interface.h"
#include "main/game_object.h"
#include "main/audio/sfx_ids.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/object_descriptor.h"
#include "main/dll/dll_0271_drakorhoverpad_internal.h"

const f32 gDrakorHoverpadSpeedStep = 2.0f;

f32 lbl_803DC2F8 = 5.0f;
s16 lbl_803DC2FC = 3;
f32 lbl_803DC300 = 5.0f;
f32 lbl_803DC304 = -40.0f;

void drakorhoverpad_resetPendingMotion(GameObject* obj)
{
    u8* p = obj->extra;
    DrakorHoverpadPathFlags* g = (DrakorHoverpadPathFlags*)(p + 0x179);
    if (g->p6 != 0)
    {
        g->p6 = 0;
        ((DrakorHoverpadState*)p)->commandSpeed = (*(f32*)&gDrakorHoverpadSpeedStep);
    }
}

void drakorhoverpad_func17(GameObject* obj, int sel, int* out)
{
    switch (sel)
    {
    case 2:
        *out = obj->anim.rotX;
        break;
    case 3:
        *out = 0x1000;
        break;
    case 4:
        *out = 1;
        break;
    }
}

void drakorhoverpad_renderGroundMarker(GameObject* obj, f32 scale)
{
    f32* mtx;
    MatrixTransform pos;
    mtx = (f32*)ObjPath_GetPointModelMtx(obj, 0);
    pos.x = 0.0f;
    pos.y = 10.0f;
    pos.z = 0.0f;
    pos.rotX = 0;
    pos.rotY = 0;
    pos.rotZ = 0;
    pos.scale = scale / (obj)->anim.modelInstance->rootMotionScaleBase;
    setMatrixFromObjectPos(gDrakorHoverpadMtx, &pos);
    mtx44_mult(gDrakorHoverpadMtx, mtx, gDrakorHoverpadMtx);
    objSetModelMatrixOverride(gDrakorHoverpadMtx);
}

void drakorhoverpad_func15(void)
{
}

int drakorhoverpad_func14(void)
{
    return 0x0;
}

f32 drakorhoverpad_func13(int obj, f32* out)
{
    *out = 5.0f;
    return 0.0f;
}

void drakorhoverpad_func12(int obj, f32* outFloat, int* outFlag)
{
    *outFloat = 0.0f;
    *outFlag = 0;
}

void drakorhoverpad_func11(void)
{
}

int drakorhoverpad_func10(void)
{
    return 0x0;
}

void drakorhoverpad_func0F(int obj, f32* ox, f32* oy, f32* oz)
{
    MatrixTransform pos;
    f32 mtx[16];
    GameObject* src = Obj_GetPlayerObject();
    if (src == NULL)
    {
        src = (GameObject*)obj;
    }
    pos.x = src->anim.localPosX;
    pos.y = src->anim.localPosY;
    pos.z = src->anim.localPosZ;
    pos.rotX = src->anim.rotX;
    pos.rotY = src->anim.rotY;
    pos.rotZ = src->anim.rotZ;
    pos.scale = 1.0f;
    setMatrixFromObjectPos(mtx, &pos);
    Matrix_TransformPoint(mtx, 0.0f, lbl_803DC300, lbl_803DC304, ox, oy, oz);
}

static inline f32 drakorhoverpad_nodeWobbleSin(DrakorCurveNode** slot, int angle)
{
    return (*(f32*)&gDrakorHoverpadSpeedStep) * ((f32)(u32)(*slot)->tangentMag * mathSinf(3.1415927f * (f32)angle / 32768.0f));
}

static inline f32 drakorhoverpad_nodeWobbleCos(DrakorCurveNode** slot, int angle)
{
    return (*(f32*)&gDrakorHoverpadSpeedStep) * ((f32)(u32)(*slot)->tangentMag * mathCosf(3.1415927f * (f32)angle / 32768.0f));
}

int drakorhoverpad_func0E(void)
{
    return 0x1;
}

int drakorhoverpad_render2(GameObject* obj)
{
    u8* p = obj->extra;
    return ((p[0x179] >> 2) & 1) == 0;
}

void drakorhoverpad_modelMtxFn(GameObject* obj, f32* ox, f32* oy, f32* oz)
{
    *ox = obj->anim.localPosX;
    *oy = 10.0f + obj->anim.localPosY;
    *oz = obj->anim.localPosZ;
}

int drakorhoverpad_func0B(void)
{
    return 0x1;
}

int drakorhoverpad_setScale(GameObject* obj)
{
    u8* p = obj->extra;
    return (p[0x179] >> 2) & 1;
}

int drakorhoverpad_pickMaskedNextPoint(int* pad, int exclude, int maxIndex);

int drakorhoverpad_pickUnmaskedNextPoint(int* pad, int exclude, int maxIndex);

int drakorhoverpad_pickMaskedNextPoint(int* pad, int exclude, int maxIndex)
{
    int collected[4];
    int pt;
    int count;
    u32 bit;
    int i;

    count = 0;
    bit = 1;
    for (i = 0; i < 4; i++)
    {
        pt = pad[7 + i];
        if (pt > -1 && (*(s8*)((char*)pad + 0x1b) & bit) != 0 && pt != exclude)
        {
            collected[count++] = pt;
        }
        bit <<= 1;
    }
    if (count != 0)
    {
        if (maxIndex != -1 && maxIndex > count - 1)
        {
            maxIndex = count - 1;
        }
        if (maxIndex == -1)
        {
            maxIndex = randomGetRange(0, count - 1);
        }
        return collected[maxIndex];
    }
    return -1;
}

int drakorhoverpad_pickUnmaskedNextPoint(int* pad, int exclude, int maxIndex)
{
    int collected[4];
    int pt;
    int count;
    u32 bit;
    int i;

    count = 0;
    bit = 1;
    for (i = 0; i < 4; i++)
    {
        pt = pad[7 + i];
        if (pt > -1 && (*(s8*)((char*)pad + 0x1b) & bit) == 0 && pt != exclude)
        {
            collected[count++] = pt;
        }
        bit <<= 1;
    }
    if (count != 0)
    {
        if (maxIndex != -1 && maxIndex > count - 1)
        {
            maxIndex = count - 1;
        }
        if (maxIndex == -1)
        {
            maxIndex = randomGetRange(0, count - 1);
        }
        return collected[maxIndex];
    }
    return -1;
}

int drakorhoverpad_update(RomCurveWalker* curve, int maxIndex)
{
    u8* p = (u8*)curve;
    u8* cur;
    int result;

    if (curve == NULL)
    {
        return 1;
    }
    cur = *(u8**)&((GameObject*)p)->anim.currentMove;
    if (cur == NULL || ((GameObject*)p)->anim.targetObj == NULL)
    {
        return 1;
    }
    *(u8**)&((GameObject*)p)->anim.activeMoveProgress = cur;
    *(u8**)&((GameObject*)p)->anim.currentMove = *(u8**)&((GameObject*)p)->anim.targetObj;
    memcpy(p + 0xa8, p + 0xb8, 16);
    memcpy(p + 0xc8, p + 0xd8, 16);
    memcpy(p + 0xe8, p + 0xf8, 16);
    if (*(int*)&((GameObject*)p)->anim.previousLocalPosX != 0)
    {
        result = drakorhoverpad_pickMaskedNextPoint(*(int**)&((GameObject*)p)->anim.currentMove, -1, maxIndex);
    }
    else
    {
        result = drakorhoverpad_pickUnmaskedNextPoint(*(int**)&((GameObject*)p)->anim.currentMove, -1, maxIndex);
    }
    if (result != -1)
    {
        ((GameObject*)p)->anim.targetObj = (*gRomCurveInterface)->getById(result);
        if (((GameObject*)p)->anim.targetObj != NULL)
        {
#define CM_SLOT  ((DrakorCurveNode**)&((GameObject*)p)->anim.currentMove)
#define AMP_SLOT ((DrakorCurveNode**)&((GameObject*)p)->anim.activeMoveProgress)
#define TGT_SLOT ((DrakorCurveNode**)&((GameObject*)p)->anim.targetObj)
#define CM_NODE  (*CM_SLOT)
#define AMP_NODE (*AMP_SLOT)
#define TGT_NODE (*TGT_SLOT)
    if (*(int*)&((GameObject*)p)->anim.previousLocalPosX != 0)
    {
        *(f32*)&((GameObject*)p)->extra = CM_NODE->x;
        *(f32*)&((GameObject*)p)->animEventCallback = AMP_NODE->x;
        *(f32*)&((GameObject*)p)->pendingParentObj =
            drakorhoverpad_nodeWobbleSin(CM_SLOT, CM_NODE->tangentYaw << 8);
        *(f32*)&((GameObject*)p)->ownerObj =
            drakorhoverpad_nodeWobbleSin(AMP_SLOT, AMP_NODE->tangentYaw << 8);
        *(f32*)&((GameObject*)p)->childObjs[4] = CM_NODE->y;
        *(f32*)&((GameObject*)p)->unkDC = AMP_NODE->y;
        *(f32*)&((GameObject*)p)->unkE0[0] =
            drakorhoverpad_nodeWobbleSin(CM_SLOT, CM_NODE->tangentPitch << 8);
        *(f32*)&((GameObject*)p)->hitVolumeIndex =
            drakorhoverpad_nodeWobbleSin(AMP_SLOT, AMP_NODE->tangentPitch << 8);
        *(f32*)&((GameObject*)p)->userData2 = CM_NODE->z;
        ((GameObject*)p)->externalVelX = AMP_NODE->z;
        ((GameObject*)p)->externalVelY =
            drakorhoverpad_nodeWobbleCos(CM_SLOT, CM_NODE->tangentYaw << 8);
        ((GameObject*)p)->externalVelZ =
            drakorhoverpad_nodeWobbleCos(AMP_SLOT, AMP_NODE->tangentYaw << 8);
    }
    else
    {
        *(f32*)&((GameObject*)p)->extra = CM_NODE->x;
        *(f32*)&((GameObject*)p)->animEventCallback = TGT_NODE->x;
        *(f32*)&((GameObject*)p)->pendingParentObj =
            drakorhoverpad_nodeWobbleSin(CM_SLOT, CM_NODE->tangentYaw << 8);
        *(f32*)&((GameObject*)p)->ownerObj =
            drakorhoverpad_nodeWobbleSin(TGT_SLOT, TGT_NODE->tangentYaw << 8);
        *(f32*)&((GameObject*)p)->childObjs[4] = CM_NODE->y;
        *(f32*)&((GameObject*)p)->unkDC = TGT_NODE->y;
        *(f32*)&((GameObject*)p)->unkE0[0] =
            drakorhoverpad_nodeWobbleSin(CM_SLOT, CM_NODE->tangentPitch << 8);
        *(f32*)&((GameObject*)p)->hitVolumeIndex =
            drakorhoverpad_nodeWobbleSin(TGT_SLOT, TGT_NODE->tangentPitch << 8);
        *(f32*)&((GameObject*)p)->userData2 = CM_NODE->z;
        ((GameObject*)p)->externalVelX = TGT_NODE->z;
        ((GameObject*)p)->externalVelY =
            drakorhoverpad_nodeWobbleCos(CM_SLOT, CM_NODE->tangentYaw << 8);
        ((GameObject*)p)->externalVelZ =
            drakorhoverpad_nodeWobbleCos(TGT_SLOT, TGT_NODE->tangentYaw << 8);
    }
#undef CM_NODE
#undef AMP_NODE
#undef TGT_NODE
#undef CM_SLOT
#undef AMP_SLOT
#undef TGT_SLOT
    if (*(int*)&((GameObject*)p)->anim.previousWorldPosY != 0)
    {
        curvesSetupMoveNetworkCurve(&curve->curve);
    }
    if (*(int*)&((GameObject*)p)->anim.previousLocalPosX != 0)
    {
        Curve_AdvanceAlongPath(&curve->curve, -1.0f);
    }
    else
    {
        Curve_AdvanceAlongPath(&curve->curve, 1.0f);
    }
    return 0;
        }
    }
    else
    {
        ((GameObject*)p)->anim.targetObj = NULL;
    }
    return 1;
}

ObjectDescriptor24 gDrakorHoverPadObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_24_SLOTS,
    (ObjectDescriptorCallback)drakorhoverpad_initialise,
    (ObjectDescriptorCallback)drakorhoverpad_release,
    0,
    (ObjectDescriptorCallback)drakorhoverpad_initMain,
    (ObjectDescriptorCallback)drakorhoverpad_updateMain,
    (ObjectDescriptorCallback)drakorhoverpad_hitDetect,
    (ObjectDescriptorCallback)drakorhoverpad_render,
    (ObjectDescriptorCallback)drakorhoverpad_free,
    (ObjectDescriptorCallback)drakorhoverpad_getObjectTypeId,
    (ObjectDescriptorExtraSizeCallback)drakorhoverpad_getExtraSize,
    (ObjectDescriptorCallback)drakorhoverpad_setScale,
    (ObjectDescriptorCallback)drakorhoverpad_func0B,
    (ObjectDescriptorCallback)drakorhoverpad_modelMtxFn,
    (ObjectDescriptorCallback)drakorhoverpad_render2,
    (ObjectDescriptorCallback)drakorhoverpad_func0E,
    (ObjectDescriptorCallback)drakorhoverpad_func0F,
    (ObjectDescriptorCallback)drakorhoverpad_func10,
    (ObjectDescriptorCallback)drakorhoverpad_func11,
    (ObjectDescriptorCallback)drakorhoverpad_func12,
    (ObjectDescriptorCallback)drakorhoverpad_func13,
    (ObjectDescriptorCallback)drakorhoverpad_func14,
    (ObjectDescriptorCallback)drakorhoverpad_func15,
    (ObjectDescriptorCallback)drakorhoverpad_renderGroundMarker,
    (ObjectDescriptorCallback)drakorhoverpad_func17,
};
int drakorhoverpad_init(GameObject* obj)
{
    u8* p = (obj)->extra;
    DrakorHoverpadFlags* f = (DrakorHoverpadFlags*)(p + 0x178);

    if (f->b40 == 0)
    {
        if (f->state > 3)
        {
            if (0.0f == ((DrakorHoverpadState*)p)->speed)
            {
                f->state = 0;
            }
        }
    }
    if (f->b01 != mainGetBit(1654))
    {
        f->b01 ^= 1;
        ((DrakorHoverpadState*)p)->commandSpeed = -((DrakorHoverpadState*)p)->commandSpeed;
        if (f->state == 3)
        {
            f->state = 0;
            ((DrakorHoverpadState*)p)->commandSpeed = (*(f32*)&gDrakorHoverpadSpeedStep);
        }
        if (f->state == 4)
        {
            f->state = 0;
            ((DrakorHoverpadState*)p)->commandSpeed = -2.0f;
        }
        if (f->b40 != 0)
        {
            if (0.0f == ((DrakorHoverpadState*)p)->commandSpeed)
            {
                ((DrakorHoverpadState*)p)->commandSpeed = (f->b01 != 0) ? -2.0f : (*(f32*)&gDrakorHoverpadSpeedStep);
            }
        }
        Sfx_PlayFromObject((int)obj, SFXTRIG_id_309);
    }
    return 0;
}

int drakorhoverpad_handlePathPointEvent(GameObject* obj, u8 eventCode, u8 subCode, void* out)
{
    u8* p = (obj)->extra;
    DrakorHoverpadFlags* f = (DrakorHoverpadFlags*)(p + 0x178);
    DrakorHoverpadPathFlags* g = (DrakorHoverpadPathFlags*)(p + 0x179);
    int player;
    f32 shakeMag;
    f32 absP;
    f32 cur;
    f32 half;

    half = 0.5f;
    player = (int)Obj_GetPlayerObject();
    *(int*)out = -1;
    switch (eventCode)
    {
    case 1:
        player = (int)Obj_GetPlayerObject();
        ((DrakorHoverpadHandlePathPointEventState*)p)->verticalVel =
            0.8f * -((DrakorHoverpadHandlePathPointEventState*)p)->verticalVel;
        ((DrakorHoverpadState*)p)->commandSpeed = 0.0f;
        if (((GameObject*)player)->anim.parent == (void*)obj)
        {
            Camera_EnableViewYOffset();
            if (((DrakorHoverpadHandlePathPointEventState*)p)->verticalVel >= 0.0f)
            {
                shakeMag = ((DrakorHoverpadHandlePathPointEventState*)p)->verticalVel;
            }
            else
            {
                shakeMag = -((DrakorHoverpadHandlePathPointEventState*)p)->verticalVel;
            }
            CameraShake_SetAllMagnitudes(shakeMag);
        }
        break;
    case 3:
        if (f->b40 != 0)
        {
            break;
        }
        if (((DrakorHoverpadHandlePathPointEventState*)p)->verticalVel <= 0.0f)
        {
            break;
        }
        if (f->bit80 != 0)
        {
            break;
        }
        player = (int)Obj_GetPlayerObject();
        ((DrakorHoverpadHandlePathPointEventState*)p)->verticalVel =
            0.8f * -((DrakorHoverpadHandlePathPointEventState*)p)->verticalVel;
        ((DrakorHoverpadState*)p)->commandSpeed = 0.0f;
        if (((GameObject*)player)->anim.parent == (void*)obj)
        {
            Camera_EnableViewYOffset();
            if (((DrakorHoverpadHandlePathPointEventState*)p)->verticalVel >= 0.0f)
            {
                shakeMag = ((DrakorHoverpadHandlePathPointEventState*)p)->verticalVel;
            }
            else
            {
                shakeMag = -((DrakorHoverpadHandlePathPointEventState*)p)->verticalVel;
            }
            CameraShake_SetAllMagnitudes(shakeMag);
        }
        return 1;
    case 4:
        if (((DrakorHoverpadHandlePathPointEventState*)p)->verticalVel <= 0.0f)
        {
            break;
        }
        if (f->b40 != 0)
        {
            mainSetBits(0x660, 1);
        }
        else if (mainGetBit(0x661) == 0)
        {
            mainSetBits(0x788, 1);
            f->state = 1;
            ((DrakorHoverpadState*)p)->commandSpeed = 0.0f;
        }
        else
        {
            ((DrakorHoverpadHandlePathPointEventState*)p)->targetSpeed +=
                (((DrakorHoverpadState*)p)->commandSpeed < 0.0f) ? -2.0f : (*(f32*)&gDrakorHoverpadSpeedStep);
        }
        break;
    case 9:
        if (((DrakorHoverpadHandlePathPointEventState*)p)->verticalVel >= 0.0f)
        {
            break;
        }
        if (mainGetBit(0x661) == 0)
        {
            f->state = 1;
            ((DrakorHoverpadState*)p)->commandSpeed = 0.0f;
        }
        else
        {
            ((DrakorHoverpadHandlePathPointEventState*)p)->targetSpeed +=
                (((DrakorHoverpadState*)p)->commandSpeed < 0.0f) ? -2.0f : (*(f32*)&gDrakorHoverpadSpeedStep);
        }
        break;
    case 5:
        if (f->b40 != 0)
        {
            break;
        }
        f->state = 2;
        break;
    case 6:
        if (f->b40 != 0)
        {
            break;
        }
        ((DrakorHoverpadHandlePathPointEventState*)p)->targetSpeed +=
            (((DrakorHoverpadState*)p)->commandSpeed < 0.0f) ? -3.0f : 3.0f;
        break;
    case 7:
        if (((DrakorHoverpadState*)p)->commandSpeed <= 0.0f)
        {
            f->state = 3;
            ((DrakorHoverpadState*)p)->commandSpeed = 0.0f;
            Sfx_PlayFromObject((int)obj, SFXTRIG_id_30b);
        }
        break;
    case 17:
        if (((DrakorHoverpadState*)p)->commandSpeed >= 0.0f)
        {
            f->state = 4;
            ((DrakorHoverpadState*)p)->commandSpeed = 0.0f;
            Sfx_PlayFromObject((int)obj, SFXTRIG_id_30b);
        }
        break;
    case 10:
        if (g->p1 == 0)
        {
            break;
        }
        if (mainGetBit(0x689) != 0)
        {
            break;
        }
        mainSetBits(0x689, 1);
        break;
    case 11:
        if (g->p1 == 0)
        {
            break;
        }
        if (((GameObject*)player)->anim.parent != (void*)obj)
        {
            break;
        }
        mainSetBits(0x68a, 1);
        break;
    case 12:
        if (g->p1 == 0)
        {
            break;
        }
        if (((GameObject*)player)->anim.parent != (void*)obj)
        {
            break;
        }
        mainSetBits(0x68b, 1);
        break;
    case 13:
        if (mainGetBit(0x68a) == 0)
        {
            break;
        }
        if (((DrakorHoverpadState*)p)->commandSpeed >= 0.0f)
        {
            player = (int)Obj_GetPlayerObject();
            ((DrakorHoverpadHandlePathPointEventState*)p)->verticalVel =
                0.8f * -((DrakorHoverpadHandlePathPointEventState*)p)->verticalVel;
            ((DrakorHoverpadState*)p)->commandSpeed = 0.0f;
            if (((GameObject*)player)->anim.parent == (void*)obj)
            {
                Camera_EnableViewYOffset();
                if (((DrakorHoverpadHandlePathPointEventState*)p)->verticalVel >= 0.0f)
                {
                    shakeMag = ((DrakorHoverpadHandlePathPointEventState*)p)->verticalVel;
                }
                else
                {
                    shakeMag = -((DrakorHoverpadHandlePathPointEventState*)p)->verticalVel;
                }
                CameraShake_SetAllMagnitudes(shakeMag);
            }
        }
        break;
    case 14:
        if (g->p1 == 0)
        {
            break;
        }
        if (((DrakorHoverpadState*)p)->commandSpeed <= 0.0f)
        {
            player = (int)Obj_GetPlayerObject();
            ((DrakorHoverpadHandlePathPointEventState*)p)->verticalVel =
                0.8f * -((DrakorHoverpadHandlePathPointEventState*)p)->verticalVel;
            ((DrakorHoverpadState*)p)->commandSpeed = 0.0f;
            if (((GameObject*)player)->anim.parent == (void*)obj)
            {
                Camera_EnableViewYOffset();
                if (((DrakorHoverpadHandlePathPointEventState*)p)->verticalVel >= 0.0f)
                {
                    shakeMag = ((DrakorHoverpadHandlePathPointEventState*)p)->verticalVel;
                }
                else
                {
                    shakeMag = -((DrakorHoverpadHandlePathPointEventState*)p)->verticalVel;
                }
                CameraShake_SetAllMagnitudes(shakeMag);
            }
        }
        break;
    case 15:
        if (f->b40 != 0)
        {
            break;
        }
        mainSetBits(0x788, 1);
        break;
    case 16:
        cur = ((DrakorHoverpadState*)p)->commandSpeed;
        if (cur >= 0.0f)
        {
            absP = cur;
        }
        else
        {
            absP = -cur;
        }
        if ((*(f32*)&gDrakorHoverpadSpeedStep) == absP)
        {
            ((DrakorHoverpadState*)p)->commandSpeed = cur * half;
        }
        else
        {
            ((DrakorHoverpadState*)p)->commandSpeed = (*(f32*)&gDrakorHoverpadSpeedStep) * cur;
        }
        Sfx_PlayFromObject((int)obj, SFXTRIG_id_309);
        break;
    case 20:
        g->f10 = !g->f10;
        break;
    case 21:
        g->p6 = 1;
        ((DrakorHoverpadState*)p)->commandSpeed = 0.0f;
        break;
    }
    switch (subCode)
    {
    case 8:
        if (mainGetBit(0x67f) != 0)
        {
            *(int*)out = 1;
        }
        else
        {
            *(int*)out = 0;
        }
        break;
    case 2:
        mainSetBits(0x7ba, 1);
        break;
    case 18:
        *(int*)out = 0;
        break;
    case 19:
        *(int*)out = 1;
        break;
    }
    return 1;
}

int drakorhoverpad_getExtraSize(void)
{
    return 0x17c;
}

int drakorhoverpad_getObjectTypeId(void)
{
    return 0x0;
}

void drakorhoverpad_free(int obj)
{
    ObjGroup_RemoveObject(obj, DRAKORHOVERPAD_OBJGROUP);
    ObjGroup_RemoveObject(obj, DRAKORHOVERPAD_OBJGROUP_SECONDARY);
}

void drakorhoverpad_render(GameObject* obj, int p2, int p3, int p4, int p5, char visible)
{
    u8* p = (obj)->extra;
    if (visible)
    {
        objRenderModelAndHitVolumes(obj, p2, p3, p4, p5, 1.0f);
        ((DrakorHoverpadRenderState*)p)->frameCounter += framesThisStep;
        if (((DrakorHoverpadRenderState*)p)->frameCounter == 0 || ((DrakorHoverpadRenderState*)p)->frameCounter > 10)
        {
            ((DrakorHoverpadRenderState*)p)->frameCounter = 0;
            ((DrakorHoverpadRenderState*)p)->particleEmitAX = (obj)->anim.localPosX + (f32)(int)randomGetRange(-30, 30);
            ((DrakorHoverpadRenderState*)p)->particleEmitAY = (obj)->anim.localPosY;
            ((DrakorHoverpadRenderState*)p)->particleEmitAZ = (obj)->anim.localPosZ + (f32)(int)randomGetRange(-30, 30);
            ((DrakorHoverpadRenderState*)p)->particleEmitBX =
                (obj)->anim.localPosX + (f32)(int)randomGetRange(-120, 120);
            ((DrakorHoverpadRenderState*)p)->particleEmitBY = (obj)->anim.localPosY - 40.0f;
            ((DrakorHoverpadRenderState*)p)->particleEmitBZ =
                (obj)->anim.localPosZ + (f32)(int)randomGetRange(-120, 120);
        }
    }
}

void drakorhoverpad_hitDetect(void)
{
}
