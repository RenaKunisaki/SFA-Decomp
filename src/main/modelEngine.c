#include "dlls/objects/198_AnimatedObj.h"
#include "dlls/objects/199_DIM2RoofRub.h"
#include "dlls/objects/200_DepthOfFieldPoint.h"
#include "dlls/objects/202.h"
#include "dlls/objects/203.h"
#include "dlls/objects/204_ChukChuk.h"
#include "dlls/objects/205_IceBall.h"
#include "dlls/objects/206.h"
#include "dlls/objects/207_CannonClaw.h"
#include "dlls/objects/208_Grimble.h"
#include "dlls/objects/209_TumbleWeedB.h"
#include "dlls/objects/210.h"
#include "dlls/objects/212_SkeetlaWall.h"
#include "dlls/objects/213_Kaldachom.h"
#include "dlls/objects/214_KaldachomMe.h"
#include "dlls/objects/215.h"
#include "dlls/objects/216_PinPonSpike.h"
#include "dlls/objects/217_Pollen.h"
#include "dlls/objects/219_MikaBomb.h"
#include "dlls/objects/220_MikaBombShadow.h"
#include "dlls/objects/221_GCbaddieShield.h"
#include "dlls/objects/222_BaddieInterestP.h"
#include "dlls/objects/223_Hagabon.h"
#include "dlls/objects/224_SwarmBaddie.h"
#include "dlls/objects/225_WispBaddie.h"
#include "dlls/objects/227_Fireball.h"
#include "dlls/objects/228_FlameThrowerspe.h"
#include "dlls/objects/229_Shield.h"
#include "dlls/objects/230_ReStartMark.h"
#include "dlls/objects/231.h"
#include "dlls/objects/232_Checkpoint4.h"
#include "dlls/objects/233_Setuppoint.h"
#include "dlls/objects/234_Sideload.h"
#include "dlls/objects/235.h"
#include "dlls/objects/236_InfoPoint.h"
#include "dlls/objects/237.h"
#include "dlls/objects/238_EffectBox.h"
#include "dlls/objects/239.h"
#include "dlls/objects/240_WarpPoint.h"
#include "dlls/objects/241_InvHit.h"
#include "dlls/objects/242_iceblast.h"
#include "dlls/objects/243_flameblast.h"
#include "dlls/objects/244.h"
#include "dlls/objects/245_SidekickBal.h"
#include "dlls/objects/246_Area.h"
#include "dlls/objects/247.h"
#include "dlls/objects/248_LevelName.h"
#include "dlls/objects/249.h"
#include "dlls/objects/250_InvisibleHi.h"
#include "dlls/objects/251.h"
#include "dlls/objects/252.h"
#include "dlls/objects/253.h"
#include "dlls/objects/254_MagicPlant.h"
#include "dlls/objects/255.h"
#include "dlls/objects/256_TrickyWarp.h"
#include "dlls/objects/257_TrickyGuard.h"
#include "dlls/objects/258_StayPoint.h"
#include "dlls/objects/259_CurveFish.h"
#include "dlls/objects/260_SmallBasket.h"
#include "dlls/objects/261_LargeCrate.h"
#include "dlls/objects/262.h"
#include "dlls/objects/263.h"
#include "dlls/objects/264_EndObject.h"
#include "dlls/objects/265.h"
#include "dlls/objects/266_Fall_Ladder.h"
#include "dlls/objects/267_FireFlyLant.h"
#include "dlls/objects/268_LanternFire.h"
#include "dlls/objects/269_PortalSpell.h"
#include "dlls/objects/270.h"
#include "dlls/objects/271_MMP_Bridge.h"
#include "dlls/objects/272.h"
#include "dlls/objects/273.h"
#include "dlls/objects/274.h"
#include "dlls/objects/275.h"
#include "dlls/objects/276_IMMultiSeq.h"
#include "dlls/objects/277.h"
#include "dlls/objects/278_WM_Column.h"
#include "dlls/objects/279_AppleOnTree.h"
#include "dlls/objects/280_Duster.h"
#include "dlls/objects/281_coldWaterCo.h"
#include "dlls/objects/282.h"
#include "dlls/objects/283_Landed_Arwi.h"
#include "dlls/objects/284.h"
#include "dlls/objects/285.h"
#include "dlls/objects/286_MagicCaveBo.h"
#include "dlls/objects/287_MagicCaveTo.h"
#include "dlls/objects/288_TrickyGuard.h"
#include "dlls/objects/289.h"
#include "dlls/objects/290_CCTestInfot.h"
#include "dlls/objects/291_fuelCell.h"
#include "dlls/objects/292.h"
#include "dlls/objects/293_curve.h"
#include "dlls/objects/295.h"
#include "dlls/objects/296_KT_Torch.h"
#include "dlls/objects/297_CampFire.h"
#include "dlls/objects/298_CFCrate.h"
#include "dlls/objects/299_FXEmit.h"
#include "dlls/objects/300_Transporter.h"
#include "dlls/objects/301_LFXEmitter.h"
#include "dlls/objects/302.h"
#include "dlls/objects/303_BarrelPad.h"
#include "dlls/objects/304_AreaFXEmit.h"
#include "dlls/objects/305.h"
#include "dlls/objects/306_WaterFallSp.h"
#include "dlls/objects/307_sfxPlayer.h"
#include "dlls/objects/308_texscroll2.h"
#include "dlls/objects/309_texscroll.h"
#include "dlls/objects/310_WaveAnimato.h"
#include "dlls/objects/311_AlphaAnimat.h"
#include "dlls/objects/312_GroundAnima.h"
#include "dlls/objects/313_HitAnimator.h"
#include "dlls/objects/314_VisAnimator.h"
#include "dlls/objects/315_WallAnimato.h"
#include "dlls/objects/316_XYZAnimator.h"
#include "dlls/objects/317_ExplodeAnim.h"
#include "dlls/objects/318.h"
#include "dlls/objects/319_TexFrameAni.h"
#include "dlls/objects/320_fogControl.h"
#include "dlls/objects/321_Lightning.h"
#include "dlls/objects/322_FElevContro.h"
#include "dlls/objects/323_FEseqobject.h"
#include "dlls/objects/324.h"
#include "dlls/objects/325_CloudPrison.h"
#include "dlls/objects/326_CloudShipCo.h"
#include "dlls/objects/327.h"
#include "dlls/objects/328_CFGuardian.h"
#include "dlls/objects/329.h"
#include "dlls/objects/330_CFPowerBase.h"
#include "dlls/objects/331_CFMainCryst.h"
#include "dlls/objects/332.h"
#include "dlls/objects/333_LaserBeam.h"
#include "dlls/objects/334_CFPrisonGua.h"
#include "dlls/objects/335_CFPrisonUnc.h"
#include "dlls/objects/336_GCRobotLigh.h"
#include "dlls/objects/337_CFScalesGal.h"
#include "dlls/objects/338_CF_ObjCreat.h"
#include "dlls/objects/339_CFPerch.h"
#include "dlls/objects/340.h"
#include "dlls/objects/341.h"
#include "dlls/objects/342.h"
#include "dlls/objects/343_SpiritDoorS.h"
#include "dlls/objects/344.h"
#include "dlls/objects/345.h"
#include "dlls/objects/346.h"
#include "dlls/objects/347_CFForceFiel.h"
#include "dlls/objects/348_CFForceFiel.h"
#include "dlls/objects/349.h"
#include "dlls/objects/350.h"
#include "dlls/objects/351.h"
#include "dlls/objects/352.h"
#include "dlls/objects/353_CFTreasRobo.h"
#include "dlls/objects/354_CFMagicWall.h"
#include "dlls/objects/355.h"
#include "dlls/objects/356_CFLevelCont.h"
#include "dlls/objects/357_CFRemovalSh.h"
#include "dlls/objects/358.h"
#include "dlls/objects/359_SpiritDoorL.h"
#include "dlls/objects/360_HoloPoint.h"
#include "dlls/objects/361_IMIceMounta.h"
#include "dlls/objects/362_CRrockfall.h"
#include "dlls/objects/363.h"
#include "dlls/objects/364.h"
#include "dlls/objects/365_IMIcePillar.h"
#include "dlls/objects/366_IMAnimSpace.h"
#include "dlls/objects/367_IMSpaceThru.h"
#include "dlls/objects/368_IMSpaceRing.h"
#include "dlls/objects/369_IMSpaceRing.h"
#include "dlls/objects/370_LINKB_levco.h"
#include "dlls/objects/371_LINK_levcon.h"
#include "dlls/objects/372_CCriverflow.h"
#include "dlls/objects/374_DFSH_Door1S.h"
#include "dlls/objects/375.h"
#include "dlls/objects/376_DFSH_Shrine.h"
#include "dlls/objects/377_DFSH_ObjCre.h"
#include "dlls/objects/378_SpiritPrize.h"
#include "dlls/objects/379_DFSH_LaserB.h"
#include "dlls/objects/380_GCRobotPatr.h"
#include "dlls/objects/381.h"
#include "dlls/objects/382_MMP_levelco.h"
#include "dlls/objects/383.h"
#include "dlls/objects/384_MMP_asteroi.h"
#include "dlls/objects/385_MMP_trenchF.h"
#include "dlls/objects/386_MMP_moonroc.h"
#include "dlls/objects/387_MMP_gyserve.h"
#include "dlls/objects/388.h"
#include "dlls/objects/389_CCgasvent.h"
#include "dlls/objects/390_CCgasventCo.h"
#include "dlls/objects/391_CCqueen.h"
#include "dlls/objects/392_CClightfoot.h"
#include "dlls/objects/393_CCSharpclaw.h"
#include "dlls/objects/394_CCpedstal.h"
#include "dlls/objects/395_CClevcontro.h"
#include "dlls/objects/396_MMSH_Shrine.h"
#include "dlls/objects/397_MMSH_Scales.h"
#include "dlls/objects/398_MMSH_WaterS.h"
#include "dlls/objects/399_ECSH_Shrine.h"
#include "dlls/objects/400_ECSH_Cup.h"
#include "dlls/objects/401_ECSH_Creato.h"
#include "dlls/objects/402_GPSH_Shrine.h"
#include "dlls/objects/403_GPSH_ObjCre.h"
#include "dlls/objects/404_GPSH_Scene.h"
#include "dlls/objects/405_DBSH_Shrine.h"
#include "dlls/objects/406_DBSH_Symbol.h"
#include "dlls/objects/407.h"
#include "dlls/objects/408_NWSH_levcon.h"
#include "dlls/objects/409.h"
#include "dlls/objects/410.h"
#include "dlls/objects/411.h"
#include "dlls/objects/412.h"
#include "dlls/objects/413.h"
#include "dlls/objects/414.h"
#include "dlls/objects/415_NW_treebrid.h"
#include "dlls/objects/416_NW_geyser.h"
#include "dlls/objects/417_NW_mammoth.h"
#include "dlls/objects/418_NW_tricky.h"
#include "dlls/objects/419.h"
#include "dlls/objects/420.h"
#include "dlls/objects/421_NW_levcontr.h"
#include "dlls/objects/422_SH_tricky.h"
#include "dlls/objects/423.h"
#include "dlls/objects/424_SH_killermu.h"
#include "dlls/objects/425_BombPlant.h"
#include "dlls/objects/426_BombPlantSp.h"
#include "dlls/objects/427_BombPlantin.h"
#include "dlls/objects/428_SH_queenear.h"
#include "dlls/objects/429_SH_thorntai.h"
#include "dlls/objects/430_SH_LevelCon.h"
#include "dlls/objects/431_SH_swaplift.h"
#include "dlls/objects/432_SH_swapston.h"
#include "dlls/objects/433_SH_staff.h"
#include "dlls/objects/434_SH_staffHaz.h"
#include "dlls/objects/435_SH_Beacon.h"
#include "dlls/objects/436_SH_EmptyTum.h"
#include "dlls/objects/437.h"
#include "dlls/objects/438_SC_levelcon.h"
#include "dlls/objects/439.h"
#include "dlls/objects/440_SC_totempol.h"
#include "dlls/objects/441_SC_Cloudrun.h"
#include "dlls/objects/442_SC_totempuz.h"
#include "dlls/objects/443_SC_totembon.h"
#include "dlls/objects/444_SC_totemstr.h"
#include "dlls/objects/445.h"
#include "dlls/objects/446.h"
#include "dlls/objects/447_DIMLavaBall.h"
#include "dlls/objects/448_DIMLogFire.h"
#include "dlls/objects/449_DIMSnowBall.h"
#include "dlls/objects/450_DIMSnowBall.h"
#include "dlls/objects/451_DIMGate.h"
#include "dlls/objects/452_DIMIceWall.h"
#include "dlls/objects/453_DIMBarrier.h"
#include "dlls/objects/454_DIMCannon.h"
#include "dlls/objects/455_DIMLavaSmas.h"
#include "dlls/objects/456_DIMBridgeCo.h"
#include "dlls/objects/457_DIMDismount.h"
#include "main/audio/sfx_ids.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/audio/sfx.h"
#include "main/dll/dll_0000_gameui_api.h"
#include "main/dll/dll_00DA_pollenfragment_api.h"
#include "main/dll/dll_0282_barrelgener.h"
#include "main/dll/dll_0293_suntemple.h"
#include "main/dll/dll_0294_wctemple.h"
#include "main/dll/WC/dll_0292_wctrexstatu.h"
#include "main/dll/WC/dll_028F_wcpressures.h"
#include "main/dll/WC/dll_0295_wcapertures.h"
#include "main/dll/WC/dll_0296_wctempledia.h"
#include "main/dll/WC/dll_0298_wcfloortile.h"
#include "main/dll/WC/WCbeacon.h"
#include "main/dll/ARW/dll_029C_arwarwingbo.h"
#include "main/dll/ARW/dll_029D_arwarwinggu.h"
#include "main/dll/dll_029B_arwingandrossstuff.h"
#include "main/dll/ARW/dll_02A1_arwlevelcon.h"
#include "main/dll/ARW/dll_029A_arwarwing.h"
#include "main/dll/DR/dll_0281_drearthcal.h"
#include "main/dll/dll_0299.h"
#include "main/dll/dll_02B1_cmbsrc.h"
#include "main/dll/dll_02B2_dustmotesou.h"
#include "main/dll/dll_02B4_cntcounter.h"
#include "main/dll/dll_02B6_cnthitobjec.h"
#include "main/dll/dll_02B7_mcupgrade.h"
#include "main/dll/dll_02B8_mcupgradema.h"
#include "main/dll/dll_02B9_mcstaffeffe.h"
#include "main/dll/dll_02BA_mclightning.h"
#include "main/dll/dll_02BB_gflevelcon.h"
#include "main/dll/dll_02BC_andross.h"
#include "main/dll/dll_02BF_androssligh.h"
#include "main/dll/dll_02BE_androssbrain.h"
#include "main/dll/dll_02BD_androsshand.h"
#include "main/dll/dll_02AF_tree.h"
#include "main/dll/dll_02B0_brokenpipe.h"
#include "main/frame_timing.h"
#include "main/game_timer_control_api.h"
#include "main/gametext_box_api.h"
#include "main/gametext_internal.h"
#include "main/gametext_show_str_api.h"
#include "main/textrender_api.h"
#include "main/gametext_color_api.h"
#include "main/gameloop_api.h"
#include "main/minimap_api.h"
#include "main/model_engine.h"
#include "main/mm.h"
#include "main/pause_menu_api.h"
#include "main/resource.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/printf.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/string.h"

