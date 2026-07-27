/*
 * DFSH_LaserB (DLL 0x17B) - the Dragon Rock shrine's sweeping/pulsing
 * laser-beam hazard: it tracks the player, animates beam
 * geometry and texture, drives sfx channels and proximity damage.
 */
#include "main/dll/modgfx_interface.h"
#include "main/dll/partfx_interface.h"
#include "main/audio/sfx_channel_volume_api.h"
#include "main/audio/sfx_play_api.h"
#include "main/audio/sfx_stop_channel_api.h"
#include "main/frame_timing.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_trig_api.h"
#include "main/vecmath_distance_api.h"
#include "main/vecmath.h"
#include "game/objects/object.h"
#include "main/dll/player_api.h"
#include "main/dll/tricky_api.h"
#include "sys/objects.h"
#include "main/pad.h"
#include "main/resource.h"
#include "main/audio/sfx_ids.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/gamebits.h"
#include "main/texture.h"
#include "main/obj_message.h"
#include "main/gamebit_ids.h"
#include "dlls/object_descriptor.h"
#include "game/objects/object_setup.h"
#include "main/dll/dll_017B_dfshlaserbeam.h"
#include "main/dll/foodbag.h"
#include "main/dll/dll_19E.h"

typedef struct DFSHLaserBeamConfig
{
    ObjPlacement head;
    s8 yawByte;
    u8 proximityMode;
    s16 rangeAngle;
    u8 pad1C[0x1E - 0x1C];
    s16 disableGameBit;
} DFSHLaserBeamConfig;

typedef struct DFSHLaserBeamRuntime
{
    void* beamTexture;
    f32 swayPhase;
    f32 swayVelocity;
    f32 swayAccel;
    f32 swayTarget;
    f32 rangeBase;
    f32 rangeValue;
    f32 beamVolumeScale;
    u8 pad20[0x24 - 0x20];
    u8 beamActive;
    u8 beamBlocked;
    s8 heightWindow;
    s8 damageCooldown;
    s16 hitStrength;
    s16 blockTimer;
    s16 cycleTimer;
    s16 warmupThreshold;
    f32 hitPos[3];
    f32 hitX;
    f32 hitY;
    f32 hitZ;
    u8 modgfxAttached;
    u8 blastPhase;
    u8 proximityMode;
    u8 pad4B[0x4C - 0x4B];
} DFSHLaserBeamRuntime;

typedef struct DFSHLaserBeamObject
{
    s16 yaw;
    s16 pitch;
    s16 roll;
    s16 flags06;
    u8 pad08[0x0C - 0x08];
    f32 localPosX;
    f32 localPosY;
    f32 localPosZ;
    f32 worldPosX;
    f32 worldPosY;
    f32 worldPosZ;
    u8 pad24[0x36 - 0x24];
    u8 alpha;
    u8 pad37[0x4C - 0x37];
    DFSHLaserBeamConfig* config;
    u8 pad50[0xB8 - 0x50];
    DFSHLaserBeamRuntime* runtime;
} DFSHLaserBeamObject;

/* texture asset loaded into runtime->beamTexture */
#define DFSHLASERBEAM_TEXTURE_ID         0x2E
#define DFSHLASERBEAM_EFFECT_RESOURCE_ID 0x81

#define DFSH_MSG_PLAYER_HIT 0x60003 /* message the player on a laser hit */

Dll81Interface** gLaserBeamEffectResource;

int DFSH_LaserBeam_getExtraSize(void)
{
    return 0x4c;
}

int DFSH_LaserBeam_getObjectTypeId(void)
{
    return 0x0;
}

void DFSH_LaserBeam_free(GameObject* obj)
{
    DFSHLaserBeamRuntime* runtime = obj->extra;
    (*gModgfxInterface)->detachSource(obj);
    Resource_Release(gLaserBeamEffectResource);
    gLaserBeamEffectResource = NULL;
    if (runtime->beamTexture != NULL)
    {
        textureFree((Texture*)runtime->beamTexture);
    }
    runtime->beamTexture = NULL;
}

void DFSH_LaserBeam_render(void)
{
}

void DFSH_LaserBeam_hitDetect(void)
{
}

void DFSH_LaserBeam_update(u32 objAddr)
{
    DFSHLaserBeamConfig* config;
    DFSHLaserBeamRuntime* runtime;
    GameObject* playerObj;
    DFSHLaserBeamObject* obj;
    f32 range;
    f32 rangeSq;
    f32 yawSin;
    f32 yawCos;
    f32 heightThreshold;
    f32 beamPlane;
    f32 heightDelta;
    f32 xDelta;
    f32 zDelta;
    f32 lateralAbs;
    f32 damageDistance;
    f32 pushDistance;

    obj = (DFSHLaserBeamObject*)objAddr;
    config = obj->config;
    runtime = obj->runtime;

    runtime->cycleTimer -= framesThisStep;
    if (mainGetBit(config->disableGameBit) == 0)
    {
        if (runtime->cycleTimer < 0)
        {
            if (runtime->beamBlocked == 0)
            {
                runtime->cycleTimer = 0x190;
            Sfx_PlayFromObject((u32)obj, SFXTRIG_dn_boar1_c_78);
                runtime->beamVolumeScale = (0.0f);
            }
            else
            {
                runtime->cycleTimer = 0x113;
            }
            runtime->blastPhase = 0;
        }
        else if (runtime->cycleTimer < runtime->warmupThreshold)
        {
            if (runtime->blastPhase == 0)
            {
        Sfx_PlayFromObject((u32)obj, SFXTRIG_dn_boar1_c_79);
                if (runtime->beamBlocked == 0)
                {
        Sfx_PlayFromObject((u32)obj, SFXTRIG_dn_boar1_c_77);
                }
                runtime->blastPhase = 1;
                if (gLaserBeamEffectResource != NULL)
                {
                    (*gLaserBeamEffectResource)->spawn((int)obj, 10, NULL, 0x10004, -1, 0);
                }
            }
            if (runtime->cycleTimer < 0x28)
            {
                Sfx_StopObjectChannel((int)obj, 0x40);
                if ((runtime->beamVolumeScale >= (0.0f)) && (runtime->beamBlocked == 0))
                {
                    runtime->beamVolumeScale -= (0.0026000000070780516f) * timeDelta;
                }
            }
            else if (runtime->cycleTimer < 0x8C)
            {
                if (runtime->blastPhase == 1)
                {
                    runtime->blastPhase = 2;
                    if (gLaserBeamEffectResource != NULL)
                    {
                        (*gLaserBeamEffectResource)->spawn((int)obj, 0xB, NULL, 0x10004, -1, 0);
                    }
                }
            }
            else if (runtime->beamVolumeScale <= (1.0f))
            {
                runtime->beamVolumeScale += (0.052000001072883606f) * timeDelta;
            }
        }
    }

    if (runtime->beamActive != 0)
    {
        Sfx_SetObjectChannelVolume((u32)obj, 0x40, 127.0f * runtime->beamVolumeScale, 0.5f);
    }

    range = (f32)(int)config->rangeAngle;
    rangeSq = range * range;
    yawSin = mathCosf(((3.1415927f) * obj->yaw) / (32768.0f));
    yawCos = mathSinf(((3.1415927f) * obj->yaw) / (32768.0f));
    beamPlane = -(obj->localPosX * yawSin + obj->localPosZ * yawCos);
    playerObj = Obj_GetPlayerObject();

    runtime->damageCooldown = (s8)(runtime->damageCooldown - framesThisStep);
    if (runtime->damageCooldown < 0)
    {
        runtime->damageCooldown = 0;
    }

    damageDistance = beamPlane + (yawSin * playerObj->anim.localPosX +
                                  yawCos * playerObj->anim.localPosZ);
    if ((runtime->proximityMode == 1) ||
        ((damageDistance > (0.0f)) && (runtime->proximityMode != 0)))
    {
        runtime->blockTimer -= framesThisStep;
        if (runtime->blockTimer < 0)
        {
            runtime->blockTimer = 0;
            runtime->beamBlocked = 0;
        }
    }
    else
    {
        runtime->blockTimer += framesThisStep;
        if (runtime->blockTimer > 0x3C)
        {
            runtime->blockTimer = 0x3C;
            runtime->beamBlocked = 1;
        }
    }

    if (runtime->beamBlocked == 0)
    {
        runtime->beamActive = runtime->blastPhase & 3;
    }
    else
    {
        runtime->beamActive = 1;
    }
    if (mainGetBit(config->disableGameBit) != 0)
    {
        runtime->beamActive = 0;
    }

    if (runtime->damageCooldown == 0)
    {
        runtime->hitStrength = 0;
    }
    if (((playerObj != NULL) && (runtime->damageCooldown == 0)) && (runtime->beamActive != 0))
    {
        heightThreshold = (5.0f) + (f32)(int)runtime->heightWindow;
        heightDelta = playerObj->anim.localPosY - obj->localPosY;
        if ((heightDelta < heightThreshold) && (heightDelta > -((25.0f) + heightThreshold)))
        {
            xDelta = playerObj->anim.localPosX - obj->localPosX;
            zDelta = playerObj->anim.localPosZ - obj->localPosZ;
            if ((xDelta * xDelta + zDelta * zDelta) < rangeSq)
            {
                damageDistance = beamPlane + (yawSin * playerObj->anim.localPosX +
                                              yawCos * playerObj->anim.localPosZ);
                lateralAbs = damageDistance;
                if (damageDistance < (0.0f))
                {
                    lateralAbs = -damageDistance;
                }
                if (lateralAbs > (63.0f))
                {
                    lateralAbs = (63.0f);
                }
                lateralAbs = (63.0f) - lateralAbs;
                runtime->hitStrength = (s16)(int)((2.0f) * lateralAbs);
                if (runtime->modgfxAttached == 1)
                {
                    (*gModgfxInterface)->detachSource(obj);
                    runtime->modgfxAttached = 0;
                }
                if ((damageDistance < heightThreshold) && (damageDistance > -heightThreshold))
                {
                    pushDistance =
                        ((beamPlane + (yawSin * playerObj->anim.previousLocalPosX +
                                       yawCos * playerObj->anim.previousLocalPosZ)) < (0.0f))
                            ? (-20.0f)
                            : (20.0f);
                    if (objGetAnimState80A(playerObj) != 0x1D7)
                    {
                        int i;
        Sfx_PlayFromObject((u32)obj, SFXTRIG_wp_espk2_c);
                        for (i = 0; i < 4; i++)
                        {
                            (*gPartfxInterface)->spawnObject(Obj_GetPlayerObject(), 0x28B, (void*)0, 4, -1,
                                                             (void*)0);
                        }
                        runtime->hitX = yawSin * pushDistance + playerObj->anim.localPosX;
                        runtime->hitZ = yawCos * pushDistance + playerObj->anim.localPosZ;
                        if ((runtime->proximityMode == 0) || (runtime->proximityMode == 1))
                        {
                            ObjMsg_SendToObject(playerObj, DFSH_MSG_PLAYER_HIT, runtime->hitPos, 0);
                        }
                        runtime->damageCooldown = 0x14;
                    }
                    else
                    {
                        mainSetBits(GAMEBIT_TRICKYCURVE_PLAYER_HIT, 1);
                    }
                }
            }
        }
    }

    if ((runtime->beamActive == 0) && (runtime->modgfxAttached == 1))
    {
        (*gModgfxInterface)->detachSource(obj);
        runtime->modgfxAttached = 0;
    }

    runtime->rangeBase = runtime->swayAccel = runtime->swayPhase = (0.0f);
    runtime->swayVelocity = runtime->swayPhase;
    runtime->swayTarget = runtime->swayAccel;
    runtime->rangeValue = runtime->rangeBase + range;
    runtime->heightWindow = 8;
    ((GameObject*)obj)->anim.currentMoveProgress += (0.04f) * timeDelta;
    if (((GameObject*)obj)->anim.currentMoveProgress > (1.0f))
    {
        ((GameObject*)obj)->anim.currentMoveProgress -= (1.0f);
    }
}