s32 gModelEngineHudNumber = -1;

f32 gModelEngineTimerValue;
f32 gModelEngineTimerDuration;
s8 gModelEngineTimerFlags;
u8 gModelEngineTimerState;
int gModelEnginePrevUiDll;
int curUiDll;
int gModelEnginePendingUiDll;
UiDllVTable** gModelEngineCurUiDllRes;
int gModelEngineTimerDigitPairXOffset = 0x10;
int gModelEngineTimerFieldXStride = 0x26;
int gModelEngineTimerColonX = 0x24;
int gModelEngineTimerDotX = 0x4A;
s32 lbl_803DB28C = -1;
char sModelEngineHudNumberFormat[] = "%d";
char sModelEngineTimerDigitFormat[] = "%01d";
char sModelEngineTimerColonText[] = ":";
char sModelEngineTimerDotText[] = ".";

#define RESOURCE_DESCRIPTOR_COUNT 0x2c1

/* gModelEngineTimerState bits (roles from accessor fns: timerSetToCountUp,
 * isGameTimerDisabled, gameTimerIsRunning). */
#define MODELENGINE_TIMER_COUNTDOWN 1
#define MODELENGINE_TIMER_DISABLED  2
#define MODELENGINE_TIMER_RUNNING   4

extern ResourceDescriptor Carryable_funcs, boneParticleEffect_funcs, dll_19;
extern ResourceDescriptor dll_1CE, dll_1CF, dll_1D6, dll_1DA, dll_1DB, dll_1FB;
extern ResourceDescriptor dll_1FF, dll_200, dll_219, dll_21B, dll_224, dll_28B, dll_2A3;
extern ResourceDescriptor dll_2A4, dll_2E, dll_54, dll_D3, expgfx_funcs;
extern ResourceDescriptor gARWBlockerObjDescriptor, gARWBombCollObjDescriptor, gARWGeneratoObjDescriptor,
    gARWProximitObjDescriptor;
extern ResourceDescriptor gARWSpeedStrObjDescriptor, gARWSquadronObjDescriptor;
extern ResourceDescriptor gBaddieObjDescriptor, gBossDrakorObjDescriptor;
extern ResourceDescriptor gChukaObjDescriptor;
extern ResourceDescriptor gControlLightObjDescriptor, gCrCloudRaceObjDescriptor, gCrFuelTankObjDescriptor;
extern ResourceDescriptor gDBHoleControl1ObjDescriptor, gDB_eggObjDescriptor, gDBstealerwormObjDescriptor,
    gDFP_LevelControlObjDescriptor, gDFP_ObjCreatorObjDescriptor, gDFP_TorchObjDescriptor;
extern ResourceDescriptor gDFP_seqpointObjDescriptor, gDFropenodeObjDescriptor, gDIM2ConveyorObjDescriptor,
    gDIM2IceFloeObjDescriptor;
extern ResourceDescriptor gDIM2IcicleObjDescriptor, gDIM2LavaControlObjDescriptor, gDIM2PathGeneratorObjDescriptor,
    gDIM2PrisonMammothObjDescriptor, gDIM2SnowBallObjDescriptor;
extern ResourceDescriptor gDIMMagicBridgeObjDescriptor;
extern ResourceDescriptor gDIMSnowHorn1ObjDescriptor, gDIMTruthHornIceObjDescriptor, gDIMWoodDoor2ObjDescriptor,
    gDIM_BossGut2ObjDescriptor, gDIM_BossGutObjDescriptor, gDIM_BossObjDescriptor;
extern ResourceDescriptor gDIM_BossSpitObjDescriptor, gDIM_BossTonsilObjDescriptor, gDIM_LevelControlObjDescriptor,
    gDIM_trickyObjDescriptor, gDIMbosscrackparObjDescriptor, gDIMbossfireObjDescriptor, gDR_CloudRunnerObjDescriptor,
    gDR_EarthWarriorObjDescriptor;
extern ResourceDescriptor gDfperchwitchObjDescriptor, gDfpfloorbarObjDescriptor, gDfplightniObjDescriptor,
    gDfppowerslObjDescriptor;
extern ResourceDescriptor gDfpstatue1ObjDescriptor, gDfptargetblockObjDescriptor, gDirectionalLightObjDescriptor;
extern ResourceDescriptor gDoorswitchObjDescriptor, gDrBarrelGrObjDescriptor, gDrCageControlObjDescriptor,
    gDrCageWithObjDescriptor, gDrChimmeyObjDescriptor, gDrCloudPerObjDescriptor, gDrCreatorObjDescriptor;
extern ResourceDescriptor gDrEnergyDiscObjDescriptor, gDrGeneratorObjDescriptor, gDrLaserCannonObjDescriptor,
    gDrLightBeaObjDescriptor, gDrMusicContObjDescriptor, gDrShackleObjDescriptor, gDrakorDThornBushObjDescriptor,
    gDrakorEnergyObjDescriptor;
extern ResourceDescriptor gDrakorHoverPadObjDescriptor, gDrakorMissileObjDescriptor;
extern ResourceDescriptor gEarthWalkerObjDescriptor, gExplodePlanObjDescriptor;
extern ResourceDescriptor gExplosionObjDescriptor, gFireFlyObjDescriptor, gFireObjDescriptor;
extern ResourceDescriptor gFirePipeObjDescriptor, gFlagObjDescriptor;
extern ResourceDescriptor gGmMazeWellObjDescriptor;
extern ResourceDescriptor gHighTopObjDescriptor;
extern ResourceDescriptor gKtFallingrocksObjDescriptor;
extern ResourceDescriptor gKtLazerlightObjDescriptor, gKtLazerwallObjDescriptor, gKtRexFloorSwitchObjDescriptor,
    gKtRexLevelObjDescriptor, gKtRexObjDescriptor, gKytesMumObjDescriptor;
extern ResourceDescriptor gLampObjDescriptor, gLaserBeamObjDescriptor, gLaserObjDescriptor,
    gLaserUnsupportedObjDescriptor;