/*
 * Object setup: initializes the rotating DragonRock Shrine laser beam state.
 */
void DFSH_LaserBeam_init(void* objArg, void* configArg)
{
    DFSHLaserBeamObject* obj;
    DFSHLaserBeamConfig* config;
    DFSHLaserBeamRuntime* runtime;
    int timer;

    obj = (DFSHLaserBeamObject*)objArg;
    config = (DFSHLaserBeamConfig*)configArg;
    runtime = obj->runtime;
    ObjMsg_AllocQueue(obj, 2);
    obj->yaw = (s16)((s32)config->yawByte << 8);
    timer = randomGetRange(-0x50, 0x50);
    runtime->cycleTimer = (s16)(timer + 0x190);
    runtime->blastPhase = 0;
    gLaserBeamEffectResource = Resource_Acquire(DFSHLASERBEAM_EFFECT_RESOURCE_ID, 1);
    runtime->beamVolumeScale = (0.0f);
    runtime->proximityMode = config->proximityMode;
    runtime->warmupThreshold = 0x118;
    if (runtime->beamTexture == NULL)
    {
        runtime->beamTexture = textureLoadAsset(DFSHLASERBEAM_TEXTURE_ID);
    }
}

void DFSH_LaserBeam_release(void)
{
}

void DFSH_LaserBeam_initialise(void)
{
}

ObjectDescriptor gDFSH_LaserBeamObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)DFSH_LaserBeam_initialise,
    (ObjectDescriptorCallback)DFSH_LaserBeam_release,
    0,
    (ObjectDescriptorCallback)DFSH_LaserBeam_init,
    (ObjectDescriptorCallback)DFSH_LaserBeam_update,
    (ObjectDescriptorCallback)DFSH_LaserBeam_hitDetect,
    (ObjectDescriptorCallback)DFSH_LaserBeam_render,
    (ObjectDescriptorCallback)DFSH_LaserBeam_free,
    (ObjectDescriptorCallback)DFSH_LaserBeam_getObjectTypeId,
    (ObjectDescriptorExtraSizeCallback)DFSH_LaserBeam_getExtraSize,
};