extern ResourceDescriptor gLightSourceObjDescriptor, gMAGICMakerObjDescriptor;
extern ResourceDescriptor gMoonSeedPlantingSpotObjDescriptor;
extern ResourceDescriptor gPlatform1ObjDescriptor, gPointLightObjDescriptor;
extern ResourceDescriptor gPressureSwitchObjDescriptor, gProjectedLightObjDescriptor, gProximityMineObjDescriptor;
extern ResourceDescriptor gRingObjDescriptor, gSB_CageKyteObjDescriptor, gSB_CannonBallObjDescriptor,
    gSB_CloudBallObjDescriptor, gSB_CloudRunnerObjDescriptor;
extern ResourceDescriptor gSB_FireBallObjDescriptor, gSB_GalleonObjDescriptor, gSB_KyteCageObjDescriptor,
    gSB_MiniFireObjDescriptor, gSB_PropellerObjDescriptor, gSB_SeqDoorObjDescriptor, gSB_ShipGunBrokeObjDescriptor,
    gSB_ShipGunObjDescriptor;
extern ResourceDescriptor gSB_ShipHeadObjDescriptor, gSB_ShipMastObjDescriptor;
extern ResourceDescriptor gSPDrapeObjDescriptor, gSPScarabObjDescriptor, gSPitembeamObjDescriptor,
    gSeqPointObjDescriptor;
extern ResourceDescriptor gSfxplayerObjDescriptor, gShipBattleObjDescriptor, gShopItemObjDescriptor,
    gShopKeeperObjDescriptor, gShopObjDescriptor;
extern ResourceDescriptor gSnowBikeObjDescriptor, gSnowClawObjDescriptor;
extern ResourceDescriptor gSoftBodyObjDescriptor, gSpellStoneObjDescriptor, gStaffObjDescriptor,
    gStaticCameraObjDescriptor;
extern ResourceDescriptor gTextBlockObjDescriptor, gTimerObjDescriptor;
extern ResourceDescriptor gTitleScreenObjDescriptor, gTrickyCurveObjDescriptor;
extern ResourceDescriptor gTrickyObjDescriptor, gTriggerObjDescriptor, gVFPDragHeadObjDescriptor, gVFPLiftObjDescriptor,
    gVFP_Block1ObjDescriptor;
extern ResourceDescriptor gVFP_DoorSwitchObjDescriptor, gVFP_LaddersObjDescriptor, gVFP_LevelControlObjDescriptor,
    gVFP_MiniFireObjDescriptor, gVFP_ObjCreatorObjDescriptor, gVFP_PlatformObjDescriptor,
    gVFP_SpellPlaceObjDescriptor, gVFP_coreplatObjDescriptor, gVFP_flamepointObjDescriptor;
extern ResourceDescriptor gVFP_lavapoolObjDescriptor, gVFP_lavastarObjDescriptor, gVFP_statueballObjDescriptor,
    gVortexObjDescriptor, gWCBouncyCraObjDescriptor;
extern ResourceDescriptor gWCLevelContObjDescriptor,
    gWCPushBlockObjDescriptor, gWCTempleBriObjDescriptor,
    gWCTileObjDescriptor;
extern ResourceDescriptor gWM_GalleonObjDescriptor, gWM_GeneralScalesObjDescriptor, gWM_LaserTargetObjDescriptor,
    gWM_LevelControlObjDescriptor, gWM_ObjCreatorObjDescriptor, gWM_PlanetsObjDescriptor;
extern ResourceDescriptor gWM_SpiritSetObjDescriptor, gWM_TorchObjDescriptor, gWM_WallCrawlerObjDescriptor,
    gWM_WormObjDescriptor, gWM_colriseObjDescriptor, gWM_newcrystalObjDescriptor, gWM_seqobjectObjDescriptor,
    gWM_seqpointObjDescriptor;
extern ResourceDescriptor gWM_spiritplaceObjDescriptor, gWM_sunObjDescriptor, gWaterFlowWeObjDescriptor;
extern ResourceDescriptor gWorldAsteroidsObjDescriptor, gWorldObjObjDescriptor, gWorldPlanetObjDescriptor,
    lbl_8030EE34;
extern ResourceDescriptor lbl_8030F414, sky2_funcs, lbl_8030F5B4, lbl_8030F788, lbl_8030F7E8, lbl_8030F830,
    lbl_8030FCA8, partfx_funcs;
extern ResourceDescriptor Effect1_funcs, Effect2_funcs, Effect3_funcs, Effect4_funcs, Effect5_funcs, Effect6_funcs,
    Effect7_funcs, Effect8_funcs;
extern ResourceDescriptor Effect9_funcs, Effect10_funcs, Effect11_funcs, Effect12_funcs, Effect14_funcs, Effect16_funcs,
    Effect15_funcs, Effect13_funcs;
extern ResourceDescriptor Effect17_funcs, Effect18_funcs, Effect19_funcs, Effect20_funcs, lbl_803112E8, lbl_80311340,
    lbl_80311378, lbl_80311438;
extern ResourceDescriptor lbl_803114B0, lbl_803114D8, lbl_803115F8, dll_15_funcs, lbl_80311900, lbl_80311BE0,
    lbl_80311D88, lbl_80311E0C;
extern ResourceDescriptor lbl_80311E80, lbl_8031210C, lbl_8031231C, lbl_8031262C, lbl_80312770, lbl_803128C4,
    lbl_803129A8, lbl_80312BB4;
extern ResourceDescriptor lbl_80312CF8, lbl_80312E38, lbl_80312F78, lbl_80313184, lbl_80313394, lbl_803135A4,
    lbl_803137B4, lbl_803137D8;
extern ResourceDescriptor lbl_80313880, lbl_80313A1C, lbl_80313AB0, Dummy6C_funcs, lbl_80313C10, lbl_80313CA0,
    lbl_80313E78, lbl_8031403C;
extern ResourceDescriptor lbl_80314268, lbl_80314490, lbl_803146B8, lbl_803148FC, lbl_80314930, lbl_80314960,
    lbl_80314990, lbl_80314AD0;
extern ResourceDescriptor lbl_80314BB0, lbl_80314C90, lbl_80314DE4, lbl_80315010, lbl_80315238, lbl_80315304,
    lbl_80315444, lbl_80315528;
extern ResourceDescriptor lbl_80315750, lbl_80315978, lbl_80315C84, lbl_80315F84, lbl_80316000, lbl_80316030,
    lbl_80316220, lbl_80316440;
extern ResourceDescriptor lbl_80316630, lbl_80316708, lbl_80316930, lbl_80316B3C, lbl_80316C20, lbl_80316C70,
    lbl_80316E0C, lbl_80316FD4;
extern ResourceDescriptor lbl_8031719C, lbl_8031723C, lbl_80317468, lbl_80317504, lbl_803175C8, lbl_803177F0,
    lbl_8031788C, lbl_80317AD4;
extern ResourceDescriptor lbl_80317B74, lbl_80317BB8, lbl_80317DE0, lbl_80318014, lbl_80318240, lbl_80318468,
    lbl_80318690, lbl_803188B8;
extern ResourceDescriptor lbl_80318AE0, lbl_80318D08, lbl_80318D28, lbl_80318DD0, lbl_80318E20, lbl_80318EC8,
    lbl_80319008, lbl_80319148;
extern ResourceDescriptor lbl_80319354, lbl_80319378, lbl_803193C0, lbl_80319410, lbl_80319460, lbl_803194A8,
    lbl_803194F8, lbl_80319548;
extern ResourceDescriptor lbl_80319598, lbl_803195E8, lbl_80319638, lbl_80319688, lbl_803196D8, lbl_80319720,
    lbl_80319768, lbl_803197B0;
extern ResourceDescriptor lbl_803197F8, lbl_80319840, lbl_80319888, lbl_803198D8, lbl_80319920, lbl_80319968,
    lbl_803199B0, lbl_803199F8;
extern ResourceDescriptor lbl_80319A40, lbl_80319A88, lbl_80319B58, lbl_80319B98, lbl_80319BC8, lbl_80319BF8,
    lbl_80319C28, lbl_80319C58;
extern ResourceDescriptor lbl_80319C88, lbl_80319CE8, lbl_80319D18, lbl_80319D48, lbl_80319D78,
    gCameraModeNpcSpeakDescriptor,
    lbl_80319E08, lbl_80319E38;
extern ResourceDescriptor lbl_80319E68, lbl_80319E98, lbl_80319EC8, lbl_80319EF8, lbl_80319F58, lbl_80319F88,
    lbl_8031A01C, lbl_8031A148;
extern ResourceDescriptor TitleScreenInit_funcs, n_rareware_funcs, lbl_8031A304, lbl_8031A82C, EnterSaveNameScreen_funcs, OptionsScreen_funcs,
    lbl_8031ADA4, Dummy39_funcs;
extern ResourceDescriptor Dummy3A_funcs, lbl_8031C020, lbl_8031C168, lbl_8031C1E4, lbl_8031C2B4, Dummy3E_funcs,
    Minimap_funcs, dll_3F_funcs;
extern ResourceDescriptor lbl_8031CC10, lbl_8031CDB8;
extern ResourceDescriptorCallbacks8 lbl_80320700;
extern ResourceDescriptor gDll1DFObjDescriptor, lbl_80328AD8, lbl_80328E28, lbl_80329340;
extern ResourceDescriptor gDll22CObjDescriptor, Dummy245, Dummy246, Dummy244, Dummy247, Dummy248, Dummy24A, Dummy24B;
extern ResourceDescriptor lbl_8032A110, lbl_8032AD00, lbl_8032AD68, gDll29EObjDescriptor;
extern ResourceDescriptor lbl_803DC0F8, lbl_803DC100, lbl_803DC108, lbl_803DC138, lbl_803DC140, lbl_803DC150;
extern ResourceDescriptor lbl_803DC158, lbl_803DC2C0, lbl_803DC2D8, lbl_803DC2E0, lbl_803DC2E8, lbl_803DC338,
    lbl_803DC358, lbl_803DC360;
extern ResourceDescriptor lbl_803DC368, lbl_803DC370, lbl_803DC378, lbl_803DC388, lbl_803DC390, lbl_803DC6E8,
    playerShadow_funcs, projgfx_funcs;

void* gResourceLoadedHandles[0x2C1];
u16 gResourceRefCounts[0x2C2];
char gModelEngineTextBuf[0x10];

RingBufferQueue* allocModelStruct_800139e8(int capacity, int elemSize)
{
    RingBufferQueue* queue = mmAlloc(elemSize * capacity + sizeof(RingBufferQueue), 0x1a, 0);
    queue->data = (u8*)queue + sizeof(RingBufferQueue);
    queue->count = 0;
    queue->capacity = capacity;
    queue->elemSize = elemSize;
    queue->writeIndex = 0;
    return queue;
}

s32 modelRenderInstrsState_getBit(ModelRenderInstrsState* state)
{
    return state->bit;
}

void modelRenderInstrsState_setBit(ModelRenderInstrsState* state, s32 bit)
{
    state->bit = bit;
}

void modelRenderInstrsState_init(ModelRenderInstrsState* state, void* instrs, int bitCount, int fieldC)
{
    state->byteCount = bitCount >> 3;
    if ((bitCount & 7) != 0)
    {
        state->byteCount++;
    }
    state->bitCount = bitCount;
    state->fieldC = fieldC;
    state->instrs = instrs;
    state->bit = 0;
}

void objList_remove(ObjLinkedList* list, int item)
{
    int head;
    int prev;
    int current;
    int next;

    head = list->head;
    if (head == item)
    {
        list->head = *(int*)(head + list->nextOffset);
        list->count--;
        return;
    }

    current = head;
    prev = head;
    while (current != 0 && current != item)
    {
        prev = current;
        current = *(int*)(current + list->nextOffset);
    }

    if (current == 0)
    {
        return;
    }

    next = *(int*)(current + list->nextOffset);
    if (current == head)
    {
        list->head = next;
    }
    else
    {
        *(int*)(prev + list->nextOffset) = next;
    }
    list->count--;
}

void objListAdd(ObjLinkedList* list, int prev, int item)
{
    int next;

    if (list->head == 0)
    {
        list->head = item;
    }
    else
    {
        if (prev == 0)
        {
            next = list->head;
            list->head = item;
        }
        else
        {
            next = *(int*)(prev + list->nextOffset);
            *(int*)(prev + list->nextOffset) = item;
        }
        *(int*)(item + list->nextOffset) = next;
    }
    list->count++;
}

void objListInit(ObjLinkedList* list, s16 nextOffset)
{
    list->head = 0;
    list->nextOffset = nextOffset;
}

BOOL model_findIdxInModelList(ModelList* list, void* header, int* outIndex)
{
    s16* entry;

    entry = list->entries;
    while (entry < list->end)
    {
        if (memcmp(entry + 1, header, list->dataSize) == 0)
        {
            *outIndex = *entry;
            return TRUE;
        }
        entry += list->strideShorts;
    }
    return FALSE;
}

BOOL ModelList_getHeader(ModelList* list, int index, void* outHeader)
{
    s16* entry;

    entry = list->entries;
    while (entry < list->end)
    {
        if (*entry == index)
        {
            memcpy(outHeader, entry + 1, list->dataSize);
            return TRUE;
        }
        entry += list->strideShorts;
    }
    return FALSE;
}

void model_adjustModelList(ModelList* list, int index)
{
    s16* entry;

    entry = list->entries;
    while (entry < list->end)
    {
        if (*entry == index)
        {
            *entry = -1;
            break;
        }
        entry += list->strideShorts;
    }

    while (list->end > list->entries && list->end[-1] == -1)
    {
        list->end -= list->strideShorts;
    }
}

void modelInitModelList(ModelList* list, s16 index, void* header)
{
    s16* entry;

    for (entry = list->entries; entry < list->end; entry += list->strideShorts)
    {
        if (*entry == -1)
        {
            break;
        }
    }

    *entry = index;
    memcpy(entry + 1, header, list->dataSize);
    if (entry == list->end)
    {
        list->end += list->strideShorts;
    }
}

ModelList* allocModelStruct(int capacity, int dataSize)
{
    int entryBytes;
    ModelList* list;

    entryBytes = dataSize + 2;
    list = mmAlloc(capacity * entryBytes + sizeof(ModelList), 0x1a, 0);
    list->entries = (s16*)((u8*)list + sizeof(ModelList));
    list->dataSize = dataSize;
    list->strideShorts = (u32)entryBytes >> 1;
    list->end = list->entries;
    list->capacityEnd = list->entries + capacity * list->strideShorts;
    memset(list->entries, -1, capacity * (list->strideShorts * 2));
    return list;
}

BOOL Resource_Release(void* handleSlot)
{
    s32 i;
    ResourceDescriptor* descriptor;

    i = 0;
    descriptor = (ResourceDescriptor*)handleSlot;
    while (i < RESOURCE_DESCRIPTOR_COUNT)
    {
        if ((void*)&gResourceLoadedHandles[i] == handleSlot)
        {
            descriptor = gResourceDescriptors[i];
            break;
        }
        i++;
    }

    gResourceRefCounts[i]--;
    if (gResourceRefCounts[i] == 0)
    {
        if (descriptor->release != NULL)
        {
            descriptor->release();
        }
        return TRUE;
    }
    return FALSE;
}

void* Resource_Acquire(u16 id, int unused)
{
    u32 index;
    ResourceDescriptor* descriptor;

    index = id;
    descriptor = gResourceDescriptors[index];
    if (gResourceRefCounts[index] == 0 && descriptor->acquire != NULL)
    {
        descriptor->acquire(descriptor);
    }
    gResourceRefCounts[index]++;
    gResourceLoadedHandles[index] = descriptor->data;
    return &gResourceLoadedHandles[index];
}

void Resource_ResetRefCounts(void)
{
    u32 i;

    for (i = 0; i < RESOURCE_DESCRIPTOR_COUNT; i++)
    {
        gResourceRefCounts[i] = 0;
    }
}

void fn_8001404C(s32 value)
{
    lbl_803DB28C = value;
}

u8 gameTimerIsRunning(void)
{
    return gModelEngineTimerState & MODELENGINE_TIMER_RUNNING;
}

void hudNumberFn_80014060(void* context)
{
    if (gModelEngineHudNumber != -1)
    {
        sprintf(gModelEngineTextBuf, sModelEngineHudNumberFormat, gModelEngineHudNumber);
        gameTextShowStr(gModelEngineTextBuf, 13, 0, 0);
    }
}

void set_hudNumber_803db278(s32 value)
{
    gModelEngineHudNumber = value;
}

void gameTimerRun(void* context)
{
    f32 dt = timeDelta;
    u8 colorFlag = 0;
    TextSlot* box = gameTextGetBox(0xD);
    int hours;
    int minutes;
    int hundredths;
    u16 boxY;
    char clamped;
    int totalSecs;
    int mins;

    if ((gModelEngineTimerState & MODELENGINE_TIMER_COUNTDOWN) || getHudHiddenFrameCount() != 0)
    {
        dt = 0.0f;
    }

    clamped = 0;
    if ((gModelEngineTimerFlags & 1) != 0)
    {
        gModelEngineTimerValue -= dt;
        if (gModelEngineTimerValue <= 0.0f)
        {
            clamped = 1;
            gModelEngineTimerValue = 0.0f;
        }
        if (gModelEngineTimerValue < 600.0f)
        {
            colorFlag = 1;
        }
    }
    else
    {
        gModelEngineTimerValue += dt;
        if (gModelEngineTimerValue > gModelEngineTimerDuration)
        {
            clamped = 1;
            gModelEngineTimerValue = gModelEngineTimerDuration;
        }
        if (gModelEngineTimerValue > gModelEngineTimerDuration - 600.0f)
        {
            colorFlag = 1;
        }
    }

    if (clamped)
    {
        if ((gModelEngineTimerFlags & 8) != 0)
        {
            Sfx_PlayFromObject(0, SFXTRIG_sc_lockon22);
        }
        gModelEngineTimerState &= ~MODELENGINE_TIMER_RUNNING;
        gModelEngineTimerState |= MODELENGINE_TIMER_DISABLED;
    }

    if ((gModelEngineTimerFlags & 4) != 0)
    {
        f32 panByte;
        f32 volume;
        Sfx_KeepAliveLoopedObjectSound(0, SFXTRIG_sc_commsbleep_28c);
        if ((gModelEngineTimerFlags & 1) != 0)
        {
            panByte = (f32)(0x7F - ((int)(80.0f * (gModelEngineTimerValue / gModelEngineTimerDuration)) & 0xFF));
            volume = 1.3f - 0.6f * (gModelEngineTimerValue / gModelEngineTimerDuration);
        }
        else
        {
            panByte = (f32)(((int)(80.0f * (gModelEngineTimerValue / gModelEngineTimerDuration)) & 0xFF) + 0x2F);
            volume = 0.6f * (gModelEngineTimerValue / gModelEngineTimerDuration) + 0.7f;
        }
        Sfx_SetObjectSfxVolume(0, SFXTRIG_sc_commsbleep_28c, panByte, volume);
    }

    if ((gModelEngineTimerFlags & 0x10) != 0 && pauseMenuState == 0 && getHudHiddenFrameCount() == 0)
    {
        totalSecs = gModelEngineTimerValue;
        mins = totalSecs / 60;
        hours = mins / 60;
        minutes = mins - hours * 60;
        hundredths = (int)(100.0f * (gModelEngineTimerValue / 60.0f));
        hundredths = hundredths - hundredths / 100 * 100;

        boxY = getMinimapY() - 0x28;
        drawHudBox(0x32, (s16)(boxY - 4), 0x78, 0x28, 0xFF, 1);
        box->y = boxY;

        if (colorFlag && hundredths < 0x32)
        {
        gameTextSetColor(0xFF, 0x40, 0x40, 0xFF);
        }
        else
        {
        gameTextSetColor(0xFF, 0xFF, 0xFF, 0xFF);
        }

        sprintf(gModelEngineTextBuf, sModelEngineTimerDigitFormat, hours / 10);
        gameTextShowStr(gModelEngineTextBuf, 0xD, 5, 3);
        sprintf(gModelEngineTextBuf, sModelEngineTimerDigitFormat, hours % 10);
        gameTextShowStr(gModelEngineTextBuf, 0xD, gModelEngineTimerDigitPairXOffset + 5, 3);
        sprintf(gModelEngineTextBuf, sModelEngineTimerDigitFormat, minutes / 10);
        gameTextShowStr(gModelEngineTextBuf, 0xD, gModelEngineTimerFieldXStride + 5, 3);
        sprintf(gModelEngineTextBuf, sModelEngineTimerDigitFormat, minutes % 10);
        gameTextShowStr(gModelEngineTextBuf, 0xD, 5 + gModelEngineTimerFieldXStride + gModelEngineTimerDigitPairXOffset, 3);
        sprintf(gModelEngineTextBuf, sModelEngineTimerDigitFormat, hundredths / 10);
        gameTextShowStr(gModelEngineTextBuf, 0xD, gModelEngineTimerFieldXStride * 2 + 5, 3);
        sprintf(gModelEngineTextBuf, sModelEngineTimerDigitFormat, hundredths % 10);
        gameTextShowStr(gModelEngineTextBuf, 0xD, 5 + gModelEngineTimerFieldXStride * 2 + gModelEngineTimerDigitPairXOffset, 3);
        if (minutes & 1)
        {
            gameTextShowStr(sModelEngineTimerColonText, 0xD, gModelEngineTimerColonX, 3);
            gameTextShowStr(sModelEngineTimerDotText, 0xD, gModelEngineTimerDotX, 3);
        }
    }
}

f32 gameTimerGetElapsedMilliseconds(void)
{
    if (((s8)gModelEngineTimerFlags & 1) != 0)
    {
        return 1000.0f * ((gModelEngineTimerDuration - gModelEngineTimerValue) / 60.0f);
    }
    return 1000.0f * (gModelEngineTimerValue / 60.0f);
}

f32 gameTimerGetValue(void)
{
    return gModelEngineTimerValue;
}

int isGameTimerDisabled(void)
{
    return gModelEngineTimerState & MODELENGINE_TIMER_DISABLED;
}

void gameTimerStop(void)
{
    gModelEngineTimerState &= ~MODELENGINE_TIMER_RUNNING;
    gModelEngineTimerState |= MODELENGINE_TIMER_DISABLED;
}

void timerSetToCountUp(void)
{
    if ((gModelEngineTimerState & MODELENGINE_TIMER_COUNTDOWN) != 0)
    {
        gModelEngineTimerState &= ~MODELENGINE_TIMER_COUNTDOWN;
    }
}

void gameTimerInit(s8 flags, int minutes)
{
    gModelEngineTimerFlags = flags;
    if ((flags & 1) != 0)
    {
        gModelEngineTimerValue = minutes * 60;
    }
    else
    {
        gModelEngineTimerValue = 0.0f;
    }
    gModelEngineTimerDuration = minutes * 60;
    gModelEngineTimerState |= MODELENGINE_TIMER_COUNTDOWN;
    gModelEngineTimerState &= ~MODELENGINE_TIMER_DISABLED;
    if ((flags & 3) != 0)
    {
        gModelEngineTimerState |= MODELENGINE_TIMER_RUNNING;
    }
    else
    {
        gModelEngineTimerState &= ~MODELENGINE_TIMER_RUNNING;
    }
}

void curUiDllDraw(int a, int b, int c, int d)
{
    UiDllVTable* callbacks;

    if (gModelEngineCurUiDllRes != NULL)
    {
        callbacks = *gModelEngineCurUiDllRes;
        callbacks->draw(a, b, c);
    }
}

void uiDll_runFrameEndAndLoadNext(void)
{
    UiDllVTable* callbacks;
    s32 resourceId;

    if (gModelEngineCurUiDllRes != NULL)
    {
        callbacks = *gModelEngineCurUiDllRes;
        callbacks->frameEnd();
    }

    if (gModelEnginePendingUiDll != 0)
    {
        gModelEnginePendingUiDll--;
        gModelEnginePrevUiDll = curUiDll;
        if (gModelEngineCurUiDllRes != NULL)
        {
            Resource_Release(gModelEngineCurUiDllRes);
            gModelEngineCurUiDllRes = NULL;
        }

        resourceId = gModelEngineUiDllResourceIds[gModelEnginePendingUiDll];
        if (resourceId != -1)
        {
            gModelEngineCurUiDllRes = Resource_Acquire((u16)resourceId, 1);
        }
        else
        {
            gModelEngineCurUiDllRes = NULL;
            gModelEnginePendingUiDll = 0;
        }
        curUiDll = gModelEnginePendingUiDll;
        gModelEnginePendingUiDll = 0;
    }
}

int uiDll_runFrameStartAndLoadNext(void)
{
    UiDllVTable* callbacks;
    int result;
    s32 resourceId;

    result = 0;
    if (gModelEngineCurUiDllRes != NULL)
    {
        callbacks = *gModelEngineCurUiDllRes;
        result = callbacks->frameStart();
    }

    if (gModelEnginePendingUiDll != 0)
    {
        gModelEnginePendingUiDll--;
        gModelEnginePrevUiDll = curUiDll;
        if (gModelEngineCurUiDllRes != NULL)
        {
            Resource_Release(gModelEngineCurUiDllRes);
            gModelEngineCurUiDllRes = NULL;
        }

        resourceId = gModelEngineUiDllResourceIds[gModelEnginePendingUiDll];
        if (resourceId != -1)
        {
            gModelEngineCurUiDllRes = Resource_Acquire((u16)resourceId, 1);
        }
        else
        {
            gModelEngineCurUiDllRes = NULL;
            gModelEnginePendingUiDll = 0;
        }
        curUiDll = gModelEnginePendingUiDll;
        gModelEnginePendingUiDll = 0;
    }
    return result;
}

void setCurUiDll(int idx)
{
    curUiDll = idx;
}

int getPrevUiDll(void)
{
    return gModelEnginePrevUiDll;
}

UiDllVTable** getDLL16(void)
{
    return gModelEngineCurUiDllRes;
}

int getCurUiDll(void)
{
    return curUiDll;
}

void loadUiDll(int index)
{
    s32 next;
    s32 current;
    s32 resourceId;

    current = curUiDll;
    if (index != current)
    {
        next = index + 1;
        gModelEnginePendingUiDll = next;
        if (gModelEngineCurUiDllRes == NULL && next != 0)
        {
            gModelEnginePendingUiDll = next - 1;
            gModelEnginePrevUiDll = current;
            if (gModelEngineCurUiDllRes != NULL)
            {
                Resource_Release(gModelEngineCurUiDllRes);
                gModelEngineCurUiDllRes = NULL;
            }

            resourceId = gModelEngineUiDllResourceIds[gModelEnginePendingUiDll];
            if (resourceId != -1)
            {
                gModelEngineCurUiDllRes = Resource_Acquire((u16)resourceId, 1);
            }
            else
            {
                gModelEngineCurUiDllRes = NULL;
                gModelEnginePendingUiDll = 0;
            }
            curUiDll = gModelEnginePendingUiDll;
            gModelEnginePendingUiDll = 0;
        }
    }
}

void initGameTimer(void)
{
    gModelEngineCurUiDllRes = NULL;
    gModelEnginePendingUiDll = 0;
    gModelEnginePrevUiDll = 0;
    curUiDll = 0;
    gModelEngineTimerState = MODELENGINE_TIMER_DISABLED;
    gModelEngineTimerFlags = 0;
    gModelEngineTimerValue = 0.0f;
    gModelEngineTimerDuration = 0.0f;
}

ResourceDescriptor* gResourceDescriptors[] = {
    &lbl_8031C020,
    &lbl_80319A88,
    &lbl_8030EE34,
    &lbl_803112E8,
    &lbl_80311378,
    &lbl_8030F414,
    &sky2_funcs,
    &lbl_8030F5B4,
    &lbl_8030F788,
    &lbl_8030F7E8,
    &expgfx_funcs,
    &lbl_8030FCA8,
    &projgfx_funcs,
    &playerShadow_funcs,
    &partfx_funcs,
    &lbl_80311438,
    &lbl_803114B0,
    &lbl_80311BE0,
    &lbl_803114D8,
    &lbl_8030F830,
    &lbl_803115F8,
    &dll_15_funcs,
    &lbl_80311340,
    &lbl_80311900,
    &boneParticleEffect_funcs,
    &dll_19,
    &Effect1_funcs,
    &Effect2_funcs,
    &Effect3_funcs,
    &Effect4_funcs,
    &Effect5_funcs,
    &Effect6_funcs,
    &Effect7_funcs,
    &Effect8_funcs,
    &Effect9_funcs,
    &Effect10_funcs,
    &Effect11_funcs,
    &Effect12_funcs,
    &Effect13_funcs,
    &Effect14_funcs,
    &Effect15_funcs,
    &Effect16_funcs,
    &Effect17_funcs,
    &Effect18_funcs,
    &Effect19_funcs,
    &Effect20_funcs,
    &dll_2E,
    &Carryable_funcs,
    &lbl_8031A148,
    &Minimap_funcs,
    &TitleScreenInit_funcs,
    &n_rareware_funcs,
    &lbl_8031A304,
    &lbl_8031A82C,
    &EnterSaveNameScreen_funcs,
    &OptionsScreen_funcs,
    &lbl_8031ADA4,
    &Dummy39_funcs,
    &Dummy3A_funcs,
    &lbl_8031C168,
    &lbl_8031C1E4,
    &lbl_8031C2B4,
    &Dummy3E_funcs,
    &dll_3F_funcs,
    &lbl_8031CC10,
    &lbl_8031CDB8,
    &lbl_80319B58,
    &lbl_80319B98,
    &lbl_80319BF8,
    &lbl_80319BC8,
    &lbl_80319C28,
    &lbl_80319C88,
    &lbl_80319C58,
    &lbl_80319CE8,
    &lbl_80319D18,
    &lbl_80319D48,
    &lbl_80319D78,
    &gCameraModeNpcSpeakDescriptor,
    &lbl_80319E08,
    &lbl_80319E38,
    &lbl_80319E68,
    &lbl_80319E98,
    &lbl_80319EC8,
    &lbl_80319EF8,
    &dll_54,
    &lbl_80319F58,
    &lbl_80319F88,
    &lbl_8031A01C,
    &lbl_803137D8,
    &lbl_80311D88,
    &lbl_80311E0C,
    &lbl_80311E80,
    &lbl_8031210C,
    &lbl_8031231C,
    &lbl_8031262C,
    &lbl_80312770,
    &lbl_803128C4,
    &lbl_803129A8,
    &lbl_80312BB4,
    &lbl_80312CF8,
    &lbl_80312E38,
    &lbl_80312F78,
    &lbl_80313394,
    &lbl_803135A4,
    &lbl_803137B4,
    &lbl_80313880,
    &lbl_80313A1C,
    &lbl_80313AB0,
    &Dummy6C_funcs,
    &lbl_80313C10,
    &lbl_80313CA0,
    &lbl_80313E78,
    &lbl_8031403C,
    &lbl_80314268,
    &lbl_80314490,
    &lbl_803146B8,
    &lbl_803148FC,
    &lbl_80314930,
    &lbl_80314960,
    &lbl_80314990,
    &lbl_80314AD0,
    &lbl_80314BB0,
    &lbl_80314C90,
    &lbl_80314DE4,
    &lbl_80315010,
    &lbl_80315238,
    &lbl_80315304,
    &lbl_80315444,
    &lbl_80315528,
    &lbl_80315750,
    &lbl_80315978,
    &lbl_80315C84,
    &lbl_80315F84,
    &lbl_80316000,
    &lbl_80316030,
    &lbl_80316220,
    &lbl_80316440,
    &lbl_80316630,
    &lbl_80316708,
    &lbl_80316930,
    &lbl_80316B3C,
    &lbl_80316C20,
    &lbl_80316C70,
    &lbl_80316E0C,
    &lbl_80316FD4,
    &lbl_8031719C,
    &lbl_8031723C,
    &lbl_80317468,
    &lbl_80317504,
    &lbl_803175C8,
    &lbl_803177F0,
    &lbl_8031788C,
    &lbl_80317AD4,
    &lbl_80317B74,
    &lbl_80317BB8,
    &lbl_80317DE0,
    &lbl_80318014,
    &lbl_80318240,
    &lbl_80318468,
    &lbl_80318690,
    &lbl_803188B8,
    &lbl_80318AE0,
    &lbl_80318D08,
    &lbl_80313184,
    &lbl_80318D28,
    &lbl_80318DD0,
    &lbl_80318E20,
    &lbl_80318EC8,
    &lbl_80319008,
    &lbl_80319148,
    &lbl_80319354,
    &lbl_80319378,
    &lbl_803193C0,
    &lbl_80319410,
    &lbl_80319460,
    &lbl_803194A8,
    &lbl_803194F8,
    &lbl_80319548,
    &lbl_80319768,
    &lbl_80319598,
    &lbl_803196D8,
    &lbl_80319720,
    &lbl_803197B0,
    &lbl_803197F8,
    &lbl_803195E8,
    &lbl_80319638,
    &lbl_80319688,
    &lbl_80319840,
    &lbl_80319888,
    &lbl_803198D8,
    &lbl_80319920,
    &lbl_80319968,
    &lbl_803199B0,
    &lbl_803199F8,
    &lbl_80319A40,
    NULL,
    &gTrickyObjDescriptor,
    (ResourceDescriptor*)&lbl_80320700,
    (ResourceDescriptor*)&gAnimatedObjDescriptor,
    (ResourceDescriptor*)&gDIM2RoofRubObjDescriptor,
    (ResourceDescriptor*)&gDepthOfFieldPointObjDescriptor,
    &gBaddieObjDescriptor,
    (ResourceDescriptor*)&gIceBaddieObjDescriptor,
    (ResourceDescriptor*)&gDllCBObjDescriptor,
    (ResourceDescriptor*)&gChukChukObjDescriptor,
    (ResourceDescriptor*)&gIceBallObjDescriptor,
    (ResourceDescriptor*)&gDllCEObjDescriptor,
    (ResourceDescriptor*)&gCannonClawObjDescriptor,
    (ResourceDescriptor*)&gGrimbleObjDescriptor,
    (ResourceDescriptor*)&gTumbleWeedBushObjDescriptor,
    (ResourceDescriptor*)&gTumbleweedObjDescriptor,
    &dll_D3,
    (ResourceDescriptor*)&gSkeetlaWallObjDescriptor,
    (ResourceDescriptor*)&gKaldachomObjDescriptor,
    (ResourceDescriptor*)&gKaldaChompMeObjDescriptor,
    (ResourceDescriptor*)&gKaldaChompSpitObjDescriptor,
    (ResourceDescriptor*)&gPinPonSpikeObjDescriptor,
    (ResourceDescriptor*)&gPollenObjDescriptor,
    (ResourceDescriptor*)&gPollenFragmentObjDescriptor,
    (ResourceDescriptor*)&gMikaBombObjDescriptor,
    (ResourceDescriptor*)&gMikaBombShadowObjDescriptor,
    (ResourceDescriptor*)&gGCbaddieShieldObjDescriptor,
    (ResourceDescriptor*)&gBaddieInterestPObjDescriptor,
    (ResourceDescriptor*)&gHagabonObjDescriptor,
    (ResourceDescriptor*)&gSwarmBaddieObjDescriptor,
    (ResourceDescriptor*)&gWispBaddieObjDescriptor,
    &gStaffObjDescriptor,
    (ResourceDescriptor*)&gFireballObjDescriptor,
    (ResourceDescriptor*)&gFlameThrowerspeObjDescriptor,
    (ResourceDescriptor*)&gShieldObjDescriptor,
    (ResourceDescriptor*)&gReStartMarkObjDescriptor,
    (ResourceDescriptor*)&gFlammableVineObjDescriptor,
    (ResourceDescriptor*)&gCheckpoint4ObjDescriptor,
    (ResourceDescriptor*)&gSetuppointObjDescriptor,
    (ResourceDescriptor*)&gSideloadObjDescriptor,
    (ResourceDescriptor*)&gSiderepelObjDescriptor,
    (ResourceDescriptor*)&gInfoPointObjDescriptor,
    (ResourceDescriptor*)&gCollectibleObjDescriptor,
    (ResourceDescriptor*)&gEffectBoxObjDescriptor,
    (ResourceDescriptor*)&gPushableObjDescriptor,
    (ResourceDescriptor*)&gWarpPointObjDescriptor,
    (ResourceDescriptor*)&gInvHitObjDescriptor,
    (ResourceDescriptor*)&gIceblastObjDescriptor,
    (ResourceDescriptor*)&gFlameblastObjDescriptor,
    (ResourceDescriptor*)&gDoorF4ObjDescriptor,
    (ResourceDescriptor*)&gSidekickBallObjDescriptor,
    (ResourceDescriptor*)&gAreaObjDescriptor,
    (ResourceDescriptor*)&dll_F7,
    (ResourceDescriptor*)&gLevelNameObjDescriptor,
    (ResourceDescriptor*)&gProjectileSwitchObjDescriptor,
    (ResourceDescriptor*)&gInvisibleHitSwitchObjDescriptor,
    (ResourceDescriptor*)&gPressureSwitchFBObjDescriptor,
    (ResourceDescriptor*)&gDllFCObjDescriptor,
    (ResourceDescriptor*)&gDllFDObjDescriptor,
    (ResourceDescriptor*)&gMagicPlantObjDescriptor,
    (ResourceDescriptor*)&gMagicGemObjDescriptor,
    (ResourceDescriptor*)&gTrickyWarpObjDescriptor,
    (ResourceDescriptor*)&gTrickyGuardObjDescriptor,
    (ResourceDescriptor*)&gStayPointObjDescriptor,
    (ResourceDescriptor*)&gCurveFishObjDescriptor,
    (ResourceDescriptor*)&gSmallBasketObjDescriptor,
    (ResourceDescriptor*)&gLargeCrateObjDescriptor,
    (ResourceDescriptor*)&gScarabObjDescriptor,
    (ResourceDescriptor*)&gWindLift107ObjDescriptor,
    (ResourceDescriptor*)&gEndObjectObjDescriptor,
    (ResourceDescriptor*)&gBreakableCarryableObjDescriptor,
    (ResourceDescriptor*)&gFall_LaddersObjDescriptor,
    (ResourceDescriptor*)&gFireFlyLanternObjDescriptor,
    (ResourceDescriptor*)&gLanternFireFlyObjDescriptor,
    (ResourceDescriptor*)&gPortalSpellDoorObjDescriptor,
    (ResourceDescriptor*)&gDeathSeqObjDescriptor,
    (ResourceDescriptor*)&gMMP_BridgeObjDescriptor,
    (ResourceDescriptor*)&gDoorObjDescriptor,
    (ResourceDescriptor*)&gDoorLockObjDescriptor,
    (ResourceDescriptor*)&gSeqObjectObjDescriptor,
    (ResourceDescriptor*)&gSeqObj2ObjDescriptor,
    (ResourceDescriptor*)&gIMMultiSeqObjDescriptor,
    (ResourceDescriptor*)&gDll115ObjDescriptor,
    (ResourceDescriptor*)&gWM_ColumnObjDescriptor,
    (ResourceDescriptor*)&gAppleOnTreeObjDescriptor,
    (ResourceDescriptor*)&gDusterObjDescriptor,
    (ResourceDescriptor*)&gColdWaterControlObjDescriptor,
    (ResourceDescriptor*)&gDecoration11AObjDescriptor,
    (ResourceDescriptor*)&gLanded_ArwingObjDescriptor,
    (ResourceDescriptor*)&gStaffActivatedObjDescriptor,
    (ResourceDescriptor*)&gTreasureChestObjDescriptor,
    (ResourceDescriptor*)&gMagicCaveBottomObjDescriptor,
    (ResourceDescriptor*)&gMagicCaveTopObjDescriptor,
    (ResourceDescriptor*)&gTrickyGuardSpotObjDescriptor,
    (ResourceDescriptor*)&gInfoTextObjDescriptor,
    (ResourceDescriptor*)&gCCTestInfotObjDescriptor,
    (ResourceDescriptor*)&gFuelCellObjDescriptor,
    (ResourceDescriptor*)&gDeathGasObjDescriptor,
    (ResourceDescriptor*)&gCurveObjDescriptor,
    &gTriggerObjDescriptor,
    (ResourceDescriptor*)&gDll127ObjDescriptor,
    (ResourceDescriptor*)&gKT_TorchObjDescriptor,
    (ResourceDescriptor*)&gCampFireObjDescriptor,
    (ResourceDescriptor*)&gCFCrateObjDescriptor,
    (ResourceDescriptor*)&gFXEmitObjDescriptor,
    (ResourceDescriptor*)&gTransporterObjDescriptor,
    (ResourceDescriptor*)&gLFXEmitterObjDescriptor,
    (ResourceDescriptor*)&gCFLightWallObjDescriptor,
    (ResourceDescriptor*)&gBarrelPadObjDescriptor,
    (ResourceDescriptor*)&gAreaFXEmitObjDescriptor,
    (ResourceDescriptor*)&gCF_DoorLightObjDescriptor,
    (ResourceDescriptor*)&gWaterFallSprayObjDescriptor,
    (ResourceDescriptor*)&gSfxPlayerObjDescriptor,
    (ResourceDescriptor*)&gTexscroll2ObjDescriptor,
    (ResourceDescriptor*)&gTexscrollObjDescriptor,
    (ResourceDescriptor*)&gWaveAnimatorObjDescriptor,
    (ResourceDescriptor*)&gAlphaAnimatorObjDescriptor,
    (ResourceDescriptor*)&gGroundAnimatorObjDescriptor,
    (ResourceDescriptor*)&gHitAnimatorObjDescriptor,
    (ResourceDescriptor*)&gVisAnimatorObjDescriptor,
    (ResourceDescriptor*)&gWallAnimatorObjDescriptor,
    (ResourceDescriptor*)&gXYZAnimatorObjDescriptor,
    (ResourceDescriptor*)&gExplodeAnimatorObjDescriptor,
    (ResourceDescriptor*)&gDIMBossIceSmashObjDescriptor,
    (ResourceDescriptor*)&gTexFrameAnimatorObjDescriptor,
    (ResourceDescriptor*)&gFogControlObjDescriptor,
    (ResourceDescriptor*)&gLightningObjDescriptor,
    (ResourceDescriptor*)&gFElevControlObjDescriptor,
    (ResourceDescriptor*)&gFEseqobjectObjDescriptor,
    (ResourceDescriptor*)&gDll144ObjDescriptor,
    (ResourceDescriptor*)&gCloudPrisonControlObjDescriptor,
    (ResourceDescriptor*)&gCloudShipControlNullResourceDescriptor,
    (ResourceDescriptor*)&gDll147NullResourceDescriptor,
    (ResourceDescriptor*)&gCFGuardianObjDescriptor,
    (ResourceDescriptor*)&gWindLiftObjDescriptor,
    (ResourceDescriptor*)&gCFPowerBaseObjDescriptor,
    (ResourceDescriptor*)&gCFMainCrystalObjDescriptor,
    (ResourceDescriptor*)&gBabyCloudRunnerObjDescriptor,
    (ResourceDescriptor*)&gLaserBeamNullResourceDescriptor,
    (ResourceDescriptor*)&gCFPrisonGuardObjDescriptor,
    (ResourceDescriptor*)&gCFPrisonUncleObjDescriptor,
    (ResourceDescriptor*)&gGCRobotLightBeamObjDescriptor,
    (ResourceDescriptor*)&gCFScalesGalNullResourceDescriptor,
    (ResourceDescriptor*)&gCFObjCreatNullResourceDescriptor,
    (ResourceDescriptor*)&gCFPerchObjDescriptor,
    (ResourceDescriptor*)&gCFPrisonCageObjDescriptor,
    (ResourceDescriptor*)&gDll155NullResourceDescriptor,
    (ResourceDescriptor*)&gDll156NullResourceDescriptor,
    (ResourceDescriptor*)&gSpiritDoorSpiritObjDescriptor,
    (ResourceDescriptor*)&gGunpowderBarrelObjDescriptor,
    (ResourceDescriptor*)&gBlastedObjDescriptor,
    (ResourceDescriptor*)&gExplodableObjDescriptor,
    (ResourceDescriptor*)&gCFForceFieldObjDescriptor,
    (ResourceDescriptor*)&gCFForceField15CNullResourceDescriptor,
    (ResourceDescriptor*)&gSlidingDoorObjDescriptor,
    (ResourceDescriptor*)&gDll350NullResourceDescriptor,
    (ResourceDescriptor*)&gAttractorObjDescriptor,
    (ResourceDescriptor*)&gDll352NullResourceDescriptor,
    (ResourceDescriptor*)&gCFTreasRoboNullResourceDescriptor,
    (ResourceDescriptor*)&gCFMagicWallObjDescriptor,
    (ResourceDescriptor*)&gDll355NullResourceDescriptor,
    (ResourceDescriptor*)&gCFLevelControlObjDescriptor,
    (ResourceDescriptor*)&gCFRemovalShNullResourceDescriptor,
    (ResourceDescriptor*)&gExplodedObjDescriptor,
    (ResourceDescriptor*)&gSpiritDoorLockObjDescriptor,
    (ResourceDescriptor*)&gHoloPointNullResourceDescriptor,
    (ResourceDescriptor*)&gIMIceMountainObjDescriptor,
    (ResourceDescriptor*)&gCRrockfallObjDescriptor,
    (ResourceDescriptor*)&gMagicLightObjDescriptor,
    (ResourceDescriptor*)&gIMSnowClawObjDescriptor,
    (ResourceDescriptor*)&gIMIcePillarObjDescriptor,
    (ResourceDescriptor*)&gIMAnimSpaceObjDescriptor,
    (ResourceDescriptor*)&gIMSpaceThrusterObjDescriptor,
    (ResourceDescriptor*)&gIMSpaceRingObjDescriptor,
    (ResourceDescriptor*)&gIMSpaceRingGeneratorObjDescriptor,
    (ResourceDescriptor*)&gLINKBLevelControlObjDescriptor,
    (ResourceDescriptor*)&gLINKLevelControlObjDescriptor,
    (ResourceDescriptor*)&gCCRiverFlowObjDescriptor,
    &gDFropenodeObjDescriptor,
    (ResourceDescriptor*)&gDFSH_Door1SNullResourceDescriptor,
    (ResourceDescriptor*)&gDll177ObjDescriptor,
    (ResourceDescriptor*)&gDFSHShrineObjDescriptor,
    (ResourceDescriptor*)&gDFSHObjCreatorObjDescriptor,
    (ResourceDescriptor*)&gSpiritPrizeObjDescriptor,
    (ResourceDescriptor*)&gDFSHLaserBeamObjDescriptor,
    (ResourceDescriptor*)&gGCRobotPatrNullResourceDescriptor,
    (ResourceDescriptor*)&gRollingBarrelObjDescriptor,
    (ResourceDescriptor*)&gMMPLevelControlObjDescriptor,
    (ResourceDescriptor*)&gMoonSeedBushObjDescriptor,
    (ResourceDescriptor*)&gMMPAsteroidReObjDescriptor,
    (ResourceDescriptor*)&gMMPTrenchFxObjDescriptor,
    (ResourceDescriptor*)&gMMPMoonRockObjDescriptor,
    (ResourceDescriptor*)&gMMPGeyserVentObjDescriptor,
    (ResourceDescriptor*)&gDll184ObjDescriptor,
    (ResourceDescriptor*)&gCCGasVentObjDescriptor,
    (ResourceDescriptor*)&gCCGasVentControlObjDescriptor,
    (ResourceDescriptor*)&gCCQueenObjDescriptor,
    (ResourceDescriptor*)&gCCLightfootObjDescriptor,
    (ResourceDescriptor*)&gCCSharpClawPadObjDescriptor,
    (ResourceDescriptor*)&gCCPedestalObjDescriptor,
    (ResourceDescriptor*)&gCCLevelControlObjDescriptor,
    (ResourceDescriptor*)&gMMSHShrineObjDescriptor,
    (ResourceDescriptor*)&gMMSHScalesObjDescriptor,
    (ResourceDescriptor*)&gMMSHWaterSpikeObjDescriptor,
    (ResourceDescriptor*)&gECSHShrineObjDescriptor,
    (ResourceDescriptor*)&gECSHCupObjDescriptor,
    (ResourceDescriptor*)&gECSHCreatorObjDescriptor,
    (ResourceDescriptor*)&gGPSHShrineObjDescriptor,
    (ResourceDescriptor*)&gGPSHObjCreatorObjDescriptor,
    (ResourceDescriptor*)&gGPSHSceneObjDescriptor,
    (ResourceDescriptor*)&gDBSHShrineObjDescriptor,
    (ResourceDescriptor*)&gDBSHSymbolObjDescriptor,
    (ResourceDescriptor*)&gDll407ObjDescriptor,
    (ResourceDescriptor*)&gNWSHLevelControlObjDescriptor,
    (ResourceDescriptor*)&gDll409ObjDescriptor,
    (ResourceDescriptor*)&gDll410ObjDescriptor,
    (ResourceDescriptor*)&gDll411ObjDescriptor,
    (ResourceDescriptor*)&gDll412ObjDescriptor,
    (ResourceDescriptor*)&gDll413ObjDescriptor,
    (ResourceDescriptor*)&gDll414ObjDescriptor,
    (ResourceDescriptor*)&gNWTreeBridgeObjDescriptor,
    (ResourceDescriptor*)&gNWGeyserObjDescriptor,
    (ResourceDescriptor*)&gNW_mammothObjDescriptor,
    (ResourceDescriptor*)&gNWTrickyObjDescriptor,
    (ResourceDescriptor*)&gDll419ObjDescriptor,
    (ResourceDescriptor*)&gNW_iceObjDescriptor,
    (ResourceDescriptor*)&gNWLevelControlObjDescriptor,
    (ResourceDescriptor*)&gSHTrickyObjDescriptor,
    (ResourceDescriptor*)&gEdibleMushroomObjDescriptor,
    (ResourceDescriptor*)&gEnemyMushroomObjDescriptor,
    (ResourceDescriptor*)&gBombPlantObjDescriptor,
    (ResourceDescriptor*)&gBombPlantSporeObjDescriptor,
    (ResourceDescriptor*)&gBombPlantingSpotObjDescriptor,
    (ResourceDescriptor*)&gSH_queenearthwalkerObjDescriptor,
    (ResourceDescriptor*)&gSH_thorntailObjDescriptor,
    (ResourceDescriptor*)&gSH_LevelControlObjDescriptor,
    (ResourceDescriptor*)&gWarpStoneLiftObjDescriptor,
    (ResourceDescriptor*)&gWarpStoneObjDescriptor,
    (ResourceDescriptor*)&gSH_staffObjDescriptor,
    (ResourceDescriptor*)&gSH_staffHazeObjDescriptor,
    (ResourceDescriptor*)&gSH_BeaconObjDescriptor,
    (ResourceDescriptor*)&gSH_EmptyTumbleWObjDescriptor,
    (ResourceDescriptor*)&gDll437ObjDescriptor,
    (ResourceDescriptor*)&gSC_levelcontrolObjDescriptor,
    (ResourceDescriptor*)&gSC_MusicTreeObjDescriptor,
    (ResourceDescriptor*)&gSC_totempoleObjDescriptor,
    (ResourceDescriptor*)&gSC_CloudrunnerAObjDescriptor,
    (ResourceDescriptor*)&gSC_totempuzzleObjDescriptor,
    (ResourceDescriptor*)&gSC_totembondObjDescriptor,
    (ResourceDescriptor*)&gSC_totemstrengthObjDescriptor,
    (ResourceDescriptor*)&gPaymentKioskObjDescriptor,
    (ResourceDescriptor*)&gLavaBall1BEObjDescriptor,
    (ResourceDescriptor*)&gLavaBall1BFObjDescriptor,
    (ResourceDescriptor*)&gDIMLogFireObjDescriptor,
    (ResourceDescriptor*)&gDIMSnowBallObjDescriptor,
    (ResourceDescriptor*)&gDIMSnowBall1C2ObjDescriptor,
    (ResourceDescriptor*)&gDIMGateObjDescriptor,
    (ResourceDescriptor*)&gDIMIceWallObjDescriptor,
    (ResourceDescriptor*)&gDIMBarrierObjDescriptor,
    (ResourceDescriptor*)&gDIMCannonObjDescriptor,
    (ResourceDescriptor*)&gDIMLavaSmashObjDescriptor,
    (ResourceDescriptor*)&gDIMBridgeCogMaiObjDescriptor,
    (ResourceDescriptor*)&gDIMDismountPointObjDescriptor,
    &gExplosionObjDescriptor,
    &gDIMWoodDoor2ObjDescriptor,
    &gDIMMagicBridgeObjDescriptor,
    &gDIM_LevelControlObjDescriptor,
    &dll_1CE,
    &dll_1CF,
    &gDIM_trickyObjDescriptor,
    &gDIMTruthHornIceObjDescriptor,
    &gWorldPlanetObjDescriptor,
    &gWorldObjObjDescriptor,
    &gWorldAsteroidsObjDescriptor,
    &gDIM2ConveyorObjDescriptor,
    &dll_1D6,
    &gDIM2SnowBallObjDescriptor,
    &gDIM2PathGeneratorObjDescriptor,
    &gDIM2PrisonMammothObjDescriptor,
    &dll_1DA,
    &dll_1DB,
    &gDIM2IceFloeObjDescriptor,
    &gDIM2IcicleObjDescriptor,
    &gDIM2LavaControlObjDescriptor,
    &gDll1DFObjDescriptor,
    &gDIM_BossObjDescriptor,
    &gDIM_BossGutObjDescriptor,
    &gDIM_BossTonsilObjDescriptor,
    &gDIM_BossGut2ObjDescriptor,
    &gMAGICMakerObjDescriptor,
    &gDIM_BossSpitObjDescriptor,
    &gDIMbosscrackparObjDescriptor,
    &gDIMbossfireObjDescriptor,
    &gSB_GalleonObjDescriptor,
    &gSB_PropellerObjDescriptor,
    &gSB_ShipHeadObjDescriptor,
    &gSB_ShipMastObjDescriptor,
    &gSB_ShipGunObjDescriptor,
    &gSB_FireBallObjDescriptor,
    &gSB_CannonBallObjDescriptor,
    &gSB_CloudBallObjDescriptor,
    &gSB_KyteCageObjDescriptor,
    &gSB_SeqDoorObjDescriptor,
    &gSB_CageKyteObjDescriptor,
    &gSB_MiniFireObjDescriptor,
    &gLampObjDescriptor,
    &gShipBattleObjDescriptor,
    &gFlagObjDescriptor,
    &gSB_ShipGunBrokeObjDescriptor,
    &gWM_GalleonObjDescriptor,
    &gWM_ObjCreatorObjDescriptor,
    &gWM_seqobjectObjDescriptor,
    &dll_1FB,
    &gLaserBeamObjDescriptor,
    &gWM_LaserTargetObjDescriptor,
    &gPressureSwitchObjDescriptor,
    &dll_1FF,
    &dll_200,
    &gWM_colriseObjDescriptor,
    &lbl_803DC0F8,
    &lbl_803DC100,
    &gWM_TorchObjDescriptor,
    &lbl_80328AD8,
    &gLightSourceObjDescriptor,
    &gWM_WormObjDescriptor,
    &lbl_803DC108,
    &gWM_LevelControlObjDescriptor,
    &gWM_GeneralScalesObjDescriptor,
    &gFireFlyObjDescriptor,
    &gWM_spiritplaceObjDescriptor,
    &gWM_seqpointObjDescriptor,
    &gWM_sunObjDescriptor,
    &gWM_SpiritSetObjDescriptor,
    &gWM_PlanetsObjDescriptor,
    &gWM_WallCrawlerObjDescriptor,
    &lbl_803DC138,
    &lbl_80328E28,
    &lbl_803DC140,
    &gWM_newcrystalObjDescriptor,
    &gVFP_LevelControlObjDescriptor,
    &gVFP_ObjCreatorObjDescriptor,
    &gVFP_MiniFireObjDescriptor,
    &dll_219,
    &gVFP_statueballObjDescriptor,
    &dll_21B,
    &gVFP_LaddersObjDescriptor,
    &gVFPLiftObjDescriptor,
    &gVFP_Block1ObjDescriptor,
    &gVFP_PlatformObjDescriptor,
    &gVFP_DoorSwitchObjDescriptor,
    &gSeqPointObjDescriptor,
    &gVFPDragHeadObjDescriptor,
    &gVFP_coreplatObjDescriptor,
    &dll_224,
    &gVFP_flamepointObjDescriptor,
    &gVFP_lavapoolObjDescriptor,
    &gVFP_lavastarObjDescriptor,
    &gVFP_SpellPlaceObjDescriptor,
    &gDFP_LevelControlObjDescriptor,
    &gDFP_ObjCreatorObjDescriptor,
    &gDFP_TorchObjDescriptor,
    &gDll22CObjDescriptor,
    &gDFP_seqpointObjDescriptor,
    &gDoorswitchObjDescriptor,
    &gDfpfloorbarObjDescriptor,
    &gChukaObjDescriptor,
    &gTrickyCurveObjDescriptor,
    &gSfxplayerObjDescriptor,
    &gDfpstatue1ObjDescriptor,
    &gDfperchwitchObjDescriptor,
    &gDfptargetblockObjDescriptor,
    &gLaserUnsupportedObjDescriptor,
    &gLaserObjDescriptor,
    &gFireObjDescriptor,
    &gTextBlockObjDescriptor,
    &gPlatform1ObjDescriptor,
    &gDfplightniObjDescriptor,
    &gDfppowerslObjDescriptor,
    &lbl_803DC150,
    &lbl_803DC158,
    &gDB_eggObjDescriptor,
    &lbl_80329340,
    &gDrakorEnergyObjDescriptor,
    &gDBstealerwormObjDescriptor,
    &gDBHoleControl1ObjDescriptor,
    &Dummy244,
    &Dummy245,
    &Dummy246,
    &Dummy247,
    &Dummy248,
    &lbl_803DC6E8,
    &Dummy24A,
    &Dummy24B,
    &lbl_8032A110,
    &gBossDrakorObjDescriptor,
    &gDrakorDThornBushObjDescriptor,
    &gKtRexLevelObjDescriptor,
    &gKtRexObjDescriptor,
    &gKtRexFloorSwitchObjDescriptor,
    &gKtLazerwallObjDescriptor,
    &gKtLazerlightObjDescriptor,
    &gKtFallingrocksObjDescriptor,
    &gSnowBikeObjDescriptor,
    &gDIMSnowHorn1ObjDescriptor,
    &gDR_EarthWarriorObjDescriptor,
    &gDR_CloudRunnerObjDescriptor,
    &gSB_CloudRunnerObjDescriptor,
    &gStaticCameraObjDescriptor,
    &gMoonSeedPlantingSpotObjDescriptor,
    &gSnowClawObjDescriptor,
    &gCrCloudRaceObjDescriptor,
    &gSpellStoneObjDescriptor,
    &gCrFuelTankObjDescriptor,
    &gProximityMineObjDescriptor,
    &gDrLaserCannonObjDescriptor,
    &gDrakorMissileObjDescriptor,
    &gGmMazeWellObjDescriptor,
    &lbl_803DC2C0,
    &gDrCreatorObjDescriptor,
    &gKytesMumObjDescriptor,
    &lbl_803DC2D8,
    &gDrCageControlObjDescriptor,
    &gExplodePlanObjDescriptor,
    &lbl_803DC2E0,
    &gDrChimmeyObjDescriptor,
    &gDrCageWithObjDescriptor,
    &lbl_803DC2E8,
    &gDrShackleObjDescriptor,
    &gDrGeneratorObjDescriptor,
    &lbl_803DC338,
    &gDrakorHoverPadObjDescriptor,
    &gHighTopObjDescriptor,
    &gFirePipeObjDescriptor,
    &lbl_803DC360,
    &lbl_803DC358,
    &lbl_803DC368,
    &lbl_803DC370,
    &lbl_803DC378,
    &gDrEnergyDiscObjDescriptor,
    &lbl_803DC388,
    &lbl_8032AD00,
    &gDrLightBeaObjDescriptor,
    &lbl_8032AD68,
    &gDrMusicContObjDescriptor,
    &lbl_803DC390,
    &gDrCloudPerObjDescriptor,
    (ResourceDescriptor*)&gDrEarthCalObjDescriptor,
    (ResourceDescriptor*)&gBarrelGenerObjDescriptor,
    &gDrBarrelGrObjDescriptor,
    &gShopItemObjDescriptor,
    &gShopObjDescriptor,
    &gShopKeeperObjDescriptor,
    &gSPScarabObjDescriptor,
    &gSPDrapeObjDescriptor,
    &gSPitembeamObjDescriptor,
    &gEarthWalkerObjDescriptor,
    &dll_28B,
    &gWCBouncyCraObjDescriptor,
    &gWCLevelContObjDescriptor,
    (ResourceDescriptor*)&gWCBeaconObjDescriptor,
    (ResourceDescriptor*)&gWCPressureSObjDescriptor,
    &gWCPushBlockObjDescriptor,
    &gWCTileObjDescriptor,
    (ResourceDescriptor*)&gWCTrexStatuObjDescriptor,
    (ResourceDescriptor*)&gSunTempleObjDescriptor,
    (ResourceDescriptor*)&gWCTempleObjDescriptor,
    (ResourceDescriptor*)&gWCApertureSObjDescriptor,
    (ResourceDescriptor*)&gWCTempleDiaObjDescriptor,
    &gWCTempleBriObjDescriptor,
    (ResourceDescriptor*)&gWCFloorTileObjDescriptor,
    (ResourceDescriptor*)&dll_299,
    (ResourceDescriptor*)&gARWArwingObjDescriptor,
    (ResourceDescriptor*)&gArwingAndrossStuffObjDescriptor,
    (ResourceDescriptor*)&gARWArwingBoObjDescriptor,
    (ResourceDescriptor*)&gARWArwingGuObjDescriptor,
    &gDll29EObjDescriptor,
    &gARWBombCollObjDescriptor,
    &gRingObjDescriptor,
    (ResourceDescriptor*)&gARWLevelConObjDescriptor,
    &gARWSpeedStrObjDescriptor,
    &dll_2A3,
    &dll_2A4,
    &gARWGeneratoObjDescriptor,
    &gARWSquadronObjDescriptor,
    &gARWProximitObjDescriptor,
    &gARWBlockerObjDescriptor,
    &gPointLightObjDescriptor,
    &gDirectionalLightObjDescriptor,
    &gProjectedLightObjDescriptor,
    &gControlLightObjDescriptor,
    &gSoftBodyObjDescriptor,
    &gWaterFlowWeObjDescriptor,
    (ResourceDescriptor*)&gTreeObjDescriptor,
    (ResourceDescriptor*)&gBrokenPipeObjDescriptor,
    (ResourceDescriptor*)&gCmbSrcObjDescriptor,
    (ResourceDescriptor*)&gDustMoteSouObjDescriptor,
    &gVortexObjDescriptor,
    (ResourceDescriptor*)&gCNTcounterObjDescriptor,
    &gTimerObjDescriptor,
    (ResourceDescriptor*)&gCNThitObjecObjDescriptor,
    (ResourceDescriptor*)&gMCUpgradeObjDescriptor,
    (ResourceDescriptor*)&gMCUpgradeMaObjDescriptor,
    (ResourceDescriptor*)&gMCStaffEffeObjDescriptor,
    (ResourceDescriptor*)&gMCLightningObjDescriptor,
    (ResourceDescriptor*)&gGF_LevelConObjDescriptor,
    (ResourceDescriptor*)&gAndrossObjDescriptor,
    (ResourceDescriptor*)&gAndrossHandObjDescriptor,
    (ResourceDescriptor*)&gAndrossBrainObjDescriptor,
    (ResourceDescriptor*)&gAndrossLighObjDescriptor,
    &gTitleScreenObjDescriptor,
    NULL,
};

s32 gModelEngineUiDllResourceIds[] = {
    -1, 16, 50, 51, 52, 53, 54, 55, 56, 57, -1, -1, 58, -1, 63, 64, 65, -1,
};
